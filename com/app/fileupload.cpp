#include "fileupload.h"
#include <QNetworkReply>
#include <QRegExp>

UpLoader::UpLoader(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    reply = 0;
    upf = 0;
    isAborted = false;
    isInProgress = false;
}
UpLoader::~UpLoader()
{

}
bool UpLoader::aborted()
{
    return isAborted;
}

/////////////////////////////public slots://///////////////////////////////////////////
void UpLoader::auth(const QString &region, const QString &email, const QString &password)
{
	QUrl url(QString("http://127.0.0.1:5000/upload"));
    QNetworkRequest request(url);
    QString returl = QString("http://127.0.0.1:5000/upload").arg(region);
#if 0
    request.setRawHeader("Host", url.encodedHost());
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows; U; Windows NT 5.1; ru; rv:1.9.1.3) Gecko/20090824 Firefox/3.5.3 (.NET CLR 3.5.30729)");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "ru,en-us;q=0.7,en;q=0.3");
    request.setRawHeader("Accept-Encoding", "gzip,deflate");
    request.setRawHeader("Accept-Charset", "windows-1251,utf-8;q=0.7,*;q=0.7");
    request.setRawHeader("Keep-Alive", "300");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Referer", returl.toAscii());
    request.setRawHeader("Cookie", "auth=NO");
#endif
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QString form = QString("retURL=%1&password=%2&email=%3").arg(returl).arg(password).arg(email);
    QByteArray formencoded = QUrl::toPercentEncoding(form, "&=", "");
    reply = manager->post(request, formencoded);
    connect(reply, SIGNAL(finished()), this, SLOT(authFinished()));
    isInProgress = true;
}

void UpLoader::upload(const QString &region, const QString &filename, const QString &passw, const QString &descr)
{
    isAborted = false;
    QByteArray boundaryRegular(QString("--"+QString::number(qrand(), 10)).toAscii());
    QByteArray boundary("\r\n--"+boundaryRegular+"\r\n");
    QByteArray boundaryLast("\r\n--"+boundaryRegular+"--\r\n");

    siteurl = QString("http://127.0.0.1:5000/").arg(region);
    QUrl url(QString(siteurl+"upload_file"));
    QNetworkRequest request(url); 

    request.setRawHeader("Host", url.encodedHost());
    request.setRawHeader("User-Agent", "Helios");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Accept-Language", "en-us");
    request.setRawHeader("Accept-Encoding", "gzip,deflate");
    request.setRawHeader("Accept-Charset", "utf-8");
    request.setRawHeader("Keep-Alive", "300");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Referer", QString("http://127.0.0.1:5000/upload_file").arg(region).toAscii());
    request.setRawHeader("Content-Type", QByteArray("multipart/form-data; boundary=").append(boundaryRegular));

    QByteArray mimedata1("--"+boundaryRegular+"\r\n");
    //mimedata1.append("Content-Disposition: form-data; name=\"action\"\r\n\r\n");
    //mimedata1.append("file_upload");
    //mimedata1.append(boundary);
    mimedata1.append("Content-Disposition: form-data; name=\"file\"; filename=\""+filename.toUtf8()+"\"\r\n");
    mimedata1.append("Content-Type: application/octet-stream\r\n\r\n");

    QByteArray mimedata2(boundary);
    mimedata2.append("Content-Disposition: form-data; name=\"json\"\r\n\r\n");
	mimedata1.append(QString("{\"type\":\"%1\", \"name\":\"%2\"}").arg("image").arg(filename));
    //mimedata2.append(passw.toUtf8());
    //mimedata2.append(boundary);
    //mimedata2.append("Content-Disposition: form-data; name=\"description\"\r\n\r\n");
    //mimedata2.append(descr.toUtf8());
    //mimedata2.append(boundary);
    //mimedata2.append("Content-Disposition: form-data; name=\"agree\"\r\n\r\n");
    //mimedata2.append("1");
    mimedata2.append(boundaryLast);

    upf = new QUpFile(filename, mimedata1, mimedata2, this);
    if (upf->openFile())
    {
        reply = manager->post(request, upf);
        connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SIGNAL(progress(qint64,qint64)));
        connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
        isInProgress = true;
        emit started();
    } else
    {
        emit finished(true, false, tr("Error: can't open file %1").arg(filename));
    }
}

void UpLoader::abort()
{
    isAborted = true;
    if (reply && isInProgress)
        reply->abort();
}

/////////////////////////////private slots:////////////////////////////////////////////
void UpLoader::replyFinished()
{
    isInProgress = false;
    if (upf)
    {
        upf->close();
        delete upf;
        upf = 0;
    }
    disconnect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SIGNAL(progress(qint64,qint64)));
    disconnect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));

    if (isAborted)
    {
        emit finished(false, true, QString());
    }
    else if (reply->error()>0)
    {
        emit finished(true, false, tr("Network error: %1").arg(QString::number(reply->error())));
    }
    else
    {
       QList<QNetworkReply::RawHeaderPair> hp =  reply->rawHeaderPairs();
	   QString json(reply->readAll());
//		QString loc = reply->rawHeader("Location");
	   if ( json.contains(QRegExp("result[: \t\"]*OK") ) )
            emit finished(false, false, "ok");
        else
            emit finished(true, false, tr("Error: %1").arg(reply->errorString()));
    }
}

void UpLoader::authFinished()
{
    isInProgress = false;
    disconnect(reply, SIGNAL(finished()), this, SLOT(authFinished()));
    if (!reply->error())
    {
        QByteArray cookie = reply->rawHeader("Set-cookie");
        int start = cookie.indexOf("auth=", 0)+QString("auth=").length();
        int end = cookie.indexOf(";", start);
        QByteArray str = cookie.mid(start, end-start);
        if (str=="YES")
        {
            emit authFinished(true, QString());
        } else
        {
            emit authFinished(false, tr("Incorrect login or password"));
        }
    }
    else
    {
        emit authFinished(false, tr("Network error: %1").arg(reply->errorString()));
    }
}
