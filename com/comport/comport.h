//=============================================================================
//  General component library for WIN32
//  Copyright (C) 2000, UAB BBDSoft ( http://www.bbdsoft.com/ )
//
// This material is provided "as is", with absolutely no warranty expressed
// or implied. Any use is at your own risk.
//
// Permission to use or copy this software for any purpose is hereby granted 
// without fee, provided the above notices are retained on all copies.
// Permission to modify the code and to distribute modified code is granted,
// provided the above notices are retained, and a notice that the code was
// modified is included with the above copyright notice.
//
//  The author of this program may be contacted at developers@bbdsoft.com
//
//  Changes:
//    Valentin Heinitz, removed dependancy to windows in header
//    Valentin Heinitz, added convenience function setNonBlockingMode                     
//    
//=============================================================================


#ifndef _COMPORT_H_INC_CL2_HG
#define _COMPORT_H_INC_CL2_HG
#if 0

#include <wchar.h>
#include <map>
#include <list>
#include <string>
#include <cldef.h>

typedef std::pair<std::string, std::string> TSerialInterfaceInfo;
typedef std::list< TSerialInterfaceInfo > TSerialInterfaceInfoList;
//-----------------------------------------------------------------------------
class CLIB_EXPORT COMPort
{
public:

    enum Parity
    {
         None = 0
       , Odd
       , Even
       , Mark
       , Space
    };

    enum DataBits
    {
         db4 = 4
       , db5
       , db6
       , db7
       , db8
    };

    enum StopBits
    {
       sb1 = 0,
       sb15,
       sb2
    };

    enum BitRate
    {
       br110 = 110,
       br300 = 300,
       br600 = 600,
       br1200 = 1200,
       br2400 = 2400,
       br4800 = 4800,
       br9600 = 9600,
       br19200 = 19200,
       br38400 = 38400,
       br56000 = 56000,
       br57600 = 57600,
       br115200 = 115200,
       br256000 = 256000
    };


    // for function getModemSignals
    struct MSPack
    {
      unsigned char DTR : 1;
      unsigned char RTS : 1;
      unsigned char     : 2;
      unsigned char CTS : 1;
      unsigned char DSR : 1;
      unsigned char RI  : 1;
      unsigned char DCD : 1;
    };

    COMPort ( const char * portName );
    COMPort ( );
    bool open ( const char * portName );
    void close();
    ~COMPort ();
    static TSerialInterfaceInfoList availableInterfaces();


    // I/O operations
    char read ();
    COMPort & write (char inChar);

    unsigned long read  ( void *
                        , unsigned long count
                        );

    unsigned long write ( const void *
                        , unsigned long count
                        );


    COMPort& setBitRate ( unsigned long Param );
    unsigned long bitRate () const;

    COMPort& setParity ( Parity Param );
    Parity parity () const;

    COMPort& setDataBits ( DataBits Param );
    DataBits dataBits () const;

    COMPort& setStopBits ( StopBits Param );
    StopBits stopBits () const;

    COMPort & setHandshaking ( bool inHandshaking = true );

    COMPort& setLineCharacteristics ( const wchar_t * Param );

    unsigned long getMaximumBitRate () const;

    COMPort & setxONxOFF ( bool Param = true);
    bool isxONxOFF () const;

    MSPack getModemSignals () const;

    void setBlockingMode ( unsigned long inReadInterval  = 0
                             , unsigned long inReadMultiplyer = 0
                             , unsigned long inReadConstant = 0
                             , unsigned long inWriteConstant = 0
                             , unsigned long inWriteMult = 0
                             );

    void setNonBlockingMode ( );
    void purge ( );

protected:

private:

   // disable copy constructor and assignment operator
   COMPort (const COMPort &);
   COMPort& operator= (const COMPort &);

   void getState () const;
   COMPort& setState ();

   static int _portHandle;
   char * _DCB;
   std::string _portName;

}; // End of COMPort class declaration

#endif

#endif
