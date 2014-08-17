#ifndef TORRENTDOWNLOADER_H
#define TORRENTDOWNLOADER_H

#include <QTimer>
#include <memory>

struct TorrentDownloaderPrivate;

class TorrentDownloader : public QObject
{
    Q_OBJECT
    QTimer progressTimer;
    
    std::unique_ptr<TorrentDownloaderPrivate, std::function<void(TorrentDownloaderPrivate*)>> p;
public:
    TorrentDownloader();
    
    void download(QString torrentFullFile, QString savePath);
signals:
    void progress(double percent);
    void finished();
};

#endif // TORRENTDOWNLOADER_H