#include <QtTest>

#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "core/json_utils.h"
#include "test_peers.h"
#include "ui/app_controller.h"

namespace {

class AppControllerTest : public QObject {
    Q_OBJECT

private slots:
    void cancelledBatchDoesNotEmitCompleted();
    void loadModelManifestClearsStaleBatchResults();
};

void AppControllerTest::cancelledBatchDoesNotEmitCompleted() {
    aitoolkit::ui::AppController controller;

    QSignalSpy completedSpy(&controller, &aitoolkit::ui::AppController::inferenceCompletedBatch);
    QSignalSpy cancelledSpy(&controller, &aitoolkit::ui::AppController::inferenceCancelled);

    aitoolkit::testing::AppControllerTestPeer::setInferenceRunning(controller, true);
    controller.cancelInference();

    QCOMPARE(cancelledSpy.count(), 1);

    QVector<aitoolkit::core::InferenceSummary> partialResults;
    aitoolkit::core::InferenceSummary summary;
    summary.inputPath = QStringLiteral("frame-001");
    partialResults.append(summary);

    emit aitoolkit::testing::AppControllerTestPeer::inferenceWorker(controller)->batchFinished(
        partialResults);
    QCoreApplication::processEvents();

    QCOMPARE(completedSpy.count(), 0);
    QCOMPARE(controller.currentBatchResults().size(), 0);
}

void AppControllerTest::loadModelManifestClearsStaleBatchResults() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString onnxPath = QDir(tempDir.path()).filePath(QStringLiteral("imported.onnx"));
    QFile onnxFile(onnxPath);
    QVERIFY(onnxFile.open(QIODevice::WriteOnly));
    onnxFile.write("fake");
    onnxFile.close();

    const QString manifestPath = QDir(tempDir.path()).filePath(QStringLiteral("imported.json"));
    aitoolkit::core::writeJsonObject(
        manifestPath,
        QJsonObject{
            {QStringLiteral("name"), QStringLiteral("Imported Model")},
            {QStringLiteral("task_type"), QStringLiteral("detection")},
            {QStringLiteral("backend"), QStringLiteral("onnxruntime")},
            {QStringLiteral("model"), QStringLiteral("imported.onnx")},
            {QStringLiteral("input_width"), 640},
            {QStringLiteral("input_height"), 640},
        });

    aitoolkit::ui::AppController controller;
    QSignalSpy contextSpy(&controller, &aitoolkit::ui::AppController::contextChanged);

    aitoolkit::core::InferenceSummary staleSummary;
    staleSummary.inputPath = QStringLiteral("D:/images/previous.png");
    staleSummary.taskType = QStringLiteral("detection");
    staleSummary.detectionCount = 3;
    aitoolkit::testing::AppControllerTestPeer::setCurrentSummary(controller, staleSummary);

    QVector<aitoolkit::core::InferenceSummary> staleBatchResults;
    staleBatchResults.push_back(staleSummary);
    aitoolkit::testing::AppControllerTestPeer::setBatchResults(controller, staleBatchResults);

    controller.loadModelManifest(manifestPath);

    QCOMPARE(controller.currentSummary().inputPath, QString());
    QCOMPARE(controller.currentBatchResults().size(), 0);
    QVERIFY(contextSpy.count() > 0);
}

}  // namespace

QTEST_MAIN(AppControllerTest)
#include "test_app_controller.moc"
