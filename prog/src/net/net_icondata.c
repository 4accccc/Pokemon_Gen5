//============================================================================================
/**
 * @file	net_icondata.c
 * @brief	通信アイコンデータ部分の定義 gflibに置けないので外部に定義してある
 * @author	ohno
 * @date	2007.05.29
 */
//============================================================================================

#include "gflib.h"
#include "wmi.naix"
#include "net_icondata.h"
#include "net/network_define.h"
#include "arc_def.h"


//==============================================================================
/**
 * @brief    通信アイコングラフィックARCIDを得る
 * @param    none
 * @return   グラフィックARCID
 */
//==============================================================================

const u32 NET_ICONDATA_GetTableID(void)
{
    return ARCID_WMI;
}


//==============================================================================
/**
 * @brief    通信アイコングラフィックNOを得る
 * @param    pNoBuff  グラフィック番号を返す配列
 * @return   none
 */
//==============================================================================

void NET_ICONDATA_GetNoBuff(int* pNoBuff)
{
  pNoBuff[GFL_NET_ICON_WIFINCGR] = NARC_wmi_wifi_NCGR;
  pNoBuff[GFL_NET_ICON_WMNCGR] = NARC_wmi_wm_NCGR;
  pNoBuff[GFL_NET_ICON_WMNCLR] = NARC_wmi_wm_NCLR;
}

