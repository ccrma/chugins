#ifndef dbFGrid_h
#define dbFGrid_h

#include "json.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <set>
#include <iostream>

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
    int SetArrangement(std::string const &);
    int GetNumLayers();
    bool IsValid() { return this->GetNumLayers() > 0; }
    int GetBarSize();    // num of signature
    int GetBeatSize(); // denom of signature - 'which note value == beat'
    int Rewind(int sectionIndex = -1);

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

    /**
     * @brief  read the enter channel data within the specified cell.
     * 
     * @param nm  name of the channel (eg Pan)
     * @param layer  layer index
     * @param col  column index of cell
     * @param row  row index of cell
     * @param results results will be placed here.
     * @return int 0 on fail, npts on success
     */
    int ReadChannel(char const *nm, int layer, int col, int row,
        std::vector<double> &results);

private:
    #define kColEpsilon .0001f /* measured in columns */

    struct Section
    {
        unsigned c0, c1;
    };

    struct ArrangementState
    {
        std::string expr;
        int index;
        
        ArrangementState()
        {
            this->index = 0;
        }

        void Init(std::string &x)
        {
            this->expr = x;
            this->index = -1;
        };

        void Rewind()
        {
            this->index = -1;
        }

        bool IsActive()
        {
            return expr.size() > 0;
        }

        bool IsDone()
        {
            if(this->expr.size() == 0) return true; // not our job
            return this->index >= this->expr.size();
        }

        // return -1 if no new section required
        int GetNextSection(float currentTime, 
                    std::vector<Section> const &sections)
        {
            float maxCol = 0.f;
            if(this->index >= 0)
            {
                int sectionNum = this->expr[this->index] - 'A';
                maxCol = sections[sectionNum].c1;
            }
            if(currentTime > maxCol - kColEpsilon)
            {
                this->index++;
                if(this->index < this->expr.size())
                    return this->expr[this->index] - 'A';
            }
            return -1;
        }
    };

    unsigned m_sampleRate;
    float m_bbox[2]; // of all layers
    int m_signature[2];  // from file (eg [4, 4] === [4, .25])
    float m_sigDenom; // from file (.25)
    float m_columnUnit;  // from file (eg .125)
    float m_colToBeat; // combines sigDenom and columnUnit
    int m_verbosity;

    float m_currentTime; // measured in fractional columns
    ArrangementState m_arrangementState;
    std::vector<Section> m_sections;

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
    struct Cell // cell events and subevents
    {
        Cell()
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

    struct Layer
    {
        enum layerType
        {
            k_noteLayer,
            k_ccLayer,
            k_sessionLayer,
            k_commentsLayer // skip comment events, so empty
        };

        Layer(layerType t)
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
        /**
         * @brief Fill data with values for the named subevent located
         * within the cell located at (col, row).
         * @return int  0 on fail, npts on success
         */
        int ReadSubEventData(char const *name,
            unsigned col, unsigned row, std::vector<double> &data);

        layerType type;
        char const *defaultKey;
        float defaultValue; // for events without one
        int defaultID; // CC id
        t_jobj fparams;
        std::vector<Cell> cells;
        std::vector<int> orderedEdges;
        float bbox[2];
        unsigned oIndex; // last accessed ordered event
        unsigned section0, section1;
    }; // end of struct layer
    std::vector<Layer> m_layers;

private:
    // We associate a MIDI channel with a note following MPE practice.
    // Since our CC events are already tied to a note, this isn't strictly
    // necessary. It may mak}

    std::vector<unsigned> m_channelPool; // max length 15
    std::map<unsigned, unsigned> m_channelsInUse; // key is layer << 7 | note
    unsigned allocateChannel(unsigned note, unsigned layer);
    unsigned findChannel(unsigned note, unsigned layer, bool release);
    bool hasCustomCCName(char const *nm);
    char const *getCustomCCName(char const *nm);
    struct cmpStr
    {
        // strict weak ordering
        bool operator()(char const *a, char const *b) const
        {
            return std::strcmp(a, b) < 0;
        }
    };
    std::set<char *, cmpStr> m_customCCNames;

private:
    int flattenPartSpec(char const *spec, std::string *partspec);
    void error(char const *msg);
    void log(char const *msg);
    int readnump(char const **);
    void skipspace(char const **);

private:
    void dumpObjectList(char const *nm, t_jobjArray const &);
    void dumpObject(char const *nm, t_jobj const &);
    void dumpMatrix();
};

#endif