//============================================================================================
/**
 * @file	net_icondata.c
 * @brief	通信アイコンデータ部分の定義
 * @author	ohno
 * @date	2007.05.29
 */
//============================================================================================

#include "gflib.h"
#include "arc/test_graphic/wmi.naix"
#include "net_icondata.h"


static	const	char	*GraphicFileTable[]={
    "test_graphic/wmi.narc",
};

//==============================================================================
/**
 * @brief    通信アイコングラフィックテーブルポインタを得る
 * @param    none
 * @return   グラフィックテーブルポインタ
 */
//==============================================================================

const char** NET_ICONDATA_GetTableData(void)
{
    return GraphicFileTable;
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
    pNoBuff[GFL_NET_ICON_WIFINCGR] = NARC_wmi_wifi_ncgr;
    pNoBuff[GFL_NET_ICON_WMNCGR] = NARC_wmi_wm_ncgr;
    pNoBuff[GFL_NET_ICON_WMNCLR] = NARC_wmi_wm_nclr;
}

