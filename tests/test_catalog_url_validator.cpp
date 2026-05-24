#include <QtTest>

#include <QUrl>

#include "core/catalog_url_validator.h"

namespace {

class CatalogUrlValidatorTest : public QObject {
    Q_OBJECT

private slots:
    void acceptsDefaultCatalogUrl();
    void rejectsHttpCatalogUrl();
    void rejectsLocalCatalogHost();
    void acceptsOfficialModelDownloadUrl();
    void rejectsNonGithubModelDownloadUrl();
};

void CatalogUrlValidatorTest::acceptsDefaultCatalogUrl() {
    const QUrl url(QStringLiteral(
        "https://raw.githubusercontent.com/jpf114/AITookit_C/main/resources/model_catalog.json"));
    QVERIFY(aitoolkit::core::isAllowedCatalogUrl(url));
    QVERIFY(aitoolkit::core::validateCatalogUrlOrError(url.toString()).isEmpty());
}

void CatalogUrlValidatorTest::rejectsHttpCatalogUrl() {
    const QUrl url(QStringLiteral("http://raw.githubusercontent.com/example/model_catalog.json"));
    QVERIFY(!aitoolkit::core::isAllowedCatalogUrl(url));
    QVERIFY(!aitoolkit::core::validateCatalogUrlOrError(url.toString()).isEmpty());
}

void CatalogUrlValidatorTest::rejectsLocalCatalogHost() {
    const QUrl url(QStringLiteral("https://localhost/catalog.json"));
    QVERIFY(!aitoolkit::core::isAllowedCatalogUrl(url));
}

void CatalogUrlValidatorTest::acceptsOfficialModelDownloadUrl() {
    const QUrl url(QStringLiteral(
        "https://github.com/ultralytics/assets/releases/download/v8.4.0/yolov8n.onnx"));
    QVERIFY(aitoolkit::core::isAllowedModelDownloadUrl(url));
    QVERIFY(aitoolkit::core::validateModelDownloadUrlOrError(url.toString()).isEmpty());
}

void CatalogUrlValidatorTest::rejectsNonGithubModelDownloadUrl() {
    const QUrl url(QStringLiteral("https://example.com/yolov8n.onnx"));
    QVERIFY(!aitoolkit::core::isAllowedModelDownloadUrl(url));
    QVERIFY(!aitoolkit::core::validateModelDownloadUrlOrError(url.toString()).isEmpty());
}

}  // namespace

QTEST_MAIN(CatalogUrlValidatorTest)
#include "test_catalog_url_validator.moc"
