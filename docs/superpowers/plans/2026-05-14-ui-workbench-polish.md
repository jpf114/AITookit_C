# UI Workbench Polish Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将当前可运行的单图检测桌面原型收口成一个更接近 `GIS_TOOL` 气质的工作台式界面，重点提升整体工具感、推理流程顺手程度和结果复核体验。

**Architecture:** 保持现有三栏外壳与 `app / ui / core / services` 分层不变，把改动集中在 UI 层。模型页、推理页、结果页重做内部结构，右侧上下文栏标准化为稳定任务摘要，样式统一收敛到更清晰的桌面工具语言。测试继续使用现有 QtTest 页面锚点，必要时增补新的 UI 锚点但不删现有覆盖。

**Tech Stack:** C++17, Qt Widgets, QtTest, CMake

---

## File Structure

本轮计划预期修改以下文件：

- Modify: `src/ui/main_window.h`
- Modify: `src/ui/main_window.cpp`
- Modify: `src/ui/pages/models_page.h`
- Modify: `src/ui/pages/models_page.cpp`
- Modify: `src/ui/pages/inference_page.h`
- Modify: `src/ui/pages/inference_page.cpp`
- Modify: `src/ui/pages/results_page.h`
- Modify: `src/ui/pages/results_page.cpp`
- Modify: `resources/themes/app.qss`
- Modify: `tests/test_main_window.cpp`
- Modify: `tests/test_models_page.cpp`

这些文件的职责约束如下：

- `main_window.*`：三栏外壳、右侧上下文栏内容编排、页面切换后状态联动
- `models_page.*`：模型页工作台化布局与摘要信息分组
- `inference_page.*`：图像预览与推理操作区的双栏工作面
- `results_page.*`：结果页摘要条、主预览区、明细表重排
- `app.qss`：统一工具型风格、页面分组、右栏状态块和结果摘要条样式
- `tests/*`：校验布局重排后核心交互与关键信息锚点仍然存在

### Task 1: 为页面重排补测试锚点与失败用例

**Files:**
- Modify: `tests/test_main_window.cpp`
- Modify: `tests/test_models_page.cpp`

- [ ] **Step 1: 给主窗口测试增加右侧上下文栏结构断言**

```cpp
void MainWindowTest::buildsThreePaneShell() {
    aitoolkit::ui::MainWindow window;

    QVERIFY(window.centralWidget() != nullptr);

    const auto stacks = window.findChildren<QStackedWidget*>();
    QCOMPARE(stacks.size(), 1);
    QCOMPARE(stacks.front()->count(), 5);

    const auto navPanel = window.findChild<QWidget*>(QStringLiteral("NavPanel"));
    QVERIFY(navPanel != nullptr);

    const auto contextPanel = window.findChild<QWidget*>(QStringLiteral("ContextPanel"));
    QVERIFY(contextPanel != nullptr);

    QVERIFY(window.findChild<QLabel*>(QStringLiteral("ContextModelTitle")) != nullptr);
    QVERIFY(window.findChild<QLabel*>(QStringLiteral("ContextImageTitle")) != nullptr);
    QVERIFY(window.findChild<QLabel*>(QStringLiteral("ContextResultTitle")) != nullptr);
    QVERIFY(window.findChild<QLabel*>(QStringLiteral("ContextNextStepTitle")) != nullptr);
}
```

- [ ] **Step 2: 运行主窗口测试，确认新断言先失败**

Run: `ctest -C Debug --output-on-failure -R "test_main_window"`

Expected: FAIL，提示找不到 `ContextModelTitle`、`ContextImageTitle`、`ContextResultTitle` 或 `ContextNextStepTitle`

- [ ] **Step 3: 给模型页测试增加工作台分组锚点断言**

```cpp
void ModelsPageTest::showsManifestSummary() {
    aitoolkit::ui::ModelsPage page;

    aitoolkit::core::ModelManifest manifest;
    manifest.manifestPath = QStringLiteral("D:/models/yolo/model.json");
    manifest.name = QStringLiteral("Warehouse Detector");
    manifest.taskType = QStringLiteral("detection");
    manifest.backendType = QStringLiteral("onnxruntime");
    manifest.modelPath = QStringLiteral("D:/models/yolo/model.onnx");
    manifest.inputWidth = 640;
    manifest.inputHeight = 640;
    manifest.labels = {QStringLiteral("person"), QStringLiteral("box"), QStringLiteral("forklift")};

    page.setCurrentManifest(manifest);

    QVERIFY(page.findChild<QWidget*>(QStringLiteral("ModelLoadSection")) != nullptr);
    QVERIFY(page.findChild<QWidget*>(QStringLiteral("ModelSummarySection")) != nullptr);

    auto* pathLabel = page.findChild<QLabel*>(QStringLiteral("ManifestPathLabel"));
    QVERIFY(pathLabel != nullptr);
    QVERIFY(pathLabel->text().contains(manifest.manifestPath));

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ManifestSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(summaryLabel->text().contains(QStringLiteral("Warehouse Detector")));
    QVERIFY(summaryLabel->text().contains(QStringLiteral("onnxruntime")));
    QVERIFY(summaryLabel->text().contains(QStringLiteral("640 x 640")));
    QVERIFY(summaryLabel->text().contains(QStringLiteral("3")));
}
```

- [ ] **Step 4: 运行模型页测试，确认新断言先失败**

Run: `ctest -C Debug --output-on-failure -R "test_models_page"`

Expected: FAIL，提示找不到 `ModelLoadSection` 或 `ModelSummarySection`

- [ ] **Step 5: 提交失败测试**

```bash
git add tests/test_main_window.cpp tests/test_models_page.cpp
git commit -m "test: cover workbench context and models page sections"
```

### Task 2: 重构主窗口右侧上下文栏

**Files:**
- Modify: `src/ui/main_window.h`
- Modify: `src/ui/main_window.cpp`

- [ ] **Step 1: 在主窗口头文件中补充右栏标题与下一步标签成员**

```cpp
private:
    QLabel* modelStatusTitleLabel_ = nullptr;
    QLabel* modelStatusLabel_ = nullptr;
    QLabel* imageStatusTitleLabel_ = nullptr;
    QLabel* imageStatusLabel_ = nullptr;
    QLabel* resultStatusTitleLabel_ = nullptr;
    QLabel* runStatusLabel_ = nullptr;
    QLabel* nextStepTitleLabel_ = nullptr;
    QLabel* nextStepLabel_ = nullptr;
```

- [ ] **Step 2: 在 `buildShell()` 中把右栏改成四段稳定结构**

```cpp
    auto* title = new QLabel(QStringLiteral("当前会话"), contextPanel_);
    title->setObjectName(QStringLiteral("ContextTitle"));

    modelStatusTitleLabel_ = new QLabel(QStringLiteral("当前模型"), contextPanel_);
    modelStatusTitleLabel_->setObjectName(QStringLiteral("ContextModelTitle"));
    modelStatusLabel_ = new QLabel(contextPanel_);
    modelStatusLabel_->setObjectName(QStringLiteral("ContextModelValue"));
    modelStatusLabel_->setWordWrap(true);

    imageStatusTitleLabel_ = new QLabel(QStringLiteral("当前图像"), contextPanel_);
    imageStatusTitleLabel_->setObjectName(QStringLiteral("ContextImageTitle"));
    imageStatusLabel_ = new QLabel(contextPanel_);
    imageStatusLabel_->setObjectName(QStringLiteral("ContextImageValue"));
    imageStatusLabel_->setWordWrap(true);

    resultStatusTitleLabel_ = new QLabel(QStringLiteral("当前结果"), contextPanel_);
    resultStatusTitleLabel_->setObjectName(QStringLiteral("ContextResultTitle"));
    runStatusLabel_ = new QLabel(contextPanel_);
    runStatusLabel_->setObjectName(QStringLiteral("ContextResultValue"));
    runStatusLabel_->setWordWrap(true);

    nextStepTitleLabel_ = new QLabel(QStringLiteral("下一步"), contextPanel_);
    nextStepTitleLabel_->setObjectName(QStringLiteral("ContextNextStepTitle"));
    nextStepLabel_ = new QLabel(contextPanel_);
    nextStepLabel_->setObjectName(QStringLiteral("ContextNextStepValue"));
    nextStepLabel_->setWordWrap(true);
```

- [ ] **Step 3: 在 `updateContextPanel()` 中统一生成任务状态文案**

```cpp
void MainWindow::updateContextPanel() {
    modelStatusLabel_->setText(
        currentManifestPath_.isEmpty()
            ? QStringLiteral("未选择模型清单")
            : QStringLiteral("已加载：%1").arg(currentManifest_.name.isEmpty() ? currentManifestPath_ : currentManifest_.name));

    imageStatusLabel_->setText(
        currentImagePath_.isEmpty()
            ? QStringLiteral("未选择图像")
            : QStringLiteral("已选择：%1").arg(currentImagePath_));

    runStatusLabel_->setText(
        currentSummary_.inputPath.isEmpty()
            ? QStringLiteral("尚未执行检测")
            : QStringLiteral("已完成，共 %1 个目标，耗时 %2 ms")
                  .arg(currentSummary_.detectionCount)
                  .arg(QString::number(currentSummary_.elapsedMs, 'f', 2)));

    if (currentManifestPath_.isEmpty()) {
        nextStepLabel_->setText(QStringLiteral("请先加载模型清单。"));
    } else if (currentImagePath_.isEmpty()) {
        nextStepLabel_->setText(QStringLiteral("请选择一张待推理图像。"));
    } else if (currentSummary_.inputPath.isEmpty()) {
        nextStepLabel_->setText(QStringLiteral("模型和图像已就绪，可以开始检测。"));
    } else {
        nextStepLabel_->setText(QStringLiteral("可查看结果明细，或直接导出 JSON。"));
    }
}
```

- [ ] **Step 4: 运行主窗口测试，确认右栏结构与原有交互都通过**

Run: `ctest -C Debug --output-on-failure -R "test_main_window"`

Expected: PASS

- [ ] **Step 5: 提交主窗口上下文栏重构**

```bash
git add src/ui/main_window.h src/ui/main_window.cpp
git commit -m "feat: standardize context rail as task summary"
```

### Task 3: 重构模型页为准备区工作台

**Files:**
- Modify: `src/ui/pages/models_page.h`
- Modify: `src/ui/pages/models_page.cpp`
- Modify: `tests/test_models_page.cpp`

- [ ] **Step 1: 在模型页头文件中补充分组控件成员**

```cpp
private:
    QLabel* manifestPathLabel_ = nullptr;
    QLabel* manifestSummaryLabel_ = nullptr;
    QWidget* loadSection_ = nullptr;
    QWidget* summarySection_ = nullptr;
```

- [ ] **Step 2: 将模型页布局改成页头 + 加载区 + 摘要区**

```cpp
    auto* headerDesc = new QLabel(QStringLiteral("先加载一个模型清单，再确认模型信息是否符合当前任务。"), this);
    headerDesc->setObjectName(QStringLiteral("PageLead"));
    headerDesc->setWordWrap(true);

    loadSection_ = new QWidget(this);
    loadSection_->setObjectName(QStringLiteral("ModelLoadSection"));
    auto* loadLayout = new QVBoxLayout(loadSection_);
    loadLayout->setContentsMargins(16, 16, 16, 16);
    loadLayout->setSpacing(10);
    loadLayout->addWidget(loadButton);
    loadLayout->addWidget(manifestPathLabel_);

    auto* loadHintLabel = new QLabel(QStringLiteral("支持选择 JSON 模型清单文件。"), loadSection_);
    loadHintLabel->setObjectName(QStringLiteral("SectionHint"));
    loadHintLabel->setWordWrap(true);
    loadLayout->addWidget(loadHintLabel);

    summarySection_ = new QWidget(this);
    summarySection_->setObjectName(QStringLiteral("ModelSummarySection"));
    auto* summaryLayout = new QVBoxLayout(summarySection_);
    summaryLayout->setContentsMargins(16, 16, 16, 16);
    summaryLayout->setSpacing(10);
    summaryLayout->addWidget(manifestSummaryLabel_);

    layout->addWidget(title);
    layout->addWidget(headerDesc);
    layout->addWidget(loadSection_);
    layout->addWidget(summarySection_);
    layout->addStretch(1);
```

- [ ] **Step 3: 保留测试锚点并清理摘要文案**

```cpp
QString manifestSummaryText(const core::ModelManifest& manifest) {
    if (manifest.manifestPath.isEmpty()) {
        return QStringLiteral("加载模型清单后，可在这里查看模型名称、输入尺寸、推理后端和标签数量。");
    }

    return QStringLiteral(
               "模型名称：%1\n任务类型：%2\n推理后端：%3\n输入尺寸：%4 x %5\n标签数量：%6\n模型文件：%7")
        .arg(manifest.name)
        .arg(manifest.taskType)
        .arg(manifest.backendType)
        .arg(manifest.inputWidth)
        .arg(manifest.inputHeight)
        .arg(manifest.labels.size())
        .arg(manifest.modelPath);
}
```

- [ ] **Step 4: 运行模型页测试，确认新结构与原有摘要断言都通过**

Run: `ctest -C Debug --output-on-failure -R "test_models_page"`

Expected: PASS

- [ ] **Step 5: 提交模型页工作台化改动**

```bash
git add src/ui/pages/models_page.h src/ui/pages/models_page.cpp tests/test_models_page.cpp
git commit -m "feat: restructure models page as preparation workbench"
```

### Task 4: 重构推理页为预览加操作的中轴工作面

**Files:**
- Modify: `src/ui/pages/inference_page.h`
- Modify: `src/ui/pages/inference_page.cpp`

- [ ] **Step 1: 在推理页头文件中增加状态说明与图像预览成员**

```cpp
class ImagePreviewWidget;

private:
    QLabel* imagePathLabel_ = nullptr;
    QLabel* readinessLabel_ = nullptr;
    QPushButton* runButton_ = nullptr;
    ImagePreviewWidget* previewWidget_ = nullptr;
    bool modelReady_ = false;
```

- [ ] **Step 2: 把推理页改成左预览右操作布局**

```cpp
    auto* shellLayout = new QVBoxLayout(this);
    shellLayout->setContentsMargins(24, 24, 24, 24);
    shellLayout->setSpacing(12);

    auto* desc = new QLabel(QStringLiteral("选择一张图像，在预览旁完成检测操作。"), this);
    desc->setObjectName(QStringLiteral("PageLead"));
    desc->setWordWrap(true);

    auto* workRow = new QHBoxLayout();
    workRow->setSpacing(16);

    previewWidget_ = new ImagePreviewWidget(this);
    previewWidget_->setMinimumSize(560, 360);

    auto* actionPanel = new QWidget(this);
    actionPanel->setObjectName(QStringLiteral("InferenceActionPanel"));
    auto* actionLayout = new QVBoxLayout(actionPanel);
    actionLayout->setContentsMargins(16, 16, 16, 16);
    actionLayout->setSpacing(10);
    actionLayout->addWidget(openButton);
    actionLayout->addWidget(runButton_);
    actionLayout->addWidget(imagePathLabel_);

    readinessLabel_ = new QLabel(QStringLiteral("请先加载模型清单。"), actionPanel);
    readinessLabel_->setObjectName(QStringLiteral("InferenceReadinessLabel"));
    readinessLabel_->setWordWrap(true);
    actionLayout->addWidget(readinessLabel_);
    actionLayout->addStretch(1);

    workRow->addWidget(previewWidget_, 1);
    workRow->addWidget(actionPanel, 0);

    shellLayout->addWidget(title);
    shellLayout->addWidget(desc);
    shellLayout->addLayout(workRow, 1);
```

- [ ] **Step 3: 在 `setCurrentImagePath()` 与 `setModelReady()` 中同步更新预览和就绪文案**

```cpp
void InferencePage::setCurrentImagePath(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        imagePathLabel_->setText(QStringLiteral("当前未选择图像"));
        previewWidget_->setImage(QImage());
    } else {
        imagePathLabel_->setText(QStringLiteral("当前图像：%1").arg(imagePath));
        previewWidget_->setImage(QImage(imagePath));
    }

    if (!modelReady_) {
        readinessLabel_->setText(QStringLiteral("请先加载模型清单。"));
    } else if (imagePath.isEmpty()) {
        readinessLabel_->setText(QStringLiteral("模型已就绪，请选择一张待推理图像。"));
    } else {
        readinessLabel_->setText(QStringLiteral("模型和图像已就绪，可以开始检测。"));
    }
}

void InferencePage::setModelReady(const bool ready) {
    modelReady_ = ready;
    runButton_->setEnabled(ready);
    if (!ready) {
        readinessLabel_->setText(QStringLiteral("请先加载模型清单。"));
    } else if (imagePathLabel_->text().contains(QStringLiteral("当前图像："))) {
        readinessLabel_->setText(QStringLiteral("模型和图像已就绪，可以开始检测。"));
    } else {
        readinessLabel_->setText(QStringLiteral("模型已就绪，请选择一张待推理图像。"));
    }
}
```

- [ ] **Step 4: 构建主窗口测试目标，确认推理页结构不会破坏最近图像回填**

Run: `cmake --build build --config Debug --target test_main_window`

Expected: target builds successfully

- [ ] **Step 5: 提交推理页中轴工作面改动**

```bash
git add src/ui/pages/inference_page.h src/ui/pages/inference_page.cpp
git commit -m "feat: redesign inference page as preview and action workbench"
```

### Task 5: 重构结果页并统一样式

**Files:**
- Modify: `src/ui/pages/results_page.h`
- Modify: `src/ui/pages/results_page.cpp`
- Modify: `resources/themes/app.qss`

- [ ] **Step 1: 在结果页头文件中增加摘要区容器成员**

```cpp
private:
    QLabel* summaryLabel_ = nullptr;
    QWidget* summaryStrip_ = nullptr;
    ImagePreviewWidget* previewWidget_ = nullptr;
    QTableWidget* detectionsTable_ = nullptr;
```

- [ ] **Step 2: 把结果页改成摘要条 + 预览区 + 明细表**

```cpp
    summaryStrip_ = new QWidget(this);
    summaryStrip_->setObjectName(QStringLiteral("ResultsSummaryStrip"));
    auto* stripLayout = new QHBoxLayout(summaryStrip_);
    stripLayout->setContentsMargins(16, 12, 16, 12);
    stripLayout->setSpacing(12);

    summaryLabel_ = new QLabel(QStringLiteral("当前还没有推理结果"), summaryStrip_);
    summaryLabel_->setObjectName(QStringLiteral("ResultsSummaryLabel"));
    summaryLabel_->setWordWrap(true);
    stripLayout->addWidget(summaryLabel_, 1);
    stripLayout->addWidget(exportButton, 0);

    layout->addWidget(title);
    layout->addWidget(summaryStrip_);
    layout->addWidget(previewWidget_, 1);
    layout->addWidget(detectionsTable_);
```

- [ ] **Step 3: 在样式表中补页面分组、摘要条和右栏块样式**

```css
QWidget#ModelLoadSection,
QWidget#ModelSummarySection,
QWidget#InferenceActionPanel,
QWidget#ResultsSummaryStrip,
QWidget#ContextCard {
    background-color: #ffffff;
    border: 1px solid #e2e5ea;
    border-radius: 8px;
}

QLabel#PageLead,
QLabel#SectionHint,
QLabel#ContextNextStepValue {
    color: #4f637a;
    font-size: 12px;
}

QLabel#ContextModelTitle,
QLabel#ContextImageTitle,
QLabel#ContextResultTitle,
QLabel#ContextNextStepTitle {
    color: #7e92a8;
    font-size: 11px;
    font-weight: 700;
}
```

- [ ] **Step 4: 运行主窗口、模型页、设置页测试，确认收口后回归通过**

Run: `ctest -C Debug --output-on-failure -R "test_main_window|test_models_page|test_settings_page"`

Expected: all selected tests PASS

- [ ] **Step 5: 提交结果页与样式收口**

```bash
git add src/ui/pages/results_page.h src/ui/pages/results_page.cpp resources/themes/app.qss
git commit -m "feat: polish results page and workbench theme"
```

## Self-Review

### Spec coverage

- 三栏职责拉直：Task 2 覆盖
- 模型页工作台化：Task 3 覆盖
- 推理页左预览右操作：Task 4 覆盖
- 结果页摘要条与复核顺序：Task 5 覆盖
- 右侧上下文栏稳定结构：Task 2 覆盖
- 中文文案与工具型语气：Task 2、Task 3、Task 4、Task 5 覆盖
- 保留现有回归验证并按需扩展：Task 1、Task 5 覆盖

### Placeholder scan

本计划未使用 `TODO`、`TBD`、`稍后处理`、`类似 Task X` 之类占位写法。每个任务都给出了明确文件、代码片段、运行命令和预期结果。

### Type consistency

- 右栏新增标签成员统一使用 `QLabel*`
- 页面锚点名称在测试与实现之间保持一致：`ContextModelTitle`、`ModelLoadSection`、`InferenceActionPanel`、`ResultsSummaryStrip`
- 推理页新增状态标签统一使用 `InferenceReadinessLabel`

