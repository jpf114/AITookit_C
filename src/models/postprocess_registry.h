#pragma once

#include <QSize>
#include <QVector>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/model_manifest.h"
#include "core/types.h"
#include "runtime/inference_backend.h"

namespace aitoolkit::models {

struct PostprocessInput {
    std::vector<runtime::InferenceTensor> tensors;
    QSize networkSize;
    QSize originalSize;
    double confidenceThreshold;
    double nmsThreshold;
    const core::ModelManifest& manifest;
};

using PostprocessFn = std::function<QVector<core::DetectionItem>(const PostprocessInput&)>;

class PostprocessRegistry {
public:
    static PostprocessRegistry& instance();

    void registerDecoder(const std::string& decoderName, PostprocessFn fn);
    PostprocessFn getDecoder(const std::string& decoderName) const;
    bool hasDecoder(const std::string& decoderName) const;

private:
    PostprocessRegistry() = default;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace aitoolkit::models
