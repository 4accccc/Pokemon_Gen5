//======================================================================
/**
 * @file	fldeff_btrain.c
 * @brief	フィールド バトルトレイン
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "field_effect.h"
#include "fldeff_btrain.h"

//======================================================================
//  define
//======================================================================
#define BTRAIN_SHAKE_FRAME (FX32_ONE*12)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_BTRAIN型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_BTRAIN FLDEFF_BTRAIN;

//--------------------------------------------------------------
///	FLDEFF_BTRAIN構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_BTRAIN
{
	FLDEFF_CTRL *fectrl;
  GFL_G3D_RES *g3d_res_mdl[FLDEFF_BTRAIN_TYPE_MAX];
  GFL_G3D_RES *g3d_res_anm[FLDEFF_BTRAIN_ANIME_TYPE_MAX];
  GFL_G3D_RND *g3d_rnd[FLDEFF_BTRAIN_TYPE_MAX];
};

//--------------------------------------------------------------
/// TASKHEADER_BTRAIN
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_BTRAIN *eff_btrain;
  FLDEFF_BTRAIN_TYPE type;
}TASKHEADER_BTRAIN;

//--------------------------------------------------------------
/// TASKWORK_BTRAIN
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  TASKHEADER_BTRAIN head;
  
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *anm[FLDEFF_BTRAIN_ANIME_TYPE_MAX];
}TASKWORK_BTRAIN;

//======================================================================
//	プロトタイプ
//======================================================================
static void btrain_InitResource( FLDEFF_BTRAIN *btrain );
static void btrain_DeleteResource( FLDEFF_BTRAIN *btrain );

static const FLDEFF_TASK_HEADER data_BTrainTaskHeader;

static const u16 data_ArcIdxBTrainMdl[FLDEFF_BTRAIN_TYPE_MAX];
static const u16 data_ArcIdxBTrainAnm[FLDEFF_BTRAIN_ANIME_TYPE_MAX];

//======================================================================
//	バトルトレイン　システム
//======================================================================
//--------------------------------------------------------------
/**
 * バトルトレイン 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_BTRAIN_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_BTRAIN *btrain;
	
	btrain = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_BTRAIN) );
	btrain->fectrl = fectrl;
  
	btrain_InitResource( btrain );
	return( btrain );
}

//--------------------------------------------------------------
/**
 * バトルトレイン 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_BTRAIN_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_BTRAIN *btrain = work;
  btrain_DeleteResource( btrain );
  GFL_HEAP_FreeMemory( btrain );
}

//======================================================================
//	バトルトレイン　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * バトルトレイン　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void btrain_InitResource( FLDEFF_BTRAIN *btrain )
{
  int i;
  const u16 *tbl;
  ARCHANDLE *handle;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( btrain->fectrl );
  
  i = 0;
  tbl = data_ArcIdxBTrainMdl;
  
  for( ; i < FLDEFF_BTRAIN_TYPE_MAX; i++, tbl++ ){
    btrain->g3d_res_mdl[i] = GFL_G3D_CreateResourceHandle( handle, *tbl );
    
    btrain->g3d_rnd[i] = GFL_G3D_RENDER_Create(
        btrain->g3d_res_mdl[i], 0, btrain->g3d_res_mdl[i] );
  }
  
  i = 0;
  tbl = data_ArcIdxBTrainAnm;

  for( ; i < FLDEFF_BTRAIN_ANIME_TYPE_MAX; i++, tbl++ ){
    btrain->g3d_res_anm[i] = GFL_G3D_CreateResourceHandle( handle, *tbl );
  }
}

//--------------------------------------------------------------
/**
 * バトルトレイン　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void btrain_DeleteResource( FLDEFF_BTRAIN *btrain )
{
  int i;
  
  for( i = 0; i < FLDEFF_BTRAIN_TYPE_MAX; i++ ){
    GFL_G3D_DeleteResource( btrain->g3d_res_mdl[i] );
	  GFL_G3D_RENDER_Delete( btrain->g3d_rnd[i] );
  }
  
  for( i = 0; i < FLDEFF_BTRAIN_ANIME_TYPE_MAX; i++ ){
    GFL_G3D_DeleteResource( btrain->g3d_res_anm[i] );
  }
}

//======================================================================
//	バトルトレイン　タスク
//======================================================================
//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 *
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_BTRAIN_SetTrain(
    FLDEFF_CTRL *fectrl, const VecFx32 *pos, FLDEFF_BTRAIN_TYPE type )
{
  int i;
  FLDEFF_TASK *task;
  TASKHEADER_BTRAIN head;
  
  head.eff_btrain = FLDEFF_CTRL_GetEffectWork(
      fectrl, FLDEFF_PROCID_BTRAIN );
  head.type = type;

  task = FLDEFF_CTRL_AddTask(
      fectrl, &data_BTrainTaskHeader, pos, 0, &head, 0 );
  return( task );
}

//--------------------------------------------------------------
/**
 * 
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void btrainTask_Init( FLDEFF_TASK *task, void *wk )
{
  int i;
  FLDEFF_BTRAIN_TYPE type;
  FLDEFF_BTRAIN *eff_btrain;
  TASKWORK_BTRAIN *work = wk;
  const TASKHEADER_BTRAIN *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  
  eff_btrain = work->head.eff_btrain;
  type = work->head.type;
  
  for( i = 0; i < FLDEFF_BTRAIN_ANIME_TYPE_MAX; i++ ){
    work->anm[i] = GFL_G3D_ANIME_Create(
      eff_btrain->g3d_rnd[type], eff_btrain->g3d_res_anm[i], 0 );
  }
  
  work->obj = GFL_G3D_OBJECT_Create(
      eff_btrain->g3d_rnd[type], work->anm, FLDEFF_BTRAIN_ANIME_TYPE_MAX );
  
//  GFL_G3D_OBJECT_EnableAnime( work->obj, 0 );
//  GFL_G3D_OBJECT_EnableAnime( work->obj, 1 );
}

static void btrainTask_Delete( FLDEFF_TASK *task, void *wk )
{
  int i;
  TASKWORK_BTRAIN *work = wk;
  
  GFL_G3D_OBJECT_Delete( work->obj );

  for( i = 0; i < FLDEFF_BTRAIN_ANIME_TYPE_MAX; i++ ){
    GFL_G3D_ANIME_Delete( work->anm[i] );
  }
}

static void btrainTask_Update( FLDEFF_TASK *task, void *wk )
{
}

static void btrainTask_Draw( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_BTRAIN *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};

  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
}

//--------------------------------------------------------------
//  バトルトレインタスク ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER data_BTrainTaskHeader =
{
  sizeof(TASKWORK_BTRAIN),
  btrainTask_Init,
  btrainTask_Delete,
  btrainTask_Update,
  btrainTask_Draw,
};

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
//  バトルトレイン　モデルアーカイブインデックス
//--------------------------------------------------------------
static const u16 data_ArcIdxBTrainMdl[FLDEFF_BTRAIN_TYPE_MAX] =
{
  NARC_fldeff_b_train01_nsbmd,
  NARC_fldeff_b_train02_nsbmd,
  NARC_fldeff_b_train03_nsbmd,
  NARC_fldeff_b_train04_nsbmd,
  NARC_fldeff_b_train05_nsbmd,
  NARC_fldeff_b_train06_nsbmd,
  NARC_fldeff_b_train07_nsbmd,
};

//--------------------------------------------------------------
//  バトルトレイン　モデルアーカイブインデックス
//--------------------------------------------------------------
static const u16 data_ArcIdxBTrainAnm[FLDEFF_BTRAIN_ANIME_TYPE_MAX] =
{
  NARC_fldeff_b_train_a1_nsbca,
  NARC_fldeff_b_train_a2_nsbca,
  NARC_fldeff_b_train_a3_nsbca,
  NARC_fldeff_b_train_a4_nsbca,
};
