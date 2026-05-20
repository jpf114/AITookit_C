#include "core/model_manifest.h"

#include "core/json_utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextStream>

#include <cmath>
#include <limits>
#include <stdexcept>

namespace aitoolkit::core {

namespace {

QString normalizedIdentifier(const QString& value) {
    return value.trimmed().toLower();
}

std::runtime_error manifestError(const QString& manifestPath, const QString& fieldName, const QString& message) {
    return std::runtime_error(
        QStringLiteral("%1 [%2]: %3")
            .arg(QDir::toNativeSeparators(manifestPath), fieldName, message)
            .toStdString());
}

QString resolvePath(const QDir& baseDir, const QString& path) {
    if (path.isEmpty()) {
        return {};
    }

    const QFileInfo fileInfo(path);
    if (fileInfo.isAbsolute()) {
        return QDir::cleanPath(path);
    }

    return QDir::cleanPath(baseDir.filePath(path));
}

QStringList parseStringArray(const QJsonValue& value) {
    QStringList result;
    const QJsonArray items = value.toArray();
    result.reserve(items.size());
    for (const QJsonValue& item : items) {
        if (!item.isString()) {
            throw std::runtime_error("labels_inline array item is not a string");
        }
        result.append(item.toString());
    }
    return result;
}

QStringList readLabelsFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(
            QStringLiteral("%1: %2").arg(QDir::toNativeSeparators(filePath), file.errorString()).toStdString());
    }

    QTextStream stream(&file);
    QStringList labels;
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (!line.isEmpty()) {
            labels.append(line);
        }
    }
    return labels;
}

QString optionalStringField(const QJsonObject& object, const QString& manifestPath, const QString& fieldName) {
    const QJsonValue value = object.value(fieldName);
    if (value.isUndefined() || value.isNull()) {
        return {};
    }
    if (!value.isString()) {
        throw manifestError(manifestPath, fieldName, QStringLiteral("field must be a string when provided"));
    }
    return value.toString().trimmed();
}

QString requireStringField(const QJsonObject& object, const QString& manifestPath, const QString& fieldName) {
    const QJsonValue value = object.value(fieldName);
    if (!value.isString()) {
        throw manifestError(manifestPath, fieldName, QStringLiteral("field is required and must be a string"));
    }

    const QString text = value.toString().trimmed();
    if (text.isEmpty()) {
        throw manifestError(manifestPath, fieldName, QStringLiteral("field must not be empty"));
    }
    return text;
}

QString requireKnownTaskTypeField(const QJsonObject& object, const QString& manifestPath, const QString& fieldName) {
    const QString taskType = normalizedIdentifier(requireStringField(object, manifestPath, fieldName));
    if (taskType == QStringLiteral("detection") ||
        taskType == QStringLiteral("classification") ||
        taskType == QStringLiteral("segmentation")) {
        return taskType;
    }

    throw manifestError(
        manifestPath,
        fieldName,
        QStringLiteral("unsupported task type: %1").arg(taskType));
}

int requirePositiveIntField(const QJsonObject& object, const QString& manifestPath, const QString& fieldName) {
    const QJsonValue value = object.value(fieldName);
    if (!value.isDouble()) {
        throw manifestError(manifestPath, fieldName, QStringLiteral("field is required and must be a number"));
    }

    const double rawNumber = value.toDouble();
    if (!std::isfinite(rawNumber) || std::floor(rawNumber) != rawNumber) {
        throw manifestError(manifestPath, fieldName, QStringLiteral("field must be an integer greater than 0"));
    }

    const int number = static_cast<int>(rawNumber);
    if (number <= 0 || rawNumber > static_cast<double>(std::numeric_limits<int>::max())) {
        throw manifestError(manifestPath, fieldName, QStringLiteral("field must be an integer greater than 0"));
    }
    return number;
}

double optionalNumericField(const QJsonObject& object, const QString& manifestPath, const QString& fieldName, const double defaultValue) {
    const QJsonValue value = object.value(fieldName);
    if (value.isUndefined() || value.isNull()) {
        return defaultValue;
    }
    if (!value.isDouble()) {
        throw manifestError(manifestPath, fieldName, QStringLiteral("field must be a number when provided"));
    }
    return value.toDouble();
}

QStringList optionalStringArrayField(const QJsonObject& object, const QString& manifestPath, const QString& fieldName) {
    const QJsonValue value = object.value(fieldName);
    if (value.isUndefined() || value.isNull()) {
        return {};
    }
    if (!value.isArray()) {
        throw manifestError(manifestPath, fieldName, QStringLiteral("field must be an array of strings when provided"));
    }

    try {
        return parseStringArray(value);
    } catch (const std::runtime_error&) {
        throw manifestError(manifestPath, fieldName, QStringLiteral("field must contain only string items"));
    }
}

}  // namespace

ModelManifest loadModelManifest(const QString& manifestPath) {
    const QFileInfo manifestInfo(manifestPath);
    const QString cleanManifestPath = QDir::cleanPath(manifestInfo.absoluteFilePath());
    const QJsonObject object = readJsonObject(cleanManifestPath);
    const QDir manifestDir = manifestInfo.absoluteDir();

    ModelManifest manifest;
    manifest.manifestPath = cleanManifestPath;
    manifest.name = requireStringField(object, cleanManifestPath, QStringLiteral("name"));
    manifest.taskType = requireKnownTaskTypeField(object, cleanManifestPath, QStringLiteral("task_type"));
    manifest.backendType = normalizedIdentifier(requireStringField(object, cleanManifestPath, QStringLiteral("backend")));
    manifest.modelPath = resolvePath(
        manifestDir,
        requireStringField(object, cleanManifestPath, QStringLiteral("model")));
    manifest.labelsPath = resolvePath(manifestDir, optionalStringField(object, cleanManifestPath, QStringLiteral("labels")));
    manifest.decoder = normalizedIdentifier(optionalStringField(object, cleanManifestPath, QStringLiteral("decoder")));
    manifest.inputWidth = requirePositiveIntField(object, cleanManifestPath, QStringLiteral("input_width"));
    manifest.inputHeight = requirePositiveIntField(object, cleanManifestPath, QStringLiteral("input_height"));
    manifest.confidenceThreshold = optionalNumericField(
        object,
        cleanManifestPath,
        QStringLiteral("confidence_threshold"),
        0.25);
    manifest.nmsThreshold = optionalNumericField(
        object,
        cleanManifestPath,
        QStringLiteral("nms_threshold"),
        0.45);
    manifest.labelsInline = optionalStringArrayField(object, cleanManifestPath, QStringLiteral("labels_inline"));

    if (!QFileInfo::exists(manifest.modelPath)) {
        throw manifestError(cleanManifestPath, QStringLiteral("model"), QStringLiteral("resolved model file does not exist"));
    }

    if (!manifest.labelsInline.isEmpty()) {
        manifest.labels = manifest.labelsInline;
    } else if (!manifest.labelsPath.isEmpty()) {
        if (!QFileInfo::exists(manifest.labelsPath)) {
            throw manifestError(cleanManifestPath, QStringLiteral("labels"), QStringLiteral("resolved labels file does not exist"));
        }
        try {
            manifest.labels = readLabelsFile(manifest.labelsPath);
        } catch (const std::runtime_error& error) {
            throw manifestError(
                cleanManifestPath,
                QStringLiteral("labels"),
                QStringLiteral("failed to read labels file: %1").arg(QString::fromStdString(error.what())));
        }
    }

    return manifest;
}

}  // namespace aitoolkit::core
