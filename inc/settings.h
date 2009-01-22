#ifndef SETTINGS_H
#define SETTINGS_H

#include <QNetworkProxy>
#include <QTranslator>
#include <QFile>
#include <QDir>
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
  QDir directoryOf( const QString& );
  void setProxy();

public slots:
  void accept();
  void switchLanguage( int );
signals:
  void settingsOK();

private:
  void createLanguageMenu();
  void retranslateUi();
  QTranslator translator;
  QFile configFile;
  QNetworkProxy proxy;
  Ui::Settings ui;
  friend class MainWindow;
};

#endif //SETTINGS_H
