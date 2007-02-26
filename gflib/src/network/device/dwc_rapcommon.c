//=============================================================================
/**
 * @file	dwc_rapcommon.c
 * @bfief	DWCラッパー。常駐モジュールに置くもの
 * @author	kazuki yoshihara  -> k.ohno改造
 * @date	06/02/23
 */
//=============================================================================

#include <nitro.h>
#include <nnsys.h>
#include <dwc.h>
#include "gf_macro.h"
#include "gf_standard.h"
#include "assert.h"
#include "heapsys.h"
#include "strbuf.h"
#include "net.h"
#include "../net_def.h"

#include "dwc_rapcommon.h"

//==============================================================================
/**
 * DWCライブラリ初期化
 * @param   GFL_WIFI_FRIENDLIST  ユーザーデータがない場合作成
 * @retval  DS本体に保存するユーザIDのチェック・作成結果。
 */
//==============================================================================
int mydwc_init(int heapID)
{
	int ret;
    u8* pWork;
    u8* pTemp;

    pWork = GFL_HEAP_AllocMemory(heapID, DWC_INIT_WORK_SIZE+32);
    pTemp = (u8 *)( ((u32)pWork + 31) / 32 * 32 );
    // DWCライブラリ初期化
    ret = DWC_Init( pTemp );
    GFL_HEAP_FreeMemory(pWork);

	return ret;
}


//==============================================================================
/**
 * DWC  UserDataを作る
 * @param   GFL_WIFI_FRIENDLIST  ユーザーデータがない場合作成
 * @retval  DS本体に保存するユーザIDのチェック・作成結果。
 */
//==============================================================================
void mydwc_createUserData( GFL_WIFI_FRIENDLIST *pWifiList )
{
    // ユーザデータ作成をする。    
    DWCUserData *userdata = WifiList_GetMyUserInfo(pWifiList);
    if( !DWC_CheckUserData( userdata ) ){
        DWC_CreateUserData( userdata, 'ADAJ' );
        DWC_ClearDirtyFlag( userdata );
        OS_TPrintf("WIFIユーザデータを作成\n");
    }
}

//==============================================================================
/**
 * 自分のGSIDを取得する
 * @param   GFL_WIFI_FRIENDLIST  
 * @retval  ０と－１は失敗   正の値は成功
 */
//==============================================================================

int mydwc_getMyGSID(GFL_WIFI_FRIENDLIST *pWifiList)
{
    DWCUserData *userdata = WifiList_GetMyUserInfo(pWifiList);
    DWCFriendData friendData;

    DWC_CreateExchangeToken(userdata,&friendData);
    return DWC_GetGsProfileId( userdata,&friendData);
}

//==============================================================================
/**
 * 自分のGSIDが正しいのか検査する
 * @param   GFL_WIFI_FRIENDLIST  
 * @retval  TRUEが成功
 */
//==============================================================================
#if _SAVE_PROGRAM
BOOL mydwc_checkMyGSID(SAVEDATA *pSV)
{
    GFL_WIFI_FRIENDLIST* pList = SaveData_GetWifiListData(pSV);
    DWCUserData *userdata = WifiList_GetMyUserInfo(pList);

    if( DWC_CheckHasProfile( userdata )
       && DWC_CheckValidConsole( userdata ) ){
       return TRUE;
   }
   return FALSE;
}
#endif //_SAVE_PROGRAM
