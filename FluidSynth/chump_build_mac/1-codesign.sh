#--------------------------------------------------------------
# codesign.sh
# pre-condition: both arm64 and intel FluidSynth.chug builds
#  are located in the proper directories 
#--------------------------------------------------------------

# where the FluidSynth build can be found
FLUIDSYNTH_ARM=./FluidSynth-arm64.chug
FLUIDSYNTH_INTEL=./FluidSynth-x86_64.chug
FLUIDSYNTH_UB=./FluidSynth.chug

# remove code signature from chugin and dylibs
codesign --remove-signature ${FLUIDSYNTH_ARM}
codesign --remove-signature ${FLUIDSYNTH_INTEL}

# codesign FluidSynth.chug
codesign --deep --force --verify --verbose --timestamp --options runtime --entitlements FluidSynth.entitlements --sign "Developer ID Application" ${FLUIDSYNTH_ARM}
codesign --deep --force --verify --verbose --timestamp --options runtime --entitlements FluidSynth.entitlements --sign "Developer ID Application" ${FLUIDSYNTH_INTEL}


lipo -create -output ${FLUIDSYNTH_UB} ${FLUIDSYNTH_ARM} ${FLUIDSYNTH_INTEL}
