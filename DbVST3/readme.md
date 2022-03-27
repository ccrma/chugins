## Known Issues

* Currently we don't save/stash plugin state.
* Presets may not appear to persist/function if they
  are emitted prior to other value-updates.
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

main/chugin
    has-a singleton app
    has-a PluginCtx (after OpenPlugin)

App is-a VST::IHostApplication
    implements OpenPlugin (returns PluginCtx)
    implements loadPlugin (returns Plugin)

PluginCtx (was DbVST3Ctx)
    has an instance of the open Plugin
    has a list of available modules
    has a current module (ActivateModule)

Module, ModulePtr (was DbVST3Plugin)
    has-a list of parameters
    has-a

ProcessCtx is a IComponentHandler, etc (was DbVST3Module)
    represents the state associated with executing
    the module.
    has-a audioProcessor
    has-a controller

ProcessCtxData is-a  Vst::ProcessData
    represents the data associated with the ProcessCtx

## Why won't LABS and ROLI Player work?

- something to do with threads?
    - messagethread
    - processingthread

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
