//===============================================================================
/*
 *  @file   fes_gimmick.c
 *  @brief  フェスミッション フィールドギミック(NOTWIFI)
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

#include "arc/fieldmap/zone_id.h"
#include "arc/fieldmap/fldmmdl_objcode.h"

#include "field/fes_gimmick.h"
#include "field/fes_gimmick_def.h"
#include "field/fes_gimmick_local.h"

SDK_COMPILER_ASSERT(sizeof(FES_GIMMICK_WORK) <= FMISSION_WORK_AREA_LEN );

static BOOL rnd_CheckConflict( u32 value, u32* p_tbl, int tbl_max );

//----------------------------------------------------------------------
/*
 *  @brief  乱数コンテキスト初期化に用いるゾーンIDを返す
 *
 *  バージョンでゾーンが異なるR04などで、抽選結果を合わせるための処理
 *  ミッションキーで初期化し、zone_id分まわす
 */
//----------------------------------------------------------------------
u16 FES_GIMMICK_SUB_GetRndInitZone( const u16 zone_id )
{
  int i;
  static const u16 tbl[][2] = {
    { ZONE_ID_R04,      ZONE_ID_R04W },
    { ZONE_ID_D24R0101,	ZONE_ID_D24R0701 },
    { ZONE_ID_D24R0201,	ZONE_ID_D24R0801 },
    { ZONE_ID_D24R0301,	ZONE_ID_D24R0901 },
    { ZONE_ID_D24R0401,	ZONE_ID_D24R1001 },
    { ZONE_ID_D24R0501,	ZONE_ID_D24R1101 },
    { ZONE_ID_D24R0601,	ZONE_ID_D24R1201 },
    { ZONE_ID_D26R0201,	ZONE_ID_D26R0901 },
    { ZONE_ID_D26R0401,	ZONE_ID_D26R1001 },
    { ZONE_ID_BC10,	    ZONE_ID_WC10 },
  };

  for( i = 0; NELEMS(tbl);i++){
    if( zone_id == tbl[i][0] ) return tbl[i][1];
  }
  return zone_id;
}

//----------------------------------------------------------------------
/*
 *  @brief  乱数コンテキスト初期化
 *  ミッションキーで初期化し、zone_id分まわす
 */
//----------------------------------------------------------------------
void FES_GIMMICK_SUB_RndInit( const u32 key, const u16 zone_id, GFL_STD_RandContext* rnd_c )
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
u32 FES_GIMMICK_SUB_RndGet( GFL_STD_RandContext* rnd_c, u32 range, u32* p_tbl, int tbl_max )
{
  u32 value;

  do{
    value = GFL_STD_Rand0( rnd_c, range );
  }while( rnd_CheckConflict( value, p_tbl, tbl_max ));

  return value;
}

//----------------------------------------------------------------------
/*
 *  @brief  フェスミッションギミック　ゾーン遷移時処理
 */
//----------------------------------------------------------------------
void FES_GIMMICK_ZoneChangeSetup( GAMESYS_WORK* gsys, u16 zone_id )
{
  LINK_FESTIVAL* fest_sys = GAMESYSTEM_GetLinkFestivalWork( gsys );
  void* gmk_wk = LINK_FESTIVAL_GetMissionGimmickWork( fest_sys );
  FEST_MISSION* fmwk;

  OSTick s_tick = OS_GetTick();

  if( !LINK_FESTIVAL_IsMissionEntry( fest_sys )){
    return;
  }
  fmwk = LINK_FESTIVAL_GetMissionWork( fest_sys );

  switch( fmwk->arc.m_type ){
  case FMISSION_TYPE_ITEM:
    FES_GIMMICK_ITEM_ZoneChangeSetup( (FES_GIMMICK_ITEM_WORK*)gmk_wk, gsys, fest_sys, fmwk, zone_id );
    break;
  case FMISSION_TYPE_NPC:
    FES_GIMMICK_NPC_ZoneChangeSetup( (FES_GIMMICK_NPC_WORK*)gmk_wk, gsys, fest_sys, fmwk, zone_id );
    break;
  }
  IWASAWA_Printf("FesGimmick %d\n",OS_GetTick()-s_tick);
}

//----------------------------------------------------------------------
/*
 *  @brief  乱数衝突検査
 */
//----------------------------------------------------------------------
static BOOL rnd_CheckConflict( u32 value, u32* p_tbl, int tbl_max )
{
  int i;

  for(i = 0;i < tbl_max;i++){
    if(p_tbl[i] == value){
      return TRUE;
    }
  }
  return FALSE;
}

