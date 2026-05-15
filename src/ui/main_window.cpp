#include "ui/main_window.h"

#include <QCloseEvent>
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
#include <QUrl>
#include <QVBoxLayout>

#include "ui/nav_panel.h"
#include "ui/dialogs/onnx_setup_dialog.h"
#include "ui/pages/home_page.h"
#include "ui/pages/inference_page.h"
#include "ui/pages/models_page.h"
#include "ui/pages/results_page.h"
#include "ui/pages/settings_page.h"
#include "services/unicode_io.h"

namespace aitoolkit::ui {
namespace {

QImage loadUsableImage(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        return QImage();
    }

    const QImage image(imagePath);
    return image.isNull() ? QImage() : image;
}

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("AI \u68c0\u6d4b\u5de5\u5177"));
    resize(1440, 900);
    setAcceptDrops(true);

    const QByteArray savedGeometry = settingsStore_.windowGeometry();
    if (!savedGeometry.isEmpty()) {
        restoreGeometry(savedGeometry);
    }

    inferenceThread_ = new QThread(this);
    inferenceWorker_ = new services::InferenceWorker();
    inferenceWorker_->moveToThread(inferenceThread_);
    inferenceThread_->start();

    buildShell();
    wireSignals();
    setupShortcuts();
    refreshSettingsPage();
    updateContextPanel();
    restoreLastModel();
}

MainWindow::~MainWindow() {
    settingsStore_.setWindowGeometry(saveGeometry());
    inferenceWorker_->cancel();
    inferenceThread_->quit();
    inferenceThread_->wait();
    delete inferenceWorker_;
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

    auto* title = new QLabel(QStringLiteral("\u4efb\u52a1\u6458\u8981"), contextPanel_);
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
                      QStringLiteral("\u5f53\u524d\u6a21\u578b"),
                      modelStatusLabel_,
                      QStringLiteral("ContextModelValue"),
                      true);
    addContextSection(imageStatusTitleLabel_,
                      QStringLiteral("ContextImageTitle"),
                      QStringLiteral("\u5f53\u524d\u56fe\u50cf"),
                      imageStatusLabel_,
                      QStringLiteral("ContextImageValue"),
                      true);
    addContextSection(resultStatusTitleLabel_,
                      QStringLiteral("ContextResultTitle"),
                      QStringLiteral("\u5f53\u524d\u7ed3\u679c"),
                      runStatusLabel_,
                      QStringLiteral("ContextResultValue"),
                      true);
    addContextSection(nextStepTitleLabel_,
                      QStringLiteral("ContextNextStepTitle"),
                      QStringLiteral("\u4e0b\u4e00\u6b65"),
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
    connect(homePage_, &HomePage::downloadSampleModelClicked, this, &MainWindow::handleDownloadSampleModel);
    connect(homePage_, &HomePage::recentModelActivated, this, &MainWindow::handleModelManifestSelected);
    connect(homePage_, &HomePage::recentInputActivated, this, [this](const QString& imagePath) {
        handleImageSelected(imagePath);
        showPage(NavPanel::InferencePageId);
    });
    connect(modelsPage_, &ModelsPage::modelManifestSelected, this, &MainWindow::handleModelManifestSelected);
    connect(modelsPage_, &ModelsPage::onnxFileSelected, this, &MainWindow::handleOnnxFileSelected);
    connect(inferencePage_, &InferencePage::imageSelected, this, &MainWindow::handleImageSelected);
    connect(inferencePage_, &InferencePage::folderSelected, this, &MainWindow::handleFolderSelected);
    connect(inferencePage_, &InferencePage::videoSelected, this, &MainWindow::handleVideoSelected);
    connect(inferencePage_, &InferencePage::runRequested, this, &MainWindow::handleRunRequested);
    connect(inferencePage_, &InferencePage::cancelRequested, this, [this]() {
        inferenceWorker_->cancel();
        inferencePage_->setRunning(false);
        runStatusLabel_->setText(QStringLiteral("已取消"));
        nextStepLabel_->setText(QStringLiteral("可重新开始检测。"));
    });
    connect(resultsPage_, &ResultsPage::exportRequested, this, &MainWindow::handleExportRequested);
    connect(resultsPage_, &ResultsPage::exportImageRequested, this, &MainWindow::handleExportImageRequested);
    connect(settingsPage_,
            &SettingsPage::defaultExportDirectoryChanged,
            this,
            &MainWindow::handleDefaultExportDirectoryChanged);
    connect(settingsPage_, &SettingsPage::recentModelActivated, this, &MainWindow::handleModelManifestSelected);
    connect(settingsPage_, &SettingsPage::recentInputActivated, this, [this](const QString& imagePath) {
        handleImageSelected(imagePath);
        showPage(NavPanel::InferencePageId);
    });
    connect(inferenceWorker_, &services::InferenceWorker::imageResultReady, this, [this](const core::InferenceSummary& summary) {
        inferencePage_->setRunning(false);
        applyInferenceResult(summary);
        showPage(NavPanel::ResultsPageId);
    });
    connect(inferenceWorker_, &services::InferenceWorker::batchProgress, this, [this](int completed, int total) {
        inferencePage_->setProgress(completed, total);
        runStatusLabel_->setText(QStringLiteral("批量推理 %1/%2…").arg(completed).arg(total));
    });
    connect(inferenceWorker_, &services::InferenceWorker::batchFinished, this, [this](const QVector<core::InferenceSummary>& results) {
        inferencePage_->setRunning(false);
        if (results.isEmpty()) {
            return;
        }
        currentSummary_ = results.first();
        currentImagePath_ = currentSummary_.inputPath;
        inferencePage_->setCurrentImagePath(currentImagePath_);
        resultsPage_->setImage(loadUsableImage(currentImagePath_));
        resultsPage_->setSummary(currentSummary_);
        resultsPage_->setResults(results);
        updateContextPanel();
        showPage(NavPanel::ResultsPageId);

        int totalDetections = 0;
        double totalElapsedMs = 0.0;
        for (const core::InferenceSummary& s : results) {
            totalDetections += s.detectionCount;
            totalElapsedMs += s.elapsedMs;
        }
        QMessageBox::information(
            this,
            QStringLiteral("批量推理完成"),
            QStringLiteral("共处理 %1 张图像，检测到 %2 个目标，总耗时 %3 ms。")
                .arg(results.size())
                .arg(totalDetections)
                .arg(QString::number(totalElapsedMs, 'f', 1)));
    });
    connect(inferenceWorker_, &services::InferenceWorker::videoProgress, this, [this](int frameIndex, int totalFrames) {
        inferencePage_->setProgress(frameIndex, totalFrames);
        if (totalFrames > 0) {
            runStatusLabel_->setText(QStringLiteral("视频推理 %1/%2…").arg(frameIndex).arg(totalFrames));
        } else {
            runStatusLabel_->setText(QStringLiteral("视频推理 第 %1 帧…").arg(frameIndex));
        }
    });
    connect(inferenceWorker_, &services::InferenceWorker::videoFinished, this, [this](const QVector<core::InferenceSummary>& results) {
        inferencePage_->setRunning(false);
        if (results.isEmpty()) {
            QMessageBox::information(this, QStringLiteral("无帧数据"), QStringLiteral("未能从视频中读取任何帧。"));
            return;
        }
        currentSummary_ = results.first();
        currentImagePath_.clear();
        inferencePage_->setCurrentImagePath(currentImagePath_);
        resultsPage_->setImage(QImage());
        resultsPage_->setSummary(currentSummary_);
        resultsPage_->setResults(results);
        updateContextPanel();
        showPage(NavPanel::ResultsPageId);

        int totalDetections = 0;
        double totalElapsedMs = 0.0;
        for (const core::InferenceSummary& s : results) {
            totalDetections += s.detectionCount;
            totalElapsedMs += s.elapsedMs;
        }
        QMessageBox::information(
            this,
            QStringLiteral("视频推理完成"),
            QStringLiteral("共处理 %1 帧，检测到 %2 个目标，总耗时 %3 ms。")
                .arg(results.size())
                .arg(totalDetections)
                .arg(QString::number(totalElapsedMs, 'f', 1)));
    });
    connect(inferenceWorker_, &services::InferenceWorker::error, this, [this](const QString& message) {
        inferencePage_->setRunning(false);
        runStatusLabel_->setText(QStringLiteral("推理失败"));
        nextStepLabel_->setText(QStringLiteral("请检查模型和输入后重试。"));
        QMessageBox::critical(this, QStringLiteral("推理失败"), message);
    });
}

void MainWindow::updateContextPanel() {
    const bool hasManifest = !currentManifestPath_.isEmpty();
    const bool hasImage = !loadUsableImage(currentImagePath_).isNull();
    const bool hasSummary = !currentSummary_.inputPath.isEmpty();

    const QString modelDisplayName = !currentManifest_.name.isEmpty()
        ? currentManifest_.name
        : QFileInfo(currentManifestPath_).completeBaseName();
    const QFileInfo imageInfo(currentImagePath_);
    const QString imageDisplayName = !imageInfo.fileName().isEmpty()
        ? imageInfo.fileName()
        : currentImagePath_;

    modelStatusLabel_->setText(
        hasManifest ? QStringLiteral("\u5df2\u52a0\u8f7d\uff1a%1").arg(modelDisplayName)
                    : QStringLiteral("\u672a\u9009\u62e9\u6a21\u578b\u6e05\u5355"));
    imageStatusLabel_->setText(
        hasImage ? QStringLiteral("\u5df2\u9009\u62e9\uff1a%1").arg(imageDisplayName)
                 : QStringLiteral("\u672a\u9009\u62e9\u56fe\u50cf"));
    runStatusLabel_->setText(
        hasSummary
            ? QStringLiteral("\u5df2\u5b8c\u6210\uff0c\u5171 %1 \u4e2a\u76ee\u6807\uff0c\u8017\u65f6 %2 ms")
                  .arg(currentSummary_.detectionCount)
                  .arg(QString::number(currentSummary_.elapsedMs, 'f', 2))
            : QStringLiteral("\u5c1a\u672a\u6267\u884c\u68c0\u6d4b"));

    if (!hasManifest) {
        nextStepLabel_->setText(QStringLiteral("\u8bf7\u5148\u52a0\u8f7d\u6a21\u578b\u6e05\u5355\u3002"));
    } else if (!hasImage) {
        nextStepLabel_->setText(QStringLiteral("\u8bf7\u9009\u62e9\u4e00\u5f20\u5f85\u63a8\u7406\u56fe\u50cf\u3002"));
    } else if (!hasSummary) {
        nextStepLabel_->setText(QStringLiteral("\u6a21\u578b\u548c\u56fe\u50cf\u5df2\u5c31\u7eea\uff0c\u53ef\u4ee5\u5f00\u59cb\u68c0\u6d4b\u3002"));
    } else {
        nextStepLabel_->setText(QStringLiteral("\u53ef\u67e5\u770b\u7ed3\u679c\u660e\u7ec6\uff0c\u6216\u76f4\u63a5\u5bfc\u51fa JSON\u3002"));
    }
}

void MainWindow::showPage(const int pageId) {
    if (pageId >= 0 && pageId < pageStack_->count()) {
        pageStack_->setCurrentIndex(pageId);
        navPanel_->setCurrentPage(pageId);
    }
}

void MainWindow::refreshSettingsPage() {
    settingsPage_->setDefaultExportDirectory(settingsStore_.defaultExportDirectory());
    settingsPage_->setRecentModels(settingsStore_.recentModels());
    settingsPage_->setRecentInputs(settingsStore_.recentInputs());
    homePage_->setRecentModels(settingsStore_.recentModels());
    homePage_->setRecentInputs(settingsStore_.recentInputs());
}

void MainWindow::handleModelManifestSelected(const QString& manifestPath) {
    try {
        currentManifest_ = modelService_.loadManifest(manifestPath);
        currentModel_.reset();
        currentManifestPath_ = currentManifest_.manifestPath;
        currentSummary_ = {};
        resultsPage_->setSummary(currentSummary_);
        resultsPage_->setImage(loadUsableImage(currentImagePath_));
        settingsStore_.addRecentModel(currentManifestPath_);
        settingsStore_.setLastModelManifestPath(currentManifestPath_);
        modelsPage_->setCurrentManifest(currentManifest_);
        inferencePage_->setModelReady(true);
        inferencePage_->setDefaultThresholds(
            currentManifest_.confidenceThreshold,
            currentManifest_.nmsThreshold);
        refreshSettingsPage();
        updateContextPanel();
        showPage(NavPanel::InferencePageId);
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("\u52a0\u8f7d\u6a21\u578b\u5931\u8d25"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleOnnxFileSelected(const QString& onnxPath) {
    auto* dialog = new dialogs::OnnxSetupDialog(onnxPath, this);
    if (dialog->exec() != QDialog::Accepted) {
        dialog->deleteLater();
        return;
    }

    try {
        const core::ModelManifest manifest = modelService_.createManifestFromOnnx(
            onnxPath,
            dialog->modelName(),
            dialog->inputWidth(),
            dialog->inputHeight(),
            dialog->confidenceThreshold(),
            dialog->nmsThreshold(),
            dialog->labels());

        currentManifest_ = manifest;
        currentManifestPath_ = manifest.manifestPath;
        currentModel_ = std::make_shared<models::YoloDetectionModel>(manifest);

        modelsPage_->setCurrentManifest(manifest);
        settingsStore_.addRecentModel(manifest.manifestPath);
        settingsStore_.setLastModelManifestPath(manifest.manifestPath);
        refreshSettingsPage();
        updateContextPanel();
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("ONNX 加载失败"), QString::fromUtf8(error.what()));
    }

    dialog->deleteLater();
}

void MainWindow::handleDownloadSampleModel() {
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

    if (QFileInfo::exists(manifestPath)) {
        const auto reply = QMessageBox::question(
            this,
            QStringLiteral("下载示例模型"),
            QStringLiteral("示例模型清单已存在，是否重新下载？"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (reply == QMessageBox::No) {
            handleModelManifestSelected(manifestPath);
            return;
        }
    }

    QMessageBox::information(
        this,
        QStringLiteral("下载示例模型"),
        QStringLiteral("即将打开 PowerShell 窗口下载 YOLOv8n 模型（约 6MB）。\n下载完成后请点击\"加载模型\"并选择 models/yolov8n.json。"));

    const QString command = QStringLiteral(
        "Start-Process powershell -ArgumentList '-ExecutionPolicy Bypass -File \"%1\" -ModelsDir \"%2\"' -Wait");
    QProcess::startDetached(QStringLiteral("powershell"),
                             {QStringLiteral("-ExecutionPolicy"), QStringLiteral("Bypass"),
                              QStringLiteral("-File"), resolvedScript,
                              QStringLiteral("-ModelsDir"), modelsDir});
}

void MainWindow::handleImageSelected(const QString& imagePath) {
    const QImage image = loadUsableImage(imagePath);
    currentImagePath_ = image.isNull() ? QString() : imagePath;
    currentSummary_ = {};
    resultsPage_->setSummary(currentSummary_);
    settingsStore_.addRecentInput(imagePath);
    inferencePage_->setCurrentImagePath(currentImagePath_);
    resultsPage_->setImage(image);
    refreshSettingsPage();
    updateContextPanel();
}

void MainWindow::handleFolderSelected(const QString& folderPath) {
    if (currentManifestPath_.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("缺少模型"), QStringLiteral("请先加载模型清单，再进行批量推理。"));
        return;
    }

    const QDir dir(folderPath);
    const QStringList nameFilters = {
        QStringLiteral("*.png"),
        QStringLiteral("*.jpg"),
        QStringLiteral("*.jpeg"),
        QStringLiteral("*.bmp"),
        QStringLiteral("*.tif"),
        QStringLiteral("*.tiff"),
        QStringLiteral("*.webp"),
    };
    const QFileInfoList entries = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
    if (entries.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("无图像文件"), QStringLiteral("所选文件夹中没有找到支持的图像文件。"));
        return;
    }

    try {
        if (!currentModel_ ||
            currentModel_->manifest().manifestPath.compare(currentManifestPath_, Qt::CaseInsensitive) != 0) {
            currentModel_ = modelService_.loadDetectionModel(currentManifestPath_);
        }

        QStringList imagePaths;
        imagePaths.reserve(entries.size());
        for (const QFileInfo& entry : entries) {
            imagePaths.append(entry.absoluteFilePath());
        }

        inferenceWorker_->setModel(currentModel_);
        inferenceWorker_->setThresholds(inferencePage_->confidenceThreshold(), inferencePage_->nmsThreshold());
        inferencePage_->setRunning(true);
        inferencePage_->setProgress(0, entries.size());
        runStatusLabel_->setText(QStringLiteral("批量推理 0/%1…").arg(entries.size()));
        nextStepLabel_->setText(QStringLiteral("请等待推理完成。"));
        settingsStore_.addRecentInput(folderPath);
        refreshSettingsPage();
        QMetaObject::invokeMethod(inferenceWorker_, "runBatch", Qt::QueuedConnection, Q_ARG(QStringList, imagePaths));
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("批量推理失败"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleVideoSelected(const QString& videoPath, const int maxFrames) {
    if (currentManifestPath_.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("缺少模型"), QStringLiteral("请先加载模型清单，再进行视频推理。"));
        return;
    }

    if (maxFrames <= 0) {
        const int totalFrames = services::probeVideoFrameCount(videoPath);
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

    try {
        if (!currentModel_ ||
            currentModel_->manifest().manifestPath.compare(currentManifestPath_, Qt::CaseInsensitive) != 0) {
            currentModel_ = modelService_.loadDetectionModel(currentManifestPath_);
        }

        inferenceWorker_->setModel(currentModel_);
        inferenceWorker_->setThresholds(inferencePage_->confidenceThreshold(), inferencePage_->nmsThreshold());
        inferencePage_->setRunning(true);
        inferencePage_->setProgress(0, 0);
        runStatusLabel_->setText(QStringLiteral("视频推理准备中…"));
        nextStepLabel_->setText(QStringLiteral("请等待推理完成。"));
        settingsStore_.addRecentInput(videoPath);
        refreshSettingsPage();
        QMetaObject::invokeMethod(inferenceWorker_, "runVideo", Qt::QueuedConnection, Q_ARG(QString, videoPath), Q_ARG(int, maxFrames));
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("视频推理失败"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleRunRequested() {
    const QImage currentImage = loadUsableImage(currentImagePath_);
    if (currentManifestPath_.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("\u7f3a\u5c11\u6a21\u578b"), QStringLiteral("\u8bf7\u5148\u52a0\u8f7d\u6a21\u578b\u6e05\u5355\u3002"));
        return;
    }
    if (currentImage.isNull()) {
        QMessageBox::warning(this, QStringLiteral("\u7f3a\u5c11\u56fe\u50cf"), QStringLiteral("\u8bf7\u5148\u9009\u62e9\u4e00\u5f20\u5f85\u63a8\u7406\u56fe\u50cf\u3002"));
        currentImagePath_.clear();
        inferencePage_->setCurrentImagePath(currentImagePath_);
        updateContextPanel();
        return;
    }

    try {
        if (!currentModel_ ||
            currentModel_->manifest().manifestPath.compare(currentManifestPath_, Qt::CaseInsensitive) != 0) {
            currentModel_ = modelService_.loadDetectionModel(currentManifestPath_);
        }
        inferenceWorker_->setModel(currentModel_);
        inferenceWorker_->setThresholds(inferencePage_->confidenceThreshold(), inferencePage_->nmsThreshold());
        inferencePage_->setRunning(true);
        runStatusLabel_->setText(QStringLiteral("\u63a8\u7406\u4e2d\u2026"));
        nextStepLabel_->setText(QStringLiteral("\u8bf7\u7b49\u5f85\u63a8\u7406\u5b8c\u6210\u3002"));
        QMetaObject::invokeMethod(inferenceWorker_, "runImage", Qt::QueuedConnection, Q_ARG(QString, currentImagePath_));
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("\u63a8\u7406\u5931\u8d25"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleExportRequested() {
    if (currentSummary_.inputPath.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("\u6682\u65e0\u7ed3\u679c"), QStringLiteral("\u8bf7\u5148\u5b8c\u6210\u4e00\u6b21\u63a8\u7406\uff0c\u518d\u5bfc\u51fa\u7ed3\u679c\u3002"));
        return;
    }

    QString initialDirectory = settingsStore_.defaultExportDirectory();
    if (initialDirectory.isEmpty() && !currentImagePath_.isEmpty()) {
        initialDirectory = QFileInfo(currentImagePath_).absolutePath();
    }

    const QString outputPath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("\u5bfc\u51fa JSON"),
        initialDirectory.isEmpty()
            ? QString()
            : QDir(initialDirectory).filePath(QStringLiteral("result.json")),
        QStringLiteral("JSON Files (*.json)"));
    if (outputPath.isEmpty()) {
        return;
    }

    if (QFileInfo::exists(outputPath)) {
        const int answer = QMessageBox::question(
            this,
            QStringLiteral("文件已存在"),
            QStringLiteral("文件 %1 已存在，是否覆盖？").arg(QFileInfo(outputPath).fileName()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            return;
        }
    }

    try {
        exportService_.exportJson(outputPath, currentSummary_);
        settingsStore_.setDefaultExportDirectory(QFileInfo(outputPath).absolutePath());
        refreshSettingsPage();
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("\u5bfc\u51fa\u5931\u8d25"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleDefaultExportDirectoryChanged(const QString& directoryPath) {
    settingsStore_.setDefaultExportDirectory(directoryPath);
    refreshSettingsPage();
}

void MainWindow::restoreLastModel() {
    const QString lastPath = settingsStore_.lastModelManifestPath();
    if (!lastPath.isEmpty() && QFileInfo::exists(lastPath)) {
        handleModelManifestSelected(lastPath);
    }
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
            handleImageSelected(path);
            showPage(NavPanel::InferencePageId);
        }
    });

    auto* shortcutSave = new QShortcut(QKeySequence::Save, this);
    connect(shortcutSave, &QShortcut::activated, this, &MainWindow::handleExportRequested);

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
            handleFolderSelected(localPath);
            return;
        }

        const QString suffix = info.suffix().toLower();
        if (kImageSuffixes.contains(suffix)) {
            handleImageSelected(localPath);
            showPage(NavPanel::InferencePageId);
            return;
        }
        if (kVideoSuffixes.contains(suffix)) {
            handleVideoSelected(localPath, inferencePage_->maxFrames());
            return;
        }
    }
}

void MainWindow::handleExportImageRequested() {
    if (currentSummary_.inputPath.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("暂无结果"), QStringLiteral("请先完成一次推理，再导出图片。"));
        return;
    }

    const QImage currentImage = loadUsableImage(currentImagePath_);
    if (currentImage.isNull()) {
        QMessageBox::warning(this, QStringLiteral("图像不可用"), QStringLiteral("当前图像无法读取，无法导出渲染图。"));
        return;
    }

    QString initialDirectory = settingsStore_.defaultExportDirectory();
    if (initialDirectory.isEmpty() && !currentImagePath_.isEmpty()) {
        initialDirectory = QFileInfo(currentImagePath_).absolutePath();
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

    if (QFileInfo::exists(outputPath)) {
        const int answer = QMessageBox::question(
            this,
            QStringLiteral("文件已存在"),
            QStringLiteral("文件 %1 已存在，是否覆盖？").arg(QFileInfo(outputPath).fileName()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            return;
        }
    }

    try {
        exportService_.exportRenderedImage(outputPath, currentImage, currentSummary_);
        settingsStore_.setDefaultExportDirectory(QFileInfo(outputPath).absolutePath());
        refreshSettingsPage();
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("导出失败"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::applyInferenceResult(const core::InferenceSummary& summary) {
    currentSummary_ = summary;
    resultsPage_->setSummary(summary);
    resultsPage_->setImage(loadUsableImage(currentImagePath_));
    updateContextPanel();
}

}  // namespace aitoolkit::ui
