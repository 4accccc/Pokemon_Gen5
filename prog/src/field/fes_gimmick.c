//===============================================================================
/*
 *  @file   fes_gimmick.c
 *  @brief  フェスミッション フィールドギミック
 *  @author Miyuki Iwasawa
 *  @date   11.08.12
 */
//===============================================================================

#include <gflib.h>
#include "system/main.h"      //GFL_HEAPID_APP参照
#include "system/time_icon.h"
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/g_power.h"
#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_nw.h"
#include "gamesystem/fest_mission_local.h"
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field/script.h"
#include "field/script_local.h"
#include "field/field_msgbg.h"
#include "field/fldeff_fes_kira.h"
#include "./event_fieldmap_control.h"
#include "./event_fmission.h"
#include "./event_subscreen.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "script_message.naix"
#include "message.naix"
#include "msg/script/msg_fmission_event.h"
#include "arc/fieldmap/zone_id.h"
#include "arc/fieldmap/fldmmdl_objcode.h"
#include "../../../resource/fldmapdata/script/fmission_scr_def.h" //SCRID_～
#include "../../../resource/fldmapdata/flagwork/flag_define.h"

#include "field/fes_gimmick.h"
#include "field/fes_gimmick_def.h"
#include "field/fes_gimmick_local.h"


//======================================================================
//======================================================================
#define FES_GIMMICK_ITEM_MAX (5)

#define ZONE_ERROR  (0xFFFF)

typedef struct _FMISSION_ITEM_POS{
  u16 x,y,z;
}FMISSION_ITEM_POS;

typedef struct _FES_GIMMICK_DATA{
  u16  num;
  u16  zone_id;
  FMISSION_ITEM_POS pos[FES_GIMMICK_ITEM_MAX];
}FES_GIMMICK_DATA;

//@todo いずれ外部ファイル化する
static const u16 DATA_FMissionGimmickZone[FES_GIMMICK_DATA_MAX] = {
  ZONE_ID_C01,
  ZONE_ID_R03,
  ZONE_ID_C02,
};

//@todo いずれ外部ファイル化する
static const FES_GIMMICK_DATA  DATA_FMissionGimmickData[FES_GIMMICK_DATA_MAX] = {
  { 5, ZONE_ID_C01,
    {
      { 746, 0, 600 },
      { 769, 0, 603 },
      { 795, 0, 600 },
      { 774, 0, 603 },
      { 771, 0, 587 },
    },
  },
  { 5, ZONE_ID_R03,
    {
      { 759, 0, 563 },
      { 711, 0, 557 },
      { 698, 0, 558 },
      { 681, 0, 582 },
      { 691, 0, 600 },
    },
  },
  { 5, ZONE_ID_C02,
    {
      { 653, 0, 581 },
      { 633, 0, 583 },
      { 608, 0, 598 },
      { 617, 0, 583 },
      { 648, 0, 582 },
    },
  },
};

static void mission_init_mushroom_enc( FES_GIMMICK* wk );

static u16 sub_GetZoneIdx( u16 zone_id );

static void itemball_Setup( LINK_FESTIVAL* fest_sys, const FEST_MISSION* fmwk, GAMESYS_WORK* gsys, u16 zone_id );
static const FES_GIMMICK_DATA* itemball_AllocZoneData( u16 zone_id, HEAPID heapID );
static void itemball_MMdlSet( LINK_FESTIVAL* fest_sys, GFL_STD_RandContext* rnd_c, const FES_GIMMICK_DATA* dat, u16 zone_id, MMDLSYS* mmdlsys );

//======================================================================
//======================================================================

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　管理ワーク初期化
 *
 *  フィールドマップ初期化時にCreateし、フィールド開放時にDeleteする
 */
//----------------------------------------------------------------------
FES_GIMMICK* FES_GIMMICK_SYS_Create( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldWork, HEAPID heapID )
{
  FES_GIMMICK* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FES_GIMMICK));
 
  wk->gsys = gsys;
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
  wk->fest_sys =  GAMESYSTEM_GetLinkFestivalWork( gsys );
  wk->fmwk = (const FEST_MISSION*)LINK_FESTIVAL_GetMissionWork( wk->fest_sys );
  wk->wk_buf = LINK_FESTIVAL_GetMissionGimmickWork( wk->fest_sys );

  wk->fieldWork = fieldWork;
  //イベントデータ
  wk->evdata = GAMEDATA_GetEventData( wk->gdata );
  wk->evwork = GAMEDATA_GetEventWork( wk->gdata );

  {
    const FLDNOGRID_MAPPER* cp_mapper = FIELDMAP_GetFldNoGridMapper( fieldWork );
    if( cp_mapper != NULL ){
      wk->railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );
    }
  }
  return wk;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　管理ワーク開放
 */
//----------------------------------------------------------------------
void FES_GIMMICK_SYS_Delete( FES_GIMMICK* wk )
{
  MI_CpuClear8( wk, sizeof(FES_GIMMICK));
  GFL_HEAP_FreeMemory( wk );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　動作モデルセットアップ処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_SYS_MMdlSetup( FES_GIMMICK* wk, MMDLSYS* mmdl_sys )
{
  //MMDLSYS
  wk->mmdl_sys = mmdl_sys;
  
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_NPC ){
    FES_GIMMICK_NPC_MMdlSetup( wk, wk->wk_buf );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フェス用フィールドエフェクトを初期化
 *
 *  システム初期化のタイミングだと、フィールドエフェクトが未初期化のため
 *  専用のエフェクト初期化関数を用意
 */
//----------------------------------------------------------------------
void FES_GIMMICK_SYS_FldEffSetup( FES_GIMMICK* wk, FLDEFF_CTRL* fectrl, FIELD_PLAYER* f_player )
{
  //フィールドエフェクトコントローラ取得
  wk->fectrl = fectrl;
  
  //エフェクト当たり判定用に自機のMMDLを取得しておく
  wk->field_player = f_player;
  wk->player_mmdl = FIELD_PLAYER_GetMMdl( wk->field_player );
  
  //道具ミッション用エフェクトタスクを常駐で持つ
  wk->eff_task = FLDEFF_FES_KIRA_SetEffect( wk->fectrl );
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フェス用フィールドエフェクトを削除
 */
//----------------------------------------------------------------------
void FES_GIMMICK_SYS_FldEffDelete( FES_GIMMICK* wk )
{
  if( wk->eff_task != NULL ){
    FLDEFF_TASK_CallDelete( wk->eff_task );
    wk->eff_task = NULL;
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック ミッション開始時初期化　
 */
//----------------------------------------------------------------------
void FES_GIMMICK_MissionInitSetup( FES_GIMMICK* wk )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return;
  }
  MI_CpuClear8( LINK_FESTIVAL_GetMissionGimmickWork( wk->fest_sys ), sizeof(FES_GIMMICK_WORK) );

  if( wk->fmwk->arc.m_type == FMISSION_TYPE_MUSHROOM_ENC ){
    mission_init_mushroom_enc( wk );
  }
}

/*
 * @brief タマゲタケシンボルエンカウントミッション用初期化処理
 * これ一つのためにファイルを用意するほどでもなかったのでここに置く
 */
static void mission_init_mushroom_enc( FES_GIMMICK* wk )
{
  int i;

  //フィールド上のすべてのタマゲタケを復活させる
  for( i = FV_FLDMONS_01; i <= FV_FLDMONS_12; i++ ){
    EVENTWORK_ResetEventFlag( wk->evwork, i );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フィールド復帰処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_FieldRecoverSetup( FES_GIMMICK* wk )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ){
    FES_GIMMICK_ITEM_FieldRecoverSetup( wk );
  }else if( wk->fmwk->arc.m_type == FMISSION_TYPE_NPC ){
    FES_GIMMICK_NPC_FieldRecoverSetup( wk, wk->wk_buf );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック 削除　
 */
//----------------------------------------------------------------------
void FES_GIMMICK_Delete( FES_GIMMICK* wk )
{
  void* gmk_wk = LINK_FESTIVAL_GetMissionGimmickWork( wk->fest_sys );

  if( wk->fmwk->arc.m_type == FMISSION_TYPE_NPC ){
    FES_GIMMICK_NPC_Delete( wk, (FES_GIMMICK_NPC_WORK*)gmk_wk );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック イベントチェック
 *
 *  @param  wk
 *  @param  cp_pos  プレイヤー座標
 */
//----------------------------------------------------------------------
GMEVENT* FES_GIMMICK_CheckPosEventGrid( FES_GIMMICK* wk, const VecFx32* cp_pos )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return NULL;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ){
    return FES_GIMMICK_ITEM_CheckEventForGrid( wk, cp_pos );
  }
  return NULL;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック イベントチェック
 *
 *  @param  wk
 *  @param  cp_location  プレイヤー座標
 */
//----------------------------------------------------------------------
GMEVENT* FES_GIMMICK_CheckPosEventRail( FES_GIMMICK* wk, const RAIL_LOCATION* cp_location )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return NULL;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ){
    return FES_GIMMICK_ITEM_CheckEventForRail( wk, cp_location );
  }
  return NULL;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック との当たり判定
 *
 *  @param  wk
 *  @param  cp_location  プレイヤー座標
 */
//----------------------------------------------------------------------
BOOL FES_GIMMICK_CheckHitGridPos( FES_GIMMICK* wk, u16 gx, u16 gz, fx32 height )
{
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return FALSE;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_ITEM ){
    return FES_GIMMICK_ITEM_CheckHitGridPos( wk, gx, gz, height );
  }
  return FALSE;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フラグセット
 */
//----------------------------------------------------------------------
void FES_GIMMICK_FlagSet( FES_GIMMICK* wk, u16 zone_id, u16 idx )
{
  void* gmk_wk = LINK_FESTIVAL_GetMissionGimmickWork( wk->fest_sys );
  
  if( !LINK_FESTIVAL_IsMissionEntry( wk->fest_sys )){
    return;
  }
  if( wk->fmwk->arc.m_type == FMISSION_TYPE_NPC ){
    FES_GIMMICK_NPC_FlagSet( wk, (FES_GIMMICK_NPC_WORK*)gmk_wk, zone_id, idx );
  }
}

//======================================================================
//======================================================================
#if 0
//----------------------------------------------------------------------
/*
 *  @brief  ゾーンキー取得
 */
//----------------------------------------------------------------------
static u16 sub_GetZoneIdx( u16 zone_id )
{
  int i;

  for( i = 0;i < FES_GIMMICK_DATA_MAX;i++){
    if( DATA_FMissionGimmickZone[i] == zone_id ){
      return i;
    }
  }
  return ZONE_ERROR;
}


//----------------------------------------------------------------------
/*
 *  @brief  アイテムボール配置
 */
//----------------------------------------------------------------------
static void itemball_Setup( LINK_FESTIVAL* fest_sys, const FEST_MISSION* fmwk, GAMESYS_WORK* gsys, u16 zone_id )
{
  GFL_STD_RandContext rnd_c;
  GAMEDATA* gdata = GAMESYSTEM_GetGameData(gsys);
  const FES_GIMMICK_DATA* dat = itemball_AllocZoneData( zone_id, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );

  if( dat == NULL){
    return;
  }
  IWASAWA_Printf("FMissionGimmick key=%d\n", fmwk->beacon.key );

  FES_GIMMICK_SUB_RndInit( fmwk->beacon.key, zone_id, &rnd_c );
  itemball_MMdlSet( fest_sys, &rnd_c, dat, zone_id, GAMEDATA_GetMMdlSys( gdata ));
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムボール配置データ取得
 *
 *  @retval NULL 該当データがない
 */
//----------------------------------------------------------------------
static const FES_GIMMICK_DATA* itemball_AllocZoneData( u16 zone_id, HEAPID heapID )
{
  u16 zone_idx = sub_GetZoneIdx( zone_id );

  if( zone_idx != ZONE_ERROR ){
    return &DATA_FMissionGimmickData[zone_idx];
  }
  return NULL;
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムボール配置
 *
 *  @retval NULL 該当データがない
 */
//----------------------------------------------------------------------
static void itemball_MMdlSet( LINK_FESTIVAL* fest_sys, GFL_STD_RandContext* rnd_c, const FES_GIMMICK_DATA* dat, u16 zone_id, MMDLSYS* mmdlsys )
{
  int i, max, rnd;
  u32 tbl[FES_GIMMICK_ITEM_MAX]; 
  MMDL* mmdl;
  u16 zone_idx = sub_GetZoneIdx( zone_id );
 
  if( dat->num < FES_GIMMICK_MAX ){
    max = dat->num;
  }else{
    max = FES_GIMMICK_MAX;
  }
  for( i = 0;i < max;i++){
    tbl[i] = FES_GIMMICK_SUB_RndGet( rnd_c, dat->num, tbl, i );
    rnd = GFL_STD_Rand0( rnd_c, 4);

#if 0
    if(LINK_FESTIVAL_MissionFlagCheck( fest_sys, zone_idx, i )){
      continue;
    }
#endif
    mmdl = MMDLSYS_AddMMdlParam( mmdlsys, dat->pos[tbl[i]].x, dat->pos[tbl[i]].z,
            DIR_DOWN, MMDL_ID_FMISSION_OBJ_01+i, FESITEM, MV_DMY, zone_id );
    MMDL_SetEventID( mmdl, SCRID_FMISSION_ITEM_GET );
    MMDL_SetParam( mmdl, rnd, MMDL_PARAM_0 );
    MMDL_OnMoveBit( mmdl, MMDL_MOVEBIT_NOT_SAVE );  //セーブ対象にしない
  }
}

#endif

