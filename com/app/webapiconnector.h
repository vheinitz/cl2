#ifndef WebApiConnector_HG
#define WebApiConnector_HG

#include <QObject>
#include <cldef.h>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QMap>
#include <QList>
#include <QSharedPointer>
#include <QNetworkReply>


class CLIB_EXPORT WebApiConnector : public QObject
{
    Q_OBJECT
    
	QNetworkAccessManager *_manager;

	void replyFinished(QNetworkReply*);

	WebApiConnector();

public:    
	static WebApiConnector * instance()
	{
		static WebApiConnector *inst = new WebApiConnector;
		return inst;
	};
	~WebApiConnector();

	void sendMailNotificationFinished( const QString & = QString::null );
	void sendMailNotificationError( const QString & = QString::null );
	void sendMailNotification(QString);

	void sendSmsNotificationFinished( const QString & = QString::null );
	void sendSmsNotificationError( const QString & = QString::null );	
	void sendSmsNotification(QString);

	void sendNotificationFinished();
	void sendNotificationError();
	void sendNotificationError(QString);

	QMap<QString, QString> _params; //k - key, t - text, d- destination, a- api url
};

#endif // WebApiConnector_HG
