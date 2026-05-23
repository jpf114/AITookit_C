#include "app/app_bootstrap.h"

#include "core/app_logger.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QLocale>
#include <QSettings>
#include <QStringList>
#include <QTranslator>

#include "runtime/onnx_plugin.h"
#include "runtime/openvino_plugin.h"
#include "runtime/tensorrt_plugin.h"

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
    core::AppLogger::install();

    app.setApplicationName(QStringLiteral("AIToolkit"));
    app.setApplicationDisplayName(QStringLiteral("AI 检测工具"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("AIToolkit"));

    QSettings settings;
    QString langCode = settings.value(QStringLiteral("language")).toString();
    if (langCode.isEmpty()) {
        langCode = QLocale::system().name();
    }

    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList translationDirs = {
        appDir.filePath(QStringLiteral("translations")),
        appDir.filePath(QStringLiteral("share/translations")),
        appDir.filePath(QStringLiteral("../translations")),
        appDir.filePath(QStringLiteral("../share/translations")),
        appDir.filePath(QStringLiteral("../../translations")),
        appDir.filePath(QStringLiteral("../../share/translations")),
    };

    auto* translator = new QTranslator(&app);
    for (const QString& dir : translationDirs) {
        const QString qmPath = QDir(dir).filePath(QStringLiteral("ai_toolkit_%1.qm").arg(langCode));
        if (QFile::exists(qmPath) && translator->load(qmPath)) {
            app.installTranslator(translator);
            break;
        }
    }

    QIcon appIcon(QStringLiteral(":/icons/app_icon.png"));
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }

    QFile styleFile(applicationStylePath());
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    runtime::registerOnnxRuntimePlugin();
    runtime::registerTensorRtPlugin();
    runtime::registerOpenVinoPlugin();
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
