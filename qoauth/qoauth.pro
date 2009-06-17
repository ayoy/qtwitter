TARGET = qoauth

include(../qtwitter.pri)
include(qoauth.pri)
TEMPLATE = lib
QT += network
QT -= gui
CONFIG += dll \
    crypto
DEFINES += QOAUTH
macx {
    CONFIG += lib_bundle
    LIBS += -install_name \
    @executable_path/../Frameworks/$${TARGET}.framework/Versions/$${VER_MAJ}/$${TARGET}
    DESTDIR = $${TOP}/qtwitter.app/Contents/Frameworks
    FRAMEWORK_HEADERS.files = qoauth.h qoauth_global.h
    FRAMEWORK_HEADERS.path = Versions/$${VER_MAJ}/Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
}
else:unix { 
    DESTDIR = $${TOP}
    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}
    target.path = $${INSTALL_PREFIX}/lib
    lib_headers.files = qoauth.h qoauth_global.h
    lib_headers.path = $${INSTALL_PREFIX}/include/$${TARGET}
    INSTALLS += target \
        lib_headers
}
else:win32 { 
    DESTDIR = $${TOP}
    DLLDESTDIR = $${DESTDIR}
}
SOURCES += qoauth.cpp
HEADERS += qoauth_global.h \
    qoauth.h \
    qoauth_p.h
MOC_DIR = tmp
OBJECTS_DIR = tmp
INCLUDEPATH += tmp
