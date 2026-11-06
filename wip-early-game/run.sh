#!/usr/bin/env bash
# run.sh <name> <seconds> <args...>: boots the game, shoots the window every EVERY s from FROM s. INPUT=<file> plays a script.
SC=/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000
W=$SC/play
n=$1; secs=$2; shift 2
out=$SC/runs/$n; rm -rf $out; mkdir -p $out
: > $out/plan.txt
for ((t=${FROM:-20}; t<secs; t+=${EVERY:-5})); do echo "$t shot s$(printf %03d $t)" >> $out/plan.txt; done
echo "$secs end" >> $out/plan.txt
inp=""; [ -n "$INPUT" ] && inp="--input $(cygpath -w $INPUT)"
echo "--boot --report --window ${WIN:-960x720} $inp $*" > $out/args.txt
pwsh -NoProfile -File "$(cygpath -w $SC/cap.ps1)" -Exe "$(cygpath -w $W/DinosaurPlanet.exe)" -WorkDir "$(cygpath -w $W)" -Plan "$(cygpath -w $out/plan.txt)" -OutDir "$(cygpath -w $out)" -ArgsFile "$(cygpath -w $out/args.txt)" > $out/capture.log 2>&1
echo done $n; tail -8 $out/capture.log | cut -c1-160
