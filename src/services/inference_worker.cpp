#include "services/inference_worker.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include <stdexcept>

#include "core/unicode_path.h"
#include "services/unicode_io.h"

namespace aitoolkit::services {

InferenceWorker::InferenceWorker(QObject* parent)
    : QObject(parent) {}

void InferenceWorker::setModel(std::shared_ptr<models::InferenceBackend> model) {
    QMutexLocker locker(&mutex_);
    model_ = std::move(model);
}

void InferenceWorker::setThresholds(const double confidenceThreshold, const double nmsThreshold) {
    QMutexLocker locker(&mutex_);
    confidenceThreshold_ = confidenceThreshold;
    nmsThreshold_ = nmsThreshold;
}

void InferenceWorker::runImage(const QString& imagePath) {
    cancelled_.store(false);
    const QString cleanPath = QDir::cleanPath(imagePath);

    std::shared_ptr<models::InferenceBackend> model;
    double confThreshold = -1.0;
    double nmsThresholdVal = -1.0;
    {
        QMutexLocker locker(&mutex_);
        model = model_;
        confThreshold = confidenceThreshold_;
        nmsThresholdVal = nmsThreshold_;
    }

    if (!model) {
        emit error(QStringLiteral("No model loaded"));
        return;
    }

    try {
        const cv::Mat image = imreadUnicode(cleanPath, cv::IMREAD_COLOR);
        if (image.empty()) {
            emit error(QStringLiteral("Failed to read input image: %1").arg(QDir::toNativeSeparators(cleanPath)));
            return;
        }

        QElapsedTimer timer;
        timer.start();

        core::InferenceSummary summary;
        summary.modelName = model->manifest().name;
        summary.inputPath = cleanPath;
        summary.taskType = model->manifest().taskType;
        summary.imageWidth = image.cols;
        summary.imageHeight = image.rows;
        summary.elapsedMs = 0.0;

        const QString taskType = model->manifest().taskType.toLower();

        if (taskType == QStringLiteral("classification")) {
            const auto results = model->classify(image, confThreshold);
            summary.classifications = results;
            summary.detectionCount = results.size();
        } else if (taskType == QStringLiteral("segmentation")) {
            const auto results = model->segment(image, confThreshold, nmsThresholdVal);
            summary.segmentations = results;
            summary.detectionCount = results.size();
        } else {
            const auto results = model->detect(image, confThreshold, nmsThresholdVal);
            summary.detections = results;
            summary.detectionCount = results.size();
        }

        summary.elapsedMs = static_cast<double>(timer.nsecsElapsed()) / 1000000.0;

        emit imageResultReady(summary);
    } catch (const std::exception& e) {
        emit error(QString::fromUtf8(e.what()));
    }
}

void InferenceWorker::runBatch(const QStringList& imagePaths) {
    cancelled_.store(false);

    std::shared_ptr<models::InferenceBackend> model;
    double confThreshold = -1.0;
    double nmsThresholdVal = -1.0;
    {
        QMutexLocker locker(&mutex_);
        model = model_;
        confThreshold = confidenceThreshold_;
        nmsThresholdVal = nmsThreshold_;
    }

    if (!model) {
        emit error(QStringLiteral("No model loaded"));
        return;
    }

    QVector<core::InferenceSummary> results;
    results.reserve(imagePaths.size());
    const int total = imagePaths.size();

    for (int i = 0; i < total; ++i) {
        if (cancelled_.load()) {
            break;
        }

        try {
            const QString cleanPath = QDir::cleanPath(imagePaths[i]);
            const cv::Mat image = imreadUnicode(cleanPath, cv::IMREAD_COLOR);
            if (image.empty()) {
                core::InferenceSummary skipped;
                skipped.modelName = model->manifest().name;
                skipped.inputPath = cleanPath;
                results.append(skipped);
                emit batchProgress(i + 1, total);
                continue;
            }

            QElapsedTimer timer;
            timer.start();

            core::InferenceSummary summary;
            summary.modelName = model->manifest().name;
            summary.inputPath = cleanPath;
            summary.taskType = model->manifest().taskType;
            summary.imageWidth = image.cols;
            summary.imageHeight = image.rows;
            summary.elapsedMs = 0.0;

            const QString taskType = model->manifest().taskType.toLower();

            if (taskType == QStringLiteral("classification")) {
                const auto results = model->classify(image, confThreshold);
                summary.classifications = results;
                summary.detectionCount = results.size();
            } else if (taskType == QStringLiteral("segmentation")) {
                const auto results = model->segment(image, confThreshold, nmsThresholdVal);
                summary.segmentations = results;
                summary.detectionCount = results.size();
            } else {
                const auto results = model->detect(image, confThreshold, nmsThresholdVal);
                summary.detections = results;
                summary.detectionCount = results.size();
            }

            summary.elapsedMs = static_cast<double>(timer.nsecsElapsed()) / 1000000.0;
            results.append(summary);
        } catch (const std::exception&) {
            core::InferenceSummary skipped;
            skipped.modelName = model->manifest().name;
            skipped.inputPath = QDir::cleanPath(imagePaths[i]);
            results.append(skipped);
        }

        emit batchProgress(i + 1, total);
    }

    emit batchFinished(results);
}

void InferenceWorker::runVideo(const QString& videoPath, const int maxFrames) {
    cancelled_.store(false);

    std::shared_ptr<models::InferenceBackend> model;
    double confThreshold = -1.0;
    double nmsThresholdVal = -1.0;
    {
        QMutexLocker locker(&mutex_);
        model = model_;
        confThreshold = confidenceThreshold_;
        nmsThresholdVal = nmsThreshold_;
    }

    if (!model) {
        emit error(QStringLiteral("No model loaded"));
        return;
    }

    try {
        const QString cleanPath = QDir::cleanPath(videoPath);
        cv::VideoCapture capture;
        if (!openVideoCapture(cleanPath, capture)) {
            emit error(QStringLiteral("Failed to open video file: %1").arg(QDir::toNativeSeparators(cleanPath)));
            return;
        }

        const int totalFrames = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
        QVector<core::InferenceSummary> results;
        cv::Mat frame;
        int frameIndex = 0;

        while (capture.read(frame)) {
            if (frame.empty() || cancelled_.load()) {
                break;
            }
            if (maxFrames > 0 && frameIndex >= maxFrames) {
                break;
            }

            try {
                QElapsedTimer timer;
                timer.start();

                core::InferenceSummary summary;
                summary.modelName = model->manifest().name;
                summary.inputPath = QStringLiteral("%1 [frame %2]").arg(QFileInfo(cleanPath).absoluteFilePath()).arg(frameIndex);
                summary.taskType = model->manifest().taskType;
                summary.imageWidth = frame.cols;
                summary.imageHeight = frame.rows;
                summary.elapsedMs = 0.0;

                const QString taskType = model->manifest().taskType.toLower();

                if (taskType == QStringLiteral("classification")) {
                    const auto results = model->classify(frame, confThreshold);
                    summary.classifications = results;
                    summary.detectionCount = results.size();
                } else if (taskType == QStringLiteral("segmentation")) {
                    const auto results = model->segment(frame, confThreshold, nmsThresholdVal);
                    summary.segmentations = results;
                    summary.detectionCount = results.size();
                } else {
                    const auto results = model->detect(frame, confThreshold, nmsThresholdVal);
                    summary.detections = results;
                    summary.detectionCount = results.size();
                }

                summary.elapsedMs = static_cast<double>(timer.nsecsElapsed()) / 1000000.0;
                results.append(summary);
            } catch (const std::exception&) {
                core::InferenceSummary skipped;
                skipped.modelName = model->manifest().name;
                skipped.inputPath = QStringLiteral("%1 [frame %2]").arg(QFileInfo(cleanPath).absoluteFilePath()).arg(frameIndex);
                results.append(skipped);
            }

            ++frameIndex;
            emit videoProgress(frameIndex, totalFrames > 0 ? totalFrames : 0);
        }

        emit videoFinished(results);
    } catch (const std::exception& e) {
        emit error(QString::fromUtf8(e.what()));
    }
}

void InferenceWorker::cancel() {
    cancelled_.store(true);
}

}  // namespace aitoolkit::services
