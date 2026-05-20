#include "ui/widgets/image_preview_widget.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QWheelEvent>

#include "ui/image_utils.h"

namespace aitoolkit::ui {

ImagePreviewWidget::ImagePreviewWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName(QStringLiteral("ImagePreviewWidget"));
    setAutoFillBackground(true);
    setMouseTracking(true);
}

void ImagePreviewWidget::setImage(const QImage& image) {
    image_ = image;
    resetView();
}

void ImagePreviewWidget::setSummary(const core::InferenceSummary& summary) {
    summary_ = summary;
    update();
}

void ImagePreviewWidget::resetView() {
    zoomLevel_ = 1.0;
    panOffset_ = QPointF();
    update();
}

void ImagePreviewWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), palette().window().color());

    if (image_.isNull()) {
        painter.setPen(palette().windowText().color());
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("暂无可预览结果"));
        return;
    }

    const QRectF targetRect = scaledImageRect();
    painter.drawImage(targetRect, image_);

    const double xScale = targetRect.width() / static_cast<double>(image_.width());
    const double yScale = targetRect.height() / static_cast<double>(image_.height());

    for (const core::DetectionItem& item : summary_.detections) {
        const QColor color = item.renderColor.isValid() ? item.renderColor : QColor(QStringLiteral("#ef4444"));
        QRectF box(
            targetRect.left() + (item.boundingBox.left() * xScale),
            targetRect.top() + (item.boundingBox.top() * yScale),
            item.boundingBox.width() * xScale,
            item.boundingBox.height() * yScale);

        painter.setPen(QPen(color, 2.0));
        painter.drawRect(box);

        const QString labelText = QStringLiteral("%1 %2%")
            .arg(item.label)
            .arg(QString::number(item.confidence * 100.0, 'f', 0));

        QRectF labelRect = painter.fontMetrics().boundingRect(labelText);
        labelRect.moveTopLeft(box.topLeft() + QPointF(2.0, -labelRect.height() - 2.0));

        painter.fillRect(labelRect.adjusted(-2, -1, 2, 1), color);
        painter.setPen(QColor(QStringLiteral("#ffffff")));
        painter.drawText(labelRect, labelText);
        painter.setPen(QPen(color, 2.0));
    }

    for (const core::SegmentationItem& item : summary_.segmentations) {
        const QColor color = item.renderColor.isValid() ? item.renderColor : QColor(QStringLiteral("#3b82f6"));

        QRectF box(
            targetRect.left() + (item.boundingBox.left() * xScale),
            targetRect.top() + (item.boundingBox.top() * yScale),
            item.boundingBox.width() * xScale,
            item.boundingBox.height() * yScale);

        painter.setPen(QPen(color, 2.0));
        painter.drawRect(box);

        if (!item.mask.isNull()) {
            QColor overlayColor = color;
            overlayColor.setAlpha(80);
            painter.drawImage(
                box,
                colorizeMask(
                    item.mask,
                    QSize(static_cast<int>(box.width()), static_cast<int>(box.height())),
                    overlayColor));
        }

        const QString labelText = QStringLiteral("%1 %2%")
            .arg(item.label)
            .arg(QString::number(item.confidence * 100.0, 'f', 0));

        QRectF labelRect = painter.fontMetrics().boundingRect(labelText);
        labelRect.moveTopLeft(box.topLeft() + QPointF(2.0, -labelRect.height() - 2.0));

        painter.fillRect(labelRect.adjusted(-2, -1, 2, 1), color);
        painter.setPen(QColor(QStringLiteral("#ffffff")));
        painter.drawText(labelRect, labelText);
        painter.setPen(QPen(color, 2.0));
    }

    if (zoomLevel_ > 1.0001) {
        painter.setPen(QColor(QStringLiteral("#7e92a8")));
        painter.setBrush(Qt::NoBrush);
        const QFont oldFont = painter.font();
        QFont smallFont = oldFont;
        smallFont.setPointSize(oldFont.pointSize() - 1);
        painter.setFont(smallFont);
        painter.drawText(
            rect().adjusted(8, 8, -8, -8),
            Qt::AlignBottom | Qt::AlignLeft,
            QStringLiteral("%1%  双击重置").arg(static_cast<int>(zoomLevel_ * 100)));
    }
}

void ImagePreviewWidget::wheelEvent(QWheelEvent* event) {
    if (image_.isNull()) {
        return;
    }

    const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    const double newZoom = zoomLevel_ * factor;

    if (newZoom < 0.1 || newZoom > 50.0) {
        return;
    }

    const QPointF mousePos = event->position();
    const QPointF imagePoint = (mousePos - panOffset_) / zoomLevel_;

    zoomLevel_ = newZoom;

    panOffset_ = mousePos - imagePoint * zoomLevel_;
    update();

    event->accept();
}

void ImagePreviewWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && !image_.isNull()) {
        panning_ = true;
        lastPanPos_ = event->position();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
}

void ImagePreviewWidget::mouseMoveEvent(QMouseEvent* event) {
    if (panning_) {
        const QPointF delta = event->position() - lastPanPos_;
        panOffset_ += delta;
        lastPanPos_ = event->position();
        update();
        event->accept();
    }
}

void ImagePreviewWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && panning_) {
        panning_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    }
}

void ImagePreviewWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        resetView();
        event->accept();
    }
}

QRectF ImagePreviewWidget::fitRect() const {
    if (image_.isNull()) {
        return {};
    }

    const QSizeF available = rect().adjusted(12, 12, -12, -12).size();
    const QSizeF sourceSize = image_.size();
    const double scale = std::min(available.width() / sourceSize.width(), available.height() / sourceSize.height());
    const QSizeF scaledSize(sourceSize.width() * scale, sourceSize.height() * scale);

    return QRectF(
        (width() - scaledSize.width()) * 0.5,
        (height() - scaledSize.height()) * 0.5,
        scaledSize.width(),
        scaledSize.height());
}

QRectF ImagePreviewWidget::scaledImageRect() const {
    if (image_.isNull()) {
        return {};
    }

    const QRectF base = fitRect();
    const QSizeF sourceSize = image_.size();
    const double baseScale = std::min(base.width() / sourceSize.width(), base.height() / sourceSize.height());
    const double totalScale = baseScale * zoomLevel_;

    const QSizeF scaledSize(sourceSize.width() * totalScale, sourceSize.height() * totalScale);
    const QPointF center = rect().center() + panOffset_;

    return QRectF(
        center.x() - scaledSize.width() * 0.5,
        center.y() - scaledSize.height() * 0.5,
        scaledSize.width(),
        scaledSize.height());
}

}  // namespace aitoolkit::ui
