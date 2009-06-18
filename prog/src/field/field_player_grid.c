//======================================================================
/**
 * @file	field_player_grid.c
 * @brief グリッド専用 フィールドプレイヤー制御
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

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
///	PLAYER_MOVE
//--------------------------------------------------------------
typedef enum
{
	PLAYER_MOVE_STOP = 0,
	PLAYER_MOVE_WALK,
	PLAYER_MOVE_TURN,
	PLAYER_MOVE_HITCH,
}PLAYER_MOVE;

//--------------------------------------------------------------
///	PLAYER_SET
//--------------------------------------------------------------
typedef enum
{
	PLAYER_SET_NON = 0,
	PLAYER_SET_STOP,
	PLAYER_SET_WALK,
	PLAYER_SET_TURN,
	PLAYER_SET_HITCH,
  PLAYER_SET_JUMP,
}PLAYER_SET;

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
/// FIELD_PLAYER_GRID
//--------------------------------------------------------------
struct _TAG_FIELD_PLAYER_GRID
{
	int move_state;
	fx16 scale_size;	//FX16_ONE*8
  FIELD_PLAYER_GRID_REQBIT req_bit;

	FIELD_PLAYER *fld_player;
	FIELDMAP_WORK *fieldWork;
};

//======================================================================
//	proto
//======================================================================
static void jikiMove_Normal(
		FIELD_PLAYER_GRID *g_jiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag );

static PLAYER_SET gjiki_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjiki_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjiki_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjiki_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjiki_SetMove_Non(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Stop(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Walk(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Turn(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Hitch(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Jump(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void jikiMove_Cycle(
		FIELD_PLAYER_GRID *g_jiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag );

static PLAYER_SET gjikiCycle_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjikiCycle_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjikiCycle_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjikiCycle_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjikiCycle_SetMove_Non(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Stop(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Walk(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Turn(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Hitch(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Jump(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjiki_Sound_MoveStop( void );
static void gjiki_Sound_Move( void );

//======================================================================
//	グリッド移動 フィールドプレイヤー制御
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　生成
 * @param	fld_player FIELD_PLAYER
 * @param heapID HEAPID
 * @retval FIELD_PLAYER_GRID
 */
//--------------------------------------------------------------
FIELD_PLAYER_GRID * FIELD_PLAYER_GRID_Init(
		FIELD_PLAYER *fld_player, HEAPID heapID )
{
	FIELD_PLAYER_GRID *g_jiki;
	
	g_jiki = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_PLAYER_GRID) );
	g_jiki->fld_player = fld_player;
	g_jiki->fieldWork = FIELD_PLAYER_GetFieldMapWork( fld_player );
	g_jiki->scale_size = FX16_ONE*8-1;

#if 0  
  {
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
    FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( g_jiki->fieldWork );
    FLDEFF_SHADOW_SetMMdl( fmmdl, fectrl );
  }
#endif

//SetGridPlayerActTrans( g_jiki->pActCont, &g_jiki->vec_pos );
	return( g_jiki );
}

//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　削除
 * @param	g_jiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_Delete( FIELD_PLAYER_GRID *g_jiki )
{
	GFL_HEAP_FreeMemory( g_jiki );
}

//======================================================================
//	グリッド移動 フィールドプレイヤー制御　移動
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動 フィールドプレイヤー制御　移動
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_Move(
		FIELD_PLAYER_GRID *g_jiki, int key_trg, int key_cont )
{
	u16 dir;
	BOOL debug_flag;
  PLAYER_MOVE_FORM form;
  
	dir = DIR_NOT;
	if( (key_cont&PAD_KEY_UP) ){
		dir = DIR_UP;
	}else if( (key_cont&PAD_KEY_DOWN) ){
		dir = DIR_DOWN;
	}else if( (key_cont&PAD_KEY_LEFT) ){
		dir = DIR_LEFT;
	}else if( (key_cont&PAD_KEY_RIGHT) ){
		dir = DIR_RIGHT;
	}
	
	debug_flag = FALSE;
#ifdef PM_DEBUG
	if( key_cont & PAD_BUTTON_R ){
		debug_flag = TRUE;
	}
#endif

#if 0 
  if( key_trg & PAD_BUTTON_SELECT ){
    PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( g_jiki->fld_player );
    switch( form ){
    case PLAYER_MOVE_FORM_NORMAL:
      FIELD_PLAYER_GRID_SetRequest( g_jiki, FIELD_PLAYER_GRID_REQBIT_CYCLE );
      break;
    case PLAYER_MOVE_FORM_CYCLE:
      FIELD_PLAYER_GRID_SetRequest( g_jiki, FIELD_PLAYER_GRID_REQBIT_NORMAL );
      break;
    }
  }
#endif
  
  FIELD_PLAYER_GRID_UpdateRequest( g_jiki );
  
  form = FIELD_PLAYER_GetMoveForm( g_jiki->fld_player );
  switch( form ){
  case PLAYER_MOVE_FORM_NORMAL:
    jikiMove_Normal( g_jiki, key_trg, key_cont, dir, debug_flag );
	  break;
  case PLAYER_MOVE_FORM_CYCLE:
    jikiMove_Cycle( g_jiki, key_trg, key_cont, dir, debug_flag );
	  break;
  default:
    GF_ASSERT( 0 );
  }
}

//======================================================================
//  グリッド移動　フィールドプレイヤー制御　通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　通常移動
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
static void jikiMove_Normal(
		FIELD_PLAYER_GRID *g_jiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag )
{
	PLAYER_SET set;
	MMDL *fmmdl = FIELD_PLAYER_GetMMdl( g_jiki->fld_player );
   
	set = PLAYER_SET_NON;
	switch( g_jiki->move_state ){
	case PLAYER_MOVE_STOP:
		set = gjiki_CheckMoveStart_Stop(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_WALK:
		set = gjiki_CheckMoveStart_Walk(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_TURN:
		set = gjiki_CheckMoveStart_Turn(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_HITCH:
		set = gjiki_CheckMoveStart_Hitch(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	default:
		GF_ASSERT( 0 );
	}
	
	switch( set ){
	case PLAYER_SET_NON:
		gjiki_SetMove_Non(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_STOP:
		gjiki_SetMove_Stop(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_WALK:
		gjiki_SetMove_Walk(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_TURN:
		gjiki_SetMove_Turn(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_HITCH:
		gjiki_SetMove_Hitch(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
  case PLAYER_SET_JUMP:
		gjiki_SetMove_Jump(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
    break;
	}
}

//======================================================================
//	移動開始チェック　通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始チェック　停止中
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(fmmdl) == TRUE ){
		if( dir != DIR_NOT ){
			u16 old_dir;
			old_dir = MMDL_GetDirDisp( fmmdl );
			
			if( dir != old_dir && debug_flag == FALSE ){
				return( PLAYER_SET_TURN );
			}
			
			return( gjiki_CheckMoveStart_Walk(
				g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
		}
		
		return( PLAYER_SET_STOP );
	}
	
	return( PLAYER_SET_NON );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　移動中
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT )
  {
		return( gjiki_CheckMoveStart_Stop(
			g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	{
		u32 hit = MMDL_HitCheckMoveDir( fmmdl, dir );
		
		if( debug_flag == TRUE )
    {
			if( hit != MMDL_MOVEHITBIT_NON &&
          !(hit&MMDL_MOVEHITBIT_OUTRANGE) )
      {
				hit = MMDL_MOVEHITBIT_NON;
			}
		}
		
		if( hit == MMDL_MOVEHITBIT_NON )
    {
			return( PLAYER_SET_WALK );
		}
    
    if( (hit & MMDL_MOVEHITBIT_ATTR) )
    {
      BOOL ret;
      u32 attr;
      VecFx32 pos;
      
      MMDL_GetVectorPos( fmmdl, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      ret = MMDL_GetMapPosAttr( fmmdl, &pos, &attr );
      
      if( ret == TRUE )
      {
        u16 attr_dir = DIR_NOT;
        MAPATTR_VAL val = MAPATTR_GetAttrValue( attr );
        MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
        
        if( MAPATTR_CheckAttrValueJumpUp(val) ){
          attr_dir = DIR_UP;
        }else if( MAPATTR_CheckAttrValueJumpDown(val) ){
          attr_dir = DIR_DOWN;
        }else if( MAPATTR_CheckAttrValueJumpLeft(val) ){
          attr_dir = DIR_LEFT;
        }else if( MAPATTR_CheckAttrValueJumpRight(val) ){
          attr_dir = DIR_RIGHT;
        }
        
        if( attr_dir != DIR_NOT && attr_dir == dir ) //ジャンプ方向一致
        {
          return( PLAYER_SET_JUMP );
        }
      }
    }
	}
	
	return( PLAYER_SET_HITCH );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　振り向き中
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjiki_CheckMoveStart_Stop(
			g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjiki_CheckMoveStart_Walk(
		g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　障害物ヒット中
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( fmmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( fmmdl );
	  return( gjiki_CheckMoveStart_Walk(
		  g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
	if( MMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjiki_CheckMoveStart_Stop(
			g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjiki_CheckMoveStart_Walk(
		g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
//	移動セット　通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Non(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Stop(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	if( dir == DIR_NOT ){
		dir = MMDL_GetDirDisp( fmmdl );
	}
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_STOP;
	
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_STOP );

	gjiki_Sound_MoveStop();
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Walk(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	if( debug_flag == TRUE ){
		code = AC_WALK_U_2F;
	}else if( key_cont & PAD_BUTTON_B ){
		code = AC_DASH_U_4F;
	}else{
		code = AC_WALK_U_8F;
	}
  
	code = MMDL_ChangeDirAcmdCode( dir, code );
	
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_WALK;

  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_WALK );
	
	gjiki_Sound_Move();
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Turn(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
	
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_TURN;
	
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_TURN );
	gjiki_Sound_MoveStop();
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Hitch(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
	
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_HITCH;
	
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
	gjiki_Sound_MoveStop();
}

//--------------------------------------------------------------
/**
 * 移動セット ジャンプ
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Jump(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
	
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_WALK;
  
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_WALK );
  PMSND_PlaySE( SEQ_SE_DANSA );
	gjiki_Sound_Move();
}

//======================================================================
//  グリッド移動　フィールドプレイヤー制御　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　自転車移動
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
static void jikiMove_Cycle(
		FIELD_PLAYER_GRID *g_jiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag )
{
	PLAYER_SET set;
	MMDL *fmmdl = FIELD_PLAYER_GetMMdl( g_jiki->fld_player );
   
	set = PLAYER_SET_NON;
	switch( g_jiki->move_state ){
	case PLAYER_MOVE_STOP:
		set = gjikiCycle_CheckMoveStart_Stop(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_WALK:
		set = gjikiCycle_CheckMoveStart_Walk(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_TURN:
		set = gjikiCycle_CheckMoveStart_Turn(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_HITCH:
		set = gjikiCycle_CheckMoveStart_Hitch(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	default:
		GF_ASSERT( 0 );
	}
	
	switch( set ){
	case PLAYER_SET_NON:
		gjikiCycle_SetMove_Non(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_STOP:
		gjikiCycle_SetMove_Stop(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_WALK:
		gjikiCycle_SetMove_Walk(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_TURN:
		gjikiCycle_SetMove_Turn(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_HITCH:
		gjikiCycle_SetMove_Hitch(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
		break;
  case PLAYER_SET_JUMP:
		gjikiCycle_SetMove_Jump(
			g_jiki, fmmdl, key_trg, key_cont, dir, debug_flag );
    break;
	}
}

//======================================================================
//	移動開始チェック　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始チェック　停止中
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiCycle_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(fmmdl) == TRUE ){
		if( dir != DIR_NOT ){
			u16 old_dir;
			old_dir = MMDL_GetDirDisp( fmmdl );
			
			if( dir != old_dir && debug_flag == FALSE ){
				return( PLAYER_SET_TURN );
			}
			
			return( gjikiCycle_CheckMoveStart_Walk(
				g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
		}
		
		return( PLAYER_SET_STOP );
	}
	
	return( PLAYER_SET_NON );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　移動中
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiCycle_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT )
  {
		return( gjikiCycle_CheckMoveStart_Stop(
			g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	{
		u32 hit = MMDL_HitCheckMoveDir( fmmdl, dir );
		
		if( debug_flag == TRUE )
    {
			if( hit != MMDL_MOVEHITBIT_NON &&
          !(hit&MMDL_MOVEHITBIT_OUTRANGE) )
      {
				hit = MMDL_MOVEHITBIT_NON;
			}
		}
		
		if( hit == MMDL_MOVEHITBIT_NON )
    {
			return( PLAYER_SET_WALK );
		}
    
    if( (hit & MMDL_MOVEHITBIT_ATTR) )
    {
      BOOL ret;
      u32 attr;
      VecFx32 pos;
      
      MMDL_GetVectorPos( fmmdl, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      ret = MMDL_GetMapPosAttr( fmmdl, &pos, &attr );
      
      if( ret == TRUE )
      {
        u16 attr_dir = DIR_NOT;
        MAPATTR_VAL val = MAPATTR_GetAttrValue( attr );
        MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
        
        if( MAPATTR_CheckAttrValueJumpUp(val) ){
          attr_dir = DIR_UP;
        }else if( MAPATTR_CheckAttrValueJumpDown(val) ){
          attr_dir = DIR_DOWN;
        }else if( MAPATTR_CheckAttrValueJumpLeft(val) ){
          attr_dir = DIR_LEFT;
        }else if( MAPATTR_CheckAttrValueJumpRight(val) ){
          attr_dir = DIR_RIGHT;
        }
        
        if( attr_dir != DIR_NOT && attr_dir == dir ) //ジャンプ方向一致
        {
          return( PLAYER_SET_JUMP );
        }
      }
    }
	}
	
	return( PLAYER_SET_HITCH );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　振り向き中
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiCycle_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjikiCycle_CheckMoveStart_Stop(
			g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjikiCycle_CheckMoveStart_Walk(
		g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　障害物ヒット中
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiCycle_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( fmmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( fmmdl );
	  return( gjikiCycle_CheckMoveStart_Walk(
		  g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
	if( MMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjikiCycle_CheckMoveStart_Stop(
			g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjikiCycle_CheckMoveStart_Walk(
		g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
//	移動セット　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Non(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Stop(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	if( dir == DIR_NOT ){
		dir = MMDL_GetDirDisp( fmmdl );
	}
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_STOP;
	
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_STOP );

	gjiki_Sound_MoveStop();
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Walk(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	if( debug_flag == TRUE ){
		code = AC_WALK_U_2F;
	}else{
		code = AC_WALK_U_4F;
	}
  
	code = MMDL_ChangeDirAcmdCode( dir, code );
	
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_WALK;

  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_WALK );
	
	gjiki_Sound_Move();
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Turn(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
	
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_TURN;
	
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_TURN );
	gjiki_Sound_MoveStop();
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Hitch(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
	
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_HITCH;
	
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
	gjiki_Sound_MoveStop();
}

//--------------------------------------------------------------
/**
 * 移動セット ジャンプ
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Jump(
	FIELD_PLAYER_GRID *g_jiki, MMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
	
	MMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_WALK;
  
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_WALK );
  PMSND_PlaySE( SEQ_SE_DANSA );
	gjiki_Sound_Move();
}

//======================================================================
//  リクエスト
//======================================================================
static void (* const data_gjikiRequestProcTbl[FIELD_PLAYER_GRID_REQBIT_MAX])( FIELD_PLAYER_GRID *gjiki );

//--------------------------------------------------------------
/**
 * 自機リクエスト 
 * @param gjiki FIELD_PLAYER_GRID
 * @param reqbit FIELD_PLAYER_GRID_REQBIT
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_SetRequest(
  FIELD_PLAYER_GRID *gjiki, FIELD_PLAYER_GRID_REQBIT req_bit )
{
  gjiki->req_bit = req_bit;
}

//--------------------------------------------------------------
/**
 * リクエストを更新
 * @param gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_UpdateRequest( FIELD_PLAYER_GRID *gjiki )
{
  int i = 0;
  FIELD_PLAYER_GRID_REQBIT req_bit = gjiki->req_bit;
  
  while( i < FIELD_PLAYER_GRID_REQBIT_MAX ){
    if( (req_bit&0x01) ){
      data_gjikiRequestProcTbl[i]( gjiki );
    }
    req_bit >>= 1;
    i++;
  }
   
  gjiki->req_bit = 0;
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　通常移動に変更
 * @param gjiki FIELD_PLAYER_GIRD
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetNormal( FIELD_PLAYER_GRID *gjiki )
{
  MMDL *fmmdl;
  
  fmmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  
  if( MMDL_GetOBJCode(fmmdl) != HERO ){
    MMDL_ChangeOBJCode( fmmdl, HERO );
  }

  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_NORMAL );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　自転車移動に変更
 * @param gjiki FIELD_PLAYER_GIRD
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetCycle( FIELD_PLAYER_GRID *gjiki )
{
  MMDL *fmmdl;
  
  fmmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  
  if( MMDL_GetOBJCode(fmmdl) != CYCLEHERO ){
    MMDL_ChangeOBJCode( fmmdl, CYCLEHERO );
  }
  
  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_CYCLE );
}

//--------------------------------------------------------------
/// 自機リクエスト処理テーブル
//--------------------------------------------------------------
static void (* const data_gjikiRequestProcTbl[FIELD_PLAYER_GRID_REQBIT_MAX])( FIELD_PLAYER_GRID *gjiki ) =
{
  gjikiReq_SetNormal, //FIELD_PLAYER_GRID_REQBIT_NORMAL
  gjikiReq_SetCycle, //FIELD_PLAYER_GRID_REQBIT_CYCLE
};

//======================================================================
//	サウンド
//======================================================================
//--------------------------------------------------------------
/**
 * サウンドテスト　自機停止
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_Sound_MoveStop( void )
{
	//サウンドテスト（停止）
	u16 trackBit = 0xfcff; // track 9,10 OFF
	PMSND_ChangeBGMtrack(trackBit);
}

//--------------------------------------------------------------
/**
 * サウンドテスト　自機移動
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_Sound_Move( void )
{
	//サウンドテスト（移動）
	u16 trackBit = 0xffff; // 全track ON
	PMSND_ChangeBGMtrack(trackBit);
}

//======================================================================
//  ツール
//======================================================================
//--------------------------------------------------------------
/**
 * 自機を強制停止させる。グリッド専用
 * @param fld_player FIELD_PLAYER
 * @retval nothing
 * @note 自機が強制停止出来ない場合はそのまま
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_ForceStop( FIELD_PLAYER *fld_player )
{
  FIELDMAP_WORK *fieldWork = FIELD_PLAYER_GetFieldMapWork( fld_player );
  FIELDMAP_CTRL_GRID *gridMap = FIELDMAP_GetMapCtrlWork( fieldWork );
  FIELD_PLAYER_GRID *g_jiki = FIELDMAP_CTRL_GRID_GetFieldPlayerGrid( gridMap );

  if( g_jiki->move_state == PLAYER_MOVE_HITCH ){
    MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
    u16 dir = MMDL_GetDirDisp( fmmdl );
    MMDL_FreeAcmd( fmmdl );
    MMDL_SetDirDisp( fmmdl, dir );
    MMDL_SetDrawStatus( fmmdl, DRAW_STA_STOP );
    FIELD_PLAYER_SetMoveValue( fld_player, PLAYER_MOVE_VALUE_STOP );
  }
}
