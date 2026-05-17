#pragma once

#include <QImage>
#include <QWidget>

#include "core/types.h"

class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QTableWidget;
class QHBoxLayout;

namespace aitoolkit::ui {

class ImagePreviewWidget;

class ResultsPage : public QWidget {
    Q_OBJECT

public:
    explicit ResultsPage(QWidget* parent = nullptr);

    void setImage(const QImage& image);
    void setSummary(const core::InferenceSummary& summary);
    void setResults(const QVector<core::InferenceSummary>& results);
    void clearResults();

signals:
    void exportRequested();
    void exportImageRequested();
    void exportBatchJsonRequested();
    void resultSelectionChanged(const core::InferenceSummary& summary);

private:
    QString buildSummaryText(const core::InferenceSummary& summary) const;
    static QString buildSourceLabel(const core::InferenceSummary& summary);
    void updateBatchExportVisibility(bool hasMultipleResults);
    void updateExportImageTooltip(bool hasSummary, bool hasPreviewImage);
    void updateExportActionCopy(bool hasMultipleResults);
    void updateExportButtonStates(bool hasSummary, bool hasPreviewImage);
    void showResultAtIndex(int index);
    void populateTable(const core::InferenceSummary& summary);
    void populateClassificationTable(const core::InferenceSummary& summary);
    void resetDetectionTableHeaders();
    void populateCategoryFilter(const QVector<core::DetectionItem>& detections);
    void applyCategoryFilter();
    void setCategoryFilterVisible(bool visible);
    static QString formatDetectionConfidence(float confidence);
    static QString formatBoundingBox(const QRectF& boundingBox);

    QLabel* summaryLabel_ = nullptr;
    QWidget* summaryStrip_ = nullptr;
    ImagePreviewWidget* previewWidget_ = nullptr;
    QPushButton* exportButton_ = nullptr;
    QPushButton* exportImageButton_ = nullptr;
    QTableWidget* detectionsTable_ = nullptr;
    QListWidget* resultsList_ = nullptr;
    QComboBox* categoryFilter_ = nullptr;
    QLabel* categoryFilterLabel_ = nullptr;
    QHBoxLayout* tableHeaderLayout_ = nullptr;
    QPushButton* exportBatchBtn_ = nullptr;
    QVector<core::InferenceSummary> results_;
    QVector<core::DetectionItem> currentDetections_;
    QVector<core::SegmentationItem> currentSegmentations_;
    QString currentTaskType_;
    int currentIndex_ = -1;
};

}  // namespace aitoolkit::ui
