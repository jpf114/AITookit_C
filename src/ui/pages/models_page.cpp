#include "ui/pages/models_page.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace aitoolkit::ui {

namespace {

QString defaultSummaryText() {
    return QStringLiteral("选择一个模型后，可在这里查看模型名称、输入尺寸、后端类型和标签数量。");
}

QString manifestSummaryText(const core::ModelManifest& manifest) {
    if (manifest.manifestPath.isEmpty()) {
        return defaultSummaryText();
    }

    const QString taskLabel = manifest.taskType == QStringLiteral("segmentation")
        ? QStringLiteral("实例分割")
        : manifest.taskType == QStringLiteral("classification")
            ? QStringLiteral("图像分类")
            : QStringLiteral("目标检测");

    return QStringLiteral(
               "模型名称：%1\n任务类型：%2\n推理后端：%3\n解码器：%4\n输入尺寸：%5 x %6\n标签数量：%7\n模型文件：%8")
        .arg(manifest.name)
        .arg(taskLabel)
        .arg(manifest.backendType)
        .arg(manifest.decoder.isEmpty() ? QStringLiteral("无（分类模型）") : manifest.decoder)
        .arg(manifest.inputWidth)
        .arg(manifest.inputHeight)
        .arg(manifest.labels.size())
        .arg(QDir::toNativeSeparators(manifest.modelPath));
}

}  // namespace

QVector<BuiltinModelEntry> ModelsPage::builtinModels() const {
    return {
        {QStringLiteral("YOLOv8n — 目标检测"),
         QStringLiteral("detection"),
         QStringLiteral("yolov8n.json"),
         QStringLiteral("YOLOv8 Nano 检测模型 — COCO 80 类，极速推理（约 12MB），mAP 37.3")},
        {QStringLiteral("YOLOv8s — 目标检测"),
         QStringLiteral("detection"),
         QStringLiteral("yolov8s.json"),
         QStringLiteral("YOLOv8 Small 检测模型 — COCO 80 类，精度更高（约 43MB），mAP 44.9")},
        {QStringLiteral("YOLOv8n — 图像分类"),
         QStringLiteral("classification"),
         QStringLiteral("yolov8n-cls.json"),
         QStringLiteral("YOLOv8 Nano 分类模型 — ImageNet 1000 类，极速推理（约 10MB），Top-1 66.6%")},
        {QStringLiteral("YOLOv8n — 实例分割"),
         QStringLiteral("segmentation"),
         QStringLiteral("yolov8n-seg.json"),
         QStringLiteral("YOLOv8 Nano 分割模型 — COCO 80 类实例分割（约 13MB），mAP 36.7")},
        {QStringLiteral("YOLOv8s — 实例分割"),
         QStringLiteral("segmentation"),
         QStringLiteral("yolov8s-seg.json"),
         QStringLiteral("YOLOv8 Small 分割模型 — COCO 80 类实例分割（约 45MB），mAP 44.6")},
    };
}

ModelsPage::ModelsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto* title = new QLabel(QStringLiteral("模型选择"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* lead = new QLabel(QStringLiteral("选择一个内置模型即可开始推理，也可导入自定义模型。"), this);
    lead->setObjectName(QStringLiteral("PageLead"));
    lead->setWordWrap(true);

    auto* builtinSection = new QWidget(this);
    builtinSection->setObjectName(QStringLiteral("ModelBuiltinSection"));
    auto* builtinLayout = new QVBoxLayout(builtinSection);
    builtinLayout->setContentsMargins(16, 16, 16, 16);
    builtinLayout->setSpacing(10);

    auto* builtinTitle = new QLabel(QStringLiteral("内置模型"), builtinSection);
    builtinTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 600;"));

    auto* filterRow = new QHBoxLayout();
    auto* filterLabel = new QLabel(QStringLiteral("任务类型："), builtinSection);
    taskFilterCombo_ = new QComboBox(builtinSection);
    taskFilterCombo_->addItem(QStringLiteral("全部"), QString());
    taskFilterCombo_->addItem(QStringLiteral("目标检测"), QStringLiteral("detection"));
    taskFilterCombo_->addItem(QStringLiteral("图像分类"), QStringLiteral("classification"));
    taskFilterCombo_->addItem(QStringLiteral("实例分割"), QStringLiteral("segmentation"));
    filterRow->addWidget(filterLabel);
    filterRow->addWidget(taskFilterCombo_);
    filterRow->addStretch(1);

    modelList_ = new QListWidget(builtinSection);
    modelList_->setSelectionMode(QAbstractItemView::SingleSelection);
    modelList_->setMaximumHeight(160);

    descriptionLabel_ = new QLabel(builtinSection);
    descriptionLabel_->setWordWrap(true);
    descriptionLabel_->setMinimumHeight(40);
    descriptionLabel_->setStyleSheet(QStringLiteral("color: #64748b; padding: 4px;"));

    activateButton_ = new QPushButton(QStringLiteral("使用此模型"), builtinSection);
    activateButton_->setObjectName(QStringLiteral("PrimaryButton"));
    activateButton_->setEnabled(false);

    builtinLayout->addWidget(builtinTitle);
    builtinLayout->addLayout(filterRow);
    builtinLayout->addWidget(modelList_);
    builtinLayout->addWidget(descriptionLabel_);
    builtinLayout->addWidget(activateButton_);

    auto* importSection = new QWidget(this);
    importSection->setObjectName(QStringLiteral("ModelImportSection"));
    auto* importLayout = new QVBoxLayout(importSection);
    importLayout->setContentsMargins(16, 16, 16, 16);
    importLayout->setSpacing(10);

    auto* importTitle = new QLabel(QStringLiteral("自定义模型"), importSection);
    importTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 600;"));

    auto* importHint = new QLabel(
        QStringLiteral("支持导入 JSON 模型清单文件或直接导入 ONNX 模型文件。"), importSection);
    importHint->setObjectName(QStringLiteral("SectionHint"));
    importHint->setWordWrap(true);

    auto* importButtonRow = new QHBoxLayout();
    importJsonButton_ = new QPushButton(QStringLiteral("导入模型清单"), importSection);
    importJsonButton_->setObjectName(QStringLiteral("SecondaryButton"));
    importOnnxButton_ = new QPushButton(QStringLiteral("导入 ONNX 文件"), importSection);
    importOnnxButton_->setObjectName(QStringLiteral("SecondaryButton"));
    importButtonRow->addWidget(importJsonButton_);
    importButtonRow->addWidget(importOnnxButton_);
    importButtonRow->addStretch(1);

    importLayout->addWidget(importTitle);
    importLayout->addWidget(importHint);
    importLayout->addLayout(importButtonRow);

    summarySection_ = new QWidget(this);
    summarySection_->setObjectName(QStringLiteral("ModelSummarySection"));
    auto* summaryLayout = new QVBoxLayout(summarySection_);
    summaryLayout->setContentsMargins(16, 16, 16, 16);
    summaryLayout->setSpacing(10);

    auto* summaryTitle = new QLabel(QStringLiteral("当前模型"), summarySection_);
    summaryTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 600;"));

    manifestPathLabel_ = new QLabel(QStringLiteral("未选择模型"), summarySection_);
    manifestPathLabel_->setObjectName(QStringLiteral("ManifestPathLabel"));
    manifestPathLabel_->setWordWrap(true);

    manifestSummaryLabel_ = new QLabel(defaultSummaryText(), summarySection_);
    manifestSummaryLabel_->setObjectName(QStringLiteral("ManifestSummaryLabel"));
    manifestSummaryLabel_->setWordWrap(true);

    summaryLayout->addWidget(summaryTitle);
    summaryLayout->addWidget(manifestPathLabel_);
    summaryLayout->addWidget(manifestSummaryLabel_);

    labelsSection_ = new QWidget(this);
    auto* labelsLayout = new QVBoxLayout(labelsSection_);
    labelsLayout->setContentsMargins(0, 0, 0, 0);
    labelsLayout->setSpacing(4);

    auto* labelsHeader = new QLabel(QStringLiteral("标签列表"), this);
    labelsHeader->setStyleSheet(QStringLiteral("font-size: 13px; font-weight: 600;"));

    labelsList_ = new QListWidget(this);
    labelsList_->setMaximumHeight(200);
    labelsList_->setSelectionMode(QAbstractItemView::NoSelection);

    labelsLayout->addWidget(labelsHeader);
    labelsLayout->addWidget(labelsList_);
    labelsSection_->hide();

    summaryLayout->addStretch(1);

    connect(taskFilterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        const QString filter = taskFilterCombo_->currentData().toString();
        modelList_->clear();
        for (int i = 0; i < entries_.size(); ++i) {
            const BuiltinModelEntry& entry = entries_[i];
            if (!filter.isEmpty() && entry.taskType != filter) continue;
            auto* item = new QListWidgetItem(entry.displayName, modelList_);
            item->setData(Qt::UserRole, i);
        }
        descriptionLabel_->clear();
        activateButton_->setEnabled(false);
    });

    connect(modelList_, &QListWidget::currentRowChanged, this, &ModelsPage::updateDescription);
    connect(modelList_, &QListWidget::itemDoubleClicked, this, &ModelsPage::activateSelectedModel);
    connect(activateButton_, &QPushButton::clicked, this, &ModelsPage::activateSelectedModel);

    connect(importJsonButton_, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("选择模型清单"),
            QString(),
            QStringLiteral("JSON Files (*.json)"));
        if (!path.isEmpty()) {
            emit modelManifestSelected(path);
        }
    });

    connect(importOnnxButton_, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("选择 ONNX 文件"),
            QString(),
            QStringLiteral("ONNX Files (*.onnx)"));
        if (!path.isEmpty()) {
            emit onnxFileSelected(path);
        }
    });

    layout->addWidget(title);
    layout->addWidget(lead);
    layout->addWidget(builtinSection);
    layout->addWidget(importSection);
    layout->addWidget(summarySection_);
    layout->addWidget(labelsSection_);
    layout->addStretch(1);

    refreshBuiltinModels();
}

void ModelsPage::refreshBuiltinModels() {
    entries_ = builtinModels();
    modelList_->clear();
    for (int i = 0; i < entries_.size(); ++i) {
        const BuiltinModelEntry& entry = entries_[i];
        auto* item = new QListWidgetItem(entry.displayName, modelList_);
        item->setData(Qt::UserRole, i);
    }
    descriptionLabel_->clear();
    activateButton_->setEnabled(false);
}

void ModelsPage::updateDescription() {
    const int row = modelList_->currentRow();
    if (row < 0 || row >= modelList_->count()) {
        descriptionLabel_->clear();
        activateButton_->setEnabled(false);
        return;
    }

    const int idx = modelList_->item(row)->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= entries_.size()) {
        descriptionLabel_->clear();
        activateButton_->setEnabled(false);
        return;
    }

    const BuiltinModelEntry& entry = entries_[idx];
    descriptionLabel_->setText(entry.description);

    const QString modelsDir = findModelsDir();
    const QString manifestPath = modelsDir + QStringLiteral("/") + entry.manifestFileName;
    const bool exists = QFileInfo::exists(manifestPath);
    activateButton_->setEnabled(exists);
    if (!exists) {
        descriptionLabel_->setText(entry.description + QStringLiteral("\n\n⚠ 模型文件未找到，请确认 models 目录中包含所需文件。"));
    }
}

void ModelsPage::activateSelectedModel() {
    const int row = modelList_->currentRow();
    if (row < 0 || row >= modelList_->count()) return;

    const int idx = modelList_->item(row)->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= entries_.size()) return;

    const BuiltinModelEntry& entry = entries_[idx];
    const QString modelsDir = findModelsDir();
    const QString manifestPath = modelsDir + QStringLiteral("/") + entry.manifestFileName;

    if (QFileInfo::exists(manifestPath)) {
        emit modelManifestSelected(manifestPath);
    }
}

QString ModelsPage::findModelsDir() const {
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList searchPaths = {
        appDir.filePath(QStringLiteral("../models")),
        appDir.filePath(QStringLiteral("../../models")),
        appDir.filePath(QStringLiteral("models"))
    };

    for (const QString& path : searchPaths) {
        if (QDir(path).exists()) {
            return QDir::cleanPath(path);
        }
    }
    return QDir::cleanPath(appDir.filePath(QStringLiteral("../models")));
}

void ModelsPage::setCurrentManifest(const core::ModelManifest& manifest) {
    setCurrentManifestPath(manifest.manifestPath);
    manifestSummaryLabel_->setText(manifestSummaryText(manifest));

    if (manifest.labels.isEmpty()) {
        labelsSection_->hide();
    } else {
        labelsList_->clear();
        for (int i = 0; i < manifest.labels.size(); ++i) {
            labelsList_->addItem(QStringLiteral("%1: %2").arg(i).arg(manifest.labels[i]));
        }
        labelsSection_->show();
    }
}

void ModelsPage::setCurrentManifestPath(const QString& manifestPath) {
    if (manifestPath.isEmpty()) {
        manifestPathLabel_->setText(QStringLiteral("未选择模型"));
        if (manifestSummaryLabel_ != nullptr) {
            manifestSummaryLabel_->setText(defaultSummaryText());
        }
        return;
    }

    manifestPathLabel_->setText(QStringLiteral("当前模型：%1").arg(QDir::toNativeSeparators(manifestPath)));
}

}  // namespace aitoolkit::ui
