#include "core/update_checker.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QUrl>

namespace aitoolkit::core {

UpdateCheckResult UpdateChecker::checkForUpdates(const QString& currentVersion) {
    UpdateCheckResult result;
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QStringLiteral("https://api.github.com/repos/AIToolkit/AITookit_C/releases/latest")));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AIToolkit-UpdateChecker/1.0"));

    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        result.errorMessage = reply->errorString();
        reply->deleteLater();
        return result;
    }

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();

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
