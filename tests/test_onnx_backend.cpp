#include <QtTest>

#include <QDir>
#include <QFileInfo>

#include "runtime/onnx_backend.h"

#include <numeric>
#include <stdexcept>

namespace {

QString testModelPath() {
    return QDir(QStringLiteral(AITOOLKIT_TEST_MODELS_DIR)).filePath(QStringLiteral("yolov8n.onnx"));
}

std::vector<int64_t> resolveRuntimeShape(const std::vector<int64_t>& modelShape) {
    std::vector<int64_t> shape = modelShape;
    if (shape.size() != 4) {
        return {1, 3, 640, 640};
    }

    if (shape[0] <= 0) {
        shape[0] = 1;
    }
    if (shape[2] <= 0) {
        shape[2] = 640;
    }
    if (shape[3] <= 0) {
        shape[3] = 640;
    }
    return shape;
}

std::size_t elementCount(const std::vector<int64_t>& shape) {
    return std::accumulate(
        shape.begin(),
        shape.end(),
        static_cast<std::size_t>(1),
        [](const std::size_t product, const int64_t dimension) {
            return product * static_cast<std::size_t>(dimension);
        });
}

class OnnxBackendTest : public QObject {
    Q_OBJECT

private:
    bool hasModel_ = false;
    QString modelPath_;

private slots:
    void initTestCase();
    void rejectsEmptyModelPath();
    void loadsModelWhenPresent();
    void warmupAndRunWhenPresent();
};

void OnnxBackendTest::initTestCase() {
    modelPath_ = testModelPath();
    hasModel_ = QFileInfo::exists(modelPath_);
    if (!hasModel_) {
        qWarning("yolov8n.onnx not found at %s — ONNX integration tests will be skipped",
                 qPrintable(modelPath_));
    }
}

void OnnxBackendTest::rejectsEmptyModelPath() {
    QVERIFY_EXCEPTION_THROWN(aitoolkit::runtime::OnnxBackend(QString(), 1, false), std::runtime_error);
}

void OnnxBackendTest::loadsModelWhenPresent() {
    if (!hasModel_) {
        QSKIP("yolov8n.onnx not available");
    }

    aitoolkit::runtime::OnnxBackend backend(modelPath_, 1, false);
    QVERIFY(backend.isLoaded());
    QCOMPARE(backend.modelPath(), QDir::cleanPath(modelPath_));
    QVERIFY(!backend.inputNames().empty());
    QVERIFY(!backend.outputNames().empty());
    QVERIFY(!backend.inputShape().empty());
}

void OnnxBackendTest::warmupAndRunWhenPresent() {
    if (!hasModel_) {
        QSKIP("yolov8n.onnx not available");
    }

    aitoolkit::runtime::OnnxBackend backend(modelPath_, 1, false);
    backend.warmup();

    const std::vector<int64_t> runtimeShape = resolveRuntimeShape(backend.inputShape());
    const std::size_t count = elementCount(runtimeShape);
    QVERIFY(count > 0);

    const std::vector<float> inputData(count, 0.5f);
    const auto outputs = backend.run(inputData, runtimeShape);
    QVERIFY(!outputs.empty());
    QVERIFY(!outputs.front().values.empty());
}

}  // namespace

QTEST_MAIN(OnnxBackendTest)

#include "test_onnx_backend.moc"
