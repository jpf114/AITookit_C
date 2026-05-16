#pragma once

#include <QSize>
#include <QVector>

#include <opencv2/core.hpp>

#include <vector>

#include "core/model_manifest.h"
#include "core/types.h"
#include "models/inference_backend.h"
#include "runtime/onnx_backend.h"

namespace aitoolkit::models {

class ClassificationModel : public InferenceBackend {
public:
    explicit ClassificationModel(core::ModelManifest manifest, int threadCount = 1, bool useGPU = false);

    const core::ModelManifest& manifest() const noexcept override;
    QVector<core::DetectionItem> detect(
        const cv::Mat& image,
        double confidenceThreshold = -1.0,
        double nmsThreshold = -1.0) const override;
    QVector<core::ClassificationItem> classify(
        const cv::Mat& image,
        double confidenceThreshold = -1.0) const override;
    QString backendName() const noexcept override;

private:
    static std::vector<float> preprocessImage(
        const cv::Mat& image,
        const core::ModelManifest& manifest);
    static QVector<core::ClassificationItem> postprocessClassifications(
        const std::vector<runtime::InferenceTensor>& tensors,
        const core::ModelManifest& manifest,
        double confidenceThreshold);

    core::ModelManifest manifest_;
    runtime::OnnxBackend backend_;
};

}  // namespace aitoolkit::models
