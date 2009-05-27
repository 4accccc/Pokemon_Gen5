//======================================================================
/**
 * @file	fldeff_shadow.c
 * @brief	フィールドOBJ影
 * @author	kagaya
 * @data	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"
#include "fldeff_shadow.h"

//#include "..\..\include\system\timezone.h"
//#include "..\..\include\system\pm_rtc.h"

//======================================================================
//	define
//======================================================================
#define DEBUG_SHADOW_PL_NON
//#define DEBUG_SHADOW_WRITE_OFF	//定義で影描画OFF
//#define SHADOW_DRAW_Z_OFFSET (FX32_ONE*(3))
//#define SHADOW_DRAW_Z_OFFSET (FX32_ONE*(5))			///<影描画オフセットZ軸
#define SHADOW_DRAW_Z_OFFSET (FX32_ONE*(3))				

#define SHADOW_SCALE_SPEED (0x0010)
#define SHADOW_ALPHA_SPEED (0x0200)

/*
#define	TIMEZONE_MORNING	(0)
#define	TIMEZONE_NOON		(1)
#define	TIMEZONE_EVENING	(2)
#define TIMEZONE_NIGHT		(3)
#define TIMEZONE_MIDNIGHT	(4)
*/

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FE_SHADOW型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_SHADOW FLDEFF_SHADOW;

//--------------------------------------------------------------
///	FE_SHADOW構造体
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_SHADOW
{
	int time_seq_no;
	int time_zone;
	int next_time_zone;
	int frame;
	fx32 alpha;
	VecFx32 scale;
	FLDEFF_CTRL *fectrl;
  
  GFL_G3D_RES *g3d_res;
  GFL_G3D_RND *g3d_rnd;
  GFL_G3D_OBJ *g3d_obj;

#ifndef DEBUG_SHADOW_PL_NON
	TCB_PTR tcb_time_proc;
	FRO_MDL rmdl[SHADOW_MAX];
	FRO_OBJ robj[SHADOW_MAX];
#endif
}FE_SHADOW;

#define FE_SHADOW_SIZE (sizeof(FE_SHADOW)) ///<FE_SHADOWサイズ

//--------------------------------------------------------------
///	SHADOW_ADD_H構造体
//--------------------------------------------------------------
typedef struct
{
	FLDEFF_CTRL *fectrl;								///<FLDEFF_CTRL *
	FLDEFF_SHADOW *shadow;						///<FLDEFF_SHADOW
	FLDMMDL * fmmdl;						///<影の対象FLDMMDL *
}SHADOW_ADD_H;

#define SHADOW_ADD_H_SIZE (sizeof(SHADOW_ADD_H)) ///<SHADOW_ADD_Hサイズ

//--------------------------------------------------------------
///	SHADOW_WORK構造体
//--------------------------------------------------------------
typedef struct
{
	int obj_code;								///<影対象OBJコード
	int obj_id;									///<影対象OBJID
	int zone_id;								///<影対象ゾーンID
	int vanish_sw;								///<非表示SW
	int type;									///<SHADOW_BLACK等
	SHADOW_ADD_H head;							///<追加時のSHADOW_ADD_H
}SHADOW_WORK;

//======================================================================
//	プロトタイプ
//======================================================================
#ifndef DEBUG_SHADOW_PL_NON
static void Shadow_TimeProcAdd( FLDEFF_SHADOW *sd );
static void Shadow_TimeProcDelete( FLDEFF_SHADOW *sd );
static void Shadow_TimeScaleGet( FLDEFF_SHADOW *sd, VecFx32 *scale );
static int Shadow_TimeAlpha031( int alpha );
static void Shadow_TimeAlphaScaleSet( FLDEFF_SHADOW *sd, int zone );
static void Shadow_ValueAdd( fx32 *val0, fx32 val1, fx32 add );
static void Shadow_TimeProc( TCB_PTR tcb, void *wk );

static void Shadow_GraphicInit( FLDEFF_SHADOW *sd );
static void Shadow_GraphicDelete( FLDEFF_SHADOW *sd );
static void Shadow_GraphicAlphaSet( FLDEFF_SHADOW *sd, int alpha );

static const EOA_H_NPP DATA_EoaH_Shadow;
static const EOA_H_NPP DATA_EoaH_ShadowType;

const VecFx32 DATA_ShadowTimeScaleTbl[];
const fx32 DATA_ShadowTimeAlphaTbl[];
static const DATA_ShadowArcIdx[SHADOW_MAX];
#endif

static void shadow_InitResource( FLDEFF_SHADOW *sd );
static void shadow_DeleteResource( FLDEFF_SHADOW *sd );

//======================================================================
//	影　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 影初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_SHADOW_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_SHADOW *sd;
	
	sd = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_SHADOW) );
	sd->fectrl = fectrl;
  
	shadow_InitResource( sd );
	return( sd );
}

//--------------------------------------------------------------
/**
 * 影削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_SHADOW_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_SHADOW *sd = work;
  shadow_DeleteResource( sd );
  GFL_HEAP_FreeMemory( sd );
}

//======================================================================
//	影　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 影　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void shadow_InitResource( FLDEFF_SHADOW *sd )
{
  ARCHANDLE *handle;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( sd->fectrl );
  
  sd->g3d_res	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_kage_nsbmd );
  GFL_G3D_TransVramTexture( sd->g3d_res );
  
  sd->g3d_rnd =
    GFL_G3D_RENDER_Create( sd->g3d_res, 0, sd->g3d_res );
  sd->g3d_obj =
    GFL_G3D_OBJECT_Create( sd->g3d_rnd, NULL, 0 );
}

//--------------------------------------------------------------
/**
 * 影　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void shadow_DeleteResource( FLDEFF_SHADOW *sd )
{
  GFL_G3D_OBJECT_Delete( sd->g3d_obj );
	GFL_G3D_RENDER_Delete( sd->g3d_rnd );
 	GFL_G3D_DeleteResource( sd->g3d_res );
}

//======================================================================
//	影　タスク
//======================================================================
//--------------------------------------------------------------
/// SHADOW_TASKHEADER
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_SHADOW *eff_shadow;
  FLDMMDL *fmmdl;
}SHADOW_TASKHEADER;

//--------------------------------------------------------------
/// SHADOW_TASKWORK
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_SHADOW *eff_shadow;
  FLDMMDL *fmmdl;
  u16 obj_id;
  u16 zone_id;
}TASKWORK_SHADOW;

static const FLDEFF_TASK_HEADER DATA_shadowTaskHeader;

//--------------------------------------------------------------
/**
 * 動作モデル用影　追加
 * @param fmmdl FLDMMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_SHADOW_SetFldMMdl( FLDMMDL *fmmdl, FLDEFF_CTRL *fectrl )
{
  FLDEFF_SHADOW *sd;
  SHADOW_TASKHEADER head;
  
  sd = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_SHADOW );
  head.eff_shadow = sd;
  head.fmmdl = fmmdl;
  
  FLDEFF_CTRL_AddTask(
      fectrl, &DATA_shadowTaskHeader, NULL, 0, &head, 0 );
}

//--------------------------------------------------------------
/**
 * 影タスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void shadowTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_SHADOW *work = wk;
  const SHADOW_TASKHEADER *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->eff_shadow = head->eff_shadow;
  work->fmmdl = head->fmmdl;
  work->obj_id = FLDMMDL_GetOBJID( work->fmmdl );
  work->zone_id = FLDMMDL_GetZoneID( work->fmmdl );
}

//--------------------------------------------------------------
/**
 * 影タスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void shadowTask_Delete( FLDEFF_TASK *task, void *wk )
{
// TASKWORK_SHADOW *work = wk;
}

//--------------------------------------------------------------
/**
 * 影タスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void shadowTask_Update( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_SHADOW *work = wk;
  
  if( FLDMMDL_CheckSameID(work->fmmdl,work->obj_id,work->zone_id) == FALSE ){
    FLDEFF_TASK_CallDelete( task );
    return;
  }
  
  FLDMMDL_GetVectorPos( work->fmmdl, &pos );
//  pos.x += NUM_FX32(-1)/2;
  pos.x += NUM_FX32(1) / 4;
  pos.y += NUM_FX32(-2);
  pos.z += NUM_FX32(6)+0x800;
  FLDEFF_TASK_SetPos( task, &pos );
}

//--------------------------------------------------------------
/**
 * 影タスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void shadowTask_Draw( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_SHADOW *work = wk;
  GFL_G3D_OBJ *obj = work->eff_shadow->g3d_obj;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,0xc00},{0}};
  
  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( obj, &status );
}

//--------------------------------------------------------------
//  影タスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_shadowTaskHeader =
{
  sizeof(TASKWORK_SHADOW),
  shadowTask_Init,
  shadowTask_Delete,
  shadowTask_Update,
  shadowTask_Draw,
};

#ifndef DEBUG_SHADOW_PL_NON
//--------------------------------------------------------------
/**
 * フィールドOBJ用影追加
 * @param	fmmdl		FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FE_fmmdlShadow_Add( FLDMMDL * fmmdl )
{
	int param,pri;
	SHADOW_ADD_H head;
	FLDEFF_CTRL *fectrl;
	VecFx32 mtx;
	
	fectrl = FE_FieldOBJ_FLDEFF_CTRL_Get( fmmdl );
	
	head.fectrl = fectrl;
	head.shadow = FE_EffectWorkGet( fectrl, FE_FLD_SHADOW );
	head.fmmdl = fmmdl;
	FieldOBJ_VecPosGet( fmmdl, &mtx );
	
	param = SHADOW_BLACK;
	pri = FieldOBJ_TCBPriGet( fmmdl, fmmdl_TCBPRI_OFFS_AFTER );
	FE_EoaAddNpp( fectrl, &DATA_EoaH_Shadow, &mtx, param, &head, pri );
}

//--------------------------------------------------------------
/**
 * EOA 影　初期化
 * @param	eoa		EOA_PTR
 * @param	wk		eoa work *
 * @retval	int		TRUE=正常終了。FALSE=異常終了
 */
//--------------------------------------------------------------
static int EoaShadow_Init( EOA_PTR eoa, void *wk )
{
	SHADOW_WORK *work;
	const SHADOW_ADD_H *head;
	
	work = wk;
	head = EOA_AddPtrGet( eoa );
	work->head = *head;
	
	work->type = EOA_AddParamGet( eoa );
	work->obj_code = FieldOBJ_OBJCodeGet( work->head.fmmdl );
	work->obj_id = FieldOBJ_OBJIDGet( work->head.fmmdl );
	
	if( FieldOBJ_StatusBitCheck_Alies(work->head.fmmdl) == TRUE ){
		work->zone_id = FieldOBJ_ZoneIDGetAlies( work->head.fmmdl );
	}else{
		work->zone_id = FieldOBJ_ZoneIDGet( work->head.fmmdl );
	}
	
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * EOA 影　削除
 * @param	eoa		EOA_PTR
 * @param	wk		eoa work *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void EoaShadow_Delete( EOA_PTR eoa, void *wk )
{
}

//--------------------------------------------------------------
/**
 * EOA 影　動作
 * @param	eoa		EOA_PTR
 * @param	wk		eoa work *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void EoaShadow_Move( EOA_PTR eoa, void *wk )
{
	SHADOW_WORK *work;
	FLDMMDL * fmmdl;
	
	work = wk;
	fmmdl = work->head.fmmdl;
	
	if( FieldOBJ_CheckSameIDOBJCodeIn(
		fmmdl,work->obj_code,work->obj_id,work->zone_id) == FALSE ){
		FE_EoaDelete( eoa );										//同一ではない
		return;
	}
	
	if( FieldOBJ_FieldOBJSysStatusBitCheck(
        fmmdl,fmmdlSYS_STA_BIT_SHADOW_JOIN_NOT) ){
		FE_EoaDelete( eoa );
		return;
	}
	
	work->vanish_sw = FALSE;
	
	if( FieldOBJ_StatusBit_CheckEasy(fmmdl,
		fmmdl_STA_BIT_VANISH|fmmdl_STA_BIT_SHADOW_VANISH) == TRUE ){
		work->vanish_sw = TRUE;									//非表示
		return;
	}
	
	{
		VecFx32 vec;
		
		FieldOBJ_VecPosGet( fmmdl, &vec );
		EOA_MatrixSet( eoa, &vec );
	}
}

//--------------------------------------------------------------
/**
 * EOA 影　描画
 * @param	eoa		EOA_PTR
 * @param	wk		eoa work *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void EoaShadow_Draw( EOA_PTR eoa, void *wk )
{
	SHADOW_WORK *work = wk;
	
	if( work->vanish_sw == FALSE ){
		VecFx32 pos,scale;
		MtxFx33 rot = { FX32_ONE, 0,0,0, FX32_ONE, 0,0,0,FX32_ONE};
		FRO_OBJ *robj = &work->head.shadow->robj[SHADOW_BLACK];
		
		Shadow_TimeScaleGet( work->head.shadow, &scale );
		EOA_MatrixGet( eoa, &pos );
		pos.x += NUM_FX32(-1) / 2;
		pos.y += NUM_FX32(-4);
		pos.z += NUM_FX32(1);
		FRO_OBJ_Draw( robj, &pos, &scale, &rot );
	}
}

//--------------------------------------------------------------
///	影EOA_H
//--------------------------------------------------------------
static const EOA_H_NPP DATA_EoaH_Shadow =
{
	SHADOW_WORK_SIZE,
	EoaShadow_Init,
	EoaShadow_Delete,
	EoaShadow_Move,
	EoaShadow_Draw,
};
#endif

//======================================================================
//	data
//======================================================================
#ifndef DEBUG_SHADOW_PL_NON
//--------------------------------------------------------------
///	時間帯別拡大率
//--------------------------------------------------------------
static const VecFx32 DATA_ShadowTimeScaleTbl[] =
{
	{ FX32_ONE, FX32_ONE, FX32_ONE },	//TIMEZONE_MORNING
	{ FX32_ONE+(FX32_ONE/4), FX32_ONE, FX32_ONE+(FX32_ONE/4)},	//TIMEZONE_NOON
	{ FX32_ONE+(FX32_ONE/4), FX32_ONE, FX32_ONE },	//TIMEZONE_EVENING
	{ FX32_ONE+(FX32_ONE/8), FX32_ONE, FX32_ONE },	//TIMEZONE_NIGHT
	{ FX32_ONE-(FX32_ONE/8), FX32_ONE, FX32_ONE-(FX32_ONE/8) },	//TIMEZONE_MIDNIGHT
};

//--------------------------------------------------------------
///	時間帯別半透明濃度
//--------------------------------------------------------------
static const fx32 DATA_ShadowTimeAlphaTbl[] =
{
	FX32_ONE*14, //TIMEZONE_MORNING
	FX32_ONE*18,	//TIMEZONE_NOON
	FX32_ONE*18,	//TIMEZONE_EVENING
	FX32_ONE*8,	//TIMEZONE_NIGHT
	FX32_ONE*4,	//TIMEZONE_MIDNIGHT
};

//--------------------------------------------------------------
///	影種類別アーカイブインデックス
//--------------------------------------------------------------
static const DATA_ShadowArcIdx[SHADOW_MAX] =
{
	NARC_fldeff_kage_nsbmd,
	NARC_fldeff_red_mark_nsbmd,
	NARC_fldeff_blue_mark_nsbmd,
};
#endif
