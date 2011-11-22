//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_infinity_dungeon.c
 *	@brief  無限ダンジョン　スクリプト
 *	@author	Masayuki Onoue
 *	@date		2011.02.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_infinity_dungeon.h"
#include "infinity_dungeon.h"
#include "savedata/infinity_dungeon_save.h"
#include "tr_tool/trno_def.h"
#include "arc/fieldmap/fldmmdl_objcode.h"  //for OBJCODE
#include "infinity_dungeon_gimmick.h"
#include "fld_btl_inst_tool.h"
#include "script_def.h"   // for SCR_INFDUNGEON_*
#include "savedata/misc.h"               // for MISC

// グローバル変数
#ifdef PM_DEBUG
extern u32  g_debug_inf_master_seed;
#endif

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	BOSSデータ
//=====================================
typedef struct {
  u32 tr_id;
  u32 obj_code;
}INF_BOSS_DATA;

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
//	ボスデータ
//===================================== 
// ホワイト
static const INF_BOSS_DATA sc_boss_data_w[] = {
  { TRID_BREEDERW_1_01 , BREEDERW},
  { TRID_BREEDERM_1_01 , BREEDERM},
  { TRID_RANGERW_1_01 , RANGERW},
  { TRID_RANGERM_1_01 , RANGERM},
  { TRID_VETERANW_3_03 , VETERANW}, // TODO 指定のトレーナーIDに書き換える
  { TRID_ELITEM_2_01 , TRAINERM},
  { TRID_ELITEW_2_01 , TRAINERW},
  { TRID_VETERANM_3_01 , VETERANM},
  { TRID_VETERANW_3_01 , VETERANW},
  { TRID_MUGENBOSS_01 , MUGENBOSS},
  { TRID_VETERANM_3_02 , VETERANM}, // TODO 指定のトレーナーIDに書き換える
};

// ブラック
static const INF_BOSS_DATA sc_boss_data_b[] = {
  { TRID_BREEDERM_2_01 , BREEDERM},
  { TRID_BREEDERW_2_01 , BREEDERW},
  { TRID_RANGERM_2_01 , RANGERM},
  { TRID_RANGERW_2_01 , RANGERW},
  { TRID_VETERANM_4_03 , VETERANM},
  { TRID_ELITEW_3_01 , TRAINERW},
  { TRID_ELITEM_3_01 , TRAINERM},
  { TRID_VETERANW_4_01 , VETERANW},
  { TRID_VETERANM_4_01 , VETERANM},
  { TRID_MUGENBOSS_02 , MUGENBOSS},
  { TRID_VETERANW_4_02 , VETERANW},
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static const INF_BOSS_DATA* GetBossData( SCRCMD_WORK *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内復帰処理
 *
 *	@param	*core
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK   *p_wk     = wk;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);

  // マップの復帰イベント
  INFINITY_DUNGEON_Recovery( p_gsys ,p_dungeon_data );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン内の指定方向ワープ
 *
 *	@param	*core
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonWarp( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK   *p_wk     = wk;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  u16           warp_dir  = SCRCMD_GetVMWorkValue( core, p_wk );
  u16           fade_out_param  = SCRCMD_GetVMWorkValue( core, p_wk );
  u16           fade_in_param  = SCRCMD_GetVMWorkValue( core, p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);

  ONOUE_Printf("warp_dir:%u\n",warp_dir);
  INFINITY_DUNGEON_WarpNextMap( p_scr_wk ,p_gsys ,p_dungeon_data ,warp_dir ,fade_in_param ,fade_out_param );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョン内のフロア移動
 *
 *	@param	*core
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonFloorMove( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK   *p_wk     = wk;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  int           floor_no  = SCRCMD_GetVMWorkValue( core, p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);

  ONOUE_Printf("%d階に移動\n",floor_no);

  // 次のマップへワープ
  INFINITY_DUNGEON_WarpNextFloor( p_scr_wk ,p_gsys ,p_dungeon_data ,floor_no );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンのステータス設定
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonSetStatus( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u8 status  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  FIELD_WFBC_CORE_TYPE wfbc_type = GAMEDATA_GetMyWFBCType( p_gamedata );

  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  INFINITYDUNGEON_SAVE_SetDungeonStatus( p_save ,status ,wfbc_type );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン生成用シードをランダム初期化
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonResetSeed( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  MISC *p_misc;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u32 seed = GFL_STD_MtRand( GFL_STD_RAND_MAX );
  u32 gold;
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  p_misc = SaveData_GetMisc( p_save_data );

  // セーブデータの初期化
  INFINITYDUNGEON_SAVE_InitDungeonData( p_save );

  // ダンジョン開始時の所持金を保存しておく
  gold = MISC_GetGold( p_misc );
  INFINITYDUNGEON_SAVE_SetDungeonStartGold( p_save ,gold );

#ifdef PM_DEBUG
  if( g_debug_inf_master_seed == 0 )
  {
    INFINITYDUNGEON_SAVE_SetRandomSeed( p_save ,seed );
  }
  else
  {
    INFINITYDUNGEON_SAVE_SetRandomSeed( p_save ,g_debug_inf_master_seed );
  }
#else
  INFINITYDUNGEON_SAVE_SetRandomSeed( p_save ,seed );
#endif

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のフロア番号を取得する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetFloorNo( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );

  // フロアIDを取得する
  *p_ret_wk = INFINITYDUNGEON_SAVE_GetFloorID( p_save );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のエリアIDを取得する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetAreaNo( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );

  // エリアIDを取得する
  *p_ret_wk = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内トレーナーとの戦闘
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonBattleTrainer( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  const MMDL *mmdl = SCRIPT_GetTargetObj( p_scr_wk );
  u8 obj_id = MMDL_GetOBJID( mmdl ) - INFINITY_DUNGEON_TR_OBJ_ID_OFS;

  SCRIPT_CallEvent(
      p_scr_wk, INFINITY_DUNGEON_EVENT_TrainerBattle(p_gamesys,p_fieldmap,obj_id) );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベーターを開ける
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonOpenElevator( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  u16 id  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  BOOL is_open  = SCRCMD_GetVMWorkValue( p_core, p_wk );

  SCRIPT_CallEvent(
      p_scr_wk, INFINITY_DUNGEON_EVENT_ElevatorOpen(p_gamesys,p_fieldmap,is_open,id) );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エレベーター開放イベント
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonLiftBanElevator( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );

  SCRIPT_CallEvent(
      p_scr_wk, INFINITY_DUNGEON_EVENT_ElevatorLiftBan(p_gamesys,p_fieldmap) );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内に配置されているボストレーナーナンバーを取得
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetBossNo( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  const INF_BOSS_DATA* p_boss = GetBossData( p_wk );

  // ボスインデックスを返す
  *p_ret_wk = p_boss->tr_id;

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョントレーナーの戦闘済みフラグを設定する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonSetTrainerBattleFlag( VMHANDLE *p_core, void *p_wk_adrs )
{
  u8 floor_id;
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 id_pos  = SCRCMD_GetVMWorkValue( p_core, p_wk ) - INFINITY_DUNGEON_TR_OBJ_ID_OFS;
  u16 status  = SCRCMD_GetVMWorkValue( p_core, p_wk );

  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  floor_id = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
  INFINITYDUNGEON_SAVE_SetTrainerFlag( p_save ,floor_id ,id_pos ,status );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョントレーナーの戦闘済みフラグを取得する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetTrainerBattleFlag( VMHANDLE *p_core, void *p_wk_adrs )
{
  u8 floor_id;
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 id_pos  = SCRCMD_GetVMWorkValue( p_core, p_wk )- INFINITY_DUNGEON_TR_OBJ_ID_OFS;
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  floor_id = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
  *p_ret_wk = INFINITYDUNGEON_SAVE_GetTrainerFlag( p_save ,floor_id ,id_pos );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョントレーナーのヒントメッセージIDを取得する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetHintMsgID( VMHANDLE *p_core, void *p_wk_adrs )
{
  int i,j;
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys  = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  u16           *p_ret_wk   = SCRCMD_GetVMWork( p_core, p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  const MMDL    *mmdl = SCRIPT_GetTargetObj( p_scr_wk );
  INFINITYDUNGEON_SAVE  *p_save;
  SAVE_CONTROL_WORK     *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 obj_code = MMDL_GetOBJCode( mmdl );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);
  u8 id_pos = MMDL_GetOBJID( mmdl ) - INFINITY_DUNGEON_TR_OBJ_ID_OFS;

  // セーブデータから必要情報を取得
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );

  // ヒントメッセージIDを設定する
  *p_ret_wk = INFINITY_DUNGEON_GetHintMessage( p_save ,p_dungeon_data ,obj_code,id_pos);

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボスの見た目（OBJ_CODE)を返す
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetBossOBJCode( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  const INF_BOSS_DATA* p_boss = GetBossData( p_wk );

  // OBJ_CODEを返す
  *p_ret_wk = p_boss->obj_code;

  return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン内トレーナーの戦闘後会話パターンを返す
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetAfterBtlMsgType( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  const MMDL *mmdl = SCRIPT_GetTargetObj( p_scr_wk );
  u16 obj_code = MMDL_GetOBJCode( mmdl );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  if( obj_code == DOCTOR )
  {
    // ドクター
    *p_ret_wk = INF_SRC_MSG_TYPE_DOCTOR;
  }
  else if( obj_code == NURSE )
  {
    // ナース
    *p_ret_wk = INF_SRC_MSG_TYPE_NURSE;
  }
  else
  {
    // ヒント
    *p_ret_wk = INF_SRC_MSG_TYPE_HINT;
  }

  return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン回復トレーナーの回復フラグを設定する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonSetRecoverFlag( VMHANDLE *p_core, void *p_wk_adrs )
{
  INF_DUNGEON_DATA *p_dungeon_data;
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  u16 tr_id  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 status  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u32 id_pos;

  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);
  id_pos = INFINITY_DUNGEON_GetRecoverFlgPos( p_dungeon_data ,tr_id );
  INFINITYDUNGEON_SAVE_SetRecoverFlag( p_save ,id_pos ,status );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン回復トレーナーの回復フラグを取得
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetRecoverFlag( VMHANDLE *p_core, void *p_wk_adrs )
{
  INF_DUNGEON_DATA *p_dungeon_data;
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 tr_id  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  u32 id_pos;
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);
  id_pos = INFINITY_DUNGEON_GetRecoverFlgPos( p_dungeon_data ,tr_id );
  *p_ret_wk = INFINITYDUNGEON_SAVE_GetRecoverFlag( p_save ,id_pos );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボスが存在するフロア番号を返す
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetBossFloor( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gsys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gsys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);
  u8  area_id;
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  u32 boss_floor;
  u8 start_floor;
  
  // エリアIDを取得
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  area_id = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );
  
  // ボス部屋の位置を検索
  boss_floor = INFINITY_DUNGEON_GetBossFloor( p_dungeon_data );

  start_floor = INFINITYDUNGEON_SAVE_GetAreaStartFloor( area_id );

  // ボスのいるフロアを返す
  *p_ret_wk = boss_floor + start_floor + 1; // ゼロオリジン修正

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートトレーナーが存在するフロアに存在するトレーナーの人数を取得
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetGateTrainerRoomTrNum( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gsys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  // ゲートトレーナーの場所取得
  s32 floor_id  = p_dungeon_data->gate_trainer_floor;
  s32 room_id   = p_dungeon_data->gate_trainer_room_id;

  (*p_ret_wk) = INF_TOOL_GetMemberNum(p_dungeon_data->room_data[floor_id][room_id].tr_pos_type);

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョントレーナーのトレーナータイプを取得
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetTrainerType( VMHANDLE *p_core, void *p_wk_adrs )
{
  u8 floor_id;
  INF_DUNGEON_DATA *p_dungeon_data;
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  const u16 heapID = FIELDMAP_GetHeapID( p_fieldmap );
  u16 tr_id  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  // 無限ダンジョントレーナーIDに変換
  //tr_id = INFINITY_DUNGEON_SAVE_GetTrNo_ByObjId( tr_id );

  *p_ret_wk = FBI_TOOL_GetTrtype(ARCID_INFINITY_DUGENON_TR , tr_id ,heapID); 

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  MAPステータスの取得
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetMapStatus( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  *p_ret_wk = INFINITYDUNGEON_SAVE_GetMapStatus( p_save );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  MAPステータスの設定
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonSetMapStatus( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u8 status  = SCRCMD_GetVMWorkValue( p_core, p_wk );

  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  INFINITYDUNGEON_SAVE_SetMapStatus( p_save ,status );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ID位置からトレーナーIDを取得する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetTrIDByIDPos( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK       *p_wk           = p_wk_adrs;
  GAMESYS_WORK      *p_gsys         = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA          *p_gamedata     = GAMESYSTEM_GetGameData(p_gsys);
  SAVE_CONTROL_WORK *p_save_data    = GAMEDATA_GetSaveControlWork(p_gamedata);
  FIELDMAP_WORK     *p_fieldmap     = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA  *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);
  u8                id_pos          = SCRCMD_GetVMWorkValue( p_core, p_wk ) - INFINITY_DUNGEON_TR_OBJ_ID_OFS;
  u16               *p_ret_wk	      = SCRCMD_GetVMWork( p_core, p_wk );
  u8                floor_id;

      
  p_save    = SaveData_GetINFINITYDUNGEON( p_save_data );
  floor_id  = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
  *p_ret_wk = INFINITY_DUNGEON_GetTrIDByIDPos( p_dungeon_data ,floor_id ,id_pos );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンのバージョンを取得する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetVersion( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk     = p_wk_adrs;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  
  // バージョンを設定する
  *p_ret_wk = INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンデータを取得する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetDungeonData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK   *p_wk     = p_wk_adrs;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);

  u16 param_id  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 area_id  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  
  //セーブデータの取得
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  INFINITYDUNGEON_SAVE *p_save    = SaveData_GetINFINITYDUNGEON( p_save_data );

  switch( param_id )
  {
  // 開始フロアを返す
  case SCR_INFDUNGEON_PARAM_START_FLOOR:
    {
      *p_ret_wk = INFINITYDUNGEON_SAVE_GetAreaStartFloor( area_id );
    }
    break;
  // 何階数あるか返す
  case SCR_INFDUNGEON_PARAM_FLOOR_NUM:
    {
      *p_ret_wk = INFINITYDUNGEON_SAVE_GetFloorNum( area_id );
    }
    break;
  case SCR_INFDUNGEON_PARAM_ROOM_ID:
    {
      *p_ret_wk = INFINITYDUNGEON_SAVE_GetRoomID( p_save );
    }
    break;
  // フロア内の何階にいるか返す
  case SCR_INFDUNGEON_PARAM_NOW_LOCAL_FLOOR_ID:
    {
      *p_ret_wk = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
    }
    break;
  case SCR_INFDUNGEON_PARAM_TRAINER_COUNT:
    {
      *p_ret_wk = INFINITY_DUNGEON_TrainerCount( p_dungeon_data );
    }
    break;
  case SCR_INFDUNGEON_PARAM_WIN_TRAINER_COUNT:
    {
      *p_ret_wk = INFINITYDUNGEON_SAVE_GetWinTrainerCount( p_save );
    }
    break;
  case SCR_INFDUNGEON_PARAM_CHECK_UP_ROOMTYPE_BOSS:
    {
      u8 room_id = INFINITYDUNGEON_SAVE_GetRoomID( p_save );
      u8 floor_id = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
      *p_ret_wk = INFINITY_DUNGEON_GetUpPosRoomTypeIsBoss( p_dungeon_data,floor_id,room_id );
    }
    break;
  default:
    {
      GF_ASSERT(0);
    }
    break;
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンドワークから、現在のボスデータを取得する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static const INF_BOSS_DATA* GetBossData( SCRCMD_WORK *p_wk )
{
  u8  area_id;
  u32 rom_version;
  u32 dungeon_version;
  INFINITYDUNGEON_SAVE  *p_save;
  const INF_BOSS_DATA* p_boss;

  // 無限ダンジョンデータを取得
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);

  // セーブデータを取得
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gsys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );

  // エリアIDを取得
  area_id = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );

  // バージョン指定
  rom_version = GET_VERSION();
  dungeon_version = INFINITY_DUNGEON_GetDungeonColor( p_dungeon_data );

  // バージョンごとにテーブルを取得
  if( dungeon_version == VERSION_B2 )
  {
    p_boss = sc_boss_data_b;
  }
  else
  {
    p_boss = sc_boss_data_w;
  }

  // 最上階の場合でかつ、バージョン違いの場合はインデックスをずらす
  if( area_id == INFINITY_DUNGEON_AREA_ID_10 )
  {
    if( rom_version != dungeon_version )
    {
      area_id ++;
    }
  }

  return &p_boss[area_id];
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゲートトレーナーかどうかを取得する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetIsGateTrainer( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;

  // 無限ダンジョンデータ
  GAMESYS_WORK  *p_gsys             = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_fieldmap         = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data  = INFINITY_DUNGEON_DataGet(p_fieldmap);
  
  // セーブデータ
  GAMEDATA *p_gamedata            = GAMESYSTEM_GetGameData(p_gsys);
  SAVE_CONTROL_WORK *p_save_data  = GAMEDATA_GetSaveControlWork(p_gamedata);
  INFINITYDUNGEON_SAVE* p_save    = SaveData_GetINFINITYDUNGEON( p_save_data );

  // パラメータ取得
  u16 id_pos  = SCRCMD_GetVMWorkValue( p_core, p_wk )- INFINITY_DUNGEON_TR_OBJ_ID_OFS;
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  u8 floor_id = INFINITYDUNGEON_SAVE_GetFloorID( p_save );

  (*p_ret_wk) = INF_TOOL_DUNGEON_GetIsGateTrainer( p_dungeon_data ,floor_id ,id_pos);
  return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンゲートトレーナーのバトル前メッセージIDを取得する
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetGateBattleMsgID( VMHANDLE *p_core, void *p_wk_adrs )
{
  int i,j;
  SCRCMD_WORK   *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys  = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  BOOL          is_before_battle  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16           *p_ret_wk   = SCRCMD_GetVMWork( p_core, p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  const MMDL    *mmdl = SCRIPT_GetTargetObj( p_scr_wk );
  INFINITYDUNGEON_SAVE  *p_save;
  SAVE_CONTROL_WORK     *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 obj_code = MMDL_GetOBJCode( mmdl );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);

  // セーブデータから必要情報を取得
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );

  // 戦闘前後メッセージ
  *p_ret_wk = INFINITY_DUNGEON_GetGateBeforeBattleMessage( p_save ,p_dungeon_data ,obj_code ,is_before_battle);

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ペナルティー時に減算する金額を取得
 *
 *	@param	*p_core
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetPenaltyGold( VMHANDLE *p_core, void *p_wk_adrs )
{
  u32 gold;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  INFINITYDUNGEON_SAVE  *p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  MISC *p_misc = SaveData_GetMisc( p_save_data );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  u32 now_gold    = MISC_GetGold( p_misc );
  u32 start_gold  = INFINITYDUNGEON_SAVE_GetDungeonStartGold( p_save );

  // 増えた分を返す
  (*p_ret_wk) = (now_gold - start_gold);
  return VMCMD_RESULT_CONTINUE;
}

