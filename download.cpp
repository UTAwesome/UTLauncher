#include "download.h"
#include <QCoreApplication>
#include <QUrl>
#include <QNetworkRequest>
#include <QFile>
#include <QDebug>
#include <QNetworkReply>

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
    
    request.sslConfiguration().setProtocol(QSsl::AnyProtocol);
    
    request.setRawHeader( "User-Agent" , QString("UTLauncher %1.%2.%3 / %4").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH).arg(
#if defined Q_OS_WINDOWS
		"Windows"
#elif defined Q_OS_LINUX
		"Linux"
#elif defined Q_OS_MAX
		"MacOSX"
#else
		"Unknown"
#endif
		).toUtf8() );
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    if(receivers(SIGNAL(chunk(QByteArray)))) {
        connect(reply, &QNetworkReply::readyRead, [=] {
            emit chunk(reply->readAll());
        });
    }
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(downloadError(QNetworkReply::NetworkError)));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(downloadSslErrors(QList<QSslError>)));
    
}


void Download::downloadSslErrors(QList<QSslError> errors) {
    for(auto error: errors) {
        qDebug() << "SSL Error" << error;
    }
}

void Download::downloadError(QNetworkReply::NetworkError error) {
    qDebug() << "Got download error" << error;
}

void Download::downloadProgress(qint64 recieved, qint64 total) {
    qDebug() << recieved << total;
}