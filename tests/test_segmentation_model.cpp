#include <QtTest>

#include <QColor>
#include <QImage>

#include "core/model_manifest.h"
#include "core/types.h"
#include "models/segmentation_model.h"

namespace {

class SegmentationModelTest : public QObject {
    Q_OBJECT

private slots:
    void generatesMasksFrom4dTensor();
    void generatesMasksFrom3dTensor();
    void returnsEmptyForNoDetections();
    void returnsEmptyForInvalidMaskDimensions();
    void masksHaveCorrectAlphaBasedOnThreshold();
    void limitsResultsToMaskRows();
    void assignsColorsToItems();
};

QVector<aitoolkit::core::DetectionItem> makeDetections(int count) {
    QVector<aitoolkit::core::DetectionItem> detections;
    for (int i = 0; i < count; ++i) {
        aitoolkit::core::DetectionItem det;
        det.classId = i;
        det.label = QStringLiteral("class_%1").arg(i);
        det.confidence = 0.9f - i * 0.1f;
        det.boundingBox = QRectF(i * 10.0, i * 10.0, 50.0, 50.0);
        detections.append(det);
    }
    return detections;
}

void SegmentationModelTest::generatesMasksFrom4dTensor() {
    const auto detections = makeDetections(2);

    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 2, 4, 4};
    tensor.values.resize(2 * 4 * 4, 0.8f);

    const auto results = aitoolkit::models::SegmentationModel::postprocessSegmentations(
        detections, tensor, QSize(4, 4));

    QCOMPARE(results.size(), 2);
    QCOMPARE(results[0].classId, 0);
    QCOMPARE(results[0].label, QStringLiteral("class_0"));
    QCOMPARE(results[1].classId, 1);
    QCOMPARE(results[1].label, QStringLiteral("class_1"));
    QVERIFY(!results[0].mask.isNull());
    QVERIFY(!results[1].mask.isNull());
}

void SegmentationModelTest::generatesMasksFrom3dTensor() {
    const auto detections = makeDetections(2);

    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {2, 4, 4};
    tensor.values.resize(2 * 4 * 4, 0.8f);

    const auto results = aitoolkit::models::SegmentationModel::postprocessSegmentations(
        detections, tensor, QSize(4, 4));

    QCOMPARE(results.size(), 2);
    QCOMPARE(results[0].classId, 0);
    QCOMPARE(results[1].classId, 1);
    QVERIFY(!results[0].mask.isNull());
    QVERIFY(!results[1].mask.isNull());
}

void SegmentationModelTest::returnsEmptyForNoDetections() {
    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 2, 4, 4};
    tensor.values.resize(2 * 4 * 4, 0.8f);

    const auto results = aitoolkit::models::SegmentationModel::postprocessSegmentations(
        {}, tensor, QSize(4, 4));

    QVERIFY(results.isEmpty());
}

void SegmentationModelTest::returnsEmptyForInvalidMaskDimensions() {
    const auto detections = makeDetections(1);

    aitoolkit::runtime::InferenceTensor tensor2d;
    tensor2d.shape = {4, 4};
    tensor2d.values.resize(16, 0.8f);

    const auto results = aitoolkit::models::SegmentationModel::postprocessSegmentations(
        detections, tensor2d, QSize(4, 4));

    QVERIFY(results.isEmpty());
}

void SegmentationModelTest::masksHaveCorrectAlphaBasedOnThreshold() {
    const auto detections = makeDetections(1);

    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 1, 2, 2};
    tensor.values = {0.9f, 0.1f, 0.1f, 0.9f};

    const auto results = aitoolkit::models::SegmentationModel::postprocessSegmentations(
        detections, tensor, QSize(2, 2));

    QCOMPARE(results.size(), 1);

    const QImage& mask = results[0].mask;
    QCOMPARE(mask.width(), 2);
    QCOMPARE(mask.height(), 2);

    QCOMPARE(mask.pixelColor(0, 0).alpha(), 180);
    QCOMPARE(mask.pixelColor(1, 0).alpha(), 0);
    QCOMPARE(mask.pixelColor(0, 1).alpha(), 0);
    QCOMPARE(mask.pixelColor(1, 1).alpha(), 180);
}

void SegmentationModelTest::limitsResultsToMaskRows() {
    const auto detections = makeDetections(5);

    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 5, 4, 2};
    tensor.values.resize(5 * 2 * 4, 0.8f);

    const auto results = aitoolkit::models::SegmentationModel::postprocessSegmentations(
        detections, tensor, QSize(4, 2));

    QVERIFY(results.size() < detections.size());
    QCOMPARE(results.size(), 2);
}

void SegmentationModelTest::assignsColorsToItems() {
    const auto detections = makeDetections(2);

    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 2, 4, 4};
    tensor.values.resize(2 * 4 * 4, 0.8f);

    const auto results = aitoolkit::models::SegmentationModel::postprocessSegmentations(
        detections, tensor, QSize(4, 4));

    QCOMPARE(results.size(), 2);
    QVERIFY(results[0].renderColor.isValid());
    QVERIFY(results[1].renderColor.isValid());
    QVERIFY(results[0].renderColor != results[1].renderColor);
}

}  // namespace

QTEST_MAIN(SegmentationModelTest)

#include "test_segmentation_model.moc"
