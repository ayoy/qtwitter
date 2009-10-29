isEmpty(TRANSLATIONS_PRI) {
    TRANSLATIONS_PRI = 1

    mac: qmpath = $${DESTDIR}/$${APP_NAME}.app/Contents/Resources
    else: qmpath = $${DESTDIR}/loc

    LANGS = \
        ca_ES \
        cs_CZ \
        de_DE \
        es_ES \
        fr_FR \
        it_IT \
        ja_JP \
        nb_NO \
        pl_PL \
        pt_BR


    for(lang,LANGS): {
        tsfiles += $${PWD}/$${APP_NAME}_$${lang}.ts
        qmfiles += $${qmpath}/$${APP_NAME}_$${lang}.qm
    }
    TRANSLATIONS = $$tsfiles
}
