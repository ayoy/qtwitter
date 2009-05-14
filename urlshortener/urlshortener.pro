TARGET = urlshortener

include(../qtwitter.pri)
include(urlshortener.pri)

TEMPLATE = lib
QT += network
QT -= gui
CONFIG += dll
DEFINES += URLSHORTENER
macx { 
    CONFIG += lib_bundle
    LIBS += -install_name \
        @executable_path/../Frameworks/$${TARGET}.framework/Versions/$${VER_MAJ}/$${TARGET}
    DESTDIR = $${TOP}/qtwitter.app/Contents/Frameworks
    FRAMEWORK_HEADERS.files = urlshortener.h
    FRAMEWORK_HEADERS.path = Versions/$${VER_MAJ}/Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
}
else:unix {
    DESTDIR = $${TOP}
    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}
    target.path = $${INSTALL_PREFIX}/lib
    lib_headers.files = urlshortener.h
    lib_headers.path = $${INSTALL_PREFIX}/include/$${TARGET}
    INSTALLS += target \
        lib_headers
}
else:win32 { 
    DESTDIR = $${TOP}
    DLLDESTDIR = $${DESTDIR}
}

SOURCES += urlshortener.cpp \
    urlshortenerimplementation.cpp
HEADERS += urlshortener_global.h \
    urlshortener.h \
    urlshortenerimplementation.h

MOC_DIR = tmp
OBJECTS_DIR = tmp
INCLUDEPATH += tmp
