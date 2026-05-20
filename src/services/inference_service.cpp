#include "services/inference_service.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include <stdexcept>

#include "core/unicode_path.h"
#include "services/unicode_io.h"

namespace aitoolkit::services {

core::InferenceSummary InferenceService::runImageFromMat(
    const models::InferenceBackend& model,
    const cv::Mat& image,
    const QString& inputPath,
    double confidenceThreshold,
    double nmsThreshold) const {
    QElapsedTimer timer;
    timer.start();

    core::InferenceSummary summary;
    summary.modelName = model.manifest().name;
    summary.inputPath = inputPath;
    summary.taskType = model.manifest().taskType;
    summary.imageWidth = image.cols;
    summary.imageHeight = image.rows;
    summary.elapsedMs = 0.0;

    const QString taskType = model.manifest().taskType.toLower();
    if (taskType == QStringLiteral("classification")) {
        summary.classifications = model.classify(image, confidenceThreshold);
        summary.detectionCount = summary.classifications.size();
    } else if (taskType == QStringLiteral("segmentation")) {
        summary.segmentations = model.segment(image, confidenceThreshold, nmsThreshold);
        summary.detectionCount = summary.segmentations.size();
    } else {
        summary.detections = model.detect(image, confidenceThreshold, nmsThreshold);
        summary.detectionCount = summary.detections.size();
    }

    summary.elapsedMs = static_cast<double>(timer.nsecsElapsed()) / 1000000.0;
    return summary;
}

core::InferenceSummary InferenceService::runImage(
    const models::InferenceBackend& model,
    const QString& imagePath) const {
    const QString cleanImagePath = QDir::cleanPath(imagePath);
    const cv::Mat image = imreadUnicode(cleanImagePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        throw std::runtime_error(
            QStringLiteral("Failed to read input image: %1").arg(QDir::toNativeSeparators(cleanImagePath)).toStdString());
    }
    return runImageFromMat(model, image, QFileInfo(cleanImagePath).absoluteFilePath());
}

QVector<core::InferenceSummary> InferenceService::runBatch(
    const models::InferenceBackend& model,
    const QStringList& imagePaths) const {
    QVector<core::InferenceSummary> results;
    results.reserve(imagePaths.size());

    for (const QString& imagePath : imagePaths) {
        try {
            results.append(runImage(model, imagePath));
        } catch (const std::exception& e) {
            core::InferenceSummary skipped;
            skipped.modelName = model.manifest().name;
            skipped.inputPath = QDir::cleanPath(imagePath);
            skipped.success = false;
            skipped.errorMessage = QString::fromUtf8(e.what());
            results.append(skipped);
        }
    }

    return results;
}

QVector<core::InferenceSummary> InferenceService::runVideo(
    const models::InferenceBackend& model,
    const QString& videoPath,
    const int maxFrames) const {
    const QString cleanPath = QDir::cleanPath(videoPath);
    cv::VideoCapture capture;
    if (!openVideoCapture(cleanPath, capture)) {
        throw std::runtime_error(
            QStringLiteral("Failed to open video file: %1").arg(QDir::toNativeSeparators(cleanPath)).toStdString());
    }

    QVector<core::InferenceSummary> results;
    cv::Mat frame;
    int frameIndex = 0;

    while (capture.read(frame)) {
        if (frame.empty()) {
            break;
        }
        if (maxFrames > 0 && frameIndex >= maxFrames) {
            break;
        }

        try {
            const QString framePath = QStringLiteral("%1 [frame %2]").arg(QFileInfo(cleanPath).absoluteFilePath()).arg(frameIndex);
            auto summary = runImageFromMat(model, frame, framePath);
            results.append(summary);
        } catch (const std::exception& e) {
            core::InferenceSummary skipped;
            skipped.modelName = model.manifest().name;
            skipped.inputPath = QStringLiteral("%1 [frame %2]").arg(QFileInfo(cleanPath).absoluteFilePath()).arg(frameIndex);
            skipped.success = false;
            skipped.errorMessage = QString::fromUtf8(e.what());
            results.append(skipped);
        }

        ++frameIndex;
    }

    return results;
}

}  // namespace aitoolkit::services
