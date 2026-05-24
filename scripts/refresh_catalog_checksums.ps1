[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$ProgressPreference = 'SilentlyContinue'

$modelsDir = Join-Path $PSScriptRoot '..\models'
$catalogPath = Join-Path $PSScriptRoot '..\resources\model_catalog.json'
$catalog = Get-Content $catalogPath -Raw | ConvertFrom-Json

foreach ($model in $catalog.models) {
    $target = Join-Path $modelsDir $model.fileName
    if (Test-Path $target) {
        Remove-Item $target -Force
    }

    Write-Host "Downloading $($model.fileName)..."
    for ($attempt = 1; $attempt -le 3; $attempt++) {
        try {
            Invoke-WebRequest -Uri $model.url -OutFile $target -UseBasicParsing -TimeoutSec 600
            if ((Get-Item $target).Length -gt 1000000) {
                Write-Host "  Done."
                break
            }
            Remove-Item $target -Force -ErrorAction SilentlyContinue
            Write-Host "  File too small."
        } catch {
            Write-Host "  Attempt $attempt failed: $($_.Exception.Message)"
            Start-Sleep -Seconds 3
        }
    }
}

& (Join-Path $PSScriptRoot 'update_model_checksums.ps1') -ModelsDirectory $modelsDir
