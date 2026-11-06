# WIP: the early game's tools (remove before this branch lands)

Scripts for reading the game's DLLs and maps and for playing it unattended, kept here only so that the card's next
session has them. `README.txt` says how each is used. Paths in them point at the scratch folder and worktree of the
session that wrote them: change `SC` and `W`. Nothing here is from the ROM: `bits.txt` and `idx.pickle` are made by
`xref.py` and `d.py` from a build's `RecompiledFuncs/`.

`ngreal.txt` is the `--input` script of a real New Game through the title and name screens. `--warp` only works from
real play, after about 115 s of it.
