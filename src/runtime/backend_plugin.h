#pragma once

#include <QString>
#include <QStringList>

#include <memory>

#include "core/model_manifest.h"

namespace aitoolkit::models {
class InferenceBackend;
}

namespace aitoolkit::runtime {

struct BackendInfo {
    QString name;
    QString displayName;
    QString version;
    bool supportsGPU = false;
    bool isAvailable = false;
};

class BackendPlugin {
public:
    virtual ~BackendPlugin() = default;

    virtual BackendInfo info() const = 0;
    virtual QStringList supportedTaskTypes() const = 0;
    virtual std::unique_ptr<models::InferenceBackend> createModel(
        const core::ModelManifest& manifest,
        int threadCount = 1) const = 0;
};

}  // namespace aitoolkit::runtime
