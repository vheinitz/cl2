#include "eikjufunction.h"
#include <QtCore>

EiKjuFunction::EiKjuFunction()
{
}

QString EiKjuFunction::process ( QString function, QString format, const EKHttpRequest & r ) const
{
    QString outhtml(" ");
    if ( function == "function_name" )
    {
        qDebug( "TODO function_name" ) ;
    }
    return outhtml;
}
