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

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"

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
  FIELD_MAIN_WORK * fieldmap;
  BATTLE_SETUP_PARAM para;
}EVENT_COLOSSEUM_BATTLE_WORK;



//============================================================================================
//
//		サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_ColosseumBattleMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_COLOSSEUM_BATTLE_WORK * cbw = work;
  GAMESYS_WORK * gsys = cbw->gsys;
  
  switch (*seq) {
	case 0:
		// サウンドテスト
		// ＢＧＭ一時停止→退避
		PMSND_PauseBGM(TRUE);
		PMSND_PushBGM();

    GMEVENT_CallEvent( event, EVENT_ObjPauseAll(gsys, cbw->fieldmap) );
    (*seq)++;
    break;
  case 1:
		GMEVENT_CallEvent(event, EVENT_FieldFadeOut(gsys, cbw->fieldmap, FIELD_FADE_BLACK));
		(*seq)++;
		break;
	case 2:
		GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, cbw->fieldmap));
		//配置していた動作モデルを削除
		MMDLSYS_DeleteMMdl( GAMEDATA_GetMMdlSys(GAMESYSTEM_GetGameData(gsys)) );
		(*seq)++;
		break;
	case 3:
		{
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );
      GFL_NET_AddCommandTable(GFL_NET_CMD_BATTLE, BtlRecvFuncTable, 5, NULL);
      GFL_NET_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_BATTLE_ADD_CMD_TBL_AFTER);
      (*seq) ++;
    }
    break;
  case 4:
    if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_BATTLE_ADD_CMD_TBL_AFTER)){
      (*seq) ++;
    }
    break;
  case 5:
    PMSND_PlayBGM(cbw->para.musicDefault);

    GAMESYSTEM_CallProc(gsys, NO_OVERLAY_ID, &BtlProcData, &cbw->para);
    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 1);
    (*seq)++;
    break;
  case 6:
    if (GAMESYSTEM_IsProcExists(gsys)){
      break;
    }
    OS_TPrintf("バトル完了\n");
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
		(*seq)++;
    break;

  case 7:
    OS_TPrintf("_FIELD_OPEN\n");
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case 8:
		OS_TPrintf("_FIELD_FADEIN\n");
    GMEVENT_CallEvent(event, EVENT_FieldFadeIn(gsys, cbw->fieldmap, 0));
    (*seq) ++;
    break;
  case 9:
		// サウンドテスト
		// ＢＧＭ取り出し→再開
		PMSND_PopBGM();
		PMSND_PauseBGM(FALSE);
		PMSND_FadeInBGM(60);

    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_ColosseumBattle(GAMESYS_WORK * gsys, FIELD_MAIN_WORK * fieldmap, UNION_PLAY_CATEGORY play_category, const COLOSSEUM_BATTLE_SETUP *setup)
{
  BATTLE_SETUP_PARAM * para;
  EVENT_COLOSSEUM_BATTLE_WORK * cbw;
  BtlRule rule;
  int multi;
  GMEVENT *event;
  
  switch(play_category){
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50:         //コロシアム
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE:       //コロシアム
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD:   //コロシアム
    rule = BTL_RULE_SINGLE;
    multi = 0;
    break;
  case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI:      //コロシアム
    rule = BTL_RULE_SINGLE;
    multi = 1;
    break;
  default:
    GF_ASSERT(0);
    return NULL;
  }
  
  event = GMEVENT_Create(
    gsys, NULL, EVENT_ColosseumBattleMain, sizeof(EVENT_COLOSSEUM_BATTLE_WORK));
  cbw = GMEVENT_GetEventWork(event);
  cbw->gsys = gsys;
  cbw->fieldmap = fieldmap;
  para = &cbw->para;
  {
    para->engine = BTL_ENGINE_ALONE;
    para->rule = rule;
    para->competitor = BTL_COMPETITOR_COMM;

    para->commMode = BTL_COMM_DS;
    para->multiMode = multi;
    
    para->partyPlayer = setup->partyPlayer;
    para->partyEnemy1 = setup->partyEnemy1;   ///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
    para->partyPartner = setup->partyPartner; ///< 2vs2時の味方AI（不要ならnull）
    para->partyEnemy2 = setup->partyEnemy2;    ///< 2vs2時の２番目敵AI用（不要ならnull）
    para->statusPlayer =  GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gsys));

    para->netHandle = GFL_NET_HANDLE_GetCurrentHandle();
    para->netID = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );

    para->musicDefault = SEQ_BA_TEST_250KB;		///< デフォルト時のBGMナンバー
    para->musicPinch = SEQ_BA_PINCH_TEST_150KB;			///< ピンチ時のBGMナンバー
  }
  return event;
}



