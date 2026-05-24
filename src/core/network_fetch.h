#pragma once

#include <QByteArray>
#include <QString>

class QUrl;

namespace aitoolkit::core {

struct NetworkFetchResult {
    bool success = false;
    QByteArray payload;
    QString errorMessage;
};

NetworkFetchResult fetchUrl(
    const QUrl& url,
  int timeoutMs = 30000,
    const QString& userAgent = QString());

}  // namespace aitoolkit::core
