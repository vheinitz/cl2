#ifndef _AUTH_MANAGER_HG_
#define _AUTH_MANAGER_HG_

#include <base/cldef.h>
#include <QString>
#include <QMap>
#include <QMultiMap>
#include <QObject>
#include <QDate>
#include <QVariant>
#include <QStandardItemModel>
#include <QSet>

struct UserData
{
	QString _user;
	QString _password;
	QSet<QString> _groups;
	QString _lang;
	QDate _expDate;
	QString _info;
	UserData( QString user, QString password, QString groups, QString lang, const QDate & expDate, QString info ):
		_user(user), _password(password), _groups(groups.split(",").toSet()), _lang(lang), _expDate(expDate), _info(info){}
	UserData(){}
};


struct PrivilegSubscriber
{
	QSet<QString> _groups;
	QString _prop;	
	QVariant _onFalse;
	QVariant _onTrue;
	PrivilegSubscriber( QString group, QString prop, QVariant onFalse, QVariant onTrue ):
	_groups(group.split(",").toSet()), _prop(prop), _onFalse(onFalse), _onTrue(onTrue){}
	PrivilegSubscriber(){}
};


typedef QMap<QString, UserData> UserMap;
typedef QList<PrivilegSubscriber> TPrivilegSubscribers;
typedef QMultiMap<QObject *, PrivilegSubscriber> Subscribers;


class ReadOnlyStandardItemModel : public QStandardItemModel
{
public:
	virtual Qt::ItemFlags flags ( const QModelIndex &  ) const { return Qt::ItemIsSelectable | Qt::ItemIsEnabled; }
};

/*! @brief Class for authentifying users in system
*/
class CLIB_EXPORT AuthManager : public QObject
{
	Q_OBJECT

private:
	AuthManager(){};
	QString _currentUser;
	QSet<QString> _currentGroups;
	QString _usersFile;
	UserMap _users;
	Subscribers _subscribers;
	ReadOnlyStandardItemModel _userModel;
private:
	void notifySubscribers();
	void updateUserModel();
public:
	static AuthManager &instance()
	{
		static AuthManager inst;
		return inst;		
	}
	QAbstractItemModel * userModel( ) {return &_userModel;}
	bool load ( QString userdat );
	bool exists ( QString user ) const;
	bool save();
	bool logInUser ( QString user, QString password=QString::null  );
	bool changeUserLanguage ( QString user, QString lang  );
	bool checkPassword ( QString password );	
	bool logOut (  );
	bool addUser ( QString user, QString password, QString group, QString lang, const QDate & expDate, QString info );
	bool removeUser ( QString user );
	bool editUser ( QString user, QString password, QString group, QString lang, const QDate & expDate, QString info );
	const QString & currentUser ( )const { return _currentUser; }
	QString userGroups ( QString user )const;
	QString userLang ( QString user )const;
	QString userInfo ( QString user )const;
	QDate passwdExpDate ( QString user )const;
	void registerSubscriber( QString requiredGroup, QObject * obj, QString prop, QVariant onFalse, QVariant onTrue );
	QStringList userLevels() const { return QStringList() << "user" << "superuser" << "administrator"; }
	QStringList users() const { return _users.keys(); }

public: //Static helper methods
	static QString encrypt ( QString text );
	static bool checkInternalPassword ( QString textm, QString enc  );
};

#endif