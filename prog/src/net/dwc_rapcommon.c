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
#include "gflib.h"

#if GFL_NET_WIFI
#include <dwc.h>

#include "net/dwc_rapcommon.h"

//==============================================================================
/**
 * DWCライブラリ初期化
 * @param   GFL_WIFI_FRIENDLIST  ユーザーデータがない場合作成
 * @retval  DS本体に保存するユーザIDのチェック・作成結果。
 */
//==============================================================================
int mydwc_init(HEAPID heapID)
{
  int ret;
  u8* pWork;
  u8* pTemp;

  pWork = GFL_HEAP_AllocClearMemory(heapID, DWC_INIT_WORK_SIZE+32);
  pTemp = (u8 *)( ((u32)pWork + 31) / 32 * 32 );
  // DWCライブラリ初期化
  ret = DWC_Init( pTemp );
  GFL_HEAP_FreeMemory(pWork);

#ifdef PM_DEBUG
  DWC_SetAuthServer(DWC_CONNECTINET_AUTH_TEST);
#else
  DWC_SetAuthServer(DWC_CONNECTINET_AUTH_RELEASE);
#endif


  return ret;
}


//==============================================================================
/**
 * @brief   DWC  UserDataを作る
 * @param   DWCUserData  ユーザーデータがない場合作成
 * @retval  none
 */
//==============================================================================
void GFL_NET_WIFI_InitUserData( void *userdata )
{
  DWCUserData *pUserdata = userdata;

  // ユーザデータ作成をする。
  MI_CpuClearFast(userdata, sizeof(DWCUserData));
  if( !DWC_CheckUserData( userdata ) ){
    DWC_CreateUserData( userdata, 'ADAJ' );
    DWC_ClearDirtyFlag( userdata );
    NET_PRINT("WIFIユーザデータを作成\n");
  }
}

//==============================================================================
/**
 * 自分のGSIDが正しいのか検査する
 * @param   GFL_WIFI_FRIENDLIST
 * @retval  TRUEが成功
 */
//==============================================================================
BOOL mydwc_checkMyGSID(void)
{
  DWCUserData *userdata = GFI_NET_GetMyDWCUserData();

  GF_ASSERT(userdata);
  if( DWC_CheckHasProfile( userdata )
      && DWC_CheckValidConsole( userdata ) ){
    return TRUE;
  }
  return FALSE;
}

#endif GFL_NET_WIFI
