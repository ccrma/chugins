#include "dbFGrid.h"

#include <iostream>
#include <fstream>

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
    std::ifstream fstr;
    fstr.open(fnm.c_str());
    if(fstr.good())
    {
        try
        {
            // Because json.hpp accesses everything *by value*, we
            // opt not to live-parse through the events array, rather
            // we'll parse once and free the results.
            json j;
            fstr >> j;
            if(j.contains("fmatrix"))
            {
                std::cerr << "DbFGrid opened " << fnm << "\n";
                auto jfm = j["fmatrix"];
                auto jlayers = jfm["layers"];
                for(int i=0;i<jlayers.size(); i++)
                {
                    this->m_layers.push_back(layer());
                    layer &l = this->m_layers[i];

                    auto jl = jlayers.at(i);
                    auto jelist = jl["events"]; // array of objects

                    for(int j=0;j<jelist.size();j++)
                    {
                        l.events.push_back(event());
                        event &e = l.events[j];
                        auto je = jelist[j]; // an event object
                        for (json::iterator it = je.begin(); it != je.end(); ++it) 
                        {
                            if(it.key() == "l")
                            {
                                auto v = it.value();
                                e.start = v[0];
                                e.dur = v[1];
                                e.row = v[2];
                            }
                            else
                                e.params[it.key()] = it.value();
                        }
                        /*--
                        if((j%5 == 0) && i == 0)
                            this->dumpObject("je", je);
                        --*/
                    }
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
dbFGrid::Close()
{
    return 0;
}

int
dbFGrid::GetNumLayers()
{
    return 0;
}

int
dbFGrid::Rewind()
{
    return 0;
}

int
dbFGrid::Read(void *)
{
    return 0;
}

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
    int i = 0;
    for(auto l : this->m_layers)
    {
        std::cerr << "  l" << i++ << " has " << l.events.size() << " events\n";
        for(auto e : l.events)
        {
            for(auto it=e.params.begin(); it!=e.params.end(); ++it)
                std::cerr << "    " << it->first << ": " << it->second << "\n";
        }
    }
}