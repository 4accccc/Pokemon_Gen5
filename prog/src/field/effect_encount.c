/**
 *  @file   encount_effect.c
 *  @brief  エンカウントエフェクト制御
 *  @author Miyuki Iwasawa
 *  @date   09.11.17
 */

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "item/itemsym.h"
#include "fieldmap.h"
#include "map_attr.h"

#include "savedata/misc.h"
#include "field_encount.h"
#include "fldeff_encount.h"
#include "script.h"
#include "script_def.h"

#include "encount_data.h"
#include "field_encount.h"
#include "field_encount_local.h"
#include "effect_encount.h"

#include "../../../resource/fldmapdata/script/field_ev_scr_def.h" // for SCRID_FLD_EV_EFFECT_ENC_ITEM_GET

///////////////////////////////////////////////////////////////
//
#define EFFENC_SEARCH_OX (5)
#define EFFENC_SEARCH_OZ (5)
#define EFFENC_SEARCH_WX  (EFFENC_SEARCH_OX*2+1)
#define EFFENC_SEARCH_WZ  (EFFENC_SEARCH_OZ*2+1)
#define EFFENC_SEARCH_AREA_MAX  (EFFENC_SEARCH_WX*EFFENC_SEARCH_WZ)

#define GRID_MAP_W  (32)

typedef struct _EFFENC_ATTR_POS{
  u8  type;
  u16  gx,gy,gz;
  fx32  height;
}EFFENC_ATTR_POS;

typedef struct _EFFENC_SEARCH{
  s16 player_x;
  s16 player_z;
  s16 player_lx;
  s16 player_lz;
  s16 block_x,block_z;
  u16 block_ox,block_oz;
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
  u16 get_item; //スクリプトに取得アイテムNoを受け渡すワーク
};

///////////////////////////////////////////////////////////////////////////////////////////
//プロトタイプ
///////////////////////////////////////////////////////////////////////////////////////////
static void effect_EffectPush( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );
static void effect_EffectPop( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );

static BOOL effenc_CheckWalkCt( ENCOUNT_WORK* ewk, EFFECT_ENCOUNT* eff_wk );
static void effenc_WalkCtClear( ENCOUNT_WORK* ewk );


static void effect_AttributeSearch( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );
static void effect_EffectSetUp( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );
static BOOL effect_CheckMMdlHit( FIELD_ENCOUNT* enc, s16 x,s16 y, s16 z, BOOL player_egnore_f );
static BOOL effect_DeleteCheck( FIELD_ENCOUNT* enc );
static void effect_EffectDelete( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk );

static void effect_AddFieldEffect( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk, EFFENC_PARAM* ep );
static void effect_DelFieldEffect( EFFECT_ENCOUNT* eff_wk );

static u16 effitem_GetItemBridge(void);
static u16 effitem_GetItemCave(void);
static GMEVENT* effitem_ItemGetEventSet( FIELD_ENCOUNT* enc, u16 itemno );

#ifdef PM_DEBUG
static EFFENC_ATTR_POS* debug_EffectPosAdjust( FIELD_ENCOUNT*enc, EFFECT_ENCOUNT* eff_wk, EWK_EFFECT_ENCOUNT* wk, u16 org_idx );

#endif  //PM_DEBUG

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
  //フィールドエフェクトコントローラ取得
  eff_wk->fectrl = FIELDMAP_GetFldEffCtrl( enc->fwork );

  //初期インターバルと確率指定
  EFFECT_ENC_SetProb( enc, eff_wk );

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
 *  @brief  エフェクトエンカウント 発生確率&歩数インターバル設定
 */
void EFFECT_ENC_SetProb( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  u8 prob,interval,type;

  static const u8 prob_tbl[][2] = {
    { EFFENC_NORMAL_INTERVAL, EFFENC_NORMAL_PROB },
    { EFFENC_CAVE_INTERVAL, EFFENC_CAVE_PROB },
    { EFFENC_BRIDGE_INTERVAL, EFFENC_BRIDGE_PROB },
  };

  if( enc->encdata->effenc_type > 2){
    type = 0;
    GF_ASSERT(0);
  }else{
    type = enc->encdata->effenc_type;
  }
  interval = prob_tbl[type][0];
  prob = prob_tbl[type][1];

#ifdef PM_DEBUG
  {
    ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork( enc->gdata );
    EWK_EFFECT_ENCOUNT* wk = &ewk->effect_encount;

    if( wk->deb_prob_fix ){
      eff_wk->walk_ct_interval = wk->deb_interval;
      eff_wk->prob = wk->deb_prob;
    }else{
      eff_wk->walk_ct_interval = wk->deb_interval = interval;
      eff_wk->prob = wk->deb_prob = prob;
    }
  }
#else
  eff_wk->walk_ct_interval = interval;
  eff_wk->prob = prob;
#endif
}


/*
 *  @brief  エンカウントエフェクト起動チェック
 */
void EFFECT_ENC_CheckEffectEncountStart( FIELD_ENCOUNT* enc )
{
  EFFECT_ENCOUNT* eff_wk = enc->eff_enc;
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  
  //エンカウントデータチェック
  if( enc->encdata == NULL || !enc->encdata->enable_f ){
    return;
  }

  //最初のジムバッジ入手前は起動しない
  if( !MISC_GetBadgeFlag( GAMEDATA_GetMiscWork(enc->gdata), BADGE_ID_01 )){
    return;
  }

  //グリッドベースマップかチェック
  if( FIELDMAP_GetBaseSystemType( enc->fwork ) != FLDMAP_BASESYS_GRID ){
    return;
  }
  //起動中チェック
  if( ewk->effect_encount.param.valid_f ){
    return;
  }
  //歩数チェック
  if( !effenc_CheckWalkCt( ewk, eff_wk ) ){
    return;
  }
  //確率チェック
  if( GFUser_GetPublicRand0( 1000 ) >= (eff_wk->prob*10) ){
    effenc_WalkCtClear( ewk );
    return;
  }

  //アトリビュートサーチ
  effect_AttributeSearch( enc, eff_wk );

  //抽選
  if( eff_wk->attr_map.count == 0){
    return;
  }
 
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
 *  @brief  エフェクトエンカウント　OBJとの接触によるエフェクト破棄チェック
 */
void EFFECT_ENC_CheckObjHit( FIELD_ENCOUNT* enc )
{
  effect_DeleteCheck( enc );
}

/*
 *  @brief  エフェクトエンカウント　座標チェック
 */
static inline BOOL effect_CheckEffectPos( const EFFENC_PARAM* ep, MMDL_GRIDPOS* pos )
{
  if( ep->gx != pos->gx ||
      ep->gz != pos->gz ||
      ep->gy != pos->gy){
    return FALSE;
  }
  return TRUE;
}

/*
 *  @brief  エフェクトエンカウント　座標チェック
 */
BOOL EFFECT_ENC_CheckEffectPos( const FIELD_ENCOUNT* enc, MMDL_GRIDPOS* pos )
{
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);

  if( !ewk->effect_encount.param.valid_f ){
    return FALSE;
  }
  return effect_CheckEffectPos( &ewk->effect_encount.param, pos );
}

/*
 *  @brief  エフェクトエンカウント　イベント侵入起動チェック
 */
GMEVENT* EFFECT_ENC_CheckEventApproch( FIELD_ENCOUNT* enc )
{
  EFFECT_ENCOUNT* eff_wk = enc->eff_enc;
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  EFFENC_PARAM* ep = &ewk->effect_encount.param; 
  GMEVENT* event = NULL;
  u8  ev_no = 0;
  u16 rnd;

  if( !ep->valid_f ){
    return NULL;
  }
  { //座標チェック
    MMDL_GRIDPOS pos;
    FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
    MMDL* mmdl = FIELD_PLAYER_GetMMdl( fplayer );
    MMDL_GetGridPos( mmdl, &pos );

    if( !effect_CheckEffectPos( ep, &pos )){
      return NULL;
    }
  }
  //イベント分岐
  rnd = GFUser_GetPublicRand0( 1000 );
  if( ep->type == EFFENC_TYPE_BRIDGE ){
    if( rnd < 200 ){
      event = FIELD_ENCOUNT_CheckEncount( enc, ENC_TYPE_EFFECT );
    }else{
      event = effitem_ItemGetEventSet( enc, effitem_GetItemBridge() );
      ev_no = 1;
    } 
  }else if( ep->type == EFFENC_TYPE_CAVE ){
    if( rnd < 400){
      event = FIELD_ENCOUNT_CheckEncount( enc, ENC_TYPE_EFFECT );
    }else{
      event = effitem_ItemGetEventSet( enc, effitem_GetItemCave() );
      ev_no = 1;
    }
  }else{
    event = FIELD_ENCOUNT_CheckEncount( enc, ENC_TYPE_EFFECT );
  }
  if( event == NULL){
    effect_EffectDelete( enc, eff_wk );
    return NULL;
  }
//  GF_ASSERT( event );

  if(ev_no){
    effect_EffectDelete( enc, eff_wk );
  }else{
    ep->push_cancel_f = TRUE;
  }
  return event;
}

/*
 *  @brief  エフェクトエンカウント　自機位置との距離を返す
 *
 *  @retval エフェクトがないときはFALSEを返す
 */
BOOL EFFECT_ENC_GetDistanceToPlayer( FIELD_ENCOUNT* enc, u16* o_distance )
{
  EFFECT_ENCOUNT* eff_wk = enc->eff_enc;
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  EFFENC_PARAM* ep = &ewk->effect_encount.param; 
  
  if( !ep->valid_f ){
    *o_distance = 0;
    return FALSE;
  }
  { //座標チェック
    s16 dx,dz;
    MMDL_GRIDPOS pos;
    MMDL* mmdl = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( enc->fwork ) );
    MMDL_GetGridPos( mmdl, &pos );

    dx = ep->gx - pos.gx;
    if(dx < 0){
      dx *= -1;
    }
    dz = ep->gz - pos.gz;
    if( dz < 0 ){
      dz *= -1;
    }
    if( dx > dz){
      *o_distance = dx;
    }else{
      *o_distance = dz;
    }
  }
  return TRUE;
}

/*
 *  @brief  エフェクトエンカウント　フィールド生成時エフェクト復帰強制キャンセル
 */
void EFFECT_ENC_EffectRecoverCancel( FIELD_ENCOUNT* enc )
{
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  EFFENC_PARAM* ep = &ewk->effect_encount.param; 

  if( ep->valid_f ){
    ep->push_cancel_f = TRUE;
    if( enc->eff_enc->eff_task != NULL){
      FLDEFF_ENCOUNT_AnmPauseSet( enc->eff_enc->eff_task,TRUE );
    }
  }
}

/*
 *  @brief  エフェクトエンカウント　アニメ再生ポーズ
 */
void EFFECT_ENC_EffectAnmPauseSet( FIELD_ENCOUNT* enc, BOOL pause_f )
{
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  EFFECT_ENCOUNT* eff_wk = enc->eff_enc; 
  
  //起動中チェック
  if( !ewk->effect_encount.param.valid_f ){
    return;
  }
  if( eff_wk->eff_task != NULL){
    FLDEFF_ENCOUNT_AnmPauseSet( eff_wk->eff_task, pause_f );
  }
}

//--------------------------------------------------------------------
/**
 * @brief   エフェクトエンカウトアイテム取得イベントで手に入れるItemNoを取得する
 *
 * @retval itemno
 */
//--------------------------------------------------------------------
u16 EFFECT_ENC_GetEffectEncountItem( FIELD_ENCOUNT* enc )
{
  return enc->eff_enc->get_item;
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
  effect_AddFieldEffect( enc, eff_wk, ep );  //保存されたパラメータで復帰
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
    return TRUE;
  }
  return FALSE;
}
static void effenc_WalkCtClear( ENCOUNT_WORK* ewk )
{
  ewk->effect_encount.walk_ct = 0;
}

/*
 *  @brief  エフェクトアトリビュート サーチ領域取得
 */
static void effect_SearchAreaGet( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk, EFFENC_SEARCH* esw )
{
  s16 sx,ex,sz,ez;
  const s8* tbl;
  static const s8 DATA_SearchTbl[][4] = {
    { -EFFENC_SEARCH_OX, EFFENC_SEARCH_OX, -EFFENC_SEARCH_OZ, EFFENC_SEARCH_OZ }, //All
    { 0, EFFENC_SEARCH_OX, -EFFENC_SEARCH_OZ, EFFENC_SEARCH_OZ }, //右
    { -EFFENC_SEARCH_OX, 0, -EFFENC_SEARCH_OZ, EFFENC_SEARCH_OZ }, //左
    { -EFFENC_SEARCH_OX, EFFENC_SEARCH_OX, -EFFENC_SEARCH_OZ, 0 }, //上
    { -EFFENC_SEARCH_OX, EFFENC_SEARCH_OX, 0, EFFENC_SEARCH_OZ }, //下
  };

  {
    FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
    MMDL* mmdl = FIELD_PLAYER_GetMMdl( fplayer );

    esw->player_x = MMDL_GetGridPosX( mmdl );
    esw->player_z = MMDL_GetGridPosZ( mmdl );
    esw->player_lx = esw->player_x % GRID_MAP_W;
    esw->player_lz = esw->player_z % GRID_MAP_W;
    esw->block_x = esw->player_x / GRID_MAP_W;
    esw->block_z = esw->player_z / GRID_MAP_W;
    esw->block_ox = esw->player_x-esw->player_lx;
    esw->block_oz = esw->player_z-esw->player_lz;

    if( enc->encdata->effenc_type != EFFENC_SIT_BRIDGE ){
      tbl = DATA_SearchTbl[GFUser_GetPublicRand0( 4 )+1];
    }else{
      tbl = DATA_SearchTbl[0];
    }
  }
  sx = esw->player_lx + tbl[0];
  if( sx < 0 ){
    sx = 0;
  }
  ex = esw->player_lx + tbl[1];
  if( ex >= GRID_MAP_W){
    ex = GRID_MAP_W-1;
  }
  sz = esw->player_lz + tbl[2];
  if( sz < 0 ){
    sz = 0;
  }
  ez = esw->player_lz + tbl[3];
  if( ez >= GRID_MAP_W){
    ez = GRID_MAP_W-1;
  }

  esw->sx = sx + esw->block_ox;
  esw->ex = ex + esw->block_ox;
  esw->sz = sz + esw->block_oz;
  esw->ez = ez + esw->block_oz; 
}

/*
 *  @brief  エフェクトアトリビュートサーチ
 */
static void effect_AttributeSearch( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk )
{
  int blockIdx;
  s16 i,j;
  VecFx32 vec;
  fx32  vx;
  EFFENC_TYPE_ID id;
  EFFENC_SEARCH esw;
  FLDMAPPER_GRIDINFODATA gridData;
  const FLDMAPPER* g3dMapper = (const FLDMAPPER*)FIELDMAP_GetFieldG3Dmapper( enc->fwork ); 

#ifdef DEBUG_ONLY_FOR_iwasawa
  OSTick start_tick,end_tick;
#endif

  MI_CpuClear8(&eff_wk->attr_map,sizeof(EFFENC_ATTR_MAP));
  vec.y = 0;
  blockIdx = FLDMAPPER_GetCurrentBlockAccessIdx( g3dMapper );
 
  if( !FLDMAPPER_IsGridDataEnableForEffectEncount(g3dMapper, blockIdx)){
    return;
  }
  //検索領域取得
  effect_SearchAreaGet( enc, eff_wk, &esw );
/*
  IWASAWA_Printf("EffSearch\nPlayer(%d,%d), x(%d～%d), z(%d～%d)\n",
      esw.player_x,esw.player_z,esw.sx,esw.ex,esw.sz,esw.ez);
*/

#ifdef DEBUG_ONLY_FOR_iwasawa
  start_tick = OS_GetTick();
#endif
  
 
  vec.z = FX32_CONST(esw.sz*16)+FX32_CONST(8);
  vx = FX32_CONST(esw.sx*16)+FX32_CONST(8);

  for(i = esw.sz; i <= esw.ez;i++){
    vec.x = vx; 
    for(j = esw.sx; j <= esw.ex;j++){
      FLDMAPPER_GetGridDataForEffectEncount( g3dMapper, blockIdx, &vec, &gridData );
      
      id = MAPATTR_GetEffectEncountType( gridData.attr );
      if(id != EFFENC_TYPE_MAX ){
        eff_wk->attr_map.attr[eff_wk->attr_map.count].type = id;
        eff_wk->attr_map.attr[eff_wk->attr_map.count].gx = j;
        eff_wk->attr_map.attr[eff_wk->attr_map.count].gz = i;
        eff_wk->attr_map.attr[eff_wk->attr_map.count++].height = gridData.height;
      }
      vec.x += FX32_CONST(16); 
    }
    vec.z += FX32_CONST(16); 
  }
#ifdef DEBUG_ONLY_FOR_iwasawa
  {
    end_tick = OS_GetTick()-start_tick;
    IWASAWA_Printf("EncEffAttrSearch tick = %d count %d\n",end_tick,eff_wk->attr_map.count);
  }
#endif
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

#ifdef PM_DEBUG
  //エフェクト出現位置デバッグ調整
  attr = debug_EffectPosAdjust( enc, eff_wk, &ewk->effect_encount, idx);
  if( attr == NULL ){
    return;
  }
#endif

  attr->gy = SIZE_GRID_FX32( attr->height );

  //フィールドモデルの位置をチェック
  if( effect_CheckMMdlHit( enc, attr->gx, attr->gy, attr->gz, FALSE )){
    IWASAWA_Printf("HitCancel FieldModelHit\n");
    return;
  }

  //パラメータセット
  ep->gx = attr->gx;
  ep->gy = attr->gy;
  ep->gz = attr->gz;
  ep->height = attr->height;
  ep->type = attr->type;
  ep->zone_id = FIELDMAP_GetZoneID( enc->fwork );
  ep->valid_f = TRUE;

  //カウンタークリア
  effenc_WalkCtClear( ewk );

  effect_AddFieldEffect( enc, eff_wk, ep );
}

/*
 *  @brief  エフェクト座標チェック
 */
static BOOL effect_CheckMMdlHit( FIELD_ENCOUNT* enc, s16 x,s16 y, s16 z, BOOL player_egnore_f )
{
  const MMDLSYS* fos;
  const OBJCODE_PARAM* obj_prm;

  MMDL_GRIDPOS pos;
  MMDL* mmdl;
  u32 i = 0;
  MMDL* fplayer = FIELD_PLAYER_GetMMdl( FIELDMAP_GetFieldPlayer( enc->fwork ) );

  fos = FIELDMAP_GetMMdlSys( enc->fwork );
 
//  IWASAWA_Printf(" req ( %d, %d, %d ) player_egnore %d\n", x, y, z, player_egnore_f);
  while(TRUE){
    if(!MMDLSYS_SearchUseMMdl( fos, &mmdl, &i)){
      break;
    }
    if( MMDL_CheckEndAcmd(mmdl) == FALSE ){
      continue;
    }
    if( (mmdl == fplayer) && player_egnore_f ){
//      IWASAWA_Printf("player_egnore\n");
      continue;
    }
    obj_prm = MMDL_GetOBJCodeParam( mmdl );

    MMDL_GetGridPos(mmdl,&pos);
    if( y != pos.gy ||
        x < pos.gx || 
        x >= (pos.gx+obj_prm->size_width) ||
        z > pos.gz ||
        z <= (pos.gz-obj_prm->size_depth) ){
//      IWASAWA_Printf("pos( %d, %d, %d ) size( %d, %d)\n",pos.gx,pos.gy,pos.gz,obj_prm->size_width, obj_prm->size_depth);
      continue;
    }
    return TRUE;
  }
  return FALSE;
}

/*
 *  @brief  フィールドモデルとの接触によるエフェクト破棄
 */
static BOOL effect_DeleteCheck( FIELD_ENCOUNT* enc )
{
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork(enc->gdata);
  EFFENC_PARAM* ep = &ewk->effect_encount.param; 
  
  if( !ep->valid_f ){
    return FALSE;
  }
  if( effect_CheckMMdlHit( enc, ep->gx, ep->gy, ep->gz, TRUE )){
    effect_EffectDelete( enc, enc->eff_enc );
    return TRUE;
  }
  return FALSE;
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
static void effect_AddFieldEffect( FIELD_ENCOUNT* enc, EFFECT_ENCOUNT* eff_wk, EFFENC_PARAM* ep )
{
  if( eff_wk->eff_task != NULL){
    GF_ASSERT(0);
    return;
  }
  eff_wk->eff_task = FLDEFF_ENCOUNT_SetEffect( enc, eff_wk->fectrl, ep->gx, ep->gz, ep->height, ep->type );

  if( eff_wk->eff_task == NULL ){
    GF_ASSERT(0);
    MI_CpuClear8( ep, sizeof(EFFENC_PARAM));
    return;
  }
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


///////////////////////////////////////////////////////////////
//アイテムリストデータ

#define ITEM_TBL_STONE_NUM (10)
#define ITEM_TBL_JUWEL_NUM (17)
#define ITEM_TBL_FEATHER_NUM (6)
static const u16 DATA_ItemTableStone[ITEM_TBL_STONE_NUM] = {
 ITEM_TAIYOUNOISI,
 ITEM_TUKINOISI,
 ITEM_HONOONOISI,
 ITEM_KAMINARINOISI,
 ITEM_MIZUNOISI,
 ITEM_RIIHUNOISI,
 ITEM_HIKARINOISI,
 ITEM_YAMINOISI,
 ITEM_MEZAMEISI,
 ITEM_MANMARUISI,
};

static u16 effitem_GetItemCave(void)
{
  u16 rnd = GFUser_GetPublicRand0( 1000 );

  if(rnd < 100){ //石
    rnd = GFUser_GetPublicRand0(ITEM_TBL_STONE_NUM*100) / 100;
    return DATA_ItemTableStone[rnd];
  }else if( rnd < 950 ){
    rnd = GFUser_GetPublicRand0(ITEM_TBL_JUWEL_NUM*100) / 100;
    return ITEM_HONOONOZYUERU+rnd;
  }
  return ITEM_KAWARAZUNOISI;
}

static u16 effitem_GetItemBridge(void)
{
  u16 rnd = GFUser_GetPublicRand0( 1000 );

  if( rnd < 900){
    rnd = GFUser_GetPublicRand0(ITEM_TBL_FEATHER_NUM*100) / 100;
    return ITEM_TAIRYOKUNOHANE+rnd;
  }
  return ITEM_KIREINAHANE;
}

static GMEVENT* effitem_ItemGetEventSet( FIELD_ENCOUNT* enc, u16 itemno )
{
  EFFECT_ENCOUNT* eff_wk = enc->eff_enc;
 
  eff_wk->get_item = itemno;
  return SCRIPT_SetEventScript( enc->gsys, SCRID_FLD_EV_EFFECT_ENC_ITEM_GET, NULL, FIELDMAP_GetHeapID( enc->fwork ) );
}


/////////////////////////////////////////////////////////////////////////
//デバッグ用ルーチン
#ifdef PM_DEBUG

/**
 *  @brief  デバッグ数値入力　
 */
u32 EFFENC_DEB_NumInputParamGet( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param )
{
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_ENCOUNT* encount = FIELDMAP_GetEncount( fieldmap ); 
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork( gamedata );
  EFFECT_ENCOUNT* eff_wk = encount->eff_enc;
  EWK_EFFECT_ENCOUNT* wk = &ewk->effect_encount;

  switch( param ){
  case EFFENC_DNI_INTERVAL: 
    return wk->deb_interval;
  case EFFENC_DNI_PROB: 
    return wk->deb_prob;
  case EFFENC_DNI_PROB_FIX: 
    return wk->deb_prob_fix;
  case EFFENC_DNI_OFSX: 
    return wk->deb_ofsx;
  case EFFENC_DNI_OFSZ: 
    return wk->deb_ofsz;
  case EFFENC_DNI_POSITION: 
    {
      u32 num = 0;
      s32 x,y,z;
      if( !wk->param.valid_f){
        return 0;
      }
      x = wk->param.gx;
      z = wk->param.gz;
      y = wk->param.gy;
      if(y < 0 ){ y += 999; }
      return (x*1000000+y*1000+z);
    }
    break;
  case EFFENC_DNI_ADD_DELETE:
    return wk->param.valid_f;
  }
  return 0;
}

///値を設定するための関数
void EFFENC_DEB_NumInputParamSet( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param, u32 value )
{
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_ENCOUNT* encount = FIELDMAP_GetEncount( fieldmap ); 
  ENCOUNT_WORK* ewk = GAMEDATA_GetEncountWork( gamedata );
  EFFECT_ENCOUNT* eff_wk = encount->eff_enc;
  EWK_EFFECT_ENCOUNT* wk = &ewk->effect_encount;

  switch( param ){
  case EFFENC_DNI_INTERVAL: 
    wk->deb_interval = value;
    eff_wk->walk_ct_interval = wk->deb_interval;
    break;
  case EFFENC_DNI_PROB: 
    wk->deb_prob = value;
    eff_wk->prob = wk->deb_prob;
    break;
  case EFFENC_DNI_PROB_FIX: 
    wk->deb_prob_fix = value;
    break;
  case EFFENC_DNI_OFSX: 
    wk->deb_ofsx = value;
    break;
  case EFFENC_DNI_OFSZ: 
    wk->deb_ofsz = value;
    break;
  case EFFENC_DNI_ADD_DELETE:
    if( wk->param.valid_f && value == 0 ){
      effect_EffectDelete( encount, eff_wk );
    }else if( !wk->param.valid_f && value == 1 ){
      EFFECT_ENC_CheckEffectEncountStart( encount );
    }
    break;
  }
}

/**
 *  @brief  デバッグ　エフェクト出現位置調整
 */
static EFFENC_ATTR_POS* debug_EffectPosAdjust( FIELD_ENCOUNT*enc, EFFECT_ENCOUNT* eff_wk, EWK_EFFECT_ENCOUNT* wk, u16 org_idx )
{
  int i;
  s16 gx,gz,ox,oz;
  EFFENC_ATTR_POS* pos;
  FIELD_PLAYER *fplayer = FIELDMAP_GetFieldPlayer( enc->fwork );
  MMDL* mmdl = FIELD_PLAYER_GetMMdl( fplayer );

  ox = oz = 0;
  if(wk->deb_ofsx){
    if( wk->deb_ofsx > 5 ){
      ox = (wk->deb_ofsx-5)*-1;
    }else{
      ox = wk->deb_ofsx;
    }
  }
  if(wk->deb_ofsz){
    if( wk->deb_ofsz > 5 ){
      oz = (wk->deb_ofsz-5)*-1;
    }else{
      oz = wk->deb_ofsz;
    }
  }
  gx = MMDL_GetGridPosX( mmdl ) + ox;
  gz = MMDL_GetGridPosZ( mmdl ) + oz;
  IWASAWA_Printf("指定　%d,%d\n",gx,gz);

  if(wk->deb_ofsx == 0 && wk->deb_ofsz == 0){
    return &eff_wk->attr_map.attr[org_idx];
  }
  for( i = 0;i < eff_wk->attr_map.count;i++){
    pos = &eff_wk->attr_map.attr[i];
    if( pos->gx != gx || pos->gz != gz) {
      continue;
    }
    return &eff_wk->attr_map.attr[i];
  }
#ifdef DEBUG_ONLY_FOR_iwasawa
  return NULL;
#else
  return &eff_wk->attr_map.attr[org_idx];
#endif
}

#endif  //PM_DEBUG



