#include "ui/main_window.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QUrl>
#include <QVBoxLayout>

#include "ui/app_controller.h"
#include "ui/nav_panel.h"
#include "ui/image_utils.h"
#include "ui/dialogs/onnx_setup_dialog.h"
#include "ui/dialogs/model_catalog_dialog.h"
#include "ui/pages/home_page.h"
#include "ui/pages/inference_page.h"
#include "ui/pages/models_page.h"
#include "ui/pages/results_page.h"
#include "ui/pages/settings_page.h"
#include "services/unicode_io.h"

namespace aitoolkit::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("AI 检测工具 v%1").arg(QCoreApplication::applicationVersion()));
    resize(1440, 900);
    setAcceptDrops(true);

    controller_ = new AppController(this);

    const QByteArray savedGeometry = controller_->settingsStore().windowGeometry();
    if (!savedGeometry.isEmpty()) {
        restoreGeometry(savedGeometry);
    }

    buildShell();
    wireSignals();
    setupShortcuts();
    refreshSettingsPage();
    updateContextPanel();

    controller_->tryLoadDefaultModel();
}

MainWindow::~MainWindow() {
    controller_->settingsStore().setWindowGeometry(saveGeometry());
}

void MainWindow::buildShell() {
    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);

    navPanel_ = new NavPanel(central);

    pageStack_ = new QStackedWidget(central);
    homePage_ = new HomePage(pageStack_);
    modelsPage_ = new ModelsPage(pageStack_);
    inferencePage_ = new InferencePage(pageStack_);
    resultsPage_ = new ResultsPage(pageStack_);
    settingsPage_ = new SettingsPage(pageStack_);
    pageStack_->addWidget(homePage_);
    pageStack_->addWidget(modelsPage_);
    pageStack_->addWidget(inferencePage_);
    pageStack_->addWidget(resultsPage_);
    pageStack_->addWidget(settingsPage_);

    contextPanel_ = new QFrame(central);
    contextPanel_->setObjectName(QStringLiteral("ContextPanel"));
    auto* contextLayout = new QVBoxLayout(contextPanel_);
    contextLayout->setContentsMargins(16, 16, 16, 16);
    contextLayout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("任务摘要"), contextPanel_);
    title->setObjectName(QStringLiteral("ContextTitle"));
    title->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));

    const auto configureTitleLabel = [](QLabel* label, const QString& objectName, const QString& text) {
        label->setObjectName(objectName);
        label->setText(text);
        label->setStyleSheet(QStringLiteral("font-size: 13px; font-weight: 600; color: #334155;"));
    };
    const auto configureValueLabel = [](QLabel* label, const QString& objectName) {
        label->setObjectName(objectName);
        label->setWordWrap(true);
        label->setStyleSheet(QStringLiteral("font-size: 13px; color: #0f172a;"));
    };
    const auto buildDivider = [contextPanel_ = contextPanel_]() {
        auto* divider = new QFrame(contextPanel_);
        divider->setFrameShape(QFrame::HLine);
        divider->setFrameShadow(QFrame::Plain);
        divider->setStyleSheet(QStringLiteral("color: #cbd5e1;"));
        return divider;
    };
    const auto addContextSection = [contextLayout, &buildDivider, &configureTitleLabel, &configureValueLabel](
                                       QLabel*& titleLabel,
                                       const QString& titleObjectName,
                                       const QString& titleText,
                                       QLabel*& valueLabel,
                                       const QString& valueObjectName,
                                       const bool addTrailingDivider) {
        titleLabel = new QLabel(contextLayout->parentWidget());
        configureTitleLabel(titleLabel, titleObjectName, titleText);
        valueLabel = new QLabel(contextLayout->parentWidget());
        configureValueLabel(valueLabel, valueObjectName);
        contextLayout->addWidget(titleLabel);
        contextLayout->addWidget(valueLabel);
        if (addTrailingDivider) {
            contextLayout->addWidget(buildDivider());
        }
    };

    contextLayout->addWidget(title);
    contextLayout->addWidget(buildDivider());
    addContextSection(modelStatusTitleLabel_,
                      QStringLiteral("ContextModelTitle"),
                      QStringLiteral("当前模型"),
                      modelStatusLabel_,
                      QStringLiteral("ContextModelValue"),
                      true);
    addContextSection(imageStatusTitleLabel_,
                      QStringLiteral("ContextImageTitle"),
                      QStringLiteral("当前图像"),
                      imageStatusLabel_,
                      QStringLiteral("ContextImageValue"),
                      true);
    addContextSection(resultStatusTitleLabel_,
                      QStringLiteral("ContextResultTitle"),
                      QStringLiteral("当前结果"),
                      runStatusLabel_,
                      QStringLiteral("ContextResultValue"),
                      true);
    addContextSection(nextStepTitleLabel_,
                      QStringLiteral("ContextNextStepTitle"),
                      QStringLiteral("下一步"),
                      nextStepLabel_,
                      QStringLiteral("ContextNextStepValue"),
                      false);
    contextLayout->addStretch(1);

    layout->addWidget(navPanel_);
    layout->addWidget(pageStack_, 1);
    layout->addWidget(contextPanel_);

    setCentralWidget(central);
}

void MainWindow::wireSignals() {
    connect(navPanel_, &NavPanel::pageRequested, this, &MainWindow::showPage);
    connect(homePage_, &HomePage::loadModelClicked, this, [this]() { showPage(NavPanel::ModelsPageId); });
    connect(homePage_, &HomePage::selectImageClicked, this, [this]() { showPage(NavPanel::InferencePageId); });
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
                this, [this, downloadProcess, manifestPath, onnxPath](int, QProcess::ExitStatus) {
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
    connect(homePage_, &HomePage::modelCatalogRequested, this, [this]() {
        const QDir appDir(QCoreApplication::applicationDirPath());
        const QStringList searchPaths = {
            appDir.filePath(QStringLiteral("../models")),
            appDir.filePath(QStringLiteral("../../models")),
            appDir.filePath(QStringLiteral("models"))
        };

        QString modelsDir;
        for (const QString& path : searchPaths) {
            if (QDir(path).exists()) {
                modelsDir = path;
                break;
            }
        }
        if (modelsDir.isEmpty()) {
            modelsDir = appDir.filePath(QStringLiteral("../models"));
            QDir().mkpath(modelsDir);
        }

        ModelCatalogDialog dialog(modelsDir, this);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        const QString url = dialog.selectedModelUrl();
        const QString fileName = dialog.selectedModelFileName();
        if (url.isEmpty() || fileName.isEmpty()) {
            return;
        }

        statusBar()->showMessage(QStringLiteral("正在下载 %1...").arg(dialog.selectedModelName()));

        auto* downloadProcess = new QProcess(this);
        const QString scriptPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../scripts/download_sample_model.ps1");
        const QString altScriptPath = QDir::cleanPath(
            QCoreApplication::applicationDirPath() + QStringLiteral("/../../scripts/download_sample_model.ps1"));

        QString resolvedScript;
        if (QFileInfo::exists(scriptPath)) {
            resolvedScript = scriptPath;
        } else if (QFileInfo::exists(altScriptPath)) {
            resolvedScript = altScriptPath;
        }

        if (resolvedScript.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("下载失败"), QStringLiteral("未找到下载脚本。"));
            return;
        }

        connect(downloadProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, downloadProcess, modelsDir, fileName](int, QProcess::ExitStatus) {
            downloadProcess->deleteLater();

            const QString onnxPath = modelsDir + QStringLiteral("/") + fileName;
            const QString jsonFileName = fileName.left(fileName.lastIndexOf('.')) + QStringLiteral(".json");
            const QString manifestPath = modelsDir + QStringLiteral("/") + jsonFileName;

            if (QFileInfo::exists(manifestPath) && QFileInfo::exists(onnxPath)) {
                controller_->loadModelManifest(manifestPath);
                statusBar()->showMessage(QStringLiteral("模型下载完成并已加载"), 5000);
            } else {
                QMessageBox::warning(this, QStringLiteral("下载失败"),
                    QStringLiteral("模型下载未完成。请检查网络连接后重试。"));
            }
        });

        downloadProcess->start(
            QStringLiteral("powershell"),
            {QStringLiteral("-ExecutionPolicy"), QStringLiteral("Bypass"),
             QStringLiteral("-File"), resolvedScript,
             QStringLiteral("-ModelsDir"), modelsDir,
             QStringLiteral("-ModelUrl"), url,
             QStringLiteral("-ModelName"), fileName.left(fileName.lastIndexOf('.'))});
    });
    connect(homePage_, &HomePage::recentModelActivated, this, [this](const QString& path) {
        controller_->loadModelManifest(path);
    });
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
    connect(homePage_, &HomePage::recentInputActivated, this, [this](const QString& imagePath) {
        controller_->selectImage(imagePath);
        showPage(NavPanel::InferencePageId);
    });
    connect(modelsPage_, &ModelsPage::modelManifestSelected, this, [this](const QString& path) {
        controller_->loadModelManifest(path);
    });
    connect(modelsPage_, &ModelsPage::onnxFileSelected, this, [this](const QString& onnxPath) {
        auto* dialog = new dialogs::OnnxSetupDialog(onnxPath, this);
        if (dialog->exec() != QDialog::Accepted) {
            dialog->deleteLater();
            return;
        }
        controller_->loadOnnxFile(
            onnxPath,
            dialog->modelName(),
            dialog->inputWidth(),
            dialog->inputHeight(),
            dialog->confidenceThreshold(),
            dialog->nmsThreshold(),
            dialog->labels());
        modelsPage_->setCurrentManifest(controller_->currentManifest());
        dialog->deleteLater();
    });
    connect(inferencePage_, &InferencePage::imageSelected, this, [this](const QString& path) {
        controller_->selectImage(path);
    });
    connect(inferencePage_, &InferencePage::folderSelected, this, [this](const QString& path) {
        controller_->selectFolder(path, inferencePage_->confidenceThreshold(), inferencePage_->nmsThreshold());
    });
    connect(inferencePage_, &InferencePage::videoSelected, this, [this](const QString& path, int maxFrames) {
        if (maxFrames <= 0) {
            const int totalFrames = services::probeVideoFrameCount(path);
            if (totalFrames > 1000) {
                const auto reply = QMessageBox::question(
                    this,
                    QStringLiteral("大视频预警"),
                    QStringLiteral("该视频共有 %1 帧，处理可能需要较长时间。是否继续？").arg(totalFrames),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No);
                if (reply == QMessageBox::No) {
                    return;
                }
            }
        }
        controller_->selectVideo(path, maxFrames, inferencePage_->confidenceThreshold(), inferencePage_->nmsThreshold());
    });
    connect(inferencePage_, &InferencePage::runRequested, this, [this]() {
        controller_->runInference(inferencePage_->confidenceThreshold(), inferencePage_->nmsThreshold());
    });
    connect(inferencePage_, &InferencePage::cancelRequested, this, [this]() {
        controller_->cancelInference();
        inferencePage_->setRunning(false);
        runStatusLabel_->setText(QStringLiteral("已取消"));
        nextStepLabel_->setText(QStringLiteral("可重新开始检测。"));
    });
    connect(resultsPage_, &ResultsPage::exportRequested, this, [this]() {
        if (controller_->currentSummary().inputPath.isEmpty()) {
            QMessageBox::information(this, QStringLiteral("暂无结果"), QStringLiteral("请先完成一次推理，再导出结果。"));
            return;
        }

        QString initialDirectory = controller_->settingsStore().defaultExportDirectory();
        if (initialDirectory.isEmpty() && !controller_->currentImagePath().isEmpty()) {
            initialDirectory = QFileInfo(controller_->currentImagePath()).absolutePath();
        }

        const QString outputPath = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("导出 JSON"),
            initialDirectory.isEmpty()
                ? QString()
                : QDir(initialDirectory).filePath(QStringLiteral("result.json")),
            QStringLiteral("JSON Files (*.json)"));
        if (outputPath.isEmpty()) {
            return;
        }

        controller_->exportJson(outputPath);
        refreshSettingsPage();
        statusBar()->showMessage(QStringLiteral("JSON 已导出至 %1").arg(QDir::toNativeSeparators(outputPath)), 3000);
    });
    connect(resultsPage_, &ResultsPage::exportImageRequested, this, [this]() {
        if (controller_->currentSummary().inputPath.isEmpty()) {
            QMessageBox::information(this, QStringLiteral("暂无结果"), QStringLiteral("请先完成一次推理，再导出图片。"));
            return;
        }

        const QImage currentImage = loadUsableImage(controller_->currentImagePath());
        if (currentImage.isNull()) {
            QMessageBox::warning(this, QStringLiteral("图像不可用"), QStringLiteral("当前图像无法读取，无法导出渲染图。"));
            return;
        }

        QString initialDirectory = controller_->settingsStore().defaultExportDirectory();
        if (initialDirectory.isEmpty() && !controller_->currentImagePath().isEmpty()) {
            initialDirectory = QFileInfo(controller_->currentImagePath()).absolutePath();
        }

        const QString outputPath = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("导出图片"),
            initialDirectory.isEmpty()
                ? QString()
                : QDir(initialDirectory).filePath(QStringLiteral("result.png")),
            QStringLiteral("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp);;TIFF (*.tif *.tiff)"));
        if (outputPath.isEmpty()) {
            return;
        }

        controller_->exportImage(outputPath);
        refreshSettingsPage();
        statusBar()->showMessage(QStringLiteral("图片已导出至 %1").arg(QDir::toNativeSeparators(outputPath)), 3000);
    });
    connect(resultsPage_, &ResultsPage::exportBatchJsonRequested, this, [this]() {
        const auto results = controller_->currentBatchResults();
        if (results.isEmpty()) {
            QMessageBox::information(this, QStringLiteral("暂无结果"), QStringLiteral("没有可导出的批量结果。"));
            return;
        }

        QString initialDirectory = controller_->settingsStore().defaultExportDirectory();
        const QString outputPath = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("批量导出 JSON"),
            initialDirectory.isEmpty()
                ? QString()
                : QDir(initialDirectory).filePath(QStringLiteral("batch_results.json")),
            QStringLiteral("JSON (*.json)"));
        if (outputPath.isEmpty()) {
            return;
        }

        if (controller_->exportService().exportBatchJson(results, outputPath)) {
            statusBar()->showMessage(QStringLiteral("批量结果已导出至 %1").arg(QDir::toNativeSeparators(outputPath)), 3000);
        } else {
            QMessageBox::warning(this, QStringLiteral("导出失败"), QStringLiteral("无法写入文件：%1").arg(outputPath));
        }
    });
    connect(settingsPage_,
            &SettingsPage::defaultExportDirectoryChanged,
            this,
            [this](const QString& dir) { controller_->setDefaultExportDirectory(dir); });
    connect(settingsPage_, &SettingsPage::recentModelActivated, this, [this](const QString& path) {
        controller_->loadModelManifest(path);
    });
    connect(settingsPage_, &SettingsPage::recentInputActivated, this, [this](const QString& imagePath) {
        controller_->selectImage(imagePath);
        showPage(NavPanel::InferencePageId);
    });
    connect(settingsPage_, &SettingsPage::inferenceThreadCountChanged, this, [this](int count) {
        controller_->settingsStore().setInferenceThreadCount(count);
        controller_->modelService().setThreadCount(count);
    });
    connect(settingsPage_, &SettingsPage::useGPUChanged, this, [this](bool useGPU) {
        controller_->settingsStore().setUseGPUInference(useGPU);
        controller_->modelService().setUseGPU(useGPU);
    });

    connect(controller_, &AppController::modelLoaded, this, [this](const core::ModelManifest& manifest) {
        modelsPage_->setCurrentManifest(manifest);
        inferencePage_->setModelReady(true);
        inferencePage_->setDefaultThresholds(manifest.confidenceThreshold, manifest.nmsThreshold);
        resultsPage_->setSummary({});
        resultsPage_->setImage(loadUsableImage(controller_->currentImagePath()));
        homePage_->setQuickStartVisible(true);
        refreshSettingsPage();
        updateContextPanel();
        showPage(NavPanel::InferencePageId);
    });
    connect(controller_, &AppController::imageSelected, this, [this](const QString& imagePath, const QImage& image) {
        inferencePage_->setCurrentImagePath(imagePath);
        resultsPage_->setImage(image);
        resultsPage_->setSummary({});
        refreshSettingsPage();
        updateContextPanel();
    });
    connect(controller_, &AppController::inferenceStarted, this, [this]() {
        inferencePage_->setRunning(true);
        inferencePage_->setProgress(0, 0);
        runStatusLabel_->setText(QStringLiteral("推理中…"));
        nextStepLabel_->setText(QStringLiteral("请等待推理完成。"));
        statusBar()->showMessage(QStringLiteral("正在推理..."));
    });
    connect(controller_, &AppController::inferenceCompleted, this, [this](const core::InferenceSummary& summary) {
        inferencePage_->setRunning(false);
        inferencePage_->setCurrentImagePath(controller_->currentImagePath());
        resultsPage_->setImage(loadUsableImage(controller_->currentImagePath()));
        resultsPage_->setSummary(summary);
        resultsPage_->clearResults();
        updateContextPanel();
        showPage(NavPanel::ResultsPageId);
        statusBar()->showMessage(
            QStringLiteral("推理完成 | 目标数：%1 | 耗时：%2 ms")
                .arg(summary.detectionCount)
                .arg(summary.elapsedMs, 0, 'f', 1),
            5000);
    });
    connect(controller_, &AppController::inferenceCompletedBatch, this, [this](const QVector<core::InferenceSummary>& results) {
        inferencePage_->setRunning(false);
        if (results.isEmpty()) {
            return;
        }
        inferencePage_->setCurrentImagePath(controller_->currentImagePath());
        resultsPage_->setImage(loadUsableImage(controller_->currentImagePath()));
        resultsPage_->setSummary(results.first());
        resultsPage_->setResults(results);
        updateContextPanel();
        showPage(NavPanel::ResultsPageId);

        int totalDetections = 0;
        double totalElapsedMs = 0.0;
        int validCount = 0;
        for (const core::InferenceSummary& s : results) {
            totalDetections += s.detectionCount;
            if (s.elapsedMs > 0) {
                totalElapsedMs += s.elapsedMs;
                ++validCount;
            }
        }
        const double avgMs = validCount > 0 ? totalElapsedMs / validCount : 0.0;
        statusBar()->showMessage(
            QStringLiteral("批量推理完成 | 图像数：%1 | 平均耗时：%2 ms | 总耗时：%3 ms")
                .arg(results.size())
                .arg(avgMs, 0, 'f', 1)
                .arg(totalElapsedMs, 0, 'f', 1),
            5000);
        QMessageBox::information(
            this,
            QStringLiteral("批量推理完成"),
            QStringLiteral("共处理 %1 张图像，检测到 %2 个目标，总耗时 %3 ms。")
                .arg(results.size())
                .arg(totalDetections)
                .arg(QString::number(totalElapsedMs, 'f', 1)));
    });
    connect(controller_, &AppController::inferenceCompletedVideo, this, [this](const QVector<core::InferenceSummary>& results) {
        inferencePage_->setRunning(false);
        if (results.isEmpty()) {
            QMessageBox::information(this, QStringLiteral("无帧数据"), QStringLiteral("未能从视频中读取任何帧。"));
            return;
        }
        inferencePage_->setCurrentImagePath(QString());
        resultsPage_->setImage(QImage());
        resultsPage_->setSummary(results.first());
        resultsPage_->setResults(results);
        updateContextPanel();
        showPage(NavPanel::ResultsPageId);

        int totalDetections = 0;
        double totalElapsedMs = 0.0;
        int validCount = 0;
        for (const core::InferenceSummary& s : results) {
            totalDetections += s.detectionCount;
            if (s.elapsedMs > 0) {
                totalElapsedMs += s.elapsedMs;
                ++validCount;
            }
        }
        const double avgMs = validCount > 0 ? totalElapsedMs / validCount : 0.0;
        const double fps = avgMs > 0 ? 1000.0 / avgMs : 0.0;
        statusBar()->showMessage(
            QStringLiteral("视频推理完成 | 帧数：%1 | 平均耗时：%2 ms | FPS：%3 | 总耗时：%4 ms")
                .arg(results.size())
                .arg(avgMs, 0, 'f', 1)
                .arg(fps, 0, 'f', 1)
                .arg(totalElapsedMs, 0, 'f', 1),
            5000);
        QMessageBox::information(
            this,
            QStringLiteral("视频推理完成"),
            QStringLiteral("共处理 %1 帧，检测到 %2 个目标，总耗时 %3 ms。")
                .arg(results.size())
                .arg(totalDetections)
                .arg(QString::number(totalElapsedMs, 'f', 1)));
    });
    connect(controller_, &AppController::inferenceProgress, this, [this](int current, int total) {
        inferencePage_->setProgress(current, total);
        if (total > 0) {
            runStatusLabel_->setText(QStringLiteral("推理 %1/%2…").arg(current).arg(total));
        }
    });
    connect(controller_, &AppController::inferenceError, this, [this](const QString& message) {
        inferencePage_->setRunning(false);
        runStatusLabel_->setText(QStringLiteral("推理失败"));
        nextStepLabel_->setText(QStringLiteral("请检查模型和输入后重试。"));
        statusBar()->showMessage(QStringLiteral("推理失败 | %1").arg(message), 5000);
        QMessageBox::critical(this, QStringLiteral("推理失败"), message);
    });
    connect(controller_, &AppController::contextChanged, this, &MainWindow::updateContextPanel);
}

void MainWindow::updateContextPanel() {
    const bool hasManifest = controller_->isModelLoaded();
    const bool hasImage = !loadUsableImage(controller_->currentImagePath()).isNull();
    const bool hasSummary = !controller_->currentSummary().inputPath.isEmpty();

    const core::ModelManifest manifest = controller_->currentManifest();
    const QString modelDisplayName = !manifest.name.isEmpty()
        ? manifest.name
        : QFileInfo(controller_->currentManifestPath()).completeBaseName();
    const QFileInfo imageInfo(controller_->currentImagePath());
    const QString imageDisplayName = !imageInfo.fileName().isEmpty()
        ? imageInfo.fileName()
        : controller_->currentImagePath();

    const core::InferenceSummary summary = controller_->currentSummary();

    modelStatusLabel_->setText(
        hasManifest ? QStringLiteral("已加载：%1").arg(modelDisplayName)
                    : QStringLiteral("未选择模型清单"));
    imageStatusLabel_->setText(
        hasImage ? QStringLiteral("已选择：%1").arg(imageDisplayName)
                 : QStringLiteral("未选择图像"));
    runStatusLabel_->setText(
        hasSummary
            ? QStringLiteral("已完成，共 %1 个目标，耗时 %2 ms")
                  .arg(summary.detectionCount)
                  .arg(QString::number(summary.elapsedMs, 'f', 2))
            : QStringLiteral("尚未执行检测"));

    if (!hasManifest) {
        nextStepLabel_->setText(QStringLiteral("请先加载模型清单。"));
    } else if (!hasImage) {
        nextStepLabel_->setText(QStringLiteral("请选择一张待推理图像。"));
    } else if (!hasSummary) {
        nextStepLabel_->setText(QStringLiteral("模型和图像已就绪，可以开始检测。"));
    } else {
        nextStepLabel_->setText(QStringLiteral("可查看结果明细，或直接导出 JSON。"));
    }
}

void MainWindow::showPage(const int pageId) {
    if (pageId >= 0 && pageId < pageStack_->count()) {
        pageStack_->setCurrentIndex(pageId);
        navPanel_->setCurrentPage(pageId);
    }
}

void MainWindow::refreshSettingsPage() {
    settingsPage_->setDefaultExportDirectory(controller_->settingsStore().defaultExportDirectory());
    settingsPage_->setRecentModels(controller_->settingsStore().recentModels());
    settingsPage_->setRecentInputs(controller_->settingsStore().recentInputs());
    settingsPage_->setInferenceThreadCount(controller_->settingsStore().inferenceThreadCount());
    settingsPage_->setUseGPU(controller_->settingsStore().useGPUInference());
    controller_->modelService().setThreadCount(controller_->settingsStore().inferenceThreadCount());
    controller_->modelService().setUseGPU(controller_->settingsStore().useGPUInference());
    homePage_->setRecentModels(controller_->settingsStore().recentModels());
    homePage_->setRecentInputs(controller_->settingsStore().recentInputs());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (inferencePage_->isRunning()) {
        const auto reply = QMessageBox::question(
            this,
            QStringLiteral("确认退出"),
            QStringLiteral("推理正在进行中，退出将丢失当前进度。是否确认退出？"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }
    event->acceptProposedAction();
    handleDroppedUrls(urls);
}

void MainWindow::setupShortcuts() {
    auto* shortcutOpen = new QShortcut(QKeySequence::Open, this);
    connect(shortcutOpen, &QShortcut::activated, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("选择图像"),
            QString(),
            QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp)"));
        if (!path.isEmpty()) {
            controller_->selectImage(path);
            showPage(NavPanel::InferencePageId);
        }
    });

    auto* shortcutSave = new QShortcut(QKeySequence::Save, this);
    connect(shortcutSave, &QShortcut::activated, this, [this]() {
        if (controller_->currentSummary().inputPath.isEmpty()) {
            QMessageBox::information(this, QStringLiteral("暂无结果"), QStringLiteral("请先完成一次推理，再导出结果。"));
            return;
        }
        QString initialDirectory = controller_->settingsStore().defaultExportDirectory();
        if (initialDirectory.isEmpty() && !controller_->currentImagePath().isEmpty()) {
            initialDirectory = QFileInfo(controller_->currentImagePath()).absolutePath();
        }
        const QString outputPath = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("导出 JSON"),
            initialDirectory.isEmpty()
                ? QString()
                : QDir(initialDirectory).filePath(QStringLiteral("result.json")),
            QStringLiteral("JSON Files (*.json)"));
        if (!outputPath.isEmpty()) {
            controller_->exportJson(outputPath);
        }
    });

    auto* shortcutQuit = new QShortcut(QKeySequence::Quit, this);
    connect(shortcutQuit, &QShortcut::activated, this, &QWidget::close);
}

void MainWindow::handleDroppedUrls(const QList<QUrl>& urls) {
    static const QStringList kImageSuffixes = {
        QStringLiteral("png"),
        QStringLiteral("jpg"),
        QStringLiteral("jpeg"),
        QStringLiteral("bmp"),
        QStringLiteral("tif"),
        QStringLiteral("tiff"),
        QStringLiteral("webp"),
    };
    static const QStringList kVideoSuffixes = {
        QStringLiteral("mp4"),
        QStringLiteral("avi"),
        QStringLiteral("mkv"),
        QStringLiteral("mov"),
        QStringLiteral("wmv"),
        QStringLiteral("webm"),
        QStringLiteral("flv"),
    };

    for (const QUrl& url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }

        const QString localPath = url.toLocalFile();
        const QFileInfo info(localPath);

        if (info.isDir()) {
            controller_->selectFolder(localPath, inferencePage_->confidenceThreshold(), inferencePage_->nmsThreshold());
            return;
        }

        const QString suffix = info.suffix().toLower();
        if (kImageSuffixes.contains(suffix)) {
            controller_->selectImage(localPath);
            showPage(NavPanel::InferencePageId);
            return;
        }
        if (kVideoSuffixes.contains(suffix)) {
            controller_->selectVideo(localPath, inferencePage_->maxFrames(), inferencePage_->confidenceThreshold(), inferencePage_->nmsThreshold());
            return;
        }
    }
}

}  // namespace aitoolkit::ui
