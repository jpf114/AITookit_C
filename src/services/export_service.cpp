#include "services/export_service.h"

#include <QJsonArray>
#include <QJsonObject>

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
    root.insert(QStringLiteral("detection_count"), summary.detectionCount);
    root.insert(QStringLiteral("elapsed_ms"), summary.elapsedMs);
    root.insert(QStringLiteral("detections"), detections);
    core::writeJsonObject(filePath, root);
}

}  // namespace aitoolkit::services
