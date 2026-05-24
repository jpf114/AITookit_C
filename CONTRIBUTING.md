# 贡献指南

感谢你对 AIToolkit 项目的关注！本文档介绍如何参与项目开发。

## 开发环境搭建

### 前置条件

- Windows 10/11 x64
- Visual Studio 2022（MSVC v143）
- CMake ≥ 3.21
- [vcpkg](https://vcpkg.io/)（**全局安装模式**，需设置 `VCPKG_ROOT` 环境变量）

### 构建步骤

```bash
# 克隆仓库
git clone <repo-url>
cd AITookit_C

# 安装最小 vcpkg 依赖（首次）
pwsh -ExecutionPolicy Bypass -File scripts/install_vcpkg_deps.ps1

# 配置
cmake --preset release

# 编译
cmake --build build/release --config Release

# 运行测试
ctest --test-dir build/release --preset release

# 下载内置模型（首次运行需要）
powershell -ExecutionPolicy Bypass -File scripts/download_builtin_models.ps1
```

### Debug 构建

```bash
cmake --preset debug
cmake --build build/debug --config Debug
```

## 代码规范

### 命名约定

- 命名空间：`aitoolkit::module`（如 `aitoolkit::core`、`aitoolkit::ui`）
- 类名：`PascalCase`（如 `InferenceService`）
- 成员变量：`trailing_underscore_`（如 `currentModel_`）
- 常量：`kCamelCase`（如 `kRecentModelsKey`）
- Qt 信号/槽：遵循 Qt 命名规范

### 代码风格

- C++17 标准，`CXX_EXTENSIONS OFF`
- 使用 `#pragma once` 作为头文件保护
- 命名空间闭合处添加 `// namespace` 注释
- MSVC 下启用 `/utf-8` 编译选项

### 样式管理

- UI 样式统一通过 `resources/themes/app.qss` 管理
- 使用 `setObjectName()` + QSS 选择器，避免内联 `setStyleSheet()`
- 色值使用项目自定义色系，不使用 Tailwind 等外部色板

## 项目架构

```
src/
├── app/            # 应用入口、引导、崩溃处理
├── core/           # 基础工具：JSON、配置、Unicode 路径、类型定义
├── models/         # 推理模型：检测/分类/分割 + 后处理注册表
├── runtime/        # 推理后端：ONNX Runtime + 插件架构
├── services/       # 业务服务：模型管理、推理、导出
└── ui/             # Qt 界面：页面、对话框、控件
```

### 关键设计模式

- **插件架构**：`BackendPlugin` + `BackendRegistry`，新增推理后端只需实现接口并注册
- **后处理注册表**：`PostprocessRegistry`，新增解码器通过 `registerDecoder()` 注册
- **模型清单**：JSON 格式描述模型配置，支持自定义解码器和标签

## 提交规范

### Commit Message 格式

```
<类型>: <简要描述>

<可选的详细说明>
```

类型包括：

- `feat`: 新功能
- `fix`: Bug 修复
- `refactor`: 重构（不改变外部行为）
- `test`: 测试补充或修改
- `docs`: 文档更新
- `style`: 代码格式调整
- `chore`: 构建、CI、依赖等工程化变更

### 示例

```
feat: 添加 TensorRT 后端插件

实现 BackendPlugin 接口，支持 NVIDIA GPU 加速推理。
自动检测 CUDA 可用性，不可用时回退到 CPU。
```

## 测试

### 运行测试

```bash
ctest --test-dir build/release --preset release --output-on-failure
```

### 编写测试

- 使用 Qt Test 框架（`QTEST_MAIN` + `QObject` 派生类）
- 测试文件放置在 `tests/` 目录，命名格式 `test_<模块名>.cpp`
- 在 `tests/CMakeLists.txt` 中注册新测试
- 使用 `QCOMPARE`、`QVERIFY`、`QSignalSpy` 等断言
- 对于需要 ONNX Runtime 的测试，使用 `FakeInferenceBackend` 隔离

### 测试覆盖重点

- 核心数据层（model_manifest、settings_store）需包含边界和异常测试
- 后处理逻辑（yolo_postprocess、classification、segmentation）需独立测试
- 服务层需使用 Fake/Mock 隔离外部依赖

## 添加新推理后端

1. 在 `src/runtime/` 创建新后端类，实现 `BackendPlugin` 接口
2. 在 `src/runtime/` 创建插件注册函数，类似 `onnx_plugin.h/cpp`
3. 在 `app_bootstrap.cpp` 中调用注册函数
4. 更新 `src/CMakeLists.txt` 添加新源文件
5. 补充单元测试

## 添加新模型类型

1. 在 `src/models/` 创建新模型类，继承 `InferenceBackend`
2. 在 `onnx_plugin.cpp` 的 `createModel()` 中添加新任务类型映射
3. 如需自定义后处理，在 `PostprocessRegistry` 中注册新解码器
4. 更新 UI 页面支持新任务类型
5. 补充单元测试

## 许可证

本项目采用 [MIT License](LICENSE)。提交代码即表示你同意以相同许可证授权。

注意：Ultralytics YOLO 模型采用 AGPL-3.0 许可证，商业使用需获取 Ultralytics 商业许可。

## 平台范围

当前 **仅支持 Windows 10/11 x64**，为明确的产品定位，**暂不计划** Linux/macOS 移植或跨平台 CI。

代码中通过 `#ifdef Q_OS_WIN` 隔离平台相关逻辑（崩溃处理、Unicode I/O、Qt 平台插件复制）。若社区有平台适配需求，可先与维护者讨论是否纳入范围。
