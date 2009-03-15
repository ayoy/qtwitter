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


#ifndef SETTINGS_H
#define SETTINGS_H

#include <QNetworkProxy>
#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QAuthenticator>
#include <QSettings>
#include "ui_settings.h"

class QCheckBox;
class QLineEdit;
class QPushButton;
class TweetModel;
class MainWindow;
class Core;

/*!
  \brief A struct containing customization data for themed UI elements.

  This struct provides the stylesheet data and palette colors of the customizable
  UI elements. Its elements are loaded and dynamically applied when selected from
  the settings dialog.
*/
struct ThemeElement
{
  QString styleSheet; /*!< Tweet class stylesheet. */
  QString linkColor; /*!< A color for links in status text field in Tweet class. */
  QColor listBackgroundColor; /*!< A color for MainWindow class list view background. */
  ThemeElement() :
      styleSheet( QString() ),
      linkColor( QString() ),
      listBackgroundColor( QColor() )
  {}

  /*!
    Creates the new instance of a struct and initializes its fields according to given parameters.
  */
  ThemeElement( const QString &newStyleSheet, const QString &newLinkColor, const QColor &newListBackgroundColor ) :
      styleSheet( newStyleSheet ),
      linkColor( newLinkColor ),
      listBackgroundColor( newListBackgroundColor )
  {}

  /*!
    A copy constructor.
  */
  ThemeElement( const ThemeElement &other ) :
      styleSheet( other.styleSheet ),
      linkColor( other.linkColor ),
      listBackgroundColor( other.listBackgroundColor )
  {}
};

/*!
  \brief A struct holding complete theme settings.

  This struct consists of three ThemeElement members, for three states of
  a Tweet (Unread, Active and Read).
*/
struct ThemeData
{
  ThemeElement unread; /*!< Theme settings for the Unread state. */
  ThemeElement active; /*!< Theme settings for the Active state. */
  ThemeElement read; /*!< Theme settings for the Read state. */
  ThemeData() :
      unread(),
      active(),
      read()
  {}

  /*!
    Creates the new instance of a struct and initializes its fields according to given parameters.
  */
  ThemeData( const ThemeElement &_unread, const ThemeElement &_active, const ThemeElement &_read ) :
      unread( _unread ),
      active( _active ),
      read( _read )
  {}
};

typedef QPair<QString,ThemeData> ThemeInfo; /*!< Type consisting of a pair of theme name stored as a string and theme data stored as a ThemeData struct. */

/*!
  \brief A class defining the settings dialog and data.

  This class is a centralized repository of the application's settings.
  It contains the UI elements of a settings dialog, as well as the themes' data
  for customizing applicatoin's UI.
*/
class Settings : public QDialog
{
  Q_OBJECT

public:

  static const ThemeInfo STYLESHEET_CARAMEL; /*!< Caramel theme info. */
  static const ThemeInfo STYLESHEET_COCOA; /*!< Cocoa theme info. */
  static const ThemeInfo STYLESHEET_GRAY; /*!< Gray theme info. */
  static const ThemeInfo STYLESHEET_GREEN; /*!< Green theme info. */
  static const ThemeInfo STYLESHEET_PURPLE; /*!< Purple theme info. */
  static const ThemeInfo STYLESHEET_SKY; /*!< Sky theme info. */

  /*!
    A default constructor. Creates the Settings class instance with the
    given \a parent and sets its values according to given parameters.
    \param tweetModel A pointer to TweetModel class instance, for applying its settings.
    \param mainwinSettings A pointer to MainWindow class instance, for applying its settings.
    \param coreSettings A pointer to Core class instance, for applying its settings.
    \param parent A class instance's parent.
  */
  Settings( TweetModel *tweetModel, MainWindow *mainwinSettings, Core *coreSettings, QWidget *parent = 0 );

  /*!
    A destructor.
  */
  ~Settings();

  /*!
    Loads the configuration from a settings file at application startup and whenever
    the settings dialog is rejected. Skips restoring dialog's and main window's size
    and position when \a dialogRejected is false.
    \param dialogRejected Indicates wether the method is invoked because of the
                          settings dialog being rejected or from other reason.
    \sa saveConfig(), reject()
  */
  void loadConfig( bool dialogRejected = false );

  /*!
    Sets application proxy according to dialog fields edited by User.
  */
  void setProxy();

public slots:
  /*!
    Saves the configuration to a settings file. The saving is done when application
    quits and whenever the settings dialog is accepted or Apply button is pressed.
    Calls applySettings() when application isn't quitting at the moment.
    \param quitting When value is other than 0, method assumes that application
           is now quitting and doesn't apply the saved configuration. This avoids
           popping up a login window when quitting in some situations.
    \sa loadConfig(), accept()
  */
  void saveConfig( int quitting = 0 );

  /*!
    Method reimplemented to always switch to the first settings' tab.
  */
  void show();

  /*!
    Method reimplemented to save config upon accepting a dialog.
    \sa reject()
  */
  void accept();

  /*!
    Method reimplemented to load saved config upon rejecting a dialog.
    \sa accept()
  */
  void reject();

  /*!
    Switches application's UI language to specified by \a index. Invokes
    retranslateUi() for all the widgets, including self.
    \param index Indicates a requested language by a language combo box's current index.
  */
  void switchLanguage( int index );

  /*!
    Applies timeline setting change made from a login window to a settings file.
  */
  void switchToPublic();

  /*!
    Applies authentication data change made from a login window to a settings dialog.
    \param authData Authentication data to be saved.
  */
  void setAuthDataInDialog( const QAuthenticator &authData );

private slots:
  void changeTheme( const QString& );
  void retranslateUi();
#ifdef Q_WS_X11
  void setBrowser();
#endif

private:
  void applySettings();
  void createLanguageMenu();
  QTranslator translator;
  QFile configFile;
  QNetworkProxy proxy;
  TweetModel *model;
  MainWindow *mainWindow;
  Core *core;
  QMap<QString,ThemeData> themes;
  Ui::Settings ui;
#ifdef Q_WS_X11
  QCheckBox *useCustomBrowserCheckBox;
  QLineEdit *selectBrowserEdit;
  QPushButton *selectBrowserButton;
#endif
};

#endif //SETTINGS_H
