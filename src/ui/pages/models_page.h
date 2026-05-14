#pragma once

#include <QString>
#include <QWidget>

#include "core/model_manifest.h"

class QLabel;

namespace aitoolkit::ui {

class ModelsPage : public QWidget {
    Q_OBJECT

public:
    explicit ModelsPage(QWidget* parent = nullptr);

    void setCurrentManifest(const core::ModelManifest& manifest);
    void setCurrentManifestPath(const QString& manifestPath);

signals:
    void modelManifestSelected(const QString& manifestPath);

private:
    QLabel* manifestPathLabel_ = nullptr;
    QLabel* manifestSummaryLabel_ = nullptr;
};

}  // namespace aitoolkit::ui
