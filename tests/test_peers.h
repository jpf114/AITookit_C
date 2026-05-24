#pragma once

#ifdef AITOOLKIT_TESTING

#include <QLabel>
#include <QListWidget>
#include <QPushButton>

#include "core/types.h"
#include "services/inference_worker.h"
#include "ui/app_controller.h"
#include "ui/main_window.h"
#include "ui/pages/inference_page.h"
#include "ui/pages/results_page.h"
#include "ui/pages/settings_page.h"
#include "ui/widgets/image_preview_widget.h"

namespace aitoolkit::testing {

struct AppControllerTestPeer {
    static void setInferenceRunning(aitoolkit::ui::AppController& controller, const bool running) {
        controller.inferenceRunning_ = running;
    }

    static aitoolkit::services::InferenceWorker* inferenceWorker(
        aitoolkit::ui::AppController& controller) {
        return controller.inferenceWorker_;
    }

    static void clearBatchResults(aitoolkit::ui::AppController& controller) {
        controller.batchResults_.clear();
    }

    static void clearCurrentImagePath(aitoolkit::ui::AppController& controller) {
        controller.currentImagePath_.clear();
    }

    static void setCurrentSummary(
        aitoolkit::ui::AppController& controller,
        const aitoolkit::core::InferenceSummary& summary) {
        controller.currentSummary_ = summary;
    }

    static void setBatchResults(
        aitoolkit::ui::AppController& controller,
        const QVector<aitoolkit::core::InferenceSummary>& results) {
        controller.batchResults_ = results;
    }
};

struct MainWindowTestPeer {
    static aitoolkit::ui::AppController* controller(aitoolkit::ui::MainWindow& window) {
        return window.controller_;
    }

    static aitoolkit::ui::ResultsPage* resultsPage(aitoolkit::ui::MainWindow& window) {
        return window.resultsPage_;
    }

    static aitoolkit::ui::InferencePage* inferencePage(aitoolkit::ui::MainWindow& window) {
        return window.inferencePage_;
    }

    static aitoolkit::ui::SettingsPage* settingsPage(aitoolkit::ui::MainWindow& window) {
        return window.settingsPage_;
    }

    static QLabel* runStatusLabel(aitoolkit::ui::MainWindow& window) {
        return window.runStatusLabel_;
    }

    static QLabel* nextStepLabel(aitoolkit::ui::MainWindow& window) {
        return window.nextStepLabel_;
    }

    static QLabel* imageStatusLabel(aitoolkit::ui::MainWindow& window) {
        return window.imageStatusLabel_;
    }

    static QLabel* modelStatusTitleLabel(aitoolkit::ui::MainWindow& window) {
        return window.modelStatusTitleLabel_;
    }

    static QLabel* imageStatusTitleLabel(aitoolkit::ui::MainWindow& window) {
        return window.imageStatusTitleLabel_;
    }

    static QLabel* resultStatusTitleLabel(aitoolkit::ui::MainWindow& window) {
        return window.resultStatusTitleLabel_;
    }

    static QLabel* nextStepTitleLabel(aitoolkit::ui::MainWindow& window) {
        return window.nextStepTitleLabel_;
    }

    static QString defaultJsonExportFileName(const aitoolkit::core::InferenceSummary& summary) {
        return aitoolkit::ui::MainWindow::defaultJsonExportFileName(summary);
    }

    static QString defaultImageExportFileName(const aitoolkit::core::InferenceSummary& summary) {
        return aitoolkit::ui::MainWindow::defaultImageExportFileName(summary);
    }

    static QString defaultBatchJsonExportFileName(const QString& sourcePath) {
        return aitoolkit::ui::MainWindow::defaultBatchJsonExportFileName(sourcePath);
    }

    static QString resolveDownloadScriptPath(const QString& appDirPath) {
        return aitoolkit::ui::MainWindow::resolveDownloadScriptPath(appDirPath);
    }
};

struct ResultsPageTestPeer {
    static aitoolkit::ui::ImagePreviewWidget* previewWidget(aitoolkit::ui::ResultsPage& page) {
        return page.previewWidget_;
    }

    static QListWidget* resultsList(aitoolkit::ui::ResultsPage& page) {
        return page.resultsList_;
    }

    static QPushButton* exportButton(aitoolkit::ui::ResultsPage& page) {
        return page.exportButton_;
    }

    static QPushButton* exportImageButton(aitoolkit::ui::ResultsPage& page) {
        return page.exportImageButton_;
    }

    static QPushButton* exportBatchButton(aitoolkit::ui::ResultsPage& page) {
        return page.exportBatchBtn_;
    }
};

struct ImagePreviewWidgetTestPeer {
    static const QImage& image(const aitoolkit::ui::ImagePreviewWidget& widget) {
        return widget.image_;
    }
};

}  // namespace aitoolkit::testing

#endif
