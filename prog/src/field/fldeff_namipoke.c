//======================================================================
/**
 * @file	fldeff_namipoke.c
 * @brief	フィールド 波乗りポケモン
 * @author	kagaya
 * @date	05.07.13
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
typedef struct _TAG_FLDEFF_NAMIPOKE_EFFECT FLDEFF_NAMIPOKE_EFFECT;

//--------------------------------------------------------------
/// RIPPLE_RES構造体
//--------------------------------------------------------------
typedef struct
{
  GFL_G3D_RES *g3d_res_mdl;
  GFL_G3D_RES *g3d_res_anm[2];
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
  
  u8 joint;
  u8 ripple_off;

  u16 dir;
  fx32 shake_offs;
  fx32 shake_value;
  
  TASKHEADER_NAMIPOKE head;
  
  GFL_G3D_OBJ *obj;
  GFL_G3D_RND *obj_rnd;
  
  RIPPLE_WORK ripple_work;
}TASKWORK_NAMIPOKE;

//--------------------------------------------------------------
/// FLDEFF_NAMIPOKE_EFFECT構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_NAMIPOKE_EFFECT
{
  FLDEFF_CTRL *fectrl;
  
  GFL_G3D_RES *g3d_res_mdl_taki_land;
  GFL_G3D_RES *g3d_res_anm_taki_land_ima;
  GFL_G3D_RES *g3d_res_anm_taki_land_ica;
  
  GFL_G3D_RES *g3d_res_mdl_taki_start_f;
  GFL_G3D_RES *g3d_res_anm_taki_start_f_ica;
  GFL_G3D_RES *g3d_res_anm_taki_start_f_itp;
  
  GFL_G3D_RES *g3d_res_mdl_taki_loop_f;
  GFL_G3D_RES *g3d_res_anm_taki_loop_f_ica;
  GFL_G3D_RES *g3d_res_anm_taki_loop_f_itp;
  
  GFL_G3D_RES *g3d_res_mdl_taki_start_s;
  GFL_G3D_RES *g3d_res_anm_taki_start_s_ica;
  GFL_G3D_RES *g3d_res_anm_taki_start_s_itp;
  
  GFL_G3D_RES *g3d_res_mdl_taki_loop_s;
  GFL_G3D_RES *g3d_res_anm_taki_loop_s_ica;
  GFL_G3D_RES *g3d_res_anm_taki_loop_s_itp;
};

//--------------------------------------------------------------
/// TASKHEADER_NAMIPOKE_EFFECT構造体
//--------------------------------------------------------------
typedef struct
{
  BOOL dead_flag;
  FLDEFF_NAMIPOKE_EFFECT *eff_npoke_eff;
  NAMIPOKE_EFFECT_TYPE type;
  const FLDEFF_TASK *efftask_namipoke;
}TASKHEADER_NAMIPOKE_EFFECT;

//--------------------------------------------------------------
/// TASKWORK_NAMIPOKE_EFFECT構造体
//--------------------------------------------------------------
typedef struct
{
  GFL_G3D_OBJ *obj;
  GFL_G3D_ANM *obj_anm[2];
  GFL_G3D_RND *obj_rnd;
  VecFx32 offset;
  BOOL end_flag;
  TASKHEADER_NAMIPOKE_EFFECT head;
}TASKWORK_NAMIPOKE_EFFECT;

//======================================================================
//	プロトタイプ
//======================================================================
static void namipoke_InitResource( FLDEFF_NAMIPOKE *namipoke );
static void namipoke_DeleteResource( FLDEFF_NAMIPOKE *namipoke );

static const FLDEFF_TASK_HEADER DATA_namipokeTaskHeader;

static void npoke_eff_InitResource( FLDEFF_NAMIPOKE_EFFECT *npoke_eff );
static void npoke_eff_DeleteResource( FLDEFF_NAMIPOKE_EFFECT *npoke_eff );

static const FLDEFF_TASK_HEADER data_npoke_effTaskHeader;

//======================================================================
//	波乗りポケモン システム
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りポケモン 初期化
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
 * 波乗りポケモン 削除
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
//	波乗りポケモン　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りポケモン　リソース初期化
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
 * 波乗りポケモン　リソース削除
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
//	波乗りポケモン　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用波乗りポケモン　追加
 * @param FLDEFF_CTRL*
 * @param pos 表示座標
 * @param mmdl MMDL ポケモンに乗る動作モデル
 * @param joint 動作モデルとの同期 OFF=ジョイントしない
 * @retval nothing
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_NAMIPOKE_SetMMdl( FLDEFF_CTRL *fectrl,
    u16 dir, const VecFx32 *pos, MMDL *mmdl, NAMIPOKE_JOINT joint )
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
 * 動作モデル用波乗りポケモン　ジョイントフラグセット
 * @param task FLDEFF_TASK*
 * @param flag TRUE=接続,FALSE=非接続
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_NAMIPOKE_SetJointFlag( FLDEFF_TASK *task, NAMIPOKE_JOINT flag )
{
  TASKWORK_NAMIPOKE *work = FLDEFF_TASK_GetWork( task );
  work->joint = flag;
}

//--------------------------------------------------------------
/**
 * 動作モデル用波乗りポケモン　水飛沫エフェクトON,OFF
 * @param task FLDEFF_TASK*
 * @param flag TRUE=表示 FALSE=非表示
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_NAMIPOKE_SetRippleEffect( FLDEFF_TASK *task, BOOL flag )
{
  TASKWORK_NAMIPOKE *work = FLDEFF_TASK_GetWork( task );
  if( flag == TRUE ){
    work->ripple_off = FALSE;
  }else{
    work->ripple_off = TRUE;
  }
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
 * 波乗りポケモンタスク　初期化
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
 * 波乗りポケモンタスク　削除
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
 * 波乗りポケモンタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void namipokeTask_Update( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE *work = wk;
  VecFx32 oya_offs = { 0, 0, 0 };
  
  if( work->joint == NAMIPOKE_JOINT_OFF ){ //接続しない
    return;
  }
 	
  { //方向
    work->dir = MMDL_GetDirDisp( work->head.mmdl );
  }
  
  if( work->joint == NAMIPOKE_JOINT_ON ){ //揺れ
		work->shake_offs += work->shake_value;
		
		if( work->shake_offs >= NAMIPOKE_SHAKE_MAX ){
			work->shake_offs = NAMIPOKE_SHAKE_MAX;
			work->shake_value = -work->shake_value;
		}else if( work->shake_offs <= FX32_ONE ){
			work->shake_offs = FX32_ONE;
			work->shake_value = -work->shake_value;
		}
    
    { //運転手に揺れを追加
      oya_offs.x = 0;
      oya_offs.y = work->shake_offs + NAMIPOKE_RIDE_Y_OFFSET;
      oya_offs.z = NAMIPOKE_RIDE_Z_OFFSET;
      MMDL_SetVectorOuterDrawOffsetPos( work->head.mmdl, &oya_offs );
    }
  }

  { //座標
    #if 0  
    VecFx32 pos;
    MMDL_GetVectorPos( work->head.mmdl, &pos );
    pos.y += work->shake_offs - FX32_ONE;
    FLDEFF_TASK_SetPos( task, &pos );
    #else
    VecFx32 pos;
    MMDL_GetDrawVectorPos( work->head.mmdl, &pos );
    
    pos.x -= oya_offs.x;
    pos.y -= oya_offs.y;
    pos.z -= oya_offs.z;
    
    pos.y += work->shake_offs - FX32_ONE;
    FLDEFF_TASK_SetPos( task, &pos );
    #endif
  }
  
  if( work->joint == NAMIPOKE_JOINT_ONLY || work->ripple_off == TRUE ){
    RIPPLE_WORK *rip = &work->ripple_work;      //波を出さない
    rip->dir = DIR_NOT; //次回更新時に初期化
    
    if( work->ripple_off == TRUE ){
      rip->vanish_flag = TRUE;
    }
  }else{ //波エフェクト
    fx32 ret;
    VecFx32 pos;
    RIPPLE_WORK *rip = &work->ripple_work;
	  
    GFL_G3D_OBJECT_LoopAnimeFrame( rip->obj, 0, FX32_ONE );
	  GFL_G3D_OBJECT_LoopAnimeFrame( rip->obj, 1, FX32_ONE );
    
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
  }
}

#define RIPPLE_OFFS_Y (-0x4000)

//--------------------------------------------------------------
/**
 * 波乗りポケモンタスク　描画
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
//  波乗りポケモンタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_namipokeTaskHeader =
{
  sizeof(TASKWORK_NAMIPOKE),
  namipokeTask_Init,
  namipokeTask_Delete,
  namipokeTask_Update,
  namipokeTask_Draw,
};

//======================================================================
//  波乗りポケモンエフェクト　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト　初期化
 * @param	fectrl FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_NAMIPOKE_EFFECT_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_NAMIPOKE_EFFECT *npoke_eff;
	
	npoke_eff = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(FLDEFF_NAMIPOKE_EFFECT) );
	npoke_eff->fectrl = fectrl;
  
	npoke_eff_InitResource( npoke_eff );
	return( npoke_eff );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_NAMIPOKE_EFFECT_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_NAMIPOKE_EFFECT *npoke_eff = work;
  npoke_eff_DeleteResource( npoke_eff );
  GFL_HEAP_FreeMemory( npoke_eff );
}

//======================================================================
//	波乗りポケモンエフェクト　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りポケモン　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void npoke_eff_InitResource( FLDEFF_NAMIPOKE_EFFECT *npoke_eff )
{
  ARCHANDLE *handle;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( npoke_eff->fectrl );
  
  npoke_eff->g3d_res_mdl_taki_land =
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_land_nsbmd );
  GFL_G3D_TransVramTexture( npoke_eff->g3d_res_mdl_taki_land );
  npoke_eff->g3d_res_anm_taki_land_ima	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_land_nsbma );
  npoke_eff->g3d_res_anm_taki_land_ica	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_land_nsbca );
  
  npoke_eff->g3d_res_mdl_taki_start_f	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_start_f_nsbmd );
  GFL_G3D_TransVramTexture( npoke_eff->g3d_res_mdl_taki_start_f );
  npoke_eff->g3d_res_anm_taki_start_f_ica =
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_start_f_nsbca );
  npoke_eff->g3d_res_anm_taki_start_f_itp	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_start_f_nsbtp );

  npoke_eff->g3d_res_mdl_taki_loop_f =
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_roop_f_nsbmd );
  GFL_G3D_TransVramTexture( npoke_eff->g3d_res_mdl_taki_loop_f );
  npoke_eff->g3d_res_anm_taki_loop_f_ica =
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_roop_f_nsbca );
  npoke_eff->g3d_res_anm_taki_loop_f_itp	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_roop_f_nsbtp );

  npoke_eff->g3d_res_mdl_taki_start_s	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_start_s_nsbmd );
  GFL_G3D_TransVramTexture( npoke_eff->g3d_res_mdl_taki_start_s );
  npoke_eff->g3d_res_anm_taki_start_s_ica =
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_start_s_nsbca );
  npoke_eff->g3d_res_anm_taki_start_s_itp	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_start_s_nsbtp );
  
  npoke_eff->g3d_res_mdl_taki_loop_s =
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_loop_s_nsbmd );
  GFL_G3D_TransVramTexture( npoke_eff->g3d_res_mdl_taki_loop_s );
  npoke_eff->g3d_res_anm_taki_loop_s_ica =
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_loop_s_nsbca );
  npoke_eff->g3d_res_anm_taki_loop_s_itp	=
    GFL_G3D_CreateResourceHandle( handle, NARC_fldeff_taki_loop_s_nsbtp );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void npoke_eff_DeleteResource( FLDEFF_NAMIPOKE_EFFECT *npoke_eff )
{
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_mdl_taki_land );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_land_ima );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_land_ica );
  
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_mdl_taki_start_f );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_start_f_ica );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_start_f_itp );
  
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_mdl_taki_loop_f );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_loop_f_ica );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_loop_f_itp );
  
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_mdl_taki_start_s );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_start_s_ica );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_start_s_itp );
  
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_mdl_taki_loop_s );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_loop_s_ica );
  GFL_G3D_DeleteResource( npoke_eff->g3d_res_anm_taki_loop_s_itp );
}

//======================================================================
//  波乗りポケモンエフェクト　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト　追加
 * @param FLDEFF_CTRL*
 * @param type NAMIPOKE_EFFECT_TYPE 表示種類
 * @retval FLDEFF_TASK*
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_NAMIPOKE_EFFECT_SetEffect( FLDEFF_CTRL *fectrl,
    NAMIPOKE_EFFECT_TYPE type, const FLDEFF_TASK *efftask_namipoke )
{
  FLDEFF_TASK *task;
  FLDEFF_NAMIPOKE_EFFECT *npoke_eff;
  TASKHEADER_NAMIPOKE_EFFECT head;
  
  head.eff_npoke_eff = FLDEFF_CTRL_GetEffectWork(
      fectrl, FLDEFF_PROCID_NAMIPOKE_EFFECT );
  head.type = type;
  head.efftask_namipoke = efftask_namipoke;
  head.dead_flag = FALSE;

  task = FLDEFF_CTRL_AddTask(
      fectrl, &data_npoke_effTaskHeader, NULL, 0, &head, 1 );
  return( task );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト　追加　波乗りポケモンに依存しない座標指定型
 * @param FLDEFF_CTRL*
 * @param type NAMIPOKE_EFFECT_TYPE 表示種類
 * @param pos 表示する座標
 * @retval FLDEFF_TASK*
 * @attention NAMIPOKE_EFFECT_TYPE_TAKI_SPLASHのみエフェクト終了
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_NAMIPOKE_EFFECT_SetEffectAlone( FLDEFF_CTRL *fectrl,
    NAMIPOKE_EFFECT_TYPE type, const VecFx32 *pos )
{
  FLDEFF_TASK *task;
  FLDEFF_NAMIPOKE_EFFECT *npoke_eff;
  TASKHEADER_NAMIPOKE_EFFECT head;
  
  head.eff_npoke_eff = FLDEFF_CTRL_GetEffectWork(
      fectrl, FLDEFF_PROCID_NAMIPOKE_EFFECT );
  head.type = type;
  head.efftask_namipoke = NULL;
  head.dead_flag = TRUE;
  
  task = FLDEFF_CTRL_AddTask(
      fectrl, &data_npoke_effTaskHeader, pos, 0, &head, 1 );
  return( task );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクト　終了チェック
 * @param task FLDEFF_TASK
 * @retval BOOL TRUE=終了
 * @note NAMIPOKE_EFFECT_TYPE_TAKI_SPLASHのみ
 */
//--------------------------------------------------------------
BOOL FLDEFF_NAMIPOKE_EFFECT_CheckTaskEnd( const FLDEFF_TASK *task )
{
  TASKWORK_NAMIPOKE_EFFECT *work = FLDEFF_TASK_GetWork( (FLDEFF_TASK*)task );
  return( work->end_flag );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void npoke_effTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE_EFFECT *work = wk;
  const TASKHEADER_NAMIPOKE_EFFECT *head;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  
  switch( work->head.type ){
  case NAMIPOKE_EFFECT_TYPE_TAKI_SPLASH:
    work->obj_rnd = GFL_G3D_RENDER_Create(
        work->head.eff_npoke_eff->g3d_res_mdl_taki_land, 0,
        work->head.eff_npoke_eff->g3d_res_mdl_taki_land );
    work->obj_anm[0] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_land_ima, 0 );
    work->obj_anm[1] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_land_ica, 0 );
    break;
  case NAMIPOKE_EFFECT_TYPE_TAKI_START_F:
    work->obj_rnd = GFL_G3D_RENDER_Create(
        work->head.eff_npoke_eff->g3d_res_mdl_taki_start_f, 0,
        work->head.eff_npoke_eff->g3d_res_mdl_taki_start_f );
    work->obj_anm[0] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_start_f_ica, 0 );
    work->obj_anm[1] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_start_f_itp, 0 );
    break;
  case NAMIPOKE_EFFECT_TYPE_TAKI_LOOP_F:
    work->obj_rnd = GFL_G3D_RENDER_Create(
        work->head.eff_npoke_eff->g3d_res_mdl_taki_loop_f, 0,
        work->head.eff_npoke_eff->g3d_res_mdl_taki_loop_f );
    work->obj_anm[0] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_loop_f_ica, 0 );
    work->obj_anm[1] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_loop_f_itp, 0 );
    break;
  case NAMIPOKE_EFFECT_TYPE_TAKI_START_S:
    work->obj_rnd = GFL_G3D_RENDER_Create(
        work->head.eff_npoke_eff->g3d_res_mdl_taki_start_s, 0,
        work->head.eff_npoke_eff->g3d_res_mdl_taki_start_s );
    work->obj_anm[0] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_start_s_ica, 0 );
    work->obj_anm[1] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_start_s_itp, 0 );
    break;
  case NAMIPOKE_EFFECT_TYPE_TAKI_LOOP_S:
    work->obj_rnd = GFL_G3D_RENDER_Create(
        work->head.eff_npoke_eff->g3d_res_mdl_taki_loop_s, 0,
        work->head.eff_npoke_eff->g3d_res_mdl_taki_loop_s );
    work->obj_anm[0] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_loop_s_ica, 0 );
    work->obj_anm[1] = GFL_G3D_ANIME_Create( work->obj_rnd,
        work->head.eff_npoke_eff->g3d_res_anm_taki_loop_s_itp, 0 );
    break;
  }
  
  work->obj = GFL_G3D_OBJECT_Create(
      work->obj_rnd, work->obj_anm, 2 );
  GFL_G3D_OBJECT_EnableAnime( work->obj, 0 );
  GFL_G3D_OBJECT_EnableAnime( work->obj, 1 );
  
  if( work->head.efftask_namipoke != NULL ){
    VecFx32 pos;
    VecFx32 tbl[NAMIPOKE_EFFECT_TYPE_MAX] =
    {
      {0,0,0},
      {0,0,0},
      {0,NUM_FX32(-4),0},
      {0,0,0},
      {0,0,0},
    };
    
    work->offset = tbl[work->head.type];
    
    FLDEFF_TASK_GetPos( work->head.efftask_namipoke, &pos );
    pos.x += work->offset.x;
    pos.y += work->offset.y;
    pos.z += work->offset.z;
    FLDEFF_TASK_SetPos( task, &pos );
  }
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void npoke_effTask_Delete( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE_EFFECT *work = wk;
  GFL_G3D_ANIME_Delete( work->obj_anm[0] );
  GFL_G3D_ANIME_Delete( work->obj_anm[1] );
  GFL_G3D_OBJECT_Delete( work->obj );
	GFL_G3D_RENDER_Delete( work->obj_rnd );
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void npoke_effTask_Update( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_NAMIPOKE_EFFECT *work = wk;
  
  if( work->end_flag == TRUE && work->head.dead_flag == TRUE ){
    FLDEFF_TASK_CallDelete( task );
    return;
  }
  
  if( work->head.type == NAMIPOKE_EFFECT_TYPE_TAKI_SPLASH ){
    if( work->end_flag == FALSE ){
      BOOL ret0,ret1;
      ret0 = GFL_G3D_OBJECT_IncAnimeFrame( work->obj, 0, FX32_ONE );
      ret1 = GFL_G3D_OBJECT_IncAnimeFrame( work->obj, 1, FX32_ONE );
      
      if( ret0 == FALSE || ret1 == FALSE ){
        work->end_flag = TRUE;
      }
    }
  }else{
	  GFL_G3D_OBJECT_LoopAnimeFrame( work->obj, 0, FX32_ONE );
	  GFL_G3D_OBJECT_LoopAnimeFrame( work->obj, 1, FX32_ONE );
  }
  
  if( work->head.efftask_namipoke != NULL ){
    VecFx32 pos;
    FLDEFF_TASK_GetPos( work->head.efftask_namipoke, &pos );
    pos.x += work->offset.x;
    pos.y += work->offset.y;
    pos.z += work->offset.z;
    FLDEFF_TASK_SetPos( task, &pos );
  }
}

//--------------------------------------------------------------
/**
 * 波乗りポケモンエフェクトタスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void npoke_effTask_Draw( FLDEFF_TASK *task, void *wk )
{
  VecFx32 pos;
  TASKWORK_NAMIPOKE_EFFECT *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
}

//--------------------------------------------------------------
/// 波乗りポケモンエフェクトタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER data_npoke_effTaskHeader = 
{
  sizeof(TASKWORK_NAMIPOKE_EFFECT),
  npoke_effTask_Init,
  npoke_effTask_Delete,
  npoke_effTask_Update,
  npoke_effTask_Draw,
};
