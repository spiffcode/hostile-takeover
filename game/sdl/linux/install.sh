#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
HT_ROOT=$SCRIPT_DIR/../../..

set -v
mkdir -p ~/Games
mkdir -p ~/Games/HostileTakeover

# If release build, htdata832.pdb needs to be marked
cp $HT_ROOT/game/htdata832.pdb ~/Games/HostileTakeover
cp $HT_ROOT/game/htsfx.pdb ~/Games/HostileTakeover
set +v

if [ "$(uname -s)" == "Darwin" ]; then
    cp -r $HT_ROOT/game/sdl/SDL2/osx/SDL2.framework ~/Games/HostileTakeover
else
    if ! dpkg -l | grep libsdl2-dev > /dev/null ; then
        echo "sudo password may be required to download and install the SDL2 library..."
        sudo apt-get install libsdl2-dev
    fi
fi
