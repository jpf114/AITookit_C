#pragma once

#include <QDialog>
#include <QVector>

class QListWidget;
class QPushButton;
class QLabel;

namespace aitoolkit::ui {

struct CatalogModelEntry {
    QString name;
    QString taskType;
    QString fileName;
    QString url;
    QString description;
    int inputSize;
};

class ModelCatalogDialog : public QDialog {
    Q_OBJECT

public:
    explicit ModelCatalogDialog(const QString& modelsDir, QWidget* parent = nullptr);

    QString selectedModelName() const;
    QString selectedModelUrl() const;
    QString selectedModelFileName() const;
    QString modelsDir() const;

private:
    void populateCatalog();
    void updateDescription();

    QListWidget* catalogList_ = nullptr;
    QLabel* descriptionLabel_ = nullptr;
    QPushButton* downloadButton_ = nullptr;
    QString modelsDir_;
    QVector<CatalogModelEntry> entries_;
};

}  // namespace aitoolkit::ui
