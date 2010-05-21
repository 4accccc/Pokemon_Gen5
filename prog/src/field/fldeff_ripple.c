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
#define RIPPLE_DRAW_Y_OFFSET (FX32_ONE*-8)		///<水波紋描画オフセット
#define RIPPLE_DRAW_Z_OFFSET (FX32_ONE*-5)

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
  GFL_BBDACT_SYS *bbdact_sys;
  u16 res_idx_ripple;
};

//--------------------------------------------------------------
/// TASKHEADER_RIPPLE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_RIPPLE *eff_ripple;
  VecFx32 pos;
}TASKHEADER_RIPPLE;

//--------------------------------------------------------------
/// TASKWORK_RIPPLE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_RIPPLE *eff_ripple;
  u16 act_id;
}TASKWORK_RIPPLE;

//======================================================================
//	プロトタイプ
//======================================================================
static void ripple_InitResource( FLDEFF_RIPPLE *ripple );
static void ripple_DeleteResource( FLDEFF_RIPPLE *ripple );

static const FLDEFF_TASK_HEADER DATA_rippleTaskHeader;

static const GFL_BBDACT_ANM * data_BlActAnm_RippleTbl[1];

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
	FLDEFF_RIPPLE *ripple;
	FIELDMAP_WORK *fieldmap;
	
	ripple = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_RIPPLE) );
	ripple->fectrl = fectrl;
  
  fieldmap = FLDEFF_CTRL_GetFieldMapWork( ripple->fectrl );
  ripple->bbdact_sys = FIELDMAP_GetEffBbdActSys( fieldmap );

	ripple_InitResource( ripple );
	return( ripple );
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
	FLDEFF_RIPPLE *ripple = work;
  ripple_DeleteResource( ripple );
  GFL_HEAP_FreeMemory( ripple );
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
static void ripple_InitResource( FLDEFF_RIPPLE *ripple )
{
  ARCHANDLE *handle;
  GFL_BBDACT_G3DRESDATA data;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( ripple->fectrl );
  
  data.texFmt = GFL_BBD_TEXFMT_PAL4;
  data.texSiz = GFL_BBD_TEXSIZ_16x64;
  data.celSizX = 16;
  data.celSizY = 16;
  data.dataCut = GFL_BBDACT_RESTYPE_DATACUT;
  
  data.g3dres = GFL_G3D_CreateResourceHandle(
      handle, NARC_fldeff_ripple_nsbtx );
  FLDEFF_CTRL_SetGrayScaleG3DResource( ripple->fectrl, data.g3dres );
  ripple->res_idx_ripple = 
      GFL_BBDACT_AddResourceG3DResUnit( ripple->bbdact_sys, &data, 1 );
}

//--------------------------------------------------------------
/**
 * 水波紋　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void ripple_DeleteResource( FLDEFF_RIPPLE *ripple )
{
  GFL_BBDACT_RemoveResourceUnit(
      ripple->bbdact_sys, ripple->res_idx_ripple, 1 );
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
void FLDEFF_RIPPLE_Set( FLDEFF_CTRL *fectrl, s16 gx, s16 gz, fx32 y )
{
  VecFx32 pos;
  FLDEFF_RIPPLE *ripple;
  TASKHEADER_RIPPLE head;
  
  MMDL_TOOL_GetCenterGridPos( gx, gz, &pos );
  pos.y = y + RIPPLE_DRAW_Y_OFFSET;
  pos.z += RIPPLE_DRAW_Z_OFFSET;
  
  ripple = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_RIPPLE );
  head.eff_ripple = ripple;
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
  
  {
    GFL_BBDACT_ANMTBL tbl;
    GFL_BBDACT_ACTDATA actData;
    FLDEFF_RIPPLE *ripple = work->eff_ripple;
    GFL_BBDACT_SYS *bbdact_sys = ripple->bbdact_sys;
    
    actData.resID = ripple->res_idx_ripple;
    actData.sizX = FX16_ONE*1;
    actData.sizY = FX16_ONE*1;
    actData.alpha = 31;
    actData.drawEnable = TRUE;
    actData.lightMask = GFL_BBD_LIGHTMASK_0;
    actData.trans = head->pos;
    actData.func = NULL;
    actData.work = work;
    
    work->act_id = GFL_BBDACT_AddActEx(
        bbdact_sys, 0, &actData, 1, GFL_BBD_DRAWMODE_XZ_FLAT_BILLBORD );
    
    if( work->act_id == GFL_BBDACT_ACTUNIT_ID_INVALID ){
      FLDEFF_TASK_CallDelete( task );
      return;
    }
    
    tbl = (GFL_BBDACT_ANMTBL)data_BlActAnm_RippleTbl;
    GFL_BBDACT_SetAnimeTable( bbdact_sys, work->act_id,  tbl, 1 );
    GFL_BBDACT_SetAnimeIdxOn( bbdact_sys, work->act_id, 0 );
  }
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
  GFL_BBDACT_SYS *bbdact_sys = work->eff_ripple->bbdact_sys;
  
  if( work->act_id != GFL_BBDACT_ACTUNIT_ID_INVALID ){
    GFL_BBDACT_RemoveAct( bbdact_sys, work->act_id, 1 );
  }
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
  u16 comm;
  TASKWORK_RIPPLE *work = wk;
  FLDEFF_RIPPLE *ripple = work->eff_ripple;
  GFL_BBDACT_SYS *bbdact_sys = work->eff_ripple->bbdact_sys;
    
  if( GFL_BBDACT_GetAnimeLastCommand(bbdact_sys,work->act_id,&comm) ){
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
#if 0 //ビルボードアクターに移行
  VecFx32 pos;
  TASKWORK_RIPPLE *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};

  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
#endif
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

//======================================================================
//  data
//======================================================================
//ripple anime table
static const GFL_BBDACT_ANM data_BlActAnm_Ripple[] =
{
  {0,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {1,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {2,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {3,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {GFL_BBDACT_ANMCOM_END,0,0,0},
};

static const GFL_BBDACT_ANM * data_BlActAnm_RippleTbl[1] =
{
  data_BlActAnm_Ripple,
};
