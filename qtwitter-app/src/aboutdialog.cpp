/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#include "aboutdialog.h"
#include "configfile.h"

AboutDialog::AboutDialog( QWidget *parent ) :
        QDialog( parent ),
        ui( new Ui::AboutDialog )
{
    ui->setupUi( this );
    fontSize = font().pointSize();
    fontFamily = font().family();
    populateAuthors();
    populateCredits();
    displayAbout();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

QString AboutDialog::contributor( const QString &name, const QString &email, const QString &role )
{
    if ( role.isNull() )
        return QString( "<p><b>%1</b> - <a href=\"mailto:%2\">%2</a></p>" ).arg( name, email );
    return QString( "<p><b>%1</b> - <a href=\"mailto:%2\">%2</a></p>"
                    "<p>%3</p>"
                    "<p style=\"-qt-paragraph-type:empty; \"></p>" ).arg( name, email, role );
}

void AboutDialog::populateAuthors()
{
    QString authorsHtml( QString( "<html>"
                                  "<head>"
                                  "<style type=\"text/css\">p, li { white-space: pre-wrap; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; }</style>"
                                  "</head>"
                                  "<body style=\" font-family:'%1'; font-size:%2pt; font-weight:400; font-style:normal;\">" ).arg(fontFamily, QString::number(fontSize)) );

    authorsHtml.append( QString( "<p><b>%1</b></p>"
                                 "<p style=\"-qt-paragraph-type:empty; \"></p>" ).arg( tr( "Development team as for %1:" ).arg( ConfigFile::APP_VERSION ) ) );

    authorsHtml.append( contributor( "Dominik Kapusta", "d@ayoy.net", tr( "Application concept and main development" ) ) );
    authorsHtml.append( contributor( "Mariusz Pietrzyk", "wijet@wijet.pl", tr( "URL shortening library" ) ) );

    authorsHtml.append( QString( "<p>%1</p>"
                                 "<p style=\"-qt-paragraph-type:empty; \"></p>" ).arg( tr( "Other contributions:" ) ) );
    authorsHtml.append( contributor( "Stephan Beyer", "s-beyer@gmx.net" ) );
    authorsHtml.append( contributor( "Flavio Castelli", "flavio@castelli.name" ) );
    authorsHtml.append( contributor( "Thulio Costa", "thulio.costa@gmail.com" ) );
    authorsHtml.append( contributor( "Anna Nowak", "wiorka@gmail.com" ) );
    authorsHtml.append( contributor( "Daniel Reidy", "dreidy@sigterm.us" ) );

    authorsHtml.append( QString( "<br/><p><b>%1</b></p>"
                                 "<p style=\"-qt-paragraph-type:empty; \"></p>" ).arg( tr( "Translations:" ) ) );

    authorsHtml.append( contributor( "Dragon Jake", "wjake@wjake.com", tr( "Czech translation") ) );
    authorsHtml.append( contributor( "Harry Bellemare", "behr62@gmail.com", tr( "French translation") ) );
    authorsHtml.append( contributor( "Jan Schummers", "darkadmiral@onlinehome.de", tr( "German translation" ) ) );
    authorsHtml.append( contributor( "Faster", "faster3ck@gmail.com", tr( "Italian translation" ) ) );
    authorsHtml.append( contributor( QString::fromUtf8( "Maciej Dębiński" ), "misanthroposs@gmail.com", tr( "Japanese translation" ) ) );
    authorsHtml.append( contributor( "Egil Hanger", "egilkh@gmail.com", tr( "Norwegian translation" ) ) );
    authorsHtml.append( contributor( QString::fromUtf8( "Márcio Moraes" ), "marciopanto@gmail.com", tr( "Portugese translation" ) ) );
    authorsHtml.append( contributor( "Alvaro Ortiz", "fausto17@gmail.com", tr( "Spanish translation" ) ) );
    authorsHtml.append( contributor( "Dominik Kapusta", "d@ayoy.net", tr( "Polish and Catalan translation" ) ) );

    authorsHtml.append(   "</body>"
                          "</html>" );

    ui->authorsText->setHtml( authorsHtml );
    ui->authorsText->moveCursor( QTextCursor::Start );
}

void AboutDialog::populateCredits()
{
    QString creditsHtml( QString( "<html>"
                                  "<head>"
                                  "<style type=\"text/css\">p, li { white-space: pre-wrap; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; }</style>"
                                  "</head>"
                                  "<body style=\" font-family:'%1'; font-size:%2pt; font-weight:400; font-style:normal;\">" ).arg(fontFamily, QString::number(fontSize)) );

    //: qTwitter icon design by thedesignsuperhero.com
    creditsHtml.append( "<p>" + tr( "qTwitter icon design by %1" ).arg( "<a href=\"http://thedesignsuperhero.com/2008/10/free-psds-give-away-high-resolution-twitter-bird-icons/\">thedesignsuperhero.com</span></a>" ) + "</p><p style=\"-qt-paragraph-type:empty; \"></p>" );

    //: Other icons by wefunction.com
    creditsHtml.append( "<p>" + tr( "Other icons by %1" ).arg( "<a href=\"http://wefunction.com/2008/07/function-free-icon-set/\">wefunction.com</a>" ) + "</p>" );

    creditsHtml.append( QString( "<br/><p><b>%1</b></p>"
                                 "<p style=\"-qt-paragraph-type:empty; \"></p>" ).arg( tr( "Testing:" ) ) );
    creditsHtml.append( contributor( "Harry Bellemare", "behr62@gmail.com" ) );
    creditsHtml.append( contributor( "Piotr Gackowski", "pmgpmg@wp.pl" ) );

    creditsHtml.append( QString( "<br/><p><b>%1</b></p>"
                                 "<p style=\"-qt-paragraph-type:empty; \"></p>" ).arg( tr( "Releasing:" ) ) );
    creditsHtml.append( contributor( "Eugene Pivnev", "ti.eugene@gmail.com", "Mandriva, openSuse" ) );
    creditsHtml.append( contributor( "Nick B", "shirakawasuna@gmail.com", "Arch" ) );
    creditsHtml.append( contributor( "Dominik Kapusta", "d@ayoy.net", "Gentoo, Ubuntu/Debian, Fedora, MacOS X, Windows" ) );

    creditsHtml.append(   "</body>"
                          "</html>" );

    ui->creditsText->setHtml( creditsHtml );
    ui->creditsText->moveCursor( QTextCursor::Start );
}

void AboutDialog::displayAbout()
{
    QString html( QString(  "<html>"
                            "<head>"
                            "<style type=\"text/css\">p, li { white-space: pre-wrap; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; }</style>"
                            "</head>"
                            "<body align=\"center\" style=\" font-family:'%1'; font-size:%2pt; font-weight:400; font-style:normal;\">" ).arg(fontFamily, QString::number(fontSize)) );

    html.append( tr( "<p>qTwitter - a Qt-based microblogging client</p>"
                     "<p>version %1</p>"
                     "<p>Copyright &copy; 2008-2009</p>"
                     "<p>by <a href=\"http://twitter.com/ayoy\"><span style=\" text-decoration: underline; color:#0000ff;\">Dominik Kapusta</span></a></p>"
                     "<p style=\"-qt-paragraph-type:empty; \"></p>"
                     "<p>Distributed under the LGPL license</p>"
                     "<p>version 2.1 or later</p>" ).arg( ConfigFile::APP_VERSION ) );

    html.append(   "</body>"
                   "</html>" );

    ui->aboutText->setHtml( html );
}
