#include "app/app_bootstrap.h"
#include "app/crash_handler.h"

#include <QApplication>
#include <QByteArray>

#include "ui/main_window.h"

int main(int argc, char* argv[]) {
#ifdef Q_OS_WIN
    qputenv("QT_QPA_PLATFORM", "windows");
#endif

    aitoolkit::app::installCrashHandler();

    QApplication app(argc, argv);
    aitoolkit::app::AppBootstrap::initialize(app);

    aitoolkit::ui::MainWindow window;
    window.show();

    return app.exec();
}
