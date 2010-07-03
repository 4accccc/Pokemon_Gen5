//======================================================================
/**
 * @file	fldeff_fishing.c
 * @brief	フィールド 釣りイベント用エフェクト
 * @author	iwasawa
 * @date	09.12.18
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_fishing.h"

//======================================================================
//	define
//======================================================================
typedef enum{
  ANIME_ITP,
  ANIME_NUM
} ANIME_INDEX;

static u16 dat_id[ANIME_NUM] =
{
  NARC_fldeff_fishing_lure_nsbtp,
};


//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_FISHING_LURE型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_FISHING_LURE FLDEFF_FISHING_LURE;

//--------------------------------------------------------------
///	FLDEFF_FISHING_LURE構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_FISHING_LURE
{
	FLDEFF_CTRL *fectrl;
  
  GFL_G3D_RES *g3d_res_mdl;
  GFL_G3D_RES *g3d_res_anm[ANIME_NUM];

  GFL_G3D_OBJ *g3d_obj;
  GFL_G3D_ANM *g3d_obj_anm[ANIME_NUM];
  GFL_G3D_RND *g3d_obj_rnd;
};

//--------------------------------------------------------------
/// TASKWORK_FISHING_LURE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_FISHING_LURE *eff_lure;
  u16 anm_id;
  u16 anm_change_req;
}TASKWORK_FISHING_LURE;

//--------------------------------------------------------------
/// TASKHEADER_FISHING_LURE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_FISHING_LURE *eff_lure;
  VecFx32 pos;
}TASKHEADER_FISHING_LURE;

//======================================================================
//	プロトタイプ
//======================================================================
static void fishing_lure_InitResource( FLDEFF_FISHING_LURE *wk );
static void fishing_lure_DeleteResource( FLDEFF_FISHING_LURE *wk );
static const FLDEFF_TASK_HEADER DATA_fishing_lure_TaskHeader;

//======================================================================
//	ルアー　システム
//======================================================================
//--------------------------------------------------------------
/**
 * ルアー 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_FISHING_LURE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_FISHING_LURE *wk;
	
	wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_FISHING_LURE) );
	wk->fectrl = fectrl;
  
	fishing_lure_InitResource( wk );
	return( wk );
}

//--------------------------------------------------------------
/**
 * ルアー 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_FISHING_LURE_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_FISHING_LURE *wk = work;
  fishing_lure_DeleteResource( wk );
  GFL_HEAP_FreeMemory( wk );
}

//======================================================================
//	ルアー　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * ルアー　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void fishing_lure_InitResource( FLDEFF_FISHING_LURE *wk )
{
  int i;
  ARCHANDLE *handle;
  BOOL ret;

  handle = FLDEFF_CTRL_GetArcHandleEffect( wk->fectrl );
  
  wk->g3d_res_mdl	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_fishing_lure_nsbmd );

  ret = GFL_G3D_TransVramTexture( wk->g3d_res_mdl );
  GF_ASSERT( ret );

  for( i=0; i<ANIME_NUM; i++ )
  {
    wk->g3d_res_anm[i] =
      GFL_G3D_CreateResourceHandle( handle, dat_id[i] );
  }

  wk->g3d_obj_rnd = GFL_G3D_RENDER_Create( wk->g3d_res_mdl, 0, wk->g3d_res_mdl );
  
  for( i=0; i<ANIME_NUM; i++ )
  {
    wk->g3d_obj_anm[i] =
      GFL_G3D_ANIME_Create( wk->g3d_obj_rnd, wk->g3d_res_anm[i], 0 );
  }
  
  wk->g3d_obj = GFL_G3D_OBJECT_Create( wk->g3d_obj_rnd, wk->g3d_obj_anm, ANIME_NUM );
  for( i=0; i<ANIME_NUM; i++ )
  { 
    GFL_G3D_OBJECT_EnableAnime( wk->g3d_obj, i );
  }

}

//--------------------------------------------------------------
/**
 * ルアー　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void fishing_lure_DeleteResource( FLDEFF_FISHING_LURE *wk )
{
  int i;

  GFL_G3D_OBJECT_Delete( wk->g3d_obj );
  for( i=0; i<ANIME_NUM; i++ )
  {
    GFL_G3D_ANIME_Delete( wk->g3d_obj_anm[i] );
  }
	GFL_G3D_RENDER_Delete( wk->g3d_obj_rnd );

  for( i=0; i<ANIME_NUM; i++ )
  {
    GFL_G3D_DeleteResource( wk->g3d_res_anm[i] );
  }
	GFL_G3D_FreeVramTexture( wk->g3d_res_mdl );
 	GFL_G3D_DeleteResource( wk->g3d_res_mdl );
}

//======================================================================
//	ルアー　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * ルアー　追加
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
FLDEFF_TASK* FLDEFF_FISHING_LURE_Set( FLDEFF_CTRL *fectrl, VecFx32* tpos, u8 dir, BOOL y_diff )
{
  VecFx32 pos;
  FLDEFF_FISHING_LURE *wk;
  TASKHEADER_FISHING_LURE head;
  const u8 ofs_tbl[][4] = {
   { 0, 0, 0, 0 },  //岸 up,down,left,right
   { 0, 0, 3, 3 },  //浅瀬
  };

  wk = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_FISHING_LURE );
  head.eff_lure = wk;
  head.pos = *tpos;

  head.pos.z += FX32_CONST( ofs_tbl[ y_diff ][dir] ); 
 
  return FLDEFF_CTRL_AddTask(
      fectrl, &DATA_fishing_lure_TaskHeader, NULL, 0, &head, 0 );
}

//--------------------------------------------------------------
/**
 * ルアー アニメーションステート変更
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_FISHING_LURE_ChangeAnime( FLDEFF_TASK* task, FISHING_LURE_ANM_ID anm_id )
{
  if( task != NULL ){
    TASKWORK_FISHING_LURE *work = (TASKWORK_FISHING_LURE*)FLDEFF_TASK_GetWork( task );

    work->anm_change_req = TRUE;
    work->anm_id = anm_id;
  }else{
    GF_ASSERT( 0 );
  }
}

//--------------------------------------------------------------
/**
 * ルアータスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fishingLureTask_Init( FLDEFF_TASK *task, void *wk )
{
  int i;
  TASKWORK_FISHING_LURE *work = wk;
  const TASKHEADER_FISHING_LURE *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->eff_lure = head->eff_lure;
  FLDEFF_TASK_SetPos( task, &head->pos );
}

//--------------------------------------------------------------
/**
 * ルアータスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fishingLureTask_Delete( FLDEFF_TASK *task, void *wk )
{
}

//--------------------------------------------------------------
/**
 * ルアータスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fishingLureTask_Update( FLDEFF_TASK *task, void *wk )
{
  int i;
  BOOL end = TRUE;
  TASKWORK_FISHING_LURE *work = wk;
  FLDEFF_FISHING_LURE *eff_lure = work->eff_lure;
  
  for( i=0; i<ANIME_NUM; i++ )
  {
    GFL_G3D_OBJECT_LoopAnimeFrame( eff_lure->g3d_obj, i, FX32_CONST( (work->anm_id*2)+1 ) );
  }
}

//--------------------------------------------------------------
/**
 * ルアータスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fishingLureTask_Draw( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_FISHING_LURE *work = wk;
  FLDEFF_FISHING_LURE *eff_lure = work->eff_lure;

  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};

  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( eff_lure->g3d_obj, &status );
}

//--------------------------------------------------------------
//  ルアータスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_fishing_lure_TaskHeader =
{
  sizeof(TASKWORK_FISHING_LURE),
  fishingLureTask_Init,
  fishingLureTask_Delete,
  fishingLureTask_Update,
  fishingLureTask_Draw,
};
