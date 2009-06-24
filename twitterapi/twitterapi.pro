TARGET = twitterapi

include(../qtwitter.pri)
include(twitterapi.pri)

contains( DEFINES, OAUTH ) {
    CONFIG += oauth
}

TEMPLATE = lib
QT += network \
    xml
QT -= gui
CONFIG += dll
DEFINES += TWITTERAPI
macx { 
    CONFIG += lib_bundle
    QMAKE_LFLAGS += -F$${TOP}/qtwitter.app/Contents/Frameworks
    LIBS += -install_name \
        @executable_path/../Frameworks/$${TARGET}.framework/Versions/$${VER_MAJ}/$${TARGET}

    DESTDIR = $${TOP}/qtwitter.app/Contents/Frameworks
    FRAMEWORK_HEADERS.files = twitterapi.h
    FRAMEWORK_HEADERS.path = Versions/$${VER_MAJ}/Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
}
else:unix { 
    DESTDIR = $${TOP}

    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}
    target.path = $${INSTALL_PREFIX}/lib
    lib_headers.files = twitterapi.h
    lib_headers.path = $${INSTALL_PREFIX}/include/$${TARGET}
    INSTALLS += target \
        lib_headers
}
else:win32 { 
    DESTDIR = $${TOP}
    DLLDESTDIR = $${DESTDIR}
}
SOURCES += xmlparser.cpp \
    twitterapi.cpp \
    entry.cpp \
    userinfo.cpp
HEADERS += twitterapi_global.h \
    xmlparser.h \
    twitterapi.h \
    entry.h \
    userinfo.h
MOC_DIR = tmp
OBJECTS_DIR = tmp
INCLUDEPATH += $${TOP} tmp
