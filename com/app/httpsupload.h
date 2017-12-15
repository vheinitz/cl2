#ifndef _HTTPSUPLOAD_H_hg
#define _HTTPSUPLOAD_H_hg

#include <cldef.h>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QMap>
#include <QList>
#include <QSharedPointer>
#include <QNetworkReply>
#include <QStandardItemModel>


typedef QList<QNetworkReply*> NetworkReplyList;

class QFile;
class QSslError;
class QAuthenticator;
class QNetworkReply;

struct RequestData
{
    RequestData(){}
    RequestData ( QString file, QUrl url, QMap<QString, QString> params ):
        _file(file),
        _url(url),
        _params(params)
    {}
    QString _file;
    QUrl _url;
    QMap<QString, QString> _params;
};

typedef QList<RequestData> TRequestDataList;


class CLIB_EXPORT HttpsUpload : public QObject
{
    Q_OBJECT  
signals:
    void message( QString );
    void uploadError( QString );
    void completed( );

public:
    
    HttpsUpload( QObject * parent);
    QAbstractItemModel * progressModel(){ return &_progressModel;}


    void startRequest(QUrl url, QString file, QMap<QString, QString> params = QMap<QString, QString>());
    void startRequests(QUrl url, QStringList files, QMap<QString, QString> params = QMap<QString, QString>());

    QString userName() const { return _userName; }
    QString password() const { return _password; }

    void setUserName( QString userName ){ _userName = userName; _authRequestCnt=0; _lastError=0; }
    void setPassword( QString password ){ _password = password; _authRequestCnt=0; _lastError=0; }

private slots:
    void uploadFinished();
    void updateUploadProgress(qint64 bytesRead, qint64 totalBytes);
    void processAuthenticationRequired(QNetworkReply*,QAuthenticator *);
    void sslErrors(QNetworkReply*,const QList<QSslError> &errors);

private:
    QNetworkAccessManager _nwAccMngr;
    NetworkReplyList _replyList;
    QString _userName;
    QString _password;
    QUrl _lastUrl;
    QMap<QString, int> _progressInfo;
    QStandardItemModel _progressModel;
    int _authRequestCnt;
    int _lastError;
    int _openConnections;
    int _maxOpenConnections;
    TRequestDataList _pendingRequests;

    void removeReply( QNetworkReply* );
    void processNextRequest( );
    void updateProgress( );

};

#endif
