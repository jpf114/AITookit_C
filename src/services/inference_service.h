#pragma once

#include <QString>
#include <QVector>

#include "core/types.h"
#include "models/yolo_detection_model.h"

namespace aitoolkit::services {

class InferenceService {
public:
    core::InferenceSummary runImage(
        const models::YoloDetectionModel& model,
        const QString& imagePath) const;

    QVector<core::InferenceSummary> runBatch(
        const models::YoloDetectionModel& model,
        const QStringList& imagePaths) const;

    QVector<core::InferenceSummary> runVideo(
        const models::YoloDetectionModel& model,
        const QString& videoPath,
        int maxFrames = 0) const;
};

}  // namespace aitoolkit::services
