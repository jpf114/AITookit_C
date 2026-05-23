# 第三方组件声明

AIToolkit（AI 检测工具）使用以下开源及第三方组件。完整许可证文本请参阅各项目官方仓库。

## 应用程序

| 组件 | 许可证 | 说明 |
|------|--------|------|
| AIToolkit 应用代码 | [MIT License](LICENSE) | Copyright (c) 2026 AIToolkit |

## 运行时依赖

| 组件 | 许可证 | 用途 | 主页 |
|------|--------|------|------|
| Qt 6 | [LGPL v3](https://www.qt.io/licensing/) | 图形界面框架（动态链接） | https://www.qt.io/ |
| ONNX Runtime | [MIT License](https://github.com/microsoft/onnxruntime/blob/main/LICENSE) | AI 模型推理引擎 | https://onnxruntime.ai/ |
| OpenCV 4 | [Apache License 2.0](https://opencv.org/license/) | 图像/视频读写与处理 | https://opencv.org/ |

## 可选/开发依赖

| 组件 | 许可证 | 用途 |
|------|--------|------|
| vcpkg | [MIT License](https://github.com/microsoft/vcpkg/blob/master/LICENSE.txt) | C++ 包管理 |
| CMake | [BSD 3-Clause](https://cmake.org/licensing/) | 构建系统 |

## 预置与可下载模型

通过「模型目录」下载的 Ultralytics YOLO 系列模型（如 YOLOv8、YOLO11）采用：

**GNU Affero General Public License v3.0 (AGPL-3.0)**

- 模型来源：https://github.com/ultralytics/assets
- 许可说明：https://ultralytics.com/license
- **商业使用限制：** 若您的使用场景不符合 AGPL-3.0 要求，请向 Ultralytics 获取商业许可

本应用 **不预装** ONNX 模型权重；用户需主动下载。下载前会显示许可提示。

## 分发说明

- 本应用以 MIT 许可证发布源代码
- 安装包中动态链接 Qt 库，符合 LGPL v3 要求
- 安装包包含 ONNX Runtime 与 OpenCV 的运行时 DLL
- 使用 `-DUSE_STATIC_QT=ON` 静态链接 Qt 时，须自行确保 LGPL 合规或持有 Qt 商业许可

## 归属声明

使用本软件时，请保留上述组件的版权声明。如需完整许可证文本，请参阅各组件官方仓库中的 LICENSE 文件。
