//=============================================================================
//  Serial port interface. Implementation for Win32
//  Valentin Heinitz, 2011-11-18
//
//  The code was derived from:
//     General component library for WIN32
//     Copyright (C) 2000, UAB BBDSoft ( http://www.bbdsoft.com/ )
//     The author of original program may be contacted at developers@bbdsoft.com
//=============================================================================


#include "comport.h"
#include <windows.h>

#include <string>
#include <sstream>
#include <algorithm>

#include <log/clog.h>

using namespace std;

#if 0

int COMPort::_portHandle=HFILE_ERROR; //ugly WA, get comport by factory in units!!!

//----------------------------------------------------------------------------
COMPort::COMPort ( const char * portName )
  : _DCB (0)
  //, _portHandle(HFILE_ERROR)
{    
    C_TRACE("");
    if ( !open(portName) )
    {
        C_ERROR( "Cant open port \"%s\"", portName  );
    }

} // end constructor

COMPort::COMPort ( ):
_DCB (0)
//, _portHandle(HFILE_ERROR)
{

}

bool COMPort::open ( const char * portName )
{
    C_TRACE("");
    if ( _portHandle != HFILE_ERROR )
    {
        C_WARNING( "COMPort already uses port \"%s\"",portName  )
		close();
        //return false;
    }

    _portName = portName ;
    std::wstring w_portName;    
    w_portName.resize( _portName.size() );
    std::copy ( _portName.begin(), _portName.end(), w_portName.begin() );
    
    _portHandle = (unsigned int) CreateFile ( w_portName.data()
                                       , GENERIC_READ | GENERIC_WRITE
                                       , 0
                                       , NULL
                                       , OPEN_EXISTING
                                       , FILE_FLAG_NO_BUFFERING
                                       , NULL
                                       );
    if (_portHandle != HFILE_ERROR)
    {
        purge();
        _DCB = new char [sizeof(DCB)];
        getState();
        setBlockingMode();
        setHandshaking();
        return true;
    }
	else{
		return false;
	}
}

void COMPort::close()
{
    C_TRACE("");
    if ( _portHandle == HFILE_ERROR )
    {
        C_WARNING( "COMPort not opened"  )
        return;
    }

    delete [] _DCB;
    if (_portHandle != HFILE_ERROR)
    {
        CloseHandle((HANDLE)_portHandle);
    }
    _portHandle = HFILE_ERROR;
}

//Implementation basen on code from Norbert Wiedmann. 
TSerialInterfaceInfoList COMPort::availableInterfaces()
{
    C_TRACE("");
    TSerialInterfaceInfoList iflist;
    //Use QueryDosDevice to look for all devices of the form COMxx. Since QueryDosDevice does
    //not consitently report the required size of buffer, lets start with a reasonable buffer size
    //of 4096 characters and go up from there
    int nChars = 4096;
    BOOL stopQuery = FALSE;

    while(!stopQuery){
        LPTSTR szDevices = NULL;
		
		szDevices = (LPTSTR)malloc(nChars * sizeof(TCHAR));
		if(szDevices){
			DWORD dwChars = QueryDosDevice(NULL, szDevices, nChars);
			if (dwChars == 0){
				DWORD dwError = GetLastError();
				if (dwError == ERROR_INSUFFICIENT_BUFFER){
					//Expand the buffer and loop around again
					nChars *= 2;
				}
				else{
					stopQuery = TRUE;
				}
			}
			else {
				stopQuery = TRUE;
				size_t idx = 0;
				while (idx < dwChars){
					//Get the current device name
					TCHAR* pszCurrentDevice = &(szDevices[idx]);
#ifdef UNICODE
					QString devName = QString::fromUtf16(pszCurrentDevice);
#else
					QString devName = QString::fromAscii(pszCurrentDevice);
#endif
					//check if it looks like "COMx" or "COMxx"
					if(devName.indexOf("COM", 0, Qt::CaseSensitive) == 0){
						if((devName.length() > 3) && (devName.length() < 6)){
							QString portNr;
							portNr = devName.mid(3);
							if(portNr.toInt() > 0){
								QString userName = devName;
								devName.prepend("\\\\.\\");
								iflist.push_back( TSerialInterfaceInfo(userName.toStdString(), devName.toStdString()) );
								C_TRACE(" Available Port: '%s' detected.", QS2CS(devName));
							}
						}
					}

					//Go to next device name
					idx += (devName.length() + 1);
				}
			}
		}
		else{
			stopQuery = TRUE;
			SetLastError(ERROR_OUTOFMEMORY);        
		}
		free(szDevices);
    }
    return iflist;
}

void COMPort::purge ( )
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        PurgeComm((HANDLE)_portHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    }
}


//----------------------------------------------------------------------------
COMPort::~COMPort()
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        close();
    }    
}


//----------------------------------------------------------------------------
void COMPort::getState () const
{

    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        GetCommState ( (HANDLE) _portHandle
                  , (LPDCB) _DCB
                  );
    }

}


//----------------------------------------------------------------------------
COMPort& COMPort::setState ()
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        SetCommState ( (HANDLE) _portHandle
                  , (LPDCB) _DCB
                  );
    }
    return *this;
} 


//-----------------------------------------------------------------------------
COMPort& COMPort::setBitRate ( unsigned long Param )
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        DCB & aDCB = *((LPDCB)_DCB);
        aDCB.BaudRate = Param;
    }
    return setState();
}


//-----------------------------------------------------------------------------
unsigned long COMPort::bitRate() const
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        DCB & aDCB = *((LPDCB)_DCB);
        return aDCB.BaudRate;
    }
    return 0;
}


//-----------------------------------------------------------------------------
COMPort& COMPort::setLineCharacteristics( const wchar_t * inConfig )
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        COMMTIMEOUTS aTimeout;
        BuildCommDCBAndTimeouts ( inConfig
                              , (LPDCB)_DCB
                              , &aTimeout
                              );
        SetCommTimeouts ( (HANDLE(_portHandle))
                       , &aTimeout
                       );
    }
    return setState();
}


//----------------------------------------------------------------------------
char COMPort::read ()
{
    C_TRACE("");
    char buffer = '\0';
    if (_portHandle != HFILE_ERROR)
    {    
        DWORD charsRead = 0;
        do
        {
           ReadFile ( (HANDLE(_portHandle))
                          , &buffer
                          , sizeof(char)
                          , &charsRead
                          , NULL
                          );
        } while ( !charsRead );
    }
    return  buffer;
}


//----------------------------------------------------------------------------
unsigned long COMPort::read ( void *inBuffer
                            , const unsigned long inCharsReq
                            )
{
    C_TRACE("");
    DWORD charsRead = 0;
    if (_portHandle != HFILE_ERROR)
    {
        ReadFile ( (HANDLE(_portHandle))
                   , inBuffer
                   , inCharsReq
                   , &charsRead
                   , NULL
                   );
    }
    return charsRead;
}


//----------------------------------------------------------------------------
COMPort & COMPort::write ( const char inChar )
{
    C_TRACE("");
    char buffer = inChar;
    DWORD charsWritten = 0;
    if (_portHandle != HFILE_ERROR)
    {
        WriteFile ( (HANDLE(_portHandle))
                , &buffer
                , sizeof(char)
                , &charsWritten
                , NULL
                );
    }
    return  *this;
}


//----------------------------------------------------------------------------
unsigned long COMPort::write ( const void *inBuffer
                             , const unsigned long inBufSize
                             )
{
    C_TRACE("");
    DWORD charsWritten = 0;
    if (_portHandle != HFILE_ERROR)
    {
        WriteFile ( (HANDLE(_portHandle))
                , inBuffer
                , inBufSize
                , &charsWritten
                , NULL
                );
    }
    return  charsWritten;
}


//-----------------------------------------------------------------------------
COMPort& COMPort::setxONxOFF ( bool x )
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
        {
        DCB & aDCB = *((LPDCB)_DCB);
        aDCB.fOutX = x ? 1 : 0;
        aDCB.fInX = x ? 1 : 0;
        if ( !x ) // VH
        {
           aDCB.XonLim=0;
           aDCB.XoffLim=0;
        }
    }
    return setState();
}


//-----------------------------------------------------------------------------
bool COMPort::isxONxOFF () const
{
    C_TRACE("");
    DCB & aDCB = *((LPDCB)_DCB);
    return (aDCB.fOutX && aDCB.fInX);
}


//VH. added by me. 2007-01-22, vheinitz@googlemail.com
void COMPort::setNonBlockingMode ( )
{
    C_TRACE("");
    setBlockingMode( MAXDWORD, 0,0, 0,0 );
}


//----------------------------------------------------------------------------
void COMPort::setBlockingMode ( unsigned long inReadInterval
                                  , unsigned long inReadMultiplyer
                                  , unsigned long inReadConstant
                                  , unsigned long inWriteMultiplyer 
                                  , unsigned long inWriteConstant 
                                  )
{
    C_TRACE("");
    COMMTIMEOUTS commTimeout;
    if (_portHandle != HFILE_ERROR)
    {
        GetCommTimeouts ( (HANDLE(_portHandle))
                      , &commTimeout
                      );    
        commTimeout.ReadIntervalTimeout = inReadInterval;

        commTimeout.WriteTotalTimeoutMultiplier = inWriteMultiplyer;
        commTimeout.WriteTotalTimeoutConstant = inWriteConstant;

        if ( inReadInterval==MAXDWORD )
        {
           commTimeout.ReadTotalTimeoutMultiplier = 0;
           commTimeout.ReadTotalTimeoutConstant = 0;
        }
        else
        {
           commTimeout.ReadTotalTimeoutMultiplier = inReadMultiplyer;     
           commTimeout.ReadTotalTimeoutConstant = inReadConstant;
        }  // endifelse

        SetCommTimeouts ( (HANDLE(_portHandle))
                              , &commTimeout
                              );
    }
}


//-----------------------------------------------------------------------------
COMPort & COMPort::setHandshaking ( bool inHandshaking )
{
    C_TRACE("");
    DCB & aDCB = *((LPDCB)_DCB);
    if (inHandshaking)
    {
       aDCB.fOutxCtsFlow = TRUE;
       aDCB.fOutxDsrFlow = FALSE;
       aDCB.fRtsControl = RTS_CONTROL_HANDSHAKE;
    }
    else
    {
       aDCB.fOutxCtsFlow = FALSE;
       aDCB.fOutxDsrFlow = FALSE;
       aDCB.fRtsControl = RTS_CONTROL_ENABLE;
    } // endifelse

return setState();
} // end COMPort::setHandshaking (..)


//-----------------------------------------------------------------------------
unsigned long COMPort::getMaximumBitRate() const
{
    C_TRACE("");
    COMMPROP aProp = COMMPROP();
    if (_portHandle != HFILE_ERROR)
    {
        GetCommProperties ( (HANDLE)_portHandle
                        , &aProp );
    }
    return aProp.dwMaxBaud;
}


//-----------------------------------------------------------------------------
COMPort::MSPack COMPort::getModemSignals() const
{
    C_TRACE("");
    MSPack aPack=MSPack();
    // 1 bit - DTR, 2 - bit RTS                           (output signals)
    // 4 bit - CTS, 5 bit - DSR, 6 bit - RI, 7 bit - DCD  (input signals)
    if (_portHandle != HFILE_ERROR)
    {
        GetCommModemStatus ( (HANDLE)_portHandle
                         , (LPDWORD)&aPack );
    }
    return aPack;
}

//-----------------------------------------------------------------------------
COMPort& COMPort::setParity ( Parity Param )
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        DCB & aDCB = *((LPDCB)_DCB);
        aDCB.Parity = Param;
    }
    return setState();
}


//-----------------------------------------------------------------------------
COMPort& COMPort::setDataBits ( DataBits Param )
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        DCB & aDCB = *((LPDCB)_DCB);
        aDCB.ByteSize = Param;
    }
    return setState();
}


//-----------------------------------------------------------------------------
COMPort& COMPort::setStopBits ( StopBits Param )
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        DCB & aDCB = *((LPDCB)_DCB);
        aDCB.StopBits = Param;
    }
    return setState();
}


//-----------------------------------------------------------------------------
COMPort::Parity COMPort::parity () const
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        DCB & aDCB = *((LPDCB)_DCB);
        return (COMPort::Parity)aDCB.Parity;
    }
    return COMPort::Parity();
}


//-----------------------------------------------------------------------------
COMPort::DataBits COMPort::dataBits () const
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        DCB & aDCB = *((LPDCB)_DCB);
        return (COMPort::DataBits)aDCB.ByteSize;
    }
    return COMPort::DataBits();
}


//-----------------------------------------------------------------------------
COMPort::StopBits COMPort::stopBits () const
{
    C_TRACE("");
    if (_portHandle != HFILE_ERROR)
    {
        DCB & aDCB = *((LPDCB)_DCB);
        return (COMPort::StopBits)aDCB.StopBits;
    }
    return COMPort::StopBits();
}

#endif
