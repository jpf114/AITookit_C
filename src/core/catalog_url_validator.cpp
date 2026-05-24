#include "core/catalog_url_validator.h"

#include <QHostAddress>
#include <QUrl>

namespace aitoolkit::core {

namespace {

bool hostMatchesSuffix(const QString& host, const QString& suffix) {
    return host == suffix || host.endsWith(QStringLiteral(".") + suffix, Qt::CaseInsensitive);
}

bool isPrivateOrLocalHost(const QString& host) {
    if (host.compare(QStringLiteral("localhost"), Qt::CaseInsensitive) == 0) {
        return true;
    }

    const QHostAddress address(host);
    return address.isLoopback() || address.isLinkLocal() || address.isSiteLocal();
}

}  // namespace

bool isHttpsUrl(const QUrl& url) {
    return url.isValid() && url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0;
}

bool isAllowedCatalogHost(const QString& host) {
    const QString normalizedHost = host.trimmed().toLower();
    if (normalizedHost.isEmpty() || isPrivateOrLocalHost(normalizedHost)) {
        return false;
    }

    return hostMatchesSuffix(normalizedHost, QStringLiteral("githubusercontent.com"))
        || hostMatchesSuffix(normalizedHost, QStringLiteral("github.com"));
}

bool isAllowedModelDownloadHost(const QString& host) {
    const QString normalizedHost = host.trimmed().toLower();
    if (normalizedHost.isEmpty() || isPrivateOrLocalHost(normalizedHost)) {
        return false;
    }

    return hostMatchesSuffix(normalizedHost, QStringLiteral("github.com"));
}

bool isAllowedCatalogUrl(const QUrl& url) {
    return isHttpsUrl(url) && isAllowedCatalogHost(url.host());
}

bool isAllowedModelDownloadUrl(const QUrl& url) {
    return isHttpsUrl(url) && isAllowedModelDownloadHost(url.host());
}

QString validateCatalogUrlOrError(const QString& urlText) {
    const QString trimmed = urlText.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    const QUrl url(trimmed);
    if (!url.isValid()) {
        return QStringLiteral("Invalid catalog URL.");
    }
    if (!isHttpsUrl(url)) {
        return QStringLiteral("Catalog URL must use HTTPS.");
    }
    if (!isAllowedCatalogHost(url.host())) {
        return QStringLiteral("Catalog URL host is not allowed. Use GitHub raw content URLs.");
    }
    return {};
}

QString validateModelDownloadUrlOrError(const QString& urlText) {
    const QString trimmed = urlText.trimmed();
    if (trimmed.isEmpty()) {
        return QStringLiteral("Model download URL is empty.");
    }

    const QUrl url(trimmed);
    if (!url.isValid()) {
        return QStringLiteral("Invalid model download URL.");
    }
    if (!isHttpsUrl(url)) {
        return QStringLiteral("Model download URL must use HTTPS.");
    }
    if (!isAllowedModelDownloadHost(url.host())) {
        return QStringLiteral("Model download URL host is not allowed. Use official GitHub release URLs.");
    }
    return {};
}

}  // namespace aitoolkit::core
