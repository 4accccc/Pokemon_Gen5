//============================================================================================
/**
 * @file    event_gpower.c
 * @brief	  イベント：Gパワー関連イベント
 * @author  iwasawa	
 * @date	  2009.02.17
 */
//============================================================================================
#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/g_power.h"
#include "poke_tool/poke_tool.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"
#include "field/script.h"
#include "field/script_local.h"
#include "./event_fieldmap_control.h"
#include "./event_gpower.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "system/main.h"      //GFL_HEAPID_APP参照

#include "../../../resource/fldmapdata/script/gpower_scr_def.h" //SCRID_～

#define SE_POWER_USE  (SEQ_SE_W295_01)

typedef struct POWER_FLD_EFF{
  u8  ct;
  GAMESYS_WORK * gsys;
  GAMEDATA * gdata;
}POWER_FLD_EFF;

static GMEVENT_RESULT event_GPowerUseEffectMain(GMEVENT * event, int *  seq, void * work);
static void sub_InstantPowerUse( GAMESYS_WORK* gsys, WORDSET* wordset, GPOWER_TYPE type );

/*
 *  @brief  Gパワー発動イベント
 *
 *  @param  g_power 発動するGパワーID
 *  @param  mine    自分のGパワーかどうか？
 */
GMEVENT* EVENT_GPowerEffectStart(GAMESYS_WORK * gsys, GPOWER_ID g_power, BOOL mine )
{
  GMEVENT* event;
  SCRIPT_WORK* sc;

  event = SCRIPT_SetEventScript( gsys, SCRID_GPOWER_EFFECT_START, NULL, HEAPID_FIELDMAP );
  sc = SCRIPT_GetScriptWorkFromEvent( event );
  {
    POWER_CONV_DATA * p_data = GPOWER_PowerData_LoadAlloc( GFL_HEAP_LOWID(HEAPID_FIELDMAP) );
    GPOWER_TYPE type = GPOWER_ID_to_Type( p_data, g_power );
    GPOWER_Set_OccurID( g_power, p_data, mine );

    SCRIPT_SetScriptWorkParam( sc, g_power, type, mine, 0 );
    sub_InstantPowerUse( gsys, SCRIPT_GetWordSet( sc ), type );
 
    GPOWER_PowerData_Unload( p_data );
  }
  if( mine ){ //自分のを使った時だけビーコン送信
    GAMEBEACON_Set_GPower( g_power );
  }
  return event;
}

/*
 *  @brief  Gパワー効果終了イベント
 */
GMEVENT* EVENT_GPowerEffectEnd( GAMESYS_WORK * gsys )
{
  GMEVENT* event;

  event = SCRIPT_SetEventScript( gsys, SCRID_GPOWER_EFFECT_END, NULL, HEAPID_FIELDMAP );
  return event;
}

/*
 *  @brief  Gパワー効果発動エフェクト
 */
GMEVENT* EVENT_GPowerUseEffect( GAMESYS_WORK * gsys )
{
  GMEVENT* event;
  POWER_FLD_EFF* ewk;

  event = GMEVENT_Create(gsys, NULL, event_GPowerUseEffectMain, sizeof(POWER_FLD_EFF));
  
  ewk = GMEVENT_GetEventWork(event);
  ewk->gdata = GAMESYSTEM_GetGameData(gsys);
  ewk->gsys = gsys;

  return event;
}

//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT event_GPowerUseEffectMain(GMEVENT * event, int *  seq, void * work)
{
  POWER_FLD_EFF* ewk = work;

  switch (*seq) {
  case 0:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, 1);
    PMSND_PlaySE( SE_POWER_USE );
    (*seq)++;
    break;
  case 1:
		if( GFL_FADE_CheckFade() ){
      break;
    }
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, 15);
    (*seq)++;
  case 2:
		if( GFL_FADE_CheckFade() ){
      break;
    }
  default:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

/*
 *  @brief  イベント発動時に効果が現れるパワーの実行処理
 */
static void sub_InstantPowerUse( GAMESYS_WORK* gsys, WORDSET* wordset, GPOWER_TYPE type )
{
  u8 pos;
  POKEPARTY* party;
  POKEMON_PARAM* pp;

  party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData( gsys ) );
  pos = PokeParty_GetMemberTopIdxBattleEnable( party );
  pp = PokeParty_GetMemberPointer(party, pos);
  
  WORDSET_RegisterPokeNickName( wordset, 1, pp );

  switch(type){
  case GPOWER_TYPE_HP_RESTORE:
    GPOWER_Calc_HPRestore(party);
    break;
  case GPOWER_TYPE_PP_RESTORE:
    GPOWER_Calc_PPRestore(party);
    break;
  }
}

