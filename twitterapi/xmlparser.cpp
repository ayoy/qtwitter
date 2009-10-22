/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Anna Nowak           <wiorka@gmail.com>         *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#include <QRegExp>
#include <QDebug>
#include "xmlparser.h"

const QString XmlParser::TAG_STATUS = "status";
const QString XmlParser::TAG_USER = "user";
const QString XmlParser::TAG_STATUS_ID = "id";
const QString XmlParser::TAG_USER_TEXT = "text";
const QString XmlParser::TAG_USER_ID = "id";
const QString XmlParser::TAG_USER_NAME = "name";
const QString XmlParser::TAG_USER_SCREENNAME = "screen_name";
const QString XmlParser::TAG_USER_IMAGE = "profile_image_url";
const QString XmlParser::TAG_USER_HOMEPAGE = "url";
const QString XmlParser::TAG_USER_TIMESTAMP = "created_at";
const QString XmlParser::TAG_INREPLYTO_STATUS_ID = "in_reply_to_status_id";
const QString XmlParser::TAG_INREPLYTO_SCREEN_NAME = "in_reply_to_screen_name";
const QString XmlParser::TAG_FAVORITED = "favorited";
const QString XmlParser::TAG_LOCATION = "location";
const QString XmlParser::TAG_DESCRIPTION = "description";
const QString XmlParser::TAG_FOLLOWERS_COUNT = "followers_count";
const QString XmlParser::TAG_FRIENDS_COUNT = "friends_count";
const QString XmlParser::TAG_PROFILE_TIMESTAMP = "created_at";
const QString XmlParser::TAG_UTC_OFFSET = "utc_offset";
const QString XmlParser::TAG_STATUS_COUNT = "statuses_count";

const QString XmlParserDirectMsg::TAG_DIRECT_MESSAGE = "direct_message";
const QString XmlParserDirectMsg::TAG_SENDER = "sender";

const QSet<QString> XmlParser::tags = QSet<QString>() << TAG_STATUS_ID << TAG_USER_TEXT << TAG_USER_NAME << TAG_USER_SCREENNAME
                                      << TAG_USER_IMAGE << TAG_USER_HOMEPAGE << TAG_USER_TIMESTAMP << TAG_INREPLYTO_STATUS_ID
                                      << TAG_INREPLYTO_SCREEN_NAME << TAG_USER_ID << TAG_LOCATION << TAG_DESCRIPTION
                                      << TAG_FOLLOWERS_COUNT << TAG_FRIENDS_COUNT << TAG_STATUS_COUNT << TAG_FAVORITED
                                      << TAG_UTC_OFFSET;

const int XmlParser::timeShift = XmlParser::calculateTimeShift();

XmlParser::XmlParser( const QString &serviceUrl, const QString &login, QObject *parent) :
        QObject( parent ),
        QXmlDefaultHandler(),
        currentTag( QString() ),
        entry(),
        important( false ),
        parsingUser( false )
{
    m_serviceUrl = serviceUrl;
    m_login = login;
}

XmlParser::XmlParser( const QString &serviceUrl, const QString &login, Entry::Type entryType, QObject *parent) :
        QObject( parent ),
        QXmlDefaultHandler(),
        currentTag( QString() ),
        entry( entryType ),
        important( false ),
        parsingUser( false )
{
    m_serviceUrl = serviceUrl;
    m_login = login;
}

QString XmlParser::login() const
{
    return m_login;
}

void XmlParser::setLogin( const QString &login )
{
    m_login = login;
}

QString XmlParser::serviceUrl() const
{
    return m_serviceUrl;
}

void XmlParser::setServiceUrl( const QString &serviceUrl )
{
    m_serviceUrl = serviceUrl;
}

bool XmlParser::startDocument()
{
    data.clear();
    return true;
}

bool XmlParser::endDocument()
{
    emit parsed(data);
    return true;
}

bool XmlParser::startElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName, const QXmlAttributes & /*atts*/ )
{
    if ( qName == TAG_STATUS ) {
        entry.initialize();
        favoritedSet = false;
    }
    if( qName == TAG_USER ) {
        parsingUser = true;
    }
    important = tags.contains( qName );
    if ( important )
        currentTag = qName;
    return true;
}

bool XmlParser::endElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName )
{
    if ( qName == TAG_STATUS ) {
        data << entry;
    }
    if( qName == TAG_USER) {
        parsingUser = false;
    }
    return true;
}

bool XmlParser::characters( const QString &ch )
{
    if ( important ) {
        if ( parsingUser ) {
            parseUserInfo(ch);
        } else {
            if ( currentTag == TAG_STATUS_ID && entry.id == Q_UINT64_C(0) ) {
                entry.id = ch.toULongLong();
            } else if ( currentTag == TAG_USER_TEXT && entry.text.isNull() ) {
                entry.originalText = ch;
                entry.originalText.replace( "&lt;", "<" );
                entry.originalText.replace( "&gt;", ">" );
                entry.text = textToHtml( entry.originalText );
            } else if ( currentTag == TAG_USER_TIMESTAMP && entry.timestamp.isNull() ) {
                entry.timestamp = toDateTime( ch ); //utc
                /* It's better to leave UTC timestamp alone; Additional member localTime is added to store local time when
         user's system supports timezones. */
                entry.localTime = entry.timestamp.addSecs( timeShift ); //now - utc
            } else if ( currentTag == TAG_INREPLYTO_STATUS_ID && entry.inReplyToStatusId == 0) {
                if( !ch.trimmed().isEmpty() ) {
                    /* In reply to status id exists and is not empty; Hack for dealing with tags that are opened and closed
           at the same time, e.g. <in_reply_to_screen_name/>  */
                    entry.hasInReplyToStatusId = true;
                    entry.inReplyToStatusId = ch.toULongLong();
                }
            } else if ( currentTag == TAG_INREPLYTO_SCREEN_NAME && entry.hasInReplyToStatusId ) {
                /* When hasInReplyToStatusId is true, inReplyToScreenName should be present, but it won't hurt to check it again
         just in case */
                if( !ch.trimmed().isEmpty() ) {
                    entry.inReplyToScreenName = ch;
                }
            } else if ( currentTag == TAG_FAVORITED && !favoritedSet ) {
                if ( ch.compare("false") == 0 )
                    entry.favorited = false;
                else
                    entry.favorited = true;
                favoritedSet = true;
            }
        }
    }

    return true;
}


void XmlParser::parseUserInfo(const QString &ch)
{

    if ( currentTag == TAG_USER_ID && parsingUser && entry.userInfo.id == -1 ) {
        entry.userInfo.id = ch.toInt();
    } else if ( currentTag == TAG_USER_NAME && entry.userInfo.name.isNull() ) {
        entry.userInfo.name = ch;
    } else if ( currentTag == TAG_USER_SCREENNAME && entry.userInfo.screenName.isNull() ) {
        entry.userInfo.screenName = ch;
        if ( entry.userInfo.screenName == m_login )
            entry.isOwn = true;
    } else if ( currentTag == TAG_USER_HOMEPAGE ) {
        if ( !ch.trimmed().isEmpty() ) {
            entry.userInfo.hasHomepage = true;
            entry.userInfo.homepage = ch;
            if ( entry.userInfo.homepage.endsWith( '/' ) )
                entry.userInfo.homepage.chop(1);
        }
    } else if ( currentTag == TAG_USER_IMAGE && entry.userInfo.imageUrl.isNull() ) {
        entry.userInfo.imageUrl = ch;
    } else if ( currentTag == TAG_LOCATION && entry.userInfo.location.isNull() ) {
        if( !ch.trimmed().isEmpty() ) {
            entry.userInfo.location = ch;
        }
    } else if ( currentTag == TAG_DESCRIPTION && entry.userInfo.description.isNull() ) {
        if( !ch.trimmed().isEmpty() ) {
            entry.userInfo.description = ch;
        }
    } else if ( currentTag == TAG_FRIENDS_COUNT && entry.userInfo.friendsCount == -1 ) {
        entry.userInfo.friendsCount = ch.toInt();
    } else if ( currentTag == TAG_FOLLOWERS_COUNT && entry.userInfo.followersCount == -1 ) {
        entry.userInfo.followersCount = ch.toInt();
    } else if ( currentTag == TAG_STATUS_COUNT && entry.userInfo.statusesCount == -1 ) {
        entry.userInfo.statusesCount = ch.toInt();
    } else if ( currentTag == TAG_UTC_OFFSET && entry.userInfo.utcOffset == -1 ) {
        entry.userInfo.utcOffset = ch.toInt();
    }
}

QDateTime XmlParser::toDateTime( const QString &timestamp )
{
    QRegExp rx( "(\\w+) (\\w+) (\\d{2}) (\\d{1,2}):(\\d{2}):(\\d{2}) .+ (\\d{4})" );
    rx.indexIn( timestamp );
    return QDateTime( QDate( rx.cap(7).toInt(), getMonth( rx.cap(2) ), rx.cap(3).toInt() ),
                      QTime( rx.cap(4).toInt(), rx.cap(5).toInt(), rx.cap(6).toInt() ) );
}

int XmlParser::getMonth( const QString &month )
{
    if ( month == "Jan" )
        return 1;
    if ( month == "Feb" )
        return 2;
    if ( month == "Mar" )
        return 3;
    if ( month == "Apr" )
        return 4;
    if ( month == "May" )
        return 5;
    if ( month == "Jun" )
        return 6;
    if ( month == "Jul" )
        return 7;
    if ( month == "Aug" )
        return 8;
    if ( month == "Sep" )
        return 9;
    if ( month == "Oct" )
        return 10;
    if ( month == "Nov" )
        return 11;
    if ( month == "Dec" )
        return 12;
    else
        return -1;
}

int XmlParser::calculateTimeShift()
{
    QString currentTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    QDateTime now = QDateTime::fromString(currentTime, Qt::ISODate);
    QString currentUtcTime = QDateTime::currentDateTime().toUTC().toString(Qt::ISODate);
    QDateTime utc = QDateTime::fromString(currentUtcTime, Qt::ISODate);
    return utc.secsTo(now);
}

QString XmlParser::textToHtml( QString newText )
{
    // URL_IDENTICA = http://identi.ca/api
    QString networkUrl = m_serviceUrl.replace( QRegExp( "/api$" ), "" );
    newText.replace( "<", "&lt;" );
    newText.replace( ">", "&gt;" );

    // recognize web/ftp links
    QRegExp ahref( "((https?|ftp)://[^ ]+)( ?)", Qt::CaseInsensitive );
    newText.replace( ahref, "<a href='\\1'>\\1</a>\\3" );

    // recognize @mentions (letters, numbers and _ are allowed in nicks)
    newText.replace( QRegExp( "(^| |[^a-zA-Z0-9])@([\\w\\d_]+)" ),
                     QString( "\\1<a href='%1/\\2'>@\\2</a>").arg( networkUrl ) );

    // recognize e-mail addresses
    QRegExp mailto( "([a-z0-9\\._%-]+@[a-z0-9\\.-]+\\.[a-z]{2,4})", Qt::CaseInsensitive );
    newText.replace( mailto, "<a href='mailto:\\1'>\\1</a>" );

    // recognize #hashtags
    QRegExp tag( "#([\\w\\d-]+)( ?)", Qt::CaseInsensitive );
    newText.replace( tag, m_serviceUrl == TwitterAPI::URL_TWITTER ?
                     "<a href='http://search.twitter.com/search?q=\\1'>#\\1</a>\\2" :
                     QString( "<a href='%1/tag/\\1'>#\\1</a>\\2" ).arg(networkUrl) );

    // recognize !groups
    if ( m_serviceUrl != TwitterAPI::URL_TWITTER ) {
        QRegExp group( "!([\\w\\d-]+)( ?)", Qt::CaseInsensitive );
        newText.replace( group, QString( "<a href='%1/group/\\1'>!\\1</a>\\2" ).arg(networkUrl) );
    }
    return newText;
}


XmlParserDirectMsg::XmlParserDirectMsg( const QString &serviceUrl, const QString &login, QObject *parent ) :
        XmlParser( serviceUrl, login, Entry::DirectMessage, parent ),
        parsingSender( false )
{}

bool XmlParserDirectMsg::startElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName, const QXmlAttributes & /*atts*/ )
{
    if ( qName == TAG_DIRECT_MESSAGE ) {
        entry.initialize();
    }
    if ( qName == TAG_SENDER ) {
        parsingSender = true;
    }
    important = tags.contains( qName );
    if ( important )
        currentTag = qName;
    return true;
}

bool XmlParserDirectMsg::endElement( const QString & /* namespaceURI */, const QString & /* localName */, const QString &qName )
{
    if ( qName == TAG_DIRECT_MESSAGE ) {
        data << entry;
    }
    if ( qName == TAG_SENDER ) {
        parsingSender = false;
    }
    return true;
}

bool XmlParserDirectMsg::characters( const QString &ch )
{
    if ( important ) {
        if (parsingSender)
            parseUserInfo(ch);
        else {
            if ( currentTag == TAG_STATUS_ID && entry.id == Q_UINT64_C(0) ) {
                entry.id = ch.toULongLong();
            } else if ( currentTag == TAG_USER_TEXT && entry.text.isNull() ) {
                entry.originalText = ch;
                entry.text = textToHtml( ch );
            } else if ( currentTag == TAG_USER_TIMESTAMP && entry.timestamp.isNull() ) {
                entry.timestamp = toDateTime( ch );
                entry.localTime = entry.timestamp.addSecs( timeShift );
            }
        }
    }
    return true;
}

/*! \class XmlParser
    \brief A class for parsing XML data from Twitter REST API.

    This class inherits QXmlDefaultHandler and reimplements its methods to comply
    with the structure of XML documents provided by Twitter REST API.
*/

/*! \enum XmlParser::FieldType
    Determines the currently parsed field type.
*/

/*! \var XmlParser::FieldType XmlParser::None
    Either unsupported or unknown type or parser error.
*/

/*! \var XmlParser::FieldType XmlParser::Id
    The status id.
*/

/*! \var XmlParser::FieldType XmlParser::Text
    The text (status message).
*/

/*! \var XmlParser::FieldType XmlParser::Name
    The owner's screen name.
*/

/*! \var XmlParser::FieldType XmlParser::Login
    The owner's login.
*/

/*! \var XmlParser::FieldType XmlParser::Image
    The owner's profile image URL.
*/

/*! \var XmlParser::FieldType XmlParser::Homepage
    The owner's homepage.
*/

/*! \var XmlParser::FieldType XmlParser::Timestamp
    The status timestamp.
*/

/*! \fn XmlParser::XmlParser( QObject *parent = 0 )
    Creates a new XML data parser with a given \a parent.
    \param parent The object's parent.
*/

/*! \fn XmlParser::XmlParser( Entry::Type entryType = Entry::Status, QObject *parent = 0 )
    Creates a new XML data parser with a given \a parent and the entry type
    to be parsed.
    \param entryType Entry type to be parsed by the parser.
    \param parent The object's parent.
*/

/*! \fn virtual bool XmlParser::startDocument()
    Parser activities at the beginning of XML document.
*/

/*! \fn virtual bool XmlParser::endDocument()
    Parser activities at the end of XML document.
*/

/*! \fn virtual bool XmlParser::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts )
    Parser activities at start of the XML element.
*/

/*! \fn virtual bool XmlParser::endElement( const QString &namespaceURI, const QString &localName, const QString &qName )
    Parser activities at end of the XML element.
*/

/*! \fn virtual bool XmlParser::characters( const QString &ch )
    Parser activities when reading XML element's text. The actual values are
    being read here.
*/

/*! \fn void XmlParser::newEntry( network, Entry *entry )
    Emitted when a complete entry is read.
    \param entry A parsed entry.
*/

/*! \fn XmlParser::FieldType XmlParser::checkFieldType( const QString &element )
    Figures out what type of element is currently being parsed.
    \param element The element's text.
    \returns Currently parsed element type.
*/

/*! \fn QDateTime XmlParser::toDateTime( const QString &timestamp )
    Converts a \a timestamp string to a QDateTime object.
    \param timestamp Parsed timestamp string.
    \returns QDateTime-compatible entry timestamp.
*/

/*! \fn int XmlParser::getMonth( const QString &month )
    Converts a short month name to a corresponding number.
    \param month A string containing parsed month.
    \returns A month number.
*/

/*! \fn QString XmlParser::textToHtml( QString newText )
    Reads \a newText and wraps up encountered URL links in \<a href="..."\> HTML tags.
    \param newText Text to be parsed.
*/

/*! \var int XmlParser::currentField
    Holds the currently processed field type.
*/

/*! \var Entry XmlParser::entry
    Holds the parsed entry data.
*/

/*! \fn bool XmlParser::important
    Returns true if the currently parsed element is significant for the application.
*/

/*! \var static const QByteArray XmlParser::USER_ID
    XML document tag for a status id.
*/

/*! \var static const QByteArray XmlParser::USER_TEXT
    XML document tag for a status text.
*/

/*! \var static const QByteArray XmlParser::USER_NAME
    XML document tag for user's screen name.
*/

/*! \var static const QByteArray XmlParser::USER_LOGIN
    XML document tag for user login.
*/

/*! \var static const QByteArray XmlParser::USER_PHOTO
    XML document tag for user's profile image URL.
*/

/*! \var static const QByteArray XmlParser::USER_HOMEPAGE
    XML document tag for user's homepage.
*/

/*! \var static const QByteArray XmlParser::USER_TIMESTAMP
    XML document tag for status timestamp.
*/

/*! \class XmlParserDirectMsg
    \brief A class for parsing XML data of direct messages.

    This class inherits XmlParser and reimplements its methods to comply with
    the structure of XML document for direct messages list provided by Twitter REST API.
*/

/*! \fn XmlParserDirectMsg::XmlParserDirectMsg( QObject *parent = 0 )
    Creates an XML parser for direct messages processing with a given \a parent.
    \param parent An object's parent.
*/

/*! \fn bool XmlParserDirectMsg::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts )
    Parser activities at start of the XML element.
*/

/*! \fn bool XmlParserDirectMsg::endElement( const QString &namespaceURI, const QString &localName, const QString &qName )
    Parser activities at end of the XML element.
*/

/*! \fn bool XmlParserDirectMsg::characters( const QString &ch )
    Parser activities when reading XML element's text. The actual values are being read here.
*/
