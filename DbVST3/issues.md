## Coding Issues

## hot

* TALVocoder regression.

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

```

'AudioPluginHost.exe' (Win32): Loaded 'C:\Windows\System32\netapi32.dll'.
'AudioPluginHost.exe' (Win32): Loaded 'C:\Windows\System32\netutils.dll'.
'AudioPluginHost.exe' (Win32): Loaded 'C:\Windows\System32\wkscli.dll'.
'AudioPluginHost.exe' (Win32): Loaded 'C:\Windows\System32\cscapi.dll'.
'AudioPluginHost.exe' (Win32): Loaded 'C:\Windows\System32\IPHLPAPI.DLL'.
'AudioPluginHost.exe' (Win32): Loaded 'C:\Windows\System32\nsi.dll'.
'AudioPluginHost.exe' (Win32): Loaded 'C:\Windows\System32\dhcpcsvc6.dll'.
'AudioPluginHost.exe' (Win32): Loaded 'C:\Windows\System32\dhcpcsvc.dll'.
'AudioPluginHost.exe' (Win32): Loaded 'C:\Windows\System32\dnsapi.dll'.
VST3HostContext queryInterface IHostApplication (1)
VST3PluginInstance constructor
VST3PluginInstance init
EditController != Component
VST3HostContext queryInterface IComponentHandler2 (1)
synchronizeStates
VST3HostContext.restartComponentOnMessageThread begin ----------------
restartComponent paramvalueschanged
VST3HostContext.restartComponentOnMessageThread end--------------------
VST3PluginInstance.setupIO (setupProcessing)
activateEventBuses (in)
activateEventBuses (out)
VST3PluginInstance prepareToPlay (setupProcessing)
activateAudioOutputBus
activateEventBuses (in)
activateEventBuses (out)
prepareToPlay complete (isActive:true)
```

chuck testLABS.ck
```
# across plugin-load
'chuck.exe' (Win32): Loaded 'C:\Program Files\Common Files\VST3\LABS (64 Bit).vst3'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\comdlg32.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\WinSxS\amd64_microsoft.windows.common-controls_6595b64144ccf1df_6.0.22000.120_none_9d947278b86cc467\comctl32.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\wininet.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\version.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\dbghelp.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\netapi32.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\opengl32.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\glu32.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\netutils.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\DXCore.dll'.
# factory create-instance
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\msctf.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\wkscli.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\cscapi.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\IPHLPAPI.DLL'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\nsi.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\dhcpcsvc6.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\dhcpcsvc.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\dnsapi.dll'.
'chuck.exe' (Win32): Loaded 'C:\Windows\System32\profapi.dll'.

VST3PluginInstance query  IHostApplication (1)
VST3PluginInstance query  IHostApplication (1)
VST3PluginInstance query  IComponentHandler2 (1)
synchronizeStates
VST3PluginInstance.restartComponent ParamValuesChanged, updateProcessor:0
SetupProcessing
PluginInstance.InitBuses
activate output bus 0 sa:3
setBusArrangements in:(0), out(1)
Input event busses
 - 0 MIDI Input
LABS setEventBusState 1 nin:1 nout:0
make a sound please 
LABS (64 Bit).vst3 done (in .ck)
```

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


### See Also

https://github.com/Spacechild1/vstplugin/blob/master/sc/src/VSTPlugin.cpp


### TALVocoder-in-Fiddle


VST3PluginInstance query IHostApplication (1)
VST3PluginInstance query IHostApplication (1)
VSTPluginInstance.createInstance HostMessage
VST3PluginInstance query IComponentHandler2 (1)
synchronizeStates
VST3PluginInstance.restartComponent ParamValuesChanged, updateProcessor:1
TAL-Vocoder-2 synchronized 7380 bytes of state
VST3PluginInstance.Init TAL-Vocoder-2
SetupProcessing
PluginInstance.InitBuses
activate inbus 0 true
activate inbus 1 false
activate output bus 0 sa:3
setBusArrangements in:(2), out(1)
TAL-Vocoder-2 setEventBusState 1 nin:1 nout:0
Activating TAL-Vocoder-2
(activating succeeded)
VST3PluginInstance.InitProcessing TAL-Vocoder-2

vs test/vocoder.ck

VST3PluginInstance query  IHostApplication (1)
VST3PluginInstance query  IHostApplication (1)
VSTPluginInstance.createInstance HostMessage
VST3PluginInstance query  IComponentHandler2 (1)
synchronizeStates
VST3PluginInstance.restartComponent ParamValuesChanged, updateProcessor:1
TAL-Vocoder-2 synchronized 7437 bytes of state
VST3PluginInstance.Init TAL-Vocoder-2
SetupProcessing
PluginInstance.InitBuses
activate inbus 0 true
activate inbus 1 false
activate output bus 0 sa:3
setBusArrangements in:(2), out(1)
TAL-Vocoder-2 setEventBusState 1 nin:1 nout:0
Activating TAL-Vocoder-2
(activating succeeded)
VST3PluginInstance.InitProcessing TAL-Vocoder-2
Vocoder loaded, 36 parameters.
performEdit called 1862642762 0
performEdit called 1336600346 0.416
performEdit called 1727365094 0
performEdit called 248894132 0
performEdit called 790965235 0.512
performEdit called 1572884547 1
performEdit called 1039623321 0.664
performEdit called 1877784346 1
performEdit called 1916191535 0
performEdit called 968503834 0
performEdit called 1929258769 0.54
performEdit called 1874206561 0
performEdit called 1100839755 0.436
performEdit called 906021787 0.592
performEdit called 1524350849 0.616
performEdit called 561974127 1
performEdit called 270278353 0
performEdit called 3571704 0.5
performEdit called 106433143 0
performEdit called 1707711597 0
performEdit called 786268582 0
performEdit called 1366601271 0.328
performEdit called 1754441473 0.1
performEdit called 1754441474 0.232
performEdit called 1754441475 0.268
performEdit called 1754441476 0.424
performEdit called 1754441477 0.5
performEdit called 1754441478 0.5
performEdit called 1754441479 0.46
performEdit called 1754441480 0.5
performEdit called 1754441481 0.5
performEdit called 1754441482 0.5
performEdit called 1754441504 0.424
performEdit called 302364618 0