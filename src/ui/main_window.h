#pragma once

#include <QMainWindow>

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
