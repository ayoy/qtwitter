QT += network \
    xml
QT -= gui
TARGET = twitterapi
TEMPLATE = lib
CONFIG += dll
include(twitterapi.pri)
macx {
    CONFIG += lib_bundle
    LIBS += -install_name @executable_path/../Frameworks/$${TARGET}.framework/Versions/$${VER_MAJ}/$${TARGET}
    DESTDIR = ../qtwitter.app/Contents/Frameworks
    FRAMEWORK_HEADERS.files = twitterapi.h entry.h
    FRAMEWORK_HEADERS.path = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
} else:unix {
    DESTDIR = ../lib
    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}
    target.path = $${INSTALL_PREFIX}/lib
    INSTALLS += target
} else:win32 {
    DESTDIR = ../lib
    DLLDESTDIR = ..
}
DEFINES += TWITTERAPI_LIBRARY
SOURCES += twitterapi.cpp \
    xmlparserdirectmsg.cpp \
    xmlparser.cpp \
    xmldownload.cpp \
    httpconnection.cpp \
    entry.cpp
HEADERS += twitterapi.h \
    twitterapi_global.h \
    xmlparserdirectmsg.h \
    xmlparser.h \
    xmldownload.h \
    httpconnection.h \
    entry.h
MOC_DIR = tmp
OBJECTS_DIR = tmp
INCLUDEPATH += tmp
