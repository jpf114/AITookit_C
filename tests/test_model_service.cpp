#include <QtTest>

#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QTemporaryDir>

#include "core/json_utils.h"
#include "runtime/backend_registry.h"
#include "services/model_service.h"

namespace {

class FakeInferenceBackend final : public aitoolkit::models::InferenceBackend {
public:
    explicit FakeInferenceBackend(aitoolkit::core::ModelManifest manifest)
        : manifest_(std::move(manifest)) {
    }

    const aitoolkit::core::ModelManifest& manifest() const noexcept override {
        return manifest_;
    }

    QVector<aitoolkit::core::DetectionItem> detect(const cv::Mat&, double, double) const override {
        return {};
    }

    QString backendName() const noexcept override {
        return QStringLiteral("Fake Backend");
    }

private:
    aitoolkit::core::ModelManifest manifest_;
};

class FakeBackendPlugin final : public aitoolkit::runtime::BackendPlugin {
public:
    mutable int lastThreadCount = -1;
    mutable bool lastUseGPU = false;
    mutable int createModelCalls = 0;

    aitoolkit::runtime::BackendInfo info() const override {
        return {
            QStringLiteral("fake_backend"),
            QStringLiteral("Fake Backend"),
            QStringLiteral("1.0"),
            true,
            true,
        };
    }

    QStringList supportedTaskTypes() const override {
        return {QStringLiteral("detection")};
    }

    std::unique_ptr<aitoolkit::models::InferenceBackend> createModel(
        const aitoolkit::core::ModelManifest& manifest,
        int threadCount,
        bool useGPU) const override {
        ++createModelCalls;
        lastThreadCount = threadCount;
        lastUseGPU = useGPU;
        return std::make_unique<FakeInferenceBackend>(manifest);
    }
};

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

}  // namespace

QTEST_MAIN(ModelServiceTest)

#include "test_model_service.moc"
