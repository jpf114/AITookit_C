#pragma once

#include <QString>

#include <memory>

#include "core/model_manifest.h"
#include "models/yolo_detection_model.h"

namespace aitoolkit::services {

class ModelService {
public:
    core::ModelManifest loadManifest(const QString& manifestPath) const;
    std::unique_ptr<models::YoloDetectionModel> loadDetectionModel(const QString& manifestPath) const;
};

}  // namespace aitoolkit::services
