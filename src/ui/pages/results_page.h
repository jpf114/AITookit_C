#pragma once

#include <QImage>
#include <QWidget>

#include "core/types.h"

class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QTableWidget;

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

private:
    void showResultAtIndex(int index);
    void populateTable(const core::InferenceSummary& summary);
    void populateClassificationTable(const core::InferenceSummary& summary);
    void populateCategoryFilter(const QVector<core::DetectionItem>& detections);
    void applyCategoryFilter();

    QLabel* summaryLabel_ = nullptr;
    QWidget* summaryStrip_ = nullptr;
    ImagePreviewWidget* previewWidget_ = nullptr;
    QTableWidget* detectionsTable_ = nullptr;
    QListWidget* resultsList_ = nullptr;
    QComboBox* categoryFilter_ = nullptr;
    QPushButton* exportBatchBtn_ = nullptr;
    QVector<core::InferenceSummary> results_;
    QVector<core::DetectionItem> currentDetections_;
    int currentIndex_ = -1;
};

}  // namespace aitoolkit::ui
