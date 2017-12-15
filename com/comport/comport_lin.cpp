//=============================================================================
//  Valentin Heinitz,
//  Linux stub implementation for serial interface class from
//  UAB BBDSoft ( http://www.bbdsoft.com/ )
//=============================================================================
#if 0

#include "comport.h"
#include <string>
#include <wchar.h>

COMPort::COMPort ( const char * const portName )
{
}

void COMPort::purge ( )
{    
}

//----------------------------------------------------------------------------
COMPort::~COMPort()
{

}


//----------------------------------------------------------------------------
void COMPort::getState () const
{
}


//----------------------------------------------------------------------------
COMPort& COMPort::setState ()
{
return *this;
}


//-----------------------------------------------------------------------------
COMPort& COMPort::setBitRate ( unsigned long Param )
{
}


//-----------------------------------------------------------------------------
unsigned long COMPort::bitRate() const
{

return 0;
} // end COMPort::bitRate () const


//-----------------------------------------------------------------------------
COMPort& COMPort::setLineCharacteristics( const wchar_t * inConfig )
{
    return *this;
}


//----------------------------------------------------------------------------
char COMPort::read ()
{
    return 0;
} 


//----------------------------------------------------------------------------
unsigned long COMPort::read ( void *inBuffer
                            , const unsigned long inCharsReq
                            )
{
   return 0;
}

//----------------------------------------------------------------------------
COMPort & COMPort::write ( const char inChar )
{
    return  *this;
}


//----------------------------------------------------------------------------
unsigned long COMPort::write ( const void *inBuffer
                             , const unsigned long inBufSize
                             )
{
    return  0;
}


//-----------------------------------------------------------------------------
COMPort& COMPort::setxONxOFF ( bool x )
{
    return *this;
}


//-----------------------------------------------------------------------------
bool COMPort::isxONxOFF () const
{
    return false;
}


void COMPort::setNonBlockingMode ( )
{
    
}


//----------------------------------------------------------------------------
void COMPort::setBlockingMode ( unsigned long inReadInterval
                                  , unsigned long inReadMultiplyer
                                  , unsigned long inReadConstant
                                  , unsigned long inWriteMultiplyer 
                                  , unsigned long inWriteConstant 
                                  )
{

}

//-----------------------------------------------------------------------------
COMPort & COMPort::setHandshaking ( bool inHandshaking )
{
    return *this;
}


//-----------------------------------------------------------------------------
unsigned long COMPort::getMaximumBitRate() const
{
    return 0;
}

//-----------------------------------------------------------------------------
COMPort::MSPack COMPort::getModemSignals() const
{
    return MSPack();
}

//-----------------------------------------------------------------------------
COMPort& COMPort::setParity ( Parity Param )
{
    return *this;
}

//-----------------------------------------------------------------------------
COMPort& COMPort::setDataBits ( DataBits Param )
{
    return *this;
}

//-----------------------------------------------------------------------------
COMPort& COMPort::setStopBits ( StopBits Param )
{
    return *this;
}

//-----------------------------------------------------------------------------
COMPort::Parity COMPort::parity () const
{
    COMPort::None;
}


//-----------------------------------------------------------------------------
COMPort::DataBits COMPort::dataBits () const
{
    return DataBits();
} 


//-----------------------------------------------------------------------------
COMPort::StopBits COMPort::stopBits () const
{
    return COMPort::StopBits();
}

#endif