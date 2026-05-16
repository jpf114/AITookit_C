#include "app/app_bootstrap.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStringList>

#include "runtime/onnx_plugin.h"

namespace aitoolkit::app {

namespace {

QString findExistingPath(const QStringList& candidatePaths) {
    for (const QString& candidatePath : candidatePaths) {
        if (QFile::exists(candidatePath)) {
            return QDir::cleanPath(candidatePath);
        }
    }
    return {};
}

}  // namespace

void AppBootstrap::initialize(QApplication& app) {
    app.setApplicationName(QStringLiteral("AI 检测工具"));
    app.setApplicationDisplayName(QStringLiteral("AI 检测工具"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));
    app.setOrganizationName(QStringLiteral("MyProject"));

    QFile styleFile(applicationStylePath());
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    runtime::registerOnnxRuntimePlugin();
}

QString AppBootstrap::applicationStylePath() {
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList candidates{
        appDir.filePath("resources/themes/app.qss"),
        appDir.filePath("share/themes/app.qss"),
        appDir.filePath("../resources/themes/app.qss"),
        appDir.filePath("../share/themes/app.qss"),
        appDir.filePath("../../resources/themes/app.qss"),
        appDir.filePath("../../share/themes/app.qss"),
        appDir.filePath("../../../resources/themes/app.qss"),
        appDir.filePath("../../../share/themes/app.qss"),
        appDir.filePath("../../../../resources/themes/app.qss"),
        appDir.filePath("../../../../share/themes/app.qss"),
        appDir.filePath("../../../../../resources/themes/app.qss"),
        appDir.filePath("../../../../../share/themes/app.qss"),
    };

    return findExistingPath(candidates);
}

}  // namespace aitoolkit::app
