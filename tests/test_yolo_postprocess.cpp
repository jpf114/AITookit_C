#include <QtTest>

#include <opencv2/core.hpp>

#include "core/model_manifest.h"
#include "models/yolo_detection_model.h"

namespace {

class YoloPostprocessTest : public QObject {
    Q_OBJECT

private slots:
    void converts2dTensorWithEightyFiveAttributes();
    void converts3dDetectionsFirstTensorWithEightyFiveAttributesWithoutLabels();
    void filtersByConfidenceAndMapsLabels();
    void suppressesOverlappingBoxesWithNms();
    void convertsAttrsFirst3dTensorToDetectionMatrix();
    void failsForAmbiguous3dTensorLayout();
    void treatsColsFiveAsObjectnessOnlyConfidence();
    void failsForNonFloatPostprocessInput();
};

aitoolkit::core::ModelManifest makeManifest() {
    aitoolkit::core::ModelManifest manifest;
    manifest.name = QStringLiteral("Test YOLO");
    manifest.taskType = QStringLiteral("detection");
    manifest.backendType = QStringLiteral("onnxruntime");
    manifest.decoder = QStringLiteral("yolo");
    manifest.inputWidth = 640;
    manifest.inputHeight = 640;
    manifest.confidenceThreshold = 0.5;
    manifest.nmsThreshold = 0.4;
    manifest.labels = {QStringLiteral("person"), QStringLiteral("car"), QStringLiteral("dog")};
    return manifest;
}

void YoloPostprocessTest::converts2dTensorWithEightyFiveAttributes() {
    aitoolkit::runtime::OnnxTensor tensor;
    tensor.shape = {2, 85};
    tensor.values.resize(2 * 85);
    tensor.values[0] = 10.0f;
    tensor.values[84] = 0.75f;
    tensor.values[85] = 20.0f;
    tensor.values[(2 * 85) - 1] = 0.25f;

    const cv::Mat matrix = aitoolkit::models::YoloDetectionModel::tensorToDetectionMatrix(tensor);

    QCOMPARE(matrix.rows, 2);
    QCOMPARE(matrix.cols, 85);
    QCOMPARE(matrix.at<float>(0, 0), 10.0f);
    QCOMPARE(matrix.at<float>(0, 84), 0.75f);
    QCOMPARE(matrix.at<float>(1, 0), 20.0f);
    QCOMPARE(matrix.at<float>(1, 84), 0.25f);
}

void YoloPostprocessTest::converts3dDetectionsFirstTensorWithEightyFiveAttributesWithoutLabels() {
    aitoolkit::runtime::OnnxTensor tensor;
    tensor.shape = {1, 2, 85};
    tensor.values.resize(1 * 2 * 85);
    tensor.values[0] = 10.0f;
    tensor.values[84] = 0.75f;
    tensor.values[85] = 20.0f;
    tensor.values[(2 * 85) - 1] = 0.25f;

    const cv::Mat matrix = aitoolkit::models::YoloDetectionModel::tensorToDetectionMatrix(tensor);

    QCOMPARE(matrix.rows, 2);
    QCOMPARE(matrix.cols, 85);
    QCOMPARE(matrix.at<float>(0, 0), 10.0f);
    QCOMPARE(matrix.at<float>(0, 84), 0.75f);
    QCOMPARE(matrix.at<float>(1, 0), 20.0f);
    QCOMPARE(matrix.at<float>(1, 84), 0.25f);
}

void YoloPostprocessTest::filtersByConfidenceAndMapsLabels() {
    const aitoolkit::core::ModelManifest manifest = makeManifest();
    const cv::Mat detections = (cv::Mat_<float>(3, 8) <<
        100.0f, 120.0f, 80.0f, 60.0f, 0.9f, 0.1f, 0.8f, 0.2f,
        200.0f, 220.0f, 50.0f, 40.0f, 0.4f, 0.9f, 0.1f, 0.0f,
        320.0f, 300.0f, 90.0f, 70.0f, 0.7f, 0.2f, 0.1f, 0.85f);

    const QVector<aitoolkit::core::DetectionItem> results =
        aitoolkit::models::YoloDetectionModel::postprocessDetections(
            detections,
            QSize(640, 640),
            manifest,
            QSize(640, 640));

    QCOMPARE(results.size(), 2);

    QCOMPARE(results.at(0).classId, 1);
    QCOMPARE(results.at(0).label, QStringLiteral("car"));
    QVERIFY(qAbs(results.at(0).confidence - 0.72f) < 0.0001f);
    QCOMPARE(results.at(0).boundingBox, QRectF(60.0, 90.0, 80.0, 60.0));

    QCOMPARE(results.at(1).classId, 2);
    QCOMPARE(results.at(1).label, QStringLiteral("dog"));
    QVERIFY(qAbs(results.at(1).confidence - 0.595f) < 0.0001f);
    QCOMPARE(results.at(1).boundingBox, QRectF(275.0, 265.0, 90.0, 70.0));
}

void YoloPostprocessTest::suppressesOverlappingBoxesWithNms() {
    aitoolkit::core::ModelManifest manifest = makeManifest();
    manifest.labels = {QStringLiteral("person"), QStringLiteral("car")};
    const cv::Mat detections = (cv::Mat_<float>(3, 7) <<
        100.0f, 100.0f, 80.0f, 80.0f, 0.95f, 0.9f, 0.1f,
        104.0f, 104.0f, 78.0f, 78.0f, 0.90f, 0.88f, 0.12f,
        400.0f, 400.0f, 60.0f, 60.0f, 0.92f, 0.2f, 0.85f);

    const QVector<aitoolkit::core::DetectionItem> results =
        aitoolkit::models::YoloDetectionModel::postprocessDetections(
            detections,
            QSize(640, 640),
            manifest,
            QSize(1280, 720));

    QCOMPARE(results.size(), 2);

    QCOMPARE(results.at(0).classId, 0);
    QCOMPARE(results.at(0).label, QStringLiteral("person"));
    QVERIFY(results.at(0).confidence > results.at(1).confidence);
    QCOMPARE(results.at(0).boundingBox, QRectF(120.0, 67.5, 160.0, 90.0));

    QCOMPARE(results.at(1).classId, 1);
    QCOMPARE(results.at(1).label, QStringLiteral("car"));
    QCOMPARE(results.at(1).boundingBox, QRectF(740.0, 416.25, 120.0, 67.5));
}

void YoloPostprocessTest::convertsAttrsFirst3dTensorToDetectionMatrix() {
    aitoolkit::runtime::OnnxTensor tensor;
    tensor.shape = {1, 8, 2};
    tensor.values = {
        10.0f, 20.0f,
        30.0f, 40.0f,
        50.0f, 60.0f,
        70.0f, 80.0f,
        0.9f, 0.8f,
        0.1f, 0.2f,
        0.3f, 0.4f,
        0.5f, 0.6f,
    };

    const cv::Mat matrix = aitoolkit::models::YoloDetectionModel::tensorToDetectionMatrix(tensor, 3);

    QCOMPARE(matrix.rows, 2);
    QCOMPARE(matrix.cols, 8);
    QCOMPARE(matrix.at<float>(0, 0), 10.0f);
    QCOMPARE(matrix.at<float>(0, 7), 0.5f);
    QCOMPARE(matrix.at<float>(1, 0), 20.0f);
    QCOMPARE(matrix.at<float>(1, 7), 0.6f);
}

void YoloPostprocessTest::failsForAmbiguous3dTensorLayout() {
    aitoolkit::runtime::OnnxTensor tensor;
    tensor.shape = {1, 7, 20};
    tensor.values.resize(1 * 7 * 20);

    QVERIFY_EXCEPTION_THROWN(
        aitoolkit::models::YoloDetectionModel::tensorToDetectionMatrix(tensor),
        std::runtime_error);
}

void YoloPostprocessTest::treatsColsFiveAsObjectnessOnlyConfidence() {
    aitoolkit::core::ModelManifest manifest = makeManifest();
    manifest.labels.clear();
    const cv::Mat detections = (cv::Mat_<float>(2, 5) <<
        100.0f, 120.0f, 80.0f, 60.0f, 0.9f,
        200.0f, 220.0f, 50.0f, 40.0f, 0.4f);

    const QVector<aitoolkit::core::DetectionItem> results =
        aitoolkit::models::YoloDetectionModel::postprocessDetections(
            detections,
            QSize(640, 640),
            manifest,
            QSize(640, 640));

    QCOMPARE(results.size(), 1);
    QCOMPARE(results.at(0).classId, -1);
    QVERIFY(results.at(0).label.isEmpty());
    QVERIFY(qAbs(results.at(0).confidence - 0.9f) < 0.0001f);
    QCOMPARE(results.at(0).boundingBox, QRectF(60.0, 90.0, 80.0, 60.0));
}

void YoloPostprocessTest::failsForNonFloatPostprocessInput() {
    const aitoolkit::core::ModelManifest manifest = makeManifest();
    const cv::Mat detections = (cv::Mat_<double>(1, 8) <<
        100.0, 120.0, 80.0, 60.0, 0.9, 0.1, 0.8, 0.2);

    QVERIFY_EXCEPTION_THROWN(
        aitoolkit::models::YoloDetectionModel::postprocessDetections(
            detections,
            QSize(640, 640),
            manifest,
            QSize(640, 640)),
        std::runtime_error);
}

}  // namespace

QTEST_MAIN(YoloPostprocessTest)

#include "test_yolo_postprocess.moc"
