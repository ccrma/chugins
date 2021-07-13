#ifndef dbGrainMgr_h
#define dbGrainMgr_h

#include <forward_list>
#include <vector>

#include "dbGrain.h"

class dbGrainMgr
{
public:
    dbGrainMgr(unsigned max):
        active(0)
    {
        this->grainPool.resize(max);

    };

    ~dbGrainMgr() {};

    void Prune()
    {
        this->ActiveGrains.remove_if(this->pruner);
    }

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
        bool operator() (Grain * g) {return !g->active;}
    } pruner;
};

#endif