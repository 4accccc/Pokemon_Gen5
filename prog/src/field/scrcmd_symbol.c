//==============================================================================
/**
 * @file    scrcmd_symbol.c
 * @brief   侵入：シンボルポケモン
 * @author  matsuda --> tamada
 * @date    2010.03.16(火)
 */
//==============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_symbol.h"
#include "savedata/symbol_save.h"
#include "savedata/symbol_save_field.h"
#include "event_symbol.h"

#include "symbol_map.h"
#include "symbol_pokemon.h"

#include "field/intrude_symbol.h"
#include "field_comm/intrude_work.h"    //Intrude_Check_CommConnect
#include "field_comm/intrude_field.h"   //SYMBOL_MAP_DOWN_ENTRANCE_X

#include "arc/fieldmap/zone_id.h"
#include "event_mapchange.h"

#include "../../../resource/fldmapdata/flagwork/work_define.h"  //SCWK_TARGET_OBJID

//==============================================================================
//==============================================================================
static MMDL * getPokeMMdl( SCRCMD_WORK * work, u16 obj_id );
static void getFrontSymPoke( SCRCMD_WORK * work, SYMBOL_POKEMON * sympoke );
static POKEMON_PARAM * createPokemon( SCRCMD_WORK * work, MMDL * mmdl, HEAPID heapID, SYMBOL_POKEMON *sympoke );
static void sendDataChange( SCRCMD_WORK * work );


//==============================================================================
//==============================================================================
//==================================================================
/**
 * シンボルポケモンバトル
 *
 * @param   core		
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 *
 * 捕獲した場合、結果にTRUEを。捕獲しなかった場合、FALSEになります
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolPokeBattle( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  u16 obj_id = SCRCMD_GetVMWorkValue(core,work);
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  //結果を返すワーク
  SYMBOL_POKEMON sympoke;

  MMDL * mmdl = getPokeMMdl( work, obj_id );
  if ( mmdl )
  {
    POKEMON_PARAM * pp = createPokemon( work, mmdl, HEAPID_PROC, &sympoke );
    if ( pp )
    {
      SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
      GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
      FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
      GMEVENT * event = EVENT_SymbolPokeBattle( gsys, fieldmap, pp, sympoke, ret_wk, HEAPID_PROC );
      SCRIPT_CallEvent( scw, event );
    }
  }

  return VMCMD_RESULT_SUSPEND;
}

//==================================================================
/**
 * シンボルポケモンセッティング
 *
 * @param   core		仮想マシン制御構造体へのポインタ
 * @param   wk		
 *
 * @retval  VMCMD_RESULT		
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolMapPokeSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  INTRUDE_SYMBOL_WORK * isw;
  u32 start_no;
  
  isw = SYMBOLMAP_AllocSymbolWork( heapID, gsys, &start_no );
  if ( isw )
  {
    SYMBOLPOKE_Add( fieldmap, start_no, isw->spoke_array, isw->num, isw->symbol_map_id );
    GFL_HEAP_FreeMemory( isw );
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * @brief シンボルポケモンを入手する
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolPokeGet( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  u16 obj_id = SCRCMD_GetVMWorkValue(core,work);

  GAMEDATA *  gamedata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(gamedata);
  SYMBOL_SAVE_WORK *symbol_save = SymbolSave_GetSymbolData(sv_ctrl);

  MMDL * mmdl = getPokeMMdl( work, obj_id );

  if ( mmdl ) {
    u32 no = SYMBOLPOKE_GetSymbolNo( mmdl );
    SymbolSave_DataShift( symbol_save, no );
    sendDataChange( work ); //シンボルポケモンの更新を通信で通知する
  }

  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * @brief シンボルポケモンを移動する
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolMapMovePokemon( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  u16 obj_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );

  GAMEDATA *  gamedata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(gamedata);
  SYMBOL_SAVE_WORK *symbol_save = SymbolSave_GetSymbolData(sv_ctrl);

  MMDL * mmdl = getPokeMMdl( work, obj_id );

  if ( mmdl ) {
    u32 no = SYMBOLPOKE_GetSymbolNo( mmdl );
    *ret_wk = SymbolSave_Field_MoveAuto( symbol_save, no );
    sendDataChange( work ); //シンボルポケモンの更新を通信で通知する
  } else {
    *ret_wk = FALSE;
  }

  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * @brief シンボルエンカウント：対象ポケモンの名前をセット
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolMapSetMonsName( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK *work = wk;
  u16 obj_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 idx = SCRCMD_GetVMWorkValue( core, wk );
  SYMBOL_POKEMON sympoke;
  
  MMDL * mmdl = getPokeMMdl( work, obj_id );

  if ( mmdl )
  {
    HEAPID     heap_id = SCRCMD_WORK_GetHeapID( work );
    POKEMON_PARAM * pp = createPokemon( work, mmdl, heap_id, &sympoke );
    if ( pp )
    {
      SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
      WORDSET *wordset    = SCRIPT_GetWordSet( sc );

      WORDSET_RegisterPokeMonsName( wordset, idx, pp );
      GFL_HEAP_FreeMemory( pp );
    }
  }
  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * @brief シンボルエンカウントマップ：情報取得
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolMapGetInfo( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  u16 mode = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  GAMESYS_WORK *  gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *  gamedata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(gamedata);
  SYMBOL_SAVE_WORK *symbol_save = SymbolSave_GetSymbolData(sv_ctrl);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	 
  switch ( mode )
  {
  case SCR_SYMMAP_INFO_IS_MINE:
    //※下記関数で通信時も非通信時も自分かどうかの判定ができる
    if ( IntrudeSymbol_CheckIntrudeNetID( game_comm, gamedata ) == INTRUDE_NETID_NULL ) {
      *ret_wk = TRUE;
    } else {
      *ret_wk = FALSE;
    }
    break;
  case SCR_SYMMAP_INFO_IS_KEEPZONE:
    {
      SYMBOL_MAP_ID sid = GAMEDATA_GetSymbolMapID( gamedata );
      *ret_wk = SYMBOLMAP_IsKeepzoneID( sid );
    }
    break;
  case SCR_SYMMAP_INFO_IS_ENTRANCE:
    {
      SYMBOL_MAP_ID sid = GAMEDATA_GetSymbolMapID( gamedata );
      *ret_wk = SYMBOLMAP_IsEntranceID( sid );
    }
    break;
  case SCR_SYMMAP_INFO_FRONT_MONSNO:
    {
      SYMBOL_POKEMON sympoke;
      getFrontSymPoke( work, &sympoke );
      *ret_wk = sympoke.monsno;
    }
    break;
  case SCR_SYMMAP_INFO_FRONT_FORMNO:
    {
      SYMBOL_POKEMON sympoke;
      getFrontSymPoke( work, &sympoke );
      *ret_wk = sympoke.form_no;
    }
    break;
  }
  return VMCMD_RESULT_CONTINUE;
}

//==================================================================
/**
 * @brief シンボルエンカウントマップ：マップ遷移
 */
//==================================================================
VMCMD_RESULT EvCmdSymbolMapWarp( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *   work = wk;
  SCRIPT_WORK *     sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *  gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *  gamedata = SCRCMD_WORK_GetGameData( work );
  u16         warp_dir = SCRCMD_GetVMWorkValue( core, work );

  SYMBOL_MAP_ID sid;
  GMEVENT * new_event;

  sid = GAMEDATA_GetSymbolMapID( gamedata );
  OS_TPrintf( "SCRCMD:SYMMAP_WARP:id=%2d, dir=%d\n", sid, warp_dir );

  if ( warp_dir == DIR_DOWN && SYMBOLMAP_IsEntranceID(sid) )
  { //入ったところのマップで下向き→パレスに戻る
    VecFx32 warp_pos, player_pos;
    FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
    FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer( fieldmap );
    GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
    FIELD_PLAYER_GetPos( player, &player_pos );
    IntrudeSymbol_GetPosPalaceForestDoorway( game_comm, gamedata, &warp_pos );
    //入った位置によって出現先もずらす
    warp_pos.x += player_pos.x - SYMBOL_MAP_DOWN_ENTRANCE_X;
    new_event = EVENT_ChangeMapPos( gsys, fieldmap, ZONE_ID_PALACE01, &warp_pos, DIR_DOWN, FALSE );
  }
  else
  { //パレスの森の中を移動
    if ( warp_dir != DIR_NOT ) {
      sid = SYMBOLMAP_GetNextSymbolMapID( gsys, GAMEDATA_GetSymbolMapID( gamedata ), warp_dir );
    }
    new_event = EVENT_SymbolMapWarpEasy( gsys, warp_dir, sid );
    OS_TPrintf("SCRCMD:SYMMAP_WARP:next id = %d\n", sid );
  }
  SCRIPT_CallEvent( sc, new_event );
  return VMCMD_RESULT_SUSPEND;
}


//==============================================================================
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief 話しかけ対象のポケモン動作モデル取得
 */
//--------------------------------------------------------------
static MMDL * getPokeMMdl( SCRCMD_WORK * work, u16 obj_id )
{
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  MMDLSYS *mmdlsys = FIELDMAP_GetMMdlSys( fieldmap );
  MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
  GF_ASSERT( mmdl != NULL );
  return mmdl;
}

//--------------------------------------------------------------
/**
 * @brief 話しかけ対象のSYMBOL_POKEMONデータを取得
 */
//--------------------------------------------------------------
static void getFrontSymPoke( SCRCMD_WORK * work, SYMBOL_POKEMON * sympoke )
{
  SCRIPT_WORK * sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  u16 obj_id = SCRIPT_GetEventWorkValue( sc, gamedata, SCWK_TARGET_OBJID );
  MMDL * mmdl = getPokeMMdl( work, obj_id );
  SYMBOLPOKE_GetParam( sympoke, mmdl );
}

//--------------------------------------------------------------
/**
 * @brief シンボルポケモンデータからポケモン生成
 */
//--------------------------------------------------------------
static POKEMON_PARAM * createPokemon( SCRCMD_WORK * work, MMDL * mmdl, HEAPID heapID, SYMBOL_POKEMON *sympoke )
{
    GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
    POKEMON_PARAM * pp;
    SYMBOLPOKE_GetParam( sympoke, mmdl );
    pp = SYMBOLPOKE_PP_Create( HEAPID_PROC, gamedata, sympoke );
    return pp;
}

//--------------------------------------------------------------
/**
 * @brief シンボルポケモンの状況が更新されたことを通知する
 */
//--------------------------------------------------------------
static void sendDataChange( SCRCMD_WORK * work )
{
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(gamedata);
  SYMBOL_SAVE_WORK *symbol_save = SymbolSave_GetSymbolData(sv_ctrl);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr( gsys );

  if ( 1 )//IntrudeSymbol_CheckIntrudeNetID( game_comm, gamedata ) != INTRUDE_NETID_NULL )
  {
    INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect( game_comm );
    if ( intcomm == NULL )
    { //通信エラー
      return;
    }
    IntrudeSymbol_SendSymbolDataChange( intcomm, GAMEDATA_GetSymbolMapID( gamedata ) );
    OS_TPrintf( "送信リクエスト：シンボルポケモン更新\n" );
  }
}





