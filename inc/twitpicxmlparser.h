/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef TWITPICXMLPARSER_H
#define TWITPICXMLPARSER_H

#include <QXmlDefaultHandler>

class TwitPicXmlParser : public QObject, public QXmlDefaultHandler
{
  Q_OBJECT
public:
  TwitPicXmlParser( QObject *parent = 0 );

  /*!
    Parser activities at start of the XML element.
  */
  virtual bool startElement( const QString &namespaceURI,
                             const QString &localName,
                             const QString &qName,
                             const QXmlAttributes &atts );

  /*!
    Parser activities when reading XML element's text. The actual values are
    being read here.
  */
  virtual bool characters( const QString &ch );

signals:
  void completed( bool responseStatus, QString message, bool newStatus );

private:

  bool success;
  bool newStatus;
  bool urlIncoming;

  static const QString TAG_RESPONSE; /*!< XML document tag for a response element. */
  static const QString TAG_RESPONSE_STATUS; /*!< XML document tag for a response status for a photo upload. */
  static const QString TAG_RESPONSE_STATUS2; /*!< XML document tag for a response status for a photo upload with posting an update. */
  static const QString TAG_RESPONSE_OK; /*!< XML document tag for a request completed successfully. */
  static const QString TAG_RESPONSE_NOK; /*!< XML document tag for a failed request. */
  static const QString TAG_STATUS_ID; /*!< XML document tag for a new status id. */
  static const QString TAG_MEDIA_URL; /*!< XML document tag for a photo url. */
  static const QString TAG_ERROR; /*!< XML document tag for an error. */
  static const QString TAG_ERROR_CODE; /*!< XML document tag for an error code. */
  static const QString TAG_ERROR_MESSAGE; /*!< XML document tag for an error message. */
};

#endif // TWITPICXMLPARSER_H
