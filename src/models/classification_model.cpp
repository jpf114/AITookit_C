#include "models/classification_model.h"

#include <QDir>

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace aitoolkit::models {

namespace {

QString resolveOnnxPath(const core::ModelManifest& manifest) {
    const QFileInfo info(manifest.manifestPath);
    const QDir manifestDir = info.absoluteDir();
    return QDir::cleanPath(manifestDir.filePath(manifest.modelPath));
}

}  // namespace

ClassificationModel::ClassificationModel(core::ModelManifest manifest, const int threadCount, const bool useGPU)
    : manifest_(std::move(manifest))
    , backend_(resolveOnnxPath(manifest_), threadCount, useGPU) {
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
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(manifest.inputWidth, manifest.inputHeight));

    cv::Mat floatImg;
    resized.convertTo(floatImg, CV_32F, 1.0 / 255.0);

    std::vector<float> blob;
    blob.reserve(static_cast<std::size_t>(floatImg.channels()) * manifest.inputWidth * manifest.inputHeight);

    std::vector<cv::Mat> channels(floatImg.channels());
    cv::split(floatImg, channels);

    for (const cv::Mat& channel : channels) {
        blob.insert(blob.end(), channel.begin<float>(), channel.end<float>());
    }

    return blob;
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
