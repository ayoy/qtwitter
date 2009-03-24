VER_MAJ = 0
VER_MIN = 5
VER_PAT = 1
VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}

win32: TWITTERAPI_LIB = -ltwitterapi$${VER_MAJ}
else:  TWITTERAPI_LIB = -ltwitterapi

INCLUDEPATH += $$TOP/twitterapi
DEPENDPATH  += $$TOP/twitterapi
