#include "runtime/backend_registry.h"

#include <map>
#include <mutex>

namespace aitoolkit::runtime {

struct BackendRegistry::Impl {
    std::map<QString, std::unique_ptr<BackendPlugin>> backends;
};

BackendRegistry& BackendRegistry::instance() {
    static BackendRegistry registry;
    static std::once_flag initFlag;
    std::call_once(initFlag, [&]() {
        registry.impl_ = std::make_unique<Impl>();
    });
    return registry;
}

void BackendRegistry::registerBackend(std::unique_ptr<BackendPlugin> plugin) {
    if (!plugin) {
        return;
    }
    const QString name = plugin->info().name;
    impl_->backends[name] = std::move(plugin);
}

void BackendRegistry::unregisterBackend(const QString& backendName) {
    impl_->backends.erase(backendName);
}

BackendPlugin* BackendRegistry::getBackend(const QString& backendName) const {
    const auto it = impl_->backends.find(backendName);
    if (it == impl_->backends.end()) {
        return nullptr;
    }
    return it->second.get();
}

QVector<BackendInfo> BackendRegistry::allBackendInfos() const {
    QVector<BackendInfo> infos;
    infos.reserve(static_cast<int>(impl_->backends.size()));
    for (const auto& [name, plugin] : impl_->backends) {
        infos.append(plugin->info());
    }
    return infos;
}

QStringList BackendRegistry::availableBackendNames() const {
    QStringList names;
    for (const auto& [name, plugin] : impl_->backends) {
        if (plugin->info().isAvailable) {
            names.append(name);
        }
    }
    return names;
}

}  // namespace aitoolkit::runtime
