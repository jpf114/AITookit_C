#include <QtTest>

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include "core/model_checksum.h"

namespace {

class ModelChecksumTest : public QObject {
    Q_OBJECT

private slots:
    void skipsVerificationWhenHashMissing();
    void verifiesMatchingSha256();
    void rejectsMismatchedSha256();
};

void ModelChecksumTest::skipsVerificationWhenHashMissing() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString modelsDir = tempDir.path();
    const QString checksumsPath = QDir(modelsDir).filePath(QStringLiteral("checksums.json"));
    QFile checksumsFile(checksumsPath);
    QVERIFY(checksumsFile.open(QIODevice::WriteOnly));
    checksumsFile.write(
        R"({"version":1,"files":{"demo.onnx":""}})");
    checksumsFile.close();

    const QString modelPath = QDir(modelsDir).filePath(QStringLiteral("demo.onnx"));
    QFile modelFile(modelPath);
    QVERIFY(modelFile.open(QIODevice::WriteOnly));
    modelFile.write("model-bytes");
    modelFile.close();

    QVERIFY(aitoolkit::core::verifyDownloadedModel(modelsDir, QStringLiteral("demo.onnx")));
}

void ModelChecksumTest::verifiesMatchingSha256() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString modelsDir = tempDir.path();
    const QString modelPath = QDir(modelsDir).filePath(QStringLiteral("demo.onnx"));
    QFile modelFile(modelPath);
    QVERIFY(modelFile.open(QIODevice::WriteOnly));
    modelFile.write("model-bytes");
    modelFile.close();

    const QString expected = QStringLiteral("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    QVERIFY(!aitoolkit::core::verifyFileSha256(modelPath, expected));

    const QByteArray payload = "verified-model";
    QVERIFY(modelFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    modelFile.write(payload);
    modelFile.close();

    const QString hash = QString::fromLatin1(
        QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex());
    const QString checksumsPath = QDir(modelsDir).filePath(QStringLiteral("checksums.json"));
    QFile checksumsFile(checksumsPath);
    QVERIFY(checksumsFile.open(QIODevice::WriteOnly));
    checksumsFile.write(
        QStringLiteral(R"({"version":1,"files":{"demo.onnx":"%1"}})").arg(hash).toUtf8());
    checksumsFile.close();

    QVERIFY(aitoolkit::core::verifyDownloadedModel(modelsDir, QStringLiteral("demo.onnx")));
}

void ModelChecksumTest::rejectsMismatchedSha256() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString modelsDir = tempDir.path();
    const QString checksumsPath = QDir(modelsDir).filePath(QStringLiteral("checksums.json"));
    QFile checksumsFile(checksumsPath);
    QVERIFY(checksumsFile.open(QIODevice::WriteOnly));
    checksumsFile.write(
        R"({"version":1,"files":{"demo.onnx":"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"}})");
    checksumsFile.close();

    const QString modelPath = QDir(modelsDir).filePath(QStringLiteral("demo.onnx"));
    QFile modelFile(modelPath);
    QVERIFY(modelFile.open(QIODevice::WriteOnly));
    modelFile.write("different-bytes");
    modelFile.close();

    QString errorMessage;
    QVERIFY(!aitoolkit::core::verifyDownloadedModel(
        modelsDir, QStringLiteral("demo.onnx"), &errorMessage));
    QVERIFY(errorMessage.contains(QStringLiteral("SHA256")));
}

}  // namespace

QTEST_MAIN(ModelChecksumTest)
#include "test_model_checksum.moc"
