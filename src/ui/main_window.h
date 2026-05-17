#pragma once

#include <QDir>
#include <QFileInfo>
#include <QMainWindow>
#include <QRegularExpression>
#include <QStringList>

#include <memory>

#include "core/types.h"

class QLabel;
class QShortcut;
class QStackedWidget;
class QWidget;

namespace aitoolkit::ui {

class AppController;
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
    static QString resolveDownloadScriptPath(const QString& appDirPath) {
        const QDir appDir(appDirPath);
        const QStringList candidatePaths = {
            appDir.filePath(QStringLiteral("../scripts/download_sample_model.ps1")),
            appDir.filePath(QStringLiteral("../../scripts/download_sample_model.ps1")),
            appDir.filePath(QStringLiteral("../share/scripts/download_sample_model.ps1")),
            appDir.filePath(QStringLiteral("../../share/scripts/download_sample_model.ps1")),
        };

        for (const QString& candidatePath : candidatePaths) {
            if (QFileInfo::exists(candidatePath)) {
                return QDir::cleanPath(candidatePath);
            }
        }

        return {};
    }
    static QString sanitizeExportStem(QString stem) {
        stem = stem.trimmed();
        if (stem.isEmpty()) {
            return QStringLiteral("result");
        }

        static const QString invalidChars = QStringLiteral("\\/:*?\"<>|");
        for (const QChar ch : invalidChars) {
            stem.replace(ch, QChar('_'));
        }

        stem.replace(QStringLiteral("["), QStringLiteral("_"));
        stem.replace(QStringLiteral("]"), QStringLiteral(""));
        stem.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("_"));
        stem.replace(QRegularExpression(QStringLiteral("_+")), QStringLiteral("_"));
        stem.remove(QRegularExpression(QStringLiteral("^_+|_+$")));

        return stem.isEmpty() ? QStringLiteral("result") : stem;
    }
    static QString defaultJsonExportFileName(const core::InferenceSummary& summary) {
        QString fileName = QFileInfo(summary.inputPath).fileName();
        if (fileName.contains(QStringLiteral("[frame"), Qt::CaseInsensitive)) {
            fileName.replace(QChar('.'), QChar('_'));
            return sanitizeExportStem(fileName) + QStringLiteral(".json");
        }

        fileName = QFileInfo(summary.inputPath).completeBaseName();
        if (fileName.isEmpty()) {
            fileName = QFileInfo(summary.inputPath).fileName();
        }
        if (fileName.isEmpty()) {
            fileName = summary.modelName;
        }
        return sanitizeExportStem(fileName) + QStringLiteral(".json");
    }
    static QString defaultImageExportFileName(const core::InferenceSummary& summary) {
        QString fileName = QFileInfo(summary.inputPath).fileName();
        if (fileName.contains(QStringLiteral("[frame"), Qt::CaseInsensitive)) {
            fileName.replace(QChar('.'), QChar('_'));
            return sanitizeExportStem(fileName) + QStringLiteral(".png");
        }

        fileName = QFileInfo(summary.inputPath).completeBaseName();
        if (fileName.isEmpty()) {
            fileName = QFileInfo(summary.inputPath).fileName();
        }
        if (fileName.isEmpty()) {
            fileName = summary.modelName;
        }
        return sanitizeExportStem(fileName) + QStringLiteral(".png");
    }
    static QString defaultBatchJsonExportFileName(const QString& sourcePath) {
        if (sourcePath.isEmpty()) {
            return QStringLiteral("batch_results.json");
        }

        const QFileInfo info(sourcePath);
        QString stem = info.isDir() ? QDir(sourcePath).dirName() : info.completeBaseName();
        if (stem.isEmpty()) {
            stem = info.fileName();
        }
        stem = sanitizeExportStem(stem);
        return stem.isEmpty()
            ? QStringLiteral("batch_results.json")
            : stem + QStringLiteral("_batch_results.json");
    }
    void buildShell();
    void wireSignals();
    void updateContextPanel();
    void refreshSettingsPage();
    void showPage(int pageId);
    void setupShortcuts();
    void handleDroppedUrls(const QList<QUrl>& urls);

protected:
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    AppController* controller_ = nullptr;

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
};

}  // namespace aitoolkit::ui
