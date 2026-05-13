#include "core/json_utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>

#include <stdexcept>

namespace aitoolkit::core {

namespace {

std::runtime_error makePathError(const QString& filePath, const QString& message) {
    return std::runtime_error(QStringLiteral("%1: %2").arg(QDir::toNativeSeparators(filePath), message).toStdString());
}

}  // namespace

QJsonObject readJsonObject(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw makePathError(filePath, file.errorString());
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        throw makePathError(filePath, parseError.errorString());
    }
    if (!document.isObject()) {
        throw makePathError(filePath, QStringLiteral("JSON root is not an object"));
    }

    return document.object();
}

void writeJsonObject(const QString& filePath, const QJsonObject& object) {
    const QFileInfo fileInfo(filePath);
    const QDir dir = fileInfo.dir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        throw makePathError(filePath, QStringLiteral("Unable to create parent directory"));
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw makePathError(filePath, file.errorString());
    }

    const QJsonDocument document(object);
    if (file.write(document.toJson(QJsonDocument::Indented)) < 0) {
        throw makePathError(filePath, file.errorString());
    }
    if (!file.commit()) {
        throw makePathError(filePath, file.errorString());
    }
}

}  // namespace aitoolkit::core
