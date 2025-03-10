#!/bin/sh

echo xcrun notarytool submit "$1" \
    --key "$NOTARIZATION_KEY" \
    --key-id "$NOTARIZATION_KEY_ID" \
    --issuer "$NOTARIZATION_ISSUER" \
    --wait

xcrun notarytool submit "$1" \
    --key "$NOTARIZATION_KEY" \
    --key-id "$NOTARIZATION_KEY_ID" \
    --issuer "$NOTARIZATION_ISSUER" \
    --wait
