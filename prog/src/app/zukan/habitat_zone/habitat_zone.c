//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_zone.c
 *	@brief  生息地リスト：ゾーン詳細画面
 *	@author	Masayuki Onoue
 *	@date		2011.08.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

#include "habitat_zone.h"
#include "habitat_zone_main.h"
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
static GFL_PROC_RESULT ZukanHabitatZone_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanHabitatZone_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT ZukanHabitatZone_Exit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

// VBlank-------------------
static void ZukanHabitatZone_VBlankInit( HABITAT_ZONE_WK *p_wk );
static void ZukanHabitatZone_VBlankExit( HABITAT_ZONE_WK *p_wk );
static void ZukanHabitatZone_VBlank( GFL_TCB* tcb, void* wk );

// ワーク関連 --------------
static HABITAT_ZONE_WK* CreateWkData( GFL_PROC * proc ,ZUKANHABITAT_ZONE_DATA *p_zone_info );
static void DeleteWkData( GFL_PROC * proc , HABITAT_ZONE_WK *p_wk );

// プロセスデータ
const GFL_PROC_DATA ZUKANHABITAT_ZONE_ProcData = {
  ZukanHabitatZone_Init,
  ZukanHabitatZone_Main,
  ZukanHabitatZone_Exit,
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
static GFL_PROC_RESULT ZukanHabitatZone_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  HABITAT_ZONE_WK *p_wk;
  ZUKANHABITAT_ZONE_DATA *p_zone_info = pwk;

  // プロセスワークを生成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ZUKAN_HABITAT_ZONE, 0x60000 );

  // ワークの生成
  p_wk = CreateWkData( proc ,p_zone_info );

  // グラフィック初期化
  HABITAT_ZONE_GRA_Init( &p_wk->gra_sys );

  // Vブランクタスク初期化
  ZukanHabitatZone_VBlankInit( p_wk );

  return GFL_PROC_RES_FINISH;
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
static GFL_PROC_RESULT ZukanHabitatZone_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  HABITAT_ZONE_WK *p_wk = mywk;

  // メイン関数を呼び出す
  if( HABITAT_ZONE_Main( p_wk ) )
  {
    // グラフィックメイン
    HABITAT_ZONE_GRA_Main( &p_wk->gra_sys );
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
static GFL_PROC_RESULT ZukanHabitatZone_Exit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  HABITAT_ZONE_WK *p_wk = mywk;

  // Vブランクタスクの終了
  ZukanHabitatZone_VBlankExit( p_wk );
  // グラフィックの終了処理
  HABITAT_ZONE_GRA_Exit( &p_wk->gra_sys );
  // ワークデータの開放
  DeleteWkData( proc ,p_wk );
  // ヒープ終了
	GFL_HEAP_DeleteHeap( HEAPID_ZUKAN_HABITAT_ZONE );
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
static void ZukanHabitatZone_VBlankInit( HABITAT_ZONE_WK *p_wk )
{
	p_wk->p_vtask = GFUser_VIntr_CreateTCB( ZukanHabitatZone_VBlank, p_wk, 0 );
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
static void ZukanHabitatZone_VBlankExit( HABITAT_ZONE_WK *p_wk )
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
static void ZukanHabitatZone_VBlank( GFL_TCB* tcb, void* wk )
{
  HABITAT_ZONE_WK *p_wk = wk;
  // グラフィックVブランク処理
  HABITAT_ZONE_GRA_VblankMain( &p_wk->gra_sys );
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
static HABITAT_ZONE_WK* CreateWkData( GFL_PROC * proc ,ZUKANHABITAT_ZONE_DATA *p_zone_info )
{
  HABITAT_ZONE_WK *p_wk;

  // 確保
  p_wk = GFL_PROC_AllocWork( proc, sizeof(HABITAT_ZONE_WK),
      HEAPID_ZUKAN_HABITAT_ZONE );

  // クリア
  GFL_STD_MemClear( p_wk, sizeof(HABITAT_ZONE_WK) );

  // 外部データ設定
  p_wk->p_init_param = p_zone_info;

  // メッセージ取得
  p_wk->p_app_msg =GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
      ARCID_MESSAGE, NARC_message_seisoku_dat, HEAPID_ZUKAN_HABITAT_ZONE );

  // ワードセットを作成
  p_wk->p_word_set = WORDSET_Create( HEAPID_ZUKAN_HABITAT_ZONE ); 

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
static void DeleteWkData( GFL_PROC * proc , HABITAT_ZONE_WK *p_wk )
{
  // ワードセットの開放
  WORDSET_Delete( p_wk->p_word_set );
  // メッセージ開放
  GFL_MSG_Delete( p_wk->p_app_msg );
  // プロセスワークの開放
  GFL_PROC_FreeWork( proc );
}

