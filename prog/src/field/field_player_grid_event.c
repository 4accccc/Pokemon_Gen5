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
static GMEVENT * gjiki_CheckMoveHitEvent(
    FIELD_PLAYER_GRID *gjiki, u16 dir, PLAYER_EVENTBIT evbit );
static GMEVENT * gjiki_CheckEventKairiki(
    FIELD_PLAYER_GRID *gjiki, u16 dir, PLAYER_MOVE_VALUE val );

static GMEVENT * gjiki_SetEventKairiki(
    FIELD_PLAYER_GRID *gjiki, u16 dir, MMDL *mmdl, BOOL ana_flag );
static GMEVENT_RESULT event_Kairki( GMEVENT *event, int *seq, void *wk );

static MMDL * gjiki_SearchDirMMdl( FIELD_PLAYER_GRID *gjiki, u16 dir );

//======================================================================
//  グリッド自機　イベント移動関連
//======================================================================
//--------------------------------------------------------------
/**
 * 自機イベント移動チェック
 * @param *gjiki FIELD_PLAYER_GRID 
 * @param key_trg 入力キートリガ情報
 * @param key_cont 入力キーコンテニュー情報
 * @param evbit PLAYER_EVENTBIT
 * @retval BOOL TRUE=自機イベント移動発生
 */
//--------------------------------------------------------------
GMEVENT * FIELD_PLAYER_GRID_CheckMoveEvent( FIELD_PLAYER *fld_player,
    int key_trg, int key_cont, PLAYER_EVENTBIT evbit )
{
  u16 dir;
  MMDL *mmdl;
  GMEVENT *event;
  FIELDMAP_WORK *fieldmap;
  FIELDMAP_CTRL_GRID *gridmap;
  FIELD_PLAYER_GRID *gjiki;
  
  event = NULL;
  fieldmap = FIELD_PLAYER_GetFieldMapWork( fld_player );
  gridmap = FIELDMAP_GetMapCtrlWork( fieldmap );
  gjiki = FIELDMAP_CTRL_GRID_GetFieldPlayerGrid( gridmap );
  mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  dir = FIELD_PLAYER_GRID_GetKeyDir( gjiki, key_cont );
  
#if 0 //強制移動イベント
 	if( Player_MoveBitCheck_Force(jiki) == TRUE ){
		if( JikiEventCheck_ForceMoveHitEvent(fsys,jiki,flag) == TRUE ){
			return( TRUE );
		}
#endif
  
  //移動開始可能チェック
  if( FIELD_PLAYER_GRID_CheckStartMove(gjiki,dir) == FALSE ){
    return( NULL );
  }
  
#if 0 //キー入力無しでも発生するイベントチェック
	if( dir == DIR_NOT ){
		if( JikiEventCheck_KeyOFFEvent(fsys,jiki,dir,flag) == TRUE ){
			return( TRUE );
		}
		
		return( FALSE );
	}
#endif
 
  //イベントヒットチェック
  event = gjiki_CheckMoveHitEvent( gjiki, dir, evbit );
  
  if( event != NULL ){
    return( event );
  }
  
  return( NULL );
}

//======================================================================
//  移動ヒット系イベント発動チェック
//======================================================================
//--------------------------------------------------------------
/**
 * 移動ヒット系イベント発動チェック
 * @param gjiki FIELD_PLAYER_GRID
 * @param dir  移動方向。DIR_UP等
 * @param evbit PLAYER_EVENTBIT
 * @retval GMEVENT* NULL=イベント発動無し
 */
//--------------------------------------------------------------
static GMEVENT * gjiki_CheckMoveHitEvent(
    FIELD_PLAYER_GRID *gjiki, u16 dir, PLAYER_EVENTBIT evbit )
{
  GMEVENT *event = NULL;
  PLAYER_MOVE_VALUE val;
  
  val = FIELD_PLAYER_GRID_GetMoveValue( gjiki, dir );
  
#ifdef DEBUG_ONLY_FOR_kagaya
  evbit |= PLAYER_EVENTBIT_KAIRIKI;
#endif

  if( (evbit & PLAYER_EVENTBIT_KAIRIKI) ){
    event = gjiki_CheckEventKairiki( gjiki, dir, val );
  }
  
  return( event );
}

//--------------------------------------------------------------
/**
 * 怪力移動チェック
 * @param gjiki FIELD_PLAYER_GRID
 * @param dir 移動方向。DIR_UP等
 * @param val PLAYER_MOVE_VALUE
 * @retval GMEVENT* NULL=移動無し
 */
//--------------------------------------------------------------
static GMEVENT * gjiki_CheckEventKairiki(
    FIELD_PLAYER_GRID *gjiki, u16 dir, PLAYER_MOVE_VALUE val )
{
  if( val == PLAYER_MOVE_VALUE_WALK )
  {
    MMDL *mmdl = gjiki_SearchDirMMdl( gjiki, dir );
    
    if( mmdl != NULL )
    {
      u16 code = MMDL_GetOBJCode( mmdl );
      
      if( code == ROCK )
      {
        u32 ret;
        BOOL flag;
        KAGAYA_Printf( "怪力岩ヒット\n" );
        
        ret = MMDL_HitCheckMoveDir( mmdl, dir );
        ret &= ~MMDL_MOVEHITBIT_LIM; //移動制限は無視
        
        flag = FALSE;
        
        if( (ret & MMDL_MOVEHITBIT_ATTR) )
        {
          VecFx32 pos;
          MAPATTR attr;
          MAPATTR_VALUE attr_val;
          FIELDMAP_WORK *fieldmap;
          FIELD_PLAYER *fld_player;
          FLDMAPPER *mapper;
          
          fld_player = FIELD_PLAYER_GRID_GetFieldPlayer( gjiki );
          fieldmap = FIELD_PLAYER_GetFieldMapWork( fld_player );
          mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );
          
          MMDL_GetVectorPos( mmdl, &pos );
          MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
          
          attr = MAPATTR_GetAttribute( mapper, &pos );
          attr_val = MAPATTR_GetAttrValue( attr );
          
          if( MAPATTR_VALUE_CheckKairikiAna(attr_val) == TRUE )
          {
            ret = MMDL_MOVEHITBIT_NON;
            flag = TRUE;
          }
        }
        
        if( ret == MMDL_MOVEHITBIT_NON )
        {
          KAGAYA_Printf( "怪力開始\n" );
          return( gjiki_SetEventKairiki(gjiki,dir,mmdl,flag) );
        }
      }
    }
  }
  
  return( NULL );
}

//======================================================================
//  怪力イベント
//======================================================================
//--------------------------------------------------------------
/// EVENT_KAIRIKI_WORK
//--------------------------------------------------------------
typedef struct
{
  FIELD_PLAYER_GRID *gjiki;
  u16 dir;
  u16 ana_flag;
  s16 frame;
  VecFx32 offs;
  MMDL *mmdl;
  MMDL *mmdl_jiki;
}EVENT_KAIRIKI_WORK;

//--------------------------------------------------------------
/**
 * 怪力イベント発動
 * @param gjiki FIELD_PLAYER_GRID
 * @param dir 移動方向
 * @param mmdl 移動対象の動作モデル
 * @param ana_flag 移動先が怪力穴である。
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
static GMEVENT * gjiki_SetEventKairiki(
    FIELD_PLAYER_GRID *gjiki, u16 dir, MMDL *mmdl, BOOL ana_flag )
{
  GMEVENT *event;
  GAMESYS_WORK *gsys;
  FIELD_PLAYER *fld_player;
  EVENT_KAIRIKI_WORK *work;
  
  fld_player = FIELD_PLAYER_GRID_GetFieldPlayer( gjiki );
  gsys = FIELD_PLAYER_GetGameSysWork( fld_player );
  
  event = GMEVENT_Create( gsys, NULL,
      event_Kairki, sizeof(EVENT_KAIRIKI_WORK) );
  
  work = GMEVENT_GetEventWork( event );
  MI_CpuClear8( work, sizeof(EVENT_KAIRIKI_WORK) );
  
  work->gjiki = gjiki;
  work->dir = dir;
  work->ana_flag = ana_flag;
  work->mmdl = mmdl;
  work->mmdl_jiki = FIELD_PLAYER_GetMMdl( fld_player );
  
  return( event );
}

//--------------------------------------------------------------
/**
 * 怪力イベント
 * @param event GMEVNET
 * @param seq event seq
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_Kairki( GMEVENT *event, int *seq, void *wk )
{
  EVENT_KAIRIKI_WORK *work = wk;
  
  switch( (*seq) ){
  case 0:
    if( MMDL_CheckPossibleAcmd(work->mmdl) == FALSE ){
      break;
    }
    
    if( MMDL_CheckPossibleAcmd(work->mmdl_jiki) == FALSE ){
      if( FIELD_PLAYER_GRID_SetMoveStop(work->gjiki) == FALSE ){
        break;
      }
    }
    
    if( work->ana_flag == TRUE ){
      MMDL_SetStatusBitHeightGetOFF( work->mmdl, TRUE );
    }

    {
      u16 code = MMDL_ChangeDirAcmdCode( work->dir, AC_WALK_U_16F );
      MMDL_SetAcmd( work->mmdl, code );

      code = MMDL_ChangeDirAcmdCode( work->dir, AC_STAY_WALK_U_8F );
      MMDL_SetAcmd( work->mmdl_jiki, code );
      
      PMSND_PlaySE( SEQ_SE_FLD_03 );
    }
    
    (*seq)++;
    break;
  case 1:
    if( MMDL_CheckEndAcmd(work->mmdl) == FALSE ){
      break;
    }
    
    if( MMDL_CheckEndAcmd(work->mmdl_jiki) == FALSE ){
      break;
    }
    
    (*seq)++;
    
    if( work->ana_flag == FALSE ){
      (*seq) = 9;
    }
  
    PMSND_PlaySE( SEQ_SE_FLD_04 );
    break;
  case 2: //怪力穴はまり
    {
      fx32 y;
      VecFx32 pos;
      MMDL_GetVectorPos( work->mmdl, &pos );
      MMDL_GetMapPosHeight( work->mmdl, &pos, &y );
      
      if( y < pos.y ){
        pos.y -= 0x4000;
        if( y > pos.y ){
          pos.y = y;
        }
      }else if( y > pos.y ){
        pos.y += 0x4000;
        if( y < pos.y ){
          pos.y = y;
        }
      }
      
	    MMDL_SetVectorPos( work->mmdl, &pos );

      if( y == pos.y ){
        work->offs.y = NUM_FX32( 2 );
        (*seq)++;
      }
    }
    break;
  case 3:
    MMDL_SetVectorOuterDrawOffsetPos( work->mmdl, &work->offs );
    work->offs.y = -work->offs.y;
    work->frame++;
    if( (work->frame&0x04) ){
      if( work->offs.y < 0 ){
        work->offs.y += NUM_FX32( 1 );
      }else if( work->offs.y > 0 ){
        work->offs.y -= NUM_FX32( 1 );
      }
    }
    if( work->offs.y == 0 ){
      VecFx32 pos;
      MMDL_GetVectorPos( work->mmdl, &pos );
      MMDL_InitPosition( work->mmdl, &pos, work->dir );
      MMDL_SetVectorOuterDrawOffsetPos( work->mmdl, &work->offs );
      MMDL_SetStatusBitHeightGetOFF( work->mmdl, FALSE );
      (*seq) = 9;
    }
    break;
  case 9:
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * 自機位置から指定方向に動作モデルが在るか
 * @param gjiki FIELD_PLAYER_GRID
 * @param dir 参照方向。DIR_UP等
 * @retval MMDL* NULL=居ない
 */
//--------------------------------------------------------------
static MMDL * gjiki_SearchDirMMdl( FIELD_PLAYER_GRID *gjiki, u16 dir )
{
  MMDL *hmmdl,*mmdl = FIELD_PLAYER_GRID_GetMMdl( gjiki );
  const MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
  int gx = MMDL_GetGridPosX( mmdl ) + MMDL_TOOL_GetDirAddValueGridX( dir );
  int gz = MMDL_GetGridPosZ( mmdl ) + MMDL_TOOL_GetDirAddValueGridZ( dir );
  fx32 y = MMDL_GetVectorPosY( mmdl );
  
  hmmdl = MMDLSYS_SearchGridPosEx( mmdlsys, gx, gz, y, 1, FALSE );
  return( hmmdl );
}
