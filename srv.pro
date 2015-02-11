#-------------------------------------------------
#
# Project created by QtCreator 2014-11-26T14:31:29
#
#-------------------------------------------------

QT += core gui widgets network

TARGET = srv
TEMPLATE = app



SOURCES += main.cpp \
    Serveur.cpp \
    Client.cpp \
    ssh2_exec.cpp

HEADERS  += \
    Serveur.h \
    Client.h \
    libssh2_config.h \
    ssh2_exec.h

FORMS    += \
    Serveur.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/release/ -lssh2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../usr/local/lib/debug/ -lssh2
else:unix: LIBS += -L$$PWD/../../../../../usr/local/lib/ -lssh2

INCLUDEPATH += $$PWD/../../../../../usr/local/lib
DEPENDPATH += $$PWD/../../../../../usr/local/lib
