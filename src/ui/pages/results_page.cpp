#include "ui/pages/results_page.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QHBoxLayout>
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

    auto* title = new QLabel(QStringLiteral("\u7ed3\u679c"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* exportButton = new QPushButton(QStringLiteral("\u5bfc\u51fa JSON"), this);
    exportButton->setObjectName(QStringLiteral("SecondaryButton"));

    auto* exportImageButton = new QPushButton(QStringLiteral("\u5bfc\u51fa\u56fe\u7247"), this);
    exportImageButton->setObjectName(QStringLiteral("SecondaryButton"));

    summaryStrip_ = new QWidget(this);
    summaryStrip_->setObjectName(QStringLiteral("ResultsSummaryStrip"));

    auto* summaryStripLayout = new QHBoxLayout(summaryStrip_);
    summaryStripLayout->setContentsMargins(16, 12, 16, 12);
    summaryStripLayout->setSpacing(12);

    summaryLabel_ = new QLabel(QStringLiteral("\u5f53\u524d\u8fd8\u6ca1\u6709\u63a8\u7406\u7ed3\u679c"), summaryStrip_);
    summaryLabel_->setObjectName(QStringLiteral("ResultsSummaryLabel"));
    summaryLabel_->setWordWrap(true);

    summaryStripLayout->addWidget(summaryLabel_, 1);
    summaryStripLayout->addWidget(exportImageButton, 0);
    summaryStripLayout->addWidget(exportButton, 0);

    previewWidget_ = new ImagePreviewWidget(this);
    previewWidget_->setMinimumSize(480, 320);

    detectionsTable_ = new QTableWidget(this);
    detectionsTable_->setObjectName(QStringLiteral("DetectionsTable"));
    detectionsTable_->setColumnCount(4);
    detectionsTable_->setHorizontalHeaderLabels({
        QStringLiteral("\u7c7b\u522b"),
        QStringLiteral("\u6807\u7b7e"),
        QStringLiteral("\u7f6e\u4fe1\u5ea6"),
        QStringLiteral("\u6846\u9009\u8303\u56f4"),
    });
    detectionsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    detectionsTable_->horizontalHeader()->setStretchLastSection(true);
    detectionsTable_->verticalHeader()->setVisible(false);
    detectionsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    detectionsTable_->setSelectionMode(QAbstractItemView::NoSelection);
    detectionsTable_->setMinimumHeight(180);

    connect(exportButton, &QPushButton::clicked, this, &ResultsPage::exportRequested);
    connect(exportImageButton, &QPushButton::clicked, this, &ResultsPage::exportImageRequested);

    layout->addWidget(title);
    layout->addWidget(summaryStrip_);
    layout->addWidget(previewWidget_, 1);
    layout->addWidget(detectionsTable_);
}

void ResultsPage::setImage(const QImage& image) {
    previewWidget_->setImage(image);
}

void ResultsPage::setSummary(const core::InferenceSummary& summary) {
    previewWidget_->setSummary(summary);

    if (summary.inputPath.isEmpty()) {
        summaryLabel_->setText(QStringLiteral("\u5f53\u524d\u8fd8\u6ca1\u6709\u63a8\u7406\u7ed3\u679c"));
        detectionsTable_->clearContents();
        detectionsTable_->setRowCount(0);
        return;
    }

    summaryLabel_->setText(
        QStringLiteral("\u6a21\u578b\uff1a%1 | \u76ee\u6807\u6570\uff1a%2 | \u8017\u65f6\uff1a%3 ms")
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
