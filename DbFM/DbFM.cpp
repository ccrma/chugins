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
    Exp2::init();
    Tanh::init();
    Sin::init();

    m_synthTuningState = createStandardTuning();

    m_controllers.values_[kControllerPitchRangeUp] = 3;
    m_controllers.values_[kControllerPitchRangeDn] = 3;
    m_controllers.values_[kControllerPitchStep] = 0;
    m_controllers.masterTune = 0;
    
    this->SetCurrentProgram(0);

    // prepare to play
    m_currentNote = 0;
    m_sustain = false;
    m_monoMode = false; // ie polyphonic
    m_normalizeDxVelocity = false;
    m_extra_buf_size = 0;
    for(int note = 0; note < k_MaxActiveVoices; ++note) 
    {
        m_voices[note].dx7_note = new Dx7Note(m_synthTuningState);
        m_voices[note].keydown = false;
        m_voices[note].sustained = false;
        m_voices[note].live = false;
    }
    m_lfo.reset(m_currentProgram + 137);
}

DbFM::~DbFM()
{
    for(int note = 0; note < k_MaxActiveVoices; ++note) 
    {
        delete m_voices[note].dx7_note;
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
    int channel = status & 0x0F;
    int req = status & 0xF0;

    if( m_controllers.mpeEnabled && channel != 1 &&
        (
            (req == 0xb0 && data1 == 74 ) || //timbre
            (req == 0xd0 ) || // aftertouch
            (req == 0xe0 ) // pb
        ))
    {
        // OK so find my voice index
        int voiceIndex = -1;
        for(int i=0; i<k_MaxActiveVoices; ++i)
        {
            if(m_voices[i].keydown && m_voices[i].channel == channel )
            {
                voiceIndex = i;
                break;
            }
        }
        if(voiceIndex >= 0)
        {
            int i = voiceIndex;
            switch(req) 
            {
            case 0xb0:
                m_voices[i].mpeTimbre = data2;
                m_voices[i].dx7_note->mpeTimbre = data2;
                break;
            case 0xd0:
                m_voices[i].mpePressure = data1;
                m_voices[i].dx7_note->mpePressure = data1;
                break;
            case 0xe0:
                m_voices[i].mpePitchBend = (int)( data1 | (data2 << 7) );
                m_voices[i].dx7_note->mpePitchBend = m_voices[i].mpePitchBend;
                break;
            }
        }
    }
    else
    {
        switch(req)
        {
        case 0x80:
            keyup(channel, data1, data2);
            return;

        case 0x90 :
            keydown(channel, data1, data2);
            return;
            
        case 0xb0 : 
            {
            int ctrl = data1;
            int value = data2;
            switch(ctrl) 
            {
                case 1:
                    m_controllers.modwheel_cc = value;
                    m_controllers.refresh();
                    break;
                case 2:
                    m_controllers.breath_cc = value;
                    m_controllers.refresh();
                    break;
                case 4:
                    m_controllers.foot_cc = value;
                    m_controllers.refresh();
                    break;
                case 64:
                    m_sustain = value > 63;
                    if (!m_sustain) 
                    {
                        for(int note = 0; note < k_MaxActiveVoices; note++) 
                        {
                            if (m_voices[note].sustained && !m_voices[note].keydown) 
                            {
                                m_voices[note].dx7_note->keyup();
                                m_voices[note].sustained = false;
                            }
                        }
                    }
                    break;
                case 123:
                    for(int note = 0; note < k_MaxActiveVoices; note++) 
                    {
                        if(m_voices[note].keydown)
                            keyup(channel, m_voices[note].midi_note, 0);
                    }
                    break;
                case 120:
                    break;
                default:
                    TRACE("handle CC %d %d", ctrl, value);
                    #if 0
                    if ( mappedMidiCC.contains(ctrl) ) {
                        Ctrl *linkedCtrl = mappedMidiCC[ctrl];
                        
                        // We are not publishing this in the DSP thread, moving that in the
                        // event thread
                        linkedCtrl->publishValueAsync((float) value / 127);
                    }
                    // this is used to notify the dialog that a CC value was received.
                    lastCCUsed.setValue(ctrl);
                    #endif
                    break;
                }
            }
            return;
            
        case 0xc0 : /* program change */
            SetCurrentProgram(data1);
            return;
            
        case 0xd0 :
            m_controllers.aftertouch_cc = data1;
            m_controllers.refresh();
            return;
        
		case 0xe0 :
			m_controllers.values_[kControllerPitch] = data1 | (data2 << 7);
            return;
        }
    }
} // add midi event

void
DbFM::GetSamples(int numSamples, float *outbuf)
{
    int i;
    // flush first events
    for(i=0; i < numSamples && i < m_extra_buf_size; i++) 
        outbuf[i] = m_extra_buf[i];
    
    // remaining buffer is still to be processed
    if(m_extra_buf_size > numSamples) 
    {
        for(int j = 0; j < m_extra_buf_size - numSamples; j++) 
            m_extra_buf[j] = m_extra_buf[j + numSamples];
        m_extra_buf_size -= numSamples;
        
        #if  0
        // flush the events, they will be process in the next cycle
        while(getNextEvent(&it, numSamples)) {
            processMidiMessage(midiMsg);
        }
        #endif
    } 
    else 
    {
        // finally we synthesize
        for(; i < numSamples; i += N) 
        {
            AlignedBuf<int32_t, N> audiobuf;
            float sumbuf[N];
            for(int j = 0; j < N; ++j) 
            {
                audiobuf.get()[j] = 0;
                sumbuf[j] = 0;
            }
            int32_t lfovalue = m_lfo.getsample();
            int32_t lfodelay = m_lfo.getdelay();
            
            for(int note = 0; note < k_MaxActiveVoices; ++note) 
            {
                if (m_voices[note].live) 
                {
                    m_voices[note].dx7_note->compute(audiobuf.get(), 
                        lfovalue, lfodelay, &m_controllers);
                    for (int j=0; j < N; ++j) 
                    {
                        // XXX: we convert to signed 15bits before float (yuk)
                        int32_t val = audiobuf.get()[j];
                        val = val >> 4;
                        int clip_val = val < -(1 << 24) ? 0x8000 : val >= (1 << 24) ? 0x7fff : val >> 9;
                        float f = ((float) clip_val) / (float) 0x8000;
                        if(f > 1) f = 1;
                        if(f < -1) f = -1;
                        sumbuf[j] += f;
                        audiobuf.get()[j] = 0;
                    }
                }
            }
            
            int jmax = numSamples - i;
            for(int j = 0; j < N; ++j) 
            {
                if(j < jmax) 
                    outbuf[i + j] = sumbuf[j];
                else 
                    m_extra_buf[j - jmax] = sumbuf[j];
            }
        }
        m_extra_buf_size = i - numSamples;
    }
    
    #if 0
    while(getNextEvent(&it, numSamples)) {
        processMidiMessage(midiMsg);
    }
    #endif

    m_fx.process(outbuf, numSamples);
}

void
DbFM::LoadCartridge(char const *path)
{
    assert(!"Implement me!");
}

void 
DbFM::unpackOpSwitch(char packOpValue) 
{
    m_controllers.opSwitch[5] = ((packOpValue >> 5) &1) + 48;
    m_controllers.opSwitch[4] = ((packOpValue >> 4) &1) + 48;
    m_controllers.opSwitch[3] = ((packOpValue >> 3) &1) + 48;
    m_controllers.opSwitch[2] = ((packOpValue >> 2) &1) + 48;
    m_controllers.opSwitch[1] = ((packOpValue >> 1) &1) + 48;
    m_controllers.opSwitch[0] = (packOpValue &1) + 48;
}

void 
DbFM::SetCurrentProgram(int index) 
{
    TRACE("setting program %d state", index);
    m_currentProgramIndex = index > 31 ? 31 : index;
    m_cartridge.unpackProgram(m_currentProgram, index);
    unpackOpSwitch(0x3F);
    m_lfo.reset(m_currentProgram + 137);
    
}

void 
DbFM::keydown(uint8_t channel, uint8_t pitch, uint8_t velo) 
{
    if(velo == 0) 
    {
        keyup(channel, pitch, velo);
        return;
    }

    pitch += tuningTranspositionShift();
    
    if(m_normalizeDxVelocity) 
    {
        velo = ((float)velo) * 0.7874015; // 100/127
    }

    if(m_controllers.mpeEnabled)
    {
        int note = m_currentNote; // round-robin
        for(int i=0; i<k_MaxActiveVoices; ++i)
        {
            if(m_voices[note].keydown && m_voices[note].channel == channel)
            {
                // If we get two keydowns on the same channel we are getting information from a non-mpe device
                m_controllers.mpeEnabled = false;
            }
            note = (note + 1) % k_MaxActiveVoices;
        }
    }
    
    int note = m_currentNote;
    for (int i=0; i<k_MaxActiveVoices; i++) 
    {
        if (!m_voices[note].keydown) 
        {
            m_currentNote = (note + 1) % k_MaxActiveVoices;
            m_lfo.keydown();  // TODO: should only do this if # keys down was 0
            m_voices[note].channel = channel;
            m_voices[note].midi_note = pitch;
            m_voices[note].velocity = velo;
            m_voices[note].sustained = m_sustain;
            m_voices[note].keydown = true;
            m_voices[note].dx7_note->init(m_currentProgram, pitch, velo);
            if(m_currentProgram[136] )
                m_voices[note].dx7_note->oscSync();
            break;
        }
        note = (note + 1) % k_MaxActiveVoices;
    }
    
    if(m_monoMode) // not polyphonic
    {
        for(int i=0; i<k_MaxActiveVoices; i++) 
        {
            if(m_voices[i].live) 
            {
                // all keys are up, only transfer signal
                if(!m_voices[i].keydown ) 
                {
                    m_voices[i].live = false;
                    m_voices[note].dx7_note->transferSignal(*m_voices[i].dx7_note);
                    break;
                }
                if(m_voices[i].midi_note < pitch ) 
                {
                    m_voices[i].live = false;
                    m_voices[note].dx7_note->transferState(*m_voices[i].dx7_note);
                    break;
                }
                return;
            }
        }
    }
 
    m_voices[note].live = true;
	//TRACE("activate %d [ %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ]", pitch, ACT(voices[0]), ACT(voices[1]), ACT(voices[2]), ACT(voices[3]), ACT(voices[4]), ACT(voices[5]), ACT(voices[6]), ACT(voices[7]), ACT(voices[8]), ACT(voices[9]), ACT(voices[10]), ACT(voices[11]), ACT(voices[12]), ACT(voices[13]), ACT(voices[14]), ACT(voices[15]));
}

void 
DbFM::keyup(uint8_t chan, uint8_t pitch, uint8_t velo) 
{
    pitch += tuningTranspositionShift();

    int note;
    for(note=0; note<k_MaxActiveVoices; ++note) 
    {
        if ( ( ( m_controllers.mpeEnabled && m_voices[note].channel == chan ) || // MPE node - find voice by channel
               (!m_controllers.mpeEnabled && m_voices[note].midi_note == pitch ) ) && // regular mode find voice by pitch
             m_voices[note].keydown ) // but still only grab the one which is keydown
        {
            m_voices[note].keydown = false;
			//TRACE("deactivate %d [ %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ]", pitch, ACT(voices[0]), ACT(voices[1]), ACT(voices[2]), ACT(voices[3]), ACT(voices[4]), ACT(voices[5]), ACT(voices[6]), ACT(voices[7]), ACT(voices[8]), ACT(voices[9]), ACT(voices[10]), ACT(voices[11]), ACT(voices[12]), ACT(voices[13]), ACT(voices[14]), ACT(voices[15]));
            break;
        }
    }
    
    // note not found ?
    if(note >= k_MaxActiveVoices) 
    {
		TRACE("note found ??? %d [ %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ]", pitch, ACT(voices[0]), ACT(voices[1]), ACT(voices[2]), ACT(voices[3]), ACT(voices[4]), ACT(voices[5]), ACT(voices[6]), ACT(voices[7]), ACT(voices[8]), ACT(voices[9]), ACT(voices[10]), ACT(voices[11]), ACT(voices[12]), ACT(voices[13]), ACT(voices[14]), ACT(voices[15]));
        return;
    }
    
    if(m_monoMode) 
    {
        int highNote = -1;
        int target = 0;
        for(int i=0; i<k_MaxActiveVoices;i++) 
        {
            if(m_voices[i].keydown && m_voices[i].midi_note > highNote) 
            {
                target = i;
                highNote = m_voices[i].midi_note;
            }
        }
        
        if(highNote != -1 && m_voices[note].live)
        {
            m_voices[note].live = false;
            m_voices[target].live = true;
            m_voices[target].dx7_note->transferState(*m_voices[note].dx7_note);
        }
    }
    
    if(m_sustain) 
    {
        m_voices[note].sustained = true;
    }
    else 
    {
        m_voices[note].dx7_note->keyup();
    }
}

int 
DbFM::tuningTranspositionShift()
{
    if(m_synthTuningState->is_standard_tuning() || 
       !m_controllers.transpose12AsScale)
    {
        return m_currentProgram[144] - 24;
    }
    else
    {
        int d144 = m_currentProgram[144];
        if(d144 % 12 == 0)
        {
            int oct = (d144 - 24) / 12;
            int res = oct * m_synthTuningState->scale_length();
            return res;
        }
        else
            return m_currentProgram[144] - 24;
    }
}