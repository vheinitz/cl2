#ifndef EIKJUFUNCTION_H
#define EIKJUFUNCTION_H

#include "common.h"
#include "ekhttprequest.h"
#include <QtCore>

class EIKJU_EXPORT EiKjuFunction
{
    EiKjuFunction();
    EiKjuFunction & operator=( const EiKjuFunction &);
    EiKjuFunction ( const EiKjuFunction &);
public:
    static EiKjuFunction & instance()
    {
        static EiKjuFunction inst;
        return inst;
    }
    QString process ( QString function, QString format, const EKHttpRequest & r ) const;
};

#endif // EIKJUFUNCTION_H
