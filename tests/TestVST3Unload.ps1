param([string]$BuildDir = "build-ez")
$ErrorActionPreference = "Stop"

# Exercise the real VST3 binary's Windows loader lifecycle repeatedly.
# This complements (but does not replace) a Cubase project-close test.
$plugin = Get-ChildItem -Path $BuildDir -Recurse -File -Filter "SteamMaster EZ.vst3" |
    Where-Object { $_.FullName -match "x86_64-win" } |
    Select-Object -First 1
if (-not $plugin) { throw "Compiled Windows VST3 module not found" }

Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class NativeVST3Loader {
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true, EntryPoint = "LoadLibraryW")]
    public static extern IntPtr Load(string path);
    [DllImport("kernel32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool FreeLibrary(IntPtr handle);
}
'@

for ($n = 1; $n -le 20; $n++) {
    $handle = [NativeVST3Loader]::Load($plugin.FullName)
    if ($handle -eq [IntPtr]::Zero) {
        $code = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
        throw "VST3 LoadLibrary failed at pass $n (Win32 $code)"
    }
    if (-not [NativeVST3Loader]::FreeLibrary($handle)) {
        $code = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
        throw "VST3 FreeLibrary failed at pass $n (Win32 $code)"
    }
    Write-Host "VST3_LOAD_UNLOAD_PASS $n/20"
}
Write-Host "VST3_LOADER_STRESS_PASS"
