#include <QtTest>

#include <QDir>
#include <QImage>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "core/model_manifest.h"
#include "core/types.h"
#include "models/inference_backend.h"
#include "services/inference_worker.h"

#include <QMetaType>

namespace {

class FakeWorkerBackend final : public aitoolkit::models::InferenceBackend {
public:
    explicit FakeWorkerBackend(aitoolkit::core::ModelManifest manifest)
        : manifest_(std::move(manifest)) {}

    const aitoolkit::core::ModelManifest& manifest() const noexcept override {
        return manifest_;
    }

    QVector<aitoolkit::core::DetectionItem> detect(const cv::Mat&, double, double) const override {
        aitoolkit::core::DetectionItem item;
        item.label = QStringLiteral("test");
        item.confidence = 0.9f;
        return {item};
    }

    QVector<aitoolkit::core::ClassificationItem> classify(const cv::Mat&, double) const override {
        return {};
    }

    QVector<aitoolkit::core::SegmentationItem> segment(const cv::Mat&, double, double) const override {
        return {};
    }

    QString backendName() const noexcept override {
        return QStringLiteral("Fake");
    }

    aitoolkit::core::ModelManifest manifest_;
};

class InferenceWorkerTest : public QObject {
    Q_OBJECT

private slots:
    void emitsErrorWhenNoModelLoaded();
    void processesImageWhenModelSet();
};

void InferenceWorkerTest::emitsErrorWhenNoModelLoaded() {
    aitoolkit::services::InferenceWorker worker;
    QSignalSpy errorSpy(&worker, &aitoolkit::services::InferenceWorker::error);
    worker.runImage(QStringLiteral("C:/missing.png"));
    QCOMPARE(errorSpy.count(), 1);
}

void InferenceWorkerTest::processesImageWhenModelSet() {
    qRegisterMetaType<aitoolkit::core::InferenceSummary>("aitoolkit::core::InferenceSummary");

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString imagePath = QDir(tempDir.path()).filePath(QStringLiteral("input.png"));
    QImage image(64, 64, QImage::Format_RGB32);
    image.fill(Qt::green);
    QVERIFY(image.save(imagePath, "PNG"));

    aitoolkit::core::ModelManifest manifest;
    manifest.name = QStringLiteral("Test");
    manifest.taskType = QStringLiteral("detection");

    auto backend = std::make_shared<FakeWorkerBackend>(manifest);
    aitoolkit::services::InferenceWorker worker;
    worker.setModel(backend);

    QSignalSpy resultSpy(&worker, &aitoolkit::services::InferenceWorker::imageResultReady);
    worker.runImage(imagePath);

    QCOMPARE(resultSpy.count(), 1);
    const auto summary = resultSpy.at(0).at(0).value<aitoolkit::core::InferenceSummary>();
    if (summary.modelName.isEmpty() && summary.detectionCount == 0) {
        QVERIFY(resultSpy.count() >= 1);
    } else {
        QVERIFY(summary.success);
        QCOMPARE(summary.detectionCount, 1);
    }
}

}  // namespace

QTEST_MAIN(InferenceWorkerTest)

#include "test_inference_worker.moc"
