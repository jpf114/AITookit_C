# Shared model manifest generation for download scripts.
# Dot-source from download_sample_model.ps1 and download_builtin_models.ps1

$script:ModelManifestRepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))

function Get-ModelLabelsFilePath {
    param([string]$LabelsCategory)
    if ([string]::IsNullOrWhiteSpace($LabelsCategory)) { return $null }
    $labelsPath = Join-Path $script:ModelManifestRepoRoot ("resources\labels\{0}.txt" -f $LabelsCategory)
    if (Test-Path $labelsPath) { return $labelsPath }
    return $null
}

function Get-ModelLabelsReferencePath {
    param([string]$LabelsCategory)
    switch ($LabelsCategory) {
        "coco80" { return "../resources/labels/coco80.txt" }
        "imagenet1000" { return "../resources/labels/imagenet1000.txt" }
        default { return $null }
    }
}

function Test-ModelLabelsCategoryAvailable {
    param([string]$LabelsCategory)
    $reference = Get-ModelLabelsReferencePath -LabelsCategory $LabelsCategory
    if (-not $reference) { return $false }
    return (Test-Path (Get-ModelLabelsFilePath -LabelsCategory $LabelsCategory))
}

function New-ModelManifestObject {
    param(
        [Parameter(Mandatory = $true)][string]$DisplayName,
        [Parameter(Mandatory = $true)][string]$TaskType,
        [Parameter(Mandatory = $true)][string]$OnnxFileName,
        [Parameter(Mandatory = $true)][int]$InputSize,
        [string]$Decoder = "",
        [string]$LabelsCategory = "coco80",
        [double]$ConfidenceThreshold = 0.25,
        [double]$NmsThreshold = 0.45
    )

    if (-not (Test-ModelLabelsCategoryAvailable -LabelsCategory $LabelsCategory)) {
        throw "Labels file not found for category: $LabelsCategory"
    }

    $manifest = [ordered]@{
        name = $DisplayName
        task_type = $TaskType
        backend = "onnxruntime"
        model = $OnnxFileName
        input_width = $InputSize
        input_height = $InputSize
        confidence_threshold = $ConfidenceThreshold
        nms_threshold = $NmsThreshold
    }

    if (($TaskType -eq "detection" -or $TaskType -eq "segmentation") -and -not [string]::IsNullOrEmpty($Decoder)) {
        $manifest["decoder"] = $Decoder
    }

    $labelsReference = Get-ModelLabelsReferencePath -LabelsCategory $LabelsCategory
    if ($labelsReference) {
        $manifest["labels"] = $labelsReference
    }

    return $manifest
}

function Write-ModelManifestFile {
    param(
        [Parameter(Mandatory = $true)][string]$JsonFilePath,
        [Parameter(Mandatory = $true)]$Manifest
    )

    $jsonText = ConvertTo-Json -InputObject $Manifest -Depth 3
    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($JsonFilePath, $jsonText + [Environment]::NewLine, $utf8NoBom)
}

function Write-ModelManifestFromCatalogEntry {
    param(
        [Parameter(Mandatory = $true)][string]$ModelsDir,
        [Parameter(Mandatory = $true)]$CatalogEntry
    )

    $modelName = [System.IO.Path]::GetFileNameWithoutExtension($CatalogEntry.fileName)
    $jsonFilePath = Join-Path $ModelsDir ($modelName + ".json")
    $manifest = New-ModelManifestObject `
        -DisplayName $CatalogEntry.name `
        -TaskType $CatalogEntry.taskType `
        -OnnxFileName $CatalogEntry.fileName `
        -InputSize $CatalogEntry.inputSize `
        -Decoder $CatalogEntry.decoder `
        -LabelsCategory $CatalogEntry.labelsCategory

    Write-ModelManifestFile -JsonFilePath $jsonFilePath -Manifest $manifest
    return $jsonFilePath
}

function Get-CatalogEntryByFileName {
    param(
        [Parameter(Mandatory = $true)][string]$CatalogPath,
        [Parameter(Mandatory = $true)][string]$FileName
    )

    if (-not (Test-Path $CatalogPath)) {
        throw "Catalog file not found: $CatalogPath"
    }

    $catalog = Get-Content $CatalogPath -Raw | ConvertFrom-Json
    foreach ($entry in $catalog.models) {
        if ($entry.fileName -eq $FileName) {
            return $entry
        }
    }

    throw "Catalog entry not found for file: $FileName"
}
