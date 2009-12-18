//======================================================================
/**
 * @file  fldeff_footmark.c
 * @brief  フィールド 足跡
 * @author  kagaya
 * @date  05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_footmark.h"

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
///  足跡識別
//--------------------------------------------------------------
enum
{
  FOOTMARK_WALK_UP = 0,              ///<足跡　上
  FOOTMARK_WALK_DOWN,                ///<足跡　下
  FOOTMARK_WALK_LEFT,                ///<足跡　左
  FOOTMARK_WALK_RIGHT,              ///<足跡　右
  FOOTMARK_CYCLE_UD,                ///<自転車足跡 上下
  FOOTMARK_CYCLE_LR,                ///<自転車足跡 左右
  FOOTMARK_CYCLE_UL,                ///<自転車足跡 上左
  FOOTMARK_CYCLE_UR,                ///<自転車足跡 上右
  FOOTMARK_CYCLE_DL,                ///<自転車足跡 下左
  FOOTMARK_CYCLE_DR,                ///<自転車足跡 下右
  FOOTMARK_MAX,                    ///<地面足跡最大数
  
  FOOTMARK_SNOW_WALK_UP = FOOTMARK_MAX,   ///<足跡　上
  FOOTMARK_SNOW_WALK_DOWN,                ///<足跡　下
  FOOTMARK_SNOW_WALK_LEFT,                ///<足跡　左
  FOOTMARK_SNOW_WALK_RIGHT,              ///<足跡　右
  FOOTMARK_SNOW_CYCLE_UD,                ///<自転車足跡 上下
  FOOTMARK_SNOW_CYCLE_LR,                ///<自転車足跡 左右
  FOOTMARK_SNOW_CYCLE_UL,                ///<自転車足跡 上左
  FOOTMARK_SNOW_CYCLE_UR,                ///<自転車足跡 上右
  FOOTMARK_SNOW_CYCLE_DL,                ///<自転車足跡 下左
  FOOTMARK_SNOW_CYCLE_DR,                ///<自転車足跡 下右
  FOOTMARK_SNOW_MAX,                  ///<雪足跡最大数
  
  FOOTMARK_DEEPSNOW_UD = FOOTMARK_SNOW_MAX, //深い雪　上下
  FOOTMARK_DEEPSNOW_LR, //深い雪　左右
  FOOTMARK_DEEPSNOW_UL, //深い雪　上左
  FOOTMARK_DEEPSNOW_UR, //深い雪　上右
  FOOTMARK_DEEPSNOW_DL, //深い雪　下左
  FOOTMARK_DEEPSNOW_DR, //深い雪　下右
  FOOTMARK_DEEPSNOW_MAX,
  
  FOOTMARK_ALL_MAX = FOOTMARK_DEEPSNOW_MAX, ///<全足跡最大数
};

///雪 足跡リソース位置
#define FOOTMARK_SNOW_START (FOOTMARK_SNOW_WALK_UP)
///自転車足跡リソース位置
#define FOOTMARK_CYCLE_START (FOOTMARK_CYCLE_UD)
///雪　自転車足跡リソース位置
#define FOOTMARK_SNOW_CYCLE_START (FOOTMARK_SNOW_CYCLE_UD)
///深い雪 リソース位置
#define FOOTMARK_DEEPSNOW_START (FOOTMARK_DEEPSNOW_UD)

#define FOOTMARK_OFFSPOS_Y (NUM_FX32(-8))
#define FOOTMARK_OFFSPOS_Z (NUM_FX32(1))
#define FOOTMARK_OFFSPOS_Z_WALK_LR (NUM_FX32(-2))

#define FOOTMARK_VANISH_START_FRAME (16)			///<点滅開始フレーム(赤緑40)
#define FOOTMARK_VANISH_END_FRAME (28)				///<点滅終了フレーム(赤緑56)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///  FLDEFF_FOOTMARK型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_FOOTMARK FLDEFF_FOOTMARK;

//--------------------------------------------------------------
///  FLDEFF_FOOTMARK構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_FOOTMARK
{
  FLDEFF_CTRL *fectrl;
  
  GFL_G3D_RES *g3d_res_mdl[FOOTMARK_ALL_MAX];
  GFL_G3D_RND *g3d_rnd[FOOTMARK_ALL_MAX];
  GFL_G3D_OBJ *g3d_obj[FOOTMARK_ALL_MAX];
};

//--------------------------------------------------------------
/// TASKHEADER_FOOTMARK
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_FOOTMARK *eff_fmark;
  GFL_G3D_OBJ *obj;
  NNSG3dResMdl *resMdl;
}TASKHEADER_FOOTMARK;

//--------------------------------------------------------------
/// TASKWORK_FOOTMARK
//--------------------------------------------------------------
typedef struct
{
  TASKHEADER_FOOTMARK head;
  int seq_no;
  int vanish;
  int frame;
  int alpha;
}TASKWORK_FOOTMARK;

//======================================================================
//  プロトタイプ
//======================================================================
static void fmark_InitResource( FLDEFF_FOOTMARK *fmark );
static void fmark_DeleteResource( FLDEFF_FOOTMARK *fmark );
static int fmark_GetObject( FLDEFF_FOOTMARK *fmark,
    FOOTMARK_TYPE type, u16 now_dir, u16 old_dir, GFL_G3D_OBJ **outobj );

static const FLDEFF_TASK_HEADER data_fmarkTaskHeader;
static const u32 data_FootMarkArcIdx[FOOTMARK_ALL_MAX];
static const u8 data_FootMarkCycleDirTbl[DIR_MAX4][DIR_MAX4];

//======================================================================
//  足跡　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 足跡 初期化
 * @param  fectrl    FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval  void*  エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_FOOTMARK_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
  FLDEFF_FOOTMARK *fmark;
  
  fmark = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_FOOTMARK) );
  fmark->fectrl = fectrl;
  
  fmark_InitResource( fmark );
  return( fmark );
}

//--------------------------------------------------------------
/**
 * 足跡 削除
 * @param fectrl FLDEFF_CTRL
 * @param  work  エフェクト使用ワーク
 * @retval  nothing
 */
//--------------------------------------------------------------
void FLDEFF_FOOTMARK_Delete( FLDEFF_CTRL *fectrl, void *work )
{
  FLDEFF_FOOTMARK *fmark = work;
  fmark_DeleteResource( fmark );
  GFL_HEAP_FreeMemory( fmark );
}

//======================================================================
//  足跡　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 足跡　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void fmark_InitResource( FLDEFF_FOOTMARK *fmark )
{
  int i;
  NNSG3dResMdl *pMdl;
  NNSG3dRenderObj *rnd;
  const u32 *idx = data_FootMarkArcIdx;
  ARCHANDLE *handle = FLDEFF_CTRL_GetArcHandleEffect( fmark->fectrl );
  
  for( i = 0; i < FOOTMARK_ALL_MAX; i++, idx++ )
  {
    fmark->g3d_res_mdl[i] =
      GFL_G3D_CreateResourceHandle( handle, *idx );
      GFL_G3D_TransVramTexture( fmark->g3d_res_mdl[i] );
    fmark->g3d_rnd[i] =
      GFL_G3D_RENDER_Create(
          fmark->g3d_res_mdl[i], 0, fmark->g3d_res_mdl[i] );
  
    rnd = GFL_G3D_RENDER_GetRenderObj( fmark->g3d_rnd[i] );
    pMdl = NNS_G3dRenderObjGetResMdl( rnd );
    NNS_G3dMdlUseGlbDiff( pMdl );
    NNS_G3dMdlUseGlbAmb( pMdl );
    NNS_G3dMdlUseGlbSpec( pMdl );
    
    fmark->g3d_obj[i] = 
      GFL_G3D_OBJECT_Create( fmark->g3d_rnd[i], NULL, 0 );
  }
}

//--------------------------------------------------------------
/**
 * 足跡　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void fmark_DeleteResource( FLDEFF_FOOTMARK *fmark )
{
  int i;
  
  for( i = 0; i < FOOTMARK_ALL_MAX; i++ )
  {
    GFL_G3D_OBJECT_Delete( fmark->g3d_obj[i] );
    GFL_G3D_RENDER_Delete( fmark->g3d_rnd[i] );
    GFL_G3D_DeleteResource( fmark->g3d_res_mdl[i] );
  }
}
  
//--------------------------------------------------------------
/**
 * 足跡　指定タイプのOBJ取得
 * @param fmark FLDEFF_FOOTMARK *fmark
 * @param type FOOTMARK_TYPE
 * @param now_dir 現在方向
 * @param old_dir 過去方向
 * @param outobj GFL_G3D_OBJ格納先
 * @retval int FOOTMARK_WALK_UP等
 */
//--------------------------------------------------------------
static int fmark_GetObject( FLDEFF_FOOTMARK *fmark,
    FOOTMARK_TYPE type, u16 now_dir, u16 old_dir, GFL_G3D_OBJ **outobj )
{
  int no = 0;
  switch( type ){
  case FOOTMARK_TYPE_HUMAN:
    no = FOOTMARK_WALK_UP + now_dir;
    *outobj = fmark->g3d_obj[no];
    break;
  case FOOTMARK_TYPE_CYCLE:
    no = data_FootMarkCycleDirTbl[old_dir][now_dir];
    *outobj = fmark->g3d_obj[no];
    break;
  case FOOTMARK_TYPE_HUMAN_SNOW:
    no = FOOTMARK_SNOW_WALK_UP + now_dir;
    *outobj = fmark->g3d_obj[no];
    break;
  case FOOTMARK_TYPE_CYCLE_SNOW:
    no = data_FootMarkCycleDirTbl[old_dir][now_dir] - FOOTMARK_CYCLE_START;
    no += FOOTMARK_SNOW_CYCLE_START;
    *outobj = fmark->g3d_obj[no];
    break;
  case FOOTMARK_TYPE_DEEPSNOW:
    no = data_FootMarkCycleDirTbl[old_dir][now_dir] - FOOTMARK_CYCLE_START;
    no += FOOTMARK_DEEPSNOW_START;
    *outobj = fmark->g3d_obj[no];
    break;
  default:
    GF_ASSERT( 0 );
    *outobj = fmark->g3d_obj[no];
  }
  return( no );
}

//======================================================================
//  足跡　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用足跡　追加
 * @param mmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_FOOTMARK_SetMMdl(
    MMDL *mmdl, FLDEFF_CTRL *fectrl, FOOTMARK_TYPE type )
{
  int no;
  VecFx32 pos;
  FLDEFF_FOOTMARK *fmark;
  TASKHEADER_FOOTMARK head;
  
  fmark = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_FOOTMARK );
  head.eff_fmark = fmark;
  no = fmark_GetObject( fmark, type,
      MMDL_GetDirDisp(mmdl), MMDL_GetDirDispOld(mmdl), &head.obj );
  
  {
    GFL_G3D_RND *g3drnd = GFL_G3D_OBJECT_GetG3Drnd( head.obj );
    NNSG3dRenderObj *rnd = GFL_G3D_RENDER_GetRenderObj( g3drnd );
    head.resMdl = NNS_G3dRenderObjGetResMdl( rnd );
  }

  MMDL_GetVectorPos( mmdl, &pos ); //y
  MMDL_TOOL_GetCenterGridPos(
      MMDL_GetOldGridPosX(mmdl), MMDL_GetOldGridPosZ(mmdl), &pos );
  
  switch( type ){
  case FOOTMARK_TYPE_HUMAN:
  case FOOTMARK_TYPE_CYCLE:
    pos.y += FOOTMARK_OFFSPOS_Y;
    
    if( type == FOOTMARK_TYPE_HUMAN &&
        (no == FOOTMARK_WALK_LEFT || no == FOOTMARK_WALK_RIGHT) ){
      pos.z += FOOTMARK_OFFSPOS_Z_WALK_LR;
    }else{
      pos.z += FOOTMARK_OFFSPOS_Z;
    }
    break;
  case FOOTMARK_TYPE_HUMAN_SNOW:
  case FOOTMARK_TYPE_DEEPSNOW:
    pos.y += NUM_FX32( -7 );
    break;
  }
  
  FLDEFF_CTRL_AddTask(
      fectrl, &data_fmarkTaskHeader, &pos, 0, &head, 0 );
}

//--------------------------------------------------------------
/**
 * 足跡タスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fmarkTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_FOOTMARK *work = wk;
  const TASKHEADER_FOOTMARK *head;
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  work->alpha = 31;
}

//--------------------------------------------------------------
/**
 * 足跡タスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fmarkTask_Delete( FLDEFF_TASK *task, void *wk )
{
}

//--------------------------------------------------------------
/**
 * 足跡タスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fmarkTask_Update( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_FOOTMARK *work = wk;
  
  switch( work->seq_no ){
  case 0:
    work->frame++;
    if( work->frame >= FOOTMARK_VANISH_START_FRAME ){
      work->seq_no++;
    }
    break;
  case 1:
    work->alpha -= 2;
    if( work->alpha < 0 ){
      FLDEFF_TASK_CallDelete( task );
    }
  }
} 

//--------------------------------------------------------------
/**
 * 足跡タスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fmarkTask_Draw( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_FOOTMARK *work = wk;
  
  if( work->vanish == FALSE ){
    GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
    NNS_G3dMdlUseMdlAlpha( work->head.resMdl );
		NNS_G3dMdlSetMdlAlphaAll( work->head.resMdl, work->alpha );
    MTX_Identity33( &status.rotate );
    FLDEFF_TASK_GetPos( task, &status.trans );
    GFL_G3D_DRAW_DrawObjectCullingON( work->head.obj, &status );
  }
}

//--------------------------------------------------------------
//  足跡タスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER data_fmarkTaskHeader =
{
  sizeof(TASKWORK_FOOTMARK),
  fmarkTask_Init,
  fmarkTask_Delete,
  fmarkTask_Update,
  fmarkTask_Draw,
};

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
///	足跡imdテーブル　並びはFOOTMARK_WALK_UP等の値に一致
//--------------------------------------------------------------
static const u32 data_FootMarkArcIdx[FOOTMARK_ALL_MAX] =
{
  //通常
	NARC_fldeff_f_mark_u_nsbmd,
	NARC_fldeff_f_mark_d_nsbmd,
	NARC_fldeff_f_mark_l_nsbmd,
	NARC_fldeff_f_mark_r_nsbmd,
	NARC_fldeff_c_mark_u_nsbmd,
	NARC_fldeff_c_mark_l_nsbmd,
	NARC_fldeff_c_mark_ul_nsbmd,
	NARC_fldeff_c_mark_ur_nsbmd,
	NARC_fldeff_c_mark_dl_nsbmd,
	NARC_fldeff_c_mark_dr_nsbmd,
  
  //雪
	NARC_fldeff_nf_mark_u_nsbmd,
	NARC_fldeff_nf_mark_d_nsbmd,
	NARC_fldeff_nf_mark_l_nsbmd,
	NARC_fldeff_nf_mark_r_nsbmd,
	NARC_fldeff_sc_mark_u_nsbmd,
	NARC_fldeff_sc_mark_l_nsbmd,
	NARC_fldeff_sc_mark_ul_nsbmd,
	NARC_fldeff_sc_mark_ur_nsbmd,
	NARC_fldeff_sc_mark_dl_nsbmd,
	NARC_fldeff_sc_mark_dr_nsbmd,

  //深い雪
	NARC_fldeff_nc_mark_u_nsbmd,
	NARC_fldeff_nc_mark_l_nsbmd,
	NARC_fldeff_nc_mark_ul_nsbmd,
	NARC_fldeff_nc_mark_ur_nsbmd,
	NARC_fldeff_nc_mark_dl_nsbmd,
	NARC_fldeff_nc_mark_dr_nsbmd,
};

//--------------------------------------------------------------
///	自転車足跡テーブル [過去方向][現在方向]
//--------------------------------------------------------------
static const u8 data_FootMarkCycleDirTbl[DIR_MAX4][DIR_MAX4] =
{
	{FOOTMARK_CYCLE_UD,FOOTMARK_CYCLE_UD,FOOTMARK_CYCLE_UR,FOOTMARK_CYCLE_UL},
	{FOOTMARK_CYCLE_UD,FOOTMARK_CYCLE_UD,FOOTMARK_CYCLE_DR,FOOTMARK_CYCLE_DL},
	{FOOTMARK_CYCLE_DL,FOOTMARK_CYCLE_UL,FOOTMARK_CYCLE_LR,FOOTMARK_CYCLE_LR},
	{FOOTMARK_CYCLE_DR,FOOTMARK_CYCLE_UR,FOOTMARK_CYCLE_LR,FOOTMARK_CYCLE_LR},
};
