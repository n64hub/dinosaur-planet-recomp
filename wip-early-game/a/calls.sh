cd /c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/
for s in $(./py a/xr.py 8008C9F4 44 | awk '{print $1"@"$2}'); do f=${s%@*}; a=${s#*@}; awk -v f="$f" -v a="$a" '/^== /{p=($2==f)} p' a/all.asm | grep -n -B7 -A3 "^$a:" | grep -E 'addiu \$a[01], |or \$a[01], |sb? |lbu \$a|lb \$a|lh \$a|lw \$a[01]|jalr' | cut -c1-60 | tr '\n' ';' | sed "s/^/$f $a: /"; echo; done
