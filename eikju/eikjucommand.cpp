#include "eikjucommand.h"
#include "ekhttprequest.h"

EiKjuCommand::EiKjuCommand()
{
}

void EiKjuCommand::process ( const EKHttpRequest & r )const
{
    QString command = r.getVars()["command"];

    if( command == "xyz" ) // 
    {
        qDebug( QString( "xyz: %1").arg(r.getVars()["param"]).toStdString( ).c_str()  );
    }

}
