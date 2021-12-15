#include "dbFGrid.h"

#include <iostream>
#include <fstream>
#include <algorithm>

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
                for(int i=0;i<jlayers.size(); i++)
                {
                    auto jl = jlayers.at(i);
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
                    layer &l = this->m_layers[i];
                    l.fparams = jl["fparams"];
                    l.defaultValue = l.fparams.count("v") ? l.fparams["v"] : .75f;
                    l.defaultID = l.fparams.count("id") ? l.fparams["id"] : 0;
                    l.bbox[0] = 1e10f;
                    l.bbox[1] = 0;

                    auto jelist = jl["events"]; // array of objects
                    for(int j=0;j<jelist.size();j++)
                    {
                        l.events.push_back(event());
                        event &e = l.events[j];
                        auto je = jelist[j]; // an event object
                        for (json::iterator it = je.begin(); it != je.end(); ++it) 
                        {
                            if(it.key() == "_")
                            {
                                auto v = it.value();
                                e.start = v[0].get<float>();
                                e.end = e.start + v[1].get<float>();
                                e.row = v[2];

                                if(e.start < l.bbox[0])
                                    l.bbox[0] = e.start;
                                if(e.end > l.bbox[1])
                                    l.bbox[1] = e.end;
                            }
                            else
                                e.params[it.key()] = it.value();
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
    int nextEventLayer = -1;
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
            nextEventLayer = i;
        }
    }
    if(nextEventLayer == -1)
        return -2;
    else
    {
        evt->layer = nextEventLayer;
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
            layer &l = this->m_layers[nextEventLayer];
            l.GetEvent(this->m_currentTime, evt);
        }
    }
    return 0;
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
    std::sort(this->orderedEdges.begin(), this->orderedEdges.end(), *this);
}

bool 
dbFGrid::layer::operator()(int a, int b)
{
    event &aEvt = this->events[a/2];
    event &bEvt = this->events[b/2];
    float ax = (a & 1) ? aEvt.end : aEvt.start;
    float bx = (b & 1) ? bEvt.end : bEvt.start;
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
    if(ievt & 1)
        return e.end - current;
    else
        return e.start - current;
}

void
dbFGrid::layer::GetEvent(float current, Event *evt)
{
    // Until we implement super-sampling we only emit at start or end
    // It's up to caller to call GetEvent only when appropriate.
    int ievt = this->orderedEdges[this->oIndex];
    event &e = this->events[ievt/2];
    bool isStart = !(ievt & 1);
    if(this->type == k_noteLayer)
    {
        evt->eType = isStart ? Event::k_NoteOn : Event::k_NoteOff;
        evt->note = e.row; // XXX: could add "detuning"
        if(isStart)
            evt->value = e.GetParam("v", this->defaultValue);
        else
            evt->value = 0; // 0 velocity
        evt->ccID = 0;
    }
    else
    if(this->type == k_ccLayer)
    {
        evt->eType = Event::k_CC;
        evt->note = e.row; // this is our "MPE" content
        evt->value = e.GetParam("v", this->defaultValue);
        evt->ccID = e.GetParam("id", this->defaultID);
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