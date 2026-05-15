#pragma once

#include <QImage>
#include <QPointF>
#include <QWidget>

#include "core/types.h"

namespace aitoolkit::ui {

class ImagePreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImagePreviewWidget(QWidget* parent = nullptr);

    void setImage(const QImage& image);
    void setSummary(const core::InferenceSummary& summary);

    void resetView();

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    QRectF fitRect() const;
    QRectF scaledImageRect() const;

    QImage image_;
    core::InferenceSummary summary_;

    double zoomLevel_ = 1.0;
    QPointF panOffset_;
    bool panning_ = false;
    QPointF lastPanPos_;
};

}  // namespace aitoolkit::ui
