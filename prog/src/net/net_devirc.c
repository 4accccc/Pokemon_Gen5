//=============================================================================
/**
 * @file	net_devirc.c
 * @bfief	IRC通信ドライバー
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	08/12/22
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"

extern GFLNetDevTable *NET_GetIrcDeviceTable(void);


//--------------------------------------------
// 構造体
//--------------------------------------------

static GFLNetDevTable netDevTbl={
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

};


//------------------------------------------------------------------------------
/**
 * @brief   ワイヤレスデバイステーブルを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
GFLNetDevTable *NET_GetIrcDeviceTable(void)
{
    return &netDevTbl;
}
