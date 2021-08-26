#ifndef AbcQueue_h
#define AbcQueue_h
#include <cassert>

/* a queue to track notes of varying duration 
 * and generate note-off events in the right order.
 */

/* cpp rewrite based on 
 * abc2midi - program to convert abc files to MIDI files.
 * Copyright (C) 1999 James Allwright
 * e-mail: J.R.Allwright@westminster.ac.uk
 * (GNU license version 2 or later)
 */

/* genmidi.c communicates with queues.c mainly through the
 * functions addtoQ and timestep. The complexity comes in the
 * handling of chords. When another note in a chord is passed,
 * addtoQ detemines whether other notes in the Q structure
 * overlap in time with this chord and modifies the delay item
 * of the note which finish later so that it is relative to the
 * end of the earlier note. Normally all notes in the chord end
 * at the same as specifiedy abc standard, so the delay of the
 * other notes cached in the Q structure should be set to zero.
 */

class AbcQueue
{
public:
    static const int QSize = 50;

    class Client
    {
    public:
        virtual int getid() = 0;
        virtual void error(char const *) = 0;
        virtual void progress_sequence(int chan) = 0;
        virtual void midi_noteoff(long delta_time, int pitch, int chan) = 0;
        virtual void midi_event(long delta_time, int evt, int chan, 
                        char data[], int len) = 0;
        virtual void midi_event_with_delay(long delta_time, 
                        int evt, int chan, 
                        char data[], int len) = 0;
                        
        virtual void getEffectsState(long **delta_time, 
            int *bendstate, int *bendvelocity, int *bendacceleration) = 0;
        virtual void getEffectsState(long **delta_time, 
            int *bendstate, int *bendnvals, int **benddata) = 0;
        virtual void getEffectsState(long **delta_time, 
            int *bendstate, int *bendnvals, int **benddata,
            int *nlayers, int **controlnvals,  int **controldefaults,
            int **controldata) = 0;
    };

    AbcQueue(Client *c) : client(c) {}

    struct Qitem
    {
        Qitem() { next = -1; }
        Qitem(Qitem const &rhs)
        {
            this->delay = rhs.delay;
            this->pitch = rhs.pitch;
            this->chan = rhs.chan;
            this->effect = rhs.effect;
            this->next = rhs.next;
        }

        int delay;
        int pitch;
        int chan;
        int effect;
        int next;
    };

    Qitem queue[QSize];

    void init();
    void clear(long &delta_time, long &delta_time_track0,
               int &totalnotedelay, long &tracklen);
    void append(int num, int denom, int pitch, int chan, int effect, int dur,
                int div_factor, int notedelay);
    void remove(int i);

    // see timestep comments in .cpp
    void timestep(int t, int atEnd, 
                long &delta_time, long &delta_time_track0,
                int &totalnotedelay, long &tracklen); 

private:
    void advance(int);
    void dump();
    void check();

    void note_effect();
    void note_effect2();
    void note_effect3();
    void note_effect5(int chan);

    struct event  // used by effect5
    {
        int time;
        char cmd;
        char data1;
        char data2;
    };
    static int compare_events(const void *a, const void *b);
    void output_eventlist(event *list, int nsize, int chan);

private:
    Client *client;

    int head;
    int freehead;
    int freetail;
};

#endif