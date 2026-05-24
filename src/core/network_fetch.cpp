#include "core/network_fetch.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace aitoolkit::core {

NetworkFetchResult fetchUrl(
    const QUrl& url,
    const int timeoutMs,
    const QString& userAgent) {
    NetworkFetchResult result;
    if (!url.isValid()) {
        result.errorMessage = QStringLiteral("Invalid URL");
        return result;
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    if (!userAgent.isEmpty()) {
        request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
    } else {
        reply->abort();
        result.errorMessage = QStringLiteral("Network request timed out");
        reply->deleteLater();
        return result;
    }

    if (reply->error() != QNetworkReply::NoError) {
        result.errorMessage = reply->errorString();
        reply->deleteLater();
        return result;
    }

    result.success = true;
    result.payload = reply->readAll();
    reply->deleteLater();
    return result;
}

}  // namespace aitoolkit::core
