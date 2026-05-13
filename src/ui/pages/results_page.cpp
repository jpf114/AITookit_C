#include "ui/pages/results_page.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "ui/widgets/image_preview_widget.h"

namespace aitoolkit::ui {

ResultsPage::ResultsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("推理结果"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* exportButton = new QPushButton(QStringLiteral("导出 JSON"), this);
    summaryLabel_ = new QLabel(QStringLiteral("暂无结果"), this);
    previewWidget_ = new ImagePreviewWidget(this);
    previewWidget_->setMinimumSize(480, 320);

    connect(exportButton, &QPushButton::clicked, this, &ResultsPage::exportRequested);

    layout->addWidget(title);
    layout->addWidget(exportButton);
    layout->addWidget(summaryLabel_);
    layout->addWidget(previewWidget_, 1);
}

void ResultsPage::setImage(const QImage& image) {
    previewWidget_->setImage(image);
}

void ResultsPage::setSummary(const core::InferenceSummary& summary) {
    previewWidget_->setSummary(summary);
    summaryLabel_->setText(
        QStringLiteral("模型：%1 | 检测数：%2 | 耗时：%3 ms")
            .arg(summary.modelName)
            .arg(summary.detectionCount)
            .arg(QString::number(summary.elapsedMs, 'f', 2)));
}

}  // namespace aitoolkit::ui
