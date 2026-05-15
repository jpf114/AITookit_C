#pragma once

#include <QVector>

#include <opencv2/core.hpp>

#include "core/model_manifest.h"
#include "core/types.h"

namespace aitoolkit::models {

class InferenceBackend {
public:
    virtual ~InferenceBackend() = default;

    virtual const core::ModelManifest& manifest() const noexcept = 0;
    virtual QVector<core::DetectionItem> detect(
        const cv::Mat& image,
        double confidenceThreshold = -1.0,
        double nmsThreshold = -1.0) const = 0;
};

}  // namespace aitoolkit::models
