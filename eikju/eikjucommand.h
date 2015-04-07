#ifndef EIKJUCOMMAND_H
#define EIKJUCOMMAND_H
#include "ekhttprequest.h"
#include "common.h"
#include <QtCore>

class EIKJU_EXPORT EiKjuCommand
{
    EiKjuCommand();
    EiKjuCommand & operator=( const EiKjuCommand &);
    EiKjuCommand ( const EiKjuCommand &);
public:
    static EiKjuCommand & instance()
    {
        static EiKjuCommand inst;
        return inst;
    }
    void process ( const EKHttpRequest & r )const;
};

#endif // EIKJUCOMMAND_H
