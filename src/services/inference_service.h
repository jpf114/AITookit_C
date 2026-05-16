#pragma once

#include <QString>
#include <QVector>

#include <opencv2/core.hpp>

#include "core/types.h"
#include "models/inference_backend.h"

namespace aitoolkit::services {

class InferenceService {
public:
    core::InferenceSummary runImageFromMat(
        const models::InferenceBackend& model,
        const cv::Mat& image,
        const QString& inputPath = QString(),
        double confidenceThreshold = -1.0,
        double nmsThreshold = -1.0) const;

    core::InferenceSummary runImage(
        const models::InferenceBackend& model,
        const QString& imagePath) const;

    QVector<core::InferenceSummary> runBatch(
        const models::InferenceBackend& model,
        const QStringList& imagePaths) const;

    QVector<core::InferenceSummary> runVideo(
        const models::InferenceBackend& model,
        const QString& videoPath,
        int maxFrames = 0) const;
};

}  // namespace aitoolkit::services
