[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$ProgressPreference = 'SilentlyContinue'
$modelsDir = Join-Path $PSScriptRoot '..\models'
if (-not (Test-Path $modelsDir)) { New-Item -ItemType Directory -Path $modelsDir -Force | Out-Null }

function Get-ExpectedSha256 {
    param([string]$FileName)
    $checksumsPath = Join-Path $modelsDir 'checksums.json'
    if (-not (Test-Path $checksumsPath)) { return $null }
    try {
        $checksums = Get-Content $checksumsPath -Raw | ConvertFrom-Json
        if ($checksums.files.PSObject.Properties.Name -contains $FileName) {
            $hash = $checksums.files.$FileName
            if (-not [string]::IsNullOrWhiteSpace($hash)) { return $hash.ToLower() }
        }
    } catch {
        Write-Host "  Warning: could not read checksums.json"
    }
    return $null
}

function Test-FileSha256 {
    param([string]$FilePath, [string]$ExpectedSha256)
    if ([string]::IsNullOrWhiteSpace($ExpectedSha256)) { return $true }
    $actual = (Get-FileHash -Path $FilePath -Algorithm SHA256).Hash.ToLower()
    if ($actual -ne $ExpectedSha256.ToLower()) {
        Write-Host "  SHA256 mismatch for $(Split-Path $FilePath -Leaf)"
        Write-Host "    expected: $ExpectedSha256"
        Write-Host "    actual:   $actual"
        return $false
    }
    Write-Host "  SHA256 verified."
    return $true
}

$models = @(
    @{ Name = 'yolov8n.onnx'; Url = 'https://github.com/ultralytics/assets/releases/download/v8.4.0/yolov8n.onnx' },
    @{ Name = 'yolov8n-cls.onnx'; Url = 'https://github.com/ultralytics/assets/releases/download/v8.4.0/yolov8n-cls.onnx' },
    @{ Name = 'yolov8n-seg.onnx'; Url = 'https://github.com/ultralytics/assets/releases/download/v8.4.0/yolov8n-seg.onnx' }
)

foreach ($m in $models) {
    $target = Join-Path $modelsDir $m.Name
    $expectedSha256 = Get-ExpectedSha256 -FileName $m.Name

    if (Test-Path $target) {
        if (-not (Test-FileSha256 -FilePath $target -ExpectedSha256 $expectedSha256)) {
            Remove-Item $target -Force
        } else {
            Write-Host "$($m.Name) already exists, skipping."
            continue
        }
    }

    Write-Host "Downloading: $($m.Url)"
    try {
        Invoke-WebRequest -Uri $m.Url -OutFile $target -UseBasicParsing -TimeoutSec 300
        $size = (Get-Item $target).Length
        if ($size -lt 1000000) {
            Remove-Item $target -Force -ErrorAction SilentlyContinue
            Write-Host "  File too small, likely an error page."
            continue
        }
        if (-not (Test-FileSha256 -FilePath $target -ExpectedSha256 $expectedSha256)) {
            Remove-Item $target -Force -ErrorAction SilentlyContinue
            Write-Host "  Checksum verification failed."
            continue
        }
        Write-Host "  Done. Size: $([math]::Round($size / 1MB, 2)) MB"
    } catch {
        Write-Host "  Failed: $($_.Exception.Message)"
        Write-Host "  Please download manually from: $($m.Url)"
    }
}
