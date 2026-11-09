# Grabs the game's window as it plays, for unattended runs on Windows: every -Every seconds, -Count times, the client
# area of the running DinosaurPlanet.exe at -Exe, at the window's own size, into <-Prefix><index>-<seconds>.png (the
# seconds since the grabs started). Start it beside the run, e.g. from bash:
#
#   powershell.exe -NoProfile -ExecutionPolicy Bypass -File tools/grab_window.ps1 -Exe "$(cygpath -w build/DinosaurPlanet.exe)" \
#       -Prefix "$(cygpath -w shots/run-)" -Every 1 -Count 40 &
#   build/DinosaurPlanet.exe --boot --story before-dragon-rock --input <script> --report
#
# It uses PrintWindow, which draws the window itself, so a window over the game is never in a grab (copying the screen
# copies whatever covers it). And it is DPI-aware: on a display scaled past 100% a process that isn't gets the window's
# size scaled down from GetClientRect while PrintWindow still draws its real pixels, so a grab held only the top left
# of the picture (853x480 of a 1280x720 window at 150%), and what the HUD draws at the right and the bottom, the C
# buttons, the C-right and C-left menus and the panel under the minimap, was never in it.
param([string]$Exe, [string]$Prefix, [double]$Every = 1, [int]$Count = 30)

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public class GrabWindow {
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr window, IntPtr dc, uint flags);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr window, out RECT rect);
    public struct RECT { public int Left, Top, Right, Bottom; }
}
"@
[void][GrabWindow]::SetProcessDPIAware()
$exePath = (Resolve-Path $Exe).Path
$clock = [Diagnostics.Stopwatch]::StartNew()
for ($i = 0; $i -lt $Count; $i++) {
    Start-Sleep -Milliseconds ([int]($Every * 1000))
    $process = Get-Process DinosaurPlanet -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $exePath } | Select-Object -First 1
    if (-not $process -or $process.MainWindowHandle -eq 0) { continue }
    $rect = New-Object GrabWindow+RECT
    [void][GrabWindow]::GetClientRect($process.MainWindowHandle, [ref]$rect)
    if ($rect.Right -le 0 -or $rect.Bottom -le 0) { continue }
    $bitmap = New-Object System.Drawing.Bitmap $rect.Right, $rect.Bottom
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $dc = $graphics.GetHdc()
    # 3: PW_CLIENTONLY | PW_RENDERFULLCONTENT, the client area as the GPU drew it.
    [void][GrabWindow]::PrintWindow($process.MainWindowHandle, $dc, 3)
    $graphics.ReleaseHdc($dc)
    $bitmap.Save(("{0}{1:D3}-{2}.png" -f $Prefix, $i, [int]$clock.Elapsed.TotalSeconds))
    $graphics.Dispose()
    $bitmap.Dispose()
}
