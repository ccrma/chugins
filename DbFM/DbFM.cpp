#include "DbFM.h"

#include "dexed/Dexed.h"
#include "dexed/msfa/synth.h"
#include "dexed/msfa/freqlut.h"
#include "dexed/msfa/sin.h"
#include "dexed/msfa/exp2.h"
#include "dexed/msfa/env.h"
#include "dexed/msfa/pitchenv.h"
#include "dexed/msfa/aligned_buf.h"
#include "dexed/msfa/fm_op_kernel.h"
#include "dexed/msfa/dx7note.h"
#include "dexed/tuning-library/include/Tunings.h"
#include "dexed/msfa/tuning.h"

const float s_invMax = 1.0f / (1<<24);

DbFM::DbFM(double sampleRate)
{
    Freqlut::init(sampleRate);
    Lfo::init(sampleRate);
    PitchEnv::init(sampleRate);
    Env::init_sr(sampleRate);
    m_fx.init(sampleRate);

    m_synthTuningState = createStandardTuning();
    
    for(int note = 0; note < k_MaxActiveVoices; ++note) 
    {
        m_voices[note].dx7_note = new Dx7Note(m_synthTuningState);
        m_voices[note].keydown = false;
        m_voices[note].sustained = false;
        m_voices[note].live = false;
    }
}

void 
DbFM::AddNoteOn(int note, float vel)
{
    this->AddMidiEvent(144, note, (int)(vel*127.f));
}

void 
DbFM::AddNoteOff(int note, float vel)
{
    this->AddMidiEvent(128, note, (int)(vel*127.f));
}

void 
DbFM::AddMidiEvent(uint8_t status, uint8_t data1, uint8_t data2)
{
    // bypass ringbuffer, proceed straight to ProcessMidiMessage
    // We can do this because chugins execute in the processing thread.
    uint8_t buf[3];
    buf[0] = status;
    buf[1] = data1;
    buf[2] = data2;
    // this->ProcessMidiMessage(buf, 3);
}

void
DbFM::GetSamples(int numSamples, float *buffer)
{
}