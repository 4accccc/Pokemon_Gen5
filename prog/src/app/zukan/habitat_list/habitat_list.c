//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_list.c
 *	@brief  生息地リスト：リスト画面
 *	@author	Masayuki Onoue
 *	@date		2011.08.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

#include "habitat_list.h"
#include "habitat_list_main.h"
#include "habitat_list_main_def.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "app/zukan.h"

#include "arc_def.h"              // ARC_ID
#include "message.naix"           // MESSAGE_ARC_ID

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

// プロセス-------------------
static GFL_PROC_RESULT ZukanHabitatList_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanHabitatList_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanHabitatList_Exit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

// VBlank-------------------
static void ZukanHabitatList_VBlankInit( HABITAT_LIST_WK *p_wk );
static void ZukanHabitatList_VBlankExit( HABITAT_LIST_WK *p_wk );
static void ZukanHabitatList_VBlank( GFL_TCB* tcb, void* wk );

// HBlank-------------------
static void ZukanHabitatList_HBlankInit( HABITAT_LIST_WK *p_wk );
static void ZukanHabitatList_HBlankExit( HABITAT_LIST_WK *p_wk );
static void ZukanHabitatList_HBlank( GFL_TCB* tcb, void* wk );

// ワーク関連 --------------
static HABITAT_LIST_WK* CreateWkData( GFL_PROC * proc ,ZUKANHABITAT_LIST_DATA *p_list_info );
static void DeleteWkData( GFL_PROC * proc , HABITAT_LIST_WK *p_wk );

// リスト関連 --------------
static void MakeFrameListData( HABITAT_LIST_WK *p_wk );
static void DeleteFrameListData( HABITAT_LIST_WK *p_wk );

// プロセスデータ
const GFL_PROC_DATA ZUKANHABITAT_LIST_ProcData = {
  ZukanHabitatList_Init,
  ZukanHabitatList_Main,
  ZukanHabitatList_Exit,
};


//----------------------------------------------------------------------------
/**
 *	@brief  プロセス初期化
 *
 *	@param	* proc
 *	@param	* seq
 *	@param	* pwk
 *	@param	* mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT ZukanHabitatList_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  HABITAT_LIST_WK *p_wk;
  ZUKANHABITAT_LIST_DATA *p_list_info = pwk;
  
  // プロセスワークを取得
  p_wk = mywk;

  switch( *seq )
  {
  case 0:
    {
      // プロセスワークを生成
      GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_HABITAT_LIST, 0x60000 );

      // ワークの生成
      p_wk = CreateWkData( proc ,p_list_info );

      // サブシーケンスを進める
      (*seq)++;
    }
    break;
  case 1:
    {
      // グラフィック初期化
      HABITAT_LIST_GRA_Init( &p_wk->gra_sys ,p_wk->p_init_param->p_gdata );

      // Vブランクタスク初期化
      ZukanHabitatList_VBlankInit( p_wk );

      // Hブランクタスク初期化
      ZukanHabitatList_HBlankInit( p_wk );
      
      // リストデータの作成
      MakeFrameListData( p_wk );
      (*seq)++;
    }
    break;
  case 2:
    {
      // フレームリスト初期化
      if( HABITAT_LIST_GRA_FrameListInit( &p_wk->gra_sys ) == TRUE )
      {
        return GFL_PROC_RES_FINISH;
      }
      else
      {
        ONOUE_Printf("フレームリスト初期化中\n");
      }
    }
    break;
  }
  
  return GFL_PROC_RES_CONTINUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief  プロセスメイン
 *
 *	@param	* proc
 *	@param	* seq
 *	@param	* pwk
 *	@param	* mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT ZukanHabitatList_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  HABITAT_LIST_WK *p_wk = mywk;

  // メイン関数を呼び出す
  if( HABITAT_LIST_Main( p_wk ) )
  {
    // グラフィックメイン
    HABITAT_LIST_GRA_Main( &p_wk->gra_sys );
    return GFL_PROC_RES_CONTINUE;
  }

  // 終了へ移行
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  プロセス終了処理
 *
 *	@param	* proc
 *	@param	* seq
 *	@param	* pwk
 *	@param	* mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT ZukanHabitatList_Exit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  HABITAT_LIST_WK *p_wk = mywk;

  // フレームリストデータを開放する
  DeleteFrameListData( p_wk );
  // Vブランクタスクの終了
  ZukanHabitatList_VBlankExit( p_wk );
  // Hブランクタスクの終了
  ZukanHabitatList_HBlankExit( p_wk );
  // グラフィックの終了処理
  HABITAT_LIST_GRA_Exit( &p_wk->gra_sys );
  // ワークデータの開放
  DeleteWkData( proc ,p_wk );
  // ヒープ終了
	GFL_HEAP_DeleteHeap( HEAPID_ZUKAN_HABITAT_LIST );
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  Vブランクタスク初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ZukanHabitatList_VBlankInit( HABITAT_LIST_WK *p_wk )
{
	p_wk->p_vtask = GFUser_VIntr_CreateTCB( ZukanHabitatList_VBlank, p_wk, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  Vブランクタスク終了処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ZukanHabitatList_VBlankExit( HABITAT_LIST_WK *p_wk )
{
  GFL_TCB_DeleteTask( p_wk->p_vtask );
}

//----------------------------------------------------------------------------
/**
 *	@brief  Vブランクタスク
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ZukanHabitatList_VBlank( GFL_TCB* tcb, void* wk )
{
  HABITAT_LIST_WK *p_wk = wk;
  // グラフィックVブランク処理
  HABITAT_LIST_GRA_VblankMain( &p_wk->gra_sys );
}


//----------------------------------------------------------------------------
/**
 *	@brief  Hブランクタスク初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ZukanHabitatList_HBlankInit( HABITAT_LIST_WK *p_wk )
{
	p_wk->p_htask = GFUser_HIntr_CreateTCB( ZukanHabitatList_HBlank, p_wk, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  Hブランクタスク終了処理
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ZukanHabitatList_HBlankExit( HABITAT_LIST_WK *p_wk )
{
  GFL_TCB_DeleteTask( p_wk->p_htask );
}

//----------------------------------------------------------------------------
/**
 *	@brief  Hブランクタスク
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ZukanHabitatList_HBlank( GFL_TCB* tcb, void* wk )
{
  HABITAT_LIST_WK *p_wk = wk;
  // グラフィックVブランク処理
  HABITAT_LIST_GRA_HblankMain( &p_wk->gra_sys );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ワークデータの作成
 *
 *	@param	* proc 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static HABITAT_LIST_WK* CreateWkData( GFL_PROC * proc ,ZUKANHABITAT_LIST_DATA *p_list_info )
{
  HABITAT_LIST_WK *p_wk;
	SAVE_CONTROL_WORK *p_svcont;
 
  // 確保
  p_wk = GFL_PROC_AllocWork( proc, sizeof(HABITAT_LIST_WK),
      HEAPID_ZUKAN_HABITAT_LIST );

  // クリア
  GFL_STD_MemClear( p_wk, sizeof(HABITAT_LIST_WK) );

  // 外部データ設定
  p_wk->p_init_param = p_list_info;

  // メッセージ取得
  p_wk->p_app_msg =GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
      ARCID_MESSAGE, NARC_message_seisoku_dat, HEAPID_ZUKAN_HABITAT_LIST );

  // セーブデータの取得
  p_svcont = GAMEDATA_GetSaveControlWork( p_list_info->p_gdata );
  p_wk->p_habitat_sv = SaveData_GetHabitatListSave( p_svcont );
 
  // ワードセットを作成
  p_wk->p_word_set = WORDSET_Create( HEAPID_ZUKAN_HABITAT_LIST ); 

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワークデータの破棄
 *
 *	@param	* proc 
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DeleteWkData( GFL_PROC * proc , HABITAT_LIST_WK *p_wk )
{
  // ワードセットの開放
  WORDSET_Delete( p_wk->p_word_set );
  // メッセージ開放
  GFL_MSG_Delete( p_wk->p_app_msg );
  // プロセスワークの開放
  GFL_PROC_FreeWork( proc );
}

//----------------------------------------------------------------------------
/**
 *	@brief  リストデータを生成する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MakeFrameListData( HABITAT_LIST_WK *p_wk )
{
  ZUKANHABITAT_LIST_LISTDATA *p_ld = p_wk->p_init_param->list_data;
  u8 list_size = p_wk->p_init_param->list_size;
  u16 default_zone_group = HABITATLIST_SAVE_GetDefaultZoneGroupID( p_wk->p_habitat_sv );
  HABITAT_LIST_GRA_MakeList( &p_wk->gra_sys ,list_size ,p_ld ,default_zone_group ,0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フレームリストを開放する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DeleteFrameListData( HABITAT_LIST_WK *p_wk )
{
  HABITAT_LIST_GRA_DeleteList( &p_wk->gra_sys );
}

