#pragma once

#include <QString>
#include <QVector>

#include "core/types.h"
#include "models/inference_backend.h"

namespace aitoolkit::services {

class InferenceService {
public:
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
