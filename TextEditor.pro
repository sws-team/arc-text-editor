QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
TARGET = ArcTextEditor
DESTDIR = bin


SOURCES += \
    commands.cpp \
    editdialog.cpp \
    findwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp\
    localedata.cpp \
    textmodel.cpp

HEADERS += \
    commands.h \
    editdialog.h \
    findwidget.h \
    mainwindow.h \
    settingsdef.h \
    settingsdialog.h \
    localedata.h \
    textmodel.h

FORMS += \
    editdialog.ui \
    findwidget.ui \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
	res.qrc \
	tr/translations.qrc

TRANSLATIONS = tr/tr_ru.ts

RC_ICONS = icon.ico

include($$PWD/Widgets/version.pri)
include($$PWD/Widgets/AboutWidget/AboutWidget.pri)
include($$PWD/Widgets/RecentFiles/RecentFiles.pri)
include($$PWD/Widgets/Styles/Styles.pri)
