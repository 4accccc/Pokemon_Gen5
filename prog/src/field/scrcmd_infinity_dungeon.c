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
#include "field/infinity_dungeon.h"
#include "savedata/infinity_dungeon_save.h"
#include "tr_tool/trno_def.h"
#include "arc/fieldmap/fldmmdl_objcode.h"  //for OBJCODE
#include "infinity_dungeon_gimmick.h"

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
static const INF_BOSS_DATA sc_boss_data[] = {
  // ホワイト
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},

  // ブラック
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
  { TRID_BIGFOUR1_01 , TRAINERM},
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

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
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_gsys );
  INF_DUNGEON_DATA *p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);

  ONOUE_Printf("warp_dir:%u\n",warp_dir);
  INFINITY_DUNGEON_WarpNextMap( p_scr_wk ,p_gsys ,p_dungeon_data ,warp_dir );

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

  // 次のマップへワープ
  INFINITY_DUNGEON_WarpNextFloor( p_scr_wk ,p_gsys ,p_dungeon_data ,floor_no );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ダンジョンのステータス取得
 *
 *	@param	*core
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdInfinityDungeonGetStatus( VMHANDLE *p_core, void *p_wk_adrs )
{
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  *p_ret_wk = INFINITYDUNGEON_SAVE_GetDungeonStatus( p_save );

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
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u32 seed = GFL_STD_MtRand( GFL_STD_RAND_MAX );
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  INFINITYDUNGEON_SAVE_SetRandomSeed( p_save ,seed );

  ONOUE_Printf("rand_seed=%u\n",seed);
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
  u16 obj_id = MMDL_GetOBJID( mmdl );

  SCRIPT_CallEvent(
      p_scr_wk, INFINITY_DUNGEON_EVENT_TrainerBattle(p_gamesys,p_fieldmap,obj_id) );

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
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );

  // ボスインデックスを返す
  *p_ret_wk = TRID_LEADER3_02;

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
  floor_id = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
  p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);
  id_pos = INFINITY_DUNGEON_GetTrIDPos( p_dungeon_data ,floor_id ,tr_id );
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
  floor_id = INFINITYDUNGEON_SAVE_GetFloorID( p_save );
  p_dungeon_data = INFINITY_DUNGEON_DataGet(p_fieldmap);
  id_pos = INFINITY_DUNGEON_GetTrIDPos( p_dungeon_data ,floor_id ,tr_id );
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
  u8 hint_level;
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


  // セーブデータから必要情報を取得
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );
  hint_level = INFINITY_DUNGEON_SAVE_GetHintLevel( p_save );

  // ヒントメッセージIDを設定する
  *p_ret_wk = INFINITY_DUNGEON_GetHintMessage( p_save ,p_dungeon_data ,hint_level ,obj_code);

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
  INFINITYDUNGEON_SAVE  *p_save;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  SAVE_CONTROL_WORK *p_save_data = GAMEDATA_GetSaveControlWork(p_gamedata);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  u8  area_id;
  u32 version;
  p_save = SaveData_GetINFINITYDUNGEON( p_save_data );

  // エリアIDを取得
  area_id = INFINITYDUNGEON_SAVE_GetDungeonAreaID( p_save );

  // バージョン指定
  version = GET_VERSION();

  // オフセット分ずらす
  if( version == VERSION_B2 )
  {
    area_id += 10;
  }

  // OBJ_CODEを返す
  *p_ret_wk = sc_boss_data[area_id].obj_code;

  ONOUE_Printf("見た目を設定\n");

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

