#include "ui/dialogs/model_catalog_dialog.h"

#include <QComboBox>
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
        {QStringLiteral("YOLO11n"),
         QStringLiteral("detection"),
         QStringLiteral("yolo11n.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n.onnx"),
         QStringLiteral("YOLO11 Nano — 最新一代检测模型，极速推理（约 5MB），mAP 39.5"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLO11s"),
         QStringLiteral("detection"),
         QStringLiteral("yolo11s.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11s.onnx"),
         QStringLiteral("YOLO11 Small — 最新一代，精度与速度平衡（约 18MB），mAP 47.0"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLO11m"),
         QStringLiteral("detection"),
         QStringLiteral("yolo11m.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11m.onnx"),
         QStringLiteral("YOLO11 Medium — 最新一代高精度检测（约 40MB），mAP 51.5"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLOv8n"),
         QStringLiteral("detection"),
         QStringLiteral("yolov8n.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n.onnx"),
         QStringLiteral("YOLOv8 Nano — 经典检测模型，极速推理（约 6MB），mAP 37.3"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLOv8s"),
         QStringLiteral("detection"),
         QStringLiteral("yolov8s.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8s.onnx"),
         QStringLiteral("YOLOv8 Small — 经典检测，精度与速度平衡（约 22MB），mAP 44.9"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLOv8m"),
         QStringLiteral("detection"),
         QStringLiteral("yolov8m.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8m.onnx"),
         QStringLiteral("YOLOv8 Medium — 经典高精度检测（约 52MB），mAP 50.2"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLOv5nu"),
         QStringLiteral("detection"),
         QStringLiteral("yolov5nu.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov5nu.onnx"),
         QStringLiteral("YOLOv5 Nano — YOLOv5 架构轻量版（约 5MB），mAP 37.1"),
         640,
         QStringLiteral("yolo_v5"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLO11n-cls"),
         QStringLiteral("classification"),
         QStringLiteral("yolo11n-cls.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n-cls.onnx"),
         QStringLiteral("YOLO11 分类 Nano — 最新一代图像分类，极速推理（约 5MB），Top-1 69.0%"),
         224,
         QString(),
         QStringLiteral("imagenet1000")},

        {QStringLiteral("YOLO11s-cls"),
         QStringLiteral("classification"),
         QStringLiteral("yolo11s-cls.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11s-cls.onnx"),
         QStringLiteral("YOLO11 分类 Small — 最新一代分类，平衡精度（约 11MB），Top-1 75.4%"),
         224,
         QString(),
         QStringLiteral("imagenet1000")},

        {QStringLiteral("YOLOv8n-cls"),
         QStringLiteral("classification"),
         QStringLiteral("yolov8n-cls.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n-cls.onnx"),
         QStringLiteral("YOLOv8 分类 Nano — 经典图像分类，极速推理（约 6MB），Top-1 66.6%"),
         224,
         QString(),
         QStringLiteral("imagenet1000")},

        {QStringLiteral("YOLO11n-seg"),
         QStringLiteral("segmentation"),
         QStringLiteral("yolo11n-seg.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n-seg.onnx"),
         QStringLiteral("YOLO11 分割 Nano — 最新一代实例分割，极速推理（约 6MB），mAP 38.0"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLO11s-seg"),
         QStringLiteral("segmentation"),
         QStringLiteral("yolo11s-seg.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11s-seg.onnx"),
         QStringLiteral("YOLO11 分割 Small — 最新一代分割，平衡精度（约 19MB），mAP 45.2"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLOv8n-seg"),
         QStringLiteral("segmentation"),
         QStringLiteral("yolov8n-seg.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n-seg.onnx"),
         QStringLiteral("YOLOv8 分割 Nano — 经典实例分割，极速推理（约 6MB），mAP 36.7"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},

        {QStringLiteral("YOLOv8s-seg"),
         QStringLiteral("segmentation"),
         QStringLiteral("yolov8s-seg.onnx"),
         QStringLiteral("https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8s-seg.onnx"),
         QStringLiteral("YOLOv8 分割 Small — 经典分割，平衡精度（约 23MB），mAP 44.6"),
         640,
         QStringLiteral("yolo_v8"),
         QStringLiteral("coco80")},
    };
}

}  // namespace

ModelCatalogDialog::ModelCatalogDialog(const QString& modelsDir, QWidget* parent)
    : QDialog(parent), modelsDir_(modelsDir) {
    setWindowTitle(QStringLiteral("模型目录"));
    setMinimumSize(600, 500);

    auto* layout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel(QStringLiteral("选择要下载的模型："), this);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 600;"));

    auto* filterRow = new QHBoxLayout();
    auto* filterLabel = new QLabel(QStringLiteral("任务类型："), this);
    filterCombo_ = new QComboBox(this);
    filterCombo_->addItem(QStringLiteral("全部"), QString());
    filterCombo_->addItem(QStringLiteral("目标检测"), QStringLiteral("detection"));
    filterCombo_->addItem(QStringLiteral("图像分类"), QStringLiteral("classification"));
    filterCombo_->addItem(QStringLiteral("实例分割"), QStringLiteral("segmentation"));
    filterRow->addWidget(filterLabel);
    filterRow->addWidget(filterCombo_);
    filterRow->addStretch(1);

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
    layout->addLayout(filterRow);
    layout->addWidget(catalogList_, 1);
    layout->addWidget(descriptionLabel_);
    layout->addLayout(buttonRow);

    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        applyFilter(filterCombo_->currentData().toString());
    });
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
    applyFilter(QString());
}

void ModelCatalogDialog::applyFilter(const QString& taskType) {
    catalogList_->clear();
    for (int i = 0; i < entries_.size(); ++i) {
        const CatalogModelEntry& entry = entries_[i];
        if (!taskType.isEmpty() && entry.taskType != taskType) {
            continue;
        }
        const QString taskLabel = entry.taskType == QStringLiteral("segmentation")
            ? QStringLiteral("分割")
            : entry.taskType == QStringLiteral("classification")
                ? QStringLiteral("分类")
                : QStringLiteral("检测");
        auto* item = new QListWidgetItem(
            QStringLiteral("%1 [%2]").arg(entry.name, taskLabel), catalogList_);
        item->setData(Qt::UserRole, i);
    }
    descriptionLabel_->clear();
    downloadButton_->setEnabled(false);
}

void ModelCatalogDialog::updateDescription() {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) {
        descriptionLabel_->clear();
        downloadButton_->setEnabled(false);
        return;
    }

    const int entryIndex = catalogList_->item(row)->data(Qt::UserRole).toInt();
    if (entryIndex < 0 || entryIndex >= entries_.size()) {
        descriptionLabel_->clear();
        downloadButton_->setEnabled(false);
        return;
    }

    const CatalogModelEntry& entry = entries_[entryIndex];
    descriptionLabel_->setText(entry.description);
    downloadButton_->setEnabled(true);
}

QString ModelCatalogDialog::selectedModelName() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].name : QString();
}

QString ModelCatalogDialog::selectedModelUrl() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].url : QString();
}

QString ModelCatalogDialog::selectedModelFileName() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].fileName : QString();
}

QString ModelCatalogDialog::selectedModelDecoder() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].decoder : QString();
}

QString ModelCatalogDialog::selectedModelLabelsCategory() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].labelsCategory : QString();
}

int ModelCatalogDialog::selectedModelInputSize() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return 640;
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].inputSize : 640;
}

QString ModelCatalogDialog::modelsDir() const {
    return modelsDir_;
}

}  // namespace aitoolkit::ui
