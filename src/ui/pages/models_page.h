#pragma once

#include <QString>
#include <QWidget>

#include "core/model_manifest.h"

class QLabel;
class QListWidget;

namespace aitoolkit::ui {

class ModelsPage : public QWidget {
    Q_OBJECT

public:
    explicit ModelsPage(QWidget* parent = nullptr);

    void setCurrentManifest(const core::ModelManifest& manifest);
    void setCurrentManifestPath(const QString& manifestPath);

signals:
    void modelManifestSelected(const QString& manifestPath);
    void onnxFileSelected(const QString& onnxPath);

private:
    QLabel* manifestPathLabel_ = nullptr;
    QLabel* manifestSummaryLabel_ = nullptr;
    QListWidget* labelsList_ = nullptr;
    QWidget* loadSection_ = nullptr;
    QWidget* summarySection_ = nullptr;
    QWidget* labelsSection_ = nullptr;
};

}  // namespace aitoolkit::ui
