#ifndef EKHTTPREQUEST_H
#define EKHTTPREQUEST_H
#include <QtCore>
#include "common.h"

class EIKJU_EXPORT EKHttpRequest
{
    QStringList _data;
    QMap<QString,QString> _vars;
    QString _url;
    public:
    EKHttpRequest( ):_url("/"){};
    void append(QString data){ _data.append(data); }
    void parse()
    {
        if(isValide())
        {
            QString get = _data[0].section(" ",1,1).section(" ",0,0);
            _url = get.section('?',0,0);
            QStringList vvpairs = get.section('?',1,1).split('&');
            foreach( QString vvp, vvpairs )
            {
                _vars.insert(vvp.section('=',0,0),vvp.section('=',1,1));
            }
        }
    }
    bool isValide( ) const {return _data.size() > 0;}

    const QString & url() const {return _url;}
    const QMap<QString,QString> & getVars() const { return _vars;}
    const QStringList & httpData() const { return _data;}
};

#endif // EKHTTPREQUEST_H
