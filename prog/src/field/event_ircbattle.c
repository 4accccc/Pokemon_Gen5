//============================================================================================
/**
 * @file  event_ircbattle.c
 * @brief イベント：赤外線バトル
 * @author  k.ohno
 * @date  2009.03.24
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/btl_setup.h"


#include "field/fieldmap.h"
#include "field/field_sound.h"

#include "./event_fieldmap_control.h"
#include "./event_ircbattle.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "net_app/irc_match/ircbattlematch.h"
#include "net_app/irc_match/ircbattlemenu.h"
#include "net_app/friend_trade/ircbattlefriend.h"
#include "net_app/pokemontrade.h"
#include "net_app/irc_compatible.h"
#include "sound/pm_sndsys.h"
#include "battle/battle.h"
#include "poke_tool/monsno_def.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"
#include "savedata/battle_box_save.h"
//============================================================================================
//============================================================================================

//----------------------------------------------------------------
// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];
//----------------------------------------------------------------


FS_EXTERN_OVERLAY(irc_compatible);

#define HEAPID_CORE GFL_HEAPID_APP

FS_EXTERN_OVERLAY(battle);
FS_EXTERN_OVERLAY(fieldmap);
FS_EXTERN_OVERLAY(ircbattlematch);
FS_EXTERN_OVERLAY(pokemon_trade);

#define _LOCALMATCHNO (100)

enum _EVENT_IRCBATTLE {
  _IRCBATTLE_START,
  _IRCBATTLE_START_FIELD_CLOSE,
  _CALL_IRCBATTLE_MENU,
  _WAIT_IRCBATTLE_MENU,
  _FIELD_FADEOUT,
  _CALL_IRCBATTLE_MATCH,
  _WAIT_IRCBATTLE_MATCH,
  _BATTLE_MATCH_START,
  _TIMING_SYNC_CALL_BATTLE,
  _CALL_BATTLE,
  _WAIT_BATTLE,
  _CALL_IRCBATTLE_FRIEND,
  _WAIT_IRCBATTLE_FRIEND,
  _CALL_TRADE,
  _WAIT_TRADE,
  _CALL_NET_END,
  _WAIT_NET_END,
  _FIELD_OPEN,
  _FIELD_FADEIN,
  _FIELD_END,

  _FIELD_FADEOUT_IRCBATTLE,
  _FIELD_END_IRCBATTLE,
  _CALL_IRCCOMMPATIBLE,
  _WAIT_IRCCOMMPATIBLE,
};

struct _EVENT_IRCBATTLE_WORK{
  GAMESYS_WORK * gsys;
  GAMEDATA* gamedata;
  POKEMONTRADE_PARAM aPokeTr;
  POKEPARTY* pParty;
  SAVE_CONTROL_WORK *ctrl;
  int selectType;
  IRC_COMPATIBLE_PARAM  compatible_param; //赤外線メニューに渡す情報
  BOOL push;
#if PM_DEBUG
  int debugseq;
#endif
  BATTLE_SETUP_PARAM* para;
};

static void _battleParaFree(EVENT_IRCBATTLE_WORK *dbw);

//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_IrcBattleMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_IRCBATTLE_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;
  FIELDMAP_WORK * pFieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

#if PM_DEBUG
  if(dbw->debugseq != *seq){
    OS_TPrintf("e irc %d\n");
    dbw->debugseq = *seq;
  }
#endif

  switch (*seq) {
  case _IRCBATTLE_START:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, pFieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case _IRCBATTLE_START_FIELD_CLOSE:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, pFieldmap));
    //
    (*seq) = _CALL_IRCBATTLE_MENU;
    break;
  case _CALL_IRCBATTLE_MENU:
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(fieldmap), &IrcBattleMenuProcData, dbw);
    (*seq)++;
    break;
  case _WAIT_IRCBATTLE_MENU:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    if(dbw->selectType == EVENTIRCBTL_ENTRYMODE_COMPATIBLE )
    {
      *seq = _FIELD_FADEOUT_IRCBATTLE;
    }
    else if(dbw->selectType == EVENTIRCBTL_ENTRYMODE_EXIT){
      *seq = _FIELD_OPEN;
    }
    else{
      (*seq) ++;
    }
    break;
  case _FIELD_FADEOUT:
    (*seq)++;
    break;
  case _CALL_IRCBATTLE_MATCH:
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(ircbattlematch), &IrcBattleMatchProcData, dbw);
    (*seq)++;
    break;
  case _WAIT_IRCBATTLE_MATCH:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      // マッチング内容によりゲーム分岐
      switch(dbw->selectType){
      case EVENTIRCBTL_ENTRYMODE_EXIT:
        *seq = _WAIT_NET_END;
        break;
      case EVENTIRCBTL_ENTRYMODE_RETRY:
        *seq = _CALL_IRCBATTLE_MENU;
        break;
      case EVENTIRCBTL_ENTRYMODE_FRIEND:
        *seq = _CALL_IRCBATTLE_FRIEND;
        break;
      case EVENTIRCBTL_ENTRYMODE_TRADE:
        *seq = _CALL_TRADE;
        break;
      default:
        *seq = _BATTLE_MATCH_START;
        break;
      }
    }
    break;
  case _BATTLE_MATCH_START:
    {
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );
      GFL_NET_AddCommandTable(GFL_NET_CMD_BATTLE, BtlRecvFuncTable, BTL_NETFUNCTBL_ELEMS, NULL);
      GFL_NET_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_LOCALMATCHNO);
      (*seq) ++;
    }
    break;
  case _TIMING_SYNC_CALL_BATTLE:
    if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_LOCALMATCHNO)){
      (*seq) ++;
    }
    break;
  case _CALL_BATTLE:
    switch(dbw->selectType){
    case EVENTIRCBTL_ENTRYMODE_SINGLE:
      BTL_SETUP_Single_Comm( dbw->para , dbw->gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
      break;
    case EVENTIRCBTL_ENTRYMODE_DOUBLE:
      BTL_SETUP_Double_Comm( dbw->para , dbw->gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
      break;
    case EVENTIRCBTL_ENTRYMODE_TRI:
      BTL_SETUP_Triple_Comm( dbw->para , dbw->gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
      break;
    case EVENTIRCBTL_ENTRYMODE_ROTATE:
      BTL_SETUP_Rotation_Comm( dbw->para , dbw->gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
      break;
    case EVENTIRCBTL_ENTRYMODE_MULTH:
      BTL_SETUP_Multi_Comm( dbw->para , dbw->gamedata , GFL_NET_HANDLE_GetCurrentHandle(),
                            BTL_COMM_DS, GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() ), HEAPID_PROC );
      dbw->para->multiMode = 1;
      dbw->para->rule = BTL_RULE_DOUBLE;
      NET_PRINT("multiMode\n");
      break;
    default:
      GF_ASSERT(0);
      break;
    }
    BATTLE_PARAM_SetPokeParty( dbw->para, dbw->pParty, BTL_CLIENT_PLAYER );

    {
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PushPlayEventBGM( fsnd, dbw->para->musicDefault );
      dbw->push=TRUE;
    }

    GAMESYSTEM_CallProc(gsys, NO_OVERLAY_ID, &BtlProcData, dbw->para);
//    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 1);
    (*seq)++;
    break;
  case _WAIT_BATTLE:
    if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL){
      break;
    }
    NET_PRINT("バトル完了 event_ircbattle\n");
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
    (*seq) = _CALL_NET_END;
    break;
  case _CALL_IRCBATTLE_FRIEND:  //  ともだちコード交換
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(ircbattlematch), &IrcBattleFriendProcData, dbw);
    (*seq)++;
    break;
  case _WAIT_IRCBATTLE_FRIEND:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      NET_PRINT("ともだちコード交換おわり\n");
      (*seq) = _CALL_NET_END;
    }
    break;

  case _CALL_TRADE:  //  ポケモン交換
    dbw->aPokeTr.gsys=dbw->gsys;
    dbw->aPokeTr.type=POKEMONTRADE_IRC;
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(pokemon_trade), &PokemonTradeIrcProcData, &dbw->aPokeTr);
    (*seq)++;
    break;
  case _WAIT_TRADE:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      NET_PRINT("ポケモン交換おわり\n");
      (*seq) = _CALL_NET_END;
    }
    break;


  case _CALL_NET_END:
    if(GFL_NET_IsParentMachine()){
      GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),GFL_NET_CMD_EXIT_REQ,0,NULL);
    }
    (*seq) ++;
    break;
  case _WAIT_NET_END:
    if(GFL_NET_IsExit()){
      (*seq) ++;
    }
    break;
  case _FIELD_OPEN:
    _battleParaFree(dbw);
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case _FIELD_FADEIN:
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, pFieldmap, FIELD_FADE_WAIT);
        GMEVENT_CallEvent(event, fade_event);
    }
    (*seq) ++;
    break;
  case _FIELD_END:
    if(dbw->push){
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      FIELD_SOUND_PopBGM( fsnd );
      dbw->push=FALSE;
    }
    PMSND_FadeInBGM(60);
    return GMEVENT_RES_FINISH;

  //相性チェックはプロセス移動
  case _FIELD_FADEOUT_IRCBATTLE:
/*    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, pFieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }*/
    (*seq)++;
    break;
  case _FIELD_END_IRCBATTLE:
    (*seq)++;
    break;
  case _CALL_IRCCOMMPATIBLE:  //相性チェック画面へ
    dbw->compatible_param.p_gamesys   = dbw->gsys;
#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
    dbw->compatible_param.is_only_play  = FALSE;
#endif //DEBUG_IRC_COMPATIBLE_ONLYPLAY
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(irc_compatible), &IrcCompatible_ProcData, &dbw->compatible_param );
    (*seq)++;
    break;
  case _WAIT_IRCCOMMPATIBLE:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL)
    {
      GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
      NET_PRINT("相性チェック画面おわり\n");
      (*seq)=_CALL_IRCBATTLE_MENU;
    }
    break;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT* EVENT_IrcBattle(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,GMEVENT * prevevent,BOOL bCreate)
{
  GMEVENT * event = prevevent;
  EVENT_IRCBATTLE_WORK * dbw;
  BATTLE_BOX_SAVE * bxsv;

  if(bCreate){
    event = GMEVENT_Create(gsys, NULL, EVENT_IrcBattleMain, sizeof(EVENT_IRCBATTLE_WORK));
  }
  else{
    GMEVENT_Change( event,EVENT_IrcBattleMain, sizeof(EVENT_IRCBATTLE_WORK) );
  }
  dbw = GMEVENT_GetEventWork(event);
  dbw->ctrl =  GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(gsys) );  //SaveControl_GetPointer();
  dbw->gamedata = GAMESYSTEM_GetGameData(gsys);
  dbw->gsys = gsys;

  dbw->para =BATTLE_PARAM_Create(HEAPID_PROC);

  bxsv = BATTLE_BOX_SAVE_GetBattleBoxSave(dbw->ctrl);
  if(!BATTLE_BOX_SAVE_IsIn(bxsv)){
    dbw->pParty = PokeParty_AllocPartyWork( HEAPID_PROC );  ///< プレイヤーのパーティ
    PokeParty_Copy(GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(gsys)), dbw->pParty);
  }
  else{
    dbw->pParty  = BATTLE_BOX_SAVE_MakePokeParty( bxsv, HEAPID_PROC );
  }
  return event;
}


static void _battleParaFree(EVENT_IRCBATTLE_WORK *dbw)
{
  GFL_HEAP_FreeMemory(dbw->pParty);
  BATTLE_PARAM_Delete(dbw->para);
}



typedef struct{
  int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { WB_NET_COMPATI_CHECK };


//--------------------------------------------------------------
/**
 * @brief   ビーコンデータ取得関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------

void* IrcBattleBeaconGetFunc(void* pWork)
{
  return &_testBeacon;
}

///< ビーコンデータサイズ取得関数
int IrcBattleBeaconGetSizeFunc(void* pWork)
{
  return sizeof(_testBeacon);
}

///< ビーコンデータ取得関数
BOOL IrcBattleBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)
{
  if(myNo != beaconNo){
    return FALSE;
  }
  return TRUE;
}


//--------------------------------------------------------------
/**
 * @brief   赤外線モードの設定
 * @param   pWork      ワークエリア
 * @param   type       タイプ
 * @retval  none
 */
//--------------------------------------------------------------
void EVENT_IrcBattleSetType(EVENT_IRCBATTLE_WORK* pWork, int type)
{
  pWork->selectType = type;
}

int EVENT_IrcBattleGetType(EVENT_IRCBATTLE_WORK* pWork)
{
  return pWork->selectType;
}


//--------------------------------------------------------------
/**
 * @brief   EVENT_IRCBATTLE_WORKからGAMESYS_WORKを得る
 * @param   pWork      EVENT_IRCBATTLE_WORK
 * @retval  GAMESYS_WORK
 */
//--------------------------------------------------------------
GAMESYS_WORK* IrcBattle_GetGAMESYS_WORK(EVENT_IRCBATTLE_WORK* pWork)
{
  return pWork->gsys;
}

//--------------------------------------------------------------
/**
 * @brief   EVENT_IRCBATTLE_WORKからSAVE_CONTROL_WORKをえる
 * @param   pWork      EVENT_IRCBATTLE_WORK
 * @retval  SAVE_CONTROL_WORK
 */
//--------------------------------------------------------------
SAVE_CONTROL_WORK* IrcBattle_GetSAVE_CONTROL_WORK(EVENT_IRCBATTLE_WORK* pWork)
{
  return pWork->ctrl;
}



