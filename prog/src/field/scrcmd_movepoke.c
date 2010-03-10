//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_movepoke.c
 *	@brief  移動ポケモン  スクリプトコマンド
 *	@author	tomoya takahashi
 *	@date		2010.03.09
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

#include "scrcmd_movepoke.h"

#include "event_movepoke.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

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





// 移動ポケモン　ポケモンワーク生成・破棄
//----------------------------------------------------------------------------
/**
 *	@brief  移動ポケモンワーク　生成
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdMovePoke_Create( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  EV_MOVEPOKE_WORK* p_movepoke;
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  HEAPID heapID = FIELDMAP_GetHeapID( fparam->fieldMap );
  u16 id = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  VecFx32 pos;


  // 自機の位置を基準に動く
  FIELD_PLAYER_GetPos( p_player, &pos );

  p_movepoke = EVENT_MOVEPOKE_Create( fparam->fieldMap, id, &pos, heapID );
  SCRIPT_SetMovePokeWork( sc, p_movepoke );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動ポケモンワーク　破棄
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdMovePoke_Delete( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  EV_MOVEPOKE_WORK* p_movepoke;

  p_movepoke = SCRIPT_GetMovePokeWork( sc );
  GF_ASSERT( p_movepoke );

  EVENT_MOVEPOKE_Delete( p_movepoke );

  SCRIPT_SetMovePokeWork( sc, NULL );

  return VMCMD_RESULT_CONTINUE;
}


//　移動ぽけもん　アニメーション開始
//----------------------------------------------------------------------------
/**
 *	@brief  移動ポケモンワーク　アニメーション開始
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdMovePoke_StartAnime( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  EV_MOVEPOKE_WORK* p_movepoke;
  u16 index = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数

  p_movepoke = SCRIPT_GetMovePokeWork( sc );
  GF_ASSERT( p_movepoke );

  GF_ASSERT( index < EV_MOVEPOKE_ANIME_MAX );
  EVENT_MOVEPOKE_StartAnime( p_movepoke, index );

  return VMCMD_RESULT_CONTINUE;
}


//　移動ぽけもん　アニメーション終了待ち
//-------------------------------------
///	ワーク
//=====================================
typedef struct {
  EV_MOVEPOKE_WORK* p_movepoke;
} SCR_EVMOVEPOKE_WORK;

static GMEVENT_RESULT EVMovePoke_AnimeEndWait( GMEVENT * event, int *seq, void*work )
{
  SCR_EVMOVEPOKE_WORK* p_wk =  work;

  if( EVENT_MOVEPOKE_IsAnimeEnd( p_wk->p_movepoke ) == TRUE ){
		return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動ポケモンワーク　アニメーション終了待ち
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdMovePoke_IsEnd( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  EV_MOVEPOKE_WORK* p_movepoke;
	GMEVENT * event = GMEVENT_Create( gsys, NULL, EVMovePoke_AnimeEndWait, sizeof(SCR_EVMOVEPOKE_WORK) );
	SCR_EVMOVEPOKE_WORK * p_wk = GMEVENT_GetEventWork(event);

  p_movepoke = SCRIPT_GetMovePokeWork( sc );
  GF_ASSERT( p_movepoke );

  p_wk->p_movepoke = p_movepoke;
  
  SCRIPT_CallEvent( sc, event );
  
  return VMCMD_RESULT_SUSPEND;
}




