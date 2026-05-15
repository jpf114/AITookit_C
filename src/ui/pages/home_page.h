#pragma once

#include <QWidget>

class QPushButton;
class QListWidget;

namespace aitoolkit::ui {

class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget* parent = nullptr);

    void setRecentModels(const QStringList& paths);
    void setRecentInputs(const QStringList& paths);
    void setQuickStartVisible(bool visible);

signals:
    void loadModelClicked();
    void selectImageClicked();
    void downloadSampleModelClicked();
    void quickStartClicked();
    void recentModelActivated(const QString& path);
    void recentInputActivated(const QString& path);

private:
    QListWidget* recentModelsList_ = nullptr;
    QListWidget* recentInputsList_ = nullptr;
    QPushButton* quickStartBtn_ = nullptr;
};

}  // namespace aitoolkit::ui
