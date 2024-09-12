#!/usr/bin/env bash

set -e

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
MISC_DIR="$SCRIPT_DIR/misc"
PATCH_DIR="$SCRIPT_DIR/patch"
STAGE1_DIR="$SCRIPT_DIR/stage1"
PAYLOAD_DIR="$SCRIPT_DIR/payload"
EXPLOIT_DIR="$SCRIPT_DIR/exploit"
DIST_DIR="$SCRIPT_DIR/dist"

function print_help {
    echo \
        "wfc-patcher-wii build script

Usage: make.sh [OPTIONS] -- [ADITIONAL ARGS]

Options:
 --clean             Remove 'dist' and build artifacts in 'patch', 'stage1', and 'payload'
 --clean-keys        Remove keys in 'misc'
 --keys              Generate new keys in 'misc', copying the public key to 'include'
 --patch             Generate patches
 --stage1            Build stage1
 --payload           Build payloads
 --exploit           Build exploit
 --all               Builds payloads, stage1, and patches. Makes keys if none are present.
 -h|--help           Print this message and exit.

Note:
 * Anything after '--' will be passed to the individual build scripts (except for --keys), such as -j8 to specify thread count.
 * The files that make up the 'payload' directory of wfc-server will be placed in the 'dist' directory adjacent to the script.
"
    exit 1
}

function check_keys {
    if ! [ -e "$SCRIPT_DIR/misc/private-key-OUT.pem" ]; then
        return 1
    fi

    return 0
}

function remove_keys {
    echo "Removing old keys!"

    if [ -e "$MISC_DIR/private-key-OUT.pem" ]; then
        echo "Removing private key"
        rm "$MISC_DIR/private-key-OUT.pem"
    fi

    if [ -e "$MISC_DIR/wwfcPayloadPublicKey-OUT.hpp" ]; then
        echo "Removing public key"
        rm "$MISC_DIR/wwfcPayloadPublicKey-OUT.hpp"
    fi
}

if [ $# -eq 0 ]; then
    >&2 echo "No arguments provided"
    print_help
fi

clean=false
clean_keys=false
try_make_keys=false
keys=false
patch=false
stage1=false
payload=false
exploit=false
args=""
while [[ $# -gt 0 ]]; do
    case "$1" in
    --clean)
        clean=true
        ;;
    --clean-keys)
        clean_keys=true
        ;;
    --all)
        patch=true
        stage1=true
        payload=true
        try_make_keys=true
        ;;
    --keys)
        keys=true
        ;;
    --patch)
        patch=true
        ;;
    --stage1)
        stage1=true
        ;;
    --payload)
        payload=true
        ;;
    --exploit)
        exploit=true
        ;;
    --)
        shift
        args="$*"
        break
        ;;
    -h | --help)
        print_help
        ;;
    *)
        echo -e "Bad option: $1\n"
        print_help
        ;;
    esac
    shift
done

if $clean; then
    if [ -d "$DIST_DIR" ]; then
        echo "Removing dist"
        rm -r "$DIST_DIR"
    fi

    if [ -d "$PATCH_DIR/build" ]; then
        echo "Cleaning $PATCH_DIR"
        rm -r "$PATCH_DIR/build"
    fi

    if [ -d "$STAGE1_DIR/build" ]; then
        echo "Cleaning $STAGE1_DIR"
        rm -r "$STAGE1_DIR/build"
    fi

    if [ -d "$PAYLOAD_DIR/build" ]; then
        echo "Removing $PAYLOAD_DIR/build"
        rm -r "$PAYLOAD_DIR/build"
    fi

    if [ -d "$PAYLOAD_DIR/binary" ]; then
        echo "Removing $PAYLOAD_DIR/binary"
        rm -r "$PAYLOAD_DIR/binary"
    fi

    if [ -d "$EXPLOIT_DIR/build" ]; then
        echo "Cleaning $EXPLOIT_DIR"
        rm -r "$EXPLOIT_DIR/build"
    fi
fi

if $clean_keys; then
    remove_keys
fi

mkdir -p "$DIST_DIR"

if $keys || ( (! check_keys) && $try_make_keys); then
    remove_keys

    cd "$MISC_DIR"
    echo "Creating new keys!"

    python generate-key.py new private-key-OUT.pem wwfcPayloadPublicKey-OUT.hpp

    echo "Copying wwfcPayloadPublicKey-OUT.hpp to $SCRIPT_DIR/include/wwfcPayloadPublicKey.hpp"
    cp "$MISC_DIR/wwfcPayloadPublicKey-OUT.hpp" "$SCRIPT_DIR/include/wwfcPayloadPublicKey.hpp"
    echo "Copying private-key-OUT.pem to dist"
    cp "$MISC_DIR/private-key-OUT.pem" "$DIST_DIR/private-key.pem"
fi

if $payload || $stage1 || $patch; then
    if ! check_keys; then
        echo "$SCRIPT_DIR/misc/private-key-OUT.pem not found! Please generate keys using '--keys' before attempting to build anything else"
        exit 1
    fi
fi

if $payload; then
    cd "$PAYLOAD_DIR"
    echo "Making payloads!"

    python make-payload.py $args

    if [ -e "$SCRIPT_DIR/dist/binary" ]; then
        rm -r "$SCRIPT_DIR/dist/binary"
    fi

    echo "Copying payloads to dist!"
    cp -r "$SCRIPT_DIR/payload/binary/" "$SCRIPT_DIR/dist/binary"
fi

if $stage1; then
    cd "$STAGE1_DIR"
    echo "Making stage1!"

    python make-stage1.py $args

    echo "Copying stage1.bin to dist!"
    cp "$SCRIPT_DIR/stage1/build/stage1.bin" "$SCRIPT_DIR/dist/stage1.bin"
fi

if $patch; then
    cd "$PATCH_DIR"
    echo -e "Generating patches!"

    python make-patch.py $args
fi

if $exploit; then
    cd "$EXPLOIT_DIR"
    echo -e "Making exploit!"

    python make-sbcm-patch.py $args
fi
