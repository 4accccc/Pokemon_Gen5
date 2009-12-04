//======================================================================
/**
 * @file	fldeff_iaigiri.c
 * @brief	フィールド 居合い切り
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"
#include "field/field_const.h"

#include "fldeff_iaigiri.h"

//======================================================================
//	define
//======================================================================
typedef enum{
  ANIME_ICA,
  ANIME_IMA,
  ANIME_ITA,
  ANIME_NUM
} ANIME_INDEX;

static u32 dat_id[ANIME_NUM] =
{
  NARC_fldeff_iaigiri_nsbca,
  NARC_fldeff_iaigiri_nsbma,
  NARC_fldeff_iaigiri_nsbta,
};


//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_IAIGIRI型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_IAIGIRI FLDEFF_IAIGIRI;

//--------------------------------------------------------------
///	FLDEFF_IAIGIRI構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_IAIGIRI
{
	FLDEFF_CTRL *fectrl;
  
  GFL_G3D_RES *g3d_res_mdl;
  GFL_G3D_RES *g3d_res_anm[ANIME_NUM];
};

//--------------------------------------------------------------
/// TASKWORK_IAIGIRI
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_IAIGIRI *eff_iaigiri;
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *obj_anm[ANIME_NUM];
  GFL_G3D_RND *obj_rnd;
}TASKWORK_IAIGIRI;

//--------------------------------------------------------------
/// TASKHEADER_IAIGIRI
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_IAIGIRI *eff_iaigiri;
  VecFx32 pos;
}TASKHEADER_IAIGIRI;

//======================================================================
//	プロトタイプ
//======================================================================
static void iaigiri_InitResource( FLDEFF_IAIGIRI *iai );
static void iaigiri_DeleteResource( FLDEFF_IAIGIRI *iai );

static const FLDEFF_TASK_HEADER DATA_iaigiriTaskHeader;

//======================================================================
//	居合い切り　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 居合い切り 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_IAIGIRI_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_IAIGIRI *iai;
	
	iai = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_IAIGIRI) );
	iai->fectrl = fectrl;
  
	iaigiri_InitResource( iai );
	return( iai );
}

//--------------------------------------------------------------
/**
 * 居合い切り 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_IAIGIRI_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_IAIGIRI *iai = work;
  iaigiri_DeleteResource( iai );
  GFL_HEAP_FreeMemory( iai );
}

//======================================================================
//	居合い切り　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 居合い切り　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void iaigiri_InitResource( FLDEFF_IAIGIRI *iai )
{
  int i;
  ARCHANDLE *handle;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( iai->fectrl );
  
  iai->g3d_res_mdl	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_iaigiri_nsbmd );
  GFL_G3D_TransVramTexture( iai->g3d_res_mdl );

  for( i=0; i<ANIME_NUM; i++ )
  {
    iai->g3d_res_anm[i] =
      GFL_G3D_CreateResourceHandle( handle, dat_id[i] );
  }
}

//--------------------------------------------------------------
/**
 * 居合い切り　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void iaigiri_DeleteResource( FLDEFF_IAIGIRI *iai )
{
  int i;
  for( i=0; i<ANIME_NUM; i++ )
  {
    GFL_G3D_DeleteResource( iai->g3d_res_anm[i] );
  }
 	GFL_G3D_DeleteResource( iai->g3d_res_mdl );
}

//======================================================================
//	居合い切り　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用居合い切り　追加
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_IAIGIRI_SetMMdl( MMDL *fmmdl, FLDEFF_CTRL *fectrl )
{
  u16 dir;
  VecFx32 pos;
  FLDEFF_IAIGIRI *iai;
  TASKHEADER_IAIGIRI head;
  
  // 1グリッド前方に表示
  MMDL_GetVectorPos( fmmdl, &pos );
  dir = MMDL_GetDirDisp( fmmdl );
  switch( dir )
  {
  case DIR_UP:    pos.z -= FIELD_CONST_GRID_FX32_SIZE;  break;
  case DIR_DOWN:  pos.z += FIELD_CONST_GRID_FX32_SIZE;  break;
  case DIR_LEFT:  pos.x -= FIELD_CONST_GRID_FX32_SIZE;  break;
  case DIR_RIGHT: pos.x += FIELD_CONST_GRID_FX32_SIZE;  break;
  }
  
  if( FLDEFF_CTRL_CheckRegistEffect( fectrl, FLDEFF_PROCID_IAIGIRI ) == FALSE )
  { // エフェクトを登録
    FLDEFF_PROCID id = FLDEFF_PROCID_IAIGIRI;
    FLDEFF_CTRL_RegistEffect( fectrl, &id, 1 );
  }

  iai = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_IAIGIRI );
  head.eff_iaigiri = iai;
  head.pos = pos; 
  
  FLDEFF_CTRL_AddTask(
      fectrl, &DATA_iaigiriTaskHeader, NULL, 0, &head, 0 );
}

//--------------------------------------------------------------
/**
 * 居合い切りタスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void iaigiriTask_Init( FLDEFF_TASK *task, void *wk )
{
  int i;
  TASKWORK_IAIGIRI *work = wk;
  const TASKHEADER_IAIGIRI *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->eff_iaigiri = head->eff_iaigiri;
  FLDEFF_TASK_SetPos( task, &head->pos );
  
  work->obj_rnd =
    GFL_G3D_RENDER_Create(
        work->eff_iaigiri->g3d_res_mdl, 0, work->eff_iaigiri->g3d_res_mdl );
  
  for( i=0; i<ANIME_NUM; i++ )
  {
    work->obj_anm[i] =
      GFL_G3D_ANIME_Create(
          work->obj_rnd, work->eff_iaigiri->g3d_res_anm[i], 0 );
  }
  
  work->obj = GFL_G3D_OBJECT_Create(
      work->obj_rnd, work->obj_anm, ANIME_NUM );
  for( i=0; i<ANIME_NUM; i++ )
  { 
    GFL_G3D_OBJECT_EnableAnime( work->obj, i );
  }
}

//--------------------------------------------------------------
/**
 * 居合い切りタスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void iaigiriTask_Delete( FLDEFF_TASK *task, void *wk )
{
  int i;
  TASKWORK_IAIGIRI *work = wk;

  for( i=0; i<ANIME_NUM; i++ )
  {
    GFL_G3D_ANIME_Delete( work->obj_anm[i] );
  }
  GFL_G3D_OBJECT_Delete( work->obj );
	GFL_G3D_RENDER_Delete( work->obj_rnd );
}

//--------------------------------------------------------------
/**
 * 居合い切りタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void iaigiriTask_Update( FLDEFF_TASK *task, void *wk )
{
  int i;
  BOOL end = TRUE;
  TASKWORK_IAIGIRI *work = wk;
  
  for( i=0; i<ANIME_NUM; i++ )
  {
    if( GFL_G3D_OBJECT_IncAnimeFrame(work->obj,i,FX32_ONE) == TRUE ){
      end = FALSE;
    } 
  }
  if( end ){
    FLDEFF_TASK_CallDelete( task );
  }
}

//--------------------------------------------------------------
/**
 * 居合い切りタスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void iaigiriTask_Draw( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_IAIGIRI *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};

  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
}

//--------------------------------------------------------------
//  居合い切りタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_iaigiriTaskHeader =
{
  sizeof(TASKWORK_IAIGIRI),
  iaigiriTask_Init,
  iaigiriTask_Delete,
  iaigiriTask_Update,
  iaigiriTask_Draw,
};

