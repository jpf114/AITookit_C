#include <QtTest>

#include "core/model_manifest.h"
#include "models/classification_model.h"

namespace {

class ClassificationModelTest : public QObject {
    Q_OBJECT

private slots:
    void appliesSoftmaxAndSortsByConfidence();
    void filtersByConfidenceThreshold();
    void mapsLabelsFromManifest();
    void returnsEmptyForEmptyTensors();
    void returnsEmptyWhenAllBelowThreshold();
    void handlesSingleClass();
    void handlesDominantLogit();
    void handlesMoreClassesThanLabels();
};

aitoolkit::core::ModelManifest makeClassificationManifest() {
    aitoolkit::core::ModelManifest manifest;
    manifest.name = QStringLiteral("Test Classification");
    manifest.taskType = QStringLiteral("classification");
    manifest.confidenceThreshold = 0.1;
    manifest.labels = {QStringLiteral("cat"), QStringLiteral("dog"), QStringLiteral("bird")};
    return manifest;
}

void ClassificationModelTest::appliesSoftmaxAndSortsByConfidence() {
    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 3};
    tensor.values = {1.0f, 2.0f, 3.0f};

    const auto manifest = makeClassificationManifest();
    const auto results = aitoolkit::models::ClassificationModel::postprocessClassifications(
        {tensor}, manifest, 0.0);

    QCOMPARE(results.size(), 3);

    QVERIFY(results[0].confidence > results[1].confidence);
    QVERIFY(results[1].confidence > results[2].confidence);

    QCOMPARE(results[0].classId, 2);
    QCOMPARE(results[0].label, QStringLiteral("bird"));

    float sum = 0.0f;
    for (const auto& item : results) {
        sum += item.confidence;
    }
    QVERIFY(qAbs(sum - 1.0f) < 0.001f);
}

void ClassificationModelTest::filtersByConfidenceThreshold() {
    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 3};
    tensor.values = {1.0f, 2.0f, 3.0f};

    const auto manifest = makeClassificationManifest();
    const auto results = aitoolkit::models::ClassificationModel::postprocessClassifications(
        {tensor}, manifest, 0.5);

    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].classId, 2);
}

void ClassificationModelTest::mapsLabelsFromManifest() {
    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 3};
    tensor.values = {3.0f, 1.0f, 2.0f};

    const auto manifest = makeClassificationManifest();
    const auto results = aitoolkit::models::ClassificationModel::postprocessClassifications(
        {tensor}, manifest, 0.0);

    QCOMPARE(results[0].classId, 0);
    QCOMPARE(results[0].label, QStringLiteral("cat"));
    QCOMPARE(results[1].classId, 2);
    QCOMPARE(results[1].label, QStringLiteral("bird"));
    QCOMPARE(results[2].classId, 1);
    QCOMPARE(results[2].label, QStringLiteral("dog"));
}

void ClassificationModelTest::returnsEmptyForEmptyTensors() {
    const auto manifest = makeClassificationManifest();
    const auto results = aitoolkit::models::ClassificationModel::postprocessClassifications(
        {}, manifest, 0.0);

    QVERIFY(results.isEmpty());
}

void ClassificationModelTest::returnsEmptyWhenAllBelowThreshold() {
    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 3};
    tensor.values = {1.0f, 1.0f, 1.0f};

    const auto manifest = makeClassificationManifest();
    const auto results = aitoolkit::models::ClassificationModel::postprocessClassifications(
        {tensor}, manifest, 0.99);

    QVERIFY(results.isEmpty());
}

void ClassificationModelTest::handlesSingleClass() {
    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 1};
    tensor.values = {5.0f};

    aitoolkit::core::ModelManifest manifest;
    manifest.labels = {QStringLiteral("only_class")};

    const auto results = aitoolkit::models::ClassificationModel::postprocessClassifications(
        {tensor}, manifest, 0.0);

    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].classId, 0);
    QVERIFY(qAbs(results[0].confidence - 1.0f) < 0.001f);
    QCOMPARE(results[0].label, QStringLiteral("only_class"));
}

void ClassificationModelTest::handlesDominantLogit() {
    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 3};
    tensor.values = {-10.0f, 0.0f, -10.0f};

    const auto manifest = makeClassificationManifest();
    const auto results = aitoolkit::models::ClassificationModel::postprocessClassifications(
        {tensor}, manifest, 0.01);

    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].classId, 1);
    QVERIFY(results[0].confidence > 0.99f);
    QCOMPARE(results[0].label, QStringLiteral("dog"));
}

void ClassificationModelTest::handlesMoreClassesThanLabels() {
    aitoolkit::runtime::InferenceTensor tensor;
    tensor.shape = {1, 5};
    tensor.values = {0.1f, 0.2f, 0.3f, 5.0f, 0.4f};

    aitoolkit::core::ModelManifest manifest;
    manifest.labels = {QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")};

    const auto results = aitoolkit::models::ClassificationModel::postprocessClassifications(
        {tensor}, manifest, 0.0);

    QCOMPARE(results[0].classId, 3);
    QVERIFY(results[0].label.isEmpty());
}

}  // namespace

QTEST_MAIN(ClassificationModelTest)

#include "test_classification_model.moc"
