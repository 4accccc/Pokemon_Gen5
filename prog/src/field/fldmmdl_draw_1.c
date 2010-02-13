//======================================================================
/**
 * @file  fldmmdl_draw_1.c
 * @brief  フィールド動作モデル 描画処理その1　モデル関連
 * @author  kagaya
 * @date  05.07.13
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procdraw.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// DRAW_MODEL_WORK
//--------------------------------------------------------------
typedef struct
{
  FLD_G3DOBJ_OBJIDX obj_idx;
}DRAW_MODEL_WORK;

//======================================================================
//  proto
//======================================================================

//======================================================================
//  描画処理　モデル描画　表示、アニメループ再生
//======================================================================
//--------------------------------------------------------------
/**
 * モデル描画　初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawModel_Init( MMDL * mmdl )
{
  u16 code;
  DRAW_MODEL_WORK *work;

  work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_MODEL_WORK) );
  
  code = MMDL_GetOBJCode( mmdl );
  work->obj_idx = MMDL_G3DOBJCONT_AddObject( mmdl, code );
}

//--------------------------------------------------------------
/**
 * モデル描画　描画
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawModel_Draw( MMDL * mmdl )
{
  u16 code;
  VecFx32 pos;
  BOOL flag;
  DRAW_MODEL_WORK *work;
  FLD_G3DOBJ_CTRL *objctrl;
  
  work = MMDL_GetDrawProcWork( mmdl );
  objctrl = MMDL_G3DOBJCONT_GetFldG3dObjCtrl( mmdl );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  FLD_G3DOBJ_CTRL_SetObjPos( objctrl, work->obj_idx, &pos );
  
  flag = FALSE;
  if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
    flag = TRUE;
  }
  FLD_G3DOBJ_CTRL_SetObjVanishFlag( objctrl, work->obj_idx, flag );
  
  if( MMDL_CheckDrawPause(mmdl) == FALSE ){
    FLD_G3DOBJ_CTRL_LoopAnimeObject( objctrl, work->obj_idx, FX32_ONE );
  }
}

//--------------------------------------------------------------
/**
 * モデル描画　削除
 * @param  mmdl  MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawModel_Delete( MMDL * mmdl )
{
  DRAW_MODEL_WORK *work = MMDL_GetDrawProcWork( mmdl );
  MMDL_G3DOBJCONT_DeleteObject( mmdl, work->obj_idx );
}

//--------------------------------------------------------------
/**
 * モデル描画　退避
 * @param  mmdl    MMDL * 
 * @retval  int      TRUE=初期化成功
 */
//--------------------------------------------------------------
static void MMdl_DrawModel_Push( MMDL * mmdl )
{
  //現状、削除呼んで終了
  MMdl_DrawModel_Delete( mmdl );
}

//--------------------------------------------------------------
/**
 * モデル描画　復帰
 * 退避した情報を元に再描画。
 * @param  mmdl    MMDL * 
 * @retval  int      TRUE=初期化成功
 */
//--------------------------------------------------------------
static void MMdl_DrawModel_Pop( MMDL * mmdl )
{
  //現状、初期化呼んで終了
  MMdl_DrawModel_Init( mmdl );
}

//--------------------------------------------------------------
///  描画処理　モデル描画　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_Model =
{
  MMdl_DrawModel_Init,
  MMdl_DrawModel_Draw,
  MMdl_DrawModel_Delete,
  MMdl_DrawModel_Push,
  MMdl_DrawModel_Pop,
  NULL,
};
