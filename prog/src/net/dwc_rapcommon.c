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

#define NET_MEMORY_DEBUG  (0)

#else // GFL_NET_DEBUG

#define NET_MEMORY_DEBUG  (0)

#endif// GFL_NET_DEBUG 


#if NET_MEMORY_DEBUG
#define NET_MEMORY_PRINT(...) \
  if(GFL_NET_DebugGetPrintOn()) (void) ((OS_TPrintf(__VA_ARGS__)))
#else   //NET_MEMORY_DEBUG
#define NET_MEMORY_PRINT(...)           ((void) 0)
#endif  // NET_MEMORY_DEBUG

#define DWCRAPCOMMON_SUBHEAP_GROUPID  0xea

typedef struct{
	NNSFndHeapHandle headHandle;
  void *heapPtr;

  NNSFndHeapHandle headHandleSub; //サブヒープ
  void          *heapPtrSub;
  DWCAllocType  SubAllocType;  //サブヒープから確保するタイプ
  u32           subheap_alloc_cnt;  //メモリ取得数
  u32           subheap_border_size; //
  BOOL          is_leak_err;
  HEAPID        auto_release_subheapID;
} DWCRAPCOMMON_WORK;

static DWCRAPCOMMON_WORK* pDwcRapWork = NULL;

static void DWC_RAPCOMMON_ResetSubHeapIDCore(void);
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
  DWC_RAPCOMMON_SetSubHeapIDEx( SubAllocType, size, 0, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  DWCのサブヒープを作成 取得サイズに上限設定をつける版
 *	        nameがDWC_ALLOCTYPE_GSであってもマッチメイクとSCライブラリの
 *	        ２つが来る場合がある。
 *	        ここ無理やり切り分けるためにSCライブラリで大きいサイズが来たときのみ
 *	        ヒープを切り分ける
 *	        そのために上限設定をできる関数を用意しました
 *
 *	@param	DWCAllocType SubAllocType このヒープでからアロケートするタイプ
 *	@param	size                      サブヒープのサイズ
 *	@param  border                    この値以上のアロケートならば切り分ける
 *	@param	heapID                    ヒープID
 */
//-----------------------------------------------------------------------------
void DWC_RAPCOMMON_SetSubHeapIDEx( DWCAllocType SubAllocType, u32 size, u32 border_size, HEAPID heapID )
{
  GF_ASSERT_HEAVY( pDwcRapWork != NULL );
  GF_ASSERT_HEAVY( pDwcRapWork->heapPtrSub == NULL );
  GF_ASSERT_HEAVY( pDwcRapWork->headHandleSub == NULL );
  pDwcRapWork->heapPtrSub    = GFL_HEAP_AllocMemory(heapID, size-0x80);
  pDwcRapWork->headHandleSub = NNS_FndCreateExpHeap( (void *)( ((u32)pDwcRapWork->heapPtrSub + 31) / 32 * 32 ), size-0x80-64); 
  pDwcRapWork->SubAllocType = SubAllocType;

  NNS_FndSetAllocModeForExpHeap( pDwcRapWork->headHandleSub, NNS_FND_EXPHEAP_ALLOC_MODE_NEAR );
  NNS_FndSetGroupIDForExpHeap(
	      pDwcRapWork->headHandleSub,DWCRAPCOMMON_SUBHEAP_GROUPID );

  pDwcRapWork->subheap_border_size  = border_size;
}

//----------------------------------------------------------------------------
/**
 *	@brief  DWCのサブヒープを破棄
 */
//-----------------------------------------------------------------------------
void DWC_RAPCOMMON_ResetSubHeapID(void)
{
  GF_ASSERT_HEAVY(pDwcRapWork);

  //もし破棄した段階でメモリが残っていればエラーにする
  if( pDwcRapWork->subheap_alloc_cnt != 0 )
  {
    NAGI_Printf( "！！ヒープ強制エラー！！\n" );
    GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_HEAP );


    //ランダムマッチで、レポートの処理をサブヒープで切り分けています
    //切り分けているときにリークが起こる場合
    //マッチメイクのものがサブヒープに残っている可能性が高いです。
    //ですので、リークが起こった場合はすぐに解放処理へ行かず、
    //Wi-Fiの終了時にメインヒープとまとめてクリアします
    pDwcRapWork->is_leak_err  = TRUE;

    //GF_ASSERT_MSG_HEAVY( 0, "SUB HEAP ERROR! LEAK!! count=%d\n", pDwcRapWork->subheap_alloc_cnt );
  }
  else
  {
    DWC_RAPCOMMON_ResetSubHeapIDCore();
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  DWCのサブヒープ解放コア部分
 */
//-----------------------------------------------------------------------------
static void DWC_RAPCOMMON_ResetSubHeapIDCore(void)
{
  if( pDwcRapWork->heapPtrSub )
  {
    NNS_FndDestroyExpHeap( pDwcRapWork->headHandleSub );
    GFL_HEAP_FreeMemory( pDwcRapWork->heapPtrSub );
    pDwcRapWork->headHandleSub  = NULL;
    pDwcRapWork->heapPtrSub = NULL;

    if( pDwcRapWork->auto_release_subheapID != DWC_RAPCOMMON_AUTORELEASE_HEAPID_NONE )
    {
      GFL_HEAP_DeleteHeap( pDwcRapWork->auto_release_subheapID );
      pDwcRapWork->auto_release_subheapID = DWC_RAPCOMMON_AUTORELEASE_HEAPID_NONE;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  サブヒープのReset時にヒープのデリートをする場合の設定
 *
 *	@param	HEAPID heapID ヒープID
 */
//-----------------------------------------------------------------------------
void DWC_RAPCOMMON_SetAutoDeleteSubHeap( HEAPID heapID )
{
  pDwcRapWork->auto_release_subheapID = heapID;
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

  //サブヒープが登録されていてかつ、指定したアロケートタイプ、かつ指定したサイズ以上ならば
  if( (pDwcRapWork->headHandleSub != NULL ) 
      && (name == pDwcRapWork->SubAllocType ) 
      && (size > pDwcRapWork->subheap_border_size ) )
  { 
    {
    OSIntrMode  enable = OS_DisableInterrupts();
    ptr = NNS_FndAllocFromExpHeapEx( pDwcRapWork->headHandleSub, size, align );
    (void)OS_RestoreInterrupts(enable);
    NET_MEMORY_PRINT( "sub alloc memory size=%d rest=%d %d\n", size, NNS_FndGetTotalFreeSizeForExpHeap(pDwcRapWork->headHandleSub),name );
    }
    pDwcRapWork->subheap_alloc_cnt++;
    NAGI_Printf( "sub heap alloc cnt=%d size=%d\n", pDwcRapWork->subheap_alloc_cnt, size );
  }
  else
  { 
    OSIntrMode  enable = OS_DisableInterrupts();
    ptr = NNS_FndAllocFromExpHeapEx( pDwcRapWork->headHandle, size, align );
    (void)OS_RestoreInterrupts(enable);

    NET_MEMORY_PRINT( "main alloc memory size=%d rest=%d %d\n", size, NNS_FndGetTotalFreeSizeForExpHeap(pDwcRapWork->headHandle),name );
  }

  //残りヒープ調査
#ifdef DEBUG_ONLY_FOR_toru_nagihashi
  /*
  { 
    static u32 s_wifi_buff  = 0xFFFFFFFF;
    u32 free_area;
    free_area  = NNS_FndGetTotalFreeSizeForExpHeap( pDwcRapWork->headHandle );
    if( s_wifi_buff > free_area )
    { 
      s_wifi_buff = free_area;
    }
    NET_MEMORY_PRINT( "HEAP[WIFI] most low free area rest=0x%x\n", s_wifi_buff );
  }
  */
#endif //DEBUG_ONLY_FOR_toru_nagihashi


  if(ptr == NULL){  // ヒープが無い場合
//    GF_ASSERT_MSG_HEAVY(ptr,"dwcalloc not allocate! size %d,align %d rest %d name %d\n", size, align, NNS_FndGetTotalFreeSizeForExpHeap(pDwcRapWork->headHandle), name );
//    GFL_NET_StateSetError(GFL_NET_ERROR_RESET_SAVEPOINT);
//    returnNo = ERRORCODE_HEAP;
#ifdef BUGFIX_BTS7819_20100715
    GF_PANIC("%d\n",size);
    //DWCLIB割り込み途中にHEAPがなくなる症状がでたため。エラーを出す事もできなかったのでPANICしにしました
    //事前にエラー画面を用意しておくとか、ライブラリを変えてもらうとか、メモリを増やすとか DUMMYをつかませてメインでエラーとか
    //抜本的、または奇抜な対処が必要です
#endif //BUGFIX_BTS7819_20100715
    GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_HEAP );  //エラーになる
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

  //NAGI_Printf( "[>name%d ptr%x size%d\n", name, ptr, size );

  if ( !ptr ){
    return;  //NULL開放を認める
  }
  NET_MEMORY_PRINT( "free memory size=%d\n", size );

  //サブヒープが登録されていてかつ、指定したアロケートタイプだったら
//  if( (pDwcRapWork->headHandleSub != NULL ) &&  (name == pDwcRapWork->SubAllocType ) )
  if( DWCRAPCOMMON_SUBHEAP_GROUPID == NNS_FndGetGroupIDForMBlockExpHeap(ptr) )
  { 
    if( pDwcRapWork->headHandleSub != NULL )
    {
      OSIntrMode  enable = OS_DisableInterrupts();
      NNS_FndFreeToExpHeap( pDwcRapWork->headHandleSub, ptr );
      (void)OS_RestoreInterrupts(enable);
      pDwcRapWork->subheap_alloc_cnt--;
      NAGI_Printf( "sub_heap alloc cnt=%d\n", pDwcRapWork->subheap_alloc_cnt );
    }
    else
    {
      //もしサブヒープが存在していないのに解放しにきた場合は
      //ヒープエラーとしてしまう
      NAGI_Printf( "！！ヒープ強制エラー！！\n" );
      GFL_NET_StateSetWifiError( 0, 0, 0, ERRORCODE_HEAP );

      //GF_ASSERT_MSG_HEAVY( 0, "SUB HEAP ERROR! NOT_SUBHEAP!! name=%d ptr=%x size=%d \n", name, ptr, size );
    }
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
  GF_ASSERT_HEAVY(pDwcRapWork==NULL);

  pDwcRapWork=GFL_HEAP_AllocClearMemory(GFL_HEAPID_SYSTEM,sizeof(DWCRAPCOMMON_WORK));
  pDwcRapWork->heapPtr = GFL_HEAP_AllocMemory(id, size-0x80);
  pDwcRapWork->headHandle = NNS_FndCreateExpHeap( (void *)( ((u32)pDwcRapWork->heapPtr + 31) / 32 * 32 ), size-0x80-64);

  pDwcRapWork->auto_release_subheapID = DWC_RAPCOMMON_AUTORELEASE_HEAPID_NONE;
  
}

//==============================================================================
/**
 * @brief   WIFIで使うHEAPIDをリセットする
 * @param   id     変更するHEAPID
 * @retval  none
 */
//==============================================================================

void DWC_RAPCOMMON_ResetHeapID(void)
{
  GF_ASSERT_HEAVY(pDwcRapWork);

  if( pDwcRapWork->is_leak_err == TRUE
      || pDwcRapWork->heapPtrSub != NULL )
  {
    DWC_RAPCOMMON_ResetSubHeapIDCore();
  }

  NNS_FndDestroyExpHeap( pDwcRapWork->headHandle );
  GFL_HEAP_FreeMemory( pDwcRapWork->heapPtr );
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

  GF_ASSERT_HEAVY(userdata);
  if( DWC_CheckHasProfile( userdata )
      && DWC_CheckValidConsole( userdata ) ){
    return TRUE;
  }
  return FALSE;
}

#endif //GFL_NET_WIFI
