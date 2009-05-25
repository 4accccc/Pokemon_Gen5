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
//	PLAYER_SET
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
	FIELD_PLAYER *fld_player;
	FIELDMAP_WORK *fieldWork;
	int move_state;
	fx16 scale_size;	//FX16_ONE*8
};

//======================================================================
//	proto
//======================================================================
static PLAYER_SET gjiki_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjiki_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjiki_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET gjiki_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void gjiki_SetMove_Non(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Stop(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Walk(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Turn(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Hitch(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void gjiki_SetMove_Jump(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
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
	PLAYER_SET set;
	FLDMMDL *fmmdl = FIELD_PLAYER_GetFldMMdl( g_jiki->fld_player );
	
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
	if( key_cont & PAD_BUTTON_R ){
		debug_flag = TRUE;
	}
	
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
//	移動開始チェック
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始チェック　停止中
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Stop(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( FLDMMDL_CheckPossibleAcmd(fmmdl) == TRUE ){
		if( dir != DIR_NOT ){
			u16 old_dir;
			old_dir = FLDMMDL_GetDirDisp( fmmdl );
			
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
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Walk(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( FLDMMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT )
  {
		return( gjiki_CheckMoveStart_Stop(
			g_jiki,fmmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	{
		u32 hit = FLDMMDL_HitCheckMoveDir( fmmdl, dir );
		
		if( debug_flag == TRUE )
    {
			if( hit != FLDMMDL_MOVEHITBIT_NON &&
          !(hit&FLDMMDL_MOVEHITBIT_OUTRANGE) )
      {
				hit = FLDMMDL_MOVEHITBIT_NON;
			}
		}
		
		if( hit == FLDMMDL_MOVEHITBIT_NON )
    {
			return( PLAYER_SET_WALK );
		}
    
    if( (hit & FLDMMDL_MOVEHITBIT_ATTR) )
    {
      BOOL ret;
      u32 attr;
      VecFx32 pos;
      
      FLDMMDL_GetVectorPos( fmmdl, &pos );
      FLDMMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      ret = FLDMMDL_GetMapPosAttr( fmmdl, &pos, &attr );
      
      if( ret == TRUE )
      {
        u16 attr_dir = DIR_NOT;
        MAPATTR_VAL val = MAPATTR_GetMapAttrValue( attr );
        MAPATTR_FLAG flag = MAPATTR_GetMapAttrFlag( attr );
        
        switch( val ) //ジャンプアトリビュートチェック
        {
        case 0x72:
          attr_dir = DIR_RIGHT;
          break;
        case 0x73:
          attr_dir = DIR_LEFT;
          break;
        case 0x74:
          attr_dir = DIR_UP;
          break;
        case 0x75:
          attr_dir = DIR_DOWN;
          break;
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
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Turn(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( FLDMMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
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
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET gjiki_CheckMoveStart_Hitch(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( FLDMMDL_CheckPossibleAcmd(fmmdl) == FALSE ){
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
//	移動セット
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Non(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Stop(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	if( dir == DIR_NOT ){
		dir = FLDMMDL_GetDirDisp( fmmdl );
	}
	
	code = FLDMMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
	FLDMMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_STOP;
	
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_STOP );

	gjiki_Sound_MoveStop();
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Walk(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
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
	
	if( key_cont & PAD_BUTTON_A ){ //kari
		code = AC_JUMP_U_2G_16F;
	}
	
	code = FLDMMDL_ChangeDirAcmdCode( dir, code );
	
	FLDMMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_WALK;

  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_WALK );
	
	gjiki_Sound_Move();
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Turn(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = FLDMMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
	
	FLDMMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_TURN;
	
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_TURN );
	gjiki_Sound_MoveStop();
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Hitch(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = FLDMMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_32F );
	
	FLDMMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_HITCH;
	
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_STOP );
	gjiki_Sound_MoveStop();
}

//--------------------------------------------------------------
/**
 * 移動セット ジャンプ
 * @param	g_jiki FIELD_PLAYER_GRID
 * @param fmmdl FLDMMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetMove_Jump(
	FIELD_PLAYER_GRID *g_jiki, FLDMMDL *fmmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	code = FLDMMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
	
	FLDMMDL_SetAcmd( fmmdl, code );
	g_jiki->move_state = PLAYER_MOVE_WALK;
  
  FIELD_PLAYER_SetMoveValue( g_jiki->fld_player, PLAYER_MOVE_VALUE_WALK );
	gjiki_Sound_Move();
}


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
	//　↓ サウンドテスト（停止）
	u16 trackBit = 0xfcff; // track 9,10 OFF
	PMSND_ChangeBGMtrack(trackBit);
	//　↑
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
	//　↓ サウンドテスト（移動）
	u16 trackBit = 0xffff; // 全track ON
	PMSND_ChangeBGMtrack(trackBit);
	//　↑
}
