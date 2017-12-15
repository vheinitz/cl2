#include <QtNetwork>

#include "httpsupload.h"
#include <log/clog.h>

HttpsUpload::HttpsUpload(QObject *parent): QObject(parent),
    _authRequestCnt(0),
    _lastError(QSslError::NoError),
    _openConnections(0),
    _maxOpenConnections(1)
{
    connect(&_nwAccMngr, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(processAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(&_nwAccMngr, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
}

void HttpsUpload::startRequests(QUrl url, QStringList fileNames, QMap<QString, QString> params)
{    
    foreach( QString fileName, fileNames)
        startRequest(url, fileName , params);
}

void HttpsUpload::startRequest(QUrl url, QString fileName, QMap<QString, QString> params)
{
    _pendingRequests.append( RequestData( fileName, url, params ) );
    _progressInfo[ fileName ] = 0;
    if ( _openConnections < _maxOpenConnections )
    {
        processNextRequest( );
    }
}

void HttpsUpload::processNextRequest( )
{
	/*
    if ( _openConnections < _maxOpenConnections )
        ++_openConnections;

    if ( _pendingRequests.isEmpty() )
    {
        emit completed();
        return;
    }
    RequestData rd = _pendingRequests.takeFirst();
    if ( _lastUrl != rd._url )
    {
        _authRequestCnt=0;
        _lastError=0;
        _lastUrl = rd._url;
    }

    QFile file( rd._file, this );
    if ( file.open(QIODevice::ReadOnly) )
    {
        QNetworkRequest request(rd._url);
        request.setRawHeader("Host", rd._url.encodedHost());
//        request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows; U; Windows NT 6.0; de; rv:1.9.2.22) Gecko/20110902 Firefox/3.6.22 ( .NET CLR 3.5.30729; .NET4.0C)");
//        request.setRawHeader("Accept-Language", "de-de,de;q=0.8,en-us;q=0.5,en;q=0.3");
//        request.setRawHeader("Accept-Encoding", "gzip,deflate");
//        request.setRawHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
//        request.setRawHeader("Keep-Alive", "115");
//        request.setRawHeader("Connection", "keep-alive");
         request.setRawHeader("Content-Type", "image\png" );
	  QByteArray data = file.readAll();
	  request.setRawHeader("Content-Length", QString::number( data.length() ) );
    	  request.setRawHeader("Expect", "100-continue" );


        QNetworkReply * reply = _nwAccMngr.put( request, data);
        reply->setProperty( "_uploadFile", rd._file );
        connect(reply, SIGNAL(finished()),
                this, SLOT(uploadFinished()));
        connect(reply, SIGNAL(uploadProgress(qint64,qint64)),
                this, SLOT(updateUploadProgress(qint64,qint64)));
        _replyList.append( reply );
    }*/
}



void HttpsUpload::uploadFinished()
{    
    if ( _openConnections > 0 ) 
        --_openConnections;

    QNetworkReply * finishedReply = qobject_cast<QNetworkReply*>( sender() );    
    QString file = finishedReply->property( "_uploadFile" ).toString();
    _progressInfo.remove( file );
    removeReply(finishedReply);
    if ( _lastError == 0 )
    {
        emit message( QString ("Finished: %1").arg(file) );
        if (_progressInfo.isEmpty())
        {
            emit message( "Completed" );
            emit completed();
        }
    }
    updateProgress();
    processNextRequest( );
}

void HttpsUpload::updateUploadProgress(qint64 bytesRead, qint64 totalBytes)
{
    QNetworkReply * reply = qobject_cast<QNetworkReply*>( sender() );
    QString file = reply->property( "_uploadFile" ).toString();
    if ( bytesRead != 0  )
        _progressInfo[ file ] = (bytesRead / totalBytes) *100;

    updateProgress();
}

/*! Finds reply object in list and removes it

*/
void HttpsUpload::removeReply( QNetworkReply* removeReply )
{
    int idx=-1;
    int i=0;

    foreach ( QNetworkReply *reply, _replyList ){
        if ( reply ==  removeReply ){
            idx = i;
            break;
        } else ++i;
    }

    if ( idx >= 0 )
    {
        QNetworkReply * reply = _replyList.takeAt(idx);
        reply->deleteLater();
    }
    else
    {
        C_ERROR( "Internal error. Inconsistent data-structures" )
        return;
    }
}

void HttpsUpload::updateProgress( )
{
    int row=0;
    _progressModel.clear();
    for ( QMap<QString, int>::Iterator it =_progressInfo.begin(); it !=_progressInfo.end(); ++it )
    {
        _progressModel.setItem(row, 0, new QStandardItem(it.key()) );
        _progressModel.setItem(row, 1, new QStandardItem( QString("%1%").arg( it.value() ) ) );
        ++row;
    }
}

void HttpsUpload::processAuthenticationRequired(QNetworkReply* reply,QAuthenticator *authenticator)
{
  (void) reply; // avoid unused parameter warning
    _authRequestCnt++;
    if ( _authRequestCnt > 3)
    {
        return;
    }
    else if ( _authRequestCnt > 2)
    {
        emit uploadError( "Authentification error" );
        return;
    }
    authenticator->setUser(_userName);
    authenticator->setPassword(_password); 
}

void HttpsUpload::sslErrors(QNetworkReply*reply,const QList<QSslError> &errors)
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
        emit uploadError( errorString );
    }
    reply->ignoreSslErrors();
}
