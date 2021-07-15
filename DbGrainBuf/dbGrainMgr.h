#ifndef dbGrainMgr_h
#define dbGrainMgr_h

#include <forward_list>
#include <vector>

#include "dbGrain.h"

class dbGrainMgr
{
public:
    dbGrainMgr(unsigned max):
        active(0),
        pruner(this->active)
    {
        this->grainPool.resize(max);
    };

    ~dbGrainMgr() {};

    unsigned GetActiveGrainCount()
    {
        return this->active;
    }

    unsigned GrainPoolSize()
    {
        return this->grainPool.size();
    }

    Grain *Allocate()
    {
        for(auto& value: this->grainPool) 
        {
            if(!value.active)
            {
                this->active++;
                value.active = true;
                this->ActiveGrains.push_front(&value);
                return &value;
            }
        }
        return nullptr;
    }

    void Prune()
    {
        this->ActiveGrains.remove_if(this->pruner);
    }

    void Release(Grain *g)
    {
        if(g) 
        {
            this->active--;
            this->ActiveGrains.remove(g);
            g->active = false;
        }
    }

    std::forward_list<Grain*> ActiveGrains;

private:
    std::vector<Grain> grainPool;
    int active;

    class isInactive
    {
    public:
        isInactive(int &a) : active(a) {}
        bool operator() (Grain * g) 
        {
            if(!g->active)
            {
                this->active--; // owned by enclosing class
                // std::cout << "pruning grain " << g->start  << std::endl;
            }
            return !g->active;
        }
        int &active;
    } pruner;
};

#endif