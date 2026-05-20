#include "core/app_paths.h"

#include <QCoreApplication>
#include <QDir>

namespace aitoolkit::core {

QString findModelsDirectory() {
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList searchPaths = {
        appDir.filePath(QStringLiteral("../models")),
        appDir.filePath(QStringLiteral("../../models")),
        appDir.filePath(QStringLiteral("models"))
    };

    for (const QString& path : searchPaths) {
        if (QDir(path).exists()) {
            return QDir::cleanPath(path);
        }
    }

    return QDir::cleanPath(appDir.filePath(QStringLiteral("../models")));
}

}  // namespace aitoolkit::core
