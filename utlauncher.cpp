#include "utlauncher.h"
#include "serverbrowser.h"
#include "awesome.h"

#include <QStatusBar>

#include "configdialog.h"

QtAwesome* awesome;

QColor UTLauncher::iconColor() const {
    if(getenv("ICON_COLOR")) {
        return QColor(getenv("ICON_COLOR"));
    } else {
        QWidget w;
        return w.palette().color(w.foregroundRole());
    }
}

UTLauncher::UTLauncher(int& argc, char** argv) :
            QApplication(argc, argv),
            settings(QSettings::IniFormat, QSettings::UserScope, "CodeCharm", "UTLauncher"),
            bootstrap(settings), systemTray(QIcon(":/icon.png")) {
    QPixmap pixmap(":/splash.jpg");
    splash = new UTSplash(pixmap);
    
    awesome = new QtAwesome(qApp);
    awesome->initFontAwesome();
    
    awesome->setDefaultOption( "color" , iconColor() );
    awesome->setDefaultOption( "color-active" , iconColor() );
    awesome->setDefaultOption( "color-selected" , iconColor() );
    
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
    
    if(!settings.value("StartMinimized", false).toBool()) {
        splash->show();
    }
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

    if(!settings.value("StartMinimized", false).toBool()) {
        browser->show();
    } else {
        qApp->setQuitOnLastWindowClosed(false); // workaround for app not starting
    }
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
        
        browser->setHideOnClose(settings.value("MinimizeToTrayOnClose").toBool());
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
    
    browser->setHideOnClose(settings.value("MinimizeToTrayOnClose", false).toBool());
    
    systemTray.setToolTip("UTLauncher");
    auto systemTrayMenu = new QMenu(browser);
    
    auto showBrowser = new QAction(awesome->icon(fa::listalt), "Server List", this);
    connect(showBrowser, &QAction::triggered, [=]() {
        browser->showNormal();
        browser->raise();
        browser->activateWindow();
    });
    
    auto runUTAction = new QAction(awesome->icon( fa::gamepad ),"Run UT", this);
    connect(runUTAction, &QAction::triggered, [=]() {
        QString exePath = bootstrap.programExePath();
        if(!exePath.length()) {
            browser->show();
            openSettings();
            return;
        }
        QProcess::startDetached(exePath);
    });

    auto runEditorAction = new QAction(awesome->icon( fa::code ),"Run Editor", this);
    connect(runEditorAction, &QAction::triggered, [=]() {
        QString editorPath = bootstrap.editorExePath();
        QString projectPath = bootstrap.projectPath();
        QProcess::startDetached(editorPath, QStringList() << projectPath);
    });
    

    auto quitAction = new QAction(awesome->icon( fa::signout ),"Quit", this);
    connect(quitAction, &QAction::triggered, [=]() {
        QApplication::quit();
    });
    
    systemTrayMenu->addAction(showBrowser);
    systemTrayMenu->addSeparator();
    systemTrayMenu->addAction(runUTAction);
    systemTrayMenu->addAction(runEditorAction);
    systemTrayMenu->addSeparator();
    systemTrayMenu->addAction(quitAction);
    
    systemTray.setContextMenu(systemTrayMenu);
    systemTray.show();
    
    connect(&systemTray, &QSystemTrayIcon::activated, [=](QSystemTrayIcon::ActivationReason reason) {
        qApp->setQuitOnLastWindowClosed(true);
        
        runEditorAction->setVisible(hasEditorSupport());
        
        switch(reason) {
            
            case QSystemTrayIcon::Trigger:
            {
                if(browser->isHidden()) {
                    browser->show();
                } else {
                    browser->hide();
                }
                break;
            }

        }
    });
    
}