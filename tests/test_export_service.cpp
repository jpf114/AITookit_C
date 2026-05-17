#include <QtTest>

#include <QJsonArray>
#include <QImage>
#include <QJsonObject>
#include <QTemporaryDir>

#include "core/json_utils.h"
#include "services/export_service.h"

namespace {

class ExportServiceTest : public QObject {
    Q_OBJECT

private slots:
    void writesSummaryAsJson();
    void rendersSegmentationMaskWithInstanceColor();
};

void ExportServiceTest::writesSummaryAsJson() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("Sample YOLO");
    summary.inputPath = QStringLiteral("D:/images/demo.jpg");
    summary.detectionCount = 1;
    summary.elapsedMs = 12.5;

    aitoolkit::core::DetectionItem detection;
    detection.classId = 2;
    detection.label = QStringLiteral("dog");
    detection.confidence = 0.91f;
    detection.boundingBox = QRectF(10.0, 20.0, 30.0, 40.0);
    summary.detections.push_back(detection);

    const QString outputPath = tempDir.filePath(QStringLiteral("result.json"));

    aitoolkit::services::ExportService service;
    service.exportJson(outputPath, summary);

    const QJsonObject root = aitoolkit::core::readJsonObject(outputPath);
    QCOMPARE(root.value(QStringLiteral("model_name")).toString(), summary.modelName);
    QCOMPARE(root.value(QStringLiteral("input_path")).toString(), summary.inputPath);
    QCOMPARE(root.value(QStringLiteral("detection_count")).toInt(), 1);
    QCOMPARE(root.value(QStringLiteral("elapsed_ms")).toDouble(), 12.5);

    const QJsonArray detections = root.value(QStringLiteral("detections")).toArray();
    QCOMPARE(detections.size(), 1);

    const QJsonObject detectionObject = detections.at(0).toObject();
    QCOMPARE(detectionObject.value(QStringLiteral("class_id")).toInt(), 2);
    QCOMPARE(detectionObject.value(QStringLiteral("label")).toString(), QStringLiteral("dog"));
    QVERIFY(qAbs(detectionObject.value(QStringLiteral("confidence")).toDouble() - 0.91) < 0.0001);

    const QJsonObject boundingBox = detectionObject.value(QStringLiteral("bounding_box")).toObject();
    QCOMPARE(boundingBox.value(QStringLiteral("x")).toDouble(), 10.0);
    QCOMPARE(boundingBox.value(QStringLiteral("y")).toDouble(), 20.0);
    QCOMPARE(boundingBox.value(QStringLiteral("width")).toDouble(), 30.0);
    QCOMPARE(boundingBox.value(QStringLiteral("height")).toDouble(), 40.0);
}

void ExportServiceTest::rendersSegmentationMaskWithInstanceColor() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    QImage sourceImage(24, 24, QImage::Format_RGB32);
    sourceImage.fill(Qt::black);

    QImage mask(8, 8, QImage::Format_Alpha8);
    mask.fill(255);

    aitoolkit::core::InferenceSummary summary;
    summary.taskType = QStringLiteral("segmentation");

    aitoolkit::core::SegmentationItem item;
    item.classId = 1;
    item.label = QStringLiteral("lane");
    item.confidence = 0.87f;
    item.boundingBox = QRectF(4.0, 4.0, 8.0, 8.0);
    item.mask = mask;
    item.renderColor = QColor(QStringLiteral("#ef4444"));
    summary.segmentations.push_back(item);

    const QString outputPath = tempDir.filePath(QStringLiteral("rendered.png"));

    aitoolkit::services::ExportService service;
    service.exportRenderedImage(outputPath, sourceImage, summary);

    const QImage rendered(outputPath);
    QVERIFY2(!rendered.isNull(), "Rendered export image should be readable");

    const QColor centerPixel = rendered.pixelColor(8, 8);
    QVERIFY2(centerPixel.red() > centerPixel.green(), "Segmentation mask should be tinted by the instance color.");
    QVERIFY2(centerPixel.red() > centerPixel.blue(), "Segmentation mask should not render as an untinted grayscale patch.");
}

}  // namespace

QTEST_MAIN(ExportServiceTest)

#include "test_export_service.moc"
