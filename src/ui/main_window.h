#pragma once

#include <QMainWindow>

#include <memory>

#include "core/settings_store.h"
#include "core/types.h"
#include "models/yolo_detection_model.h"
#include "services/export_service.h"
#include "services/inference_service.h"
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

private:
    void buildShell();
    void wireSignals();
    void updateContextPanel();
    void refreshSettingsPage();
    void showPage(int pageId);
    void handleManifestSelected(const QString& manifestPath);
    void handleImageSelected(const QString& imagePath);
    void handleRunRequested();
    void handleExportRequested();
    void handleDefaultExportDirectoryChanged(const QString& directoryPath);
    void applyInferenceResult(const core::InferenceSummary& summary);

    NavPanel* navPanel_ = nullptr;
    QStackedWidget* pageStack_ = nullptr;
    QWidget* contextPanel_ = nullptr;
    QLabel* modelStatusLabel_ = nullptr;
    QLabel* imageStatusLabel_ = nullptr;
    QLabel* runStatusLabel_ = nullptr;

    HomePage* homePage_ = nullptr;
    ModelsPage* modelsPage_ = nullptr;
    InferencePage* inferencePage_ = nullptr;
    ResultsPage* resultsPage_ = nullptr;
    SettingsPage* settingsPage_ = nullptr;

    services::ModelService modelService_;
    services::InferenceService inferenceService_;
    services::ExportService exportService_;
    core::SettingsStore settingsStore_;

    std::unique_ptr<models::YoloDetectionModel> currentModel_;
    QString currentManifestPath_;
    QString currentImagePath_;
    core::InferenceSummary currentSummary_;
};

}  // namespace aitoolkit::ui
