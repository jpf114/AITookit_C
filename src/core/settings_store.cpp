#include "core/settings_store.h"

#include <QDir>

namespace aitoolkit::core {

namespace {

constexpr auto kRecentModelsKey = "recent/models";
constexpr auto kRecentInputsKey = "recent/inputs";
constexpr auto kDefaultExportDirectoryKey = "export/defaultDirectory";
constexpr auto kLastModelManifestKey = "state/lastModelManifest";
constexpr auto kWindowGeometryKey = "state/windowGeometry";
constexpr auto kInferenceThreadCountKey = "inference/threadCount";
constexpr auto kUseGPUInferenceKey = "inference/useGPU";
constexpr auto kLanguageKey = "language";
constexpr auto kModelCatalogUrlKey = "catalog/modelUrl";

}  // namespace

SettingsStore::SettingsStore(const QString& organizationName, const QString& applicationName)
    : settings_(organizationName, applicationName) {
}

SettingsStore::SettingsStore(const QString& filePath, const QSettings::Format format)
    : settings_(filePath, format) {
}

QStringList SettingsStore::recentModels() const {
    return recentValues(QString::fromLatin1(kRecentModelsKey));
}

QStringList SettingsStore::recentInputs() const {
    return recentValues(QString::fromLatin1(kRecentInputsKey));
}

QString SettingsStore::defaultExportDirectory() const {
    return QDir::cleanPath(settings_.value(QString::fromLatin1(kDefaultExportDirectoryKey)).toString().trimmed());
}

void SettingsStore::setRecentModels(const QStringList& paths) {
    setRecentValues(QString::fromLatin1(kRecentModelsKey), paths);
}

void SettingsStore::setRecentInputs(const QStringList& paths) {
    setRecentValues(QString::fromLatin1(kRecentInputsKey), paths);
}

void SettingsStore::setDefaultExportDirectory(const QString& directoryPath) {
    settings_.setValue(
        QString::fromLatin1(kDefaultExportDirectoryKey),
        QDir::cleanPath(directoryPath.trimmed()));
    settings_.sync();
}

QString SettingsStore::lastModelManifestPath() const {
    return settings_.value(QString::fromLatin1(kLastModelManifestKey)).toString().trimmed();
}

QByteArray SettingsStore::windowGeometry() const {
    return settings_.value(QString::fromLatin1(kWindowGeometryKey)).toByteArray();
}

void SettingsStore::setLastModelManifestPath(const QString& path) {
    settings_.setValue(QString::fromLatin1(kLastModelManifestKey), QDir::cleanPath(path.trimmed()));
    settings_.sync();
}

void SettingsStore::setWindowGeometry(const QByteArray& geometry) {
    settings_.setValue(QString::fromLatin1(kWindowGeometryKey), geometry);
    settings_.sync();
}

int SettingsStore::inferenceThreadCount() const {
    return settings_.value(QString::fromLatin1(kInferenceThreadCountKey), 1).toInt();
}

void SettingsStore::setInferenceThreadCount(const int count) {
    settings_.setValue(QString::fromLatin1(kInferenceThreadCountKey), count);
    settings_.sync();
}

bool SettingsStore::useGPUInference() const {
    return settings_.value(QString::fromLatin1(kUseGPUInferenceKey), false).toBool();
}

void SettingsStore::setUseGPUInference(const bool useGPU) {
    settings_.setValue(QString::fromLatin1(kUseGPUInferenceKey), useGPU);
    settings_.sync();
}

QString SettingsStore::language() const {
    return settings_.value(QString::fromLatin1(kLanguageKey)).toString().trimmed();
}

void SettingsStore::setLanguage(const QString& langCode) {
    settings_.setValue(QString::fromLatin1(kLanguageKey), langCode);
    settings_.sync();
}

QString SettingsStore::modelCatalogUrl() const {
    return settings_.value(QString::fromLatin1(kModelCatalogUrlKey)).toString().trimmed();
}

void SettingsStore::setModelCatalogUrl(const QString& url) {
    settings_.setValue(QString::fromLatin1(kModelCatalogUrlKey), url.trimmed());
    settings_.sync();
}

void SettingsStore::addRecentModel(const QString& path, const int maxItems) {
    QStringList values = recentModels();
    values.prepend(path);
    setRecentModels(normalizeRecentValues(values, maxItems));
}

void SettingsStore::addRecentInput(const QString& path, const int maxItems) {
    QStringList values = recentInputs();
    values.prepend(path);
    setRecentInputs(normalizeRecentValues(values, maxItems));
}

void SettingsStore::setRecentValues(const QString& key, const QStringList& values) {
    settings_.setValue(key, normalizeRecentValues(values, values.size()));
    settings_.sync();
}

QStringList SettingsStore::recentValues(const QString& key) const {
    return normalizeRecentValues(settings_.value(key).toStringList(), 10);
}

bool SettingsStore::pathsEqual(const QString& lhs, const QString& rhs) {
#ifdef Q_OS_WIN
    return QString::compare(lhs, rhs, Qt::CaseInsensitive) == 0;
#else
    return lhs == rhs;
#endif
}

QStringList SettingsStore::normalizeRecentValues(const QStringList& values, const int maxItems) {
    QStringList normalized;
    normalized.reserve(values.size());

    for (const QString& value : values) {
        const QString cleaned = QDir::cleanPath(value.trimmed());
        if (cleaned.isEmpty()) {
            continue;
        }

        bool alreadyPresent = false;
        for (const QString& existing : normalized) {
            if (pathsEqual(existing, cleaned)) {
                alreadyPresent = true;
                break;
            }
        }
        if (alreadyPresent) {
            continue;
        }

        normalized.append(cleaned);
        if (maxItems > 0 && normalized.size() >= maxItems) {
            break;
        }
    }

    return normalized;
}

}  // namespace aitoolkit::core
