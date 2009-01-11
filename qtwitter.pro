TEMPLATE = app

TARGET = qtwitter

SOURCES      += src/main.cpp \
                src/mainwindow.cpp \
                src/statusedit.cpp \
                src/xmlparser.cpp \
                src/httpconnection.cpp \
                src/xmldownload.cpp \
                src/imagedownload.cpp \
                src/core.cpp \
                src/settings.cpp

HEADERS      += inc/mainwindow.h \
                inc/statusfilter.h \
                inc/statusedit.h \
                inc/xmlparser.h \
                inc/entry.h \
                inc/httpconnection.h \
                inc/xmldownload.h \
                inc/imagedownload.h \
                inc/core.h \
                inc/settings.h

FORMS        += ui/mainwindow.ui \
                ui/authdialog.ui \
                ui/settings.ui

RESOURCES    += res/resources.qrc

QT           += network xml

TRANSLATIONS += loc/qtwitter_pl.ts \
                loc/qtwitter_ca.ts \
                loc/qtwitter_es.ts
UI_DIR = tmp
MOC_DIR = tmp
RCC_DIR = tmp
OBJECTS_DIR = tmp
INCLUDEPATH = inc
