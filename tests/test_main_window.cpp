#include <QtTest>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QWidget>

#include "core/json_utils.h"
#define private public
#include "ui/main_window.h"
#undef private
#include "ui/pages/results_page.h"

namespace {

QString writeManifestFile(const QString& directoryPath, const QString& baseName, const QString& displayName) {
    const QString modelPath = QDir(directoryPath).filePath(baseName + QStringLiteral(".onnx"));
    QFile modelFile(modelPath);
    if (!modelFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return QString();
    }
    modelFile.write("dummy-model");
    modelFile.close();

    const QString manifestPath = QDir(directoryPath).filePath(baseName + QStringLiteral(".json"));
    aitoolkit::core::writeJsonObject(
        manifestPath,
        QJsonObject{
            {QStringLiteral("name"), displayName},
            {QStringLiteral("task_type"), QStringLiteral("detection")},
            {QStringLiteral("backend"), QStringLiteral("onnxruntime")},
            {QStringLiteral("model"), baseName + QStringLiteral(".onnx")},
            {QStringLiteral("input_width"), 640},
            {QStringLiteral("input_height"), 640},
            {QStringLiteral("labels_inline"), QJsonArray{QStringLiteral("person"), QStringLiteral("box")}},
        });
    return manifestPath;
}

aitoolkit::core::InferenceSummary makeSummary(const QString& modelName, const QString& imagePath) {
    aitoolkit::core::InferenceSummary summary;
    summary.modelName = modelName;
    summary.inputPath = imagePath;
    summary.detectionCount = 3;
    summary.elapsedMs = 12.5;
    summary.detections.append({0, QStringLiteral("box"), 0.9f, QRectF(1.0, 2.0, 3.0, 4.0)});
    return summary;
}

void seedCurrentResult(aitoolkit::ui::MainWindow& window, const aitoolkit::core::InferenceSummary& summary) {
    window.currentSummary_ = summary;
    window.resultsPage_->setSummary(summary);
    window.runStatusLabel_->setText(
        QStringLiteral("已完成，共 %1 个目标，耗时 %2 ms")
            .arg(summary.detectionCount)
            .arg(QString::number(summary.elapsedMs, 'f', 2)));
    window.nextStepLabel_->setText(QStringLiteral("可查看结果明细，或直接导出 JSON。"));
}

void verifyHasResultsState(aitoolkit::ui::MainWindow& window) {
    auto* contextResultValue = window.findChild<QLabel*>(QStringLiteral("ContextResultValue"));
    QVERIFY(contextResultValue != nullptr);
    QVERIFY(contextResultValue->text().contains(QStringLiteral("已完成")));

    auto* contextNextStepValue = window.findChild<QLabel*>(QStringLiteral("ContextNextStepValue"));
    QVERIFY(contextNextStepValue != nullptr);
    QVERIFY(contextNextStepValue->text().contains(QStringLiteral("导出 JSON")));
}

void verifyClearedResultsState(aitoolkit::ui::MainWindow& window) {
    QCOMPARE(window.currentSummary_.inputPath, QString());
    QCOMPARE(window.currentSummary_.detections.size(), 0);

    auto* resultsSummaryLabel = window.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(resultsSummaryLabel != nullptr);
    QCOMPARE(resultsSummaryLabel->text(), QStringLiteral("当前还没有推理结果"));

    auto* detectionsTable = window.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(detectionsTable != nullptr);
    QCOMPARE(detectionsTable->rowCount(), 0);

    auto* contextResultValue = window.findChild<QLabel*>(QStringLiteral("ContextResultValue"));
    QVERIFY(contextResultValue != nullptr);
    QCOMPARE(contextResultValue->text(), QStringLiteral("尚未执行检测"));
}

class MainWindowTest : public QObject {
    Q_OBJECT

private slots:
    void buildsThreePaneShell();
    void changingImageClearsStoredAndVisibleResults();
    void changingModelClearsStoredAndVisibleResults();
    void recentInputClickReturnsToInferencePage();
    void recentModelClickLoadsManifestAndReturnsToInferencePage();
};

void MainWindowTest::buildsThreePaneShell() {
    aitoolkit::ui::MainWindow window;

    QVERIFY(window.centralWidget() != nullptr);

    const auto stacks = window.findChildren<QStackedWidget*>();
    QCOMPARE(stacks.size(), 1);
    QCOMPARE(stacks.front()->count(), 5);

    const auto navPanel = window.findChild<QWidget*>(QStringLiteral("NavPanel"));
    QVERIFY(navPanel != nullptr);

    const auto contextPanel = window.findChild<QWidget*>(QStringLiteral("ContextPanel"));
    QVERIFY(contextPanel != nullptr);

    const auto contextModelTitle = window.findChild<QLabel*>(QStringLiteral("ContextModelTitle"));
    QVERIFY(contextModelTitle != nullptr);

    const auto contextImageTitle = window.findChild<QLabel*>(QStringLiteral("ContextImageTitle"));
    QVERIFY(contextImageTitle != nullptr);

    const auto contextResultTitle = window.findChild<QLabel*>(QStringLiteral("ContextResultTitle"));
    QVERIFY(contextResultTitle != nullptr);

    const auto contextNextStepTitle = window.findChild<QLabel*>(QStringLiteral("ContextNextStepTitle"));
    QVERIFY(contextNextStepTitle != nullptr);
}

void MainWindowTest::changingImageClearsStoredAndVisibleResults() {
    aitoolkit::ui::MainWindow window;

    auto* stack = window.findChild<QStackedWidget*>();
    QVERIFY(stack != nullptr);

    auto* inferencePage = stack->widget(2);
    QVERIFY(inferencePage != nullptr);
    QVERIFY(QMetaObject::invokeMethod(
        inferencePage, "imageSelected", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("D:/images/first.jpg"))));

    const auto summary = makeSummary(QStringLiteral("Warehouse Detector"), QStringLiteral("D:/images/first.jpg"));
    seedCurrentResult(window, summary);
    QVERIFY(!window.currentSummary_.inputPath.isEmpty());
    verifyHasResultsState(window);

    auto* resultsSummaryLabel = window.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(resultsSummaryLabel != nullptr);
    QVERIFY(resultsSummaryLabel->text().contains(QStringLiteral("Warehouse Detector")));

    auto* detectionsTable = window.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(detectionsTable != nullptr);
    QCOMPARE(detectionsTable->rowCount(), 1);

    QVERIFY(QMetaObject::invokeMethod(
        inferencePage, "imageSelected", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("D:/images/second.jpg"))));

    verifyClearedResultsState(window);
}

void MainWindowTest::changingModelClearsStoredAndVisibleResults() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString firstManifestPath =
        writeManifestFile(tempDir.path(), QStringLiteral("model_a"), QStringLiteral("Warehouse Detector A"));
    const QString secondManifestPath =
        writeManifestFile(tempDir.path(), QStringLiteral("model_b"), QStringLiteral("Warehouse Detector B"));
    QVERIFY(!firstManifestPath.isEmpty());
    QVERIFY(!secondManifestPath.isEmpty());

    aitoolkit::ui::MainWindow window;

    auto* stack = window.findChild<QStackedWidget*>();
    QVERIFY(stack != nullptr);

    auto* modelsPage = stack->widget(1);
    QVERIFY(modelsPage != nullptr);
    QVERIFY(QMetaObject::invokeMethod(
        modelsPage, "modelManifestSelected", Qt::DirectConnection, Q_ARG(QString, firstManifestPath)));
    QVERIFY(QMetaObject::invokeMethod(
        stack->widget(2), "imageSelected", Qt::DirectConnection, Q_ARG(QString, QStringLiteral("D:/images/first.jpg"))));

    const auto summary = makeSummary(QStringLiteral("Warehouse Detector A"), QStringLiteral("D:/images/first.jpg"));
    seedCurrentResult(window, summary);
    QVERIFY(!window.currentSummary_.inputPath.isEmpty());
    verifyHasResultsState(window);

    auto* resultsSummaryLabel = window.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(resultsSummaryLabel != nullptr);
    QVERIFY(resultsSummaryLabel->text().contains(QStringLiteral("Warehouse Detector A")));

    auto* detectionsTable = window.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(detectionsTable != nullptr);
    QCOMPARE(detectionsTable->rowCount(), 1);

    QVERIFY(QMetaObject::invokeMethod(
        modelsPage, "modelManifestSelected", Qt::DirectConnection, Q_ARG(QString, secondManifestPath)));

    verifyClearedResultsState(window);
}

void MainWindowTest::recentInputClickReturnsToInferencePage() {
    aitoolkit::ui::MainWindow window;

    const QString imagePath = QStringLiteral("D:/images/example.jpg");
    auto* stack = window.findChild<QStackedWidget*>();
    QVERIFY(stack != nullptr);

    auto* inferencePage = stack->widget(2);
    QVERIFY(inferencePage != nullptr);
    QVERIFY(QMetaObject::invokeMethod(inferencePage, "imageSelected", Qt::DirectConnection, Q_ARG(QString, imagePath)));

    stack->setCurrentIndex(4);

    auto* recentInputsList = window.findChild<QListWidget*>(QStringLiteral("RecentInputsList"));
    QVERIFY(recentInputsList != nullptr);
    QVERIFY(recentInputsList->item(0) != nullptr);

    QVERIFY(QMetaObject::invokeMethod(
        recentInputsList,
        "itemClicked",
        Qt::DirectConnection,
        Q_ARG(QListWidgetItem*, recentInputsList->item(0))));

    QCOMPARE(stack->currentIndex(), 2);

    auto* imagePathLabel = window.findChild<QLabel*>(QStringLiteral("InferenceImagePathLabel"));
    QVERIFY(imagePathLabel != nullptr);
    QVERIFY(imagePathLabel->text().contains(imagePath));
}

void MainWindowTest::recentModelClickLoadsManifestAndReturnsToInferencePage() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString manifestPath =
        writeManifestFile(tempDir.path(), QStringLiteral("model"), QStringLiteral("Warehouse Detector"));
    QVERIFY(!manifestPath.isEmpty());

    aitoolkit::ui::MainWindow window;

    auto* stack = window.findChild<QStackedWidget*>();
    QVERIFY(stack != nullptr);

    auto* modelsPage = stack->widget(1);
    QVERIFY(modelsPage != nullptr);
    QVERIFY(QMetaObject::invokeMethod(modelsPage, "modelManifestSelected", Qt::DirectConnection, Q_ARG(QString, manifestPath)));

    stack->setCurrentIndex(4);

    auto* recentModelsList = window.findChild<QListWidget*>(QStringLiteral("RecentModelsList"));
    QVERIFY(recentModelsList != nullptr);
    QVERIFY(recentModelsList->item(0) != nullptr);

    QVERIFY(QMetaObject::invokeMethod(
        recentModelsList,
        "itemClicked",
        Qt::DirectConnection,
        Q_ARG(QListWidgetItem*, recentModelsList->item(0))));

    QCOMPARE(stack->currentIndex(), 2);

    auto* manifestPathLabel = window.findChild<QLabel*>(QStringLiteral("ManifestPathLabel"));
    QVERIFY(manifestPathLabel != nullptr);
    QVERIFY(manifestPathLabel->text().contains(QDir::cleanPath(manifestPath)));

    auto* manifestSummaryLabel = window.findChild<QLabel*>(QStringLiteral("ManifestSummaryLabel"));
    QVERIFY(manifestSummaryLabel != nullptr);
    QVERIFY(manifestSummaryLabel->text().contains(QStringLiteral("Warehouse Detector")));
}

}  // namespace

QTEST_MAIN(MainWindowTest)

#include "test_main_window.moc"
