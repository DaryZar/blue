#include "mainwindow.h"
#include "tests.h" // Подключаем наши тесты
#include <QApplication>

int main(int argc, char *argv[])
{
#ifdef RUN_UNIT_TESTS
    // Если тесты включены в blue.pro, запускаем их и не открываем графическое окно
    return runAllTests(argc, argv);
#else
    // Иначе запускаем наше красивое голубое приложение
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
#endif
}
