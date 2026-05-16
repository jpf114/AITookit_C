#pragma once

#include <QSettings>
#include <QString>
#include <QStringList>

namespace aitoolkit::core {

class SettingsStore {
public:
    explicit SettingsStore(
        const QString& organizationName = QStringLiteral("MyProject"),
        const QString& applicationName = QStringLiteral("AI Toolkit C"));
    SettingsStore(const QString& filePath, QSettings::Format format);

    [[nodiscard]] QStringList recentModels() const;
    [[nodiscard]] QStringList recentInputs() const;
    [[nodiscard]] QString defaultExportDirectory() const;
    [[nodiscard]] QString lastModelManifestPath() const;
    [[nodiscard]] QByteArray windowGeometry() const;
    [[nodiscard]] int inferenceThreadCount() const;
    [[nodiscard]] bool useGPUInference() const;

    void setRecentModels(const QStringList& paths);
    void setRecentInputs(const QStringList& paths);
    void setDefaultExportDirectory(const QString& directoryPath);
    void setLastModelManifestPath(const QString& path);
    void setWindowGeometry(const QByteArray& geometry);
    void setInferenceThreadCount(int count);
    void setUseGPUInference(bool useGPU);

    void addRecentModel(const QString& path, int maxItems = 10);
    void addRecentInput(const QString& path, int maxItems = 10);

private:
    void setRecentValues(const QString& key, const QStringList& values);
    [[nodiscard]] QStringList recentValues(const QString& key) const;
    static bool pathsEqual(const QString& lhs, const QString& rhs);
    static QStringList normalizeRecentValues(const QStringList& values, int maxItems);

    QSettings settings_;
};

}  // namespace aitoolkit::core
