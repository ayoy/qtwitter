TEMPLATE = app
TARGET = qtwitter
unix { 
    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}
    target.path = $${INSTALL_PREFIX}/bin
    INSTALLS += target
}
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/statusedit.cpp \
    src/xmlparser.cpp \
    src/entry.cpp \
    src/httpconnection.cpp \
    src/xmldownload.cpp \
    src/imagedownload.cpp \
    src/core.cpp \
    src/settings.cpp \
    src/tweet.cpp \
    src/xmlparserdirectmsg.cpp \
    src/tweetmodel.cpp
HEADERS += inc/mainwindow.h \
    inc/statusfilter.h \
    inc/statusedit.h \
    inc/xmlparser.h \
    inc/entry.h \
    inc/httpconnection.h \
    inc/xmldownload.h \
    inc/imagedownload.h \
    inc/core.h \
    inc/settings.h \
    inc/tweet.h \
    inc/xmlparserdirectmsg.h \
    inc/tweetmodel.h
FORMS += ui/mainwindow.ui \
    ui/authdialog.ui \
    ui/settings.ui \
    ui/tweet.ui
RESOURCES += res/resources.qrc
QT += network \
    xml

# QTPLUGIN += qjpeg \
# qgif
TRANSLATIONS += loc/qtwitter_pl.ts \
    loc/qtwitter_ca.ts \
    loc/qtwitter_es.ts
UI_DIR = tmp
MOC_DIR = tmp
RCC_DIR = tmp
OBJECTS_DIR = tmp
INCLUDEPATH = inc \
    tmp
