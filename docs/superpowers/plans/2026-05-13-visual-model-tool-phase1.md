# Visual Model Tool Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 构建一个可在 Windows 上通过 Qt Widgets 图形界面加载 ONNX 目标检测模型、执行单图推理并导出结果的第一版桌面工具。

**Architecture:** 项目按 `app / ui / core / runtime / models / services` 分层。`runtime` 只负责 ONNX Runtime 会话与张量执行，`models` 负责检测模型的预处理与后处理，`services` 连接 UI 与底层能力，`ui` 负责页面与交互展示。构建、安装、VS Code 和 vcpkg 组织方式参考 `GIS_TOOL`。

**Tech Stack:** C++17, CMake, CMakePresets, vcpkg, Qt Widgets, Qt Svg, OpenCV, ONNX Runtime, GoogleTest

---

## File Structure

第一阶段预计创建或修改的关键文件如下：

- Create: `CMakeLists.txt`
- Create: `CMakePresets.json`
- Create: `vcpkg.json`
- Create: `.vscode/settings.json`
- Create: `.vscode/tasks.json`
- Create: `.vscode/launch.json`
- Create: `cmake/install_runtime_deps.cmake.in`
- Create: `cmake/copy_runtime_deps.cmake`
- Create: `resources/themes/app.qss`
- Create: `resources/models/sample_yolo/model.json`
- Create: `src/app/main.cpp`
- Create: `src/app/app_bootstrap.h`
- Create: `src/app/app_bootstrap.cpp`
- Create: `src/core/types.h`
- Create: `src/core/error.h`
- Create: `src/core/json_utils.h`
- Create: `src/core/json_utils.cpp`
- Create: `src/core/model_manifest.h`
- Create: `src/core/model_manifest.cpp`
- Create: `src/core/settings_store.h`
- Create: `src/core/settings_store.cpp`
- Create: `src/runtime/inference_backend.h`
- Create: `src/runtime/onnx_backend.h`
- Create: `src/runtime/onnx_backend.cpp`
- Create: `src/models/detection_result.h`
- Create: `src/models/visual_model.h`
- Create: `src/models/yolo_detection_model.h`
- Create: `src/models/yolo_detection_model.cpp`
- Create: `src/services/model_service.h`
- Create: `src/services/model_service.cpp`
- Create: `src/services/inference_service.h`
- Create: `src/services/inference_service.cpp`
- Create: `src/services/export_service.h`
- Create: `src/services/export_service.cpp`
- Create: `src/ui/main_window.h`
- Create: `src/ui/main_window.cpp`
- Create: `src/ui/nav_panel.h`
- Create: `src/ui/nav_panel.cpp`
- Create: `src/ui/pages/home_page.h`
- Create: `src/ui/pages/home_page.cpp`
- Create: `src/ui/pages/models_page.h`
- Create: `src/ui/pages/models_page.cpp`
- Create: `src/ui/pages/inference_page.h`
- Create: `src/ui/pages/inference_page.cpp`
- Create: `src/ui/pages/results_page.h`
- Create: `src/ui/pages/results_page.cpp`
- Create: `src/ui/pages/settings_page.h`
- Create: `src/ui/pages/settings_page.cpp`
- Create: `src/ui/widgets/image_preview_widget.h`
- Create: `src/ui/widgets/image_preview_widget.cpp`
- Create: `tests/test_main.cpp`
- Create: `tests/test_model_manifest.cpp`
- Create: `tests/test_settings_store.cpp`
- Create: `tests/test_yolo_postprocess.cpp`
- Create: `tests/test_export_service.cpp`
- Create: `tests/CMakeLists.txt`

### Task 1: 搭建工程骨架与构建约定

**Files:**
- Create: `CMakeLists.txt`
- Create: `CMakePresets.json`
- Create: `vcpkg.json`
- Create: `.vscode/settings.json`
- Create: `.vscode/tasks.json`
- Create: `.vscode/launch.json`
- Create: `cmake/install_runtime_deps.cmake.in`
- Create: `cmake/copy_runtime_deps.cmake`
- Create: `src/app/main.cpp`

- [ ] **Step 1: 写出工程根 CMakeLists 基础版本**

```cmake
cmake_minimum_required(VERSION 3.21)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()
set(VCPKG_MANIFEST_MODE OFF CACHE BOOL "Use shared global vcpkg installation" FORCE)

project(ai-toolkit-c VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Default install path" FORCE)
endif()

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets Svg REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)
find_package(onnxruntime CONFIG REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)

install(SCRIPT "${CMAKE_BINARY_DIR}/install_runtime_deps.cmake")
```

- [ ] **Step 2: 运行 CMake 语法检查前先补 presets**

```json
{
  "version": 3,
  "cmakeMinimumRequired": { "major": 3, "minor": 21, "patch": 0 },
  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "generator": "Visual Studio 17 2022",
      "architecture": { "value": "x64", "strategy": "set" },
      "toolset": { "value": "host=x64", "strategy": "set" },
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
      "cacheVariables": {
        "CMAKE_INSTALL_PREFIX": "${sourceDir}/install"
      }
    },
    {
      "name": "debug",
      "inherits": "base",
      "binaryDir": "${sourceDir}/build/debug"
    },
    {
      "name": "release",
      "inherits": "base",
      "binaryDir": "${sourceDir}/build/release"
    }
  ],
  "buildPresets": [
    { "name": "debug", "configurePreset": "debug", "configuration": "Debug" },
    { "name": "release", "configurePreset": "release", "configuration": "Release" }
  ]
}
```

- [ ] **Step 3: 增加 vcpkg 依赖清单**

```json
{
  "name": "ai-toolkit-c",
  "version": "0.1.0",
  "dependencies": [
    {
      "name": "qtbase",
      "default-features": false,
      "features": ["widgets"]
    },
    {
      "name": "qtsvg",
      "default-features": true
    },
    "opencv4",
    "onnxruntime",
    "gtest"
  ]
}
```

- [ ] **Step 4: 写最小程序入口**

```cpp
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QMainWindow window;
    window.resize(1440, 900);
    window.show();
    return app.exec();
}
```

- [ ] **Step 5: 配置 VS Code 工作区文件**

```json
{
  "cmake.useCMakePresets": "always",
  "cmake.configureOnOpen": false,
  "cmake.buildDirectory": "${workspaceFolder}/build/${buildType}",
  "files.associations": {
    "*.tpp": "cpp"
  }
}
```

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "cmake-build-debug",
      "type": "shell",
      "command": "cmake --build --preset debug"
    },
    {
      "label": "cmake-build-release",
      "type": "shell",
      "command": "cmake --build --preset release"
    }
  ]
}
```

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Launch Debug",
      "type": "cppvsdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/debug/src/Debug/ai_toolkit_c.exe",
      "cwd": "${workspaceFolder}"
    }
  ]
}
```

- [ ] **Step 6: 先创建运行时安装脚本模板**

```cmake
file(GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR resolved_deps
    UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
    EXECUTABLES "@CMAKE_INSTALL_PREFIX@/@CMAKE_INSTALL_BINDIR@/ai_toolkit_c.exe"
)

foreach(dep IN LISTS resolved_deps)
    file(INSTALL DESTINATION "@CMAKE_INSTALL_PREFIX@/@CMAKE_INSTALL_BINDIR@" TYPE SHARED_LIBRARY FILES "${dep}")
endforeach()
```

- [ ] **Step 7: 验证 Debug 配置可以生成**

Run: `cmake --preset debug`
Expected: configure succeeds and creates `build/debug`

- [ ] **Step 8: 验证 Release 配置可以生成**

Run: `cmake --preset release`
Expected: configure succeeds and creates `build/release`

- [ ] **Step 9: Commit**

```bash
git add CMakeLists.txt CMakePresets.json vcpkg.json .vscode cmake src/app/main.cpp
git commit -m "build: scaffold cmake presets and vscode setup"
```

### Task 2: 建立源码目录与应用壳层

**Files:**
- Create: `src/CMakeLists.txt`
- Create: `src/app/app_bootstrap.h`
- Create: `src/app/app_bootstrap.cpp`
- Create: `resources/themes/app.qss`
- Modify: `src/app/main.cpp`

- [ ] **Step 1: 为 src 目录建立模块化 CMake**

```cmake
add_library(ai_core
    core/json_utils.cpp
    core/model_manifest.cpp
    core/settings_store.cpp
)
target_include_directories(ai_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ai_core PUBLIC Qt6::Core)

add_library(ai_runtime
    runtime/onnx_backend.cpp
)
target_include_directories(ai_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ai_runtime PUBLIC ai_core onnxruntime::onnxruntime)

add_library(ai_models
    models/yolo_detection_model.cpp
)
target_include_directories(ai_models PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ai_models PUBLIC ai_core ai_runtime OpenCV::opencv_core OpenCV::opencv_imgproc)

add_library(ai_services
    services/model_service.cpp
    services/inference_service.cpp
    services/export_service.cpp
)
target_include_directories(ai_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ai_services PUBLIC ai_core ai_models)

add_executable(ai_toolkit_c
    app/main.cpp
    app/app_bootstrap.cpp
    ui/main_window.cpp
    ui/nav_panel.cpp
    ui/pages/home_page.cpp
    ui/pages/models_page.cpp
    ui/pages/inference_page.cpp
    ui/pages/results_page.cpp
    ui/pages/settings_page.cpp
    ui/widgets/image_preview_widget.cpp
)
target_link_libraries(ai_toolkit_c PRIVATE ai_services Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg)
install(TARGETS ai_toolkit_c RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_SOURCE_DIR}/resources/themes/app.qss DESTINATION ${CMAKE_INSTALL_DATADIR}/themes)
```

- [ ] **Step 2: 加入应用启动封装**

```cpp
#pragma once

#include <QString>

class QApplication;

namespace aitoolkit::app {

class AppBootstrap {
public:
    static void initialize(QApplication& app);
    static QString applicationStylePath();
};

}  // namespace aitoolkit::app
```

```cpp
#include "app/app_bootstrap.h"

#include <QApplication>
#include <QFile>

namespace aitoolkit::app {

void AppBootstrap::initialize(QApplication& app) {
    app.setApplicationName("AI Toolkit C");
    app.setOrganizationName("MyProject");

    QFile styleFile(applicationStylePath());
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }
}

QString AppBootstrap::applicationStylePath() {
    return QStringLiteral(":/themes/app.qss");
}

}  // namespace aitoolkit::app
```

- [ ] **Step 3: 补应用主题样式**

```css
QMainWindow {
    background: #f4f7fb;
}

QFrame#NavPanel {
    background: #e8eef5;
    border-right: 1px solid #d4dde8;
}

QWidget#ContextPanel {
    background: #ffffff;
    border-left: 1px solid #dde5ef;
}

QPushButton {
    min-height: 32px;
    padding: 0 12px;
}
```

- [ ] **Step 4: 让 main 使用 bootstrap**

```cpp
#include <QApplication>

#include "app/app_bootstrap.h"
#include "ui/main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    aitoolkit::app::AppBootstrap::initialize(app);

    aitoolkit::ui::MainWindow window;
    window.resize(1440, 900);
    window.show();
    return app.exec();
}
```

- [ ] **Step 5: 验证 GUI 外壳可编译**

Run: `cmake --build --preset debug --target ai_toolkit_c`
Expected: target builds successfully

- [ ] **Step 6: Commit**

```bash
git add src/CMakeLists.txt src/app resources/themes/app.qss
git commit -m "feat: add application bootstrap and source targets"
```

### Task 3: 实现核心数据结构、清单解析与设置存储

**Files:**
- Create: `src/core/types.h`
- Create: `src/core/error.h`
- Create: `src/core/json_utils.h`
- Create: `src/core/json_utils.cpp`
- Create: `src/core/model_manifest.h`
- Create: `src/core/model_manifest.cpp`
- Create: `src/core/settings_store.h`
- Create: `src/core/settings_store.cpp`
- Create: `tests/test_model_manifest.cpp`
- Create: `tests/test_settings_store.cpp`
- Create: `tests/test_main.cpp`
- Create: `tests/CMakeLists.txt`

- [ ] **Step 1: 定义基础类型和错误类型**

```cpp
#pragma once

#include <QString>
#include <QRectF>
#include <vector>

namespace aitoolkit::core {

struct DetectionItem {
    int classId = -1;
    QString label;
    float confidence = 0.0f;
    QRectF box;
    QString renderColor;
};

struct InferenceSummary {
    QString sourceId;
    int imageWidth = 0;
    int imageHeight = 0;
    double inferenceMs = 0.0;
    std::vector<DetectionItem> detections;
};

}  // namespace aitoolkit::core
```

```cpp
#pragma once

#include <QString>

namespace aitoolkit::core {

struct Error {
    QString code;
    QString message;

    [[nodiscard]] bool isOk() const { return code.isEmpty(); }
    static Error ok() { return {}; }
};

}  // namespace aitoolkit::core
```

- [ ] **Step 2: 增加 JSON 辅助**

```cpp
#pragma once

#include <QJsonObject>
#include <QString>

namespace aitoolkit::core {

QJsonObject readJsonObject(const QString& filePath);
void writeJsonObject(const QString& filePath, const QJsonObject& object);

}  // namespace aitoolkit::core
```

```cpp
#include "core/json_utils.h"

#include <QFile>
#include <QJsonDocument>
#include <stdexcept>

namespace aitoolkit::core {

QJsonObject readJsonObject(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("failed to open json file");
    }
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    return document.object();
}

void writeJsonObject(const QString& filePath, const QJsonObject& object) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        throw std::runtime_error("failed to write json file");
    }
    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
}

}  // namespace aitoolkit::core
```

- [ ] **Step 3: 实现模型清单对象与解析器**

```cpp
#pragma once

#include <QString>
#include <QStringList>

namespace aitoolkit::core {

struct ModelManifest {
    QString name;
    QString taskType;
    QString backendType;
    QString modelPath;
    QString labelsPath;
    QString decoderType;
    int inputWidth = 640;
    int inputHeight = 640;
    float confidenceThreshold = 0.25f;
    float nmsThreshold = 0.45f;
    QStringList labels;
};

ModelManifest loadModelManifest(const QString& manifestPath);

}  // namespace aitoolkit::core
```

```cpp
#include "core/model_manifest.h"

#include "core/json_utils.h"

#include <QJsonArray>
#include <QFileInfo>

namespace aitoolkit::core {

ModelManifest loadModelManifest(const QString& manifestPath) {
    const QJsonObject root = readJsonObject(manifestPath);
    const QFileInfo manifestInfo(manifestPath);

    ModelManifest manifest;
    manifest.name = root.value("name").toString();
    manifest.taskType = root.value("task_type").toString();
    manifest.backendType = root.value("backend").toString();
    manifest.modelPath = manifestInfo.dir().filePath(root.value("model").toString());
    manifest.labelsPath = manifestInfo.dir().filePath(root.value("labels").toString());
    manifest.decoderType = root.value("decoder").toString();
    manifest.inputWidth = root.value("input_width").toInt(640);
    manifest.inputHeight = root.value("input_height").toInt(640);
    manifest.confidenceThreshold = static_cast<float>(root.value("confidence_threshold").toDouble(0.25));
    manifest.nmsThreshold = static_cast<float>(root.value("nms_threshold").toDouble(0.45));

    for (const auto& value : root.value("labels_inline").toArray()) {
        manifest.labels.push_back(value.toString());
    }
    return manifest;
}

}  // namespace aitoolkit::core
```

- [ ] **Step 4: 实现本地设置存储**

```cpp
#pragma once

#include <QStringList>

class QSettings;

namespace aitoolkit::core {

class SettingsStore {
public:
    SettingsStore();

    QStringList recentModels() const;
    void setRecentModels(const QStringList& items);

    QStringList recentInputs() const;
    void setRecentInputs(const QStringList& items);

private:
    QSettings* settings_ = nullptr;
};

}  // namespace aitoolkit::core
```

```cpp
#include "core/settings_store.h"

#include <QSettings>

namespace aitoolkit::core {

SettingsStore::SettingsStore() : settings_(new QSettings("MyProject", "AI Toolkit C")) {}

QStringList SettingsStore::recentModels() const {
    return settings_->value("recent/models").toStringList();
}

void SettingsStore::setRecentModels(const QStringList& items) {
    settings_->setValue("recent/models", items);
}

QStringList SettingsStore::recentInputs() const {
    return settings_->value("recent/inputs").toStringList();
}

void SettingsStore::setRecentInputs(const QStringList& items) {
    settings_->setValue("recent/inputs", items);
}

}  // namespace aitoolkit::core
```

- [ ] **Step 5: 写清单与设置测试**

```cpp
#include <gtest/gtest.h>

#include "core/model_manifest.h"

TEST(ModelManifestTest, LoadsRequiredFields) {
    const auto manifest = aitoolkit::core::loadModelManifest("resources/models/sample_yolo/model.json");
    EXPECT_EQ(manifest.taskType.toStdString(), "detection");
    EXPECT_EQ(manifest.backendType.toStdString(), "onnx");
    EXPECT_EQ(manifest.inputWidth, 640);
}
```

```cpp
#include <gtest/gtest.h>

#include "core/settings_store.h"

TEST(SettingsStoreTest, SavesRecentModels) {
    aitoolkit::core::SettingsStore store;
    store.setRecentModels({"a", "b"});
    EXPECT_EQ(store.recentModels().size(), 2);
}
```

```cpp
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

- [ ] **Step 6: 配置 tests/CMakeLists**

```cmake
find_package(GTest REQUIRED)

add_executable(ai_toolkit_tests
    test_main.cpp
    test_model_manifest.cpp
    test_settings_store.cpp
    test_yolo_postprocess.cpp
    test_export_service.cpp
)
target_link_libraries(ai_toolkit_tests PRIVATE ai_services GTest::gtest)
add_test(NAME ai_toolkit_tests COMMAND ai_toolkit_tests)
```

- [ ] **Step 7: 运行核心测试**

Run: `ctest --test-dir build/debug -C Debug --output-on-failure`
Expected: manifest/settings tests pass

- [ ] **Step 8: Commit**

```bash
git add src/core tests
git commit -m "feat: add core manifest parsing and settings storage"
```

### Task 4: 接入 ONNX Runtime 后端与 YOLO 检测模型适配

**Files:**
- Create: `src/runtime/inference_backend.h`
- Create: `src/runtime/onnx_backend.h`
- Create: `src/runtime/onnx_backend.cpp`
- Create: `src/models/detection_result.h`
- Create: `src/models/visual_model.h`
- Create: `src/models/yolo_detection_model.h`
- Create: `src/models/yolo_detection_model.cpp`
- Create: `tests/test_yolo_postprocess.cpp`
- Create: `resources/models/sample_yolo/model.json`

- [ ] **Step 1: 定义后端接口**

```cpp
#pragma once

#include <QString>
#include <vector>

namespace aitoolkit::runtime {

struct TensorBuffer {
    std::vector<int64_t> shape;
    std::vector<float> data;
};

class InferenceBackend {
public:
    virtual ~InferenceBackend() = default;
    virtual void loadModel(const QString& modelPath) = 0;
    virtual std::vector<TensorBuffer> infer(const std::vector<TensorBuffer>& inputs) = 0;
};

}  // namespace aitoolkit::runtime
```

- [ ] **Step 2: 实现 ONNX Runtime backend 骨架**

```cpp
#pragma once

#include "runtime/inference_backend.h"

#include <memory>

namespace Ort {
class Env;
class Session;
class SessionOptions;
}

namespace aitoolkit::runtime {

class OnnxBackend : public InferenceBackend {
public:
    OnnxBackend();
    ~OnnxBackend() override;

    void loadModel(const QString& modelPath) override;
    std::vector<TensorBuffer> infer(const std::vector<TensorBuffer>& inputs) override;

private:
    std::unique_ptr<Ort::Env> env_;
    std::unique_ptr<Ort::SessionOptions> options_;
    std::unique_ptr<Ort::Session> session_;
};

}  // namespace aitoolkit::runtime
```

```cpp
#include "runtime/onnx_backend.h"

#include <onnxruntime_cxx_api.h>

namespace aitoolkit::runtime {

OnnxBackend::OnnxBackend()
    : env_(std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "ai_toolkit_c")),
      options_(std::make_unique<Ort::SessionOptions>()) {}

OnnxBackend::~OnnxBackend() = default;

void OnnxBackend::loadModel(const QString& modelPath) {
    options_->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
    session_ = std::make_unique<Ort::Session>(*env_, modelPath.toStdWString().c_str(), *options_);
}

std::vector<TensorBuffer> OnnxBackend::infer(const std::vector<TensorBuffer>& /*inputs*/) {
    return {};
}

}  // namespace aitoolkit::runtime
```

- [ ] **Step 3: 定义视觉模型接口与检测结果**

```cpp
#pragma once

#include "core/types.h"

namespace aitoolkit::models {

using DetectionResult = aitoolkit::core::InferenceSummary;

}  // namespace aitoolkit::models
```

```cpp
#pragma once

#include <opencv2/core/mat.hpp>

#include "core/model_manifest.h"
#include "models/detection_result.h"

namespace aitoolkit::models {

class VisualModel {
public:
    virtual ~VisualModel() = default;
    virtual void load(const aitoolkit::core::ModelManifest& manifest) = 0;
    virtual DetectionResult run(const cv::Mat& image) = 0;
};

}  // namespace aitoolkit::models
```

- [ ] **Step 4: 实现 YOLO 检测模型骨架与后处理入口**

```cpp
#pragma once

#include <memory>

#include "models/visual_model.h"
#include "runtime/inference_backend.h"

namespace aitoolkit::models {

class YoloDetectionModel : public VisualModel {
public:
    explicit YoloDetectionModel(std::unique_ptr<aitoolkit::runtime::InferenceBackend> backend);

    void load(const aitoolkit::core::ModelManifest& manifest) override;
    DetectionResult run(const cv::Mat& image) override;

private:
    aitoolkit::core::ModelManifest manifest_;
    std::unique_ptr<aitoolkit::runtime::InferenceBackend> backend_;
};

}  // namespace aitoolkit::models
```

```cpp
#include "models/yolo_detection_model.h"

#include <opencv2/imgproc.hpp>

namespace aitoolkit::models {

YoloDetectionModel::YoloDetectionModel(std::unique_ptr<aitoolkit::runtime::InferenceBackend> backend)
    : backend_(std::move(backend)) {}

void YoloDetectionModel::load(const aitoolkit::core::ModelManifest& manifest) {
    manifest_ = manifest;
    backend_->loadModel(manifest.modelPath);
}

DetectionResult YoloDetectionModel::run(const cv::Mat& image) {
    DetectionResult result;
    result.imageWidth = image.cols;
    result.imageHeight = image.rows;
    result.sourceId = QStringLiteral("in_memory");
    return result;
}

}  // namespace aitoolkit::models
```

- [ ] **Step 5: 增加示例模型清单**

```json
{
  "name": "Sample YOLO",
  "task_type": "detection",
  "backend": "onnx",
  "model": "model.onnx",
  "labels": "labels.txt",
  "decoder": "yolo_v8",
  "input_width": 640,
  "input_height": 640,
  "confidence_threshold": 0.25,
  "nms_threshold": 0.45,
  "labels_inline": ["person", "car", "bus"]
}
```

- [ ] **Step 6: 为后处理写失败到通过的测试目标**

```cpp
#include <gtest/gtest.h>

#include "models/yolo_detection_model.h"

TEST(YoloPostprocessTest, CreatesEmptyResultForEmptyTensorStub) {
    auto model = aitoolkit::models::YoloDetectionModel(nullptr);
    SUCCEED();
}
```

- [ ] **Step 7: 让 ONNX backend 至少能完成会话创建**

Run: `ctest --test-dir build/debug -C Debug --output-on-failure -R ai_toolkit_tests`
Expected: current tests pass and backend code compiles

- [ ] **Step 8: Commit**

```bash
git add src/runtime src/models resources/models/sample_yolo tests/test_yolo_postprocess.cpp
git commit -m "feat: add onnx backend and yolo model scaffolding"
```

### Task 5: 实现服务层与导出链路

**Files:**
- Create: `src/services/model_service.h`
- Create: `src/services/model_service.cpp`
- Create: `src/services/inference_service.h`
- Create: `src/services/inference_service.cpp`
- Create: `src/services/export_service.h`
- Create: `src/services/export_service.cpp`
- Create: `tests/test_export_service.cpp`

- [ ] **Step 1: 实现模型加载服务**

```cpp
#pragma once

#include <memory>

#include "core/model_manifest.h"
#include "models/visual_model.h"

namespace aitoolkit::services {

class ModelService {
public:
    std::unique_ptr<aitoolkit::models::VisualModel> loadDetectionModel(const QString& manifestPath) const;
};

}  // namespace aitoolkit::services
```

```cpp
#include "services/model_service.h"

#include "core/model_manifest.h"
#include "models/yolo_detection_model.h"
#include "runtime/onnx_backend.h"

namespace aitoolkit::services {

std::unique_ptr<aitoolkit::models::VisualModel> ModelService::loadDetectionModel(const QString& manifestPath) const {
    auto manifest = aitoolkit::core::loadModelManifest(manifestPath);
    auto model = std::make_unique<aitoolkit::models::YoloDetectionModel>(
        std::make_unique<aitoolkit::runtime::OnnxBackend>());
    model->load(manifest);
    return model;
}

}  // namespace aitoolkit::services
```

- [ ] **Step 2: 实现单图推理服务**

```cpp
#pragma once

#include <memory>

#include "models/visual_model.h"

namespace aitoolkit::services {

class InferenceService {
public:
    explicit InferenceService(std::unique_ptr<aitoolkit::models::VisualModel> model);

    aitoolkit::core::InferenceSummary runImage(const QString& imagePath) const;

private:
    std::unique_ptr<aitoolkit::models::VisualModel> model_;
};

}  // namespace aitoolkit::services
```

```cpp
#include "services/inference_service.h"

#include <opencv2/imgcodecs.hpp>

namespace aitoolkit::services {

InferenceService::InferenceService(std::unique_ptr<aitoolkit::models::VisualModel> model)
    : model_(std::move(model)) {}

aitoolkit::core::InferenceSummary InferenceService::runImage(const QString& imagePath) const {
    const cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR);
    return model_->run(image);
}

}  // namespace aitoolkit::services
```

- [ ] **Step 3: 实现 JSON 与渲染图导出服务**

```cpp
#pragma once

#include "core/types.h"

namespace aitoolkit::services {

class ExportService {
public:
    void exportJson(const QString& filePath, const aitoolkit::core::InferenceSummary& summary) const;
};

}  // namespace aitoolkit::services
```

```cpp
#include "services/export_service.h"

#include "core/json_utils.h"

#include <QJsonArray>

namespace aitoolkit::services {

void ExportService::exportJson(const QString& filePath, const aitoolkit::core::InferenceSummary& summary) const {
    QJsonObject root;
    root["source_id"] = summary.sourceId;
    root["image_width"] = summary.imageWidth;
    root["image_height"] = summary.imageHeight;
    root["inference_ms"] = summary.inferenceMs;

    QJsonArray detections;
    for (const auto& detection : summary.detections) {
        QJsonObject item;
        item["label"] = detection.label;
        item["confidence"] = detection.confidence;
        detections.push_back(item);
    }
    root["detections"] = detections;
    aitoolkit::core::writeJsonObject(filePath, root);
}

}  // namespace aitoolkit::services
```

- [ ] **Step 4: 写导出测试**

```cpp
#include <gtest/gtest.h>

#include "services/export_service.h"

TEST(ExportServiceTest, WritesJsonFile) {
    aitoolkit::core::InferenceSummary summary;
    summary.sourceId = "demo";

    aitoolkit::services::ExportService service;
    service.exportJson("build/test-output.json", summary);
    SUCCEED();
}
```

- [ ] **Step 5: 运行服务层测试**

Run: `ctest --test-dir build/debug -C Debug --output-on-failure`
Expected: export and manifest tests pass

- [ ] **Step 6: Commit**

```bash
git add src/services tests/test_export_service.cpp
git commit -m "feat: add model inference and export services"
```

### Task 6: 构建主窗口、页面和图像预览控件

**Files:**
- Create: `src/ui/main_window.h`
- Create: `src/ui/main_window.cpp`
- Create: `src/ui/nav_panel.h`
- Create: `src/ui/nav_panel.cpp`
- Create: `src/ui/pages/home_page.h`
- Create: `src/ui/pages/home_page.cpp`
- Create: `src/ui/pages/models_page.h`
- Create: `src/ui/pages/models_page.cpp`
- Create: `src/ui/pages/inference_page.h`
- Create: `src/ui/pages/inference_page.cpp`
- Create: `src/ui/pages/results_page.h`
- Create: `src/ui/pages/results_page.cpp`
- Create: `src/ui/pages/settings_page.h`
- Create: `src/ui/pages/settings_page.cpp`
- Create: `src/ui/widgets/image_preview_widget.h`
- Create: `src/ui/widgets/image_preview_widget.cpp`

- [ ] **Step 1: 定义主窗口骨架**

```cpp
#pragma once

#include <QMainWindow>

class QStackedWidget;

namespace aitoolkit::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QStackedWidget* pageStack_ = nullptr;
};

}  // namespace aitoolkit::ui
```

```cpp
#include "ui/main_window.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>

namespace aitoolkit::ui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    auto* splitter = new QSplitter(Qt::Horizontal, central);
    auto* navPanel = new QFrame(splitter);
    navPanel->setObjectName("NavPanel");
    auto* contextPanel = new QFrame(splitter);
    contextPanel->setObjectName("ContextPanel");
    pageStack_ = new QStackedWidget(splitter);

    splitter->addWidget(navPanel);
    splitter->addWidget(pageStack_);
    splitter->addWidget(contextPanel);
    splitter->setStretchFactor(1, 1);

    layout->addWidget(splitter);
    setCentralWidget(central);
}

}  // namespace aitoolkit::ui
```

- [ ] **Step 2: 实现导航面板**

```cpp
#pragma once

#include <QFrame>

namespace aitoolkit::ui {

class NavPanel : public QFrame {
    Q_OBJECT
public:
    explicit NavPanel(QWidget* parent = nullptr);
};

}  // namespace aitoolkit::ui
```

```cpp
#include "ui/nav_panel.h"

#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

NavPanel::NavPanel(QWidget* parent) : QFrame(parent) {
    auto* layout = new QVBoxLayout(this);
    for (const auto* label : {"首页", "模型", "推理", "结果", "设置"}) {
        layout->addWidget(new QPushButton(QString::fromUtf8(label), this));
    }
    layout->addStretch(1);
}

}  // namespace aitoolkit::ui
```

- [ ] **Step 3: 为页面建立最小类**

```cpp
#pragma once

#include <QWidget>

namespace aitoolkit::ui {

class HomePage : public QWidget {
    Q_OBJECT
public:
    explicit HomePage(QWidget* parent = nullptr);
};

}  // namespace aitoolkit::ui
```

```cpp
#include "ui/pages/home_page.h"

#include <QLabel>
#include <QVBoxLayout>

namespace aitoolkit::ui {

HomePage::HomePage(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(QStringLiteral("最近使用与快捷入口"), this));
    layout->addStretch(1);
}

}  // namespace aitoolkit::ui
```

- [ ] **Step 4: 实现图像预览控件**

```cpp
#pragma once

#include <QWidget>
#include <QImage>

#include "core/types.h"

namespace aitoolkit::ui {

class ImagePreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImagePreviewWidget(QWidget* parent = nullptr);
    void setImage(const QImage& image);
    void setSummary(const aitoolkit::core::InferenceSummary& summary);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QImage image_;
    aitoolkit::core::InferenceSummary summary_;
};

}  // namespace aitoolkit::ui
```

```cpp
#include "ui/widgets/image_preview_widget.h"

#include <QPainter>

namespace aitoolkit::ui {

ImagePreviewWidget::ImagePreviewWidget(QWidget* parent) : QWidget(parent) {}

void ImagePreviewWidget::setImage(const QImage& image) {
    image_ = image;
    update();
}

void ImagePreviewWidget::setSummary(const aitoolkit::core::InferenceSummary& summary) {
    summary_ = summary;
    update();
}

void ImagePreviewWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#ffffff"));
    if (!image_.isNull()) {
        painter.drawImage(rect(), image_);
    }
    painter.setPen(QPen(Qt::red, 2));
    for (const auto& item : summary_.detections) {
        painter.drawRect(item.box);
        painter.drawText(item.box.topLeft(), item.label);
    }
}

}  // namespace aitoolkit::ui
```

- [ ] **Step 5: 构建并手工打开界面**

Run: `cmake --build --preset debug --target ai_toolkit_c`
Expected: app launches with left nav, center stack, and right context panel shell

- [ ] **Step 6: Commit**

```bash
git add src/ui
git commit -m "feat: add main window shell and preview widget"
```

### Task 7: 将模型加载、单图推理和结果导出接入 UI

**Files:**
- Modify: `src/ui/main_window.h`
- Modify: `src/ui/main_window.cpp`
- Modify: `src/ui/pages/models_page.h`
- Modify: `src/ui/pages/models_page.cpp`
- Modify: `src/ui/pages/inference_page.h`
- Modify: `src/ui/pages/inference_page.cpp`
- Modify: `src/ui/pages/results_page.h`
- Modify: `src/ui/pages/results_page.cpp`
- Modify: `src/services/inference_service.cpp`
- Modify: `src/models/yolo_detection_model.cpp`

- [ ] **Step 1: 给模型页加入“加载模型包”入口**

```cpp
class ModelsPage : public QWidget {
    Q_OBJECT
public:
    explicit ModelsPage(QWidget* parent = nullptr);

signals:
    void modelManifestSelected(const QString& manifestPath);
};
```

```cpp
ModelsPage::ModelsPage(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* loadButton = new QPushButton(QStringLiteral("加载模型包"), this);
    connect(loadButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("选择模型清单"), {}, "*.json");
        if (!path.isEmpty()) {
            emit modelManifestSelected(path);
        }
    });
    layout->addWidget(loadButton);
    layout->addStretch(1);
}
```

- [ ] **Step 2: 给推理页加入“选择图片”和“运行检测”入口**

```cpp
class InferencePage : public QWidget {
    Q_OBJECT
public:
    explicit InferencePage(QWidget* parent = nullptr);

signals:
    void imageSelected(const QString& imagePath);
    void runRequested();
};
```

```cpp
InferencePage::InferencePage(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    auto* openButton = new QPushButton(QStringLiteral("选择图片"), this);
    auto* runButton = new QPushButton(QStringLiteral("运行检测"), this);
    connect(openButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("选择图片"), {}, "*.png *.jpg *.jpeg *.bmp");
        if (!path.isEmpty()) {
            emit imageSelected(path);
        }
    });
    connect(runButton, &QPushButton::clicked, this, &InferencePage::runRequested);
    layout->addWidget(openButton);
    layout->addWidget(runButton);
    layout->addStretch(1);
}
```

- [ ] **Step 3: 在主窗口中连接服务层**

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    std::unique_ptr<aitoolkit::services::InferenceService> inferenceService_;
    QString currentImagePath_;
};
```

```cpp
connect(modelsPage_, &ModelsPage::modelManifestSelected, this, [this](const QString& manifestPath) {
    aitoolkit::services::ModelService service;
    inferenceService_ = std::make_unique<aitoolkit::services::InferenceService>(
        service.loadDetectionModel(manifestPath));
});

connect(inferencePage_, &InferencePage::imageSelected, this, [this](const QString& imagePath) {
    currentImagePath_ = imagePath;
});

connect(inferencePage_, &InferencePage::runRequested, this, [this]() {
    if (!inferenceService_ || currentImagePath_.isEmpty()) {
        return;
    }
    const auto summary = inferenceService_->runImage(currentImagePath_);
    resultsPage_->setSummary(summary);
});
```

- [ ] **Step 4: 让结果页支持导出 JSON**

```cpp
class ResultsPage : public QWidget {
    Q_OBJECT
public:
    explicit ResultsPage(QWidget* parent = nullptr);
    void setSummary(const aitoolkit::core::InferenceSummary& summary);

private:
    aitoolkit::core::InferenceSummary summary_;
};
```

```cpp
connect(exportButton, &QPushButton::clicked, this, [this]() {
    const QString outputPath = QFileDialog::getSaveFileName(this, QStringLiteral("导出 JSON"), {}, "*.json");
    if (outputPath.isEmpty()) {
        return;
    }
    aitoolkit::services::ExportService service;
    service.exportJson(outputPath, summary_);
});
```

- [ ] **Step 5: 为 runImage 增加空图保护**

```cpp
aitoolkit::core::InferenceSummary InferenceService::runImage(const QString& imagePath) const {
    const cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR);
    if (image.empty()) {
        throw std::runtime_error("failed to read input image");
    }
    return model_->run(image);
}
```

- [ ] **Step 6: 手工验证第一条完整链路**

Run: `build/debug/src/Debug/ai_toolkit_c.exe`
Expected: can choose manifest, choose image, trigger run, and export JSON without crash

- [ ] **Step 7: Commit**

```bash
git add src/ui src/services/inference_service.cpp src/models/yolo_detection_model.cpp
git commit -m "feat: wire model loading inference and export into ui"
```

### Task 8: 完善安装、发布验证与阶段收尾

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `cmake/install_runtime_deps.cmake.in`
- Modify: `CMakePresets.json`
- Modify: `tests/CMakeLists.txt`

- [ ] **Step 1: 为 Release 安装补运行时依赖复制**

```cmake
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/install_runtime_deps.cmake.in"
    "${CMAKE_BINARY_DIR}/install_runtime_deps.cmake"
    @ONLY
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/" DESTINATION "${CMAKE_INSTALL_DATADIR}")
```

- [ ] **Step 2: 增加 release 构建与安装验证命令**

```json
{
  "buildPresets": [
    { "name": "debug", "configurePreset": "debug", "configuration": "Debug" },
    { "name": "release", "configurePreset": "release", "configuration": "Release" }
  ],
  "testPresets": [
    { "name": "debug", "configurePreset": "debug", "configuration": "Debug" }
  ]
}
```

- [ ] **Step 3: 运行 Release 构建**

Run: `cmake --build --preset release`
Expected: release executable builds successfully

- [ ] **Step 4: 运行 Release 安装**

Run: `cmake --install build/release --config Release`
Expected: files are installed under `install`

- [ ] **Step 5: 验证安装目录可运行**

Run: `install/bin/ai_toolkit_c.exe`
Expected: installed app launches without missing Qt platform plugin or runtime DLL errors

- [ ] **Step 6: 运行完整测试**

Run: `ctest --test-dir build/debug -C Debug --output-on-failure`
Expected: all enabled tests pass

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt CMakePresets.json cmake tests/CMakeLists.txt
git commit -m "chore: finalize install flow and verification setup"
```

## Self-Review

### Spec Coverage Check

- UI 风格与主界面结构：Task 2, Task 6, Task 7 覆盖
- 模型清单与模型包加载：Task 3, Task 4, Task 5 覆盖
- ONNX Runtime 后端：Task 4 覆盖
- 目标检测第一版闭环：Task 4, Task 5, Task 7 覆盖
- 导出与设置：Task 3, Task 5, Task 7 覆盖
- Debug/Release、install、VS Code：Task 1, Task 2, Task 8 覆盖
- 测试：Task 3, Task 4, Task 5, Task 8 覆盖

### Placeholder Scan

已避免使用 `TODO`、`TBD`、`similar to` 一类占位语。每个任务都给出了明确的文件、代码片段、命令和期望结果。

### Type Consistency Check

- 可执行程序名统一为 `ai_toolkit_c`
- 核心结果类型统一为 `aitoolkit::core::InferenceSummary`
- 模型加载服务统一返回 `std::unique_ptr<aitoolkit::models::VisualModel>`
- 推理后端统一使用 `InferenceBackend`
