//======================================================================
/**
 * @file	fldeff_shadow.c
 * @brief	フィールドOBJ影
 * @author	kagaya
 * @date	05.07.13
 *
 * 2010.04.14 tamada  処理負荷軽減のため、G3DOBJでなくビルボード描画に切り替え
 * 2010.04.22 tamada  DP世代の時間帯制御などコメント部分を削除
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"
#include "fldeff_shadow.h"

#include "debug/debug_flg.h" //DEBUG_FLG_～
//======================================================================
//	define
//======================================================================


//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FE_SHADOW型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_SHADOW FLDEFF_SHADOW;

//--------------------------------------------------------------
///	影エフェクト全体用のワーク定義
//--------------------------------------------------------------
struct _TAG_FLDEFF_SHADOW
{
	FLDEFF_CTRL *fectrl;

  VecFx32 scale;

  GFL_BBDACT_SYS *bbdactsys;
  GFL_BBD_SYS *bbdsys;
  int resIdx;
};

//--------------------------------------------------------------
/// 影エフェクトタスク登録用データ定義
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_SHADOW *eff_shadow;  ///<影エフェクト全体ワークへのポインタ
  MMDL *fmmdl;                ///<影が追随する対象の動作モデルへのポインタ
}SHADOW_TASKHEADER;

//--------------------------------------------------------------
/// 個別の影エフェクトごとのワーク定義
//--------------------------------------------------------------
typedef struct
{
  u32 vanish_flag;              ///<表示制御フラグ
  FLDEFF_SHADOW *eff_shadow;    ///<影エフェクト全体ワークへのポインタ
  MMDL *fmmdl;                  ///<影が追随する対象の動作モデルへのポインタ
  MMDL_CHECKSAME_DATA samedata; ///<動作モデル同一性チェック用ワーク
  int bbdUnitIdx;               ///<表示しているBBDACTのユニットインデックス
}TASKWORK_SHADOW;

//======================================================================
//	プロトタイプ
//======================================================================
static void shadow_InitResource( FLDEFF_SHADOW *sd );
static void shadow_DeleteResource( FLDEFF_SHADOW *sd );

static const FLDEFF_TASK_HEADER DATA_shadowTaskHeader;

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
  
  { //wb
    VecFx32 scale = {FX32_ONE,FX32_ONE,0xc00};
    sd->scale = scale;
  }
  
  {
    FIELDMAP_WORK * fieldmap = FLDEFF_CTRL_GetFieldMapWork( fectrl );
    sd->bbdactsys = FIELDMAP_GetSubBbdActSys( fieldmap );
    sd->bbdsys = GFL_BBDACT_GetBBDSystem( sd->bbdactsys );
  }

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

//--------------------------------------------------------------
/**
 * 影　グローバル縮尺値を指定
 * @param fectrl FLDEFF_CTRL
 * @pram scale 縮尺値
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_SHADOW_SetGlobalScale( FLDEFF_CTRL *fectrl, const VecFx32 *scale )
{
	FLDEFF_SHADOW *sd = FLDEFF_CTRL_GetEffectWork(
      fectrl, FLDEFF_PROCID_SHADOW  );
  sd->scale = *scale;
}

//======================================================================
//	影　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 影　リソース初期化
 * @param sd FLDEFF_SHADOW
 * @retval nothing
 */
//--------------------------------------------------------------
static void shadow_InitResource( FLDEFF_SHADOW *sd )
{
  ARCHANDLE *handle;
  GFL_G3D_RES *g3d_res;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( sd->fectrl );
  
  g3d_res	= GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_kage_nsbmd );
  {
    GFL_BBDACT_G3DRESDATA data;
    data.g3dres = g3d_res;
    data.texFmt = GFL_BBD_TEXFMT_PAL4;
    data.texSiz = GFL_BBD_TEXSIZ_16x16;
    data.celSizX = 16;
    data.celSizY = 16;
    data.dataCut = GFL_BBDACT_RESTYPE_DATACUT;

    sd->resIdx = GFL_BBDACT_AddResourceG3DResUnit( sd->bbdactsys, &data, 1 );
  }
}

//--------------------------------------------------------------
/**
 * 影　リソース削除
 * @param sd FLDEFF_SHADOW
 * @retval nothing
 */
//--------------------------------------------------------------
static void shadow_DeleteResource( FLDEFF_SHADOW *sd )
{
  GFL_BBDACT_RemoveResourceUnit( sd->bbdactsys, sd->resIdx, 1 );
}

//======================================================================
//	影　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用影　追加
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_SHADOW_SetMMdl( MMDL *fmmdl, FLDEFF_CTRL *fectrl )
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
  MMDL_InitCheckSameData( head->fmmdl, &work->samedata );

  {
    GFL_BBDACT_ACTDATA actData;
    actData.resID = 0;  //ResUnit内オフセット指定
    actData.sizX = FX32_ONE;
    actData.sizY = FX32_ONE;
    actData.alpha = 19;
    actData.drawEnable = TRUE;
    actData.lightMask = GFL_BBD_LIGHTMASK_0;
    actData.trans = (VecFx32){ 0, 0, 0 };
    actData.func = NULL;
    actData.work = work;
    work->bbdUnitIdx = GFL_BBDACT_AddActEx(
        work->eff_shadow->bbdactsys, work->eff_shadow->resIdx,
        &actData, 1, GFL_BBD_DRAWMODE_XZ_FLAT_BILLBORD );
    
    if( work->bbdUnitIdx == GFL_BBDACT_ACTUNIT_ID_INVALID ){
      FLDEFF_TASK_CallDelete( task );
      return;
    }
    {
      int bbdIdx = work->bbdUnitIdx + 0;  //BBDのINDEX == BBDACTのUNITIDX + Unit内Idx
      u8 polyID = 2;    //Base 0 + 2 == 2 が影のPolyID
      GFL_BBD_SetObjectPolID( work->eff_shadow->bbdsys, bbdIdx, &polyID );
    }
  }
  
  FLDEFF_TASK_CallUpdate( task ); //即、座標を反映する
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
  TASKWORK_SHADOW *work = wk;
  
  if( work->bbdUnitIdx != GFL_BBDACT_ACTUNIT_ID_INVALID ){
    GFL_BBDACT_RemoveAct(
        work->eff_shadow->bbdactsys, work->bbdUnitIdx, 1 );
  }
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
  
  if( MMDL_CheckSameData(work->fmmdl,&work->samedata) == FALSE ){
    FLDEFF_TASK_CallDelete( task );
    return;
  }else{
    const MMDLSYS *mmdlsys = MMDL_GetMMdlSys( work->fmmdl );
    
    if( MMDLSYS_CheckJoinShadow(mmdlsys) == FALSE ){
      FLDEFF_TASK_CallDelete( task );
      return;
    }
  }

  if( MMDL_CheckStatusBitVanish(work->fmmdl) == TRUE ||
      MMDL_CheckMoveBit(work->fmmdl,MMDL_MOVEBIT_SHADOW_VANISH) ){
    work->vanish_flag = TRUE;
  }
  else
  {
    work->vanish_flag = FALSE;
  }
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
  TASKWORK_SHADOW *work = wk;
  int bbdIdx = work->bbdUnitIdx + 0;  //BBDのINDEX == BBDACTのUNITIDX + Unit内Idx
  
  if( work->vanish_flag == FALSE ){
    int flag = TRUE;
    VecFx32 pos,offs;
    MMDL_GetVectorPos( work->fmmdl, &pos );
    MMDL_GetControlOffsetPos( work->fmmdl, &offs );
    VEC_Add( &pos, &offs, &pos );
    
    GFL_BBD_SetObjectDrawEnable( work->eff_shadow->bbdsys, bbdIdx, &flag );
    pos.y += FX32_CONST(+1);
    GFL_BBD_SetObjectTrans( work->eff_shadow->bbdsys, bbdIdx, &pos );
  } else {
    int flag = FALSE;
    GFL_BBD_SetObjectDrawEnable( work->eff_shadow->bbdsys, bbdIdx, &flag );
  }
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

