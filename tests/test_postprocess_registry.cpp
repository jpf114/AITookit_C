#include <QtTest>

#include "models/postprocess_registry.h"
#include "models/yolo_detection_model.h"
#include "core/model_manifest.h"

using aitoolkit::models::PostprocessRegistry;
using aitoolkit::models::PostprocessInput;
using aitoolkit::models::PostprocessFn;
using aitoolkit::models::YoloDetectionModel;
using aitoolkit::core::DetectionItem;
using aitoolkit::core::ModelManifest;

class TestPostprocessRegistry : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        YoloDetectionModel::registerBuiltinDecoders();
        registry_ = &PostprocessRegistry::instance();
    }

    void testRegisterAndGetDecoder() {
        PostprocessFn fn = [](const PostprocessInput&) -> QVector<DetectionItem> {
            return {DetectionItem{0, QStringLiteral("test"), 0.9f, QRectF(0, 0, 100, 100), QColor(Qt::red)}};
        };
        registry_->registerDecoder("test_decoder_unit", fn);

        QVERIFY(registry_->hasDecoder("test_decoder_unit"));
        auto retrieved = registry_->getDecoder("test_decoder_unit");
        QVERIFY(retrieved != nullptr);
    }

    void testGetNonexistentDecoder() {
        QVERIFY(!registry_->hasDecoder("nonexistent"));
        QVERIFY(registry_->getDecoder("nonexistent") == nullptr);
    }

    void testYoloV8DecoderIsRegistered() {
        QVERIFY(registry_->hasDecoder("yolo_v8"));
        QVERIFY(registry_->getDecoder("yolo_v8") != nullptr);
    }

private:
    PostprocessRegistry* registry_ = nullptr;
};

QTEST_MAIN(TestPostprocessRegistry)
#include "test_postprocess_registry.moc"
