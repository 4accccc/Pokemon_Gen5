/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - SSL - libraries - stubs
  File:     ssl_stub.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-31#$
  $Rev: 84 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/
#include <nitroWiFi.h>

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPS_SetSsl(int mode)
{
#pragma unused(mode)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPS_SslAddRandomSeed(void *random_seed, u32 len)
{
#pragma unused(random_seed)
#pragma unused(len)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPS_SetRootCa(CPSCaInfo *ca[], int ca_num)
{
#pragma unused(ca)
#pragma unused(ca_num)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPS_SetMyCert(CPSCertificate *c, CPSPrivateKey *k)
{
#pragma unused(c)
#pragma unused(k)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPS_SetSslHandshakePriority(u32 new_prio)
{
#pragma unused(new_prio)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL u32 CPS_GetSslHandshakePriority(void)
{
return 0;
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPSi_SslListen(CPSSoc* soc)
{
#pragma unused(soc)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL u32 CPSi_SslConnect(CPSSoc* soc)
{
#pragma unused(soc)
    return 0;
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL u8* CPSi_SslRead(u32* len, CPSSoc* soc)
{
#pragma unused(len, soc)
    return NULL;
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPSi_SslConsume(u32 len, CPSSoc* soc)
{
#pragma unused(len, soc)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL s32 CPSi_SslGetLength(CPSSoc* soc)
{
#pragma unused(soc)
    return 0;
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL u32 CPSi_SslWrite2(u8* buf, u32 len, u8* buf2, u32 len2, CPSSoc* soc)
{
#pragma unused(buf, len, buf2, len2, soc)
    return 0;
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPSi_SslShutdown(CPSSoc* soc)
{
#pragma unused(soc)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPSi_SslClose(CPSSoc *soc)
{
#pragma unused(soc)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPSi_SslPeriodical(u32 now)
{
#pragma unused(now)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPSi_SslCleanup(void)
{
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPSi_SocConsumeRaw(u32 len, CPSSoc* soc)
{
#pragma unused(len, soc)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL u32 CPSi_TcpWrite2Raw(u8* buf, u32 len, u8* buf2, u32 len2, CPSSoc* soc)
{
#pragma unused(buf, len, buf2, len2, soc)
    return 0;
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL u32 CPSi_TcpConnectRaw(CPSSoc* soc)
{
#pragma unused(soc)
    return 0;
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPSi_TcpShutdownRaw(CPSSoc* soc)
{
#pragma unused(soc)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL void CPSi_TcpListenRaw(CPSSoc* soc)
{
#pragma unused(soc)
}

//----------------------------------------------------------------
SDK_WEAK_SYMBOL u8* CPSi_TcpReadRaw(u32* len, CPSSoc* soc)
{
#pragma unused(len, soc)
    return NULL;
}

//----------------------------------------------------------------
