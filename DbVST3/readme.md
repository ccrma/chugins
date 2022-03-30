## Known Issues

* Currently we don't save/stash plugin state. Plugin presets loaded
  by VST3Fiddler cause parameter updates to be stashed in the .chg file.
  Plugins that support preset parameters trigger a complete update of
  parameters when its value is set.
* MIDI events may produce stuck keys.

## Our Terms and Class Hierarchy

`PluginFile` refers to the vst3 file itself.

`Plugin` refers to the loaded version of the file.

`Module` refers to a subset of the Plugin that is 
user-selectable.  A Plugin can have more than one Module.
Typically there are 2X modules in a Plugin, where X 
represents the number of `AudioEffect` modules found
therein. There are usually 1 AudioEffect and its companion
Controller.  The mda suite of effects are *all* housed
within a single Plugin.

> Confusingly the word module is used in two ways in VST3
> land.  One represents any dynamically loaded plugin 
> constituent (including the `Plugin` object). The other
> use maps onto ours: one of N effects found within
> a plugin. We currently indentify a module by its index 
> in that file. 

main
    creates chugin instances, one for each DbVST3 instance.

VST3Host is-a VST::IHostApplication
    implements OpenPlugin (returns VST3Ctx *)
    implements loadPlugin (returns ModulePtr)

chugin
    has-a singleton Host
    has-a PluginCtx (after OpenPlugin)

VST3Ctx
    has an instance of the open Plugin
    has a list of available modules std::vector<VST3AudioModulePtr>
    has a current module (VST3AudioModulePtr)

VST3AudioModule, VST3AudioModulePtr
    has-a list of parameters

ProcessCtx is a IComponentHandler, etc (was DbVST3Module)
    represents the state associated with executing
    the module.
    has-a audioProcessor
    has-a controller

ProcessCtxData is-a  Vst::ProcessData
    represents the data associated with the ProcessCtx

## Why won't LABS and ROLI Player work?

### mystery JUCE Example Audio Host

Application has-many VST3ComponentHolder

VST3ComponentHolder hasa VST3HostContext

VST3HostContext isa IHostApplication, (implements restartComponent)
HostContext hasa single plugin and a single componentHolder
    implements createInstance(cid, iid) // for IMessage and IAttributeList




### Need a message thread

From JUCE:

It's highly advisable to create your plugins using the message thread.
The VST3 spec requires that many of the functions called during
initialisation are only called from the message thread.

* plugin initialization
* get/set state calls 
* restartComponent
* prepareToPlay (setupProcessing)

ChucK:

* executes plugin-load and all its methods in the audio thread.
* the tick method is the only one that properly resides in the 
  audio thread

Solution:

* when we create the host (singleton) object, we also create
  a message thread.  All plugin instances route all but the tick method
  to that thread.  We need to know when a plugin is ready-to-roll.
* in the dumpVST3 condition, no threads need be constructed.

Details (from https://stackoverflow.com/questions/68438145)

Passing tasks to background thread could be accomplished by a 
producer-consumer queue. Simple C++11 implementation, that 
does not depend on 3rd party libraries would have std::condition_variable 
which is waited by the background thread and notified by main thead, 
std::queue of tasks, and std::mutex to guard these.

Getting the result back to main thread can be done by std::promise/std::future. 
The simplest way is to make std::packaged_task as queue objects, so that main 
thread creates packaged_task, puts it to the queue, notifies condition_variable 
and waits on packaged_task's future.

You would not actually need std::queue if you will create tasks by one at 
once, from one thread - just one std::unique_ptr<std::packaged_task>> would 
be enough. The queue adds flexibility to simultaneosly add many backround 
tasks.

### JUCE Host, LABS

VST3PluginInstance constructor
VST3PluginInstance init
EditController != Component
VST3HostContext.restartComponentOnMessageThread
restartComponent paramvalueschanged
VST3PluginInstance.setupIO (setupProcessing)
VST3PluginInstance prepareToPlay (setupProcessing)
VST3PluginInstance destructor
VST3PluginInst cleanup
VST3PluginInstance releaseResources


### DbVST3 host, LABS
14:12 info ChucK Start
14:12 note waiting for chuck startup
14:12 note [chuck] OTF port 58546
14:12 note [chuck] OscListener on port 19164
14:12 note [chuck] Workspace.Init, root: C:/Users/dana/Documents/Fiddle/ search locs: 5
14:12 info Generated chuck code to C:/Users/dana/Documents/Fiddle/tests/vst3/vst3lab.chg.ck
14:12 note [chuck] pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
14:12 note [chuck] restartComponent ParamValuesChanged
                   LABS synchronized 292 bytes of state
14:12 note [chuck] PluginInstance.Init LABS
                   SetupProcessing
14:12 note [chuck] activate output bus 0 3  (means one stereo output)
                   LABS setEventBusState 1 nin:1 nout:0
                   Activating LABS
                   (activating succeeded)
                   PluginInstance.InitProcessing LABS

14:12 note [chuck] Performance Begin 0


### (vs MDA)

14:18 info ChucK Start
14:18 note waiting for chuck startup
14:18 note [chuck] OTF port 58331
14:18 note [chuck] OscListener on port 19564
14:18 note [chuck] Workspace.Init, root: C:/Users/dana/Documents/Fiddle/ search locs: 5
14:18 info Generated chuck code to C:/Users/dana/Documents/Fiddle/tests/vst3/mdaDX.chg.ck
14:18 note [chuck] pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
14:18 note [chuck] mda Ambience synchronized 244 bytes of state
                   PluginInstance.Init mda Ambience
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Bandisto synchronized 268 bytes of state
                   PluginInstance.Init mda Bandisto
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda BeatBox synchronized 276 bytes of state
                   PluginInstance.Init mda BeatBox
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   
14:18 note [chuck] mda Combo synchronized 256 bytes of state
                   PluginInstance.Init mda Combo
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda DeEsser synchronized 240 bytes of state
                   PluginInstance.Init mda DeEsser
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Degrade synchronized 252 bytes of state
                   PluginInstance.Init mda Degrade
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Delay synchronized 256 bytes of state
                   PluginInstance.Init mda Delay
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Detune synchronized 244 bytes of state
                   PluginInstance.Init mda Detune
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Dither synchronized 248 bytes of state
                   PluginInstance.Init mda Dither
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda DubDelay synchronized 256 bytes of state
                   PluginInstance.Init mda DubDelay
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda DX10 synchronized 4012 bytes of state
                   PluginInstance.Init mda DX10
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Dynamics synchronized 268 bytes of state
                   PluginInstance.Init mda Dynamics
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda EPiano synchronized 588 bytes of state
                   PluginInstance.Init mda EPiano
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Image synchronized 252 bytes of state
                   PluginInstance.Init mda Image
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda JX10 synchronized 8076 bytes of state
                   PluginInstance.Init mda JX10
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Leslie synchronized 264 bytes of state
                   PluginInstance.Init mda Leslie
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Limiter synchronized 248 bytes of state
                   PluginInstance.Init mda Limiter
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Loudness synchronized 240 bytes of state
                   PluginInstance.Init mda Loudness
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda MultiBand synchronized 280 bytes of state
                   PluginInstance.Init mda MultiBand
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Overdrive synchronized 240 bytes of state
                   PluginInstance.Init mda Overdrive
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Piano synchronized 1004 bytes of state
                   PluginInstance.Init mda Piano
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda RePsycho! synchronized 256 bytes of state
                   PluginInstance.Init mda RePsycho!
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda RezFilter synchronized 268 bytes of state
                   PluginInstance.Init mda RezFilter
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda RingMod synchronized 244 bytes of state
                   PluginInstance.Init mda RingMod
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Round Panner synchronized 236 bytes of state
                   PluginInstance.Init mda Round Panner
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Shepard synchronized 240 bytes of state
                   PluginInstance.Init mda Shepard
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Splitter synchronized 256 bytes of state
                   PluginInstance.Init mda Splitter
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Stereo Simulator synchronized 248 bytes of state
                   PluginInstance.Init mda Stereo Simulator
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Sub-Bass Synthesizer synchronized 252 bytes of state
                   PluginInstance.Init mda Sub-Bass Synthesizer
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda TalkBox synchronized 244 bytes of state
                   PluginInstance.Init mda TalkBox
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   restartComponent ParamValuesChanged
                   mda TestTone synchronized 260 bytes of state
                   PluginInstance.Init mda TestTone
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Thru-Zero Flanger synchronized 248 bytes of state
                   PluginInstance.Init mda Thru-Zero Flanger
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda Tracker synchronized 260 bytes of state
                   PluginInstance.Init mda Tracker
                   pluginInstance query f040b4b345eca36045c0cdabcca2d5b4
                   mda SpecMeter synchronized 228 bytes of state
                   PluginInstance.Init mda SpecMeter

14:18 note [chuck] SetupProcessing
                   activate output bus 0 3
                   mda Ambience setEventBusState 1 nin:0 nout:0
                   Activating mda Ambience
                   (activating succeeded)
                   PluginInstance.InitProcessing mda Ambience
                   SetupProcessing
                   activate output bus 0 3
                   mda DX10 setEventBusState 1 nin:1 nout:0
                   Activating mda DX10
                   (activating succeeded)
                   PluginInstance.InitProcessing mda DX10
                   restartComponent ParamValuesChanged

14:18 note [chuck] Performance Begin 0
14:18 note [chuck] Opened MIDI device: 0 -> A-Series Keyboard


## testCleanup.ck with early out

*** Leaked objects detected: 3 instance(s) of class Param
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class JuceVST3EditController
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class AudioParameterBool
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class JuceAudioProcessor
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class HashMap
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class JuceVST3Component
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class SynthesiserSound
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 8 instance(s) of class SynthesiserVoice
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class JuceDemoPluginAudioProcessor
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class Synthesiser
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class OwnedArray
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 2 instance(s) of class AudioBuffer
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 10 instance(s) of class AudioBuffer
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 4 instance(s) of class SharedObject
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 2 instance(s) of class AudioProcessorParameterNode
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class TimerThread
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 2 instance(s) of class AsyncUpdater
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class Thread
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 3 instance(s) of class WaitableEvent
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class AudioProcessorValueTreeState
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 2 instance(s) of class AudioParameterFloat
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 3 instance(s) of class AudioProcessorParameterWithID
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 3 instance(s) of class AudioProcessorParameter
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class MidiKeyboardState
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 2 instance(s) of class MidiBuffer
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class AudioProcessor
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 2 instance(s) of class AudioProcessorParameterGroup
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 2 instance(s) of class OwnedArray
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 2 instance(s) of class OwnedArray
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 8 instance(s) of class BigInteger
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class InternalMessageQueue
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 1 instance(s) of class MessageManager
JUCE Assertion failure in juce_leakedobjectdetector.h:92
chuck.exe has triggered a breakpoint.

JUCE Assertion failure in juce_singleton.h:50
chuck.exe has triggered a breakpoint.

*** Leaked objects detected: 5 instance(s) of class StringArray
JUCE Assertion failure in juce_leakedobjectdetector.h:92