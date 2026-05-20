#include "models/postprocess_registry.h"

#include <map>
#include <mutex>
#include <stdexcept>

namespace aitoolkit::models {

struct PostprocessRegistry::Impl {
    std::map<std::string, PostprocessFn> decoders;
    mutable std::mutex mutex;
};

PostprocessRegistry::PostprocessRegistry()
    : impl_(std::make_unique<Impl>()) {}

PostprocessRegistry& PostprocessRegistry::instance() {
    static PostprocessRegistry registry;
    return registry;
}

void PostprocessRegistry::registerDecoder(const std::string& decoderName, PostprocessFn fn) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->decoders[decoderName] = std::move(fn);
}

PostprocessFn PostprocessRegistry::getDecoder(const std::string& decoderName) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    const auto it = impl_->decoders.find(decoderName);
    if (it == impl_->decoders.end()) {
        return nullptr;
    }
    return it->second;
}

bool PostprocessRegistry::hasDecoder(const std::string& decoderName) const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->decoders.find(decoderName) != impl_->decoders.end();
}

}  // namespace aitoolkit::models
