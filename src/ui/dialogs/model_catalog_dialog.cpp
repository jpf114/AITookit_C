#include "ui/dialogs/model_catalog_dialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

namespace {

QVector<CatalogModelEntry> builtinCatalog() {
    return {
        {QStringLiteral("YOLOv8n"), QStringLiteral("detection"), QStringLiteral("yolov8n.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n.onnx"),
         QStringLiteral("YOLOv8 Nano — 最小最快的检测模型（约 6MB），适合实时推理"), 640},
        {QStringLiteral("YOLOv8s"), QStringLiteral("detection"), QStringLiteral("yolov8s.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8s.onnx"),
         QStringLiteral("YOLOv8 Small — 平衡精度和速度（约 22MB）"), 640},
        {QStringLiteral("YOLOv5nu"), QStringLiteral("detection"), QStringLiteral("yolov5nu.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov5nu.onnx"),
         QStringLiteral("YOLOv5 Nano Ultralytics — YOLOv5 架构的轻量版本（约 5MB）"), 640},
        {QStringLiteral("YOLOv8n-seg"), QStringLiteral("segmentation"), QStringLiteral("yolov8n-seg.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n-seg.onnx"),
         QStringLiteral("YOLOv8 Nano 分割 — 实例分割模型（约 6MB）"), 640},
    };
}

}  // namespace

ModelCatalogDialog::ModelCatalogDialog(const QString& modelsDir, QWidget* parent)
    : QDialog(parent), modelsDir_(modelsDir) {
    setWindowTitle(QStringLiteral("模型目录"));
    setMinimumSize(500, 400);

    auto* layout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel(QStringLiteral("选择要下载的模型："), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 600;"));

    catalogList_ = new QListWidget(this);
    catalogList_->setSelectionMode(QAbstractItemView::SingleSelection);

    descriptionLabel_ = new QLabel(this);
    descriptionLabel_->setWordWrap(true);
    descriptionLabel_->setMinimumHeight(60);
    descriptionLabel_->setStyleSheet(QStringLiteral("color: #64748b; padding: 8px;"));

    downloadButton_ = new QPushButton(QStringLiteral("下载"), this);
    downloadButton_->setObjectName(QStringLiteral("PrimaryButton"));
    downloadButton_->setEnabled(false);

    auto* cancelButton = new QPushButton(QStringLiteral("取消"), this);
    cancelButton->setObjectName(QStringLiteral("SecondaryButton"));

    auto* buttonRow = new QHBoxLayout();
    buttonRow->addStretch(1);
    buttonRow->addWidget(cancelButton);
    buttonRow->addWidget(downloadButton_);

    layout->addWidget(titleLabel);
    layout->addWidget(catalogList_, 1);
    layout->addWidget(descriptionLabel_);
    layout->addLayout(buttonRow);

    connect(catalogList_, &QListWidget::currentRowChanged, this, &ModelCatalogDialog::updateDescription);
    connect(catalogList_, &QListWidget::itemDoubleClicked, this, [this]() {
        if (downloadButton_->isEnabled()) accept();
    });
    connect(downloadButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    populateCatalog();
}

void ModelCatalogDialog::populateCatalog() {
    entries_ = builtinCatalog();
    for (int i = 0; i < entries_.size(); ++i) {
        const CatalogModelEntry& entry = entries_[i];
        const QString taskLabel = entry.taskType == QStringLiteral("segmentation")
            ? QStringLiteral("分割")
            : entry.taskType == QStringLiteral("classification")
                ? QStringLiteral("分类")
                : QStringLiteral("检测");
        auto* item = new QListWidgetItem(
            QStringLiteral("%1 [%2]").arg(entry.name, taskLabel), catalogList_);
        item->setData(Qt::UserRole, i);
    }
}

void ModelCatalogDialog::updateDescription() {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= entries_.size()) {
        descriptionLabel_->clear();
        downloadButton_->setEnabled(false);
        return;
    }

    const CatalogModelEntry& entry = entries_[row];
    descriptionLabel_->setText(entry.description);
    downloadButton_->setEnabled(true);
}

QString ModelCatalogDialog::selectedModelName() const {
    const int row = catalogList_->currentRow();
    return row >= 0 && row < entries_.size() ? entries_[row].name : QString();
}

QString ModelCatalogDialog::selectedModelUrl() const {
    const int row = catalogList_->currentRow();
    return row >= 0 && row < entries_.size() ? entries_[row].url : QString();
}

QString ModelCatalogDialog::selectedModelFileName() const {
    const int row = catalogList_->currentRow();
    return row >= 0 && row < entries_.size() ? entries_[row].fileName : QString();
}

QString ModelCatalogDialog::modelsDir() const {
    return modelsDir_;
}

}  // namespace aitoolkit::ui
