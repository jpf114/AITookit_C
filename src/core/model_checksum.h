#pragma once

#include <QString>

namespace aitoolkit::core {

QString findChecksumsFilePath(const QString& modelsDirectory = QString());

[[nodiscard]] QString expectedSha256ForFile(
    const QString& fileName,
    const QString& modelsDirectory = QString());

[[nodiscard]] bool verifyFileSha256(const QString& filePath, const QString& expectedSha256);

[[nodiscard]] bool verifyDownloadedModel(
    const QString& modelsDirectory,
    const QString& fileName,
    QString* errorMessage = nullptr);

}  // namespace aitoolkit::core
