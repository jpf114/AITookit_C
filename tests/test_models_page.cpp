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

    auto* pageLead = page.findChild<QLabel*>(QStringLiteral("PageLead"));
    QVERIFY(pageLead != nullptr);
    QCOMPARE(pageLead->text(), QStringLiteral("先加载一个模型清单，再确认模型信息是否符合当前任务。"));
    QVERIFY(pageLead->wordWrap());

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

    auto* sectionHint = page.findChild<QLabel*>(QStringLiteral("SectionHint"));
    QVERIFY(sectionHint != nullptr);
    QCOMPARE(sectionHint->text(), QStringLiteral("支持选择 JSON 模型清单文件。"));
    QVERIFY(sectionHint->wordWrap());

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
