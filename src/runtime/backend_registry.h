#pragma once

#include <QString>
#include <QVector>

#include <memory>

#include "runtime/backend_plugin.h"

namespace aitoolkit::runtime {

class BackendRegistry {
public:
    static BackendRegistry& instance();

    void registerBackend(std::unique_ptr<BackendPlugin> plugin);
    void unregisterBackend(const QString& backendName);
    BackendPlugin* getBackend(const QString& backendName) const;
    QVector<BackendInfo> allBackendInfos() const;
    QStringList availableBackendNames() const;

private:
    BackendRegistry();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace aitoolkit::runtime
