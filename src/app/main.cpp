#include "app/app_bootstrap.h"

#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    aitoolkit::app::AppBootstrap::initialize(app);

    // Task 6 will replace this temporary shell with ui::MainWindow.
    QMainWindow window;
    window.setWindowTitle(QStringLiteral("AI Toolkit C"));
    window.resize(1440, 900);
    window.show();

    return app.exec();
}
