TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../qtwitter-app/src
HEADERS = autotagplugin.h \
    autotagwidget.h
SOURCES = autotagplugin.cpp \
    autotagwidget.cpp
FORMS = autotagwidget.ui
TARGET = $$qtLibraryTarget(AutoTag)
DESTDIR = ..

UI_DIR = tmp
MOC_DIR = tmp
RCC_DIR = tmp
OBJECTS_DIR = tmp

# install
isEmpty( PREFIX ):INSTALL_PREFIX = /usr
else:INSTALL_PREFIX = $${PREFIX}
target.path = $${INSTALL_PREFIX}/lib/qtwitter/plugins

INSTALLS += target
