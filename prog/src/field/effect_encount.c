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

#include "field_encount.h"
#include "fldeff_encount.h"

#include "encount_data.h"
#include "field_encount.h"
#include "field_encount_local.h"
#include "effect_encount.h"
#include "map_attr.h"
#include "map_attr_def.h"

#define EFFENC_SEARCH_OX (10)
#define EFFENC_SEARCH_OZ (10)
#define EFFENC_SEARCH_WX  (EFFENC_SEARCH_OX*2+1)
#define EFFENC_SEARCH_WZ  (EFFENC_SEARCH_OZ*2+1)
#define EFFENC_SEARCH_AREA_MAX  (EFFENC_SEARCH_WX*EFFENC_SEARCH_WZ)

#define GRID_MAP_W  (32)

typedef struct _EFFENC_ATTR_POS{
  u8  type;
  u16  gx,gz;
  fx32  height;
}EFFENC_ATTR_POS;

typedef struct _EFFENC_SEARCH{
  s16 player_x;
  s16 player_z;
  s16 player_lx;
  s16 player_lz;
  s16 block_x,block_z;
  s16 sx,sz,ex,ez;
}EFFENC_SEARCH;

///エンカウントエフェクトアトリビュート探索マップ
typedef struct _EFFENC_ATTR_MAP{
  u16 count;
  EFFENC_ATTR_POS attr[EFFENC_SEARCH_AREA_MAX];
}EFFENC_ATTR_MAP;

/////////////////////////////////////////////////////
///エフェクトエンカウント制御ワーク fieldWork常駐
struct _TAG_EFFECT_ENCOUNT{
  FLDEFF_CTRL* fectrl;

  EFFENC_ATTR_MAP attr_map;
  FLDEFF_TASK* eff_task;
  
  u16 walk_ct_interval;
  u16 prob;
};

///////////////////////////////////////////////////////////////////////////////////////////
//プロトタイプ
///////////////////////////////////////////////////////////////////////////////////////////
static void effect_EffectPush( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );
static void effect_EffectPop( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );

static BOOL effenc_CheckWalkCt( ENCOUNT_WORK* ewk, EFFECT_ENCOUNT* eff_wk );
static void effect_AttributeSearch( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );

static void effect_EffectSetUp( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );
static void effect_EffectDelete( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );

static void effect_AddFieldEffect( EFFECT_ENCOUNT* eff_wk, EFFENC_PARAM* ep );
static void effect_DelFieldEffect( EFFECT_ENCOUNT* eff_wk );



///////////////////////////////////////////////////////////////////////////////////////////
//グローバル関数群
///////////////////////////////////////////////////////////////////////////////////////////
/**
 *  @brief  エンカウントエフェクトワーク生成
 */
EFFECT_ENCOUNT* EFFECT_ENC_CreateWork( HEAPID heapID )
{
  EFFECT_ENCOUNT* eff_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(EFFECT_ENCOUNT));
  
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
 *  @brief  エフェクトエンカウント システム初期化
 */
void EFFECT_ENC_Init( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  eff_wk->fectrl = FIELDMAP_GetFldEffCtrl( enc->fwork );
  eff_wk->walk_ct_interval = 10;
 
  //パラメータをPop
  effect_EffectPop( enc, eff_wk );
}

/*
 *  @brief  エフェクトエンカウント　システム終了処理
 */
void EFFECT_ENC_End( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  //パラメータをPush 
  effect_EffectPush( enc, eff_wk );
  
  //フィールドエフェクト破棄
  effect_DelFieldEffect( eff_wk );
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

  //エンカウントデータチェック
  if( !enc->encdata->enable_f ){
    return;
  }

  //アトリビュートサーチ
  effect_AttributeSearch( enc, eff_wk );

  //抽選
  if( eff_wk->attr_map.count == 0){
    return;
  }
 
  effect_EffectDelete( enc, eff_wk );
  effect_EffectSetUp( enc, eff_wk );
}

/*
 *  @brief  エフェクトエンカウント  強制破棄　
 */
void EFFECT_ENC_EffectDelete( FIELD_ENCOUNT* enc )
{
  effect_EffectDelete( enc, enc->eff_enc );
}

/*
 *  @brief  エフェクトエンカウントプッシュ
 */
static void effect_EffectPush( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  EFFENC_PARAM* ep = &ewk->effect_encount.param; 

  if( !ep->valid_f ){
    return;
  }
  ep->push_f = TRUE;
}

/*
 *  @brief  エフェクトエンカウントポップ 
 */
static void effect_EffectPop( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  EFFENC_PARAM* ep = &ewk->effect_encount.param; 

  if( !ep->valid_f || !ep->push_f){
    return;
  }

  ep->push_f = FALSE; //フラグ落とす
  {
    u16 zone_id = FIELDMAP_GetZoneID( enc->fwork );

    if(zone_id != ep->zone_id || ep->push_cancel_f ){
      MI_CpuClear8( ep, sizeof(EFFENC_PARAM));
      return;
    }
  }
  effect_AddFieldEffect( eff_wk, ep );  //保存されたパラメータで復帰
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
 *  @brief  エフェクトアトリビュート サーチ領域取得
 */
static void effect_SearchAreaGet( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk, EFFENC_SEARCH* esw )
{
  s16 sx,ex,sz,ez;
  {
    FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
    MMDL* mmdl = FIELD_PLAYER_GetMMdl( fplayer );

    esw->player_x = MMDL_GetGridPosX( mmdl );
    esw->player_z = MMDL_GetGridPosZ( mmdl );
    esw->player_lx = esw->player_x % GRID_MAP_W;
    esw->player_lz = esw->player_z % GRID_MAP_W;
    esw->block_x = esw->player_x / GRID_MAP_W;
    esw->block_z = esw->player_z / GRID_MAP_W;
  }
  sx = esw->player_lx - EFFENC_SEARCH_OX;
  if( sx < 0 ){
    sx = 0;
  }
  ex = esw->player_lx + EFFENC_SEARCH_OX;
  if( ex >= GRID_MAP_W){
    ex = GRID_MAP_W-1;
  }
  sz = esw->player_lz - EFFENC_SEARCH_OZ;
  if( sz < 0 ){
    sz = 0;
  }
  ez = esw->player_lz + EFFENC_SEARCH_OZ;
  if( ez >= GRID_MAP_W){
    ez = GRID_MAP_W-1;
  }

  {
    s16 block;
    block = esw->block_x*32;
    esw->sx = sx + block;
    esw->ex = ex + block;
    block = esw->block_z*32;
    esw->sz = sz + block;
    esw->ez = ez + block;
  }
}

/*
 *  @breif  アトリビュートタイプチェック
 */
static inline EFFENC_TYPE_ID effect_GetAttributeType( MAPATTR attr )
{
  MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
  MAPATTR_VALUE value = MAPATTR_GetAttrValue( attr );

  if( value == MATTR_BRIDGE_01 ){
    return EFFENC_TYPE_BRIDGE;
  }
  if( !(flag & MAPATTR_FLAGBIT_ENCOUNT)){
    return EFFENC_TYPE_MAX;
  }
  switch(value){
  case MATTR_E_GRASS_LOW:
    return EFFENC_TYPE_SGRASS;

  case MATTR_E_LGRASS_LOW:
    return EFFENC_TYPE_LGRASS;

  case MATTR_E_ZIMEN_01:
    return EFFENC_TYPE_CAVE;

  case MATTR_WATER_01:
  case MATTR_DEEP_MARSH_01:
    return EFFENC_TYPE_WATER;

  case MATTR_SEA_01:
    return EFFENC_TYPE_SEA;
  }
  return EFFENC_TYPE_MAX;
}

/*
 *  @brief  エフェクトアトリビュートサーチ
 */
static void effect_AttributeSearch( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  int blockIdx;
  s16 i,j;
  VecFx32 vec;
  EFFENC_TYPE_ID id;
  EFFENC_SEARCH esw;
  FLDMAPPER_GRIDINFODATA gridData;
  const FLDMAPPER* g3dMapper = (const FLDMAPPER*)FIELDMAP_GetFieldG3Dmapper( enc->fwork ); 

  //検索領域取得
  effect_SearchAreaGet( enc, eff_wk, &esw );

  IWASAWA_Printf("EffSearch\n");
  IWASAWA_Printf(" Player(%d,%d), x(%d～%d), z(%d～%d)\n",
      esw.player_x,esw.player_z,esw.sx,esw.ex,esw.sz,esw.ez);

  MI_CpuClear8(&eff_wk->attr_map,sizeof(EFFENC_ATTR_MAP));
  vec.y = 0;
  blockIdx = FLDMAPPER_GetCurrentBlockAccessIdx( g3dMapper );
  for(i = esw.sz; i <= esw.ez;i++){
    vec.z = FX32_CONST(i*16); 
    for(j = esw.sx; j <= esw.ex;j++){
      vec.x = FX32_CONST(j*16); 
      FLDMAPPER_GetGridDataForEffectEncount( g3dMapper, blockIdx, &vec, &gridData );
//    IWASAWA_Printf("0x%02x,",gridData.attr&0xFFFF);
      id = effect_GetAttributeType( gridData.attr );
      if(id != EFFENC_TYPE_MAX ){
        eff_wk->attr_map.attr[eff_wk->attr_map.count].type = id;
        eff_wk->attr_map.attr[eff_wk->attr_map.count].gx = j;
        eff_wk->attr_map.attr[eff_wk->attr_map.count].gz = i;
        eff_wk->attr_map.attr[eff_wk->attr_map.count++].height = gridData.height;
      }
    }
//    IWASAWA_Printf("\n");
  }
}

/*
 *  @brief  エフェクト抽選＆登録
 */
static void effect_EffectSetUp( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  u16 idx = GFUser_GetPublicRand0( eff_wk->attr_map.count);
  EFFENC_ATTR_POS* attr = &eff_wk->attr_map.attr[idx];
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  EFFENC_PARAM* ep = &ewk->effect_encount.param; 

  //パラメータセット
  ep->gx = attr->gx;
  ep->gz = attr->gz;
  ep->height = attr->height;
  ep->type = attr->type;
  ep->zone_id = FIELDMAP_GetZoneID( enc->fwork );
  ep->valid_f = TRUE;

  effect_AddFieldEffect( eff_wk, ep );
}

/*
 *  @brief  エフェクト破棄
 */
static void effect_EffectDelete( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  EFFENC_PARAM* ep = &ewk->effect_encount.param; 

  effect_DelFieldEffect( eff_wk );

  MI_CpuClear8( ep, sizeof(EFFENC_PARAM));
}

/*
 *  @brief  フィールドエフェクト登録
 */
static void effect_AddFieldEffect( EFFECT_ENCOUNT* eff_wk, EFFENC_PARAM* ep )
{
  eff_wk->eff_task = FLDEFF_ENCOUNT_SetEffect( eff_wk->fectrl, ep->gx, ep->gz, ep->height, ep->type );
//  eff_wk->pos.eff_task = FLDEFF_ENCOUNT_SetEffect( eff_wk->fectrl, 788, 715, ep->height, ep->type );  
  IWASAWA_Printf(" EffectAdd( %d, %d, h = 0x%08x) type=%d <0x%08x\n",ep->gx, ep->gz, ep->height, ep->type,eff_wk->eff_task );
}

/**
 *  @brief  フィールドエフェクト削除
 */
static void effect_DelFieldEffect( EFFECT_ENCOUNT* eff_wk )
{
  if( eff_wk->eff_task == NULL){
    return; //なにもしない
  }
  IWASAWA_Printf(" EffectDel <0x%08x\n",eff_wk->eff_task );
  FLDEFF_TASK_CallDelete( eff_wk->eff_task );
  eff_wk->eff_task = NULL;
}

