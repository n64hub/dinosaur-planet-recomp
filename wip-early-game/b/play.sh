#!/usr/bin/env bash
# play.sh <name> <secs> <uids> <args...>; env INPUT POKES FROM EVERY
SC=/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000
n=$1; s=$2; u=$3; shift 3
rm -rf $SC/runs/$n
pk=""
[ -n "$POKES" ] && pk="$(cygpath -w $POKES)"
pwsh -NoProfile -File "$(cygpath -w $SC/b/mon.ps1)" -Run $n -Uids "$u" -Log "$(cygpath -w $SC/b/mon-$n.log)" -Pokes "$pk" > $SC/b/mon-$n.out 2>&1 &
sleep 2
cd $SC; bash run.sh $n $s "$@"
wait
