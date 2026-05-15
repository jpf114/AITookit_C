#pragma once

#include <QMainWindow>
#include <QThread>

#include <memory>

#include "core/settings_store.h"
#include "core/types.h"
#include "models/yolo_detection_model.h"
#include "services/export_service.h"
#include "services/inference_worker.h"
#include "services/model_service.h"

class QLabel;
class QStackedWidget;
class QWidget;

namespace aitoolkit::ui {

class HomePage;
class InferencePage;
class ModelsPage;
class NavPanel;
class ResultsPage;
class SettingsPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void buildShell();
    void wireSignals();
    void updateContextPanel();
    void refreshSettingsPage();
    void showPage(int pageId);
    void handleModelManifestSelected(const QString& manifestPath);
    void handleOnnxFileSelected(const QString& onnxPath);
    void handleImageSelected(const QString& imagePath);
    void handleFolderSelected(const QString& folderPath);
    void handleVideoSelected(const QString& videoPath, int maxFrames);
    void handleRunRequested();
    void handleExportRequested();
    void handleExportImageRequested();
    void handleDefaultExportDirectoryChanged(const QString& directoryPath);
    void applyInferenceResult(const core::InferenceSummary& summary);

    NavPanel* navPanel_ = nullptr;
    QStackedWidget* pageStack_ = nullptr;
    QWidget* contextPanel_ = nullptr;
    QLabel* modelStatusTitleLabel_ = nullptr;
    QLabel* modelStatusLabel_ = nullptr;
    QLabel* imageStatusTitleLabel_ = nullptr;
    QLabel* imageStatusLabel_ = nullptr;
    QLabel* resultStatusTitleLabel_ = nullptr;
    QLabel* runStatusLabel_ = nullptr;
    QLabel* nextStepTitleLabel_ = nullptr;
    QLabel* nextStepLabel_ = nullptr;

    HomePage* homePage_ = nullptr;
    ModelsPage* modelsPage_ = nullptr;
    InferencePage* inferencePage_ = nullptr;
    ResultsPage* resultsPage_ = nullptr;
    SettingsPage* settingsPage_ = nullptr;

    services::ModelService modelService_;
    services::ExportService exportService_;
    core::SettingsStore settingsStore_;

    services::InferenceWorker* inferenceWorker_ = nullptr;
    QThread* inferenceThread_ = nullptr;

    core::ModelManifest currentManifest_;
    std::shared_ptr<models::YoloDetectionModel> currentModel_;
    QString currentManifestPath_;
    QString currentImagePath_;
    core::InferenceSummary currentSummary_;
};

}  // namespace aitoolkit::ui
