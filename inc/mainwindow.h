/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "tweetmodel.h"

#include <QSystemTrayIcon>
#include <QModelIndex>

class TweetModel;

/*!
  \brief A class defining the main window of the application.

  This class contains all the GUI elements of the main application window.
  It receives signals from Core and TweetModel classes and provides means
  of visualization for them.
*/
class MainWindow : public QWidget
{
  Q_OBJECT

  static const QString APP_VERSION;

public:
  /*!
    A default constructor. Creates a MainWindow instance with the given \a parent.
  */
  MainWindow( QWidget *parent = 0 );
  /*!
    A default destructor.
  */
  ~MainWindow();

  /*!
    A method for external access to the list view used for displaying Tweets.
    Used for initialization of TweetModel class's instance.
    \returns A pointer to the list view instance of MainWindow.
  */
  StatusList* getListView();

  /*!
    A method for accessing the list view scrollbar's width, needed for computing width
    of Tweet class instances.
    \returns List view scrollbar's width.
  */
  int getScrollBarWidth();

  /*!
    Assigns the \a model to be a list view model.
    \param model The model for the list view.
  */
  void setListViewModel( TweetModel *model );
  
public slots:
  /*!
    Sets the background color of a list view. Used when changing color theme.
    \param newColor List view new background color.
  */
  void changeListBackgroundColor( const QColor &newColor );

  /*!
    Pops up a tray icon notification message containing information about
    new Tweets and direct messages (if any). Displays total messages count and
    their authors' names for status updates and direct messages separately.
    \param statusesCount The amount of new status updates.
    \param namesForStatuses List of statuses senders' names.
    \param messagesCount The amount of new direct messages.
    \param namesForMessages List of direct messages senders' names.
  */
  void popupMessage( int statusesCount, QStringList namesForStatuses, int messagesCount, QStringList namesForMessages );

  /*!
    Pops up a dialog with an error or information for User. This is an interface
    for all the classes that notify User about any problems (e.g. Core, XmlDownload
    and ImageDownload).
    \param message An information to be shown to User.
  */
  void popupError( const QString &message );

  /*!
    Retranslates all the translatable GUI elements of the class. Used when changing
    UI language on the fly.
  */
  void retranslateUi();

  /*!
    Resets status edit field if necessary. Invoked mainly when updating timeline finishes.
  */
  void resetStatusEdit();

  /*!
    Pops up a small dialog with credits and short info on the application and its author.
  */
  void about();

signals:
  /*!
    Emitted to force timeline update, assigned to pressing the update button.
  */
  void updateTweets();

  void openTwitPicDialog();

  /*!
    Emitted to post a status update. Assigned to pressing Enter inside the status edit field.
    \param status A status to be posted.
    \param inReplyTo In case the status is a reply - optional id of the existing status to which the reply is posted.
  */
  void post( const QByteArray& status, int inReplyTo = -1 );

  /*!
    Emitted when home button pressed, asks to open a default browser.
    \param address Requested URL, defaults to a null string.
    \sa Core::openBrowser()
  */
  void openBrowser( QString address = QString() );

  /*!
    Emitted when settings button pressed, requests opening the settings dialog.
  */
  void settingsDialogRequested();

  /*!
    Works as a proxy between Tweet class instance and status edit field. Passes the request
    to initiate editing a reply.
    \param user Login of a User to whom the current User replies.
    \param inReplyTo Id of the existing status to which the reply is posted.
  */
  void addReplyString( const QString& user, int inReplyTo );

  /*!
    Works as a proxy between Tweet class instance and status edit field. Passes the request
    to initiate editing a retweet.
    \param message A retweet message
  */
  void addRetweetString( QString message );

  /*!
    Emitted when resizing a window, to inform all the Tweets about the size change.
    \param width The width after resizing.
    \param oldWidth The width before resizing.
  */
  void resizeView( int width, int oldWidth );

protected:
  /*!
    An event reimplemented in order to provide hiding instead of closing the application.
    Closing is provided only via a shortcut or tray icon menu option.
    \param e A QCloseEvent event's representation.
  */
  void closeEvent( QCloseEvent *e );

private slots:
  void iconActivated( QSystemTrayIcon::ActivationReason reason );
  void changeLabel();
  void sendStatus();
  void resetStatus();

private:
  void resizeEvent( QResizeEvent* );
  bool resetUiWhenFinished;
  QMenu *trayMenu;
  QMenu *buttonMenu;
  QAction *newtweetAction;
  QAction *newtwitpicAction;
  QAction *gototwitterAction;
  QSystemTrayIcon *trayIcon;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
