# Compileaza, ruleaza testele unitare si testul spectral pe toate fisierele NIST.
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

Write-Host "`n=== teste unitare (doctest) ===`n"
& .\build\dft_tests.exe

Write-Host "`n=== assessment peste fisierele de date NIST ===`n"
& .\build\nist_assess.exe -t dft data\data.e data\data.pi data\data.sqrt2 data\data.sqrt3 data\data.sha1 |
    Tee-Object -FilePath tests\results.txt
