#pragma once

#include <QString>

class QUrl;

namespace aitoolkit::core {

[[nodiscard]] bool isHttpsUrl(const QUrl& url);

[[nodiscard]] bool isAllowedCatalogHost(const QString& host);

[[nodiscard]] bool isAllowedModelDownloadHost(const QString& host);

[[nodiscard]] bool isAllowedCatalogUrl(const QUrl& url);

[[nodiscard]] bool isAllowedModelDownloadUrl(const QUrl& url);

[[nodiscard]] QString validateCatalogUrlOrError(const QString& urlText);

[[nodiscard]] QString validateModelDownloadUrlOrError(const QString& urlText);

}  // namespace aitoolkit::core
