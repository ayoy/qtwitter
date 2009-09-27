unix:!mac {
    SOURCES += $${PWD}/qtwitterapp_adaptor.cpp \
        $${PWD}/knotification_interface.cpp
    HEADERS += $${PWD}/qtwitterapp_adaptor.h \
        $${PWD}/knotification_interface.h
    INCLUDEPATH += $${PWD}
    DEPENDPATH += $${PWD}
    QT += dbus
}
