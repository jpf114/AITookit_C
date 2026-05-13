#pragma once

#include <QString>

#include "core/types.h"
#include "models/yolo_detection_model.h"

namespace aitoolkit::services {

class InferenceService {
public:
    core::InferenceSummary runImage(
        const models::YoloDetectionModel& model,
        const QString& imagePath) const;
};

}  // namespace aitoolkit::services
