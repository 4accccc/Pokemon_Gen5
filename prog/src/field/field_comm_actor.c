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

//SCRID_DUMMY
#include "../../../resource/fldmapdata/script/common_scr_def.h"

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
  u16 del_flag;
  u16 dash_flag;
  MMDL *mmdl;
}FIELD_COMM_ACTOR;

//--------------------------------------------------------------
/// FIELD_COMM_ACTOR_CTRL
//--------------------------------------------------------------
struct _TAG_FIELD_COMM_ACTOR_CTRL
{
  u16 max;
  u16 dash_flag;
  MMDLSYS *mmdlsys;
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

static MMDL * fldcommAct_mmdl_Add(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 code,
    const u16 *watch_dir, const VecFx32 *watch_pos, const BOOL *watch_vanish,
    FIELD_COMM_ACTOR *comm_actor );
static void fldcommAct_mmdl_SetWatchData(
    MMDL *mmdl, const u16 *dir, const VecFx32 *pos, const BOOL *vanish,
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
 * @param mmdlsys MMDLSYS*
 * @param heapID HEAPID
 * @param dash_flag TRUE=アクターOBJがHERO,HEROINEであれば走り演出をする
 * @retval FIELD_COMM_ACTOR_CTRL
 */
//--------------------------------------------------------------
FIELD_COMM_ACTOR_CTRL * FIELD_COMM_ACTOR_CTRL_Create(
    u16 max, MMDLSYS *mmdlsys, HEAPID heapID, BOOL dash_flag )
{
  FIELD_COMM_ACTOR_CTRL *act_ctrl;
  
  act_ctrl = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(FIELD_COMM_ACTOR_CTRL) );
  act_ctrl->max = max;
  act_ctrl->dash_flag = dash_flag;
  act_ctrl->mmdlsys = mmdlsys;
  
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
  u16 i = 0;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  for( i = 0; i < act_ctrl->max; i++, act++ ){
    if( act->mmdl != NULL ){
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
  u16 i,dir;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  dir = grid_ChangeFourDir( *watch_dir ); //角度->四方向
  
  for( i = 0; i < act_ctrl->max; i++, act++ ){
    if( act->mmdl == NULL ){
      act->mmdl = fldcommAct_mmdl_Add(
          act_ctrl, code, watch_dir, watch_pos, watch_vanish, act );
      act->id = id;
      act->dash_flag = act_ctrl->dash_flag;
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
  u16 i;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  for( i = 0; i < act_ctrl->max; i++, act++ ){
    if( act->mmdl != NULL ){
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
 * @param gy 調べるグリッドY座標
 * @param gz 調べるグリッドZ座標
 * @param outID gx,gzに居るアクターIDの格納先
 * @param no 検索開始ワークNo。先頭から検索する際は初期値0を指定。
 * @retval BOOL TRUE=gx,gzに通信アクターがいる。FALSE=居ない
 * @note 引数noは呼び出し後、取得位置+1の値になる。
 */
//--------------------------------------------------------------
BOOL FIELD_COMM_ACTOR_CTRL_SearchGridPos(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, s16 gx, s16 gy, s16 gz,
    u32 *outID, u32 *no )
{
  u32 max = act_ctrl->max;
   
  if( (*no) < max ){
    s16 cy,sy;
    FIELD_COMM_ACTOR *act = &(act_ctrl->act_tbl[*no]);
    
    do{
      (*no)++;
      
      if( act->mmdl != NULL ){
        cy = MMDL_GetGridPosY( act->mmdl );
        sy = cy - gy;
        
        if( sy < 0 ){
          sy = -sy;
        }
        
        if( sy < H_GRID_FELLOW_SIZE ){
          if( MMDL_HitCheckXZ(act->mmdl,gx,gz,TRUE) ){
            *outID = act->id;
            return( TRUE );
          }
        }
        
        act++;
      }
    }while( (*no) < max );
  }
  
  return( FALSE );
}

#if 0 //old 100622 Y座標を考慮しない　BTS6450
BOOL FIELD_COMM_ACTOR_CTRL_SearchGridPos(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, s16 gx, s16 gz, u32 *outID )
{
  int i = 0;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  for( ; i < act_ctrl->max; i++, act++ ){
    if( act->mmdl != NULL ){
      if( MMDL_HitCheckXZ(act->mmdl,gx,gz,TRUE) ){
        *outID = act->id;
        return( TRUE );
      }
    }
  }
  
  return( FALSE );
}
#endif

#if 0 //old 100603 通信アクターのみをチェックする
BOOL FIELD_COMM_ACTOR_CTRL_SearchGridPos(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, s16 gx, s16 gz, u32 *outID )
{
  MMDL *mmdl;
  
  mmdl = MMDLSYS_SearchGridPos( act_ctrl->mmdlsys, gx, gz, FALSE );
  
  if( mmdl != NULL ){
    int i;
    FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
    
    for( i = 0; i < act_ctrl->max; i++, act++ ){
      if( act->mmdl != NULL && act->mmdl == mmdl ){
        *outID = act->id;
        return( TRUE );
      }
    }
  }
  
  return( FALSE );
}
#endif

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
  u16 i;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  for( i = 0; i < act_ctrl->max; i++, act++ ){
    if( act->mmdl != NULL ){
      if( act->id == id ){
        return( act->mmdl );
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
    MMDL_Delete( act->mmdl );
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
static MMDL * fldcommAct_mmdl_Add(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 code,
    const u16 *watch_dir, const VecFx32 *watch_pos, const BOOL *watch_vanish,
    FIELD_COMM_ACTOR *comm_actor )
{
  MMDL *mmdl;
  MMDLSYS *mmdlsys = act_ctrl->mmdlsys;
  MMDL_HEADER head = fldcommActro_MMdlHeader;
  
  head.obj_code = code;
  mmdl = MMDLSYS_AddMMdl( mmdlsys, &head, 0 );
  
  fldcommAct_mmdl_SetWatchData(
      mmdl, watch_dir, watch_pos, watch_vanish, comm_actor );
  
  MMDL_InitPosition( mmdl, watch_pos, grid_ChangeFourDir(*watch_dir) );
  MMDL_SetStatusBitHeightGetOFF( mmdl, TRUE ); //高さ取得無し
  MMDL_SetStatusBitAttrGetOFF( mmdl, TRUE );   //アトリビュート取得無し
  MMDL_SetStatusBitNotZoneDelete( mmdl, TRUE ); //ゾーン変更で削除しない
  MMDL_SetStatusBitFellowHit( mmdl, FALSE ); //当り判定無し
  MMDL_SetMoveBitRejectPauseMove( mmdl, TRUE ); //ポーズを無視して動く
  MMDL_SetMoveBitNotSave( mmdl, TRUE ); //セーブしない
  return( mmdl );
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　動作初期化
 * @param   mmdl MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCommActor_Init( MMDL *mmdl )
{
  MV_COMMACT_WORK *work;
  work = MMDL_InitMoveProcWork( mmdl, sizeof(MV_COMMACT_WORK) );
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　動作削除
 * @param mmdl MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCommActor_Delete( MMDL *mmdl )
{
  MV_COMMACT_WORK *work;
  work = MMDL_GetMoveProcWork( mmdl );
  work->comm_actor->del_flag = TRUE; //通信側アクターに削除通知
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　動作
 * @param mmdl MMDL*
 * @retval nothing
 */
//--------------------------------------------------------------
void MMDL_MoveCommActor_Move( MMDL *mmdl )
{
  MV_COMMACT_WORK *work;
  work = MMDL_GetMoveProcWork( mmdl );
  
  {
    u16 dir;
    dir = grid_ChangeFourDir( *work->watch_dir );
    MMDL_SetDirDisp( mmdl, dir );
  }
  
  {
    VecFx32 pos;
    u16 status = DRAW_STA_STOP;
    
    MMDL_GetVectorPos( mmdl, &pos );
    
    if( pos.x != work->watch_pos->x ||
        pos.y != work->watch_pos->y ||
        pos.z != work->watch_pos->z )
    {
      MMDL_InitPosition( mmdl,
          work->watch_pos, MMDL_GetDirDisp(mmdl) );
       
      status = DRAW_STA_WALK_8F;
      
      if( work->comm_actor->dash_flag ) //フラグ制御 100626
      {
        u16 code = MMDL_GetOBJCode( mmdl );
          
        if( code == HERO || code == HEROINE )
        {
          fx32 val,diff = 0;
            
          val = pos.x - work->watch_pos->x;
          if( val < 0 ){
            val = -val;
          }
          diff += val;
            
          val = pos.z - work->watch_pos->z;
          if( val < 0 ){
            val = -val; 
          }
          diff += val;
          
          if( diff >= NUM_FX32(4) ){
            status = DRAW_STA_HERO_DASH_4F;
          }
        }
      }
    }
    
    MMDL_SetDrawStatus( mmdl, status );
  }

#if 0 //old 100628 当たり判定は常に無しになりました
  if( work->watch_vanish != NULL ){
    BOOL hit = TRUE;
    BOOL vanish = FALSE;
    
    if( (*work->watch_vanish) == TRUE ){
      hit = FALSE;
      vanish = TRUE;
    }
    
    MMDL_SetStatusBitVanish( mmdl, vanish );
    MMDL_SetStatusBitFellowHit( mmdl, hit ); //100525 当り判定も操作
  }
#else
  if( work->watch_vanish != NULL ){
    BOOL vanish = FALSE;
    
    if( (*work->watch_vanish) == TRUE ){
      vanish = TRUE;
    }
    
    MMDL_SetStatusBitVanish( mmdl, vanish );
  }
#endif
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　参照座標をセット
 * @param mmdl MMDL*
 * @param pos 参照する座標
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldcommAct_mmdl_SetWatchData(
    MMDL *mmdl, const u16 *dir, const VecFx32 *pos, const BOOL *vanish,
    FIELD_COMM_ACTOR *comm_actor )
{
  MV_COMMACT_WORK *work;
  work = MMDL_GetMoveProcWork( mmdl );
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
  SCRID_DUMMY,  ///<イベントID
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
