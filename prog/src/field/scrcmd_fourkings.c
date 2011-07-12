//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_fourkings.c
 *	@brief  四天王スクリプトコマンド
 *	@author	tomoya takahashi
 *	@date		2009.11.25
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"

#include "scrcmd_fourkings.h"

#include "fourkings_evil.h"
#include "fourkings_esp.h"
#include "fourkings_ghost.h"
#include "fourkings_fight.h"

#include "rail_line/c09r0101.h" //for RAIL
#include "rail_line/c09r0201.h" //for RAIL
#include "rail_line/c09r0301.h" //for RAIL
#include "rail_line/c09r0401.h" //for RAIL

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
// 頂上カメラID
static const u32 sc_EV_DEMO_CAMERA_ID_TBL[ FIELD_EVENT_FOURKINGS_MAX ] = 
{
  28,
  29,
  30,
  31
};

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// レールOFF
static void FOURKINGS_RailOff( FIELDMAP_WORK *p_fieldmap, u32 fourkings_no );
// 頂上カメラID取得
static u32 FOURKINGS_GetCameraID( u32 fourkings_no );

//----------------------------------------------------------------------------
/**
 *	@brief  四天王部屋 レールOFF
 *
 *	@param	core
 *	@param	*wk 
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_RailOff( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  u16          index = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数

  GF_ASSERT( index > 0 );

  // ZONE ID の IDでわたってくる（1オリジン）ので、-1する
  index -= 1;

  // レールOFF
  FOURKINGS_RailOff( fieldWork, index );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  四天王頂上カメラの指定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_SetCameraParam( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldWork = GAMESYSTEM_GetFieldMapWork( gsys );
  u16          index = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( fieldWork );

  GF_ASSERT( index > 0 );

  // ZONE ID の IDでわたってくる（1オリジン）ので、-1する
  index -= 1;


  // カメラの状態を頂上にする
  FIELD_CAMERA_SetCameraType( p_camera, FOURKINGS_GetCameraID( index ) );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  四天王部屋サウンドシステム　設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_SetSoundSystem( VMHANDLE *core, void *wk )
{
  //

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  四天王各部屋のレールOFF
 *
 *	@param	FIELDMAP_WORK *p_fieldmap
 *	@param	fourkings_no 
 */
//-----------------------------------------------------------------------------
static void FOURKINGS_RailOff( FIELDMAP_WORK *p_fieldmap, u32 fourkings_no )
{
  int i;
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( p_fieldmap );

  GF_ASSERT( fourkings_no < FIELD_EVENT_FOURKINGS_MAX );

  // 頂上のレールにはPOSを置いているのでOFFにしない
  FLDNOGRID_MAPPER_SetLineActive( p_mapper, div_1_RE_LINE_0, FALSE );
  FLDNOGRID_MAPPER_SetLineActive( p_mapper, div_2_RE_LINE_0, FALSE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  四天王各部屋の頂上カメラIDを取得
 *
 *	@param	fourkings_no 四天王ナンバー
 *
 *	@return カメラID
 */
//-----------------------------------------------------------------------------
static u32 FOURKINGS_GetCameraID( u32 fourkings_no )
{
  GF_ASSERT( fourkings_no < FIELD_EVENT_FOURKINGS_MAX );
  return sc_EV_DEMO_CAMERA_ID_TBL[ fourkings_no ];
}


//==== 悪 =====
//----------------------------------------------------------------------------
/**
 *	@brief  炎のアニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Evil_Fire( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // 0:階段下アニメ再生
  // 1:階段上アニメ再生
  id  = VMGetU16( core );

  FOURKINGS_EVIL_FireAnm( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  爪のアニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Evil_Claw( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // 0:開くアニメ再生
  // 1:閉じるアニメ再生
  id  = VMGetU16( core );

  FOURKINGS_EVIL_ClawAnm( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}

//==== エスパー =====
//----------------------------------------------------------------------------
/**
 *	@brief  階段演出
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Esp_Stairs( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // 0:表示
  // 1:消滅
  id  = VMGetU16( core );

  FOURKINGS_ESP_StairsAnm( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  花演出
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Esp_Flower( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  event = FOURKINGS_ESP_FlowerAnm( gsys );

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/**
 *	@brief  キューブアニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Esp_Cube( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  FOURKINGS_ESP_CubeAnm( gsys );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カトレア演出
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Esp_Cattleya( VMHANDLE *core, void *wk )
{
  int obj_id;
  int fall_spd;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // カトレアのOBJID
  obj_id  = VMGetU16( core );
  // 下降のスピード
  fall_spd  = VMGetU16( core );

  FOURKINGS_ESP_Cattleya( gsys, obj_id, fall_spd );

  return VMCMD_RESULT_CONTINUE;
}

//==== ゴースト =====
//----------------------------------------------------------------------------
/*	@brief  もや晴れるアニメ */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Ghost_Mist( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  event = FOURKINGS_GHOST_MistAnm( gsys );

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/*	@brief  炎アニメ */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Ghost_Fire( VMHANDLE *core, void *wk )
{
  int fire_id;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // 再生する炎のID
  fire_id = VMGetU16( core );

  FOURKINGS_GHOST_FireAnm( gsys, fire_id );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/*	@brief  本落ちるアニメ */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Ghost_BookFall( VMHANDLE *core, void *wk )
{
  int wait;
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // 暗転までのウェイト値
  wait = VMGetU16( core );

  event = FOURKINGS_GHOST_BookFall( gsys, wait );

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/*	@brief  ランプアニメ */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Ghost_Lamp( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  event = FOURKINGS_GHOST_LampAnm( gsys );

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//==== 階段 =====
//----------------------------------------------------------------------------
/**
 *	@brief  ドア演出
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Fight_Door( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  FOURKINGS_FIGHT_DoorAnm( gsys );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  階段演出
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Fight_Stairs( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  // 0:表示
  // 1:消滅
  id  = VMGetU16( core );

  FOURKINGS_FIGHT_StairsAnm( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライト点灯演出
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Fight_Light( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  FOURKINGS_FIGHT_LightAnm( gsys );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ステージ落ちる演出
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdFourKings_Fight_Stage( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  event = FOURKINGS_FIGHT_StageFall( gsys );

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
