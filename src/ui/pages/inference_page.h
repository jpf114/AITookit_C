#pragma once

#include <QString>
#include <QWidget>

class QLabel;
class QPushButton;

namespace aitoolkit::ui {

class ImagePreviewWidget;

class InferencePage : public QWidget {
    Q_OBJECT

public:
    explicit InferencePage(QWidget* parent = nullptr);

    void setCurrentImagePath(const QString& imagePath);
    void setModelReady(bool ready);

signals:
    void imageSelected(const QString& imagePath);
    void runRequested();

private:
    QLabel* imagePathLabel_ = nullptr;
    QLabel* readinessLabel_ = nullptr;
    QPushButton* runButton_ = nullptr;
    ImagePreviewWidget* previewWidget_ = nullptr;
    QString currentImagePath_;
    bool hasValidImage_ = false;
    bool modelReady_ = false;
};

}  // namespace aitoolkit::ui
