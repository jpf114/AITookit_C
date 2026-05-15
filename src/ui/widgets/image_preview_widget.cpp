#include "ui/widgets/image_preview_widget.h"

#include <QPainter>
#include <QPen>

namespace aitoolkit::ui {

ImagePreviewWidget::ImagePreviewWidget(QWidget* parent)
    : QWidget(parent) {
    setAutoFillBackground(true);
}

void ImagePreviewWidget::setImage(const QImage& image) {
    image_ = image;
    update();
}

void ImagePreviewWidget::setSummary(const core::InferenceSummary& summary) {
    summary_ = summary;
    update();
}

void ImagePreviewWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(QStringLiteral("#f8fafc")));

    if (image_.isNull()) {
        painter.setPen(QColor(QStringLiteral("#64748b")));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("暂无可预览结果"));
        return;
    }

    const QRectF targetRect = imageTargetRect();
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
}

QRectF ImagePreviewWidget::imageTargetRect() const {
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

}  // namespace aitoolkit::ui
