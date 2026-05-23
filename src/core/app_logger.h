#pragma once

#include <QString>

namespace aitoolkit::core {

class AppLogger {
public:
    static void install();
    static void logInfo(const QString& message);
    static void logWarning(const QString& message);
    static void logError(const QString& message);
    static QString logFilePath();
};

}  // namespace aitoolkit::core
