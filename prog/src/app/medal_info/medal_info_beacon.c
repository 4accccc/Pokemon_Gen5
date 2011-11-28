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
  u8                            view_ofs;     //詳細画面コール時のターゲットのすれ違いログのViewオフセット
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
#define MEDALINFO_HEAP_SIZE (0x31000)

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

  {
    BEACON_STATUS *b_status = GAMEDATA_GetBeaconStatus( p_wk->p_param->gdata );
    GAMEBEACON_INFO_TBL *infoLog = BEACON_STATUS_GetInfoTbl( b_status );
    u32 last_data_index = p_wk->init_param.last_data_index;
    u32 target_data_index      = p_wk->init_param.target;
    MEDAL_INFO *p_start_info = &p_wk->init_param.p_info_list[target_data_index];
    MEDAL_INFO *p_new_info = &p_wk->init_param.p_info_list[last_data_index];
    int list_top_ini  = p_start_info->beacon_log_index;
    int new_top       = p_new_info->beacon_log_index;
    int max = GAMEBEACON_InfoTblRing_GetEntryNum( infoLog );
    int view_ofs = p_wk->p_param->view_ofs;

    ONOUE_Printf("開始インデックス：%d\n",list_top_ini);
    ONOUE_Printf("終了インデックス：%d\n",new_top);
    ONOUE_Printf("ビューオフセット：%d\n",view_ofs);

    {
      //すれ違いログ画面で最後に見えていた範囲を計算
      int view_top = list_top_ini-view_ofs;
      int view_bottom = list_top_ini+((BEACON_VIEW_PANEL_NUM_MAX-1)-view_ofs);

      if( new_top < view_top || new_top > view_bottom ){
        if( (max-new_top) < BEACON_VIEW_PANEL_NUM_MAX ){  //見える範囲には制限がある
          new_top = max-BEACON_VIEW_PANEL_NUM_MAX;
        }
        ONOUE_Printf("設定インデックス：%d\n",new_top);
        BEACON_STATUS_SetViewTopOffset( b_status, new_top );
      }
    }
  }

  // 名前データを開放
  for( i = 0; i < p_wk->init_param.list_size; i++ )
  {
    MEDAL_INFO *p_info = &p_wk->init_param.p_info_list[i];
    GFL_STR_DeleteBuffer( p_info->p_trainer_name );
    ONOUE_Printf("データ開放[%d]\n",i);
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
  u16 data_index = 0;

  //テンポラリを作成
  GAMEBEACON_INFO *tmpInfo = GAMEBEACON_Alloc( p_wk->heapID );

  BEACON_STATUS       *b_status = GAMEDATA_GetBeaconStatus( p_wk->p_param->gdata );
  GAMEBEACON_INFO_TBL *infoLog = BEACON_STATUS_GetInfoTbl( b_status );

  // 初期用データを設定
  p_wk->init_param.list_size  = 0;
  p_wk->init_param.call_mode  = MEDALVIEWER_CALL_LIST;

  max = GAMEBEACON_InfoTblRing_GetEntryNum( infoLog );

  // データ数を調べる
  for(i = 0;i < max;i++){
    u32 pm_version;
    GAMEBEACON_InfoTblRing_GetBeacon( infoLog, tmpInfo, &tmpTime, i );
    pm_version = GAMEBEACON_Get_PmVersion( tmpInfo );

    if( GAMEBEACON_Check_Error( tmpInfo ) || 
        GAMEBEACON_Check_NPC( tmpInfo ) ){
      continue;
    }
    // バージョン指定
    if( !(( pm_version == VERSION_W2 ) || ( pm_version == VERSION_B2 )) ){
      continue;
    }

    // 未登録の場合はスキップ
    if( GAMEBEACON_Get_PushMedal( tmpInfo ) == MDLID_MAX )
    {
      continue;
    }

    p_wk->init_param.list_size ++;
  }

  // 必要なデータ領域を確保する
  {
    s32 data_lng = p_wk->init_param.list_size;
    p_wk->init_param.p_info_list = 
      GFL_HEAP_AllocClearMemory( p_wk->heapID, sizeof(MEDAL_INFO)*data_lng);
    ONOUE_Printf("データ確保数[%d]\n",data_lng);
  }
 
  // データを格納する
  for(i = 0;i < max;i++){
    u32 pm_version;
    MEDAL_INFO *p_info;
    GAMEBEACON_InfoTblRing_GetBeacon( infoLog, tmpInfo, &tmpTime, i );
    pm_version = GAMEBEACON_Get_PmVersion( tmpInfo );

    if( GAMEBEACON_Check_Error( tmpInfo ) || 
        GAMEBEACON_Check_NPC( tmpInfo ) ){
         continue;
    }
    // バージョン指定
    if( !(( pm_version == VERSION_W2 ) || ( pm_version == VERSION_B2 )) ){
      continue;
    }

    // メダル詳細画面用インデックス
    if( p_wk->p_param->target == i )
    {
      p_wk->init_param.target = data_index; 
    }

    
    // 未登録の場合はスキップ
    if( GAMEBEACON_Get_PushMedal( tmpInfo ) == MDLID_MAX )
    {
      continue;
    }

    // データ各の先を取得
    p_info = &p_wk->init_param.p_info_list[data_index];

    // 名前用バッファを生成
    p_info->p_trainer_name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE ,p_wk->heapID );
    ONOUE_Printf("名前生成[%d]\n",data_index);

    GAMEBEACON_Get_PlayerNameToBuf( tmpInfo , p_info->p_trainer_name );
    p_info->medal_id          = GAMEBEACON_Get_PushMedal( tmpInfo );
    p_info->medal_cnt         = GAMEBEACON_Get_MedalCount( tmpInfo );
    p_info->box_rank          = 0;
    p_info->beacon_log_index  = i;

    // データインデックスを進める
    data_index ++;
  }

  // データが取得できなかった場合はアプリを終了させる
  if( p_wk->init_param.list_size == 0 )
  {
    GF_ASSERT(0);
  }
}
