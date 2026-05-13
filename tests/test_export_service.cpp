#include <QtTest>

#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>

#include "core/json_utils.h"
#include "services/export_service.h"

namespace {

class ExportServiceTest : public QObject {
    Q_OBJECT

private slots:
    void writesSummaryAsJson();
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

}  // namespace

QTEST_MAIN(ExportServiceTest)

#include "test_export_service.moc"
