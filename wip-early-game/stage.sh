#!/usr/bin/env bash
# stage.sh: refreshes the portable play folder: exe, DLLs, assets, Vulkan settings, the ROM store
SC=/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000
W=/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000
P=$SC/play; L=/c/Users/Benla/AppData/Local/DinosaurPlanetPrototype
mkdir -p $P; cp -u $W/build/win/DinosaurPlanet.exe $W/build/win/*.dll $P/; [ -d $P/assets ] || cp -r $W/assets $P/assets
touch $P/portable.txt
cp -n $L/*.json $P/ 2>/dev/null; [ -d $P/roms ] || cp -r $L/roms $P/roms 2>/dev/null
sed -i 's/"api_option": "[A-Za-z0-9]*"/"api_option": "Vulkan"/; s/"rr_option": "[A-Za-z]*"/"rr_option": "Manual"/' $P/graphics.json
ls $P
