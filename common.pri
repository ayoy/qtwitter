isEmpty(COMMON_PRI) {
    COMMON_PRI = 1

    APP_NAME = qtwitter
    TOP = $${PWD}

    isEmpty( PREFIX ):INSTALL_PREFIX = /usr
    else:INSTALL_PREFIX = $${PREFIX}

    SHARE_DIR = $${INSTALL_PREFIX}/share/$${APP_NAME}

    DEFINES += SHARE_DIR='\\\"$${SHARE_DIR}\\\"'
    DEFINES += HAVE_OAUTH

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
}
