#pragma once

#include "runtime/backend_plugin.h"

namespace aitoolkit::runtime {

class TensorRtPlugin : public BackendPlugin {
public:
    BackendInfo info() const override;
    QStringList supportedTaskTypes() const override;
    std::unique_ptr<models::InferenceBackend> createModel(
        const core::ModelManifest& manifest,
        int threadCount = 1,
        bool useGPU = false) const override;
};

void registerTensorRtPlugin();

}  // namespace aitoolkit::runtime
