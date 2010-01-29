//======================================================================
/**
 * @file  field_comm_actor.c
 * @brief  
 * @author
 * @date
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "field_comm_actor.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// FIELD_COMM_ACTOR
//--------------------------------------------------------------
typedef struct
{
  u32 id;
  u32 del_flag;
  MMDL *fmmdl;
}FIELD_COMM_ACTOR;

//--------------------------------------------------------------
/// FIELD_COMM_ACTOR_CTRL
//--------------------------------------------------------------
struct _TAG_FIELD_COMM_ACTOR_CTRL
{
  int max;
  HEAPID heapID;
  MMDLSYS *fmmdlsys;
  FIELD_COMM_ACTOR *act_tbl;
};

//--------------------------------------------------------------
/// MV_COMMACT_WORK
//--------------------------------------------------------------
typedef struct
{
  const u16 *watch_dir;
  const VecFx32 *watch_pos;
  const BOOL *watch_vanish;
  FIELD_COMM_ACTOR *comm_actor;
}MV_COMMACT_WORK;

//======================================================================
//  proto
//======================================================================
static void fldcommAct_DeleteActor( FIELD_COMM_ACTOR *act );

static MMDL * fldcommAct_fmmdl_Add(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 code,
    const u16 *watch_dir, const VecFx32 *watch_pos, const BOOL *watch_vanish,
    FIELD_COMM_ACTOR *comm_actor );
static void fldcommAct_fmmdl_SetWatchData(
    MMDL *fmmdl, const u16 *dir, const VecFx32 *pos, const BOOL *vanish,
    FIELD_COMM_ACTOR *comm_actor );

static u16 grid_ChangeFourDir( u16 dir );

static const MMDL_HEADER fldcommActro_MMdlHeader;

//======================================================================
//  フィールド　通信用アクター制御
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド通信用アクター制御　初期化
 * @param max アクター最大数
 * @param fmmdlsys MMDLSYS*
 * @param heapID HEAPID
 * @retval FIELD_COMM_ACTOR_CTRL
 */
//--------------------------------------------------------------
FIELD_COMM_ACTOR_CTRL * FIELD_COMM_ACTOR_CTRL_Create(
    int max, MMDLSYS *fmmdlsys, HEAPID heapID )
{
  FIELD_COMM_ACTOR_CTRL *act_ctrl;
  
  act_ctrl = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(FIELD_COMM_ACTOR_CTRL) );
  act_ctrl->max = max;
  act_ctrl->fmmdlsys = fmmdlsys;
  
  act_ctrl->act_tbl =
    GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_COMM_ACTOR)*max );
  return( act_ctrl );
}

//--------------------------------------------------------------
/**
 * フィールド通信用アクター制御　削除。
 * 追加されていたアクターも削除される。
 * @param act_ctrl FIELD_COMM_ACTOR_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_COMM_ACTOR_CTRL_Delete( FIELD_COMM_ACTOR_CTRL *act_ctrl )
{
  int i = 0;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  for( i = 0; i < act_ctrl->max; i++, act++ ){
    if( act->fmmdl != NULL ){
      fldcommAct_DeleteActor( act );
    }
  }
  
  GFL_HEAP_FreeMemory( act_ctrl->act_tbl );
  GFL_HEAP_FreeMemory( act_ctrl );
}

//--------------------------------------------------------------
/**
 * フィールド通信用アクター制御　アクター追加
 * @param act_ctrl FIELD_COMM_ACTOR_CTRL*
 * @param id 任意の管理ID
 * @param code 表示コード HERO等
 * @param watch_dir 方向参照先
 * @param watch_pos 座標参照先
 * @param watch_vanish 表示フラグ TRUE=非表示 NULL指定=参照しません。
 * @retval nothing
 * @attention watch_dirとwatch_pos,vanish_flagを常に参照し、
 * アクターへ反映させるので、アクター動作中はwatch_dirと
 * watch_pos,watch_vanishは常に保持して下さい。
 */
//--------------------------------------------------------------
void FIELD_COMM_ACTOR_CTRL_AddActor(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 id, u16 code,
    const u16 *watch_dir, const VecFx32 *watch_pos, const BOOL *watch_vanish )
{
  int i;
  u16 dir;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  dir = grid_ChangeFourDir( *watch_dir ); //角度->四方向
  
  for( i = 0; i < act_ctrl->max; i++, act++ ){
    if( act->fmmdl == NULL ){
      act->fmmdl = fldcommAct_fmmdl_Add(
          act_ctrl, code, watch_dir, watch_pos, watch_vanish, act );
      act->id = id;
      OS_Printf( "FIELD_COMM_ACTOR AddActor ID %d\n", id );
      return;
    }
  }
  
  GF_ASSERT( 0 && "ERROR COMM ACTOR MAX\n");
}

//--------------------------------------------------------------
/**
 * フィールド通信用アクター制御　アクター削除
 * @param act_ctrl  FIELD_COMM_ACTOR_CTRL*
 * @param id 追加時に指定した管理ID
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_COMM_ACTOR_CTRL_DeleteActro(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 id )
{
  int i;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  for( i = 0; i < act_ctrl->max; i++, act++ ){
    if( act->fmmdl != NULL ){
      if( act->id == id ){
        fldcommAct_DeleteActor( act );
        return;
      }
    }
  }
  
  GF_ASSERT( 0 && "ERROR COMM ACTOR DELETE\n" );
}

//--------------------------------------------------------------
/**
 * フィールド通信用アクター制御　指定座標にいる通信アクターIDを返す
 * @param FIELD_COMM_ACTOR_CTRL
 * @param gx 調べるグリッドX座標
 * @param gz 調べるグリッドZ座標
 * @param outID gx,gzに居るアクターIDの格納先
 * @retval BOOL TRUE=gx,gzに通信アクターがいる。FALSE=居ない
 */
//--------------------------------------------------------------
BOOL FIELD_COMM_ACTOR_CTRL_SearchGridPos(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, s16 gx, s16 gz, u32 *outID )
{
  MMDL *mmdl;
  
  mmdl = MMDLSYS_SearchGridPos( act_ctrl->fmmdlsys, gx, gz, FALSE );
  
  if( mmdl != NULL ){
    int i;
    FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
    
    for( i = 0; i < act_ctrl->max; i++, act++ ){
      if( act->fmmdl != NULL && act->fmmdl == mmdl ){
        *outID = act->id;
        return( TRUE );
      }
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * フィールド通信用アクター　アクターが使用している動作モデル取得
 * @param act_ctrl FIELD_COMM_ACTOR_CTRL
 * @param id 追加時に指定した管理ID
 * @retval MMDL MMDL*
 */
//--------------------------------------------------------------
MMDL * FIELD_COMM_ACTOR_CTRL_GetMMdl(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 id )
{
  int i;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  for( i = 0; i < act_ctrl->max; i++, act++ ){
    if( act->fmmdl != NULL ){
      if( act->id == id ){
        return( act->fmmdl );
      }
    }
  }
  
  GF_ASSERT( 0 && "ERROR COMM ACTOR UNKNOW ID\n" );
  return( NULL );
}

//--------------------------------------------------------------
/**
 * フィールド通信用アクター　アクター削除処理
 * @param act FIELD_COMM_ACTOR
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldcommAct_DeleteActor( FIELD_COMM_ACTOR *act )
{
  if( act->del_flag == FALSE ){
    MMDL_Delete( act->fmmdl );
  }
  
  MI_CpuClear8( act, sizeof(FIELD_COMM_ACTOR) );
}

//======================================================================
//  通信用アクター　動作モデル
//======================================================================
//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル追加
 * @param act_ctrl  FIELD_COMM_ACTOR_CTRL
 * @param code  表示コード HERO等
 * @param watch_pos 表示する座標
 * @retval MMDL* 追加されたMMDL
 */
//--------------------------------------------------------------
static MMDL * fldcommAct_fmmdl_Add(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 code,
    const u16 *watch_dir, const VecFx32 *watch_pos, const BOOL *watch_vanish,
    FIELD_COMM_ACTOR *comm_actor )
{
  MMDL *fmmdl;
  MMDLSYS *fmmdlsys = act_ctrl->fmmdlsys;
  MMDL_HEADER head = fldcommActro_MMdlHeader;
  
  head.obj_code = code;
  fmmdl = MMDLSYS_AddMMdl( fmmdlsys, &head, 0 );
  
  fldcommAct_fmmdl_SetWatchData(
      fmmdl, watch_dir, watch_pos, watch_vanish, comm_actor );
  
  MMDL_InitPosition( fmmdl, watch_pos, grid_ChangeFourDir(*watch_dir) );
  MMDL_SetStatusBitHeightGetOFF( fmmdl, TRUE );
  MMDL_SetStatusBitNotZoneDelete( fmmdl, TRUE );
  MMDL_SetMoveBitRejectPauseMove( fmmdl, TRUE );
  return( fmmdl );
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　動作初期化
 * @param   fmmdl MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCommActor_Init( MMDL *fmmdl )
{
  MV_COMMACT_WORK *work;
  work = MMDL_InitMoveProcWork( fmmdl, sizeof(MV_COMMACT_WORK) );
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　動作削除
 * @param fmmdl MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCommActor_Delete( MMDL *fmmdl )
{
  MV_COMMACT_WORK *work;
  work = MMDL_GetMoveProcWork( fmmdl );
  work->comm_actor->del_flag = TRUE; //通信側アクターに削除通知
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　動作
 * @param fmmdl MMDL*
 * @retval nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCommActor_Move( MMDL *fmmdl )
{
  MV_COMMACT_WORK *work;
  work = MMDL_GetMoveProcWork( fmmdl );
  
  {
    u16 dir;
    dir = grid_ChangeFourDir( *work->watch_dir );
    MMDL_SetDirDisp( fmmdl, dir );
  }
  
  {
    VecFx32 pos;
    u16 status = DRAW_STA_STOP;
    
    MMDL_GetVectorPos( fmmdl, &pos );
    
    if( pos.x != work->watch_pos->x ||
        pos.y != work->watch_pos->y ||
        pos.z != work->watch_pos->z ){
        MMDL_InitPosition( fmmdl,
            work->watch_pos, MMDL_GetDirDisp(fmmdl) );
        status = DRAW_STA_WALK_8F;
    }
    
    MMDL_SetDrawStatus( fmmdl, status );
  }
  
  if( work->watch_vanish != NULL ){
    if( (*work->watch_vanish) == TRUE ){
      MMDL_SetStatusBitVanish( fmmdl, TRUE );
    }else{
      MMDL_SetStatusBitVanish( fmmdl, FALSE );
    }
  }
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　参照座標をセット
 * @param fmmdl MMDL*
 * @param pos 参照する座標
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldcommAct_fmmdl_SetWatchData(
    MMDL *fmmdl, const u16 *dir, const VecFx32 *pos, const BOOL *vanish,
    FIELD_COMM_ACTOR *comm_actor )
{
  MV_COMMACT_WORK *work;
  work = MMDL_GetMoveProcWork( fmmdl );
  work->watch_dir = dir;
  work->watch_pos = pos;
  work->watch_vanish = vanish;
  work->comm_actor = comm_actor;
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * 360度方向->４方向に
 * @param  dir  方向　0x10000単位
 * @retval  u16 DIR_UP等
 */
//--------------------------------------------------------------
static u16 grid_ChangeFourDir( u16 dir )
{
#if 0
  if( (dir>0x2000) && (dir<0x6000) ){
    dir = DIR_LEFT;
  }else if( (dir >= 0x6000) && (dir <= 0xa000) ){
    dir = DIR_DOWN;
  }else if( (dir > 0xa000)&&(dir < 0xe000) ){
    dir = DIR_RIGHT;
  }else{
    dir = DIR_UP;
  }
#else
  //通信はFIELD_PLAYER_GetDir()で採る
#endif
  return( dir );
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
/// 通信アクター用MMDL_HEADER
//--------------------------------------------------------------
static const MMDL_HEADER fldcommActro_MMdlHeader =
{
  MMDL_ID_COMMACTOR,
  0,
  MV_COMM_ACTOR,
  0,  ///<イベントタイプ
  0,  ///<イベントフラグ
  0,  ///<イベントID
  0,  ///<指定方向
  0,  ///<指定パラメタ 0
  0,  ///<指定パラメタ 1
  0,  ///<指定パラメタ 2
  4,  ///<X方向移動制限
  4,  ///<Z方向移動制限
  0,  ///<グリッドX
  0,  ///<グリッドZ
  0,  ///<Y値 fx32型
};
