#include <QtTest>

#include <QDir>
#include <QImage>
#include <QTemporaryDir>

#include "core/model_manifest.h"
#include "core/types.h"
#include "services/inference_service.h"
#include "test_helpers.h"

namespace {

using test_helpers::FakeInferenceBackend;

class InferenceServiceTest : public QObject {
    Q_OBJECT

private slots:
    void dispatchesToClassifyForClassificationTask();
    void dispatchesToSegmentForSegmentationTask();
    void dispatchesToDetectForDetectionTask();
    void populatesSummaryCorrectly();
    void runBatchProcessesMultipleImages();
    void runBatchHandlesFailedImages();
};

aitoolkit::core::ModelManifest makeManifest(const QString& taskType) {
    aitoolkit::core::ModelManifest manifest;
    manifest.name = QStringLiteral("Test Model");
    manifest.taskType = taskType;
    return manifest;
}

void InferenceServiceTest::dispatchesToClassifyForClassificationTask() {
    FakeInferenceBackend backend(makeManifest(QStringLiteral("classification")));
    aitoolkit::core::ClassificationItem item;
    item.classId = 0;
    item.confidence = 0.95f;
    item.label = QStringLiteral("cat");
    backend.fakeClassifications.append(item);

    aitoolkit::services::InferenceService service;
    const auto summary = service.runImageFromMat(backend, cv::Mat::zeros(100, 100, CV_8UC3));

    QVERIFY(backend.classifyCalls > 0);
    QVERIFY(backend.detectCalls == 0);
    QVERIFY(backend.segmentCalls == 0);
    QCOMPARE(summary.classifications.size(), 1);
    QCOMPARE(summary.detectionCount, 1);
}

void InferenceServiceTest::dispatchesToSegmentForSegmentationTask() {
    FakeInferenceBackend backend(makeManifest(QStringLiteral("segmentation")));
    aitoolkit::core::SegmentationItem item;
    item.classId = 0;
    item.confidence = 0.85f;
    item.label = QStringLiteral("person");
    backend.fakeSegmentations.append(item);

    aitoolkit::services::InferenceService service;
    const auto summary = service.runImageFromMat(backend, cv::Mat::zeros(100, 100, CV_8UC3));

    QVERIFY(backend.segmentCalls > 0);
    QVERIFY(backend.detectCalls == 0);
    QVERIFY(backend.classifyCalls == 0);
    QCOMPARE(summary.segmentations.size(), 1);
    QCOMPARE(summary.detectionCount, 1);
}

void InferenceServiceTest::dispatchesToDetectForDetectionTask() {
    FakeInferenceBackend backend(makeManifest(QStringLiteral("detection")));
    aitoolkit::core::DetectionItem item;
    item.classId = 0;
    item.confidence = 0.9f;
    item.label = QStringLiteral("car");
    item.boundingBox = QRectF(10, 20, 30, 40);
    backend.fakeDetections.append(item);

    aitoolkit::services::InferenceService service;
    const auto summary = service.runImageFromMat(backend, cv::Mat::zeros(100, 100, CV_8UC3));

    QVERIFY(backend.detectCalls > 0);
    QVERIFY(backend.classifyCalls == 0);
    QVERIFY(backend.segmentCalls == 0);
    QCOMPARE(summary.detections.size(), 1);
    QCOMPARE(summary.detectionCount, 1);
}

void InferenceServiceTest::populatesSummaryCorrectly() {
    FakeInferenceBackend backend(makeManifest(QStringLiteral("detection")));
    aitoolkit::core::DetectionItem item;
    item.classId = 0;
    item.confidence = 0.9f;
    backend.fakeDetections.append(item);

    aitoolkit::services::InferenceService service;
    const cv::Mat image = cv::Mat::zeros(200, 300, CV_8UC3);
    const auto summary = service.runImageFromMat(
        backend, image, QStringLiteral("/test/image.png"));

    QCOMPARE(summary.modelName, QStringLiteral("Test Model"));
    QCOMPARE(summary.inputPath, QStringLiteral("/test/image.png"));
    QCOMPARE(summary.taskType, QStringLiteral("detection"));
    QCOMPARE(summary.imageWidth, 300);
    QCOMPARE(summary.imageHeight, 200);
    QVERIFY(summary.elapsedMs >= 0.0);
}

void InferenceServiceTest::runBatchProcessesMultipleImages() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QStringList paths;
    for (int i = 0; i < 3; ++i) {
        const QString path = QDir(tempDir.path()).filePath(QStringLiteral("img%1.png").arg(i));
        QImage image(64, 64, QImage::Format_ARGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(path));
        paths.append(path);
    }

    FakeInferenceBackend backend(makeManifest(QStringLiteral("classification")));
    aitoolkit::core::ClassificationItem item;
    item.classId = 0;
    item.confidence = 0.9f;
    backend.fakeClassifications.append(item);

    aitoolkit::services::InferenceService service;
    const auto results = service.runBatch(backend, paths);

    QCOMPARE(results.size(), 3);
    for (const auto& summary : results) {
        QCOMPARE(summary.detectionCount, 1);
        QVERIFY(summary.elapsedMs >= 0.0);
    }
}

void InferenceServiceTest::runBatchHandlesFailedImages() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString validPath = QDir(tempDir.path()).filePath(QStringLiteral("valid.png"));
    QImage image(64, 64, QImage::Format_ARGB32);
    image.fill(Qt::blue);
    QVERIFY(image.save(validPath));

    const QString invalidPath = QDir(tempDir.path()).filePath(QStringLiteral("nonexistent.png"));

    FakeInferenceBackend backend(makeManifest(QStringLiteral("detection")));
    aitoolkit::core::DetectionItem item;
    item.classId = 0;
    item.confidence = 0.9f;
    backend.fakeDetections.append(item);

    aitoolkit::services::InferenceService service;
    const auto results = service.runBatch(backend, {validPath, invalidPath});

    QCOMPARE(results.size(), 2);
    QCOMPARE(results[0].detectionCount, 1);
    QCOMPARE(results[1].detectionCount, 0);
    QCOMPARE(results[1].modelName, QStringLiteral("Test Model"));
}

}  // namespace

QTEST_MAIN(InferenceServiceTest)

#include "test_inference_service.moc"
