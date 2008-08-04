TEMPLATE = app
TARGET = qtwitter

SOURCES   += main.cpp mainwindow.cpp statusedit.cpp xmlparser.cpp httpconnection.cpp xmldownload.cpp imagedownload.cpp core.cpp
HEADERS   += mainwindow.h statusfilter.h statusedit.h xmlparser.h entry.h httpconnection.h xmldownload.h imagedownload.h core.h
FORMS     += mainwindow.ui authdialog.ui settings.ui
RESOURCES += resources.qrc
QT        += network xml
