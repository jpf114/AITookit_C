[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$ProgressPreference = 'SilentlyContinue'

$modelsDir = Join-Path $PSScriptRoot '..\models'
$catalogPath = Join-Path $PSScriptRoot '..\resources\model_catalog.json'
$catalog = Get-Content $catalogPath -Raw | ConvertFrom-Json

foreach ($model in $catalog.models) {
    $target = Join-Path $modelsDir $model.fileName
    if (Test-Path $target) {
        Write-Host "$($model.fileName) already exists, skipping download."
        continue
    }

    Write-Host "Downloading $($model.fileName)..."
    try {
        Invoke-WebRequest -Uri $model.url -OutFile $target -UseBasicParsing -TimeoutSec 300
        $size = (Get-Item $target).Length
        if ($size -lt 1000000) {
            Remove-Item $target -Force -ErrorAction SilentlyContinue
            Write-Host "  File too small, likely an error page."
            continue
        }
        Write-Host "  Done. Size: $([math]::Round($size / 1MB, 2)) MB"
    } catch {
        Write-Host "  Failed: $($_.Exception.Message)"
    }
}

& (Join-Path $PSScriptRoot 'update_model_checksums.ps1') -ModelsDirectory $modelsDir
