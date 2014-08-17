
#include "torrentdownloader.h"

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/lazy_entry.hpp"
#include "libtorrent/session.hpp"

#include <QDebug>

using namespace libtorrent;

struct TorrentDownloaderPrivate {
    session s;
    torrent_handle handle;
    size_type total_size;   
};

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args )
{
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}

TorrentDownloader::TorrentDownloader() 
{
    p = std::unique_ptr<TorrentDownloaderPrivate, std::function<void(TorrentDownloaderPrivate*)>>(new TorrentDownloaderPrivate, [](TorrentDownloaderPrivate* ptr) {
        delete ptr;
    });
    p->s.listen_on(std::make_pair(6881, 6889));
}

void TorrentDownloader::download(QString torrentFullFile, QString savePath)
{
    add_torrent_params params;
    params.save_path = savePath.toStdString();
    params.ti = new torrent_info(torrentFullFile.toStdString());
//    params.auto_managed = true;
//    params.storage_mode = storage_mode_sparse;
    p->handle = p->s.add_torrent(params);
    
    qDebug() << "Save to" << savePath;
    
    p->total_size = p->handle.get_torrent_info().total_size();
    
    connect(&progressTimer, &QTimer::timeout, [&] {
        std::vector<size_type> progressv;
        p->handle.file_progress(progressv);

        //qDebug() << QString(p->handle.status().error.c_str()) << (int)p->handle.status().state << p->handle.status().storage_mode << p->handle.status().num_seeds;
        qDebug() << progressv[0] << p->total_size;
        emit progress(100.0f * progressv[0] / p->total_size);
        if(progressv[0] == p->total_size) {
            progressTimer.stop();
            emit finished();
        }
    });

    progressTimer.start(100);
}