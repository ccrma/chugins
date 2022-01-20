#include "dbFGrid.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

#define kColEpsilon .0001f /* measured in columns */

/* ----------------------------------------------------------------------- */
dbFGrid::dbFGrid(unsigned int sampleRate)
{
    this->m_sampleRate = sampleRate;
    this->m_verbosity = 0;
}

dbFGrid::~dbFGrid()
{
    std::set<char const *>::iterator it = this->m_customCCNames.begin();
    while (it != this->m_customCCNames.end())
    {
        delete [] *it;
    }
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
    this->m_sections.clear();
    this->Rewind();

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
                if(this->m_verbosity > 0)
                    std::cerr << "DbFGrid opened " << fnm << "\n";
                auto jfm = j["fmatrix"];
                if(jfm.count("sections"))
                {
                    // sections is array of objects of "name", "column"
                    auto sections = jfm["sections"];
                    for(int is=0;is<sections.size();is++)
                    {
                        auto jsection = sections.at(is); // on o bje
                        auto jcolumns = jsection["columns"];
                        section s;
                        s.c0 = jcolumns.at(0).get<int>();
                        s.c1 = jcolumns.at(1).get<int>();
                        this->m_sections.push_back(s);
                    }
                    if(this->m_verbosity > 0)
                    {
                        std::cerr << "dbFGrid found " 
                            << this->m_sections.size() << " sections\n";
                    }
                }
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
                    if(fnm == "Session")
                        lt = layer::k_sessionLayer;
                    else
                    if(fnm == "Comments")
                    {
                        // not performable but we don't want to mess with id
                        this->m_layers.push_back(layer(layer::k_commentsLayer));
                        continue; 
                    }
                    else
                    {
                        std::cerr << "DbFGrid unimplemented layer type " << fnm << "\n";
                        continue;
                    }
                    this->m_layers.push_back(layer(lt));
                    layer &l = this->m_layers.back();
                    l.fparams = jl["fparams"];

                    const char *defKey = "v"; // value
                    l.defaultKey = defKey;
                    l.defaultValue = l.fparams.count(defKey) ? l.fparams[defKey].get<float>() : .75f;
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
                                // dictionary whose keys are the CCid/label and
                                // whose values are an array of subevent 
                                // objects (CCs associated with a note)
                                // these are flattened into the total event list.
                                auto seobj = it.value();
                                for(json::iterator seit = seobj.begin(); seit != seobj.end(); ++seit)
                                {
                                    auto sekey = seit.key(); // like "id:192", "Rate"
                                    auto selist = seit.value();
                                    const char *ccName = this->getCustomCCName(sekey);
                                    // foreach subevent
                                    l.events.reserve(l.events.size() + selist.size());
                                    for(int k=0;k<selist.size();k++)
                                    {
                                        l.events.push_back(event());
                                        event &se = l.events.back();
                                        se.subEvent = true;
                                        se.customName = ccName;
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
                                            {
                                                // seKey
                                                se.params[seKey] = seVal;
                                            }
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
                std::cerr << "DbFGrid invalid file '" << fnm << "'\n";
                err = 1;
            }
            
            if(err == 0 && this->m_verbosity > 0)
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
        std::cerr << "DbFGrid: '" << fnm << "' not found\n";
        err = -1;
    }
    return err;
}

/* tokenize incoming custom CCNames... */
char const *
dbFGrid::getCustomCCName(std::string const &str)
{
    char const *ret;
    if(str.size() > 3 && str[0] == 'i' && str[1] == 'd' && str[2] == ':')
        ret = nullptr;
    else
    {
        std::set<char const *>::iterator it = this->m_customCCNames.find(str.c_str());
        if(it != this->m_customCCNames.end())
            ret = *it;
        else
        {
            char *c = new char[str.size() + 1];
            if(c)
                strcpy(c, str.c_str());
            this->m_customCCNames.insert(c);
            ret = c;
        }
    }
    return ret;
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
dbFGrid::Rewind(int sectionIndex)
{
    // Define sectionIndex of 0 as valid even without sections since 
    // CC values are on [0, 1].
    if(sectionIndex != -1 && this->m_sections.size() <= sectionIndex)
    {
        if(sectionIndex != 0)
            std::cerr << "dbFGrid undefined section: " << sectionIndex << "\n";
        sectionIndex = -1;
    }
    if(sectionIndex == -1)
    {
        for(layer &l : this->m_layers)
            l.Rewind();
        this->m_currentTime = 0;
    }
    else
    {
        section &s = this->m_sections[sectionIndex];
        unsigned il=0;
        for(layer &l : this->m_layers)
            l.Rewind(s.c0, s.c1, il++, this->m_verbosity);
        this->m_currentTime = s.c0;
    }
    this->m_channelPool.clear();
    this->m_channelsInUse.clear();
    for(unsigned i=0;i<15;i++)
    {
        // below, we access new elements from back. For "cosmetics"
        // we'll initialize the pool so that low-numbered channels
        // come online before high-numbered channels
        this->m_channelPool.push_back(15-i);
    }    
    return 0;
}

/* return 0 on success, non-zero when we reach the end 
 */
int 
dbFGrid::Read(Event *evt, int soloLayer)
{
    if(this->m_currentTime > (this->m_bbox[1]+kColEpsilon))
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
        if(this->m_currentTime > l.GetMaxTime())
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
        if(minDist > kColEpsilon)
        {
            evt->eType = Event::k_Wait;
            evt->value = minDist * this->m_colToBeat;
            evt->note = 0;
            evt->ccID = -1;
            evt->ccName = nullptr;
            this->m_currentTime += minDist; // time in "columns"
        }
        else
        {
            layer &l = this->m_layers[nextLIndex];
            l.GetEvent(this->m_currentTime, evt, this->m_verbosity);
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
            std::cerr << "dbFGrid MIDI/MPE channel allocation botch for note " 
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
        if(note > 16) // poor man's check for !k_sessionLayer
        {
            std::cerr << "dbFGrid MIDI/MPE channel read botch for note " 
                 << note << "\n";
        }
        // else chan 0 is fine
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
dbFGrid::layer::operator()(int a, int b) // sort function
{
    event &aEvt = this->events[a/2];
    event &bEvt = this->events[b/2];
    bool aStart = !(a&1);
    bool bStart = !(b&1);
    float ax = aStart ? aEvt.start : aEvt.end;
    float bx = bStart ? bEvt.start : bEvt.end;
    float dx = fabs(ax - bx);
    if(dx < kColEpsilon)
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

void
dbFGrid::layer::Rewind(unsigned c0, unsigned c1, unsigned layerIndex, 
                        int verbosity)
{
    this->section0 = c0;
    this->section1 = c1;

    bool found = false;;
    // update oIndex to prior to or at c0
    for(unsigned i=0;i<this->orderedEdges.size();i++)
    {
        int ievt = this->orderedEdges[i];
        event &e = this->events[ievt >> 1];
        bool isDown = !(ievt & 1);
        if(isDown && !e.subEvent)
        {
            // Find the first real notedown after or equal to c0
            // Note that some notes may cross this line and so their
            // associated subEvents will be "orphaned" and result in
            // the MPE read "botch" state in findChannel, above.
            if(e.start > c0 || fabs(e.start - c0) < kColEpsilon)
            {
                found = true;
                this->oIndex = i;
                if(verbosity > 0)
                {
                    std::cerr << "dbFGrid Layer " << layerIndex << " section rewind, col " 
                        << c0 << " -> " << this->oIndex << " @ " << e.start << "\n";
                }
                break;
            }
        }
    }
    if(!found)
    {
        std::cerr << "dbFGrid Layer section botch " << c0 << "\n";
    }
}

float
dbFGrid::layer::NextDistance(float current)
{
    if(this->oIndex >= this->orderedEdges.size())
    {
        // this case occurs when we've processed our final events but
        // the current time hasn't yet been advanced.
        if(current < this->GetMaxTime())
            return this->GetMaxTime() - current; // produce a wait
        else
            return 1e10;
    }

    int ievt = this->orderedEdges[this->oIndex];
    event &e = this->events[ievt >> 1];
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

float
dbFGrid::event::GetParam(char const *nm, float fallback)
{
    if(this->params.count(nm))
        return this->params[nm].get<float>();
    else
    {
        // It's quite normal for a value not to be present.
        // See caller for more details.
        // std::cerr << "Can't find " << nm << "\n";
        return fallback;
    }
}

void
dbFGrid::layer::GetEvent(float current, Event *evt, int verbosity)
{
    // Until we implement super-sampling we only emit at start or end
    // It's up to caller to call GetEvent only when appropriate.
    int ievt = this->orderedEdges[this->oIndex];
    event &e = this->events[ievt/2];
    bool isDown = !(ievt & 1);
    evt->timestamp = isDown ? e.start : e.end;
    if(this->type != k_ccLayer && e.subEvent == false)
    {
        if(verbosity > 0)
        {
            std::cerr << "dbFGrid Note " << e.row << " "  
                << (isDown ? "down" : "up") <<  " @ " << 
                    (isDown ? e.start : e.end) << "\n";
        }
        evt->eType = isDown ? Event::k_NoteOn : Event::k_NoteOff;
        evt->note = e.row; // XXX: could add "detuning"
        if(isDown)
            evt->value = e.GetParam(this->defaultKey, this->defaultValue);
        else
            evt->value = 0; // 0 velocity
        evt->ccID = 0;
        evt->ccName = nullptr;
    }
    else
    {
        // We only deliver *start* CC events (piecewise constant).
        // Global CCs, like channel volume, pan, etc can occur in
        // session layers.
        evt->eType = Event::k_CC;
        evt->note = e.row; // this is our "MPE" content
        evt->ccID = e.GetParam("id", -1); // default when customName
        evt->ccName = e.customName; // usually nulltpr, otherwise token
        if(evt->ccID == -1 && evt->ccName == nullptr)
        {
            std::cerr << "DbFGrid unidentifiable CC\n";
            evt->ccID = this->defaultID;
        }

        // currently a value is only present if it overrides the default
        // value. Problem is, we don't know the default value for every
        // possible CC.  Let's elect .5 as a good choice.
        evt->value = e.GetParam("v", .5f);
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
    std::cerr << nm << "\n";
    for (auto i : o)
    {
        std::cout << "  " << i.first << ": " << i.second << '\n';
    }
}

void
dbFGrid::dumpMatrix()
{
    std::cerr << "dbFGrid has:\n";
    std::cerr << "  " << this->m_layers.size() << " layers\n";
    std::cerr << "  " << this->m_sections.size() << " sections\n";
    for(int s=0;s<this->m_sections.size();s++)
    {
        int c0 = m_sections[s].c0;
        int c1 = m_sections[s].c1;
        std::cerr << "    " << s << ": [" << c0 << "," << c1 <<"]\n";
    }
    this->dumpObject("  params", this->m_params);
    int il = 0;
    for(layer &l : this->m_layers)
    {
        std::cerr << "  L" << il++ << " has " << l.events.size() << " events\n";
        int i = 0;
        int ie=0, ise=-666;

        // these are the events, not the ordered events
        if(this->m_verbosity > 1)
        {
            for(event &e : l.events)
            {
                for(auto it=e.params.begin(); it!=e.params.end(); ++it)
                {
                    if(e.subEvent)
                    {
                        if(this->m_verbosity > 2)
                        {
                            std::cerr << "     se" << ise++ << " " 
                                << it->first << ": " << it->second << "\n";
                        }
                    }
                    else
                    {
                        std::cerr << "    e" << ie++ << " " << e.start << "\n";
                        ise = 0;
                    }
                }
                i++;
            }
        }
    }
}