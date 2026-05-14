#pragma once

#include <QImage>
#include <QWidget>

#include "core/types.h"

class QLabel;
class QTableWidget;

namespace aitoolkit::ui {

class ImagePreviewWidget;

class ResultsPage : public QWidget {
    Q_OBJECT

public:
    explicit ResultsPage(QWidget* parent = nullptr);

    void setImage(const QImage& image);
    void setSummary(const core::InferenceSummary& summary);

signals:
    void exportRequested();
    void exportImageRequested();

private:
    QLabel* summaryLabel_ = nullptr;
    QWidget* summaryStrip_ = nullptr;
    ImagePreviewWidget* previewWidget_ = nullptr;
    QTableWidget* detectionsTable_ = nullptr;
};

}  // namespace aitoolkit::ui
