#pragma once

#include <QCoreApplication>
#include <QString>
#include <stdexcept>

namespace aitoolkit::core {

class AppError : public std::runtime_error {
public:
    explicit AppError(const QString& message)
        : std::runtime_error(message.toUtf8().constData()), message_(message) {}

    [[nodiscard]] const QString& qMessage() const noexcept {
        return message_;
    }

private:
    QString message_;
};

}  // namespace aitoolkit::core
