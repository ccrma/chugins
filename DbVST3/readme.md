Code organization could be better, however, VST3 peculiarities
trigger mysterious double-defined link error. Solution was
to ensure only a single compilation unit.

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
> use is maps onto ours: one of N effects found within
> a plugin and indentified by its index in that file.

main/chugin
    has-a singleton app
    has-a PluginCtx (after OpenPlugin)

App is-a VST::IHostApplication
    implements OpenPlugin (returns PluginCtx)
    implements loadPlugin (returns Plugin)

PluginCtx (was DbVST3Ctx)
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



