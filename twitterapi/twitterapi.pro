QT += network \
    xml
QT -= gui
TARGET = twitterapi
TEMPLATE = lib
include(twitterapi.pri)
macx {
    CONFIG += lib_bundle
    VER_MAJ = 0
    VER_MIN = 5
    VER_PAT = 1
    VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
    LIBS += -install_name @executable_path/../Frameworks/$${TARGET}.framework/Versions/$${VER_MAJ}/$${TARGET}
    DESTDIR = ../qtwitter.app/Contents/Frameworks
    FRAMEWORK_HEADERS.files = twitterapi.h entry.h
    FRAMEWORK_HEADERS.path = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
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
