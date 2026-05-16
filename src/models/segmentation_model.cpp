#include "models/segmentation_model.h"

#include <QDir>
#include <QImage>

#include <opencv2/imgproc.hpp>

#include <stdexcept>

#include "models/yolo_detection_model.h"

namespace aitoolkit::models {

namespace {

QString resolveOnnxPath(const core::ModelManifest& manifest) {
    const QFileInfo info(manifest.manifestPath);
    const QDir manifestDir = info.absoluteDir();
    return QDir::cleanPath(manifestDir.filePath(manifest.modelPath));
}

QVector<QColor> generateSegmentationColors(int count) {
    QVector<QColor> colors;
    colors.reserve(count);
    for (int i = 0; i < count; ++i) {
        const int hue = (i * 137) % 360;
        colors.append(QColor::fromHsv(hue, 200, 230));
    }
    return colors;
}

QImage maskToQImage(const cv::Mat& mask, const QSize& targetSize) {
    cv::Mat resized;
    cv::resize(mask, resized, cv::Size(targetSize.width(), targetSize.height()), 0, 0, cv::INTER_LINEAR);

    QImage image(targetSize, QImage::Format_ARGB32);
    for (int y = 0; y < resized.rows; ++y) {
        for (int x = 0; x < resized.cols; ++x) {
            const float val = resized.at<float>(y, x);
            const int alpha = val > 0.5f ? 180 : 0;
            image.setPixelColor(x, y, QColor(255, 255, 255, alpha));
        }
    }
    return image;
}

}  // namespace

SegmentationModel::SegmentationModel(core::ModelManifest manifest, const int threadCount)
    : manifest_(std::move(manifest))
    , backend_(resolveOnnxPath(manifest_), threadCount) {
    backend_.warmup();
}

const core::ModelManifest& SegmentationModel::manifest() const noexcept {
    return manifest_;
}

QVector<core::DetectionItem> SegmentationModel::detect(
    const cv::Mat& image, const double confThreshold, const double nmsThreshold) const {
    const auto prepared = YoloDetectionModel::preprocessImage(image, manifest_);
    const auto inputShape = backend_.inputShape();
    const auto outputs = backend_.run(prepared.blob, inputShape);

    if (outputs.empty()) {
        return {};
    }

    const cv::Mat output = YoloDetectionModel::tensorToDetectionMatrix(outputs.front());
    return YoloDetectionModel::postprocessDetections(
        output, prepared.networkSize, manifest_,
        prepared.originalSize, confThreshold, nmsThreshold);
}

QVector<core::SegmentationItem> SegmentationModel::segment(
    const cv::Mat& image, const double confidenceThreshold, const double nmsThreshold) const {
    const auto prepared = YoloDetectionModel::preprocessImage(image, manifest_);
    const auto inputShape = backend_.inputShape();
    const auto outputs = backend_.run(prepared.blob, inputShape);

    if (outputs.size() < 2) {
        return {};
    }

    const cv::Mat detOutput = YoloDetectionModel::tensorToDetectionMatrix(outputs.front());
    const auto detections = YoloDetectionModel::postprocessDetections(
        detOutput, prepared.networkSize, manifest_,
        prepared.originalSize, confidenceThreshold, nmsThreshold);

    const runtime::InferenceTensor& maskTensor = outputs[1];
    const int maskDim = static_cast<int>(maskTensor.shape.size());

    int maskH = 0;
    int maskW = 0;
    if (maskDim == 4) {
        maskW = static_cast<int>(maskTensor.shape[2]);
        maskH = static_cast<int>(maskTensor.shape[3]);
    } else if (maskDim == 3) {
        maskH = static_cast<int>(maskTensor.shape[1]);
        maskW = static_cast<int>(maskTensor.shape[2]);
    }

    if (maskH == 0 || maskW == 0) {
        return {};
    }

    const auto colors = generateSegmentationColors(detections.size());
    QVector<core::SegmentationItem> results;
    results.reserve(detections.size());

    for (int i = 0; i < detections.size() && i < maskH; ++i) {
        core::SegmentationItem item;
        item.classId = detections[i].classId;
        item.label = detections[i].label;
        item.confidence = detections[i].confidence;
        item.boundingBox = detections[i].boundingBox;
        item.renderColor = colors[i % colors.size()];

        cv::Mat singleMask(maskH, maskW, CV_32F);
        for (int y = 0; y < maskH; ++y) {
            for (int x = 0; x < maskW; ++x) {
                const int idx = i * maskH * maskW + y * maskW + x;
                if (idx < static_cast<int>(maskTensor.values.size())) {
                    singleMask.at<float>(y, x) = maskTensor.values[idx];
                }
            }
        }

        item.mask = maskToQImage(singleMask, QSize(image.cols, image.rows));
        results.append(item);
    }

    return results;
}

QString SegmentationModel::backendName() const noexcept {
    return QStringLiteral("ONNX Runtime");
}

}  // namespace aitoolkit::models
