#include "app/app_bootstrap.h"
#include "app/crash_handler.h"

#include <QApplication>

#include "ui/main_window.h"

int main(int argc, char* argv[]) {
    aitoolkit::app::installCrashHandler();

    QApplication app(argc, argv);
    aitoolkit::app::AppBootstrap::initialize(app);

    aitoolkit::ui::MainWindow window;
    window.show();

    return app.exec();
}
