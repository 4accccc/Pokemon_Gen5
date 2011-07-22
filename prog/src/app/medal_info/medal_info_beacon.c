//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medal_info_beacon.c
 *	@brief  ビーコン情報からメダル詳細画面を構築するアプリ
 *	@author	Masayuki Onoue
 *	@date		2011.07.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "app/medal_info_beacon.h"
#include "app/mdlid_def.h"              // for MedalID
#include "medal_info_viewer.h"      // アプリ描画モジュール
#include "system/main.h"        // HEAPID

// ビーコン関連
#include "gamesystem/beacon_status.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_accessor.h"
#include "gamesystem/game_beacon_types.h"

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メダル情報画面メインワーク
//=====================================
typedef struct {

	HEAPID heapID;
  BEACON_MEDALINFO_INIT_PARAM   *p_param;     // アプリ初期化用パラメータ
  MEDALVIEWER_INITPARAM         init_param;   // 描画モジュール初期化用パラメータ
  MEDALVIEWER_WK                *p_viewer_wk; // ビューワーク
}BEACON_MEDALINFO_WK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static GFL_PROC_RESULT BEACON_MEDALINFO_ProcInit( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BEACON_MEDALINFO_ProcMain( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BEACON_MEDALINFO_ProcTerm( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );
static void BEACON_MEDALINFO_SetupMedalData( BEACON_MEDALINFO_WK* p_wk );

// 外部参照用プロセスデータ定義
GFL_PROC_DATA beacon_medalInfo_ProcData =
{
  BEACON_MEDALINFO_ProcInit,
  BEACON_MEDALINFO_ProcMain,
  BEACON_MEDALINFO_ProcTerm
};


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
///	ヒープサイズ
//=====================================
#define MEDALINFO_HEAP_SIZE (0x21000)

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステムプロセス初期化処理
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BEACON_MEDALINFO_ProcInit( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  // ワーク定義
  BEACON_MEDALINFO_WK          *p_wk;
  BEACON_MEDALINFO_INIT_PARAM  *p_param;

  // プロセス用ヒープID
  HEAPID heapID = HEAPID_MEDALINFO;
	
  // 初期化用パラメータ
  p_param	= p_param_adrs;

	// ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, heapID, MEDALINFO_HEAP_SIZE );

	//ワーク作成
	p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(BEACON_MEDALINFO_WK), heapID );
	GFL_STD_MemClear( p_wk, sizeof(BEACON_MEDALINFO_WK) );

  // パラメータの保存
  p_wk->p_param = p_param;

  // メダルデータを設定
  BEACON_MEDALINFO_SetupMedalData( p_wk );

  // パラメータの設定
  p_wk->init_param.heapID     = heapID;

//  if(  p_param->call_mode == MEDALINFO_CALL_TOP )
//  {
//    p_wk->init_param.call_mode  = MEDALINFO_CALL_TOP;
//  }
//  else
//  {
//    p_wk->init_param.call_mode = MEDALINFO_CALL_SUB;
//  }



  // viewerの生成
  p_wk->p_viewer_wk = MEDALVIEWER_Setup( &p_wk->init_param );

  // 初期化呼び出し
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル詳細画面メイン
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BEACON_MEDALINFO_ProcMain( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  BEACON_MEDALINFO_WK *p_wk = p_wk_adrs;
  GFL_PROC_RESULT  proc_ret = GFL_PROC_RES_FINISH;

  // メイン処理呼び出し
  proc_ret = MEDALVIEWER_Main( p_wk->p_viewer_wk );

  // メイン呼び出し
  return proc_ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステム終了処理
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BEACON_MEDALINFO_ProcTerm( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  int i;
  BEACON_MEDALINFO_WK *p_wk = p_wk_adrs;

  // メイン処理呼び出し
  u32 result = MEDALVIEWER_Delete( p_wk->p_viewer_wk );

  // 終了方法の判定
  if( result == MEDALVIEWER_RESULT_RETURN )
  {
    p_wk->p_param->ret_mode = FALSE;
  }
  else if( result == MEDALVIEWER_RESULT_EXIT )
  {
    p_wk->p_param->ret_mode = TRUE;
  }

  // 名前データを開放
  for( i = 0; i < p_wk->init_param.list_size; i++ )
  {
    MEDAL_INFO *p_info = &p_wk->init_param.p_info_list[i];
    GFL_STR_DeleteBuffer( p_info->p_trainer_name );
  }

  // メダル情報を開放する
  GFL_HEAP_FreeMemory( p_wk->init_param.p_info_list );

  // ワーク開放
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_MEDALINFO );

  // 終了処理呼び出し
  return GFL_PROC_RES_FINISH;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンデータからメダルデータを取得し、設定する
 *
 *	@param	p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void BEACON_MEDALINFO_SetupMedalData( BEACON_MEDALINFO_WK* p_wk )
{

  int i,max;
  u16 tmpTime;

  //テンポラリを作成
  GAMEBEACON_INFO *tmpInfo = GAMEBEACON_Alloc( p_wk->heapID );

  BEACON_STATUS       *b_status = GAMEDATA_GetBeaconStatus( p_wk->p_param->gdata );
  GAMEBEACON_INFO_TBL *infoLog = BEACON_STATUS_GetInfoTbl( b_status );

  // 初期用データを設定
  p_wk->init_param.target     = p_wk->p_param->target;
  p_wk->init_param.list_size  = 0;

  max = GAMEBEACON_InfoTblRing_GetEntryNum( infoLog );

  // データ数を調べる
  for(i = 0;i < max;i++){
    GAMEBEACON_InfoTblRing_GetBeacon( infoLog, tmpInfo, &tmpTime, i );

    if( GAMEBEACON_Check_Error( tmpInfo ) || 
        GAMEBEACON_Check_NPC( tmpInfo ) ){
      continue;
    }
    p_wk->init_param.list_size ++;
  }

  // 必要なデータ領域を確保する
  {
    s32 data_lng = p_wk->init_param.list_size;
    p_wk->init_param.p_info_list = 
      GFL_HEAP_AllocClearMemory( p_wk->heapID, sizeof(GAMEBEACON_INFO)*data_lng);
  }
 
  // データを格納する
  for(i = 0;i < max;i++){
    MEDAL_INFO *p_info = &p_wk->init_param.p_info_list[i];
    GAMEBEACON_InfoTblRing_GetBeacon( infoLog, tmpInfo, &tmpTime, i );

    if( GAMEBEACON_Check_Error( tmpInfo ) || 
        GAMEBEACON_Check_NPC( tmpInfo ) ){
      continue;
    }

    // 名前用バッファを生成
    p_info->p_trainer_name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE ,p_wk->heapID );

    GAMEBEACON_Get_PlayerNameToBuf( tmpInfo , p_info->p_trainer_name );
    p_info->medal_id  = GAMEBEACON_Get_PushMedal( tmpInfo );
    p_info->medal_cnt = GAMEBEACON_Get_MedalCount( tmpInfo );
    p_info->box_rank  = 0;
  }

  // データが取得できなかった場合はアプリを終了させる
  if( p_wk->init_param.list_size == 0 )
  {
    GF_ASSERT(0);
  }
}
