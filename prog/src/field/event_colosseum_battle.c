//==============================================================================
/**
 * @file    event_colosseum_battle.c
 * @brief   イベント：コロシアムバトル
 * @author  matsuda
 * @date    2009.07.22(水)
 */
//==============================================================================
#include <gflib.h>
#include "net/network_define.h"

#include "system\main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "field/field_sound.h"

#include "./event_fieldmap_control.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "sound/pm_sndsys.h"
#include "battle/battle.h"
#include "poke_tool/monsno_def.h"

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"

#include "net_app/union/union_types.h"
#include "net_app/union/colosseum_types.h"
#include "field/event_fldmmdl_control.h"
#include "field/event_colosseum_battle.h"
#include "gamesystem\btl_setup.h"
#include "field/event_battle_call.h"


//============================================================================================
//============================================================================================

//----------------------------------------------------------------
// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];
//----------------------------------------------------------------

FS_EXTERN_OVERLAY(battle);

//==============================================================================
//  構造体定義
//==============================================================================
typedef struct{
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  BATTLE_SETUP_PARAM para;
  COMM_BTL_DEMO_PARAM *demo_prm;
}EVENT_COLOSSEUM_BATTLE_WORK;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _SetBattleMode(int *dest_work, BATTLE_MODE battle_mode);


//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_ColosseumBattleMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_COLOSSEUM_BATTLE_WORK * cbw = work;
  GAMESYS_WORK * gsys = cbw->gsys;

  switch (*seq) {
  case 0:
#if 0 //ポケモンリスト後、直接来るようになったので削除
    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gsys, cbw->fieldmap) );
    (*seq)++;
    break;
  case 1:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, cbw->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq)++;
    break;
  case 2:
    GMEVENT_CallEvent( event, EVENT_FieldClose(gsys, cbw->fieldmap));
    (*seq)++;
    break;
#else
    (*seq) = 3;
    break;
#endif
  case 3:
    GMEVENT_CallEvent(event, 
        EVENT_FSND_PushPlayBattleBGM(gsys, cbw->para.musicDefault)); 
    (*seq)++;
    break;
  case 4:
    {
      GMEVENT* next_event;
      EV_BATTLE_CALL_PARAM battlecall_param;

      battlecall_param.btl_setup_prm = &cbw->para;
      battlecall_param.demo_prm = cbw->demo_prm;
      battlecall_param.error_auto_disp = FALSE; //エラー画面の表示はユニオンで制御

      next_event = GMEVENT_CreateOverlayEventCall( gsys, 
                    FS_OVERLAY_ID(event_battlecall), 
                    EVENT_CallCommBattle, &battlecall_param );

      GMEVENT_CallEvent( event, next_event );
    }
    (*seq)++;
    break;
  case 5:
    BATTLE_PARAM_Release( &cbw->para ); //バトルSetupParam解放
    GMEVENT_CallEvent(event, EVENT_FSND_PopPlayBGM_fromBattle(gsys));
    (*seq) ++;
    break;
    
  case 6:
    OS_TPrintf("_FIELD_OPEN\n");
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case 7:
    OS_TPrintf("_FIELD_FADEIN\n");
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, cbw->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case 8:
    GMEVENT_CallEvent(event, EVENT_FSND_WaitBGMPop(gsys));
    (*seq) ++;
    break;
  case 9:
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_ColosseumBattle(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, UNION_PLAY_CATEGORY play_category, COLOSSEUM_BATTLE_SETUP *setup)
{
  BATTLE_SETUP_PARAM * para;
  EVENT_COLOSSEUM_BATTLE_WORK * cbw;
  BtlRule rule;
  int multi;
  int recmode = BATTLE_MODE_MAX;
  GMEVENT *event;

  event = GMEVENT_Create(
    gsys, NULL, EVENT_ColosseumBattleMain, sizeof(EVENT_COLOSSEUM_BATTLE_WORK));
  cbw = GMEVENT_GetEventWork(event);
  cbw->gsys = gsys;
  cbw->fieldmap = fieldmap;
  cbw->demo_prm = &setup->demo;
  para = &cbw->para;

  switch(play_category){
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE:       //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_SINGLE_FREE);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER:       //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_SINGLE_FREE_SHOOTER);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT:   //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_SINGLE_50);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT_SHOOTER:   //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_SINGLE_50_SHOOTER);
    BTL_SETUP_Single_Comm(para, GAMESYSTEM_GetGameData(gsys), 
      GFL_NET_HANDLE_GetCurrentHandle(), BTL_COMM_DS, HEAPID_PROC );
    break;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE:       //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_DOUBLE_FREE);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER:       //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_DOUBLE_FREE_SHOOTER);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT:   //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_DOUBLE_50);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT_SHOOTER:   //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_DOUBLE_50_SHOOTER);
    BTL_SETUP_Double_Comm(para, GAMESYSTEM_GetGameData(gsys), 
      GFL_NET_HANDLE_GetCurrentHandle(), BTL_COMM_DS, HEAPID_PROC );
    break;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE:       //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_TRIPLE_FREE);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER:       //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_TRIPLE_FREE_SHOOTER);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT:   //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_TRIPLE_50);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT_SHOOTER:   //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_TRIPLE_50_SHOOTER);
    BTL_SETUP_Triple_Comm(para, GAMESYSTEM_GetGameData(gsys), 
      GFL_NET_HANDLE_GetCurrentHandle(), BTL_COMM_DS, HEAPID_PROC );
    break;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE:       //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_ROTATION_FREE);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER:       //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_ROTATION_FREE_SHOOTER);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT:   //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_ROTATION_50);
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT_SHOOTER:   //コロシアム
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_ROTATION_50_SHOOTER);
    BTL_SETUP_Rotation_Comm(para, GAMESYSTEM_GetGameData(gsys), 
      GFL_NET_HANDLE_GetCurrentHandle(), BTL_COMM_DS, HEAPID_PROC );
    break;
  case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE:
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_MULTI_FREE);
  case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE_SHOOTER:
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_MULTI_FREE_SHOOTER);
  case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT:
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_MULTI_50);
  case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT_SHOOTER:
    _SetBattleMode(&recmode, BATTLE_MODE_COLOSSEUM_MULTI_50_SHOOTER);
    BTL_SETUP_Multi_Comm(para, GAMESYSTEM_GetGameData(gsys), 
      GFL_NET_HANDLE_GetCurrentHandle(), BTL_COMM_DS, setup->standing_pos, HEAPID_PROC );
    break;
  default:
    GF_ASSERT_MSG_HEAVY(0, "play_category = %d\n", play_category);
    return NULL;
  }
  BATTLE_PARAM_SetPokeParty( para, setup->partyPlayer, BTL_CLIENT_PLAYER );
  BATTLE_PARAM_SetRegulation( para, setup->regulation, HEAPID_PROC );
  para->musicDefault = setup->music_no;
  cbw->demo_prm->battle_mode = recmode;
  cbw->demo_prm->fight_count = 0;
#if 0 //戦闘内部でシェアしあうので、ここで指定はいらない
  para->partyEnemy1 = setup->partyEnemy1;   ///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
  para->partyPartner = setup->partyPartner; ///< 2vs2時の味方AI（不要ならnull）
  para->partyEnemy2 = setup->partyEnemy2;    ///< 2vs2時の２番目敵AI用（不要ならnull）
#endif
  BTL_SETUP_AllocRecBuffer(para, GFL_HEAPID_APP); //録画バッファ確保
  return event;
}

//--------------------------------------------------------------
/**
 * 録画用のバトルモードを設定
 *
 * @param   dest_work		
 * @param   battle_mode		
 */
//--------------------------------------------------------------
static void _SetBattleMode(int *dest_work, BATTLE_MODE battle_mode)
{
  if(*dest_work == BATTLE_MODE_MAX){
    *dest_work = battle_mode;
  }
}


