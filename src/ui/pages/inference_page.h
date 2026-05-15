#pragma once

#include <QString>
#include <QWidget>

class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QProgressBar;
class QSpinBox;

namespace aitoolkit::ui {

class ImagePreviewWidget;

class InferencePage : public QWidget {
    Q_OBJECT

public:
    explicit InferencePage(QWidget* parent = nullptr);

    void setCurrentImagePath(const QString& imagePath);
    void setModelReady(bool ready);
    void setRunning(bool running);
    void setProgress(int current, int total);
    void setDefaultThresholds(double confidence, double nms);

    double confidenceThreshold() const;
    double nmsThreshold() const;
    int maxFrames() const;
    bool isRunning() const;

signals:
    void imageSelected(const QString& imagePath);
    void folderSelected(const QString& folderPath);
    void videoSelected(const QString& videoPath, int maxFrames);
    void runRequested();
    void cancelRequested();

private:
    void updateRunButtonState();

    QLabel* imagePathLabel_ = nullptr;
    QLabel* readinessLabel_ = nullptr;
    QPushButton* runButton_ = nullptr;
    QPushButton* cancelButton_ = nullptr;
    QProgressBar* progressBar_ = nullptr;
    QSpinBox* maxFramesSpin_ = nullptr;
    QDoubleSpinBox* confSpin_ = nullptr;
    QDoubleSpinBox* nmsSpin_ = nullptr;
    ImagePreviewWidget* previewWidget_ = nullptr;
    bool hasValidImage_ = false;
    bool modelReady_ = false;
    bool running_ = false;
};

}  // namespace aitoolkit::ui
