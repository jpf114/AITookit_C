param(
    [string]$ModelsDir = (Join-Path $PSScriptRoot "..\models"),
    [string]$ModelName = "yolov8n",
    [string]$ModelUrl = "https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n.onnx"
)

$ErrorActionPreference = "Stop"

$ModelsDir = [System.IO.Path]::GetFullPath($ModelsDir)

if (-not (Test-Path $ModelsDir)) {
    New-Item -ItemType Directory -Path $ModelsDir -Force | Out-Null
    Write-Host "Created directory: $ModelsDir"
}

$onnxFileName = "$ModelName.onnx"
$onnxFilePath = Join-Path $ModelsDir $onnxFileName
$jsonFileName = "$ModelName.json"
$jsonFilePath = Join-Path $ModelsDir $jsonFileName

$cocoLabels = @(
    "person","bicycle","car","motorcycle","airplane","bus","train","truck","boat",
    "traffic light","fire hydrant","stop sign","parking meter","bench","bird","cat",
    "dog","horse","sheep","cow","elephant","bear","zebra","giraffe","backpack",
    "umbrella","handbag","tie","suitcase","frisbee","skis","snowboard","sports ball",
    "kite","baseball bat","baseball glove","skateboard","surfboard","tennis racket",
    "bottle","wine glass","cup","fork","knife","spoon","bowl","banana","apple",
    "sandwich","orange","broccoli","carrot","hot dog","pizza","donut","cake","chair",
    "couch","potted plant","bed","dining table","toilet","tv","laptop","mouse","remote",
    "keyboard","cell phone","microwave","oven","toaster","sink","refrigerator","book",
    "clock","vase","scissors","teddy bear","hair drier","toothbrush"
)

$manifest = [ordered]@{
    name = "YOLOv8n COCO"
    task_type = "detection"
    backend = "onnxruntime"
    model = $onnxFileName
    input_width = 640
    input_height = 640
    confidence_threshold = 0.25
    nms_threshold = 0.45
    labels_inline = $cocoLabels
}

$jsonText = ConvertTo-Json -InputObject $manifest -Depth 3
Set-Content -Path $jsonFilePath -Value $jsonText -Encoding UTF8
Write-Host "Generated manifest: $jsonFilePath"

if (Test-Path $onnxFilePath) {
    Write-Host "Model file already exists: $onnxFilePath"
} else {
    Write-Host ""
    Write-Host "Attempting to download $ModelName ONNX model..."
    Write-Host "  Source: $ModelUrl"
    Write-Host "  Target: $onnxFilePath"

    try {
        Invoke-WebRequest -Uri $ModelUrl -OutFile $onnxFilePath -UseBasicParsing
        $fileSize = (Get-Item $onnxFilePath).Length
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
        Write-Host "    yolo export model=yolov8n.pt format=onnx"
        Write-Host "    Copy the exported .onnx file to: $onnxFilePath"
        Write-Host ""
        Write-Host "  Option 3 - Using conda:"
        Write-Host "    conda install -c conda-forge ultralytics"
        Write-Host "    python -c `"from ultralytics import YOLO; YOLO('yolov8n.pt').export(format='onnx')`""
        Write-Host "    Copy the exported .onnx file to: $onnxFilePath"
    }
}

Write-Host ""
Write-Host "Usage:"
Write-Host "  1. Launch AI Toolkit"
Write-Host "  2. On the Models page, click 'Load Model Manifest'"
Write-Host "  3. Select: $jsonFilePath"
Write-Host "  4. Go to the Inference page, select an image, and click 'Start Detection'"
