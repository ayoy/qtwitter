# To disable OAuth support, run
#   # qmake -r "DEFINES-=OAUTH"
# or modify qtwitter.pri

include(qtwitter.pri)
TEMPLATE = subdirs
SUBDIRS = urlshortener twitterapi qtwitter-app
CONFIG += ordered

contains( DEFINES, OAUTH ) {
    message( "Configuring with OAuth support..." )
} else {
    message( "Configuring without OAuth support..." )
}
