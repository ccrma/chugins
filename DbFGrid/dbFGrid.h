#ifndef dbFGrid_h
#define dbFGrid_h

#include "json.hpp"

#include <string>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;

typedef std::unordered_map<std::string, json> t_jobj;
typedef std::vector<t_jobj> t_jobjArray;


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
    int Read(void *);

private:
    unsigned m_sampleRate;
    unsigned m_nrows, m_ncols;
    struct event
    {
        int row; // midi-note
        float start, dur;
        t_jobj params;
    };
    struct layer
    {
        std::vector<event> events;
        int ievt;
    };
    std::vector<layer> m_layers;

    void dumpObjectList(char const *nm, t_jobjArray const &);
    void dumpObject(char const *nm, t_jobj const &);
    void dumpMatrix();
};

#endif