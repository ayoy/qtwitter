TEMPLATE = app
TARGET = qtwitter

CONFIG += oauth

# sets the TOP variable to the root source code dir
include(../common.pri)
DESTDIR = $${TOP}
include($${TOP}/twitterapi/twitterapi.pri)
include($${TOP}/urlshortener/urlshortener.pri)
include(src/dbus/dbus.pri)
include(src/oauth/oauth.pri)
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
TRANSLATIONS += loc/qtwitter_ca_ES.ts \
    loc/qtwitter_de_DE.ts \
    loc/qtwitter_es_ES.ts \
    loc/qtwitter_ja_JP.ts \
    loc/qtwitter_it_IT.ts \
    loc/qtwitter_fr_FR.ts \
    loc/qtwitter_nb_NO.ts \
    loc/qtwitter_pl_PL.ts \
    loc/qtwitter_pt_BR.ts \
    loc/qtwitter_cs_CZ.ts

linux-* {
    RESOURCES = res/resources_x11.qrc
} else {
    RESOURCES = res/resources.qrc
}
#linux-* {
#    RESOURCES = res/resources_x11.qrc
#    i18n.commands = lrelease $${_PRO_FILE_} && mkdir -p $${_PRO_FILE_PWD_}/res/loc && mv $${_PRO_FILE_PWD_}/loc/*.qm $${_PRO_FILE_PWD_}/res/loc
#    QMAKE_EXTRA_TARGETS += i18n
#    PRE_TARGETDEPS += i18n
#} else {
#    # systems other than linux provide translations inside the binary
#    # - they have to exist at compile time
#    system("lrelease $${_PRO_FILE_} && mkdir -p $${_PRO_FILE_PWD_}/res/loc && mv $${_PRO_FILE_PWD_}/loc/*.qm $${_PRO_FILE_PWD_}/res/loc")
#    RESOURCES = res/resources.qrc
#}

UI_DIR = tmp
MOC_DIR = tmp
RCC_DIR = tmp
OBJECTS_DIR = tmp
INCLUDEPATH += $${TOP} \
    src \
    tmp


macx {
    ICON = macx/qtwitter.icns
    QMAKE_INFO_PLIST = macx/Info.plist
    QMAKE_LFLAGS += -F$${TOP}/$${TARGET}.app/Contents/Frameworks
    LIBS += -framework \
        twitterapi \
        -framework \
        urlshortener
}
else:unix {
    LIBS += -L$${TOP} \
        -Wl,-rpath,$${TOP} \
        $$TWITTERAPI_LIB \
        $$URLSHORTENER_LIB
    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}
    target.path = $${INSTALL_PREFIX}/bin
    SHARE_DIR = $${INSTALL_PREFIX}/share/$${TARGET}
    PLUGINS_DIR = $${INSTALL_PREFIX}/lib/$${TARGET}/plugins
    DEFINES += SHARE_DIR='\\\"$${SHARE_DIR}\\\"'
    DEFINES += PLUGINS_DIR='\\\"$${PLUGINS_DIR}\\\"'
    translations.path = $${SHARE_DIR}/loc
    translations.files = $${TRANSLATIONS}
    translations.files ~= s/\.ts/.qm/g
    translations.files ~= s!^loc!res/loc!g
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
        translations \
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
    LIBS += -L$${TOP} \
        $$TWITTERAPI_LIB \
        $$URLSHORTENER_LIB \
        $$QOAUTH_LIB
}
