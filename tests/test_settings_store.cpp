#include <QtTest>

#include <QDir>
#include <QTemporaryDir>

#include "core/settings_store.h"

namespace {

class SettingsStoreTest : public QObject {
    Q_OBJECT

private slots:
    void savesAndLoadsRecentLists();
    void deduplicatesAndCapsRecentEntries();
    void deduplicatesRecentModelsIgnoringCaseOnWindows();
    void respectsRecentModelsReadbackLimit();
    void savesAndLoadsDefaultExportDirectory();
    void savesAndLoadsGpuThreadLanguageAndGeometry();
};

QString settingsFilePath(QTemporaryDir& tempDir, const QString& fileName) {
    return QDir(tempDir.path()).filePath(fileName);
}

void SettingsStoreTest::savesAndLoadsRecentLists() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");
    const QString iniPath = settingsFilePath(tempDir, QStringLiteral("settings.ini"));

    {
        aitoolkit::core::SettingsStore store(iniPath, QSettings::IniFormat);
        store.setRecentModels({QStringLiteral("C:/models/a.onnx"), QStringLiteral("C:/models/b.onnx")});
        store.setRecentInputs({QStringLiteral("C:/inputs/1.jpg"), QStringLiteral("C:/inputs/2.jpg")});
    }

    aitoolkit::core::SettingsStore store(iniPath, QSettings::IniFormat);
    const QStringList expectedModels{QStringLiteral("C:/models/a.onnx"), QStringLiteral("C:/models/b.onnx")};
    const QStringList expectedInputs{QStringLiteral("C:/inputs/1.jpg"), QStringLiteral("C:/inputs/2.jpg")};
    QCOMPARE(store.recentModels(), expectedModels);
    QCOMPARE(store.recentInputs(), expectedInputs);
}

void SettingsStoreTest::deduplicatesAndCapsRecentEntries() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");
    const QString iniPath = settingsFilePath(tempDir, QStringLiteral("settings.ini"));

    aitoolkit::core::SettingsStore store(iniPath, QSettings::IniFormat);
    store.addRecentModel(QStringLiteral("C:/models/a.onnx"), 3);
    store.addRecentModel(QStringLiteral("C:/models/b.onnx"), 3);
    store.addRecentModel(QStringLiteral("C:/models/a.onnx"), 3);
    store.addRecentModel(QStringLiteral("C:/models/c.onnx"), 3);
    store.addRecentModel(QStringLiteral("C:/models/d.onnx"), 3);

    const QStringList expectedModels{
        QStringLiteral("C:/models/d.onnx"),
        QStringLiteral("C:/models/c.onnx"),
        QStringLiteral("C:/models/a.onnx"),
    };
    QCOMPARE(store.recentModels(), expectedModels);
}

void SettingsStoreTest::deduplicatesRecentModelsIgnoringCaseOnWindows() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");
    const QString iniPath = settingsFilePath(tempDir, QStringLiteral("settings.ini"));

    aitoolkit::core::SettingsStore store(iniPath, QSettings::IniFormat);
    store.addRecentModel(QStringLiteral("C:/Models/A.onnx"), 10);
    store.addRecentModel(QStringLiteral("c:/models/a.onnx"), 10);

#ifdef Q_OS_WIN
    const QStringList expectedModels{QStringLiteral("c:/models/a.onnx")};
#else
    const QStringList expectedModels{QStringLiteral("c:/models/a.onnx"), QStringLiteral("C:/Models/A.onnx")};
#endif
    QCOMPARE(store.recentModels(), expectedModels);
}

void SettingsStoreTest::respectsRecentModelsReadbackLimit() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");
    const QString iniPath = settingsFilePath(tempDir, QStringLiteral("settings.ini"));

    aitoolkit::core::SettingsStore store(iniPath, QSettings::IniFormat);
    for (int index = 0; index < 12; ++index) {
        store.addRecentModel(QStringLiteral("C:/models/model_%1.onnx").arg(index), 12);
    }

    const QStringList recentModels = store.recentModels();
    QCOMPARE(recentModels.size(), 10);
    QCOMPARE(recentModels.front(), QStringLiteral("C:/models/model_11.onnx"));
    QCOMPARE(recentModels.back(), QStringLiteral("C:/models/model_2.onnx"));
}

void SettingsStoreTest::savesAndLoadsDefaultExportDirectory() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");
    const QString iniPath = settingsFilePath(tempDir, QStringLiteral("settings.ini"));

    {
        aitoolkit::core::SettingsStore store(iniPath, QSettings::IniFormat);
        store.setDefaultExportDirectory(QStringLiteral("C:/exports/results"));
    }

    aitoolkit::core::SettingsStore store(iniPath, QSettings::IniFormat);
    QCOMPARE(store.defaultExportDirectory(), QStringLiteral("C:/exports/results"));
}

void SettingsStoreTest::savesAndLoadsGpuThreadLanguageAndGeometry() {
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Temporary directory should be created");
    const QString iniPath = settingsFilePath(tempDir, QStringLiteral("settings.ini"));

    const QByteArray geometry = QByteArray::fromHex("0102030405");
    {
        aitoolkit::core::SettingsStore store(iniPath, QSettings::IniFormat);
        store.setInferenceThreadCount(8);
        store.setUseGPUInference(true);
        store.setLanguage(QStringLiteral("en"));
        store.setWindowGeometry(geometry);
        store.setLastModelManifestPath(QStringLiteral("C:/models/test.json"));
    }

    aitoolkit::core::SettingsStore store(iniPath, QSettings::IniFormat);
    QCOMPARE(store.inferenceThreadCount(), 8);
    QVERIFY(store.useGPUInference());
    QCOMPARE(store.language(), QStringLiteral("en"));
    QCOMPARE(store.windowGeometry(), geometry);
    QCOMPARE(store.lastModelManifestPath(), QStringLiteral("C:/models/test.json"));
}

}  // namespace

QTEST_MAIN(SettingsStoreTest)

#include "test_settings_store.moc"
