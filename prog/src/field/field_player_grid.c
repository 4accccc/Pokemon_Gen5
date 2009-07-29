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

#include "fldeff_namipoke.h"

#include "field_sound.h"

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
  
  FLDEFF_TASK *fldeff_joint;
};

//======================================================================
//	proto
//======================================================================
//通常移動
static void jikiMove_Normal(
		FIELD_PLAYER_GRID *gjiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag );

static PLAYER_SET gjiki_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjiki_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjiki_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjiki_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjiki_SetMove_Non(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Jump(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

//自転車移動
static void jikiMove_Cycle(
		FIELD_PLAYER_GRID *gjiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag );

static PLAYER_SET gjikiCycle_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjikiCycle_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjikiCycle_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjikiCycle_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjikiCycle_SetMove_Non(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiCycle_SetMove_Jump(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

//波乗り移動
static void jikiMove_Swim(
		FIELD_PLAYER_GRID *gjiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag );

static PLAYER_SET gjikiSwim_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjikiSwim_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjikiSwim_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjikiSwim_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjikiSwim_SetMove_Non(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiSwim_SetMove_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiSwim_SetMove_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiSwim_SetMove_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjikiSwim_SetMove_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

//サウンド
#if 0
static void gjiki_Sound_MoveStop( void );
static void gjiki_Sound_Move( void );
#endif

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
	FIELD_PLAYER_GRID *gjiki;
	
	gjiki = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_PLAYER_GRID) );
	gjiki->fld_player = fld_player;
	gjiki->fieldWork = FIELD_PLAYER_GetFieldMapWork( fld_player );
	gjiki->scale_size = FX16_ONE*8-1;
  
  //復帰
  {
    PLAYER_MOVE_FORM form;
    form = FIELD_PLAYER_GetMoveForm( gjiki->fld_player );
    
    switch( form ){
    case PLAYER_MOVE_FORM_SWIM:
      FIELD_PLAYER_GRID_SetRequest(
          gjiki, FIELD_PLAYER_GRID_REQBIT_SWIM );
      FIELD_PLAYER_GRID_UpdateRequest( gjiki );
      break;
    }
  }
  
//SetGridPlayerActTrans( gjiki->pActCont, &gjiki->vec_pos );
	return( gjiki );
}

//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　削除
 * @param	gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_Delete( FIELD_PLAYER_GRID *gjiki )
{
	GFL_HEAP_FreeMemory( gjiki );
}

//======================================================================
//	グリッド移動 フィールドプレイヤー制御　移動
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動 フィールドプレイヤー制御　移動
 * @param	gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_Move(
		FIELD_PLAYER_GRID *gjiki, int key_trg, int key_cont )
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
    VecFx32 pos;
    FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( gjiki->fieldWork );
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
    u16 dir = MMDL_GetDirDisp( mmdl );
    MMDL_GetVectorPos( mmdl, &pos );
    MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
    
    gjiki->fldeff_joint = FLDEFF_NAMIPOKE_SetMMdl(
        fectrl, dir, &pos, mmdl, FALSE );
  }
#endif
  
  FIELD_PLAYER_GRID_UpdateRequest( gjiki );
  
  form = FIELD_PLAYER_GetMoveForm( gjiki->fld_player );
  switch( form ){
  case PLAYER_MOVE_FORM_NORMAL:
    jikiMove_Normal( gjiki, key_trg, key_cont, dir, debug_flag );
	  break;
  case PLAYER_MOVE_FORM_CYCLE:
    jikiMove_Cycle( gjiki, key_trg, key_cont, dir, debug_flag );
	  break;
  case PLAYER_MOVE_FORM_SWIM:
    jikiMove_Swim( gjiki, key_trg, key_cont, dir, debug_flag );
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
 * @param	gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
static void jikiMove_Normal(
		FIELD_PLAYER_GRID *gjiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag )
{
	PLAYER_SET set;
	MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
   
	set = PLAYER_SET_NON;
	switch( gjiki->move_state ){
	case PLAYER_MOVE_STOP:
		set = gjiki_CheckMoveStart_Stop(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_WALK:
		set = gjiki_CheckMoveStart_Walk(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_TURN:
		set = gjiki_CheckMoveStart_Turn(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_HITCH:
		set = gjiki_CheckMoveStart_Hitch(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	default:
		GF_ASSERT( 0 );
	}
	
	switch( set ){
	case PLAYER_SET_NON:
		gjiki_SetMove_Non(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_STOP:
		gjiki_SetMove_Stop(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_WALK:
		gjiki_SetMove_Walk(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_TURN:
		gjiki_SetMove_Turn(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_HITCH:
		gjiki_SetMove_Hitch(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
  case PLAYER_SET_JUMP:
		gjiki_SetMove_Jump(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
	}
}

//======================================================================
//	移動開始チェック　通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始チェック　停止中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
		if( dir != DIR_NOT ){
			u16 old_dir;
			old_dir = MMDL_GetDirDisp( mmdl );
			
			if( dir != old_dir && debug_flag == FALSE ){
				return( PLAYER_SET_TURN );
			}
			
			return( gjiki_CheckMoveStart_Walk(
				gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
		}
		
		return( PLAYER_SET_STOP );
	}
	
	return( PLAYER_SET_NON );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　移動中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT )
  {
		return( gjiki_CheckMoveStart_Stop(
			gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	{
		u32 hit = MMDL_HitCheckMoveDir( mmdl, dir );
		
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
      
      MMDL_GetVectorPos( mmdl, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      ret = MMDL_GetMapPosAttr( mmdl, &pos, &attr );
      
      if( ret == TRUE )
      {
        u16 attr_dir = DIR_NOT;
        MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
        MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
        
        if( MAPATTR_VALUE_CheckJumpUp(val) ){
          attr_dir = DIR_UP;
        }else if( MAPATTR_VALUE_CheckJumpDown(val) ){
          attr_dir = DIR_DOWN;
        }else if( MAPATTR_VALUE_CheckumpLeft(val) ){
          attr_dir = DIR_LEFT;
        }else if( MAPATTR_VALUE_CheckJumpRight(val) ){
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
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjiki_CheckMoveStart_Stop(
			gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjiki_CheckMoveStart_Walk(
		gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　障害物ヒット中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( mmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( mmdl );
	  return( gjiki_CheckMoveStart_Walk(
		  gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjiki_CheckMoveStart_Stop(
			gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjiki_CheckMoveStart_Walk(
		gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
//	移動セット　通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Non(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	if( dir == DIR_NOT ){
		dir = MMDL_GetDirDisp( mmdl );
	}
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_STOP;
	
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  FIELD_SOUND_ChangeBGMTrackStill();
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
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
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_WALK;

  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
  FIELD_SOUND_ChangeBGMTrackAction();
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_TURN;
	
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_TURN );
  FIELD_SOUND_ChangeBGMTrackStill();
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_HITCH;
	
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
  FIELD_SOUND_ChangeBGMTrackStill();
}

//--------------------------------------------------------------
/**
 * 移動セット ジャンプ
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Jump(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_WALK;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
  PMSND_PlaySE( SEQ_SE_DANSA );
  FIELD_SOUND_ChangeBGMTrackAction();
}

//======================================================================
//	グリッド移動 フィールドプレイヤー制御　波乗り移動
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　波乗り移動
 * @param	gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
static void jikiMove_Swim(
		FIELD_PLAYER_GRID *gjiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag )
{
	PLAYER_SET set;
	MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
   
	set = PLAYER_SET_NON;
	switch( gjiki->move_state ){
	case PLAYER_MOVE_STOP:
		set = gjikiSwim_CheckMoveStart_Stop(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_WALK:
		set = gjikiSwim_CheckMoveStart_Walk(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_TURN:
		set = gjikiSwim_CheckMoveStart_Turn(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_HITCH:
		set = gjikiSwim_CheckMoveStart_Hitch(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	default:
		GF_ASSERT( 0 );
	}
	
	switch( set ){
	case PLAYER_SET_NON:
		gjikiSwim_SetMove_Non(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_STOP:
		gjikiSwim_SetMove_Stop(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_WALK:
		gjikiSwim_SetMove_Walk(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_TURN:
		gjikiSwim_SetMove_Turn(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_HITCH:
		gjikiSwim_SetMove_Hitch(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
  default:
    GF_ASSERT( 0 );
    break;
	}
}

//======================================================================
//	移動開始チェック　波乗り移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始チェック　停止中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiSwim_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
		if( dir != DIR_NOT ){
			u16 old_dir;
			old_dir = MMDL_GetDirDisp( mmdl );
			
			if( dir != old_dir && debug_flag == FALSE ){
				return( PLAYER_SET_TURN );
			}
			
			return( gjikiSwim_CheckMoveStart_Walk(
				gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
		}
		
		return( PLAYER_SET_STOP );
	}
	
	return( PLAYER_SET_NON );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　移動中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiSwim_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT )
  {
		return( gjikiSwim_CheckMoveStart_Stop(
			gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	{
		u32 hit = MMDL_HitCheckMoveDir( mmdl, dir );
		
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
      
      MMDL_GetVectorPos( mmdl, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      ret = MMDL_GetMapPosAttr( mmdl, &pos, &attr );
      
      if( ret == TRUE )
      {
        u16 attr_dir = DIR_NOT;
        MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
        MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
        
        if( (flag & MAPATTR_FLAGBIT_WATER) ){
          return( PLAYER_SET_WALK );
        }
      }
    }
	}
	
	return( PLAYER_SET_HITCH );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　振り向き中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiSwim_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjikiSwim_CheckMoveStart_Stop(
			gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjikiSwim_CheckMoveStart_Walk(
		gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　障害物ヒット中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiSwim_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( mmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( mmdl );
	  return( gjikiSwim_CheckMoveStart_Walk(
		  gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjikiSwim_CheckMoveStart_Stop(
			gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjikiSwim_CheckMoveStart_Walk(
		gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
//	移動セット　波乗り移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Non(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	if( dir == DIR_NOT ){
		dir = MMDL_GetDirDisp( mmdl );
	}
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_STOP;
	
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  FIELD_SOUND_ChangeBGMTrackStill();
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
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
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_WALK;

  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
  FIELD_SOUND_ChangeBGMTrackAction();
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_TURN;
	
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_TURN );
  FIELD_SOUND_ChangeBGMTrackStill();
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiSwim_SetMove_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_HITCH;
	
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
  FIELD_SOUND_ChangeBGMTrackStill();
}

//======================================================================
//  グリッド移動　フィールドプレイヤー制御　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　自転車移動
 * @param	gjiki FIELD_PLAYER_GRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
static void jikiMove_Cycle(
		FIELD_PLAYER_GRID *gjiki, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag )
{
	PLAYER_SET set;
	MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
   
	set = PLAYER_SET_NON;
	switch( gjiki->move_state ){
	case PLAYER_MOVE_STOP:
		set = gjikiCycle_CheckMoveStart_Stop(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_WALK:
		set = gjikiCycle_CheckMoveStart_Walk(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_TURN:
		set = gjikiCycle_CheckMoveStart_Turn(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_HITCH:
		set = gjikiCycle_CheckMoveStart_Hitch(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	default:
		GF_ASSERT( 0 );
	}
	
	switch( set ){
	case PLAYER_SET_NON:
		gjikiCycle_SetMove_Non(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_STOP:
		gjikiCycle_SetMove_Stop(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_WALK:
		gjikiCycle_SetMove_Walk(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_TURN:
		gjikiCycle_SetMove_Turn(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_HITCH:
		gjikiCycle_SetMove_Hitch(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
  case PLAYER_SET_JUMP:
		gjikiCycle_SetMove_Jump(
			gjiki, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
	}
}

//======================================================================
//	移動開始チェック　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始チェック　停止中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiCycle_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
		if( dir != DIR_NOT ){
			u16 old_dir;
			old_dir = MMDL_GetDirDisp( mmdl );
			
			if( dir != old_dir && debug_flag == FALSE ){
				return( PLAYER_SET_TURN );
			}
			
			return( gjikiCycle_CheckMoveStart_Walk(
				gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
		}
		
		return( PLAYER_SET_STOP );
	}
	
	return( PLAYER_SET_NON );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　移動中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiCycle_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT )
  {
		return( gjikiCycle_CheckMoveStart_Stop(
			gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	{
		u32 hit = MMDL_HitCheckMoveDir( mmdl, dir );
		
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
      
      MMDL_GetVectorPos( mmdl, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      ret = MMDL_GetMapPosAttr( mmdl, &pos, &attr );
      
      if( ret == TRUE )
      {
        u16 attr_dir = DIR_NOT;
        MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
        MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
        
        if( MAPATTR_VALUE_CheckJumpUp(val) ){
          attr_dir = DIR_UP;
        }else if( MAPATTR_VALUE_CheckJumpDown(val) ){
          attr_dir = DIR_DOWN;
        }else if( MAPATTR_VALUE_CheckumpLeft(val) ){
          attr_dir = DIR_LEFT;
        }else if( MAPATTR_VALUE_CheckJumpRight(val) ){
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
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiCycle_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjikiCycle_CheckMoveStart_Stop(
			gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjikiCycle_CheckMoveStart_Walk(
		gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　障害物ヒット中
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjikiCycle_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( mmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( mmdl );
	  return( gjikiCycle_CheckMoveStart_Walk(
		  gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( gjikiCycle_CheckMoveStart_Stop(
			gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( gjikiCycle_CheckMoveStart_Walk(
		gjiki,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
//	移動セット　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Non(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Stop(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	if( dir == DIR_NOT ){
		dir = MMDL_GetDirDisp( mmdl );
	}
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_STOP;
	
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  FIELD_SOUND_ChangeBGMTrackStill();
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Walk(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
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
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_WALK;

  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
  FIELD_SOUND_ChangeBGMTrackAction();
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Turn(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_TURN;
	
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_TURN );
  FIELD_SOUND_ChangeBGMTrackStill();
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Hitch(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_HITCH;
	
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
  FIELD_SOUND_ChangeBGMTrackStill();
}

//--------------------------------------------------------------
/**
 * 移動セット ジャンプ
 * @param	gjiki FIELD_PLAYER_GRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiCycle_SetMove_Jump(
	FIELD_PLAYER_GRID *gjiki, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
	
	MMDL_SetAcmd( mmdl, code );
	gjiki->move_state = PLAYER_MOVE_WALK;
  
  FIELD_PLAYER_SetMoveValue( gjiki->fld_player, PLAYER_MOVE_VALUE_WALK );
  PMSND_PlaySE( SEQ_SE_DANSA );
  FIELD_SOUND_ChangeBGMTrackAction();
}

//======================================================================
//  リクエスト
//======================================================================
static void (* const data_gjikiRequestProcTbl[FIELD_PLAYER_GRID_REQBIT_MAX])( FIELD_PLAYER_GRID *gjiki );

static void gjiki_PlayBGM( FIELD_PLAYER_GRID *gjiki )
{
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( gjiki->fieldWork );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( gjiki->fld_player );
  u32 zone_id = FIELDMAP_GetZoneID( gjiki->fieldWork );
  u32 no = FIELD_SOUND_GetFieldBGMNo( gdata, form, zone_id );
  FIELD_SOUND_PlayNextBGM( no );
}

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
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_NORMAL );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }

  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_NORMAL );
  gjiki_PlayBGM( gjiki );
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
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_CYCLE );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
   
  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_CYCLE );
  gjiki_PlayBGM( gjiki );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　波乗り移動に変更
 * @param gjiki FIELD_PLAYER_GRID
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetSwim( FIELD_PLAYER_GRID *gjiki )
{
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_SWIM );

  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  
  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_SWIM );
  gjiki_PlayBGM( gjiki );
  
  if( gjiki->fldeff_joint == NULL ){ //波乗りポケモン
    u16 dir;
    VecFx32 pos;
    FLDEFF_CTRL *fectrl;
    
    fectrl = FIELDMAP_GetFldEffCtrl( gjiki->fieldWork );
    dir = MMDL_GetDirDisp( mmdl );
    MMDL_GetVectorPos( mmdl, &pos );
    
    gjiki->fldeff_joint = FLDEFF_NAMIPOKE_SetMMdl(
        fectrl, dir, &pos, mmdl, TRUE );
  }
}

//--------------------------------------------------------------
/// 自機リクエスト処理テーブル
//--------------------------------------------------------------
static void (* const data_gjikiRequestProcTbl[FIELD_PLAYER_GRID_REQBIT_MAX])( FIELD_PLAYER_GRID *gjiki ) =
{
  gjikiReq_SetNormal, //FIELD_PLAYER_GRID_REQBIT_NORMAL
  gjikiReq_SetCycle, //FIELD_PLAYER_GRID_REQBIT_CYCLE
  gjikiReq_SetSwim, //FIELD_PLAYER_GRID_REQBIT_SWIM
};

//======================================================================
//	サウンド
//======================================================================
#if 0
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
#endif

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
  FIELD_PLAYER_GRID *gjiki = FIELDMAP_CTRL_GRID_GetFieldPlayerGrid( gridMap );

  if( gjiki->move_state == PLAYER_MOVE_HITCH ){
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
    u16 dir = MMDL_GetDirDisp( mmdl );
    MMDL_FreeAcmd( mmdl );
    MMDL_SetDirDisp( mmdl, dir );
    MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
    FIELD_PLAYER_SetMoveValue( fld_player, PLAYER_MOVE_VALUE_STOP );
  }
}

//--------------------------------------------------------------
/**
 * 自機に波乗りポケモンのタスクポインタをセット
 * @param
 * @retval
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_SetEffectTaskWork(
    FIELD_PLAYER_GRID *gjiki, FLDEFF_TASK *task )
{
  gjiki->fldeff_joint = task;
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
FLDEFF_TASK * FIELD_PLAYER_GRID_GetEffectTaskWork(
    FIELD_PLAYER_GRID *gjiki )
{
  return( gjiki->fldeff_joint );
}


//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 *
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetNaminori( FIELD_PLAYER_GRID *gjiki )
{
  int sex;
  u16 code;
  MMDL *mmdl;
   
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_SWIM );

  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  
  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_SWIM );
  gjiki_PlayBGM( gjiki );
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 *
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetNaminoriEnd( FIELD_PLAYER_GRID *gjiki )
{
  VecFx32 offs = {0,0,0};
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_GetSex( gjiki->fld_player );
  mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_NORMAL );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  
  FIELD_PLAYER_SetMoveForm( gjiki->fld_player, PLAYER_MOVE_FORM_NORMAL );
  gjiki_PlayBGM( gjiki );
  
  if( gjiki->fldeff_joint != NULL ){
    FLDEFF_TASK_CallDelete( gjiki->fldeff_joint );
    gjiki->fldeff_joint = NULL;
  }
  
  MMDL_SetVectorOuterDrawOffsetPos( mmdl, &offs );
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 *
 */
//--------------------------------------------------------------
u16 FIELD_PLAYER_GRID_GetKeyDir( FIELD_PLAYER_GRID *gjiki, int key )
{
	u16 dir = DIR_NOT;
	if( (key&PAD_KEY_UP) ){
		dir = DIR_UP;
	}else if( (key&PAD_KEY_DOWN) ){
		dir = DIR_DOWN;
	}else if( (key&PAD_KEY_LEFT) ){
		dir = DIR_LEFT;
	}else if( (key&PAD_KEY_RIGHT) ){
		dir = DIR_RIGHT;
	}
  return( dir );
}
