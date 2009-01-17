#ifndef UPDATESLIST_H
#define UPDATESLIST_H

#include <QListView>

#include <QMouseEvent>
#include <QDebug>

class UpdatesList : public QListView {

  Q_OBJECT
public:
  UpdatesList( QWidget *parent ) : QListView( parent ) {}

signals:
  void contextMenuRequested();

private:
  void mousePressEvent( QMouseEvent *event ) {
    if ( event->button() == Qt::RightButton ) {
      qDebug() << "right click on the list";
      emit contextMenuRequested();
    }
    QListView::mousePressEvent( event );
  }
};

#endif // UPDATESLIST_H
