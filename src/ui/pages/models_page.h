#pragma once

#include <QString>
#include <QWidget>

#include "core/model_manifest.h"

class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;

namespace aitoolkit::ui {

struct BuiltinModelEntry {
    QString displayName;
    QString taskType;
    QString manifestFileName;
    QString description;
};

class ModelsPage : public QWidget {
    Q_OBJECT

public:
    explicit ModelsPage(QWidget* parent = nullptr);

    void setCurrentManifest(const core::ModelManifest& manifest);
    void setCurrentManifestPath(const QString& manifestPath);
    void refreshBuiltinModels();

signals:
    void modelManifestSelected(const QString& manifestPath);
    void onnxFileSelected(const QString& onnxPath);

private:
    void updateDescription();
    void activateSelectedModel();
    QString findModelsDir() const;
    QVector<BuiltinModelEntry> builtinModels() const;

    QComboBox* taskFilterCombo_ = nullptr;
    QListWidget* modelList_ = nullptr;
    QLabel* descriptionLabel_ = nullptr;
    QPushButton* activateButton_ = nullptr;
    QPushButton* importJsonButton_ = nullptr;
    QPushButton* importOnnxButton_ = nullptr;
    QLabel* manifestPathLabel_ = nullptr;
    QLabel* manifestSummaryLabel_ = nullptr;
    QListWidget* labelsList_ = nullptr;
    QWidget* summarySection_ = nullptr;
    QWidget* labelsSection_ = nullptr;
    QVector<BuiltinModelEntry> entries_;
};

}  // namespace aitoolkit::ui
