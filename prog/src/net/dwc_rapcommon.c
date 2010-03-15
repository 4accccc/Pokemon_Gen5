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

 

// メモリの残量表示処理 1:ON  0:OFF
#if GFL_NET_DEBUG

#define NET_MEMORY_DEBUG  (1)

#else // GFL_NET_DEBUG

#define NET_MEMORY_DEBUG  (1)

#endif// GFL_NET_DEBUG 


#if NET_MEMORY_DEBUG
#define NET_MEMORY_PRINT(...) \
  if(GFL_NET_DebugGetPrintOn()) (void) ((OS_TPrintf(__VA_ARGS__)))
#else   //NET_MEMORY_DEBUG
#define NET_MEMORY_PRINT(...)           ((void) 0)
#endif  // NET_MEMORY_DEBUG


typedef struct{
	NNSFndHeapHandle headHandle;
  void *heapPtr;

  NNSFndHeapHandle headHandleSub; //サブヒープ
  void          *heapPtrSub;
  DWCAllocType  SubAllocType;  //サブヒープから確保するタイプ
} DWCRAPCOMMON_WORK;

static DWCRAPCOMMON_WORK* pDwcRapWork = NULL;

//----------------------------------------------------------------------------
/**
 *	@brief  DWCのサブヒープを作成
 *	        （サブヒープは特定のタイプのみアロケートさせるものです
 *	        　現状、WIFI大会とランダムマッチレーティングで使用しております）
 *
 *	@param	DWCAllocType SubAllocType このヒープでからアロケートするタイプ
 *	@param	size                      サブヒープのサイズ
 *	@param	heapID                    ヒープID
 */
//-----------------------------------------------------------------------------
void DWC_RAPCOMMON_SetSubHeapID( DWCAllocType SubAllocType, u32 size, HEAPID heapID )
{ 
  GF_ASSERT( pDwcRapWork != NULL );
  GF_ASSERT( pDwcRapWork->heapPtrSub == NULL );
  GF_ASSERT( pDwcRapWork->headHandleSub == NULL );
  pDwcRapWork->heapPtrSub    = GFL_HEAP_AllocMemory(heapID, size-0x80);
  pDwcRapWork->headHandleSub = NNS_FndCreateExpHeap( (void *)( ((u32)pDwcRapWork->heapPtrSub + 31) / 32 * 32 ), size-0x80-64); 
  pDwcRapWork->SubAllocType = SubAllocType;
}

//----------------------------------------------------------------------------
/**
 *	@brief  DWCのサブヒープを破棄
 */
//-----------------------------------------------------------------------------
void DWC_RAPCOMMON_ResetSubHeapID(void)
{
  GF_ASSERT(pDwcRapWork);
  NNS_FndDestroyExpHeap( pDwcRapWork->headHandleSub );
  GFL_HEAP_FreeMemory( pDwcRapWork->heapPtrSub );
  pDwcRapWork->headHandleSub  = NULL;
  pDwcRapWork->heapPtrSub = NULL;
}


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

  void *ptr;

  //サブヒープが登録されていてかつ、指定したアロケートタイプだったら
  if( (pDwcRapWork->headHandleSub != NULL ) &&  (name == pDwcRapWork->SubAllocType ) )
  { 
    OSIntrMode  enable = OS_DisableInterrupts();
    ptr = NNS_FndAllocFromExpHeapEx( pDwcRapWork->headHandleSub, size, align );
    (void)OS_RestoreInterrupts(enable);
    NET_MEMORY_PRINT( "sub alloc memory size=%d rest=%d\n", size, NNS_FndGetTotalFreeSizeForExpHeap(pDwcRapWork->headHandleSub) );
  }
  else
  { 
    OSIntrMode  enable = OS_DisableInterrupts();
    ptr = NNS_FndAllocFromExpHeapEx( pDwcRapWork->headHandle, size, align );
    (void)OS_RestoreInterrupts(enable);

    NET_MEMORY_PRINT( "main alloc memory size=%d rest=%d\n", size, NNS_FndGetTotalFreeSizeForExpHeap(pDwcRapWork->headHandle) );
  }

  NAGI_Printf( "[>name%d size%d align%d\n", name, size, align );

  if(ptr == NULL){
    GF_ASSERT_MSG(ptr,"dwcalloc not allocate! size %d,align %d rest %d name %d\n", size, align, NNS_FndGetTotalFreeSizeForExpHeap(pDwcRapWork->headHandle), name );
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

  NAGI_Printf( "[>name%d ptr%x size%d\n", name, ptr, size );

  if ( !ptr ){
    return;  //NULL開放を認める
  }
  NET_MEMORY_PRINT( "free memory size=%d\n", size );


  //サブヒープが登録されていてかつ、指定したアロケートタイプだったら
  if( (pDwcRapWork->headHandleSub != NULL ) &&  (name == pDwcRapWork->SubAllocType ) )
  { 
    OSIntrMode  enable = OS_DisableInterrupts();
    NNS_FndFreeToExpHeap( pDwcRapWork->headHandleSub, ptr );
    (void)OS_RestoreInterrupts(enable);
  }
  else
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
  GF_ASSERT( pDwcRapWork->heapPtrSub == NULL );
  GF_ASSERT( pDwcRapWork->headHandleSub == NULL );

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
