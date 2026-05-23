#include "app/app_bootstrap.h"
#include "app/crash_handler.h"

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QTextStream>

#include "ui/main_window.h"

int main(int argc, char* argv[]) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_PLATFORM", "windows");
#endif

    for (int index = 1; index < argc; ++index) {
        const QByteArray arg = argv[index];
        if (arg == "--version" || arg == "-v") {
            QCoreApplication coreApp(argc, argv);
            coreApp.setApplicationName(QStringLiteral("AIToolkit"));
            coreApp.setApplicationVersion(QStringLiteral("1.0.0"));
            QTextStream(stdout) << coreApp.applicationName() << ' ' << coreApp.applicationVersion() << '\n';
            return 0;
        }
    }

    aitoolkit::app::installCrashHandler();

    QApplication app(argc, argv);
    aitoolkit::app::AppBootstrap::initialize(app);

    aitoolkit::ui::MainWindow window;
    window.show();

    return app.exec();
}
