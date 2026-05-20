#include "models/classification_model.h"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <numeric>
#include <stdexcept>

#include "models/yolo_detection_model.h"

namespace aitoolkit::models {

ClassificationModel::ClassificationModel(core::ModelManifest manifest, const int threadCount, const bool useGPU)
    : manifest_(std::move(manifest))
    , backend_(manifest_.modelPath, threadCount, useGPU) {
    if (manifest_.inputWidth <= 0 || manifest_.inputHeight <= 0) {
        throw std::runtime_error("Classification model input dimensions must be greater than zero");
    }
    backend_.warmup();
}

const core::ModelManifest& ClassificationModel::manifest() const noexcept {
    return manifest_;
}

QVector<core::DetectionItem> ClassificationModel::detect(
    const cv::Mat&, double, double) const {
    return {};
}

QVector<core::ClassificationItem> ClassificationModel::classify(
    const cv::Mat& image, const double confidenceThreshold) const {
    const auto blob = preprocessImage(image, manifest_);
    const auto inputShape = backend_.inputShape();
    const auto outputs = backend_.run(blob, inputShape);

    const double threshold = confidenceThreshold < 0
        ? manifest_.confidenceThreshold
        : confidenceThreshold;

    return postprocessClassifications(outputs, manifest_, threshold);
}

QString ClassificationModel::backendName() const noexcept {
    return QStringLiteral("ONNX Runtime");
}

std::vector<float> ClassificationModel::preprocessImage(
    const cv::Mat& image,
    const core::ModelManifest& manifest) {
    return YoloDetectionModel::preprocessImage(image, manifest).blob;
}

QVector<core::ClassificationItem> ClassificationModel::postprocessClassifications(
    const std::vector<runtime::InferenceTensor>& tensors,
    const core::ModelManifest& manifest,
    const double confidenceThreshold) {
    if (tensors.empty()) {
        return {};
    }

    const runtime::InferenceTensor& output = tensors.front();
    const std::vector<float>& values = output.values;
    const int numClasses = static_cast<int>(values.size());

    float maxVal = -std::numeric_limits<float>::max();
    for (float v : values) {
        if (v > maxVal) maxVal = v;
    }

    std::vector<float> expValues(numClasses);
    float sumExp = 0.0f;
    for (int i = 0; i < numClasses; ++i) {
        expValues[i] = std::exp(values[i] - maxVal);
        sumExp += expValues[i];
    }

    QVector<core::ClassificationItem> results;
    for (int i = 0; i < numClasses; ++i) {
        const float confidence = expValues[i] / sumExp;
        if (confidence < static_cast<float>(confidenceThreshold)) {
            continue;
        }

        core::ClassificationItem item;
        item.classId = i;
        item.confidence = confidence;
        if (i < manifest.labels.size()) {
            item.label = manifest.labels[i];
        }
        results.append(item);
    }

    std::sort(results.begin(), results.end(),
        [](const core::ClassificationItem& a, const core::ClassificationItem& b) {
            return a.confidence > b.confidence;
        });

    return results;
}

}  // namespace aitoolkit::models
