#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QTextEdit>

#include "core/model_manifest.h"
#include "ui/dialogs/onnx_setup_dialog.h"
#include "ui/pages/home_page.h"
#include "ui/pages/inference_page.h"
#include "ui/pages/models_page.h"
#include "ui/pages/results_page.h"
#include "ui/pages/settings_page.h"

class GuiValidationTest : public QObject {
    Q_OBJECT

private slots:
    void testOnnxSetupDialogDefaults();
    void testOnnxSetupDialogNameRequired();
    void testOnnxSetupDialogStepSize32();
    void testOnnxSetupDialogLabelsDedup();
    void testOnnxSetupDialogLabelsTrim();
    void testOnnxSetupDialogConfRange();
    void testOnnxSetupDialogNmsRange();
    void testOnnxSetupDialogInputSizeRange();
    void testOnnxSetupDialogEmptyLabels();
    void testOnnxSetupDialogWhitespaceOnlyLabels();
    void testOnnxSetupDialogShowsDetectionOnlyNotice();

    void testInferencePageEmptyState();
    void testInferencePageRunDisabledWithoutImage();
    void testInferencePageProgressControls();
    void testInferencePageRunEnabledWhenBothReady();
    void testInferencePageRunDisabledWithoutModel();
    void testInferencePageCancelButtonVisibility();
    void testInferencePageMaxFramesDefaults();
    void testInferencePageProgressIndeterminate();
    void testInferencePageEmptyImagePathResets();
    void testInferencePageImageTooSmall();
    void testInferencePageImageTooLarge();
    void testInferencePageRunningDisablesRunButton();
    void testInferencePageConfidenceThresholdDefaults();
    void testInferencePageNmsThresholdDefaults();
    void testInferencePageSetDefaultThresholds();

    void testResultsPageExportButtonsExist();
    void testResultsPageDefaultSummary();
    void testResultsPageEmptySummaryReset();
    void testResultsPageTableHeaders();

    void testHomePageDefaultState();
    void testHomePageSetRecentModels();
    void testHomePageSetRecentInputs();
    void testHomePageEmptyRecentLists();

    void testModelsPageDefaultState();
    void testModelsPageEmptyManifestPath();
    void testModelsPageSetManifestSummary();

    void testSettingsPageDefaultExportPlaceholder();
    void testSettingsPageSetExportDirectory();
    void testSettingsPageSetRecentModels();
    void testSettingsPageSetRecentInputs();

private:
    QString createTestImage(int width, int height);
};

void GuiValidationTest::testOnnxSetupDialogDefaults() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test_model.onnx"));
    QCOMPARE(dialog.modelName(), QStringLiteral("test_model"));
    QCOMPARE(dialog.inputWidth(), 640);
    QCOMPARE(dialog.inputHeight(), 640);
    QCOMPARE(dialog.confidenceThreshold(), 0.25);
    QCOMPARE(dialog.nmsThreshold(), 0.45);
    QVERIFY(dialog.labels().isEmpty());
}

void GuiValidationTest::testOnnxSetupDialogNameRequired() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));

    auto* nameEdit = dialog.findChild<QLineEdit*>();
    QVERIFY(nameEdit != nullptr);

    auto* buttonBox = dialog.findChild<QDialogButtonBox*>();
    QVERIFY(buttonBox != nullptr);
    auto* okButton = buttonBox->button(QDialogButtonBox::Ok);
    QVERIFY(okButton != nullptr);

    nameEdit->setText(QStringLiteral("  "));
    QVERIFY(!okButton->isEnabled());

    nameEdit->setText(QStringLiteral("valid_name"));
    QVERIFY(okButton->isEnabled());
}

void GuiValidationTest::testOnnxSetupDialogStepSize32() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));

    auto spinBoxes = dialog.findChildren<QSpinBox*>();
    QVERIFY(spinBoxes.size() >= 2);

    for (auto* spin : spinBoxes) {
        if (spin->maximum() > 100) {
            QCOMPARE(spin->singleStep(), 32);
        }
    }
}

void GuiValidationTest::testOnnxSetupDialogLabelsDedup() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));
    auto* labelsEdit = dialog.findChild<QTextEdit*>();
    QVERIFY(labelsEdit != nullptr);

    labelsEdit->setPlainText(QStringLiteral("cat\ndog\ncat\nbird"));
    const QStringList labels = dialog.labels();
    QCOMPARE(labels.size(), 3);
    QCOMPARE(labels[0], QStringLiteral("cat"));
    QCOMPARE(labels[1], QStringLiteral("dog"));
    QCOMPARE(labels[2], QStringLiteral("bird"));
}

void GuiValidationTest::testOnnxSetupDialogLabelsTrim() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));
    auto* labelsEdit = dialog.findChild<QTextEdit*>();
    QVERIFY(labelsEdit != nullptr);

    labelsEdit->setPlainText(QStringLiteral("  cat  \n\n  dog  \n  "));
    const QStringList labels = dialog.labels();
    QCOMPARE(labels.size(), 2);
    QCOMPARE(labels[0], QStringLiteral("cat"));
    QCOMPARE(labels[1], QStringLiteral("dog"));
}

void GuiValidationTest::testOnnxSetupDialogConfRange() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));
    auto spinBoxes = dialog.findChildren<QDoubleSpinBox*>();

    QDoubleSpinBox* confSpin = nullptr;
    for (auto* spin : spinBoxes) {
        if (qFuzzyCompare(spin->value(), 0.25)) {
            confSpin = spin;
            break;
        }
    }
    QVERIFY(confSpin != nullptr);
    QCOMPARE(confSpin->minimum(), 0.0);
    QCOMPARE(confSpin->maximum(), 1.0);
    QCOMPARE(confSpin->singleStep(), 0.05);
    QCOMPARE(confSpin->decimals(), 2);
}

void GuiValidationTest::testOnnxSetupDialogNmsRange() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));
    auto spinBoxes = dialog.findChildren<QDoubleSpinBox*>();

    QDoubleSpinBox* nmsSpin = nullptr;
    for (auto* spin : spinBoxes) {
        if (qFuzzyCompare(spin->value(), 0.45)) {
            nmsSpin = spin;
            break;
        }
    }
    QVERIFY(nmsSpin != nullptr);
    QCOMPARE(nmsSpin->minimum(), 0.0);
    QCOMPARE(nmsSpin->maximum(), 1.0);
    QCOMPARE(nmsSpin->singleStep(), 0.05);
    QCOMPARE(nmsSpin->decimals(), 2);
}

void GuiValidationTest::testOnnxSetupDialogInputSizeRange() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));
    auto spinBoxes = dialog.findChildren<QSpinBox*>();

    for (auto* spin : spinBoxes) {
        if (spin->maximum() > 100) {
            QCOMPARE(spin->minimum(), 32);
            QCOMPARE(spin->maximum(), 4096);
        }
    }
}

void GuiValidationTest::testOnnxSetupDialogEmptyLabels() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));
    auto* labelsEdit = dialog.findChild<QTextEdit*>();
    QVERIFY(labelsEdit != nullptr);

    labelsEdit->setPlainText(QString());
    QVERIFY(dialog.labels().isEmpty());

    labelsEdit->setPlainText(QStringLiteral("   "));
    QVERIFY(dialog.labels().isEmpty());
}

void GuiValidationTest::testOnnxSetupDialogWhitespaceOnlyLabels() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));
    auto* labelsEdit = dialog.findChild<QTextEdit*>();
    QVERIFY(labelsEdit != nullptr);

    labelsEdit->setPlainText(QStringLiteral("   \n\n   \n  "));
    QVERIFY(dialog.labels().isEmpty());

    labelsEdit->setPlainText(QStringLiteral("cat\n   \ndog"));
    const QStringList labels = dialog.labels();
    QCOMPARE(labels.size(), 2);
    QCOMPARE(labels[0], QStringLiteral("cat"));
    QCOMPARE(labels[1], QStringLiteral("dog"));
}

void GuiValidationTest::testOnnxSetupDialogShowsDetectionOnlyNotice() {
    aitoolkit::ui::dialogs::OnnxSetupDialog dialog(QStringLiteral("test.onnx"));

    bool foundNotice = false;
    for (auto* label : dialog.findChildren<QLabel*>()) {
        if (label->text().contains(QStringLiteral("detection"))
            && label->text().contains(QStringLiteral("ONNX"))) {
            foundNotice = true;
            break;
        }
    }

    QVERIFY2(foundNotice, "Dialog should clearly state that ONNX import currently creates a detection manifest.");
}

void GuiValidationTest::testInferencePageEmptyState() {
    aitoolkit::ui::InferencePage page;

    auto* imagePathLabel = page.findChild<QLabel*>(QStringLiteral("InferenceImagePathLabel"));
    QVERIFY(imagePathLabel != nullptr);
    QVERIFY(imagePathLabel->text().contains(QStringLiteral("未选择")));

    auto* readinessLabel = page.findChild<QLabel*>(QStringLiteral("InferenceReadinessLabel"));
    QVERIFY(readinessLabel != nullptr);
    QVERIFY(readinessLabel->text().contains(QStringLiteral("模型")));

    auto* runButton = page.findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
    QVERIFY(runButton != nullptr);
    QVERIFY(!runButton->isEnabled());
}

void GuiValidationTest::testInferencePageRunDisabledWithoutImage() {
    aitoolkit::ui::InferencePage page;
    page.setModelReady(true);

    auto* runButton = page.findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
    QVERIFY(runButton != nullptr);
    QVERIFY(!runButton->isEnabled());
}

void GuiValidationTest::testInferencePageProgressControls() {
    aitoolkit::ui::InferencePage page;
    page.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page));

    auto* progressBar = page.findChild<QProgressBar*>(QStringLiteral("InferenceProgressBar"));
    QVERIFY(progressBar != nullptr);
    QVERIFY(!progressBar->isVisible());

    page.setRunning(true);
    QTest::qWait(50);
    QVERIFY2(progressBar->isVisible(), "Progress bar should be visible when running");

    page.setProgress(5, 10);
    QCOMPARE(progressBar->value(), 5);
    QCOMPARE(progressBar->maximum(), 10);

    page.setRunning(false);
    QTest::qWait(50);
    QVERIFY2(!progressBar->isVisible(), "Progress bar should be hidden when not running");
}

void GuiValidationTest::testInferencePageRunEnabledWhenBothReady() {
    aitoolkit::ui::InferencePage page;
    page.setModelReady(true);

    const QString imagePath = createTestImage(64, 64);
    page.setCurrentImagePath(imagePath);

    auto* runButton = page.findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
    QVERIFY(runButton != nullptr);
    QVERIFY2(runButton->isEnabled(), "Run button should be enabled when both model and image are ready");
}

void GuiValidationTest::testInferencePageRunDisabledWithoutModel() {
    aitoolkit::ui::InferencePage page;

    const QString imagePath = createTestImage(64, 64);
    page.setCurrentImagePath(imagePath);

    auto* runButton = page.findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
    QVERIFY(runButton != nullptr);
    QVERIFY2(!runButton->isEnabled(), "Run button should be disabled without model");
}

void GuiValidationTest::testInferencePageCancelButtonVisibility() {
    aitoolkit::ui::InferencePage page;
    page.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page));

    auto* cancelButton = page.findChild<QPushButton*>(QStringLiteral("SecondaryButton"));
    QVERIFY(cancelButton != nullptr);

    page.setRunning(true);
    QTest::qWait(50);

    bool cancelFound = false;
    for (auto* btn : page.findChildren<QPushButton*>()) {
        if (btn->text().contains(QStringLiteral("取消")) && btn->isVisible()) {
            cancelFound = true;
            break;
        }
    }
    QVERIFY2(cancelFound, "Cancel button should be visible when running");

    page.setRunning(false);
    QTest::qWait(50);

    cancelFound = false;
    for (auto* btn : page.findChildren<QPushButton*>()) {
        if (btn->text().contains(QStringLiteral("取消")) && btn->isVisible()) {
            cancelFound = true;
            break;
        }
    }
    QVERIFY2(!cancelFound, "Cancel button should be hidden when not running");
}

void GuiValidationTest::testInferencePageMaxFramesDefaults() {
    aitoolkit::ui::InferencePage page;

    auto* maxFramesSpin = page.findChild<QSpinBox*>();
    QVERIFY(maxFramesSpin != nullptr);
    QCOMPARE(maxFramesSpin->value(), 0);
    QCOMPARE(maxFramesSpin->minimum(), 0);
    QVERIFY(maxFramesSpin->maximum() >= 100000);
    QVERIFY(!maxFramesSpin->specialValueText().isEmpty());
}

void GuiValidationTest::testInferencePageProgressIndeterminate() {
    aitoolkit::ui::InferencePage page;
    page.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page));

    auto* progressBar = page.findChild<QProgressBar*>(QStringLiteral("InferenceProgressBar"));
    QVERIFY(progressBar != nullptr);

    page.setRunning(true);
    QTest::qWait(50);

    page.setProgress(0, 0);
    QCOMPARE(progressBar->minimum(), 0);
    QCOMPARE(progressBar->maximum(), 0);
}

void GuiValidationTest::testInferencePageEmptyImagePathResets() {
    aitoolkit::ui::InferencePage page;
    page.setModelReady(true);

    const QString imagePath = createTestImage(64, 64);
    page.setCurrentImagePath(imagePath);

    auto* runButton = page.findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
    QVERIFY(runButton->isEnabled());

    page.setCurrentImagePath(QString());
    QVERIFY2(!runButton->isEnabled(), "Run button should be disabled after clearing image path");

    auto* imagePathLabel = page.findChild<QLabel*>(QStringLiteral("InferenceImagePathLabel"));
    QVERIFY(imagePathLabel != nullptr);
    QVERIFY(imagePathLabel->text().contains(QStringLiteral("未选择")));
}

void GuiValidationTest::testInferencePageImageTooSmall() {
    aitoolkit::ui::InferencePage page;
    page.setModelReady(true);

    const QString imagePath = createTestImage(16, 16);
    page.setCurrentImagePath(imagePath);

    auto* runButton = page.findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
    QVERIFY2(!runButton->isEnabled(), "Run button should be disabled for too-small image");

    auto* imagePathLabel = page.findChild<QLabel*>(QStringLiteral("InferenceImagePathLabel"));
    QVERIFY(imagePathLabel != nullptr);
    QVERIFY2(imagePathLabel->text().contains(QStringLiteral("过小")),
             "Label should indicate image is too small");
}

void GuiValidationTest::testInferencePageImageTooLarge() {
    aitoolkit::ui::InferencePage page;
    page.setModelReady(true);

    const QString imagePath = createTestImage(4097, 64);
    page.setCurrentImagePath(imagePath);

    auto* runButton = page.findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
    QVERIFY2(!runButton->isEnabled(), "Run button should be disabled for too-large image");

    auto* imagePathLabel = page.findChild<QLabel*>(QStringLiteral("InferenceImagePathLabel"));
    QVERIFY(imagePathLabel != nullptr);
    QVERIFY2(imagePathLabel->text().contains(QStringLiteral("过大")),
             "Label should indicate image is too large");
}

void GuiValidationTest::testInferencePageRunningDisablesRunButton() {
    aitoolkit::ui::InferencePage page;
    page.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page));
    page.setModelReady(true);

    const QString imagePath = createTestImage(64, 64);
    page.setCurrentImagePath(imagePath);

    auto* runButton = page.findChild<QPushButton*>(QStringLiteral("PrimaryButton"));
    QVERIFY(runButton->isEnabled());

    page.setRunning(true);
    QTest::qWait(50);
    QVERIFY2(!runButton->isEnabled(), "Run button should be disabled while running");
    QVERIFY(!runButton->isVisible());

    page.setRunning(false);
    QTest::qWait(50);
    QVERIFY2(runButton->isEnabled(), "Run button should be re-enabled after stopping");
    QVERIFY2(runButton->isVisible(), "Run button should be visible after stopping");
}

void GuiValidationTest::testInferencePageConfidenceThresholdDefaults() {
    aitoolkit::ui::InferencePage page;

    QCOMPARE(page.confidenceThreshold(), 0.25);

    auto spinBoxes = page.findChildren<QDoubleSpinBox*>();
    QDoubleSpinBox* confSpin = nullptr;
    for (auto* spin : spinBoxes) {
        if (qFuzzyCompare(spin->value(), 0.25)) {
            confSpin = spin;
            break;
        }
    }
    QVERIFY(confSpin != nullptr);
    QCOMPARE(confSpin->minimum(), 0.0);
    QCOMPARE(confSpin->maximum(), 1.0);
    QCOMPARE(confSpin->singleStep(), 0.05);
}

void GuiValidationTest::testInferencePageNmsThresholdDefaults() {
    aitoolkit::ui::InferencePage page;

    QCOMPARE(page.nmsThreshold(), 0.45);

    auto spinBoxes = page.findChildren<QDoubleSpinBox*>();
    QDoubleSpinBox* nmsSpin = nullptr;
    for (auto* spin : spinBoxes) {
        if (qFuzzyCompare(spin->value(), 0.45)) {
            nmsSpin = spin;
            break;
        }
    }
    QVERIFY(nmsSpin != nullptr);
    QCOMPARE(nmsSpin->minimum(), 0.0);
    QCOMPARE(nmsSpin->maximum(), 1.0);
    QCOMPARE(nmsSpin->singleStep(), 0.05);
}

void GuiValidationTest::testInferencePageSetDefaultThresholds() {
    aitoolkit::ui::InferencePage page;

    page.setDefaultThresholds(0.5, 0.3);
    QCOMPARE(page.confidenceThreshold(), 0.5);
    QCOMPARE(page.nmsThreshold(), 0.3);

    page.setDefaultThresholds(0.1, 0.7);
    QCOMPARE(page.confidenceThreshold(), 0.1);
    QCOMPARE(page.nmsThreshold(), 0.7);
}

void GuiValidationTest::testResultsPageExportButtonsExist() {
    aitoolkit::ui::ResultsPage page;

    bool foundJsonExport = false;
    bool foundImageExport = false;
    for (auto* btn : page.findChildren<QPushButton*>()) {
        if (btn->text().contains(QStringLiteral("JSON"))) {
            foundJsonExport = true;
        }
        if (btn->text().contains(QStringLiteral("图片"))) {
            foundImageExport = true;
        }
    }
    QVERIFY(foundJsonExport);
    QVERIFY(foundImageExport);
}

void GuiValidationTest::testResultsPageDefaultSummary() {
    aitoolkit::ui::ResultsPage page;

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QCOMPARE(summaryLabel->text(), QStringLiteral("当前还没有推理结果"));
}

void GuiValidationTest::testResultsPageEmptySummaryReset() {
    aitoolkit::ui::ResultsPage page;

    aitoolkit::core::InferenceSummary summary;
    summary.modelName = QStringLiteral("TestModel");
    summary.inputPath = QStringLiteral("test.jpg");
    summary.detectionCount = 1;
    summary.elapsedMs = 10.0;
    summary.detections.push_back({});
    page.setSummary(summary);

    page.setSummary({});

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ResultsSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QCOMPARE(summaryLabel->text(), QStringLiteral("当前还没有推理结果"));

    auto* table = page.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(table != nullptr);
    QCOMPARE(table->rowCount(), 0);
}

void GuiValidationTest::testResultsPageTableHeaders() {
    aitoolkit::ui::ResultsPage page;

    auto* table = page.findChild<QTableWidget*>(QStringLiteral("DetectionsTable"));
    QVERIFY(table != nullptr);
    QCOMPARE(table->columnCount(), 4);

    QStringList expectedHeaders = {
        QStringLiteral("类别"),
        QStringLiteral("标签"),
        QStringLiteral("置信度"),
        QStringLiteral("框选范围"),
    };
    for (int i = 0; i < 4; ++i) {
        QCOMPARE(table->horizontalHeaderItem(i)->text(), expectedHeaders[i]);
    }
}

void GuiValidationTest::testHomePageDefaultState() {
    aitoolkit::ui::HomePage page;

    auto* recentModelsList = page.findChild<QListWidget*>(QStringLiteral("HomeRecentModelsList"));
    QVERIFY(recentModelsList != nullptr);
    QCOMPARE(recentModelsList->count(), 0);

    auto* recentInputsList = page.findChild<QListWidget*>(QStringLiteral("HomeRecentInputsList"));
    QVERIFY(recentInputsList != nullptr);
    QCOMPARE(recentInputsList->count(), 0);
}

void GuiValidationTest::testHomePageSetRecentModels() {
    aitoolkit::ui::HomePage page;

    page.setRecentModels({QStringLiteral("D:/models/yolo/model.json"),
                          QStringLiteral("D:/models/ssd/model.json")});

    auto* recentModelsList = page.findChild<QListWidget*>(QStringLiteral("HomeRecentModelsList"));
    QVERIFY(recentModelsList != nullptr);
    QCOMPARE(recentModelsList->count(), 2);
    QCOMPARE(recentModelsList->item(0)->data(Qt::UserRole).toString(),
             QStringLiteral("D:/models/yolo/model.json"));
    QCOMPARE(recentModelsList->item(1)->data(Qt::UserRole).toString(),
             QStringLiteral("D:/models/ssd/model.json"));
}

void GuiValidationTest::testHomePageSetRecentInputs() {
    aitoolkit::ui::HomePage page;

    page.setRecentInputs({QStringLiteral("D:/images/cat.jpg"),
                          QStringLiteral("D:/images/dog.png")});

    auto* recentInputsList = page.findChild<QListWidget*>(QStringLiteral("HomeRecentInputsList"));
    QVERIFY(recentInputsList != nullptr);
    QCOMPARE(recentInputsList->count(), 2);
    QCOMPARE(recentInputsList->item(0)->data(Qt::UserRole).toString(),
             QStringLiteral("D:/images/cat.jpg"));
    QCOMPARE(recentInputsList->item(1)->data(Qt::UserRole).toString(),
             QStringLiteral("D:/images/dog.png"));
}

void GuiValidationTest::testHomePageEmptyRecentLists() {
    aitoolkit::ui::HomePage page;

    page.setRecentModels({QStringLiteral("D:/models/yolo/model.json")});
    page.setRecentModels({});

    auto* recentModelsList = page.findChild<QListWidget*>(QStringLiteral("HomeRecentModelsList"));
    QCOMPARE(recentModelsList->count(), 0);

    page.setRecentInputs({QStringLiteral("D:/images/cat.jpg")});
    page.setRecentInputs({});

    auto* recentInputsList = page.findChild<QListWidget*>(QStringLiteral("HomeRecentInputsList"));
    QCOMPARE(recentInputsList->count(), 0);
}

void GuiValidationTest::testModelsPageDefaultState() {
    aitoolkit::ui::ModelsPage page;

    auto* pathLabel = page.findChild<QLabel*>(QStringLiteral("ManifestPathLabel"));
    QVERIFY(pathLabel != nullptr);
    QVERIFY(pathLabel->text().contains(QStringLiteral("未选择")));

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ManifestSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(summaryLabel->text().contains(QStringLiteral("选择")));
}

void GuiValidationTest::testModelsPageEmptyManifestPath() {
    aitoolkit::ui::ModelsPage page;

    page.setCurrentManifestPath(QStringLiteral("D:/models/model.json"));
    auto* pathLabel = page.findChild<QLabel*>(QStringLiteral("ManifestPathLabel"));
    QVERIFY(pathLabel->text().contains(QStringLiteral("model.json")));

    page.setCurrentManifestPath(QString());
    QVERIFY(pathLabel->text().contains(QStringLiteral("未选择")));
}

void GuiValidationTest::testModelsPageSetManifestSummary() {
    aitoolkit::ui::ModelsPage page;

    aitoolkit::core::ModelManifest manifest;
    manifest.manifestPath = QStringLiteral("D:/models/model.json");
    manifest.name = QStringLiteral("YOLOv8n");
    manifest.taskType = QStringLiteral("detection");
    manifest.backendType = QStringLiteral("onnxruntime");
    manifest.modelPath = QStringLiteral("D:/models/model.onnx");
    manifest.inputWidth = 640;
    manifest.inputHeight = 640;
    manifest.labels = {QStringLiteral("person"), QStringLiteral("car")};

    page.setCurrentManifest(manifest);

    auto* summaryLabel = page.findChild<QLabel*>(QStringLiteral("ManifestSummaryLabel"));
    QVERIFY(summaryLabel != nullptr);
    QVERIFY(summaryLabel->text().contains(QStringLiteral("YOLOv8n")));
    QVERIFY(summaryLabel->text().contains(QStringLiteral("640 x 640")));
    QVERIFY(summaryLabel->text().contains(QStringLiteral("2")));
}

void GuiValidationTest::testSettingsPageDefaultExportPlaceholder() {
    aitoolkit::ui::SettingsPage page;

    auto* exportEdit = page.findChild<QLineEdit*>();
    QVERIFY(exportEdit != nullptr);
    QVERIFY(!exportEdit->placeholderText().isEmpty());
}

void GuiValidationTest::testSettingsPageSetExportDirectory() {
    aitoolkit::ui::SettingsPage page;

    page.setDefaultExportDirectory(QStringLiteral("D:/exports"));
    auto* exportEdit = page.findChild<QLineEdit*>();
    QVERIFY(exportEdit != nullptr);
    QCOMPARE(exportEdit->text(), QStringLiteral("D:/exports"));
}

void GuiValidationTest::testSettingsPageSetRecentModels() {
    aitoolkit::ui::SettingsPage page;

    page.setRecentModels({QStringLiteral("D:/models/yolo/model.json"),
                          QStringLiteral("D:/models/ssd/model.json")});

    auto* recentModelsList = page.findChild<QListWidget*>(QStringLiteral("RecentModelsList"));
    QVERIFY(recentModelsList != nullptr);
    QCOMPARE(recentModelsList->count(), 2);
}

void GuiValidationTest::testSettingsPageSetRecentInputs() {
    aitoolkit::ui::SettingsPage page;

    page.setRecentInputs({QStringLiteral("D:/images/cat.jpg"),
                          QStringLiteral("D:/images/dog.png")});

    auto* recentInputsList = page.findChild<QListWidget*>(QStringLiteral("RecentInputsList"));
    QVERIFY(recentInputsList != nullptr);
    QCOMPARE(recentInputsList->count(), 2);
}

QString GuiValidationTest::createTestImage(int width, int height) {
    QTemporaryFile tempFile(QStringLiteral("aitoolkit_test_XXXXXX.bmp"));
    tempFile.setAutoRemove(false);
    if (!tempFile.open()) {
        return {};
    }
    const QString filePath = tempFile.fileName();
    tempFile.close();

    QImage image(width, height, QImage::Format_RGB32);
    image.fill(Qt::red);
    if (!image.save(filePath, "BMP")) {
        return {};
    }

    return filePath;
}

QTEST_MAIN(GuiValidationTest)
#include "test_gui_validation.moc"
