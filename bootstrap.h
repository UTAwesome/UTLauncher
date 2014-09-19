#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <QObject>

#include "torrentdownloader.h"
#include "download.h"
#include <QFile>
#include <QProcess>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

class Bootstrap : public QObject {
    Q_OBJECT
    Download download;
    Download download2;
    Download downloadServers;
    
    QFile torrentFile;
    QString motd;
    
    //TorrentDownloader& torrentDownloader;
    QProcess unpackProcess;
    QString downloadedPath;
    QString releasePath;
    QString torrentFullFile;
    QSettings& settings;
    QSet<QString> stockMaps;
    
    
    QTimer bootstrapRedownloadTimer;
    
    enum class ErrorReason {
        UnpackingProcessError
        
    };
    
private slots:
    void unpackFinished(int code) {
        if(code == 0) {
            settings.setValue("LastInstalledTorrent", torrentFullFile);
            settings.sync();
            emit ready();
        } else {
            emit fatalError(ErrorReason::UnpackingProcessError);
        }
    }
    
    
public:  
    QString MOTD() {
        return motd;
    }
    
    bool isStockMap(QString map) const {
        return stockMaps.contains(map);
    }
    
    Bootstrap(QSettings& _settings, QObject* parent = nullptr) : QObject(parent), settings(_settings) {
        download.setTarget("https://utlauncher.rushbase.net/bootstrap.json");

        connect(&download, &Download::done, [=](QByteArray a) {
            auto json = QJsonDocument::fromJson(a).object();
            
            QString url = json.value("torrentUrl").toObject().value("windows64").toString();
            //QString url = "https://utlauncher.rushbase.net/openSUSE-13.1-KDE-Live-x86_64.iso.torrent";
            
            QRegExp rx("(\\d+)\\.(\\d+)\\.(\\d+)");
            auto latestVersion = json.value("latestVersion").toString();
            if(rx.indexIn(latestVersion) == 0) {
                auto latestMajor = rx.cap(1).toInt();
                auto latestMinor = rx.cap(2).toInt();
                auto latestPatch = rx.cap(3).toInt();
                
                int numLatest = 1000 * latestMajor + 100*latestMinor + latestPatch;
                int numCurrent = 1000 * VERSION_MAJOR + 100*VERSION_MINOR + VERSION_PATCH;
                
                if(numLatest > numCurrent) {
                    QMessageBox::information(NULL, "Newer version is available", QString("Current version: %1.%2.%3<br>Latest version: %4 %5 %6<br><br>").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH).arg(latestMajor).arg(latestMinor).arg(latestPatch) + json.value("downloadMessage").toString());
                }
            }
            
            motd = json.value("MOTD").toString();
            
            downloadServers.setTarget(json.value("serversUrl").toString());
            downloadServers.download();
            
            bootstrapRedownloadTimer.singleShot(15*60000, &download, SLOT(download()));
            stockMaps.clear();
            for(auto stockMap : json.value("stockMaps").toArray()) {
                auto mapString = stockMap.toString();
                stockMaps.insert(mapString);
            }
            
#ifdef NO_DOWNLOAD
            emit ready();
            return;
#endif
/*            
            QString filename =  QFileInfo(url).fileName();
            download2.setTarget(url);
            auto settingsDir =  QFileInfo(this->settings.fileName()).dir();
#ifdef __WIN32__
            QFile::copy(":/unrar.exe" , settingsDir.absolutePath() + "/unrar.exe");
#endif
            settingsDir.mkpath(QString("UTLauncher/Torrents"));
            settingsDir.mkpath(QString("UTLauncher/Downloads"));
            settingsDir.mkpath(QString("UTLauncher/Release"));
        
            torrentFullFile = settingsDir.absolutePath() + "/UTLauncher/Torrents/" + filename;
                        
            auto torrentDownloadDir = settingsDir.absolutePath() + "/UTLauncher/Downloads/" + filename;
            QString downloadedFile = filename;
            downloadedFile.replace(".torrent", "");
            downloadedPath = settingsDir.absolutePath() + "/UTLauncher/Downloads/" + filename + "/" + downloadedFile;

            releasePath = settingsDir.absolutePath() + "/UTLauncher/Release";
            
            if(settings.value("LastInstalledTorrent").toString() == torrentFullFile) {
                emit ready();
                return;
            }
            
            qDebug() << downloadedPath;
            emit message("Downloading torrent...");
            
            qDebug() << torrentFullFile;
            torrentFile.setFileName(torrentFullFile);
            qDebug() << torrentFile.open(QIODevice::WriteOnly);
            connect(&download2, &Download::chunk, [&](QByteArray chunk) {
                torrentFile.write(chunk);
            });
            connect(&download2, &Download::done, [=]() {
                torrentFile.close();
                
                emit message("Downloading UT release...");
                
                torrentDownloader.download(torrentFullFile, torrentDownloadDir);
            });
            download2.download();
            */
        });

//         connect(&torrentDownloader, &TorrentDownloader::progress, [&](double progress) {
//             emit message(QString("Downloading latest UT build... %1").arg(QString::number(progress, 'f', 2)));
//         });
//         connect(&torrentDownloader, &TorrentDownloader::finished,  [=]() {
//             emit message(QString("Preparing the build..."));
// #ifdef __WIN32__
//             auto unrar = QFileInfo(this->settings.fileName()).dir().absolutePath() + "/unrar.exe";
// #else
//             auto unrar = "unrar";
// #endif
//             
//             unpackProcess.start(unrar, QStringList() << "x" << "-y" << downloadedPath << releasePath );
//             connect(&unpackProcess, &QProcess::started, [=]() {
//                 
//             });
//             
//             connect(&unpackProcess, SIGNAL(finished(int)), this, SLOT(unpackFinished(int)));
//         });
        
        connect(&downloadServers, &Download::done, [=](QByteArray a) {
            emit serversInfo(QJsonDocument::fromJson(a));
        });
    };
    
    void start() {
        download.download();
    }
    
    QString programExePath() {
#ifdef NO_DOWNLOAD
        QString path = settings.value("UTExePathUE4").toString();
        if(QFile::exists(path))
            return path;
        return "";
#else
        return releasePath + "/WindowsNoEditor/UnrealTournament/Binaries/Win64/UnrealTournament.exe";
#endif
    }
    
    QString editorExePath() {
        QString path = settings.value("UE4ExePath").toString();
        if(QFile::exists(path))
            return path;
        return "";
    }

    QString projectPath() {
        QString path = settings.value("UTProjectPath").toString();
        if(QFile::exists(path))
            return path;
        return "";
    }
public slots:
    void refreshServers() {
        downloadServers.download();
    }
    
signals:
    void message(QString message);
    void serversInfo(QJsonDocument document);
    void ready();
    void fatalError(ErrorReason reason);
};


#endif // BOOTSTRAP_H