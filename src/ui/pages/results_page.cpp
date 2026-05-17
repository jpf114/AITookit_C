#include "ui/pages/results_page.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "ui/image_utils.h"
#include "ui/widgets/image_preview_widget.h"

namespace aitoolkit::ui {

namespace {

QString baseResultListLabel(const core::InferenceSummary& summary, const int index) {
    const QFileInfo info(summary.inputPath);
    const QString fileName = info.fileName();
    if (fileName.isEmpty()) {
        return QStringLiteral("结果 %1").arg(index + 1);
    }

    static const QRegularExpression kFramePattern(QStringLiteral(R"(^(.*)\s+\[(frame\s+\d+)\]$)"),
                                                  QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = kFramePattern.match(fileName);
    if (match.hasMatch()) {
        return QStringLiteral("%1 | %2").arg(match.captured(1).trimmed(), match.captured(2).trimmed());
    }

    return fileName;
}

QString uniqueResultListLabel(const QVector<core::InferenceSummary>& results,
                              const QVector<QString>& baseLabels,
                              const int targetIndex) {
    const QString& baseLabel = baseLabels.at(targetIndex);
    int sameCount = 0;
    for (const QString& label : baseLabels) {
        if (label == baseLabel) {
            ++sameCount;
        }
    }
    if (sameCount <= 1) {
        return baseLabel;
    }

    const QString normalizedPath = QDir::fromNativeSeparators(QFileInfo(results.at(targetIndex).inputPath).path());
    QStringList segments = normalizedPath.split('/', Qt::SkipEmptyParts);
    QString prefix;
    while (!segments.isEmpty()) {
        prefix = prefix.isEmpty() ? segments.takeLast() : segments.takeLast() + QStringLiteral("/") + prefix;
        const QString candidate = prefix + QStringLiteral("/") + baseLabel;

        bool hasConflict = false;
        for (int i = 0; i < results.size(); ++i) {
            if (i == targetIndex || baseLabels.at(i) != baseLabel) {
                continue;
            }

            const QString otherPath = QDir::fromNativeSeparators(QFileInfo(results.at(i).inputPath).path());
            QStringList otherSegments = otherPath.split('/', Qt::SkipEmptyParts);
            QString otherPrefix;
            int neededDepth = prefix.count('/') + 1;
            while (!otherSegments.isEmpty() && neededDepth-- > 0) {
                otherPrefix = otherPrefix.isEmpty()
                    ? otherSegments.takeLast()
                    : otherSegments.takeLast() + QStringLiteral("/") + otherPrefix;
            }
            if (otherPrefix + QStringLiteral("/") + baseLabel == candidate) {
                hasConflict = true;
                break;
            }
        }

        if (!hasConflict) {
            return candidate;
        }
    }

    return QStringLiteral("%1 (%2)").arg(baseLabel).arg(targetIndex + 1);
}

}  // namespace

QString ResultsPage::formatDetectionConfidence(const float confidence) {
    return QString::number(confidence, 'f', 3);
}

QString ResultsPage::formatBoundingBox(const QRectF& boundingBox) {
    return QStringLiteral("%1, %2, %3 x %4")
        .arg(QString::number(boundingBox.x(), 'f', 1))
        .arg(QString::number(boundingBox.y(), 'f', 1))
        .arg(QString::number(boundingBox.width(), 'f', 1))
        .arg(QString::number(boundingBox.height(), 'f', 1));
}

ResultsPage::ResultsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("\u7ed3\u679c"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    exportButton_ = new QPushButton(QStringLiteral("\u5bfc\u51fa JSON"), this);
    exportButton_->setObjectName(QStringLiteral("SecondaryButton"));

    exportImageButton_ = new QPushButton(QStringLiteral("\u5bfc\u51fa\u56fe\u7247"), this);
    exportImageButton_->setObjectName(QStringLiteral("SecondaryButton"));

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
    summaryStripLayout->addWidget(exportImageButton_, 0);
    summaryStripLayout->addWidget(exportButton_, 0);

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
    resetDetectionTableHeaders();
    detectionsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    detectionsTable_->horizontalHeader()->setStretchLastSection(true);
    detectionsTable_->verticalHeader()->setVisible(false);
    detectionsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    detectionsTable_->setSelectionMode(QAbstractItemView::NoSelection);
    detectionsTable_->setMinimumHeight(180);

    tableHeaderLayout_ = new QHBoxLayout();
    tableHeaderLayout_->setSpacing(8);
    categoryFilterLabel_ = new QLabel(QStringLiteral("类别筛选："), this);
    tableHeaderLayout_->addWidget(categoryFilterLabel_);
    tableHeaderLayout_->addWidget(categoryFilter_);
    tableHeaderLayout_->addStretch();

    rightColumn->addWidget(previewWidget_, 1);
    rightColumn->addLayout(tableHeaderLayout_);
    rightColumn->addWidget(detectionsTable_);

    contentSplit->addWidget(resultsList_);
    contentSplit->addLayout(rightColumn, 1);

    connect(exportButton_, &QPushButton::clicked, this, &ResultsPage::exportRequested);
    connect(exportImageButton_, &QPushButton::clicked, this, &ResultsPage::exportImageRequested);
    connect(exportBatchBtn_, &QPushButton::clicked, this, &ResultsPage::exportBatchJsonRequested);
    connect(resultsList_, &QListWidget::currentRowChanged, this, &ResultsPage::showResultAtIndex);

    layout->addWidget(title);
    layout->addWidget(summaryStrip_);
    layout->addLayout(contentSplit);

    setCategoryFilterVisible(true);
    updateExportActionCopy(false);
    updateExportButtonStates(false, false);
}

void ResultsPage::setImage(const QImage& image) {
    previewWidget_->setImage(image);
}

void ResultsPage::setSummary(const core::InferenceSummary& summary) {
    results_.clear();
    resultsList_->clear();
    resultsList_->hide();
    currentIndex_ = -1;
    updateBatchExportVisibility(false);
    updateExportActionCopy(false);

    previewWidget_->setImage(loadUsableImage(summary.inputPath));
    previewWidget_->setSummary(summary);

    if (summary.inputPath.isEmpty()) {
        updateExportButtonStates(false, false);
        summaryLabel_->setText(QStringLiteral("\u5f53\u524d\u8fd8\u6ca1\u6709\u63a8\u7406\u7ed3\u679c"));
        detectionsTable_->clearContents();
        detectionsTable_->setRowCount(0);
        setCategoryFilterVisible(true);
        currentDetections_.clear();
        currentSegmentations_.clear();
        currentTaskType_.clear();
        return;
    }

    updateExportButtonStates(true, !loadUsableImage(summary.inputPath).isNull());

    summaryLabel_->setText(buildSummaryText(summary));

    populateTable(summary);
}

void ResultsPage::setResults(const QVector<core::InferenceSummary>& results) {
    results_ = results;
    resultsList_->clear();
    currentIndex_ = -1;

    if (results.isEmpty()) {
        resultsList_->hide();
        updateBatchExportVisibility(false);
        updateExportActionCopy(false);
        return;
    }

    QVector<QString> baseLabels;
    baseLabels.reserve(results.size());
    for (int i = 0; i < results.size(); ++i) {
        baseLabels.push_back(baseResultListLabel(results.at(i), i));
    }

    for (int i = 0; i < results.size(); ++i) {
        const core::InferenceSummary& s = results.at(i);
        const QString label = uniqueResultListLabel(results, baseLabels, i);
        auto* item = new QListWidgetItem(label, resultsList_);
        item->setData(Qt::UserRole, i);
        item->setToolTip(s.inputPath);
    }

    const bool hasMultipleResults = results.size() > 1;
    resultsList_->setVisible(hasMultipleResults);
    updateBatchExportVisibility(hasMultipleResults);
    updateExportActionCopy(hasMultipleResults);
    resultsList_->setCurrentRow(0);
}

void ResultsPage::clearResults() {
    results_.clear();
    resultsList_->clear();
    resultsList_->hide();
    updateBatchExportVisibility(false);
    currentIndex_ = -1;
    previewWidget_->setImage(QImage());
    previewWidget_->setSummary({});
    updateExportButtonStates(false, false);
    summaryLabel_->setText(QStringLiteral("\u5f53\u524d\u8fd8\u6ca1\u6709\u63a8\u7406\u7ed3\u679c"));
    detectionsTable_->clearContents();
    detectionsTable_->setRowCount(0);
    categoryFilter_->blockSignals(true);
    categoryFilter_->clear();
    categoryFilter_->addItem(QStringLiteral("全部类别"));
    categoryFilter_->blockSignals(false);
    setCategoryFilterVisible(true);
    currentDetections_.clear();
    currentSegmentations_.clear();
    currentTaskType_.clear();
    updateExportActionCopy(false);
}

void ResultsPage::updateBatchExportVisibility(const bool hasMultipleResults) {
    if (exportBatchBtn_ == nullptr) {
        return;
    }

    exportBatchBtn_->setVisible(hasMultipleResults);
    exportBatchBtn_->setToolTip(
        hasMultipleResults ? QStringLiteral("一次导出当前批量结果的全部 JSON") : QString());
}

void ResultsPage::showResultAtIndex(const int index) {
    if (index < 0 || index >= results_.size()) {
        return;
    }

    currentIndex_ = index;
    const core::InferenceSummary& summary = results_.at(index);

    const QImage previewImage = loadUsableImage(summary.inputPath);
    previewWidget_->setImage(previewImage);
    previewWidget_->setSummary(summary);
    updateExportButtonStates(true, !previewImage.isNull());

    summaryLabel_->setText(buildSummaryText(summary));

    populateTable(summary);
    emit resultSelectionChanged(summary);
}

QString ResultsPage::buildSummaryText(const core::InferenceSummary& summary) const {
    const QString countLabel = summary.taskType == QStringLiteral("classification")
        ? QStringLiteral("类别数：%1").arg(summary.classifications.size())
        : summary.taskType == QStringLiteral("segmentation")
            ? QStringLiteral("实例数：%1").arg(summary.segmentations.size())
            : QStringLiteral("目标数：%1").arg(summary.detectionCount);

    QString positionLabel;
    if (results_.size() > 1 && currentIndex_ >= 0 && currentIndex_ < results_.size()) {
        positionLabel = QStringLiteral("第 %1 / %2 项 | ").arg(currentIndex_ + 1).arg(results_.size());
    }

    const QString sourceLabel = summary.imageWidth > 0 && summary.imageHeight > 0
        ? QStringLiteral("图像：%1×%2").arg(summary.imageWidth).arg(summary.imageHeight)
        : QStringLiteral("来源：%1").arg(buildSourceLabel(summary));

    return QStringLiteral("%1模型：%2 | %3 | %4 | 耗时：%5 ms")
        .arg(positionLabel)
        .arg(summary.modelName)
        .arg(sourceLabel)
        .arg(countLabel)
        .arg(QString::number(summary.elapsedMs, 'f', 2));
}

QString ResultsPage::buildSourceLabel(const core::InferenceSummary& summary) {
    const QFileInfo info(summary.inputPath);
    const QString fileName = info.fileName();
    if (fileName.isEmpty()) {
        return summary.inputPath;
    }

    static const QRegularExpression kFramePattern(QStringLiteral(R"(^(.*)\s+\[(frame\s+\d+)\]$)"),
                                                  QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = kFramePattern.match(fileName);
    if (match.hasMatch()) {
        return QStringLiteral("%1 | %2").arg(match.captured(1).trimmed(), match.captured(2).trimmed());
    }

    return fileName;
}

void ResultsPage::populateTable(const core::InferenceSummary& summary) {
    if (summary.taskType == QStringLiteral("classification")) {
        populateClassificationTable(summary);
        return;
    }

    currentTaskType_ = summary.taskType;
    resetDetectionTableHeaders();
    setCategoryFilterVisible(true);
    detectionsTable_->clearContents();

    if (summary.taskType == QStringLiteral("segmentation")) {
        currentDetections_.clear();
        currentSegmentations_ = summary.segmentations;

        QVector<core::DetectionItem> filterItems;
        filterItems.reserve(summary.segmentations.size());
        for (const core::SegmentationItem& item : summary.segmentations) {
            core::DetectionItem filterItem;
            filterItem.classId = item.classId;
            filterItem.label = item.label;
            filterItem.confidence = item.confidence;
            filterItem.boundingBox = item.boundingBox;
            filterItems.push_back(filterItem);
        }
        populateCategoryFilter(filterItems);

        detectionsTable_->setRowCount(summary.segmentations.size());
        for (int index = 0; index < summary.segmentations.size(); ++index) {
            const core::SegmentationItem& item = summary.segmentations.at(index);
            detectionsTable_->setItem(index, 0, new QTableWidgetItem(QString::number(item.classId)));
            detectionsTable_->setItem(index, 1, new QTableWidgetItem(item.label));
            detectionsTable_->setItem(index, 2, new QTableWidgetItem(formatDetectionConfidence(item.confidence)));
            detectionsTable_->setItem(index, 3, new QTableWidgetItem(formatBoundingBox(item.boundingBox)));
        }
        return;
    }

    currentSegmentations_.clear();
    currentDetections_ = summary.detections;
    populateCategoryFilter(summary.detections);

    detectionsTable_->setRowCount(summary.detections.size());
    for (int index = 0; index < summary.detections.size(); ++index) {
        const core::DetectionItem& detection = summary.detections.at(index);
        detectionsTable_->setItem(index, 0, new QTableWidgetItem(QString::number(detection.classId)));
        detectionsTable_->setItem(index, 1, new QTableWidgetItem(detection.label));
        detectionsTable_->setItem(index, 2, new QTableWidgetItem(formatDetectionConfidence(detection.confidence)));
        detectionsTable_->setItem(index, 3, new QTableWidgetItem(formatBoundingBox(detection.boundingBox)));
    }
}

void ResultsPage::resetDetectionTableHeaders() {
    detectionsTable_->setHorizontalHeaderLabels({
        QStringLiteral("类别"),
        QStringLiteral("标签"),
        QStringLiteral("置信度"),
        QStringLiteral("框选范围"),
    });
}

void ResultsPage::populateClassificationTable(const core::InferenceSummary& summary) {
    currentTaskType_ = summary.taskType;
    setCategoryFilterVisible(false);
    categoryFilter_->blockSignals(true);
    categoryFilter_->clear();
    categoryFilter_->addItem(QStringLiteral("全部类别"));
    categoryFilter_->blockSignals(false);
    currentDetections_.clear();
    currentSegmentations_.clear();

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

void ResultsPage::setCategoryFilterVisible(const bool visible) {
    if (categoryFilterLabel_ != nullptr) {
        categoryFilterLabel_->setVisible(visible);
    }
    if (categoryFilter_ != nullptr) {
        categoryFilter_->setVisible(visible);
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
    if (currentTaskType_ == QStringLiteral("classification")) {
        return;
    }

    const QString selectedCategory = categoryFilter_->currentText();
    const bool showAll = selectedCategory == QStringLiteral("全部类别");

    detectionsTable_->setRowCount(0);

    if (currentTaskType_ == QStringLiteral("segmentation")) {
        for (const auto& item : currentSegmentations_) {
            if (!showAll && item.label != selectedCategory) {
                continue;
            }
            const int row = detectionsTable_->rowCount();
            detectionsTable_->insertRow(row);
            detectionsTable_->setItem(row, 0, new QTableWidgetItem(QString::number(item.classId)));
            detectionsTable_->setItem(row, 1, new QTableWidgetItem(item.label));
            detectionsTable_->setItem(row, 2, new QTableWidgetItem(formatDetectionConfidence(item.confidence)));
            detectionsTable_->setItem(row, 3, new QTableWidgetItem(formatBoundingBox(item.boundingBox)));
        }
        return;
    }

    for (const auto& item : currentDetections_) {
        if (!showAll && item.label != selectedCategory) {
            continue;
        }
        const int row = detectionsTable_->rowCount();
        detectionsTable_->insertRow(row);
        detectionsTable_->setItem(row, 0, new QTableWidgetItem(QString::number(item.classId)));
        detectionsTable_->setItem(row, 1, new QTableWidgetItem(item.label));
        detectionsTable_->setItem(row, 2, new QTableWidgetItem(formatDetectionConfidence(item.confidence)));
        detectionsTable_->setItem(row, 3, new QTableWidgetItem(formatBoundingBox(item.boundingBox)));
    }
}

void ResultsPage::updateExportButtonStates(const bool hasSummary, const bool hasPreviewImage) {
    if (exportButton_ != nullptr) {
        exportButton_->setEnabled(hasSummary);
    }
    if (exportImageButton_ != nullptr) {
        exportImageButton_->setEnabled(hasSummary && hasPreviewImage);
    }
    updateExportImageTooltip(hasSummary, hasPreviewImage);
}

void ResultsPage::updateExportImageTooltip(const bool hasSummary, const bool hasPreviewImage) {
    if (exportImageButton_ == nullptr) {
        return;
    }

    if (!hasSummary) {
        exportImageButton_->setToolTip(QStringLiteral("请先完成一次推理，再导出图片。"));
        return;
    }

    if (!hasPreviewImage) {
        exportImageButton_->setToolTip(QStringLiteral("当前结果没有可导出的预览图，请先选择可读取的图像结果。"));
        return;
    }

    const bool hasMultipleResults = !results_.isEmpty() && results_.size() > 1;
    exportImageButton_->setToolTip(
        hasMultipleResults ? QStringLiteral("导出当前选中结果的渲染图片")
                           : QStringLiteral("导出当前结果的渲染图片"));
}

void ResultsPage::updateExportActionCopy(const bool hasMultipleResults) {
    if (exportButton_ != nullptr) {
        exportButton_->setText(hasMultipleResults ? QStringLiteral("导出当前 JSON") : QStringLiteral("导出 JSON"));
        exportButton_->setToolTip(
            hasMultipleResults ? QStringLiteral("导出当前选中结果的 JSON") : QStringLiteral("导出当前结果的 JSON"));
    }
    if (exportImageButton_ != nullptr) {
        exportImageButton_->setText(hasMultipleResults ? QStringLiteral("导出当前图片") : QStringLiteral("导出图片"));
    }
    if (exportBatchBtn_ != nullptr) {
        exportBatchBtn_->setText(QStringLiteral("导出全部 JSON"));
    }
}

}  // namespace aitoolkit::ui
