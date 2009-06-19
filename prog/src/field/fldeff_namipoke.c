//======================================================================
/**
 * @file	fldeff_namipoke.c
 * @brief	フィールド 波乗りポケモン
 * @author	kagaya
 * @data	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_namipoke.h"

//======================================================================
//  define
//======================================================================
#define NAMIPOKE_SHAKE_VALUE (0x0400) ///<初期揺れ幅
#define NAMIPOKE_SHAKE_MAX (FX32_ONE*4) ///<揺れ幅最大

#define NAMIPOKE_RIDE_Y_OFFSET (FX32_ONE*7) ///<運転手表示オフセットY
#define NAMIPOKE_RIDE_Z_OFFSET (FX32_ONE*4) ///<運転手表示オフセットZ

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_NAMIPOKE型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_NAMIPOKE FLDEFF_NAMIPOKE;

//--------------------------------------------------------------
///	FLDEFF_NAMIPOKE構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_NAMIPOKE
{
	FLDEFF_CTRL *fectrl;
  GFL_G3D_RES *g3d_res_mdl;
};

//--------------------------------------------------------------
/// TASKHEADER_NAMIPOKE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_NAMIPOKE *eff_namipoke;
  MMDL *mmdl;
  u16 obj_id;
  u16 zone_id;
  u16 dir;
  u16 dmy;
  VecFx32 pos;
}TASKHEADER_NAMIPOKE;

//--------------------------------------------------------------
/// TASKWORK_NAMIPOKE
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  
  BOOL joint;
  
  u16 dir;
  fx32 shake_offs;
  fx32 shake_value;
  
  TASKHEADER_NAMIPOKE head;

  GFL_G3D_OBJ *obj;
  GFL_G3D_RND *obj_rnd;
}TASKWORK_NAMIPOKE;

//======================================================================
//	プロトタイプ
//======================================================================
static void namipoke_InitResource( FLDEFF_NAMIPOKE *namipoke );
static void namipoke_DeleteResource( FLDEFF_NAMIPOKE *namipoke );

static const FLDEFF_TASK_HEADER DATA_namipokeTaskHeader;

//======================================================================
//	波乗りポケモンエフェクト　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_NAMIPOKE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_NAMIPOKE *namipoke;
	
	namipoke = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_NAMIPOKE) );
	namipoke->fectrl = fectrl;
  
	namipoke_InitResource( namipoke );
	return( namipoke );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_NAMIPOKE_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_NAMIPOKE *namipoke = work;
  namipoke_DeleteResource( namipoke );
  GFL_HEAP_FreeMemory( namipoke );
}

//======================================================================
//	波乗りポケモンエフェクト　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipoke_InitResource( FLDEFF_NAMIPOKE *namipoke )
{
  BOOL ret;
  ARCHANDLE *handle;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( namipoke->fectrl );
  
  namipoke->g3d_res_mdl	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_sea_ride_nsbmd );
  ret = GFL_G3D_TransVramTexture( namipoke->g3d_res_mdl );
  GF_ASSERT( ret );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipoke_DeleteResource( FLDEFF_NAMIPOKE *namipoke )
{
 	GFL_G3D_DeleteResource( namipoke->g3d_res_mdl );
}

//======================================================================
//	波乗りポケモンエフェクト　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用波乗りポケモンエフェクト　追加
 * @param FLDEFF_CTRL*
 * @param pos 表示座標
 * @param mmdl MMDL ポケモンに乗る動作モデル
 * @param joint 動作モデルとの同期 OFF=ジョイントしない
 * @retval nothing
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_NAMIPOKE_SetMMdl(
    FLDEFF_CTRL *fectrl, u16 dir, const VecFx32 *pos, MMDL *mmdl, BOOL joint )
{
  fx32 h;
  FLDEFF_TASK *task;
  TASKHEADER_NAMIPOKE head;
  FLDEFF_NAMIPOKE *namipoke;
  
  namipoke = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_NAMIPOKE );
  head.eff_namipoke = namipoke;
  head.mmdl = mmdl;
  head.obj_id = MMDL_GetOBJID( mmdl );
  head.zone_id = MMDL_GetZoneID( mmdl );
  head.dir = dir;
  head.pos = *pos;
  
  task = FLDEFF_CTRL_AddTask(
      fectrl, &DATA_namipokeTaskHeader, &head.pos, joint, &head, 0 );
  return( task );
}

//--------------------------------------------------------------
/**
 * 動作モデル用波乗りポケモンエフェクト　ジョイントフラグセット
 * @param task FLDEFF_TASK*
 * @param flag TRUE=接続,FALSE=非接続
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_NAMIPOKE_SetJointFlag( FLDEFF_TASK *task, BOOL flag )
{
  TASKWORK_NAMIPOKE *work = FLDEFF_TASK_GetWork( task );
  work->joint = flag;
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipokeTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE *work = wk;
  const TASKHEADER_NAMIPOKE *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  FLDEFF_TASK_SetPos( task, &head->pos );
  
  work->dir = head->dir;
  
  work->obj_rnd =
    GFL_G3D_RENDER_Create(
        work->head.eff_namipoke->g3d_res_mdl, 0,
        work->head.eff_namipoke->g3d_res_mdl );
  
  work->obj = GFL_G3D_OBJECT_Create( work->obj_rnd, NULL, 0 );
  work->joint = FLDEFF_TASK_GetAddParam( task );
  work->shake_offs = FX32_ONE;
  work->shake_value = NAMIPOKE_SHAKE_VALUE;
  
  FLDEFF_TASK_CallUpdate( task ); //即動作
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipokeTask_Delete( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE *work = wk;
  GFL_G3D_OBJECT_Delete( work->obj );
	GFL_G3D_RENDER_Delete( work->obj_rnd );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipokeTask_Update( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE *work = wk;
  
  if( work->joint == FALSE ){ //接続しない
    return;
  }
 	
  { //方向
    work->dir = MMDL_GetDirDisp( work->head.mmdl );
  }
  
  { //揺れ
		work->shake_offs += work->shake_value;
		
		if( work->shake_offs >= NAMIPOKE_SHAKE_MAX ){
			work->shake_offs = NAMIPOKE_SHAKE_MAX;
			work->shake_value = -work->shake_value;
		}else if( work->shake_offs <= FX32_ONE ){
			work->shake_offs = FX32_ONE;
			work->shake_value = -work->shake_value;
		}
	}
  
  { //運転手に揺れを追加
    VecFx32 offs;
    offs.x = 0;
    offs.y = work->shake_offs + NAMIPOKE_RIDE_Y_OFFSET;
    offs.z = NAMIPOKE_RIDE_Z_OFFSET;
    MMDL_SetVectorOuterDrawOffsetPos( work->head.mmdl,  &offs );
  }

  { //座標
    VecFx32 pos;
    MMDL_GetVectorPos( work->head.mmdl, &pos );
    pos.y += work->shake_offs - FX32_ONE;
    FLDEFF_TASK_SetPos( task, &pos );
  }
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipokeTask_Draw( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_NAMIPOKE *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
  u16 rtbl[DIR_MAX4][3] = {{0,0x8000,0},{0,0,0},{0,0xc000,0},{0,0x4000,0}};
  u16 *rot = rtbl[work->dir];
  GFL_CALC3D_MTX_CreateRot( rot[0], rot[1], rot[2], &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
}

//--------------------------------------------------------------
//  波乗りポケモンエフェクトタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_namipokeTaskHeader =
{
  sizeof(TASKWORK_NAMIPOKE),
  namipokeTask_Init,
  namipokeTask_Delete,
  namipokeTask_Update,
  namipokeTask_Draw,
};
