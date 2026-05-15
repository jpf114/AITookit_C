#include "services/export_service.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QPen>

#include "core/json_utils.h"

namespace aitoolkit::services {

void ExportService::exportJson(const QString& filePath, const core::InferenceSummary& summary) const {
    QJsonArray detections;
    for (const core::DetectionItem& detection : summary.detections) {
        QJsonObject boundingBox;
        boundingBox.insert(QStringLiteral("x"), detection.boundingBox.x());
        boundingBox.insert(QStringLiteral("y"), detection.boundingBox.y());
        boundingBox.insert(QStringLiteral("width"), detection.boundingBox.width());
        boundingBox.insert(QStringLiteral("height"), detection.boundingBox.height());

        QJsonObject detectionObject;
        detectionObject.insert(QStringLiteral("class_id"), detection.classId);
        detectionObject.insert(QStringLiteral("label"), detection.label);
        detectionObject.insert(QStringLiteral("confidence"), detection.confidence);
        detectionObject.insert(QStringLiteral("bounding_box"), boundingBox);
        detections.append(detectionObject);
    }

    QJsonObject root;
    root.insert(QStringLiteral("model_name"), summary.modelName);
    root.insert(QStringLiteral("input_path"), summary.inputPath);
    root.insert(QStringLiteral("image_width"), summary.imageWidth);
    root.insert(QStringLiteral("image_height"), summary.imageHeight);
    root.insert(QStringLiteral("detection_count"), summary.detectionCount);
    root.insert(QStringLiteral("elapsed_ms"), summary.elapsedMs);
    root.insert(QStringLiteral("detections"), detections);
    core::writeJsonObject(filePath, root);
}

void ExportService::exportRenderedImage(
    const QString& filePath,
    const QImage& image,
    const core::InferenceSummary& summary) const {
    if (image.isNull()) {
        throw std::runtime_error("Cannot export rendered image: source image is empty");
    }

    QImage rendered = image.convertToFormat(QImage::Format_RGB32);
    QPainter painter(&rendered);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen boxPen(QColor(QStringLiteral("#ef4444")), 3.0);

    QFont labelFont = painter.font();
    labelFont.setPointSize(12);
    labelFont.setBold(true);
    painter.setFont(labelFont);

    for (const core::DetectionItem& item : summary.detections) {
        const QColor color = item.renderColor.isValid() ? item.renderColor : QColor(QStringLiteral("#ef4444"));
        boxPen.setColor(color);
        painter.setPen(boxPen);
        painter.drawRect(item.boundingBox);

        const QString labelText = QStringLiteral("%1 %2%")
            .arg(item.label)
            .arg(QString::number(item.confidence * 100.0, 'f', 0));

        QRectF labelRect = painter.fontMetrics().boundingRect(labelText);
        labelRect.moveTopLeft(item.boundingBox.topLeft() + QPointF(2.0, -labelRect.height() - 2.0));

        painter.fillRect(labelRect.adjusted(-2, -1, 2, 1), color);
        painter.setPen(QColor(QStringLiteral("#ffffff")));
        painter.drawText(labelRect, labelText);
    }

    painter.end();

    if (!rendered.save(filePath)) {
        throw std::runtime_error("Failed to save rendered image");
    }
}

}  // namespace aitoolkit::services
