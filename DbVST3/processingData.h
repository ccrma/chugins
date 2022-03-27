#ifndef processingData_h
#define processingData_h

#include "vst3.h"
#include "processingUtil.h"

#include <vector>
#include <algorithm>
#include <cassert>

/* DbVST3ProcessData manages state around the all-important process
 * method invoked in DbVST3Processing.h. This includes audio buffer
 * management.
 * 
 *    processMode (0)
 *    symbolicSampleSize (kSample32)
 *    numSamples (0)
 *    numInputs (0)
 *    numOutputs (0)
 *    inputs (nullptr)
 *    outputs (nullptr)
 *    inputParameterChanges (nullptr)
 *    outputParameterChanges (nullptr)
 *    inputEvents (nullptr)
 *    outputEvents (nullptr)
 *    processContext (nullptr)
 * 
 */
class ProcessingData : public Steinberg::Vst::ProcessData
{
private:
    // we subclass ProcessData and thus can manipulate
    // inputEvents, etc..
    Steinberg::Vst::ParameterChanges inPChanges; // outPChanges are optional.
    Steinberg::Vst::ProcessContext processCtx; // assigned to this->processContext
    EventList inEvents;
    EventList outEvents;
    int eventSampleOffset;
    int verbosity;
    int debug;
    struct chuckInstBuffer
    {
        chuckInstBuffer()
        {
            this->size = 0;
            this->buffers[0] = nullptr;
            this->buffers[1] = nullptr;
        }
        ~chuckInstBuffer()
        {
            if(this->size)
            {
                delete [] this->buffers[0];
                delete [] this->buffers[1];
            }
        }
        void Init(int nchan, int sz=256) /* at 44.1KHz, 256 is .005 sec */
        {
            this->size = sz;
            this->index = sz; // signals empty
            this->buffers[0] = new float[sz];
            this->buffers[1] = new float[sz];
        }
        bool Active() { return this->size != 0; }
        bool Empty() { return this->index == this->size;}
        int index;
        int size;
        float *buffers[2];
    } instBuffer;

public:
    BusUsage busUsage; 

    ProcessingData();
    virtual ~ProcessingData();

    void SetVerbosity(int v);
    void Initialize(Steinberg::Vst::ProcessSetup &pd);
    void Process(
            Steinberg::Vst::IAudioProcessor* audioEffect,
            float *in, int inCh, float *out, int outCh, int nframes);
    void PrepareParamChange(Steinberg::Vst::ParamID paramId, 
                        Steinberg::Vst::ParamValue value);
    void PrepareMidiEvent(int status, int data1, int data2,
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMap);

};

#endif