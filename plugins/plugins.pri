include(../common.pri)
include($${TOP}/translations/translations.pri)

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += $${TOP}/qtwitter-app/src
mac: DESTDIR = $${DESTDIR}/$${APP_NAME}.app/Contents/plugins
else: DESTDIR = $${DESTDIR}/plugins

# install
isEmpty( PREFIX ):INSTALL_PREFIX = /usr/local
else:INSTALL_PREFIX = $${PREFIX}
target.path = $${INSTALL_PREFIX}/lib$${LIB_SUFFIX}/$${APP_NAME}/plugins
INSTALLS += target
