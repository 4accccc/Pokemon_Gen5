//============================================================================================
/**
 * @file  wbt_setup.c
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.03.30
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"
#include "wbt_system_local.h"
#include "app/wbt_list.h"
#include "wbt_tool.h"

#include "poke_tool/regulation_def.h"
#include "wbt_trlist_struct.h"  //WBT_TRDATA

#include "arc_def.h"
#include "arc/wbt/wbt_trlist.naix"
#include "arc/fieldmap/fldmmdl_mdlres.naix"

#include "msg/msg_wbt_tournament.h"
#include "arc/message.naix"
#include "system/main.h"


#include "fldmmdl.h"
#include "fieldmap/fldmmdl_mdlparam.naix"

#include "tr_tool/trtype_def.h"
#include "sound/pm_sndsys.h"

//============================================================================================
//============================================================================================

static void setupWBTLISTtrainer( HEAPID heapID, GAMEDATA * gamedata,
    WBTLIST_TRAINER * wbtlist_tr, const WBTTRAINER * wbt_tr );

static void setupWBTLISTResultAll( WBTSYS * wbtsys, WBTLIST_PARAM * wbtlist_param );
static void getMMdlObjCodeParam( ARCHANDLE * handle, u16 mmdl_id, u8 * anm_id, u16 * rsc_idx );

//============================================================================================
//
//  WBTトーナメントリスト用パラメータ生成
//
//============================================================================================
//------------------------------------------------------------------
///  WBTトーナメントリスト用パラメータOBJECT生成
//------------------------------------------------------------------
WBTLIST_PARAM * WBTLIST_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys )
{
  int i;
  WBTLIST_PARAM * wbtlist_param;

  wbtlist_param = GFL_HEAP_AllocClearMemory( heapID, sizeof(WBTLIST_PARAM) );
  wbtlist_param->gamesys = gsys;
  wbtlist_param->stage = WBTLIST_STAGE_ENTRY;

  wbtlist_param->tournament_name = GFL_STR_CreateBuffer( BUFLEN_TOURNAMENT_NAME, heapID );
  wbtlist_param->tournament_rule = GFL_STR_CreateBuffer( BUFLEN_TOURNAMENT_NAME, heapID );
  //トレーナー情報の初期化
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTLIST_TRAINER * wbtlist_tr = &wbtlist_param->trainer[i];
    wbtlist_tr->name = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, heapID );
    wbtlist_tr->mmdl_id = NARC_fldmmdl_mdlres_trainerm_nsbtx;
    wbtlist_tr->pattern_no = 2;
    wbtlist_tr->sex = PM_MALE;
    wbtlist_tr->priority = 1;
    wbtlist_tr->poketype = POKETYPE_NORMAL;
  }
  //勝敗結果情報の初期化

  TAMADA_Printf("WBTLIST_PARAM:Create:%08x\n", wbtlist_param );
  return wbtlist_param;
}

//------------------------------------------------------------------
///  WBTトーナメントリスト用パラメータOBJECT削除
//------------------------------------------------------------------
void WBTLIST_PARAM_Delete( WBTLIST_PARAM * wbtlist_param )
{
  int i;
  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    WBTLIST_TRAINER * wbtlist_tr = &wbtlist_param->trainer[i];
    GFL_STR_DeleteBuffer( wbtlist_tr->name );
  }
  GFL_STR_DeleteBuffer( wbtlist_param->tournament_name );
  GFL_STR_DeleteBuffer( wbtlist_param->tournament_rule );
  GFL_HEAP_FreeMemory( wbtlist_param );
  TAMADA_Printf("WBTLIST_PARAM:Delete:%08x\n", wbtlist_param );
}

//------------------------------------------------------------------
/**
 * @brief トーナメントリスト用パラメータのセットアップ
 */
//------------------------------------------------------------------
void WBTLIST_PARAM_Setup( GAMESYS_WORK * gsys, WBTSYS * wbtsys, WBTLIST_PARAM * wbtlist_param )
{
  int i;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  WBTSTAGE wbtstage;

  for ( i = 0; i < WBT_TRAINER_NUM; i ++ )
  {
    const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, i );
    setupWBTLISTtrainer( HEAPID_FIELDMAP, gamedata, &wbtlist_param->trainer[i], wbt_tr );
  }
  {
    GFL_STD_RandContext rand_context;
    GFL_STD_RandInit( &rand_context, WBTSYS_GetRandomSeed( wbtsys ) );
    WBTSYS_CalcResult( wbtsys, &rand_context );
    setupWBTLISTResultAll( wbtsys, wbtlist_param );
  }

  //ステージ設定
  wbtstage = WBTSYS_GetStage( wbtsys );
  switch ( wbtstage )
  {
  default:
    GF_ASSERT_MSG( 0, "stage(%d)には対応していません\n", wbtstage );
    /* FALL THROUGH */

  case WBTSTAGE_ENTRY:
    wbtlist_param->stage = WBTLIST_STAGE_ENTRY;
    break;
  case WBTSTAGE_1ST:
    wbtlist_param->stage = WBTLIST_STAGE_1ST;
    break;
  case WBTSTAGE_2ND:
    wbtlist_param->stage = WBTLIST_STAGE_2ND;
    break;
  }
  WBTTOOL_SetTournamentName( WBTSYS_GetType(wbtsys), wbtlist_param->tournament_name );
  WBTTOOL_SetRuleName( WBTSYS_GetRule(wbtsys), wbtlist_param->tournament_rule );
}


#ifdef  PM_DEBUG
//------------------------------------------------------------------
/**
 * @brief トーナメントリスト用パラメータのセットアップ（デバッグ用）
 *
 * デバッグ用にWBTSYSを生成し、それを元にセットアップを行う
 */
//------------------------------------------------------------------
void WBTLIST_PARAM_Setup_Debug(
    GAMESYS_WORK * gsys, WBTLIST_PARAM * wbtlist_param,
    WBTTYPE wbttype, WBTRULE wbtrule,
    WBTSTAGE stage, const u16 * trid_table, int debug_value )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  WBTSYS * wbtsys;

  wbtsys = WBTSYS_Create( HEAPID_FIELDMAP );
  WBTSYS_SetupTrainer( wbtsys, gamedata, trid_table );
  WBTSYS_SetType( wbtsys, wbttype );
  WBTSYS_SetRule( wbtsys, wbtrule );
  WBTSYS_SetStage( wbtsys, stage );

  WBTLIST_PARAM_Setup( gsys, wbtsys, wbtlist_param );
  wbtlist_param->debug_value = debug_value;

  WBTSYS_Delete( wbtsys );
}
#endif

//============================================================================================
//
//============================================================================================
//------------------------------------------------------------------
/// WBTTRAINER　-->　WBTLIST_TRAINERへの変換処理
//------------------------------------------------------------------
static void setupWBTLISTtrainer( HEAPID heapID, GAMEDATA * gamedata,
    WBTLIST_TRAINER * wbtlist_tr, const WBTTRAINER * wbt_tr )
{
  WBT_TRDATA tr_rom_data;
  ARCHANDLE * handle;
  u16 mmdl_id;

  GF_ASSERT( wbt_tr );
  GF_ASSERT( wbtlist_tr );

  WBTTOOL_GetTrRomData( heapID, wbt_tr->tr_id, &tr_rom_data );

  switch ( wbt_tr->wbt_tr_type )
  {
  case WBT_TRTYPE_DL:
  case WBT_TRTYPE_SAVE:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case WBT_TRTYPE_ROM:
    wbtlist_tr->is_player = FALSE;
    wbtlist_tr->priority = tr_rom_data.priority;
    wbtlist_tr->poketype = tr_rom_data.type;
    wbtlist_tr->sex      = tr_rom_data.sex;
    {
      WBTTOOL_SetRomTrainerName( heapID, wbt_tr->tr_id, wbtlist_tr->name );
    }
    mmdl_id = tr_rom_data.mmdl_id;
    break;
  case WBT_TRTYPE_PLAYER:
    wbtlist_tr->is_player = TRUE;
    wbtlist_tr->priority = wbt_tr->pri;
    wbtlist_tr->poketype = wbt_tr->poketype;
    wbtlist_tr->sex = wbt_tr->sex;
    {
      const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );
      MyStatus_CopyNameString( mystat, wbtlist_tr->name );
    }
    mmdl_id = wbt_tr->mmdl_id;
    break;
  }

  handle = GFL_ARC_OpenDataHandle( ARCID_MMDL_PARAM, heapID );
  getMMdlObjCodeParam( handle, mmdl_id, &wbtlist_tr->pattern_no, &wbtlist_tr->mmdl_id );
  GFL_ARC_CloseDataHandle( handle );
}

//------------------------------------------------------------------
/// WBTRESULT --> WBTLIST_RESULTへの変換処理
//------------------------------------------------------------------
static void setupWBTLISTResultAll( WBTSYS * wbtsys, WBTLIST_PARAM * wbtlist_param )
{
  int i, j;
  static const u16 comment_tbl[ WBT_COMMENT_TOTAL ] = {
    msg_wbt_comment_01,
    msg_wbt_comment_06,
    msg_wbt_comment_11,
    msg_wbt_comment_16,
    msg_wbt_comment_21,
  };

  //1回戦結果生成
  for ( i = 0; i < 4 ; i ++ )
  {
    WBTLIST_RESULT * list_res = &wbtlist_param->first_stage[ i ];
    const WBTRESULT * wbt_res = &wbtsys->wbt_result[i];
    list_res->upside_win = wbt_res->upside_win;
    list_res->msg_type   = comment_tbl[ wbt_res->msg_id ];
  }

  //2回戦結果生成
  for ( i = 0; i < 2 ; i ++ )
  {
    WBTLIST_RESULT * list_res = &wbtlist_param->second_stage[i];
    const WBTRESULT * wbt_res = &wbtsys->wbt_result[WBT_GAME_2ND_1 + i];
    list_res->upside_win = wbt_res->upside_win;
    list_res->msg_type   = comment_tbl[ wbt_res->msg_id ];
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void DEBUG_DumpWBTLISTResult( const WBTLIST_RESULT * result,
    const WBTLIST_TRAINER * tr_a, const WBTLIST_TRAINER * tr_b )
{
  TAMADA_Printf("upside_win %d \n", result->upside_win );
  TAMADA_Printf("msg_id = %d\n", result->msg_type );
  TAMADA_Printf("tr_a:pri(%d) type(%d) sex(%d) id(%d)\n",
      tr_a->priority, tr_a->poketype, tr_a->sex, tr_a->mmdl_id );
  TAMADA_Printf("tr_b:pri(%d) type(%d) sex(%d) id(%d)\n",
      tr_b->priority, tr_b->poketype, tr_b->sex, tr_b->mmdl_id );
}


//============================================================================================
//
//  動作モデルデータの参照
//
//============================================================================================
extern const MMDL_BBDACT_ANMTBL DATA_MMDL_BBDACT_ANM_ListTable[MMDL_BLACT_ANMTBLNO_MAX];
//-----------------------------------------------------------------------------
/**
 * @brief 動作モデル正面絵パターンナンバーの取得
 *
 * fldmmdlのOBJCODE_PARAMから正面絵の位置を引っ張る
 * OBJCODE_PARAM::anm_idからアニメパターンの種類がわかるので、
 * それぞれのアニメパターンの種類から下向き静止アニメをとりだし、
 * その指定パターンを正面絵のパターンナンバーとして取り出している
 */
//-----------------------------------------------------------------------------
static u8 getFrontAnimePatternNo( anm_id )
{
  const MMDL_BBDACT_ANMTBL * mmdl_anm_tbl;
  GF_ASSERT( anm_id < MMDL_BLACT_ANMTBLNO_MAX );
  mmdl_anm_tbl = &DATA_MMDL_BBDACT_ANM_ListTable[ anm_id ];
  GF_ASSERT( AC_DIR_D < mmdl_anm_tbl->anm_max );

  return mmdl_anm_tbl->pAnmTbl[AC_DIR_D]->anmData.celIdx;
}

//-----------------------------------------------------------------------------
/**
 * @brief 動作モデルIDから動作モデルパラメータを取得する
 * @param handle
 * @param mmdl_id
 * @param anm_id
 * @param rsc_idx
 */
//-----------------------------------------------------------------------------
static void getMMdlObjCodeParam( ARCHANDLE * handle, u16 mmdl_id, u8 * anm_id, u16 * rsc_idx )
{
  OBJCODE_PARAM objcode_param;
  u32 size = sizeof( OBJCODE_PARAM );
  u32 offs = OBJCODE_PARAM_TOTAL_NUMBER_SIZE + ( size * MMDL_TOOL_OBJCodeToDataNumber(mmdl_id) );
  GFL_ARC_LoadDataOfsByHandle( handle, NARC_fldmmdl_mdlparam_fldmmdl_mdlparam_bin,
      offs, size, &objcode_param );

  TAMADA_Printf("OBJCODE_PARAM:mmdl_id = %d\n",mmdl_id );
  TAMADA_Printf("OBJCODE_PARAM:offs = %d\n", (offs - OBJCODE_PARAM_TOTAL_NUMBER_SIZE) / size );
  TAMADA_Printf("OBJCODE_PARAM:anm_id = %d\n",objcode_param.anm_id );
  TAMADA_Printf("OBJCODE_PARAM:res_idx = %d\n",
      ((OBJCODE_PARAM_BUF_BBD *)objcode_param.buf)->res_idx );

  *anm_id = getFrontAnimePatternNo( objcode_param.anm_id );
  *rsc_idx = ((OBJCODE_PARAM_BUF_BBD *)objcode_param.buf)->res_idx;
}



//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
///  WIN_RECORD用パラメータOBJECT生成
//------------------------------------------------------------------
WIN_RECORD_PARAM * WIN_RECORD_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	WIN_RECORD_PARAM * wr_param;
	
	wr_param 			= GFL_HEAP_AllocClearMemory( heapID, sizeof(WIN_RECORD_PARAM) );
	wr_param->gamesys	= gsys;
	// 初期化
	wr_param->status	  = WIN_RECORD_STATUS_LV1;
  wr_param->wbtsave   = GAMEDATA_GetWBTSave( gamedata );  //WBT優勝回数
  wr_param->my_status = GAMEDATA_GetMyStatus( gamedata ); //自分情報
	//
	return wr_param;
}

//------------------------------------------------------------------
///  WBTトーナメントリスト用パラメータOBJECT削除
//------------------------------------------------------------------
void WIN_RECORD_PARAM_Delete( WIN_RECORD_PARAM * wr_param )
{
	// 必要であれば削除を追加
	GFL_HEAP_FreeMemory( wr_param );
}

#ifdef  PM_DEBUG
//------------------------------------------------------------------
/**
 * @brief トーナメントリスト用パラメータのセットアップ（デバッグ用）
 *
 * デバッグ用にWBTSYSを生成し、それを元にセットアップを行う
 */
//------------------------------------------------------------------
void WIN_RECORD_PARAM_Setup_Debug( GAMESYS_WORK * gsys, WIN_RECORD_PARAM * wr_param )
{
	// 必要であれば、ここでパラメータのセットをするらしい
	
}
#endif


//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
///  TDOWNLOAD用パラメータOBJECT生成
//------------------------------------------------------------------
T_DOWNLOAD_PARAM * T_DOWNLOAD_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys )
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	T_DOWNLOAD_PARAM 	*td_param;
	
	td_param 			= GFL_HEAP_AllocClearMemory( heapID, sizeof(T_DOWNLOAD_PARAM) );
	td_param->gamesys	= gsys;
	// 初期化
	td_param->status	  = T_DOWNLOAD_STATUS_VIEW;
//  wr_param->wbtsave   = GAMEDATA_GetWBTSave( gamedata );  //WBT優勝回数
//  wr_param->my_status = GAMEDATA_GetMyStatus( gamedata ); //自分情報
	//
	return td_param;
}

//------------------------------------------------------------------
///  TDOWNLOAD用パラメータOBJECT削除
//------------------------------------------------------------------
void T_DOWNLOAD_PARAM_Delete( T_DOWNLOAD_PARAM * td_param )
{
	// 必要であれば削除を追加
	GFL_HEAP_FreeMemory( td_param );
}

#ifdef  PM_DEBUG
//------------------------------------------------------------------
/**
 * @brief TDOWNLOAD用パラメータのセットアップ（デバッグ用）
 *
 * デバッグ用にWBTSYSを生成し、それを元にセットアップを行う
 */
//------------------------------------------------------------------
void T_DOWNLOAD_PARAM_Setup_Debug( GAMESYS_WORK * gsys, T_DOWNLOAD_PARAM * td_param )
{
	// 必要であれば、ここでパラメータのセットをするらしい
	
}
#endif




