#include <QtTest>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTextStream>

#include <stdexcept>

#include "core/json_utils.h"
#include "core/model_manifest.h"

namespace {

class ModelManifestTest : public QObject {
    Q_OBJECT

private slots:
    void loadsManifestAndResolvesRelativePaths();
    void fallsBackToLabelsFileWhenInlineLabelsAreMissing();
    void normalizesCaseForManifestIdentifiers();
    void failsWhenRequiredFieldIsMissing();
    void failsWhenTaskTypeIsUnsupported();
    void failsWhenLabelsFileIsMissing();
    void failsWhenInputDimensionIsFractional();
    void failsWhenThresholdFieldIsNotNumeric();
    void failsWhenLabelsInlineIsNotAnArray();
    void failsWhenLabelsInlineContainsNonStringItems();
};

void writeTextFile(const QString& filePath, const QString& content) {
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << content;
}

std::runtime_error expectManifestLoadError(const QString& manifestPath) {
    try {
        static_cast<void>(aitoolkit::core::loadModelManifest(manifestPath));
    } catch (const std::runtime_error& error) {
        return error;
    }

    throw std::runtime_error("Expected loadModelManifest() to throw std::runtime_error");
}

QJsonObject makeBaseManifestObject() {
    return QJsonObject{
        {QStringLiteral("name"), QStringLiteral("Sample YOLO")},
        {QStringLiteral("task_type"), QStringLiteral("detection")},
        {QStringLiteral("backend"), QStringLiteral("onnxruntime")},
        {QStringLiteral("model"), QStringLiteral("model.onnx")},
        {QStringLiteral("input_width"), 640},
        {QStringLiteral("input_height"), 640},
    };
}

void ModelManifestTest::loadsManifestAndResolvesRelativePaths() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("weights/model.onnx");
    QVERIFY(QDir(tempDir.path()).mkpath("weights"));
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    const QJsonObject manifestObject{
        {QStringLiteral("name"), QStringLiteral("Sample YOLO")},
        {QStringLiteral("task_type"), QStringLiteral("detection")},
        {QStringLiteral("backend"), QStringLiteral("onnxruntime")},
        {QStringLiteral("model"), QStringLiteral("weights/model.onnx")},
        {QStringLiteral("labels"), QStringLiteral("labels.txt")},
        {QStringLiteral("decoder"), QStringLiteral("yolo")},
        {QStringLiteral("input_width"), 640},
        {QStringLiteral("input_height"), 640},
        {QStringLiteral("confidence_threshold"), 0.25},
        {QStringLiteral("nms_threshold"), 0.45},
        {QStringLiteral("labels_inline"), QJsonArray{QStringLiteral("cat"), QStringLiteral("dog")}},
    };

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const aitoolkit::core::ModelManifest manifest = aitoolkit::core::loadModelManifest(manifestPath);
    const QStringList expectedInlineLabels{QStringLiteral("cat"), QStringLiteral("dog")};

    QCOMPARE(manifest.manifestPath, QDir::cleanPath(manifestPath));
    QCOMPARE(manifest.name, QStringLiteral("Sample YOLO"));
    QCOMPARE(manifest.taskType, QStringLiteral("detection"));
    QCOMPARE(manifest.backendType, QStringLiteral("onnxruntime"));
    QCOMPARE(manifest.decoder, QStringLiteral("yolo"));
    QCOMPARE(manifest.inputWidth, 640);
    QCOMPARE(manifest.inputHeight, 640);
    QCOMPARE(manifest.confidenceThreshold, 0.25);
    QCOMPARE(manifest.nmsThreshold, 0.45);
    QCOMPARE(manifest.modelPath, QDir(tempDir.path()).filePath("weights/model.onnx"));
    QCOMPARE(manifest.labelsPath, QDir(tempDir.path()).filePath("labels.txt"));
    QCOMPARE(manifest.labelsInline, expectedInlineLabels);
    QCOMPARE(manifest.labels, expectedInlineLabels);
}

void ModelManifestTest::fallsBackToLabelsFileWhenInlineLabelsAreMissing() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString labelsPath = QDir(tempDir.path()).filePath("labels.txt");
    writeTextFile(labelsPath, QStringLiteral("person\ncar\n"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    const QJsonObject manifestObject{
        {QStringLiteral("name"), QStringLiteral("Labels From File")},
        {QStringLiteral("task_type"), QStringLiteral("detection")},
        {QStringLiteral("backend"), QStringLiteral("onnxruntime")},
        {QStringLiteral("model"), QStringLiteral("model.onnx")},
        {QStringLiteral("labels"), QStringLiteral("labels.txt")},
        {QStringLiteral("input_width"), 320},
        {QStringLiteral("input_height"), 320},
    };

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const aitoolkit::core::ModelManifest manifest = aitoolkit::core::loadModelManifest(manifestPath);
    const QStringList expectedLabels{QStringLiteral("person"), QStringLiteral("car")};

    QCOMPARE(manifest.labelsInline, QStringList());
    QCOMPARE(manifest.labels, expectedLabels);
}

void ModelManifestTest::normalizesCaseForManifestIdentifiers() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    const QJsonObject manifestObject{
        {QStringLiteral("name"), QStringLiteral("Case Variant")},
        {QStringLiteral("task_type"), QStringLiteral("Detection")},
        {QStringLiteral("backend"), QStringLiteral("OnnxRuntime")},
        {QStringLiteral("model"), QStringLiteral("model.onnx")},
        {QStringLiteral("decoder"), QStringLiteral("Yolo_V8")},
        {QStringLiteral("input_width"), 640},
        {QStringLiteral("input_height"), 640},
    };

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const aitoolkit::core::ModelManifest manifest = aitoolkit::core::loadModelManifest(manifestPath);
    QCOMPARE(manifest.taskType, QStringLiteral("detection"));
    QCOMPARE(manifest.backendType, QStringLiteral("onnxruntime"));
    QCOMPARE(manifest.decoder, QStringLiteral("yolo_v8"));
}

void ModelManifestTest::failsWhenRequiredFieldIsMissing() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    const QJsonObject manifestObject{
        {QStringLiteral("task_type"), QStringLiteral("detection")},
        {QStringLiteral("backend"), QStringLiteral("onnxruntime")},
        {QStringLiteral("model"), QStringLiteral("model.onnx")},
        {QStringLiteral("input_width"), 640},
        {QStringLiteral("input_height"), 640},
    };

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const std::runtime_error error = expectManifestLoadError(manifestPath);
    const QString message = QString::fromStdString(error.what());
    QVERIFY(message.contains(QDir::toNativeSeparators(manifestPath)));
    QVERIFY(message.contains(QStringLiteral("name")));
}

void ModelManifestTest::failsWhenTaskTypeIsUnsupported() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    QJsonObject manifestObject = makeBaseManifestObject();
    manifestObject.insert(QStringLiteral("task_type"), QStringLiteral("tracking"));

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const std::runtime_error error = expectManifestLoadError(manifestPath);
    const QString message = QString::fromStdString(error.what());
    QVERIFY(message.contains(QDir::toNativeSeparators(manifestPath)));
    QVERIFY(message.contains(QStringLiteral("task_type")));
    QVERIFY(message.contains(QStringLiteral("unsupported")));
}

void ModelManifestTest::failsWhenLabelsFileIsMissing() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    const QJsonObject manifestObject{
        {QStringLiteral("name"), QStringLiteral("Missing Labels")},
        {QStringLiteral("task_type"), QStringLiteral("detection")},
        {QStringLiteral("backend"), QStringLiteral("onnxruntime")},
        {QStringLiteral("model"), QStringLiteral("model.onnx")},
        {QStringLiteral("labels"), QStringLiteral("missing-labels.txt")},
        {QStringLiteral("input_width"), 640},
        {QStringLiteral("input_height"), 640},
    };

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const std::runtime_error error = expectManifestLoadError(manifestPath);
    const QString message = QString::fromStdString(error.what());
    QVERIFY(message.contains(QDir::toNativeSeparators(manifestPath)));
    QVERIFY(message.contains(QStringLiteral("labels")));
}

void ModelManifestTest::failsWhenInputDimensionIsFractional() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    QJsonObject manifestObject = makeBaseManifestObject();
    manifestObject.insert(QStringLiteral("input_width"), 640.5);

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const std::runtime_error error = expectManifestLoadError(manifestPath);
    const QString message = QString::fromStdString(error.what());
    QVERIFY(message.contains(QDir::toNativeSeparators(manifestPath)));
    QVERIFY(message.contains(QStringLiteral("input_width")));
}

void ModelManifestTest::failsWhenThresholdFieldIsNotNumeric() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    QJsonObject manifestObject = makeBaseManifestObject();
    manifestObject.insert(QStringLiteral("confidence_threshold"), QStringLiteral("high"));

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const std::runtime_error error = expectManifestLoadError(manifestPath);
    const QString message = QString::fromStdString(error.what());
    QVERIFY(message.contains(QDir::toNativeSeparators(manifestPath)));
    QVERIFY(message.contains(QStringLiteral("confidence_threshold")));
}

void ModelManifestTest::failsWhenLabelsInlineIsNotAnArray() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    QJsonObject manifestObject = makeBaseManifestObject();
    manifestObject.insert(QStringLiteral("labels_inline"), QStringLiteral("cat"));

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const std::runtime_error error = expectManifestLoadError(manifestPath);
    const QString message = QString::fromStdString(error.what());
    QVERIFY(message.contains(QDir::toNativeSeparators(manifestPath)));
    QVERIFY(message.contains(QStringLiteral("labels_inline")));
}

void ModelManifestTest::failsWhenLabelsInlineContainsNonStringItems() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    writeTextFile(modelPath, QStringLiteral("dummy-model"));

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    QJsonObject manifestObject = makeBaseManifestObject();
    manifestObject.insert(QStringLiteral("labels_inline"), QJsonArray{QStringLiteral("cat"), 42});

    aitoolkit::core::writeJsonObject(manifestPath, manifestObject);

    const std::runtime_error error = expectManifestLoadError(manifestPath);
    const QString message = QString::fromStdString(error.what());
    QVERIFY(message.contains(QDir::toNativeSeparators(manifestPath)));
    QVERIFY(message.contains(QStringLiteral("labels_inline")));
}

}  // namespace

QTEST_MAIN(ModelManifestTest)

#include "test_model_manifest.moc"
