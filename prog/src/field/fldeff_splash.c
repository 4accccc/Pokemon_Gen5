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
  GFL_BBDACT_SYS *bbdact_sys;
  
  u16 res_idx_splash;
  u16 res_idx_shoal;
};

//--------------------------------------------------------------
/// TASKHEADER_SPLASH
//--------------------------------------------------------------
typedef struct
{
  BOOL joint;
  FLDEFF_SPLASH *eff_splash;
  const MMDL *mmdl;
}TASKHEADER_SPLASH;

//--------------------------------------------------------------
/// TASKWORK_SPLASH
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  TASKHEADER_SPLASH head;
  MMDL_CHECKSAME_DATA samedata;
  u16 act_id;
}TASKWORK_SPLASH;

//======================================================================
//	プロトタイプ
//======================================================================
static void splash_InitResource( FLDEFF_SPLASH *splash );
static void splash_DeleteResource( FLDEFF_SPLASH *splash );

static const FLDEFF_TASK_HEADER DATA_splashTaskHeader;

static const GFL_BBDACT_ANM * data_BlActAnm_SplashTbl[1];
static const GFL_BBDACT_ANM * data_BlActAnm_ShoalTbl[1];

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
  data.texSiz = GFL_BBD_TEXSIZ_32x128;
  data.celSizX = 32;
  data.celSizY = 32;
  data.dataCut = GFL_BBDACT_RESTYPE_DATACUT;
  
  data.g3dres = GFL_G3D_CreateResourceHandle(
      handle, NARC_fldeff_shibuki_nsbtx );
  splash->res_idx_splash = 
      GFL_BBDACT_AddResourceG3DResUnit( splash->bbdact_sys, &data, 1 );
  
  data.texFmt = GFL_BBD_TEXFMT_PAL4;
  data.texSiz = GFL_BBD_TEXSIZ_16x64;
  data.celSizX = 16;
  data.celSizY = 16;
  data.dataCut = GFL_BBDACT_RESTYPE_DATACUT;
  
  data.g3dres = GFL_G3D_CreateResourceHandle(
      handle, NARC_fldeff_shibuki02_nsbtx );
  splash->res_idx_shoal = 
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
  GFL_BBDACT_RemoveResourceUnit(
      splash->bbdact_sys, splash->res_idx_shoal, 1 );
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
void FLDEFF_SPLASH_SetMMdl(
    FLDEFF_CTRL *fectrl, const MMDL *mmdl, BOOL joint )
{
  TASKHEADER_SPLASH head;
  
  head.joint = joint;
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
  GFL_BBDACT_ACTDATA actData;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  
  if( work->head.joint == TRUE ){
    MMDL_InitCheckSameData( work->head.mmdl, &work->samedata );
  }
  
  MMDL_GetVectorPos( work->head.mmdl, &pos );
  pos.z += SPLASH_DRAW_Z_OFFSET;
  FLDEFF_TASK_SetPos( task, &pos );
  
  {
    FLDEFF_SPLASH *splash = work->head.eff_splash;
    GFL_BBDACT_SYS *bbdact_sys = splash->bbdact_sys;
    
    actData.resID = splash->res_idx_splash;
    
    if( work->head.joint == TRUE ){
      actData.resID = splash->res_idx_shoal;
    }
    
    actData.sizX = FX16_ONE*8-1;
    actData.sizY = FX16_ONE*8-1;
    actData.alpha = 31;
    actData.drawEnable = TRUE;
    actData.lightMask = GFL_BBD_LIGHTMASK_0;
    actData.trans = pos;
    actData.func = NULL;
    actData.work = work;
    
    work->act_id = GFL_BBDACT_AddAct( bbdact_sys, 0, &actData, 1 );
    
    {
      GFL_BBDACT_ANMTBL tbl = (GFL_BBDACT_ANMTBL)data_BlActAnm_SplashTbl;
      
      if( work->head.joint == TRUE ){
        tbl = (GFL_BBDACT_ANMTBL)data_BlActAnm_ShoalTbl;
      }
      
      GFL_BBDACT_SetAnimeTable( bbdact_sys, work->act_id,  tbl, 1 );
      GFL_BBDACT_SetAnimeIdxOn( bbdact_sys, work->act_id, 0 );
    }
  }
  
//即反映すると親がjointフラグがセットされていない状態。
//  FLDEFF_TASK_CallUpdate( task );
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
  GFL_BBDACT_RemoveAct( bbdact_sys, work->act_id, 1 );
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
  
  if( work->head.joint == TRUE ){
    if( MMDL_CheckSameData(work->head.mmdl,&work->samedata) == FALSE ){
      FLDEFF_TASK_CallDelete( task );
      return;
    }
  }
  
  if( work->head.joint == TRUE ){
    if( MMDL_CheckMoveBitShoalEffect(work->head.mmdl) == FALSE ){
      FLDEFF_TASK_CallDelete( task );
      return;
    }
  }

  {
    u16 comm;
    FLDEFF_SPLASH *splash = work->head.eff_splash;
    GFL_BBDACT_SYS *bbdact_sys = work->head.eff_splash->bbdact_sys;
    
    if( GFL_BBDACT_GetAnimeLastCommand(bbdact_sys,work->act_id,&comm) ){
      if( work->head.joint == FALSE ){
        FLDEFF_TASK_CallDelete( task );
        return;
      }

      GFL_BBDACT_SetAnimeFrmIdx( bbdact_sys, work->act_id, 0 );
    }
    
    if( work->head.joint == TRUE ){
      VecFx32 pos;
      
      MMDL_GetVectorPos( work->head.mmdl, &pos );
      pos.z += SPLASH_DRAW_Z_OFFSET;
      FLDEFF_TASK_SetPos( task, &pos );
      
      GFL_BBD_SetObjectTrans(
        GFL_BBDACT_GetBBDSystem(bbdact_sys), work->act_id, &pos );
    }
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
//  data
//======================================================================
//splash anime table
static const GFL_BBDACT_ANM data_BlActAnm_Splash[] =
{
  {0,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,4},
  {1,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,4},
  {2,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,4},
  {GFL_BBDACT_ANMCOM_END,0,0,0},
};

static const GFL_BBDACT_ANM * data_BlActAnm_SplashTbl[1] =
{
  data_BlActAnm_Splash,
};

//shoal anime table
static const GFL_BBDACT_ANM data_BlActAnm_Shoal[] =
{
  {0,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {1,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {2,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {3,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {GFL_BBDACT_ANMCOM_END,0,0,0},
};

static const GFL_BBDACT_ANM * data_BlActAnm_ShoalTbl[1] =
{
  data_BlActAnm_Shoal,
};
