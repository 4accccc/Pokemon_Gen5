//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_animal_trail.c
 *	@brief  獣道用スクリプト
 *	@author	Masayuki Onoue
 *	@date		2011.05.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/vm_cmd.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_animal_trail.h"
#include "event_mapchange.h"
#include "event_field_fade.h"
#include "field/field_const.h"
#include "system/main.h"          //HEAPID_FIELDMAP
#include "fieldmap/zone_id.h"
#include "field/tpoke_data.h"
#include "savedata/etc_save.h"
#include "animal_trail.h"
#include "arc/fieldmap/fldmmdl_objcode.h"  //for MONSTERBALL
#include "../../../resource/fldmapdata/script/d34r0101_def.h" // for SCRID_*
#include "fldmmdl_code.h"	// for MMDL_ID_ANIMAL_TRAIL_OBJ
#include "script_def.h"
#include "poke_tool/monsnum_def.h"  // for MONSNO_数字
#include "animal_trail_def.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h" // for SYS_FLAG_AT_DOWSING_HIT

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define WARP_POS_X  (15)
#define WARP_POS_Z  (27)

#include "../../../resource/animal_trail/animal_trail_pos_data.cdat"  // 帰る先データ

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	イベントチェンジ用ワーク
//=====================================
typedef struct {
  GAMESYS_WORK        *p_gsys;
  FIELDMAP_WORK       *p_field_work;
  MMDLSYS             *p_mmdlsys;
  ETC_SAVE_WORK       *p_save;
  EVENTWORK           *p_evwork;
  u8  trail_id;
}MAP_CHAGE_EVENT_WORK;

//-------------------------------------
///	マップ脱出用ワーク
//=====================================
typedef struct {
  GAMESYS_WORK        *p_gsys;
  FIELDMAP_WORK       *p_field_work;
  ETC_SAVE_WORK       *p_save;
  u8  trail_id;
}MAP_EXIT_EVENT_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT MapExitEvent( GMEVENT *event, int *seq, void *wk );
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z );
static void SetObject( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetPokemon( MAP_CHAGE_EVENT_WORK *p_wk );
static void SetItem( MMDLSYS *p_mmdlsys );
static void SetPokemonMMDL( MMDLSYS *p_mmdlsys ,u16 monsno ,u8 sex ,u16 formno ,HEAPID heapID );
static BOOL is_ex_monster( u16 monsno );


//----------------------------------------------------------------------------
/**
 *	@brief  けものみちデータ取得
 *
 *	@param	core
 *	@param	wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetAnimalTrailData( VMHANDLE* core, void* wk )
{
  AT_POKEMON_DATA *p_poke;
  ANIMAL_TRAIL_ARC_DATA data;
  u8  trail_id;
  u8 main_table_index;
  u8 data_table_index;
  ETC_SAVE_WORK *p_save;
  SCRCMD_WORK   *p_wk     = wk;
  GAMEDATA      *p_gdata  = SCRCMD_WORK_GetGameData( p_wk );
  u16 get_type            = SCRCMD_GetVMWorkValue( core, p_wk );
  u16 *p_ret_wk	          = SCRCMD_GetVMWork( core, p_wk );
  SAVE_CONTROL_WORK*      p_save_control;

  // セーブデータの取得
  p_save_control  = GAMEDATA_GetSaveControlWork( p_gdata );
  p_save          = SaveData_GetEtc( p_save_control );

  trail_id = ANIMAL_TRAIL_SAVE_GetNowAnimalTrailID(p_save);
  main_table_index = ANIMAL_TRAIL_SAVE_GetMainTableIndex( p_save ,trail_id );
  data_table_index = ANIMAL_TRAIL_SAVE_GetDataTableIndex( p_save ,trail_id );

  switch( get_type )
  {
  // モンスターナンバーを返す
  case SCR_ANIMAL_TRAIL_GET_MONSNO:
    {
      GFL_ARC_LoadData( &data, ARCID_ANIMAL_TRAIL, trail_id );
      p_poke = &data.poke_data[main_table_index];
      (*p_ret_wk) = p_poke->pokemon_id[data_table_index];
    }
    break;
  case SCR_ANIMAL_TRAIL_GET_LV:
    {
      u8 lv_range;
      u8 max_lv,min_lv;
      GFL_ARC_LoadData( &data, ARCID_ANIMAL_TRAIL, trail_id );
      p_poke = &data.poke_data[main_table_index];
      max_lv = p_poke->max_lv[data_table_index];
      min_lv = p_poke->min_lv[data_table_index];
      lv_range = ( max_lv - min_lv);
      OS_Printf("max_lv:%u\n",max_lv);
      OS_Printf("min_lv:%u\n",min_lv);
      OS_Printf("lv_range:%u\n",lv_range);
      (*p_ret_wk) = min_lv + GFUser_GetPublicRand( lv_range );
    }
    break;
  case SCR_ANIMAL_TRAIL_GET_IS_FEMALE:
    {
      u8 female_num = ANIMAL_TRAIL_SAVE_GetFemaleNum( p_save, trail_id );
      u8 is_female = FALSE;

      GFL_ARC_LoadData( &data, ARCID_ANIMAL_TRAIL, trail_id );
      p_poke = &data.poke_data[main_table_index];
      // メス判定
      if( female_num < p_poke->female_probability[data_table_index]  )
      {
        is_female = TRUE;
      }
      (*p_ret_wk) = is_female;
    }
    break;
  case SCR_ANIMAL_TRAIL_GET_ITEM_ID:
    {
      u8 table_index = main_table_index-ANIMAL_TRAIL_DATA_TABLE_3;
      GFL_ARC_LoadData( &data, ARCID_ANIMAL_TRAIL, trail_id );
      (*p_ret_wk) = data.item_id[table_index][data_table_index];
    }
    break;
  case SCR_ANIMAL_TRAIL_GET_HIDE_ITEM_ID:
    {
      u8 table_index = main_table_index-ANIMAL_TRAIL_DATA_TABLE_7;
      GFL_ARC_LoadData( &data, ARCID_ANIMAL_TRAIL, trail_id );
      (*p_ret_wk) = data.hide_item_id[table_index][data_table_index];
    }
    break;
  case SCR_ANIMAL_TRAIL_GET_IS_HIDE_ITEM:
    {
      AT_OBJ_TYPE obj_type = ANIMAL_TRAIL_SAVE_GetObjType( p_save ,trail_id );
      if( obj_type == AT_OBJ_TYPE_HIDE_ITEM )
      {
        (*p_ret_wk) = TRUE;
      }
      else
      {
        (*p_ret_wk) = FALSE;
      }
    }
    break;
  case SCR_ANIMAL_TRAIL_GET_NOW_TRAIL_ID:
    {
      (*p_ret_wk) = ANIMAL_TRAIL_SAVE_GetNowAnimalTrailID( p_save );
    }
    break;
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のけものみちに設置されているアイテムをリセットする
 *
 *	@param	core
 *	@param	wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResetNowAnimalTrailObject( VMHANDLE* core, void* wk )
{
  u8 trail_id;
  ETC_SAVE_WORK *p_save;
  SCRCMD_WORK   *p_wk     = wk;
  GAMEDATA      *p_gdata  = SCRCMD_WORK_GetGameData( p_wk );
  SAVE_CONTROL_WORK*      p_save_control;

  // セーブデータの取得
  p_save_control  = GAMEDATA_GetSaveControlWork( p_gdata );
  p_save          = SaveData_GetEtc( p_save_control );
  trail_id = ANIMAL_TRAIL_SAVE_GetNowAnimalTrailID( p_save );
  ANIMAL_TRAIL_SAVE_SetObjectSetFlag( p_save ,trail_id ,FALSE);

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  けものみちへ移動コマンド
 *
 *	@param	core
 *	@param	wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWarpAnimalTrail( VMHANDLE* core, void* wk )
{
	GMEVENT       *p_event = NULL;
  SCRCMD_WORK   *p_wk     = wk;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gdata  = SCRCMD_WORK_GetGameData( p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  u16 trail_id  = SCRCMD_GetVMWorkValue( core, p_wk );
  MAP_CHAGE_EVENT_WORK *p_ev_wk;
  SAVE_CONTROL_WORK* p_save_control;

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL,	MapWarpEvent, sizeof(MAP_CHAGE_EVENT_WORK));
  SCRIPT_CallEvent( p_scr_wk, p_event );
	
  // イベントのパラメータ設定
  p_ev_wk               = GMEVENT_GetEventWork( p_event );
  p_save_control        = GAMEDATA_GetSaveControlWork( p_gdata );
  p_ev_wk->p_gsys       = p_gsys;
  p_ev_wk->p_field_work = GAMESYSTEM_GetFieldMapWork( p_gsys );
  p_ev_wk->p_mmdlsys    = FIELDMAP_GetMMdlSys( p_ev_wk->p_field_work );
  p_ev_wk->p_save       = SaveData_GetEtc( p_save_control );
  p_ev_wk->trail_id     = trail_id;
  p_ev_wk->p_evwork     = GAMEDATA_GetEventWork( p_gdata );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップチェンジイベント
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT MapWarpEvent( GMEVENT *event, int *seq, void *wk )
{
  MAP_CHAGE_EVENT_WORK *p_wk = wk;

  switch( *seq )
  {
  case 0:
    {
      // フェードアウト
      MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( p_wk->p_field_work ) );
      GMEVENT_CallEvent( event, EVENT_FieldFadeOut_Hole( p_wk->p_gsys, p_wk->p_field_work ) );
      (*seq)++;
      break;
    }
  case 1:
    {
      // マップ移動
      VecFx32 warp_pos;       // 遷移先でのマップ位置
      SetGridPos( &warp_pos ,WARP_POS_X ,0 ,WARP_POS_Z);
      ANIMAL_TRAIL_SAVE_SetNowAnimalTrailID( p_wk->p_save ,p_wk->trail_id );  // 獣道IDを保存
      GMEVENT_CallEvent(event,
          EVENT_ChangeMapPosNoFade(p_wk->p_gsys, p_wk->p_field_work,ANIMAL_TRAIL_ZONE_ID ,&warp_pos,DIR_UP));
      (*seq)++;
      break;
    }
  case 2:
    {
      // フェードイン
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
      if ( MMDL_BLACTCONT_IsThereReserve( mmdlsys ) == TRUE )
      {
        break;
      }

      // オブジェクトの設置
      SetObject( p_wk );

      GMEVENT_CallEvent( event, EVENT_FieldFadeIn_PlayerDir( p_wk->p_gsys, p_wk->p_field_work ) );
      (*seq)++;
      break;
    }
  case 3:
    {
      // 動作モデル移動開始
      MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(p_wk->p_field_work));
      
      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  グリッド座標をFx32に変換してセットする
 *
 *	@param	*vec 
 *	@param	x     Xグリッド座標
 *	@param	y     Yグリッド座標
 *	@param	z     Xグリッド座標
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetGridPos( VecFx32 *vec ,s32 x ,s32 y ,s32 z )
{
  vec->x = (x * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->y = (y * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
  vec->z = (z * FIELD_CONST_GRID_SIZE + FIELD_CONST_GRID_HALF_SIZE ) << FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief  獣道内にオブジェクトを配置する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetObject( MAP_CHAGE_EVENT_WORK *p_wk )
{
  // 配置するオブジェクトタイプごとの処理を行う
  AT_OBJ_TYPE obj_type = ANIMAL_TRAIL_SAVE_GetObjType( p_wk->p_save ,p_wk->trail_id );
  switch( obj_type )
  {
  case AT_OBJ_TYPE_MONSTER:
    OS_Printf("ポケモン\n");
    SetPokemon( p_wk );
    EVENTWORK_ResetEventFlag(p_wk->p_evwork ,SYS_FLAG_AT_DOWSING_HIT);
    break;
  case AT_OBJ_TYPE_ITEM:
    OS_Printf("配置アイテム\n");
    SetItem( p_wk->p_mmdlsys );
    EVENTWORK_ResetEventFlag(p_wk->p_evwork ,SYS_FLAG_AT_DOWSING_HIT);
    break;
  case AT_OBJ_TYPE_HIDE_ITEM:
    // 隠しアイテムイベントはイベントエディタで配置済み
    OS_Printf("隠しアイテム\n");
    EVENTWORK_SetEventFlag(p_wk->p_evwork ,SYS_FLAG_AT_DOWSING_HIT);
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  抽選されたデータからポケモンを設置する
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetPokemon( MAP_CHAGE_EVENT_WORK *p_wk )
{
  int i,n;
  AT_POKEMON_DATA *p_poke;
  ANIMAL_TRAIL_ARC_DATA data;
  u8 main_table_index = ANIMAL_TRAIL_SAVE_GetMainTableIndex( p_wk->p_save ,p_wk->trail_id );
  u8 data_table_index = ANIMAL_TRAIL_SAVE_GetDataTableIndex( p_wk->p_save ,p_wk->trail_id );
  u8 female_num = ANIMAL_TRAIL_SAVE_GetFemaleNum( p_wk->p_save, p_wk->trail_id );
  u8 is_female = FALSE;

  // アーカイブ読み込み
  GF_ASSERT( main_table_index < ANIMAL_TRAIL_DATA_TABLE_3 );
  GFL_ARC_LoadData( &data, ARCID_ANIMAL_TRAIL, p_wk->trail_id );
  p_poke = &data.poke_data[main_table_index];

  // メス判定
  if( female_num < p_poke->female_probability[data_table_index]  )
  {
    is_female = TRUE;
  }

  // ポケモン動作モデルの追加
  SetPokemonMMDL( p_wk->p_mmdlsys ,p_poke->pokemon_id[data_table_index] ,is_female ,p_poke->forum[data_table_index] ,HEAPID_FIELDMAP );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンの動作モデルを設置する
 *
 *	@param	*p_mmdlsys  動作モデルシステム
 *	@param	monsno      モンスターナンバー
 *	@param	sex         性別
 *	@param	formno      フォルムナンバー
 *	@param	heapID      ヒープID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetPokemonMMDL( MMDLSYS *p_mmdlsys ,u16 monsno ,u8 sex ,u16 formno ,HEAPID heapID )
{
  u16 obj_code;
  MMDL *mmdl;

  if( is_ex_monster( monsno ) == FALSE )
  {
    // obj_codeの取得
    TPOKE_DATA *p_tpdata = TPOKE_DATA_Create( HEAPID_FIELDMAP );
    obj_code = TPOKE_DATA_GetObjCode( p_tpdata ,monsno ,sex ,formno );
    TPOKE_DATA_Delete( p_tpdata );
    GF_ASSERT( obj_code );

    // 動作モデルの設置(ポケモン)
    mmdl = MMDLSYS_AddMMdlParam( p_mmdlsys, ANIMAL_TRAIL_OBJ_POS_GX, ANIMAL_TRAIL_OBJ_POS_GZ, DIR_DOWN,
        MMDL_ID_ANIMAL_TRAIL_OBJ ,obj_code, MV_DMY ,ANIMAL_TRAIL_ZONE_ID  );
  }
  else
  {
    // 動作モデルの設置(タマゲタケ・モロバレル/モンスターボール)
    mmdl = MMDLSYS_AddMMdlParam( p_mmdlsys, ANIMAL_TRAIL_OBJ_POS_GX, ANIMAL_TRAIL_OBJ_POS_GZ, DIR_DOWN,
        MMDL_ID_ANIMAL_TRAIL_OBJ ,MONSTERBALL, MV_DMY ,ANIMAL_TRAIL_ZONE_ID  );
  }
  MMDL_SetEventID( mmdl, SCRID_ANIMAL_TRAIL_BATTLE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  特殊モンスター判定
 *
 *	@param	monsno 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL is_ex_monster( u16 monsno )
{
  // タマゲタケ・モロバレル
  if( monsno != MONSNO_581 && monsno != MONSNO_580 )
  {
    return FALSE;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アイテムを設置
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetItem( MMDLSYS *p_mmdlsys )
{
  // 動作モデルの設置
  MMDL *mmdl = MMDLSYS_AddMMdlParam( p_mmdlsys, ANIMAL_TRAIL_OBJ_POS_GX, ANIMAL_TRAIL_OBJ_POS_GZ, DIR_DOWN,
      MMDL_ID_ANIMAL_TRAIL_OBJ ,MONSTERBALL, MV_DMY ,ANIMAL_TRAIL_ZONE_ID  );
  MMDL_SetEventID( mmdl, SCRID_ANIMAL_TRAIL_NORMAL_ITEM );
}

//----------------------------------------------------------------------------
/**
 *	@brief  けものみちから脱出する
 *
 *	@param	core
 *	@param	wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdExitAnimalTrail( VMHANDLE* core, void* wk )
{
	GMEVENT       *p_event = NULL;
  SCRCMD_WORK   *p_wk     = wk;
  GAMESYS_WORK  *p_gsys   = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gdata  = SCRCMD_WORK_GetGameData( p_wk );
  SCRIPT_WORK   *p_scr_wk = SCRCMD_WORK_GetScriptWork( p_wk );
  MAP_EXIT_EVENT_WORK *p_ev_wk;
  SAVE_CONTROL_WORK* p_save_control;

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL,	MapExitEvent, sizeof(MAP_EXIT_EVENT_WORK));
  SCRIPT_CallEvent( p_scr_wk, p_event );
	
  // イベントのパラメータ設定
  p_ev_wk               = GMEVENT_GetEventWork( p_event );
  p_save_control        = GAMEDATA_GetSaveControlWork( p_gdata );
  p_ev_wk->p_gsys       = p_gsys;
  p_ev_wk->p_field_work = GAMESYSTEM_GetFieldMapWork( p_gsys );
  p_ev_wk->p_save       = SaveData_GetEtc( p_save_control );
  p_ev_wk->trail_id     = ANIMAL_TRAIL_SAVE_GetNowAnimalTrailID( p_ev_wk->p_save );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップ脱出イベント
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT MapExitEvent( GMEVENT *event, int *seq, void *wk )
{
  MAP_EXIT_EVENT_WORK *p_wk = wk;

  switch( *seq )
  {
  case 0:
    {
      // フェードアウト
      MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( p_wk->p_field_work ) );
      GMEVENT_CallEvent( event, EVENT_FieldFadeOut_Hole( p_wk->p_gsys, p_wk->p_field_work ) );
      (*seq)++;
      break;
    }
  case 1:
    {
      // マップ移動
      VecFx32 warp_pos;       // 遷移先でのマップ位置
      SetGridPos( &warp_pos ,sc_exit_data[p_wk->trail_id].gx ,sc_exit_data[p_wk->trail_id].gy ,sc_exit_data[p_wk->trail_id].gz);
      GMEVENT_CallEvent(event,
          EVENT_ChangeMapPosNoFade(p_wk->p_gsys, p_wk->p_field_work,sc_exit_data[p_wk->trail_id].zone_id ,&warp_pos,DIR_DOWN));
      (*seq)++;
      break;
    }
  case 2:
    {
      // フェードイン
      MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( p_wk->p_field_work );
      if ( MMDL_BLACTCONT_IsThereReserve( mmdlsys ) == TRUE )
      {
        break;
      }

      GMEVENT_CallEvent( event, EVENT_FieldFadeIn_PlayerDir( p_wk->p_gsys, p_wk->p_field_work ) );
      (*seq)++;
      break;
    }
  case 3:
    {
      // 動作モデル移動開始
      MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(p_wk->p_field_work));
      
      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}

