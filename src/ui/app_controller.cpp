#include "ui/app_controller.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>

#include <stdexcept>

#include "models/yolo_detection_model.h"
#include "services/unicode_io.h"
#include "ui/image_utils.h"

namespace aitoolkit::ui {

AppController::AppController(QObject* parent)
    : QObject(parent) {
    inferenceThread_ = new QThread(this);
    inferenceWorker_ = new services::InferenceWorker();
    inferenceWorker_->moveToThread(inferenceThread_);
    inferenceThread_->start();

    connect(inferenceWorker_, &services::InferenceWorker::imageResultReady, this, [this](const core::InferenceSummary& summary) {
        inferenceRunning_ = false;
        applyInferenceResult(summary);
        emit inferenceCompleted(summary);
    });
    connect(inferenceWorker_, &services::InferenceWorker::batchProgress, this, [this](int completed, int total) {
        emit inferenceProgress(completed, total);
    });
    connect(inferenceWorker_, &services::InferenceWorker::batchFinished, this, [this](const QVector<core::InferenceSummary>& results) {
        inferenceRunning_ = false;
        if (!results.isEmpty()) {
            applyInferenceResult(results.first());
        }
        emit inferenceCompletedBatch(results);
    });
    connect(inferenceWorker_, &services::InferenceWorker::videoProgress, this, [this](int frameIndex, int totalFrames) {
        emit inferenceProgress(frameIndex, totalFrames);
    });
    connect(inferenceWorker_, &services::InferenceWorker::videoFinished, this, [this](const QVector<core::InferenceSummary>& results) {
        inferenceRunning_ = false;
        if (!results.isEmpty()) {
            applyInferenceResult(results.first());
        }
        emit inferenceCompletedVideo(results);
    });
    connect(inferenceWorker_, &services::InferenceWorker::error, this, [this](const QString& message) {
        inferenceRunning_ = false;
        emit inferenceError(message);
    });
}

AppController::~AppController() {
    inferenceWorker_->cancel();
    inferenceThread_->quit();
    inferenceThread_->wait();
    delete inferenceWorker_;
}

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
                if (!QFileInfo::exists(manifest.modelPath)) {
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

void AppController::loadModelManifest(const QString& manifestPath) {
    try {
        currentManifest_ = modelService_.loadManifest(manifestPath);
        currentModel_.reset();
        currentManifestPath_ = currentManifest_.manifestPath;
        currentSummary_ = {};
        settingsStore_.addRecentModel(currentManifestPath_);
        settingsStore_.setLastModelManifestPath(currentManifestPath_);
        emit modelLoaded(currentManifest_);
        emit contextChanged();
    } catch (const std::exception& e) {
        emit inferenceError(QString::fromUtf8(e.what()));
    }
}

void AppController::loadOnnxFile(const QString& onnxPath, const QString& name, int width, int height,
                                  double confidence, double nms, const QStringList& labels) {
    try {
        const core::ModelManifest manifest = modelService_.createManifestFromOnnx(
            onnxPath, name, width, height, confidence, nms, labels);

        currentManifest_ = manifest;
        currentManifestPath_ = manifest.manifestPath;
        currentModel_ = std::make_shared<models::YoloDetectionModel>(manifest);

        settingsStore_.addRecentModel(manifest.manifestPath);
        settingsStore_.setLastModelManifestPath(manifest.manifestPath);
        emit modelLoaded(manifest);
        emit contextChanged();
    } catch (const std::exception& e) {
        emit inferenceError(QString::fromUtf8(e.what()));
    }
}

void AppController::selectImage(const QString& imagePath) {
    const QImage image = loadUsableImage(imagePath);
    currentImagePath_ = image.isNull() ? QString() : imagePath;
    currentSummary_ = {};
    settingsStore_.addRecentInput(imagePath);
    emit imageSelected(currentImagePath_, image);
    emit contextChanged();
}

void AppController::selectFolder(const QString& folderPath, double confidence, double nms) {
    if (currentManifestPath_.isEmpty()) {
        emit inferenceError(QStringLiteral("请先加载模型清单，再进行批量推理。"));
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
        emit inferenceError(QStringLiteral("所选文件夹中没有找到支持的图像文件。"));
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
        inferenceWorker_->setThresholds(confidence, nms);
        settingsStore_.addRecentInput(folderPath);
        inferenceRunning_ = true;
        emit inferenceStarted();
        QMetaObject::invokeMethod(inferenceWorker_, "runBatch", Qt::QueuedConnection, Q_ARG(QStringList, imagePaths));
    } catch (const std::exception& e) {
        emit inferenceError(QString::fromUtf8(e.what()));
    }
}

void AppController::selectVideo(const QString& videoPath, const int maxFrames, double confidence, double nms) {
    if (currentManifestPath_.isEmpty()) {
        emit inferenceError(QStringLiteral("请先加载模型清单，再进行视频推理。"));
        return;
    }

    try {
        if (!currentModel_ ||
            currentModel_->manifest().manifestPath.compare(currentManifestPath_, Qt::CaseInsensitive) != 0) {
            currentModel_ = modelService_.loadDetectionModel(currentManifestPath_);
        }

        inferenceWorker_->setModel(currentModel_);
        inferenceWorker_->setThresholds(confidence, nms);
        settingsStore_.addRecentInput(videoPath);
        inferenceRunning_ = true;
        emit inferenceStarted();
        QMetaObject::invokeMethod(inferenceWorker_, "runVideo", Qt::QueuedConnection, Q_ARG(QString, videoPath), Q_ARG(int, maxFrames));
    } catch (const std::exception& e) {
        emit inferenceError(QString::fromUtf8(e.what()));
    }
}

void AppController::runInference(double confidence, double nms) {
    const QImage currentImage = loadUsableImage(currentImagePath_);
    if (currentManifestPath_.isEmpty()) {
        emit inferenceError(QStringLiteral("请先加载模型清单。"));
        return;
    }
    if (currentImage.isNull()) {
        currentImagePath_.clear();
        emit contextChanged();
        return;
    }

    try {
        if (!currentModel_ ||
            currentModel_->manifest().manifestPath.compare(currentManifestPath_, Qt::CaseInsensitive) != 0) {
            currentModel_ = modelService_.loadDetectionModel(currentManifestPath_);
        }
        inferenceWorker_->setModel(currentModel_);
        inferenceWorker_->setThresholds(confidence, nms);
        inferenceRunning_ = true;
        emit inferenceStarted();
        QMetaObject::invokeMethod(inferenceWorker_, "runImage", Qt::QueuedConnection, Q_ARG(QString, currentImagePath_));
    } catch (const std::exception& e) {
        emit inferenceError(QString::fromUtf8(e.what()));
    }
}

void AppController::cancelInference() {
    inferenceRunning_ = false;
    inferenceWorker_->cancel();
    emit inferenceCancelled();
}

void AppController::exportJson(const QString& outputPath) {
    try {
        exportService_.exportJson(outputPath, currentSummary_);
        settingsStore_.setDefaultExportDirectory(QFileInfo(outputPath).absolutePath());
        emit exportCompleted();
    } catch (const std::exception& e) {
        emit inferenceError(QString::fromUtf8(e.what()));
    }
}

void AppController::exportImage(const QString& outputPath) {
    const QImage currentImage = loadUsableImage(currentImagePath_);
    try {
        exportService_.exportRenderedImage(outputPath, currentImage, currentSummary_);
        settingsStore_.setDefaultExportDirectory(QFileInfo(outputPath).absolutePath());
        emit exportCompleted();
    } catch (const std::exception& e) {
        emit inferenceError(QString::fromUtf8(e.what()));
    }
}

void AppController::setDefaultExportDirectory(const QString& directoryPath) {
    settingsStore_.setDefaultExportDirectory(directoryPath);
}

core::ModelManifest AppController::currentManifest() const {
    return currentManifest_;
}

QString AppController::currentManifestPath() const {
    return currentManifestPath_;
}

QString AppController::currentImagePath() const {
    return currentImagePath_;
}

core::InferenceSummary AppController::currentSummary() const {
    return currentSummary_;
}

core::SettingsStore& AppController::settingsStore() {
    return settingsStore_;
}

services::ModelService& AppController::modelService() {
    return modelService_;
}

services::ExportService& AppController::exportService() {
    return exportService_;
}

bool AppController::isModelLoaded() const {
    return !currentManifestPath_.isEmpty();
}

bool AppController::isRunning() const {
    return inferenceRunning_;
}

void AppController::applyInferenceResult(const core::InferenceSummary& summary) {
    currentSummary_ = summary;
    if (!summary.inputPath.isEmpty()) {
        currentImagePath_ = summary.inputPath;
    }
    emit contextChanged();
}

}  // namespace aitoolkit::ui
