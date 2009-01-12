#ifndef SETTINGS_H
#define SETTINGS_H

#include <QNetworkProxy>
#include <QTranslator>
#include "ui_settings.h"

class Settings : public QDialog
{
  Q_OBJECT

public:
  Settings( QTranslator&, QWidget *parent = 0 );
  ~Settings();
public slots:
  void accept();
  void changeLanguage( const QString& );
private:
  QTranslator &translator;
  QNetworkProxy proxy;
  Ui::Settings ui;
};

#endif //SETTINGS_H
