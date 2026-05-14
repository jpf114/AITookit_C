#include <QtTest>

#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QWidget>

#include "core/json_utils.h"
#include "ui/main_window.h"

namespace {

class MainWindowTest : public QObject {
    Q_OBJECT

private slots:
    void buildsThreePaneShell();
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

    const QString modelPath = QDir(tempDir.path()).filePath("model.onnx");
    QFile modelFile(modelPath);
    QVERIFY(modelFile.open(QIODevice::WriteOnly | QIODevice::Text));
    modelFile.write("dummy-model");
    modelFile.close();

    const QString manifestPath = QDir(tempDir.path()).filePath("model.json");
    aitoolkit::core::writeJsonObject(
        manifestPath,
        QJsonObject{
            {QStringLiteral("name"), QStringLiteral("Warehouse Detector")},
            {QStringLiteral("task_type"), QStringLiteral("detection")},
            {QStringLiteral("backend"), QStringLiteral("onnxruntime")},
            {QStringLiteral("model"), QStringLiteral("model.onnx")},
            {QStringLiteral("input_width"), 640},
            {QStringLiteral("input_height"), 640},
            {QStringLiteral("labels_inline"), QJsonArray{QStringLiteral("person"), QStringLiteral("box")}},
        });

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
