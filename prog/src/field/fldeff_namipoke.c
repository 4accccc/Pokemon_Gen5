//======================================================================
/**
 * @file	fldeff_namipoke.c
 * @brief	フィールド 波乗りポケモン
 * @author	kagaya
 * @data	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_namipoke.h"

//======================================================================
//  define
//======================================================================
//#define DEBUG_PARAM_KEY_CHANGE

#define NAMIPOKE_SHAKE_VALUE (0x0400) ///<初期揺れ幅
#define NAMIPOKE_SHAKE_MAX (FX32_ONE*4) ///<揺れ幅最大

#define NAMIPOKE_RIDE_Y_OFFSET (FX32_ONE*7) ///<運転手表示オフセットY
#define NAMIPOKE_RIDE_Z_OFFSET (FX32_ONE*4) ///<運転手表示オフセットZ

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_NAMIPOKE型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_NAMIPOKE FLDEFF_NAMIPOKE;

//--------------------------------------------------------------
/// RIPPLE_RES構造体
//--------------------------------------------------------------
typedef struct
{
  GFL_G3D_RES *g3d_res_mdl;
  GFL_G3D_RES *g3d_res_anm[2];
  
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *obj_anm[2];
  GFL_G3D_RND *obj_rnd;
}RIPPLE_RES;

//--------------------------------------------------------------
/// RIPPLE_WORK構造体
//--------------------------------------------------------------
typedef struct
{
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *obj_anm[2];
  GFL_G3D_RND *obj_rnd;
  
  u16 vanish_flag;
  u16 dir;
  VecFx32 pos;
  VecFx32 save_pos;
  
  u16 rot_y;
  VecFx32 scale;
  VecFx32 offs;
}RIPPLE_WORK;

//--------------------------------------------------------------
///	FLDEFF_NAMIPOKE構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_NAMIPOKE
{
	FLDEFF_CTRL *fectrl;
  GFL_G3D_RES *g3d_res_mdl;
  
  RIPPLE_RES ripple_res;
};

//--------------------------------------------------------------
/// TASKHEADER_NAMIPOKE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_NAMIPOKE *eff_namipoke;
  MMDL *mmdl;
  u16 obj_id;
  u16 zone_id;
  u16 dir;
  u16 dmy;
  VecFx32 pos;
}TASKHEADER_NAMIPOKE;

//--------------------------------------------------------------
/// TASKWORK_NAMIPOKE
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  
  BOOL joint;
  
  u16 dir;
  fx32 shake_offs;
  fx32 shake_value;
  
  TASKHEADER_NAMIPOKE head;
  
  GFL_G3D_OBJ *obj;
  GFL_G3D_RND *obj_rnd;
  
  RIPPLE_WORK ripple_work;
}TASKWORK_NAMIPOKE;

//======================================================================
//	プロトタイプ
//======================================================================
static void namipoke_InitResource( FLDEFF_NAMIPOKE *namipoke );
static void namipoke_DeleteResource( FLDEFF_NAMIPOKE *namipoke );

static const FLDEFF_TASK_HEADER DATA_namipokeTaskHeader;

//======================================================================
//	波乗りポケモンエフェクト　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_NAMIPOKE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_NAMIPOKE *namipoke;
	
	namipoke = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_NAMIPOKE) );
	namipoke->fectrl = fectrl;
  
	namipoke_InitResource( namipoke );
	return( namipoke );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_NAMIPOKE_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_NAMIPOKE *namipoke = work;
  namipoke_DeleteResource( namipoke );
  GFL_HEAP_FreeMemory( namipoke );
}

//======================================================================
//	波乗りポケモンエフェクト　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipoke_InitResource( FLDEFF_NAMIPOKE *namipoke )
{
  BOOL ret;
  ARCHANDLE *handle;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( namipoke->fectrl );
  
  namipoke->g3d_res_mdl	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_sea_ride_nsbmd );
  ret = GFL_G3D_TransVramTexture( namipoke->g3d_res_mdl );
  GF_ASSERT( ret );
  
  {
    RIPPLE_RES *rip_res = &namipoke->ripple_res;
    rip_res->g3d_res_mdl =
      GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_shibuki01_nsbmd );
    ret = GFL_G3D_TransVramTexture( rip_res->g3d_res_mdl );
    GF_ASSERT( ret );
    
    rip_res->g3d_res_anm[0]	=
      GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_shibuki01_nsbca );
    rip_res->g3d_res_anm[1]	=
      GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_shibuki01_nsbta );
  }
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipoke_DeleteResource( FLDEFF_NAMIPOKE *namipoke )
{
 	GFL_G3D_DeleteResource( namipoke->g3d_res_mdl );
  
  {
    RIPPLE_RES *rip_res = &namipoke->ripple_res;
 	  GFL_G3D_DeleteResource( rip_res->g3d_res_mdl );
 	  GFL_G3D_DeleteResource( rip_res->g3d_res_anm[0] );
 	  GFL_G3D_DeleteResource( rip_res->g3d_res_anm[1] );
  }
}

//======================================================================
//	波乗りポケモンエフェクト　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用波乗りポケモンエフェクト　追加
 * @param FLDEFF_CTRL*
 * @param pos 表示座標
 * @param mmdl MMDL ポケモンに乗る動作モデル
 * @param joint 動作モデルとの同期 OFF=ジョイントしない
 * @retval nothing
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_NAMIPOKE_SetMMdl(
    FLDEFF_CTRL *fectrl, u16 dir, const VecFx32 *pos, MMDL *mmdl, BOOL joint )
{
  fx32 h;
  FLDEFF_TASK *task;
  TASKHEADER_NAMIPOKE head;
  FLDEFF_NAMIPOKE *namipoke;
  
  namipoke = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_NAMIPOKE );
  head.eff_namipoke = namipoke;
  head.mmdl = mmdl;
  head.obj_id = MMDL_GetOBJID( mmdl );
  head.zone_id = MMDL_GetZoneID( mmdl );
  head.dir = dir;
  head.pos = *pos;
  
  task = FLDEFF_CTRL_AddTask(
      fectrl, &DATA_namipokeTaskHeader, &head.pos, joint, &head, 0 );
  return( task );
}

//--------------------------------------------------------------
/**
 * 動作モデル用波乗りポケモンエフェクト　ジョイントフラグセット
 * @param task FLDEFF_TASK*
 * @param flag TRUE=接続,FALSE=非接続
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_NAMIPOKE_SetJointFlag( FLDEFF_TASK *task, BOOL flag )
{
  TASKWORK_NAMIPOKE *work = FLDEFF_TASK_GetWork( task );
  work->joint = flag;
}

#define RIP_SCALE_VECTOR_MAX (GRID_SIZE_FX32(4))
#define RIP_SCALE_VECTOR_PER (RIP_SCALE_VECTOR_MAX/100)
#define RIP_SCALE_MAX (FX32_ONE)
#define RIP_SCALE_PER (RIP_SCALE_MAX/100)
#define RIP_SCALE_VECTOR_LES (0x4000)

static fx32 ripple_scale_get( fx32 vec )
{
  if( vec > RIP_SCALE_VECTOR_MAX ){
    vec = RIP_SCALE_VECTOR_MAX;
  }
  vec /= RIP_SCALE_VECTOR_PER;
  vec = RIP_SCALE_PER * vec;
  return( vec );
}

static fx32 ripple_vecpos(
    const VecFx32 *m_pos, const VecFx32 *rip_pos, u16 dir )
{
  fx32 vec = 0;
  
  switch( dir ){
  case DIR_UP:
    vec = rip_pos->z - m_pos->z;
    break;
  case DIR_DOWN:
    vec = m_pos->z - rip_pos->z;
    break;
  case DIR_LEFT:
    vec = rip_pos->x - m_pos->x;
    break;
  case DIR_RIGHT:
    vec = m_pos->x - rip_pos->x;
    break;
  }
  return( vec );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipokeTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE *work = wk;
  const TASKHEADER_NAMIPOKE *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  FLDEFF_TASK_SetPos( task, &head->pos );
  
  work->dir = head->dir;
  
  work->obj_rnd =
    GFL_G3D_RENDER_Create(
        work->head.eff_namipoke->g3d_res_mdl, 0,
        work->head.eff_namipoke->g3d_res_mdl );
  
  work->obj = GFL_G3D_OBJECT_Create( work->obj_rnd, NULL, 0 );
  work->joint = FLDEFF_TASK_GetAddParam( task );
  work->shake_offs = FX32_ONE;
  work->shake_value = NAMIPOKE_SHAKE_VALUE;
  
  {
    RIPPLE_RES *rip_res = &work->head.eff_namipoke->ripple_res;
    RIPPLE_WORK *rip = &work->ripple_work;
    
    rip->obj_rnd = GFL_G3D_RENDER_Create(
      rip_res->g3d_res_mdl, 0, rip_res->g3d_res_mdl );
    
    rip->obj_anm[0] = GFL_G3D_ANIME_Create(
          rip->obj_rnd, rip_res->g3d_res_anm[0], 0 );
    rip->obj_anm[1] = GFL_G3D_ANIME_Create(
          rip->obj_rnd, rip_res->g3d_res_anm[1], 0 );
    
    rip->obj = GFL_G3D_OBJECT_Create( rip->obj_rnd, rip->obj_anm, 2 );
	  GFL_G3D_OBJECT_EnableAnime( rip->obj, 0 );
	  GFL_G3D_OBJECT_EnableAnime( rip->obj, 1 );
    
    rip->vanish_flag = TRUE;
    rip->dir = DIR_NOT;
    
    rip->scale.x = FX32_ONE;
    rip->scale.y = FX32_ONE;
    rip->scale.z = FX32_ONE;
  }
  
  FLDEFF_TASK_CallUpdate( task ); //即動作
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipokeTask_Delete( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE *work = wk;
  GFL_G3D_OBJECT_Delete( work->obj );
	GFL_G3D_RENDER_Delete( work->obj_rnd );
  
  {
    RIPPLE_WORK *rip = &work->ripple_work;
    GFL_G3D_OBJECT_Delete( rip->obj );
    GFL_G3D_RENDER_Delete( rip->obj_rnd );
    GFL_G3D_ANIME_Delete( rip->obj_anm[0] );
    GFL_G3D_ANIME_Delete( rip->obj_anm[1] );
  }
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipokeTask_Update( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE *work = wk;
  
  if( work->joint == FALSE ){ //接続しない
    return;
  }
 	
  { //方向
    work->dir = MMDL_GetDirDisp( work->head.mmdl );
  }
  
  { //揺れ
		work->shake_offs += work->shake_value;
		
		if( work->shake_offs >= NAMIPOKE_SHAKE_MAX ){
			work->shake_offs = NAMIPOKE_SHAKE_MAX;
			work->shake_value = -work->shake_value;
		}else if( work->shake_offs <= FX32_ONE ){
			work->shake_offs = FX32_ONE;
			work->shake_value = -work->shake_value;
		}
	}
  
  { //運転手に揺れを追加
    VecFx32 offs;
    offs.x = 0;
    offs.y = work->shake_offs + NAMIPOKE_RIDE_Y_OFFSET;
    offs.z = NAMIPOKE_RIDE_Z_OFFSET;
    MMDL_SetVectorOuterDrawOffsetPos( work->head.mmdl,  &offs );
  }

  { //座標
    VecFx32 pos;
    MMDL_GetVectorPos( work->head.mmdl, &pos );
    pos.y += work->shake_offs - FX32_ONE;
    FLDEFF_TASK_SetPos( task, &pos );
  }

  { //波紋エフェクト
    fx32 ret;
    VecFx32 pos;
    RIPPLE_WORK *rip = &work->ripple_work;
	  
    GFL_G3D_OBJECT_LoopAnimeFrame( rip->obj, 0, FX32_ONE );
	  GFL_G3D_OBJECT_LoopAnimeFrame( rip->obj, 1, FX32_ONE );

#if 1    
    MMDL_GetVectorPos( work->head.mmdl, &pos );
    
    if( work->dir != rip->dir ){ //方向更新
      rip->dir = work->dir;
      rip->pos = pos;
    }else if( pos.x == rip->save_pos.x && //停止 減衰
        pos.y == rip->save_pos.y && pos.z == rip->save_pos.z ){
      
      switch( rip->dir ){
      case DIR_UP:
        rip->pos.z -= RIP_SCALE_VECTOR_LES;
        if( rip->pos.z < pos.z ){ rip->pos.z = pos.z; }
        break;
      case DIR_DOWN:
        rip->pos.z += RIP_SCALE_VECTOR_LES;
        if( rip->pos.z > pos.z ){ rip->pos.z = pos.z; }
        break;
      case DIR_LEFT:
        rip->pos.x -= RIP_SCALE_VECTOR_LES;
        if( rip->pos.x < pos.x ){ rip->pos.x = pos.x; }
        break;
      case DIR_RIGHT:
        rip->pos.x += RIP_SCALE_VECTOR_LES;
        if( rip->pos.x > pos.x ){ rip->pos.x = pos.x; }
        break;
      }
    }
    
    ret = ripple_vecpos( &pos, &rip->pos, rip->dir );
    
    if( ret >= RIP_SCALE_VECTOR_MAX ){
      switch( rip->dir ){
      case DIR_UP:
        rip->pos.z = pos.z + RIP_SCALE_VECTOR_MAX;
        break;
      case DIR_DOWN:
        rip->pos.z = pos.z - RIP_SCALE_VECTOR_MAX;
        break;
      case DIR_LEFT:
        rip->pos.x = pos.x + RIP_SCALE_VECTOR_MAX;
        break;
      case DIR_RIGHT:
        rip->pos.x = pos.x - RIP_SCALE_VECTOR_MAX;
        break;
      }
    }

    ret = ripple_scale_get( ret );
    
    if( ret < RIP_SCALE_PER ){
      rip->vanish_flag = TRUE;
    }else{
      rip->vanish_flag = FALSE;
      rip->scale.x = ret;
      rip->scale.y = ret;
      rip->scale.z = ret;
    }
    
    rip->save_pos = pos;
#else
    rip->vanish_flag = FALSE;
    rip->dir = DIR_UP;
    rip->scale.x = FX32_ONE*1;
    rip->scale.y = FX32_ONE*1;
    rip->scale.z = FX32_ONE*1;
#endif
  }
}

#define RIPPLE_OFFS_Y (-0x4000)

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipokeTask_Draw( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_NAMIPOKE *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
  u16 rtbl[DIR_MAX4][3] = {{0,0x8000,0},{0,0,0},{0,0xc000,0},{0,0x4000,0}};
  u16 *rot = rtbl[work->dir];
  GFL_CALC3D_MTX_CreateRot( rot[0], rot[1], rot[2], &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
  
  if( work->ripple_work.vanish_flag == FALSE ){
    RIPPLE_WORK *rip = &work->ripple_work;
    VecFx32 rip_otbl[DIR_MAX4] = {
      {0,RIPPLE_OFFS_Y,0xe000},
      {0,RIPPLE_OFFS_Y,-0xe000},
      {0xe000,RIPPLE_OFFS_Y,0},
      {-0xe000,RIPPLE_OFFS_Y,0},
    };
    u16 rip_rtbl[DIR_MAX4][3] = {
      {0,0,0},{0,0x8000,0},{0,0x4000,0},{0,0xc000,0} };
    GFL_G3D_OBJSTATUS rip_status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
    VecFx32 *rip_offs = &rip_otbl[work->dir];
    u16 *rip_rot = rip_rtbl[work->dir];
    
#ifdef DEBUG_PARAM_KEY_CHANGE
    {
		  int key_trg = GFL_UI_KEY_GetTrg( );
		  int key_cont = GFL_UI_KEY_GetCont( );
    
      if( key_cont&PAD_BUTTON_Y ){
        if( key_trg&PAD_BUTTON_A ){
          rip->offs.x = 0;
          rip->offs.y = 0;
          rip->offs.z = 0;
        }

        if( key_cont&PAD_KEY_LEFT ){
          rip->offs.x -= 0x800;
        }else if( key_cont&PAD_KEY_RIGHT ){
          rip->offs.x += 0x800;
        }
        
        if( key_cont&PAD_BUTTON_B ){
          if( key_cont&PAD_KEY_UP ){
            rip->offs.y -= 0x800;
          }else if( key_cont&PAD_KEY_DOWN ){
            rip->offs.y += 0x800;
          }
        }else{
          if( key_cont&PAD_KEY_UP ){
            rip->offs.z -= 0x800;
          }else if( key_cont&PAD_KEY_DOWN ){
            rip->offs.z += 0x800;
          }
        }
        
        if( key_trg & PAD_BUTTON_L ){
          rip->rot_y += 0x4000;
        }
        
        if( key_trg & PAD_BUTTON_X ){
          OS_Printf( "OFFS X=%xH,Y=%xH,Z=%xH,ROT=%xH\n",
              rip->offs.x,
              rip->offs.y,
              rip->offs.z, rip->rot_y );
        }
      }
    }
#endif

    {
      GFL_CALC3D_MTX_CreateRot(
          rip_rot[0], rip_rot[1], rip_rot[2], &status.rotate );
      
      FLDEFF_TASK_GetPos( task, &status.trans );
      status.trans.x += rip->offs.x + rip_offs->x;
      status.trans.y += rip->offs.y + rip_offs->y;
      status.trans.z += rip->offs.z + rip_offs->z;
      
      status.scale = rip->scale;
    }

    GFL_G3D_DRAW_DrawObjectCullingON( rip->obj, &status );
  }
}

//--------------------------------------------------------------
//  波乗りポケモンエフェクトタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_namipokeTaskHeader =
{
  sizeof(TASKWORK_NAMIPOKE),
  namipokeTask_Init,
  namipokeTask_Delete,
  namipokeTask_Update,
  namipokeTask_Draw,
};
