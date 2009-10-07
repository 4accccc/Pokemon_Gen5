//======================================================================
/**
 * @file	fldeff_ripple.c
 * @brief	フィールド 水波紋
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_ripple.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_RIPPLE型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_RIPPLE FLDEFF_RIPPLE;

//--------------------------------------------------------------
///	FLDEFF_RIPPLE構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_RIPPLE
{
	FLDEFF_CTRL *fectrl;
  
  GFL_G3D_RES *g3d_res_mdl;
  GFL_G3D_RES *g3d_res_anm;
};

//--------------------------------------------------------------
/// TASKWORK_RIPPLE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_RIPPLE *eff_ripple;
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *obj_anm;
  GFL_G3D_RND *obj_rnd;
}TASKWORK_RIPPLE;

//--------------------------------------------------------------
/// TASKHEADER_RIPPLE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_RIPPLE *eff_ripple;
  VecFx32 pos;
}TASKHEADER_RIPPLE;

//======================================================================
//	プロトタイプ
//======================================================================
static void ripple_InitResource( FLDEFF_RIPPLE *kemu );
static void ripple_DeleteResource( FLDEFF_RIPPLE *kemu );

static const FLDEFF_TASK_HEADER DATA_rippleTaskHeader;

//======================================================================
//	水波紋　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 水波紋 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_RIPPLE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_RIPPLE *kemu;
	
	kemu = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_RIPPLE) );
	kemu->fectrl = fectrl;
  
	ripple_InitResource( kemu );
	return( kemu );
}

//--------------------------------------------------------------
/**
 * 水波紋 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_RIPPLE_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_RIPPLE *kemu = work;
  ripple_DeleteResource( kemu );
  GFL_HEAP_FreeMemory( kemu );
}

//======================================================================
//	水波紋　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 水波紋　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void ripple_InitResource( FLDEFF_RIPPLE *kemu )
{
  ARCHANDLE *handle;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( kemu->fectrl );
  
  kemu->g3d_res_mdl	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_ripple_nsbmd );
  GFL_G3D_TransVramTexture( kemu->g3d_res_mdl );

  kemu->g3d_res_anm	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_ripple_nsbtp );
}

//--------------------------------------------------------------
/**
 * 水波紋　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void ripple_DeleteResource( FLDEFF_RIPPLE *kemu )
{
 	GFL_G3D_DeleteResource( kemu->g3d_res_anm );
 	GFL_G3D_DeleteResource( kemu->g3d_res_mdl );
}

//======================================================================
//	水波紋　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用水波紋　追加
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_RIPPLE_SetMMdl( MMDL *fmmdl, FLDEFF_CTRL *fectrl )
{
  VecFx32 pos;
  FLDEFF_RIPPLE *kemu;
  TASKHEADER_RIPPLE head;
  
  MMDL_GetVectorPos( fmmdl, &pos );
  pos.y += FX32_ONE*1;
  pos.z += FX32_ONE*12;
  
  kemu = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_RIPPLE );
  head.eff_ripple = kemu;
  head.pos = pos;
  
  FLDEFF_CTRL_AddTask(
      fectrl, &DATA_rippleTaskHeader, NULL, 0, &head, 0 );
}

//--------------------------------------------------------------
/**
 * 水波紋タスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void rippleTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_RIPPLE *work = wk;
  const TASKHEADER_RIPPLE *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->eff_ripple = head->eff_ripple;
  FLDEFF_TASK_SetPos( task, &head->pos );
  
  work->obj_rnd =
    GFL_G3D_RENDER_Create(
        work->eff_ripple->g3d_res_mdl, 0, work->eff_ripple->g3d_res_mdl );
  
  work->obj_anm =
    GFL_G3D_ANIME_Create(
        work->obj_rnd, work->eff_ripple->g3d_res_anm, 0 );
  
  work->obj = GFL_G3D_OBJECT_Create(
      work->obj_rnd, &work->obj_anm, 1 );
  GFL_G3D_OBJECT_EnableAnime( work->obj, 0 );
}

//--------------------------------------------------------------
/**
 * 水波紋タスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void rippleTask_Delete( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_RIPPLE *work = wk;
  GFL_G3D_ANIME_Delete( work->obj_anm );
  GFL_G3D_OBJECT_Delete( work->obj );
	GFL_G3D_RENDER_Delete( work->obj_rnd );
}

//--------------------------------------------------------------
/**
 * 水波紋タスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void rippleTask_Update( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_RIPPLE *work = wk;
  
  if( GFL_G3D_OBJECT_IncAnimeFrame(work->obj,0,FX32_ONE) == FALSE ){
    FLDEFF_TASK_CallDelete( task );
  }
}

//--------------------------------------------------------------
/**
 * 水波紋タスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void rippleTask_Draw( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_RIPPLE *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};

  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
}

//--------------------------------------------------------------
//  水波紋タスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_rippleTaskHeader =
{
  sizeof(TASKWORK_RIPPLE),
  rippleTask_Init,
  rippleTask_Delete,
  rippleTask_Update,
  rippleTask_Draw,
};
