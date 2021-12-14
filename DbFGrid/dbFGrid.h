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
        int layer;
        float note;  // associated with row
        float value; // velocity, ccvalue, k_Wait dur 
        int ccID;
        int chan;
    };
    int Read(Event *); // return 0 on success, non-zero means "done"

private:
    unsigned m_sampleRate;
    float m_bbox[2]; // of all layers
    float m_currentTime;

    t_jobj m_params;
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
    struct event
    {
        int row; // midi-note
        float start, end; // in "column coords"
        t_jobj params;

        float GetParam(char const *nm, float fallback)
        {
            return this->params.count(nm) ? this->params[nm] : fallback;
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
    void dumpObjectList(char const *nm, t_jobjArray const &);
    void dumpObject(char const *nm, t_jobj const &);
    void dumpMatrix();
};

#endif