# 最小 vcpkg 依赖（全局安装模式）
#
# 用法（需先设置 VCPKG_ROOT 环境变量）:
#   pwsh -ExecutionPolicy Bypass -File scripts/install_vcpkg_deps.ps1
#
# 可选 GPU 推理:
#   vcpkg install onnxruntime[cuda] --triplet x64-windows
# 可选后端骨架（TensorRT / OpenVINO，需 SDK 才能完成集成）:
#   cmake --preset release -DAI_ENABLE_TENSORRT=ON -DAI_ENABLE_OPENVINO=ON

$ErrorActionPreference = "Stop"

if (-not $env:VCPKG_ROOT) {
    Write-Error "请先设置 VCPKG_ROOT 环境变量指向 vcpkg 安装目录。"
}

$triplet = "x64-windows"
$packages = @(
    "qtbase[widgets,testlib,network]",
    "qtsvg",
    "opencv4[jpeg,png,msmf]",
    "onnxruntime"
)

Write-Host "Installing minimal vcpkg packages for AIToolkit ($triplet)..."
foreach ($pkg in $packages) {
    Write-Host "  -> vcpkg install $pkg --triplet $triplet"
    vcpkg install $pkg --triplet $triplet
}

Write-Host ""
Write-Host "Done. Configure with: cmake --preset release"
