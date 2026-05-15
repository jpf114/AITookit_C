#pragma once

#include <QObject>
#include <QThread>

#include <memory>

#include "core/settings_store.h"
#include "core/types.h"
#include "models/inference_backend.h"
#include "services/export_service.h"
#include "services/inference_worker.h"
#include "services/model_service.h"

namespace aitoolkit::ui {

class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    void loadModelManifest(const QString& manifestPath);
    void loadOnnxFile(const QString& onnxPath, const QString& name, int width, int height,
                      double confidence, double nms, const QStringList& labels);
    void selectImage(const QString& imagePath);
    void selectFolder(const QString& folderPath, double confidence, double nms);
    void selectVideo(const QString& videoPath, int maxFrames, double confidence, double nms);
    void runInference(double confidence, double nms);
    void cancelInference();
    void exportJson(const QString& outputPath);
    void exportImage(const QString& outputPath);
    void setDefaultExportDirectory(const QString& directoryPath);
    bool tryLoadDefaultModel();

    core::ModelManifest currentManifest() const;
    QString currentManifestPath() const;
    QString currentImagePath() const;
    core::InferenceSummary currentSummary() const;
    QVector<core::InferenceSummary> currentBatchResults() const;
    core::SettingsStore& settingsStore();
    services::ModelService& modelService();
    services::ExportService& exportService();
    bool isModelLoaded() const;
    bool isRunning() const;

    void applyInferenceResult(const core::InferenceSummary& summary);

signals:
    void modelLoaded(const core::ModelManifest& manifest);
    void imageSelected(const QString& imagePath, const QImage& preview);
    void inferenceStarted();
    void inferenceCompleted(const core::InferenceSummary& summary);
    void inferenceCompletedBatch(const QVector<core::InferenceSummary>& results);
    void inferenceCompletedVideo(const QVector<core::InferenceSummary>& results);
    void inferenceProgress(int current, int total);
    void inferenceError(const QString& message);
    void inferenceCancelled();
    void exportCompleted();
    void contextChanged();

private:

    services::ModelService modelService_;
    services::ExportService exportService_;
    core::SettingsStore settingsStore_;

    services::InferenceWorker* inferenceWorker_ = nullptr;
    QThread* inferenceThread_ = nullptr;

    core::ModelManifest currentManifest_;
    std::shared_ptr<models::InferenceBackend> currentModel_;
    QString currentManifestPath_;
    QString currentImagePath_;
    core::InferenceSummary currentSummary_;
    QVector<core::InferenceSummary> batchResults_;
    bool inferenceRunning_ = false;
};

}  // namespace aitoolkit::ui
