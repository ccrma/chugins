//-----------------------------------------------------------------------------
// DrumBuf-chuck.cpp - ChucK DLL bindings for DrumBuf
// Provides the ChucK interface to the DrumBuf granular sampler
//-----------------------------------------------------------------------------

#include "chugin.h"
#include "Sampler.h"
#include <iostream>

// Forward declarations
CK_DLL_CTOR( drumbuf_ctor );
CK_DLL_CTOR( drumbuf_ctor_filename );
CK_DLL_DTOR( drumbuf_dtor );
CK_DLL_TICKF( drumbuf_tickf );

CK_DLL_MFUN( drumbuf_read );
CK_DLL_MFUN( drumbuf_noteOn );
CK_DLL_MFUN( drumbuf_noteOff );
CK_DLL_MFUN( drumbuf_setTranspose );
CK_DLL_MFUN( drumbuf_getTranspose );
CK_DLL_MFUN( drumbuf_setRate );
CK_DLL_MFUN( drumbuf_getRate );
CK_DLL_MFUN( drumbuf_setGrainSize );
CK_DLL_MFUN( drumbuf_getGrainSize );
CK_DLL_MFUN( drumbuf_setPlaybackMode );
CK_DLL_MFUN( drumbuf_getPlaybackMode );
CK_DLL_MFUN( drumbuf_setLoopStart );
CK_DLL_MFUN( drumbuf_getLoopStart );
CK_DLL_MFUN( drumbuf_setLoopEnd );
CK_DLL_MFUN( drumbuf_getLoopEnd );
CK_DLL_MFUN( drumbuf_setNotePosition );
CK_DLL_MFUN( drumbuf_setGain );
CK_DLL_MFUN( drumbuf_getGain );
CK_DLL_MFUN( drumbuf_setEnvelope );
CK_DLL_MFUN( drumbuf_getEnvelope );
CK_DLL_MFUN( drumbuf_setAttack );
CK_DLL_MFUN( drumbuf_getAttack );
CK_DLL_MFUN( drumbuf_setHold );
CK_DLL_MFUN( drumbuf_getHold );
CK_DLL_MFUN( drumbuf_setRelease );
CK_DLL_MFUN( drumbuf_getRelease );
CK_DLL_MFUN( drumbuf_setVelocitySens );
CK_DLL_MFUN( drumbuf_getVelocitySens );
CK_DLL_MFUN( drumbuf_setStartMarker );
CK_DLL_MFUN( drumbuf_getStartMarker );
CK_DLL_MFUN( drumbuf_setEndMarker );
CK_DLL_MFUN( drumbuf_getEndMarker );
CK_DLL_MFUN( drumbuf_setReverse );
CK_DLL_MFUN( drumbuf_getReverse );
CK_DLL_MFUN( drumbuf_setInterp );
CK_DLL_MFUN( drumbuf_getInterp );
CK_DLL_MFUN( drumbuf_setNoteTracking );
CK_DLL_MFUN( drumbuf_getNoteTracking );
CK_DLL_MFUN( drumbuf_setRootNote );
CK_DLL_MFUN( drumbuf_getRootNote );
CK_DLL_MFUN( drumbuf_channels );
CK_DLL_MFUN( drumbuf_samplerate );
CK_DLL_MFUN( drumbuf_samples );
CK_DLL_MFUN( drumbuf_length );
CK_DLL_MFUN( drumbuf_isLoaded );
CK_DLL_MFUN( drumbuf_filename );
CK_DLL_MFUN( drumbuf_dump );
CK_DLL_MFUN( drumbuf_setPolyphony );
CK_DLL_MFUN( drumbuf_getPolyphony );
CK_DLL_MFUN( drumbuf_setStealingPolicy );
CK_DLL_MFUN( drumbuf_getStealingPolicy );
CK_DLL_MFUN( drumbuf_noteOffNote );
CK_DLL_MFUN( drumbuf_allNotesOff );
CK_DLL_MFUN( drumbuf_getActiveVoices );
CK_DLL_MFUN( drumbuf_setDecay );
CK_DLL_MFUN( drumbuf_getDecay );
CK_DLL_MFUN( drumbuf_setFadeOut );
CK_DLL_MFUN( drumbuf_getFadeOut );

t_CKINT drumbuf_data_offset = 0;

//-----------------------------------------------------------------------------
// ChucK Info
//-----------------------------------------------------------------------------
CK_DLL_INFO( DrumBuf )
{
    QUERY->setinfo( QUERY, CHUGIN_INFO_CHUGIN_VERSION, "0.0.1" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_AUTHORS, "David Braun" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_EMAIL, "braun@ccrma.stanford.edu" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_DESCRIPTION,
        "Granular sampler with pitch-shifting and time-stretching. "
        "Inspired by Amiga-style samplers and Ableton's Drum Sampler." );
}

CK_DLL_MFUN( drumbuf_setDecay )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT seconds = GET_NEXT_FLOAT(ARGS);
    ds_obj->setDecay(seconds);
    RETURN->v_float = ds_obj->getDecay();
}

CK_DLL_MFUN( drumbuf_getDecay )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->getDecay();
}

CK_DLL_MFUN( drumbuf_setFadeOut )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT seconds = GET_NEXT_FLOAT(ARGS);
    ds_obj->setFadeOut(seconds);
    RETURN->v_float = ds_obj->getFadeOut();
}

CK_DLL_MFUN( drumbuf_getFadeOut )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->getFadeOut();
}

//-----------------------------------------------------------------------------
// ChucK Query
//-----------------------------------------------------------------------------
CK_DLL_QUERY( DrumBuf )
{
    QUERY->setname( QUERY, "DrumBuf" );

    QUERY->begin_class( QUERY, "DrumBuf", "UGen" );
    QUERY->doc_class( QUERY,
        "Granular sampler with two-window overlap for pitch and time stretching. "
        "Supports looping (forward, reverse, ping-pong) and MIDI note mapping." );

    QUERY->add_ctor( QUERY, drumbuf_ctor );

    // Overload constructor with filename parameter
    QUERY->add_ctor( QUERY, drumbuf_ctor_filename );
    QUERY->add_arg( QUERY, "string", "filename" );
    QUERY->doc_func( QUERY, "Constructor that loads a file immediately." );

    QUERY->add_dtor( QUERY, drumbuf_dtor );
    QUERY->add_ugen_funcf( QUERY, drumbuf_tickf, NULL, 1, 2 );

    // File loading
    QUERY->add_mfun( QUERY, drumbuf_read, "int", "read" );
    QUERY->add_arg( QUERY, "string", "filename" );
    QUERY->doc_func( QUERY, "Load an audio file into the sampler. Returns 1 on success, 0 on failure." );

    // Note triggering
    QUERY->add_mfun( QUERY, drumbuf_noteOn, "void", "noteOn" );
    QUERY->add_arg( QUERY, "int", "note" );
    QUERY->add_arg( QUERY, "int", "velocity" );
    QUERY->doc_func( QUERY, "Trigger playback at the position mapped to this MIDI note (0-127)." );

    QUERY->add_mfun( QUERY, drumbuf_noteOff, "void", "noteOff" );
    QUERY->doc_func( QUERY, "Release all voices: runs the envelope's release stage when envelope is on, otherwise stops them immediately (in every playback mode, including LOOP/PINGPONG)." );

    QUERY->add_mfun( QUERY, drumbuf_setNotePosition, "void", "notePos" );
    QUERY->add_arg( QUERY, "int", "note" );
    QUERY->add_arg( QUERY, "float", "position" );
    QUERY->doc_func( QUERY, "Map a MIDI note to a playback start position in seconds. In ONESHOT mode a voice plays its slice [pos, next higher note position or end marker) and auto-stops, so a slice map is one call per slice; with reverse on it plays the same slice backwards." );

    // Pitch/rate control
    QUERY->add_mfun( QUERY, drumbuf_setTranspose, "float", "transpose" );
    QUERY->add_arg( QUERY, "float", "semitones" );
    QUERY->doc_func( QUERY, "Set pitch transpose in semitones." );

    QUERY->add_mfun( QUERY, drumbuf_getTranspose, "float", "transpose" );
    QUERY->doc_func( QUERY, "Get current transpose in semitones." );

    QUERY->add_mfun( QUERY, drumbuf_setRate, "float", "rate" );
    QUERY->add_arg( QUERY, "float", "rate" );
    QUERY->doc_func( QUERY, "Set playback rate (1.0 = normal, 0.5 = half speed, 2.0 = double speed)." );

    QUERY->add_mfun( QUERY, drumbuf_getRate, "float", "rate" );
    QUERY->doc_func( QUERY, "Get current playback rate." );

    // Grain control
    QUERY->add_mfun( QUERY, drumbuf_setGrainSize, "float", "grainSize" );
    QUERY->add_arg( QUERY, "float", "seconds" );
    QUERY->doc_func( QUERY, "Set grain size in seconds (0.001-1.0). Default: 0.05 (50ms)." );

    QUERY->add_mfun( QUERY, drumbuf_getGrainSize, "float", "grainSize" );
    QUERY->doc_func( QUERY, "Get current grain size in seconds." );

    // Playback mode control
    QUERY->add_mfun( QUERY, drumbuf_setPlaybackMode, "int", "playbackMode" );
    QUERY->add_arg( QUERY, "int", "mode" );
    QUERY->doc_func( QUERY, "Set playback mode: 0=ONESHOT (stop at end marker, polyphonic), 1=LOOP (loop between loopStart/loopEnd, polyphonic), 2=PINGPONG (bounce between loopStart/loopEnd, polyphonic), 3=THRU (play through end marker to EOF, MONOPHONIC - new notes stop old ones for drum choking). Default: 0 (ONESHOT)." );

    QUERY->add_mfun( QUERY, drumbuf_getPlaybackMode, "int", "playbackMode" );
    QUERY->doc_func( QUERY, "Get current playback mode." );

    QUERY->add_mfun( QUERY, drumbuf_setLoopStart, "float", "loopStart" );
    QUERY->add_arg( QUERY, "float", "position" );
    QUERY->doc_func( QUERY, "Set loop start position (0.0-1.0)." );

    QUERY->add_mfun( QUERY, drumbuf_getLoopStart, "float", "loopStart" );
    QUERY->doc_func( QUERY, "Get loop start position." );

    QUERY->add_mfun( QUERY, drumbuf_setLoopEnd, "float", "loopEnd" );
    QUERY->add_arg( QUERY, "float", "position" );
    QUERY->doc_func( QUERY, "Set loop end position (0.0-1.0)." );

    QUERY->add_mfun( QUERY, drumbuf_getLoopEnd, "float", "loopEnd" );
    QUERY->doc_func( QUERY, "Get loop end position." );

    // Gain control
    QUERY->add_mfun( QUERY, drumbuf_setGain, "float", "gain" );
    QUERY->add_arg( QUERY, "float", "dB" );
    QUERY->doc_func( QUERY, "Set output gain in decibels (0 = unity, -6 = half volume, +6 = double volume)." );

    QUERY->add_mfun( QUERY, drumbuf_getGain, "float", "gain" );
    QUERY->doc_func( QUERY, "Get current gain in decibels." );

    // Envelope control
    QUERY->add_mfun( QUERY, drumbuf_setEnvelope, "int", "envelope" );
    QUERY->add_arg( QUERY, "int", "enable" );
    QUERY->doc_func( QUERY, "Enable/disable AHR envelope (1 = on, 0 = off). Default: off." );

    QUERY->add_mfun( QUERY, drumbuf_getEnvelope, "int", "envelope" );
    QUERY->doc_func( QUERY, "Get envelope enable state." );

    QUERY->add_mfun( QUERY, drumbuf_setAttack, "float", "attack" );
    QUERY->add_arg( QUERY, "float", "seconds" );
    QUERY->doc_func( QUERY, "Set envelope attack time in seconds (0.0 - 10.0, 0.0 = instant). Default: 0.01." );

    QUERY->add_mfun( QUERY, drumbuf_getAttack, "float", "attack" );
    QUERY->doc_func( QUERY, "Get envelope attack time." );

    QUERY->add_mfun( QUERY, drumbuf_setHold, "float", "hold" );
    QUERY->add_arg( QUERY, "float", "seconds" );
    QUERY->doc_func( QUERY, "Set minimum hold time in seconds (0.0 - 10.0). When > 0.0, release won't start until hold time elapsed, even if noteOff() called early. Default: 0.0 (immediate release on noteOff)." );

    QUERY->add_mfun( QUERY, drumbuf_getHold, "float", "hold" );
    QUERY->doc_func( QUERY, "Get envelope hold time." );

    QUERY->add_mfun( QUERY, drumbuf_setRelease, "float", "release" );
    QUERY->add_arg( QUERY, "float", "seconds" );
    QUERY->doc_func( QUERY, "Set envelope release time in seconds (0.0 - 10.0, 0.0 = instant). Default: 0.1." );

    QUERY->add_mfun( QUERY, drumbuf_getRelease, "float", "release" );
    QUERY->doc_func( QUERY, "Get envelope release time." );

    // Per-hit shaping (independent of the AHR envelope)
    QUERY->add_mfun( QUERY, drumbuf_setDecay, "float", "decay" );
    QUERY->add_arg( QUERY, "float", "seconds" );
    QUERY->doc_func( QUERY, "Set per-hit exponential decay: seconds from onset to -60 dB (0 = off, default). Punchier hits, less smear between overlapping slices." );

    QUERY->add_mfun( QUERY, drumbuf_getDecay, "float", "decay" );
    QUERY->doc_func( QUERY, "Get per-hit decay time in seconds (0 = off)." );

    QUERY->add_mfun( QUERY, drumbuf_setFadeOut, "float", "fadeOut" );
    QUERY->add_arg( QUERY, "float", "seconds" );
    QUERY->doc_func( QUERY, "Set a linear fade over the last N seconds before a ONESHOT/THRU voice reaches its stop boundary (the slice end), so cuts don't click (0 = off, default)." );

    QUERY->add_mfun( QUERY, drumbuf_getFadeOut, "float", "fadeOut" );
    QUERY->doc_func( QUERY, "Get fade-out time in seconds (0 = off)." );

    // Velocity sensitivity
    QUERY->add_mfun( QUERY, drumbuf_setVelocitySens, "float", "velocitySens" );
    QUERY->add_arg( QUERY, "float", "sensitivity" );
    QUERY->doc_func( QUERY, "Set velocity sensitivity (0 = no sensitivity, 1 = linear, >1 = exponential). Default: 1.0." );

    QUERY->add_mfun( QUERY, drumbuf_getVelocitySens, "float", "velocitySens" );
    QUERY->doc_func( QUERY, "Get velocity sensitivity." );

    // Start/end markers
    QUERY->add_mfun( QUERY, drumbuf_setStartMarker, "float", "startMarker" );
    QUERY->add_arg( QUERY, "float", "seconds" );
    QUERY->doc_func( QUERY, "Set start marker in seconds (sample-rate independent). Defines the start of the playable region." );

    QUERY->add_mfun( QUERY, drumbuf_getStartMarker, "float", "startMarker" );
    QUERY->doc_func( QUERY, "Get start marker in seconds." );

    QUERY->add_mfun( QUERY, drumbuf_setEndMarker, "float", "endMarker" );
    QUERY->add_arg( QUERY, "float", "seconds" );
    QUERY->doc_func( QUERY, "Set end marker in seconds (sample-rate independent). Defines the end of the playable region." );

    QUERY->add_mfun( QUERY, drumbuf_getEndMarker, "float", "endMarker" );
    QUERY->doc_func( QUERY, "Get end marker in seconds." );

    // Reverse playback
    QUERY->add_mfun( QUERY, drumbuf_setReverse, "int", "reverse" );
    QUERY->add_arg( QUERY, "int", "enable" );
    QUERY->doc_func( QUERY, "Enable/disable reverse playback (1 = on, 0 = off). ONESHOT: the note's own slice plays backwards from its end; LOOP/PINGPONG: playback starts at the loop end; THRU: from the end of file. Latched per voice at noteOn. Default: off." );

    QUERY->add_mfun( QUERY, drumbuf_getReverse, "int", "reverse" );
    QUERY->doc_func( QUERY, "Get reverse playback state." );

    // Interpolation mode control
    QUERY->add_mfun( QUERY, drumbuf_setInterp, "int", "interp" );
    QUERY->add_arg( QUERY, "int", "mode" );
    QUERY->doc_func( QUERY, "Set sample interpolation mode: 0=DROP (none/fastest), 1=LINEAR (default), 2=LAGRANGE (4-point, higher quality)." );

    QUERY->add_mfun( QUERY, drumbuf_getInterp, "int", "interp" );
    QUERY->doc_func( QUERY, "Get current interpolation mode." );

    // Note tracking control
    QUERY->add_mfun( QUERY, drumbuf_setNoteTracking, "int", "noteTracking" );
    QUERY->add_arg( QUERY, "int", "mode" );
    QUERY->doc_func( QUERY, "Set note tracking mode: 0=OFF (default, note doesn't affect pitch), 1=TRADITIONAL (note affects pitch+time, like tape), 2=GRANULAR (note affects pitch only). Auto-disabled in THRU mode." );

    QUERY->add_mfun( QUERY, drumbuf_getNoteTracking, "int", "noteTracking" );
    QUERY->doc_func( QUERY, "Get current note tracking mode." );

    QUERY->add_mfun( QUERY, drumbuf_setRootNote, "int", "rootNote" );
    QUERY->add_arg( QUERY, "int", "note" );
    QUERY->doc_func( QUERY, "Set root note for pitch tracking (0-127). Default: 60 (middle C)." );

    QUERY->add_mfun( QUERY, drumbuf_getRootNote, "int", "rootNote" );
    QUERY->doc_func( QUERY, "Get current root note." );

    // Query methods
    QUERY->add_mfun( QUERY, drumbuf_channels, "int", "channels" );
    QUERY->doc_func( QUERY, "Get number of channels in loaded file (1=mono, 2=stereo). Returns 0 if no file loaded." );

    QUERY->add_mfun( QUERY, drumbuf_samplerate, "float", "samplerate" );
    QUERY->doc_func( QUERY, "Get sample rate of loaded file in Hz. Returns 0.0 if no file loaded." );

    QUERY->add_mfun( QUERY, drumbuf_samples, "int", "samples" );
    QUERY->doc_func( QUERY, "Get number of samples (frames) in loaded file. Returns 0 if no file loaded." );

    QUERY->add_mfun( QUERY, drumbuf_length, "float", "length" );
    QUERY->doc_func( QUERY, "Get length of loaded file in seconds. Returns 0.0 if no file loaded." );

    QUERY->add_mfun( QUERY, drumbuf_isLoaded, "int", "isLoaded" );
    QUERY->doc_func( QUERY, "Check if a file is currently loaded. Returns 1 if loaded, 0 if not." );

    QUERY->add_mfun( QUERY, drumbuf_filename, "string", "filename" );
    QUERY->doc_func( QUERY, "Get the path of the currently loaded file. Returns empty string if no file loaded." );

    QUERY->add_mfun( QUERY, drumbuf_dump, "void", "dump" );
    QUERY->doc_func( QUERY, "Print loaded file information to console (frames, channels, sample rate, duration)." );

    // Polyphony control
    QUERY->add_mfun( QUERY, drumbuf_setPolyphony, "int", "polyphony" );
    QUERY->add_arg( QUERY, "int", "voices" );
    QUERY->doc_func( QUERY, "Set polyphony (1-64 voices). Default: 16. Shrinking it stops the voices above the new limit." );

    QUERY->add_mfun( QUERY, drumbuf_getPolyphony, "int", "polyphony" );
    QUERY->doc_func( QUERY, "Get current polyphony setting." );

    QUERY->add_mfun( QUERY, drumbuf_setStealingPolicy, "int", "stealingPolicy" );
    QUERY->add_arg( QUERY, "int", "policy" );
    QUERY->doc_func( QUERY, "Set voice stealing policy: 0=STEAL_OLDEST, 1=STEAL_OLDEST_RELEASE (default), 2=STEAL_QUIETEST, 3=STEAL_ROUND_ROBIN." );

    QUERY->add_mfun( QUERY, drumbuf_getStealingPolicy, "int", "stealingPolicy" );
    QUERY->doc_func( QUERY, "Get current voice stealing policy." );

    QUERY->add_mfun( QUERY, drumbuf_noteOffNote, "void", "noteOff" );
    QUERY->add_arg( QUERY, "int", "note" );
    QUERY->doc_func( QUERY, "Release all voices playing this MIDI note (envelope release if enabled, else immediate stop)." );

    QUERY->add_mfun( QUERY, drumbuf_allNotesOff, "void", "allNotesOff" );
    QUERY->doc_func( QUERY, "Immediately stop all voices (panic button)." );

    QUERY->add_mfun( QUERY, drumbuf_getActiveVoices, "int", "activeVoices" );
    QUERY->doc_func( QUERY, "Get count of currently active voices." );

    drumbuf_data_offset = QUERY->add_mvar( QUERY, "int", "@ds_data", false );

    QUERY->end_class( QUERY );

    return TRUE;
}

//-----------------------------------------------------------------------------
// ChucK DLL Functions
//-----------------------------------------------------------------------------

CK_DLL_CTOR( drumbuf_ctor )
{
    OBJ_MEMBER_INT( SELF, drumbuf_data_offset ) = 0;
    Sampler * ds_obj = new Sampler( (double)API->vm->srate(VM) );
    OBJ_MEMBER_INT( SELF, drumbuf_data_offset ) = (t_CKINT)ds_obj;
}

CK_DLL_CTOR( drumbuf_ctor_filename )
{
    // Call the default constructor first
    drumbuf_ctor( SELF, ARGS, VM, SHRED, API );

    // Get the DrumBuf object
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );

    // Get filename argument and load the file (a null string is "no file")
    Chuck_String * ckstr = GET_NEXT_STRING(ARGS);
    std::string filename = ckstr ? API->object->str(ckstr) : "";
    if (!filename.empty()) {
        ds_obj->read(filename);
    }
}

CK_DLL_DTOR( drumbuf_dtor )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    CK_SAFE_DELETE( ds_obj );
    OBJ_MEMBER_INT( SELF, drumbuf_data_offset ) = 0;
}

CK_DLL_TICKF( drumbuf_tickf )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT(SELF, drumbuf_data_offset);
    if( ds_obj )
    {
        // The shared Sampler core ticks in float; SAMPLE is float in stock
        // builds but convert explicitly so a 64-bit-sample ChucK works too.
        float i[2] = { 0.0f, 0.0f };
        float o[2];
        for( t_CKUINT f = 0; f < nframes; f++ )
        {
            i[0] = (float)in[f];
            ds_obj->tick( i, o );
            out[f*2] = (SAMPLE)o[0];
            out[f*2+1] = (SAMPLE)o[1];
        }
    }
    else
    {
        for( t_CKUINT f = 0; f < nframes*2; f++ ) out[f] = 0;
    }
    return TRUE;
}

CK_DLL_MFUN( drumbuf_read )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    Chuck_String * ckstr = GET_NEXT_STRING(ARGS);
    if (!ckstr) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->read(API->object->str(ckstr)) ? 1 : 0;
}

CK_DLL_MFUN( drumbuf_noteOn )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKINT velocity = GET_NEXT_INT(ARGS);
    ds_obj->noteOn(note, velocity);
}

CK_DLL_MFUN( drumbuf_noteOff )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    ds_obj->noteOff();
}

CK_DLL_MFUN( drumbuf_setNotePosition )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT note = GET_NEXT_INT(ARGS);
    t_CKFLOAT position = GET_NEXT_FLOAT(ARGS);
    ds_obj->setNotePosition(note, position);
}

CK_DLL_MFUN( drumbuf_setTranspose )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT semitones = GET_NEXT_FLOAT(ARGS);
    ds_obj->setTranspose(semitones);
    RETURN->v_float = ds_obj->getTranspose();
}

CK_DLL_MFUN( drumbuf_getTranspose )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->getTranspose();
}

CK_DLL_MFUN( drumbuf_setRate )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT rate = GET_NEXT_FLOAT(ARGS);
    ds_obj->setRate(rate);
    RETURN->v_float = ds_obj->getRate();
}

CK_DLL_MFUN( drumbuf_getRate )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 1.0; return; }
    RETURN->v_float = ds_obj->getRate();
}

CK_DLL_MFUN( drumbuf_setGrainSize )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT seconds = GET_NEXT_FLOAT(ARGS);
    ds_obj->setGrainSize(seconds);
    RETURN->v_float = ds_obj->getGrainSize();
}

CK_DLL_MFUN( drumbuf_getGrainSize )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.05; return; }
    RETURN->v_float = ds_obj->getGrainSize();
}

CK_DLL_MFUN( drumbuf_setPlaybackMode )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT mode = GET_NEXT_INT(ARGS);
    ds_obj->setPlaybackMode(mode);
    RETURN->v_int = ds_obj->getPlaybackMode();
}

CK_DLL_MFUN( drumbuf_getPlaybackMode )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->getPlaybackMode();
}

CK_DLL_MFUN( drumbuf_setLoopStart )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT position = GET_NEXT_FLOAT(ARGS);
    ds_obj->setLoopStart(position);
    RETURN->v_float = ds_obj->getLoopStart();
}

CK_DLL_MFUN( drumbuf_getLoopStart )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->getLoopStart();
}

CK_DLL_MFUN( drumbuf_setLoopEnd )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT position = GET_NEXT_FLOAT(ARGS);
    ds_obj->setLoopEnd(position);
    RETURN->v_float = ds_obj->getLoopEnd();
}

CK_DLL_MFUN( drumbuf_getLoopEnd )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 1.0; return; }
    RETURN->v_float = ds_obj->getLoopEnd();
}

CK_DLL_MFUN( drumbuf_setGain )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT dB = GET_NEXT_FLOAT(ARGS);
    ds_obj->setGain(dB);
    RETURN->v_float = ds_obj->getGain();
}

CK_DLL_MFUN( drumbuf_getGain )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->getGain();
}

CK_DLL_MFUN( drumbuf_setEnvelope )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT enable = GET_NEXT_INT(ARGS);
    ds_obj->setEnvelope(enable != 0);
    RETURN->v_int = (ds_obj->getEnvelope() ? 1 : 0);
}

CK_DLL_MFUN( drumbuf_getEnvelope )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->getEnvelope() ? 1 : 0;
}

CK_DLL_MFUN( drumbuf_setAttack )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT seconds = GET_NEXT_FLOAT(ARGS);
    ds_obj->setAttack(seconds);
    RETURN->v_float = ds_obj->getAttack();
}

CK_DLL_MFUN( drumbuf_getAttack )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.01; return; }
    RETURN->v_float = ds_obj->getAttack();
}

CK_DLL_MFUN( drumbuf_setHold )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT seconds = GET_NEXT_FLOAT(ARGS);
    ds_obj->setHold(seconds);
    RETURN->v_float = ds_obj->getHold();
}

CK_DLL_MFUN( drumbuf_getHold )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->getHold();
}

CK_DLL_MFUN( drumbuf_setRelease )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT seconds = GET_NEXT_FLOAT(ARGS);
    ds_obj->setRelease(seconds);
    RETURN->v_float = ds_obj->getRelease();
}

CK_DLL_MFUN( drumbuf_getRelease )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.1; return; }
    RETURN->v_float = ds_obj->getRelease();
}

CK_DLL_MFUN( drumbuf_setVelocitySens )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT sens = GET_NEXT_FLOAT(ARGS);
    ds_obj->setVelocitySens(sens);
    RETURN->v_float = ds_obj->getVelocitySens();
}

CK_DLL_MFUN( drumbuf_getVelocitySens )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 1.0; return; }
    RETURN->v_float = ds_obj->getVelocitySens();
}

CK_DLL_MFUN( drumbuf_setStartMarker )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT seconds = GET_NEXT_FLOAT(ARGS);
    ds_obj->setStartMarker(seconds);
    RETURN->v_float = ds_obj->getStartMarker();
}

CK_DLL_MFUN( drumbuf_getStartMarker )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->getStartMarker();
}

CK_DLL_MFUN( drumbuf_setEndMarker )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKFLOAT seconds = GET_NEXT_FLOAT(ARGS);
    ds_obj->setEndMarker(seconds);
    RETURN->v_float = ds_obj->getEndMarker();
}

CK_DLL_MFUN( drumbuf_getEndMarker )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->getEndMarker();
}

CK_DLL_MFUN( drumbuf_setReverse )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT enable = GET_NEXT_INT(ARGS);
    ds_obj->setReverse(enable != 0);
    RETURN->v_int = (ds_obj->getReverse() ? 1 : 0);
}

CK_DLL_MFUN( drumbuf_getReverse )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->getReverse() ? 1 : 0;
}

CK_DLL_MFUN( drumbuf_setInterp )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT mode = GET_NEXT_INT(ARGS);
    ds_obj->setInterp(mode);
    RETURN->v_int = ds_obj->getInterp();
}

CK_DLL_MFUN( drumbuf_getInterp )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->getInterp();
}

CK_DLL_MFUN( drumbuf_setNoteTracking )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT mode = GET_NEXT_INT(ARGS);
    ds_obj->setNoteTracking(mode);
    RETURN->v_int = ds_obj->getNoteTracking();
}

CK_DLL_MFUN( drumbuf_getNoteTracking )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->getNoteTracking();
}

CK_DLL_MFUN( drumbuf_setRootNote )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT note = GET_NEXT_INT(ARGS);
    ds_obj->setRootNote(note);
    RETURN->v_int = ds_obj->getRootNote();
}

CK_DLL_MFUN( drumbuf_getRootNote )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 60; return; }
    RETURN->v_int = ds_obj->getRootNote();
}

CK_DLL_MFUN( drumbuf_channels )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->channels();
}

CK_DLL_MFUN( drumbuf_samplerate )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->samplerate();
}

CK_DLL_MFUN( drumbuf_samples )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->samples();
}

CK_DLL_MFUN( drumbuf_length )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_float = 0.0; return; }
    RETURN->v_float = ds_obj->length();
}

CK_DLL_MFUN( drumbuf_isLoaded )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->isLoaded() ? 1 : 0;
}

CK_DLL_MFUN( drumbuf_filename )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) {
        RETURN->v_string = (Chuck_String*)API->object->create_string(VM, "", false);
        return;
    }
    std::string fname = ds_obj->filename();
    RETURN->v_string = (Chuck_String*)API->object->create_string(VM, fname.c_str(), false);
}

CK_DLL_MFUN( drumbuf_dump )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    ds_obj->dump();
}

CK_DLL_MFUN( drumbuf_setPolyphony )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT voices = GET_NEXT_INT(ARGS);
    ds_obj->setPolyphony(voices);
    RETURN->v_int = ds_obj->getPolyphony();
}

CK_DLL_MFUN( drumbuf_getPolyphony )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 16; return; }
    RETURN->v_int = ds_obj->getPolyphony();
}

CK_DLL_MFUN( drumbuf_setStealingPolicy )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT policy = GET_NEXT_INT(ARGS);
    ds_obj->setStealingPolicy(policy);
    RETURN->v_int = ds_obj->getStealingPolicy();
}

CK_DLL_MFUN( drumbuf_getStealingPolicy )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 1; return; }  // Default: STEAL_OLDEST_RELEASE
    RETURN->v_int = ds_obj->getStealingPolicy();
}

CK_DLL_MFUN( drumbuf_noteOffNote )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    t_CKINT note = GET_NEXT_INT(ARGS);
    ds_obj->noteOff(note);
}

CK_DLL_MFUN( drumbuf_allNotesOff )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) return;
    ds_obj->allNotesOff();
}

CK_DLL_MFUN( drumbuf_getActiveVoices )
{
    Sampler * ds_obj = (Sampler *)OBJ_MEMBER_INT( SELF, drumbuf_data_offset );
    if (!ds_obj) { RETURN->v_int = 0; return; }
    RETURN->v_int = ds_obj->getActiveVoices();
}
