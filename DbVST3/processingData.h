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
    int verbosity;
    int debug;

public:
    BusUsage busUsage; 

    ProcessingData();
    virtual ~ProcessingData();

    void SetVerbosity(int v);

    void initialize(Steinberg::Vst::ProcessSetup &pd);

    void prepareParamChange(Steinberg::Vst::ParamID paramId, 
                        Steinberg::Vst::ParamValue value);

    void prepareMidiEvent(int status, int data1, int data2,
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMap);

    // caller (Processing) simply does
    //      this->beginAudioProcessing()
    //      caller->plugin->process()
    //      this->endAudioProcessing()
    // so all the goodies are in this file.
    void beginAudioProcessing(float *in, int inCh, 
        float *out, int outCh, int nframes);
    void endAudioProcessing();
};

#endif