#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QObject>
#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QList>
#include <QSslError>

class Download : public QObject {
    Q_OBJECT
public:
    explicit Download();
    ~Download();

    void setTarget(const QString& t);
    
private:
    int httpCode;
    QNetworkAccessManager manager;
    QString target;

signals:
    void done(QByteArray data);
    
    void progress(double progress);
    void chunk(QByteArray chunk);
    void error(int code, QByteArray data);

public slots:
    void download();
private slots:
    void downloadError(QNetworkReply::NetworkError code);
    void downloadFinished(QNetworkReply* data);
    void downloadProgress(qint64 recieved, qint64 total);
    void downloadSslErrors(QList<QSslError> errors);
};

#endif // DOWNLOAD_H