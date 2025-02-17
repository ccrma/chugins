#--------------------------------------------------------------
# codesign.sh
# pre-condition: both arm64 and intel PlinkyRev.chug builds
#  are located in the proper directories 
#--------------------------------------------------------------

# where the PlinkyRev build can be found
CHUGIN_UB=./Line.chug

# codesign Line.chug
codesign --deep --force --verify --verbose --timestamp --options runtime --entitlements Chugin.entitlements --sign "Developer ID Application" ${CHUGIN_UB}
