//======================================================================
/**
 * @file	fldeff_fes_kira.c
 * @brief	フィールド フェスミッション用 キラキラエフェクト
 * @author  Miyuki Iwasawa	
 * @date	09.11.18
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field/fieldmap_call.h"
#include "field_effect.h"
#include "fldmmdl.h"
#include "effect_encount.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "fldeff_fes_kira.h"
#include "field_encount_local.h"

//======================================================================
//  define
//======================================================================
#define FES_KIRA_OBJ_MAX      (8)
#define FES_KIRA_ANMRES_MAX   (4)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///	FES_KIRA型
//--------------------------------------------------------------
typedef struct _TAG_FES_KIRA FES_KIRA;

//--------------------------------------------------------------
/// リソース型定義 
//--------------------------------------------------------------
typedef struct _FE_FES_KIRA_RES{
  u8  anm_num;
  u16 mdl_id;
  u16 anm_tbl[FES_KIRA_ANMRES_MAX];
}FE_FES_KIRA_RES;

typedef struct _EFFECT_OBJ{
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *obj_anm[FES_KIRA_ANMRES_MAX];
  GFL_G3D_RND *obj_rnd;
}EFFECT_OBJ;

//--------------------------------------------------------------
///	FES_KIRA構造体
//--------------------------------------------------------------
struct _TAG_FES_KIRA
{
	FLDEFF_CTRL *fectrl;
  GFL_G3D_RES *g3d_res_mdl;
  GFL_G3D_RES *g3d_res_anm[FES_KIRA_ANMRES_MAX];
  EFFECT_OBJ  g3d_obj;

  FE_FES_KIRA_RES data;
};

//--------------------------------------------------------------
/// TASKHEADER_FES_KIRA
//--------------------------------------------------------------
typedef struct
{
  FES_KIRA *eff_wk;
  MMDLSYS* mmdl_sys;
  MMDL *fmmdl;

}TASKHEADER_FES_KIRA;

//--------------------------------------------------------------
/// FES_KIRA_OBJ
//--------------------------------------------------------------
typedef struct{
  VecFx32 pos;
  int     visible_f;
}FES_KIRA_OBJ;

//--------------------------------------------------------------
/// TASKWK_FES_KIRA
//--------------------------------------------------------------
typedef struct
{
  TASKHEADER_FES_KIRA head;

  FES_KIRA_OBJ  obj[FES_KIRA_OBJ_MAX];
#if 0
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *obj_anm[FES_KIRA_ANMRES_MAX];
  GFL_G3D_RND *obj_rnd;
#endif

}TASKWK_FES_KIRA;

/*
 * エフェクトリソース定義データ
*/
static FE_FES_KIRA_RES DATA_FesKiraRes = {
 1,
 NARC_fldeff_fes_kira_nsbmd,
 {
   NARC_fldeff_fes_kira_nsbta,
   0,0,0
 },
};

//======================================================================
//	プロトタイプ
//======================================================================
static FES_KIRA* eff_CreateWork( FLDEFF_CTRL* fectrl, HEAPID heapID );
static void eff_InitResource( FES_KIRA* eff_wk );
static void eff_InitResourceCore( FES_KIRA* eff_wk, FE_FES_KIRA_RES* data, ARCHANDLE* handle );
static void eff_DeleteResource( FES_KIRA* eff_wk );

static const FLDEFF_TASK_HEADER DATA_fldeffFesKiraTaskHeader;

//======================================================================
//	フェスミッションエフェクト　エフェクト　システム
//======================================================================
//--------------------------------------------------------------
/**
 * フェスミッションエフェクト　エフェクト 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_FES_KIRA_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
  return eff_CreateWork( fectrl, heapID );
}

//--------------------------------------------------------------
/**
 * フェスミッションエフェクト　エフェクト 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_FES_KIRA_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FES_KIRA *eff_wk = work;
  eff_DeleteResource( eff_wk );
  GFL_HEAP_FreeMemory( eff_wk );
}

//======================================================================
//	フェスミッションエフェクト　エフェクト　リソース
//======================================================================

//--------------------------------------------------------------
/*
 * @brief   コモンワーク生成＆初期化
 * @param   fectrl FLDEFF_CTRL*
 * @param   heapID ヒープID
 * @param   eff_id  リソース初期化タイプID
 * @retval  FES_KIRA* 
 */
//--------------------------------------------------------------
static FES_KIRA* eff_CreateWork( FLDEFF_CTRL* fectrl, HEAPID heapID ) 
{
  FES_KIRA* eff_wk;
	eff_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FES_KIRA) );
	eff_wk->fectrl = fectrl;

  eff_InitResource( eff_wk );
  return eff_wk;
}

//--------------------------------------------------------------
/*
 * @brief  コモンリソース初期化
 * @param   eff_wk FES_KIRA*
 * @param   リソース初期化タイプ
 * @retval  nothing
 */
//--------------------------------------------------------------
static void eff_InitResource( FES_KIRA* eff_wk ) 
{
  ARCHANDLE *handle;

  eff_wk->data = DATA_FesKiraRes;

  handle = FLDEFF_CTRL_GetArcHandleEffect( eff_wk->fectrl );

  eff_InitResourceCore( eff_wk, &eff_wk->data, handle );
}

static void eff_CreateObject( EFFECT_OBJ* obj, FES_KIRA* eff_wk )
{
  int i;

  obj->obj_rnd =
    GFL_G3D_RENDER_Create( eff_wk->g3d_res_mdl, 0, eff_wk->g3d_res_mdl );

  for(i = 0;i < eff_wk->data.anm_num;i++){
    obj->obj_anm[i] =
    GFL_G3D_ANIME_Create( obj->obj_rnd, eff_wk->g3d_res_anm[i], 0 );
  }
  
  obj->obj = GFL_G3D_OBJECT_Create( obj->obj_rnd, obj->obj_anm, eff_wk->data.anm_num );

  for(i = 0;i < eff_wk->data.anm_num;i++){
    GFL_G3D_OBJECT_EnableAnime( obj->obj, i );
  }
}

static void eff_DeleteObject( EFFECT_OBJ* obj, FES_KIRA* eff_wk )
{
  int i;

  for( i = 0;i < eff_wk->data.anm_num; i++){
    GFL_G3D_ANIME_Delete( obj->obj_anm[i] );
  }
  GFL_G3D_OBJECT_Delete( obj->obj );
	GFL_G3D_RENDER_Delete( obj->obj_rnd );
}

/*
 *  @brief  リソース初期化
 */
static void eff_InitResourceCore( FES_KIRA* eff_wk, FE_FES_KIRA_RES* data, ARCHANDLE* handle )
{
  int i = 0;
  BOOL ret;
  
  eff_wk->g3d_res_mdl = GFL_G3D_CreateResourceHandle( handle, data->mdl_id );
  ret = GFL_G3D_TransVramTexture( eff_wk->g3d_res_mdl );
  GF_ASSERT( ret );
    
  for(i = 0;i < data->anm_num;i++){
    eff_wk->g3d_res_anm[i]	=
      GFL_G3D_CreateResourceHandle( handle, data->anm_tbl[i] );
  }
  eff_CreateObject( &eff_wk->g3d_obj, eff_wk );
}

//--------------------------------------------------------------
/*
 * @brief  コモンリソース解放
 * @param   eff_wk FES_KIRA*
 * @retval  nothing
 */
//--------------------------------------------------------------
static void eff_DeleteResource( FES_KIRA* eff_wk ) 
{
  int i;
  
  eff_DeleteObject( &eff_wk->g3d_obj, eff_wk );

  for(i = 0;i < eff_wk->data.anm_num; i++){
 	  GFL_G3D_DeleteResource( eff_wk->g3d_res_anm[i] );
  }
  GFL_G3D_FreeVramTexture( eff_wk->g3d_res_mdl );
  GFL_G3D_DeleteResource( eff_wk->g3d_res_mdl );
}

//======================================================================
//	フェスミッションエフェクト　エフェクト　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * フェスミッションエフェクト　エフェクトタスク追加
 * @param FLDEFF_CTRL*
 * @retval nothing
 *
 * これを呼んだだけだと、何も描画されないので注意
 */
//--------------------------------------------------------------
FLDEFF_TASK* FLDEFF_FES_KIRA_SetEffect( FLDEFF_CTRL *fectrl )
{
  FES_KIRA* eff_wk;
  TASKHEADER_FES_KIRA head;
  FIELDMAP_WORK* fieldMapWork = FLDEFF_CTRL_GetFieldMapWork( fectrl );

  eff_wk = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_FES_KIRA );

  if( eff_wk == NULL ){
    return NULL;
  }

  MI_CpuClear8( &head, sizeof(TASKHEADER_FES_KIRA));
  head.eff_wk = eff_wk;
  head.mmdl_sys = FIELDMAP_GetMMdlSys( fieldMapWork );

  return FLDEFF_CTRL_AddTask(
      fectrl, &DATA_fldeffFesKiraTaskHeader, NULL, 0, &head, 0 );
}

//--------------------------------------------------------------
/**
 * フェスミッションエフェクト 指定のオブジェクト表示追加　
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_FES_KIRA_ObjAdd( FLDEFF_TASK* task, u8 id, VecFx32* pos )
{
  TASKWK_FES_KIRA *work = (TASKWK_FES_KIRA*)FLDEFF_TASK_GetWork( task );
  
  if( id >= FES_KIRA_OBJ_MAX ){
    return; 
  }
  work->obj[id].visible_f = TRUE; //描画リストに入れる
  work->obj[id].pos = *pos;
}

//--------------------------------------------------------------
/**
 * フェスミッションエフェクト 指定のオブジェクト表示削除　
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_FES_KIRA_ObjDel( FLDEFF_TASK* task, u8 id )
{
  TASKWK_FES_KIRA *work = (TASKWK_FES_KIRA*)FLDEFF_TASK_GetWork( task );
  
  if( id >= FES_KIRA_OBJ_MAX ){
    return; 
  }
  work->obj[id].visible_f = FALSE; //描画からはずす
}

//--------------------------------------------------------------
/**
 * フェスミッションエフェクト 全オブジェクト表示削除　
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_FES_KIRA_ObjDelAll( FLDEFF_TASK* task )
{
  int i;
  TASKWK_FES_KIRA *work = (TASKWK_FES_KIRA*)FLDEFF_TASK_GetWork( task );
  
  for( i = 0;i < FES_KIRA_OBJ_MAX;i++){
    work->obj[i].visible_f = FALSE; //描画からはずす
  }
}

//--------------------------------------------------------------
/**
 * フェスミッションエフェクト　エフェクトタスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldeffFesKiraTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWK_FES_KIRA *work = wk;
  const TASKHEADER_FES_KIRA *head;
 
  head = FLDEFF_TASK_GetAddPointer( task );
//  work->eff_wk = head->eff_wk;
  work->head = *head;

//  FLDEFF_TASK_SetPos( task, &head->pos );
 
  FLDEFF_TASK_CallUpdate( task ); //即動作
}

//--------------------------------------------------------------
/**
 * フェスミッションエフェクト　エフェクトタスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldeffFesKiraTask_Delete( FLDEFF_TASK *task, void *wk )
{
}

//--------------------------------------------------------------
/**
 * フェスミッションエフェクト　エフェクトタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldeffFesKiraTask_Update( FLDEFF_TASK *task, void *wk )
{
  int i;
  TASKWK_FES_KIRA *work = wk;
  FES_KIRA* eff_wk = work->head.eff_wk;
  
  for( i = 0;i < eff_wk->data.anm_num; i++){
    GFL_G3D_OBJECT_LoopAnimeFrame( eff_wk->g3d_obj.obj, i, FX32_ONE );
  }
}

//--------------------------------------------------------------
/**
 * フェスミッションエフェクト　エフェクトタスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldeffFesKiraTask_Draw( FLDEFF_TASK *task, void *wk )
{
  int i;
  VecFx32 pos;
  TASKWK_FES_KIRA *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
  FES_KIRA* eff_wk = work->head.eff_wk;
  
  MTX_Identity33( &status.rotate );
  
  for( i = 0; i < FES_KIRA_OBJ_MAX;i++ ){
    if( !work->obj[i].visible_f ){
      continue;
    }
    status.trans = work->obj[i].pos;
    GFL_G3D_DRAW_DrawObjectCullingON( eff_wk->g3d_obj.obj, &status );
  }
}

//--------------------------------------------------------------
//  フェスミッションエフェクト　エフェクトタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_fldeffFesKiraTaskHeader =
{
  sizeof(TASKWK_FES_KIRA),
  fldeffFesKiraTask_Init,
  fldeffFesKiraTask_Delete,
  fldeffFesKiraTask_Update,
  fldeffFesKiraTask_Draw,
};
