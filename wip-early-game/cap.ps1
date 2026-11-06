param([string]$Exe,[string]$WorkDir,[string]$Plan,[string]$OutDir,[string]$ArgsFile,[string[]]$EnvVars=@())
$a = (Get-Content -Raw $ArgsFile).Trim()
& "C:/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/capture_window.ps1" -Exe $Exe -WorkDir $WorkDir -Plan $Plan -OutDir $OutDir -ExeArgs $a -Env $EnvVars
