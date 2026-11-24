$ErrorActionPreference = "Stop"
New-Item -ItemType Directory -Force build | Out-Null

# 1) descobre modulos e dependencias
$script:mod = @{}
foreach ($f in Get-ChildItem TADs/*.cppm, src/*.cppm) {
    $txt = Get-Content $f.FullName -Raw
    $m = [regex]::Match($txt, '(?m)^\s*export\s+module\s+([\w\.]+)\s*;')
    if (-not $m.Success) { Write-Host "Ignorando (sem export module): $($f.Name)"; continue }
    $deps = @([regex]::Matches($txt, '(?m)^\s*(?:export\s+)?import\s+([\w\.]+)\s*;') | ForEach-Object { $_.Groups[1].Value })
    $script:mod[$m.Groups[1].Value] = @{ Fonte = $f.FullName; Deps = $deps; Base = $f.BaseName }
}

# 2) ordem topologica (dependencias primeiro)
$script:ordem = New-Object System.Collections.ArrayList
$script:visitado = @{}
function Visitar($n) {
    if ($script:visitado.ContainsKey($n)) { return }
    $script:visitado[$n] = $true
    foreach ($d in $script:mod[$n].Deps) { if ($script:mod.ContainsKey($d)) { Visitar $d } }
    [void]$script:ordem.Add($n)
}
foreach ($n in @($script:mod.Keys)) { Visitar $n }

# 3) pre-compila (.pcm) e compila (.o) cada modulo
$flags = @()
$objs = @()
foreach ($n in $script:ordem) {
    $i = $script:mod[$n]
    $pcm = "build/$($i.Base).pcm"
    $obj = "build/$($i.Base).o"
    Write-Host "== $n"
    clang++ -std=c++20 -Isrc @flags --precompile $i.Fonte -o $pcm
    if ($LASTEXITCODE -ne 0) { throw "Falhou ao pre-compilar: $n" }
    clang++ -std=c++20 -Isrc @flags -c $pcm -o $obj
    if ($LASTEXITCODE -ne 0) { throw "Falhou ao compilar: $n" }
    $flags += "-fmodule-file=$n=$pcm"
    $objs += $obj
}

# 4) timer, main e ligacao
Write-Host "== timer e main"
clang++ -std=c++20 -Isrc -c src/timer.cpp -o build/timer.o
if ($LASTEXITCODE -ne 0) { throw "Falhou: timer.cpp" }
clang++ -std=c++20 -Isrc @flags -c src/main.cpp -o build/main.o
if ($LASTEXITCODE -ne 0) { throw "Falhou: main.cpp" }
clang++ build/main.o build/timer.o @objs -o build/dado_burger.exe
if ($LASTEXITCODE -ne 0) { throw "Falhou na ligacao" }
Write-Host "OK: build/dado_burger.exe"