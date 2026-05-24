[CmdletBinding()]
param(
    [string]$ModelsDirectory = (Join-Path $PSScriptRoot '..\models')
)

$checksumsPath = Join-Path $ModelsDirectory 'checksums.json'
if (-not (Test-Path $checksumsPath)) {
    Write-Error "checksums.json not found at $checksumsPath"
    exit 1
}

$data = Get-Content $checksumsPath -Raw | ConvertFrom-Json
$updated = 0

foreach ($name in $data.files.PSObject.Properties.Name) {
    $path = Join-Path $ModelsDirectory $name
    if (-not (Test-Path $path)) {
        Write-Host "Skip missing: $name"
        continue
    }

    $hash = (Get-FileHash -Path $path -Algorithm SHA256).Hash.ToLower()
    if ($data.files.$name -ne $hash) {
        $data.files.$name = $hash
        ++$updated
    }
    Write-Host "$name -> $hash"
}

$json = $data | ConvertTo-Json -Depth 4
[System.IO.File]::WriteAllText($checksumsPath, $json + [Environment]::NewLine, [System.Text.UTF8Encoding]::new($false))
Write-Host "Updated $updated checksum(s) in $checksumsPath"
