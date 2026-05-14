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
    connect(modelsPage_, &ModelsPage::modelManifestSelected, this, &MainWindow::handleManifestSelected);
    connect(inferencePage_, &InferencePage::imageSelected, this, &MainWindow::handleImageSelected);
    connect(inferencePage_, &InferencePage::runRequested, this, &MainWindow::handleRunRequested);
    connect(resultsPage_, &ResultsPage::exportRequested, this, &MainWindow::handleExportRequested);
    connect(settingsPage_,
            &SettingsPage::defaultExportDirectoryChanged,
            this,
            &MainWindow::handleDefaultExportDirectoryChanged);
    connect(settingsPage_, &SettingsPage::recentModelActivated, this, &MainWindow::handleManifestSelected);
    connect(settingsPage_, &SettingsPage::recentInputActivated, this, [this](const QString& imagePath) {
        handleImageSelected(imagePath);
        showPage(NavPanel::InferencePageId);
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
}

void MainWindow::handleManifestSelected(const QString& manifestPath) {
    try {
        currentManifest_ = modelService_.loadManifest(manifestPath);
        currentModel_.reset();
        currentManifestPath_ = currentManifest_.manifestPath;
        currentSummary_ = {};
        resultsPage_->setSummary(currentSummary_);
        resultsPage_->setImage(loadUsableImage(currentImagePath_));
        settingsStore_.addRecentModel(currentManifestPath_);
        modelsPage_->setCurrentManifest(currentManifest_);
        inferencePage_->setModelReady(true);
        refreshSettingsPage();
        updateContextPanel();
        showPage(NavPanel::InferencePageId);
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("\u52a0\u8f7d\u6a21\u578b\u5931\u8d25"), QString::fromUtf8(error.what()));
    }
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
        applyInferenceResult(inferenceService_.runImage(*currentModel_, currentImagePath_));
        showPage(NavPanel::ResultsPageId);
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

void MainWindow::applyInferenceResult(const core::InferenceSummary& summary) {
    currentSummary_ = summary;
    resultsPage_->setSummary(summary);
    resultsPage_->setImage(loadUsableImage(currentImagePath_));
    updateContextPanel();
}

}  // namespace aitoolkit::ui
