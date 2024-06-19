project('warpbuf', 'cpp',
  version : 'v0.0.1',
  default_options : ['cpp_std=c++17'],
  license : 'MIT')

add_global_arguments(language : 'cpp')

warpbuf_sources = [
  'src/WarpBufChugin.cpp',
  'src/WarpBufChuginDLL.cpp'
]

warpbuf_headers = [
  'src/AbletonClipInfo.h',
  'src/WarpBufChugin.h',
]

chuck_inc = include_directories('../../chuck/include')
portable_endian_inc = include_directories('../thirdparty/portable_endian/include')
inc = [chuck_inc, portable_endian_inc]

# dependencies
# subdir('thirdparty/rubberband')
rubberband = dependency('rubberband')
libsndfile = dependency('sndfile', static: true)

warpbuf_lib = library(
  'warpbuf_lib',
  sources: warpbuf_sources,
  include_directories: inc,
  dependencies: [rubberband, libsndfile],
  install: true,
)

# rename the object to a proper chugin file
fs = import('fs')
fs.copyfile(warpbuf_lib.path(), 'WarpBuf.chug')
