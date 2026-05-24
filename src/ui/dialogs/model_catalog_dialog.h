#pragma once

#include <QDialog>
#include <QVector>

class QComboBox;
class QListWidget;
class QPushButton;
class QLabel;
class QCheckBox;
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

namespace aitoolkit::ui {

struct CatalogModelEntry {
    QString name;
    QString taskType;
    QString fileName;
    QString url;
    QString description;
    int inputSize;
    QString decoder;
    QString labelsCategory;
};

class ModelCatalogDialog : public QDialog {
    Q_OBJECT

public:
    explicit ModelCatalogDialog(
        const QString& modelsDir,
        const QString& catalogUrl = QString(),
        QWidget* parent = nullptr);
    ~ModelCatalogDialog() override;

    QString selectedModelName() const;
    QString selectedModelUrl() const;
    QString selectedModelFileName() const;
    QString selectedModelDecoder() const;
    QString selectedModelLabelsCategory() const;
    int selectedModelInputSize() const;
    QString modelsDir() const;

private slots:
    void handleRemoteCatalogFinished();

private:
    void populateCatalogFromLocal();
    void startRemoteCatalogFetch();
    void setCatalogEntries(const QVector<CatalogModelEntry>& entries);
    void updateDescription();
    void applyFilter(const QString& taskType);

    QComboBox* filterCombo_ = nullptr;
    QListWidget* catalogList_ = nullptr;
    QLabel* descriptionLabel_ = nullptr;
    QPushButton* downloadButton_ = nullptr;
    QCheckBox* licenseAcceptCheckBox_ = nullptr;
    QString modelsDir_;
    QString catalogUrl_;
    QVector<CatalogModelEntry> entries_;
    QNetworkAccessManager* networkManager_ = nullptr;
    QNetworkReply* remoteCatalogReply_ = nullptr;
    QTimer* remoteCatalogTimeout_ = nullptr;
};

}  // namespace aitoolkit::ui
