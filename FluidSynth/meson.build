project('FluidSynth', ['c', 'cpp'],
  version : '1.0.0',
  default_options : ['cpp_std=c++11', 'default_library=static'],
  license : '?')

inc = [include_directories('../chuck/include')]
fluidsynth_dep = subproject('fluidsynth', required: true).get_variable('fluidsynth_dep')

shared_library(
  'FluidSynth',
  'FluidSynth.cpp',
  name_prefix: '', # 'lib' is prepended otherwise
  name_suffix: 'chug',
  include_directories: inc,
  dependencies: fluidsynth_dep,
)
