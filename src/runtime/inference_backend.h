#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace aitoolkit::runtime {

struct InferenceTensor {
    std::string name;
    std::vector<int64_t> shape;
    std::vector<float> values;
};

}  // namespace aitoolkit::runtime
