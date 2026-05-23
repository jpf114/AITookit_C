#include <QtTest>

#include <QDir>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>

#include "services/unicode_io.h"

#include <opencv2/imgcodecs.hpp>

namespace {

class UnicodeIoTest : public QObject {
    Q_OBJECT

private slots:
    void readsImageViaUnicodePath();
    void rejectsOversizedFile();
    void returnsEmptyForMissingFile();
};

void UnicodeIoTest::readsImageViaUnicodePath() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString subDir = QDir(tempDir.path()).filePath(QStringLiteral("测试目录"));
    QVERIFY(QDir().mkpath(subDir));

    const QString imagePath = QDir(subDir).filePath(QStringLiteral("样本.png"));
    QImage image(32, 32, QImage::Format_RGB32);
    image.fill(Qt::blue);
    QVERIFY(image.save(imagePath, "PNG"));

    const cv::Mat mat = aitoolkit::services::imreadUnicode(imagePath, cv::IMREAD_COLOR);
    QVERIFY(!mat.empty());
    QCOMPARE(mat.cols, 32);
    QCOMPARE(mat.rows, 32);
}

void UnicodeIoTest::rejectsOversizedFile() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString filePath = QDir(tempDir.path()).filePath(QStringLiteral("large.bin"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.resize(513LL * 1024 * 1024));
    file.close();

    const std::vector<char> buffer = aitoolkit::services::readFileToBuffer(filePath);
    QVERIFY(buffer.empty());
}

void UnicodeIoTest::returnsEmptyForMissingFile() {
    const cv::Mat mat = aitoolkit::services::imreadUnicode(
        QStringLiteral("C:/nonexistent/path/不存在.png"), cv::IMREAD_COLOR);
    QVERIFY(mat.empty());
}

}  // namespace

QTEST_MAIN(UnicodeIoTest)

#include "test_unicode_io.moc"
