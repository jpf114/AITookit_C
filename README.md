# AI 检测工具

基于 YOLO 的 AI 目标检测/分类/分割桌面工具，使用 C++ / Qt6 / ONNX Runtime 构建。

## 功能特性

- **多任务推理** — 支持目标检测、图像分类、实例分割三种任务
- **多模型支持** — YOLOv5 / YOLOv8 / YOLOX 解码器，通过模型清单自动分发
- **GPU 加速** — 可选 CUDA Provider，自动回退 CPU
- **批量推理** — 支持单张图像、批量图像、视频帧推理
- **结果浏览** — 检测框叠加、分类 Top-K 列表、分割掩码叠加，按类别过滤
- **导出** — JSON 结果 + 渲染图片导出，批量导出支持
- **模型目录** — 内置模型一键下载，自动加载
- **插件架构** — BackendPlugin 接口 + BackendRegistry 注册表，可扩展推理后端

## 快速开始

### 环境要求

- Windows 10/11 x64
- Visual Studio 2022（MSVC v143）
- CMake ≥ 3.21
- [vcpkg](https://vcpkg.io/)（全局安装模式，需设置 `VCPKG_ROOT` 环境变量）

### 安装依赖（首次）

```bash
# 最小依赖集：Qt6、OpenCV、ONNX Runtime
pwsh -ExecutionPolicy Bypass -File scripts/install_vcpkg_deps.ps1
```

### 构建

```bash
# 配置（使用 VCPKG_ROOT 全局 installed 目录）
cmake --preset release

# 编译
cmake --build build/release --config Release

# 运行测试
ctest --test-dir build/release --preset release

# 安装
cmake --install build/release --config Release
```

### GPU 构建（可选）

```bash
vcpkg install onnxruntime[cuda] --triplet x64-windows
cmake --preset release -DAI_ENABLE_CUDA=ON
cmake --build build/release --config Release
```

构建时会自动生成中英文翻译文件（`.qm`）并复制到可执行文件目录。

### 打包

```bash
# 生成 NSIS 安装程序 + ZIP 便携包
cd build/release
cpack -C Release
```

### 使用

1. 启动应用，点击「下载示例模型」或「模型目录」获取预配置模型
2. 模型自动加载后，点击「快速体验」选择图像进行推理
3. 在推理页调整置信度/NMS 阈值，查看结果
4. 结果页支持类别过滤、图像缩放平移、多结果浏览，以及当前结果 / 批量结果导出
5. 直接导入 `.onnx` 文件时，会根据文件名自动推断任务类型（`-cls` 分类、`-seg` 分割）；也可手动提供 JSON 清单

### 可选后端编译标志

```bash
# GPU（ONNX Runtime CUDA）
cmake --preset release -DAI_ENABLE_CUDA=ON

# 预留 TensorRT / OpenVINO 插件（当前为骨架，需 SDK 才能完成推理）
cmake --preset release -DAI_ENABLE_TENSORRT=ON -DAI_ENABLE_OPENVINO=ON
```

详见 [扩展推理后端](docs/算法说明/推理后端/扩展推理后端.md)。

## 模型清单

模型通过 JSON 清单文件描述，放置在 `models/` 目录下：

```json
{
    "name": "YOLOv8n COCO",
    "task_type": "detection",
    "backend": "onnxruntime",
    "model": "yolov8n.onnx",
    "input_width": 640,
    "input_height": 640,
    "confidence_threshold": 0.25,
    "nms_threshold": 0.45,
    "labels_inline": ["person", "bicycle", "car", ...]
}
```

支持的 `task_type`：`detection`、`classification`、`segmentation`

支持的 `decoder`（后处理策略）：`yolo_v8`（默认）、`yolo_v5`、`yolo_x`

## 项目结构

```
src/
├── app/            # 应用入口、引导、崩溃处理
├── core/           # 基础工具：JSON、配置、Unicode 路径
├── models/         # 推理模型：检测/分类/分割 + 后处理注册表
├── runtime/        # 推理后端：ONNX Runtime + 插件架构
├── services/       # 业务服务：模型管理、推理、导出
└── ui/             # Qt 界面：页面、对话框、控件
```

## 技术栈

| 组件 | 技术 |
|------|------|
| UI 框架 | Qt 6 (Widgets) |
| 推理引擎 | ONNX Runtime |
| 图像处理 | OpenCV 4 |
| 包管理 | vcpkg manifest 模式 |
| 构建系统 | CMake + CPack |
| CI/CD | GitHub Actions |

## 许可证

[MIT License](LICENSE)

## 文档

- [用户指南](docs/USER_GUIDE.md) — 首次使用、GPU 设置、FAQ
- [隐私政策](PRIVACY.md)
- [服务条款](TERMS.md)
- [第三方声明](THIRD_PARTY_NOTICES.md)
- [贡献指南](CONTRIBUTING.md)
