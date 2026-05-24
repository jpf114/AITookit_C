#include "runtime/tensorrt_plugin.h"

#include <stdexcept>

#include "runtime/backend_registry.h"

namespace aitoolkit::runtime {

BackendInfo TensorRtPlugin::info() const {
    return {
        QStringLiteral("tensorrt"),
        QStringLiteral("TensorRT"),
#ifdef AI_TENSORRT_ENABLED
        QStringLiteral("0.0.0-dev"),
#else
        QStringLiteral("0.0.0-stub"),
#endif
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
#ifdef AI_TENSORRT_ENABLED
        "TensorRT backend is not yet implemented. Build with TensorRT SDK linked or use ONNX Runtime.");
#else
        "TensorRT backend is not enabled. Configure with -DAI_ENABLE_TENSORRT=ON and link TensorRT SDK, "
        "or use ONNX Runtime.");
#endif
}

void registerTensorRtPlugin() {
    BackendRegistry::instance().registerBackend(std::make_unique<TensorRtPlugin>());
}

}  // namespace aitoolkit::runtime
