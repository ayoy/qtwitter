TARGET = twitterapi

include(../common.pri)
include(twitterapi.pri)

contains( DEFINES, HAVE_OAUTH ) {
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
    # TODO: probably not needed
    QMAKE_LFLAGS += -F$${TOP}/$${APP_NAME}.app/Contents/Frameworks
    LIBS += -install_name \
        @executable_path/../Frameworks/$${TARGET}.framework/Versions/$${VER_MAJ}/$${TARGET}

    DESTDIR = $${DESTDIR}/$${APP_NAME}.app/Contents/Frameworks
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
SOURCES += xmlparser.cpp \
    twitterapi.cpp \
    entry.cpp \
    userinfo.cpp
HEADERS += twitterapi_global.h \
    xmlparser.h \
    twitterapi.h \
    twitterapi_p.h \
    entry.h \
    userinfo.h
