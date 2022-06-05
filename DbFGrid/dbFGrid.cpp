#include "dbFGrid.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <random>
#include <climits>
#include <cassert>

/* ----------------------------------------------------------------------- */
dbFGrid::dbFGrid(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
    m_verbosity = 0;
}

dbFGrid::~dbFGrid()
{
    std::set<char *>::iterator it = m_customCCNames.begin();
    while (it != m_customCCNames.end())
    {
        char *s = *it;
        delete [] s;
        it++;
    }
}

int
dbFGrid::Open(std::string const &fnm)
{
    int err = 0;
    m_currentTime = 0;
    m_bbox[0] = 1e6;
    m_bbox[1] = 0;
    m_columnUnit = .25f; // size of a column (here 1/4 note)
    m_signature[0] = 4;
    m_signature[1] = 4;  // size of a bat (here 1/4 note)
    m_colToBeat = 1.f; // converts columns to beats (here 1)
    m_sections.clear();
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
                if(m_verbosity > 0)
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
                        Section s;
                        s.c0 = jcolumns.at(0).get<int>();
                        s.c1 = jcolumns.at(1).get<int>();
                        m_sections.push_back(s);
                    }
                    if(m_verbosity > 0)
                    {
                        std::cerr << "dbFGrid found " 
                            << m_sections.size() << " sections\n";
                    }
                }
                m_params = jfm["params"];
                float beatSize;
                if(m_params.count("signature"))
                {
                    m_signature[0] = m_params["signature"]["x"].get<int>();
                    beatSize = m_params["signature"]["y"].get<float>();
                    m_signature[1] = int(.5f + 1.f/ beatSize);
                }
                else
                    beatSize = 1.0f / m_signature[1];
                if(m_params.count("columnUnit"))
                    m_columnUnit = m_params["columnUnit"].get<float>();
                // example: 1/8 colunit, 1/4 sigDenom => colToBeat: .5 
                m_colToBeat = m_columnUnit / beatSize; 
                auto jlayers = jfm["layers"];

                // foreach layer
                for(int il=0;il<jlayers.size(); il++)
                {
                    auto jl = jlayers.at(il);
                    auto jelist = jl["events"]; // array of objects
                    if(jelist.size() == 0)
                        continue;

                    auto fnm = jl["fnm"];
                    Layer::layerType lt;
                    if(fnm == "MidiNote")
                        lt = Layer::k_noteLayer;
                    else
                    if(fnm == "MidiCC")
                        lt = Layer::k_ccLayer;
                    else
                    if(fnm == "Session")
                        lt = Layer::k_sessionLayer;
                    else
                    if(fnm == "Comments")
                    {
                        // not performable but we don't want to mess with id
                        m_layers.push_back(Layer(Layer::k_commentsLayer));
                        continue; 
                    }
                    else
                    {
                        std::cerr << "DbFGrid unimplemented layer type " << fnm << "\n";
                        continue;
                    }
                    m_layers.push_back(Layer(lt));
                    Layer &l = m_layers.back();
                    l.fparams = jl["fparams"];

                    const char *defKey = "v"; // value
                    l.defaultKey = defKey;
                    l.defaultValue = l.fparams.count(defKey) ? l.fparams[defKey].get<float>() : .75f;
                    l.defaultID = l.fparams.count("id") ? l.fparams["id"].get<int>() : 0;
                    l.bbox[0] = 1e10f;
                    l.bbox[1] = 0;
                    l.cells.reserve(jelist.size()); // more when subevents

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
                        unsigned eIndex = l.cells.size(); // before we add one
                        l.cells.push_back(Cell());

                        // NB: this ref is only valid 'til array resizing happens
                        Cell &e = l.cells[eIndex];
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
                                    const char *ccName = this->getCustomCCName(sekey.c_str());
                                    // foreach subevent
                                    l.cells.reserve(l.cells.size() + selist.size());
                                    for(int k=0;k<selist.size();k++)
                                    {
                                        l.cells.push_back(Cell());
                                        Cell &se = l.cells.back();
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
                                l.cells[eIndex].params[it.key()] = it.value();
                                // e.params[it.key()] = it.value();
                            }
                        }
                    }
                    if(m_bbox[0] > l.bbox[0])
                        m_bbox[0] = l.bbox[0];
                    if(m_bbox[1] < l.bbox[1])
                        m_bbox[1] = l.bbox[1];
                    l.ComputeEdgeOrdering();
                }
            }
            else
            {
                std::cerr << "DbFGrid invalid file '" << fnm << "'\n";
                err = 1;
            }
            
            if(err == 0 && m_verbosity > 0)
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

int
dbFGrid::SetArrangement(std::string const &expr)
{
    if(expr.size() == 0) return 0;

    std::string exprFlat;
    this->flattenPartSpec(expr.c_str(), &exprFlat); // result reported when verbose
    for(int i=0;i<exprFlat.size();i++)
    {
        int sectionNum = (exprFlat[i] - 'A');
        if(sectionNum >= m_sections.size())
        {
            std::cerr << "DbFGrid: unknown section in arrangement '" 
                    << exprFlat[i] << "' (" 
                    << m_sections.size() << " sections)\n";
            return -1;
        }
    }
    m_arrangementState.Init(exprFlat);
    return 0;
}

/* tokenize incoming custom CCNames... */
bool
dbFGrid::hasCustomCCName(char const *nm)
{
    return m_customCCNames.count((char *) nm);
}

char const *
dbFGrid::getCustomCCName(char const *nm)
{
    char const *ret;
    int len = nm ? strlen(nm) : 0;
    if(len > 3 && nm[0] == 'i' && nm[1] == 'd' && nm[2] == ':')
        ret = nullptr;
    else
    {
        std::set<char *>::iterator it = m_customCCNames.find((char *)nm);
        if(it != m_customCCNames.end())
            ret = *it;
        else
        {
            char *c = new char[len + 1];
            if(c)
                strcpy(c, nm);
            m_customCCNames.insert(c);
            ret = c;
        }
    }
    return ret;
}

int
dbFGrid::GetNumLayers()
{
    return (int) m_layers.size();
}

int
dbFGrid::GetBarSize()
{
    return m_signature[0];
}

int
dbFGrid::GetBeatSize()
{
    return m_signature[1];
}

int
dbFGrid::Rewind(int sectionIndex)
{
    // Define sectionIndex of 0 as valid even without sections since 
    // CC values are on [0, 1].
    if(sectionIndex != -1 && m_sections.size() <= sectionIndex)
    {
        if(sectionIndex != 0)
            std::cerr << "dbFGrid undefined section: " << sectionIndex << "\n";
        sectionIndex = -1;
    }
    if(sectionIndex == -1)
    {
        for(Layer &l : m_layers)
            l.Rewind();
        m_currentTime = 0;
    }
    else
    {
        Section &s = m_sections[sectionIndex];
        unsigned il=0;
        for(Layer &l : m_layers)
            l.Rewind(s.c0, s.c1, il++, m_verbosity);
        m_currentTime = s.c0;
    }
    m_channelPool.clear();
    m_channelsInUse.clear();
    for(unsigned i=0;i<15;i++)
    {
        // below, we access new elements from back. For "cosmetics"
        // we'll initialize the pool so that low-numbered channels
        // come online before high-numbered channels
        m_channelPool.push_back(15-i);
    }    
    return 0;
}

int
dbFGrid::ReadChannel(char const *nm, int ll, int col, int row,
    std::vector<double> &result)
{
    int npts = 0;
    if(this->hasCustomCCName(nm))
    {
        char const *nmtok = dbFGrid::getCustomCCName(nm);
        if(this->GetNumLayers() > ll)
        {
            Layer &l = m_layers[ll];
            npts = l.ReadSubEventData(nmtok, col, row, result);
        }
    }
    return npts;
}

/**
 * @brief return the next event in the grid. 
 * @param evt 
 * @param soloLayer 
 * @return int 0 on sucess, non-zero on error or EOF.
 */
int 
dbFGrid::Read(Event *evt, int soloLayer)
{
    if(!m_arrangementState.IsActive())
    {
        if(this->getNextEvent(evt, soloLayer))
            return 0; // no error
        else
            return -1; // done
    }
    else
    {
        int endOfSection = m_arrangementState.GetEndOfSection(m_sections);
        if(m_arrangementState.IsInit() || !this->getNextEvent(evt, soloLayer, endOfSection))
        {
            int nextSection = m_arrangementState.GetNextSection(m_sections);
            if(nextSection != -1)
            {
                // a section has changed, but there may be multiple danging
                // note-ends
                if(m_verbosity)
                {
                    std::cerr << "arrangement " 
                        << m_arrangementState.index 
                        << " next section " << nextSection << " -----\n";
                }
                if(nextSection >= 0)
                {
                    this->Rewind(nextSection); // changes currentTime
                    return this->Read(evt, soloLayer);
                }
            }
            return -1;  // done
        }
        else
            return 0;
    }
}

bool
dbFGrid::getNextEvent(Event *evt, int soloLayer, int endOfSection)
{
    if(m_currentTime > (m_bbox[1]+kColEpsilon))
        return false;
    
    if(endOfSection != 0 && m_currentTime > endOfSection)
        return false;
    
    // first nominate the best next event defined as the one whose
    // start or end is closest to m_currentTime
    float minDist = 1e6;
    int nextLIndex = -1;
    for(int i=0;i<m_layers.size();i++)
    {
        if(soloLayer != -1 && i != soloLayer)
            continue;
        
        Layer &l = m_layers[i];
        if(m_currentTime > l.GetMaxTime() || l.type == Layer::k_commentsLayer)
            continue;
        
        float dist = l.NextDistance(m_currentTime, endOfSection);
        if(dist < minDist)
        {
            minDist = dist;
            nextLIndex = i;
        }
    }
    if(nextLIndex == -1) // no events found in file or section
    {
        if(endOfSection > 0) 
        {
            float dt = (endOfSection - m_currentTime);
            if(dt > kColEpsilon)
            {
                // generate a wait to end-of-section
                // std::cerr << "End of section wait " << dt << "\n";
                evt->eType = Event::k_Wait; // <-----------------
                evt->value = dt * m_colToBeat;
                evt->note = 0;
                evt->ccID = -1;
                evt->ccName = nullptr;
                m_currentTime += dt; // time in "columns"
                return true;
            }
            else
                return false;
        }
        return false;
    }
    else
        evt->layer = nextLIndex;

    if(minDist > kColEpsilon)
    {
        evt->eType = Event::k_Wait; // <-----------------
        evt->value = minDist * m_colToBeat;
        evt->note = 0;
        evt->ccID = -1;
        evt->ccName = nullptr;
        m_currentTime += minDist; // time in "columns"
    }
    else
    {
        Layer &l = m_layers[nextLIndex];
        l.GetEvent(m_currentTime, evt, m_verbosity);
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
        case Event::k_Wait:
        default:
            break;
        }
    }
    return true;
}

unsigned
dbFGrid::allocateChannel(unsigned note, unsigned layer)
{
    unsigned chan = 0;
    if(m_channelPool.size() > 0)
    {
        chan = m_channelPool.back();
        m_channelPool.pop_back();

        unsigned key = (layer << 7) | note;
        if(m_channelsInUse.count(key) != 0)
        {
            std::cerr << "dbFGrid MIDI/MPE channel allocation botch for note " 
                      << note << "\n";
        }
        m_channelsInUse[key] = chan;
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
    if(m_channelsInUse.count(key) != 0)
    {
        auto it = m_channelsInUse.find(key);
        chan = it->second;
        if(release)
        {
            m_channelsInUse.erase(it);
            m_channelPool.push_back(chan);
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

/* ---------------------------------------------------------------- */
int
dbFGrid::Layer::ReadSubEventData(char const *name, unsigned col, unsigned row,
    std::vector<double>&data)
{
    // cells aren't organized into rows.. But are sorted by col.
    // parent cells precede subevents.
    int npts = 0; // means fail
    const float eps = .0001f;

    for(unsigned i=0;i<this->cells.size();i++)
    {
        Cell &cell = this->cells[i];
        if(cell.row == row) // here we are
        {
            if(cell.start > col+eps)
                break; // not found
            else
            if(cell.start < col-eps)
                continue;
            // examine all cells of the same row and within cell's timewindow
            while(++i < this->cells.size())
            {
                Cell &cc = this->cells[i];
                if(cc.end > cell.end+eps)
                    break;
                // we can compare nameptrs because they are tokenized
                if(cc.subEvent && cc.row == row && cc.customName == name)
                {
                    data.push_back(cc.GetParam("v", 0.f));
                    npts++;
                }
            }
            break;
        }
    }
    return npts;
}

void
dbFGrid::Layer::ComputeEdgeOrdering()
{
    int nEdges = this->cells.size() * 2;
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
dbFGrid::Layer::operator()(int a, int b) // sort function
{
    Cell &aEvt = this->cells[a/2];
    Cell &bEvt = this->cells[b/2];
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
dbFGrid::Layer::Rewind(unsigned c0, unsigned c1, unsigned layerIndex, 
                        int verbosity)
{
    this->section0 = c0;
    this->section1 = c1;

    if(this->type == k_commentsLayer)
    {
        this->oIndex = 0;
        return;
    }

    bool found = false;;
    // update oIndex to prior to or at c0
    for(unsigned i=0;i<this->orderedEdges.size();i++)
    {
        int ievt = this->orderedEdges[i];
        Cell &e = this->cells[ievt >> 1];
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
dbFGrid::Layer::NextDistance(float current, int maxDist /* end-of-section is non-0 column */)
{
    if(this->oIndex >= this->orderedEdges.size())
    {
        // this case occurs when we've processed our final events but
        // the current time hasn't yet been advanced.
        if(maxDist != 0 && current < maxDist)
            return maxDist - current; // request a wait
        else
        if(current < this->GetMaxTime())
            return this->GetMaxTime() - current; // request a wait
        else
            return 1e10;
    }

    int ievt = this->orderedEdges[this->oIndex];
    Cell &e = this->cells[ievt >> 1];
    bool isDown = !(ievt&1); // even/odd test
    if(isDown || !e.subEvent)
    {
        if(isDown)
        {
            if(maxDist != 0 && e.start >= maxDist)
                return 1e10; // no start-events at maxDist or beyond
            else
                return e.start - current;
        }
        else
        {
            if(maxDist != 0 && e.end > maxDist)
                return maxDist - current;
            else
                return e.end - current;
        }
    }
    else
    {
        // skip subEvent release messages (CC values are sample+hold)
        this->oIndex++;
        return this->NextDistance(current, maxDist); 
    }
}

float
dbFGrid::Cell::GetParam(char const *nm, float fallback)
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
dbFGrid::Layer::GetEvent(float current, Event *evt, int verbosity)
{
    // Until we implement super-sampling we only emit at start or end
    // It's up to caller to call GetEvent only when appropriate.
    int ievt = this->orderedEdges[this->oIndex];
    Cell &e = this->cells[ievt/2];
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

/* ----------------------------------------------------------------- */
/* converts an in-header P: field to a list of part labels 
 * returns the number of distinct parts enountered.
 * caller can inspect return.  If == 1, it may be an in-body
 * partlabel (since the line between header and notes is ill-defined)
 */
/* e.g. P:A(AB)3(CD)2 becomes P:AABABABCDCD 
        P: A3[A|B|C]3
        P: A3[A|A|A|B|C]3
        P: A3(X[A|A|A|B|C]3)5
        P: A3[C|D|[E|F|G]2]3  // chooser flattens results when its ] is encountered
 */
int 
dbFGrid::flattenPartSpec(char const *spec, std::string *partspec)
{
    int stack[10]; // index of beginning of pattern seq
    char lastch =  ' '; /* [SDG] 2020-06-03 */
    char errmsg[80];
    int stackptr = 0;
    int spec_length = strlen(spec);
    int k = 0;
    int inChoice = 0;
    char const *in = spec;

    while(*in != 0 && k < spec_length) 
    { 
        k++;
        if(*in == '.' || *in == ' ')  // for readability
            in = in + 1;
        else
        if(((*in >= 'A') && (*in <= 'Z')) || 
            (*in == '(') || (*in == ')') || (*in == ']') || (*in == '[') ||
            (*in == '|' && inChoice) ||
            ((*in >= '0') && (*in <= '9'))) 
        {
            if(((*in >= 'A') && (*in <= 'Z')) || (*in == '|'))
            {
                partspec->push_back(*in);
                lastch = *in;
                in = in + 1;
            }
            if(*in == '(' || *in == '[') 
            {
                if(*in == '[')
                    inChoice++;
                if(stackptr < 10) 
                {
                    stack[stackptr] = partspec->size();
                    stackptr = stackptr + 1;
                } 
                else 
                    this->error("nesting too deep in part specification");
                in = in + 1;
            }
            if(*in == ')' || *in == ']')  // nb: we lookahead for number following
            {
                int chooser = (*in == ']');
                if(chooser) inChoice--;
                in = in + 1;
                if(stackptr > 0) 
                {
                    int repeats, start, stop;
                    if((*in >= '0') && (*in <= '9')) 
                        repeats = this->readnump(&in);
                    else 
                        repeats = 1;
                    stackptr = stackptr - 1;
                    start = stack[stackptr];
                    stop = partspec->size();
                    if(chooser)
                    {
                        std::string input = partspec->substr(start, stop);
                        std::vector<std::string> choices;
                        std::string::size_type pos = 0;
                        while(pos != std::string::npos)
                        {
                            auto nextpos = input.find_first_of('|', pos);
                            if(nextpos == std::string::npos)
                            {
                                choices.push_back(input.substr(pos));
                                break;
                            }
                            else
                            {
                                choices.push_back(input.substr(pos, nextpos-pos));
                                pos = nextpos+1;
                            }
                        }
                        std::default_random_engine generator;
                        std::uniform_int_distribution<int> distribution(0,choices.size()-1);
                        partspec->erase(start, stop);
                        for(int i=0; i<repeats; i++) 
                        {
                            int r = distribution(generator);
                            partspec->append(choices[r]);
                        }
                    }
                    else
                    for(int i=1; i<repeats; i++)  // <-- starts at 1
                    {
                        for(int j=0; j<((int) (stop-start)); j++) 
                        {
                            char c = partspec->at(start+j);
                            partspec->push_back(c);
                        }
                    }
                } 
                else 
                    this->error("Too many )'s in part specification");
            }
            else
            if((*in >= '0') && (*in <= '9')) 
            {
                // handles: A3 (not (AB)3),CASEis->parser->readnump(&in);
                int repeats = this->readnump(&in);
                if(partspec->size() > 0) 
                {
                    for(int i = 1; i<repeats; i++) 
                        partspec->push_back(lastch);
                } 
                else 
                    this->error("No part to repeat in part specification");
            }
        }
        else 
        {
            if(m_verbosity > 1)
            {
                snprintf(errmsg, 80, 
                        "illegal character \'%c\' in part specification.\n"
                        "The P: is ignored.", *in);
                this->error(errmsg);
            }
            break;
        }
    } /* end of spec */
    if(stackptr != 0) 
    {
        this->error("Too many ('s in part specification");
        return 0;
    }
    if(m_verbosity)
    {
        std::string x("pattern '");
        x.append(spec);
        x.append("' => ");
        x.append(partspec->c_str());
        this->log(x.c_str());
    }
    return partspec->size();
}

int
dbFGrid::readnump(char const **p)
{
    int sign = 1;
    if(**p == '-')
    {
        *p = *p + 1;
        this->skipspace(p);
        sign = -1;
    }
    int t = 0;
    while(isdigit(**p) && (t < (INT_MAX-9)/10))
    {
        t = t * 10 + (int) **p - '0';
        *p = *p + 1;
    }
    return sign * t;
}

void
dbFGrid::skipspace(char const **p)
{
    char c = **p;
    while((c == ' ') || (c == '\t'))
    {
        *p = *p + 1;
        c = **p;
    }
}
void 
dbFGrid::error(char const *msg)
{
    std::cerr << "DbFGrid ERROR " << msg << "\n";
}

void 
dbFGrid::log(char const *msg)
{
    std::cerr << "DbFGrid " << msg << "\n";
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
    std::cerr << "  " << m_layers.size() << " layers\n";
    std::cerr << "  " << m_sections.size() << " sections\n";
    for(int s=0;s<m_sections.size();s++)
    {
        int c0 = m_sections[s].c0;
        int c1 = m_sections[s].c1;
        std::cerr << "    " << s << ": [" << c0 << "," << c1 <<"]\n";
    }
    this->dumpObject("  params", m_params);
    int il = 0;
    for(Layer &l : m_layers)
    {
        std::cerr << "  L" << il++ << " has " << l.cells.size() << " events\n";
        int i = 0;
        int ie=0, ise=-666;

        // these are the events, not the ordered events
        if(m_verbosity > 1)
        {
            for(Cell &e : l.cells)
            {
                for(auto it=e.params.begin(); it!=e.params.end(); ++it)
                {
                    if(e.subEvent)
                    {
                        if(m_verbosity > 2)
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