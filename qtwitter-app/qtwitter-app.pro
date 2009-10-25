TEMPLATE = app
TARGET = qtwitter

CONFIG += oauth

# sets the TOP variable to the root source code dir
include(../common.pri)
include($${TOP}/translations/translations.pri)
include($${TOP}/twitterapi/twitterapi.pri)
include(src/dbus/dbus.pri)
include(src/oauth/oauth.pri)
include(src/pluginmgr/pluginmgr.pri)
include(src/accounts/accounts.pri)
include(src/qticonloader/qticonloader.pri)
QT += network \
    xml

SOURCES += src/main.cpp \
    src/qtwitterapp.cpp \
    src/mainwindow.cpp \
    src/statusedit.cpp \
    src/imagedownload.cpp \
    src/core.cpp \
    src/settings.cpp \
    src/statuswidget.cpp \
    src/statusmodel.cpp \
    src/twitpicview.cpp \
    src/twitpicengine.cpp \
    src/aboutdialog.cpp \
    src/statuslist.cpp \
    src/userinfobutton.cpp \
    src/userinfopopup.cpp \
    src/dmdialog.cpp \
    src/configfile.cpp \
    src/themes.cpp \
    src/updater.cpp \
    src/welcomedialog.cpp
HEADERS += src/qtwitterapp.h \
    src/mainwindow.h \
    src/statusedit.h \
    src/imagedownload.h \
    src/core.h \
    src/settings.h \
    src/statuswidget.h \
    src/statusmodel.h \
    src/statustextbrowser.h \
    src/statuslistview.h \
    src/twitpicview.h \
    src/twitpicengine.h \
    src/twitpictextedit.h \
    src/aboutdialog.h \
    src/statuslist.h \
    src/statuslist_p.h \
    src/userinfobutton.h \
    src/userinfopopup.h \
    src/dmdialog.h \
    src/configfile.h \
    src/themes.h \
    src/updater.h \
    src/welcomedialog.h \
    src/plugininterfaces.h
FORMS += ui/mainwindow.ui \
    ui/authdialog.ui \
    ui/settings.ui \
    ui/statuswidget.ui \
    ui/aboutdialog.ui \
    ui/twitpicview.ui \
    ui/twitpicnewphoto.ui \
    ui/userinfopopup.ui \
    ui/dmdialog.ui \
    ui/welcomedialog.ui

RESOURCES = res/resources.qrc
INCLUDEPATH += src

macx {
    ICON = macx/qtwitter.icns
    QMAKE_INFO_PLIST = macx/Info.plist
    QMAKE_LFLAGS += -F$${DESTDIR}/$${TARGET}.app/Contents/Frameworks
    LIBS += -framework twitterapi
}
else:unix {
    LIBS += -L$${DESTDIR} \
        -Wl,-rpath,$${DESTDIR} \
        $$TWITTERAPI_LIB
    target.path = $${INSTALL_PREFIX}/bin
    PLUGINS_DIR = $${INSTALL_PREFIX}/lib$${LIB_SUFFIX}/$${TARGET}/plugins
    DEFINES += PLUGINS_DIR='\\\"$${PLUGINS_DIR}\\\"'
    icons.path = $${INSTALL_PREFIX}/share/icons/scalable/apps
    icons.files = x11/icons/scalable/qtwitter.svg
    icons16.path = $${INSTALL_PREFIX}/share/icons/hicolor/16x16/apps
    icons16.files = x11/icons/16x16/qtwitter.png
    icons22.path = $${INSTALL_PREFIX}/share/icons/hicolor/22x22/apps
    icons22.files = x11/icons/22x22/qtwitter.png
    icons32.path = $${INSTALL_PREFIX}/share/icons/hicolor/32x32/apps
    icons32.files = x11/icons/32x32/qtwitter.png
    icons48.path = $${INSTALL_PREFIX}/share/icons/hicolor/48x48/apps
    icons48.files = x11/icons/48x48/qtwitter.png
    icons64.path = $${INSTALL_PREFIX}/share/icons/hicolor/64x64/apps
    icons64.files = x11/icons/64x64/qtwitter.png
    icons128.path = $${INSTALL_PREFIX}/share/icons/hicolor/128x128/apps
    icons128.files = x11/icons/128x128/qtwitter.png
    icons256.path = $${INSTALL_PREFIX}/share/icons/hicolor/256x256/apps
    icons256.files = x11/icons/256x256/qtwitter.png
    desktop.path = $${INSTALL_PREFIX}/share/applications
    desktop.files = x11/qtwitter.desktop
    INSTALLS += target \
        icons \
        icons16 \
        icons22 \
        icons32 \
        icons48 \
        icons64 \
        icons128 \
        icons256 \
        desktop
}
else:win32 {
    RC_FILE = win32/qtwitter.rc
    LIBS += -L$${DESTDIR} \
        $$TWITTERAPI_LIB \
}
