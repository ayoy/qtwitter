include(../plugins.pri)

QT += network xml
HEADERS += urlshortenerplugin.h \
    urlshortener.h \
    urlshortenerimplementation.h \
    urlshortenerwidget.h
SOURCES += urlshortenerplugin.cpp \
    urlshortener.cpp \
    urlshortenerimplementation.cpp \
    urlshortenerwidget.cpp
FORMS = urlshortenerwidget.ui

TARGET = UrlShortener
