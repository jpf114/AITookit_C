#pragma once

#include <QObject>
#include <QString>
#include <QVector>

#include "core/types.h"
#include "models/yolo_detection_model.h"

namespace aitoolkit::services {

class InferenceWorker : public QObject {
    Q_OBJECT

public:
    explicit InferenceWorker(QObject* parent = nullptr);

    void setModel(std::shared_ptr<const models::YoloDetectionModel> model);

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
    std::shared_ptr<const models::YoloDetectionModel> model_;
    std::atomic<bool> cancelled_{false};
};

}  // namespace aitoolkit::services
