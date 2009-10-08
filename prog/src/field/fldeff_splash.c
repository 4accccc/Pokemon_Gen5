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
#define SPLASH_DRAW_Z_OFFSET (FX32_ONE*(8))

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_SPLASH型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_SPLASH FLDEFF_SPLASH;

//--------------------------------------------------------------
///	FLDEFF_SPLASH構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_SPLASH
{
	FLDEFF_CTRL *fectrl;
  
  GFL_G3D_RES *g3d_res_mdl;
  GFL_G3D_RES *g3d_res_anm;
};

//--------------------------------------------------------------
/// TASKWORK_SPLASH
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_SPLASH *eff_splash;
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *obj_anm;
  GFL_G3D_RND *obj_rnd;
}TASKWORK_SPLASH;

//--------------------------------------------------------------
/// TASKHEADER_SPLASH
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_SPLASH *eff_splash;
  VecFx32 pos;
}TASKHEADER_SPLASH;

//======================================================================
//	プロトタイプ
//======================================================================
static void splash_InitResource( FLDEFF_SPLASH *splash );
static void splash_DeleteResource( FLDEFF_SPLASH *splash );

static const FLDEFF_TASK_HEADER DATA_splashTaskHeader;

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
	
	splash = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_SPLASH) );
	splash->fectrl = fectrl;
  
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
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( splash->fectrl );
  
  splash->g3d_res_mdl	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_shibuki_nsbmd );
  GFL_G3D_TransVramTexture( splash->g3d_res_mdl );

  splash->g3d_res_anm	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_shibuki_nsbtp );
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
 	GFL_G3D_DeleteResource( splash->g3d_res_anm );
 	GFL_G3D_DeleteResource( splash->g3d_res_mdl );
}

//======================================================================
//	水飛沫　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用水飛沫　追加
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_SPLASH_Set( FLDEFF_CTRL *fectrl, s16 gx, s16 gz, fx32 y )
{
  VecFx32 pos;
  FLDEFF_SPLASH *splash;
  TASKHEADER_SPLASH head;
  
  MMDL_TOOL_GetCenterGridPos( gx, gz, &pos );
  pos.z += SPLASH_DRAW_Z_OFFSET;
  
  splash = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_SPLASH );
  head.eff_splash = splash;
  head.pos = pos;
  
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
  TASKWORK_SPLASH *work = wk;
  const TASKHEADER_SPLASH *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->eff_splash = head->eff_splash;
  FLDEFF_TASK_SetPos( task, &head->pos );
  
  work->obj_rnd =
    GFL_G3D_RENDER_Create(
        work->eff_splash->g3d_res_mdl, 0, work->eff_splash->g3d_res_mdl );
  
  work->obj_anm =
    GFL_G3D_ANIME_Create(
        work->obj_rnd, work->eff_splash->g3d_res_anm, 0 );
  
  work->obj = GFL_G3D_OBJECT_Create(
      work->obj_rnd, &work->obj_anm, 1 );
  GFL_G3D_OBJECT_EnableAnime( work->obj, 0 );
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
  GFL_G3D_ANIME_Delete( work->obj_anm );
  GFL_G3D_OBJECT_Delete( work->obj );
	GFL_G3D_RENDER_Delete( work->obj_rnd );
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
  TASKWORK_SPLASH *work = wk;
  
  if( GFL_G3D_OBJECT_IncAnimeFrame(work->obj,0,FX32_ONE) == FALSE ){
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
  VecFx32 pos;
  TASKWORK_SPLASH *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};

  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
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
