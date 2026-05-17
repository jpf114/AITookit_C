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
#define private public
#include "ui/main_window.h"
#include "ui/app_controller.h"
#undef private
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
    QImage image(64, 64, QImage::Format_ARGB32);
    image.fill(Qt::red);
    return image.save(imagePath) ? imagePath : QString();
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
    window.controller_->applyInferenceResult(summary);
    window.resultsPage_->setSummary(summary);
    window.runStatusLabel_->setText(
        QStringLiteral("\u5df2\u5b8c\u6210\uff0c\u5171 %1 \u4e2a\u76ee\u6807\uff0c\u8017\u65f6 %2 ms")
            .arg(summary.detectionCount)
            .arg(QString::number(summary.elapsedMs, 'f', 2)));
    window.nextStepLabel_->setText(QStringLiteral("\u53ef\u67e5\u770b\u7ed3\u679c\u660e\u7ec6\uff0c\u6216\u76f4\u63a5\u5bfc\u51fa JSON\u3002"));
}

void verifyHasResultsState(aitoolkit::ui::MainWindow& window) {
    auto* contextResultValue = window.findChild<QLabel*>(QStringLiteral("ContextResultValue"));
    QVERIFY(contextResultValue != nullptr);
    QVERIFY(contextResultValue->text().contains(QStringLiteral("12.50")));

    auto* contextNextStepValue = window.findChild<QLabel*>(QStringLiteral("ContextNextStepValue"));
    QVERIFY(contextNextStepValue != nullptr);
    QVERIFY(contextNextStepValue->text().contains(QStringLiteral("JSON")));
}

void verifyClearedResultsState(aitoolkit::ui::MainWindow& window) {
    QCOMPARE(window.controller_->currentSummary().inputPath, QString());
    QCOMPARE(window.controller_->currentSummary().detections.size(), 0);

    auto* resultsSummaryLabel = window.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(resultsSummaryLabel != nullptr);
    QVERIFY(!resultsSummaryLabel->text().contains(QStringLiteral("Warehouse Detector")));

    auto* detectionsTable = window.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(detectionsTable != nullptr);
    QCOMPARE(detectionsTable->rowCount(), 0);

    auto* contextResultValue = window.findChild<QLabel*>(QStringLiteral("ContextResultValue"));
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
    QVERIFY(!window.controller_->currentSummary().inputPath.isEmpty());
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
    QVERIFY(!window.controller_->currentSummary().inputPath.isEmpty());
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
    window.controller_->loadModelManifest(manifestPath);

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("Warehouse Detector");
    summary.inputPath = QStringLiteral("video://frame-001");
    summary.taskType = QStringLiteral("detection");
    summary.detectionCount = 4;
    summary.elapsedMs = 18.5;

    window.controller_->applyInferenceResult(summary);
    window.controller_->currentImagePath_.clear();
    emit window.controller_->contextChanged();

    auto* contextImageValue = window.findChild<QLabel*>(QStringLiteral("ContextImageValue"));
    QVERIFY(contextImageValue != nullptr);
    QVERIFY(contextImageValue->text().contains(QStringLiteral("视频")));

    auto* contextNextStepValue = window.findChild<QLabel*>(QStringLiteral("ContextNextStepValue"));
    QVERIFY(contextNextStepValue != nullptr);
    QVERIFY(contextNextStepValue->text().contains(QStringLiteral("JSON")));
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

    window.controller_->applyInferenceResult(summary);

    auto* contextResultValue = window.findChild<QLabel*>(QStringLiteral("ContextResultValue"));
    QVERIFY(contextResultValue != nullptr);
    QVERIFY(contextResultValue->text().contains(QStringLiteral("2 个类别")));
    QVERIFY(!contextResultValue->text().contains(QStringLiteral("目标")));
}

void MainWindowTest::selectingBatchResultUpdatesCurrentExportContext() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString firstImagePath = writeImageFile(tempDir.path(), QStringLiteral("first.png"));
    const QString secondImagePath = writeImageFile(tempDir.path(), QStringLiteral("second.png"));
    QVERIFY(!firstImagePath.isEmpty());
    QVERIFY(!secondImagePath.isEmpty());

    aitoolkit::ui::MainWindow window;

    const auto firstSummary = makeSummary(QStringLiteral("Warehouse Detector"), firstImagePath);
    const auto secondSummary = makeSummary(QStringLiteral("Warehouse Detector"), secondImagePath);

    window.resultsPage_->setSummary(firstSummary);
    window.resultsPage_->setResults({firstSummary, secondSummary});

    auto* resultsList = window.findChild<QListWidget*>(QStringLiteral("ResultsList"));
    QVERIFY(resultsList != nullptr);
    QCOMPARE(resultsList->currentRow(), 0);

    resultsList->setCurrentRow(1);

    QCOMPARE(window.controller_->currentSummary().inputPath, secondImagePath);
    QCOMPARE(window.controller_->currentImagePath(), secondImagePath);

    auto* contextImageValue = window.findChild<QLabel*>(QStringLiteral("ContextImageValue"));
    QVERIFY(contextImageValue != nullptr);
    QVERIFY(contextImageValue->text().contains(QStringLiteral("second.png")));
}

void MainWindowTest::exportFileNamesFollowSelectedResult() {
    aitoolkit::core::InferenceSummary imageSummary;
    imageSummary.inputPath = QStringLiteral("D:/images/second.png");
    QCOMPARE(
        aitoolkit::ui::MainWindow::defaultJsonExportFileName(imageSummary),
        QStringLiteral("second.json"));
    QCOMPARE(
        aitoolkit::ui::MainWindow::defaultImageExportFileName(imageSummary),
        QStringLiteral("second.png"));

    aitoolkit::core::InferenceSummary videoSummary;
    videoSummary.inputPath = QStringLiteral("D:/videos/demo.mp4 [frame 12]");
    QCOMPARE(
        aitoolkit::ui::MainWindow::defaultJsonExportFileName(videoSummary),
        QStringLiteral("demo_mp4_frame_12.json"));
    QCOMPARE(
        aitoolkit::ui::MainWindow::defaultImageExportFileName(videoSummary),
        QStringLiteral("demo_mp4_frame_12.png"));
}

void MainWindowTest::batchExportFileNameFollowsSourceContext() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    const QString folderPath = QDir(tempDir.path()).filePath(QStringLiteral("street-scenes"));
    QVERIFY(QDir().mkpath(folderPath));

    QCOMPARE(
        aitoolkit::ui::MainWindow::defaultBatchJsonExportFileName(folderPath),
        QStringLiteral("street-scenes_batch_results.json"));
    QCOMPARE(
        aitoolkit::ui::MainWindow::defaultBatchJsonExportFileName(QStringLiteral("D:/videos/street.mp4")),
        QStringLiteral("street_batch_results.json"));
    QCOMPARE(
        aitoolkit::ui::MainWindow::defaultBatchJsonExportFileName(QString()),
        QStringLiteral("batch_results.json"));
}

void MainWindowTest::recentInputClickReturnsToInferencePage() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");

    aitoolkit::ui::MainWindow window;

    const QString imagePath = writeImageFile(tempDir.path(), QStringLiteral("example.png"));
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

    const QString imagePath = writeImageFile(tempDir.path(), QStringLiteral("recent.png"));
    QVERIFY(!imagePath.isEmpty());

    auto* stack = window.findChild<QStackedWidget*>();
    QVERIFY(stack != nullptr);

    auto* inferencePage = stack->widget(2);
    QVERIFY(inferencePage != nullptr);
    QVERIFY(QMetaObject::invokeMethod(inferencePage, "imageSelected", Qt::DirectConnection, Q_ARG(QString, imagePath)));

    QVERIFY(QFile::remove(imagePath));

    stack->setCurrentIndex(4);

    QVERIFY(QMetaObject::invokeMethod(
        window.settingsPage_,
        "recentInputActivated",
        Qt::DirectConnection,
        Q_ARG(QString, imagePath)));

    QCOMPARE(stack->currentIndex(), 2);

    auto* imagePathLabel = window.findChild<QLabel*>(QStringLiteral("InferenceImagePathLabel"));
    QVERIFY(imagePathLabel != nullptr);
    QCOMPARE(imagePathLabel->text(), QStringLiteral("\u5f53\u524d\u672a\u9009\u62e9\u56fe\u50cf"));

    auto* contextImageValue = window.findChild<QLabel*>(QStringLiteral("ContextImageValue"));
    QVERIFY(contextImageValue != nullptr);
    QVERIFY(!contextImageValue->text().contains(QFileInfo(imagePath).fileName()));

    auto* runButton = window.inferencePage_->findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
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
    QVERIFY(manifestPathLabel->text().contains(QDir::cleanPath(manifestPath)));

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
        aitoolkit::ui::MainWindow::resolveDownloadScriptPath(root.filePath(QStringLiteral("bin")));
    QCOMPARE(QDir::cleanPath(resolvedPath), QDir::cleanPath(scriptPath));
}

}  // namespace

QTEST_MAIN(MainWindowTest)

#include "test_main_window.moc"
