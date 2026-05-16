#include "runtime/onnx_plugin.h"

#include <stdexcept>

#include "models/classification_model.h"
#include "models/inference_backend.h"
#include "models/segmentation_model.h"
#include "models/yolo_detection_model.h"
#include "runtime/backend_registry.h"

namespace aitoolkit::runtime {

BackendInfo OnnxRuntimePlugin::info() const {
    BackendInfo info;
    info.name = QStringLiteral("onnxruntime");
    info.displayName = QStringLiteral("ONNX Runtime");
    info.version = QStringLiteral("1.x");
    info.supportsGPU = false;
    info.isAvailable = true;
    return info;
}

QStringList OnnxRuntimePlugin::supportedTaskTypes() const {
    return {
        QStringLiteral("detection"),
        QStringLiteral("classification"),
        QStringLiteral("segmentation")
    };
}

std::unique_ptr<models::InferenceBackend> OnnxRuntimePlugin::createModel(
    const core::ModelManifest& manifest,
    const int threadCount) const {
    if (manifest.taskType == QStringLiteral("detection")) {
        return std::make_unique<models::YoloDetectionModel>(manifest, threadCount);
    }
    if (manifest.taskType == QStringLiteral("classification")) {
        return std::make_unique<models::ClassificationModel>(manifest, threadCount);
    }
    if (manifest.taskType == QStringLiteral("segmentation")) {
        return std::make_unique<models::SegmentationModel>(manifest, threadCount);
    }
    throw std::runtime_error(
        QStringLiteral("ONNX Runtime plugin does not support task type: %1")
            .arg(manifest.taskType).toStdString());
}

void registerOnnxRuntimePlugin() {
    BackendRegistry::instance().registerBackend(
        std::make_unique<OnnxRuntimePlugin>());
}

}  // namespace aitoolkit::runtime
