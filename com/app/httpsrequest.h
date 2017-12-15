#ifndef _HTTPS_REQUEST__H
#define _HTTPS_REQUEST__H

#include <cldef.h>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QMap>
#include <QList>
#include <QSharedPointer>
#include <QNetworkReply>
#include <QStandardItemModel>


class QSslError;
class QAuthenticator;
class QNetworkReply;


class CLIB_EXPORT HttpsRequest : public QObject
{
    Q_OBJECT  
signals:
    void message( QString );
    void error( QString );
    void finished( );

public:
    
    HttpsRequest( QObject * parent);    

    void startRequest(QString url, QMap<QString, QString> params = QMap<QString, QString>());

    QString userName() const { return _userName; }
    QString password() const { return _password; }

    void setUserName( QString userName ){ _userName = userName; _authRequestCnt=0; _lastError=0; }
    void setPassword( QString password ){ _password = password; _authRequestCnt=0; _lastError=0; }

private slots:
    void processFinished();
    void processAuthenticationRequired(QNetworkReply*,QAuthenticator *);
    void processSslErrors(QNetworkReply*,const QList<QSslError> &errors);

private:
    QNetworkAccessManager _nwAccMngr;
    QNetworkReply * _reply;
    QString _userName;
    QString _password;
    QString _url;
    int _lastError;
	int _authRequestCnt;
};

#endif
