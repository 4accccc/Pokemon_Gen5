//======================================================================
/**
 * @file  fldeff_reflect.c
 * @brief  フィールド 映り込み
 * @author  kagaya
 * @date  05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_reflect.h"

//======================================================================
//  define
//======================================================================
#ifdef DEBUG_ONLY_FOR_kagaya
//#define DEBUG_REFLECT_CHECK
#endif

#define REF_SCALE_X_UP (FX16_ONE/4)
#define REF_SCALE_X_DOWN (-FX16_ONE/4)
#define REF_SCALE_X_SPEED (FX16_ONE/64)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///  FLDEFF_REFLECT型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_REFLECT FLDEFF_REFLECT;

//--------------------------------------------------------------
///  FLDEFF_REFLECT構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_REFLECT
{
  FLDEFF_CTRL *fectrl;

#ifdef DEBUG_REFLECT_CHECK
  int d_select;
  VecFx32 d_offs[MMDL_BLACT_MDLSIZE_MAX];
#endif
};

//--------------------------------------------------------------
/// TASKHEADER_REFLECT
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_REFLECT *eff_reflect;
  REFLECT_TYPE type;
  MMDL *mmdl;
  MMDLSYS *mmdlsys;
  GFL_BBDACT_SYS *bbdactsys;
}TASKHEADER_REFLECT;

//--------------------------------------------------------------
/// TASKWORK_REFLECT
//--------------------------------------------------------------
typedef struct
{
  fx32 scale_x_org;
  fx32 scale_y_org;
  fx32 scale_x;
  fx32 scale_val_x;
  u8 flag_initact;
  u8 flag_initfunc;
  u8 padding[2];
  TASKHEADER_REFLECT head;
  MMDL_BLACTWORK_USER actWork;
  MMDL_CHECKSAME_DATA samedata;
}TASKWORK_REFLECT;

//======================================================================
//  プロトタイプ
//======================================================================
static void reflect_InitResource( FLDEFF_REFLECT *reflect );
static void reflect_DeleteResource( FLDEFF_REFLECT *reflect );

static void reflectTask_UpdateBlAct( u16 actID, void *wk );
static void reflectBlAct_Update(
    GFL_BBDACT_SYS *bbdactsys, int actID, void *wk );

static const FLDEFF_TASK_HEADER data_reflectTaskHeader;

#ifdef PM_DEBUG
static BOOL debug_CheckMMdl( const MMDL *mmdl );
#endif

//======================================================================
//  映り込み　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 映り込み 初期化
 * @param  fectrl    FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval  void*  エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_REFLECT_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
  FLDEFF_REFLECT *reflect;
  
  reflect = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_REFLECT) );
  reflect->fectrl = fectrl;
  
  reflect_InitResource( reflect );

#ifdef DEBUG_REFLECT_CHECK
  reflect->d_select = MMDL_BLACT_MDLSIZE_MAX;
#endif
  return( reflect );
}

//--------------------------------------------------------------
/**
 * 映り込み 削除
 * @param fectrl FLDEFF_CTRL
 * @param  work  エフェクト使用ワーク
 * @retval  nothing
 */
//--------------------------------------------------------------
void FLDEFF_REFLECT_Delete( FLDEFF_CTRL *fectrl, void *work )
{
  FLDEFF_REFLECT *reflect = work;
  reflect_DeleteResource( reflect );
  GFL_HEAP_FreeMemory( reflect );
}

//======================================================================
//  映り込み　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 映り込み　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void reflect_InitResource( FLDEFF_REFLECT *reflect )
{
}

//--------------------------------------------------------------
/**
 * 映り込み　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void reflect_DeleteResource( FLDEFF_REFLECT *reflect )
{
}

//======================================================================
//  映り込み　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル用映り込み　追加
 * @param mmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_REFLECT_SetMMdl( MMDLSYS *mmdlsys,
    MMDL *mmdl, FLDEFF_CTRL *fectrl, REFLECT_TYPE type )
{
  TASKHEADER_REFLECT head;

  head.type = type;
  head.mmdl = mmdl;
  head.mmdlsys = mmdlsys;
  head.bbdactsys = MMDL_BLACTCONT_GetBbdActSys(
      MMDLSYS_GetBlActCont(mmdlsys) );
  head.eff_reflect = FLDEFF_CTRL_GetEffectWork(
      fectrl, FLDEFF_PROCID_REFLECT );
   
#ifdef PM_DEBUG
  debug_CheckMMdl( mmdl );
#endif

  FLDEFF_CTRL_AddTask( fectrl, &data_reflectTaskHeader, NULL, 0, &head, 0 );
}

//--------------------------------------------------------------
/**
 * 映り込みタスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void reflectTask_Init( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_REFLECT *work = wk;
  const TASKHEADER_REFLECT *head;
  head = FLDEFF_TASK_GetAddPointer( task );
  
  work->head = *head;
  work->scale_x_org = 0;
  work->scale_x = 0;
  work->scale_val_x = REF_SCALE_X_SPEED;
  
  if( work->head.type == REFLECT_TYPE_MIRROR ){ //伸縮無し
    work->scale_val_x = 0;
  }
  
  MMDL_InitCheckSameData( head->mmdl, &work->samedata ); 
  
#ifdef PM_DEBUG
  debug_CheckMMdl( work->head.mmdl );
#endif
  
//  //即動作...は親のフラグ初期化タイミングが間に合わない事を考慮して無し
//  FLDEFF_TASK_CallUpdate( task );
}

//--------------------------------------------------------------
/**
 * 映り込みタスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void reflectTask_Delete( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_REFLECT *work = wk;
  
  if( work->flag_initact ){
    work->flag_initact = FALSE;
    MMDL_BLACTCONT_USER_DeleteActor( work->head.mmdlsys, &work->actWork );
  }
}

//--------------------------------------------------------------
/**
 * 映り込みタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void reflectTask_Update( FLDEFF_TASK *task, void *wk )
{
  u32 actID;
  TASKWORK_REFLECT *work = wk;
  
  if( MMDL_CheckSameData(work->head.mmdl,&work->samedata) == FALSE ||
    MMDL_CheckMoveBitReflect(work->head.mmdl) == FALSE ){
    FLDEFF_TASK_CallDelete( task );
    return;
  }
  
#ifdef PM_DEBUG
  if( debug_CheckMMdl(work->head.mmdl) == TRUE ){
    return;
  }
#endif

  actID = MMDL_CallDrawGetProc( work->head.mmdl, 0 );
  
  if( work->flag_initact == FALSE ){
    if( actID == MMDL_BLACTID_NULL ){ //動作モデルビルボード追加まだ
      return;
    }else{
      VecFx32 pos = {0,0,0};
      const OBJCODE_PARAM *param = MMDL_GetOBJCodeParam( work->head.mmdl );
      
      MMDL_BLACTCONT_USER_AddActor( work->head.mmdlsys,
          param, &work->actWork, &pos,
          reflectTask_UpdateBlAct, work );
      
      work->flag_initact = TRUE;
    }
  }
  
  if( work->actWork.actID == MMDL_BLACTID_NULL ){ //アクター追加まだ
    return;
  }
  
#if 0  
  if( work->flag_initfunc == FALSE ){
    GFL_BBDACT_SetFunc( work->head.bbdactsys,
        work->actWork.actID, reflectBlAct_Update );
    work->flag_initfunc = TRUE;
  }
#else
  if( work->flag_initfunc == FALSE ){
    u16 size_x,size_y;
    MMDL_BLACTCONT_GetMMdlDrawSize( work->head.mmdl, &size_x, &size_y );
    work->scale_x_org = size_x;
    work->scale_y_org = size_y;
    work->scale_x = work->scale_x_org;
    work->flag_initfunc = TRUE;
  }
#endif
  
  work->scale_x += work->scale_val_x;
  
 	if( work->scale_x >= (work->scale_x_org+REF_SCALE_X_UP) ){
		work->scale_x = work->scale_x_org + REF_SCALE_X_UP;
		work->scale_val_x = -work->scale_val_x;
	}else if( work->scale_x <= (work->scale_x_org+REF_SCALE_X_DOWN) ){
		work->scale_x = work->scale_x_org + REF_SCALE_X_DOWN;
		work->scale_val_x = -work->scale_val_x;
	}
	 
//reflectTask_UpdateBlAct( work->actWork.actID, work );
} 

//--------------------------------------------------------------
/**
 * 映り込みタスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void reflectTask_Draw( FLDEFF_TASK *task, void *wk )
{
#if 1
  TASKWORK_REFLECT *work = wk;
  
  //ここでアップデートすればいけると思う。
  if( MMDL_CheckSameData(work->head.mmdl,&work->samedata) == TRUE ){
    if( work->flag_initact ){ //初期化済み
      if( work->actWork.actID != MMDL_BLACTID_NULL ){ //アクターあり
        reflectTask_UpdateBlAct( work->actWork.actID, work );
      }
    }
  }
#endif
}

//--------------------------------------------------------------
/**
 * 映り込みタスク 動作モデルとの同期
 * @param actID ビルボードアクターID
 * @param wk TASKWORK_REFLECT
 * @retval nothing
 */
//--------------------------------------------------------------
static const fx32 data_TypeOffsetY[REFLECT_TYPE_MAX] =
{
  -(FX32_ONE*24+FX32_ONE),
  -(FX32_ONE*32+FX32_ONE),
  -(FX32_ONE*24+FX32_ONE),
};

//-0xe800, 0

static const fx32 data_MdlOffsetY[MMDL_BLACT_MDLSIZE_MAX] =
{
  -0x5800, //32x32
  -0x5800/2, //16x16
  -0x14800, //64x64
};

//#define REF_OFFS_Z (FX32_ONE*12)
#define REF_OFFS_Z (0)

static const fx32 data_MdlOffsetZ[MMDL_BLACT_MDLSIZE_MAX] =
{
  0x7800, //32x32
  0x7800/2, //16x32
  0xc000, //64x64
};

static void reflectTask_UpdateBlAct( u16 actID, void *wk )
{
  u16 ret;
  TASKWORK_REFLECT *work = wk;
  GFL_BBDACT_SYS *bbdactsys = work->head.bbdactsys;
  GFL_BBD_SYS *bbdsys = GFL_BBDACT_GetBBDSystem( bbdactsys );
  u32 m_actID = MMDL_CallDrawGetProc( work->head.mmdl, 0 );
  
  if( actID == MMDL_BLACTID_NULL ){ //親モデルビルボード無し。
    GF_ASSERT( 0 );
    return;
  }
  
  ret = GFL_BBDACT_GetDrawEnable( bbdactsys, m_actID );
  GFL_BBDACT_SetDrawEnable( bbdactsys, actID, ret );
  GFL_BBDACT_SetAnimeEnable( bbdactsys, actID, FALSE );
  
  {
    fx32 y;
    VecFx32 pos,offs0,offs1;
    const MMDL *mmdl;
    const OBJCODE_PARAM *param;
    
    mmdl = work->head.mmdl;
    param = MMDL_GetOBJCodeParam( mmdl );
    
    MMDL_GetVectorPos( mmdl, &pos );
    
    MMDL_GetVectorDrawOffsetPos( mmdl, &offs0 );
    offs0.y = -offs0.y; //yオフセットは無視する
    offs0.z = -offs0.z; //flip
    
    MMDL_GetVectorOuterDrawOffsetPos( mmdl, &offs1 );
    offs1.y = -offs1.y; //flip
    offs1.z = -offs1.z; //flip
    
    VEC_Add( &offs0, &offs1, &offs0 );
    
    offs0.y += data_TypeOffsetY[work->head.type];
    offs0.y += data_MdlOffsetY[param->mdl_size];
    offs0.z += data_MdlOffsetZ[param->mdl_size];
    
#ifdef DEBUG_REFLECT_CHECK
    if( MMDL_GetOBJID(work->head.mmdl) == MMDL_ID_PLAYER ){
      int select;
      int printf = 0;
      int repeat = GFL_UI_KEY_GetRepeat();
      int cont = GFL_UI_KEY_GetCont();
      int trg = GFL_UI_KEY_GetTrg();
      VecFx32 *d_offs;
      
      if( trg & PAD_BUTTON_START ){
        int *pFlag = &work->head.eff_reflect->d_select;
        
        (*pFlag)++;
        if( (*pFlag) > MMDL_BLACT_MDLSIZE_MAX ){
          (*pFlag) = 0;
        }
        
        KAGAYA_Printf( "reflect操作 " );
        
        switch( (*pFlag) ){
        case MMDL_BLACT_MDLSIZE_32x32:
          KAGAYA_Printf( "32x32\n" );
          break;
        case MMDL_BLACT_MDLSIZE_16x16:
          KAGAYA_Printf( "16x16\n" );
          break;
        case MMDL_BLACT_MDLSIZE_64x64:
          KAGAYA_Printf( "64x64\n" );
          break;
        default:
          KAGAYA_Printf( "OFF\n" );
        }
      }
      
      if( work->head.eff_reflect->d_select < MMDL_BLACT_MDLSIZE_MAX ){
        d_offs =
          &work->head.eff_reflect->d_offs[work->head.eff_reflect->d_select];
        
        if( repeat & PAD_BUTTON_L ){
          if( cont & PAD_BUTTON_B ){
            d_offs->z -= 0x800;
            printf = TRUE;
          }else{
            d_offs->y -= 0x800;
            printf = TRUE;
          }
        }else if( repeat & PAD_BUTTON_R ){
          if( cont & PAD_BUTTON_B ){
            d_offs->z += 0x800;
            printf = TRUE;
          }else{
            d_offs->y += 0x800;
            printf = TRUE;
          }
        }
      }
      
      if( (trg & PAD_BUTTON_A) || printf ){
        int i;
        d_offs = work->head.eff_reflect->d_offs;

        for( i = 0; i < MMDL_BLACT_MDLSIZE_MAX; i++, d_offs++ ){
          KAGAYA_Printf( "reflect " );
          
          switch( i ){
          case MMDL_BLACT_MDLSIZE_32x32:
            KAGAYA_Printf( "32x32 " );
            break;
          case MMDL_BLACT_MDLSIZE_16x16:
            KAGAYA_Printf( "16x16 " );
            break;
          case MMDL_BLACT_MDLSIZE_64x64:
            KAGAYA_Printf( "64x64 " );
            break;
          }

          KAGAYA_Printf( "X=0x%x,Y=0x%x,Z=0x%x\n",
              d_offs->x, d_offs->y, d_offs->z );
        }
      }
    }
    
    {
      VecFx32 d_offs = work->head.eff_reflect->d_offs[param->mdl_size];
      VEC_Add( &offs0, &d_offs, &offs0 );
    }

    #if 0
    {
      VecFx32 camPos,camUp,target,c_offs;
      MtxFx43 mtx43;
      MtxFx33 mtx33; 
      FIELDMAP_WORK *fieldmap =
        FLDEFF_CTRL_GetFieldMapWork( work->head.eff_reflect->fectrl );
      FIELD_CAMERA *fld_cam =
        FIELDMAP_GetFieldCamera( fieldmap );
      const GFL_G3D_CAMERA * g3Dcamera =
        FIELD_CAMERA_GetCameraPtr( fld_cam );
      
      GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
      GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
      GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );
      
      MTX_LookAt( &camPos, &camUp, &target, &mtx43 ); //カメラ行列取得
      MTX_Copy43To33( &mtx43, &mtx33 ); //回転行列である3x3部分を取得
      MTX_Inverse33( &mtx33, &mtx33 ); //反転し逆行列にする
      
      //逆行列で回転を無効化し平行化
      MTX_MultVec33( &offs0, &mtx33, &c_offs );
      offs0.x = c_offs.x;
      offs0.y = c_offs.y;
      offs0.z = c_offs.z;
    }
    #endif    
#endif
    
    VEC_Add( &offs0, &pos, &pos );
    GFL_BBD_SetObjectTrans( bbdsys, actID, &pos );
  }
  
  {
    BOOL flip = TRUE;
    int m_idx = GFL_BBDACT_GetBBDActIdxResIdx( bbdactsys, m_actID );
    int idx = GFL_BBDACT_GetBBDActIdxResIdx( bbdactsys, actID );
    fx16 sx = work->scale_x;
    fx16 sy = work->scale_y_org;
    
    {
      u16 res_idx = 0;
      u16 cell_idx = 0;
      GFL_BBD_GetObjectResIdx( bbdsys, m_idx, &res_idx );
      GFL_BBD_SetObjectResIdx( bbdsys, idx, &res_idx );
      
			GFL_BBD_GetObjectCelIdx( bbdsys, m_idx, &cell_idx );
			GFL_BBD_SetObjectCelIdx( bbdsys, idx, &cell_idx );
    }
    
    GFL_BBD_SetObjectSiz( bbdsys, idx, &sx, &sy );
    GFL_BBD_SetObjectFlipT( bbdsys, idx, &flip );
    
    flip = GFL_BBD_GetObjectFlipS( bbdsys, m_idx ); //横Flip受け継ぐ
    GFL_BBD_SetObjectFlipS( bbdsys, idx, &flip );
  }
}

/*
 * y軸補正だけですむならばズレない版
 * WBのカメラアングルによるビルボード補正と水面地形モデルの造りでは
 * Z方向の調整が必要なため難しい。
 */
#if 0
static const fx32 data_TypeOffsetY[REFLECT_TYPE_MAX] =
{
  0,
  0,
  0,
};

static const fx32 data_MdlOffsetY[MMDL_BLACT_MDLSIZE_MAX] =
{
  0, //32x32
  0, //16x16
  0, //64x64
};

static const fx32 data_offsetZ[MMDL_BLACT_MDLSIZE_MAX] =
{
  0, //32x32
  0, //16x32
  0, //64x64
};

static void reflectTask_UpdateBlAct( u16 actID, void *wk )
{
  u16 ret;
  TASKWORK_REFLECT *work = wk;
  GFL_BBDACT_SYS *bbdactsys = work->head.bbdactsys;
  GFL_BBD_SYS *bbdsys = GFL_BBDACT_GetBBDSystem( bbdactsys );
  u32 m_actID = MMDL_CallDrawGetProc( work->head.mmdl, 0 );
  
  if( actID == MMDL_BLACTID_NULL ){ //親モデルビルボード無し。
    GF_ASSERT( 0 );
    return;
  }
  
  ret = GFL_BBDACT_GetDrawEnable( bbdactsys, m_actID );
  GFL_BBDACT_SetDrawEnable( bbdactsys, actID, ret );
  GFL_BBDACT_SetAnimeEnable( bbdactsys, actID, FALSE );
  
  {
    fx32 y;
    VecFx32 pos,offs0,offs1;
    const MMDL *mmdl;
    const OBJCODE_PARAM *param;
    
    mmdl = work->head.mmdl;
    param = MMDL_GetOBJCodeParam( mmdl );
    
    MMDL_GetVectorPos( mmdl, &pos );
    
    MMDL_GetVectorDrawOffsetPos( mmdl, &offs0 );
    offs0.y = 0; //yオフセットは無視する
    offs0.z = -offs0.z; //flip
    
    MMDL_GetVectorOuterDrawOffsetPos( mmdl, &offs1 );
    offs1.y = -offs1.y; //flip
    offs1.z = -offs1.z; //flip
    
    VEC_Add( &offs0, &offs1, &offs0 );
    
    offs0.y += data_TypeOffsetY[work->head.type];
    offs0.y += data_MdlOffsetY[param->mdl_size];
    offs0.z += data_MdlOffsetZ[param->mdl_size];
    
#ifdef DEBUG_REFLECT_CHECK
    if( MMDL_GetOBJID(work->head.mmdl) == MMDL_ID_PLAYER ){
      int printf = 0;
      int repeat = GFL_UI_KEY_GetRepeat();
      int cont = GFL_UI_KEY_GetCont();
      int trg = GFL_UI_KEY_GetTrg();
      VecFx32 *d_offs = &(work->head.eff_reflect->d_offs);
      
      if( trg & PAD_BUTTON_START ){
        work->head.eff_reflect->d_select ^= 1;
        KAGAYA_Printf( "リフレクト座標操作機能 " );
        
        if( work->head.eff_reflect->d_select ){
          KAGAYA_Printf( "ON\n" );
        }else{
          KAGAYA_Printf( "OFF\n" );
        }
      }
      
      if( work->head.eff_reflect->d_select ){
        if( repeat & PAD_BUTTON_L ){
          if( cont & PAD_BUTTON_B ){
            d_offs->z -= 0x800;
            printf = TRUE;
          }else{
            d_offs->y -= 0x800;
            printf = TRUE;
          }
        }else if( repeat & PAD_BUTTON_R ){
          if( cont & PAD_BUTTON_B ){
            d_offs->z += 0x800;
            printf = TRUE;
          }else{
            d_offs->y += 0x800;
            printf = TRUE;
          }
        }
      }
      
      if( printf ){
        KAGAYA_Printf( "リフレクト X=0x%x,Y=0x%x,Z=0x%x\n",
            d_offs->x, d_offs->y, d_offs->z );
      }
    }
    
    {
      VecFx32 *d_offs = &(work->head.eff_reflect->d_offs);
      VEC_Add( &offs0, d_offs, &offs0 );
    }
    
    #if 1
    {
      VecFx32 camPos,camUp,target,c_offs;
      MtxFx43 mtx43;
      MtxFx33 mtx33; 
      FIELDMAP_WORK *fieldmap =
        FLDEFF_CTRL_GetFieldMapWork( work->head.eff_reflect->fectrl );
      FIELD_CAMERA *fld_cam =
        FIELDMAP_GetFieldCamera( fieldmap );
      const GFL_G3D_CAMERA * g3Dcamera =
        FIELD_CAMERA_GetCameraPtr( fld_cam );
      
      GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
      GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
      GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );
      
      MTX_LookAt( &camPos, &camUp, &target, &mtx43 ); //カメラ行列取得
      MTX_Copy43To33( &mtx43, &mtx33 ); //回転行列である3x3部分を取得
      MTX_Inverse33( &mtx33, &mtx33 ); //反転し逆行列にする
      
      //逆行列で回転を無効化し平行化
      MTX_MultVec33( &offs0, &mtx33, &c_offs );
      offs0.x = c_offs.x;
      offs0.y = c_offs.y;
    }
    #endif    
#endif
    
    VEC_Add( &offs0, &pos, &pos );
    GFL_BBD_SetObjectTrans( bbdsys, actID, &pos );
  }
  
  {
    BOOL flip = TRUE;
    int m_idx = GFL_BBDACT_GetBBDActIdxResIdx( bbdactsys, m_actID );
    int idx = GFL_BBDACT_GetBBDActIdxResIdx( bbdactsys, actID );
    fx16 sx = work->scale_x;
    fx16 sy = work->scale_y_org;
    
    {
      u16 res_idx = 0;
      u16 cell_idx = 0;
      GFL_BBD_GetObjectResIdx( bbdsys, m_idx, &res_idx );
      GFL_BBD_SetObjectResIdx( bbdsys, idx, &res_idx );
      
			GFL_BBD_GetObjectCelIdx( bbdsys, m_idx, &cell_idx );
			GFL_BBD_SetObjectCelIdx( bbdsys, idx, &cell_idx );
    }
    
    GFL_BBD_SetObjectSiz( bbdsys, idx, &sx, &sy );
    GFL_BBD_SetObjectFlipT( bbdsys, idx, &flip );
    
    flip = GFL_BBD_GetObjectFlipS( bbdsys, m_idx ); //横Flip受け継ぐ
    GFL_BBD_SetObjectFlipS( bbdsys, idx, &flip );
  }
}
#endif

static void reflectBlAct_Update(
    GFL_BBDACT_SYS *bbdactsys, int actID, void *wk )
{
  reflectTask_UpdateBlAct( actID, wk );
}

//======================================================================
//  data
//======================================================================
//--------------------------------------------------------------
//  映り込みタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER data_reflectTaskHeader =
{
  sizeof(TASKWORK_REFLECT),
  reflectTask_Init,
  reflectTask_Delete,
  reflectTask_Update,
  reflectTask_Draw,
};

//--------------------------------------------------------------
/// 映り込み表示位置
//--------------------------------------------------------------
#if 0 //old
static const fx32 data_TypeOffsetY[REFLECT_TYPE_MAX] =
{
  -(NUM_FX32(12*2)+NUM_FX32(1)),
  -(NUM_FX32(16*2)+NUM_FX32(1)),
  -(NUM_FX32(12*2)+NUM_FX32(1)),
};

static const fx32 data_offsetZ[MMDL_BLACT_MDLSIZE_MAX] =
{
  REF_OFFS_Z, //32
  REF_OFFS_Z/2, //16
  REF_OFFS_Z*2, //64
};
#endif

//======================================================================
//  define
//======================================================================
#ifdef PM_DEBUG
//--------------------------------------------------------------
//  映り込み対象以外のOBJに対して
//  映り込みが発生してないかチェック
//--------------------------------------------------------------
static BOOL debug_CheckMMdl( const MMDL *mmdl )
{
  const OBJCODE_PARAM *prm = MMDL_GetOBJCodeParam( mmdl );
    
  if( prm->draw_type != MMDL_DRAWTYPE_BLACT ){
    GF_ASSERT( 0 && "FLDEFF REFLECT NOT REFLECT OBJ" );
    return( TRUE );
  }
  
  if( prm->reflect_type == MMDL_REFLECT_NON ){
    GF_ASSERT( 0 && "FLDEFF REFLECT NOT REFLECT OBJ" );
    return( TRUE );
  }
  return( FALSE );
}
#endif

