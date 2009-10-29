include(../common.pri)
include(translations.pri)

DESTDIR = $${qmpath}

LRELEASE = $$[QT_INSTALL_BINS]/lrelease
win32: LRELEASE = $$[QT_INSTALL_BINS]\lrelease

TEMPLATE = app
TARGET = phony_target2
CONFIG -= qt
QT =
LIBS =

mac: CONFIG -= app_bundle

unix {
    system(mkdir -p $${qmpath} && touch $${qmfiles})
    translations.files = $${qmfiles}
    translations.path = $${SHARE_DIR}/loc
    INSTALLS += translations

    removeqm.input = qmfiles
    removeqm.output = .qm-removed
    removeqm.variable_out = PRE_TARGETDEPS
    removeqm.commands = rm -f ${QMAKE_FILE_IN} && touch ${QMAKE_FILE_OUT}
    removeqm.name = REMOVEQM ${QMAKE_FILE_IN}
    removeqm.CONFIG += no_link combine explicit_dependencies
    QMAKE_EXTRA_COMPILERS += removeqm
}

updateqm.input = TRANSLATIONS
updateqm.output = $${DESTDIR}/${QMAKE_FILE_BASE}.qm
updateqm.variable_out = PRE_TARGETDEPS
updateqm.commands = $${LRELEASE} ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.depends = .qm-removed
updateqm.name = LRELEASE ${QMAKE_FILE_IN}
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm

QMAKE_LINK = @: IGNORE_THIS_LINE
OBJECTS_DIR =

