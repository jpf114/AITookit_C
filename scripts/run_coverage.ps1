[CmdletBinding()]
param(
    [string]$BuildDirectory = (Join-Path $PSScriptRoot '..\build\release'),
    [string]$Config = 'Release'
)

$openCppCoverage = @(
    "${env:ProgramFiles}\OpenCppCoverage\OpenCppCoverage.exe",
    "${env:ProgramFiles(x86)}\OpenCppCoverage\OpenCppCoverage.exe"
) | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $openCppCoverage) {
    Write-Error "OpenCppCoverage not found. Install via: choco install opencppcoverage"
    exit 1
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot '..')
$coberturaPath = Join-Path $repoRoot 'cobertura.xml'
$htmlPath = Join-Path $repoRoot 'coverage-html'

$env:QT_QPA_PLATFORM = 'minimal'

& $openCppCoverage `
    --sources "$repoRoot\src" `
    --export_type "cobertura:$coberturaPath" `
    --export_type "html:$htmlPath" `
    -- `
    ctest --test-dir $BuildDirectory -C $Config --output-on-failure

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "Coverage report: $coberturaPath"
Write-Host "HTML report: $htmlPath"
