//======================================================================
/**
 * @file	fldeff_splash.c
 * @brief	フィールド 水飛沫
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_splash.h"

//======================================================================
//	define
//======================================================================
#define SPLASH_DRAW_Y_OFFSET (FX32_ONE*-14)
#define SPLASH_DRAW_Z_OFFSET (FX32_ONE*-10)

#define SHOAL_DRAW_Z_OFFSET (FX32_ONE*8)

#define SHOAL_OBJ_ONLY_ONE //浅瀬エフェクトOBJ一つのみ

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_SPLASH型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_SPLASH FLDEFF_SPLASH;
typedef struct _TAG_FLDEFF_SHOAL FLDEFF_SHOAL;

//--------------------------------------------------------------
///	FLDEFF_SPLASH構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_SPLASH
{
	FLDEFF_CTRL *fectrl;
  GFL_BBDACT_SYS *bbdact_sys;
  u16 res_idx_splash;
};

//--------------------------------------------------------------
/// TASKHEADER_SPLASH
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_SPLASH *eff_splash;
  const MMDL *mmdl;
}TASKHEADER_SPLASH;

//--------------------------------------------------------------
/// TASKWORK_SPLASH
//--------------------------------------------------------------
typedef struct
{
  TASKHEADER_SPLASH head;
  u16 act_id;
}TASKWORK_SPLASH;

//--------------------------------------------------------------
///	FLDEFF_SHOAL構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_SHOAL
{
	FLDEFF_CTRL *fectrl;
  FLD_G3DOBJ_RESIDX res_idx_shoal;
#ifdef SHOAL_OBJ_ONLY_ONE //浅瀬エフェクトOBJ一つのみ
  FLD_G3DOBJ_OBJIDX obj_idx_shoal;
#endif
};

//--------------------------------------------------------------
/// TASKHEADER_SHOAL
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_SHOAL *eff_shoal;
  const MMDL *mmdl;
}TASKHEADER_SHOAL;

//--------------------------------------------------------------
/// TASKWORK_SHOAL
//--------------------------------------------------------------
typedef struct
{
  TASKHEADER_SHOAL head;
  MMDL_CHECKSAME_DATA samedata;
  FLD_G3DOBJ_OBJIDX obj_idx;
}TASKWORK_SHOAL;

//======================================================================
//	プロトタイプ
//======================================================================
static void splash_InitResource( FLDEFF_SPLASH *splash );
static void splash_DeleteResource( FLDEFF_SPLASH *splash );
static const FLDEFF_TASK_HEADER DATA_splashTaskHeader;

static void shoal_InitResource( FLDEFF_SHOAL *shoal );
static void shoal_DeleteResource( FLDEFF_SHOAL *shoal );
static const FLDEFF_TASK_HEADER DATA_shoalTaskHeader;

static const GFL_BBDACT_ANM * data_BlActAnm_SplashTbl[1];

//======================================================================
//	水飛沫　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 水飛沫 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_SPLASH_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_SPLASH *splash;
	FIELDMAP_WORK *fieldmap;
	
	splash = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_SPLASH) );
	splash->fectrl = fectrl;
  
  fieldmap = FLDEFF_CTRL_GetFieldMapWork( splash->fectrl );
  splash->bbdact_sys = FIELDMAP_GetEffBbdActSys( fieldmap );
  
	splash_InitResource( splash );
	return( splash );
}

//--------------------------------------------------------------
/**
 * 水飛沫 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_SPLASH_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_SPLASH *splash = work;
  splash_DeleteResource( splash );
  GFL_HEAP_FreeMemory( splash );
}

//======================================================================
//	水飛沫　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 水飛沫　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void splash_InitResource( FLDEFF_SPLASH *splash )
{
  ARCHANDLE *handle;
  GFL_BBDACT_G3DRESDATA data;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( splash->fectrl );
  
  data.texFmt = GFL_BBD_TEXFMT_PAL4;
  data.texSiz = GFL_BBD_TEXSIZ_16x64;
  data.celSizX = 16;
  data.celSizY = 16;
  data.dataCut = GFL_BBDACT_RESTYPE_DATACUT;
  
  data.g3dres = GFL_G3D_CreateResourceHandle(
      handle, NARC_fldeff_shibuki02_nsbtx );
  FLDEFF_CTRL_SetGrayScaleG3DResource( splash->fectrl, data.g3dres );
  splash->res_idx_splash = 
      GFL_BBDACT_AddResourceG3DResUnit( splash->bbdact_sys, &data, 1 );
} 

//--------------------------------------------------------------
/**
 * 水飛沫　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void splash_DeleteResource( FLDEFF_SPLASH *splash )
{
  GFL_BBDACT_RemoveResourceUnit(
      splash->bbdact_sys, splash->res_idx_splash, 1 );
}

//======================================================================
//	水飛沫　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用水飛沫　追加
 * @param FLDEFF_CTRL*
 * @param fmmdl MMDL
 * @param joint 接続フラグ TRUE=接続。
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_SPLASH_SetMMdl( FLDEFF_CTRL *fectrl, const MMDL *mmdl )
{
  TASKHEADER_SPLASH head;
  
  head.eff_splash = FLDEFF_CTRL_GetEffectWork(
      fectrl, FLDEFF_PROCID_SPLASH );
  head.mmdl = mmdl;
  
  FLDEFF_CTRL_AddTask(
      fectrl, &DATA_splashTaskHeader, NULL, 0, &head, 0 );
}

//--------------------------------------------------------------
/**
 * 水飛沫タスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void splashTask_Init( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_SPLASH *work = wk;
  const TASKHEADER_SPLASH *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  
  MMDL_GetVectorPos( work->head.mmdl, &pos );
  
  {
    GFL_BBDACT_ANMTBL tbl;
    GFL_BBDACT_ACTDATA actData;
    FLDEFF_SPLASH *splash = work->head.eff_splash;
    GFL_BBDACT_SYS *bbdact_sys = splash->bbdact_sys;
    
    pos.y += SPLASH_DRAW_Y_OFFSET;
    pos.z += SPLASH_DRAW_Z_OFFSET;

    actData.resID = splash->res_idx_splash;
    actData.sizX = FX16_ONE*1;
    actData.sizY = FX16_ONE*1;
    actData.alpha = 31;
    actData.drawEnable = TRUE;
    actData.lightMask = GFL_BBD_LIGHTMASK_0;
    actData.trans = pos;
    actData.func = NULL;
    actData.work = work;
      
    work->act_id = GFL_BBDACT_AddActEx(
        bbdact_sys, 0, &actData, 1, GFL_BBD_DRAWMODE_XZ_FLAT_BILLBORD );
      
    if( work->act_id == GFL_BBDACT_ACTUNIT_ID_INVALID ){
      FLDEFF_TASK_CallDelete( task );
      return;
    }
    
    tbl = (GFL_BBDACT_ANMTBL)data_BlActAnm_SplashTbl;
    GFL_BBDACT_SetAnimeTable( bbdact_sys, work->act_id,  tbl, 1 );
    GFL_BBDACT_SetAnimeIdxOn( bbdact_sys, work->act_id, 0 );
  }
}

//--------------------------------------------------------------
/**
 * 水飛沫タスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void splashTask_Delete( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_SPLASH *work = wk;
  GFL_BBDACT_SYS *bbdact_sys = work->head.eff_splash->bbdact_sys;
  
  if( work->act_id != GFL_BBDACT_ACTUNIT_ID_INVALID ){
    GFL_BBDACT_RemoveAct( bbdact_sys, work->act_id, 1 );
  }
}

//--------------------------------------------------------------
/**
 * 水飛沫タスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void splashTask_Update( FLDEFF_TASK *task, void *wk )
{
  u16 comm;
  TASKWORK_SPLASH *work = wk;
  FLDEFF_SPLASH *splash = work->head.eff_splash;
  GFL_BBDACT_SYS *bbdact_sys = work->head.eff_splash->bbdact_sys;
    
  if( GFL_BBDACT_GetAnimeLastCommand(bbdact_sys,work->act_id,&comm) ){
    FLDEFF_TASK_CallDelete( task );
  }
}

//--------------------------------------------------------------
/**
 * 水飛沫タスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void splashTask_Draw( FLDEFF_TASK *task, void *wk )
{
#if 0 //FLD_G3DOBJに任せる
  VecFx32 pos;
  TASKWORK_SPLASH *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
  
  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
#endif
}

//--------------------------------------------------------------
//  水飛沫タスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_splashTaskHeader =
{
  sizeof(TASKWORK_SPLASH),
  splashTask_Init,
  splashTask_Delete,
  splashTask_Update,
  splashTask_Draw,
};

//======================================================================
//  浅瀬　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 浅瀬 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_SHOAL_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_SHOAL *shoal;
	
	shoal = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_SHOAL) );
	shoal->fectrl = fectrl;
  
	shoal_InitResource( shoal );
	return( shoal );
}

//--------------------------------------------------------------
/**
 * 浅瀬 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_SHOAL_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_SHOAL *shoal = work;
  shoal_DeleteResource( shoal );
  GFL_HEAP_FreeMemory( shoal );
}

//======================================================================
//	浅瀬　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 浅瀬　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void shoal_InitResource( FLDEFF_SHOAL *shoal )
{
  ARCHANDLE *handle;
  FLD_G3DOBJ_CTRL *obj_ctrl;
  FLD_G3DOBJ_RES_HEADER head;
  
  obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( shoal->fectrl );
  handle = FLDEFF_CTRL_GetArcHandleEffect( shoal->fectrl );
  
  FLD_G3DOBJ_RES_HEADER_Init( &head );
  FLD_G3DOBJ_RES_HEADER_SetMdl(
        &head, handle, NARC_fldeff_shibuki_nsbmd );
  FLD_G3DOBJ_RES_HEADER_SetAnmArcHandle( &head, handle );
  FLD_G3DOBJ_RES_HEADER_SetAnmArcIdx(
      &head, NARC_fldeff_shibuki_nsbtp );
  
  shoal->res_idx_shoal = 
      FLD_G3DOBJ_CTRL_CreateResource( obj_ctrl, &head, FALSE );
  
#ifdef SHOAL_OBJ_ONLY_ONE //浅瀬エフェクトOBJ一つのみ
  shoal->obj_idx_shoal = FLD_G3DOBJ_CTRL_AddObject(
        obj_ctrl, shoal->res_idx_shoal, 0, NULL );
  FLD_G3DOBJ_CTRL_SetOuterDrawFlag(
      obj_ctrl, shoal->obj_idx_shoal, TRUE );
#endif

}

//--------------------------------------------------------------
/**
 * 浅瀬　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void shoal_DeleteResource( FLDEFF_SHOAL *shoal )
{
  FLD_G3DOBJ_CTRL *obj_ctrl;
  
  obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( shoal->fectrl );
  
#ifdef SHOAL_OBJ_ONLY_ONE //浅瀬エフェクトOBJ一つのみ
  FLD_G3DOBJ_CTRL_DeleteObject( obj_ctrl, shoal->obj_idx_shoal );
#endif
  
  FLD_G3DOBJ_CTRL_DeleteResource( obj_ctrl, shoal->res_idx_shoal );
}

//======================================================================
//	浅瀬　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用浅瀬　追加
 * @param FLDEFF_CTRL*
 * @param fmmdl MMDL
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_SHOAL_SetMMdl( FLDEFF_CTRL *fectrl, const MMDL *mmdl )
{
  TASKHEADER_SHOAL head;
  
  head.eff_shoal = FLDEFF_CTRL_GetEffectWork(
      fectrl, FLDEFF_PROCID_SHOAL );
  head.mmdl = mmdl;
  
  FLDEFF_CTRL_AddTask(
      fectrl, &DATA_shoalTaskHeader, NULL, 0, &head, 0 );
}

//--------------------------------------------------------------
/**
 * 浅瀬タスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void shoalTask_Init( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_SHOAL *work = wk;
  const TASKHEADER_SHOAL *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  
  MMDL_InitCheckSameData( work->head.mmdl, &work->samedata );
  
  MMDL_GetVectorPos( work->head.mmdl, &pos );
  pos.z += SHOAL_DRAW_Z_OFFSET;
  FLDEFF_TASK_SetPos( task, &pos );

#ifndef SHOAL_OBJ_ONLY_ONE //浅瀬エフェクトOBJ複数
  {
    FLDEFF_SHOAL *shoal = work->head.eff_shoal;
    u16 idx = shoal->res_idx_shoal;
    FLD_G3DOBJ_CTRL *obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( shoal->fectrl );
    work->obj_idx = FLD_G3DOBJ_CTRL_AddObject( obj_ctrl, idx, 0, &pos );
  }
#else //浅瀬エフェクトOBJ一つのみ
  {
    FLDEFF_SHOAL *shoal = work->head.eff_shoal;
    work->obj_idx = shoal->obj_idx_shoal;
  }
#endif
  
  //即反映すると親がjointフラグがセットされていない状態。
  //FLDEFF_TASK_CallUpdate( task );
}

//--------------------------------------------------------------
/**
 * 浅瀬タスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void shoalTask_Delete( FLDEFF_TASK *task, void *wk )
{
#ifndef SHOAL_OBJ_ONLY_ONE //浅瀬エフェクトOBJ複数
  TASKWORK_SHOAL *work = wk;
  FLDEFF_SHOAL *shoal = work->head.eff_shoal;
  FLD_G3DOBJ_CTRL *obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( shoal->fectrl );
  FLD_G3DOBJ_CTRL_DeleteObject( obj_ctrl, work->obj_idx );
#endif
}

//--------------------------------------------------------------
/**
 * 浅瀬タスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void shoalTask_Update( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_SHOAL *work = wk;
  
  if( MMDL_CheckSameData(work->head.mmdl,&work->samedata) == FALSE ){
    FLDEFF_TASK_CallDelete( task );
    return;
  }
  
  if( MMDL_CheckMoveBitShoalEffect(work->head.mmdl) == FALSE ){
    FLDEFF_TASK_CallDelete( task );
    return;
  }
  
#ifndef SHOAL_OBJ_ONLY_ONE //浅瀬エフェクトOBJ複数
  {
    VecFx32 pos;
    u16 idx = work->obj_idx;
    FLDEFF_SHOAL *shoal = work->head.eff_shoal;
    FLD_G3DOBJ_CTRL *obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( shoal->fectrl );
    GFL_G3D_OBJSTATUS *st = FLD_G3DOBJ_CTRL_GetObjStatus( obj_ctrl, idx );
    
    FLD_G3DOBJ_CTRL_LoopAnimeObject( obj_ctrl, idx, FX32_ONE );
    
    MMDL_GetVectorPos( work->head.mmdl, &pos );
    pos.z += SHOAL_DRAW_Z_OFFSET;
    FLDEFF_TASK_SetPos( task, &pos );
    st->trans = pos;
  }
#else //浅瀬エフェクトOBJ一つのみ
  {
    VecFx32 pos;
    u16 idx = work->obj_idx;
    FLDEFF_SHOAL *shoal = work->head.eff_shoal;
    FLD_G3DOBJ_CTRL *obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( shoal->fectrl );
    GFL_G3D_OBJSTATUS *st = FLD_G3DOBJ_CTRL_GetObjStatus( obj_ctrl, idx );
    
    if( FLD_G3DOBJ_CTRL_GetFrameFlag(obj_ctrl,idx) == FALSE ){
      FLD_G3DOBJ_CTRL_LoopAnimeObject( obj_ctrl, idx, FX32_ONE );
      FLD_G3DOBJ_CTRL_SetFrameFlag( obj_ctrl, idx, TRUE );
    }
    
    MMDL_GetVectorPos( work->head.mmdl, &pos );
    pos.z += SHOAL_DRAW_Z_OFFSET;
    FLDEFF_TASK_SetPos( task, &pos );
  }
#endif
}

//--------------------------------------------------------------
/**
 * 浅瀬タスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void shoalTask_Draw( FLDEFF_TASK *task, void *wk )
{
#ifndef SHOAL_OBJ_ONLY_ONE //浅瀬エフェクトOBJ複数
  //FLD_G3DOBJに任せる
  /*
  VecFx32 pos;
  TASKWORK_SHOAL *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
  
  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
  */
#else //浅瀬エフェクトOBJ一つのみ
  VecFx32 pos;
  TASKWORK_SHOAL *work = wk;
  u16 idx = work->obj_idx;
  FLDEFF_SHOAL *shoal = work->head.eff_shoal;
  FLD_G3DOBJ_CTRL *obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( shoal->fectrl );
  GFL_G3D_OBJSTATUS *st = FLD_G3DOBJ_CTRL_GetObjStatus( obj_ctrl, idx );
  
  FLDEFF_TASK_GetPos( task, &pos );
  st->trans = pos;
  FLD_G3DOBJ_CTRL_DrawObject( obj_ctrl, idx );
#endif
}

//--------------------------------------------------------------
//  浅瀬タスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_shoalTaskHeader =
{
  sizeof(TASKWORK_SHOAL),
  shoalTask_Init,
  shoalTask_Delete,
  shoalTask_Update,
  shoalTask_Draw,
};

//======================================================================
//  data
//======================================================================
//splash anime table
static const GFL_BBDACT_ANM data_BlActAnm_Splash[] =
{
  {0,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {1,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {2,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {3,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {GFL_BBDACT_ANMCOM_END,0,0,0},
};

static const GFL_BBDACT_ANM * data_BlActAnm_SplashTbl[1] =
{
  data_BlActAnm_Splash,
};
