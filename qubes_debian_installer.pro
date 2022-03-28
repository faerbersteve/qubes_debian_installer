TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        debiancontrolfile.cpp \
        main.cpp \
        qubesinstallhelper.cpp \
        qubespkg.cpp

HEADERS += \
    debiancontrolfile.h \
    qubesinstallhelper.h \
    qubespkg.h
