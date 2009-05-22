//======================================================================
/**
 * @file  field_encount.c
 * @brief	フィールド　エンカウント処理
 * @author kagaya
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "map_attr.h"

#include "field_encount.h"

//======================================================================
//  define
//======================================================================
#define CALC_SHIFT (8) ///<計算シフト値
#define WALK_COUNT_GLOBAL (8) ///<エンカウントしない歩数
#define WALK_COUNT_MAX (0xffff) ///<歩数カウント最大
#define NEXT_PERCENT (40) ///<エンカウントする基本確率
#define	WALK_NEXT_PERCENT	(5) ///<歩数カウント失敗で次の処理に進む確率

#ifdef DEBUG_ONLY_FOR_kagaya
#define DEBUG_ENCOUNT_CHECKOFF_ATTR
#endif

//--------------------------------------------------------------
//  ENCOUNT_LOCATION
//--------------------------------------------------------------
typedef enum
{
  ENCOUNT_LOCATION_GROUND = 0,
  ENCOUNT_LOCATION_WATER,
  ENCOUNT_LOCATION_FISHING,
}ENCOUNT_LOCATION;

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// FIELD_ENCOUNT
//--------------------------------------------------------------
struct _TAG_FIELD_ENCOUNT
{
  int walk_count;
  FIELDMAP_WORK *fwork;
};

//======================================================================
//  proto
//======================================================================
static u32 enc_GetAttrPercent( FIELD_ENCOUNT *enc,
    MAPATTR_VAL attr_val, ENCOUNT_LOCATION *outEncLocation );

static BOOL enc_CheckEncount(
    FIELD_ENCOUNT *enc, const u32 per, const MAPATTR_VAL attr_val );
static BOOL enc_CheckEncountWalk( FIELD_ENCOUNT *enc, u32 per );

static u32 enc_GetPercentRand( void );

//======================================================================
//  フィールドエンカウント
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドエンカウント　ワーク作成
 * @param fwork FIELDMAP_WORK
 * @retval FIELD_ENCOUNT*
 */
//--------------------------------------------------------------
FIELD_ENCOUNT * FIELD_ENCOUNT_Create( FIELDMAP_WORK *fwork )
{
  HEAPID heapID;
  FIELD_ENCOUNT *enc;
  
  heapID = FIELDMAP_GetHeapID( fwork );
  enc = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_ENCOUNT) );
  
  enc->fwork = fwork;
  return( enc );
}

//--------------------------------------------------------------
/**
 * フィールドエンカウント　削除
 * @param enc FIELD_ENCOUNT*
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_ENCOUNT_Delete( FIELD_ENCOUNT *enc )
{
  GFL_HEAP_FreeMemory( enc );
}

//======================================================================
//  フィールドエンカウント　チェック
//======================================================================
//--------------------------------------------------------------
/**
 * エンカウントチェック
 * @param enc FIELD_ENCOUNT
 * @retval BOOL TRUE=エンカウントした
 */
//--------------------------------------------------------------
BOOL FIELD_ENCOUNT_CheckEncount( FIELD_ENCOUNT *enc )
{
  u32 pre;
  BOOL ret = FALSE;
  int gx,gz;
  VecFx32 pos;
  MAPATTR attr;
  MAPATTR_VAL attr_val;
  MAPATTR_FLAG attr_flag;
  ENCOUNT_LOCATION enc_loc;
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
  FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( enc->fwork );
  
  FIELD_PLAYER_GetPos( fplayer, &pos );
  attr = MAPATTR_GetMapAttribute( mapper, &pos );
  
  if( attr == MAPATTR_ERROR ){
    return( FALSE );
  }
  
  attr_flag = MAPATTR_GetMapAttrFlag( attr );

#ifndef DEBUG_ENCOUNT_CHECKOFF_ATTR
  if( (attr_flag & MAPATTR_FLAGBIT_ENCOUNT) == 0 ){
    return( FALSE );
  }
#endif

  attr_val = MAPATTR_GetMapAttrValue( attr );
  
  { //手持ちポケモンチェック
  }
  
  { //スプレーチェック
  }
  
  { //エンカウント確率取得
    pre = enc_GetAttrPercent( enc, attr_val, &enc_loc );
  }
  
  { //パラメタによる確率変動
  }
  
  if( enc_CheckEncount(enc,pre,attr_val) == TRUE ){ //エンカウントチェック
    ret = TRUE;
  }
  
  if( ret == FALSE ){
    OS_Printf( "フィールドエンカウント　はずれ\n" );
  }
  return( ret );
}

//======================================================================
//  サブ　エンカウント確率
//======================================================================
//--------------------------------------------------------------
/**
 * エンカウント率をアトリビュートから取得
 * @param enc FIELD_ENCOUNT*
 * @param attr_val チェックするMAPATTR_VAL
 * @param outEncLocation にエンカウント場所を格納するENCOUNT_LOCATION
 * @retval u32 エンカウント率
 */
//--------------------------------------------------------------
static u32 enc_GetAttrPercent( FIELD_ENCOUNT *enc,
    MAPATTR_VAL attr_val, ENCOUNT_LOCATION *outEncLocation )
{
  u32 per = 0;
  
  {
    //イベントデータが持つエンカウントデータから
    //アトリビュート別に確率取得
  }
  
  { //仮
    *outEncLocation = ENCOUNT_LOCATION_GROUND;
    per = 30;
  }

  return( per );
}

//======================================================================
//  サブ　エンカウントチェック
//======================================================================
//--------------------------------------------------------------
/**
 * エンカウントチェック
 * @param enc FIELD_ENCOUNT
 * @param pre エンカウント確率
 * @param attr MAPATTR
 * @retval BOOL TRUE=エンカウントした
 */
//--------------------------------------------------------------
static BOOL enc_CheckEncount(
    FIELD_ENCOUNT *enc, u32 per, const MAPATTR_VAL attr_val )
{
  u32 calc_per,next_per;
  
  if( per > 100 ){ //100%
    per = 100;
  }
  
  calc_per = per << CALC_SHIFT;
  
  if( enc_CheckEncountWalk(enc,calc_per) == FALSE ){
    if( enc->walk_count < WALK_COUNT_MAX ){
      enc->walk_count++; //歩数カウント
    }
    
    //5%で次の処理へ
    if( enc_GetPercentRand() >= WALK_NEXT_PERCENT ){
      return( FALSE );
    }
  }
  
  next_per = NEXT_PERCENT; //エンカウント基本確率
  
  {
    //アトリビュート別確率変動
    //長い草の場合は+30%
    //自転車の場合は+30%
  }
  
  {
    //日付による確率変動 
  }
  
  if( next_per > 100 ){
    next_per = 100;
  }
  
  if( enc_GetPercentRand() < next_per ){ //変動後の確率をチェック
    if( enc_GetPercentRand() >= per ){
      return( TRUE );
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * エンカウントチェック　歩数チェック
 * @param enc FIELD_ENCOUNT
 * @param per エンカウント確率
 * @retval BOOL TRUE=エンカウントした
 */
//--------------------------------------------------------------
static BOOL enc_CheckEncountWalk( FIELD_ENCOUNT *enc, u32 per )
{
  per = (per/10) >> CALC_SHIFT;
  
  if( per > WALK_COUNT_GLOBAL ){
    per = WALK_COUNT_GLOBAL;
  }
  
  per = WALK_COUNT_GLOBAL - per;
  
  if( enc->walk_count >= per ){
    return( TRUE );
  }
   
  return( FALSE );
}

//======================================================================
//  パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * ランダム確率取得
 * @retval  u32 確率
 */
//--------------------------------------------------------------
static u32 enc_GetPercentRand( void )
{
  u32 per = (0xffff/100) + 1;
  u32 val = GFUser_GetPublicRand(0xffff) / per;
  return( val );
}

