//-----------------------------------------------------------------------------
// Entaro ChucK Developer!
// This is a chugin boilerplate, generated by chuginate!
//-----------------------------------------------------------------------------
// NOTE by default, chuginate generates a new UGen subclass in this file
//      but it is possible, of course, to create non-UGen classes in a chugin!
// To modify this generated file for a non-UGen class...
//      1. in QUERY->begin_class(), change "UGen" to a different ChucK class
//         (e.g., `QUERY->begin_class(QUERY, "AZA", "Object");`)
//      2. remove or commment out the line containing QUERY->add_ugen_func()
//      3. that's it; the rest is no different for UGens/non-UGens
//-----------------------------------------------------------------------------
// NOTE once you have built this into a chugin (AZA.chug), here are a few
//      helpful tools for testing / probing / verifying your new chugin!
//
// chuginate also generated a AZA-test.ck boilerplate ChucK program
//      to help test your chugin (see AZA-test.ck for more instructions)
//
// run `chuck --chugin-probe` to probe what chugins would be loaded, and
//      from where in the chugin search paths
//
// run `chuck -v3 --loop` to see what chugins are actually loaded at runtime,
//      with more info and error reporting than with --chugin-probe
//
// other helpful chugin-related flags include:
//      --chugin:<filename>
//      --chugin-path:(path) / -G(path)
//      --chugin-load:{on/off}
//
// for more information on command-line options:
//      https://chuck.stanford.edu/doc/program/options.html
// for more information on chugins:
//      https://chuck.stanford.edu/extend/
//-----------------------------------------------------------------------------
// happy chucking & chugging!
//-----------------------------------------------------------------------------
#include "chugin.h"

// Valley
#include "Valley/Plateau/Dattorro.hpp"
#include "Valley/dsp/modulation/LinearEnvelope.hpp"

#include <cstdio>

// Helpers ============================================================

#define BEGIN_CLASS(type, base) QUERY->begin_class(QUERY, type, base)
#define END_CLASS() QUERY->end_class(QUERY)
#define CTOR(func) QUERY->add_ctor(QUERY, func)
#define DTOR(func) QUERY->add_dtor(QUERY, func)
#define MVAR(type, name, is_const) QUERY->add_mvar(QUERY, type, name, is_const)
#define MFUN(func, ret, name) QUERY->add_mfun(QUERY, func, ret, name)
#define SFUN(func, ret, name) QUERY->add_sfun(QUERY, func, ret, name)
#define SVAR(type, name, val) QUERY->add_svar(QUERY, type, name, true, val)
#define ARG(type, name) QUERY->add_arg(QUERY, type, name)
#define DOC_FUNC(doc) QUERY->doc_func(QUERY, doc)
#define DOC_CLASS(doc) QUERY->doc_class(QUERY, doc)
#define DOC_VAR(doc) QUERY->doc_var(QUERY, doc)
#define ADD_EX(path) QUERY->add_ex(QUERY, path)
#define TICKF(func, in_channels, out_channels) QUERY->add_ugen_funcf(QUERY, func, NULL, in_channels, out_channels)

#define GET_NEXT_INT_ARRAY(ptr) (*((Chuck_ArrayInt **&)ptr)++)
#define GET_NEXT_INT_ARRAY(ptr) (*((Chuck_ArrayInt **&)ptr)++)
#define GET_NEXT_FLOAT_ARRAY(ptr) (*((Chuck_ArrayFloat **&)ptr)++)
#define GET_NEXT_VEC2_ARRAY(ptr) (*((Chuck_ArrayVec2 **&)ptr)++)
#define GET_NEXT_VEC3_ARRAY(ptr) (*((Chuck_ArrayVec3 **&)ptr)++)
#define GET_NEXT_VEC4_ARRAY(ptr) (*((Chuck_ArrayVec4 **&)ptr)++)
#define GET_NEXT_OBJECT_ARRAY(ptr) (*((Chuck_ArrayInt **&)ptr)++)

#define CK_TYPE_VOID "void"
#define CK_TYPE_INT "int"
#define CK_TYPE_FLOAT "float"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(x, lo, hi) (MIN(hi, MAX(lo, x)))
#define CLAMP01(x) (MIN(1.f, MAX(0.f, x)))

float lerp(float x, float a, float b)
{
    return a + x * (b - a);
}

float invLerp(float x, float a, float b)
{
    return (x - a) / (b - a);
}

float rescale(float x, float a, float b, float c, float d)
{
    return lerp(invLerp(x, a, b), c, d);
}

// ==================================================================

CK_DLL_INFO(AZA)
{
    // the version string of this chugin, e.g., "v1.2.1"
    QUERY->setinfo(QUERY, CHUGIN_INFO_CHUGIN_VERSION, "v0.0.0");
    // the author(s) of this chugin, e.g., "Alice Baker & Carl Donut"
    QUERY->setinfo(QUERY, CHUGIN_INFO_AUTHORS, "Andrew Zhu Aday");
    // text description of this chugin; what is it? what does it do? who is it for?
    QUERY->setinfo(QUERY, CHUGIN_INFO_DESCRIPTION, "AZA Collection");
    // (optional) URL of the homepage for this chugin
    QUERY->setinfo(QUERY, CHUGIN_INFO_URL, "");
    // (optional) contact email
    QUERY->setinfo(QUERY, CHUGIN_INFO_EMAIL, "");
}

CK_DLL_CTOR(plateau_ctor);
CK_DLL_DTOR(plateau_dtor);
CK_DLL_TICKF(plateau_tick);

CK_DLL_MFUN(plateau_set_dry);
CK_DLL_MFUN(plateau_get_dry);
CK_DLL_MFUN(plateau_set_wet);
CK_DLL_MFUN(plateau_get_wet);
CK_DLL_MFUN(plateau_set_hold);
CK_DLL_MFUN(plateau_get_hold);

CK_DLL_MFUN(plateau_clear);

CK_DLL_MFUN(plateau_get_predelay_time);
CK_DLL_MFUN(plateau_set_predelay_time);

CK_DLL_MFUN(plateau_get_predelay_smoothing);
CK_DLL_MFUN(plateau_set_predelay_smoothing);

CK_DLL_MFUN(plateau_get_size);
CK_DLL_MFUN(plateau_set_size);

CK_DLL_MFUN(plateau_get_size_smoothing);
CK_DLL_MFUN(plateau_set_size_smoothing);

CK_DLL_MFUN(plateau_get_diffusion);
CK_DLL_MFUN(plateau_set_diffusion);

CK_DLL_MFUN(plateau_get_decay);
CK_DLL_MFUN(plateau_set_decay);

CK_DLL_MFUN(plateau_get_inputLow);
CK_DLL_MFUN(plateau_set_inputLow);
CK_DLL_MFUN(plateau_get_inputHigh);
CK_DLL_MFUN(plateau_set_inputHigh);

CK_DLL_MFUN(plateau_get_reverbLow);
CK_DLL_MFUN(plateau_set_reverbLow);
CK_DLL_MFUN(plateau_get_reverbHigh);
CK_DLL_MFUN(plateau_set_reverbHigh);

CK_DLL_MFUN(plateau_get_mod_speed);
CK_DLL_MFUN(plateau_set_mod_speed);
CK_DLL_MFUN(plateau_get_mod_depth);
CK_DLL_MFUN(plateau_set_mod_depth);
CK_DLL_MFUN(plateau_get_mod_shape);
CK_DLL_MFUN(plateau_set_mod_shape);

CK_DLL_MFUN(plateau_get_tuned);
CK_DLL_MFUN(plateau_set_tuned);

CK_DLL_MFUN(plateau_get_diffuse);
CK_DLL_MFUN(plateau_set_diffuse);

// this is a special offset reserved for chugin internal data
t_CKINT plateau_data_offset = 0;

struct Plateau
{
    // CV scaling
    const float sizeMin = 0.0025f;
    const float sizeMax = 4.0f;
    const float decayMin = 0.1f;
    const float decayMax = 0.9999f;
    const float modSpeedMin = 0.f;
    const float modSpeedMax = 1.f;
    const float modDepthMin = 0.f;
    const float modDepthMax = 16.f;
    const float modShapeMin = 0.001f;
    const float modShapeMax = 0.999f;

    float minus20dBGain = 0.1f;

    float wet = 0.5f;
    float dry = 1.f;

    float preDelay_smoothing = 0.0001f;
    float preDelay_curr = 0.f;
    float preDelay_target = preDelay_curr;

    float size = .5f;
    float scaled_size_curr = 1.0f;
    float scaled_size_target = scaled_size_curr;
    float size_smoothing = .0001f;

    float diffusion = 1.f;
    float decay = 0.75f;

    float inputLowpassCutoff = 0.f;
    float inputHighpassCutoff = 1.f;
    float reverbLowpassCutoff = 0.f;
    float reverbHighpassCutoff = 1.f;

    float modSpeed = 0.01f;
    float modShape = 0.5f;
    float modDepth = 0.03125f;

    bool freeze = false;
    bool cleared = true;
    bool fadeOut = false;
    bool fadeIn = false;

    Dattorro reverb;
    LinearEnvelope envelope;

    bool tuned = false;
    bool diffuseInput = true;

    Plateau(float sample_rate);
    void updateParams();
    void tick(SAMPLE *in, SAMPLE *out);
    void onSampleRateChange(float sample_rate);
};

Plateau::Plateau(float sample_rate) : reverb(192000, 16, sizeMax)
{
    onSampleRateChange(sample_rate);
    envelope.setTime(0.004f);
    envelope._value = 1.f;
    updateParams();
}

void Plateau::updateParams()
{
    reverb.freeze(freeze);

    reverb.setInputFilterLowCutoffPitch(10 * inputLowpassCutoff);
    reverb.setInputFilterHighCutoffPitch(10 * inputHighpassCutoff);
    reverb.enableInputDiffusion(diffuseInput);
    reverb.setDecay(decay);

    // now rescale the size for reverb tank
    reverb.setTankDiffusion(10 * diffusion);

    reverb.setTankFilterLowCutFrequency(10 * reverbLowpassCutoff);
    reverb.setTankFilterHighCutFrequency(10 * reverbHighpassCutoff);

    reverb.setTankModSpeed(modSpeed * modSpeed * 99.f + 1.f);
    reverb.setTankModDepth(rescale(modDepth, 0.f, 1.f, modDepthMin, modDepthMax));
    reverb.setTankModShape(rescale(modShape, 0.f, 1.f, modShapeMin, modShapeMax));
}

void Plateau::tick(SAMPLE *in, SAMPLE *out)
{
    { // clear logic
        if (fadeOut && envelope._justFinished)
        {
            reverb.clear();
            fadeOut = false;
            fadeIn = true;
            envelope.setStartEndPoints(0.f, 1.f);
            envelope.trigger();
        }
        if (fadeIn && envelope._justFinished)
        {
            fadeIn = false;
            cleared = true;
            envelope._value = 1.f;
        }
        envelope.process();
    }

    // update reverb params
    {
        // interpolate size_curr torwards target
        if (tuned)
        {
            scaled_size_target = sizeMin * powf(2.f, size * 5.f);
            scaled_size_target = CLAMP(scaled_size_target, sizeMin, 2.5f);
        }
        else
        {
            scaled_size_target = rescale(size * size, 0.f, 1.f, 0.01f, sizeMax);
            scaled_size_target = CLAMP(scaled_size_target, 0.01f, sizeMax);
        }
        scaled_size_curr = scaled_size_curr + size_smoothing * (scaled_size_target - scaled_size_curr);
        reverb.setTimeScale(scaled_size_curr);

        preDelay_curr = preDelay_curr + preDelay_smoothing * (preDelay_target - preDelay_curr);
        reverb.setPreDelay(preDelay_curr);
    }

    float leftInput = CLAMP(in[0], -1.f, 1.f);
    float rightInput = CLAMP(in[1], -1.f, 1.f);

    // reverb.process(leftInput * minus20dBGain * envelope._value,
    //                rightInput * minus20dBGain * envelope._value);
    reverb.process(leftInput * envelope._value,
                   rightInput * envelope._value);

    float wet_scale = 1.f; // taken from Plateau.cpp L314
    SAMPLE left_wet = wet_scale * reverb.getLeftOutput() * wet * envelope._value;
    SAMPLE right_wet = wet_scale * reverb.getRightOutput() * wet * envelope._value;

    SAMPLE leftOutput = leftInput * dry + left_wet;
    SAMPLE rightOutput = rightInput * dry + right_wet;

    out[0] = CLAMP(leftOutput, -1.f, 1.f);
    out[1] = CLAMP(rightOutput, -1.f, 1.f);
}

void Plateau::onSampleRateChange(float sample_rate)
{
    reverb.setSampleRate(sample_rate);
    envelope.setSampleRate(sample_rate);
}

CK_DLL_QUERY(AZA)
{
    QUERY->setname(QUERY, "AZA");

    { // Plateau
        BEGIN_CLASS("Plateau", "UGen");
        plateau_data_offset = MVAR("int", "@plateau_data", false);

        CTOR(plateau_ctor);
        DTOR(plateau_dtor);

        TICKF(plateau_tick, 2, 2);

        MFUN(plateau_set_dry, CK_TYPE_VOID, "dry");
        ARG(CK_TYPE_FLOAT, "level");
        DOC_FUNC("level clamped to [0, 1]");

        MFUN(plateau_get_dry, "float", "dry");

        MFUN(plateau_set_wet, "void", "wet");
        ARG("float", "level");
        DOC_FUNC("level clamped to [0, 1]");

        MFUN(plateau_get_wet, "float", "wet");

        MFUN(plateau_set_hold, "void", "hold");
        ARG("int", "latch");
        DOC_FUNC("if true, sets the decay of the reverb to infinite so that it will continuously reverberate.");

        MFUN(plateau_get_hold, "int", "hold");

        MFUN(plateau_clear, "void", "clear");
        DOC_FUNC(" Purges the reverberator. Useful for creating gated reverb effects, or diminshing complete chaos.");

        MFUN(plateau_get_predelay_time, "float", "preDelay");
        DOC_FUNC("Get pre-delay time in seconds");

        MFUN(plateau_set_predelay_time, "void", "preDelay");
        ARG("float", "time_secs");
        DOC_FUNC("Set pre-delay time in seconds. Clamped to [0,1]");

        MFUN(plateau_get_predelay_smoothing, "float", "preDelaySmoothing");

        MFUN(plateau_set_predelay_smoothing, "void", "preDelaySmoothing");
        ARG("float", "alpha");
        DOC_FUNC("The interpolation factor used to smoothly transition between delay amount. Default .0001. Set closer to 1 for more instantaneous updates, at the cost of introducing crunchy distortion from skipping too far in the delay line");

        MFUN(plateau_get_size, "float", "size");

        MFUN(plateau_set_size, "void", "size");
        ARG("float", "size");
        DOC_FUNC("Sets the overall delay time and apparent 'size' of the reverb. Ranges from very short to extremely long. Clamped to [0,1]");

        MFUN(plateau_get_size_smoothing, "float", "sizeSmoothing");

        MFUN(plateau_set_size_smoothing, "void", "sizeSmoothing");
        ARG("float", "alpha");
        DOC_FUNC("The interpolation factor used to smoothly transition between reverb sizes. Default .0001. Set closer to 1 for more instantaneous updates, at the cost of introducing crunchy distortion from skipping too far in the delay line");

        MFUN(plateau_get_diffusion, "float", "diffusion");

        MFUN(plateau_set_diffusion, "void", "diffusion");
        ARG("float", "amt");
        DOC_FUNC("Controls how diffused and smeared the reverb is. No diffusion results in audible echoes. Clamped to [0,1]");

        MFUN(plateau_get_decay, "float", "decay");

        MFUN(plateau_set_decay, "void", "decay");
        ARG("float", "amt");
        DOC_FUNC("Sets the speed at which the signal decays over time. The maximum setting results in a long reverb that evolves over time into a very slowly dying cloud of sound. Clamped to [0,1]");

        MFUN(plateau_get_inputLow, "float", "inputLow");
        MFUN(plateau_set_inputLow, "void", "inputLow");
        ARG("float", "amt");
        DOC_FUNC("Sets input lowpass filter cutoff. Input is normalized and clamped between [0,1]. Internally remapped to [14, 14080]hz");

        MFUN(plateau_get_inputHigh, "float", "inputHigh");
        MFUN(plateau_set_inputHigh, "void", "inputHigh");
        ARG("float", "amt");
        DOC_FUNC("Sets input highpass filter cutoff. Input is normalized and clamped between [0,1]. Internally remapped to [14, 14080]hz");

        MFUN(plateau_get_reverbLow, "float", "reverbLow");
        MFUN(plateau_set_reverbLow, "void", "reverbLow");
        ARG("float", "amt");
        DOC_FUNC("Sets reverberator lowpass filter cutoff. Input is normalized and clamped between [0,1]. Internally remapped to [14, 14080]hz");

        MFUN(plateau_get_reverbHigh, "float", "reverbHigh");
        MFUN(plateau_set_reverbHigh, "void", "reverbHigh");
        ARG("float", "amt");
        DOC_FUNC("Sets reverberator highpass filter cutoff. Input is normalized and clamped between [0,1]. Internally remapped to [14, 14080]hz");

        MFUN(plateau_get_mod_speed, "float", "modSpeed");
        MFUN(plateau_get_mod_depth, "float", "modDepth");
        MFUN(plateau_get_mod_shape, "float", "modShape");
        MFUN(plateau_set_mod_speed, "void", "modSpeed");
        ARG("float", "amt");
        DOC_FUNC("input clamped to [0,1]. Controls the speed of the LFOs which modulate the delay times of the all-pass filters in the reverberation tank");

        MFUN(plateau_set_mod_depth, "void", "modDepth");
        ARG("float", "amt");
        DOC_FUNC("input clamped to [0,1]. Controls the modulation depth of the LFOs which modulate the delay times of the all-pass filters in the reverberation tank");

        MFUN(plateau_set_mod_shape, "void", "modShape");
        ARG("float", "amt");
        DOC_FUNC("input clamped to [0,1]. Controls the shape of the LFOs which modulate the delay times of the all-pass filters in the reverberation tank");

        MFUN(plateau_get_tuned, "int", "tuned");

        MFUN(plateau_set_tuned, "void", "tuned");
        ARG("int", "tuned");
        DOC_FUNC("If true, enables Tuned Mode: Shortens the delay times and tunes the all-pass filters to 1V/Oct so that the reverb can be 'played'.");

        MFUN(plateau_get_diffuse, "int", "diffuse");

        MFUN(plateau_set_diffuse, "void", "diffuse");
        ARG("int", "diffuse");
        DOC_FUNC("If true, Enages the input diffusion stage that pre-diffuses and smears the signal before reverberation. Setting to false sharpens the input signal.");

        END_CLASS();
    } // Plateau

    return true;
}

CK_DLL_CTOR(plateau_ctor)
{
    OBJ_MEMBER_INT(SELF, plateau_data_offset) = 0;
    Plateau *plateau = new Plateau(API->vm->srate(VM));
    OBJ_MEMBER_INT(SELF, plateau_data_offset) = (t_CKINT)plateau;
}

CK_DLL_DTOR(plateau_dtor)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    CK_SAFE_DELETE(plateau);
    OBJ_MEMBER_INT(SELF, plateau_data_offset) = 0;
}

CK_DLL_TICKF(plateau_tick)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    for (int i = 0; i < nframes; i++)
        plateau->tick(in + 2 * i, out + 2 * i);

    return TRUE;
}

CK_DLL_MFUN(plateau_set_dry)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    t_CKFLOAT dry = GET_NEXT_FLOAT(ARGS);
    plateau->dry = CLAMP(dry, 0.f, 1.f);
}

CK_DLL_MFUN(plateau_get_dry)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->dry;
}

CK_DLL_MFUN(plateau_set_wet)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    t_CKFLOAT wet = GET_NEXT_FLOAT(ARGS);
    plateau->wet = CLAMP01(wet);
}

CK_DLL_MFUN(plateau_get_wet)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->wet;
}

CK_DLL_MFUN(plateau_set_hold)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    plateau->freeze = (bool)GET_NEXT_INT(ARGS);
    plateau->reverb.freeze(plateau->freeze);
}

CK_DLL_MFUN(plateau_get_hold)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_int = plateau->freeze ? 1 : 0;
}

CK_DLL_MFUN(plateau_clear)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);

    bool not_in_middle_of_clear = plateau->cleared;
    if (not_in_middle_of_clear)
    {
        plateau->cleared = false;
        plateau->fadeOut = true;
        plateau->envelope.setStartEndPoints(1.f, 0.f);
        plateau->envelope.trigger();
    }
    // remainder of clear is handled in plateau->tick()
}

CK_DLL_MFUN(plateau_get_predelay_time)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->preDelay_target;
}

CK_DLL_MFUN(plateau_set_predelay_time)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float preDelay = GET_NEXT_FLOAT(ARGS);
    plateau->preDelay_target = CLAMP(preDelay, 0.f, 1.f);
}

CK_DLL_MFUN(plateau_get_predelay_smoothing)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->preDelay_smoothing;
}

CK_DLL_MFUN(plateau_set_predelay_smoothing)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    plateau->preDelay_smoothing = GET_NEXT_FLOAT(ARGS);
}

CK_DLL_MFUN(plateau_get_size)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->size;
}

CK_DLL_MFUN(plateau_set_size)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float size = GET_NEXT_FLOAT(ARGS);
    size = CLAMP(size, 0.f, 1.f);
    plateau->size = size;
}

CK_DLL_MFUN(plateau_get_size_smoothing)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->size_smoothing;
}

CK_DLL_MFUN(plateau_set_size_smoothing)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    plateau->size_smoothing = GET_NEXT_FLOAT(ARGS);
}

CK_DLL_MFUN(plateau_get_diffusion)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->diffusion;
}

CK_DLL_MFUN(plateau_set_diffusion)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float diff = GET_NEXT_FLOAT(ARGS);
    diff = CLAMP01(diff);
    plateau->diffusion = diff;

    plateau->reverb.setTankDiffusion(10 * diff);
}

CK_DLL_MFUN(plateau_get_decay)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->decay;
}

CK_DLL_MFUN(plateau_set_decay)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float decay = GET_NEXT_FLOAT(ARGS);
    decay = CLAMP(decay, 0.f, 1.f);
    plateau->decay = decay;

    plateau->reverb.setDecay(decay);
}

CK_DLL_MFUN(plateau_get_inputLow)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->inputLowpassCutoff;
}

CK_DLL_MFUN(plateau_set_inputLow)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float lp_cutoff = GET_NEXT_FLOAT(ARGS);
    lp_cutoff = CLAMP01(lp_cutoff);
    plateau->inputLowpassCutoff = lp_cutoff;

    plateau->reverb.setInputFilterLowCutoffPitch(10 * lp_cutoff);
}

CK_DLL_MFUN(plateau_get_inputHigh)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->inputHighpassCutoff;
}

CK_DLL_MFUN(plateau_set_inputHigh)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float cutoff = GET_NEXT_FLOAT(ARGS);
    cutoff = CLAMP01(cutoff);
    plateau->inputHighpassCutoff = cutoff;

    plateau->reverb.setInputFilterHighCutoffPitch(10 * cutoff);
}

CK_DLL_MFUN(plateau_get_reverbLow)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->reverbLowpassCutoff;
}

CK_DLL_MFUN(plateau_set_reverbLow)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float lp_cutoff = GET_NEXT_FLOAT(ARGS);
    lp_cutoff = CLAMP01(lp_cutoff);
    plateau->reverbLowpassCutoff = lp_cutoff;

    plateau->reverb.setTankFilterLowCutFrequency(10 * lp_cutoff);
}

CK_DLL_MFUN(plateau_get_reverbHigh)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->reverbHighpassCutoff;
}

CK_DLL_MFUN(plateau_set_reverbHigh)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float cutoff = GET_NEXT_FLOAT(ARGS);
    cutoff = CLAMP01(cutoff);
    plateau->reverbHighpassCutoff = cutoff;

    plateau->reverb.setTankFilterHighCutFrequency(10 * cutoff);
}

CK_DLL_MFUN(plateau_get_mod_speed)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->modSpeed;
}

CK_DLL_MFUN(plateau_set_mod_speed)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float mod = GET_NEXT_FLOAT(ARGS);
    plateau->modSpeed = CLAMP01(mod);

    plateau->reverb.setTankModSpeed(plateau->modSpeed * plateau->modSpeed * 99.f + 1.f);
}

CK_DLL_MFUN(plateau_get_mod_depth)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->modDepth;
}

CK_DLL_MFUN(plateau_set_mod_depth)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float mod = GET_NEXT_FLOAT(ARGS);
    plateau->modDepth = CLAMP01(mod);

    plateau->reverb.setTankModDepth(rescale(plateau->modDepth, 0.f, 1.f, plateau->modDepthMin, plateau->modDepthMax));
}

CK_DLL_MFUN(plateau_get_mod_shape)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_float = plateau->modShape;
}

CK_DLL_MFUN(plateau_set_mod_shape)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    float mod = GET_NEXT_FLOAT(ARGS);
    plateau->modShape = CLAMP01(mod);

    plateau->reverb.setTankModShape(rescale(plateau->modShape, 0.f, 1.f, plateau->modShapeMin, plateau->modShapeMax));
}

CK_DLL_MFUN(plateau_get_tuned)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_int = plateau->tuned ? 1 : 0;
}

CK_DLL_MFUN(plateau_set_tuned)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    plateau->tuned = (bool)GET_NEXT_INT(ARGS);
}

CK_DLL_MFUN(plateau_get_diffuse)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    RETURN->v_int = plateau->diffuseInput ? 1 : 0;
}

CK_DLL_MFUN(plateau_set_diffuse)
{
    Plateau *plateau = (Plateau *)OBJ_MEMBER_INT(SELF, plateau_data_offset);
    plateau->diffuseInput = (bool)GET_NEXT_INT(ARGS);
    plateau->reverb.enableInputDiffusion(plateau->diffuseInput);
}