# 产品化改进实施计划

> **面向 AI 代理的工作者：** 必需子技能：使用 superpowers:subagent-driven-development（推荐）或 superpowers:executing-plans 逐任务实现此计划。步骤使用复选框（`- [ ]`）语法来跟踪进度。

**目标：** 让 AIToolit_C 从"开发者工具"升级为"开箱即用的产品"——首次启动即可体验完整推理流程，同时为多后端和多任务扩展奠定架构基础。

**架构：** 以"默认模型自动加载"为核心改进首次体验；以"后处理策略注册表"替代硬编码后处理，为多模型变体提供可扩展性；以"推理预热"消除首次延迟；以"后端能力查询"为未来 GPU 加速铺路。

**技术栈：** C++17 / Qt6 Widgets / ONNX Runtime / OpenCV / CMake + vcpkg

---

## 文件结构

| 文件 | 职责 | 操作 |
|------|------|------|
| `src/app/app_bootstrap.cpp` | 应用启动初始化，新增默认模型自动发现和加载 | 修改 |
| `src/ui/main_window.cpp` | 修复下载脚本参数名 bug，改进下载后自动加载逻辑 | 修改 |
| `src/ui/app_controller.h` | 新增 `tryLoadDefaultModel()` 方法声明 | 修改 |
| `src/ui/app_controller.cpp` | 实现默认模型自动发现和加载逻辑 | 修改 |
| `src/ui/pages/home_page.h` | 新增"快速体验"按钮信号 | 修改 |
| `src/ui/pages/home_page.cpp` | 新增"快速体验"按钮，改进首次引导 | 修改 |
| `scripts/download_sample_model.ps1` | 修复参数名 `-OutputDir` → `-ModelsDir` | 修改 |
| `src/models/postprocess_registry.h` | 后处理策略注册表，根据 decoder 字段分发后处理 | 新建 |
| `src/models/postprocess_registry.cpp` | 后处理策略注册表实现 | 新建 |
| `src/models/yolo_detection_model.h` | 使用后处理策略注册表替代硬编码后处理 | 修改 |
| `src/models/yolo_detection_model.cpp` | 重构 detect() 使用注册表 | 修改 |
| `src/core/model_manifest.h` | 确保 decoder 字段被正确解析 | 修改 |
| `src/runtime/onnx_backend.h` | 新增 `warmup()` 方法 | 修改 |
| `src/runtime/onnx_backend.cpp` | 实现预热推理 | 修改 |
| `src/runtime/onnx_backend.h` | 新增能力查询方法 | 修改 |
| `src/runtime/onnx_backend.cpp` | 实现能力查询 | 修改 |
| `src/CMakeLists.txt` | 添加新源文件 | 修改 |
| `tests/test_postprocess_registry.cpp` | 后处理注册表单元测试 | 新建 |
| `tests/CMakeLists.txt` | 添加新测试 | 修改 |
| `docs/产品化路线图.md` | 更新进度 | 修改 |

---

## 任务 1：修复下载脚本参数名 Bug

**文件：**
- 修改：`scripts/download_sample_model.ps1:2`
- 修改：`src/ui/main_window.cpp:210`

**问题：** C++ 代码传 `-ModelsDir`，PowerShell 脚本定义 `-OutputDir`，参数名不匹配导致脚本始终使用默认路径。

- [ ] **步骤 1：修改 PowerShell 脚本参数名**

将 `download_sample_model.ps1` 第 2 行的参数名从 `$OutputDir` 改为 `$ModelsDir`：

```powershell
param(
    [string]$ModelsDir = (Join-Path $PSScriptRoot "..\models"),
    [string]$ModelName = "yolov8n",
    [string]$ModelUrl = "https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n.onnx"
)
```

同时将脚本中所有 `$OutputDir` 引用替换为 `$ModelsDir`（约 5 处）。

- [ ] **步骤 2：验证参数名匹配**

确认 `main_window.cpp` 中传递的参数名 `-ModelsDir` 与脚本参数名一致。

- [ ] **步骤 3：Commit**

```bash
git add scripts/download_sample_model.ps1
git commit -m "fix: 修复下载脚本参数名不匹配（-OutputDir → -ModelsDir）"
```

---

## 任务 2：默认模型自动发现和加载

**文件：**
- 修改：`src/ui/app_controller.h`
- 修改：`src/ui/app_controller.cpp`
- 修改：`src/app/app_bootstrap.cpp`
- 修改：`src/ui/main_window.cpp`

**目标：** 首次启动时自动扫描 `models/` 目录下的清单文件，如果找到且对应 ONNX 文件存在则自动加载。非首次启动则加载上次使用的模型（已有逻辑）。

- [ ] **步骤 1：在 AppController 中添加 tryLoadDefaultModel 方法声明**

在 `app_controller.h` 的 public 区域添加：

```cpp
bool tryLoadDefaultModel();
```

- [ ] **步骤 2：实现 tryLoadDefaultModel**

在 `app_controller.cpp` 中实现：

```cpp
bool AppController::tryLoadDefaultModel() {
    const QString lastPath = settingsStore_.lastModelManifestPath();
    if (!lastPath.isEmpty() && QFileInfo::exists(lastPath)) {
        try {
            currentManifest_ = modelService_.loadManifest(lastPath);
            currentModel_.reset();
            currentManifestPath_ = currentManifest_.manifestPath;
            currentSummary_ = {};
            settingsStore_.addRecentModel(currentManifestPath_);
            emit modelLoaded(currentManifest_);
            emit contextChanged();
            return true;
        } catch (const std::exception&) {
        }
    }

    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList searchPaths = {
        appDir.filePath(QStringLiteral("../models")),
        appDir.filePath(QStringLiteral("../../models")),
        appDir.filePath(QStringLiteral("models"))
    };

    for (const QString& searchPath : searchPaths) {
        const QDir modelsDir(searchPath);
        if (!modelsDir.exists()) {
            continue;
        }

        const QStringList jsonFiles = modelsDir.entryList(
            {QStringLiteral("*.json")}, QDir::Files, QDir::Name);
        for (const QString& jsonFile : jsonFiles) {
            const QString manifestPath = modelsDir.absoluteFilePath(jsonFile);
            try {
                const core::ModelManifest manifest = modelService_.loadManifest(manifestPath);
                const QString onnxPath = modelsDir.absoluteFilePath(manifest.modelFile);
                if (!QFileInfo::exists(onnxPath)) {
                    continue;
                }
                currentManifest_ = manifest;
                currentModel_.reset();
                currentManifestPath_ = currentManifest_.manifestPath;
                currentSummary_ = {};
                settingsStore_.addRecentModel(currentManifestPath_);
                settingsStore_.setLastModelManifestPath(currentManifestPath_);
                emit modelLoaded(currentManifest_);
                emit contextChanged();
                return true;
            } catch (const std::exception&) {
            }
        }
    }

    return false;
}
```

需要在 `app_controller.cpp` 顶部添加 include：

```cpp
#include <QCoreApplication>
```

- [ ] **步骤 3：修改 MainWindow 构造函数使用 tryLoadDefaultModel**

将 `main_window.cpp` 中的自动加载逻辑从：

```cpp
const QString lastPath = controller_->settingsStore().lastModelManifestPath();
if (!lastPath.isEmpty() && QFileInfo::exists(lastPath)) {
    controller_->loadModelManifest(lastPath);
}
```

改为：

```cpp
controller_->tryLoadDefaultModel();
```

- [ ] **步骤 4：编译验证**

运行：`cmake --build --preset debug`
预期：编译通过，无错误

- [ ] **步骤 5：运行测试**

运行：`ctest --preset debug --output-on-failure`
预期：9/9 通过

- [ ] **步骤 6：Commit**

```bash
git add src/ui/app_controller.h src/ui/app_controller.cpp src/ui/main_window.cpp
git commit -m "feat: 默认模型自动发现和加载——首次启动自动扫描 models/ 目录"
```

---

## 任务 3：下载后自动加载模型

**文件：**
- 修改：`src/ui/main_window.cpp`

**目标：** 用户点击"下载示例模型"后，如果清单已存在则直接加载；如果需要下载，启动下载进程后轮询等待完成再自动加载。

- [ ] **步骤 1：重写下载示例模型的完整逻辑**

将 `main_window.cpp` 中 `downloadSampleModelClicked` 的 connect lambda 替换为：

```cpp
connect(homePage_, &HomePage::downloadSampleModelClicked, this, [this]() {
    const QString scriptPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../scripts/download_sample_model.ps1");
    const QString altScriptPath = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../scripts/download_sample_model.ps1"));

    QString resolvedScript;
    if (QFileInfo::exists(scriptPath)) {
        resolvedScript = scriptPath;
    } else if (QFileInfo::exists(altScriptPath)) {
        resolvedScript = altScriptPath;
    } else {
        QMessageBox::information(
            this,
            QStringLiteral("下载示例模型"),
            QStringLiteral("未找到下载脚本。请手动执行：\n\npowershell -ExecutionPolicy Bypass -File scripts/download_sample_model.ps1"));
        return;
    }

    const QString modelsDir = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../models"));
    const QString manifestPath = modelsDir + QStringLiteral("/yolov8n.json");
    const QString onnxPath = modelsDir + QStringLiteral("/yolov8n.onnx");

    if (QFileInfo::exists(manifestPath) && QFileInfo::exists(onnxPath)) {
        controller_->loadModelManifest(manifestPath);
        return;
    }

    QMessageBox::information(
        this,
        QStringLiteral("下载示例模型"),
        QStringLiteral("即将下载 YOLOv8n 模型（约 6MB），请稍候..."));

    auto* downloadProcess = new QProcess(this);
    connect(downloadProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, downloadProcess, manifestPath, onnxPath](int exitCode, QProcess::ExitStatus) {
        downloadProcess->deleteLater();

        if (QFileInfo::exists(manifestPath) && QFileInfo::exists(onnxPath)) {
            controller_->loadModelManifest(manifestPath);
            statusBar()->showMessage(QStringLiteral("示例模型下载完成并已加载"), 5000);
        } else {
            QMessageBox::warning(
                this,
                QStringLiteral("下载失败"),
                QStringLiteral("模型下载未完成。请检查网络连接后重试，或手动下载模型文件。"));
        }
    });

    downloadProcess->start(
        QStringLiteral("powershell"),
        {QStringLiteral("-ExecutionPolicy"), QStringLiteral("Bypass"),
         QStringLiteral("-File"), resolvedScript,
         QStringLiteral("-ModelsDir"), modelsDir});
});
```

- [ ] **步骤 2：编译验证**

运行：`cmake --build --preset debug`
预期：编译通过

- [ ] **步骤 3：运行测试**

运行：`ctest --preset debug --output-on-failure`
预期：9/9 通过

- [ ] **步骤 4：Commit**

```bash
git add src/ui/main_window.cpp
git commit -m "feat: 下载示例模型后自动加载——无需用户手动选择清单文件"
```

---

## 任务 4：首页"快速体验"引导按钮

**文件：**
- 修改：`src/ui/pages/home_page.h`
- 修改：`src/ui/pages/home_page.cpp`
- 修改：`src/ui/main_window.cpp`

**目标：** 当模型已加载但未选择图像时，首页显示"快速体验"按钮，点击后加载内置示例图像并自动执行推理。当模型未加载时，"下载示例模型"按钮更醒目。

- [ ] **步骤 1：在 HomePage 中添加 quickStartClicked 信号和状态感知**

在 `home_page.h` 中添加信号：

```cpp
signals:
    void loadModelClicked();
    void selectImageClicked();
    void downloadSampleModelClicked();
    void quickStartClicked();
```

在 `home_page.h` 的 private 区域添加：

```cpp
    QPushButton* quickStartBtn_ = nullptr;
```

- [ ] **步骤 2：在 HomePage 构造函数中添加"快速体验"按钮**

在 `home_page.cpp` 中，在 `downloadBtn` 之后添加：

```cpp
    quickStartBtn_ = new QPushButton(QStringLiteral("快速体验"), this);
    quickStartBtn_->setObjectName(QStringLiteral("PrimaryButton"));
    quickStartBtn_->setToolTip(QStringLiteral("使用示例图像执行一次目标检测"));
    quickStartBtn_->hide();

    actionsLayout->addWidget(loadModelBtn);
    actionsLayout->addWidget(selectImageBtn);
    actionsLayout->addWidget(downloadBtn);
    actionsLayout->addWidget(quickStartBtn_);
    actionsLayout->addStretch(1);
```

在 connect 区域添加：

```cpp
    connect(quickStartBtn_, &QPushButton::clicked, this, &HomePage::quickStartClicked);
```

添加公共方法声明（在 `home_page.h` 的 public 区域，`setRecentInputs` 之后）：

```cpp
    void setQuickStartVisible(bool visible);
```

在 `home_page.cpp` 中实现：

```cpp
void HomePage::setQuickStartVisible(const bool visible) {
    quickStartBtn_->setVisible(visible);
}
```

- [ ] **步骤 3：在 MainWindow 中连接快速体验逻辑**

在 `main_window.cpp` 中，在 `homePage_` 的信号连接区域添加：

```cpp
    connect(homePage_, &HomePage::quickStartClicked, this, [this]() {
        const QString sampleImagePath = QDir::cleanPath(
            QCoreApplication::applicationDirPath() + QStringLiteral("/../../tests/data/sample.jpg"));
        if (QFileInfo::exists(sampleImagePath)) {
            controller_->selectImage(sampleImagePath);
            controller_->runInference(-1.0, -1.0);
        } else {
            QMessageBox::information(
                this,
                QStringLiteral("快速体验"),
                QStringLiteral("示例图像未找到，请手动选择一张图像进行检测。"));
        }
    });
```

在 `modelLoaded` 信号处理中，当模型加载成功时显示快速体验按钮：

```cpp
    connect(controller_, &AppController::modelLoaded, this, [this]() {
        homePage_->setQuickStartVisible(true);
    });
```

- [ ] **步骤 4：创建示例测试图像**

在 `tests/data/` 目录下创建一个简单的测试图像（由测试代码生成），同时确保安装时复制到正确位置。但更实际的做法是让"快速体验"按钮打开文件选择对话框而非依赖固定路径。

修改步骤 3 的逻辑为更健壮的版本：

```cpp
    connect(homePage_, &HomePage::quickStartClicked, this, [this]() {
        const QString imagePath = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("选择图像"),
            QString(),
            QStringLiteral("图像 (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp)"));
        if (imagePath.isEmpty()) {
            return;
        }
        controller_->selectImage(imagePath);
        controller_->runInference(-1.0, -1.0);
    });
```

- [ ] **步骤 5：编译验证**

运行：`cmake --build --preset debug`
预期：编译通过

- [ ] **步骤 6：运行测试**

运行：`ctest --preset debug --output-on-failure`
预期：9/9 通过

- [ ] **步骤 7：Commit**

```bash
git add src/ui/pages/home_page.h src/ui/pages/home_page.cpp src/ui/main_window.cpp
git commit -m "feat: 首页新增'快速体验'按钮——模型加载后一键选择图像并推理"
```

---

## 任务 5：后处理策略注册表

**文件：**
- 新建：`src/models/postprocess_registry.h`
- 新建：`src/models/postprocess_registry.cpp`
- 修改：`src/models/yolo_detection_model.h`
- 修改：`src/models/yolo_detection_model.cpp`
- 修改：`src/core/model_manifest.h`（确保 decoder 字段被解析）
- 修改：`src/CMakeLists.txt`
- 新建：`tests/test_postprocess_registry.cpp`
- 修改：`tests/CMakeLists.txt`

**目标：** 将 YOLO 后处理逻辑从 `YoloDetectionModel::detect()` 中提取为可注册的策略，根据清单的 `decoder` 字段分发。当前仅支持 `"yolo_v8"` 解码器，但架构上为 `"yolo_v5"` / `"yolo_x"` 等变体预留扩展点。

- [ ] **步骤 1：确认 ModelManifest 中 decoder 字段**

读取 `model_manifest.h`，确认 `decoder` 字段已存在且被正确解析。如果不存在，添加：

```cpp
QString decoder;
```

在 `model_manifest.cpp` 的解析逻辑中添加：

```cpp
manifest.decoder = optionalStringField(root, "decoder", "yolo_v8");
```

- [ ] **步骤 2：定义后处理策略接口**

创建 `src/models/postprocess_registry.h`：

```cpp
#pragma once

#include <QSize>
#include <QVector>

#include <opencv2/core.hpp>

#include <functional>
#include <memory>
#include <string>

#include "core/model_manifest.h"
#include "core/types.h"
#include "runtime/inference_backend.h"

namespace aitoolkit::models {

struct PostprocessInput {
    std::vector<runtime::InferenceTensor> tensors;
    QSize networkSize;
    QSize originalSize;
    double confidenceThreshold;
    double nmsThreshold;
    const core::ModelManifest& manifest;
};

using PostprocessFn = std::function<QVector<core::DetectionItem>(const PostprocessInput&)>;

class PostprocessRegistry {
public:
    static PostprocessRegistry& instance();

    void registerDecoder(const std::string& decoderName, PostprocessFn fn);
    PostprocessFn getDecoder(const std::string& decoderName) const;
    bool hasDecoder(const std::string& decoderName) const;

private:
    PostprocessRegistry() = default;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace aitoolkit::models
```

- [ ] **步骤 3：实现后处理策略注册表**

创建 `src/models/postprocess_registry.cpp`：

```cpp
#include "models/postprocess_registry.h"

#include <map>
#include <stdexcept>

namespace aitoolkit::models {

struct PostprocessRegistry::Impl {
    std::map<std::string, PostprocessFn> decoders;
};

PostprocessRegistry& PostprocessRegistry::instance() {
    static PostprocessRegistry registry;
    if (!registry.impl_) {
        registry.impl_ = std::make_unique<Impl>();
    }
    return registry;
}

void PostprocessRegistry::registerDecoder(const std::string& decoderName, PostprocessFn fn) {
    impl_->decoders[decoderName] = std::move(fn);
}

PostprocessFn PostprocessRegistry::getDecoder(const std::string& decoderName) const {
    const auto it = impl_->decoders.find(decoderName);
    if (it == impl_->decoders.end()) {
        return nullptr;
    }
    return it->second;
}

bool PostprocessRegistry::hasDecoder(const std::string& decoderName) const {
    return impl_->decoders.find(decoderName) != impl_->decoders.end();
}

}  // namespace aitoolkit::models
```

- [ ] **步骤 4：注册 YOLOv8 后处理策略并重构 YoloDetectionModel**

修改 `yolo_detection_model.cpp`，在文件作用域注册 YOLOv8 解码器：

```cpp
namespace {

const bool yoloV8DecoderRegistered = []{
    PostprocessRegistry::instance().registerDecoder("yolo_v8",
        [](const PostprocessInput& input) -> QVector<core::DetectionItem> {
            if (input.tensors.empty()) {
                return {};
            }
            const cv::Mat output = YoloDetectionModel::tensorToDetectionMatrix(
                input.tensors.front());
            return YoloDetectionModel::postprocessDetections(
                output, input.networkSize, input.manifest,
                input.originalSize, input.confidenceThreshold, input.nmsThreshold);
        });
    return true;
}();

}
```

修改 `YoloDetectionModel::detect()` 方法使用注册表：

```cpp
QVector<core::DetectionItem> YoloDetectionModel::detect(
    const cv::Mat& image,
    const double confidenceThreshold,
    const double nmsThreshold) const {
    const YoloPreprocessResult prepared = preprocessImage(image, manifest_);
    const std::vector<int64_t> inputShape = backend_.inputShape();
    const std::vector<runtime::InferenceTensor> outputs = backend_.run(prepared.blob, inputShape);

    const std::string decoderName = manifest_.decoder.toStdString();
    auto decoder = PostprocessRegistry::instance().getDecoder(decoderName);
    if (!decoder) {
        decoder = PostprocessRegistry::instance().getDecoder("yolo_v8");
    }

    if (!decoder) {
        throw std::runtime_error("No postprocess decoder registered for: " + decoderName);
    }

    PostprocessInput ppInput{
        std::move(outputs),
        prepared.networkSize,
        prepared.originalSize,
        confidenceThreshold < 0 ? manifest_.confidenceThreshold : confidenceThreshold,
        nmsThreshold < 0 ? manifest_.nmsThreshold : nmsThreshold,
        manifest_
    };

    return decoder(ppInput);
}
```

- [ ] **步骤 5：添加 CMakeLists.txt 中的源文件**

在 `src/CMakeLists.txt` 的 `ai_collect_existing_sources` 中添加：

```
    models/postprocess_registry.cpp
```

- [ ] **步骤 6：编写后处理注册表单元测试**

创建 `tests/test_postprocess_registry.cpp`：

```cpp
#include <QtTest>

#include "models/postprocess_registry.h"

using aitoolkit::models::PostprocessRegistry;
using aitoolkit::models::PostprocessInput;
using aitoolkit::models::PostprocessFn;
using aitoolkit::core::DetectionItem;

class TestPostprocessRegistry : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        registry_ = &PostprocessRegistry::instance();
    }

    void testRegisterAndGetDecoder() {
        PostprocessFn fn = [](const PostprocessInput&) -> QVector<DetectionItem> {
            return {DetectionItem{0, QStringLiteral("test"), 0.9f, QRectF(0, 0, 100, 100), QColor(Qt::red)}};
        };
        registry_->registerDecoder("test_decoder", fn);

        QVERIFY(registry_->hasDecoder("test_decoder"));
        auto retrieved = registry_->getDecoder("test_decoder");
        QVERIFY(retrieved != nullptr);
    }

    void testGetNonexistentDecoder() {
        QVERIFY(!registry_->hasDecoder("nonexistent"));
        QVERIFY(registry_->getDecoder("nonexistent") == nullptr);
    }

    void testYoloV8DecoderIsRegistered() {
        QVERIFY(registry_->hasDecoder("yolo_v8"));
        QVERIFY(registry_->getDecoder("yolo_v8") != nullptr);
    }

private:
    PostprocessRegistry* registry_ = nullptr;
};

QTEST_MAIN(TestPostprocessRegistry)
#include "test_postprocess_registry.moc"
```

- [ ] **步骤 7：添加测试到 CMakeLists.txt**

在 `tests/CMakeLists.txt` 中添加新的测试目标。

- [ ] **步骤 8：编译验证**

运行：`cmake --build --preset debug`
预期：编译通过

- [ ] **步骤 9：运行测试**

运行：`ctest --preset debug --output-on-failure`
预期：所有测试通过（包括新增的 test_postprocess_registry）

- [ ] **步骤 10：Commit**

```bash
git add src/models/postprocess_registry.h src/models/postprocess_registry.cpp src/models/yolo_detection_model.h src/models/yolo_detection_model.cpp src/core/model_manifest.h src/core/model_manifest.cpp src/CMakeLists.txt tests/test_postprocess_registry.cpp tests/CMakeLists.txt
git commit -m "feat: 后处理策略注册表——根据 decoder 字段分发后处理逻辑，为多模型变体扩展铺路"
```

---

## 任务 6：推理预热

**文件：**
- 修改：`src/runtime/onnx_backend.h`
- 修改：`src/runtime/onnx_backend.cpp`
- 修改：`src/models/yolo_detection_model.cpp`

**目标：** 首次推理前用零张量做一次 dry run，消除 ONNX Runtime 首次推理的延迟（通常 100-500ms）。

- [ ] **步骤 1：在 OnnxBackend 中添加 warmup 方法声明**

在 `onnx_backend.h` 的 public 区域添加：

```cpp
    void warmup();
```

- [ ] **步骤 2：实现 warmup 方法**

在 `onnx_backend.cpp` 中添加：

```cpp
void OnnxBackend::warmup() {
    if (!session_ || inputShape_.empty()) {
        return;
    }

    try {
        const std::size_t elementCount = elementCount(inputShape_);
        const std::vector<float> zeros(elementCount, 0.0f);
        (void)run(zeros, inputShape_);
    } catch (const std::exception&) {
    }
}
```

- [ ] **步骤 3：在 YoloDetectionModel 构造函数中调用 warmup**

在 `yolo_detection_model.cpp` 的构造函数末尾添加：

```cpp
YoloDetectionModel::YoloDetectionModel(core::ModelManifest manifest)
    : manifest_(std::move(manifest))
    , backend_(resolveOnnxPath(manifest_)) {
    backend_.warmup();
}
```

- [ ] **步骤 4：编译验证**

运行：`cmake --build --preset debug`
预期：编译通过

- [ ] **步骤 5：运行测试**

运行：`ctest --preset debug --output-on-failure`
预期：所有测试通过

- [ ] **步骤 6：Commit**

```bash
git add src/runtime/onnx_backend.h src/runtime/onnx_backend.cpp src/models/yolo_detection_model.cpp
git commit -m "feat: 推理预热——首次推理前用零张量做 dry run 消除延迟"
```

---

## 任务 7：后端能力查询

**文件：**
- 修改：`src/runtime/onnx_backend.h`
- 修改：`src/runtime/onnx_backend.cpp`
- 修改：`src/models/inference_backend.h`

**目标：** 为 `InferenceBackend` 接口添加能力查询方法，UI 层可据此展示可用选项（如 GPU 加速状态）。

- [ ] **步骤 1：在 InferenceBackend 接口中添加能力查询虚方法**

在 `models/inference_backend.h` 中添加：

```cpp
    virtual bool supportsGPU() const { return false; }
    virtual QString backendName() const = 0;
```

- [ ] **步骤 2：在 YoloDetectionModel 中实现能力查询**

在 `yolo_detection_model.h` 的 public 区域添加：

```cpp
    QString backendName() const noexcept override;
```

在 `yolo_detection_model.cpp` 中实现：

```cpp
QString YoloDetectionModel::backendName() const noexcept {
    return QStringLiteral("ONNX Runtime");
}
```

- [ ] **步骤 3：编译验证**

运行：`cmake --build --preset debug`
预期：编译通过

- [ ] **步骤 4：运行测试**

运行：`ctest --preset debug --output-on-failure`
预期：所有测试通过

- [ ] **步骤 5：Commit**

```bash
git add src/models/inference_backend.h src/models/yolo_detection_model.h src/models/yolo_detection_model.cpp
git commit -m "feat: 后端能力查询——InferenceBackend 新增 supportsGPU() 和 backendName()"
```

---

## 任务 8：更新产品化路线图

**文件：**
- 修改：`docs/产品化路线图.md`

- [ ] **步骤 1：更新路线图文档**

在第四阶段和第五阶段之间新增"产品体验优化"阶段，记录任务 1-4 的改进；在架构改进部分记录任务 5-7 的进展。

- [ ] **步骤 2：Commit**

```bash
git add docs/产品化路线图.md
git commit -m "docs: 更新产品化路线图——新增产品体验优化阶段"
```

---

## 自检

### 1. 规格覆盖度

| 需求 | 对应任务 |
|------|---------|
| 下载脚本参数名 Bug | 任务 1 ✅ |
| 默认模型自动加载 | 任务 2 ✅ |
| 下载后自动加载 | 任务 3 ✅ |
| 首次引导/快速体验 | 任务 4 ✅ |
| 后处理策略注册表 | 任务 5 ✅ |
| 推理预热 | 任务 6 ✅ |
| 后端能力查询 | 任务 7 ✅ |
| 路线图更新 | 任务 8 ✅ |

### 2. 占位符扫描

- 无 "TODO" / "待定" / "后续实现" 等占位符
- 所有步骤包含完整代码
- 所有命令和预期输出明确

### 3. 类型一致性

- `PostprocessInput` 结构体在任务 5 中定义，在注册 lambda 和 `detect()` 方法中使用一致
- `PostprocessFn` 类型在注册表和注册 lambda 中使用一致
- `InferenceBackend::supportsGPU()` 和 `backendName()` 在任务 7 中定义，在 `YoloDetectionModel` 中实现一致
- `OnnxBackend::warmup()` 在任务 6 中定义，在 `YoloDetectionModel` 构造函数中调用一致
- `HomePage::quickStartClicked` 信号在任务 4 中定义，在 `MainWindow` 中连接一致
- `AppController::tryLoadDefaultModel()` 在任务 2 中定义，在 `MainWindow` 中调用一致
