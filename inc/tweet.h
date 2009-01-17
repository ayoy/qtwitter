#ifndef TWEET_H
#define TWEET_H

#include <QtGui/QWidget>

namespace Ui {
    class Tweet;
}

class Tweet : public QWidget {
  Q_OBJECT
  Q_DISABLE_COPY( Tweet )
public:
  explicit Tweet( QWidget *parent = 0 );
  explicit Tweet( const QString &name, const QString &status, const QImage &icon, QWidget *parent = 0 );
  virtual ~Tweet();
  void resize( const QSize& );
  void resize( int w, int h );

public slots:
  void adjustSize();

protected:
  virtual void changeEvent( QEvent *e );

private:
  Ui::Tweet *m_ui;
};

#endif // TWEET_H
