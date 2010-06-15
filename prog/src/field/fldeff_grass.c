//======================================================================
/**
 * @file	fldeff_grass.c
 * @brief	フィールド 草エフェクト
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_grass.h"

#include "include/gamesystem/pm_season.h"

//======================================================================
//  define
//======================================================================
#define GRASS_SHAKE_FRAME (FX32_ONE*12)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///	FLDEFF_GRASS型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_GRASS FLDEFF_GRASS;

//--------------------------------------------------------------
///	FLDEFF_GRASS構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_GRASS
{
	FLDEFF_CTRL *fectrl;
  GFL_BBDACT_SYS *bbdact_sys;
  u16 res_idx_tbl[FLDEFF_GRASS_MAX];
};

//--------------------------------------------------------------
/// TASKHEADER_GRASS
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_GRASS *eff_grass;
  MMDL *fmmdl;
  int init_gx;
  int init_gz;
  u16 obj_id;
  u16 zone_id;
  VecFx32 pos;
  FLDEFF_GRASSTYPE type;
}TASKHEADER_GRASS;

//--------------------------------------------------------------
/// TASKWORK_GRASS
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  TASKHEADER_GRASS head;
  MMDL_CHECKSAME_DATA samedata;
  u16 act_id;
}TASKWORK_GRASS;

//--------------------------------------------------------------
/// GRASS_ARCIDX
//--------------------------------------------------------------
typedef struct
{
  u16 idx_mdl;
}GRASS_ARCIDX;

//======================================================================
//	プロトタイプ
//======================================================================
static void grass_InitResource( FLDEFF_GRASS *grass );
static void grass_DeleteResource( FLDEFF_GRASS *grass );

static const FLDEFF_TASK_HEADER DATA_grassTaskHeader;

static BOOL check_MMdlPos( TASKWORK_GRASS *work );

static const GRASS_ARCIDX data_ArcIdxTbl[FLDEFF_GRASS_MAX][PMSEASON_TOTAL];
static const GFL_BBDACT_ANM * const * data_BlActAnm_GrassTbl[FLDEFF_GRASS_MAX];
static const u16 data_BlActAnmEndNo[FLDEFF_GRASS_MAX];
static const u8 data_LongShortType[FLDEFF_GRASS_MAX];

//======================================================================
//	草エフェクト　システム
//======================================================================
//--------------------------------------------------------------
/**
 * 草エフェクト 初期化
 * @param	fectrl		FLDEFF_CTRL *
 * @param heapID HEAPID
 * @retval	void*	エフェクト使用ワーク
 */
//--------------------------------------------------------------
void * FLDEFF_GRASS_Init( FLDEFF_CTRL *fectrl, HEAPID heapID )
{
	FLDEFF_GRASS *grass;
	FIELDMAP_WORK *fieldmap;

	grass = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_GRASS) );
	grass->fectrl = fectrl;
  
  fieldmap = FLDEFF_CTRL_GetFieldMapWork( grass->fectrl );
  grass->bbdact_sys = FIELDMAP_GetEffBbdActSys( fieldmap );
  
	grass_InitResource( grass );
	return( grass );
}

//--------------------------------------------------------------
/**
 * 草エフェクト 削除
 * @param fectrl FLDEFF_CTRL
 * @param	work	エフェクト使用ワーク
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDEFF_GRASS_Delete( FLDEFF_CTRL *fectrl, void *work )
{
	FLDEFF_GRASS *grass = work;
  grass_DeleteResource( grass );
  GFL_HEAP_FreeMemory( grass );
}

//======================================================================
//	草エフェクト　リソース
//======================================================================
//--------------------------------------------------------------
/**
 * 草エフェクト　リソース初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void grass_InitResource( FLDEFF_GRASS *grass )
{
  BOOL ret;
  int i,season;
  ARCHANDLE *handle;
  FIELDMAP_WORK *fieldmap;
  GAMESYS_WORK *gsys;
  GFL_BBDACT_G3DRESDATA data;
  
  handle = FLDEFF_CTRL_GetArcHandleEffect( grass->fectrl );
  fieldmap = FLDEFF_CTRL_GetFieldMapWork( grass->fectrl );
  gsys = FIELDMAP_GetGameSysWork( fieldmap );
  season = FLDEFF_CTRL_GetSeasonID( grass->fectrl );
  
  data.texFmt = GFL_BBD_TEXFMT_PAL16;
  data.texSiz = GFL_BBD_TEXSIZ_32x128;
  data.celSizX = 32;
  data.celSizY = 32;
  data.dataCut = GFL_BBDACT_RESTYPE_DATACUT;
  
  for( i = 0; i < FLDEFF_GRASS_MAX; i++ ){
    data.g3dres = GFL_G3D_CreateResourceHandle(
          handle, data_ArcIdxTbl[i][season].idx_mdl );
    FLDEFF_CTRL_SetGrayScaleG3DResource( grass->fectrl, data.g3dres );
    grass->res_idx_tbl[i] =
      GFL_BBDACT_AddResourceG3DResUnit( grass->bbdact_sys, &data, 1 );
  }
}

//--------------------------------------------------------------
/**
 * 草エフェクト　リソース削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void grass_DeleteResource( FLDEFF_GRASS *grass )
{
  int i;
  
  for( i = 0; i < FLDEFF_GRASS_MAX; i++ ){
    GFL_BBDACT_RemoveResourceUnit(
        grass->bbdact_sys, grass->res_idx_tbl[i], 1 );
  }
}

//======================================================================
//	草エフェクト　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * FLDEFF_GRASSTYPEからFLDEFF_GRASSLENを取得
 * @param type FLDEFF_GRASSTYPE
 * @retval FLDEFF_GRASSLEN
 */
//--------------------------------------------------------------
FLDEFF_GRASSLEN FLDEFF_GRASS_GetLenType( FLDEFF_GRASSTYPE type )
{
  u8 len = data_LongShortType[type];
  return( len );
}

//--------------------------------------------------------------
/**
 * 動作モデル用草エフェクト　追加
 * @param fmmdl MMDL
 * @param FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_GRASS_SetMMdl( FLDEFF_CTRL *fectrl,
    MMDL *fmmdl, BOOL anm, FLDEFF_GRASSTYPE type )
{
  fx32 h;
  VecFx32 pos;
  FLDEFF_GRASS *grass;
  TASKHEADER_GRASS head;
  
  if( type >= FLDEFF_GRASS_MAX ){
    GF_ASSERT( 0 );
    return;
  }
  
  grass = FLDEFF_CTRL_GetEffectWork( fectrl, FLDEFF_PROCID_GRASS );
  head.eff_grass = grass;
  head.fmmdl = fmmdl;
  head.type = type;
  head.obj_id = MMDL_GetOBJID( fmmdl );
  head.zone_id = MMDL_GetZoneID( fmmdl );
  head.init_gx = MMDL_GetGridPosX( fmmdl );
  head.init_gz = MMDL_GetGridPosZ( fmmdl );
  MMDL_TOOL_GetCenterGridPos( head.init_gx, head.init_gz, &pos );
  pos.y = MMDL_GetVectorPosY( fmmdl );
  
  // レール動作の設定
  if( MMDL_CheckStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE ) )
  {
    #if 0 //old
    pos.y += FX32_CONST(4);  
    #else
    pos.y += NUM_FX32(7);
    pos.z += NUM_FX32(-2);
    #endif
  }
  // グリッド動作の設定
  else
  {
    MMDL_GetMapPosHeight( fmmdl, &pos, &h );
    pos.y = h;

    /*
    pos.y += NUM_FX32(10); //ぴったりだが被る
    */
    pos.y += NUM_FX32(9);
    pos.z += NUM_FX32(-2);
  }
  
  head.pos = pos;
  
  FLDEFF_CTRL_AddTask(
      fectrl, &DATA_grassTaskHeader, NULL, anm, &head, 0 );
}

//--------------------------------------------------------------
/**
 * 草エフェクトタスク　初期化
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void grassTask_Init( FLDEFF_TASK *task, void *wk )
{
  u16 type;
  TASKWORK_GRASS *work = wk;
  const TASKHEADER_GRASS *head;
  GFL_BBDACT_ACTDATA actData;
  GFL_BBDACT_SYS *bbdact_sys;
  
  head = FLDEFF_TASK_GetAddPointer( task );
  work->head = *head;
  
  MMDL_InitCheckSameData( head->fmmdl, &work->samedata );
  FLDEFF_TASK_SetPos( task, &head->pos );
  
  type = work->head.type;
  
  actData.resID = work->head.eff_grass->res_idx_tbl[type];
  actData.sizX = FX16_ONE*2;
  actData.sizY = FX16_ONE*2;
  actData.alpha = 31;
  actData.drawEnable = TRUE;
  actData.lightMask = GFL_BBD_LIGHTMASK_0;
  actData.trans = head->pos;
  actData.func = NULL;
  actData.work = work;
  
  bbdact_sys = work->head.eff_grass->bbdact_sys;
  
  work->act_id = GFL_BBDACT_AddAct( bbdact_sys, 0, &actData, 1 );
  
  if( work->act_id == GFL_BBDACT_ACTUNIT_ID_INVALID ){
    FLDEFF_TASK_CallDelete( task );
    return;
  }
  
  GFL_BBDACT_SetAnimeTable( bbdact_sys, work->act_id,  
      (GFL_BBDACT_ANMTBL)data_BlActAnm_GrassTbl[type], 1 );
  GFL_BBDACT_SetAnimeIdxOn( bbdact_sys, work->act_id, 0 );
  
  if( FLDEFF_TASK_GetAddParam(task) == FALSE ){ //アニメ無し
    work->seq_no = 1;
  }
  
  FLDEFF_TASK_CallUpdate( task ); //即動作
}

//--------------------------------------------------------------
/**
 * 草エフェクトタスク　削除
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void grassTask_Delete( FLDEFF_TASK *task, void *wk )
{
  TASKWORK_GRASS *work = wk;
  GFL_BBDACT_SYS *bbdact_sys = work->head.eff_grass->bbdact_sys;
  if( work->act_id != GFL_BBDACT_ACTUNIT_ID_INVALID ){
    GFL_BBDACT_RemoveAct( bbdact_sys, work->act_id, 1 );
  }
}

//--------------------------------------------------------------
/**
 * 草エフェクトタスク　更新
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void grassTask_Update( FLDEFF_TASK *task, void *wk )
{
  u16 comm;
  TASKWORK_GRASS *work = wk;
  GFL_BBDACT_SYS *bbdact_sys = work->head.eff_grass->bbdact_sys;
 
  switch( work->seq_no ){
  case 0: //アニメ終了待ち
    #if 1 //BTS4949　草むらを歩いたとき不自然
    {
      //アニメ中でも、長い草で、下移動で座標が更新されたら即消し
      u8 len = data_LongShortType[work->head.type];
      
      if( len == FLDEFF_GRASSLEN_LONG ){
        MMDL *mmdl = work->head.fmmdl;
        
        if( MMDL_CheckSameData(mmdl,&work->samedata) == TRUE ){
          if( check_MMdlPos(work) == FALSE ){
            if( MMDL_GetDirMove(mmdl) == DIR_DOWN ){
              FLDEFF_TASK_CallDelete( task );
              return;
            }
          }
        }
      }
    }
    #endif

    if( GFL_BBDACT_GetAnimeLastCommand(bbdact_sys,work->act_id,&comm) ){
      work->seq_no++;
    }
    break;
  case 1: //アニメ終了位置へ
    if( MMDL_CheckSameData(work->head.fmmdl,&work->samedata) == FALSE ){
      FLDEFF_TASK_CallDelete( task );
      return;
    }
    
    GFL_BBDACT_SetAnimeFrmIdx(
        bbdact_sys, work->act_id, data_BlActAnmEndNo[work->head.type] );
    work->seq_no++;
    break;
  case 2: //アニメポーズ
    //ビルボードアクター更新によってはseq1の処理が反映されていない場合がある
    //アニメを終端まで流す事で対処
#if 0
    GFL_BBDACT_SetAnimeEnable( bbdact_sys, work->act_id, FALSE );
#endif
    work->seq_no++;
  case 3:
    if( MMDL_CheckSameData(work->head.fmmdl,&work->samedata) == FALSE ){
      FLDEFF_TASK_CallDelete( task );
      return;
    }
    
    if( check_MMdlPos(work) == FALSE ){
      FLDEFF_TASK_CallDelete( task );
    }
  }
}

//--------------------------------------------------------------
/**
 * 草エフェクトタスク　描画
 * @param task FLDEFF_TASK
 * @param wk task work
 * @retval nothing
 */
//--------------------------------------------------------------
static void grassTask_Draw( FLDEFF_TASK *task, void *wk )
{
#if 0 //field_g3dobjを利用する
  VecFx32 pos;
  TASKWORK_GRASS *work = wk;
  GFL_G3D_OBJSTATUS status = {{0},{FX32_ONE,FX32_ONE,FX32_ONE},{0}};
  
  MTX_Identity33( &status.rotate );
  FLDEFF_TASK_GetPos( task, &status.trans );
  GFL_G3D_DRAW_DrawObjectCullingON( work->obj, &status );
#endif
}

//--------------------------------------------------------------
//  草エフェクトタスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER DATA_grassTaskHeader =
{
  sizeof(TASKWORK_GRASS),
  grassTask_Init,
  grassTask_Delete,
  grassTask_Update,
  grassTask_Draw,
};

//--------------------------------------------------------------
/**
 * 草エフェクト用座標チェック処理
 * @param work TASKWORK_GRASS
 * @retval BOOL TRUE=座標変更なし FALSE=座標変更あり
 * 
 * 以下の対処の為の処理。
 * BTS4094【草むらを歩いた時の表示優先順位が不自然】
 * 草むらを画面上方向に歩くと、草が自キャラの手前に
 * 不自然に表示されるように見えます。
 * ・草むらA、Bで同様です。
 * ・長い草むらの場合も上方向に歩くと少しだけ不自然に見えます。
 * ・いずれの場合も草むらを下方向や横方向に歩いた場合はあまり目立ちません。
 */
//--------------------------------------------------------------
static BOOL check_MMdlPos( TASKWORK_GRASS *work )
{
  const MMDL *mmdl = work->head.fmmdl;
  int gx = MMDL_GetGridPosX( mmdl );
  int gz = MMDL_GetGridPosZ( mmdl );
  u8 len = data_LongShortType[work->head.type];
  
  if( work->head.init_gx != gx || work->head.init_gz != gz ){
    #if 1 //BTS4949　草むらを歩いたとき不自然
    //短い草で移動方向上向きならば、完全に座標が切り替わるまで待つ
    if( len == FLDEFF_GRASSLEN_SHORT ){
      if( MMDL_GetDirMove(mmdl) == DIR_UP ){
        gx = MMDL_GetOldGridPosX( mmdl );
        gz = MMDL_GetOldGridPosZ( mmdl );
        
        if( work->head.init_gx == gx || work->head.init_gz == gz ){
          return( TRUE );
        }
      }
    }
    #endif
    return( FALSE );
  }
  
  return( TRUE );
}

//======================================================================
//  data
//======================================================================
static const GRASS_ARCIDX data_ArcIdxTbl[FLDEFF_GRASS_MAX][PMSEASON_TOTAL] =
{
  { //短い草
    { NARC_fldeff_kusaeff_sp_nsbtx },
    { NARC_fldeff_kusaeff_sm_nsbtx },
    { NARC_fldeff_kusaeff_at_nsbtx },
    { NARC_fldeff_kusaeff_wt_nsbtx },
  },
  { //短い草 強
    { NARC_fldeff_kusaeff_sp2_nsbtx },
    { NARC_fldeff_kusaeff_sm2_nsbtx },
    { NARC_fldeff_kusaeff_at2_nsbtx },
    { NARC_fldeff_kusaeff_wt2_nsbtx },
  },
  { //長い草
    { NARC_fldeff_lgrass_sp_nsbtx },
    { NARC_fldeff_lgrass_sm_nsbtx },
    { NARC_fldeff_lgrass_at_nsbtx },
    { NARC_fldeff_lgrass_wt_nsbtx },
  },
  { //長い草 強
    { NARC_fldeff_lgrass_sp2_nsbtx },
    { NARC_fldeff_lgrass_sm2_nsbtx },
    { NARC_fldeff_lgrass_at2_nsbtx },
    { NARC_fldeff_lgrass_wt2_nsbtx },
  },
  { //豪雪地帯 １シーズンのみ
    { NARC_fldeff_kusaeff_sn_nsbtx },
    { NARC_fldeff_kusaeff_sn_nsbtx },
    { NARC_fldeff_kusaeff_sn_nsbtx },
    { NARC_fldeff_kusaeff_sn_nsbtx },
  },
  { //豪雪地帯 強 １シーズンのみ
    { NARC_fldeff_kusaeff_sn2_nsbtx },
    { NARC_fldeff_kusaeff_sn2_nsbtx },
    { NARC_fldeff_kusaeff_sn2_nsbtx },
    { NARC_fldeff_kusaeff_sn2_nsbtx },
  },
  { //四季変化なし草　弱
    { NARC_fldeff_kusaeff_sp_nsbtx },
    { NARC_fldeff_kusaeff_sp_nsbtx },
    { NARC_fldeff_kusaeff_sp_nsbtx },
    { NARC_fldeff_kusaeff_sp_nsbtx },
  },
  { //四季変化なし草　強
    { NARC_fldeff_kusaeff_sp2_nsbtx },
    { NARC_fldeff_kusaeff_sp2_nsbtx },
    { NARC_fldeff_kusaeff_sp2_nsbtx },
    { NARC_fldeff_kusaeff_sp2_nsbtx },
  },
};

//短い草アニメ
static const GFL_BBDACT_ANM data_BlActAnm_ShortGrass[] = {
  {0,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {1,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {2,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,3},
  {3,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,4},
  {GFL_BBDACT_ANMCOM_END,0,0,0},
};

static const GFL_BBDACT_ANM * data_BlActAnm_ShortGrassTbl[] =
{
  data_BlActAnm_ShortGrass,
};

//長い草アニメ
static const GFL_BBDACT_ANM data_BlActAnm_LongGrass[] = {
  {0,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,2},
  {1,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,2},
  {0,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,2},
  {1,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,2},
  {0,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,2},
  {1,GFL_BBDACT_ANMFLIP_OFF,GFL_BBDACT_ANMFLIP_OFF,2},
  {GFL_BBDACT_ANMCOM_END,0,0,0},
};

static const GFL_BBDACT_ANM * data_BlActAnm_LongGrassTbl[] =
{
  data_BlActAnm_LongGrass,
};

//種類別アニメ
static const GFL_BBDACT_ANM * const * data_BlActAnm_GrassTbl[FLDEFF_GRASS_MAX] =
{
  data_BlActAnm_ShortGrassTbl, //FLDEFF_GRASS_SHORT
  data_BlActAnm_ShortGrassTbl, //FLDEFF_GRASS_SHORT2
  data_BlActAnm_LongGrassTbl, //FLDEFF_GRASS_LONG
  data_BlActAnm_LongGrassTbl, //FLDEFF_GRASS_LONG2
  data_BlActAnm_ShortGrassTbl, //FLDEFF_GRASS_SNOW
  data_BlActAnm_ShortGrassTbl, //FLDEFF_GRASS_SNOW2
  data_BlActAnm_ShortGrassTbl, //FLDEFF_GRASS_ALLYEAR_SHORT
  data_BlActAnm_ShortGrassTbl, //FLDEFF_GRASS_ALLYEAR_SHORT2
};

//種類別アニメ終了位置
static const u16 data_BlActAnmEndNo[FLDEFF_GRASS_MAX] =
{
  3, //FLDEFF_GRASS_SHORT
  3, //FLDEFF_GRASS_SHORT2
  5, //FLDEFF_GRASS_LONG
  5, //FLDEFF_GRASS_LONG2
  3, //FLDEFF_GRASS_SNOW
  3, //FLDEFF_GRASS_SNOW2
  3, //FLDEFF_GRASS_ALLYEAR_SHORT
  3, //FLDEFF_GRASS_ALLYEAR_SHORT2
};

//種類別長短
static const u8 data_LongShortType[FLDEFF_GRASS_MAX] = 
{
  FLDEFF_GRASSLEN_SHORT, //FLDEFF_GRASS_SHORT
  FLDEFF_GRASSLEN_SHORT, //FLDEFF_GRASS_SHORT2
  FLDEFF_GRASSLEN_LONG, //FLDEFF_GRASS_LONG
  FLDEFF_GRASSLEN_LONG, //FLDEFF_GRASS_LONG2
  FLDEFF_GRASSLEN_SHORT, //FLDEFF_GRASS_SNOW
  FLDEFF_GRASSLEN_SHORT, //FLDEFF_GRASS_SNOW2
  FLDEFF_GRASSLEN_SHORT, //FLDEFF_GRASS_ALLYEAR_SHORT
  FLDEFF_GRASSLEN_SHORT, //FLDEFF_GRASS_ALLYEAR_SHORT2
};
