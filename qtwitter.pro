TEMPLATE = app
TARGET = qtwitter

SOURCES   += main.cpp mainwindow.cpp statusedit.cpp xmlparser.cpp httpconnection.cpp
HEADERS   += mainwindow.h statusfilter.h statusedit.h xmlparser.h entry.h httpconnection.h
FORMS     += mainwindow.ui authdialog.ui
RESOURCES += resources.qrc
QT        += network xml
