## Coding Issues


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

## What's up with Dexed

* VSTFidder doesn't appear to get correct parameter values and
  sends a bunch of zeros to the host.  Some parameters aren't
  affected by presets (Cutoff, Output) and if those values are
  set to 0 you get no output.

```
DexedAudioProcessor::DexedAudioProcessor Hi
Cartridge::load valid sysex found!
DexedAudioProcessor::setEngineType settings engine 1
SysexComm::setInput device  not found
SysexComm::setOutput device  not found
DexedAudioProcessor::setEngineType settings engine 1
DexedAudioProcessor::setCurrentProgram setting program 0 state
VST3HostContext queryInterface IHostApplication (1)
VST3PluginInstance constructor
VST3PluginInstance init
EditController != Component
VST3HostContext queryInterface IComponentHandler2 (1)
synchronizeStates
Dexed CtrlDX setValueHost 134 1
Dexed CtrlDX setValue 134 31
DexedAudioProcessor::setDxValue ignoring dx7 same values 134 31
Dexed CtrlDX setValueHost 135 1
Dexed CtrlDX setValue 135 7
DexedAudioProcessor::setDxValue ignoring dx7 same values 135 7
Dexed CtrlDX setValueHost 136 1
Dexed CtrlDX setValue 136 1
DexedAudioProcessor::setDxValue ignoring dx7 same values 136 1
Dexed CtrlDX setValueHost 137 0.353535
Dexed CtrlDX setValue 137 35
DexedAudioProcessor::setDxValue ignoring dx7 same values 137 35
Dexed CtrlDX setValueHost 62 0.5
Dexed CtrlDX setValue 62 7
DexedAudioProcessor::setDxValue ignoring dx7 same values 62 7
Dexed CtrlDX setValueHost 21 0.10101
Dexed CtrlDX setValue 21 10
DexedAudioProcessor::setDxValue ignoring dx7 same values 21 10
Dexed CtrlDX setValueHost 22 0.646465
Dexed CtrlDX setValue 22 64
DexedAudioProcessor::setDxValue ignoring dx7 same values 22 64
Dexed CtrlDX setValueHost 23 0.494949
Dexed CtrlDX setValue 23 49
DexedAudioProcessor::setDxValue ignoring dx7 same values 23 49
Dexed CtrlDX setValueHost 24 1
Dexed CtrlDX setValue 24 99
DexedAudioProcessor::setDxValue ignoring dx7 same values 24 99
Dexed CtrlDX setValueHost 25 0.464646
Dexed CtrlDX setValue 25 46
DexedAudioProcessor::setDxValue ignoring dx7 same values 25 46
Dexed CtrlDX setValueHost 26 1
Dexed CtrlDX setValue 26 99
DexedAudioProcessor::setDxValue ignoring dx7 same values 26 99
Dexed CtrlDX setValueHost 37 1
Dexed CtrlDX setValue 37 99
DexedAudioProcessor::setDxValue ignoring dx7 same values 37 99
Dexed CtrlDX setValueHost 39 0.0645161
Dexed CtrlDX setValue 39 2
DexedAudioProcessor::setDxValue ignoring dx7 same values 39 2
Dexed CtrlDX setValueHost 41 0.5
Dexed CtrlDX setValue 41 7
DexedAudioProcessor::setDxValue ignoring dx7 same values 41 7
Dexed CtrlDX setValueHost 0 0.0707071
Dexed CtrlDX setValue 0 7
DexedAudioProcessor::setDxValue ignoring dx7 same values 0 7
Dexed CtrlDX setValueHost 1 0.646465
Dexed CtrlDX setValue 1 64
DexedAudioProcessor::setDxValue ignoring dx7 same values 1 64
Dexed CtrlDX setValueHost 2 0.454545
Dexed CtrlDX setValue 2 45
DexedAudioProcessor::setDxValue ignoring dx7 same values 2 45
Dexed CtrlDX setValueHost 3 1
Dexed CtrlDX setValue 3 99
DexedAudioProcessor::setDxValue ignoring dx7 same values 3 99
Dexed CtrlDX setValueHost 4 0.454545
Dexed CtrlDX setValue 4 45
DexedAudioProcessor::setDxValue ignoring dx7 same values 4 45
Dexed CtrlDX setValueHost 5 1
Dexed CtrlDX setValue 5 99
DexedAudioProcessor::setDxValue ignoring dx7 same values 5 99
Dexed CtrlDX setValueHost 16 1
Dexed CtrlDX setValue 16 99
DexedAudioProcessor::setDxValue ignoring dx7 same values 16 99
Dexed CtrlDX setValueHost 20 0.5
Dexed CtrlDX setValue 20 7
DexedAudioProcessor::setDxValue ignoring dx7 same values 20 7
restartComponent
VST3HostContext.restartComponentOnMessageThread begin ----------------
restartComponent paramvalueschanged
Host:resetParameters
DexedAudioProcessor::setCurrentProgram setting program 31 state
updateHostDisplay begin
performEdit 1886548852
restartComponent
VST3HostContext.restartComponentOnMessageThread begin ----------------
restartComponent paramvalueschanged
Host:resetParameters
Cutoff 0: 0.430556
Resonance 1: 0.326389
Output 2: 1
MASTER TUNE ADJ 3: 0.854167
ALGORITHM 4: 1
FEEDBACK 5: 1
OSC KEY SYNC 6: 1
LFO SPEED 7: 0.353535
LFO KEY SYNC 11: 1
TRANSPOSE 13: 0.5
P MODE SENS. 14: 0.428571
PITCH EG RATE 1 15: 1
PITCH EG RATE 2 16: 1
PITCH EG RATE 3 17: 1
PITCH EG RATE 4 18: 1
PITCH EG LEVEL 1 19: 0.50505
PITCH EG LEVEL 2 20: 0.50505
PITCH EG LEVEL 3 21: 0.50505
PITCH EG LEVEL 4 22: 0.50505
OP1 EG RATE 1 23: 0.707071
OP1 EG RATE 2 24: 0.40404
OP1 EG RATE 3 25: 0.494949
OP1 EG RATE 4 26: 1
OP1 EG LEVEL 1 27: 1
OP1 EG LEVEL 2 28: 0.929293
OP1 OUTPUT LEVEL 31: 1
OP1 F COARSE 33: 0.225806
OP1 OSC DETUNE 35: 0.785714
OP1 SWITCH 44: 1
OP2 EG RATE 1 45: 0.252525
OP2 EG RATE 2 46: 0.646465
OP2 EG RATE 3 47: 0.494949
OP2 EG RATE 4 48: 1
OP2 EG LEVEL 1 49: 0.50505
OP2 EG LEVEL 2 50: 1
OP2 OUTPUT LEVEL 53: 1
OP2 OSC DETUNE 57: 0.5
OP2 SWITCH 66: 1
OP3 EG RATE 1 67: 0.151515
OP3 EG RATE 2 68: 0.646465
OP3 EG RATE 3 69: 0.494949
OP3 EG RATE 4 70: 1
OP3 EG LEVEL 1 71: 0.444444
OP3 EG LEVEL 2 72: 1
OP3 OUTPUT LEVEL 75: 1
OP3 F COARSE 77: 0.0645161
OP3 OSC DETUNE 79: 0.5
OP3 SWITCH 88: 1
OP4 EG RATE 1 89: 0.131313
OP4 EG RATE 2 90: 0.646465
OP4 EG RATE 3 91: 0.494949
OP4 EG RATE 4 92: 1
OP4 EG LEVEL 1 93: 0.464646
OP4 EG LEVEL 2 94: 1
OP4 OUTPUT LEVEL 97: 1
OP4 OSC DETUNE 101: 0.5
OP4 R SCALE DEPTH 104: 0.111111
OP4 SWITCH 110: 1
OP5 EG RATE 1 111: 0.10101
OP5 EG RATE 2 112: 0.646465
OP5 EG RATE 3 113: 0.494949
OP5 EG RATE 4 114: 1
OP5 EG LEVEL 1 115: 0.464646
OP5 EG LEVEL 2 116: 1
OP5 OUTPUT LEVEL 119: 1
OP5 F COARSE 121: 0.0645161
OP5 OSC DETUNE 123: 0.5
OP5 SWITCH 132: 1
OP6 EG RATE 1 133: 0.0707071
OP6 EG RATE 2 134: 0.646465
OP6 EG RATE 3 135: 0.454545
OP6 EG RATE 4 136: 1
OP6 EG LEVEL 1 137: 0.454545
OP6 EG LEVEL 2 138: 1
OP6 OUTPUT LEVEL 141: 1
OP6 OSC DETUNE 145: 0.5
OP6 SWITCH 154: 1
GraphEditorPanel parameterValueChanged 155 0 <------------- race condition?

Program 156: 0.96875 <--------------------------- race condition?
1 input parameter changes
MIDI CC 10|111 1568: 1
MIDI CC 10|113 1570: 0.5
MIDI CC 10|114 1571: 0.428571
MIDI CC 10|115 1572: 1
MIDI CC 10|116 1573: 1
MIDI CC 10|117 1574: 1
MIDI CC 10|118 1575: 1
MIDI CC 10|119 1576: 0.50505
MIDI CC 11|11 1598: 0.50505
MIDI CC 11|12 1599: 0.50505
MIDI CC 11|13 1600: 0.50505
MIDI CC 11|14 1601: 0.707071
MIDI CC 11|15 1602: 0.40404
MIDI CC 11|16 1603: 0.494949
MIDI CC 11|17 1604: 1
MIDI CC 11|18 1605: 1
MIDI CC 11|19 1606: 0.929293
MIDI CC 11|43 1630: 1
MIDI CC 11|45 1632: 0.225806
MIDI CC 11|47 1634: 0.785714
MIDI CC 11|77 1664: 1
MIDI CC 11|78 1665: 0.252525
MIDI CC 11|79 1666: 0.646465
MIDI CC 11|80 1667: 0.494949
MIDI CC 11|81 1668: 1
MIDI CC 11|82 1669: 0.50505
MIDI CC 11|104 1691: 1
MIDI CC 11|107 1694: 1
MIDI CC 11|111 1698: 0.5
MIDI CC 12|11 1728: 1
MIDI CC 12|12 1729: 0.151515
MIDI CC 12|13 1730: 0.646465
MIDI CC 12|14 1731: 0.494949
MIDI CC 12|36 1753: 1
MIDI CC 12|37 1754: 0.444444
MIDI CC 12|38 1755: 1
MIDI CC 12|41 1758: 1
MIDI CC 12|43 1760: 0.0645161
MIDI CC 12|45 1762: 0.5
MIDI CC 12|75 1792: 1
MIDI CC 12|76 1793: 0.131313
MIDI CC 12|98 1815: 0.646465
MIDI CC 12|99 1816: 0.494949
MIDI CC 12|100 1817: 1
MIDI CC 12|101 1818: 0.464646
MIDI CC 12|102 1819: 1
MIDI CC 12|105 1822: 1
VST3HostContext.restartComponentOnMessageThread end--------------------
updateHostDisplay end
Dexed::updateUI Begin (via handleAsyncUpdate())
updateHostDisplay begin
updateHostDisplay end
Dexed::updateUI End

```

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

