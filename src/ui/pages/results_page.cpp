#include "ui/pages/results_page.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "ui/widgets/image_preview_widget.h"

namespace aitoolkit::ui {

ResultsPage::ResultsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("结果"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* exportButton = new QPushButton(QStringLiteral("导出 JSON"), this);
    exportButton->setObjectName(QStringLiteral("SecondaryButton"));
    summaryLabel_ = new QLabel(QStringLiteral("当前还没有推理结果"), this);
    summaryLabel_->setObjectName(QStringLiteral("ResultsSummaryLabel"));

    previewWidget_ = new ImagePreviewWidget(this);
    previewWidget_->setMinimumSize(480, 320);

    detectionsTable_ = new QTableWidget(this);
    detectionsTable_->setObjectName(QStringLiteral("DetectionsTable"));
    detectionsTable_->setColumnCount(4);
    detectionsTable_->setHorizontalHeaderLabels({
        QStringLiteral("类别"),
        QStringLiteral("标签"),
        QStringLiteral("置信度"),
        QStringLiteral("框选范围"),
    });
    detectionsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    detectionsTable_->horizontalHeader()->setStretchLastSection(true);
    detectionsTable_->verticalHeader()->setVisible(false);
    detectionsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    detectionsTable_->setSelectionMode(QAbstractItemView::NoSelection);
    detectionsTable_->setMinimumHeight(180);

    connect(exportButton, &QPushButton::clicked, this, &ResultsPage::exportRequested);

    layout->addWidget(title);
    layout->addWidget(exportButton);
    layout->addWidget(summaryLabel_);
    layout->addWidget(previewWidget_, 1);
    layout->addWidget(detectionsTable_);
}

void ResultsPage::setImage(const QImage& image) {
    previewWidget_->setImage(image);
}

void ResultsPage::setSummary(const core::InferenceSummary& summary) {
    previewWidget_->setSummary(summary);

    if (summary.inputPath.isEmpty()) {
        summaryLabel_->setText(QStringLiteral("当前还没有推理结果"));
        detectionsTable_->clearContents();
        detectionsTable_->setRowCount(0);
        return;
    }

    summaryLabel_->setText(
        QStringLiteral("模型：%1 | 目标数：%2 | 耗时：%3 ms")
            .arg(summary.modelName)
            .arg(summary.detectionCount)
            .arg(QString::number(summary.elapsedMs, 'f', 2)));

    detectionsTable_->clearContents();
    detectionsTable_->setRowCount(summary.detections.size());
    for (int index = 0; index < summary.detections.size(); ++index) {
        const core::DetectionItem& detection = summary.detections.at(index);
        detectionsTable_->setItem(index, 0, new QTableWidgetItem(QString::number(detection.classId)));
        detectionsTable_->setItem(index, 1, new QTableWidgetItem(detection.label));
        detectionsTable_->setItem(index, 2, new QTableWidgetItem(QString::number(detection.confidence, 'f', 3)));
        detectionsTable_->setItem(
            index,
            3,
            new QTableWidgetItem(
                QStringLiteral("%1, %2, %3 x %4")
                    .arg(QString::number(detection.boundingBox.x(), 'f', 1))
                    .arg(QString::number(detection.boundingBox.y(), 'f', 1))
                    .arg(QString::number(detection.boundingBox.width(), 'f', 1))
                    .arg(QString::number(detection.boundingBox.height(), 'f', 1))));
    }
}

}  // namespace aitoolkit::ui
