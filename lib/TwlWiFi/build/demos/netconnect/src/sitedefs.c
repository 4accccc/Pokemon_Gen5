/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - demos - netconnect
  File:     sitedefs.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-20#$
  $Rev: 1071 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nitroWiFi.h>
#include "sitedefs.h"

/* *INDENT-OFF* */
static ENVResource myResources[] =
{
    //-----------------------------
    // LAN
    //-----------------------------

    // LAN:Access Point 1
    // Demos in TwlWiFi use this settings by default.
    { "WiFi.LAN.1.AP.1.ISVALID",    ENV_BOOL  ( TRUE )                       },
    { "WiFi.LAN.1.AP.1.DESC",       ENV_STRING( "Access Point for Demos" )   },
    { "WiFi.LAN.1.AP.1.ROUTERMODE", ENV_BOOL  ( TRUE )                       },
    { "WiFi.LAN.1.AP.1.ESSID",      ENV_STRING( "AP_for_Nitro" )             },
    { "WiFi.LAN.1.AP.1.BSSID",      ENV_BINARY( "\xff\xff\xff\xff\xff\xff" ) },
    { "WiFi.LAN.1.AP.1.AUTHMODE",   ENV_U32   ( WCM_OPTION_AUTH_OPENSYSTEM ) },
    { "WiFi.LAN.1.AP.1.WEP.MODE",   ENV_U8    ( WCM_WEPMODE_NONE )           },
    { "WiFi.LAN.1.AP.1.WEP.KEYID",  ENV_U8    ( 0 )                          },
    { "WiFi.LAN.1.AP.1.WEP.KEY",
        ENV_BINARY("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                   "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                   "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                   "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                   "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                   "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                   "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                   "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00") },

    // HOST with fixed address on the LAN.1.AP.1
    { "WiFi.LAN.1.AP.1.HOST.1.ISVALID",     ENV_BOOL  ( TRUE )              },
    { "WiFi.LAN.1.AP.1.HOST.1.NAME",        ENV_STRING( "192.168.2.106"  )  },
    { "WiFi.LAN.1.AP.1.HOST.1.NETMASK",     ENV_STRING( "255.255.255.0"  )  },
    { "WiFi.LAN.1.AP.1.HOST.1.GATEWAY",     ENV_STRING( "192.168.2.1"    )  },
    { "WiFi.LAN.1.AP.1.HOST.1.DNS1",        ENV_STRING( "192.168.2.1"    )  },
    { "WiFi.LAN.1.AP.1.HOST.1.DNS2",        ENV_STRING( "0.0.0.0"        )  },

    // LAN:HTTP Server
    { "WiFi.LAN.1.HTTP.1.ISVALID",   ENV_BOOL  ( TRUE )                     },
    { "WiFi.LAN.1.HTTP.1.DESC",      ENV_STRING( "linux server" )           },
    { "WiFi.LAN.1.HTTP.1.NAME",      ENV_STRING( "192.168.2.2" )            },
    { "WiFi.LAN.1.HTTP.1.PORT.1",    ENV_U16   ( 80 )                       },

    // LAN:ECHO Server
    { "WiFi.LAN.1.ECHO.1.ISVALID",   ENV_BOOL  ( TRUE )                     },
    { "WiFi.LAN.1.ECHO.1.DESC",      ENV_STRING( "linux server" )           },
    { "WiFi.LAN.1.ECHO.1.NAME",      ENV_STRING( "192.168.2.2" )            },
    { "WiFi.LAN.1.ECHO.1.PORT.1",    ENV_U16   ( 7 )                        },

    // LAN:CHARGEN Server
    { "WiFi.LAN.1.CHARGEN.1.ISVALID",ENV_BOOL  ( TRUE )                     },
    { "WiFi.LAN.1.CHARGEN.1.DESC",   ENV_STRING( "linux server" )           },
    { "WiFi.LAN.1.CHARGEN.1.NAME",   ENV_STRING( "192.168.2.2" )            },
    { "WiFi.LAN.1.CHARGEN.1.PORT.1", ENV_U16   ( 19 )                       },

    // LAN:Ping Taret
    { "WiFi.LAN.1.PING.1.ISVALID",   ENV_BOOL  ( TRUE )                     },
    { "WiFi.LAN.1.PING.1.DESC",      ENV_STRING( "default gateway" )        },
    { "WiFi.LAN.1.PING.1.NAME",      ENV_STRING( "192.168.2.1" )            },

    // LAN:FTP Server
    { "WiFi.LAN.1.FTP.1.ISVALID",    ENV_BOOL  ( TRUE )                     },
    { "WiFi.LAN.1.FTP.1.DESC",       ENV_STRING( "linux server" )           },
    { "WiFi.LAN.1.FTP.1.NAME",       ENV_STRING( "192.168.2.2" )            },
    { "WiFi.LAN.1.FTP.1.PORT.1",     ENV_U16   ( 21 )                       },

    //---- end mark
    ENV_RESOURCE_END,
};

ENVResource* resourceArray[]=
{
   myResources, NULL
};
/* *INDENT-ON* */

/*---------------------------------------------------------------------------*
  Name:         SiteDefs_Init

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SiteDefs_Init(void)
{
    //---- declaration of using ENV system
    //SDK と WIFI の両方に ENV ライブラリが入っているため、以下のように切り替える
    //SDKがENVの旧versionを持っているなら、それに合わせて ENV_Init は昔版で。
#if (SDK_VERSION_DATE <= 20050918) && (SDK_VERSION_DATE >= 20050908)
    ENV_Init(myResources);
#else
    //SDKにENVの旧versionは含まれていないので、それに合わせて ENV_Init は改良版で。
    ENV_Init();
#endif
}

/*====== End of sitedefs.c ======*/
