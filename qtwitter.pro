TEMPLATE = app
TARGET = qtwitter

SOURCES   += main.cpp mainwindow.cpp statusedit.cpp xmlparser.cpp
HEADERS   += mainwindow.h statusfilter.h statusedit.h xmlparser.h
FORMS     += mainwindow.ui
RESOURCES += resources.qrc
QT        += network xml
