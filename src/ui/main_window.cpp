#include "ui/main_window.h"

#include "ui/license_utils.h"

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
#include "core/app_paths.h"

namespace aitoolkit::ui {

namespace {

int summaryResultCount(const core::InferenceSummary& summary) {
    if (summary.taskType == QStringLiteral("classification")) {
        return summary.classifications.size();
    }
    if (summary.taskType == QStringLiteral("segmentation")) {
        return summary.segmentations.size();
    }
    return summary.detectionCount;
}

QString summaryResultLabel(const core::InferenceSummary& summary) {
    if (summary.taskType == QStringLiteral("classification")) {
        return MainWindow::tr("类别数");
    }
    if (summary.taskType == QStringLiteral("segmentation")) {
        return MainWindow::tr("实例数");
    }
    return MainWindow::tr("目标数");
}

QString summaryResultUnit(const core::InferenceSummary& summary) {
    if (summary.taskType == QStringLiteral("classification")) {
        return MainWindow::tr("个类别");
    }
    if (summary.taskType == QStringLiteral("segmentation")) {
        return MainWindow::tr("个实例");
    }
    return MainWindow::tr("个目标");
}

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(tr("AI 检测工具 v%1").arg(QCoreApplication::applicationVersion()));
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

    auto* title = new QLabel(tr("任务摘要"), contextPanel_);
    title->setObjectName(QStringLiteral("ContextTitle"));

    const auto configureTitleLabel = [](QLabel* label, const QString& text) {
        label->setObjectName(QStringLiteral("ContextSectionTitle"));
        label->setText(text);
    };
    const auto configureValueLabel = [](QLabel* label) {
        label->setObjectName(QStringLiteral("ContextSectionValue"));
        label->setWordWrap(true);
    };
    const auto buildDivider = [contextPanel_ = contextPanel_]() {
        auto* divider = new QFrame(contextPanel_);
        divider->setFrameShape(QFrame::HLine);
        divider->setFrameShadow(QFrame::Plain);
        divider->setObjectName(QStringLiteral("ContextDivider"));
        return divider;
    };
    const auto addContextSection = [contextLayout, &buildDivider, &configureTitleLabel, &configureValueLabel](
                                       QLabel*& titleLabel,
                                       const QString& titleText,
                                       QLabel*& valueLabel,
                                       const bool addTrailingDivider) {
        titleLabel = new QLabel(contextLayout->parentWidget());
        configureTitleLabel(titleLabel, titleText);
        valueLabel = new QLabel(contextLayout->parentWidget());
        configureValueLabel(valueLabel);
        contextLayout->addWidget(titleLabel);
        contextLayout->addWidget(valueLabel);
        if (addTrailingDivider) {
            contextLayout->addWidget(buildDivider());
        }
    };

    contextLayout->addWidget(title);
    contextLayout->addWidget(buildDivider());
    addContextSection(modelStatusTitleLabel_,
                      tr("当前模型"),
                      modelStatusLabel_,
                      true);
    addContextSection(imageStatusTitleLabel_,
                      tr("当前图像"),
                      imageStatusLabel_,
                      true);
    addContextSection(resultStatusTitleLabel_,
                      tr("当前结果"),
                      runStatusLabel_,
                      true);
    addContextSection(nextStepTitleLabel_,
                      tr("下一步"),
                      nextStepLabel_,
                      false);
    contextLayout->addStretch(1);

    layout->addWidget(navPanel_);
    layout->addWidget(pageStack_, 1);
    layout->addWidget(contextPanel_);

    setCentralWidget(central);
}

void MainWindow::wireSignals() {
    wireNavSignals();
    wireHomeSignals();
    wireModelsSignals();
    wireInferenceSignals();
    wireResultsSignals();
    wireSettingsSignals();
    wireControllerSignals();
}

void MainWindow::wireNavSignals() {
    connect(navPanel_, &NavPanel::pageRequested, this, &MainWindow::showPage);
}

void MainWindow::wireHomeSignals() {
    connect(homePage_, &HomePage::loadModelClicked, this, [this]() { showPage(NavPanel::ModelsPageId); });
    connect(homePage_, &HomePage::selectImageClicked, this, [this]() { showPage(NavPanel::InferencePageId); });
    connect(homePage_, &HomePage::downloadSampleModelClicked, this, [this]() {
        const QString resolvedScript = resolveDownloadScriptPath(QCoreApplication::applicationDirPath());
        if (resolvedScript.isEmpty()) {
            QMessageBox::information(
                this,
                tr("下载示例模型"),
                tr("未找到下载脚本。请手动执行：\n\npowershell -ExecutionPolicy Bypass -File scripts/download_sample_model.ps1"));
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

        if (!confirmAgplModelDownload(this)) {
            return;
        }

        QMessageBox::information(
            this,
            tr("下载示例模型"),
            tr("即将下载 YOLOv8n 模型（约 6MB），请稍候..."));

        auto* downloadProcess = new QProcess(this);
        connect(downloadProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, downloadProcess, manifestPath, onnxPath](int, QProcess::ExitStatus) {
            downloadProcess->deleteLater();

            if (QFileInfo::exists(manifestPath) && QFileInfo::exists(onnxPath)) {
                controller_->loadModelManifest(manifestPath);
                statusBar()->showMessage(tr("示例模型下载完成并已加载"), 5000);
            } else {
                QMessageBox::warning(
                    this,
                    tr("下载失败"),
                    tr("模型下载未完成。请检查网络连接后重试，或手动下载模型文件。"));
            }
        });

        downloadProcess->start(
            QStringLiteral("powershell"),
            {QStringLiteral("-ExecutionPolicy"), QStringLiteral("Bypass"),
             QStringLiteral("-File"), resolvedScript,
             QStringLiteral("-ModelsDir"), modelsDir});
    });
    connect(homePage_, &HomePage::modelCatalogRequested, this, [this]() {
        QString modelsDir = core::findModelsDirectory();
        if (!QDir(modelsDir).exists()) {
            QDir().mkpath(modelsDir);
        }

        ModelCatalogDialog dialog(modelsDir, controller_->settingsStore().modelCatalogUrl(), this);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        const QString url = dialog.selectedModelUrl();
        const QString fileName = dialog.selectedModelFileName();
        const QString decoder = dialog.selectedModelDecoder();
        const QString labelsCategory = dialog.selectedModelLabelsCategory();
        if (url.isEmpty() || fileName.isEmpty()) {
            return;
        }

        statusBar()->showMessage(tr("正在下载 %1...").arg(dialog.selectedModelName()));

        auto* downloadProcess = new QProcess(this);
        const QString resolvedScript = resolveDownloadScriptPath(QCoreApplication::applicationDirPath());

        if (resolvedScript.isEmpty()) {
            QMessageBox::warning(this, tr("下载失败"), tr("未找到下载脚本。"));
            return;
        }

        const QString modelName = fileName.left(fileName.lastIndexOf('.'));
        const QString selectedName = dialog.selectedModelName();
        const QString taskType = selectedName.contains(QStringLiteral("-cls"))
            ? QStringLiteral("classification")
            : selectedName.contains(QStringLiteral("-seg"))
                ? QStringLiteral("segmentation")
                : QStringLiteral("detection");

        connect(downloadProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, downloadProcess, modelsDir, fileName](int, QProcess::ExitStatus) {
            downloadProcess->deleteLater();

            const QString onnxPath = modelsDir + QStringLiteral("/") + fileName;
            const QString jsonFileName = fileName.left(fileName.lastIndexOf('.')) + QStringLiteral(".json");
            const QString manifestPath = modelsDir + QStringLiteral("/") + jsonFileName;

            if (QFileInfo::exists(manifestPath) && QFileInfo::exists(onnxPath)) {
                controller_->loadModelManifest(manifestPath);
                statusBar()->showMessage(tr("模型下载完成并已加载"), 5000);
            } else {
                QMessageBox::warning(this, tr("下载失败"),
                    tr("模型下载未完成。请检查网络连接后重试。"));
            }
        });

        QStringList args;
        args << QStringLiteral("-ExecutionPolicy") << QStringLiteral("Bypass")
             << QStringLiteral("-File") << resolvedScript
             << QStringLiteral("-ModelsDir") << modelsDir
             << QStringLiteral("-ModelUrl") << url
             << QStringLiteral("-ModelName") << modelName
             << QStringLiteral("-TaskType") << taskType
             << QStringLiteral("-Decoder") << (decoder.isEmpty() ? QString() : decoder)
             << QStringLiteral("-LabelsCategory") << (labelsCategory.isEmpty() ? QStringLiteral("coco80") : labelsCategory)
             << QStringLiteral("-InputSize") << QString::number(dialog.selectedModelInputSize());

        downloadProcess->start(QStringLiteral("powershell"), args);
    });
    connect(homePage_, &HomePage::recentModelActivated, this, [this](const QString& path) {
        controller_->loadModelManifest(path);
    });
    connect(homePage_, &HomePage::quickStartClicked, this, [this]() {
        const QString imagePath = QFileDialog::getOpenFileName(
            this,
            tr("选择图像"),
            QString(),
            tr("图像 (*.png *.jpg *.jpeg *.bmp *.tiff *.tif *.webp)"));
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
}

void MainWindow::wireModelsSignals() {
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
}

void MainWindow::wireInferenceSignals() {
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
                    tr("大视频预警"),
                    tr("该视频共有 %1 帧，处理可能需要较长时间。是否继续？").arg(totalFrames),
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
        runStatusLabel_->setText(tr("已取消"));
        nextStepLabel_->setText(tr("可重新开始检测。"));
    });
}

void MainWindow::wireResultsSignals() {
    connect(resultsPage_, &ResultsPage::exportRequested, this, [this]() {
        if (controller_->currentSummary().inputPath.isEmpty()) {
            QMessageBox::information(this, tr("暂无结果"), tr("请先完成一次推理，再导出结果。"));
            return;
        }

        QString initialDirectory = controller_->settingsStore().defaultExportDirectory();
        if (initialDirectory.isEmpty() && !controller_->currentImagePath().isEmpty()) {
            initialDirectory = QFileInfo(controller_->currentImagePath()).absolutePath();
        }

        const QString outputPath = QFileDialog::getSaveFileName(
            this,
            tr("导出 JSON"),
            initialDirectory.isEmpty()
                ? QString()
                : QDir(initialDirectory).filePath(defaultJsonExportFileName(controller_->currentSummary())),
            QStringLiteral("JSON Files (*.json)"));
        if (outputPath.isEmpty()) {
            return;
        }

        controller_->exportJson(outputPath);
        refreshSettingsPage();
        statusBar()->showMessage(tr("JSON 已导出至 %1").arg(QDir::toNativeSeparators(outputPath)), 3000);
    });
    connect(resultsPage_, &ResultsPage::exportImageRequested, this, [this]() {
        if (controller_->currentSummary().inputPath.isEmpty()) {
            QMessageBox::information(this, tr("暂无结果"), tr("请先完成一次推理，再导出图片。"));
            return;
        }

        const QImage currentImage = loadUsableImage(controller_->currentImagePath());
        if (currentImage.isNull()) {
            QMessageBox::warning(this, tr("图像不可用"), tr("当前图像无法读取，无法导出渲染图。"));
            return;
        }

        QString initialDirectory = controller_->settingsStore().defaultExportDirectory();
        if (initialDirectory.isEmpty() && !controller_->currentImagePath().isEmpty()) {
            initialDirectory = QFileInfo(controller_->currentImagePath()).absolutePath();
        }

        const QString outputPath = QFileDialog::getSaveFileName(
            this,
            tr("导出图片"),
            initialDirectory.isEmpty()
                ? QString()
                : QDir(initialDirectory).filePath(defaultImageExportFileName(controller_->currentSummary())),
            QStringLiteral("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp);;TIFF (*.tif *.tiff)"));
        if (outputPath.isEmpty()) {
            return;
        }

        controller_->exportImage(outputPath);
        refreshSettingsPage();
        statusBar()->showMessage(tr("图片已导出至 %1").arg(QDir::toNativeSeparators(outputPath)), 3000);
    });
    connect(resultsPage_, &ResultsPage::exportBatchJsonRequested, this, [this]() {
        const auto results = controller_->currentBatchResults();
        if (results.isEmpty()) {
            QMessageBox::information(this, tr("暂无结果"), tr("没有可导出的批量结果。"));
            return;
        }

        QString initialDirectory = controller_->settingsStore().defaultExportDirectory();
        const QString outputPath = QFileDialog::getSaveFileName(
            this,
            tr("批量导出 JSON"),
            initialDirectory.isEmpty()
                ? QString()
                : QDir(initialDirectory).filePath(
                      defaultBatchJsonExportFileName(controller_->currentInputSourcePath())),
            QStringLiteral("JSON (*.json)"));
        if (outputPath.isEmpty()) {
            return;
        }

        if (controller_->exportService().exportBatchJson(results, outputPath)) {
            statusBar()->showMessage(tr("批量结果已导出至 %1").arg(QDir::toNativeSeparators(outputPath)), 3000);
        } else {
            QMessageBox::warning(this, tr("导出失败"), tr("无法写入文件：%1").arg(outputPath));
        }
    });
    connect(resultsPage_, &ResultsPage::resultSelectionChanged, this, [this](const core::InferenceSummary& summary) {
        controller_->applyInferenceResult(summary);
        const QImage previewImage = loadUsableImage(summary.inputPath);
        inferencePage_->setCurrentImagePath(previewImage.isNull() ? QString() : summary.inputPath);
    });
}

void MainWindow::wireSettingsSignals() {
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
    connect(settingsPage_, &SettingsPage::languageChanged, this, [this](const QString& langCode) {
        controller_->settingsStore().setLanguage(langCode);
        QMessageBox::information(
            this,
            tr("语言切换"),
            tr("语言设置已保存，重启应用后生效。"));
    });
    connect(settingsPage_, &SettingsPage::modelCatalogUrlChanged, this, [this](const QString& url) {
        controller_->settingsStore().setModelCatalogUrl(url);
    });
}

void MainWindow::wireControllerSignals() {
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
        runStatusLabel_->setText(tr("推理中…"));
        nextStepLabel_->setText(tr("请等待推理完成。"));
        statusBar()->showMessage(tr("正在推理..."));
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
            tr("推理完成 | %1：%2 | 耗时：%3 ms")
                .arg(summaryResultLabel(summary))
                .arg(summaryResultCount(summary))
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
            tr("批量推理完成 | 图像数：%1 | 平均耗时：%2 ms | 总耗时：%3 ms")
                .arg(results.size())
                .arg(avgMs, 0, 'f', 1)
                .arg(totalElapsedMs, 0, 'f', 1),
            5000);
        QMessageBox::information(
            this,
            tr("批量推理完成"),
            tr("共处理 %1 张图像，得到 %2 %3，总耗时 %4 ms。")
                .arg(results.size())
                .arg(totalDetections)
                .arg(summaryResultUnit(results.first()))
                .arg(QString::number(totalElapsedMs, 'f', 1)));
    });
    connect(controller_, &AppController::inferenceCompletedVideo, this, [this](const QVector<core::InferenceSummary>& results) {
        inferencePage_->setRunning(false);
        if (results.isEmpty()) {
            QMessageBox::information(this, tr("无帧数据"), tr("未能从视频中读取任何帧。"));
            return;
        }
        if (!results.first().inputPath.isEmpty()) {
            const QImage firstFrame = loadUsableImage(results.first().inputPath);
            inferencePage_->setCurrentImagePath(firstFrame.isNull() ? QString() : results.first().inputPath);
            resultsPage_->setImage(firstFrame);
        } else {
            inferencePage_->setCurrentImagePath(QString());
            resultsPage_->setImage(QImage());
        }
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
            tr("视频推理完成 | 帧数：%1 | 平均耗时：%2 ms | FPS：%3 | 总耗时：%4 ms")
                .arg(results.size())
                .arg(avgMs, 0, 'f', 1)
                .arg(fps, 0, 'f', 1)
                .arg(totalElapsedMs, 0, 'f', 1),
            5000);
        QMessageBox::information(
            this,
            tr("视频推理完成"),
            tr("共处理 %1 帧，得到 %2 %3，总耗时 %4 ms。")
                .arg(results.size())
                .arg(totalDetections)
                .arg(summaryResultUnit(results.first()))
                .arg(QString::number(totalElapsedMs, 'f', 1)));
    });
    connect(controller_, &AppController::inferenceProgress, this, [this](int current, int total) {
        inferencePage_->setProgress(current, total);
        if (total > 0) {
            runStatusLabel_->setText(tr("推理 %1/%2…").arg(current).arg(total));
        }
    });
    connect(controller_, &AppController::inferenceError, this, [this](const QString& message) {
        inferencePage_->setRunning(false);
        runStatusLabel_->setText(tr("推理失败"));
        nextStepLabel_->setText(tr("请检查模型和输入后重试。"));
        statusBar()->showMessage(tr("推理失败 | %1").arg(message), 5000);
        QMessageBox::critical(this, tr("推理失败"), message);
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
        hasManifest ? tr("已加载：%1").arg(modelDisplayName)
                    : tr("未选择模型清单"));
    imageStatusLabel_->setText(
        hasImage ? tr("已选择：%1").arg(imageDisplayName)
                 : hasSummary
                     ? tr("当前结果来自视频或批量推理")
                     : tr("未选择图像"));
    runStatusLabel_->setText(
        hasSummary
            ? tr("已完成，共 %1 %2，耗时 %3 ms")
                  .arg(summaryResultCount(summary))
                  .arg(summaryResultUnit(summary))
                  .arg(QString::number(summary.elapsedMs, 'f', 2))
            : tr("尚未执行检测"));

    if (!hasManifest) {
        nextStepLabel_->setText(tr("请先加载模型清单。"));
    } else if (hasSummary) {
        nextStepLabel_->setText(tr("可查看结果明细，或直接导出 JSON。"));
    } else if (!hasImage) {
        nextStepLabel_->setText(tr("请选择一张待推理图像。"));
    } else if (!hasSummary) {
        nextStepLabel_->setText(tr("模型和图像已就绪，可以开始检测。"));
    } else {
        nextStepLabel_->setText(tr("可查看结果明细，或直接导出 JSON。"));
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
    settingsPage_->setLanguage(controller_->settingsStore().language());
    settingsPage_->setModelCatalogUrl(controller_->settingsStore().modelCatalogUrl());
    controller_->modelService().setThreadCount(controller_->settingsStore().inferenceThreadCount());
    controller_->modelService().setUseGPU(controller_->settingsStore().useGPUInference());
    homePage_->setRecentModels(controller_->settingsStore().recentModels());
    homePage_->setRecentInputs(controller_->settingsStore().recentInputs());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (inferencePage_->isRunning()) {
        const auto reply = QMessageBox::question(
            this,
            tr("确认退出"),
            tr("推理正在进行中，退出将丢失当前进度。是否确认退出？"),
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
            tr("选择图像"),
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
            QMessageBox::information(this, tr("暂无结果"), tr("请先完成一次推理，再导出结果。"));
            return;
        }
        QString initialDirectory = controller_->settingsStore().defaultExportDirectory();
        if (initialDirectory.isEmpty() && !controller_->currentImagePath().isEmpty()) {
            initialDirectory = QFileInfo(controller_->currentImagePath()).absolutePath();
        }
        const QString outputPath = QFileDialog::getSaveFileName(
            this,
            tr("导出 JSON"),
            initialDirectory.isEmpty()
                ? QString()
                : QDir(initialDirectory).filePath(defaultJsonExportFileName(controller_->currentSummary())),
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
            if (controller_->isRunning()) {
                statusBar()->showMessage(tr("推理进行中，请等待完成后再操作。"), 3000);
                return;
            }
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
            if (controller_->isRunning()) {
                statusBar()->showMessage(tr("推理进行中，请等待完成后再操作。"), 3000);
                return;
            }
            controller_->selectVideo(localPath, inferencePage_->maxFrames(), inferencePage_->confidenceThreshold(), inferencePage_->nmsThreshold());
            return;
        }
    }
}

}  // namespace aitoolkit::ui
