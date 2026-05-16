#pragma once

#include <QStringList>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QListWidget;
class QSpinBox;

namespace aitoolkit::ui {

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

    void setDefaultExportDirectory(const QString& directoryPath);
    void setRecentModels(const QStringList& recentModels);
    void setRecentInputs(const QStringList& recentInputs);
    void setInferenceThreadCount(int count);
    void setUseGPU(bool useGPU);

signals:
    void defaultExportDirectoryChanged(const QString& directoryPath);
    void recentModelActivated(const QString& manifestPath);
    void recentInputActivated(const QString& imagePath);
    void inferenceThreadCountChanged(int count);
    void useGPUChanged(bool useGPU);

private:
    QLineEdit* exportDirectoryEdit_ = nullptr;
    QSpinBox* threadCountSpin_ = nullptr;
    QCheckBox* gpuCheckBox_ = nullptr;
    QListWidget* recentModelsList_ = nullptr;
    QListWidget* recentInputsList_ = nullptr;
};

}  // namespace aitoolkit::ui
