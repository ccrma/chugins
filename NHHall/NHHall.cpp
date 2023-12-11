#include "nh_hall.hpp"
#include "chugin.h"

CK_DLL_CTOR(nhhall_ctor);
CK_DLL_DTOR(nhhall_dtor);

CK_DLL_MFUN(nhhall_getRt60);
CK_DLL_MFUN(nhhall_setRt60);
CK_DLL_MFUN(nhhall_getStereo);
CK_DLL_MFUN(nhhall_setStereo);
CK_DLL_MFUN(nhhall_getLowFreq);
CK_DLL_MFUN(nhhall_setLowFreq);
CK_DLL_MFUN(nhhall_getLowRatio);
CK_DLL_MFUN(nhhall_setLowRatio);
CK_DLL_MFUN(nhhall_getHiFreq);
CK_DLL_MFUN(nhhall_setHiFreq);
CK_DLL_MFUN(nhhall_getHiRatio);
CK_DLL_MFUN(nhhall_setHiRatio);
CK_DLL_MFUN(nhhall_getEarlyDiffusion);
CK_DLL_MFUN(nhhall_setEarlyDiffusion);
CK_DLL_MFUN(nhhall_getLateDiffusion);
CK_DLL_MFUN(nhhall_setLateDiffusion);
CK_DLL_MFUN(nhhall_getModRate);
CK_DLL_MFUN(nhhall_setModRate);
CK_DLL_MFUN(nhhall_getModDepth);
CK_DLL_MFUN(nhhall_setModDepth);

CK_DLL_TICKF(nhhall_tickf);

class NHHall {
public:
    // NOTE: The default allocator calls malloc.
    nh_ugens::NHHall<> m_core;

    float m_rt60 = 1.0f;
    float m_stereo = 0.5f;
    float m_lowFreq = 200;
    float m_lowRatio = 0.5f;
    float m_hiFreq = 4000;
    float m_hiRatio = 0.5f;
    float m_earlyDiffusion = 0.5f;
    float m_lateDiffusion = 0.5f;
    float m_modRate = 0.2f;
    float m_modDepth = 0.3f;

    NHHall(float sample_rate)
    : m_core(sample_rate)
    {
        m_core.set_rt60(m_rt60);
        m_core.set_stereo(m_stereo);
        m_core.set_low_shelf_parameters(m_lowFreq, m_lowRatio);
        m_core.set_hi_shelf_parameters(m_hiFreq, m_hiRatio);
        m_core.set_early_diffusion(m_earlyDiffusion);
        m_core.set_late_diffusion(m_lateDiffusion);
        m_core.set_mod_rate(m_modRate);
        m_core.set_mod_rate(m_modDepth);
    }
};

t_CKINT nhhall_unit_offset = 0;

CK_DLL_QUERY(NHHall) {
    QUERY->setname(QUERY, "NHHall");

    QUERY->begin_class(QUERY, "NHHall", "UGen");

    QUERY->add_ctor(QUERY, nhhall_ctor);
    QUERY->add_dtor(QUERY, nhhall_dtor);

    QUERY->add_ugen_funcf(QUERY, nhhall_tickf, NULL, 2, 2);

    QUERY->add_mfun(QUERY, nhhall_setRt60, "float", "rt60");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getRt60, "float", "rt60");

    QUERY->add_mfun(QUERY, nhhall_setStereo, "float", "stereo");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getStereo, "float", "stereo");

    QUERY->add_mfun(QUERY, nhhall_setLowFreq, "float", "lowFreq");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getLowFreq, "float", "lowFreq");

    QUERY->add_mfun(QUERY, nhhall_setLowRatio, "float", "lowRatio");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getLowRatio, "float", "lowRatio");

    QUERY->add_mfun(QUERY, nhhall_setHiFreq, "float", "hiFreq");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getHiFreq, "float", "hiFreq");

    QUERY->add_mfun(QUERY, nhhall_setHiRatio, "float", "hiRatio");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getHiRatio, "float", "hiRatio");

    QUERY->add_mfun(QUERY, nhhall_setEarlyDiffusion, "float", "earlyDiffusion");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getEarlyDiffusion, "float", "earlyDiffusion");

    QUERY->add_mfun(QUERY, nhhall_setLateDiffusion, "float", "lateDiffusion");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getLateDiffusion, "float", "lateDiffusion");

    QUERY->add_mfun(QUERY, nhhall_setModRate, "float", "modRate");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getModRate, "float", "modRate");

    QUERY->add_mfun(QUERY, nhhall_setModDepth, "float", "modDepth");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->add_mfun(QUERY, nhhall_getModDepth, "float", "modDepth");

    nhhall_unit_offset = QUERY->add_mvar(QUERY, "int", "@data", false);

    QUERY->end_class(QUERY);

    return TRUE;
}

CK_DLL_CTOR(nhhall_ctor) {
    OBJ_MEMBER_INT(SELF, nhhall_unit_offset) = 0;

    NHHall* unit = new NHHall(API->vm->srate(VM));

    OBJ_MEMBER_INT(SELF, nhhall_unit_offset) = (t_CKINT)unit;
}

CK_DLL_DTOR(nhhall_dtor) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    if (unit) {
        delete unit;
        OBJ_MEMBER_INT(SELF, nhhall_unit_offset) = 0;
        unit = NULL;
    }
}

CK_DLL_TICKF(nhhall_tickf) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);

    for (int i = 0; i < nframes; i++) {
        float in_left = in[i * 2 + 0];
        float in_right = in[i * 2 + 1];
        std::array<float, 2> core_out = unit->m_core.process(in_left, in_right);
        out[i * 2 + 0] = core_out[0];
        out[i * 2 + 1] = core_out[1];
    }

    return TRUE;
}

CK_DLL_MFUN(nhhall_getRt60) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_dur = unit->m_rt60;
}

CK_DLL_MFUN(nhhall_setRt60) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_rt60 = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_rt60(unit->m_rt60);
    unit->m_core.set_low_shelf_parameters(unit->m_lowFreq, unit->m_lowRatio);
    unit->m_core.set_hi_shelf_parameters(unit->m_hiFreq, unit->m_hiRatio);
    RETURN->v_dur = unit->m_rt60;
}

CK_DLL_MFUN(nhhall_getStereo) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_float = unit->m_stereo;
}

CK_DLL_MFUN(nhhall_setStereo) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_stereo = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_stereo(unit->m_stereo);
    RETURN->v_float = unit->m_stereo;
}

CK_DLL_MFUN(nhhall_getLowFreq) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_float = unit->m_lowFreq;
}

CK_DLL_MFUN(nhhall_setLowFreq) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_lowFreq = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_low_shelf_parameters(unit->m_lowFreq, unit->m_lowRatio);
    RETURN->v_float = unit->m_lowFreq;
}

CK_DLL_MFUN(nhhall_getLowRatio) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_float = unit->m_lowRatio;
}

CK_DLL_MFUN(nhhall_setLowRatio) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_lowRatio = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_low_shelf_parameters(unit->m_lowFreq, unit->m_lowRatio);
    RETURN->v_float = unit->m_lowRatio;
}

CK_DLL_MFUN(nhhall_getHiFreq) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_float = unit->m_hiFreq;
}

CK_DLL_MFUN(nhhall_setHiFreq) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_hiFreq = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_hi_shelf_parameters(unit->m_hiFreq, unit->m_hiRatio);
    RETURN->v_float = unit->m_hiFreq;
}

CK_DLL_MFUN(nhhall_getHiRatio) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_float = unit->m_hiRatio;
}

CK_DLL_MFUN(nhhall_setHiRatio) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_hiRatio = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_hi_shelf_parameters(unit->m_hiFreq, unit->m_hiRatio);
    RETURN->v_float = unit->m_hiRatio;
}

CK_DLL_MFUN(nhhall_getEarlyDiffusion) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_float = unit->m_earlyDiffusion;
}

CK_DLL_MFUN(nhhall_setEarlyDiffusion) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_earlyDiffusion = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_early_diffusion(unit->m_earlyDiffusion);
    RETURN->v_float = unit->m_earlyDiffusion;
}

CK_DLL_MFUN(nhhall_getLateDiffusion) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_float = unit->m_lateDiffusion;
}

CK_DLL_MFUN(nhhall_setLateDiffusion) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_lateDiffusion = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_late_diffusion(unit->m_lateDiffusion);
    RETURN->v_float = unit->m_lateDiffusion;
}

CK_DLL_MFUN(nhhall_getModRate) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_float = unit->m_modRate;
}

CK_DLL_MFUN(nhhall_setModRate) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_modRate = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_mod_rate(unit->m_modRate);
    RETURN->v_float = unit->m_modRate;
}

CK_DLL_MFUN(nhhall_getModDepth) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    RETURN->v_float = unit->m_modDepth;
}

CK_DLL_MFUN(nhhall_setModDepth) {
    NHHall* unit = (NHHall*)OBJ_MEMBER_INT(SELF, nhhall_unit_offset);
    unit->m_modDepth = GET_NEXT_FLOAT(ARGS);
    unit->m_core.set_mod_depth(unit->m_modDepth);
    RETURN->v_float = unit->m_modDepth;
}
