#pragma once

#include <QSize>
#include <QVector>

#include <opencv2/core.hpp>

#include <vector>

#include "core/model_manifest.h"
#include "core/types.h"
#include "runtime/onnx_backend.h"

namespace aitoolkit::models {

struct YoloPreprocessResult {
    std::vector<float> blob;
    QSize originalSize;
    QSize networkSize;
};

class YoloDetectionModel {
public:
    explicit YoloDetectionModel(core::ModelManifest manifest);

    const core::ModelManifest& manifest() const noexcept;
    QVector<core::DetectionItem> detect(const cv::Mat& image) const;

    static YoloPreprocessResult preprocessImage(
        const cv::Mat& image,
        const core::ModelManifest& manifest);
    static cv::Mat tensorToDetectionMatrix(
        const runtime::OnnxTensor& tensor,
        int expectedNumClasses = -1);
    static QVector<core::DetectionItem> postprocessDetections(
        const cv::Mat& output,
        const QSize& networkSize,
        const core::ModelManifest& manifest,
        const QSize& originalSize);

private:
    core::ModelManifest manifest_;
    runtime::OnnxBackend backend_;
};

}  // namespace aitoolkit::models
