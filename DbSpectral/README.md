# DbSpectral Chugin

## Intro

DbSpectral is a time-varying audio filter parameterized by
an image file.  DbSpectral reads image files (.png) representing
a spectrogram.  Image x is interpetted as time.  Image y is interpretted 
as frequency from low (y=0) to high.  Scan-columns are resampled to match 
the size of the fft. For example, a RealFFT of size 1024 produces 512 
frequency bins for both real and imaginary components.  The normalized
pixel value at [x, y] is multiplied by the both components of the FFT
prior to performing the IFFT.

Files are loaded in a separate thread to prevent hiccups in the
audio/realtime thread.

