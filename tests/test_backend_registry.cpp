#include <QtTest>

#include <QDir>
#include <QImage>
#include <QTemporaryDir>

#include "core/model_manifest.h"
#include "models/inference_backend.h"
#include "runtime/backend_registry.h"
#include "runtime/onnx_plugin.h"
#include "runtime/openvino_plugin.h"
#include "runtime/tensorrt_plugin.h"

namespace {

class FakeTestPlugin final : public aitoolkit::runtime::BackendPlugin {
public:
    aitoolkit::runtime::BackendInfo info() const override {
        return {QStringLiteral("fake_test"), QStringLiteral("Fake Test"), QStringLiteral("1.0"), false, true};
    }

    QStringList supportedTaskTypes() const override {
        return {QStringLiteral("detection")};
    }

    std::unique_ptr<aitoolkit::models::InferenceBackend> createModel(
        const aitoolkit::core::ModelManifest&,
        int,
        bool) const override {
        return nullptr;
    }
};

class BackendRegistryTest : public QObject {
    Q_OBJECT

private slots:
    void registersAndRetrievesBackend();
    void listsAvailableBackendNames();
    void unregistersBackend();
    void registersBuiltinPlugins();
};

void BackendRegistryTest::registersAndRetrievesBackend() {
    auto& registry = aitoolkit::runtime::BackendRegistry::instance();
  registry.registerBackend(std::make_unique<FakeTestPlugin>());

    auto* backend = registry.getBackend(QStringLiteral("fake_test"));
    QVERIFY(backend != nullptr);
    QCOMPARE(backend->info().displayName, QStringLiteral("Fake Test"));

    registry.unregisterBackend(QStringLiteral("fake_test"));
    QVERIFY(registry.getBackend(QStringLiteral("fake_test")) == nullptr);
}

void BackendRegistryTest::listsAvailableBackendNames() {
    auto& registry = aitoolkit::runtime::BackendRegistry::instance();
    registry.registerBackend(std::make_unique<FakeTestPlugin>());

    const QStringList names = registry.availableBackendNames();
    QVERIFY(names.contains(QStringLiteral("fake_test")));

    registry.unregisterBackend(QStringLiteral("fake_test"));
}

void BackendRegistryTest::unregistersBackend() {
    auto& registry = aitoolkit::runtime::BackendRegistry::instance();
    registry.registerBackend(std::make_unique<FakeTestPlugin>());
    registry.unregisterBackend(QStringLiteral("fake_test"));
    QVERIFY(registry.getBackend(QStringLiteral("fake_test")) == nullptr);
}

void BackendRegistryTest::registersBuiltinPlugins() {
    auto& registry = aitoolkit::runtime::BackendRegistry::instance();
    aitoolkit::runtime::registerOnnxRuntimePlugin();
    aitoolkit::runtime::registerTensorRtPlugin();
    aitoolkit::runtime::registerOpenVinoPlugin();

    const QStringList available = registry.availableBackendNames();
    QVERIFY(available.contains(QStringLiteral("onnxruntime")));
    QCOMPARE(available.size(), 1);

    auto* onnx = registry.getBackend(QStringLiteral("onnxruntime"));
    QVERIFY(onnx != nullptr);
    QVERIFY(onnx->info().isAvailable);

    auto* tensorrt = registry.getBackend(QStringLiteral("tensorrt"));
    QVERIFY(tensorrt != nullptr);
    QVERIFY(!tensorrt->info().isAvailable);

    registry.unregisterBackend(QStringLiteral("onnxruntime"));
    registry.unregisterBackend(QStringLiteral("tensorrt"));
    registry.unregisterBackend(QStringLiteral("openvino"));
}

}  // namespace

QTEST_MAIN(BackendRegistryTest)

#include "test_backend_registry.moc"
