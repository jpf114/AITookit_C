param(
    [string]$ModelsDir = (Join-Path $PSScriptRoot "..\models"),
    [string]$ModelName = "yolov8n",
    [string]$ModelUrl = "https://github.com/ultralytics/assets/releases/download/v8.4.0/yolov8n.onnx",
    [string]$TaskType = "detection",
    [string]$Decoder = "yolo_v8",
    [string]$LabelsCategory = "coco80",
    [int]$InputSize = 640,
    [string]$ExpectedSha256 = ""
)

$ErrorActionPreference = "Stop"

function Test-DownloadedSha256 {
    param([string]$FilePath, [string]$ExpectedHash)
    if ([string]::IsNullOrWhiteSpace($ExpectedHash)) { return $true }
    $actual = (Get-FileHash -Path $FilePath -Algorithm SHA256).Hash.ToLower()
    if ($actual -ne $ExpectedHash.ToLower()) {
        Write-Warning "SHA256 mismatch: expected $ExpectedHash, got $actual"
        return $false
    }
    Write-Host "SHA256 verified."
    return $true
}

function Get-ChecksumFromManifest {
    param([string]$FileName, [string]$ModelsDirectory)
    $checksumsPath = Join-Path $ModelsDirectory "checksums.json"
    if (-not (Test-Path $checksumsPath)) { return $null }
    try {
        $checksums = Get-Content $checksumsPath -Raw | ConvertFrom-Json
        if ($checksums.files.PSObject.Properties.Name -contains $FileName) {
            $hash = $checksums.files.$FileName
            if (-not [string]::IsNullOrWhiteSpace($hash)) { return $hash }
        }
    } catch { }
    return $null
}

$ModelsDir = [System.IO.Path]::GetFullPath($ModelsDir)

if (-not (Test-Path $ModelsDir)) {
    New-Item -ItemType Directory -Path $ModelsDir -Force | Out-Null
    Write-Host "Created directory: $ModelsDir"
}

$onnxFileName = "$ModelName.onnx"
$onnxFilePath = Join-Path $ModelsDir $onnxFileName
$jsonFileName = "$ModelName.json"
$jsonFilePath = Join-Path $ModelsDir $jsonFileName

$RepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))

function Get-LabelsFilePath {
    param([string]$LabelsCategory)
    if ([string]::IsNullOrWhiteSpace($LabelsCategory)) { return $null }
    $labelsPath = Join-Path $RepoRoot ("resources\labels\{0}.txt" -f $LabelsCategory)
    if (Test-Path $labelsPath) { return $labelsPath }
    return $null
}

function Get-LabelsReferencePath {
    param([string]$LabelsCategory)
    switch ($LabelsCategory) {
        "coco80" { return "../resources/labels/coco80.txt" }
        "imagenet1000" { return "../resources/labels/imagenet1000.txt" }
        default { return $null }
    }
}


$labels = @()
$labelsReference = Get-LabelsReferencePath -LabelsCategory $LabelsCategory
if ($labelsReference) {
    if (-not (Get-LabelsFilePath -LabelsCategory $LabelsCategory)) {
        throw "Labels file not found for category: $LabelsCategory"
    }
}

$manifest = [ordered]@{
    name = $ModelName
    task_type = $TaskType
    backend = "onnxruntime"
    model = $onnxFileName
    input_width = $InputSize
    input_height = $InputSize
    confidence_threshold = 0.25
    nms_threshold = 0.45
}

if ($TaskType -eq "detection" -or $TaskType -eq "segmentation") {
    if (-not [string]::IsNullOrEmpty($Decoder)) {
        $manifest["decoder"] = $Decoder
    }
}

if ($labelsReference) {
    $manifest["labels"] = $labelsReference
} elseif ($labels.Count -gt 0) {
    $manifest["labels_inline"] = $labels
}

$jsonText = ConvertTo-Json -InputObject $manifest -Depth 3
Set-Content -Path $jsonFilePath -Value $jsonText -Encoding UTF8
Write-Host "Generated manifest: $jsonFilePath"

if (Test-Path $onnxFilePath) {
    Write-Host "Model file already exists: $onnxFilePath"
} else {
    if ([string]::IsNullOrWhiteSpace($ExpectedSha256)) {
        $ExpectedSha256 = Get-ChecksumFromManifest -FileName $onnxFileName -ModelsDirectory $ModelsDir
    }

    Write-Host ""
    Write-Host "Attempting to download $ModelName ONNX model..."
    Write-Host "  Source: $ModelUrl"
    Write-Host "  Target: $onnxFilePath"

    try {
        Invoke-WebRequest -Uri $ModelUrl -OutFile $onnxFilePath -UseBasicParsing
        $fileSize = (Get-Item $onnxFilePath).Length
        if ($fileSize -lt 1000000) {
            Remove-Item $onnxFilePath -Force -ErrorAction SilentlyContinue
            throw "Downloaded file too small ($fileSize bytes)"
        }
        if (-not (Test-DownloadedSha256 -FilePath $onnxFilePath -ExpectedHash $ExpectedSha256)) {
            Remove-Item $onnxFilePath -Force -ErrorAction SilentlyContinue
            throw "SHA256 verification failed"
        }
        Write-Host "Download complete, file size: $([math]::Round($fileSize / 1MB, 2)) MB"
    } catch {
        Write-Warning "Automatic download failed. Please download the model manually:"
        Write-Host ""
        Write-Host "  Option 1 - Direct download:"
        Write-Host "    Visit: $ModelUrl"
        Write-Host "    Save to: $onnxFilePath"
        Write-Host ""
        Write-Host "  Option 2 - Using pip (requires Python with ultralytics):"
        Write-Host "    pip install ultralytics"
        Write-Host "    yolo export model=$ModelName.pt format=onnx"
        Write-Host "    Copy the exported .onnx file to: $onnxFilePath"
        Write-Host ""
        Write-Host "  Option 3 - Using conda:"
        Write-Host "    conda install -c conda-forge ultralytics"
        Write-Host "    python -c `"from ultralytics import YOLO; YOLO('$ModelName.pt').export(format='onnx')`""
        Write-Host "    Copy the exported .onnx file to: $onnxFilePath"
    }
}

Write-Host ""
Write-Host "Usage:"
Write-Host "  1. Launch AI Toolkit"
Write-Host "  2. On the Models page, click 'Load Model Manifest'"
Write-Host "  3. Select: $jsonFilePath"
$taskLabel = switch ($TaskType) { "detection" { "Start Detection" } "classification" { "Start Classification" } "segmentation" { "Start Segmentation" } default { "Start Inference" } }
Write-Host "  4. Go to the Inference page, select an image, and click '$taskLabel'"
