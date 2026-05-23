#pragma once

#include <QString>

namespace aitoolkit::core {

struct UpdateCheckResult {
    bool success = false;
    bool updateAvailable = false;
    QString latestVersion;
    QString releaseUrl;
    QString errorMessage;
};

class UpdateChecker {
public:
    static UpdateCheckResult checkForUpdates(const QString& currentVersion);
};

}  // namespace aitoolkit::core
