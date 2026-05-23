#include "runtime/openvino_plugin.h"

#include <stdexcept>

#include "runtime/backend_registry.h"

namespace aitoolkit::runtime {

BackendInfo OpenVinoPlugin::info() const {
    return {
        QStringLiteral("openvino"),
        QStringLiteral("OpenVINO"),
        QStringLiteral("0.0.0-stub"),
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
        "OpenVINO backend is not yet available. Build with future OpenVINO support or use ONNX Runtime.");
}

void registerOpenVinoPlugin() {
    BackendRegistry::instance().registerBackend(std::make_unique<OpenVinoPlugin>());
}

}  // namespace aitoolkit::runtime
