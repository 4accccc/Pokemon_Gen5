//===============================================================================
/*
 *  @file   fmission_gimmick.c
 *  @brief  フェスミッション フィールドギミック
 *  @author Miyuki Iwasawa
 *  @date   11.04.12
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
#include "./event_fieldmap_control.h"
#include "./event_festmission.h"
#include "./event_subscreen.h"

#include "sound/pm_sndsys.h"
#include "sound/swan_sound_data.sadl"

#include "script_message.naix"
#include "message.naix"
#include "msg/script/msg_festmission_event.h"
#include "arc/fieldmap/zone_id.h"
#include "arc/fieldmap/fldmmdl_objcode.h"
#include "../../../resource/fldmapdata/script/fmission_scr_def.h" //SCRID_～

#include "field/fmission_gimmick.h"
#include "field/fmission_gimmick_def.h"

//======================================================================
//======================================================================
#define FMISSION_GIMMICK_ITEM_MAX (5)

#define ZONE_ERROR  (0xFFFF)

typedef struct _FMISSION_ITEM_POS{
  u16 x,y,z;
}FMISSION_ITEM_POS;

typedef struct _FMISSION_GIMMICK_DATA{
  u16  num;
  u16  zone_id;
  FMISSION_ITEM_POS pos[FMISSION_GIMMICK_ITEM_MAX];
}FMISSION_GIMMICK_DATA;

//@todo いずれ外部ファイル化する
static const u16 DATA_FMissionGimmickZone[FMISSION_GIMMICK_DATA_MAX] = {
  ZONE_ID_C01,
  ZONE_ID_R03,
  ZONE_ID_C02,
};

//@todo いずれ外部ファイル化する
static const FMISSION_GIMMICK_DATA  DATA_FMissionGimmickData[FMISSION_GIMMICK_DATA_MAX] = {
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

static u16 sub_GetZoneIdx( u16 zone_id );

static void itemball_Setup( LINK_FESTIVAL* fest_sys, const FEST_MISSION* fmwk, GAMESYS_WORK* gsys, u16 zone_id );
static const FMISSION_GIMMICK_DATA* itemball_AllocZoneData( u16 zone_id, HEAPID heapID );
static void itemball_MMdlSet( LINK_FESTIVAL* fest_sys, GFL_STD_RandContext* rnd_c, const FMISSION_GIMMICK_DATA* dat, u16 zone_id, MMDLSYS* mmdlsys );

static void rnd_Init( const u32 key, const u16 zone_id, GFL_STD_RandContext* rnd_c );
static u8 rnd_Get( GFL_STD_RandContext* rnd_c, u32 range, u8* p_tbl, int tbl_max );
static BOOL rnd_CheckConflict( u8 value, u8* p_tbl, int tbl_max );

//======================================================================
//======================================================================
//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　マップ遷移後処理
 */
//----------------------------------------------------------------------
void FMISSION_GIMMICK_MapChangeSetup( LINK_FESTIVAL* fest_sys, GAMESYS_WORK* gsys, u16 zone_id )
{
  const FEST_MISSION* fmwk = (const FEST_MISSION*)LINK_FESTIVAL_GetMissionWork( fest_sys );

  if( fmwk->type == FMISSION_TYPE_BALL_ITEM ){
    itemball_Setup( fest_sys, fmwk, gsys, zone_id );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック 削除　
 */
//----------------------------------------------------------------------
void FMISSION_GIMMICK_Delete( LINK_FESTIVAL* fest_sys, GAMESYS_WORK* gsys )
{
  int i;
  GAMEDATA* gdata = GAMESYSTEM_GetGameData(gsys);
  MMDLSYS* mmdlsys =  GAMEDATA_GetMMdlSys( gdata );

  for(i = 0;i < FMISSION_GIMMICK_MAX;i++){
    MMDL* mmdl = MMDLSYS_SearchOBJID( mmdlsys, MMDL_ID_FMISSION_OBJ_01+i );
    if( mmdl != NULL ){
      MMDL_Delete( mmdl);
    }
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　フラグセット
 */
//----------------------------------------------------------------------
void FMISSION_GIMMICK_FlagSet( LINK_FESTIVAL* fest_sys, u16 zone_id, u16 idx )
{
  u16 zone_idx = sub_GetZoneIdx( zone_id );

  if( zone_idx != ZONE_ERROR ){
    LINK_FESTIVAL_MissionFlagSet( fest_sys, zone_idx, idx );
  }
}

//======================================================================
//======================================================================

//----------------------------------------------------------------------
/*
 *  @brief  ゾーンキー取得
 */
//----------------------------------------------------------------------
static u16 sub_GetZoneIdx( u16 zone_id )
{
  int i;

  for( i = 0;i < FMISSION_GIMMICK_DATA_MAX;i++){
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
  const FMISSION_GIMMICK_DATA* dat = itemball_AllocZoneData( zone_id, GFL_HEAP_LOWID(HEAPID_FIELDMAP) );

  if( dat == NULL){
    return;
  }
  IWASAWA_Printf("FMissionGimmick key=%d\n", fmwk->key );

  rnd_Init( fmwk->key, zone_id, &rnd_c );
  itemball_MMdlSet( fest_sys, &rnd_c, dat, zone_id, GAMEDATA_GetMMdlSys( gdata ));
}

//----------------------------------------------------------------------
/*
 *  @brief  アイテムボール配置データ取得
 *
 *  @retval NULL 該当データがない
 */
//----------------------------------------------------------------------
static const FMISSION_GIMMICK_DATA* itemball_AllocZoneData( u16 zone_id, HEAPID heapID )
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
static void itemball_MMdlSet( LINK_FESTIVAL* fest_sys, GFL_STD_RandContext* rnd_c, const FMISSION_GIMMICK_DATA* dat, u16 zone_id, MMDLSYS* mmdlsys )
{
  int i, max, rnd;
  u8 tbl[FMISSION_GIMMICK_ITEM_MAX]; 
  MMDL* mmdl;
  u16 zone_idx = sub_GetZoneIdx( zone_id );
 
  if( dat->num < FMISSION_GIMMICK_MAX ){
    max = dat->num;
  }else{
    max = FMISSION_GIMMICK_MAX;
  }
  for( i = 0;i < max;i++){
    tbl[i] = rnd_Get( rnd_c, dat->num, tbl, i );
    rnd = GFL_STD_Rand0( rnd_c, 4);

    if(LINK_FESTIVAL_MissionFlagCheck( fest_sys, zone_idx, i )){
      continue;
    }
    mmdl = MMDLSYS_AddMMdlParam( mmdlsys, dat->pos[tbl[i]].x, dat->pos[tbl[i]].z,
            DIR_DOWN, MMDL_ID_FMISSION_OBJ_01+i, FESITEM, MV_DMY, zone_id );
    MMDL_SetEventID( mmdl, SCRID_FMISSION_ITEM_BALL_GET );
    MMDL_SetParam( mmdl, rnd, MMDL_PARAM_0 );
    MMDL_OnMoveBit( mmdl, MMDL_MOVEBIT_NOT_SAVE );  //セーブ対象にしない
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  乱数コンテキスト初期化
 *  ミッションキーで初期化し、zone_id分まわす
 */
//----------------------------------------------------------------------
static void rnd_Init( const u32 key, const u16 zone_id, GFL_STD_RandContext* rnd_c )
{
  int i;
  u8 count;

  GFL_STD_RandInit( rnd_c, ((u64)key << 32) | (u64)key );

  count = (zone_id>>8)^(zone_id & 0xFF);

  for( i = 0;i < count;i++){
    GFL_STD_Rand( rnd_c, GFL_STD_RAND_MAX );
  }
}

//----------------------------------------------------------------------
/*
 *  @brief  乱数取得
 */
//----------------------------------------------------------------------
static u8 rnd_Get( GFL_STD_RandContext* rnd_c, u32 range, u8* p_tbl, int tbl_max )
{
  u8 value;

  do{
    value = GFL_STD_Rand0( rnd_c, range );
  }while( rnd_CheckConflict( value, p_tbl, tbl_max ));

  return value;
}

//----------------------------------------------------------------------
/*
 *  @brief  乱数衝突検査
 */
//----------------------------------------------------------------------
static BOOL rnd_CheckConflict( u8 value, u8* p_tbl, int tbl_max )
{
  int i;

  for(i = 0;i < tbl_max;i++){
    if(p_tbl[i] == value){
      return TRUE;
    }
  }
  return FALSE;
}

