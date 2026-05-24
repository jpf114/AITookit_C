[CmdletBinding()]
param(
    [string]$CatalogPath = (Join-Path $PSScriptRoot '..\resources\model_catalog.json'),
    [string]$ModelsDirectory = (Join-Path $PSScriptRoot '..\models')
)

$checksumsPath = Join-Path $ModelsDirectory 'checksums.json'
if (-not (Test-Path $CatalogPath)) {
    Write-Error "Catalog not found: $CatalogPath"
    exit 1
}

$catalog = Get-Content $CatalogPath -Raw | ConvertFrom-Json
$checksums = if (Test-Path $checksumsPath) {
    Get-Content $checksumsPath -Raw | ConvertFrom-Json
} else {
    [pscustomobject]@{
        version = 1
        description = 'Optional SHA256 checksums for downloaded ONNX models. Leave hash empty to skip verification.'
        files = [ordered]@{}
    }
}

if (-not $checksums.files) {
    $checksums | Add-Member -NotePropertyName files -NotePropertyValue ([ordered]@{}) -Force
}

$existing = @{}
foreach ($name in $checksums.files.PSObject.Properties.Name) {
    $existing[$name] = $checksums.files.$name
}

$merged = [ordered]@{}
foreach ($model in $catalog.models) {
    $fileName = $model.fileName
    if ([string]::IsNullOrWhiteSpace($fileName)) { continue }

    $hash = $existing[$fileName]
    if ([string]::IsNullOrWhiteSpace($hash)) {
        $modelPath = Join-Path $ModelsDirectory $fileName
        if (Test-Path $modelPath) {
            $hash = (Get-FileHash -Path $modelPath -Algorithm SHA256).Hash.ToLower()
        } else {
            $hash = ''
        }
    }
    $merged[$fileName] = $hash
    Write-Host "$fileName -> $(if ($hash) { $hash } else { '(empty)' })"
}

$output = [pscustomobject]@{
    version = 1
    description = $checksums.description
    files = $merged
}

$json = $output | ConvertTo-Json -Depth 4
[System.IO.File]::WriteAllText($checksumsPath, $json + [Environment]::NewLine, [System.Text.UTF8Encoding]::new($false))
Write-Host "Wrote $($merged.Count) entries to $checksumsPath"
