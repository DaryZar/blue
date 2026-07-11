QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Подключаем библиотеку тестов Qt
QT += testlib

HEADERS += \
    mainwindow.h \
    tests.h

SOURCES += \
    mainwindow.cpp \
    main.cpp

RESOURCES += \
    resources.qrc

# ====================================================================
# КАК ЗАПУСТИТЬ ТЕСТЫ:
# Раскомментируйте (уберите значок #) строчку ниже,
# чтобы запустить юнит-тесты вместо обычного приложения.
# ====================================================================
# DEFINES += RUN_UNIT_TESTS
