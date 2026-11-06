#!/usr/bin/env bash
# First build of this card's checkout, or an incremental one when it exists. RECOMPILE=1 after a us.toml change.
set -e
W=/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000
cd $W
source C:/Users/Benla/AppData/Local/recomp-wintools/env.sh
if [ ! -d RecompiledFuncs ] || [ -n "$RECOMPILE" ]; then
  time bash tools/recompile.sh -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl
fi
if [ ! -f build/win/build.ninja ]; then
  time cmake -S . -B build/win -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DPATCHES_C_COMPILER=clang -DPATCHES_LD=ld.lld
fi
MSYS_NO_PATHCONV=1 wsl -d Ubuntu-24.04 -u root -- make -C /mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000/patches CC=clang LD=ld.lld
time ninja -C build/win DinosaurPlanet -j 12
