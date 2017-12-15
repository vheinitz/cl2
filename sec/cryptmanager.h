#ifndef _CRYPT_MANAGER_HG_
#define _CRYPT_MANAGER_HG_

#include <base/cldef.h>
#include <QString>
#include <QProcess>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QCryptographicHash>



/*! @brief Class encapsulates basic en/decryption
*/
class CLIB_EXPORT CryptManager : public QObject
{
	Q_OBJECT

private:
	CryptManager();
	QString _cryptTool;
	QMap<QString,QString> _cryptKeyVersions;
public:
	static CryptManager &instance()
	{
		static CryptManager inst;
		return inst;		
	}
	bool encrypt ( QString in, QString out, QString key );
	bool decrypt ( QString in, QString out, QString key );
	QString getWorkListXmlPath ( QString in );
};

#endif