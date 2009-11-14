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
#include "net/network_define.h"
#include "system/main.h"

#if GFL_NET_WIFI
#include <dwc.h>

#include "net/dwc_rapcommon.h"

static HEAPID sc_heapID	= 0;

//----------------------------------------------------------------------------
/**
 *	@brief	DWCライブラリ初期化に渡すアロケート関数
 *
 *	@param	DWCAllocType name	無視情報
 *	@param	size							必要なサイズ
 *	@param	align							必要なバイトアライメント
 *
 *	@return	確保成功した場合、アライメントされたバッファ。　失敗した場合NULL
 */
//-----------------------------------------------------------------------------
static void* mydwc_alloc( DWCAllocType name, u32 size, int align )
{	
#pragma unused( name )

	void * ptr;
  OSIntrMode old;

#ifdef DEBUGPRINT_ON
  NET_PRINT("HEAP取得(%d, %d) \n", size, align);
#endif

	GF_ASSERT_MSG( sc_heapID != 0, "DWC＿Alloc関数がヒープID設定されずに呼ばれました" );
	GF_ASSERT(align <= 32);  // これをこえたら再対応
	old = OS_DisableInterrupts();
  ptr = GFL_NET_Align32Alloc(sc_heapID, size);
	OS_RestoreInterrupts( old );

  if(ptr == NULL){
    GF_ASSERT(ptr);
    // ヒープが無い場合の修正
    GFL_NET_StateSetError(GFL_NET_ERROR_RESET_SAVEPOINT);
    return NULL;
  }
  return ptr;


}

//----------------------------------------------------------------------------
/**
 *	@brief	DWCライブラリ初期化に渡すフリー関数
 *
 *	@param	DWCAllocType name	無視する情報
 *	@param	ptr								メモリ確保関数で確保されたバッファのポインタ
 *	@param	size							無視する情報
 */
//-----------------------------------------------------------------------------
static void mydwc_free( DWCAllocType name, void *ptr, u32 size )
{	
#pragma unused( name, size )
  OSIntrMode old;

  if ( !ptr ){
    return;  //NULL開放を認める
  }
  old = OS_DisableInterrupts();
  GFL_NET_Align32Free(ptr);
  OS_RestoreInterrupts( old );
}

//==============================================================================
/**
 * @brief   WIFIで使うHEAPIDをセットする
 * @param   id     変更するHEAPID
 * @retval  none
 */
//==============================================================================

void DWC_RAPCOMMON_SetHeapID(HEAPID id)
{
  sc_heapID = id;
}

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

	sc_heapID	= heapID;

  // DWCライブラリ初期化
#ifdef DEBUG_SERVER
	ret	= DWC_InitForDevelopment( GF_DWC_GAMENAME, GF_DWC_GAMECODE, mydwc_alloc, mydwc_free );
#else
	ret	= DWC_InitForProduction( GF_DWC_GAMENAME, GF_DWC_GAMECODE, mydwc_alloc, mydwc_free );
#endif

  sc_heapID = 0;

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
    DWC_CreateUserData( userdata );
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
