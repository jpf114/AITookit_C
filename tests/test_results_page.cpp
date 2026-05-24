#include <QtTest>

#include <QDir>
#include <QImage>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QTemporaryDir>

#define AITOOLKIT_TESTING 1
#include "test_peers.h"
#include "ui/pages/results_page.h"

namespace {

class ResultsPageTest : public QObject {
    Q_OBJECT

private slots:
    void populatesDetectionTable();
    void populatesSegmentationTable();
    void clearingSummaryShowsNoResultsState();
    void detectionHeadersResetAfterClassificationSummary();
    void detectionFilterKeepsDisplayFormatting();
    void classificationHidesCategoryFilter();
    void selectingBatchResultUpdatesPreviewImage();
    void exportButtonsReflectAvailableResultMedia();
    void batchResultsClarifyCurrentVsAllExportActions();
    void batchSummaryShowsCurrentPosition();
    void resultListLabelsDisambiguateFramesAndDuplicateNames();
    void summaryFallsBackToSourceWhenDimensionsMissing();
    void exportImageTooltipExplainsWhyItIsDisabled();
    void batchExportButtonOnlyAppearsForMultipleResults();
};

void ResultsPageTest::populatesDetectionTable() {
    aitoolkit::ui::ResultsPage page;

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("YOLO Demo");
    summary.inputPath = QStringLiteral("D:/images/demo.jpg");
    summary.detectionCount = 2;
    summary.elapsedMs = 18.25;

    aitoolkit::core::DetectionItem first;
    first.classId = 0;
    first.label = QStringLiteral("person");
    first.confidence = 0.95f;
    first.boundingBox = QRectF(10.0, 20.0, 100.0, 200.0);
    summary.detections.push_back(first);

    aitoolkit::core::DetectionItem second;
    second.classId = 1;
    second.label = QStringLiteral("dog");
    second.confidence = 0.81f;
    second.boundingBox = QRectF(40.0, 50.0, 60.0, 70.0);
    summary.detections.push_back(second);

    page.setImage(QImage(320, 240, QImage::Format_RGB32));
    page.setSummary(summary);

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(summaryLabel->text().contains(QStringLiteral("YOLO Demo")));
    QVERIFY(summaryLabel->text().contains(QStringLiteral("2")));

    auto* table = page.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(table != nullptr);
    QCOMPARE(table->rowCount(), 2);
    QCOMPARE(table->item(0, 1)->text(), QStringLiteral("person"));
    QCOMPARE(table->item(1, 1)->text(), QStringLiteral("dog"));
}

void ResultsPageTest::populatesSegmentationTable() {
    aitoolkit::ui::ResultsPage page;

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("YOLO Seg Demo");
    summary.inputPath = QStringLiteral("D:/images/seg.jpg");
    summary.taskType = QStringLiteral("segmentation");
    summary.elapsedMs = 24.5;

    aitoolkit::core::SegmentationItem first;
    first.classId = 0;
    first.label = QStringLiteral("person");
    first.confidence = 0.93f;
    first.boundingBox = QRectF(15.0, 30.0, 80.0, 140.0);
    summary.segmentations.push_back(first);

    aitoolkit::core::SegmentationItem second;
    second.classId = 1;
    second.label = QStringLiteral("bicycle");
    second.confidence = 0.77f;
    second.boundingBox = QRectF(55.0, 75.0, 90.0, 60.0);
    summary.segmentations.push_back(second);

    page.setImage(QImage(320, 240, QImage::Format_RGB32));
    page.setSummary(summary);

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(summaryLabel->text().contains(QStringLiteral("实例数：2")));

    auto* table = page.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(table != nullptr);
    QCOMPARE(table->rowCount(), 2);
    QCOMPARE(table->item(0, 1)->text(), QStringLiteral("person"));
    QCOMPARE(table->item(1, 1)->text(), QStringLiteral("bicycle"));
}

void ResultsPageTest::clearingSummaryShowsNoResultsState() {
    aitoolkit::ui::ResultsPage page;

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("YOLO Demo");
    summary.inputPath = QStringLiteral("D:/images/demo.jpg");
    summary.detectionCount = 1;
    summary.elapsedMs = 18.25;

    aitoolkit::core::DetectionItem detection;
    detection.classId = 0;
    detection.label = QStringLiteral("person");
    detection.confidence = 0.95f;
    detection.boundingBox = QRectF(10.0, 20.0, 100.0, 200.0);
    summary.detections.push_back(detection);

    page.setSummary(summary);
    page.setSummary({});

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QCOMPARE(summaryLabel->text(), QStringLiteral("当前还没有推理结果"));

    auto* table = page.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(table != nullptr);
    QCOMPARE(table->rowCount(), 0);
}

void ResultsPageTest::detectionHeadersResetAfterClassificationSummary() {
    aitoolkit::ui::ResultsPage page;

    aitoolkit::core::InferenceSummary classificationSummary;
    classificationSummary.modelName = QStringLiteral("Classifier");
    classificationSummary.inputPath = QStringLiteral("D:/images/classify.jpg");
    classificationSummary.taskType = QStringLiteral("classification");
    classificationSummary.elapsedMs = 9.5;

    aitoolkit::core::ClassificationItem top1;
    top1.classId = 7;
    top1.label = QStringLiteral("tabby");
    top1.confidence = 0.92f;
    classificationSummary.classifications.push_back(top1);

    page.setSummary(classificationSummary);

    aitoolkit::core::InferenceSummary detectionSummary;
    detectionSummary.modelName = QStringLiteral("Detector");
    detectionSummary.inputPath = QStringLiteral("D:/images/detect.jpg");
    detectionSummary.taskType = QStringLiteral("detection");
    detectionSummary.detectionCount = 1;
    detectionSummary.elapsedMs = 14.0;

    aitoolkit::core::DetectionItem detection;
    detection.classId = 3;
    detection.label = QStringLiteral("car");
    detection.confidence = 0.88f;
    detection.boundingBox = QRectF(12.0, 24.0, 120.0, 60.0);
    detectionSummary.detections.push_back(detection);

    page.setSummary(detectionSummary);

    auto* table = page.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(table != nullptr);
    QCOMPARE(table->horizontalHeaderItem(0)->text(), QStringLiteral("类别"));
    QCOMPARE(table->horizontalHeaderItem(1)->text(), QStringLiteral("标签"));
    QCOMPARE(table->horizontalHeaderItem(2)->text(), QStringLiteral("置信度"));
    QCOMPARE(table->horizontalHeaderItem(3)->text(), QStringLiteral("框选范围"));
    QCOMPARE(table->item(0, 1)->text(), QStringLiteral("car"));
}

void ResultsPageTest::detectionFilterKeepsDisplayFormatting() {
    aitoolkit::ui::ResultsPage page;

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("Detector");
    summary.inputPath = QStringLiteral("D:/images/detect.jpg");
    summary.taskType = QStringLiteral("detection");
    summary.detectionCount = 2;
    summary.elapsedMs = 14.0;

    aitoolkit::core::DetectionItem first;
    first.classId = 3;
    first.label = QStringLiteral("car");
    first.confidence = 0.88f;
    first.boundingBox = QRectF(12.0, 24.0, 120.0, 60.0);
    summary.detections.push_back(first);

    aitoolkit::core::DetectionItem second;
    second.classId = 4;
    second.label = QStringLiteral("bus");
    second.confidence = 0.73f;
    second.boundingBox = QRectF(30.0, 45.0, 150.0, 70.0);
    summary.detections.push_back(second);

    page.setSummary(summary);

    auto* table = page.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(table != nullptr);
    QCOMPARE(table->item(0, 2)->text(), QStringLiteral("0.880"));
    QCOMPARE(table->item(0, 3)->text(), QStringLiteral("12.0, 24.0, 120.0 x 60.0"));

    auto* filter = page.findChild<QComboBox*>();
    QVERIFY(filter != nullptr);
    const int carIndex = filter->findText(QStringLiteral("car"));
    QVERIFY(carIndex >= 0);
    filter->setCurrentIndex(carIndex);

    QCOMPARE(table->rowCount(), 1);
    QCOMPARE(table->item(0, 1)->text(), QStringLiteral("car"));
    QCOMPARE(table->item(0, 2)->text(), QStringLiteral("0.880"));
    QCOMPARE(table->item(0, 3)->text(), QStringLiteral("12.0, 24.0, 120.0 x 60.0"));
}

void ResultsPageTest::classificationHidesCategoryFilter() {
    aitoolkit::ui::ResultsPage page;

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("Classifier");
    summary.inputPath = QStringLiteral("D:/images/classify.jpg");
    summary.taskType = QStringLiteral("classification");
    summary.elapsedMs = 9.5;

    aitoolkit::core::ClassificationItem top1;
    top1.classId = 7;
    top1.label = QStringLiteral("tabby");
    top1.confidence = 0.92f;
    summary.classifications.push_back(top1);

    page.setSummary(summary);

    auto* filter = page.findChild<QComboBox*>();
    QVERIFY(filter != nullptr);
    QVERIFY(filter->isHidden());

    aitoolkit::core::InferenceSummary detectionSummary;
    detectionSummary.modelName = QStringLiteral("Detector");
    detectionSummary.inputPath = QStringLiteral("D:/images/detect.jpg");
    detectionSummary.taskType = QStringLiteral("detection");
    detectionSummary.detectionCount = 1;
    detectionSummary.elapsedMs = 14.0;

    aitoolkit::core::DetectionItem detection;
    detection.classId = 3;
    detection.label = QStringLiteral("car");
    detection.confidence = 0.88f;
    detection.boundingBox = QRectF(12.0, 24.0, 120.0, 60.0);
    detectionSummary.detections.push_back(detection);

    page.setSummary(detectionSummary);

    QVERIFY(!filter->isHidden());
}

void ResultsPageTest::selectingBatchResultUpdatesPreviewImage() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = QDir(tempDir.path()).filePath(QStringLiteral("first.bmp"));
    QImage firstImage(24, 24, QImage::Format_RGB32);
    firstImage.fill(QColor(QStringLiteral("#ef4444")));
    QVERIFY(firstImage.save(firstPath, "BMP"));

    const QString secondPath = QDir(tempDir.path()).filePath(QStringLiteral("second.bmp"));
    QImage secondImage(24, 24, QImage::Format_RGB32);
    secondImage.fill(QColor(QStringLiteral("#22c55e")));
    QVERIFY(secondImage.save(secondPath, "BMP"));

    aitoolkit::core::InferenceSummary firstSummary;
    firstSummary.modelName = QStringLiteral("Detector");
    firstSummary.inputPath = firstPath;
    firstSummary.taskType = QStringLiteral("detection");
    firstSummary.detectionCount = 1;
    firstSummary.elapsedMs = 10.0;
    firstSummary.detections.push_back({0, QStringLiteral("red"), 0.9f, QRectF(1.0, 1.0, 8.0, 8.0)});

    aitoolkit::core::InferenceSummary secondSummary;
    secondSummary.modelName = QStringLiteral("Detector");
    secondSummary.inputPath = secondPath;
    secondSummary.taskType = QStringLiteral("detection");
    secondSummary.detectionCount = 1;
    secondSummary.elapsedMs = 11.0;
    secondSummary.detections.push_back({1, QStringLiteral("green"), 0.8f, QRectF(2.0, 2.0, 8.0, 8.0)});

    aitoolkit::ui::ResultsPage page;
    page.setSummary(firstSummary);
    page.setResults({firstSummary, secondSummary});

    auto* previewWidget = aitoolkit::testing::ResultsPageTestPeer::previewWidget(page);
    QVERIFY(!aitoolkit::testing::ImagePreviewWidgetTestPeer::image(*previewWidget).isNull());
    QCOMPARE(
        aitoolkit::testing::ImagePreviewWidgetTestPeer::image(*previewWidget).pixelColor(0, 0),
        QColor(QStringLiteral("#ef4444")));

    aitoolkit::testing::ResultsPageTestPeer::resultsList(page)->setCurrentRow(1);

    QVERIFY(!aitoolkit::testing::ImagePreviewWidgetTestPeer::image(*previewWidget).isNull());
    QCOMPARE(
        aitoolkit::testing::ImagePreviewWidgetTestPeer::image(*previewWidget).pixelColor(0, 0),
        QColor(QStringLiteral("#22c55e")));
}

void ResultsPageTest::exportButtonsReflectAvailableResultMedia() {
    aitoolkit::ui::ResultsPage page;

    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportButton(page) != nullptr);
    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page) != nullptr);
    QVERIFY(!aitoolkit::testing::ResultsPageTestPeer::exportButton(page)->isEnabled());
    QVERIFY(!aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page)->isEnabled());

    aitoolkit::core::InferenceSummary videoSummary;
    videoSummary.modelName = QStringLiteral("Detector");
    videoSummary.inputPath = QStringLiteral("D:/videos/demo.mp4 [frame 0]");
    videoSummary.taskType = QStringLiteral("detection");
    videoSummary.detectionCount = 1;
    videoSummary.elapsedMs = 12.0;
    videoSummary.detections.push_back({0, QStringLiteral("car"), 0.9f, QRectF(1.0, 1.0, 8.0, 8.0)});
    page.setSummary(videoSummary);

    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportButton(page)->isEnabled());
    QVERIFY(!aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page)->isEnabled());

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString imagePath = QDir(tempDir.path()).filePath(QStringLiteral("frame.bmp"));
    QImage image(24, 24, QImage::Format_RGB32);
    image.fill(Qt::blue);
    QVERIFY(image.save(imagePath, "BMP"));

    aitoolkit::core::InferenceSummary imageSummary = videoSummary;
    imageSummary.inputPath = imagePath;
    page.setSummary(imageSummary);

    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportButton(page)->isEnabled());
    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page)->isEnabled());
}

void ResultsPageTest::batchResultsClarifyCurrentVsAllExportActions() {
    aitoolkit::core::InferenceSummary firstSummary;
    firstSummary.modelName = QStringLiteral("Detector");
    firstSummary.inputPath = QStringLiteral("D:/images/first.png");
    firstSummary.taskType = QStringLiteral("detection");
    firstSummary.detectionCount = 1;
    firstSummary.elapsedMs = 10.0;
    firstSummary.detections.push_back({0, QStringLiteral("red"), 0.9f, QRectF(1.0, 1.0, 8.0, 8.0)});

    aitoolkit::core::InferenceSummary secondSummary = firstSummary;
    secondSummary.inputPath = QStringLiteral("D:/images/second.png");

    aitoolkit::ui::ResultsPage page;
    page.setSummary(firstSummary);
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::exportButton(page)->text(), QStringLiteral("导出 JSON"));
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page)->text(), QStringLiteral("导出图片"));

    page.setResults({firstSummary, secondSummary});

    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::exportButton(page)->text(), QStringLiteral("导出当前 JSON"));
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page)->text(), QStringLiteral("导出当前图片"));
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->text(), QStringLiteral("导出全部 JSON"));
    QVERIFY(!aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->isHidden());
}

void ResultsPageTest::batchSummaryShowsCurrentPosition() {
    aitoolkit::core::InferenceSummary firstSummary;
    firstSummary.modelName = QStringLiteral("Detector");
    firstSummary.inputPath = QStringLiteral("D:/images/first.png");
    firstSummary.taskType = QStringLiteral("detection");
    firstSummary.imageWidth = 640;
    firstSummary.imageHeight = 480;
    firstSummary.detectionCount = 1;
    firstSummary.elapsedMs = 10.0;
    firstSummary.detections.push_back({0, QStringLiteral("red"), 0.9f, QRectF(1.0, 1.0, 8.0, 8.0)});

    aitoolkit::core::InferenceSummary secondSummary = firstSummary;
    secondSummary.inputPath = QStringLiteral("D:/images/second.png");
    secondSummary.elapsedMs = 11.0;

    aitoolkit::ui::ResultsPage page;
    page.setSummary(firstSummary);

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(!summaryLabel->text().contains(QStringLiteral("第 1 / 2 项")));

    page.setResults({firstSummary, secondSummary});
    QVERIFY(summaryLabel->text().contains(QStringLiteral("第 1 / 2 项")));

    aitoolkit::testing::ResultsPageTestPeer::resultsList(page)->setCurrentRow(1);
    QVERIFY(summaryLabel->text().contains(QStringLiteral("第 2 / 2 项")));
}

void ResultsPageTest::resultListLabelsDisambiguateFramesAndDuplicateNames() {
    aitoolkit::core::InferenceSummary firstFrame;
    firstFrame.modelName = QStringLiteral("Detector");
    firstFrame.inputPath = QStringLiteral("D:/videos/demo.mp4 [frame 0]");
    firstFrame.taskType = QStringLiteral("detection");
    firstFrame.detectionCount = 1;
    firstFrame.elapsedMs = 10.0;

    aitoolkit::core::InferenceSummary secondFrame = firstFrame;
    secondFrame.inputPath = QStringLiteral("D:/videos/demo.mp4 [frame 12]");

    aitoolkit::core::InferenceSummary duplicateA = firstFrame;
    duplicateA.inputPath = QStringLiteral("D:/captures/cam-a/frame.png");

    aitoolkit::core::InferenceSummary duplicateB = firstFrame;
    duplicateB.inputPath = QStringLiteral("D:/captures/cam-b/frame.png");

    aitoolkit::ui::ResultsPage page;
    page.setResults({firstFrame, secondFrame, duplicateA, duplicateB});

    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::resultsList(page) != nullptr);
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::resultsList(page)->item(0)->text(), QStringLiteral("demo.mp4 | frame 0"));
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::resultsList(page)->item(1)->text(), QStringLiteral("demo.mp4 | frame 12"));
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::resultsList(page)->item(2)->text(), QStringLiteral("cam-a/frame.png"));
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::resultsList(page)->item(3)->text(), QStringLiteral("cam-b/frame.png"));
}

void ResultsPageTest::summaryFallsBackToSourceWhenDimensionsMissing() {
    aitoolkit::ui::ResultsPage page;

    aitoolkit::core::InferenceSummary videoSummary;
    videoSummary.modelName = QStringLiteral("Detector");
    videoSummary.inputPath = QStringLiteral("D:/videos/demo.mp4 [frame 12]");
    videoSummary.taskType = QStringLiteral("detection");
    videoSummary.detectionCount = 1;
    videoSummary.elapsedMs = 12.0;

    page.setSummary(videoSummary);

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(summaryLabel->text().contains(QStringLiteral("来源：demo.mp4 | frame 12")));
    QVERIFY(!summaryLabel->text().contains(QStringLiteral("图像：0×0")));

    aitoolkit::core::InferenceSummary imageSummary = videoSummary;
    imageSummary.inputPath = QStringLiteral("D:/images/example.png");
    page.setSummary(imageSummary);

    QVERIFY(summaryLabel->text().contains(QStringLiteral("来源：example.png")));
    QVERIFY(!summaryLabel->text().contains(QStringLiteral("图像：0×0")));
}

void ResultsPageTest::exportImageTooltipExplainsWhyItIsDisabled() {
    aitoolkit::ui::ResultsPage page;

    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page) != nullptr);
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page)->toolTip(), QStringLiteral("请先完成一次推理，再导出图片。"));

    aitoolkit::core::InferenceSummary videoSummary;
    videoSummary.modelName = QStringLiteral("Detector");
    videoSummary.inputPath = QStringLiteral("D:/videos/demo.mp4 [frame 0]");
    videoSummary.taskType = QStringLiteral("detection");
    videoSummary.detectionCount = 1;
    videoSummary.elapsedMs = 12.0;
    page.setSummary(videoSummary);

    QCOMPARE(
        aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page)->toolTip(),
        QStringLiteral("当前结果没有可导出的预览图，请先选择可读取的图像结果。"));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString imagePath = QDir(tempDir.path()).filePath(QStringLiteral("frame.bmp"));
    QImage image(24, 24, QImage::Format_RGB32);
    image.fill(Qt::blue);
    QVERIFY(image.save(imagePath, "BMP"));

    aitoolkit::core::InferenceSummary imageSummary = videoSummary;
    imageSummary.inputPath = imagePath;
    page.setSummary(imageSummary);

    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::exportImageButton(page)->toolTip(), QStringLiteral("导出当前结果的渲染图片"));
}

void ResultsPageTest::batchExportButtonOnlyAppearsForMultipleResults() {
    aitoolkit::core::InferenceSummary firstSummary;
    firstSummary.modelName = QStringLiteral("Detector");
    firstSummary.inputPath = QStringLiteral("D:/images/first.png");
    firstSummary.taskType = QStringLiteral("detection");
    firstSummary.detectionCount = 1;
    firstSummary.elapsedMs = 10.0;

    aitoolkit::core::InferenceSummary secondSummary = firstSummary;
    secondSummary.inputPath = QStringLiteral("D:/images/second.png");

    aitoolkit::ui::ResultsPage page;
    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page) != nullptr);
    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->isHidden());
    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->toolTip().isEmpty());

    page.setSummary(firstSummary);
    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->isHidden());
    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->toolTip().isEmpty());

    page.setResults({firstSummary, secondSummary});
    QVERIFY(!aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->isHidden());
    QCOMPARE(aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->toolTip(), QStringLiteral("一次导出当前批量结果的全部 JSON"));

    page.setResults({firstSummary});
    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->isHidden());
    QVERIFY(aitoolkit::testing::ResultsPageTestPeer::exportBatchButton(page)->toolTip().isEmpty());
}

}  // namespace

QTEST_MAIN(ResultsPageTest)

#include "test_results_page.moc"
