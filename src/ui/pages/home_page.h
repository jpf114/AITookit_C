#pragma once

#include <QWidget>

class QLabel;
class QListWidget;

namespace aitoolkit::ui {

class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget* parent = nullptr);

    void setRecentModels(const QStringList& paths);
    void setRecentInputs(const QStringList& paths);

signals:
    void loadModelClicked();
    void selectImageClicked();
    void downloadSampleModelClicked();
    void recentModelActivated(const QString& path);
    void recentInputActivated(const QString& path);

private:
    QListWidget* recentModelsList_ = nullptr;
    QListWidget* recentInputsList_ = nullptr;
};

}  // namespace aitoolkit::ui
