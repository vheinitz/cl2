#include <QtNetwork>

#include "httpsrequest.h"
#include <log/clog.h>
#include <QNetworkRequest>
#include <QSslError>

HttpsRequest::HttpsRequest(QObject *parent): QObject(parent),
    _lastError(QSslError::NoError),
    _authRequestCnt(0)
{
    connect(&_nwAccMngr, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(processAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(&_nwAccMngr, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(processSslErrors(QNetworkReply*,QList<QSslError>)));
}

void HttpsRequest::startRequest(QString url, QMap<QString, QString> params)
{
   _url=url;
   _url+="?";
   

   foreach( QString k, params.keys())
   {
	   _url += QString("%1=%2&").arg(k).arg(params[k]);
   }
   QNetworkRequest request(_url); 
   _reply = _nwAccMngr.get( request );
   connect(_reply, SIGNAL(finished()),this, SLOT(processFinished()));
}


void HttpsRequest::processFinished()
{    
    QNetworkReply * finishedReply = qobject_cast<QNetworkReply*>( sender() );    
	QByteArray data = finishedReply->readAll();
	int err = finishedReply->error();
    emit finished();
}

void HttpsRequest::processAuthenticationRequired(QNetworkReply* reply,QAuthenticator *authenticator)
{
  (void) reply; // avoid unused parameter warning
    _authRequestCnt++;
    if ( _authRequestCnt > 3)
    {
        return;
    }
    else if ( _authRequestCnt > 2)
    {
        emit error( "Authentification error" );
        return;
    }
    authenticator->setUser(_userName);
    authenticator->setPassword(_password); 
}

void HttpsRequest::processSslErrors(QNetworkReply*reply,const QList<QSslError> &errors)
{
    QString errorString;
    foreach (const QSslError &error, errors) {
        _lastError |= QSslError::NoError; //required in finished for skipping invalid replies
        if ( error.error() != QSslError::NoError )
        {
            if (!errorString.isEmpty())
                errorString += "\n";
            errorString += error.errorString();
        }
    }
    
    if ( !errorString.isEmpty() )
    {     
        emit error( errorString );
    }
    reply->ignoreSslErrors();
}
