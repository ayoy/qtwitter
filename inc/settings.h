#ifndef SETTINGS_H
#define SETTINGS_H

#include <QNetworkProxy>
#include <QTranslator>
#include <QFile>
#include <QSettings>
#include "ui_settings.h"

class Settings : public QDialog
{
  Q_OBJECT

public:
  Settings( QWidget *parent = 0 );
  ~Settings();
  inline QString stateForXML ( QCheckBox* );
  bool createConfigFile();
  bool loadConfig();
  bool saveConfig();

public slots:
  void accept();
  void changeLanguage( const QString& );

private:
  QFile configFile;
  QNetworkProxy proxy;
  Ui::Settings ui;
};

#endif //SETTINGS_H
