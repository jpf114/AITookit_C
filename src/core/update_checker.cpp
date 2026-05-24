#include "core/update_checker.h"

#include "app_version.h"
#include "core/catalog_url_validator.h"
#include "core/network_fetch.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

namespace aitoolkit::core {

UpdateCheckResult UpdateChecker::checkForUpdates(const QString& currentVersion) {
    UpdateCheckResult result;

    const QUrl url(QStringLiteral("https://api.github.com/repos/%1/releases/latest")
                       .arg(QStringLiteral(AITOOLKIT_GITHUB_REPO)));
    if (!isAllowedCatalogUrl(url)) {
        result.errorMessage = QStringLiteral("Update check URL is not allowed");
        return result;
    }

    const NetworkFetchResult fetch = fetchUrl(
        url,
        30000,
        QStringLiteral("AIToolkit-UpdateChecker/1.0"));
    if (!fetch.success) {
        result.errorMessage = fetch.errorMessage;
        return result;
    }

    const QJsonDocument document = QJsonDocument::fromJson(fetch.payload);
    if (!document.isObject()) {
        result.errorMessage = QStringLiteral("Invalid release response");
        return result;
    }

    const QJsonObject object = document.object();
    const QString tagName = object.value(QStringLiteral("tag_name")).toString().remove(QLatin1Char('v'));
    const QString htmlUrl = object.value(QStringLiteral("html_url")).toString();

    result.success = true;
    result.latestVersion = tagName;
    result.releaseUrl = htmlUrl;
    result.updateAvailable = !tagName.isEmpty() && tagName != currentVersion;
    return result;
}

}  // namespace aitoolkit::core
