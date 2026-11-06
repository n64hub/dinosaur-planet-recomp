#!/usr/bin/env bash
# q.sh <script>: one queue turn
SC=/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000
"/c/Users/Benla/AppData/Local/HoloCareBot/node/node.exe" /c/Users/Benla/AppData/Local/HoloCareBot/code/gpu.mjs --run --shared --max ${MAX:-3} -- bash "$@" 2>&1 | tail -4
