#include "models/postprocess_registry.h"

#include <map>
#include <stdexcept>

namespace aitoolkit::models {

struct PostprocessRegistry::Impl {
    std::map<std::string, PostprocessFn> decoders;
};

PostprocessRegistry& PostprocessRegistry::instance() {
    static PostprocessRegistry registry;
    if (!registry.impl_) {
        registry.impl_ = std::make_unique<Impl>();
    }
    return registry;
}

void PostprocessRegistry::registerDecoder(const std::string& decoderName, PostprocessFn fn) {
    impl_->decoders[decoderName] = std::move(fn);
}

PostprocessFn PostprocessRegistry::getDecoder(const std::string& decoderName) const {
    const auto it = impl_->decoders.find(decoderName);
    if (it == impl_->decoders.end()) {
        return nullptr;
    }
    return it->second;
}

bool PostprocessRegistry::hasDecoder(const std::string& decoderName) const {
    return impl_->decoders.find(decoderName) != impl_->decoders.end();
}

}  // namespace aitoolkit::models
