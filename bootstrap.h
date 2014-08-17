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
#include <QSettings>
#include <QDir>
#include <QFileDialog>

class Bootstrap : public QObject {
    Q_OBJECT
    Download download;
    Download download2;
    Download downloadServers;
    
    QFile torrentFile;
    
    TorrentDownloader& torrentDownloader;
    QProcess unpackProcess;
    QString downloadedPath;
    QString releasePath;
    QString torrentFullFile;
    QSettings& settings;
    
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
    Bootstrap(TorrentDownloader& torrentClient, QSettings& _settings, QObject* parent = nullptr) : QObject(parent), torrentDownloader(torrentClient), settings(_settings) {
        download.setTarget("https://utlauncher.rushbase.net/bootstrap.json");

        connect(&download, &Download::done, [=](QByteArray a) {
            auto json = QJsonDocument::fromJson(a).object();
            
            QString url = json.value("torrentUrl").toObject().value("windows64").toString();
            //QString url = "https://utlauncher.rushbase.net/openSUSE-13.1-KDE-Live-x86_64.iso.torrent";
            
            downloadServers.setTarget(json.value("serversUrl").toString());
            downloadServers.download();
            
#ifdef NO_DOWNLOAD
            emit ready();
            return;
#endif
            
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
            
        });

        connect(&torrentDownloader, &TorrentDownloader::progress, [&](double progress) {
            emit message(QString("Downloading latest UT build... %1").arg(QString::number(progress, 'f', 2)));
        });
        connect(&torrentDownloader, &TorrentDownloader::finished,  [=]() {
            emit message(QString("Preparing the build..."));
#ifdef __WIN32__
            auto unrar = QFileInfo(this->settings.fileName()).dir().absolutePath() + "/unrar.exe";
#else
            auto unrar = "unrar";
#endif
            
            unpackProcess.start(unrar, QStringList() << "x" << "-y" << downloadedPath << releasePath );
            connect(&unpackProcess, &QProcess::started, [=]() {
                
            });
            
            connect(&unpackProcess, SIGNAL(finished(int)), this, SLOT(unpackFinished(int)));
        });
        
        connect(&downloadServers, &Download::done, [=](QByteArray a) {
            emit serversInfo(QJsonDocument::fromJson(a));
        });
    };
    
    void start() {
        download.download();
    }
    
    QString programExePath() {
#ifdef NO_DOWNLOAD
        QString path = settings.value("UTExePath").toString();
        QFile utExe(path);
        if(utExe.exists())
            return path;
        path = QFileDialog::getOpenFileName(NULL, "Path to UnrealTournament executable", QString(),
#ifdef __WIN32__
                                            "*.exe"
#else
                                            QString()
#endif
                                           );
        settings.setValue("UTExePath", path);
        settings.sync();
        return programExePath();
#else
        return releasePath + "/WindowsNoEditor/UnrealTournament/Binaries/Win64/UnrealTournament.exe";
#endif
    }
    
signals:
    void message(QString message);
    void serversInfo(QJsonDocument document);
    void ready();
    void fatalError(ErrorReason reason);
};


#endif // BOOTSTRAP_H