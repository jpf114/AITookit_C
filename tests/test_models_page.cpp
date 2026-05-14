#include <QtTest>

#include <QLabel>

#include "core/model_manifest.h"
#include "ui/pages/models_page.h"

namespace {

class ModelsPageTest : public QObject {
    Q_OBJECT

private slots:
    void showsManifestSummary();
};

void ModelsPageTest::showsManifestSummary() {
    aitoolkit::ui::ModelsPage page;

    aitoolkit::core::ModelManifest manifest;
    manifest.manifestPath = QStringLiteral("D:/models/yolo/model.json");
    manifest.name = QStringLiteral("Warehouse Detector");
    manifest.taskType = QStringLiteral("detection");
    manifest.backendType = QStringLiteral("onnxruntime");
    manifest.modelPath = QStringLiteral("D:/models/yolo/model.onnx");
    manifest.inputWidth = 640;
    manifest.inputHeight = 640;
    manifest.labels = {QStringLiteral("person"), QStringLiteral("box"), QStringLiteral("forklift")};

    page.setCurrentManifest(manifest);

    auto* modelLoadSection = page.findChild<QWidget*>(QStringLiteral("ModelLoadSection"));
    QVERIFY(modelLoadSection != nullptr);

    auto* modelSummarySection = page.findChild<QWidget*>(QStringLiteral("ModelSummarySection"));
    QVERIFY(modelSummarySection != nullptr);

    auto* pathLabel = page.findChild<QLabel*>(QStringLiteral("ManifestPathLabel"));
    QVERIFY(pathLabel != nullptr);
    QVERIFY(pathLabel->text().contains(manifest.manifestPath));

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ManifestSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(summaryLabel->text().contains(QStringLiteral("Warehouse Detector")));
    QVERIFY(summaryLabel->text().contains(QStringLiteral("onnxruntime")));
    QVERIFY(summaryLabel->text().contains(QStringLiteral("640 x 640")));
    QVERIFY(summaryLabel->text().contains(QStringLiteral("3")));
}

}  // namespace

QTEST_MAIN(ModelsPageTest)

#include "test_models_page.moc"
