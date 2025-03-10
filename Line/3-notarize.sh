#--------------------------------------------------------------
# notarize.sh
# pre-condition: codesigned, packaged, also developer
#     credentials are loaded into the environment
# post-condition: notarizes the chump package
#--------------------------------------------------------------

# dir location of this bash script
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo ${SCRIPT_DIR}

# where the PlinkyRev build can be found
CHUGIN=./Line_mac.zip


echo "notarizing PlinkyRev.chug..."
${SCRIPT_DIR}/notarize.sh ${CHUGIN}
