#include "download.h"
#include <QCoreApplication>
#include <QUrl>
#include <QNetworkRequest>
#include <QFile>
#include <QDebug>

Download::Download() : QObject(0) {
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(downloadFinished(QNetworkReply*)));
}

Download::~Download() {

}


void Download::setTarget(const QString &t) {
    this->target = t;
}

void Download::downloadFinished(QNetworkReply *data) {
    emit done(data->readAll());
}

void Download::download() {
    QUrl url = QUrl::fromEncoded(this->target.toLocal8Bit());
    QNetworkRequest request(url);
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    if(receivers(SIGNAL(chunk(QByteArray)))) {
        connect(reply, &QNetworkReply::readyRead, [=] {
            emit chunk(reply->readAll());
        });
    }
}


void Download::downloadProgress(qint64 recieved, qint64 total) {
    qDebug() << recieved << total;
}