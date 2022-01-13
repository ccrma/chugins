#ifndef dbFGrid_h
#define dbFGrid_h

#include "json.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <set>

using json = nlohmann::json;

typedef std::unordered_map<std::string, json> t_jobj;
typedef std::vector<t_jobj> t_jobjArray;

// parser, execution engine for .fgrd (Fiddle note/function grid files).
class dbFGrid
{
public:
    dbFGrid(unsigned int sampleRate);
    ~dbFGrid();

    void SetVerbosity(int v) { this->m_verbosity = v; }
    int Open(std::string const &filepath);
    int Close();
    int GetNumLayers();
    int GetBarSize();    // num of signature
    int GetBeatSize(); // denom of signature - 'which note value == beat'
    int Rewind(int sectionIndex=-1);

    struct Event // delivered to read
    {
        Event()
        {
            this->chan = 0;
        }
        enum
        {
            k_Wait=0,
            k_NoteOn,
            k_NoteOff,
            k_CC,
        } eType;
        unsigned layer;
        unsigned note;  // associated with row
        int ccID; // negative is invalid
        char const *ccName; // used for custom CC names. usually nullptr
        unsigned chan;
        float value; // velocity, ccvalue, k_Wait dur 
        float timestamp; // columns
    };
    int Read(Event *, int layer=-1); // return 0 on success, non-zero means "done"

private:
    struct section
    {
        unsigned c0, c1;
    };

    unsigned m_sampleRate;
    float m_bbox[2]; // of all layers
    int m_signature[2];  // from file (eg [4, 4] === [4, .25])
    float m_sigDenom; // from file (.25)
    float m_columnUnit;  // from file (eg .125)
    float m_colToBeat; // combines sigDenom and columnUnit
    std::vector<section> m_sections;
    int m_verbosity;

    float m_currentTime; // measured in fractional columns

    t_jobj m_params; // of fmatrix
    /*
        altLength: "16 : 2 : 0"
        noteFilter: 1 // Chromatic
        columnSnap: 1
        labelIndexOrigin: 1
        signature: {"x":"3","y":0.25} // 3/4 time
        columnUnit: 0.125  // 1/8
        numColumns: 100
        octaveRange: {"x":1,"y":7}
        scaleRoot: 0
    */
    struct event // events and subevents
    {
        event()
        {
            this->subEvent = false;
            this->customName = nullptr;
        }
        bool subEvent;
        int row; // midi-note
        float start, end; // in "column coords"
        t_jobj params;
        char const *customName;
        float GetParam(char const *nm, float fallback);
    };

    struct layer
    {
        enum layerType
        {
            k_noteLayer,
            k_ccLayer,
            k_sessionLayer
        };

        layer(layerType t)
        {
            this->type = t;
            this->Rewind();
        }
        void Rewind()
        {
            this->oIndex = 0;
            this->section0 = 0;
            this->section1 = 0;
        }
        void Rewind(unsigned c0, unsigned c1, unsigned layerIndex, int verbosity);
        void ComputeEdgeOrdering(); // after loading
        bool operator()(int i, int j); // for sorting event order

        float NextDistance(float current);
        void GetEvent(float current, Event *evt, int verbosity);
        float GetMaxTime() 
        { 
            return (this->section1 == 0) ? this->bbox[1] : this->section1;
        }

        layerType type;
        char const *defaultKey;
        float defaultValue; // for events without one
        int defaultID; // CC id
        t_jobj fparams;
        std::vector<event> events;
        std::vector<int> orderedEdges;
        float bbox[2];
        unsigned oIndex; // last accessed ordered event
        unsigned section0, section1;
    }; // end of struct layer
    std::vector<layer> m_layers;

private:
    // We associate a MIDI channel with a note following MPE practice.
    // Since our CC events are already tied to a note, this isn't strictly
    // necessary. It may make the lives of downstream players a little simpler.
    // The idea: Midi CCs (except note-aftertouch) are "channel-wide". The
    // MPE trick is to trick MIDI by assigning different channels to each note.
    // In MIDI there are only 15 non-default channels, so we need to dole them
    // out accordingly.  This means that no more than 15 notes with cc's can
    // be live at one time.
    std::vector<unsigned> m_channelPool; // max length 15
    std::map<unsigned, unsigned> m_channelsInUse; // key is layer << 7 | note
    unsigned allocateChannel(unsigned note, unsigned layer);
    unsigned findChannel(unsigned note, unsigned layer, bool release);
    char const *getCustomCCName(std::string const &); 
    struct cmpStr
    {
        // strict weak ordering
        bool operator()(char const *a, char const *b) const
        {
            return std::strcmp(a, b) < 0;
        }
    };
    std::set<char const *, cmpStr> m_customCCNames;

private:
    void dumpObjectList(char const *nm, t_jobjArray const &);
    void dumpObject(char const *nm, t_jobj const &);
    void dumpMatrix();
};

#endif