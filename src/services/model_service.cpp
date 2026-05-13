#include "services/model_service.h"

#include <stdexcept>

namespace aitoolkit::services {

namespace {

void validateDetectionManifest(const aitoolkit::core::ModelManifest& manifest) {
    if (manifest.taskType.compare(QStringLiteral("detection"), Qt::CaseInsensitive) != 0) {
        throw std::runtime_error("Only detection manifests are supported");
    }
    if (manifest.backendType.compare(QStringLiteral("onnxruntime"), Qt::CaseInsensitive) != 0) {
        throw std::runtime_error("Only onnxruntime manifests are supported");
    }
}

}  // namespace

core::ModelManifest ModelService::loadManifest(const QString& manifestPath) const {
    return core::loadModelManifest(manifestPath);
}

std::unique_ptr<models::YoloDetectionModel> ModelService::loadDetectionModel(const QString& manifestPath) const {
    const core::ModelManifest manifest = loadManifest(manifestPath);
    validateDetectionManifest(manifest);
    return std::make_unique<models::YoloDetectionModel>(manifest);
}

}  // namespace aitoolkit::services
