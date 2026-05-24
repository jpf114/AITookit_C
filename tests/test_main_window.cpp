#include <QtTest>

#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QWidget>

#include "core/json_utils.h"
#define AITOOLKIT_TESTING 1
#include "test_peers.h"
#include "ui/main_window.h"
#include "ui/app_controller.h"
#include "ui/pages/inference_page.h"
#include "ui/pages/results_page.h"
#include "ui/pages/settings_page.h"

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

QString writeImageFile(const QString& directoryPath, const QString& fileName) {
    const QString imagePath = QDir(directoryPath).filePath(fileName);
    QImage image(64, 64, QImage::Format_RGB32);
    image.fill(Qt::red);
    return image.save(imagePath, "BMP") ? imagePath : QString();
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
    aitoolkit::testing::MainWindowTestPeer::controller(window)->applyInferenceResult(summary);
    aitoolkit::testing::MainWindowTestPeer::resultsPage(window)->setSummary(summary);
    aitoolkit::testing::MainWindowTestPeer::runStatusLabel(window)->setText(
        QStringLiteral("\u5df2\u5b8c\u6210\uff0c\u5171 %1 \u4e2a\u76ee\u6807\uff0c\u8017\u65f6 %2 ms")
            .arg(summary.detectionCount)
            .arg(QString::number(summary.elapsedMs, 'f', 2)));
    aitoolkit::testing::MainWindowTestPeer::nextStepLabel(window)->setText(QStringLiteral("\u53ef\u67e5\u770b\u7ed3\u679c\u660e\u7ec6\uff0c\u6216\u76f4\u63a5\u5bfc\u51fa JSON\u3002"));
}

void verifyHasResultsState(aitoolkit::ui::MainWindow& window) {
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::runStatusLabel(window) != nullptr);
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::runStatusLabel(window)->text().contains(QStringLiteral("12.50")));

    QVERIFY(aitoolkit::testing::MainWindowTestPeer::nextStepLabel(window) != nullptr);
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::nextStepLabel(window)->text().contains(QStringLiteral("JSON")));
}

void verifyClearedResultsState(aitoolkit::ui::MainWindow& window) {
    QCOMPARE(aitoolkit::testing::MainWindowTestPeer::controller(window)->currentSummary().inputPath, QString());
    QCOMPARE(aitoolkit::testing::MainWindowTestPeer::controller(window)->currentSummary().detections.size(), 0);

    auto* resultsSummaryLabel = window.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(resultsSummaryLabel != nullptr);
    QVERIFY(!resultsSummaryLabel->text().contains(QStringLiteral("Warehouse Detector")));

    auto* detectionsTable = window.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(detectionsTable != nullptr);
    QCOMPARE(detectionsTable->rowCount(), 0);

    auto* contextResultValue = aitoolkit::testing::MainWindowTestPeer::runStatusLabel(window);
    QVERIFY(contextResultValue != nullptr);
    QVERIFY(!contextResultValue->text().contains(QStringLiteral("12.50")));
}

class MainWindowTest : public QObject {
    Q_OBJECT

private slots:
    void buildsThreePaneShell();
    void changingImageClearsStoredAndVisibleResults();
    void changingModelClearsStoredAndVisibleResults();
    void completedVideoSummaryKeepsResultFocusedContext();
    void classificationSummaryUsesTaskAwareContextCopy();
    void selectingBatchResultUpdatesCurrentExportContext();
    void exportFileNamesFollowSelectedResult();
    void batchExportFileNameFollowsSourceContext();
    void recentInputClickReturnsToInferencePage();
    void unreadableRecentInputClearsImageState();
    void recentModelClickLoadsManifestAndReturnsToInferencePage();
    void resolvesDownloadScriptFromInstalledShareDirectory();
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

    QVERIFY(aitoolkit::testing::MainWindowTestPeer::modelStatusTitleLabel(window) != nullptr);
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::modelStatusTitleLabel(window)->text().contains(QStringLiteral("模型")));

    QVERIFY(aitoolkit::testing::MainWindowTestPeer::imageStatusTitleLabel(window) != nullptr);
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::imageStatusTitleLabel(window)->text().contains(QStringLiteral("图像")));

    QVERIFY(aitoolkit::testing::MainWindowTestPeer::resultStatusTitleLabel(window) != nullptr);
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::resultStatusTitleLabel(window)->text().contains(QStringLiteral("结果")));

    QVERIFY(aitoolkit::testing::MainWindowTestPeer::nextStepTitleLabel(window) != nullptr);
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::nextStepTitleLabel(window)->text().contains(QStringLiteral("下一步")));
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
    QVERIFY(!aitoolkit::testing::MainWindowTestPeer::controller(window)->currentSummary().inputPath.isEmpty());
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
    QVERIFY(!aitoolkit::testing::MainWindowTestPeer::controller(window)->currentSummary().inputPath.isEmpty());
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

void MainWindowTest::completedVideoSummaryKeepsResultFocusedContext() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString manifestPath =
        writeManifestFile(tempDir.path(), QStringLiteral("video_model"), QStringLiteral("Warehouse Detector"));
    QVERIFY(!manifestPath.isEmpty());

    aitoolkit::ui::MainWindow window;
    aitoolkit::testing::MainWindowTestPeer::controller(window)->loadModelManifest(manifestPath);

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("Warehouse Detector");
    summary.inputPath = QStringLiteral("video://frame-001");
    summary.taskType = QStringLiteral("detection");
    summary.detectionCount = 4;
    summary.elapsedMs = 18.5;

    aitoolkit::testing::MainWindowTestPeer::controller(window)->applyInferenceResult(summary);
    aitoolkit::testing::AppControllerTestPeer::clearCurrentImagePath(
        *aitoolkit::testing::MainWindowTestPeer::controller(window));
    emit aitoolkit::testing::MainWindowTestPeer::controller(window)->contextChanged();

    QVERIFY(aitoolkit::testing::MainWindowTestPeer::imageStatusLabel(window) != nullptr);
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::imageStatusLabel(window)->text().contains(QStringLiteral("视频")));

    QVERIFY(aitoolkit::testing::MainWindowTestPeer::nextStepLabel(window) != nullptr);
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::nextStepLabel(window)->text().contains(QStringLiteral("JSON")));
}

void MainWindowTest::classificationSummaryUsesTaskAwareContextCopy() {
    aitoolkit::ui::MainWindow window;

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("Classifier");
    summary.inputPath = QStringLiteral("D:/images/classify.jpg");
    summary.taskType = QStringLiteral("classification");
    summary.elapsedMs = 9.5;
    summary.classifications.append({7, QStringLiteral("tabby"), 0.92f});
    summary.classifications.append({3, QStringLiteral("tiger"), 0.06f});

    aitoolkit::testing::MainWindowTestPeer::controller(window)->applyInferenceResult(summary);

    auto* contextResultValue = aitoolkit::testing::MainWindowTestPeer::runStatusLabel(window);
    QVERIFY(contextResultValue != nullptr);
    QVERIFY(contextResultValue->text().contains(QStringLiteral("2 个类别")));
    QVERIFY(!contextResultValue->text().contains(QStringLiteral("目标")));
}

void MainWindowTest::selectingBatchResultUpdatesCurrentExportContext() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString firstImagePath = writeImageFile(tempDir.path(), QStringLiteral("first.bmp"));
    const QString secondImagePath = writeImageFile(tempDir.path(), QStringLiteral("second.bmp"));
    QVERIFY(!firstImagePath.isEmpty());
    QVERIFY(!secondImagePath.isEmpty());

    aitoolkit::ui::MainWindow window;

    const auto firstSummary = makeSummary(QStringLiteral("Warehouse Detector"), firstImagePath);
    const auto secondSummary = makeSummary(QStringLiteral("Warehouse Detector"), secondImagePath);

    aitoolkit::testing::MainWindowTestPeer::resultsPage(window)->setSummary(firstSummary);
    aitoolkit::testing::MainWindowTestPeer::resultsPage(window)->setResults({firstSummary, secondSummary});

    auto* resultsList = window.findChild<QListWidget*>(QStringLiteral("ResultsList"));
    QVERIFY(resultsList != nullptr);
    QCOMPARE(resultsList->currentRow(), 0);

    resultsList->setCurrentRow(1);

    QCOMPARE(aitoolkit::testing::MainWindowTestPeer::controller(window)->currentSummary().inputPath, secondImagePath);
    QCOMPARE(aitoolkit::testing::MainWindowTestPeer::controller(window)->currentImagePath(), secondImagePath);

    QVERIFY(aitoolkit::testing::MainWindowTestPeer::imageStatusLabel(window) != nullptr);
    QVERIFY(aitoolkit::testing::MainWindowTestPeer::imageStatusLabel(window)->text().contains(QStringLiteral("second.bmp")));
}

void MainWindowTest::exportFileNamesFollowSelectedResult() {
    aitoolkit::core::InferenceSummary imageSummary;
    imageSummary.inputPath = QStringLiteral("D:/images/second.png");
    QCOMPARE(
        aitoolkit::testing::MainWindowTestPeer::defaultJsonExportFileName(imageSummary),
        QStringLiteral("second.json"));
    QCOMPARE(
        aitoolkit::testing::MainWindowTestPeer::defaultImageExportFileName(imageSummary),
        QStringLiteral("second.png"));

    aitoolkit::core::InferenceSummary videoSummary;
    videoSummary.inputPath = QStringLiteral("D:/videos/demo.mp4 [frame 12]");
    QCOMPARE(
        aitoolkit::testing::MainWindowTestPeer::defaultJsonExportFileName(videoSummary),
        QStringLiteral("demo_mp4_frame_12.json"));
    QCOMPARE(
        aitoolkit::testing::MainWindowTestPeer::defaultImageExportFileName(videoSummary),
        QStringLiteral("demo_mp4_frame_12.png"));
}

void MainWindowTest::batchExportFileNameFollowsSourceContext() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString folderPath = QDir(tempDir.path()).filePath(QStringLiteral("street-scenes"));
    QVERIFY(QDir().mkpath(folderPath));

    QCOMPARE(
        aitoolkit::testing::MainWindowTestPeer::defaultBatchJsonExportFileName(folderPath),
        QStringLiteral("street-scenes_batch_results.json"));
    QCOMPARE(
        aitoolkit::testing::MainWindowTestPeer::defaultBatchJsonExportFileName(QStringLiteral("D:/videos/street.mp4")),
        QStringLiteral("street_batch_results.json"));
    QCOMPARE(
        aitoolkit::testing::MainWindowTestPeer::defaultBatchJsonExportFileName(QString()),
        QStringLiteral("batch_results.json"));
}

void MainWindowTest::recentInputClickReturnsToInferencePage() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    aitoolkit::ui::MainWindow window;

    const QString imagePath = writeImageFile(tempDir.path(), QStringLiteral("example.bmp"));
    QVERIFY(!imagePath.isEmpty());
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

void MainWindowTest::unreadableRecentInputClearsImageState() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    aitoolkit::ui::MainWindow window;

    const QString imagePath = writeImageFile(tempDir.path(), QStringLiteral("recent.bmp"));
    QVERIFY(!imagePath.isEmpty());

    auto* stack = window.findChild<QStackedWidget*>();
    QVERIFY(stack != nullptr);

    auto* inferencePage = stack->widget(2);
    QVERIFY(inferencePage != nullptr);
    QVERIFY(QMetaObject::invokeMethod(inferencePage, "imageSelected", Qt::DirectConnection, Q_ARG(QString, imagePath)));

    QVERIFY(QFile::remove(imagePath));

    stack->setCurrentIndex(4);

    QVERIFY(QMetaObject::invokeMethod(
        aitoolkit::testing::MainWindowTestPeer::settingsPage(window),
        "recentInputActivated",
        Qt::DirectConnection,
        Q_ARG(QString, imagePath)));

    QCOMPARE(stack->currentIndex(), 2);

    auto* imagePathLabel = window.findChild<QLabel*>(QStringLiteral("InferenceImagePathLabel"));
    QVERIFY(imagePathLabel != nullptr);
    QCOMPARE(imagePathLabel->text(), QStringLiteral("\u5f53\u524d\u672a\u9009\u62e9\u56fe\u50cf"));

    QVERIFY(aitoolkit::testing::MainWindowTestPeer::imageStatusLabel(window) != nullptr);
    QVERIFY(!aitoolkit::testing::MainWindowTestPeer::imageStatusLabel(window)->text().contains(QFileInfo(imagePath).fileName()));

    auto* runButton = aitoolkit::testing::MainWindowTestPeer::inferencePage(window)->findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
    QVERIFY(runButton != nullptr);
    QVERIFY(!runButton->isEnabled());
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
    QVERIFY(manifestPathLabel->text().contains(QDir::toNativeSeparators(manifestPath)));

    auto* manifestSummaryLabel = window.findChild<QLabel*>(QStringLiteral("ManifestSummaryLabel"));
    QVERIFY(manifestSummaryLabel != nullptr);
    QVERIFY(manifestSummaryLabel->text().contains(QStringLiteral("Warehouse Detector")));
}

void MainWindowTest::resolvesDownloadScriptFromInstalledShareDirectory() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QDir root(tempDir.path());
    QVERIFY(root.mkpath(QStringLiteral("bin")));
    QVERIFY(root.mkpath(QStringLiteral("share/scripts")));

    const QString scriptPath = root.filePath(QStringLiteral("share/scripts/download_sample_model.ps1"));
    QFile scriptFile(scriptPath);
    QVERIFY(scriptFile.open(QIODevice::WriteOnly | QIODevice::Text));
    scriptFile.write("Write-Host 'ok'");
    scriptFile.close();

    const QString resolvedPath =
        aitoolkit::testing::MainWindowTestPeer::resolveDownloadScriptPath(root.filePath(QStringLiteral("bin")));
    QCOMPARE(QDir::cleanPath(resolvedPath), QDir::cleanPath(scriptPath));
}

}  // namespace

QTEST_MAIN(MainWindowTest)

#include "test_main_window.moc"
