#pragma once

#include <QImage>
#include <QWidget>

#include "core/types.h"

class QLabel;
class QListWidget;
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

private:
    void showResultAtIndex(int index);
    void populateTable(const core::InferenceSummary& summary);

    QLabel* summaryLabel_ = nullptr;
    QWidget* summaryStrip_ = nullptr;
    ImagePreviewWidget* previewWidget_ = nullptr;
    QTableWidget* detectionsTable_ = nullptr;
    QListWidget* resultsList_ = nullptr;
    QVector<core::InferenceSummary> results_;
    int currentIndex_ = -1;
};

}  // namespace aitoolkit::ui
