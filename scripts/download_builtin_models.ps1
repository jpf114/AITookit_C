[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$ProgressPreference = 'SilentlyContinue'
$modelsDir = Join-Path $PSScriptRoot '..\models'
if (-not (Test-Path $modelsDir)) { New-Item -ItemType Directory -Path $modelsDir -Force | Out-Null }

$models = @(
    @{ Name = 'yolov8n.onnx'; Url = 'https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n.onnx' },
    @{ Name = 'yolov8n-cls.onnx'; Url = 'https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n-cls.onnx' },
    @{ Name = 'yolov8n-seg.onnx'; Url = 'https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n-seg.onnx' }
)

$mirrors = @(
    { param($u) $u -replace 'https://github.com', 'https://ghgo.xyz/https://github.com' },
    { param($u) $u -replace 'https://github.com', 'https://mirror.ghproxy.com/https://github.com' },
    { param($u) $u }
)

foreach ($m in $models) {
    $target = Join-Path $modelsDir $m.Name
    if (Test-Path $target) {
        Write-Host "$($m.Name) already exists, skipping."
        continue
    }

    $downloaded = $false
    foreach ($mirrorFn in $mirrors) {
        $url = & $mirrorFn $m.Url
        Write-Host "Trying: $url"
        try {
            Invoke-WebRequest -Uri $url -OutFile $target -UseBasicParsing -TimeoutSec 300
            $size = (Get-Item $target).Length
            if ($size -gt 1000000) {
                Write-Host "  Done. Size: $([math]::Round($size / 1MB, 2)) MB"
                $downloaded = $true
                break
            } else {
                Remove-Item $target -Force -ErrorAction SilentlyContinue
                Write-Host "  File too small, likely an error page. Retrying..."
            }
        } catch {
            Write-Host "  Failed: $($_.Exception.Message)"
        }
    }
    if (-not $downloaded) {
        Write-Host "  All mirrors failed for $($m.Name). Please download manually from:"
        Write-Host "  $($m.Url)"
    }
}
