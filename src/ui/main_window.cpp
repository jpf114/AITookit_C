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
    setWindowTitle(QStringLiteral("AI 检测工具"));
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
        hasManifest ? QStringLiteral("已加载：%1").arg(modelDisplayName) : QStringLiteral("未选择模型清单"));
    imageStatusLabel_->setText(
        hasImage ? QStringLiteral("已选择：%1").arg(imageDisplayName) : QStringLiteral("未选择图像"));
    runStatusLabel_->setText(
        hasSummary
            ? QStringLiteral("已完成，共 %1 个目标，耗时 %2 ms")
                  .arg(currentSummary_.detectionCount)
                  .arg(QString::number(currentSummary_.elapsedMs, 'f', 2))
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
        QMessageBox::critical(this, QStringLiteral("加载模型失败"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleImageSelected(const QString& imagePath) {
    currentImagePath_ = imagePath;
    currentSummary_ = {};
    resultsPage_->setSummary(currentSummary_);
    settingsStore_.addRecentInput(imagePath);
    inferencePage_->setCurrentImagePath(imagePath);
    resultsPage_->setImage(loadUsableImage(imagePath));
    refreshSettingsPage();
    updateContextPanel();
}

void MainWindow::handleRunRequested() {
    const QImage currentImage = loadUsableImage(currentImagePath_);
    if (currentManifestPath_.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("缺少模型"), QStringLiteral("请先加载模型清单。"));
        return;
    }
    if (currentImage.isNull()) {
        QMessageBox::warning(this, QStringLiteral("缺少图像"), QStringLiteral("请先选择一张待推理图像。"));
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
        QMessageBox::critical(this, QStringLiteral("推理失败"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::handleExportRequested() {
    if (currentSummary_.inputPath.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("暂无结果"), QStringLiteral("请先完成一次推理，再导出结果。"));
        return;
    }

    QString initialDirectory = settingsStore_.defaultExportDirectory();
    if (initialDirectory.isEmpty() && !currentImagePath_.isEmpty()) {
        initialDirectory = QFileInfo(currentImagePath_).absolutePath();
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

    try {
        exportService_.exportJson(outputPath, currentSummary_);
        settingsStore_.setDefaultExportDirectory(QFileInfo(outputPath).absolutePath());
        refreshSettingsPage();
    } catch (const std::exception& error) {
        QMessageBox::critical(this, QStringLiteral("导出失败"), QString::fromUtf8(error.what()));
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
