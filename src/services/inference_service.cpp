#include "services/inference_service.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>

#include <opencv2/imgcodecs.hpp>

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

}  // namespace aitoolkit::services
