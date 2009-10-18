TARGET = urlshortener

include(../common.pri)
include(urlshortener.pri)

TEMPLATE = lib
QT += network xml
QT -= gui
CONFIG += dll
DEFINES += URLSHORTENER
macx { 
    CONFIG += lib_bundle
    LIBS += -install_name \
        @executable_path/../Frameworks/$${TARGET}.framework/Versions/$${VER_MAJ}/$${TARGET}
    DESTDIR = $${DESTDIR}/$${APPNAME}.app/Contents/Frameworks
}
else:unix {
    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}
    target.path = $${INSTALL_PREFIX}/lib$${LIB_SUFFIX}
    INSTALLS += target
}
else:win32 { 
    DLLDESTDIR = $${DESTDIR}
}

SOURCES += urlshortener.cpp \
    urlshortenerimplementation.cpp
HEADERS += urlshortener_global.h \
    urlshortener.h \
    urlshortenerimplementation.h

