# AI 检测工具 — 用户指南

面向最终用户的使用说明、GPU 配置与常见问题解答。

## 快速开始

1. 安装并启动 **AI 检测工具**
2. 在首页点击 **下载示例模型** 或 **模型目录**，获取预配置 YOLO 模型（需网络）
3. 模型加载后，点击 **快速体验** 选择图片进行推理
4. 在 **推理** 页调整置信度/NMS 阈值，点击运行
5. 在 **结果** 页查看检测框/分类/分割掩码，并导出 JSON 或渲染图片

## GPU 加速（CUDA）

### 要求

- NVIDIA GPU（Compute Capability 支持 CUDA）
- 已安装 [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads) 与对应驱动
- 使用 **启用 CUDA 的构建版本**（见下方开发者说明）

### 使用步骤

1. 打开 **设置** 页
2. 勾选 **使用 GPU 加速推理（CUDA）**
3. 重启推理（重新加载模型后生效）
4. 若 GPU 不可用，应用会自动回退 CPU 并提示

### 自行编译 GPU 版本（开发者）

```bash
# 安装带 CUDA 的 ONNX Runtime
vcpkg install onnxruntime[cuda] --triplet x64-windows

# 配置并启用 CUDA
cmake --preset release -DAI_ENABLE_CUDA=ON
cmake --build build/release --config Release
```

## 模型清单

模型通过 JSON 清单描述，放置在 `models/` 目录。直接导入 `.onnx` 时会根据文件名自动推断任务类型：

- 文件名含 `-cls` → **图像分类**（默认输入 224×224）
- 文件名含 `-seg` → **实例分割**
- 其他 → **目标检测**

也可在 **设置 → 模型目录 URL** 中指定自定义 catalog 地址（留空则使用 GitHub 官方 catalog）。

详见 [README.md](../README.md) 中的 manifest 示例。

## 企业部署与代码签名

GitHub Release 安装包默认 **未签名**。Windows SmartScreen 可能提示“未知发布者”，属正常现象。

如需企业内部分发，可：

1. 使用 EV 代码签名证书对 NSIS 安装包签名
2. 在 CI 中配置 `AI_CODESIGN_THUMBPRINT` secret（见 `.github/workflows/build.yml`）
3. 使用 `signtool` 本地签名后再分发

签名后 SmartScreen 警告会显著减少，但仍需证书信誉积累。

## 导出

- **单张结果：** JSON + 渲染图片（PNG/TIFF）
- **批量/视频：** 一键导出 JSON 数组
- 默认导出目录可在 **设置** 中配置

## 隐私与许可

- 所有推理在本地完成，不上传您的图像
- 模型下载仅连接 GitHub 官方源
- 详见 [PRIVACY.md](../PRIVACY.md) 与 [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md)
- Ultralytics 模型采用 AGPL-3.0，商业使用请查阅 [ultralytics.com/license](https://ultralytics.com/license)

## 常见问题（FAQ）

### 启动后界面为英文 / 切换语言无效

语言文件（`.qm`）随安装包发布。若从源码运行，请先执行 `cmake --build` 以生成翻译文件。切换语言后需 **重启应用**。

### 下载模型失败

- 检查网络与防火墙
- 确认可访问 `github.com`
- 可手动从 [Ultralytics assets](https://github.com/ultralytics/assets/releases) 下载 `.onnx` 到 `models/` 目录

### GPU 选项勾选后仍使用 CPU

- 确认安装的是 CUDA 版构建（`-DAI_ENABLE_CUDA=ON`）
- 确认 NVIDIA 驱动与 CUDA 运行库已安装
- 查看状态栏或错误提示中的回退信息

### 中文路径图片无法打开

v1.0 已支持 Unicode 路径。若仍失败，请将文件移至英文路径或更新到最新版本。

### 崩溃后 dump 文件在哪？

`%LOCALAPPDATA%\AIToolkit\crash_dumps\` — 仅保存在本机，不会自动上传。

### 如何反馈问题？

通过 GitHub Issues 提交，请附上版本号（`ai_toolkit_c.exe --version`）、操作系统与复现步骤。

## 自动更新

- 默认 **启动后 3 秒** 在后台检查 GitHub Release（每 24 小时最多一次）
- 可在 **设置** 中关闭「启动时检查更新」
- 也可随时点击 **检查更新** 手动查询

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| Ctrl+O | 打开图像 |
| Ctrl+S | 导出 JSON |
| Ctrl+Q | 退出 |

## 支持格式

- **图像：** PNG, JPG, JPEG, BMP, TIFF, WebP
- **视频：** MP4, AVI, MKV, MOV, WebM, FLV 等（依赖 OpenCV）
