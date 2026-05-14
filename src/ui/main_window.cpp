#include "ui/main_window.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "ui/nav_panel.h"
#include "ui/pages/home_page.h"
#include "ui/pages/inference_page.h"
#include "ui/pages/models_page.h"
#include "ui/pages/results_page.h"
#include "ui/pages/settings_page.h"

namespace aitoolkit::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("AI Toolkit C"));
    resize(1440, 900);

    buildShell();
    wireSignals();
    refreshSettingsPage();
    updateContextPanel();
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

    auto* title = new QLabel(QStringLiteral("Current Session"), contextPanel_);
    title->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));
    modelStatusLabel_ = new QLabel(contextPanel_);
    modelStatusLabel_->setWordWrap(true);
    imageStatusLabel_ = new QLabel(contextPanel_);
    imageStatusLabel_->setWordWrap(true);
    runStatusLabel_ = new QLabel(contextPanel_);
    runStatusLabel_->setWordWrap(true);

    contextLayout->addWidget(title);
    contextLayout->addWidget(modelStatusLabel_);
    contextLayout->addWidget(imageStatusLabel_);
    contextLayout->addWidget(runStatusLabel_);
    contextLayout->addStretch(1);

    layout->addWidget(navPanel_);
    layout->addWidget(pageStack_, 1);
    layout->addWidget(contextPanel_);

    setCentralWidget(central);
}

void MainWindow::wireSignals() {
    connect(navPanel_, &NavPanel::pageRequested, this, &MainWindow::showPage);
    connect(modelsPage_, &ModelsPage::modelManifestSelected, this, &MainWindow::handleManifestSelected);
    connect(inferencePage_, &InferencePage::imageSelected, this, &MainWindow::handleImageSelected);
    connect(inferencePage_, &InferencePage::runRequested, this, &MainWindow::handleRunRequested);
    connect(resultsPage_, &ResultsPage::exportRequested, this, &MainWindow::handleExportRequested);
    connect(settingsPage_, &SettingsPage::defaultExportDirectoryChanged, this, &MainWindow::handleDefaultExportDirectoryChanged);
    connect(settingsPage_, &SettingsPage::recentModelActivated, this, &MainWindow::handleManifestSelected);
    connect(settingsPage_, &SettingsPage::recentInputActivated, this, [this](const QString& imagePath) {
        handleImageSelected(imagePath);
        showPage(NavPanel::InferencePageId);
    });
}

void MainWindow::updateContextPanel() {
    modelStatusLabel_->setText(
        currentManifestPath_.isEmpty()
            ? QStringLiteral("Model: not selected")
            : QStringLiteral("Model: %1").arg(currentManifestPath_));
    imageStatusLabel_->setText(
        currentImagePath_.isEmpty()
            ? QStringLiteral("Image: not selected")
            : QStringLiteral("Image: %1").arg(currentImagePath_));
    runStatusLabel_->setText(
        currentSummary_.inputPath.isEmpty()
            ? QStringLiteral("Result: not run yet")
            : QStringLiteral("Result: %1 detections, %2 ms")
                  .arg(currentSummary_.detectionCount)
                  .arg(QString::number(currentSummary_.elapsedMs, 'f', 2)));
}

void MainWindow::showPage(const int pageId) {
    if (pageId >= 0 && pageId < pageStack_->count()) {
        pageStack_->setCurrentIndex(pageId);
    }
}

void MainWindow::refreshSettingsPage() {
    settingsPage_->setDefaultExportDirectory(settingsStore_.defaultExportDirectory());
    settingsPage_->setRecentModels(settingsStore_.recentModels());
    settingsPage_->setRecentInputs(settingsStore_.recentInputs());
}

void MainWindow::handleManifestSelected(const QString& manifestPath) {
    try {
        currentManifest_ = modelService_.loadManifest(manifestPath);
        currentModel_.reset();
        currentManifestPath_ = currentManifest_.manifestPath;
        settingsStore_.addRecentModel(currentManifestPath_);
        modelsPage_->setCurrentManifest(currentManifest_);
        inferencePage_->setModelReady(true);
        refreshSettingsPage();
        updateContextPanel();
        showPage(NavPanel::InferencePageId);
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("Model Load Failed"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleImageSelected(const QString& imagePath) {
    currentImagePath_ = imagePath;
    settingsStore_.addRecentInput(imagePath);
    inferencePage_->setCurrentImagePath(imagePath);
    resultsPage_->setImage(QImage(imagePath));
    refreshSettingsPage();
    updateContextPanel();
}

void MainWindow::handleRunRequested() {
    if (currentManifestPath_.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Missing Model"), QStringLiteral("Load a model manifest before running inference."));
        return;
    }
    if (currentImagePath_.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Missing Image"), QStringLiteral("Choose an image before running inference."));
        return;
    }

    try {
        if (!currentModel_ ||
            currentModel_->manifest().manifestPath.compare(currentManifestPath_, Qt::CaseInsensitive) != 0) {
            currentModel_ = modelService_.loadDetectionModel(currentManifestPath_);
        }
        applyInferenceResult(inferenceService_.runImage(*currentModel_, currentImagePath_));
        showPage(NavPanel::ResultsPageId);
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("Inference Failed"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleExportRequested() {
    if (currentSummary_.inputPath.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("No Result"), QStringLiteral("Run inference before exporting results."));
        return;
    }

    QString initialDirectory = settingsStore_.defaultExportDirectory();
    if (initialDirectory.isEmpty() && !currentImagePath_.isEmpty()) {
        initialDirectory = QFileInfo(currentImagePath_).absolutePath();
    }

    const QString outputPath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Export JSON"),
        initialDirectory.isEmpty()
            ? QString()
            : QDir(initialDirectory).filePath(QStringLiteral("result.json")),
        QStringLiteral("JSON Files (*.json)"));
    if (outputPath.isEmpty()) {
        return;
    }

    try {
        exportService_.exportJson(outputPath, currentSummary_);
        settingsStore_.setDefaultExportDirectory(QFileInfo(outputPath).absolutePath());
        refreshSettingsPage();
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("Export Failed"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleDefaultExportDirectoryChanged(const QString& directoryPath) {
    settingsStore_.setDefaultExportDirectory(directoryPath);
    refreshSettingsPage();
}

void MainWindow::applyInferenceResult(const core::InferenceSummary& summary) {
    currentSummary_ = summary;
    resultsPage_->setSummary(summary);
    resultsPage_->setImage(QImage(currentImagePath_));
    updateContextPanel();
}

}  // namespace aitoolkit::ui
