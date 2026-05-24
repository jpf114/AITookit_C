#include "core/model_checksum.h"

#include "core/app_paths.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

namespace aitoolkit::core {

namespace {

QString normalizeSha256(const QString& hash) {
    return hash.trimmed().toLower();
}

QHash<QString, QString> loadChecksumMap(const QString& checksumsPath) {
    QHash<QString, QString> checksums;
    QFile file(checksumsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return checksums;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return checksums;
    }

    const QJsonObject files = document.object().value(QStringLiteral("files")).toObject();
    for (auto it = files.begin(); it != files.end(); ++it) {
        const QString hash = normalizeSha256(it.value().toString());
        if (!hash.isEmpty()) {
            checksums.insert(it.key(), hash);
        }
    }
    return checksums;
}

QString checksumsPathForModelsDirectory(const QString& modelsDirectory) {
    const QString modelsDir = modelsDirectory.isEmpty() ? findModelsDirectory() : modelsDirectory;
    return QDir(modelsDir).filePath(QStringLiteral("checksums.json"));
}

}  // namespace

QString findChecksumsFilePath(const QString& modelsDirectory) {
    const QString directPath = checksumsPathForModelsDirectory(modelsDirectory);
    if (QFileInfo::exists(directPath)) {
        return directPath;
    }

    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList candidates = {
        appDir.filePath(QStringLiteral("../share/models/checksums.json")),
        appDir.filePath(QStringLiteral("../../share/models/checksums.json")),
        QDir::current().filePath(QStringLiteral("models/checksums.json")),
        QDir::current().filePath(QStringLiteral("../models/checksums.json")),
    };

    for (const QString& path : candidates) {
        if (QFileInfo::exists(path)) {
            return QDir::cleanPath(path);
        }
    }

    return directPath;
}

QString expectedSha256ForFile(const QString& fileName, const QString& modelsDirectory) {
    const QString checksumsPath = findChecksumsFilePath(modelsDirectory);
    const QHash<QString, QString> checksums = loadChecksumMap(checksumsPath);
    return checksums.value(fileName);
}

bool verifyFileSha256(const QString& filePath, const QString& expectedSha256) {
    const QString expected = normalizeSha256(expectedSha256);
    if (expected.isEmpty()) {
        return true;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QCryptographicHash hasher(QCryptographicHash::Sha256);
    if (!hasher.addData(&file)) {
        return false;
    }

    return hasher.result().toHex().toLower() == expected.toLatin1();
}

bool verifyDownloadedModel(
    const QString& modelsDirectory,
    const QString& fileName,
    QString* errorMessage) {
    const QString onnxPath = QDir(modelsDirectory).filePath(fileName);
    if (!QFileInfo::exists(onnxPath)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Downloaded model file not found: %1").arg(fileName);
        }
        return false;
    }

    const QString expected = expectedSha256ForFile(fileName, modelsDirectory);
    if (expected.isEmpty()) {
        return true;
    }

    if (verifyFileSha256(onnxPath, expected)) {
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral("SHA256 verification failed for %1").arg(fileName);
    }
    return false;
}

}  // namespace aitoolkit::core
