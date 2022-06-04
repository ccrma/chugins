//-----------------------------------------------------------------------------
// Entaro ChucK Developer!
// This is a Chugin boilerplate, generated by chuginate!
//-----------------------------------------------------------------------------

// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"

// general includes
#include <stdio.h>
#include <limits.h>
#include <iostream>

#include <fluidsynth.h>

CK_DLL_CTOR(fluidsynth_ctor);
CK_DLL_DTOR(fluidsynth_dtor);
CK_DLL_TICKF(fluidsynth_tickf);
CK_DLL_MFUN(fluidsynth_setVerbosity);
CK_DLL_MFUN(fluidsynth_selectPreset); // experimental
CK_DLL_MFUN(fluidsynth_open);
CK_DLL_MFUN(fluidsynth_noteOn);
CK_DLL_MFUN(fluidsynth_noteOff);
CK_DLL_MFUN(fluidsynth_noteOnChan);
CK_DLL_MFUN(fluidsynth_noteOffChan);
CK_DLL_MFUN(fluidsynth_progChange);
CK_DLL_MFUN(fluidsynth_progChangeChan);
CK_DLL_MFUN(fluidsynth_cc);
CK_DLL_MFUN(fluidsynth_ccChan);
CK_DLL_MFUN(fluidsynth_setBank);
CK_DLL_MFUN(fluidsynth_setBankChan);
CK_DLL_MFUN(fluidsynth_setTuning);
CK_DLL_MFUN(fluidsynth_setTuningChan);
CK_DLL_MFUN(fluidsynth_setOctaveTuning);
CK_DLL_MFUN(fluidsynth_setOctaveTuningChan);
CK_DLL_MFUN(fluidsynth_resetTuning);
CK_DLL_MFUN(fluidsynth_resetTuningChan);
CK_DLL_MFUN(fluidsynth_tuneNote);
CK_DLL_MFUN(fluidsynth_tuneNoteChan);
CK_DLL_MFUN(fluidsynth_tuneNotes);
CK_DLL_MFUN(fluidsynth_tuneNotesChan);
CK_DLL_MFUN(fluidsynth_setPitchBend);
CK_DLL_MFUN(fluidsynth_setPitchBendChan);
CK_DLL_MFUN(fluidsynth_resetPitchBend);
CK_DLL_MFUN(fluidsynth_resetPitchBendChan);
CK_DLL_MFUN(fluidsynth_getPitchBend);
CK_DLL_MFUN(fluidsynth_getPitchBendChan);
CK_DLL_MFUN(fluidsynth_hasPreset);

// this is a special offset reserved for Chugin internal data
t_CKINT fluidsynth_data_offset = 0;


// class definition for internal Chugin data
// (note: this isn't strictly necessary, but serves as example
// of one recommended approach)
class FluidSynth
{
public:
    // constructor
    FluidSynth(t_CKFLOAT fs)
    {
        m_srate = fs;
        m_fontId = -1;

        m_settings = new_fluid_settings();
        // midi-bank-select superceded selectPreset(bank, voice, channel)
        // gs: (default) CC0 becomes the bank number, CC32 is ignored.
        // gm: ignores CC0 and CC32 messages.
        // xg: CC32 becomes the bank number, CC0 toggles between melodic or drum channel.
        // mma: bank is calculated as CC0*128+CC32.
        // fluid_settings_setstr(m_settings, "synth.midi-bank-select", "mma");

        m_synth = new_fluid_synth(m_settings);

        fluid_synth_set_sample_rate(m_synth, m_srate);
    }

    ~FluidSynth()
    {
        delete_fluid_synth(m_synth);
        m_synth = NULL;

        delete_fluid_settings(m_settings);
        m_settings = NULL;
    }

    void setVerbosity(int v)
    {
        fluid_settings_setint(m_settings, "synth.verbose", v);
        if(m_synth)
        {
            delete_fluid_synth(m_synth);
            m_synth = NULL;
        }
        m_synth = new_fluid_synth(m_settings);
    }

    // ugen: (0 in, 2 out)
    void tick( SAMPLE *in, SAMPLE *out )
    {
        fluid_synth_write_float(m_synth, 1, out, 0, 0, out+1, 0, 0);
    }

    int open(const std::string &sfont)
    {
        m_fontId = fluid_synth_sfload(m_synth, sfont.c_str(), 1);
        return m_fontId;
    }

    void noteOn(int chan, int key, int vel)
    {
        fluid_synth_noteon(m_synth, chan, key, vel);
    }

    void noteOff(int chan, int key)
    {
        fluid_synth_noteoff(m_synth, chan, key);
    }

    void cc(int chan, int cc, int val)
    {
        fluid_synth_cc(m_synth, chan, cc, val);
    }

    void progChange(int chan, int progNum)
    {
        fluid_synth_program_change(m_synth, chan, progNum);
    }

    void setBank(int chan, int bankNum)
    {
        fluid_synth_bank_select(m_synth, chan, bankNum);
    }

    /* selectPreset combines setBank and progChange and offers
     * the advantage of establishing the correct channel type.
     */
    int selectPreset(int bank, int prog, int chan)
    {
        fluid_synth_set_channel_type(m_synth, chan,     
                (bank >= 128) ? CHANNEL_TYPE_DRUM : CHANNEL_TYPE_MELODIC);
        int result = fluid_synth_program_select(m_synth, chan, m_fontId, bank, prog);
        return 0;
    }

    void setTuning(int chan, Chuck_Array8 * tuning)
    {
        bool allChans = false;
        if (chan < 0) {
            allChans = true;
            chan = 0;
        }
        fluid_synth_activate_key_tuning(m_synth, 0, chan, "", &tuning->m_vector[0], false);
        if (allChans) {
            for (chan = 0 ; chan<16 ; chan++) {
                fluid_synth_activate_tuning(m_synth, chan, 0, 0, false);
            }
        } else {
            fluid_synth_activate_tuning(m_synth, chan, 0, chan, false);
        }
    }

    void setOctaveTuning(int chan, Chuck_Array8 * tuning)
    {
        bool allChans = false;
        if (chan < 0) {
            allChans = true;
            chan = 0;
        }
        fluid_synth_activate_octave_tuning(m_synth, 0, chan, "", &tuning->m_vector[0], false);
        if (allChans) {
            for (chan = 0 ; chan<16 ; chan++) {
                fluid_synth_activate_tuning(m_synth, chan, 0, 0, false);
            }
        } else {
            fluid_synth_activate_tuning(m_synth, chan, 0, chan, false);
        }
    }

    void resetTuning(int chan)
    {
        if (chan < 0) {
            for (chan = 0 ; chan<16 ; chan++) {
                fluid_synth_deactivate_tuning(m_synth, chan, false);
            }
        } else {
            fluid_synth_deactivate_tuning(m_synth, chan, false);
        }
    }

    void tuneNote(int noteNum, double pitch, int chan)
    {
        fluid_synth_tune_notes(m_synth, 0, chan, 1, &noteNum , &pitch, false);
    }

    void tuneNotes(Chuck_Array4 * noteNums, Chuck_Array8 * pitches, int chan)
    {
        /* 
        This ugly hack is required because Chuck_Array4 doesn't actually
        contain 4-byte ints (at least on my 64-bit linux system). So we 
        need to copy the elements into an int array.
        */
        int * noteNumArr;
        noteNumArr = new int[noteNums->size()];
        for (int i = 0; i < noteNums->size(); i++) {
            noteNumArr[i] = (int) noteNums->m_vector[i];
        }
        fluid_synth_tune_notes(m_synth, 0, chan, pitches->size(),
                               noteNumArr, &pitches->m_vector[0], false);

        delete [] noteNumArr;
    }

    void setPitchBend(int pitchbend, int chan)
    {
        fluid_synth_pitch_bend(m_synth, chan, pitchbend);
    }

    int getPitchBend(int chan)
    {
        int pitchbend;
        fluid_synth_get_pitch_bend(m_synth, chan, &pitchbend);
        return pitchbend;
    }

private:
    // instance data
    float m_srate;
    fluid_settings_t *m_settings;
    fluid_synth_t *m_synth;
    int m_fontId;
};

// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( fluidsynth )
{
    QUERY->setname(QUERY, "FluidSynth");

    // begin the class definition
    QUERY->begin_class(QUERY, "FluidSynth", "UGen");

    QUERY->add_ctor(QUERY, fluidsynth_ctor);
    QUERY->add_dtor(QUERY, fluidsynth_dtor);

    QUERY->add_ugen_funcf(QUERY, fluidsynth_tickf, NULL, 0, 2);

    // NOTE: if this is to be a UGen with more than 1 channel,
    // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
    // and declare a tickf function using CK_DLL_TICKF

    QUERY->add_mfun(QUERY, fluidsynth_open, "int", "open");
    QUERY->add_arg(QUERY, "string", "file");

    QUERY->add_mfun(QUERY, fluidsynth_noteOn, "void", "noteOn");
    QUERY->add_arg(QUERY, "int", "note");
    QUERY->add_arg(QUERY, "int", "velocity");

    QUERY->add_mfun(QUERY, fluidsynth_noteOff, "void", "noteOff");
    QUERY->add_arg(QUERY, "int", "note");

    QUERY->add_mfun(QUERY, fluidsynth_noteOnChan, "void", "noteOn");
    QUERY->add_arg(QUERY, "int", "note");
    QUERY->add_arg(QUERY, "int", "velocity");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_noteOffChan, "void", "noteOff");
    QUERY->add_arg(QUERY, "int", "note");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_cc, "void", "cc");
    QUERY->add_arg(QUERY, "int", "cc");
    QUERY->add_arg(QUERY, "int", "val");

    QUERY->add_mfun(QUERY, fluidsynth_ccChan, "void", "cc");
    QUERY->add_arg(QUERY, "int", "cc");
    QUERY->add_arg(QUERY, "int", "val");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_progChange, "void", "progChange");
    QUERY->add_arg(QUERY, "int", "progNum");

    QUERY->add_mfun(QUERY, fluidsynth_progChangeChan, "void", "progChange");
    QUERY->add_arg(QUERY, "int", "progNum");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_setBank, "void", "setBank");
    QUERY->add_arg(QUERY, "int", "bankNum");

    QUERY->add_mfun(QUERY, fluidsynth_setBankChan, "void", "setBank");
    QUERY->add_arg(QUERY, "int", "bankNum");
    QUERY->add_arg(QUERY, "int", "chan");

    // presets provided to bypass weird logic associated with GM percussion
    QUERY->add_mfun(QUERY, fluidsynth_selectPreset, "int", "selectPreset");
    QUERY->add_arg(QUERY, "int", "bank");
    QUERY->add_arg(QUERY, "int", "prog");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_setVerbosity, "int", "setVerbosity");
    QUERY->add_arg(QUERY, "int", "verbosity");

    QUERY->add_mfun(QUERY, fluidsynth_setTuning, "void", "setTuning");
    QUERY->add_arg(QUERY, "float[]", "tuning");

    QUERY->add_mfun(QUERY, fluidsynth_setTuningChan, "void", "setTuning");
    QUERY->add_arg(QUERY, "float[]", "tuning");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_setOctaveTuning, "void", "setOctaveTuning");
    QUERY->add_arg(QUERY, "float[]", "tuning");

    QUERY->add_mfun(QUERY, fluidsynth_setOctaveTuningChan, "void", "setOctaveTuning");
    QUERY->add_arg(QUERY, "float[]", "tuning");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_resetTuning, "void", "resetTuning");

    QUERY->add_mfun(QUERY, fluidsynth_resetTuningChan, "void", "resetTuning");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_tuneNote, "void", "tuneNote");
    QUERY->add_arg(QUERY, "int", "noteNum");
    QUERY->add_arg(QUERY, "float", "pitch");

    QUERY->add_mfun(QUERY, fluidsynth_tuneNoteChan, "void", "tuneNote");
    QUERY->add_arg(QUERY, "int", "noteNum");
    QUERY->add_arg(QUERY, "float", "pitch");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_tuneNotes, "void", "tuneNotes");
    QUERY->add_arg(QUERY, "int[]", "noteNums");
    QUERY->add_arg(QUERY, "float[]", "pitches");

    QUERY->add_mfun(QUERY, fluidsynth_tuneNotesChan, "void", "tuneNotes");
    QUERY->add_arg(QUERY, "int[]", "noteNums");
    QUERY->add_arg(QUERY, "float[]", "pitches");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_setPitchBend, "void", "setPitchBend");
    QUERY->add_arg(QUERY, "int", "pitchbend");

    QUERY->add_mfun(QUERY, fluidsynth_setPitchBendChan, "void", "setPitchBend");
    QUERY->add_arg(QUERY, "int", "pitchbend");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_resetPitchBend, "void", "resetPitchBend");

    QUERY->add_mfun(QUERY, fluidsynth_resetPitchBendChan, "void", "resetPitchBend");
    QUERY->add_arg(QUERY, "int", "chan");

    QUERY->add_mfun(QUERY, fluidsynth_getPitchBend, "int", "getPitchBend");

    QUERY->add_mfun(QUERY, fluidsynth_getPitchBendChan, "int", "getPitchBend");
    QUERY->add_arg(QUERY, "int", "chan");

    fluidsynth_data_offset = QUERY->add_mvar(QUERY, "int", "@f_data", false);

    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}

// implementation for the constructor
CK_DLL_CTOR(fluidsynth_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, fluidsynth_data_offset) = 0;

    // instantiate our internal c++ class representation
    FluidSynth * bcdata = new FluidSynth(API->vm->get_srate(API, SHRED));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, fluidsynth_data_offset) = (t_CKINT) bcdata;
}


// implementation for the destructor
CK_DLL_DTOR(fluidsynth_dtor)
{
    // get our c++ class pointer
    FluidSynth * bcdata = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    // check it
    if( bcdata )
    {
        // clean up
        delete bcdata;
        OBJ_MEMBER_INT(SELF, fluidsynth_data_offset) = 0;
        bcdata = NULL;
    }
}


// implementation for tick function
CK_DLL_TICKF(fluidsynth_tickf)
{
    // get our c++ class pointer
    FluidSynth * c = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    // invoke our tick function; store in the magical out variable
    if(c) c->tick(in, out);

    // yes
    return TRUE;
}


// example implementation for setter
CK_DLL_MFUN(fluidsynth_open)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    std::string str = GET_NEXT_STRING_SAFE(ARGS);

    RETURN->v_int = f_data->open(str);
}


// example implementation for getter
CK_DLL_MFUN(fluidsynth_noteOn)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKINT velocity = GET_NEXT_INT(ARGS);

    f_data->noteOn(0, note, velocity);
}

// example implementation for getter
CK_DLL_MFUN(fluidsynth_noteOff)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT note = GET_NEXT_INT(ARGS);

    f_data->noteOff(0, note);
}

// example implementation for getter
CK_DLL_MFUN(fluidsynth_noteOnChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKINT velocity = GET_NEXT_INT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    f_data->noteOn(chan, note, velocity);
}

// example implementation for getter
CK_DLL_MFUN(fluidsynth_noteOffChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    f_data->noteOff(chan, note);
}

CK_DLL_MFUN(fluidsynth_cc)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT cc = GET_NEXT_INT(ARGS);
    t_CKINT val = GET_NEXT_INT(ARGS);

    f_data->cc(0, cc, val);
}

CK_DLL_MFUN(fluidsynth_ccChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT cc = GET_NEXT_INT(ARGS);
    t_CKINT val = GET_NEXT_INT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);
    f_data->cc(chan, cc, val);
}

CK_DLL_MFUN(fluidsynth_progChange)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT progNum = GET_NEXT_INT(ARGS);

    f_data->progChange(0, progNum);
}

CK_DLL_MFUN(fluidsynth_progChangeChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT progNum = GET_NEXT_INT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    f_data->progChange(chan, progNum);
}

CK_DLL_MFUN(fluidsynth_setBank)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    t_CKINT bankNum = GET_NEXT_INT(ARGS);

    f_data->setBank(0, bankNum);
}

CK_DLL_MFUN(fluidsynth_setBankChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT bankNum = GET_NEXT_INT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    f_data->setBank(chan, bankNum);
}

CK_DLL_MFUN(fluidsynth_setVerbosity)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    f_data->setVerbosity(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(fluidsynth_selectPreset)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    t_CKINT bankNum = GET_NEXT_INT(ARGS);
    t_CKINT progNum = GET_NEXT_INT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);
    RETURN->v_int = f_data->selectPreset(bankNum, progNum, chan);
}

CK_DLL_MFUN(fluidsynth_setTuning)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    Chuck_Array8 * tuning = (Chuck_Array8 *) GET_NEXT_OBJECT(ARGS);

    if (tuning->size() != 128 ) {
        printf("FluidSynth ERROR: setTuning() requires a tuning array of exactly 128 values\n");
        return;
    }

    f_data->setTuning(-1, tuning);
}

CK_DLL_MFUN(fluidsynth_setTuningChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    Chuck_Array8 * tuning = (Chuck_Array8 *) GET_NEXT_OBJECT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    if (tuning->size() != 128 ) {
        printf("FluidSynth ERROR: setOctaveTuning() requires a tuning array of exactly 12 values\n");
        return;
    }

    f_data->setTuning(chan, tuning);
}

CK_DLL_MFUN(fluidsynth_setOctaveTuning)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    Chuck_Array8 * tuning = (Chuck_Array8 *) GET_NEXT_OBJECT(ARGS);

    if (tuning->size() != 12 ) {
        printf("FluidSynth ERROR: setOctaveTuning() requires a tuning array of exactly 12 values\n");
        return;
    }

    f_data->setOctaveTuning(-1, tuning);
}

CK_DLL_MFUN(fluidsynth_setOctaveTuningChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    Chuck_Array8 * tuning = (Chuck_Array8 *) GET_NEXT_OBJECT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    if (tuning->size() != 12 ) {
        printf("FluidSynth ERROR: setOctaveTuning() requires a tuning array of exactly 12 values\n");
        return;
    }

    f_data->setOctaveTuning(chan, tuning);
}

CK_DLL_MFUN(fluidsynth_resetTuning)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    f_data->resetTuning(-1);
}

CK_DLL_MFUN(fluidsynth_resetTuningChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    f_data->resetTuning(chan);
}

CK_DLL_MFUN(fluidsynth_tuneNote)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT noteNum = GET_NEXT_INT(ARGS);
    t_CKFLOAT pitch = GET_NEXT_FLOAT(ARGS);

    f_data->tuneNote(noteNum, pitch, 0);
}

CK_DLL_MFUN(fluidsynth_tuneNoteChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    t_CKINT noteNum = GET_NEXT_INT(ARGS);
    t_CKFLOAT pitch = GET_NEXT_FLOAT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    f_data->tuneNote(noteNum, pitch, chan);
}

CK_DLL_MFUN(fluidsynth_tuneNotes)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    Chuck_Array4 * noteNums = (Chuck_Array4 *) GET_NEXT_OBJECT(ARGS);
    Chuck_Array8 * pitches = (Chuck_Array8 *) GET_NEXT_OBJECT(ARGS);

    if (pitches->size() != noteNums->size()) {
        printf("FluidSynth ERROR: tuneNotes requires pitches and noteNums arrays to be the same length\n");
        return;
    }

    f_data->tuneNotes(noteNums, pitches, 0);
}

CK_DLL_MFUN(fluidsynth_tuneNotesChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    Chuck_Array4 * noteNums = (Chuck_Array4 *) GET_NEXT_OBJECT(ARGS);
    Chuck_Array8 * pitches = (Chuck_Array8 *) GET_NEXT_OBJECT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    if (pitches->size() != noteNums->size()) {
        printf("FluidSynth ERROR: tuneNotes requires pitches and noteNums arrays to be the same length\n");
        return;
    }

    f_data->tuneNotes(noteNums, pitches, chan);
}

CK_DLL_MFUN(fluidsynth_setPitchBend)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    t_CKINT pitchbend = GET_NEXT_INT(ARGS);

    // Reset out-of-range values to min or max
    if (pitchbend < 0) pitchbend = 0;
    else if (pitchbend > 16383) pitchbend = 16383;

    f_data->setPitchBend(pitchbend, 0);
}

CK_DLL_MFUN(fluidsynth_setPitchBendChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    t_CKINT pitchbend = GET_NEXT_INT(ARGS);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    // Reset out-of-range values to min or max
    if (pitchbend < 0) pitchbend = 0;
    else if (pitchbend > 16383) pitchbend = 16383;

    f_data->setPitchBend(pitchbend, chan);
}

CK_DLL_MFUN(fluidsynth_resetPitchBend)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    for (int chan = 0; chan < 16; chan++) {
        f_data->setPitchBend(8192, chan);
    }
}

CK_DLL_MFUN(fluidsynth_resetPitchBendChan)
{
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    f_data->setPitchBend(8192, chan);
}

CK_DLL_MFUN(fluidsynth_getPitchBend) {
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);

    RETURN->v_int = f_data->getPitchBend(0);
}

CK_DLL_MFUN(fluidsynth_getPitchBendChan) {
    FluidSynth * f_data = (FluidSynth *) OBJ_MEMBER_INT(SELF, fluidsynth_data_offset);
    t_CKINT chan = GET_NEXT_INT(ARGS);

    RETURN->v_int = f_data->getPitchBend(chan);
}
