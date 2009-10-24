/***************************************************************************
 *   Copyright (C) 2009 by Dominik Kapusta            <d@ayoy.net>         *
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


#include "urlshortenerwidget.h"
#include "ui_urlshortenerwidget.h"

#include <QStringListModel>
#include <QKeyEvent>
#include <QShortcutEvent>
#include <QKeySequence>

UrlShortenerWidget::UrlShortenerWidget( QWidget *parent ) :
        QWidget( parent ),
        m_ui( new Ui::UrlShortenerWidget ),
        shortenersModel( new QStringListModel(this) )
{
    m_ui->setupUi(this);
    m_ui->lineShortcut->installEventFilter(this);
}

UrlShortenerWidget::~UrlShortenerWidget()
{
    delete m_ui;
    m_ui = 0;
}

bool UrlShortenerWidget::isAutomatic() const
{
    return m_ui->chbAuto->isChecked();
}

void UrlShortenerWidget::setAutomatic( bool automatic )
{
    m_ui->chbAuto->setChecked( automatic );
}

void UrlShortenerWidget::setData( const QMap<QString,int> &data )
{
    m_ui->comboShorten->clear();
    foreach( const QString &name, data.keys() ) {
        m_ui->comboShorten->addItem( name, data.value(name) );
    }
}

int UrlShortenerWidget::currentIndex() const
{
    return m_ui->comboShorten->currentIndex();
}

void UrlShortenerWidget::setCurrentIndex( int index )
{
    m_ui->comboShorten->setCurrentIndex( index );
}

int UrlShortenerWidget::currentShortener() const
{
    return m_ui->comboShorten->itemData( m_ui->comboShorten->currentIndex() ).toInt();
}

QString UrlShortenerWidget::shortcut() const
{
    return QKeySequence::fromString( m_ui->lineShortcut->text(), QKeySequence::NativeText );
}

void UrlShortenerWidget::setShortcut( const QString &shortcut )
{
    m_ui->lineShortcut->setText( shortcut );
}

void UrlShortenerWidget::changeEvent( QEvent *event )
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi( this );
        break;
    default:;
    }
}

bool UrlShortenerWidget::eventFilter( QObject *watched, QEvent *event )
{
    Q_UNUSED(watched);

    if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        if ( key == Qt::Key_Shift ||
             key == Qt::Key_Control ||
             key == Qt::Key_Alt ||
             key == Qt::Key_Meta )
        {
            return true;
        }

        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        QString text = keyEvent->text();
        if ( modifiers & Qt::ShiftModifier && (text.isEmpty()
                                                || !text.at(0).isPrint()
                                                || text.at(0).isLetter()
                                                || text.at(0).isSpace()) )
            key |= Qt::SHIFT;
        if ( modifiers & Qt::ControlModifier )
            key |= Qt::CTRL;
        if ( modifiers & Qt::AltModifier )
            key |= Qt::ALT;
        if ( modifiers & Qt::MetaModifier )
            key |= Qt::META;

        QKeySequence seq( key );
        if ( m_ui->lineShortcut->text() != seq.toString( QKeySequence::NativeText ) ) {
            m_ui->lineShortcut->setText( seq.toString( QKeySequence::NativeText ) );
            emit shortcutChanged(seq);
        }

        event->accept();
        return true;
    }
    if ( event->type() == QEvent::Shortcut ||
         event->type() == QEvent::ShortcutOverride ||
         event->type() == QEvent::KeyRelease ) {
        return true;
    }
    return false;
}
