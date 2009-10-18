include(../../common.pri)

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../qtwitter-app/src
HEADERS = autotagplugin.h \
    autotagwidget.h
SOURCES = autotagplugin.cpp \
    autotagwidget.cpp
FORMS = autotagwidget.ui
TARGET = $$qtLibraryTarget(AutoTag)
DESTDIR = $${DESTDIR}/plugins

# install
isEmpty( PREFIX ):INSTALL_PREFIX = /usr
else:INSTALL_PREFIX = $${PREFIX}
target.path = $${INSTALL_PREFIX}/lib$${LIB_SUFFIX}/$${APP_NAME}/plugins

INSTALLS += target
