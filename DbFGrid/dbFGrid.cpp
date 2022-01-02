#include "dbFGrid.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

static bool sDebug = false;

/* ----------------------------------------------------------------------- */
dbFGrid::dbFGrid(unsigned int sampleRate)
{
    this->m_sampleRate = sampleRate;
}

dbFGrid::~dbFGrid()
{
}

int
dbFGrid::Open(std::string const &fnm)
{
    int err = 0;
    this->m_currentTime = 0;
    this->m_bbox[0] = 1e6;
    this->m_bbox[1] = 0;
    this->m_columnUnit = .25f; // size of a column (here 1/4 note)
    this->m_signature[0] = 4;
    this->m_signature[1] = 4;  // size of a bat (here 1/4 note)
    this->m_colToBeat = 1.f; // converts columns to beats (here 1)
    this->m_channelPool.clear();
    this->m_channelsInUse.clear();
    for(unsigned i=0;i<15;i++)
    {
        // below, we access new elements from back. For "cosmetics"
        // we'll initialize the pool so that low-numbered channels
        // come online before high-numbered channels
        this->m_channelPool.push_back(15-i);
    }    

    std::ifstream fstr;
    fstr.open(fnm.c_str());
    if(fstr.good())
    {
        try
        {
            // Because json.hpp accesses everything *by value*, we
            // opt not to live-parse through the events array, rather
            // we'll parse once and free the results.  We *do* copy the
            // optional paramlists out.
            json j;
            fstr >> j;
            if(j.contains("fmatrix"))
            {
                std::cerr << "DbFGrid opened " << fnm << "\n";
                auto jfm = j["fmatrix"];
                this->m_params = jfm["params"];
                float beatSize;
                if(this->m_params.count("signature"))
                {
                    this->m_signature[0] = this->m_params["signature"]["x"].get<int>();
                    beatSize = this->m_params["signature"]["y"].get<float>();
                    this->m_signature[1] = int(.5f + 1.f/ beatSize);
                }
                else
                    beatSize = 1.0f / this->m_signature[1];
                if(this->m_params.count("columnUnit"))
                    this->m_columnUnit = this->m_params["columnUnit"].get<float>();
                // example: 1/8 colunit, 1/4 sigDenom => colToBeat: .5 
                this->m_colToBeat = this->m_columnUnit / beatSize; 
                auto jlayers = jfm["layers"];

                // foreach layer
                for(int il=0;il<jlayers.size(); il++)
                {
                    auto jl = jlayers.at(il);
                    auto jelist = jl["events"]; // array of objects
                    if(jelist.size() == 0)
                        continue;

                    auto fnm = jl["fnm"];
                    layer::layerType lt;
                    if(fnm == "MidiNote")
                        lt = layer::k_noteLayer;
                    else
                    if(fnm == "MidiCC")
                        lt = layer::k_ccLayer;
                    else
                    {
                        std::cerr << "DbFGrid unimplemented layer type " << fnm << "\n";
                        continue;
                    }
                    this->m_layers.push_back(layer(lt));
                    layer &l = this->m_layers.back();
                    l.fparams = jl["fparams"];
                    l.defaultValue = l.fparams.count("v") ? l.fparams["v"].get<float>() : .75f;
                    l.defaultID = l.fparams.count("id") ? l.fparams["id"].get<int>() : 0;
                    l.bbox[0] = 1e10f;
                    l.bbox[1] = 0;
                    l.events.reserve(jelist.size()); // more when subevents

                    // foreach event
                    for(int j=0;j<jelist.size();j++)
                    {
                        auto je = jelist[j]; // an event object
                        // No order guarantees for je keys, we need "_" first
                        // in order to process subevents.
                        auto jeLoc = je["_"]; // a location array
                        float ebegin = jeLoc[0].get<float>();
                        float edur = jeLoc[1].get<float>();
                        int erow = jeLoc[2].get<int>();
                        unsigned eIndex = l.events.size(); // before we add one
                        l.events.push_back(event());

                        // NB: this ref is only valid 'til array resizing happens
                        event &e = l.events[eIndex];
                        e.start = ebegin;
                        e.end = ebegin + edur;
                        e.row = erow;
                        if(e.start < l.bbox[0])
                            l.bbox[0] = e.start;
                        if(e.end > l.bbox[1])
                            l.bbox[1] = e.end;
                        
                        // foreach fevent
                        for (json::iterator it = je.begin(); it != je.end(); ++it) 
                        {
                            if(it.key() == "_")
                                continue;
                            if(it.key() == "_se")
                            {
                                // dictionary whose keys are the CCid and
                                // whose values are an array of subevent 
                                // objects (CCs associated with a note)
                                // these are flattened into the total event list.
                                auto seobj = it.value();
                                for(json::iterator seit = seobj.begin(); seit != seobj.end(); ++seit)
                                {
                                    auto selist = seit.value();
                                    // foreach subevent
                                    l.events.reserve(l.events.size() + selist.size());
                                    for(int k=0;k<selist.size();k++)
                                    {
                                        l.events.push_back(event());
                                        event &se = l.events.back();
                                        se.subEvent = true;
                                        auto jse = selist[k]; // a subevent object
                                        // iterate its keys
                                        for (json::iterator itSub = jse.begin(); 
                                            itSub != jse.end(); ++itSub) 
                                        {
                                            auto seKey = itSub.key();
                                            auto seVal = itSub.value();
                                            if(seKey == "_")
                                            {
                                                // sub-event coords are relative
                                                // to parent (ie on [0,1])
                                                se.start = ebegin + edur*seVal[0].get<float>();
                                                se.end = se.start + edur*seVal[1].get<float>();
                                                se.row = erow;

                                                // Our xsize is bounded  by our 
                                                // parent event, so we don't update
                                                // l.bbox
                                            }
                                            else
                                                se.params[seKey] = seVal;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                l.events[eIndex].params[it.key()] = it.value();
                                // e.params[it.key()] = it.value();
                            }
                        }
                    }
                    if(this->m_bbox[0] > l.bbox[0])
                        this->m_bbox[0] = l.bbox[0];
                    if(this->m_bbox[1] < l.bbox[1])
                        this->m_bbox[1] = l.bbox[1];
                    l.ComputeEdgeOrdering();
                }
            }
            else
            {
                std::cerr << "DbFGrid fail opening " << fnm << "\n";
                err = 1;
            }
            
            if(err == 0 && sDebug)
            {
                this->dumpMatrix();
            }
        }
        catch(json::parse_error &e)
        {
            std::cerr << "DbFGrid error: " << e.what() << '\n'
                  << "exception id: " << e.id << std::endl;
            std::cerr << "While parsing " << fnm << "\n";

        }
        fstr.close();
    }
    else
    {
        std::cerr << "DbFGrid: " << fnm << " not found\n";
        err = -1;
    }
    return err;
}

int
dbFGrid::GetNumLayers()
{
    return (int) this->m_layers.size();
}

int
dbFGrid::GetBarSize()
{
    return this->m_signature[0];
}

int
dbFGrid::GetBeatSize()
{
    return this->m_signature[1];
}

int
dbFGrid::Rewind()
{
    for(layer &l : this->m_layers)
        l.Rewind();
    this->m_currentTime = 0;
    return 0;
}

/* return 0 on success, non-zero when we reach the end 
 */
int 
dbFGrid::Read(Event *evt, int soloLayer)
{
    if(this->m_currentTime > (this->m_bbox[1]+kEpsilon))
        return -1;

    // first nominate the best next event defined as the one whose
    // start or end is closest to this->m_currentTime
    float minDist = 1e6;
    int nextLIndex = -1;
    for(int i=0;i<this->m_layers.size();i++)
    {
        if(soloLayer != -1 && i != soloLayer)
            continue;

        layer &l = this->m_layers[i];
        if(this->m_currentTime > l.bbox[1])
          continue;
        
        float dist = l.NextDistance(this->m_currentTime);
        if(dist < minDist)
        {
            minDist = dist;
            nextLIndex = i;
        }
    }
    if(nextLIndex == -1)
        return -2;
    else
    {
        evt->layer = nextLIndex;
        if(minDist > kEpsilon)
        {
            evt->eType = Event::k_Wait;
            evt->value = minDist * this->m_colToBeat;
            evt->note = 0;
            evt->ccID = 0;
            this->m_currentTime += minDist; // time in "columns"
        }
        else
        {
            layer &l = this->m_layers[nextLIndex];
            l.GetEvent(this->m_currentTime, evt);
            switch(evt->eType)
            {
            case Event::k_NoteOn:
                evt->chan = this->allocateChannel((unsigned) evt->note, nextLIndex);
                break;
            case Event::k_NoteOff:
                evt->chan = this->findChannel((unsigned) evt->note, nextLIndex, true);
                break;
            case Event::k_CC:
                evt->chan = this->findChannel(evt->note, nextLIndex, false);
                break;
            }
        }
    }
    return 0;
}

unsigned
dbFGrid::allocateChannel(unsigned note, unsigned layer)
{
    unsigned chan = 0;
    if(this->m_channelPool.size() > 0)
    {
        chan = this->m_channelPool.back();
        this->m_channelPool.pop_back();

        unsigned key = (layer << 7) | note;
        if(this->m_channelsInUse.count(key) != 0)
        {
            std::cerr << "dbFGrid MIDI/MPE channel write botch for note " 
                      << note << "\n";
        }
        this->m_channelsInUse[key] = chan;
    }
    else
        std::cerr << "dbFGrid out of MIDI/MPE channels for note " << note << "\n";
    return chan;
}

unsigned
dbFGrid::findChannel(unsigned note, unsigned layer, bool release)
{
    unsigned chan = 0;
    unsigned key = (layer << 7) | note;
    if(this->m_channelsInUse.count(key) != 0)
    {
        auto it = this->m_channelsInUse.find(key);
        chan = it->second;
        if(release)
        {
            this->m_channelsInUse.erase(it);
            this->m_channelPool.push_back(chan);
        }
    }
    else
    {
        std::cerr << "dbFGrid MIDI/MPE channel read botch for note " 
                 << note << "\n";
    }
    return chan;
}

void
dbFGrid::layer::ComputeEdgeOrdering()
{
    int nEdges = this->events.size() * 2;
    this->orderedEdges.reserve(nEdges);
    for(int i=0;i<nEdges;i++)
    {
        // start with serialized events, event is orderedEdges index/2
        // and start/end is even/odd
        this->orderedEdges.push_back(i); 
    }

#if 0
    bool x = operator()(202, 353);
    bool y = operator()(353, 202);
    assert(x == !y);
#endif

    std::sort(this->orderedEdges.begin(), this->orderedEdges.end(), *this);
}

bool 
dbFGrid::layer::operator()(int a, int b)
{
    event &aEvt = this->events[a/2];
    event &bEvt = this->events[b/2];
    bool aStart = !(a&1);
    bool bStart = !(b&1);
    float ax = aStart ? aEvt.start : aEvt.end;
    float bx = bStart ? bEvt.start : bEvt.end;
    float dx = fabs(ax - bx);
    if(dx < .0001f)
    {
        // tie breaker
        if(aStart && bStart)
        {
            // parent Begin before sub Begin
            if(!aEvt.subEvent && bEvt.subEvent)
                return true; 
            else
                return false; 
        }
        else
        if(aStart || bStart)
        {
            // any end beats a begin
            return aStart ? false : true;
        }
        else
        {
            // sub End before parent End
            if(aEvt.subEvent && !bEvt.subEvent)
                return true;
            else
                return false;
        }
    }
    else
        return ax < bx;
}

float
dbFGrid::layer::NextDistance(float current)
{
    if(this->oIndex >= this->orderedEdges.size())
    {
        // this case occurs when we've processed our final events but
        // the current time hasn't yet been advanced.
        return 1e10;
    }

    int ievt = this->orderedEdges[this->oIndex];
    event &e = this->events[ievt/2];
    bool isDown = !(ievt&1);
    if(isDown || !e.subEvent)
    {
        if(isDown)
            return e.start - current;
        else
            return e.end - current;
    }
    else
    {
        // skip subEvent release messages (CC values are sample+hold)
        this->oIndex++;
        return this->NextDistance(current); 
    }
}

void
dbFGrid::layer::GetEvent(float current, Event *evt)
{
    // Until we implement super-sampling we only emit at start or end
    // It's up to caller to call GetEvent only when appropriate.
    int ievt = this->orderedEdges[this->oIndex];
    event &e = this->events[ievt/2];
    bool isDown = !(ievt & 1);
    if(this->type == k_noteLayer && e.subEvent == false)
    {
        evt->eType = isDown ? Event::k_NoteOn : Event::k_NoteOff;
        evt->note = e.row; // XXX: could add "detuning"
        if(isDown)
            evt->value = e.GetParam("v", this->defaultValue);
        else
            evt->value = 0; // 0 velocity
        evt->ccID = 0;
    }
    else
    {
        // We only deliver *start* CC events (piecewise constant)
        assert(isDown);
        evt->eType = Event::k_CC;
        evt->note = e.row; // this is our "MPE" content
        evt->value = e.GetParam("v", this->defaultValue);
        evt->ccID = e.GetParam("id", this->defaultID);
        if(evt->ccID == 224)
        {
            // std::cerr << "dbFGrid PitchWheel " << evt->value  << "\n";
        }
    }
    this->oIndex++;
}

/* -------------------------------------------------------------------------- */
void
dbFGrid::dumpObjectList(char const *nm, t_jobjArray const &olist)
{
    std::cerr << "objlist " << nm << " has " 
        << olist.size() << " elements.\n";
    if(olist.size() > 0)
        this->dumpObject(" [0]", olist[0]);
}

void
dbFGrid::dumpObject(char const *nm, t_jobj const &o)
{
    std::cerr << "o " << nm << "\n";
    for (auto i : o)
    {
        std::cout << "  " << i.first << ": " << i.second << '\n';
    }
}

void
dbFGrid::dumpMatrix()
{
    std::cerr << "matrix has " << this->m_layers.size() << " layers\n";
    this->dumpObject(" mparams", this->m_params);
	
    int i = 0;
    for(layer &l : this->m_layers)
    {
        std::cerr << "  L" << i++ << " has " << l.events.size() << " events\n";
        for(event &e : l.events)
        {
            for(auto it=e.params.begin(); it!=e.params.end(); ++it)
                std::cerr << "    " << it->first << ": " << it->second << "\n";
        }
    }
}