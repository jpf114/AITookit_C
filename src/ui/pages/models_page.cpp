#include "ui/pages/models_page.h"

#include <QAbstractItemView>
#include <QFileDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

namespace {

QString defaultManifestSummaryText() {
    return QStringLiteral("加载模型清单后，可在这里查看模型名称、输入尺寸、后端类型和标签数量。");
}

QString manifestSummaryText(const core::ModelManifest& manifest) {
    if (manifest.manifestPath.isEmpty()) {
        return defaultManifestSummaryText();
    }

    return QStringLiteral(
               "模型名称：%1\n任务类型：%2\n推理后端：%3\n解码器：%4\n输入尺寸：%5 x %6\n标签数量：%7\n模型文件：%8")
        .arg(manifest.name)
        .arg(manifest.taskType)
        .arg(manifest.backendType)
        .arg(manifest.decoder.isEmpty() ? QStringLiteral("yolo_v8（默认）") : manifest.decoder)
        .arg(manifest.inputWidth)
        .arg(manifest.inputHeight)
        .arg(manifest.labels.size())
        .arg(manifest.modelPath);
}

}  // namespace

ModelsPage::ModelsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto* title = new QLabel(QStringLiteral("模型准备"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* lead = new QLabel(QStringLiteral("先加载一个模型清单，再确认模型信息是否符合当前任务。"), this);
    lead->setObjectName(QStringLiteral("PageLead"));
    lead->setWordWrap(true);

    loadSection_ = new QWidget(this);
    loadSection_->setObjectName(QStringLiteral("ModelLoadSection"));
    auto* loadSectionLayout = new QVBoxLayout(loadSection_);
    loadSectionLayout->setContentsMargins(16, 16, 16, 16);
    loadSectionLayout->setSpacing(10);

    auto* loadSectionTitle = new QLabel(QStringLiteral("加载清单"), loadSection_);
    loadSectionTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 600;"));

    auto* loadHintLabel = new QLabel(QStringLiteral("支持选择 JSON 模型清单文件。"), loadSection_);
    loadHintLabel->setObjectName(QStringLiteral("SectionHint"));
    loadHintLabel->setWordWrap(true);

    auto* loadButton = new QPushButton(QStringLiteral("加载模型清单"), loadSection_);
    loadButton->setObjectName(QStringLiteral("PrimaryButton"));

    auto* loadOnnxButton = new QPushButton(QStringLiteral("加载 ONNX 文件"), loadSection_);
    loadOnnxButton->setObjectName(QStringLiteral("SecondaryButton"));

    manifestPathLabel_ = new QLabel(QStringLiteral("当前未选择模型清单"), loadSection_);
    manifestPathLabel_->setObjectName(QStringLiteral("ManifestPathLabel"));
    manifestPathLabel_->setWordWrap(true);

    loadSectionLayout->addWidget(loadSectionTitle);
    loadSectionLayout->addWidget(loadHintLabel);
    loadSectionLayout->addWidget(loadButton);
    loadSectionLayout->addWidget(loadOnnxButton);
    loadSectionLayout->addWidget(manifestPathLabel_);

    summarySection_ = new QWidget(this);
    summarySection_->setObjectName(QStringLiteral("ModelSummarySection"));
    auto* summarySectionLayout = new QVBoxLayout(summarySection_);
    summarySectionLayout->setContentsMargins(16, 16, 16, 16);
    summarySectionLayout->setSpacing(10);

    auto* summarySectionTitle = new QLabel(QStringLiteral("清单摘要"), summarySection_);
    summarySectionTitle->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 600;"));

    manifestSummaryLabel_ = new QLabel(defaultManifestSummaryText(), summarySection_);
    manifestSummaryLabel_->setObjectName(QStringLiteral("ManifestSummaryLabel"));
    manifestSummaryLabel_->setWordWrap(true);

    summarySectionLayout->addWidget(summarySectionTitle);
    summarySectionLayout->addWidget(manifestSummaryLabel_);

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

    summarySectionLayout->addStretch(1);

    connect(loadButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("选择模型清单"),
            QString(),
            QStringLiteral("JSON Files (*.json)"));
        if (!path.isEmpty()) {
            emit modelManifestSelected(path);
        }
    });
    connect(loadOnnxButton, &QPushButton::clicked, this, [this]() {
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
    layout->addWidget(loadSection_);
    layout->addWidget(summarySection_);
    layout->addWidget(labelsSection_);
    layout->addStretch(1);
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
        manifestPathLabel_->setText(QStringLiteral("当前未选择模型清单"));
        if (manifestSummaryLabel_ != nullptr) {
            manifestSummaryLabel_->setText(defaultManifestSummaryText());
        }
        return;
    }

    manifestPathLabel_->setText(QStringLiteral("当前模型清单：%1").arg(manifestPath));
}

}  // namespace aitoolkit::ui
