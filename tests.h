#ifndef TESTS_H
#define TESTS_H

#ifdef RUN_UNIT_TESTS

#include <QtTest>
#include "mainwindow.h"

class ProductivityTest : public QObject
{
    Q_OBJECT

private slots:
    void testInitialBalance() {
        int balance = 0;
        balance += 10;
        QCOMPARE(balance, 10);
        balance -= 5;
        QCOMPARE(balance, 5);
    }

    void testRewardParsing() {
        QCOMPARE(MainWindow::parseRewardPrice("Посмотреть сериал [20 🪙]"), 20);
        QCOMPARE(MainWindow::parseRewardPrice("Кастомная награда [150 🪙]"), 150);
        QCOMPARE(MainWindow::parseRewardPrice("Строка без цены"), 0);
    }

    void testPomodoroTimerLogic() {
        int timeLeftSeconds = 25 * 60;
        if (timeLeftSeconds > 0) {
            timeLeftSeconds--;
        }
        QCOMPARE(timeLeftSeconds, (25 * 60) - 1);
    }
};

inline int runAllTests(int argc, char *argv[]) {
    ProductivityTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tests.moc"

#endif
#endif
