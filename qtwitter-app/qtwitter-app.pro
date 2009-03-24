TEMPLATE = app
TARGET = qtwitter
TOP = ..
DESTDIR = $${TOP}
include($${TOP}/twitterapi/twitterapi.pri)
macx {
    ICON = macx/twitter_48.icns
    QMAKE_INFO_PLIST = macx/Info.plist
    QMAKE_LFLAGS += -F$${TOP}/$${TARGET}.app/Contents/Frameworks/
    LIBS += -framework twitterapi
} else:unix {
    LIBS += -L$${TOP}/lib $$TWITTERAPI_LIB -Wl,-rpath,$${PWD}/$${TOP}/lib
    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}
    target.path = $${INSTALL_PREFIX}/bin
    INSTALLS += target
} else:win32 {
    LIBS += -L$${TOP}/lib $$TWITTERAPI_LIB -Wl,-rpath,$${PWD}/$${TOP}/lib
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
    src/twitpicxmlparser.cpp
HEADERS += src/mainwindow.h \
    src/statusfilter.h \
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
    src/statustextedit.h
FORMS += ui/mainwindow.ui \
    ui/authdialog.ui \
    ui/settings.ui \
    ui/tweet.ui \
    ui/about.ui \
    ui/twitpicview.ui \
    ui/twitpicnewphoto.ui
RESOURCES += res/resources.qrc
QT += network \
    xml
TRANSLATIONS += loc/qtwitter_pl.ts \
    loc/qtwitter_ca.ts \
    loc/qtwitter_es.ts \
    loc/qtwitter_jp.ts
UI_DIR = tmp
MOC_DIR = tmp
RCC_DIR = tmp
OBJECTS_DIR = tmp
INCLUDEPATH += src \
    tmp
