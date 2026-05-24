#include <QtTest>

#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QTemporaryDir>

#include "core/json_utils.h"
#include "core/model_manifest.h"
#include "runtime/backend_registry.h"
#include "services/model_service.h"
#include "test_helpers.h"

namespace {

using test_helpers::FakeBackendPlugin;
using test_helpers::FakeInferenceBackend;

QString writeManifestFile(const QString& directoryPath, const QString& backendName) {
    QFile modelFile(QDir(directoryPath).filePath(QStringLiteral("dummy.onnx")));
    if (modelFile.open(QIODevice::WriteOnly)) {
        modelFile.write("fake");
        modelFile.close();
    }

    const QString manifestPath = QDir(directoryPath).filePath(QStringLiteral("model.json"));
    aitoolkit::core::writeJsonObject(
        manifestPath,
        QJsonObject{
            {QStringLiteral("name"), QStringLiteral("Fake Model")},
            {QStringLiteral("task_type"), QStringLiteral("detection")},
            {QStringLiteral("backend"), backendName},
            {QStringLiteral("model"), QStringLiteral("dummy.onnx")},
            {QStringLiteral("input_width"), 640},
            {QStringLiteral("input_height"), 640},
        });
    return manifestPath;
}

class ModelServiceTest : public QObject {
    Q_OBJECT

private slots:
    void loadModelUsesRegisteredBackendAndForwardsGpuFlag();
    void createManifestInfersClassificationTaskType();
    void createManifestInfersSegmentationTaskType();
};

void ModelServiceTest::loadModelUsesRegisteredBackendAndForwardsGpuFlag() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString manifestPath = writeManifestFile(tempDir.path(), QStringLiteral("fake_backend"));
    QVERIFY(!manifestPath.isEmpty());

    auto plugin = std::make_unique<FakeBackendPlugin>();
    auto* pluginPtr = plugin.get();
    aitoolkit::runtime::BackendRegistry::instance().registerBackend(std::move(plugin));

    aitoolkit::services::ModelService service;
    service.setThreadCount(4);
    service.setUseGPU(true);

    std::unique_ptr<aitoolkit::models::InferenceBackend> model;
    try {
        model = service.loadModel(manifestPath);
    } catch (const std::exception& e) {
        QFAIL(e.what());
    }

    QCOMPARE(pluginPtr->createModelCalls, 1);
    QCOMPARE(pluginPtr->lastThreadCount, 4);
    QCOMPARE(pluginPtr->lastUseGPU, true);
    QVERIFY(model != nullptr);

    aitoolkit::runtime::BackendRegistry::instance().unregisterBackend(QStringLiteral("fake_backend"));
}

QString writeOnnxFile(const QString& directoryPath, const QString& fileName) {
    const QString onnxPath = QDir(directoryPath).filePath(fileName);
    QFile onnxFile(onnxPath);
    if (onnxFile.open(QIODevice::WriteOnly)) {
        onnxFile.write("fake-onnx");
        onnxFile.close();
    }
    return onnxPath;
}

void ModelServiceTest::createManifestInfersClassificationTaskType() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString onnxPath = writeOnnxFile(tempDir.path(), QStringLiteral("yolov8n-cls.onnx"));
    aitoolkit::services::ModelService service;
    const auto manifest = service.createManifestFromOnnx(
        onnxPath, QStringLiteral("YOLOv8n-cls"), 640, 640, 0.25, 0.45, {});

    QCOMPARE(manifest.taskType, QStringLiteral("classification"));
    QCOMPARE(manifest.inputWidth, 224);
    QCOMPARE(manifest.inputHeight, 224);
    QCOMPARE(manifest.decoder, QString());
}

void ModelServiceTest::createManifestInfersSegmentationTaskType() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString onnxPath = writeOnnxFile(tempDir.path(), QStringLiteral("yolov8n-seg.onnx"));
    aitoolkit::services::ModelService service;
    const auto manifest = service.createManifestFromOnnx(
        onnxPath, QStringLiteral("YOLOv8n-seg"), 640, 640, 0.25, 0.45, {});

    QCOMPARE(manifest.taskType, QStringLiteral("segmentation"));
    QCOMPARE(manifest.decoder, QStringLiteral("yolo_v8"));
    QCOMPARE(manifest.inputWidth, 640);
    QCOMPARE(manifest.inputHeight, 640);
}

}  // namespace

QTEST_MAIN(ModelServiceTest)

#include "test_model_service.moc"
