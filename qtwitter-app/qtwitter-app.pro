TEMPLATE = app
TARGET = qtwitter
TOP = ..
DESTDIR = $${TOP}
include($${TOP}/twitterapi/twitterapi.pri)
macx {
    ICON = macx/qtwitter.icns
    QMAKE_INFO_PLIST = macx/Info.plist
    QMAKE_LFLAGS += -F$${TOP}/$${TARGET}.app/Contents/Frameworks
    LIBS += -framework \
        twitterapi
}
else:unix {
    LIBS += -L$${TOP}/lib \
        $$TWITTERAPI_LIB \
        -Wl,-rpath,$${PWD}/$${TOP}/lib
    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}
    target.path = $${INSTALL_PREFIX}/bin
    doc.path = $${INSTALL_PREFIX}/share/doc/$${TARGET}
    doc.files = ../CHANGELOG \
        ../README \
        ../LICENSE
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
        doc \
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
    LIBS += -L$${TOP}/lib \
        $$TWITTERAPI_LIB \
        -Wl,-rpath,$${PWD}/$${TOP}/lib
}
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/statusedit.cpp \
    src/httpconnection.cpp \
    src/imagedownload.cpp \
    src/core.cpp \
    src/settings.cpp \
    src/tweet.cpp \
    src/tweetmodel.cpp \
    src/twitpicview.cpp \
    src/twitpicengine.cpp \
    src/twitpicxmlparser.cpp \
    src/twitteraccountsdelegate.cpp \
    src/qtwitter.cpp \
    src/twitteraccountsmodel.cpp \
    src/urlshortener.cpp \
    src/aboutdialog.cpp
HEADERS += src/mainwindow.h \
    src/statusedit.h \
    src/httpconnection.h \
    src/imagedownload.h \
    src/core.h \
    src/settings.h \
    src/tweet.h \
    src/tweetmodel.h \
    src/tweettextbrowser.h \
    src/statuslist.h \
    src/twitpicview.h \
    src/twitpicengine.h \
    src/twitpicxmlparser.h \
    src/statustextedit.h \
    src/twitteraccountsdelegate.h \
    src/qtwitter.h \
    src/twitteraccountsmodel.h \
    src/urlshortener.h \
    src/aboutdialog.h
FORMS += ui/mainwindow.ui \
    ui/authdialog.ui \
    ui/settings.ui \
    ui/tweet.ui \
    ui/aboutdialog.ui \
    ui/twitpicview.ui \
    ui/twitpicnewphoto.ui
RESOURCES += res/resources.qrc
QT += network \
    xml
TRANSLATIONS += loc/qtwitter_pl.ts \
    loc/qtwitter_ca.ts \
    loc/qtwitter_de.ts \
    loc/qtwitter_es.ts \
    loc/qtwitter_jp.ts \
    loc/qtwitter_fr.ts
UI_DIR = tmp
MOC_DIR = tmp
RCC_DIR = tmp
OBJECTS_DIR = tmp
INCLUDEPATH += $${TOP} \
    src \
    tmp
