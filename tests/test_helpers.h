#pragma once

#include <QDir>
#include <QImage>
#include <QString>

#include "core/model_manifest.h"
#include "core/types.h"
#include "models/inference_backend.h"
#include "runtime/backend_plugin.h"

namespace test_helpers {

inline QString writeImageFile(const QString& directoryPath,
                               const QString& fileName,
                               int width = 64,
                               int height = 64) {
    const QString imagePath = QDir(directoryPath).filePath(fileName);
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::red);
    return image.save(imagePath) ? imagePath : QString();
}

class FakeInferenceBackend final : public aitoolkit::models::InferenceBackend {
public:
    explicit FakeInferenceBackend(aitoolkit::core::ModelManifest manifest)
        : manifest_(std::move(manifest)) {}

    const aitoolkit::core::ModelManifest& manifest() const noexcept override {
        return manifest_;
    }

    QVector<aitoolkit::core::DetectionItem> detect(const cv::Mat&, double, double) const override {
        ++detectCalls;
        return fakeDetections;
    }

    QVector<aitoolkit::core::ClassificationItem> classify(const cv::Mat&, double) const override {
        ++classifyCalls;
        return fakeClassifications;
    }

    QVector<aitoolkit::core::SegmentationItem> segment(const cv::Mat&, double, double) const override {
        ++segmentCalls;
        return fakeSegmentations;
    }

    QString backendName() const noexcept override {
        return QStringLiteral("Fake Backend");
    }

    mutable int detectCalls = 0;
    mutable int classifyCalls = 0;
    mutable int segmentCalls = 0;
    QVector<aitoolkit::core::DetectionItem> fakeDetections;
    QVector<aitoolkit::core::ClassificationItem> fakeClassifications;
    QVector<aitoolkit::core::SegmentationItem> fakeSegmentations;

private:
    aitoolkit::core::ModelManifest manifest_;
};

class FakeBackendPlugin final : public aitoolkit::runtime::BackendPlugin {
public:
    mutable int lastThreadCount = -1;
    mutable bool lastUseGPU = false;
    mutable int createModelCalls = 0;

    aitoolkit::runtime::BackendInfo info() const override {
        return {
            QStringLiteral("fake_backend"),
            QStringLiteral("Fake Backend"),
            QStringLiteral("1.0"),
            true,
            true,
        };
    }

    QStringList supportedTaskTypes() const override {
        return {QStringLiteral("detection"), QStringLiteral("classification"), QStringLiteral("segmentation")};
    }

    std::unique_ptr<aitoolkit::models::InferenceBackend> createModel(
        const aitoolkit::core::ModelManifest& manifest,
        int threadCount,
        bool useGPU) const override {
        ++createModelCalls;
        lastThreadCount = threadCount;
        lastUseGPU = useGPU;
        return std::make_unique<FakeInferenceBackend>(manifest);
    }
};

}  // namespace test_helpers
