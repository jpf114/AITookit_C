#pragma once

#include <QStringList>
#include <QWidget>

class QLineEdit;
class QListWidget;

namespace aitoolkit::ui {

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

    void setDefaultExportDirectory(const QString& directoryPath);
    void setRecentModels(const QStringList& recentModels);
    void setRecentInputs(const QStringList& recentInputs);

signals:
    void defaultExportDirectoryChanged(const QString& directoryPath);
    void recentModelActivated(const QString& manifestPath);
    void recentInputActivated(const QString& imagePath);

private:
    QLineEdit* exportDirectoryEdit_ = nullptr;
    QListWidget* recentModelsList_ = nullptr;
    QListWidget* recentInputsList_ = nullptr;
};

}  // namespace aitoolkit::ui
