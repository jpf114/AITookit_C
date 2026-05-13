#pragma once

#include <QString>

namespace aitoolkit::core {

struct Error {
    QString code;
    QString message;
    QString detail;

    [[nodiscard]] bool isEmpty() const {
        return code.isEmpty() && message.isEmpty() && detail.isEmpty();
    }
};

template <typename T>
struct Result {
    T value{};
    Error error{};

    [[nodiscard]] bool ok() const {
        return error.isEmpty();
    }
};

}  // namespace aitoolkit::core
