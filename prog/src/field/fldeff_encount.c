//======================================================================
/**
 * @file	fldeff_encount.c
 * @brief	フィールド エフェクトエンカウント用エフェクト群
 * @author  Miyuki Iwasawa	
 * @date	09.11.18
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_encount.h"

//======================================================================
//  define
//======================================================================
#define GRASS_SHAKE_FRAME (FX32_ONE*12)
#define ENCOUNT_ANMRES_MAX  (4)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_ENCOUNT型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_ENCOUNT FLDEFF_ENCOUNT;

//--------------------------------------------------------------
/// リソース型定義 
//--------------------------------------------------------------
typedef struct _EFFRES_DATA{
  u8  anm_num;
  u8  season_f:4;
  u8  area_f:4;
  u16 mdl_id;
  u16 anm_tbl[ENCOUNT_ANMRES_MAX];
}EFFRES_DATA;

//--------------------------------------------------------------
///	FLDEFF_ENCOUNT構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_ENCOUNT
{
	FLDEFF_CTRL *fectrl;
  EFFENC_TYPE_ID type;
  GFL_G3D_RES *g3d_res_mdl;
  GFL_G3D_RES *g3d_res_anm[ENCOUNT_ANMRES_MAX];

  EFFRES_DATA data;
};

//--------------------------------------------------------------
/// TASKHEADER_ENCOUNT
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_ENCOUNT *eff_enc;
  MMDL *fmmdl;
  u16 gx,gz;
  EFFENC_TYPE_ID type;
  int init_gx;
  int init_gz;
  u16 obj_id;
  u16 zone_id;
  VecFx32 pos;
}TASKHEADER_ENCOUNT;

//--------------------------------------------------------------
/// TASKWORK_ENCOUNT
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  TASKHEADER_ENCOUNT head;
  MMDL_CHECKSAME_DATA samedata;
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *obj_anm[ENCOUNT_ANMRES_MAX];
  GFL_G3D_RND *obj_rnd;
}TASKWORK_ENCOUNT;


static EFFRES_DATA DATA_EffEncountRes[] = {
 {
   1,TRUE,TRUE,NARC_fldeff_short_grass_sp_nsbmd,
   {
     NARC_fldeff_short_grass_sp_nsbta,
     0,0,0
   },
 },
};

//======================================================================
//	プロトタイプ
//======================================================================
static FLDEFF_ENCOUNT* enc_CreateWork( FLDEFF_CTRL* fectrl, HEAPID heapID, EFFENC_TYPE_ID type );
static void enc_InitResource( FLDEFF_ENCOUNT* enc, EFFENC_TYPE_ID type );
static void enc_DeleteResource( FLDEFF_ENCOUNT* enc );

static const FLDEFF_TASK_HEADER DATA_encountTaskHeader;

//======================================================================
//	エフェクトエンカウント　エフェクト　システム
//======================================================================
//--------------------------------------------------------------
/**
 * エフェクトエンカウント　エフェクト 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_ENCOUNT_SGrassInit( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
  return enc_CreateWork( fectrl, heapID, EFFENC_TYPE_SGRASS );
}
void * FLDEFF_ENCOUNT_LGrassInit( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
  return enc_CreateWork( fectrl, heapID, EFFENC_TYPE_SGRASS );
}
void * FLDEFF_ENCOUNT_CaveInit( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
  return enc_CreateWork( fectrl, heapID, EFFENC_TYPE_SGRASS );
}
void * FLDEFF_ENCOUNT_WaterInit( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
  return enc_CreateWork( fectrl, heapID, EFFENC_TYPE_SGRASS );
}
void * FLDEFF_ENCOUNT_SeaInit( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
  return enc_CreateWork( fectrl, heapID, EFFENC_TYPE_SGRASS );
}
void * FLDEFF_ENCOUNT_BridgeInit( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
  return enc_CreateWork( fectrl, heapID, EFFENC_TYPE_SGRASS );
}

//--------------------------------------------------------------
/**
 * エフェクトエンカウント　エフェクト 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_ENCOUNT_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_ENCOUNT *enc = work;
  enc_DeleteResource( enc );
  GFL_HEAP_FreeMemory( enc );
}

//======================================================================
//	エフェクトエンカウント　エフェクト　リソース
//======================================================================

//--------------------------------------------------------------
/*
 * @brief   コモンワーク生成＆初期化
 * @param   fectrl FLDEFF_CTRL*
 * @param   heapID ヒープID
 * @param   type  リソース初期化タイプ
 * @retval  FLDEFF_ENCOUNT* 
 */
//--------------------------------------------------------------
static FLDEFF_ENCOUNT* enc_CreateWork( FLDEFF_CTRL* fectrl, HEAPID heapID, EFFENC_TYPE_ID type ) 
{
  FLDEFF_ENCOUNT* enc;
	enc = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_ENCOUNT) );
	enc->fectrl = fectrl;

  enc_InitResource( enc, type );
  return enc;
}

//--------------------------------------------------------------
/*
 * @brief  コモンリソース初期化
 * @param   enc FLDEFF_ENCOUNT*
 * @param   リソース初期化タイプ
 * @retval  nothing
 */
//--------------------------------------------------------------
static void enc_InitResource( FLDEFF_ENCOUNT* enc, EFFENC_TYPE_ID type ) 
{
  int i,ofs = 0;
  BOOL ret;
  ARCHANDLE *handle;

  enc->data = DATA_EffEncountRes[type];

  handle = FLDEFF_CTRL_GetArcHandleEffect( enc->fectrl );
 
  if( enc->data.season_f ){
    ofs = FLDEFF_CTRL_GetSeasonID( enc->fectrl );
    if( enc->data.area_f ){
      ofs += FLDEFF_CTRL_GetHasSeasonDiff( enc->fectrl )*4;
    }
  }
  enc->g3d_res_mdl = GFL_G3D_CreateResourceHandle( handle, enc->data.mdl_id+ofs );
  ret = GFL_G3D_TransVramTexture( enc->g3d_res_mdl );
  GF_ASSERT( ret );
  
  for(i = 0;i < enc->data.anm_num;i++){
    enc->g3d_res_anm[i]	=
      GFL_G3D_CreateResourceHandle( handle, enc->data.anm_tbl[i]+ofs );
  }
}

//--------------------------------------------------------------
/*
 * @brief  コモンリソース解放
 * @param   enc FLDEFF_ENCOUNT*
 * @retval  nothing
 */
//--------------------------------------------------------------
static void enc_DeleteResource( FLDEFF_ENCOUNT* enc ) 
{
  int i;

  for(i = 0;i < enc->data.anm_num;i++){
 	  GFL_G3D_DeleteResource( enc->g3d_res_anm[i] );
  }
 	GFL_G3D_DeleteResource( enc->g3d_res_mdl );
}

//======================================================================
//	エフェクトエンカウント　エフェクト　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * エフェクトエンカウント　エフェクト追加(レールマップ上で呼び出しても無視されます)
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
FLDEFF_TASK* FLDEFF_ENCOUNT_SetEffect( FLDEFF_CTRL *fectrl, u16 gx, u16 gz, fx32 height, EFFENC_TYPE_ID type )
{
  FLDEFF_ENCOUNT* enc;
  TASKHEADER_ENCOUNT head;
  VecFx32 pos;
  FIELDMAP_WORK* fieldMapWork = FLDEFF_CTRL_GetFieldMapWork( fectrl );

  if( FIELDMAP_GetBaseSystemType( fieldMapWork ) == FLDMAP_BASESYS_RAIL ){
     return NULL; //レールマップでは生成しない
  }

  enc = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_ENC_SGRASS );
  head.eff_enc = enc;
  head.gx = gx;
  head.gz = gz;
  head.type = type;

  head.pos.x = GRID_SIZE_FX32(gx)+GRID_HALF_FX32;
  head.pos.z = GRID_SIZE_FX32(gz)+GRID_HALF_FX32;
  head.pos.y = height;

  return FLDEFF_CTRL_AddTask(
      fectrl, &DATA_encountTaskHeader, NULL, TRUE, &head, 0 );
}

//--------------------------------------------------------------
/**
 * エフェクトエンカウント　エフェクトタスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void encountTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_ENCOUNT *work = wk;
  const TASKHEADER_ENCOUNT *head;
  FLDEFF_ENCOUNT* enc;
 
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  enc = work->head.eff_enc;

  FLDEFF_TASK_SetPos( task, &head->pos );
  
  work->obj_rnd =
    GFL_G3D_RENDER_Create( enc->g3d_res_mdl, 0, enc->g3d_res_mdl );
  
  {
    int i;

    for(i = 0;i < enc->data.anm_num;i++){
      work->obj_anm[i] =
      GFL_G3D_ANIME_Create( work->obj_rnd, enc->g3d_res_anm[i], 0 );
    }
  
    work->obj = GFL_G3D_OBJECT_Create( work->obj_rnd, work->obj_anm, enc->data.anm_num );

    for(i = 0;i < enc->data.anm_num;i++){
      GFL_G3D_OBJECT_EnableAnime( work->obj, i );
    }
  }
  
  if( FLDEFF_TASK_GetAddParam(task) == FALSE ){ //アニメ無し
    work->seq_no = 1;
  }
  
  FLDEFF_TASK_CallUpdate( task ); //即動作
}

//--------------------------------------------------------------
/**
 * エフェクトエンカウント　エフェクトタスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void encountTask_Delete( FLDEFF_TASK *task, void *wk )
{
  int i;
  TASKWORK_ENCOUNT *work = wk;
  FLDEFF_ENCOUNT* enc = work->head.eff_enc;
 
  for( i = 0;i < enc->data.anm_num; i++){
    GFL_G3D_ANIME_Delete( work->obj_anm[i] );
  }
  GFL_G3D_OBJECT_Delete( work->obj );
	GFL_G3D_RENDER_Delete( work->obj_rnd );
}

//--------------------------------------------------------------
/**
 * エフェクトエンカウント　エフェクトタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void encountTask_Update( FLDEFF_TASK *task, void *wk )
{
  int i;
  TASKWORK_ENCOUNT *work = wk;
  FLDEFF_ENCOUNT* enc = work->head.eff_enc;
  
  for( i = 0;i < enc->data.anm_num; i++){
    GFL_G3D_OBJECT_LoopAnimeFrame( work->obj, i, FX32_ONE );
  }
  /*
  if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_SELECT ){
    FLDEFF_TASK_CallDelete( task );
  }
  */
}

//--------------------------------------------------------------
/**
 * エフェクトエンカウント　エフェクトタスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void encountTask_Draw( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_ENCOUNT *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
  
  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
}

//--------------------------------------------------------------
//  エフェクトエンカウント　エフェクトタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_encountTaskHeader =
{
  sizeof(TASKWORK_ENCOUNT),
  encountTask_Init,
  encountTask_Delete,
  encountTask_Update,
  encountTask_Draw,
};
