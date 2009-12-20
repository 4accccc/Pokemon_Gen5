//======================================================================
/**
 *
 * @file  fieldobj_move.c
 * @brief  動作モデル 動作系
 * @author  kagaya
 * @date  05.07.25
 *
 */
//======================================================================
#include "fldmmdl.h"

#include "include/gamesystem/pm_season.h"

#include "fieldmap.h"

#include "fldeff_shadow.h"
#include "fldeff_kemuri.h"
#include "fldeff_grass.h"
#include "fldeff_footmark.h"
#include "fldeff_reflect.h"
#include "fldeff_ripple.h"
#include "fldeff_splash.h"
#include "fldeff_d06denki.h"

#include "sound/pm_sndsys.h"

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
//  debug
//--------------------------------------------------------------
#ifdef DEBUG_ONLY_FOR_kagaya
//#define DEBUG_REFLECT_CHECK //定義で映り込みチェック
#endif

//--------------------------------------------------------------
///  ステータスビット
//--------------------------------------------------------------
///移動動作を禁止するステータスビット
#define STA_BIT_MOVE_ERROR (MMDL_STABIT_HEIGHT_GET_ERROR|MMDL_STABIT_ATTR_GET_ERROR)

//--------------------------------------------------------------
///  アトリビュートオフセット
//--------------------------------------------------------------
#define ATTROFFS_Y_NUMA (NUM_FX32(-12))///<沼Yオフセット
#define ATTROFFS_Y_NUMA_DEEP (NUM_FX32(-14))///<深い沼Yオフセット
#define ATTROFFS_Y_YUKI (NUM_FX32(-12))///<雪Yオフセット
#define ATTROFFS_Y_YUKI_DEEP (NUM_FX32(-14))///<深い雪Yオフセット
#define ATTROFFS_Y_YUKI_DEEP_MOST (NUM_FX32(-16))///<更に深い雪Yオフセット
#define ATTROFFS_Y_DESERT_DEEP (NUM_FX32(-6)) ///<深い砂漠Yオフセット

///高さ差分による移動制限
#define HEIGHT_DIFF_COLLISION (FX32_ONE*(20))


//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// ATTRDATA
//--------------------------------------------------------------
typedef struct
{
  MAPATTR attr_old;
  MAPATTR attr_now;
  MAPATTR_VALUE attr_val_old;
  MAPATTR_VALUE attr_val_now;
  MAPATTR_FLAG attr_flag_old;
  MAPATTR_FLAG attr_flag_now;
  
  const OBJCODE_PARAM *objcode_prm;
  FLDEFF_CTRL *fectrl;
  
  u8 season;
  u8 padding[3];
}ATTRDATA;

//======================================================================
//  proto
//======================================================================
static int MMdl_CheckMoveStart( const MMDL * mmdl );
static void MMdl_GetHeightMoveBefore( MMDL * mmdl );
static void MMdl_GetAttrMoveBefore( MMDL * mmdl );
static void MMdl_ProcMoveStartFirst( MMDL * mmdl );
static void MMdl_ProcMoveStartSecond( MMDL * mmdl );
static void MMdl_ProcMoveEnd( MMDL * mmdl );

static void mmdl_InitAttrData( const MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrProc_MoveStartFirst( MMDL * mmdl );
static void MMdl_MapAttrProc_MoveStartSecond( MMDL * mmdl );
static void MMdl_MapAttrProc_MoveStartJumpSecond( MMDL * mmdl );
static void MMdl_MapAttrProc_MoveEnd( MMDL * mmdl );
static void MMdl_MapAttrProc_MoveEndJump( MMDL * mmdl );

static void MMdl_MapAttrHeight_02( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrGrassProc_0( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrGrassProc_12( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrFootMarkProc_1( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrSplashProc_0( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrSplashProc_12( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrSplashProc_Jump1( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrShadowProc_0( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrShadowProc_1( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrShadowProc_2( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrGroundSmokeProc_2( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrLGrassProc_0( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrLGrassProc_1( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrNGrassProc_0( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrNGrassProc_1( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrPoolProc_1( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrPoolProc_2( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrSwampProc_1( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrSwampProc_2( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrReflect_01( MMDL *mmdl, ATTRDATA *data );
static void MMdl_MapAttrReflect_2( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrBridgeProc_01( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrBiriBiri_2( MMDL *mmdl, ATTRDATA *data );

static void MMdl_MapAttrSEProc_1( MMDL *mmdl, ATTRDATA *data );

static BOOL MMdl_HitCheckMoveAttr(
  const MMDL * mmdl, const VecFx32 pos );

#ifndef MMDL_PL_NULL
static BOOL (* const DATA_HitCheckAttr_Now[DIR_MAX4])( MAPATTR attr );
static BOOL (* const DATA_HitCheckAttr_Next[DIR_MAX4])( MAPATTR attr );
#endif

static BOOL MMdl_GetMapGridInfo(
  const MMDL *mmdl, const VecFx32 *pos,
  FLDMAPPER_GRIDINFO *pGridInfo );

//======================================================================
//  フィールド動作モデル 動作
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデル 動作初期化
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_InitMoveProc( MMDL * mmdl )
{
  MMDL_CallMoveInitProc( mmdl );
  MMDL_MoveSubProcInit( mmdl );
  MMDL_OnMoveBit( mmdl, MMDL_MOVEBIT_MOVEPROC_INIT );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル動作
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_UpdateMove( MMDL * mmdl )
{
  if( MMDL_CheckMMdlSysStatusBit(
    mmdl,MMDLSYS_STABIT_MOVE_PROC_STOP) ){
    return;
  }
  
  MMdl_GetHeightMoveBefore( mmdl );
  MMdl_GetAttrMoveBefore( mmdl );
  MMdl_ProcMoveStartFirst( mmdl );
  
  if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_ACMD) ){
    MMDL_ActionAcmd( mmdl );
  }else if( MMDL_CheckStatusBitMoveProcPause(mmdl) == FALSE ){
    if( MMdl_CheckMoveStart(mmdl) == TRUE ){
      if( MMDL_MoveSub(mmdl) == FALSE ){
        MMDL_CallMoveProc( mmdl );
      }
    }
  }
  
  MMdl_ProcMoveStartSecond( mmdl );
  MMdl_ProcMoveEnd( mmdl );
}

//--------------------------------------------------------------
/**
 * 動作可能かどうかチェック
 * @param  mmdl  MMDL *
 * @retval  int    TRUE=可能,FALSE=不可
 */
//--------------------------------------------------------------
static int MMdl_CheckMoveStart( const MMDL * mmdl )
{
  if( MMDL_CheckStatusBitMove(mmdl) == TRUE ){
    return( TRUE );
  }
  
  if( MMDL_CheckStatusBit(mmdl,STA_BIT_MOVE_ERROR) == 0 ){
    return( TRUE );
  }else if( MMDL_GetMoveCode(mmdl) == MV_TR_PAIR ){ //親の行動に従う
    return( TRUE );
  }
  
  #ifndef MMDL_PL_NULL
  {  //移動禁止フラグ相殺チェック
    u32 st = MMDL_GetStatusBit( mmdl );
    
    //高さ取得しない場合
    if( (st&MMDL_STABIT_HEIGHT_GET_ERROR) &&
      (st&MMDL_STABIT_HEIGHT_GET_OFF) == 0 ){
      return( FALSE );
    }
    
    //アトリビュート取得しない場合
    if( (st&MMDL_STABIT_ATTR_GET_ERROR) &&  
      MMDL_CheckMoveBitAttrGetOFF(mmdl) == FALSE ){
      return( FALSE );
    }
  }
  #endif
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 移動開始時に行う高さ取得
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_GetHeightMoveBefore( MMDL * mmdl )
{
  if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_HEIGHT_GET_ERROR) ){
    MMDL_UpdateCurrentHeight( mmdl );
  }
}

//--------------------------------------------------------------
/**
 * 移動開始時に行うアトリビュート取得
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_GetAttrMoveBefore( MMDL * mmdl )
{
  if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_ATTR_GET_ERROR) ){
    if( MMDL_UpdateCurrentMapAttr(mmdl) == TRUE ){
      MMDL_OnStatusBitMoveStart( mmdl );
    }
  }
}

//--------------------------------------------------------------
/**
 * 移動開始で発生　1st
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_ProcMoveStartFirst( MMDL * mmdl )
{
  if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_MOVE_START) ){
    MMdl_MapAttrProc_MoveStartFirst( mmdl );
  }
  
  MMDL_OffStatusBit( mmdl,
    MMDL_STABIT_MOVE_START | MMDL_STABIT_JUMP_START );
}

//--------------------------------------------------------------
/**
 * 移動開始で発生　2nd
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_ProcMoveStartSecond( MMDL * mmdl )
{
  if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_JUMP_START) ){
    MMdl_MapAttrProc_MoveStartJumpSecond( mmdl );
  }else if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_MOVE_START) ){
    MMdl_MapAttrProc_MoveStartSecond( mmdl );
  }
  
  MMDL_OffStatusBit( mmdl,
    MMDL_STABIT_MOVE_START | MMDL_STABIT_JUMP_START );
}

//--------------------------------------------------------------
/**
 * 動作終了で発生
 * @param  mmdl  MMDL *GX_WNDMASK_NONE
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_ProcMoveEnd( MMDL * mmdl )
{
  if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_JUMP_END) ){
    MMdl_MapAttrProc_MoveEndJump( mmdl );
  }else if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_MOVE_END) ){
    MMdl_MapAttrProc_MoveEnd( mmdl );
  }
  
  MMDL_OffStatusBit( mmdl,
    MMDL_STABIT_MOVE_END | MMDL_STABIT_JUMP_END );
}

//======================================================================
//  マップアトリビュート
//======================================================================
//--------------------------------------------------------------
/**
 * アトリビュートデータ初期化
 * @param mmdl MMDL
 * @param data ATTRDATA
 * @retval nothing
 */
//--------------------------------------------------------------
static void mmdl_InitAttrData( const MMDL *mmdl, ATTRDATA *data )
{
  data->attr_old = MMDL_GetMapAttrOld( mmdl );
  data->attr_now = MMDL_GetMapAttr( mmdl );
  data->attr_val_old = MAPATTR_GetAttrValue( data->attr_old );
  data->attr_val_now = MAPATTR_GetAttrValue( data->attr_now );
  data->attr_flag_old = MAPATTR_GetAttrFlag( data->attr_old );
  data->attr_flag_now = MAPATTR_GetAttrFlag( data->attr_now );
  
  data->objcode_prm =
      MMDL_GetOBJCodeParam( mmdl, MMDL_GetOBJCode(mmdl) );
   
  {
    MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
    FIELDMAP_WORK *fieldmap = MMDLSYS_GetFieldMapWork( mmdlsys );
    data->fectrl = FIELDMAP_GetFldEffCtrl( fieldmap );
    data->season = GAMEDATA_GetSeasonID( MMDLSYS_GetGameData(mmdlsys) );
  }
}

//--------------------------------------------------------------
/**
 * 動作開始で発生するアトリビュート処理 1st
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrProc_MoveStartFirst( MMDL * mmdl )
{
  MMDL_UpdateCurrentMapAttr( mmdl );
  
  if( MMDL_CheckCompletedDrawInit(mmdl) == TRUE ){
    ATTRDATA data;
    mmdl_InitAttrData( mmdl, &data );
    
    MMdl_MapAttrBridgeProc_01( mmdl, &data );
    MMdl_MapAttrGrassProc_0( mmdl, &data );
    MMdl_MapAttrSplashProc_0( mmdl, &data );
    MMdl_MapAttrShadowProc_0( mmdl, &data );
    MMdl_MapAttrHeight_02( mmdl, &data );
    MMdl_MapAttrLGrassProc_0( mmdl, &data );
    MMdl_MapAttrNGrassProc_0( mmdl, &data );
    MMdl_MapAttrReflect_01( mmdl, &data );
  }
}

//--------------------------------------------------------------
/**
 * 動作開始で発生するアトリビュート処理 2nd
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrProc_MoveStartSecond( MMDL * mmdl )
{
  MMDL_UpdateCurrentMapAttr( mmdl );
  
  if( MMDL_CheckCompletedDrawInit(mmdl) == TRUE ){
    ATTRDATA data;
    mmdl_InitAttrData( mmdl, &data );
    
    MMdl_MapAttrBridgeProc_01( mmdl, &data );
    MMdl_MapAttrGrassProc_12( mmdl, &data );
    MMdl_MapAttrFootMarkProc_1( mmdl, &data );
    MMdl_MapAttrSplashProc_12( mmdl, &data );
    MMdl_MapAttrShadowProc_1( mmdl, &data );
    MMdl_MapAttrLGrassProc_1( mmdl, &data );
    MMdl_MapAttrNGrassProc_1( mmdl, &data );
    MMdl_MapAttrPoolProc_1( mmdl, &data );
    MMdl_MapAttrSwampProc_1( mmdl, &data );
    MMdl_MapAttrReflect_01( mmdl, &data );
    
    MMdl_MapAttrSEProc_1( mmdl, &data ); //描画関係ない?
  }
}

//--------------------------------------------------------------
/**
 * 動作開始で発生するアトリビュート処理 2nd Jump
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrProc_MoveStartJumpSecond( MMDL * mmdl )
{
  MMDL_UpdateCurrentMapAttr( mmdl );
  
  if( MMDL_CheckCompletedDrawInit(mmdl) == TRUE ){
    ATTRDATA data;
    mmdl_InitAttrData( mmdl, &data );
    
    MMdl_MapAttrBridgeProc_01( mmdl, &data );
    MMdl_MapAttrShadowProc_1( mmdl, &data );
    MMdl_MapAttrReflect_01( mmdl, &data );
    MMdl_MapAttrSplashProc_Jump1( mmdl, &data );

    MMdl_MapAttrSEProc_1( mmdl, &data );//描画関係ない?
  }
}

//--------------------------------------------------------------
/**
 * 動作終了で発生するアトリビュート処理　end
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrProc_MoveEnd( MMDL * mmdl )
{
  MMDL_UpdateCurrentMapAttr( mmdl );
  
  if( MMDL_CheckCompletedDrawInit(mmdl) == TRUE ){
    ATTRDATA data;
    mmdl_InitAttrData( mmdl, &data );
    
    //終了　アトリビュート処理
    MMdl_MapAttrHeight_02( mmdl, &data );
    MMdl_MapAttrPoolProc_2( mmdl, &data );
    MMdl_MapAttrSwampProc_2( mmdl, &data );
    MMdl_MapAttrSplashProc_12( mmdl, &data );
    MMdl_MapAttrReflect_2( mmdl, &data );
    MMdl_MapAttrShadowProc_2( mmdl, &data );
    MMdl_MapAttrBiriBiri_2( mmdl, &data );
  }
}

//--------------------------------------------------------------
/**
 * 動作終了で発生するアトリビュート処理　end jump
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrProc_MoveEndJump( MMDL * mmdl )
{
  MMDL_UpdateCurrentMapAttr( mmdl );
  
  if( MMDL_CheckCompletedDrawInit(mmdl) == TRUE ){
    ATTRDATA data;
    mmdl_InitAttrData( mmdl, &data );
    
    //終了　アトリビュート処理
    MMdl_MapAttrHeight_02( mmdl, &data );
    MMdl_MapAttrPoolProc_2( mmdl, &data );
    MMdl_MapAttrSwampProc_2( mmdl, &data );
    MMdl_MapAttrSplashProc_12( mmdl, &data );
    MMdl_MapAttrReflect_2( mmdl, &data );
    MMdl_MapAttrShadowProc_2( mmdl, &data );
    MMdl_MapAttrGrassProc_12( mmdl, &data );
    MMdl_MapAttrGroundSmokeProc_2( mmdl, &data );
    MMdl_MapAttrBiriBiri_2( mmdl, &data );
  }
}

//======================================================================
//  アトリビュート可変高さ
//======================================================================
//--------------------------------------------------------------
/**
 * アトリビュート可変高さ　動作開始、動作終了
 * @param  mmdl    MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrHeight_02( MMDL *mmdl, ATTRDATA *data )
{
  if( MMDL_CheckStatusBitAttrOffsetOFF(mmdl) == FALSE ){
    if( MAPATTR_VALUE_CheckDesertDeep(data->attr_val_now) ){
      VecFx32 offs = { 0, ATTROFFS_Y_DESERT_DEEP, 0 };
      MMDL_SetVectorAttrDrawOffsetPos( mmdl, &offs );
      return;
    }
  }
  
  {
    VecFx32 vec = { 0, 0, 0 };
    MMDL_SetVectorAttrDrawOffsetPos( mmdl, &vec );
  }
}

#ifndef MMDL_PL_NULL
static void MMdl_MapAttrHeight_02(
    MMDL * mmdl, MAPATTR now, MAPATTR old, const OBJCODE_PARAM *prm )
{
  if( MMDL_CheckStatusBitAttrOffsetOFF(mmdl) == FALSE ){
    if( MAPATTR_IsSwampDeep(now) == TRUE ||
      MAPATTR_IsSwampGrassDeep(now) == TRUE ){
      VecFx32 vec = { 0, ATTROFFS_Y_NUMA_DEEP, 0 };
      MMDL_SetVectorAttrDrawOffsetPos( mmdl, &vec );
      return;
    }
    
    if( MAPATTR_IsSwamp(now) == TRUE || MAPATTR_IsSwampGrass(now) == TRUE ){
      VecFx32 vec = { 0, ATTROFFS_Y_NUMA, 0 };
      MMDL_SetVectorAttrDrawOffsetPos( mmdl, &vec );
      return;
    }
    
    if( MAPATTR_IsSnowDeepMost(now) == TRUE ){
      VecFx32 vec = { 0, ATTROFFS_Y_YUKI_DEEP_MOST, 0 };
      MMDL_SetVectorAttrDrawOffsetPos( mmdl, &vec );
      return;
    }
    
    if( MAPATTR_IsSnowDeep(now) == TRUE ){
      VecFx32 vec = { 0, ATTROFFS_Y_YUKI_DEEP, 0 };
      MMDL_SetVectorAttrDrawOffsetPos( mmdl, &vec );
      return;
    }
    
    if( MAPATTR_IsShallowSnow(now) == TRUE ){
      VecFx32 vec = { 0, ATTROFFS_Y_YUKI, 0 };
      MMDL_SetVectorAttrDrawOffsetPos( mmdl, &vec );
      return;
    }
  }
  
  {
    VecFx32 vec = { 0, 0, 0 };
    MMDL_SetVectorAttrDrawOffsetPos( mmdl, &vec );
  }
}
#endif

//======================================================================
//  アトリビュート　草
//======================================================================
//--------------------------------------------------------------
/**
 * 草　動作開始 0
 * @param  mmdl    MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrGrassProc_0( MMDL *mmdl, ATTRDATA *data )
{
  if( (data->attr_flag_now & MAPATTR_FLAGBIT_GRASS) ){
    if( MAPATTR_VALUE_CheckLongGrass(data->attr_val_now) ){
      FLDEFF_GRASS_SetMMdl( data->fectrl, mmdl, FALSE, FLDEFF_GRASS_LONG );
    }else if( MAPATTR_VALUE_CheckSnowGrass(data->attr_val_now) ){
      FLDEFF_GRASS_SetMMdl( data->fectrl, mmdl, FALSE, FLDEFF_GRASS_SNOW );
    }else{
      FLDEFF_GRASS_SetMMdl( data->fectrl, mmdl, FALSE, FLDEFF_GRASS_SHORT );
    }
  }
}

//--------------------------------------------------------------
/**
 * 草　動作 1,2
 * @param  mmdl  MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrGrassProc_12( MMDL *mmdl, ATTRDATA *data )
{
  if( data->attr_now != MAPATTR_ERROR &&
      (data->attr_flag_now & MAPATTR_FLAGBIT_GRASS) )
  {
    if( MAPATTR_VALUE_CheckLongGrass(data->attr_val_now) ){
      FLDEFF_GRASS_SetMMdl( data->fectrl, mmdl, TRUE, FLDEFF_GRASS_LONG );
    }else if( MAPATTR_VALUE_CheckSnowGrass(data->attr_val_now) ){
      FLDEFF_GRASS_SetMMdl( data->fectrl, mmdl, TRUE, FLDEFF_GRASS_SNOW );
    }else{
      FLDEFF_GRASS_SetMMdl( data->fectrl, mmdl, TRUE, FLDEFF_GRASS_SHORT );
    }
  }
}

//======================================================================
//  アトリビュート　足跡
//======================================================================
//--------------------------------------------------------------
/**
 * 足跡　動作開始 1
 * @param  mmdl  MMDL *
 * @param  now    現在のアトリビュート
 * @param  old    過去のアトリビュート
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrFootMarkProc_1( MMDL *mmdl, ATTRDATA *data )
{
  if( data->attr_old != MAPATTR_ERROR &&
      (data->attr_flag_old & MAPATTR_FLAGBIT_FOOTMARK) )
  {
    FOOTMARK_TYPE type;
    
    if( MAPATTR_VALUE_CheckSnowType(data->attr_val_old) == TRUE ||
        (data->season == PMSEASON_WINTER &&
          (MAPATTR_VALUE_CheckSeasonGround1(data->attr_val_old) ||
          MAPATTR_VALUE_CheckSeasonGround2(data->attr_val_old))) )
    {
      type = FOOTMARK_TYPE_HUMAN_SNOW;
      
      if( data->objcode_prm->footmark_type == MMDL_FOOTMARK_CYCLE )
      {
        type = FOOTMARK_TYPE_CYCLE_SNOW;
      }
    }
    else if( MAPATTR_VALUE_CheckDesertDeep(data->attr_val_old) == TRUE )
    {
      type = FOOTMARK_TYPE_DEEPSNOW;       
      PMSND_PlaySE( SEQ_SE_FLD_91 );
    }
    else
    {
      type = FOOTMARK_TYPE_HUMAN;
      
      if( data->objcode_prm->footmark_type == MMDL_FOOTMARK_CYCLE )
      {
        type = FOOTMARK_TYPE_CYCLE;
      }
    }
     
    FLDEFF_FOOTMARK_SetMMdl( mmdl, data->fectrl, type );
  }
}

//======================================================================
//  アトリビュート　水飛沫
//======================================================================
//--------------------------------------------------------------
/**
 * 水飛沫　動作開始 0
 * @param  mmdl  MMDL *
 * @param  now    現在のアトリビュート
 * @param  old    過去のアトリビュート
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrSplashProc_0( MMDL *mmdl, ATTRDATA *data )
{
  if( MAPATTR_VALUE_CheckShoal(data->attr_val_now) == TRUE ){
    if( MMDL_CheckStatusBitShoalEffect(mmdl) == FALSE ){
      FLDEFF_SPLASH_SetMMdl( data->fectrl, mmdl, TRUE );
      MMDL_SetStatusBitShoalEffect( mmdl, TRUE );
    }
  }else{
    MMDL_SetStatusBitShoalEffect( mmdl, FALSE );
  }
}

//--------------------------------------------------------------
/**
 * 水飛沫　動作開始終了 12
 * @param  mmdl  MMDL *
 * @param  now    現在のアトリビュート
 * @param  old    過去のアトリビュート
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrSplashProc_12( MMDL *mmdl, ATTRDATA *data )
{
  if( MAPATTR_VALUE_CheckShoal(data->attr_val_now) == TRUE ){
    if( MMDL_CheckStatusBitShoalEffect(mmdl) == FALSE ){
      FLDEFF_SPLASH_SetMMdl( data->fectrl, mmdl, TRUE );
      MMDL_SetStatusBitShoalEffect( mmdl, TRUE );
    }
  }else if( MAPATTR_VALUE_CheckMarsh(data->attr_val_now) == TRUE ){
    FLDEFF_SPLASH_SetMMdl( data->fectrl, mmdl, FALSE );
  }else{
    MMDL_SetStatusBitShoalEffect( mmdl, FALSE );
  }
}

//--------------------------------------------------------------
/**
 * 水飛沫　ジャンプ動作開始 1
 * @param  mmdl  MMDL *
 * @param  now    現在のアトリビュート
 * @param  old    過去のアトリビュート
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrSplashProc_Jump1( MMDL *mmdl, ATTRDATA *data )
{
  MMDL_SetStatusBitShoalEffect( mmdl, FALSE );
}

//======================================================================
//  アトリビュート　影
//======================================================================
//--------------------------------------------------------------
/**
 * 影　動作開始 0
 * @param  mmdl  MMDL *
 * @param  now    現在のアトリビュート
 * @param  old    過去のアトリビュート
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrShadowProc_0( MMDL *mmdl, ATTRDATA *data )
{
  MMdl_MapAttrShadowProc_1( mmdl, data );
}

//--------------------------------------------------------------
/**
 * 影　動作開始 1
 * @param  mmdl  MMDL *
 * @param  now    現在のアトリビュート
 * @param  old    過去のアトリビュート
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrShadowProc_1( MMDL *mmdl, ATTRDATA *data )
{
  const MMDLSYS *fos = MMDL_GetMMdlSys( mmdl );
    
  if( MMDLSYS_CheckJoinShadow(fos) == FALSE ){
    return;
  }

  if( (data->attr_flag_now & MAPATTR_FLAGBIT_SHADOW) &&
      data->objcode_prm->shadow_type == MMDL_SHADOW_ON ){
    if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_SHADOW_SET) == 0 ){
      FLDEFF_SHADOW_SetMMdl( mmdl, data->fectrl );
      MMDL_OnStatusBit( mmdl, MMDL_STABIT_SHADOW_SET );
    }
  }
}

//--------------------------------------------------------------
/**
 * 影　動作終了 2
 * @param  mmdl  MMDL *
 * @param  now    現在のアトリビュート
 * @param  old    過去のアトリビュート
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrShadowProc_2( MMDL *mmdl, ATTRDATA *data )
{
  const MMDLSYS *fos = MMDL_GetMMdlSys( mmdl );
    
  if( MMDLSYS_CheckJoinShadow(fos) == FALSE ){
    return;
  }
  
  if( data->objcode_prm->shadow_type == MMDL_SHADOW_NON ){
    return;
  }
  
  if( (data->attr_flag_now & MAPATTR_FLAGBIT_SHADOW) == 0 ){
    MMDL_OnStatusBit( mmdl, MMDL_STABIT_SHADOW_VANISH );
  }else{
    MMDL_OffStatusBit( mmdl, MMDL_STABIT_SHADOW_VANISH );
  }
}

//======================================================================
//  アトリビュート　土煙
//======================================================================
//--------------------------------------------------------------
/**
 * 土煙　動作終了 2 
 * @param  mmdl  MMDL *
 * @param  now    現在のアトリビュート
 * @param  old    過去のアトリビュート
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrGroundSmokeProc_2( MMDL *mmdl, ATTRDATA *data )
{
  FLDEFF_KEMURI_SetMMdl( mmdl, data->fectrl );
}

//======================================================================
//  アトリビュート　長い草
//======================================================================
//--------------------------------------------------------------
/**
 * 長い草　動作開始 0
 * @param  mmdl    MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrLGrassProc_0( MMDL *mmdl, ATTRDATA *data )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsLongGrass(now) == TRUE ){
    FE_mmdlLGrass_Add( mmdl, FALSE );
  }
  #endif
}

//--------------------------------------------------------------
/**
 * 長い草　動作開始 1
 * @param  mmdl  MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrLGrassProc_1( MMDL *mmdl, ATTRDATA *data )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsLongGrass(now) == TRUE ){
    FE_mmdlLGrass_Add( mmdl, TRUE );
  }
  #endif
}

//======================================================================
//  アトリビュート　沼草
//======================================================================
//--------------------------------------------------------------
/**
 * 沼草　動作開始 0
 * @param  mmdl    MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrNGrassProc_0( MMDL *mmdl, ATTRDATA *data )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsSwampGrass(now) == TRUE ){
    FE_mmdlNGrass_Add( mmdl, FALSE );
  }
  #endif
}

//--------------------------------------------------------------
/**
 * 沼草　動作開始 1
 * @param  mmdl  MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrNGrassProc_1( MMDL *mmdl, ATTRDATA *data )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsSwampGrass(now) == TRUE ){
    FE_mmdlNGrass_Add( mmdl, TRUE );
  }
  #endif
}

//======================================================================
//  アトリビュート　水たまり
//======================================================================
//--------------------------------------------------------------
/**
 * 水たまり　動作開始 1
 * @param  mmdl  MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrPoolProc_1( MMDL *mmdl, ATTRDATA *data )
{
  if( MAPATTR_VALUE_CheckPool(data->attr_val_old) == TRUE ){
    FLDEFF_RIPPLE_Set( data->fectrl,
      MMDL_GetOldGridPosX(mmdl),
      MMDL_GetOldGridPosZ(mmdl),
      MMDL_GetVectorPosY(mmdl) );
  }
}

//--------------------------------------------------------------
/**
 * 水たまり　動作終了 2
 * @param  mmdl  MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrPoolProc_2( MMDL *mmdl, ATTRDATA *data )
{
  if( MAPATTR_VALUE_CheckPool(data->attr_val_now) == TRUE ){
    FLDEFF_RIPPLE_Set( data->fectrl,
      MMDL_GetGridPosX(mmdl),
      MMDL_GetGridPosZ(mmdl),
      MMDL_GetVectorPosY(mmdl) );
  }
}

//======================================================================
//  アトリビュート　沼
//======================================================================
//--------------------------------------------------------------
/**
 * 沼　動作開始 1
 * @param  mmdl  MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrSwampProc_1( MMDL *mmdl, ATTRDATA *data )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsSwamp(old) == TRUE ){
    FE_mmdlNRippleSet( mmdl,
      MMDL_GetOldGridPosX(mmdl),
      MMDL_GetOldGridPosY(mmdl), 
      MMDL_GetOldGridPosZ(mmdl) );
  }
  #endif
}

//--------------------------------------------------------------
/**
 * 沼　動作終了 2
 * @param  mmdl  MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrSwampProc_2( MMDL *mmdl, ATTRDATA *data )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsSwamp(now) == TRUE ){
    FE_mmdlNRippleSet( mmdl,
      MMDL_GetGridPosX(mmdl),
      MMDL_GetGridPosY(mmdl), 
      MMDL_GetGridPosZ(mmdl) );
  }
  #endif
}

//======================================================================
//  アトリビュート　映りこみ
//======================================================================
//--------------------------------------------------------------
/**
 * 映りこみ　動作開始 01
 * @param  mmdl  MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrReflect_01( MMDL *mmdl, ATTRDATA *data )
{
  if( data->objcode_prm->reflect_type == MMDL_REFLECT_NON ){
    return;
  }
  
  if( MMDL_CheckStatusBitReflect(mmdl) == FALSE )
  {
    MAPATTR_FLAG flag = MAPATTR_FLAGBIT_NONE;
    MAPATTR hit_attr = MAPATTR_ERROR;
    
    #ifdef DEBUG_REFLECT_CHECK
    now = MAPATTR_FLAGBIT_REFLECT << 16;
    #endif
    
    if( data->attr_now != MAPATTR_ERROR )
    {
      if( (data->attr_flag_now & MAPATTR_FLAGBIT_REFLECT) )
      {
        hit_attr = data->attr_now;
        #ifdef DEBUG_REFLECT_CHECK
        hit_attr = 0;
        #endif
      }
      else
      {
        MAPATTR next = MMDL_GetMapDirAttr( mmdl, DIR_DOWN );
        flag = MAPATTR_GetAttrFlag( next );
      
        if( (flag & MAPATTR_FLAGBIT_REFLECT) )
        {
          hit_attr = next;
        }
      }
      
      if( hit_attr != MAPATTR_ERROR )
      {
        REFLECT_TYPE type;
        MMDLSYS *mmdlsys = (MMDLSYS*)MMDL_GetMMdlSys( mmdl );
        
        MMDL_SetStatusBitReflect( mmdl, TRUE );
         
        type = REFLECT_TYPE_POND; //本来はアトリビュート識別してタイプ決定
        FLDEFF_REFLECT_SetMMdl( mmdlsys, mmdl, data->fectrl, type );
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 映りこみ　動作終了 2
 * @param  mmdl  MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrReflect_2( MMDL *mmdl, ATTRDATA *data )
{
  if( data->objcode_prm->reflect_type == MMDL_REFLECT_NON ||
    MMDL_CheckStatusBitReflect(mmdl) == FALSE ){
    return;
  }
  
  {
    MAPATTR_FLAG flag = MAPATTR_FLAGBIT_NONE;
    MAPATTR attr = MMDL_GetMapDirAttr( mmdl, DIR_DOWN );
    
    #ifdef DEBUG_REFLECT_CHECK
    attr = MAPATTR_FLAGBIT_REFLECT << 16;
    #endif
    
    if( attr == MAPATTR_ERROR ){
      MMDL_SetStatusBitReflect( mmdl, FALSE );
    }else{
      flag = MAPATTR_GetAttrFlag( attr );
      
      if( (flag & MAPATTR_FLAGBIT_REFLECT) == 0 ){
        MMDL_SetStatusBitReflect( mmdl, FALSE );
      }
    }
  }
}

//======================================================================
//  アトリビュート　橋
//======================================================================
//--------------------------------------------------------------
/**
 * 橋　動作開始 0
 * @param  mmdl    MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrBridgeProc_01( MMDL *mmdl, ATTRDATA *data )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsBridgeFlag(now) == TRUE ){
    MMDL_SetStatusBitBridge( mmdl, TRUE );
  }else if( MMDL_CheckStatusBitBridge(mmdl) == TRUE ){
    if( MAPATTR_IsBridge(now) == FALSE ){
      MMDL_SetStatusBitBridge( mmdl, FALSE );
    }
  }
  #endif
}

//======================================================================
//  アトリビュート　ビリビリ床
//======================================================================
//--------------------------------------------------------------
/**
 * ビリビリ床　動作終了 2
 * @param  mmdl    MMDL *
 * @param  now      現在のアトリビュート
 * @param  old      過去のアトリビュート
 * @param  prm    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrBiriBiri_2( MMDL *mmdl, ATTRDATA *data )
{
  if( MAPATTR_VALUE_CheckElecFloor(data->attr_val_now) == TRUE ){
    FLDEFF_D06DENKI_BIRIBIRI_SetMMdl( mmdl, data->fectrl );
  }
}

//======================================================================
//  アトリビュート関連SE
//======================================================================
//--------------------------------------------------------------
/**
 * SE　動作開始 1
 * @param  mmdl  MMDL *
 * @param  now    現在のアトリビュート
 * @param  old    過去のアトリビュート
 * @param  param    OBJCODE_PARAM
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_MapAttrSEProc_1( MMDL *mmdl, ATTRDATA *data )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsSnow(now) ){
    Snd_SePlay( SEQ_SE_DP_YUKIASHI );
  }
  #endif
}

//======================================================================
//  移動チェック
//======================================================================
//--------------------------------------------------------------
/**
 * 指定座標障害物チェック
 * @param  fos    MMDLSYS *
 * @param  x    チェックするX座標  グリッド
 * @param  z    チェックするZ座標  グリッド
 * @retval  u32    ヒットビット。MMDL_MOVEHITBIT_LIM等
 */
//--------------------------------------------------------------
#ifndef MMDL_PL_NULL
u32 MMDL_PosHitCheck( const MMDLSYS *fos, int x, int z )
{
  u32 ret;
  
  ret = MMDL_MOVEHITBIT_NON;
  
  if(  GetHitAttr(MMDLSYS_FieldSysWorkGet(fos),x,z) == TRUE ){
    ret |= MMDL_MOVEHITBIT_ATTR;
  }
  
  if( MMDLSYS_SearchGridPos(fos,x,z,FALSE) != NULL ){
    ret |= MMDL_MOVEHITBIT_OBJ;
  }
  
  return( ret );
}
#endif

//--------------------------------------------------------------
/**
 * フィールド動作モデル移動チェック
 * @param  mmdl  MMDL *
 * @param  vec    現在実座標
 * @param  x    移動先X座標  グリッド
 * @param  y    移動先Y座標
 * @param  z    移動先Z座標  グリッド
 * @param  dir    移動方向 DIR_UP等
 * @retval  u32    ヒットビット。MMDL_MOVEHITBIT_LIM等
 */
//--------------------------------------------------------------
u32 MMDL_HitCheckMove(
  const MMDL *mmdl, const VecFx32 *vec, s16 x, s16 y, s16 z, u16 dir )
{
  u32 ret;
  VecFx32 pos;
  
  ret = MMDL_MOVEHITBIT_NON;
  
  pos.x = GRID_SIZE_FX32( x ) + GRID_HALF_FX32;
  pos.y = vec->y; //GRID_SIZE_FX32( y );
  pos.z = GRID_SIZE_FX32( z ) + GRID_HALF_FX32;
    
  if( MMDL_HitCheckMoveLimit(mmdl,x,z) == TRUE ){
    ret |= MMDL_MOVEHITBIT_LIM;
  }
  
  {
    u32 attr;
    fx32 height;
    
    if( MMdl_HitCheckMoveAttr(mmdl,pos) == TRUE ){
      ret |= MMDL_MOVEHITBIT_ATTR;
    }

    if( MMDL_GetMapPosHeight(mmdl,&pos,&height) == TRUE ){
      fx32 diff = vec->y - height;
      if( diff < 0 ){ diff = -diff; }
      if( diff >= HEIGHT_DIFF_COLLISION ){
        ret |= MMDL_MOVEHITBIT_HEIGHT;
      }
    }else{
      ret |= MMDL_MOVEHITBIT_HEIGHT;
    }
  }
  
  if( MMDL_HitCheckMoveFellow(mmdl,x,y,z) == TRUE ){
    ret |= MMDL_MOVEHITBIT_OBJ;
  }
  
  {
    const FLDMAPPER *pG3DMapper;
    
    pG3DMapper = MMDLSYS_GetG3DMapper( MMDL_GetMMdlSys(mmdl) );
    
    //pos->yはCheckOutRange()内では未評価<090916時点>
    if( FLDMAPPER_CheckOutRange(pG3DMapper,&pos) == TRUE ){
      ret |= MMDL_MOVEHITBIT_OUTRANGE;
    }
  }

  return( ret );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル移動チェック　現在位置から判定
 * @param  mmdl  MMDL * 
 * @param  x    移動先X座標  グリッド
 * @param  y    移動先Y座標
 * @param  z    移動先Z座標  グリッド
 * @param  dir    移動方向 DIR_UP等
 * @retval  u32    ヒットビット。MMDL_MOVEHITBIT_LIM等
 */
//--------------------------------------------------------------
u32 MMDL_HitCheckMoveCurrent(
  const MMDL * mmdl, s16 x, s16 y, s16 z, u16 dir )
{
  VecFx32 vec;
  MMDL_GetVectorPos( mmdl, &vec );
  return( MMDL_HitCheckMove(mmdl,&vec,x,y,z,dir) );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル移動チェック　現在位置+方向版
 * @param  mmdl  MMDL * 
 * @param  dir    移動する方向。DIR_UP等
 * @retval  u32    ヒットビット。MMDL_MOVEHITBIT_LIM等
 */
//--------------------------------------------------------------
u32 MMDL_HitCheckMoveDir( const MMDL * mmdl, u16 dir )
{
  s16 x,y,z;
  x = MMDL_GetGridPosX( mmdl ) + MMDL_TOOL_GetDirAddValueGridX( dir );
  y = MMDL_GetHeightGrid( mmdl );
  z = MMDL_GetGridPosZ( mmdl ) + MMDL_TOOL_GetDirAddValueGridZ( dir );
  return( MMDL_HitCheckMoveCurrent(mmdl,x,y,z,dir) );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル同士の衝突チェック
 * @param  mmdl  MMDL * 
 * @param  x    移動先X座標  グリッド
 * @param  y    移動先X座標  グリッド
 * @param  z    移動先X座標  グリッド
 * @retval  BOOL  TRUE=衝突アリ
 */
//--------------------------------------------------------------
BOOL MMDL_HitCheckMoveFellow( const MMDL * mmdl, s16 x, s16 y, s16 z )
{
  u32 no=0;
  int hx,hz;
  MMDL *cmmdl;
  const MMDLSYS *sys;
  
  sys = MMDL_GetMMdlSys( mmdl );
  
  while( MMDLSYS_SearchUseMMdl(sys,&cmmdl,&no) == TRUE ){
    if( cmmdl != mmdl ){
      if( MMDL_CheckStatusBit(
        cmmdl,MMDL_STABIT_FELLOW_HIT_NON) == 0 ){
        hx = MMDL_GetGridPosX( cmmdl );
        hz = MMDL_GetGridPosZ( cmmdl );
        {
          BOOL debug = FALSE;
          if( MMDL_GetOBJID(mmdl) == MMDL_ID_PLAYER ){
            debug = TRUE;
          }
          if( hx == x && hz == z ){
            int hy = MMDL_GetHeightGrid( cmmdl );
            int sy = hy - y;
            if( sy < 0 ){ sy = -sy; }
            #ifdef DEBUG_ONLY_FOR_kagaya
            if( debug ){
              OS_Printf(
                "HERO %d, %d, %d NPC %d %d %d\n",
                x, y, z, hx, hy, hz );
            }
            #endif
            if( sy < H_GRID_FELLOW_SIZE ){
              return( TRUE );
            }
          }
        
          hx = MMDL_GetOldGridPosX( cmmdl );
          hz = MMDL_GetOldGridPosZ( cmmdl );
        
          if( hx == x && hz == z ){
            int hy = MMDL_GetHeightGrid( cmmdl );
            int sy = hy - y;
            if( sy < 0 ){ sy = -sy; }
            if( sy < H_GRID_FELLOW_SIZE ){
              return( TRUE );
            }
          }
        }
      }
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル移動制限チェック
 * @param  mmdl  MMDL * 
 * @param  x    移動先X座標  グリッド
 * @param  y    移動先Y座標  グリッド
 * @param  z    移動先Z座標  グリッド
 * @retval  BOOL  TRUE=制限越え
 */
//--------------------------------------------------------------
BOOL MMDL_HitCheckMoveLimit( const MMDL * mmdl, s16 x, s16 z )
{
  s16 init,limit,min,max;
  
  init = MMDL_GetInitGridPosX( mmdl );
  limit = MMDL_GetMoveLimitX( mmdl );
  
  if( limit != MOVE_LIMIT_NOT ){
    min = init - limit;
    max = init + limit;
  
    if( min > x || max < x ){
      return( TRUE );
    }
  }
  
  init = MMDL_GetInitGridPosZ( mmdl );
  limit = MMDL_GetMoveLimitZ( mmdl );
  
  if( limit != MOVE_LIMIT_NOT ){
    min = init - limit;
    max = init + limit;
  
    if( min > z || max < z ){
      return( TRUE );
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルアトリビュートヒットチェック
 * @param  mmdl  MMDL * 
 * @param  pos    移動先X座標,現在位置のY,移動先Z座標
 * @param  dir    移動方向 DIR_UP等
 * @retval  int    TRUE=移動不可アトリビュート
 */
//--------------------------------------------------------------
static BOOL MMdl_HitCheckMoveAttr(
  const MMDL * mmdl, const VecFx32 pos )
{
  if( MMDL_CheckMoveBitAttrGetOFF(mmdl) == FALSE ){
    MAPATTR attr;
    
    if( MMDL_GetMapPosAttr(mmdl,&pos,&attr) == TRUE ){
      return MAPATTR_GetHitchFlag(attr);
    }
  }
  
  return( TRUE ); //移動不可アトリビュート
}

//--------------------------------------------------------------
///  現在位置アトリビュートから判定する移動制御アトリビュートチェック関数
//--------------------------------------------------------------
#ifndef MMDL_PL_NULL
static BOOL (* const DATA_HitCheckAttr_Now[DIR_MAX4])( MAPATTR attr ) =
{
  MAPATTR_IsBadMoveUpCheck,
  MAPATTR_IsBadMoveDownCheck,
  MAPATTR_IsBadMoveLeftCheck,
  MAPATTR_IsBadMoveRightCheck,
};

//--------------------------------------------------------------
///  未来位置アトリビュートから判定する移動制御アトリビュートチェック関数
//--------------------------------------------------------------
static BOOL (* const DATA_HitCheckAttr_Next[DIR_MAX4])( MAPATTR attr ) =
{
  MAPATTR_IsBadMoveDownCheck,
  MAPATTR_IsBadMoveUpCheck,
  MAPATTR_IsBadMoveRightCheck,
  MAPATTR_IsBadMoveLeftCheck,
};
#endif

//--------------------------------------------------------------
/**
 * 橋を考慮した水アトリビュートチェック
 * @param  mmdl  MMDL *
 * @param  attr  アトリビュート
 * @retval  BOOL  TRUE=水アトリビュート
 */
//--------------------------------------------------------------
static BOOL MMdl_CheckMapAttrKind_Water( MMDL * mmdl, u32 attr )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsBridgeWater(attr) ){
    if( MMDL_CheckStatusBitBridge(mmdl) == FALSE ){
      return( TRUE );
    }
  }else if( MAPATTR_IsWater(attr) ){
    return( TRUE );
  }
  #endif
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 橋を考慮した砂アトリビュートチェック
 * @param  mmdl  MMDL *
 * @param  attr  アトリビュート
 * @retval  int    TRUE=水アトリビュート
 */
//--------------------------------------------------------------
static BOOL MMdl_CheckMapAttrKind_Sand( MMDL * mmdl, u32 attr )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsBridgeSand(attr) ){
    if( MMDL_CheckStatusBitBridge(mmdl) == FALSE ){
      return( TRUE );
    }
  }else if( MAPATTR_IsSand(attr) ){
    return( TRUE );
  }
  #endif
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 橋を考慮した雪アトリビュートチェック
 * @param  mmdl  MMDL *
 * @param  attr  アトリビュート
 * @retval  int    TRUE=水アトリビュート
 */
//--------------------------------------------------------------
static BOOL MMdl_CheckMapAttrKind_Snow( MMDL * mmdl, u32 attr )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsBridgeSnowShallow(attr) ){
    if( MMDL_CheckStatusBitBridge(mmdl) == FALSE ){
      return( TRUE );
    }
  }else if( MAPATTR_IsSnow(attr) ){
    return( TRUE );
  }
  #endif
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 橋を考慮した浅い雪アトリビュートチェック
 * @param  mmdl  MMDL *
 * @param  attr  アトリビュート
 * @retval  int    TRUE=水アトリビュート
 */
//--------------------------------------------------------------
static BOOL MMdl_CheckMapAttrKind_MostShallowSnow(
    MMDL * mmdl, u32 attr )
{
  #ifndef MMDL_PL_NULL
  if( MAPATTR_IsBridgeSnowShallow(attr) ){
    if( MMDL_CheckStatusBitBridge(mmdl) == FALSE ){
      return( TRUE );
    }
  }else if( MAPATTR_IsMostShallowSnow(attr) ){
    return( TRUE );
  }
  #endif  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 橋を考慮した橋アトリビュートチェック
 * @param  mmdl  MMDL *
 * @param  attr  アトリビュート
 * @retval  int    TRUE=水アトリビュート
 */
//--------------------------------------------------------------
static BOOL MMdl_CheckMapAttrKind_Bridge( MMDL * mmdl, u32 attr )
{
  if( MMDL_CheckStatusBitBridge(mmdl) == TRUE ){
    #ifndef MMDL_PL_NULL
    if( MAPATTR_IsBridge(attr) == TRUE ){
      return( TRUE );
    }
    #endif
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 橋を考慮した一本橋縦アトリビュートチェック
 * @param  mmdl  MMDL *
 * @param  attr  アトリビュート
 * @retval  int    TRUE=水アトリビュート
 */
//--------------------------------------------------------------
static BOOL MMdl_CheckMapAttrKind_BridgeV( MMDL * mmdl, u32 attr )
{
  if( MMDL_CheckStatusBitBridge(mmdl) == TRUE ){
  #ifndef MMDL_PL_NULL
    if( MAPATTR_IsBridgeV(attr) == TRUE ){
      return( TRUE );
    }
  #endif
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 橋を考慮した一本橋横アトリビュートチェック
 * @param  mmdl  MMDL *
 * @param  attr  アトリビュート
 * @retval  int    TRUE=水アトリビュート
 */
//--------------------------------------------------------------
static BOOL MMdl_CheckMapAttrKind_BridgeH( MMDL * mmdl, u32 attr )
{
  if( MMDL_CheckStatusBitBridge(mmdl) == TRUE ){
  #ifndef MMDL_PL_NULL
    if( MAPATTR_IsBridgeH(attr) == TRUE ){
      return( TRUE );
    }
  #endif
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * マップグリッド情報を取得
 * @param  mmdl  MMDL
 * @param  pos  取得する座標
 * @param  pGridInfo 情報格納先
 * @retval  BOOL FALSE=マップ情報が無い
 */
//--------------------------------------------------------------
static BOOL MMdl_GetMapGridInfo(
  const MMDL *mmdl, const VecFx32 *pos, FLDMAPPER_GRIDINFO *pGridInfo )
{
  const FLDMAPPER *pG3DMapper =
    MMDLSYS_GetG3DMapper( MMDL_GetMMdlSys(mmdl) );
  
  if( FLDMAPPER_GetGridInfo(pG3DMapper,pos,pGridInfo) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * マップグリッド情報取得
 * @param  mmdl  MMDL
 * @param  pos       取得する座標(x,zに取得したい位置座標、yには現在のheightが格納されていること)
 * @param  pGridData  グリッドアトリビュートデータ格納先
 * @retval  0未満 情報が取得できない
 *
 *  @li GridInfoに取得された全階層をサーチし、現在のYに最も近いheight,attrを返します
 *  @li GridInfoの配列0の方がプライオリティが高いです
 */
//--------------------------------------------------------------
static int MMdl_GetMapPosGridData(
  const MMDL *mmdl, const VecFx32 *pos, FLDMAPPER_GRIDINFODATA* pGridData)
{
  const FLDMAPPER *pG3DMapper =
    MMDLSYS_GetG3DMapper( MMDL_GetMMdlSys(mmdl) );
  
  return FLDMAPPER_GetGridData(pG3DMapper,pos,pGridData);
}

//--------------------------------------------------------------
/**
 * マップアトリビュート取得
 * @param  mmdl  MMDL
 * @param  pos  取得する座標(x,zに取得したい位置座標、yには現在のheightが格納されていること)
 * @param  attr  アトリビュート格納先
 * @retval  BOOL  FALSE=アトリビュートが存在しない。
 */
//--------------------------------------------------------------
BOOL MMDL_GetMapPosAttr(
  const MMDL *mmdl, const VecFx32 *pos, u32 *attr )
{
  FLDMAPPER_GRIDINFODATA gridData;
  *attr = 0;
  
  if( MMdl_GetMapPosGridData(mmdl,pos,&gridData) == TRUE ){
    *attr = gridData.attr;
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * マップ高さ取得
 * @param  mmdl  MMDL
 * @param  pos  取得する座標
 * @param  height  高さ格納先
 * @retval  BOOL  FALSE=高さが存在しない。
 */
//--------------------------------------------------------------
BOOL MMDL_GetMapPosHeight(
  const MMDL *mmdl, const VecFx32 *pos, fx32 *height )
{
#if 1
  FLDMAPPER_GRIDINFODATA gridData;
  
  *height = 0;
  if( MMdl_GetMapPosGridData(mmdl,pos,&gridData) == TRUE ){
    *height = gridData.height;
    return( TRUE );
  }
#else
  FLDMAPPER_GRIDINFO GridInfo;
  *height = 0;

  if( MMdl_GetMapGridInfo(mmdl,pos,&GridInfo) == TRUE ){
    if( GridInfo.count ){
      int    i = 0;
      fx32  h_tmp,diff1,diff2;
      
      *height = GridInfo.gridData[i].height;
      i++;
      
      while( i < GridInfo.count ){
        h_tmp = GridInfo.gridData[i].height;
        diff1 = *height - pos->y;
        diff2 = h_tmp - pos->y;
        
        if( FX_Mul(diff2,diff2) < FX_Mul(diff1,diff1) ){
          *height = h_tmp;
        }
        i++;
      }
      
      return( TRUE );
    }
  }
#endif
  return( FALSE );
}

//======================================================================
//  グリッド座標移動系
//======================================================================
//--------------------------------------------------------------
/**
 * 方向で現在座標を更新。高さは更新されない。
 * @param  mmdl    MMDL * 
 * @param  dir      移動方向
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_UpdateGridPosDir( MMDL * mmdl, u16 dir )
{
  MMDL_SetOldGridPosX( mmdl, MMDL_GetGridPosX(mmdl) );
  MMDL_SetOldGridPosY( mmdl, MMDL_GetGridPosY(mmdl) );
  MMDL_SetOldGridPosZ( mmdl, MMDL_GetGridPosZ(mmdl) );
  
  MMDL_AddGridPosX( mmdl, MMDL_TOOL_GetDirAddValueGridX(dir) );
  MMDL_AddGridPosY( mmdl, 0 );
  MMDL_AddGridPosZ( mmdl, MMDL_TOOL_GetDirAddValueGridZ(dir) );
}

//--------------------------------------------------------------
/**
 * 現在座標で過去座標を更新
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_UpdateGridPosCurrent( MMDL * mmdl )
{
  MMDL_SetOldGridPosX( mmdl, MMDL_GetGridPosX(mmdl) );
  MMDL_SetOldGridPosY( mmdl, MMDL_GetHeightGrid(mmdl) );
  MMDL_SetOldGridPosZ( mmdl, MMDL_GetGridPosZ(mmdl) );
}

//--------------------------------------------------------------
/**
 * 現在のグリッド座標から指定方向先のアトリビュートを取得
 * @param  mmdl  MMDL *
 * @param  dir    DIR_UP等
 * @retval  u32    MAPATTR
 */
//--------------------------------------------------------------
MAPATTR MMDL_GetMapDirAttr( MMDL * mmdl, u16 dir )
{
  MAPATTR attr = MAPATTR_ERROR;
  VecFx32 pos;
  const FLDMAPPER *pG3DMapper;
  
  MMDL_GetVectorPos( mmdl, &pos );
  MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
  MMDL_GetMapPosAttr( mmdl, &pos, &attr );
  
  return( attr );
}

//======================================================================
//  3D座標系
//======================================================================
//--------------------------------------------------------------
/**
 * 指定値を加算する
 * @param  mmdl    MMDL * 
 * @param  val      移動量
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_AddVectorPos( MMDL * mmdl, const VecFx32 *val )
{
  VecFx32 vec;
  MMDL_GetVectorPos( mmdl, &vec );
  vec.x += val->x;
  vec.y += val->y;
  vec.z += val->z;
  MMDL_SetVectorPos( mmdl, &vec );
}

//--------------------------------------------------------------
/**
 * 指定値を指定方向に加算する
 * @param  mmdl    MMDL * 
 * @param  dir      移動4方向。DIR_UP等
 * @param  val      移動量(正
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_AddVectorPosDir( MMDL *mmdl, u16 dir, fx32 val )
{
  VecFx32 vec;
  MMDL_GetVectorPos( mmdl, &vec );
  switch( dir ){
  case DIR_UP:    vec.z -= val;  break;
  case DIR_DOWN:    vec.z += val;  break;
  case DIR_LEFT:    vec.x -= val;  break;
  case DIR_RIGHT:    vec.x += val;  break;
  }
  MMDL_SetVectorPos( mmdl, &vec );
}

//--------------------------------------------------------------
/**
 * 現在座標から高さ取得し実座標に反映
 * MMDL_STABIT_HEIGHT_GET_ERRORのセットも併せて行う
 * @param  mmdl    MMDL * 
 * @retval  BOOL  TRUE=高さが取れた。FALSE=取れない。
 * MMDL_STABIT_HEIGHT_GET_ERRORで取得可能
 */
//--------------------------------------------------------------
BOOL MMDL_UpdateCurrentHeight( MMDL * mmdl )
{
  VecFx32 vec_pos,vec_pos_h;
  
  MMDL_GetVectorPos( mmdl, &vec_pos );
  vec_pos_h = vec_pos;
  
  if( MMDL_CheckStatusBitHeightGetOFF(mmdl) == TRUE ){
    MMDL_OffStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_ERROR );
    return( FALSE );
  }
  
  {
    fx32 height;
    int ret = MMDL_GetMapPosHeight( mmdl, &vec_pos_h, &height );
    
    if( ret == TRUE ){
      vec_pos.y = height;
      MMDL_SetVectorPos( mmdl, &vec_pos );
      MMDL_OffStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_ERROR );
    }else{
      MMDL_OnStatusBit( mmdl, MMDL_STABIT_HEIGHT_GET_ERROR );
    }
    return( ret );
  }
}

//--------------------------------------------------------------
/**
 * 現在座標でアトリビュート反映
 * MMDL_STABIT_ATTR_GET_ERRORのセットも併せて行う
 * @retval  BOOL TRUE=取得成功。FALSE=失敗。
 */
//--------------------------------------------------------------
BOOL MMDL_UpdateCurrentMapAttr( MMDL * mmdl )
{
  MAPATTR old_attr = MAPATTR_ERROR;
  MAPATTR now_attr = old_attr;
  
  if( MMDL_CheckMoveBitAttrGetOFF(mmdl) == FALSE ){
    VecFx32 pos;
    int gx = MMDL_GetOldGridPosX( mmdl );
    int gy = MMDL_GetOldGridPosY( mmdl );
    int gz = MMDL_GetOldGridPosZ( mmdl );
    
    pos.x = GRID_SIZE_FX32( gx );
    pos.y = GRID_SIZE_FX32( gy );
    pos.z = GRID_SIZE_FX32( gz );
    MMDL_GetMapPosAttr( mmdl, &pos, &old_attr );
    
    gx = MMDL_GetGridPosX( mmdl );
    gz = MMDL_GetGridPosZ( mmdl );
    MMDL_TOOL_GetCenterGridPos( gx, gz, &pos );
    pos.y = MMDL_GetVectorPosY( mmdl );
    MMDL_GetMapPosAttr( mmdl, &pos, &now_attr );
  }
  
  MMDL_SetMapAttrOld( mmdl, old_attr );
  MMDL_SetMapAttr( mmdl, now_attr );
  
  if( now_attr == MAPATTR_ERROR ){
    MMDL_OnStatusBit( mmdl, MMDL_STABIT_ATTR_GET_ERROR );
    return( FALSE );
  }
  
  MMDL_OffStatusBit( mmdl, MMDL_STABIT_ATTR_GET_ERROR );
  return( TRUE );
}

//======================================================================
//  描画初期化、復帰直後に呼ばれる動作初期化関数
//======================================================================
//--------------------------------------------------------------
/**
 * 描画初期化、復帰直後に呼ばれる動作初期化関数。
 * 隠れ蓑系動作は動作関数内でエフェクト系のセットを行っている。
 * イベント等のポーズが初期化直後から発生している場合、
 * 動作関数が呼ばれない為、描画初期化時にこの関数を呼ぶ事で対策。
 * @param  mmdl  MMDL *
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_CallMoveProcAfterDrawInit( MMDL * mmdl )
{
  u16 code = MMDL_GetMoveCode( mmdl );
  if( code == MV_HIDE_SNOW || code == MV_HIDE_SAND ||
    code == MV_HIDE_GRND || code == MV_HIDE_KUSA ){
    MMDL_CallMoveProc( mmdl );
  }
}

//======================================================================
//  動作ツール
//======================================================================
//--------------------------------------------------------------
///  4方向別座標増減　X
//--------------------------------------------------------------
static const int DATA_DirGridValueX[] =
{ 0, 0, -1, 1 };

//--------------------------------------------------------------
///  4方向別座標増減　Y
//--------------------------------------------------------------
static const int DATA_DirGridValueY[] =
{ 0, 0, 0, 0 };

//--------------------------------------------------------------
///  4方向別座標増減　Z
//--------------------------------------------------------------
static const int DATA_DirGridValueZ[] =
{ -1, 1, 0, 0 };

//--------------------------------------------------------------
/**
 * 方向からX方向増減値を取得
 * @param  dir    方向。DIR_UP等
 * @retval  s16 グリッド単位での増減値
 */
//--------------------------------------------------------------
s16 MMDL_TOOL_GetDirAddValueGridX( u16 dir )
{
  return( (s16)DATA_DirGridValueX[dir] );
}

//--------------------------------------------------------------
/**
 * 方向からZ方向増減値を取得
 * @param  dir    方向。DIR_UP等
 * @retval  int    グリッド単位での増減値
 */
//--------------------------------------------------------------
s16 MMDL_TOOL_GetDirAddValueGridZ( u16 dir )
{
  return( (s16)DATA_DirGridValueZ[dir] );
}

//--------------------------------------------------------------
/**
 * 指定値を指定方向に加算する
 * @param  dir      移動4方向。DIR_UP等
 * @param  vec      加える対象
 * @param  val      移動量(正
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_TOOL_AddDirVector( u16 dir, VecFx32 *vec, fx32 val )
{
  switch( dir ){
  case DIR_UP:    vec->z -= val;  break;
  case DIR_DOWN:    vec->z += val;  break;
  case DIR_LEFT:    vec->x -= val;  break;
  case DIR_RIGHT:    vec->x += val;  break;
  }
}

//--------------------------------------------------------------
/**
 * 指定値を指定方向に加算する　グリッド
 * @param  dir      移動4方向。DIR_UP等
 * @param  gx      加える対象　グリッドx
 * @param gz      加える対象　グリッドz
 * @param  val      移動量(正
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_TOOL_AddDirGrid( u16 dir, s16 *gx, s16 *gz, s16 val )
{
  switch( dir ){
  case DIR_UP: *gz -= val; break;
  case DIR_DOWN: *gz += val; break;
  case DIR_LEFT: *gx -= val; break;
  case DIR_RIGHT: *gx += val; break;
  }
}

//--------------------------------------------------------------
/**
 * 指定されたグリッド座標の中心位置を実座標で取得
 * @param  gx  グリッドX座標  
 * @param  gz  グリッドZ座標
 * @param  vec  座標格納先
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_TOOL_GetCenterGridPos( s16 gx, s16 gz, VecFx32 *vec )
{
  vec->x = GRID_SIZE_FX32( gx ) + GRID_HALF_FX32;
  vec->z = GRID_SIZE_FX32( gz ) + GRID_HALF_FX32;
}

//--------------------------------------------------------------
/**
 * 指定された実座標をグリッド単位に変更
 * @param  gx  グリッドX座標格納先
 * @param  gy  グリッドY座標格納先
 * @param  gz  グリッドZ座標格納先
 * @param  vec  変換する座標
 * @retval  nothing
 */
//--------------------------------------------------------------
void MMDL_TOOL_VectorPosToGridPos(
  s16 *gx, s16 *gy, s16 *gz, const VecFx32 *vec )
{
  *gx = SIZE_GRID_FX32( vec->x );
  *gy = SIZE_GRID_FX32( vec->y );
  *gz = SIZE_GRID_FX32( vec->z );
}

//--------------------------------------------------------------
/**
 * 指定されたグリッド座標を実座標に変更
 * @param  gx  グリッドX座標
 * @param  gy  グリッドY座標
 * @param  gz  グリッドZ座標
 * @param  vec  座標変換先
 * @retval nothing
 */
//--------------------------------------------------------------
void MMDL_TOOL_GridPosToVectorPos(
    const s16 gx, const s16 gy, const s16 gz, VecFx32 *pos )
{
  pos->x = GRID_SIZE_FX32( gx );
  pos->y = GRID_SIZE_FX32( gy );
  pos->z = GRID_SIZE_FX32( gz );
}

//--------------------------------------------------------------
//  方向反転テーブル
//--------------------------------------------------------------
static const u32 DATA_DirFlipTbl[] =
{ DIR_DOWN, DIR_UP, DIR_RIGHT, DIR_LEFT };

//--------------------------------------------------------------
/**
 * 与えられた方向を反転する
 * @param  dir    DIR_UP等
 * @retval  int    dirの反転方向
 */
//--------------------------------------------------------------
u16 MMDL_TOOL_FlipDir( u16 dir )
{
  return( DATA_DirFlipTbl[dir] );
}

//--------------------------------------------------------------
/**
 * 距離間方向を取得
 * @param  ax  対象A X座標
 * @param  az  対象A Z座標
 * @param  bx  対象B X座標
 * @param  bz  対象B Z座標
 * @retval  int  対象Aから対象Bへの方向 DIR_UP等
 */
//--------------------------------------------------------------
u16 MMDL_TOOL_GetRangeDir( int ax, int az, int bx, int bz )
{
  if( ax > bx ){ return( DIR_LEFT ); }
  if( ax < bx ){ return( DIR_RIGHT ); }
  if( az > bz ){ return( DIR_UP ); }
  return( DIR_DOWN );
}

//--------------------------------------------------------------
/// カメラ横角度変換
//--------------------------------------------------------------
static const u8 data_angle8_4[16] =
{
  0,
  1, 1, 1, 1, 1, 1,
  2, 2,
  3, 3, 3, 3, 3, 3,
  0,
};

static const u16 data_dir_angle[DIR_MAX4] =
{
  0x8000, 0, 0x4000, 0xC000,
};

static const u8 data_angleChange360_4[4][4] =
{
  {DIR_UP,DIR_DOWN,DIR_LEFT,DIR_RIGHT}, //0 0x0000
  {DIR_RIGHT,DIR_LEFT,DIR_UP,DIR_DOWN}, //1 0x4000
  {DIR_DOWN,DIR_UP,DIR_RIGHT,DIR_LEFT}, //2 0x8000
  {DIR_LEFT,DIR_RIGHT,DIR_DOWN,DIR_UP}, //3 0xc000
};

//--------------------------------------------------------------
/**
 * カメラ横角度から表示する４方向を取得
 * @param dir 角度0時の方向。
 * @param angleYaw カメラ角度
 * @retval u16 DIR_UP等
 */
//--------------------------------------------------------------
u16 MMDL_TOOL_GetAngleYawToDirFour( u16 dir, u16 angleYaw )
{
  angleYaw += data_dir_angle[dir];  //　dirの方向が下向きになる角度分アングルを変更
  angleYaw >>= 12; // angle/0x1000(16)
  angleYaw = data_angle8_4[angleYaw];
  angleYaw = data_angleChange360_4[angleYaw][DIR_DOWN]; // 下向き基準で考えているので。
  GF_ASSERT( angleYaw < DIR_MAX4 );
  return( angleYaw );
}
