/**
 *  @file   encount_effect.c
 *  @brief  エンカウントエフェクト制御
 *  @author Miyuki Iwasawa
 *  @date   09.11.17
 */

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "fieldmap.h"
#include "map_attr.h"

#include "encount_data.h"
#include "field_encount.h"

#include "field_encount_local.h"
#include "effect_encount.h"

#define EFFENC_SEARCH_OX (10)
#define EFFENC_SEARCH_OZ (10)
#define EFFENC_SEARCH_WX  (EFFENC_SEARCH_OX*2+1)
#define EFFENC_SEARCH_WZ  (EFFENC_SEARCH_OZ*2+1)
#define EFFENC_SEARCH_AREA_MAX  (EFFENC_SEARCH_WX*EFFENC_SEARCH_WZ)

#define GRID_MAP_W  (32)

///エンカウントエフェクト制御
typedef struct _EFFENC_POS{
  u8  valid_f;
  u16 zone_id;
  u8 map_x;
  u8 map_z;
  u8  gx;
  u8  gz;
}EFFENC_POS;

typedef struct _EFFENC_ATTR_POS{
  u8  gx,gz;
  u8  attr;
}EFFENC_ATTR_POS;


///エンカウントエフェクトアトリビュート探索マップ
typedef struct _EFFENC_ATTR_MAP{
  u16 count;
  EFFENC_ATTR_POS attr[EFFENC_SEARCH_AREA_MAX];
}EFFENC_ATTR_MAP;

/////////////////////////////////////////////////////
///エフェクトエンカウント制御ワーク fieldWork常駐
struct _TAG_EFFECT_ENCOUNT{
  EFFENC_POS pos;
  EFFENC_ATTR_MAP attr_map;

  u32 walk_ct_interval;
};

///////////////////////////////////////////////////////////////////////////////////////////
//プロトタイプ
///////////////////////////////////////////////////////////////////////////////////////////
static BOOL effenc_IsEffectExist( EFFECT_ENCOUNT* eff_wk );
static BOOL effenc_CheckWalkCt( ENCOUNT_WORK* ewk, EFFECT_ENCOUNT* eff_wk );



///////////////////////////////////////////////////////////////////////////////////////////
//グローバル関数群
///////////////////////////////////////////////////////////////////////////////////////////
/**
 *  @brief  エンカウントエフェクトワーク生成
 */
EFFECT_ENCOUNT* EFFECT_ENC_CreateWork( HEAPID heapID )
{
  EFFECT_ENCOUNT* eff_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(EFFECT_ENCOUNT));

  eff_wk->walk_ct_interval = 10;
  return eff_wk;
}

/*
 *  @brief  エンカウントエフェクトワーク破棄
 */
void EFFECT_ENC_DeleteWork( EFFECT_ENCOUNT* eff_wk )
{
  MI_CpuClear8( eff_wk, sizeof(EFFECT_ENCOUNT) );
  GFL_HEAP_FreeMemory( eff_wk );
}

/*
 *  @brief  エンカウントエフェクト起動チェック
 */
void EFFECT_ENC_CheckEffectEncountStart( FIELD_ENCOUNT* enc )
{
  EFFECT_ENCOUNT* eff_wk = enc->eff_enc;
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);

  //歩数チェック
  if( !effenc_CheckWalkCt( ewk, eff_wk ) ){
    return;
  }
  //グリッドベースマップかチェック
  if( FIELDMAP_GetBaseSystemType( enc->fwork ) != FLDMAP_BASESYS_GRID ){
    return;
  }
 
  //エフェクト起動中チェック
  if(effenc_IsEffectExist( eff_wk )){
    return; //既に起動している
  }

  //エンカウントデータチェック
  if( !enc->encdata->enable_f ){
    return;
  }

  //アトリビュートサーチ
}

/*
 *  @brief  エフェクトエンカウント　マップステータス更新
 */
void EFFECT_ENC_MapStateUpdate( FIELD_ENCOUNT* enc )
{

}

///////////////////////////////////////////////////////////////////////////////////////////
//ローカル関数群
///////////////////////////////////////////////////////////////////////////////////////////
/*
 *  @brief  歩数制御＆チェック
 */
static BOOL effenc_CheckWalkCt( ENCOUNT_WORK* ewk, EFFECT_ENCOUNT* eff_wk )
{
  EWK_EFFECT_ENCOUNT* ewk_eff = &ewk->effect_encount;

  if(ewk_eff->walk_ct < 0xFFFFFFFF){
    ewk_eff->walk_ct++; //まあ、オーバーフローすることはないだろうが念のためmaxチェック
  }

  if( ewk_eff->walk_ct >= eff_wk->walk_ct_interval){
    ewk_eff->walk_ct = 0;
    return TRUE;
  }
  return FALSE;
}

/*
 *  @brief  エフェクト起動中かどうかチェック
 */
static BOOL effenc_IsEffectExist( EFFECT_ENCOUNT* eff_wk )
{
  return eff_wk->pos.valid_f;
}

/*
 *  @brief  エフェクトアトリビュートサーチ
 */
static void effect_AttributeSearch( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  s16 i,j;
  s16 sx,sz,ex,ez;
  s16 player_x,player_z;
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );

  {
    MMDL* mmdl = FIELD_PLAYER_GetMMdl( fplayer );

    player_x = MMDL_GetGridPosX( mmdl ) % GRID_MAP_W;
    player_z = MMDL_GetGridPosZ( mmdl ) % GRID_MAP_W;
  }
  sx = player_x - EFFENC_SEARCH_OX;
  if( sx < 0 ){
    sx = 0;
  }
  ex = player_x + EFFENC_SEARCH_OX;
  if( ex >= GRID_MAP_W){
    ex = GRID_MAP_W-1;
  }
  sz = player_z - EFFENC_SEARCH_OZ;
  if( sz < 0 ){
    sz = 0;
  }
  ez = player_z + EFFENC_SEARCH_OZ;
  if( ez >= GRID_MAP_W){
    ez = GRID_MAP_W-1;
  }

  for(i = sz; i <= ez;i++){
    for(j = sx; j <= ex;j++){
        
    }
  }

}


