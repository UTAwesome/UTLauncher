#include "utlauncher.h"
#include "serverbrowser.h"

UTLauncher::UTLauncher(int& argc, char** argv) : QApplication(argc, argv), settings(QSettings::IniFormat, QSettings::UserScope, "CodeCharm", "UTLauncher"), bootstrap(torrentDownloader, settings) {
    QPixmap pixmap(":/splash.jpg");
    splash = new UTSplash(pixmap);
    
    splash->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            splash->size(),
            qApp->desktop()->availableGeometry()
        ));
    
    
    settings.setValue("FirstRun", false);
    
    settings.sync();
    
    bootstrap.start();
    connect(&bootstrap, &Bootstrap::message, [=](QString msg) {
        splash->showMessage(msg);
    });
    
    connect(&bootstrap, &Bootstrap::ready, this, &UTLauncher::prepareConfig);
    
    browser = new ServerBrowser();
    connect(&bootstrap, &Bootstrap::serversInfo, this, &UTLauncher::gotServersInfo, Qt::QueuedConnection );
    
    splash->showMessage("Getting version information...");
    splash->show();
}

void UTLauncher::gotServersInfo(QJsonDocument document)
{
    browser->loadFromJson(document.object());
    serversRefreshTimer.singleShot(5*60000, &bootstrap, SLOT(refreshServers()));
}

void UTLauncher::prepareConfig()
{
    startServerBrowser();
}

void UTLauncher::startServerBrowser()
{
    splash->deleteLater();
    browser->show();
    connect(browser, &ServerBrowser::openServer, [=](QString url, bool spectate) {
        QProcess::startDetached(bootstrap.programExePath(), QStringList() << (url + (spectate?"?SpectatorOnly=1":"")) );
    });
}