#include <QtTest>

#include <QStackedWidget>
#include <QWidget>

#include "ui/main_window.h"

namespace {

class MainWindowTest : public QObject {
    Q_OBJECT

private slots:
    void buildsThreePaneShell();
};

void MainWindowTest::buildsThreePaneShell() {
    aitoolkit::ui::MainWindow window;

    QVERIFY(window.centralWidget() != nullptr);

    const auto stacks = window.findChildren<QStackedWidget*>();
    QCOMPARE(stacks.size(), 1);
    QCOMPARE(stacks.front()->count(), 5);

    const auto navPanel = window.findChild<QWidget*>(QStringLiteral("NavPanel"));
    QVERIFY(navPanel != nullptr);

    const auto contextPanel = window.findChild<QWidget*>(QStringLiteral("ContextPanel"));
    QVERIFY(contextPanel != nullptr);
}

}  // namespace

QTEST_MAIN(MainWindowTest)

#include "test_main_window.moc"
