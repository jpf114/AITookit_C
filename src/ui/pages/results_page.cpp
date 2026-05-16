#include "ui/pages/results_page.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QFileInfo>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSet>
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

    exportBatchBtn_ = new QPushButton(QStringLiteral("\u6279\u91cf\u5bfc\u51fa JSON"), this);
    exportBatchBtn_->setObjectName(QStringLiteral("SecondaryButton"));
    exportBatchBtn_->hide();

    summaryStrip_ = new QWidget(this);
    summaryStrip_->setObjectName(QStringLiteral("ResultsSummaryStrip"));

    auto* summaryStripLayout = new QHBoxLayout(summaryStrip_);
    summaryStripLayout->setContentsMargins(16, 12, 16, 12);
    summaryStripLayout->setSpacing(12);

    summaryLabel_ = new QLabel(QStringLiteral("\u5f53\u524d\u8fd8\u6ca1\u6709\u63a8\u7406\u7ed3\u679c"), summaryStrip_);
    summaryLabel_->setObjectName(QStringLiteral("ResultsSummaryLabel"));
    summaryLabel_->setWordWrap(true);

    summaryStripLayout->addWidget(summaryLabel_, 1);
    summaryStripLayout->addWidget(exportBatchBtn_, 0);
    summaryStripLayout->addWidget(exportImageButton, 0);
    summaryStripLayout->addWidget(exportButton, 0);

    categoryFilter_ = new QComboBox(this);
    categoryFilter_->addItem(QStringLiteral("全部类别"));
    categoryFilter_->setMinimumWidth(150);
    connect(categoryFilter_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ResultsPage::applyCategoryFilter);

    auto* contentSplit = new QHBoxLayout();
    contentSplit->setSpacing(12);

    resultsList_ = new QListWidget(this);
    resultsList_->setObjectName(QStringLiteral("ResultsList"));
    resultsList_->setMaximumWidth(220);
    resultsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    resultsList_->hide();

    auto* rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(12);

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

    auto* tableHeader = new QHBoxLayout();
    tableHeader->setSpacing(8);
    auto* filterLabel = new QLabel(QStringLiteral("类别筛选："), this);
    tableHeader->addWidget(filterLabel);
    tableHeader->addWidget(categoryFilter_);
    tableHeader->addStretch();

    rightColumn->addWidget(previewWidget_, 1);
    rightColumn->addLayout(tableHeader);
    rightColumn->addWidget(detectionsTable_);

    contentSplit->addWidget(resultsList_);
    contentSplit->addLayout(rightColumn, 1);

    connect(exportButton, &QPushButton::clicked, this, &ResultsPage::exportRequested);
    connect(exportImageButton, &QPushButton::clicked, this, &ResultsPage::exportImageRequested);
    connect(exportBatchBtn_, &QPushButton::clicked, this, &ResultsPage::exportBatchJsonRequested);
    connect(resultsList_, &QListWidget::currentRowChanged, this, &ResultsPage::showResultAtIndex);

    layout->addWidget(title);
    layout->addWidget(summaryStrip_);
    layout->addLayout(contentSplit);
}

void ResultsPage::setImage(const QImage& image) {
    previewWidget_->setImage(image);
}

void ResultsPage::setSummary(const core::InferenceSummary& summary) {
    results_.clear();
    resultsList_->clear();
    resultsList_->hide();
    currentIndex_ = -1;
    exportBatchBtn_->hide();

    previewWidget_->setSummary(summary);

    if (summary.inputPath.isEmpty()) {
        summaryLabel_->setText(QStringLiteral("\u5f53\u524d\u8fd8\u6ca1\u6709\u63a8\u7406\u7ed3\u679c"));
        detectionsTable_->clearContents();
        detectionsTable_->setRowCount(0);
        return;
    }

    const QString countLabel = summary.taskType == QStringLiteral("classification")
        ? QStringLiteral("类别数：%1").arg(summary.classifications.size())
        : summary.taskType == QStringLiteral("segmentation")
            ? QStringLiteral("实例数：%1").arg(summary.segmentations.size())
            : QStringLiteral("目标数：%1").arg(summary.detectionCount);

    summaryLabel_->setText(
        QStringLiteral("模型：%1 | 图像：%2×%3 | %4 | 耗时：%5 ms")
            .arg(summary.modelName)
            .arg(summary.imageWidth)
            .arg(summary.imageHeight)
            .arg(countLabel)
            .arg(QString::number(summary.elapsedMs, 'f', 2)));

    populateTable(summary);
}

void ResultsPage::setResults(const QVector<core::InferenceSummary>& results) {
    results_ = results;
    resultsList_->clear();
    currentIndex_ = -1;

    if (results.isEmpty()) {
        resultsList_->hide();
        return;
    }

    for (int i = 0; i < results.size(); ++i) {
        const core::InferenceSummary& s = results.at(i);
        const QFileInfo info(s.inputPath);
        QString label = info.fileName();
        if (label.isEmpty()) {
            label = QStringLiteral("\u7ed3\u679c %1").arg(i + 1);
        }
        auto* item = new QListWidgetItem(label, resultsList_);
        item->setData(Qt::UserRole, i);
        item->setToolTip(s.inputPath);
    }

    resultsList_->setVisible(results.size() > 1);
    exportBatchBtn_->setVisible(results.size() > 1);
    resultsList_->setCurrentRow(0);
}

void ResultsPage::clearResults() {
    results_.clear();
    resultsList_->clear();
    resultsList_->hide();
    exportBatchBtn_->hide();
    currentIndex_ = -1;
    previewWidget_->setImage(QImage());
    previewWidget_->setSummary({});
    summaryLabel_->setText(QStringLiteral("\u5f53\u524d\u8fd8\u6ca1\u6709\u63a8\u7406\u7ed3\u679c"));
    detectionsTable_->clearContents();
    detectionsTable_->setRowCount(0);
    categoryFilter_->blockSignals(true);
    categoryFilter_->clear();
    categoryFilter_->addItem(QStringLiteral("全部类别"));
    categoryFilter_->blockSignals(false);
    currentDetections_.clear();
}

void ResultsPage::showResultAtIndex(const int index) {
    if (index < 0 || index >= results_.size()) {
        return;
    }

    currentIndex_ = index;
    const core::InferenceSummary& summary = results_.at(index);

    previewWidget_->setSummary(summary);

    const QString countLabel = summary.taskType == QStringLiteral("classification")
        ? QStringLiteral("类别数：%1").arg(summary.classifications.size())
        : summary.taskType == QStringLiteral("segmentation")
            ? QStringLiteral("实例数：%1").arg(summary.segmentations.size())
            : QStringLiteral("目标数：%1").arg(summary.detectionCount);

    summaryLabel_->setText(
        QStringLiteral("模型：%1 | 图像：%2×%3 | %4 | 耗时：%5 ms")
            .arg(summary.modelName)
            .arg(summary.imageWidth)
            .arg(summary.imageHeight)
            .arg(countLabel)
            .arg(QString::number(summary.elapsedMs, 'f', 2)));

    populateTable(summary);
}

void ResultsPage::populateTable(const core::InferenceSummary& summary) {
    if (summary.taskType == QStringLiteral("classification")) {
        populateClassificationTable(summary);
        return;
    }

    currentDetections_ = summary.detections;
    populateCategoryFilter(summary.detections);

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

void ResultsPage::populateClassificationTable(const core::InferenceSummary& summary) {
    categoryFilter_->blockSignals(true);
    categoryFilter_->clear();
    categoryFilter_->addItem(QStringLiteral("全部类别"));
    categoryFilter_->blockSignals(false);
    currentDetections_.clear();

    detectionsTable_->clearContents();
    detectionsTable_->setHorizontalHeaderLabels({
        QStringLiteral("排名"),
        QStringLiteral("标签"),
        QStringLiteral("置信度"),
        QStringLiteral(""),
    });
    detectionsTable_->setRowCount(summary.classifications.size());

    for (int i = 0; i < summary.classifications.size(); ++i) {
        const core::ClassificationItem& item = summary.classifications.at(i);
        detectionsTable_->setItem(i, 0, new QTableWidgetItem(QStringLiteral("#%1").arg(i + 1)));
        detectionsTable_->setItem(i, 1, new QTableWidgetItem(item.label));
        detectionsTable_->setItem(i, 2, new QTableWidgetItem(QStringLiteral("%1%").arg(item.confidence * 100, 0, 'f', 1)));
        detectionsTable_->setItem(i, 3, new QTableWidgetItem());
    }
}

void ResultsPage::populateCategoryFilter(const QVector<core::DetectionItem>& detections) {
    categoryFilter_->blockSignals(true);
    const QString currentSelection = categoryFilter_->currentText();
    categoryFilter_->clear();
    categoryFilter_->addItem(QStringLiteral("全部类别"));

    QSet<QString> seenLabels;
    for (const auto& item : detections) {
        if (!seenLabels.contains(item.label)) {
            seenLabels.insert(item.label);
            categoryFilter_->addItem(item.label);
        }
    }

    const int index = categoryFilter_->findText(currentSelection);
    if (index >= 0) {
        categoryFilter_->setCurrentIndex(index);
    }
    categoryFilter_->blockSignals(false);
}

void ResultsPage::applyCategoryFilter() {
    if (currentDetections_.isEmpty()) {
        return;
    }

    const QString selectedCategory = categoryFilter_->currentText();
    const bool showAll = selectedCategory == QStringLiteral("全部类别");

    QVector<core::DetectionItem> filtered;
    for (const auto& item : currentDetections_) {
        if (showAll || item.label == selectedCategory) {
            filtered.append(item);
        }
    }

    detectionsTable_->setRowCount(0);
    for (const auto& item : filtered) {
        const int row = detectionsTable_->rowCount();
        detectionsTable_->insertRow(row);
        detectionsTable_->setItem(row, 0, new QTableWidgetItem(QString::number(item.classId)));
        detectionsTable_->setItem(row, 1, new QTableWidgetItem(item.label));
        detectionsTable_->setItem(row, 2, new QTableWidgetItem(QStringLiteral("%1%").arg(item.confidence * 100, 0, 'f', 1)));
        detectionsTable_->setItem(row, 3, new QTableWidgetItem(QStringLiteral("(%1, %2, %3 x %4)")
            .arg(item.boundingBox.x(), 0, 'f', 0)
            .arg(item.boundingBox.y(), 0, 'f', 0)
            .arg(item.boundingBox.width(), 0, 'f', 0)
            .arg(item.boundingBox.height(), 0, 'f', 0)));
    }
}

}  // namespace aitoolkit::ui
