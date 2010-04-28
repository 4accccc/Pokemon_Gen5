//======================================================================
/**
 * @file	fldeff_gyoe.c
 * @brief	フィールド びっくりマークエフェクト
 * @author	kagaya
 * @date	05.07.13
 *
 * 2010.04.21 tamada  カメラ逆行列で位置補正を行うように修正
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_gyoe.h"

#include "sound/pm_sndsys.h"


//======================================================================
//  define
//======================================================================
#define GYOE_FLDOBJ_Y_OFFSET (NUM_FX32(22)) ///<フィールドOBJからのYオフセット
#define GYOE_FLDOBJ_Y_MOVE_START  (0x6000)  ///<ギョエー初速
#define GYOE_FLDOBJ_Y_MOVE_OFFS   (0x2000)  ///<ギョエー速度変化
#define GYOE_END_FRAME (30) ///<ギョエー終了フレーム	

enum {
  PRO_MAT_Z_OFS = 4,
};
//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_GYOE型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_GYOE FLDEFF_GYOE;

//--------------------------------------------------------------
///	FLDEFF_GYOE構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_GYOE
{
	FLDEFF_CTRL *fectrl;
  GFL_G3D_RES *g3d_res_mdl[FLDEFF_GYOETYPE_MAX];
  GFL_G3D_RND *g3d_rnd[FLDEFF_GYOETYPE_MAX];
  GFL_G3D_OBJ *g3d_obj[FLDEFF_GYOETYPE_MAX];

  u16 gyoe_z_offs;
};

//--------------------------------------------------------------
/// TASKHEADER_GYOE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_GYOETYPE type;
  BOOL se_play;
  
  FLDEFF_GYOE *eff_gyoe;
  const MMDL *mmdl;
  MMDL_CHECKSAME_DATA samedata;
  FIELDMAP_WORK * fieldmap;
}TASKHEADER_GYOE;

//--------------------------------------------------------------
/// TASKWORK_GYOE
//--------------------------------------------------------------
typedef struct
{
  TASKHEADER_GYOE head;
  
  int seq_no;
  int frame;
  BOOL end_flag;
  fx32 offs_y;
  fx32 move_y;
}TASKWORK_GYOE;

//======================================================================
//	プロトタイプ
//======================================================================
static void gyoe_InitResource( FLDEFF_GYOE *gyoe );
static void gyoe_DeleteResource( FLDEFF_GYOE *gyoe );

static void gyoe_PlaySE( FLDEFF_GYOETYPE type );
static void gyoe_CalcPos( TASKWORK_GYOE *work, VecFx32 * pos );

static const FLDEFF_TASK_HEADER DATA_gyoeTaskHeader;
static const FLDEFF_TASK_HEADER DATA_gyoeTaskHeader_only;
static const u32 data_ArcIdxTbl[FLDEFF_GYOETYPE_MAX];

//======================================================================
//	びっくりマークエフェクト　システム
//======================================================================
//--------------------------------------------------------------
/**
 * びっくりマークエフェクト 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_GYOE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_GYOE *gyoe;
	
	gyoe = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_GYOE) );
	gyoe->fectrl = fectrl;
	gyoe_InitResource( gyoe );
	return( gyoe );
}

//--------------------------------------------------------------
/**
 * びっくりマークエフェクト 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_GYOE_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_GYOE *gyoe = work;
  gyoe_DeleteResource( gyoe );
  GFL_HEAP_FreeMemory( gyoe );
}

//======================================================================
//	びっくりマークエフェクト　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * びっくりマークエフェクト　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void gyoe_InitResource( FLDEFF_GYOE *gyoe )
{
  int i;
  BOOL ret;
  ARCHANDLE *handle;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( gyoe->fectrl );
  
  for( i = 0; i < FLDEFF_GYOETYPE_MAX; i++ ){
    gyoe->g3d_res_mdl[i]	=
      GFL_G3D_CreateResourceHandle( handle, data_ArcIdxTbl[i] );
    ret = GFL_G3D_TransVramTexture( gyoe->g3d_res_mdl[i] );
    GF_ASSERT( ret );
  
    gyoe->g3d_rnd[i] = GFL_G3D_RENDER_Create(
        gyoe->g3d_res_mdl[i], 0, gyoe->g3d_res_mdl[i] );
  
    gyoe->g3d_obj[i] = GFL_G3D_OBJECT_Create( gyoe->g3d_rnd[i], NULL, 0 );
  }

  gyoe->gyoe_z_offs = PRO_MAT_Z_OFS;
}

//--------------------------------------------------------------
/**
 * びっくりマークエフェクト　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void gyoe_DeleteResource( FLDEFF_GYOE *gyoe )
{
  int i;

  for( i = 0; i < FLDEFF_GYOETYPE_MAX; i++ ){
    GFL_G3D_OBJECT_Delete( gyoe->g3d_obj[i] );
	  GFL_G3D_RENDER_Delete( gyoe->g3d_rnd[i] );
 	  GFL_G3D_DeleteResource( gyoe->g3d_res_mdl[i] );
  }
}

#ifdef  PM_DEBUG
u32 DEBUG_GetGyoeZoffs( FLDEFF_CTRL *fectrl )
{
  FLDEFF_GYOE * eff_gyoe = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_GYOE );
  return eff_gyoe->gyoe_z_offs;
}
void DEBUG_SetGyoeZoffs( FLDEFF_CTRL *fectrl, u32 value )
{
  FLDEFF_GYOE * eff_gyoe = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_GYOE );
  eff_gyoe->gyoe_z_offs = value;
}
#endif
//======================================================================
//	びっくりマークエフェクト　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用びっくりマークエフェクト　追加
 * @param mmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval FLDEFF_TASK*
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_GYOE_SetMMdl( FLDEFF_CTRL *fectrl,
    const MMDL *mmdl, FLDEFF_GYOETYPE type, BOOL se_play )
{
  FLDEFF_GYOE *gyoe;
  TASKHEADER_GYOE head;
  
  head.type = type;
  head.se_play = se_play;
  
  head.eff_gyoe = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_GYOE );
  head.mmdl = mmdl;
  MMDL_InitCheckSameData( mmdl, &head.samedata );
  head.fieldmap = FLDEFF_CTRL_GetFieldMapWork( fectrl );
  
  if( se_play == TRUE ){
    gyoe_PlaySE( type );
  }
  
  return( FLDEFF_CTRL_AddTask(fectrl,&DATA_gyoeTaskHeader,NULL,0,&head,0) );
}

//--------------------------------------------------------------
/**
 * 動作モデル用びっくりマークエフェクト　追加　動作モデル依存無し版
 * @param mmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval FLDEFF_TASK*
 * @note 演出終了or動作モデル死亡時に終了します。
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_GYOE_SetMMdlNonDepend( FLDEFF_CTRL *fectrl,
    const MMDL *mmdl, FLDEFF_GYOETYPE type, BOOL se_play )
{
  FLDEFF_GYOE *gyoe;
  TASKHEADER_GYOE head;
  
  head.type = type;
  head.se_play = se_play;
  
  head.eff_gyoe = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_GYOE );
  head.mmdl = mmdl;
  MMDL_InitCheckSameData( mmdl, &head.samedata );
  head.fieldmap = FLDEFF_CTRL_GetFieldMapWork( fectrl );
  
  if( se_play == TRUE ){
    gyoe_PlaySE( type );
  }
  
  return( FLDEFF_CTRL_AddTask(fectrl,&DATA_gyoeTaskHeader_only,NULL,0,&head,0) );
}


//--------------------------------------------------------------
/**
 * 動作モデル用びっくりマークエフェクト　終了チェック
 * @param task FLDEFF_TASK
 * @retval
 */
//--------------------------------------------------------------
BOOL FLDEFF_GYOE_CheckEnd( FLDEFF_TASK *task )
{
  TASKWORK_GYOE *work = FLDEFF_TASK_GetWork( task );
  return( work->end_flag );
}

//--------------------------------------------------------------
/**
 * びっくりマークエフェクトタスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void gyoeTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_GYOE *work = wk;
  const TASKHEADER_GYOE *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  work->move_y = GYOE_FLDOBJ_Y_MOVE_START;
  FLDEFF_TASK_CallUpdate( task ); //即動作
}


//--------------------------------------------------------------
/**
 * びっくりマークエフェクトタスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void gyoeTask_Delete( FLDEFF_TASK *task, void *wk )
{
//  TASKWORK_GYOE *work = wk;
}

//--------------------------------------------------------------
/**
 * びっくりマークエフェクトタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void gyoeTask_Update( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_GYOE *work = wk;
  
  if( MMDL_CheckSameData(work->head.mmdl,&work->head.samedata) == FALSE ){
    GF_ASSERT( 0 ); //マーク表示中に消去
    work->end_flag = TRUE;
    return;
  }
    
  MMDL_GetVectorPos( work->head.mmdl, &pos );
  gyoe_CalcPos( work, &pos );
  FLDEFF_TASK_SetPos( task, &pos );
}

//--------------------------------------------------------------
/**
 * びっくりマークエフェクトタスク　更新　動作モデル依存無し版
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void gyoeTask_Update_only( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_GYOE *work = wk;
  
  if( work->end_flag == TRUE ||
      MMDL_CheckSameData(work->head.mmdl,&work->head.samedata) == FALSE ){
    FLDEFF_TASK_CallDelete( task ); //アニメ終了 or 動作モデル死亡　削除
    return;
  }
  
  MMDL_GetVectorPos( work->head.mmdl, &pos );
  gyoe_CalcPos( work, &pos );
  FLDEFF_TASK_SetPos( task, &pos );
}

//--------------------------------------------------------------
/**
 * びっくりマークエフェクトタスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void gyoeTask_Draw( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_GYOE *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};

  MtxFx44 org_pm,pm;
  const MtxFx44 * m = NNS_G3dGlbGetProjectionMtx();
  fx32 offs_z = FX32_CONST( work->head.eff_gyoe->gyoe_z_offs );
  org_pm = *m;
  pm = org_pm;
  pm._32 += FX_Mul( pm._22, offs_z );
  NNS_G3dGlbSetProjectionMtx(&pm);
  NNS_G3dGlbFlush();		  //　ジオメトリコマンドを転送
  NNS_G3dGeFlushBuffer(); // 転送まち

  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON(
      work->head.eff_gyoe->g3d_obj[work->head.type], &status );

  NNS_G3dGlbSetProjectionMtx(&org_pm);
  NNS_G3dGlbFlush();		//　ジオメトリコマンドを転送
  NNS_G3dGeFlushBuffer(); // 転送まち
}

//======================================================================
//  
//======================================================================
//--------------------------------------------------------------
/// タイプにあわせてSEを鳴らす
//--------------------------------------------------------------
static void gyoe_PlaySE( FLDEFF_GYOETYPE type )
{
  u32 idx = 0;

  switch( type ){
  case FLDEFF_GYOETYPE_GYOE:
    idx = SEQ_SE_FLD_07;
    break;
  case FLDEFF_GYOETYPE_HATE:
    idx = SEQ_SE_SYS_62;
    break;
  case FLDEFF_GYOETYPE_ONPU:
    idx = SEQ_SE_SYS_63;
    break;
  case FLDEFF_GYOETYPE_TEN:
    idx = SEQ_SE_SYS_64;
    break;
  }

  PMSND_PlaySE( idx );
}

//--------------------------------------------------------------
//動作モデル位置からのオフセットベクトルに、
//カメラの逆行列をかけて、ビルボードに対して正確な位置オフセットを算出する
//--------------------------------------------------------------
static void getBillboardOffset(
    const GFL_G3D_CAMERA * g3Dcamera, const VecFx32 * org_ofs, VecFx32 * bbd_ofs )
{
  VecFx32		camPos, camUp, target;
  MtxFx43		mtx43;
  MtxFx33 mtx33; 

  GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
  GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
  GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

  MTX_LookAt( &camPos, &camUp, &target, &mtx43 ); //カメラ行列取得
  MTX_Copy43To33( &mtx43, &mtx33 );               //並行移動成分はいらないので3x3行列にコピー
  MTX_Inverse33( &mtx33, &mtx33 );			          //カメラ逆行列生成

  //カメラ逆行列で回転させる＝＝カメラ行列で回転させても正位置になるはずのベクトル
  MTX_MultVec33( org_ofs, &mtx33, bbd_ofs );
}

//--------------------------------------------------------------
/**
 * @param work
 * @param pos
 */
//--------------------------------------------------------------
static void gyoe_CalcPos( TASKWORK_GYOE *work, VecFx32 * pos )
{
  VecFx32 ofs;

  switch( work->seq_no ){
  case 0:
    work->offs_y += work->move_y;
    if( work->offs_y ){
      work->move_y += -GYOE_FLDOBJ_Y_MOVE_OFFS;
    }else{
      work->move_y = 0;
      work->seq_no++;
    }
    break;
  case 1:
    work->frame++;
    
    if( work->frame >= GYOE_END_FRAME ){
      work->seq_no++;
      work->end_flag = TRUE;
    }
  }

#if 1
  {
    //動作モデル位置からビックリマーク位置までのオフセットベクトルに、
    //カメラの逆行列をかけて、ビルボードに対して正確な位置にマークをだす
    FIELD_CAMERA * fld_cam = FIELDMAP_GetFieldCamera( work->head.fieldmap );
    const GFL_G3D_CAMERA * g3Dcamera = FIELD_CAMERA_GetCameraPtr( fld_cam );
    VecFx32 ofs = (VecFx32){ 0, GYOE_FLDOBJ_Y_OFFSET + work->offs_y, 0 };
  
    getBillboardOffset( g3Dcamera, &ofs, &ofs );
    VEC_Add( &ofs, pos, pos );
  }
#else
  { //高速バージョン Y軸操作だけならばこちらのほうが計算量が少ない
    VecFx32 camera_way;
    VecFx32 camera_way_xz;
    VecFx32 camera_side;
    VecFx32 camera_up;
    VecFx32 camera_pos;
    VecFx32 camera_target;
    FIELD_CAMERA * fld_cam = FIELDMAP_GetFieldCamera( work->head.fieldmap );
    const GFL_G3D_CAMERA * g3Dcamera = FIELD_CAMERA_GetCameraPtr( fld_cam );
    static const VecFx32 up_way = { 0,FX32_ONE,0 };

    GFL_G3D_CAMERA_GetTarget( g3Dcamera, &camera_target );
    GFL_G3D_CAMERA_GetPos( g3Dcamera, &camera_pos );

    VEC_Subtract( &camera_target, &camera_pos, &camera_way );
    // XZ平面横方向取得
    camera_way_xz = camera_way;
    camera_way_xz.y = 0;
    VEC_Normalize( &camera_way_xz, &camera_way_xz );

    // 横方向を外積で求める
    VEC_CrossProduct( &camera_way_xz, &up_way, &camera_side );
    // 横とカメラ方向から上を求める
    VEC_CrossProduct( &camera_way, &camera_side, &camera_up );
    VEC_Normalize( &camera_up, &camera_up );
    
    // 上方向に移動
    pos->x += FX_Mul( camera_up.x, -( GYOE_FLDOBJ_Y_OFFSET + work->offs_y ) );
    pos->y += FX_Mul( camera_up.y, -( GYOE_FLDOBJ_Y_OFFSET + work->offs_y ) );
    pos->z += FX_Mul( camera_up.z, -( GYOE_FLDOBJ_Y_OFFSET + work->offs_y ) );
    
  }
#endif
  
}

//======================================================================
//  
//======================================================================
//--------------------------------------------------------------
//  びっくりマークエフェクトタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_gyoeTaskHeader =
{
  sizeof(TASKWORK_GYOE),
  gyoeTask_Init,
  gyoeTask_Delete,
  gyoeTask_Update,
  gyoeTask_Draw,
};

static const FLDEFF_TASK_HEADER DATA_gyoeTaskHeader_only =
{
  sizeof(TASKWORK_GYOE),
  gyoeTask_Init,
  gyoeTask_Delete,
  gyoeTask_Update_only,
  gyoeTask_Draw,
};

//======================================================================
//  
//======================================================================
//--------------------------------------------------------------
//  各アーカイブインデックス
//--------------------------------------------------------------
static const u32 data_ArcIdxTbl[FLDEFF_GYOETYPE_MAX] =
{
  NARC_fldeff_sisen_ef_nsbmd,
  NARC_fldeff_mark_hate_nsbmd,
  NARC_fldeff_mark_onpu_nsbmd,
  NARC_fldeff_mark_ten_nsbmd,
};
