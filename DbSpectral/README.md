# DbSpectral Chugin

## Intro

DbSpectral is a time-varying audio filter parameterized by
an image file.  DbSpectral reads image files (.png) representing
a spectrogram.  Image y is intrpretted as frequency from
low (y=0) to high.  Image x is interpetted as time.

Files are loaded in a separate thread to prevent hiccups in the
audio/realtime thread.

