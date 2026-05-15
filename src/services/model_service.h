#pragma once

#include <QString>
#include <QStringList>

#include <memory>

#include "core/model_manifest.h"
#include "models/inference_backend.h"

namespace aitoolkit::services {

class ModelService {
public:
    core::ModelManifest loadManifest(const QString& manifestPath) const;
    std::unique_ptr<models::InferenceBackend> loadDetectionModel(const QString& manifestPath) const;
    core::ModelManifest createManifestFromOnnx(
        const QString& onnxPath,
        const QString& modelName,
        int inputWidth,
        int inputHeight,
        double confidenceThreshold,
        double nmsThreshold,
        const QStringList& labels) const;
};

}  // namespace aitoolkit::services
