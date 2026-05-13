#include <QtTest>

#include <QImage>
#include <QLabel>
#include <QTableWidget>

#include "ui/pages/results_page.h"

namespace {

class ResultsPageTest : public QObject {
    Q_OBJECT

private slots:
    void populatesDetectionTable();
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

}  // namespace

QTEST_MAIN(ResultsPageTest)

#include "test_results_page.moc"
