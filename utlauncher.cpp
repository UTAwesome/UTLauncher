#include "utlauncher.h"
#include "serverbrowser.h"
#include "awesome.h"

#include <QStatusBar>

#include "configdialog.h"

QtAwesome* awesome;

UTLauncher::UTLauncher(int& argc, char** argv) : QApplication(argc, argv), settings(QSettings::IniFormat, QSettings::UserScope, "CodeCharm", "UTLauncher"), bootstrap(settings) {
    QPixmap pixmap(":/splash.jpg");
    splash = new UTSplash(pixmap);
    
    qDebug() << "Starting launcher";

    
    awesome = new QtAwesome(qApp);
    awesome->initFontAwesome();
    QWidget w;
    awesome->setDefaultOption( "color" , w.palette().color(w.foregroundRole()) );
    awesome->setDefaultOption( "color-active" , w.palette().color(w.foregroundRole()) );
    awesome->setDefaultOption( "color-selected" , w.palette().color(w.foregroundRole()) );
    
    
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

void UTLauncher::closeSplash()
{
    if(splash) {
        splash->deleteLater();
        splash = nullptr;
    }
}

void UTLauncher::startServerBrowser()
{
    splashTimer.singleShot(2000, this, SLOT(closeSplash()));
    browser->show();
    browser->setMOTD(bootstrap.MOTD());
    
    auto hasEditorSupport = [=] {
        QString editorPath = bootstrap.editorExePath();
        QString projectPath = bootstrap.projectPath();
        return QFile::exists(editorPath) && QFile::exists(projectPath);
    };
    
    auto openSettings = [=](bool mandatoryEditor = false) {
        ConfigDialog dialog(settings, mandatoryEditor);
        dialog.exec();
        browser->setEditorSupport(hasEditorSupport());
    };
    
    connect(browser, &ServerBrowser::openServer, [=](QString url, bool spectate, bool inEditor) {

        if(inEditor) {
            QString editorPath = bootstrap.editorExePath();
            QString projectPath = bootstrap.projectPath();
            if(!editorPath.length() || !projectPath.length()) {
                openSettings(true);
                return;
            }
            QProcess::startDetached(editorPath, QStringList() << projectPath << "-GAME" << (url + (spectate?"?SpectatorOnly=1":"")) );
        } else {
            QString exePath = bootstrap.programExePath();
            if(!exePath.length()) {
                openSettings();
                return;
            }
            QProcess::startDetached(exePath, QStringList() << (url + (spectate?"?SpectatorOnly=1":"")) );
        }
    });
    
    disconnect(&bootstrap, &Bootstrap::ready, this, &UTLauncher::prepareConfig);
    
    connect(&bootstrap, &Bootstrap::ready, this, [=] {
        browser->setMOTD(bootstrap.MOTD());
    });
    
    connect(browser, &ServerBrowser::openSettings, openSettings);
    
    browser->setEditorSupport(hasEditorSupport());
}