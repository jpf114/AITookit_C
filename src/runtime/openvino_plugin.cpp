#include "runtime/openvino_plugin.h"

#include <stdexcept>

#include "runtime/backend_registry.h"

namespace aitoolkit::runtime {

BackendInfo OpenVinoPlugin::info() const {
    return {
        QStringLiteral("openvino"),
        QStringLiteral("OpenVINO"),
#ifdef AI_OPENVINO_ENABLED
        QStringLiteral("0.0.0-dev"),
#else
        QStringLiteral("0.0.0-stub"),
#endif
        true,
        false,
    };
}

QStringList OpenVinoPlugin::supportedTaskTypes() const {
    return {
        QStringLiteral("detection"),
        QStringLiteral("classification"),
        QStringLiteral("segmentation"),
    };
}

std::unique_ptr<models::InferenceBackend> OpenVinoPlugin::createModel(
    const core::ModelManifest&,
    int,
    bool) const {
    throw std::runtime_error(
#ifdef AI_OPENVINO_ENABLED
        "OpenVINO backend is not yet implemented. Build with OpenVINO SDK linked or use ONNX Runtime.");
#else
        "OpenVINO backend is not enabled. Configure with -DAI_ENABLE_OPENVINO=ON and link OpenVINO SDK, "
        "or use ONNX Runtime.");
#endif
}

void registerOpenVinoPlugin() {
    BackendRegistry::instance().registerBackend(std::make_unique<OpenVinoPlugin>());
}

}  // namespace aitoolkit::runtime
