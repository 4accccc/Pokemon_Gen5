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

#ifdef PM_DEBUG
 #include "field/eventdata_sxy.h"
 #include "field/eventdata_type.h"
 #include "field/eventdata_local.h"
 #include "debug/debug_flg.h"
#endif

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

#ifdef PM_DEBUG
  int deb_pos_num;
  const BG_TALK_DATA *deb_pos;
  const FIELD_RAIL_MAN* deb_railman;
#endif  //PM_DEBUG
}TASKHEADER_FES_KIRA;

//--------------------------------------------------------------
/// FES_KIRA_OBJ
//--------------------------------------------------------------
typedef struct{
  VecFx32 pos;
  u16     visible_f;
  u16     item;
}FES_KIRA_OBJ;

//--------------------------------------------------------------
/// TASKWK_FES_KIRA
//--------------------------------------------------------------
typedef struct
{
  TASKHEADER_FES_KIRA head;

  FES_KIRA_OBJ  obj[FES_KIRA_OBJ_MAX];
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
 * これを呼んだだけだと、コントローラタスクの追加のみで何も描画されないので注意
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

#ifdef PM_DEBUG
  {
    GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( fieldMapWork );
    GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
    EVENTDATA_SYSTEM* evdata = GAMEDATA_GetEventData( gdata );
    const FLDNOGRID_MAPPER* cp_mapper = FIELDMAP_GetFldNoGridMapper( fieldMapWork );

    head.deb_pos = EVENTDATA_GetTalkBGEventTbl( evdata, &head.deb_pos_num ); 
    if( cp_mapper != NULL ){
      head.deb_railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );
    }
  }
#endif  //PM_DEBUG

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
void FLDEFF_FES_KIRA_ObjAdd( FLDEFF_TASK* task, u8 id, u16 item, VecFx32* pos )
{
  TASKWK_FES_KIRA *work = (TASKWK_FES_KIRA*)FLDEFF_TASK_GetWork( task );
  
  if( id >= FES_KIRA_OBJ_MAX ){
    return; 
  }
  work->obj[id].visible_f = TRUE; //描画リストに入れる
  work->obj[id].pos = *pos;
  work->obj[id].item = item;
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
 * フェスミッションエフェクト 指定オブジェクトが表示状態かチェック
 * @param FLDEFF_CTRL*
 * @param id  指定ID
 * @retval TRUE  描画されている 
 * @retval FALSE 描画されていない
 */
//--------------------------------------------------------------
BOOL FLDEFF_FES_KIRA_ObjIsVisible( FLDEFF_TASK* task, u8 id )
{
  TASKWK_FES_KIRA *work = (TASKWK_FES_KIRA*)FLDEFF_TASK_GetWork( task );
  if( id >= FES_KIRA_OBJ_MAX ){
    GF_ASSERT(0);
    return FALSE;
  } 
  return work->obj[id].visible_f;
}

//--------------------------------------------------------------
/**
 * フェスミッションエフェクト 指定オブジェクトにセットされたアイテムNoを取得
 * @param FLDEFF_CTRL*
 * @param id  指定ID
 * @retval アイテムNo
 * @retval FALSE 描画されていない
 */
//--------------------------------------------------------------
u16 FLDEFF_FES_KIRA_ObjItemGet( FLDEFF_TASK* task, u8 id )
{
  TASKWK_FES_KIRA *work = (TASKWK_FES_KIRA*)FLDEFF_TASK_GetWork( task );
  if( id >= FES_KIRA_OBJ_MAX ){
    GF_ASSERT(0);
    return 0;
  } 
  return work->obj[id].item;
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

#ifdef PM_DEBUG
  if( DEBUG_FLG_GetFlg(DEBUG_FLG_FMissionEvPosView))
  {
    int d_num = 0;
    const BG_TALK_DATA *target = work->head.deb_pos;

    if( target == NULL ) return;

    for(i = 0;i < work->head.deb_pos_num;i++, target++ ){
      if( target->type != BG_TALK_TYPE_DUMMY_FES ){
        continue;
      }
      if( d_num++ >= 20 ){
        break;  //上限超えたのでこれ以上サーチしない
      }
      if( target->pos_type == EVENTDATA_POSTYPE_GRID ){
        EVENTDATA_GetBGTalkCenterPos( target, &status.trans);
      }
      else{
        RAIL_LOCATION loc;
        EVENTDATA_GetBGTalkCenterRailLocation( target, &loc );
        FIELD_RAIL_MAN_GetLocationPosition( work->head.deb_railman, &loc, &status.trans );
      }
      GFL_G3D_DRAW_DrawObjectCullingON( eff_wk->g3d_obj.obj, &status );
    }
  }
#endif
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
