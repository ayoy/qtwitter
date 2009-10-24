TOP = $${PWD}
DEFINES += HAVE_OAUTH

APP_NAME = qtwitter

unix:!mac {
    UNAME = $$system(uname -m)
    contains( UNAME, x86_64 ): LIB_SUFFIX = 64
}

UI_DIR = $${TOP}/tmp
MOC_DIR = $${TOP}/tmp
RCC_DIR = $${TOP}/tmp
OBJECTS_DIR = $${TOP}/tmp
INCLUDEPATH += $${TOP} $${TOP}/tmp
DESTDIR = $${TOP}/build
