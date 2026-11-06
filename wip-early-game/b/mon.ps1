param([string]$Run,[string]$Uids='',[string]$Log,[string]$Pokes='',[int]$Ms=250)
$sc='C:/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000'
Add-Type -Path "$sc/b/Mon.cs"
[Mon]::Go("$sc/runs/$Run/pid.txt",$Uids,$Log,$Pokes,$Ms)
