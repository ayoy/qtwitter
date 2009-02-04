#ifndef SETTINGS_H
#define SETTINGS_H

#include <QNetworkProxy>
#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QAuthenticator>
#include <QSettings>
#include "ui_settings.h"

class MainWindow;
class LoopedSignal;
class Core;

class Settings : public QDialog
{
  Q_OBJECT

public:
  Settings( MainWindow *mainwinSettings, LoopedSignal *loopSettings, Core *coreSettings, QWidget *parent = 0 );
  ~Settings();
  bool createConfigFile();
  void loadConfig( bool dialogRejected = false );
  void saveConfig();
  QDir directoryOf( const QString& );
  void setProxy();

public slots:
  void accept();
  void reject();
  void switchLanguage( int );
  void setAuthDataInDialog( const QAuthenticator& );
signals:
  void settingsOK();

private:
  void applySettings();
  void createLanguageMenu();
  void retranslateUi();
  QTranslator translator;
  QFile configFile;
  QNetworkProxy proxy;
  Ui::Settings ui;
//  friend class MainWindow;
  MainWindow *mainWindow;
  LoopedSignal *loopedSignal;
  Core *core;
};

#endif //SETTINGS_H
