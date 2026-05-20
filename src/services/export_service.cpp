#include "services/export_service.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPen>
#include <QSaveFile>

#include "core/image_algorithms.h"
#include "core/json_utils.h"

namespace aitoolkit::services {

QJsonObject ExportService::summaryToJson(const core::InferenceSummary& summary) const {
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

    if (!summary.classifications.isEmpty()) {
        QJsonArray classifications;
        for (const core::ClassificationItem& item : summary.classifications) {
            QJsonObject obj;
            obj.insert(QStringLiteral("class_id"), item.classId);
            obj.insert(QStringLiteral("label"), item.label);
            obj.insert(QStringLiteral("confidence"), item.confidence);
            classifications.append(obj);
        }
        root.insert(QStringLiteral("classifications"), classifications);
    }

    if (!summary.segmentations.isEmpty()) {
        QJsonArray segmentations;
        for (const core::SegmentationItem& item : summary.segmentations) {
            QJsonObject boundingBox;
            boundingBox.insert(QStringLiteral("x"), item.boundingBox.x());
            boundingBox.insert(QStringLiteral("y"), item.boundingBox.y());
            boundingBox.insert(QStringLiteral("width"), item.boundingBox.width());
            boundingBox.insert(QStringLiteral("height"), item.boundingBox.height());

            QJsonObject obj;
            obj.insert(QStringLiteral("class_id"), item.classId);
            obj.insert(QStringLiteral("label"), item.label);
            obj.insert(QStringLiteral("confidence"), item.confidence);
            obj.insert(QStringLiteral("bounding_box"), boundingBox);
            segmentations.append(obj);
        }
        root.insert(QStringLiteral("segmentations"), segmentations);
    }

    if (!summary.taskType.isEmpty()) {
        root.insert(QStringLiteral("task_type"), summary.taskType);
    }

    if (!summary.success) {
        root.insert(QStringLiteral("success"), false);
        if (!summary.errorMessage.isEmpty()) {
            root.insert(QStringLiteral("error_message"), summary.errorMessage);
        }
    }

    return root;
}

void ExportService::exportJson(const QString& filePath, const core::InferenceSummary& summary) const {
    core::writeJsonObject(filePath, summaryToJson(summary));
}

bool ExportService::exportBatchJson(const QVector<core::InferenceSummary>& results, const QString& outputPath) const {
    QJsonArray array;
    for (const core::InferenceSummary& summary : results) {
        array.append(summaryToJson(summary));
    }

    QJsonDocument doc(array);
    QSaveFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(doc.toJson());
    return file.commit();
}

bool ExportService::exportRenderedImage(
    const QString& filePath,
    const QImage& image,
    const core::InferenceSummary& summary) const {
    if (image.isNull()) {
        return false;
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

    for (const core::SegmentationItem& item : summary.segmentations) {
        const QColor color = item.renderColor.isValid() ? item.renderColor : QColor(QStringLiteral("#3b82f6"));
        boxPen.setColor(color);
        painter.setPen(boxPen);
        painter.drawRect(item.boundingBox);

        if (!item.mask.isNull()) {
            QColor overlayColor = color;
            overlayColor.setAlpha(80);
            painter.drawImage(
                item.boundingBox,
                core::colorizeMask(
                    item.mask,
                    QSize(
                        static_cast<int>(item.boundingBox.width()),
                        static_cast<int>(item.boundingBox.height())),
                    overlayColor));
        }

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

    return rendered.save(filePath);
}

}  // namespace aitoolkit::services
