/* cpp rewrite based on 
 * abc2midi - program to convert abc files to MIDI files.
 * Copyright (C) 1999 James Allwright
 * e-mail: J.R.Allwright@westminster.ac.uk
 * (GNU license version 2 or later)
 * 
 * AbcQueue is a means to serialize note-end events.
 */

#include "AbcQueue.h"
#include "AbcMidiFile.h"
#include <cstdio>
#include <cstdlib> // qsort

/* initialize queue of notes waiting to finish */
void
AbcQueue::init()
{
    this->head = -1;
    this->freehead = 0;
    for(int i=0; i<QSize-1; i++) 
        this->queue[i].next = i + 1;
    this->queue[QSize - 1].next = -1;
    this->freetail = QSize - 1;
}

/* remove gchord requests */
void
AbcQueue::clear(long &delta_time, long &delta_time_track0,
               int &totalnotedelay, long &tracklen)
{
    int time = 0;
    int i;
    while((this->head != -1) && (this->queue[this->head].pitch == -1)) 
    {
        time = time + this->queue[this->head].delay;
        i = this->head;
        this->head = this->queue[i].next;
        this->queue[i].next = this->freehead;
        this->freehead = i;
    };
    if(this->head != -1)
    {
        this->timestep(time, 1, 
            delta_time, delta_time_track0, totalnotedelay, tracklen);
    }
    /* do any remaining note offs, but don't do chord request */
    while(this->head != -1) 
    {
        this->client->error("Sustained notes beyond end of track");
        this->timestep(this->queue[this->head].delay+1, 1,
            delta_time, delta_time_track0, totalnotedelay, tracklen);
    }
    /* to avoid transient artefacts at end of track */
    this->timestep(25, 0,
        delta_time, delta_time_track0, totalnotedelay, tracklen);
}

void
AbcQueue::append(int num, int denom, int pitch, int chan, int effect, 
    int dur, int div_factor, int notedelay)
{

    // printf("%d: addtoQ: %d %d %d %d %d\n",
    //    this->client->getid(), num,denom,pitch,chan,dur);
    /* find free space */
    int i;
    if (this->freehead == -1) 
    {
        this->client->error("Too many notes in chord - probably missing ']' or '+'");
        return;
    } 
    else 
    {
        i = this->freehead;
        this->freehead = this->queue[i].next;
    }
    Qitem &qi = this->queue[i];
    qi.pitch = pitch;
    qi.chan = chan;
    qi.effect = effect; 

    /* find place in queue */
    int wait = ((div_factor*num)/denom) + dur;
    int done = 0;
    int *ptr = &this->head;
    while(!done) 
    {
        if(*ptr == -1) 
        {
            *ptr = i;
            qi.next = -1;
            qi.delay = wait;
            done = 1;
        } 
        else 
        {
            Qitem &qp = this->queue[*ptr];
            if(qp.delay > wait) 
            {
                qp.delay = qp.delay - wait - notedelay;
                if(qp.delay < 0) 
                    qp.delay = 0;
                qi.next = *ptr;
                qi.delay = wait;
                *ptr = i;
                done = 1;
            } 
            else 
            {
                wait = wait - qp.delay;
                ptr = &(qp.next);
            }
        }
    }
}

void
AbcQueue::remove(int i)
{
    if(i == -1) 
    {
        this->dump();
        this->client->error("Internal error - nothing to remove from queue");
    } 
    else 
    {
        if(this->queue[this->head].delay != 0) 
        {
            this->dump();
            this->client->error("Internal error - queue head has non-zero time");
        }
        this->head = this->queue[i].next;
        this->queue[i].next = freehead;
        this->freehead = i;
    }
    // printf("%d: removeQ: %d (%d)\n", this->client->getid(), i, this->freehead);
}

/* timestep is called by delay() in AbcGenMidi typically at the 
 * end of a note, chord or rest. It is also called by clearQ in
 * this file. Timestep is not only responsible for sending the
 * midi_noteoff command for any expired notes in the Q structure
 * but also maintains the delta_time global variable which is
 * shared with genmidi.c. Timestep also calls advanceQ() in
 * this file which updates all the delay variables for the items
 * in the Q structure to reflect the current MIDI time. Timestep
 * also calls removefromQ in this file which cleans out expired
 * notes from the Q structure. To make things even more complicated
 * timestep runs the dogchords and the dodrums for bass/chordal
 * and drum accompaniments by calling the function progress_sequence.
 * Dogchords and dodrums may also call addtoQ changing the contents
 * of the Q structure array. The tracklen variable in MIDI time
 * units is also maintained here.
*/ 
void
AbcQueue::timestep(int t, int atEnd, 
            long &delta_time, long &delta_time_track0,
            int &totalnotedelay, long &tracklen)
{
    int time = t;
    /* process any notes waiting to finish */
    while((this->head != -1) && (this->queue[this->head].delay < time)) 
    {
        int headtime = this->queue[this->head].delay;
        delta_time = delta_time + (long) headtime;
        delta_time_track0 = delta_time_track0 + (long) headtime;
        time = time - headtime;
        this->advance(headtime);
        if(this->queue[this->head].pitch == -1) 
        {
            if(!atEnd) 
                this->client->progress_sequence(this->queue[this->head].chan);
        }
        else 
        {
            Qitem &qi = this->queue[this->head];
            if(qi.effect == 0) 
            {
                this->client->midi_noteoff(delta_time, qi.pitch, qi.chan);
                tracklen = tracklen + delta_time;
                delta_time = 0L;
            }
            else 
            {
                tracklen = tracklen + delta_time; 
                switch(qi.effect) 
                {
                case 1:
                    note_effect();
                    break;
   
                case 2:
                    note_effect2();
                    break;

                case 3:
                    note_effect3();
                    break;

                case 10:
                    /*note_effect4(Q[Qhead].chan);*/
                    note_effect5(qi.chan);
                    break;
                } 
                delta_time = 0L;
            }
        }
        this->remove(this->head);
    }
    if(this->head != -1) 
        this->advance(time);
    delta_time = delta_time + (long)time - totalnotedelay;
    delta_time_track0 = delta_time_track0 + (long)time - totalnotedelay; /* [SS] 2010-06-27*/
}

void
AbcQueue::advance(int t)
{
    if(this->head == -1) 
        this->client->error("Internal error - empty queue");
    else 
        this->queue[this->head].delay -= t;
}

void
AbcQueue::dump()
{
    printf("Qhead = %d freehead = %d freetail = %d\n", 
            this->head, this->freehead, this->freetail);
    int t = this->head;
    printf("Q:");
    while (t != -1) 
    {
        Qitem &qi = this->queue[t];
        printf("p(%d)-%d->", qi.pitch, qi.delay);
        t = qi.next;
    }
    printf("\n");
}

void
AbcQueue::check()
{
    int used[QSize];
    int failed = 0;
    for(int i=0; i<QSize; i++) 
        used[i] = 0;

    int qused = 0;
    int nextitem = this->head;
    while(nextitem != -1) 
    {
        qused = qused + 1;
        used[nextitem] = 1;
        nextitem = this->queue[nextitem].next;
        if((nextitem < -1) || (nextitem >= QSize)) 
        {
            failed = 1;
            printf("Queue corrupted Q[].next = %d\n", nextitem);
        }
    }
    int qfree = 0;
    nextitem = this->freehead;
    while(nextitem != -1) 
    {
        qfree = qfree + 1;
        used[nextitem] = 1;
        nextitem = this->queue[nextitem].next;
        if((nextitem < -1) || (nextitem >= QSize)) 
        {
            failed = 1;
            printf("Free Queue corrupted Q[].next = %d\n", nextitem);
        }
    }
    if (qfree + qused < QSize) 
    {
        failed = 1;
        printf("qfree = %d qused = %d\n", qused, qfree);
    };
    for(int i=0; i<QSize; i++) 
    {
        if(used[i] == 0) 
        {
            printf("Not used element %d\n", i);
            failed = 1;
        }
    }
    if(this->queue[this->freetail].next != -1) 
    {
        printf("freetail = %d, Q[freetail].next = %d\n", 
            this->freetail, this->queue[this->freetail].next);
    }
    if (failed == 1) 
    {
        this->dump();
        this->client->error("Qcheck failed");
    }
}

/* -------------------------------------------------------------------- */
/* Bends a note along the shape of a parabola. The note is
   split into 8 segments. Given the bendacceleration and
   initial bend velocity, the new pitch bend is computed
   for each time segment.
*/
/* note_effect is used only if !bend! is called without
   calling %%MIDI bendstring or %%MIDI bendvelocity */

void 
AbcQueue::note_effect()
{
    long *delta_time;
    int bendstate, bendvelocity, bendacceleration;
    this->client->getEffectsState(&delta_time, 
        &bendstate, &bendvelocity, &bendacceleration);

    int delta8 = *delta_time / 8;
    int pitchbend;
    unsigned char data[2];
    int velocity;
    pitchbend = bendstate;  /* [SS] 2014-09-09 */
    velocity = bendvelocity;

    Qitem &qh = this->queue[this->head];
    for(int i=0;i<8;i++) 
    {
        pitchbend = pitchbend + velocity;
        velocity = velocity + bendacceleration;
        if(pitchbend > 16383) 
            pitchbend = 16383;
        if(pitchbend < 0) 
            pitchbend = 0;
        data[0] = (char) (pitchbend&0x7f);
        data[1] = (char) ((pitchbend>>7)&0x7f);
        this->client->midi_event(delta8, MidiEvent::pitch_wheel, qh.chan, 
            data, 2);
        *delta_time -= delta8;
    }
    this->client->midi_noteoff(*delta_time, qh.pitch, qh.chan);
    pitchbend = bendstate; /* [SS] 2014-09-22 */
    data[0] = (char) (pitchbend&0x7f);
    data[1] = (char) ((pitchbend>>7)&0x7f);
    this->client->midi_event(*delta_time, MidiEvent::pitch_wheel, qh.chan, 
        data, 2);
}

/* implements %%MIDI bendstring command. The note is split
 *  into n segments where n is the number of pitch shift values
 *  supplied in bendstring. For every segment, the pitchbend
 *  is shifted by the next pitch shift value.
 */
void 
AbcQueue::note_effect2()
{
    long *delta_time;
    int bendnvals;
    int *benddata;
    int bendstate;
    this->client->getEffectsState(&delta_time, 
        &bendstate, &bendnvals, &benddata);
    int delta = *delta_time/bendnvals;
    int pitchbend = bendstate;
    unsigned char data[2];
    Qitem &qh = this->queue[this->head];
    for (int i=0;i<bendnvals;i++) 
    {
        pitchbend = pitchbend + benddata[i];
        if(pitchbend > 16383) 
            pitchbend = 16383;
        else
        if(pitchbend < 0) 
            pitchbend = 0;
 
        data[0] = (char) (pitchbend&0x7f);
        data[1] = (char) ((pitchbend>>7)&0x7f);
        if (i == 0) /* [SS] 2014-09-24 */
        {
            this->client->midi_event(0, MidiEvent::pitch_wheel,
                qh.chan,data, 2);
        }
        else 
        {
            this->client->midi_event(delta, MidiEvent::pitch_wheel,  
                qh.chan,data,2 );

            *delta_time -= delta;
        }
    }
    this->client->midi_noteoff(*delta_time, qh.pitch, qh.chan);
    *delta_time = 0;
    pitchbend = bendstate;
    data[0] = (char) (pitchbend&0x7f);
    data[1] = (char) ((pitchbend>>7)&0x7f);
    this->client->midi_event(*delta_time, MidiEvent::pitch_wheel,
        qh.chan, data, 2);
}

/* handles the %%MIDI bendstring command when only one pitch
 * shift is given.
 */
void 
AbcQueue::note_effect3()
{
    long *delta_time;
    int bendnvals;
    int *benddata;
    int bendstate;
    this->client->getEffectsState(&delta_time, 
        &bendstate, &bendnvals, &benddata);

    int delta = 0;
    int pitchbend;
    unsigned char data[2];
    pitchbend = bendstate; 
    pitchbend = pitchbend + benddata[0];
    if(pitchbend > 16383) 
        pitchbend = 16383;
    if(pitchbend < 0) 
        pitchbend = 0;
    data[0] = (char) (pitchbend&0x7f);
    data[1] = (char) ((pitchbend>>7)&0x7f);

    Qitem &qh = this->queue[this->head];
    this->client->midi_event(delta, MidiEvent::pitch_wheel, qh.chan, data, 2);
    this->client->midi_noteoff(*delta_time, qh.pitch, qh.chan);

    pitchbend = bendstate;
    data[0] = (char) (pitchbend&0x7f);
    data[1] = (char) ((pitchbend>>7)&0x7f);
    this->client->midi_event(delta, MidiEvent::pitch_wheel, qh.chan, data, 2);
}

/* This procedure merges the controlstring with the
 *  bendstring and uses it to shape the note. The control
 *  commands are prepared and stored in the eventstruc
 *  eventlist[] array, sorted chronologically and sent
 *  to the MIDI file.
 */
void
AbcQueue::note_effect5(int chan)
{
    long *delta_time;
    int bendnvals;
    int *benddata;
    int bendstate;
    int nlayers;
    int *controlnvals; 
    int *controldefaults; 
    int *controldata; // 2D [][256]
    this->client->getEffectsState(&delta_time, 
        &bendstate, &bendnvals, &benddata, 
        &nlayers, &controlnvals, &controldefaults, &controldata);

    event eventlist[1024];
    int delta=0, notetime;
    int last_delta;
    int controltype, controlval;
    unsigned char data[2];
    int j = 0; 
    int pitchbend = bendstate;
    int initial_bend = bendstate;

    if(bendnvals > 0) 
    {
        delta = *delta_time/bendnvals;
        notetime = 0;
        for(int i = 0; i<bendnvals; i++) 
        {
            pitchbend = benddata[i] + pitchbend;
            if (pitchbend > 16383) 
            {
               this->client->error("pitchbend exceeds 16383");
               pitchbend = 16383;
            }
            else
            if(pitchbend < 0)
            {
               this->client->error("pitchbend is less than 0");
               pitchbend = 0;
            }
            eventlist[j].time = notetime;
            eventlist[j].cmd = (char) MidiEvent::pitch_wheel;
            eventlist[j].data1 = pitchbend & 0x7f;
            eventlist[j].data2 = (pitchbend >> 7) & 0x7f;
            notetime += delta;
            j++;
        } 
    }
    for(int layer=0;layer <= nlayers; layer++) 
    {
        if(controlnvals[layer] > 1) 
        {
            delta = *delta_time/(controlnvals[layer] -1);
            notetime = 0;
            int controltype = controldata[layer*256]; // controldata is [][256]
            if (controltype > 127 || controltype < 0) 
            {
                this->client->error("controller must be in range 0 to 127");
                return;
            }
    
            for(int i=1; i < controlnvals[layer]; i++) 
            {
                controlval = controldata[layer*256 + i];
                if (controlval < 0) 
                {
                    this->client->error("control data must be zero or greater");
                    controlval = 0;
                }
                if (controlval > 127) 
                {
                    this->client->error("control data must be less or equal to 127");
                    controlval = 127;
                }
                eventlist[j].time = notetime;
                eventlist[j].cmd  = (char) MidiEvent::control_change;
                eventlist[j].data1 = controltype;
                eventlist[j].data2 = controlval;
                notetime += delta;
                if(j < 1023) 
                    j++; 
                else 
                   this->client->error("eventlist too complex");
               
            }
        }
    }
    if(j > 1)
    {
        qsort(eventlist, j, sizeof(event), compare_events);
            /*print_eventlist(eventlist,j); */
    }
    this->output_eventlist(eventlist, j, chan);

    last_delta = delta - eventlist[j-1].time; /* [SS] 2017-06-10 */
    Qitem &qh = this->queue[this->head];
    this->client->midi_noteoff(last_delta, qh.pitch, qh.chan);

    for(int layer=0;layer <= nlayers;layer++) 
    {
        controltype = controldata[layer*256]; // controldata[layer][0]
        data[0] = (char) controltype;
        data[1] = (char) controldefaults[controltype];
        this->client->midi_event_with_delay(0, 
            MidiEvent::control_change, chan, data, 2);
    }
    if(bendnvals > 0)
    {
        /* restore pitchbend to its original state */
        pitchbend = initial_bend; /* [SS] 2014-09-25 */
        data[0] = (char) (pitchbend&0x7f);
        data[1] = (char) ((pitchbend>>7)&0x7f);
        this->client->midi_event_with_delay(0, 
            MidiEvent::pitch_wheel, chan, data, 2);
    }
}

/*static*/ int
AbcQueue::compare_events(const void *a, const void *b)
{
    event const *ia = (event const *)a;
    event const *ib = (event const *)b;
    if(ia->time < ib->time)
        return -1;
    else
    if(ia->time > ib->time)
        return 1;
    else
        return 0;
}

void 
AbcQueue::output_eventlist(event *list, int nsize, int chan) 
{
    unsigned char data[2];
    int miditime = 0;
    for(int i = 0; i<nsize; i++) 
    {
        int delta = list[i].time - miditime;
        miditime = list[i].time;
        data[0] = (char) list[i].data1; 
        data[1] = (char) list[i].data2;
        char cmd = list[i].cmd;
        if(cmd == -80) 
        {
            this->client->midi_event_with_delay(delta, 
                MidiEvent::control_change,
                chan, data, 2);
        }
        else
        if(cmd == -32) 
        {
            this->client->midi_event(delta, 
                MidiEvent::pitch_wheel,
                this->queue[this->head].chan, data, 2);
        }
    }
}

