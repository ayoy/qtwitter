VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0
VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}

win32: URLSHORTENER_LIB = -lurlshortener$${VER_MAJ}
else:  URLSHORTENER_LIB = -lurlshortener

INCLUDEPATH += $${TOP}/urlshortener
DEPENDPATH += $${TOP}/urlshortener
