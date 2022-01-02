#ifndef dbFGrid_h
#define dbFGrid_h

#include "json.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>

using json = nlohmann::json;

typedef std::unordered_map<std::string, json> t_jobj;
typedef std::vector<t_jobj> t_jobjArray;

#define kEpsilon .000001f

// parser, execution engine for .fgrd (Fiddle note/function grid files).
class dbFGrid
{
public:
    dbFGrid(unsigned int sampleRate);
    ~dbFGrid();

    int Open(std::string const &filepath);
    int Close();
    int GetNumLayers();
    int GetBarSize();    // num of signature
    int GetBeatSize(); // denom of signature - 'which note value == beat'
    int Rewind();

    struct Event
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
        unsigned ccID;
        unsigned chan;
        float value; // velocity, ccvalue, k_Wait dur 
    };
    int Read(Event *, int layer=-1); // return 0 on success, non-zero means "done"

private:
    unsigned m_sampleRate;
    float m_bbox[2]; // of all layers
    int m_signature[2];  // from file (eg [4, 4] === [4, .25])
    float m_sigDenom; // from file (.25)
    float m_columnUnit;  // from file (eg .125)
    float m_colToBeat; // combines sigDenom and columnUnit

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
        }
        bool subEvent;
        int row; // midi-note
        float start, end; // in "column coords"
        t_jobj params;
        float GetParam(char const *nm, float fallback)
        {
            return this->params.count(nm) ? this->params[nm].get<float>() : fallback;
        }
    };
    struct layer
    {
        enum layerType
        {
            k_noteLayer,
            k_ccLayer,
        };

        layer(layerType t)
        {
            this->type = t;
            this->Rewind();
        }
        void Rewind()
        {
            this->oIndex = 0;
        }
        void ComputeEdgeOrdering(); // after loading
        bool operator()(int i, int j); // for sorting event order

        float NextDistance(float current);
        void GetEvent(float current, Event *evt);

        layerType type;
        float defaultValue; // for events without one
        int defaultID; // CC id
        t_jobj fparams;
        std::vector<event> events;
        std::vector<int> orderedEdges;
        float bbox[2];
        unsigned oIndex; // last accessed ordered event
    };
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

private:
    void dumpObjectList(char const *nm, t_jobjArray const &);
    void dumpObject(char const *nm, t_jobj const &);
    void dumpMatrix();
};

#endif