#include "app/app_bootstrap.h"

#include <QApplication>

#include "ui/main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    aitoolkit::app::AppBootstrap::initialize(app);

    aitoolkit::ui::MainWindow window;
    window.show();

    return app.exec();
}
