#pragma once

#include <QMutex>
#include <QObject>
#include <QString>
#include <QVector>

#include "core/types.h"
#include "models/inference_backend.h"
#include "services/inference_service.h"

namespace aitoolkit::services {

class InferenceWorker : public QObject {
    Q_OBJECT

public:
    explicit InferenceWorker(QObject* parent = nullptr);

    void setModel(std::shared_ptr<models::InferenceBackend> model);
    void setThresholds(double confidenceThreshold, double nmsThreshold);

public slots:
    void runImage(const QString& imagePath);
    void runBatch(const QStringList& imagePaths);
    void runVideo(const QString& videoPath, int maxFrames = 0);
    void cancel();

signals:
    void imageResultReady(const aitoolkit::core::InferenceSummary& summary);
    void batchProgress(int completed, int total);
    void batchFinished(const QVector<aitoolkit::core::InferenceSummary>& results);
    void videoProgress(int frameIndex, int totalFrames);
    void videoFinished(const QVector<aitoolkit::core::InferenceSummary>& results);
    void error(const QString& message);

private:
    QMutex mutex_;
    InferenceService inferenceService_;
    std::shared_ptr<models::InferenceBackend> model_;
    double confidenceThreshold_ = -1.0;
    double nmsThreshold_ = -1.0;
    std::atomic<bool> cancelled_{false};
};

}  // namespace aitoolkit::services
