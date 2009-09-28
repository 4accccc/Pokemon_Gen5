//======================================================================
/**
 * @file  field_player_grid_event.c
 * @brief グリッド専用 フィールドプレイヤー制御　イベント関連
 * @author kagaya
 */
//======================================================================
#include <gflib.h>

#include "fieldmap.h"
#include "fldmmdl.h"
#include "field_player.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "fieldmap_ctrl_grid.h"
#include "field_player_grid.h"
#include "map_attr.h"

#include "fldeff_namipoke.h"

#include "field_sound.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================

//======================================================================
//  グリッド自機　イベント移動関連
//======================================================================
//--------------------------------------------------------------
/**
 * 自機イベント移動チェック
 * @param *gjiki FIELD_PLAYER_GRID 
 * @param dir 移動方向
 * @param evbit PLAYER_EVENTBIT
 * @retval BOOL TRUE=自機イベント移動発生
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_GRID_CheckMoveEvent(
    FIELD_PLAYER *fld_player, FIELD_PLAYER_GRID *gjiki,
    u16 dir, PLAYER_EVENTBIT evbit )
{
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  
#if 0 //強制移動イベント
 	if( Player_MoveBitCheck_Force(jiki) == TRUE ){
		if( JikiEventCheck_ForceMoveHitEvent(fsys,jiki,flag) == TRUE ){
			return( TRUE );
		}
#endif
  
#if 0 //移動開始可能チェック	
	if( Player_MoveStartCheck(jiki,dir) == FALSE ){
		return( FALSE );
	}
#endif

#if 0 //キー入力無しでも発生するイベントチェック
	if( dir == DIR_NOT ){
		if( JikiEventCheck_KeyOFFEvent(fsys,jiki,dir,flag) == TRUE ){
			return( TRUE );
		}
		
		return( FALSE );
	}
#endif

#if 0 //イベントヒットチェック
	if( JikiEventCheck_MoveHitEvent(fsys,jiki,dir,flag) == TRUE ){
		return( TRUE );
	}
#endif
	return( FALSE );
}


