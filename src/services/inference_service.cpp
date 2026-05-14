#include "services/inference_service.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include <stdexcept>

namespace aitoolkit::services {

core::InferenceSummary InferenceService::runImage(
    const models::YoloDetectionModel& model,
    const QString& imagePath) const {
    const QString cleanImagePath = QDir::cleanPath(imagePath);
    const cv::Mat image = cv::imread(cleanImagePath.toStdString(), cv::IMREAD_COLOR);
    if (image.empty()) {
        throw std::runtime_error(
            QStringLiteral("Failed to read input image: %1").arg(QDir::toNativeSeparators(cleanImagePath)).toStdString());
    }

    QElapsedTimer timer;
    timer.start();
    const QVector<core::DetectionItem> detections = model.detect(image);

    core::InferenceSummary summary;
    summary.modelName = model.manifest().name;
    summary.inputPath = QFileInfo(cleanImagePath).absoluteFilePath();
    summary.detectionCount = detections.size();
    summary.elapsedMs = static_cast<double>(timer.nsecsElapsed()) / 1000000.0;
    summary.detections = detections;
    return summary;
}

QVector<core::InferenceSummary> InferenceService::runBatch(
    const models::YoloDetectionModel& model,
    const QStringList& imagePaths) const {
    QVector<core::InferenceSummary> results;
    results.reserve(imagePaths.size());

    for (const QString& imagePath : imagePaths) {
        try {
            results.append(runImage(model, imagePath));
        } catch (const std::exception&) {
            core::InferenceSummary skipped;
            skipped.modelName = model.manifest().name;
            skipped.inputPath = QDir::cleanPath(imagePath);
            results.append(skipped);
        }
    }

    return results;
}

QVector<core::InferenceSummary> InferenceService::runVideo(
    const models::YoloDetectionModel& model,
    const QString& videoPath,
    const int maxFrames) const {
    const QString cleanPath = QDir::cleanPath(videoPath);
    cv::VideoCapture capture(cleanPath.toStdString());
    if (!capture.isOpened()) {
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
            QElapsedTimer timer;
            timer.start();
            const QVector<core::DetectionItem> detections = model.detect(frame);

            core::InferenceSummary summary;
            summary.modelName = model.manifest().name;
            summary.inputPath = QStringLiteral("%1 [frame %2]").arg(QFileInfo(cleanPath).absoluteFilePath()).arg(frameIndex);
            summary.detectionCount = detections.size();
            summary.elapsedMs = static_cast<double>(timer.nsecsElapsed()) / 1000000.0;
            summary.detections = detections;
            results.append(summary);
        } catch (const std::exception&) {
            core::InferenceSummary skipped;
            skipped.modelName = model.manifest().name;
            skipped.inputPath = QStringLiteral("%1 [frame %2]").arg(QFileInfo(cleanPath).absoluteFilePath()).arg(frameIndex);
            results.append(skipped);
        }

        ++frameIndex;
    }

    return results;
}

}  // namespace aitoolkit::services
