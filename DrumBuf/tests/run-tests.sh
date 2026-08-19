#!/usr/bin/env sh
# Run every DrumBuf test headless (faster than real time, no audio device).
#   tests/run-tests.sh            # uses ../DrumBuf.chug (build it first: make mac|linux)
#   CHUCK=/path/to/chuck tests/run-tests.sh
# A test fails if chuck exits non-zero (crash) or prints a "FAIL" line
# (test-semantics.ck asserts; the other files are listening demos that must
# simply run to completion). The two Patch tests are skipped unless
# PATCH_CHUG=/path/to/Patch.chug names a Patch build with the disconnect() fix
# (see KNOWN-ISSUES.md: the released Patch segfaults there, which is not a
# DrumBuf bug).
set -u
here=$(cd "$(dirname "$0")" && pwd)
chug=${CHUG:-"$here/../DrumBuf.chug"}
chuck=${CHUCK:-chuck}
[ -f "$chug" ] || { echo "no $chug; build the chugin first (make mac|linux)"; exit 2; }
patch_arg=""
if [ -n "${PATCH_CHUG:-}" ]; then
  patch_arg="--chugin:$PATCH_CHUG"; skip_patch=0
else
  skip_patch=1
fi
status=0
for t in "$here"/*.ck; do
  name=$(basename "$t" .ck)
  case "$name" in test-patch-*) [ "$skip_patch" = 1 ] && { echo "SKIP $name (set PATCH_CHUG to a fixed Patch.chug)"; continue; } ;; esac
  extra=""
  case "$name" in test-patch-*) extra="$patch_arg" ;; esac
  out=$("$chuck" --silent --chugin:"$chug" $extra "$t" 2>&1); rc=$?
  if [ $rc -ne 0 ] || echo "$out" | grep -q "^\"FAIL\|FAIL:"; then
    echo "FAIL $name (rc=$rc)"; echo "$out" | tail -20; status=1
  else
    echo "ok   $name"
  fi
done
exit $status
