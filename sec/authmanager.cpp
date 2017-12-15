#include "authmanager.h"
//#include "languagemanager.h"
#include <log/clog.h>
#include <QStringList>
#include <QFile>
#include <QApplication>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QStandardItem>
const char ZVDFPasswordText[] = "Password:";
const char ZVDFPassword[] = "cL28SeRvICE&$2017@";


void  AuthManager::updateUserModel()
{
	QStandardItem *userName;
	QStandardItem *userLevel;
	QList<QStandardItem*> userListEntry;	
	_userModel.clear();
	_userModel.setColumnCount(2);
	for ( UserMap::iterator it = _users.begin(); it != _users.end(); ++it )
	{
		userListEntry.clear();
		userName = new QStandardItem(it->_user );
		userLevel = new QStandardItem( userGroups(it->_user) );
		userListEntry << userName << userLevel;
		_userModel.appendRow( userListEntry );
	}	
}

bool AuthManager::load ( QString userdat )
{
	_users.clear();
	_usersFile.clear();
	QFile userFile(userdat);
	
	
	if ( userFile.open(QIODevice::ReadOnly) )
	{
		QTextStream ts(&userFile);

		int line=0;
		while(!ts.atEnd())
		{
			++line;
			QString userentry = ts.readLine();
			if ( userentry.isEmpty() ) continue;

			QStringList userData = userentry.split(":");
			if ( userData.size() != 6 )
			{
				C_ERROR("Format error at %s:%d", QS2CS(userdat), line )
				_userModel.clear( );
				return false;
			}

			UserData data( userData.at(0), userData.at(1), userData.at(2), userData.at(3), QDate::fromString(userData.at(4),"yyyy-MM-dd"), userData.at(5) );
			//TODO: check user name for uniqueness, valid chars, group name, etc.
			_users[data._user] = data;
		}
		_usersFile = userdat;
		updateUserModel();
		return true;
	}
	else
	{
		C_ERROR("Users file not found %s", QS2CS(userdat) )
		_userModel.clear( );
		return false;
	}

}

bool AuthManager::changeUserLanguage ( QString user, QString lang  )
{
	if ( !_users.contains( user ) )
		return false;

	_users[user]._lang = lang;

	return save();
}

bool AuthManager::save()
{
	QFile userFile(_usersFile);
	if ( userFile.open(QIODevice::WriteOnly) )
	{
		QTextStream ts(&userFile);

		for( UserMap::const_iterator it = _users.constBegin(), end = _users.constEnd(); it != end; ++it )
		{
			UserData ud = it.value();
			ts << ud._user<<":"<<ud._password<<":"<<QStringList(ud._groups.toList()).join(",")<<":"<<ud._lang<<":"<<ud._expDate.toString("yyyy-MM-dd")<<":"<<ud._info<<"\n";
		}
		return true;
	}
	else
	{
		C_ERROR("Can't write to users-file %s", QS2CS(_usersFile) )
		return false;
	}
}

void AuthManager::notifySubscribers()
{

	if ( _currentGroups.isEmpty() )
		return;


	for ( Subscribers::iterator it = _subscribers.begin(); it != _subscribers.end(); ++it )
	{
		QString prop = it->_prop;
		QString groups = QStringList(it->_groups.toList()).join(",");
		QString cg = QStringList(_currentGroups.toList()).join(",");
		QSet<QString> ccg= _currentGroups;

		if ( it->_groups.contains( "any") || !ccg.intersect(it.value()._groups).isEmpty() )
		{
			if ( !it->_onTrue.isNull() )
			{
				QVariant val = it->_onTrue;
				QString prop = it->_prop;
				QString objName = it.key()->objectName();
				QStringList gl = it->_groups.toList();
				QString grp = gl.join(",");

				if ( !it.key()->setProperty( qPrintable(it->_prop), val ) )
				{
					C_ERROR( "Internal. Can't set property %s", qPrintable(it->_prop) )
				}	
			}
			else if ( !it->_onFalse.isNull() )
			{
				QVariant val = it->_onFalse;
				QString prop = it->_prop;
				QString objName = it.key()->objectName();
				QStringList gl = it->_groups.toList();
				QString grp = gl.join(",");

				if ( !it.key()->setProperty( qPrintable(it->_prop), val ) )
				{
					C_ERROR( "Internal. Can't set property %s", qPrintable(it->_prop) )
				}				
			}
		}		
	}
}

bool AuthManager::logOut (  )
{
	if ( _currentUser.isEmpty() || _currentGroups.isEmpty() )
	{
		C_WARNING( "Logging out invalid user" );
		return false;
	}
	_currentUser.clear();
	_currentGroups.clear();
	notifySubscribers();
	return true;
}

bool AuthManager::exists ( QString user ) const
{
	UserMap::const_iterator it = _users.constFind(user);
	return it != _users.constEnd();
}

bool AuthManager::logInUser ( QString user, QString password )
{	
	if ( !exists(user) )
	{
		C_ERROR( "Invalid user %s", QS2CS(user) )
		return false;
	}
	UserMap::const_iterator it = _users.constFind(user);
	
	QString textPasswd = password + QStringList(it.value()._groups.toList()).join(",") + it.value()._expDate.toString( "yyyy-MM-dd" );
	QString encPasswd;
	if ( !password.isEmpty() )
	{
		encPasswd = QString( QCryptographicHash::hash( textPasswd.toAscii(), QCryptographicHash::Md5).toBase64() );
	}

	if ( it.value()._password != encPasswd )
	{
		C_ERROR( "Invalid password attempt for user %s", QS2CS(user) )
		return false;
	}

	_currentGroups = it.value()._groups;
	_currentUser = user;
	notifySubscribers();
	
	return true;
}

bool AuthManager::checkPassword ( QString password )
{
	UserMap::const_iterator it = _users.constFind(_currentUser);
	if ( it == _users.constEnd() )
	{
		C_ERROR( "Internal error. Invalid current user %s", QS2CS(_currentUser) )
		return false;
	}

	QString textPasswd = password + QStringList(it.value()._groups.toList()).join(",") + it.value()._expDate.toString( "yyyy-MM-dd" );
	QString encPasswd;
	if ( !password.isEmpty() )
	{
		encPasswd = QString( QCryptographicHash::hash( textPasswd.toAscii(), QCryptographicHash::Md5).toBase64() );
	}

	if ( it.value()._password != encPasswd )
	{
		C_ERROR( "Invalid password attempt for user %s", QS2CS(QStringList(_currentGroups.toList()).join(","))  )
		return false;
	}

	return true;
}

QString AuthManager::encrypt ( QString text )
{
	return QCryptographicHash::hash( text.toAscii(), QCryptographicHash::Md5).toBase64();
}

bool AuthManager::checkInternalPassword ( QString text, QString enc )
{
	return QCryptographicHash::hash( text.toAscii(), QCryptographicHash::Md5).toBase64() == enc;
}

void AuthManager::registerSubscriber( QString requiredGroups, QObject * obj, QString prop, QVariant onFalse, QVariant onTrue )
{
	PrivilegSubscriber ps(requiredGroups, prop, onFalse, onTrue );
	_subscribers.insert(obj,ps); 
	notifySubscribers();
}

bool AuthManager::addUser ( QString user, QString password, QString group, QString lang, const QDate & expDate, QString info )
{
	if ( exists(user) )
	{
		C_ERROR( "User already exists. Can't add %s", QS2CS(user) )
		return false;
	}
	_users[user];
	editUser ( user, password, group, lang, expDate, info );	
	save();
	return true;
}

bool AuthManager::removeUser ( QString user )
{
	if ( !exists(user) )
	{
		C_ERROR( "Invalid user %s", QS2CS(user) )
		return false;
	}
	_users.remove(user);
	updateUserModel();
	save();
	return true;
}

bool AuthManager::editUser ( QString user, QString password, QString group, QString lang, const QDate & expDate, QString info )
{
	UserMap::iterator it = _users.find(user);
	if ( it == _users.end() )
	{
		C_ERROR( "Invalid user %s", QS2CS(user) )
		return false;
	}

	QString textPasswd = password + group + expDate.toString( "yyyy-MM-dd" );
	QString encPasswd = QString( QCryptographicHash::hash( textPasswd.toAscii(), QCryptographicHash::Md5).toBase64() );
	UserData userDat( user, encPasswd, group, lang, expDate, info );
	_users[user] = userDat;
	updateUserModel();
	save();
	return true;
}

QString AuthManager::userGroups ( QString user ) const
{
	if ( !exists(user) )
	{
		C_ERROR( "Invalid user %s", QS2CS(user) )
	}
	return QStringList(_users[user]._groups.toList()).join(",");
}


QString AuthManager::userLang ( QString user ) const
{
	if ( !exists(user) )
	{
		C_ERROR( "Invalid user %s", QS2CS(user) )
	}
	return _users[user]._lang;
}

QString AuthManager::userInfo ( QString user ) const
{
	if ( !exists(user) )
	{
		C_ERROR( "Invalid user %s", QS2CS(user) )
	}
	return _users[user]._info;
}

QDate  AuthManager::passwdExpDate ( QString user )const
{
	if ( !exists(user) )
	{
		C_ERROR( "Invalid user %s", QS2CS(user) )
	}
	return _users[user]._expDate;
}
