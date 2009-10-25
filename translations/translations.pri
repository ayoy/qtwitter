isEmpty(TRANSLATIONS_PRI) {
    TRANSLATIONS_PRI = 1

    qmpath = $${DESTDIR}/loc

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
