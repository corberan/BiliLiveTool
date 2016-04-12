#-------------------------------------------------
#
# Project created by QtCreator 2016-04-05T19:59:14
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BiliLiveDanmuTool
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    livedanmuhandler.cpp \
    serverclient.cpp \
    scroller/danmukutextitem.cpp \
    scroller/scroller.cpp \
    plugin/bililivedanmutoolplugin.cpp \
    plugin/intent.cpp \
    update.cpp

HEADERS  += mainwindow.h \
    livedanmuhandler.h \
    serverclient.h \
    scroller/danmukutextitem.h \
    scroller/scroller.h \
    plugin/bililivedanmutoolplugin_global.h \
    plugin/bililivedanmutoolplugin.h \
    plugin/intent.h \
    update.h

FORMS    += mainwindow.ui \
    update.ui
