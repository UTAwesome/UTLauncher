#ifndef UTLAUNCHER_H
#define UTLAUNCHER_H

#include "utsplash.h"

#include <QApplication>
#include <QFile>
#include <QProcess>
#include <QDesktopWidget>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QDesktopWidget>
#include <QStyle>
#include <QSplashScreen>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QLabel>
#include <QPainter>
#include <QTimer>

#include "download.h"
#include "torrentdownloader.h"
#include "bootstrap.h"

class ServerBrowser;

class UTLauncher: public QApplication
{
    Q_OBJECT
    QWidget a;        
    UTSplash* splash;
    ServerBrowser* browser;
    TorrentDownloader torrentDownloader;
    Bootstrap bootstrap;
    QSettings settings;
    QTimer serversRefreshTimer;
        
    void prepareConfig();
    
    void startServerBrowser();
private slots:
    void gotServersInfo(QJsonDocument document);
public:    
    UTLauncher(int& argc, char** argv);
};

#endif // UTLAUNCHER_H