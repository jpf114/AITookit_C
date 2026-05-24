param(
    [string]$CoberturaPath = "cobertura.xml",
    [double]$MinimumLineRate = 0.60
)

if (-not (Test-Path $CoberturaPath)) {
    Write-Error "Coverage report not found: $CoberturaPath"
    exit 1
}

[xml]$xml = Get-Content $CoberturaPath
$coverage = $xml.coverage
if (-not $coverage) {
    Write-Error "Invalid Cobertura XML"
    exit 1
}

$lineRate = [double]$coverage.'line-rate'
$percent = [math]::Round($lineRate * 100, 2)
Write-Host "Line coverage: $percent% (minimum: $([math]::Round($MinimumLineRate * 100, 2))%)"

if ($lineRate + 1e-9 -lt $MinimumLineRate) {
    Write-Error "Coverage below threshold"
    exit 1
}

Write-Host "Coverage threshold met."
