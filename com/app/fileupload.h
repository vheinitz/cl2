#ifndef UPLOADER_H
#define UPLOADER_H

#include <QObject>
#include "uploadfile.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

class UpLoader : public QObject
{
    Q_OBJECT

public:
    UpLoader(QObject *parent = 0);
    ~UpLoader();
    bool aborted();

private:
    QUpFile *upf;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QString siteurl;
    bool isAborted;
    bool isInProgress;

signals:
    void started();
    void progress(qint64 bytesSent, qint64 bytesTotal);
    void authFinished(bool success, const QString &errortext);
    void finished(bool error, bool aborted, const QString &text);

public slots:
    void auth(const QString &region, const QString &email, const QString &password);
    void upload(const QString &region, const QString &filename, const QString &passw, const QString &descr);
    void abort();

private slots:
    void replyFinished();
    void authFinished();
};

#endif // UPLOADER_H
