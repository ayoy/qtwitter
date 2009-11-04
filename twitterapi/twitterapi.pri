VER_MAJ = 0
VER_MIN = 10
VER_PAT = 0
VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}

win32: TWITTERAPI_LIB = -ltwitterapi$${VER_MAJ}
else:  TWITTERAPI_LIB = -ltwitterapi

INCLUDEPATH += $${PWD}
DEPENDPATH  += $${PWD}
