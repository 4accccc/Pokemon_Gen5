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

typedef struct{
	NNSFndHeapHandle headHandle;
  void *heapPtr;
} DWCRAPCOMMON_WORK;

static DWCRAPCOMMON_WORK* pDwcRapWork;

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
void* DWC_RAPCOMMON_Alloc( DWCAllocType name, u32 size, int align )
{	
#pragma unused( name )

  void *ptr;
  OSIntrMode  enable = OS_DisableInterrupts();
  ptr = NNS_FndAllocFromExpHeapEx( pDwcRapWork->headHandle, size, align );
  (void)OS_RestoreInterrupts(enable);

  if(ptr == NULL){
    GF_ASSERT_MSG(ptr,"dwcalloc not allocate! size %d,align %d rest %d\n", size, align, NNS_FndGetTotalFreeSizeForExpHeap(pDwcRapWork->headHandle) );
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
void DWC_RAPCOMMON_Free( DWCAllocType name, void *ptr, u32 size )
{	
#pragma unused( name, size )

  if ( !ptr ){
    return;  //NULL開放を認める
  }
  {
    OSIntrMode  enable = OS_DisableInterrupts();
    NNS_FndFreeToExpHeap( pDwcRapWork->headHandle, ptr );
    (void)OS_RestoreInterrupts(enable);
  }
}

//==============================================================================
/**
 * @brief   WIFIで使うHEAPIDをセットする
 * @param   id     変更するHEAPID
 * @retval  none
 */
//==============================================================================

void DWC_RAPCOMMON_SetHeapID(HEAPID id,int size)
{
  GF_ASSERT(pDwcRapWork==NULL);

  pDwcRapWork=GFL_HEAP_AllocClearMemory(GFL_HEAPID_SYSTEM,sizeof(DWCRAPCOMMON_WORK));
  pDwcRapWork->heapPtr = GFL_HEAP_AllocMemory(id, size-0x80);
  pDwcRapWork->headHandle = NNS_FndCreateExpHeap( (void *)( ((u32)pDwcRapWork->heapPtr + 31) / 32 * 32 ), size-0x80-64);

  
}

//==============================================================================
/**
 * @brief   WIFIで使うHEAPIDをセットする
 * @param   id     変更するHEAPID
 * @retval  none
 */
//==============================================================================

void DWC_RAPCOMMON_ResetHeapID(void)
{
  GF_ASSERT(pDwcRapWork);
  NNS_FndDestroyExpHeap( pDwcRapWork->headHandle );
  GFL_HEAP_FreeMemory( pDwcRapWork->heapPtr  );
  GFL_HEAP_FreeMemory(pDwcRapWork);
  pDwcRapWork=NULL;
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

  DWC_RAPCOMMON_SetHeapID(heapID, 0x40000);

  // DWCライブラリ初期化
#ifdef DEBUG_SERVER
	ret	= DWC_InitForDevelopment( GF_DWC_GAMENAME, GF_DWC_GAMECODE, DWC_RAPCOMMON_Alloc, DWC_RAPCOMMON_Free );
#else
	ret	= DWC_InitForProduction( GF_DWC_GAMENAME, GF_DWC_GAMECODE, DWC_RAPCOMMON_Alloc, DWC_RAPCOMMON_Free );
#endif

  DWC_RAPCOMMON_ResetHeapID();

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
