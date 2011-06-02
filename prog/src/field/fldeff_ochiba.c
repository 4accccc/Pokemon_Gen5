//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fldeff_ochiba.c
 *	@brief  落ち葉エフェクト
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.05.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_ochiba.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	リソースIDX
//=====================================
enum
{
  RESID_OCHIBA_UP,      // 落ち葉 上
  RESID_OCHIBA_DOWN,    // 落ち葉 下
  RESID_OCHIBA_RIGHT,   // 落ち葉 右
  RESID_OCHIBA_LEFT,    // 落ち葉 左

  RESID_MAX,    // 最大数
};


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	FLDEFF_OCHIBA型
//=====================================
typedef struct _TAG_FLDEFF_OCHIBA FLDEFF_OCHIBA;

//-------------------------------------
///	FLDEFF_OCHIBA構造体
//=====================================
struct _TAG_FLDEFF_OCHIBA{
  FLDEFF_CTRL       *fectrl;
  FLD_G3DOBJ_RESIDX res_idx;
};

//--------------------------------------------------------------
/// TASKHEADER_OCHIBA
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_OCHIBA *eff_ochiba;
  MMDL    *mmdl;
  u16     dir;
  u16     dmy;
  VecFx32 pos;
}TASKHEADER_OCHIBA;

//--------------------------------------------------------------
/// TASKWORK_OCHIBA
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_OCHIBA *eff_ochiba;
  FLD_G3DOBJ_CTRL *obj_ctrl;
  FLD_G3DOBJ_OBJIDX obj_idx;
  u16     dir;
}TASKWORK_OCHIBA;


//======================================================================
//	プロトタイプ
//======================================================================
static void ochiba_InitResource( FLDEFF_OCHIBA *ochiba );
static void ochiba_DeleteResource( FLDEFF_OCHIBA *ochiba );
static int  ochiba_GetObject( FLDEFF_OCHIBA *ochiba, u16 now_dir, u16 old_dir );

static const FLDEFF_TASK_HEADER DATA_ochibaTaskHeader;

//======================================================================
//	落ち葉　システム
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  落ち葉  初期化
 *
 *	@param	FLDEFF_CTRL *fectrl
 *	@param	heapID 
 *
 *	@return void* エフェクト使用ワーク
 */
//-----------------------------------------------------------------------------
void * FLDEFF_OCHIBA_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_OCHIBA *ochiba;
	FIELDMAP_WORK *fieldmap;
	
	ochiba = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_OCHIBA) );
	ochiba->fectrl = fectrl;
  
	ochiba_InitResource( ochiba );
	return( ochiba );
}

//----------------------------------------------------------------------------
/**
 *	@brief  落ち葉  削除
 *
 *	@param	FLDEFF_CTRL *fectrl
 *	@param	*work   エフェクト使用ワーク
 */
//-----------------------------------------------------------------------------
void FLDEFF_OCHIBA_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_OCHIBA *ochiba = work;
  ochiba_DeleteResource( ochiba );
  GFL_HEAP_FreeMemory( ochiba );
}

//======================================================================
//	落ち葉　リソース
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  落ち葉  リソース初期化
 *
 *	@param	FLDEFF_OCHIBA *ochiba 
 */
//-----------------------------------------------------------------------------
static void ochiba_InitResource( FLDEFF_OCHIBA *ochiba )
{
  ARCHANDLE *handle;
  FLD_G3DOBJ_RES_HEADER head;
  FLD_G3DOBJ_CTRL *obj_ctrl;
  
  obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( ochiba->fectrl );
  handle = FLDEFF_CTRL_GetArcHandleEffect( ochiba->fectrl );
  
  FLD_G3DOBJ_RES_HEADER_Init( &head );
  FLD_G3DOBJ_RES_HEADER_SetMdl( &head, handle, NARC_fldeff_ochiba01_nsbmd );
  FLD_G3DOBJ_RES_HEADER_SetAnmArcHandle( &head, handle );
  FLD_G3DOBJ_RES_HEADER_SetAnmArcIdx( &head, NARC_fldeff_ochiba01_nsbca );

  ochiba->res_idx = FLD_G3DOBJ_CTRL_CreateResource( obj_ctrl, &head, FALSE );
  //ochiba->obj_idx = FLD_G3DOBJ_CTRL_AddObject( obj_ctrl, ochiba->res_idx, 0, NULL );
  //FLD_G3DOBJ_CTRL_SetOuterDrawFlag( obj_ctrl, ochiba->obj_idx, TRUE );

  //ICHI_Printf("落ち葉 onj_idx = %d \n", ochiba->obj_idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief  落ち葉  リソース削除
 *
 *	@param	FLDEFF_OCHIBA *ochiba 
 */
//-----------------------------------------------------------------------------
static void ochiba_DeleteResource( FLDEFF_OCHIBA *ochiba )
{
  FLD_G3DOBJ_CTRL *obj_ctrl;
  obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( ochiba->fectrl );
  FLD_G3DOBJ_CTRL_DeleteResource( obj_ctrl, ochiba->res_idx );
}


//======================================================================
//	落ち葉　タスク
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  動作モデル用落ち葉  追加
 *
 *	@param	FLDEFF_CTRL *fectrl
 *	@param	mmdl
 */
//-----------------------------------------------------------------------------
void FLDEFF_OCHIBA_Set( FLDEFF_CTRL *fectrl, MMDL *mmdl )
{
  VecFx32 pos;
  u16     dir;
  s16     gx,gz;
  fx32    height;
  FLDEFF_OCHIBA *ochiba;
  TASKHEADER_OCHIBA head;
  
  gx = MMDL_GetGridPosX( mmdl );
  gz = MMDL_GetGridPosZ( mmdl );
  MMDL_GetVectorPos( mmdl, &pos );
  MMDL_TOOL_GetCenterGridPos( gx, gz, &pos );
  height = 0;
  MMDL_GetMapPosHeight( mmdl, &pos, &height );
  pos.y = height;
  dir = MMDL_GetDirDisp( mmdl );
  
  ochiba = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_OCHIBA );
  head.eff_ochiba = ochiba;
  head.mmdl = mmdl;
  head.pos = pos;
  head.dir = dir;
  
  FLDEFF_CTRL_AddTask(
      fectrl, &DATA_ochibaTaskHeader, &head.pos, 0, &head, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  落ち葉タスク  初期化
 *
 *	@param	FLDEFF_TASK *task
 *	@param	*wk 
 */
//-----------------------------------------------------------------------------
static void ochibaTask_Init( FLDEFF_TASK *task, void *wk )
{
  FLD_G3DOBJ_CTRL *obj_ctrl;
  TASKWORK_OCHIBA *work = wk;
  const TASKHEADER_OCHIBA *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->eff_ochiba = head->eff_ochiba;

  obj_ctrl = FLDEFF_CTRL_GetFldG3dOBJCtrl( work->eff_ochiba->fectrl );
  work->dir = head->dir;
  work->obj_ctrl = obj_ctrl;
  
  FLDEFF_TASK_SetPos( task, &head->pos );
  
  //work->obj_idx = work->eff_ochiba->obj_idx;
  //ICHI_Printf("落ち葉タスク初期化 \n");
  //ICHI_Printf("\tobj_idx = %d\n", work->obj_idx);
  //ICHI_Printf("\t(%x, %x, %x)\n", head->pos.x, head->pos.y, head->pos.z);

  //FLD_G3DOBJ_CTRL_SetObjVanishFlag( obj_ctrl, work->obj_idx, FALSE );
  //FLD_G3DOBJ_CTRL_SetAnimeFrame( obj_ctrl, work->obj_idx, 0, 0 );

  work->obj_idx = FLD_G3DOBJ_CTRL_AddObject( obj_ctrl, work->eff_ochiba->res_idx, 0, &head->pos );
  FLD_G3DOBJ_CTRL_SetOuterDrawFlag( obj_ctrl, work->obj_idx, TRUE );

  /*
  {
    GFL_G3D_OBJ *g3dobj = FLD_G3DOBJ_CTRL_GetObject( obj_ctrl, work->obj_idx );
    GFL_G3D_OBJECT_GetAnimeFrameMax( g3dobj, 0, &work->anm_frm_max );
  }
  */

  //FLDEFF_TASK_CallUpdate( task ); //即動作
}

//----------------------------------------------------------------------------
/**
 *	@brief  落ち葉タスク  削除
 *
 *	@param	FLDEFF_TASK *task
 *	@param	*wk 
 */
//-----------------------------------------------------------------------------
static void ochibaTask_Delete( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_OCHIBA *work = wk;
  FLD_G3DOBJ_CTRL_DeleteObject( work->obj_ctrl, work->obj_idx );
  //FLD_G3DOBJ_CTRL_SetObjVanishFlag( work->obj_ctrl, work->obj_idx, TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  落ち葉タスク  更新
 *
 *	@param	FLDEFF_TASK *task
 *	@param	*wk 
 */
//-----------------------------------------------------------------------------
static void ochibaTask_Update( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_OCHIBA *work = wk;
	//FLDEFF_OCHIBA *ochiba = work->eff_ochiba;

#if 0
  work->anm_frm += FX32_ONE;
  FLD_G3DOBJ_CTRL_SetAnimeFrame( work->obj_ctrl, work->obj_idx, 0, work->anm_frm );

  if( work->anm_frm >= work->anm_frm_max ){
    FLDEFF_TASK_CallDelete( task );
  }
#endif

  if( FLD_G3DOBJ_CTRL_AnimeObject( 
        work->obj_ctrl, work->obj_idx, FX32_ONE ) == FALSE )
  {
    FLDEFF_TASK_CallDelete( task );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  落ち葉タスク  描画
 *
 *	@param	FLDEFF_TASK *task
 *	@param	*wk 
 */
//-----------------------------------------------------------------------------
static void ochibaTask_Draw( FLDEFF_TASK *task, void *wk )
{
#if 1
  TASKWORK_OCHIBA *work = wk;

  u16 rtbl[DIR_MAX4][3] = {{0,0xc000,0},{0,0x4000,0},{0,0,0},{0,0x8000,0}};
  u16 *rot = rtbl[work->dir];
  GFL_G3D_OBJSTATUS *st;

  st = FLD_G3DOBJ_CTRL_GetObjStatus( work->obj_ctrl, work->obj_idx );
  GFL_CALC3D_MTX_CreateRot( rot[0], rot[1], rot[2], &st->rotate );
  FLDEFF_TASK_GetPos( task, &st->trans );

  FLD_G3DOBJ_CTRL_DrawObject( work->obj_ctrl, work->obj_idx );
#endif
}

//--------------------------------------------------------------
//  落ち葉タスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_ochibaTaskHeader =
{
  sizeof(TASKWORK_OCHIBA),
  ochibaTask_Init,
  ochibaTask_Delete,
  ochibaTask_Update,
  ochibaTask_Draw,
};

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
//  落ち葉  モデルアーカイブインデックス
//--------------------------------------------------------------
static const u16 data_ArcIdxOchibaMdl[RESID_MAX] =
{
  NARC_fldeff_ochiba01_nsbmd,
  NARC_fldeff_ochiba01_nsbmd,
  NARC_fldeff_ochiba01_nsbmd,
  NARC_fldeff_ochiba01_nsbmd,
};

//--------------------------------------------------------------
//  落ち葉  アニメアーカイブインデックス
//--------------------------------------------------------------
static const u16 data_ArcIdxOchibaAnm[RESID_MAX] =
{
  NARC_fldeff_ochiba01_nsbca,
  NARC_fldeff_ochiba01_nsbca,
  NARC_fldeff_ochiba01_nsbca,
  NARC_fldeff_ochiba01_nsbca,
};
