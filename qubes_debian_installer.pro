TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        qubesinstallhelper.cpp \
        qubespkg.cpp

HEADERS += \
    qubesinstallhelper.h \
    qubespkg.h
