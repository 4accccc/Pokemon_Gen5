//======================================================================
/**
 * @file	field_comm_actor.c
 * @brief	
 * @author
 * @data
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "field_comm_actor.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
/// FIELD_COMM_ACTOR
//--------------------------------------------------------------
typedef struct
{
  u32 id;
  FLDMMDL *fmmdl;
}FIELD_COMM_ACTOR;

//--------------------------------------------------------------
/// FIELD_COMM_ACTOR_CTRL
//--------------------------------------------------------------
struct _TAG_FIELD_COMM_ACTOR_CTRL
{
  int max;
  HEAPID heapID;
  FLDMMDLSYS *fmmdlsys;
  FIELD_COMM_ACTOR *act_tbl;
};

//--------------------------------------------------------------
/// MV_COMMACT_WORK
//--------------------------------------------------------------
typedef struct
{
  const u16 *watch_dir;
  const VecFx32 *watch_pos;
}MV_COMMACT_WORK;

//======================================================================
//	proto
//======================================================================
static void fldcommAct_DeleteActor( FIELD_COMM_ACTOR *act );

static FLDMMDL * fldcommAct_fmmdl_Add(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 code,
    const u16 *watch_dir, const VecFx32 *watch_pos );
static void fldcommAct_fmmdl_SetWatchPos(
    FLDMMDL *fmmdl, const VecFx32 *pos );

static const FLDMMDL_HEADER fldcommActro_FldMMdlHeader;

//======================================================================
//  フィールド　通信用アクター制御
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド通信用アクター制御　初期化
 * @param max アクター最大数
 * @param fmmdlsys FLDMMDLSYS*
 * @param heapID HEAPID
 * @retval FIELD_COMM_ACTOR_CTRL
 */
//--------------------------------------------------------------
FIELD_COMM_ACTOR_CTRL * FIELD_COMM_ACTOR_CTRL_Create(
    int max, FLDMMDLSYS *fmmdlsys, HEAPID heapID )
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
 * @param id 管理ID
 * @param code 表示コード HERO等
 * @param watch_pos 表示する座標
 * @retval FIELD_COMM_ACTOR* 追加されたFIELD_COMM_ACTRO*
 */
//--------------------------------------------------------------
void FIELD_COMM_ACTOR_CTRL_AddActor( FIELD_COMM_ACTOR_CTRL *act_ctrl,
    u32 id, u16 code, const u16 *watch_dir, const VecFx32 *watch_pos )
{
  int i;
  FIELD_COMM_ACTOR *act = act_ctrl->act_tbl;
  
  for( i = 0; i < act_ctrl->max; i++, act++ ){
    if( act->fmmdl == NULL ){
      act->fmmdl = fldcommAct_fmmdl_Add(
          act_ctrl, code, watch_dir, watch_pos );
      act->id = id;
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
 * フィールド通信用アクター　アクター削除処理
 * @param act FIELD_COMM_ACTOR
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldcommAct_DeleteActor( FIELD_COMM_ACTOR *act )
{
  FLDMMDL_Delete( act->fmmdl );
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
 * @retval FLDMMDL* 追加されたFLDMMDL
 */
//--------------------------------------------------------------
static FLDMMDL * fldcommAct_fmmdl_Add(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 code,
    const u16 *watch_dir, const VecFx32 *watch_pos )
{
  FLDMMDL *fmmdl;
  FLDMMDLSYS *fmmdlsys = act_ctrl->fmmdlsys;
  FLDMMDL_HEADER head = fldcommActro_FldMMdlHeader;
  
  head.obj_code = code;
  fmmdl = FLDMMDLSYS_AddFldMMdl( fmmdlsys, &head, 0 );
  fldcommAct_fmmdl_SetWatchPos( fmmdl, watch_pos );
  //座標設定
  //高さ、アトリビュート無視設定
  return( fmmdl );
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　動作初期化
 * @param   fmmdl FLDMMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveCommActor_Init( FLDMMDL *fmmdl )
{
  MV_COMMACT_WORK *work;
  work = FLDMMDL_InitMoveProcWork( fmmdl, sizeof(MV_COMMACT_WORK) );
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　動作
 * @param fmmdl FLDMMDL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveCommActor_Move( FLDMMDL *fmmdl )
{
  MV_COMMACT_WORK *work;
  work = FLDMMDL_GetMoveProcWork( fmmdl );
}

//--------------------------------------------------------------
/**
 * 通信用アクター　動作モデル　参照座標をセット
 * @param fmmdl FLDMMDL*
 * @param pos 参照する座標
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldcommAct_fmmdl_SetWatchPos(
    FLDMMDL *fmmdl, const VecFx32 *pos )
{
  MV_COMMACT_WORK *work;
  work = FLDMMDL_GetMoveProcWork( fmmdl );
  work->watch_pos = pos;
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
/// 通信アクター用FLDMMDL_HEADER
//--------------------------------------------------------------
static const FLDMMDL_HEADER fldcommActro_FldMMdlHeader =
{
  FLDMMDL_ID_COMMACTOR,
  0,
  MV_DMY,
  0,	///<イベントタイプ
  0,	///<イベントフラグ
  0,	///<イベントID
  0,	///<指定方向
  0,	///<指定パラメタ 0
  0,	///<指定パラメタ 1
  0,	///<指定パラメタ 2
  4,	///<X方向移動制限
  4,	///<Z方向移動制限
  0,	///<グリッドX
  0,	///<グリッドZ
  0,	///<Y値 fx32型
};
