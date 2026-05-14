#include <QtTest>

#include <QListWidget>
#include <QSignalSpy>

#include "ui/pages/settings_page.h"

namespace {

class SettingsPageTest : public QObject {
    Q_OBJECT

private slots:
    void emitsRecentModelActivatedSignal();
    void emitsRecentInputActivatedSignal();
};

void SettingsPageTest::emitsRecentModelActivatedSignal() {
    aitoolkit::ui::SettingsPage page;
    page.setRecentModels({QStringLiteral("D:/models/yolo/model.json")});

    auto* recentModelsList = page.findChild<QListWidget*>(QStringLiteral("RecentModelsList"));
    QVERIFY(recentModelsList != nullptr);
    QVERIFY(recentModelsList->item(0) != nullptr);

    QSignalSpy spy(&page, SIGNAL(recentModelActivated(QString)));
    QVERIFY(spy.isValid());

    QVERIFY(QMetaObject::invokeMethod(
        recentModelsList,
        "itemClicked",
        Qt::DirectConnection,
        Q_ARG(QListWidgetItem*, recentModelsList->item(0))));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QStringLiteral("D:/models/yolo/model.json"));
}

void SettingsPageTest::emitsRecentInputActivatedSignal() {
    aitoolkit::ui::SettingsPage page;
    page.setRecentInputs({QStringLiteral("D:/images/example.jpg")});

    auto* recentInputsList = page.findChild<QListWidget*>(QStringLiteral("RecentInputsList"));
    QVERIFY(recentInputsList != nullptr);
    QVERIFY(recentInputsList->item(0) != nullptr);

    QSignalSpy spy(&page, SIGNAL(recentInputActivated(QString)));
    QVERIFY(spy.isValid());

    QVERIFY(QMetaObject::invokeMethod(
        recentInputsList,
        "itemClicked",
        Qt::DirectConnection,
        Q_ARG(QListWidgetItem*, recentInputsList->item(0))));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QStringLiteral("D:/images/example.jpg"));
}

}  // namespace

QTEST_MAIN(SettingsPageTest)

#include "test_settings_page.moc"
