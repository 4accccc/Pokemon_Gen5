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

#include "fieldmap_ctrl.h"
#include "fieldmap_ctrl_grid.h"
#include "field_player_grid.h"
#include "map_attr.h"

#include "fldeff_namipoke.h"

#include "field_sound.h"
#include "fieldmap_tcb.h"

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
  FIELD_PLAYER_GRID *gjiki;
  
  event = NULL;
  fieldmap = FIELD_PLAYER_GetFieldMapWork( fld_player );
  gjiki = FIELDMAP_GetPlayerGrid( fieldmap );
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
 * 怪力穴チェック
 * @param
 * @retval
 */
//--------------------------------------------------------------
enum
{
  ANA_NON = 0,
  ANA_NOT,
  ANA_HIT,
  ANA_ALL,
};

static int kairiki_CheckAna( MMDL *mmdl, u16 dir, FLDMAPPER *mapper )
{
  int non_hit,ana_hit;
  MAPATTR attr;
  MAPATTR_VALUE attr_val;
  u8 x0,x1,z0,z1;
  VecFx32 pos,c_pos;
  
  MMDL_GetVectorPos( mmdl, &pos );
  MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
  
  x1 = MMDL_GetGridSizeX( mmdl );
  z1 = MMDL_GetGridSizeZ( mmdl );
  
  non_hit = FALSE;
  ana_hit = FALSE;
  
  for( z0 = 0; z0 < z1; z0++, pos.z -= GRID_FX32 )
  {
    for( x0 = 0, c_pos = pos; x0 < x1; x0++, c_pos.x += GRID_FX32 )
    {
      attr = MAPATTR_GetAttribute( mapper, &c_pos );
      attr_val = MAPATTR_GetAttrValue( attr );
      
      if( MAPATTR_GetHitchFlag(attr) )
      {
        if( MAPATTR_VALUE_CheckKairikiAna(attr_val) == FALSE )
        {
          return( ANA_NOT ); //怪力穴以外で障害
        }
        else
        {
          ana_hit = TRUE;
        }
      }
      else
      {
        non_hit = TRUE;
      }
    }
  }
  
  if( ana_hit == TRUE )
  {
    if( non_hit == FALSE )
    {
      return( ANA_ALL ); //全部穴
    }
    
    return( ANA_HIT );
  }
  
  return( ANA_NON );
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
      
      if( MMDL_CheckOBJCodeKairiki(code) == TRUE )
      {
        u32 ret;
        int ana;
        FIELDMAP_WORK *fieldmap;
        FIELD_PLAYER *fld_player;
        FLDMAPPER *mapper;
          
        KAGAYA_Printf( "怪力岩ヒット\n" );
        
        fld_player = FIELD_PLAYER_GRID_GetFieldPlayer( gjiki );
        fieldmap = FIELD_PLAYER_GetFieldMapWork( fld_player );
        mapper = FIELDMAP_GetFieldG3Dmapper( fieldmap );
        
        ana = kairiki_CheckAna( mmdl, dir, mapper );
        ret = MMDL_HitCheckMoveDir( mmdl, dir );
        ret &= ~MMDL_MOVEHITBIT_LIM; //移動制限は無視
        
        if( (ret & MMDL_MOVEHITBIT_ATTR) )
        {
          if( ana == ANA_HIT || ana == ANA_ALL ) //穴ヒットである
          {
            ret &= ~MMDL_MOVEHITBIT_ATTR;
          }
        }
        
        if( ret == MMDL_MOVEHITBIT_NON )
        {
          BOOL flag = FALSE;
          KAGAYA_Printf( "怪力開始\n" );
          
          if( ana == ANA_ALL )
          {
            flag = TRUE;
          }
          
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
#define KAIRIKI_FALLDOWN_HEIGHT (NUM_FX32(-14))

//--------------------------------------------------------------
/// EVENT_KAIRIKI_WORK
//--------------------------------------------------------------
typedef struct
{
  FIELD_PLAYER_GRID *gjiki;
  u16 dir;
  u16 ana_flag;
  s16 frame;
  fx32 bottom;
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
    
    if( work->ana_flag == FALSE ){
      (*seq) = 4;
    }else{
      VecFx32 pos;
      
      MMDL_GetVectorPos( work->mmdl, &pos );
      MMDL_GetMapPosHeight( work->mmdl, &pos, &work->bottom );
      work->bottom += KAIRIKI_FALLDOWN_HEIGHT;
      
      MMDL_SetStatusBitHeightGetOFF( work->mmdl, TRUE );
      MMDL_SetStatusBitFellowHit( work->mmdl, FALSE );
      
      PMSND_PlaySE( SEQ_SE_FLD_04 );
      (*seq)++;
    }
    break;
  case 2: //怪力穴はまり
    {
      VecFx32 pos;
      MMDL_GetVectorPos( work->mmdl, &pos );
  
      if( work->bottom < pos.y ){
        pos.y -= 0x4000;
        if( work->bottom > pos.y ){
          pos.y = work->bottom;
        }
      }else if( work->bottom > pos.y ){
        pos.y += 0x4000;
        if( work->bottom < pos.y ){
          pos.y = work->bottom;
        }
      }
     
  	  MMDL_SetVectorPos( work->mmdl, &pos );
     
      if( work->bottom == pos.y ){
        work->offs.y = NUM_FX32( 2 );
        (*seq)++;
      }
    }
    break;
  case 3: //怪力穴はまり 着地後の振動
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
      MMDL_ROCKPOS_SavePos( work->mmdl );
      (*seq)++;
    }
    break;
  case 4:
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//======================================================================
//  波乗りイベント
//======================================================================
typedef struct
{
  u16 seq;
  s16 wait;
  u16 end_flag;
  u16 dir;
  u16 kishi_flag;
  FIELDMAP_WORK *fieldmap;
  FIELD_PLAYER *fld_player;
  FIELD_PLAYER_GRID *gjiki;
}EVWORK_NAMINORI;

static void evtcb_NaminoriStart( GFL_TCB *tcb, void *wk );

//--------------------------------------------------------------
/**
 * 波乗り開始イベントセット
 * @param fld_player FIELD_PLAYER
 * @param dir 移動方向 DIR_UP等
 * @param attr 波乗りを行うMAPATTR
 * @param heapID イベント実行用HEAPID
 * @retval GFL_TCB*
 */
//--------------------------------------------------------------
GFL_TCB * FIELD_PLAYER_GRID_SetEventNaminoriStart(
    FIELD_PLAYER *fld_player, u16 dir, MAPATTR attr, HEAPID heapID )
{
  EVWORK_NAMINORI *work;
  
  work = GFL_HEAP_AllocClearMemoryLo( heapID, sizeof(EVWORK_NAMINORI) );
  
  work->fieldmap = FIELD_PLAYER_GetFieldMapWork( fld_player );
  work->dir = dir;
  work->fld_player = fld_player;
  
  {
    work->gjiki = FIELDMAP_GetPlayerGrid( work->fieldmap );
  }
  
  {
    MAPATTR_VALUE val;
    val = MAPATTR_GetAttrValue( attr );
    
    if( MAPATTR_VALUE_CheckShore(val) == TRUE ){
      work->kishi_flag = TRUE;
    }
  }
  
  {
    GFL_TCBSYS *tcbsys = FIELDMAP_GetFieldmapTCBSys( work->fieldmap );
    GFL_TCB *tcb = GFL_TCB_AddTask( tcbsys, evtcb_NaminoriStart, work, 0 );
    return( tcb );
  }
}

//--------------------------------------------------------------
/**
 * 波乗り開始イベント　チェック
 * @param tcb FIELD_PLAYER_GRID_SetEventNaminoriStart()戻り値
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_GRID_CheckEventNaminoriStart( GFL_TCB *tcb )
{
  EVWORK_NAMINORI *work;
  work = GFL_TCB_GetWork( tcb );
  return( work->end_flag );
}

//--------------------------------------------------------------
/**
 * 波乗り開始イベント　削除
 * @param tcb FIELD_PLAYER_GRID_SetEventNaminoriStart()戻り値
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_DeleteEventNaminoriStart( GFL_TCB *tcb )
{
  EVWORK_NAMINORI *work;
  work = GFL_TCB_GetWork( tcb );
  GFL_HEAP_FreeMemory( work );
  GFL_TCB_DeleteTask( tcb );
}

//--------------------------------------------------------------
/**
 * 波乗り開始イベントタスク
 * @param tcb GFL_TCB*
 * @param wk tcb work
 * @retval nothing
 */
//--------------------------------------------------------------
static void evtcb_NaminoriStart( GFL_TCB *tcb, void *wk )
{
  FLDEFF_TASK *task;
  EVWORK_NAMINORI *work = wk;
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( work->fld_player );
  
  switch( work->seq )
  {
  case 0: //波乗りポケモン出現
    {
      u16 dir;
      s16 gx,gz;
      fx32 height;
      VecFx32 pos;
      dir = MMDL_GetDirDisp( mmdl );
      gx = MMDL_GetGridPosX( mmdl );
      gz = MMDL_GetGridPosZ( mmdl );
      MMDL_GetVectorPos( mmdl, &pos );
      MMDL_TOOL_GetCenterGridPos( gx, gz, &pos );
      
      {
        fx32 range = GRID_FX32; //１マス先
        
        if( work->kishi_flag == TRUE ){
          range <<= 1; //２マス先
        }
      
        MMDL_TOOL_AddDirVector( dir, &pos, range );
      }
      
      height = 0;
      MMDL_GetMapPosHeight( mmdl, &pos, &height );
      pos.y = height;
     
      {
        FLDEFF_CTRL *fectrl;
        fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
        task = FLDEFF_NAMIPOKE_SetMMdl(
            fectrl, dir, &pos, mmdl, NAMIPOKE_JOINT_OFF );
        FIELD_PLAYER_GRID_SetEffectTaskWork( work->gjiki, task );
      }
    }
    
    work->seq++;
    break;
  case 1:
    work->wait++;
    if( work->wait > 15 ){
      work->seq++;
    }
    break;
  case 2:
    {
      u16 dir = MMDL_GetDirDisp( mmdl );
      u16 ac = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_1G_8F );
      
      if( work->kishi_flag == TRUE ){
        ac = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
      }
      
      MMDL_SetAcmd( mmdl, ac );
    }
    work->seq++;
    break;
  case 3:
    if( MMDL_CheckEndAcmd(mmdl) == TRUE ){
      MMDL_EndAcmd( mmdl );
      task = FIELD_PLAYER_GRID_GetEffectTaskWork( work->gjiki );
      FLDEFF_NAMIPOKE_SetJointFlag( task, NAMIPOKE_JOINT_ON );
      FIELD_PLAYER_SetNaminori( work->gjiki );
      
      work->end_flag = TRUE;
      work->seq++;
    }
    break;
  case 4:
    break;
  }
}

//======================================================================
//  滝登りイベント
//======================================================================
#define TAKI_MOVE_FRAME (64)
#define TAKI_MOVE_FRAME_Z_START (32)

//--------------------------------------------------------------
/// TAKINOBORI_WORK
//--------------------------------------------------------------
typedef struct
{
  u16 nobori_flag;
  s16 seq_no;
  u16 dir;
  u16 end_flag;
  u16 frame;
  VecFx32 calc;
  VecFx32 offset;
  VecFx32 target_pos;
  VecFx32 add_val;

  FIELD_PLAYER *fld_player;
  FIELDMAP_WORK *fieldmap;
  FIELD_PLAYER_GRID *gjiki;
  
  FLDEFF_TASK *eff_task;
}TAKINOBORI_WORK;

static void evtcb_Takinobori( GFL_TCB *tcb, void *wk );

//--------------------------------------------------------------
/**
 * 滝登りイベントセット
 * @param fld_player FIELD_PLAYER
 * @param dir 移動方向 DIR_UP=滝登り DIR_DOWN=滝下り
 * @param no 技を使用する手持ちポケモン位置番号
 * @param heapID イベント実行用HEAPID
 * @retval
 */
//--------------------------------------------------------------
GFL_TCB * FIELD_PLAYER_GRID_SetEventTakinobori(
    FIELD_PLAYER *fld_player, u16 dir, u16 no, HEAPID heapID )
{
  TAKINOBORI_WORK *work;
   
  work = GFL_HEAP_AllocClearMemoryLo( heapID, sizeof(TAKINOBORI_WORK) );
  
  work->dir = dir;
  work->fld_player = fld_player;
  work->fieldmap = FIELD_PLAYER_GetFieldMapWork( fld_player );
  
  {
    MMDL *mmdl;
    fx32 height;
    VecFx32 pos,next;
    
    mmdl = FIELD_PLAYER_GetMMdl( fld_player );
    MMDL_GetVectorPos( mmdl, &pos );
    next = pos;
    
    MMDL_TOOL_AddDirVector( work->dir, &next, GRID_FX32*2 );
    MMDL_GetMapPosHeight( mmdl, &next, &height );
    
    if( height > pos.y ){
      work->nobori_flag = TRUE;
    }
  }
  
  {
    FIELDMAP_CTRL_GRID *gridMap = FIELDMAP_GetMapCtrlWork( work->fieldmap );
    work->gjiki = FIELDMAP_GetPlayerGrid( work->fieldmap );
  }
  
  {
    GFL_TCBSYS *tcbsys = FIELDMAP_GetFieldmapTCBSys( work->fieldmap );
    GFL_TCB *tcb = GFL_TCB_AddTask( tcbsys, evtcb_Takinobori, work, 0 );
    return( tcb );
  }
}

//--------------------------------------------------------------
/**
 * 滝登りイベント　チェック
 * @param tcb FIELD_PLAYER_GRID_SetEventTakinobori()戻り値
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_GRID_CheckEventTakinobori( GFL_TCB *tcb )
{
  TAKINOBORI_WORK *work;
  work = GFL_TCB_GetWork( tcb );
  return( work->end_flag );
}

//--------------------------------------------------------------
/**
 * 滝登りイベント　削除
 * @param tcb FIELD_PLAYER_GRID_SetEventTakinobori()戻り値
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GRID_DeleteEventTakinobori( GFL_TCB *tcb )
{
  TAKINOBORI_WORK *work;
  work = GFL_TCB_GetWork( tcb );
  GFL_HEAP_FreeMemory( work );
  GFL_TCB_DeleteTask( tcb );
}

#define TAKINOBORI_START_SHAKE_FRAME (16)
#define TAKINOBORI_ZOOM_VALUE (NUM_FX32(32))
#define TAKINOBORI_ADD_VAL_Y (NUM_FX32(6))
#define TAKINOBORI_MOVE_FRAME_W (16)
#define TAKINOBORI_MOVE_FRAME_W_HALF (TAKINOBORI_MOVE_FRAME_W/2)
#define TAKINOBORI_JUMP_START_Y (NUM_FX32(6))
#define TAKINOBORI_JUMP_TOP_Y (NUM_FX32(14))

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void takinobori_SetJikiOffset( MMDL *mmdl, const VecFx32 *offset )
{
  VecFx32 offs = *offset;
  offs.y += (FX32_ONE*8);
  offs.z += (FX32_ONE*4);
  MMDL_SetVectorOuterDrawOffsetPos( mmdl, &offs );
}

//--------------------------------------------------------------
/**
 * 滝登りイベント　動作 0　初期化、振動開始
 * @param work TAKINOBORI_WORK*
 * @retval BOOL TRUE=続けて動作する。
 */
//--------------------------------------------------------------
static BOOL ev_Takinobori_0( TAKINOBORI_WORK *work )
{
  {
    MMDL *mmdl;
    VecFx32 now,half,next;
    
    mmdl = FIELD_PLAYER_GetMMdl( work->fld_player );
    MMDL_SetStatusBitHeightGetOFF( mmdl, TRUE );
    
    MMDL_GetVectorPos( mmdl, &now );
    half = now;
    next = now;
    MMDL_TOOL_AddDirVector( work->dir, &half, GRID_FX32*1 );
    MMDL_TOOL_AddDirVector( work->dir, &next, GRID_FX32*2 );
    
    MMDL_TOOL_GetCenterGridPos( SIZE_GRID_FX32(half.x),
        SIZE_GRID_FX32(half.z), &half );
    
    MMDL_TOOL_GetCenterGridPos( SIZE_GRID_FX32(next.x),
        SIZE_GRID_FX32(next.z), &work->target_pos );
    work->target_pos = next;
    MMDL_GetMapPosHeight( mmdl, &next, &work->target_pos.y );
    
    work->add_val.x = ((half.x - now.x)/2) / TAKINOBORI_START_SHAKE_FRAME;
    work->add_val.z = ((half.z - now.z)/2) / TAKINOBORI_START_SHAKE_FRAME;
    work->add_val.y = TAKINOBORI_ADD_VAL_Y;
    
    work->offset.y = NUM_FX32( 1 ); //振動
  }
  
  {
    FLDEFF_TASK *task;
    
    task = FIELD_PLAYER_GRID_GetEffectTaskWork( work->gjiki );
    FLDEFF_NAMIPOKE_SetJointFlag( task, NAMIPOKE_JOINT_ONLY );
    
    {
      FLDEFF_CTRL *fectrl;
      NAMIPOKE_EFFECT_TYPE type = NAMIPOKE_EFFECT_TYPE_TAKI_START_F;
      
      if( work->dir == DIR_LEFT || work->dir == DIR_RIGHT ){
        type = NAMIPOKE_EFFECT_TYPE_TAKI_START_S;
      }
      
      fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
      
      {
        FLDEFF_PROCID id = FLDEFF_PROCID_NAMIPOKE_EFFECT;
        FLDEFF_CTRL_RegistEffect( fectrl, &id, 1 );
      }
      
      work->eff_task =
        FLDEFF_NAMIPOKE_EFFECT_SetEffect( fectrl, type, task );
    }
  }
  
  FIELDMAP_TCB_AddTask_CameraZoom_Sharp( work->fieldmap,
      TAKINOBORI_START_SHAKE_FRAME, TAKINOBORI_ZOOM_VALUE );
  
  
  PMSND_PlaySE( SEQ_SE_FLD_81 );
  work->seq_no++;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 滝登りイベント　動作 1　振動
 * @param work TAKINOBORI_WORK*
 * @retval BOOL TRUE=続けて動作する。
 */
//--------------------------------------------------------------
static BOOL ev_Takinobori_1( TAKINOBORI_WORK *work )
{
  fx32 y;
  VecFx32 pos;
  MMDL *mmdl;
  
  mmdl = FIELD_PLAYER_GetMMdl( work->fld_player );

  MMDL_GetVectorPos( mmdl, &pos );
  pos.x += work->add_val.x;
  pos.z += work->add_val.z;
  MMDL_SetVectorPos( mmdl, &pos );
  
  work->offset.y = -work->offset.y;
  work->frame++; 
  
  if( work->frame >= TAKINOBORI_START_SHAKE_FRAME ){
    work->frame = 0;
    work->offset.y = 0;
    work->seq_no++;
    
    FLDEFF_TASK_CallDelete( work->eff_task );
    
    {
      FLDEFF_TASK *task;
      task = FIELD_PLAYER_GRID_GetEffectTaskWork( work->gjiki );
      
      {
        FLDEFF_CTRL *fectrl;
        NAMIPOKE_EFFECT_TYPE type = NAMIPOKE_EFFECT_TYPE_TAKI_LOOP_F;
        
        if( work->dir == DIR_LEFT || work->dir == DIR_RIGHT ){
          type = NAMIPOKE_EFFECT_TYPE_TAKI_LOOP_F;
        }
        
        fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
        work->eff_task =
          FLDEFF_NAMIPOKE_EFFECT_SetEffect( fectrl, type, task );
      }
    }
  }
  
  takinobori_SetJikiOffset( mmdl, &work->offset );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 滝登りイベント　動作 2　上昇
 * @param work TAKINOBORI_WORK*
 * @retval BOOL TRUE=続けて動作する。
 */
//--------------------------------------------------------------
static BOOL ev_Takinobori_2( TAKINOBORI_WORK *work )
{
  VecFx32 pos;
  MMDL *mmdl;
  
  mmdl = FIELD_PLAYER_GetMMdl( work->fld_player );
  MMDL_GetVectorPos( mmdl, &pos );
  
  pos.y += work->add_val.y;
  
  if( pos.y >= work->target_pos.y ){
    pos.y = work->target_pos.y;
    work->seq_no++;
    
    work->add_val.x = work->target_pos.x - pos.x;
    work->add_val.x /= TAKINOBORI_MOVE_FRAME_W;
    work->add_val.z = work->target_pos.z - pos.z;
    work->add_val.z /= TAKINOBORI_MOVE_FRAME_W;
    
    KAGAYA_Printf( "add_val x = %xH, z = %xH\n",
        work->add_val.x, work->add_val.z );
    
    work->add_val.y = TAKINOBORI_JUMP_START_Y;
    work->calc.y = -(TAKINOBORI_JUMP_TOP_Y - TAKINOBORI_JUMP_START_Y);
    work->calc.y /= 11;

    FIELDMAP_TCB_AddTask_CameraZoom_Sharp( work->fieldmap,
      TAKINOBORI_START_SHAKE_FRAME, -TAKINOBORI_ZOOM_VALUE );
    
    FLDEFF_TASK_CallDelete( work->eff_task );
  }
  
  MMDL_SetVectorPos( mmdl, &pos );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 滝登りイベント　動作 3 つき抜け
 * @param work TAKINOBORI_WORK*
 * @retval BOOL TRUE=続けて動作する。
 */
//--------------------------------------------------------------
static BOOL ev_Takinobori_3( TAKINOBORI_WORK *work )
{
  VecFx32 pos;
  MMDL *mmdl;
  
  mmdl = FIELD_PLAYER_GetMMdl( work->fld_player );
  MMDL_GetVectorPos( mmdl, &pos );
  
  KAGAYA_Printf(
      "NAMINORI X = %d, Z = %d, ", FX32_NUM(pos.x), FX32_NUM(pos.z) );
  
  if( pos.x < work->target_pos.x ){
    pos.x += work->add_val.x;
    if( pos.x > work->target_pos.x ){
      pos.x = work->target_pos.x;
    }
  }else{
    pos.x += work->add_val.x;
    if( pos.x < work->target_pos.x ){
      pos.x = work->target_pos.x;
    }
  }

  if( pos.z < work->target_pos.z ){
    pos.z += work->add_val.z;
    if( pos.z > work->target_pos.z ){
      pos.z = work->target_pos.z;
    }
  }else{
    pos.z += work->add_val.z;
    if( pos.z < work->target_pos.z ){
      pos.z = work->target_pos.z;
    }
  }
  
  pos.y += work->add_val.y;
  work->add_val.y += work->calc.y;
  
  work->frame++;
  
  if( work->frame >= TAKINOBORI_MOVE_FRAME_W ){
    pos = work->target_pos;
    work->offset.x = 0;
    work->offset.y = 0;
    work->offset.z = 0;
    work->seq_no++;
  }
  
  MMDL_SetVectorPos( mmdl, &pos );
  takinobori_SetJikiOffset( mmdl, &work->offset );
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 滝登りイベント　動作 4 着水、終了へ
 * @param work TAKINOBORI_WORK*
 * @retval BOOL TRUE=続けて動作する。
 */
//--------------------------------------------------------------
static BOOL ev_Takinobori_4( TAKINOBORI_WORK *work )
{
  MMDL *mmdl;
  FLDEFF_TASK *task;
  
  mmdl = FIELD_PLAYER_GetMMdl( work->fld_player );
  MMDL_SetStatusBitHeightGetOFF( mmdl, FALSE );
  MMDL_InitPosition( mmdl, &work->target_pos, work->dir );
  
  task = FIELD_PLAYER_GRID_GetEffectTaskWork( work->gjiki );
  FLDEFF_NAMIPOKE_SetJointFlag( task, NAMIPOKE_JOINT_ON );
  
  {
    FLDEFF_CTRL *fectrl;
    NAMIPOKE_EFFECT_TYPE type = NAMIPOKE_EFFECT_TYPE_TAKI_SPLASH;
    
    fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
    work->eff_task =
      FLDEFF_NAMIPOKE_EFFECT_SetEffect( fectrl, type, task );
  }
  
  PMSND_PlaySE( SEQ_SE_FLD_83 );
  work->seq_no++;
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 滝登りイベント　動作 5 終了
 * @param work TAKINOBORI_WORK*
 * @retval BOOL TRUE=続けて動作する。
 */
//--------------------------------------------------------------
static BOOL ev_Takinobori_5( TAKINOBORI_WORK *work )
{
  if( work->eff_task != NULL ){
    if( FLDEFF_NAMIPOKE_EFFECT_CheckTaskEnd(work->eff_task) == TRUE ){
      FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
      FLDEFF_TASK_CallDelete( work->eff_task );
      FLDEFF_CTRL_DeleteEffect( fectrl, FLDEFF_PROCID_NAMIPOKE_EFFECT );
      work->eff_task = NULL;
      work->end_flag = TRUE;
    }
  }
  return( FALSE );
}

//--------------------------------------------------------------
/// 滝登り処理テーブル
//--------------------------------------------------------------
static BOOL (* const data_TakinoboriTbl[])( TAKINOBORI_WORK * ) =
{
  ev_Takinobori_0,
  ev_Takinobori_1,
  ev_Takinobori_2,
  ev_Takinobori_3,
  ev_Takinobori_4,
  ev_Takinobori_5,
};

#define TAKIKUDARI_START_W_FRAME (16)
#define TAKIKUDARI_END_W_FRAME (16)
#define TAKIKUDARI_END_START_W (4)

//--------------------------------------------------------------
/**
 * 滝下りイベント
 * @param work TAKINOBORI_WORK*
 * @retval nothing
 */
//--------------------------------------------------------------
static void ev_Takikudari( TAKINOBORI_WORK *work )
{
  VecFx32 pos;
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( work->fld_player );

  switch( work->seq_no ){
  case 0: //初期化
    {
      VecFx32 now,half,next;
      
      MMDL_SetStatusBitHeightGetOFF( mmdl, TRUE );
      
      MMDL_GetVectorPos( mmdl, &now );
      half = now;
      next = now;
      
      MMDL_TOOL_AddDirVector( work->dir, &half, GRID_FX32*1 );
      MMDL_TOOL_AddDirVector( work->dir, &next, GRID_FX32*2 );
      
      MMDL_TOOL_GetCenterGridPos( SIZE_GRID_FX32(half.x),
          SIZE_GRID_FX32(half.z), &half );
      MMDL_TOOL_AddDirVector( work->dir, &half, GRID_FX32/4 );
      
      MMDL_TOOL_GetCenterGridPos( SIZE_GRID_FX32(next.x),
        SIZE_GRID_FX32(next.z), &next );
      work->target_pos = next;
      MMDL_GetMapPosHeight( mmdl, &next, &work->target_pos.y );
      
      work->add_val.x = (half.x - now.x) / TAKIKUDARI_START_W_FRAME;
      work->add_val.z = (half.z - now.z) / TAKIKUDARI_START_W_FRAME;
      work->add_val.y = -TAKINOBORI_ADD_VAL_Y;
      
      work->offset.y = NUM_FX32( 1 ); //振動
      
      {
        FLDEFF_TASK *task;
    
        task = FIELD_PLAYER_GRID_GetEffectTaskWork( work->gjiki );
        FLDEFF_NAMIPOKE_SetJointFlag( task, NAMIPOKE_JOINT_ONLY );
        
        {
          FLDEFF_CTRL *fectrl;
          NAMIPOKE_EFFECT_TYPE type = NAMIPOKE_EFFECT_TYPE_TAKI_START_F;
          
          if( work->dir == DIR_LEFT || work->dir == DIR_RIGHT ){
            type = NAMIPOKE_EFFECT_TYPE_TAKI_START_S;
          }
          
          fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
          
          {
            FLDEFF_PROCID id = FLDEFF_PROCID_NAMIPOKE_EFFECT;
            FLDEFF_CTRL_RegistEffect( fectrl, &id, 1 );
          }
          
          work->eff_task =
            FLDEFF_NAMIPOKE_EFFECT_SetEffect( fectrl, type, task );
        }
      }
  
      FIELDMAP_TCB_AddTask_CameraZoom_Sharp( work->fieldmap,
          TAKIKUDARI_START_W_FRAME, TAKINOBORI_ZOOM_VALUE );
      
      PMSND_PlaySE( SEQ_SE_FLD_82 );
      work->seq_no++;
    }
  case 1: //振動＆横方向移動
    MMDL_GetVectorPos( mmdl, &pos );
    pos.x += work->add_val.x;
    pos.z += work->add_val.z;
    MMDL_SetVectorPos( mmdl, &pos );
    
    work->offset.y = -work->offset.y;
    work->frame++; 
    
    if( work->frame >= TAKIKUDARI_START_W_FRAME ){
      work->frame = 0;
      work->offset.y = 0;
      work->seq_no++;
      
      FLDEFF_TASK_CallDelete( work->eff_task );
    
      {
        FLDEFF_TASK *task;
        task = FIELD_PLAYER_GRID_GetEffectTaskWork( work->gjiki );
      
        {
          FLDEFF_CTRL *fectrl;
          NAMIPOKE_EFFECT_TYPE type = NAMIPOKE_EFFECT_TYPE_TAKI_LOOP_F;
        
          if( work->dir == DIR_LEFT || work->dir == DIR_RIGHT ){
            type = NAMIPOKE_EFFECT_TYPE_TAKI_LOOP_F;
          }
        
          fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
          work->eff_task =
            FLDEFF_NAMIPOKE_EFFECT_SetEffect( fectrl, type, task );
        }
      }
    }
    
    takinobori_SetJikiOffset( mmdl, &work->offset );
    break;
  case 2: //下る
    MMDL_GetVectorPos( mmdl, &pos );
    pos.y += work->add_val.y;
  
    if( pos.y <= work->target_pos.y ){
      pos.y = work->target_pos.y;
      work->seq_no++;
      
      work->add_val.x = work->target_pos.x - pos.x;
      work->add_val.x /= TAKIKUDARI_END_W_FRAME;
      work->add_val.z = work->target_pos.z - pos.z;
      work->add_val.z /= TAKIKUDARI_END_W_FRAME;
      
      KAGAYA_Printf( "add_val x = %xH, z = %xH\n",
          work->add_val.x, work->add_val.z );
    
      FIELDMAP_TCB_AddTask_CameraZoom_Sharp( work->fieldmap,
        TAKIKUDARI_END_W_FRAME, -TAKINOBORI_ZOOM_VALUE );
      
      FLDEFF_TASK_CallDelete( work->eff_task );
      
      {
        FLDEFF_CTRL *fectrl;
        FLDEFF_TASK *task;
        NAMIPOKE_EFFECT_TYPE type = NAMIPOKE_EFFECT_TYPE_TAKI_SPLASH;
        
        task = FIELD_PLAYER_GRID_GetEffectTaskWork( work->gjiki );
        FLDEFF_NAMIPOKE_SetJointFlag( task, NAMIPOKE_JOINT_ON );

        fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
        work->eff_task =
          FLDEFF_NAMIPOKE_EFFECT_SetEffect( fectrl, type, task );
      }
    }
  
    MMDL_SetVectorPos( mmdl, &pos );
    break;
  case 3:
    MMDL_GetVectorPos( mmdl, &pos );
    pos.x += work->add_val.x;
    pos.z += work->add_val.z;
    MMDL_SetVectorPos( mmdl, &pos );

    work->frame++; 
    
    if( work->frame >= TAKIKUDARI_END_W_FRAME ){
      FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( work->fieldmap );
      MMDL_SetStatusBitHeightGetOFF( mmdl, FALSE );
      MMDL_InitPosition( mmdl, &work->target_pos, work->dir );
      FLDEFF_TASK_CallDelete( work->eff_task );
      FLDEFF_CTRL_DeleteEffect( fectrl, FLDEFF_PROCID_NAMIPOKE_EFFECT );
      work->eff_task = NULL;
      work->end_flag = TRUE;
      PMSND_PlaySE( SEQ_SE_FLD_83 );
      work->seq_no++;
    }
  }
}

//--------------------------------------------------------------
/**
 * 滝登りイベント
 * @param tcb GFL_TCB*
 * @param wk tcb work
 * @retval nothing
 */
//--------------------------------------------------------------
static void evtcb_Takinobori( GFL_TCB *tcb, void *wk )
{
  TAKINOBORI_WORK *work = wk;

  if( work->nobori_flag ){
    while( data_TakinoboriTbl[work->seq_no](work) == TRUE ){};
  }else{
    ev_Takikudari( work );
  }
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
