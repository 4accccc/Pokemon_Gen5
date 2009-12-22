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

#define REF_SCALE_X_DEF (FX16_ONE*4)
#define REF_SCALE_Y_DEF (FX16_ONE*4)
#define REF_SCALE_X_UP (REF_SCALE_X_DEF+(FX16_ONE/4))
#define REF_SCALE_X_DOWN (REF_SCALE_X_DEF-(FX16_ONE/4))
#define REF_SCALE_X_SPEED (FX16_ONE/64)

#if 0 //dp
#define REF_OFFS_Z (-FX32_ONE*7)
#else //wb
#define REF_OFFS_Z (FX32_ONE*12)
#endif

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
  fx32 scale_x;
  fx32 scale_val_x;
  u16 flag_initact;
  u16 flag_initfunc;
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
  work->scale_x = REF_SCALE_X_DEF;
  work->scale_val_x = REF_SCALE_X_SPEED;
  
  if( work->head.type == REFLECT_TYPE_MIRROR ){ //伸縮無し
    work->scale_val_x = 0;
  }
  
  MMDL_InitCheckSameData( head->mmdl, &work->samedata ); 
  
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
    MMDL_CheckStatusBitReflect(work->head.mmdl) == FALSE ){
    FLDEFF_TASK_CallDelete( task );
    return;
  }
  
  actID = MMDL_CallDrawGetProc( work->head.mmdl, 0 );

  if( work->flag_initact == FALSE ){
    if( actID == MMDL_BLACTID_NULL ){ //動作モデルビルボード追加まだ
      return;
    }else{
      VecFx32 pos = {0,0,0};
      u16 code = MMDL_GetOBJCode( work->head.mmdl );
      MMDL_BLACTCONT_USER_AddActor( work->head.mmdlsys,
          code, &work->actWork, &pos,
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
#endif
  
  work->scale_x += work->scale_val_x;
  
 	if( work->scale_x >= REF_SCALE_X_UP ){
		work->scale_x = REF_SCALE_X_UP;
		work->scale_val_x = -work->scale_val_x;
	}else if( work->scale_x <= REF_SCALE_X_DOWN ){
		work->scale_x = REF_SCALE_X_DOWN;
		work->scale_val_x = -work->scale_val_x;
	}
	 
  reflectTask_UpdateBlAct( work->actWork.actID, work );
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
// TASKWORK_REFLECT *work = wk;
}

//--------------------------------------------------------------
/**
 * 映り込みタスク 動作モデルとの同期
 * @param actID ビルボードアクターID
 * @param wk TASKWORK_REFLECT
 * @retval nothing
 */
//--------------------------------------------------------------
static void reflectTask_UpdateBlAct( u16 actID, void *wk )
{
  u16 ret;
  TASKWORK_REFLECT *work = wk;
  GFL_BBDACT_SYS *bbdactsys = work->head.bbdactsys;
  GFL_BBD_SYS *bbdsys = GFL_BBDACT_GetBBDSystem( bbdactsys );
  u32 m_actID = MMDL_CallDrawGetProc( work->head.mmdl, 0 );
  
  GF_ASSERT( actID != MMDL_BLACTID_NULL ); //親モデルビルボード無し。
  
  ret = GFL_BBDACT_GetDrawEnable( bbdactsys, m_actID );
  GFL_BBDACT_SetDrawEnable( bbdactsys, actID, ret );

#if 0  
  ret = GFL_BBDACT_GetAnimeEnable( bbdactsys, m_actID );
  GFL_BBDACT_SetAnimeEnable( bbdactsys, actID, ret );
  
  if( ret == TRUE ){
    ret = GFL_BBDACT_GetAnimeIdx( bbdactsys, m_actID );
    GFL_BBDACT_SetAnimeIdx( bbdactsys, actID, ret );
    ret = GFL_BBDACT_GetAnimeFrmIdx( bbdactsys, m_actID );
    GFL_BBDACT_SetAnimeFrmIdx( bbdactsys, actID, ret );
  }
#else
  GFL_BBDACT_SetAnimeEnable( bbdactsys, actID, FALSE );
#endif
  
  {
    fx32 x,y,z;
    VecFx32 pos;
 	  
    fx32 offs[REFLECT_TYPE_MAX] = {
	    NUM_FX32(12*2)+NUM_FX32(1),
		  NUM_FX32(16*2)+NUM_FX32(1),
		  NUM_FX32(12*2)+NUM_FX32(1),
	  };

    MMDL_GetVectorDrawOffsetPos( work->head.mmdl, &pos );
    x = pos.x;
    z = -pos.z;
    
    MMDL_GetVectorPos( work->head.mmdl, &pos );
    pos.x += x;
    pos.z += z + REF_OFFS_Z;
    
	  if( MMDL_GetMapPosHeight(work->head.mmdl,&pos,&y) == FALSE ){ 
      //高さ取得エラー
      #if 1
      pos.y = 0;
      #else
      pos.y -= offs[0];
      #endif
    }else{
      pos.y -= offs[work->head.type];
    }
    
    GFL_BBD_SetObjectTrans( bbdsys, actID, &pos );
  }
  
  {
    BOOL flip = TRUE;
    int m_idx = GFL_BBDACT_GetBBDActIdxResIdx( bbdactsys, m_actID );
    int idx = GFL_BBDACT_GetBBDActIdxResIdx( bbdactsys, actID );
    fx16 sx = work->scale_x;
    fx16 sy = REF_SCALE_Y_DEF;
    
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
  
  #ifdef DEBUG_REFLECT_CHECK
  if( MMDL_GetOBJID(work->head.mmdl) == 0xff ){
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A ){
      KAGAYA_Printf( "自機横サイズ 0x%x\n", work->scale_x );
    }
  }
  #endif
}

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
