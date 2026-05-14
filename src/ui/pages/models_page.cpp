#include "ui/pages/models_page.h"

#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

namespace {

QString manifestSummaryText(const core::ModelManifest& manifest) {
    if (manifest.manifestPath.isEmpty()) {
        return QStringLiteral("加载模型清单后，可在这里查看模型名称、输入尺寸、后端类型和标签数量。");
    }

    return QStringLiteral(
               "模型名称：%1\n任务类型：%2\n推理后端：%3\n输入尺寸：%4 x %5\n标签数量：%6\n模型文件：%7")
        .arg(manifest.name)
        .arg(manifest.taskType)
        .arg(manifest.backendType)
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
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("模型"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* loadButton = new QPushButton(QStringLiteral("加载模型清单"), this);
    loadButton->setObjectName(QStringLiteral("PrimaryButton"));
    manifestPathLabel_ = new QLabel(QStringLiteral("当前未选择模型清单"), this);
    manifestPathLabel_->setObjectName(QStringLiteral("ManifestPathLabel"));
    manifestPathLabel_->setWordWrap(true);

    manifestSummaryLabel_ = new QLabel(
        QStringLiteral("加载模型清单后，可在这里查看模型名称、输入尺寸、后端类型和标签数量。"),
        this);
    manifestSummaryLabel_->setObjectName(QStringLiteral("ManifestSummaryLabel"));
    manifestSummaryLabel_->setWordWrap(true);

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

    layout->addWidget(title);
    layout->addWidget(loadButton);
    layout->addWidget(manifestPathLabel_);
    layout->addWidget(manifestSummaryLabel_);
    layout->addStretch(1);
}

void ModelsPage::setCurrentManifest(const core::ModelManifest& manifest) {
    setCurrentManifestPath(manifest.manifestPath);
    manifestSummaryLabel_->setText(manifestSummaryText(manifest));
}

void ModelsPage::setCurrentManifestPath(const QString& manifestPath) {
    if (manifestPath.isEmpty()) {
        manifestPathLabel_->setText(QStringLiteral("当前未选择模型清单"));
        if (manifestSummaryLabel_ != nullptr) {
            manifestSummaryLabel_->setText(QStringLiteral("加载模型清单后，可在这里查看模型名称、输入尺寸、后端类型和标签数量。"));
        }
        return;
    }

    manifestPathLabel_->setText(QStringLiteral("当前模型清单：%1").arg(manifestPath));
}

}  // namespace aitoolkit::ui
