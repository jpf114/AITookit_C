#include "runtime/tensorrt_plugin.h"

#include <stdexcept>

#include "runtime/backend_registry.h"

namespace aitoolkit::runtime {

BackendInfo TensorRtPlugin::info() const {
    return {
        QStringLiteral("tensorrt"),
        QStringLiteral("TensorRT"),
        QStringLiteral("0.0.0-stub"),
        true,
        false,
    };
}

QStringList TensorRtPlugin::supportedTaskTypes() const {
    return {
        QStringLiteral("detection"),
        QStringLiteral("classification"),
        QStringLiteral("segmentation"),
    };
}

std::unique_ptr<models::InferenceBackend> TensorRtPlugin::createModel(
    const core::ModelManifest&,
    int,
    bool) const {
    throw std::runtime_error(
        "TensorRT backend is not yet available. Build with future TensorRT support or use ONNX Runtime.");
}

void registerTensorRtPlugin() {
    BackendRegistry::instance().registerBackend(std::make_unique<TensorRtPlugin>());
}

}  // namespace aitoolkit::runtime
