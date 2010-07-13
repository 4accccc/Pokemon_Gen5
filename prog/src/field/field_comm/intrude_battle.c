//==============================================================================
/**
 * @file    intrude_battle.c
 * @brief   侵入：通信対戦PROC
 * @author  matsuda
 * @date    2009.11.16(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "battle/battle.h"
#include "gamesystem/gamesystem.h"
#include "intrude_battle.h"
#include "gamesystem/btl_setup.h"
#include "intrude_types.h"
#include "intrude_main.h"
#include "intrude_comm_command.h"
#include "intrude_work.h"
#include "sound/pm_sndsys.h"
#include "field/intrude_snd_def.h"
#include "poke_tool/pokeparty.h"

#include "gamesystem/game_event.h"
#include "field/field_sound.h"
#include "field/event_fieldmap_control.h"
#include "field/fieldmap.h"
#include "field/enceff.h"
#include "poke_tool/status_rcv.h"


//==============================================================================
//  定数定義
//==============================================================================
///侵入の対戦でのコマンド選択制限時間(秒)
#define INTRUDE_BATTLE_LIMIT_COMMAND_TIME   (30)


//==============================================================================
//  構造体定義
//==============================================================================
///侵入通信対戦制御構造体
typedef struct{
  BATTLE_SETUP_PARAM para;
  u32 work;
  GFL_PROCSYS *procsys;
  POKEPARTY *pokeparty;
  BOOL overlay_load_battle;
}INTRUDE_BATTLE_SYS;

typedef struct{
  GAMESYS_WORK *gsys;
  INTRUDE_BATTLE_PARENT *p_ibp;
}EVENT_INTRUDEBATTLE_CALL;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT IntrudeBattleProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT IntrudeBattleProc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT IntrudeBattleProc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk);

static GMEVENT_RESULT EventIntrudeBattle_CallBattle( GMEVENT* event, int* seq, void* wk );


//==============================================================================
//  データ
//==============================================================================
static const GFL_PROC_DATA IntrudeBattleProcData = {
  IntrudeBattleProc_Init,
  IntrudeBattleProc_Main,
  IntrudeBattleProc_End,
};


//--------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT IntrudeBattleProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  INTRUDE_BATTLE_SYS *ibs;
  INTRUDE_BATTLE_PARENT *ibp = pwk;
  int i, party_max, use_count = 0;
  POKEMON_PARAM *pp;
  POKEPARTY *temoti;
  
  ibs = GFL_PROC_AllocWork(proc, sizeof(INTRUDE_BATTLE_SYS), HEAPID_PROC);
  GFL_STD_MemClear(ibs, sizeof(INTRUDE_BATTLE_SYS));

  ibs->procsys = GFL_PROC_LOCAL_boot( HEAPID_PROC );
  
  temoti = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(ibp->gsys));
  STATUS_RCV_PokeParty_RecoverAll(temoti);
  
  //手持ちからフラットレベルのPOKEPARTYを作成
  party_max = PokeParty_GetPokeCount(temoti);
  ibs->pokeparty = PokeParty_AllocPartyWork( HEAPID_PROC );
  for(i = 0; i < party_max; i++){
    if(use_count < ibp->max_poke_num){
      if(PokeParty_CheckPokeIdxBattleEnable(temoti, i) == TRUE){
        PokeParty_Add( ibs->pokeparty, PokeParty_GetMemberPointer( temoti, i ) );
        pp = PokeParty_GetMemberPointer( ibs->pokeparty, use_count );
        POKETOOL_MakeLevelRevise(pp, ibp->flat_level);
        use_count++;
      }
    }
    else{
      break;
    }
  }
  GF_ASSERT_HEAVY(use_count > 0);
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT IntrudeBattleProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  INTRUDE_BATTLE_SYS *ibs = mywk;
  INTRUDE_BATTLE_PARENT *ibp = pwk;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(ibp->gsys);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(ibp->gsys);
  GFL_PROC_MAIN_STATUS proc_status;
  enum{
    SEQ_INIT,
    SEQ_COMMAND_ADD_BEFORE_TIMING,
    SEQ_COMMAND_ADD_BEFORE_TIMING_WAIT,
    SEQ_COMMAND_ADD_AFTER_TIMING,
    SEQ_COMMAND_ADD_AFTER_TIMING_WAIT,
    SEQ_BATTLE,
    SEQ_BATTLE_WAIT,
    SEQ_FINISH,
  };

  proc_status = GFL_PROC_LOCAL_Main( ibs->procsys );
  
  if(NetErr_App_CheckError() || GFL_NET_IsExit() == TRUE){
    if((*seq) > SEQ_INIT && (*seq) < SEQ_BATTLE_WAIT){
      *seq = SEQ_FINISH;
    }
  }
  
  switch(*seq){
  case SEQ_INIT:
    BTL_SETUP_Single_Comm(&ibs->para, gamedata, GFL_NET_HANDLE_GetCurrentHandle(), BTL_COMM_DS, HEAPID_PROC );
    BATTLE_PARAM_SetPokeParty( &ibs->para, ibs->pokeparty, BTL_CLIENT_PLAYER );
    if(GFL_NET_SystemGetCurrentID() < ibp->target_netid){
      ibs->para.commPos = 0;
    }
    else{
      ibs->para.commPos = 1;
    }
    ibs->para.fieldSituation.bgAttr = BATTLE_BG_ATTR_PALACE;
    ibs->para.musicDefault = SND_INTRUDE_BATTLE_BGM;
    ibs->para.commNetIDBit = (1 << GFL_NET_SystemGetCurrentID()) | (1 << ibp->target_netid);
    ibs->para.LimitTimeCommand = INTRUDE_BATTLE_LIMIT_COMMAND_TIME;
    (*seq)++;
    break;
  case SEQ_COMMAND_ADD_BEFORE_TIMING:
    GFL_NET_HANDLE_TimeSyncBitStart(GFL_NET_HANDLE_GetCurrentHandle(), 
      INTRUDE_TIMING_BATTLE_COMMAND_ADD_BEFORE, WB_NET_PALACE_SERVICEID, 
      (1<<ibp->target_netid) | (1<<GFL_NET_SystemGetCurrentID()));
    OS_TPrintf("戦闘用通信コマンドAdd前の同期取り\n");
    (*seq)++;
    break;
  case SEQ_COMMAND_ADD_BEFORE_TIMING_WAIT:
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(), 
        INTRUDE_TIMING_BATTLE_COMMAND_ADD_BEFORE, WB_NET_PALACE_SERVICEID) == TRUE){
      OS_TPrintf("戦闘用通信コマンドAdd前の同期取り完了\n");
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );
      ibs->overlay_load_battle = TRUE;
      GFL_NET_AddCommandTable(GFL_NET_CMD_BATTLE, BtlRecvFuncTable, BTL_NETFUNCTBL_ELEMS, NULL);
      (*seq)++;
    }
    break;
  case SEQ_COMMAND_ADD_AFTER_TIMING:
    GFL_NET_HANDLE_TimeSyncBitStart(GFL_NET_HANDLE_GetCurrentHandle(), 
      INTRUDE_TIMING_BATTLE_COMMAND_ADD_AFTER, WB_NET_PALACE_SERVICEID, 
      (1<<ibp->target_netid) | (1<<GFL_NET_SystemGetCurrentID()));
    OS_TPrintf("戦闘用通信コマンドテーブルをAddしたので同期取り\n");
    (*seq) ++;
    break;
  case SEQ_COMMAND_ADD_AFTER_TIMING_WAIT:
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(), 
        INTRUDE_TIMING_BATTLE_COMMAND_ADD_AFTER, WB_NET_PALACE_SERVICEID) == TRUE){
      OS_TPrintf("戦闘用通信コマンドテーブルをAdd後の同期取り完了\n");
      (*seq) ++;
    }
    break;
  case SEQ_BATTLE:
    OS_TPrintf("バトルPROC呼び出し\n");
    GFL_PROC_LOCAL_CallProc( ibs->procsys, NO_OVERLAY_ID, &BtlProcData, &ibs->para );
    (*seq)++;
    break;
  case SEQ_BATTLE_WAIT:
    if(proc_status != GFL_PROC_MAIN_NULL){
      break;
    }
    OS_TPrintf("バトル完了\n");
    (*seq)++;
    break;
  case SEQ_FINISH:
    if(ibs->overlay_load_battle == TRUE){
      if(GFL_NET_IsInit() == TRUE){
        GFL_NET_DelCommandTable(GFL_NET_CMD_BATTLE);
      }
      GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
    }
    
    BATTLE_PARAM_Release( &ibs->para );
    return GFL_PROC_RES_FINISH;
  }
  
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT IntrudeBattleProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  INTRUDE_BATTLE_SYS *ibs = mywk;

  GFL_PROC_LOCAL_Exit( ibs->procsys );
  GFL_HEAP_FreeMemory(ibs->pokeparty);
  
  GFL_PROC_FreeWork(proc);
  return GFL_PROC_RES_FINISH;
}


//==============================================================================
//
//  バトル呼び出しイベント：常駐に配置すべき共通イベント部分
//
//==============================================================================
//---------------------------------------------------------------------------------
/**
 * @brief バトル呼び出しイベント：常駐に配置すべき共通イベント部分
 *
 * @param gameSystem
 * @param soundIdx   再生する戦闘BGM
 * 
 * @return BGM 退避イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_IntrudeBattle_CallBattle( GAMESYS_WORK* gsys, INTRUDE_BATTLE_PARENT *ibp )
{
  GMEVENT* event;
  EVENT_INTRUDEBATTLE_CALL *eic;

  event = GMEVENT_Create( 
    gsys, NULL, EventIntrudeBattle_CallBattle, sizeof(EVENT_INTRUDEBATTLE_CALL) );
  eic = GMEVENT_GetEventWork( event );
  eic->p_ibp = ibp;
  eic->gsys = gsys;
  return event;
} 
//---------------------------------------------------------------------------------
/**
 * @brief バトル呼び出しイベント：常駐に配置すべき共通イベント部分
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT EventIntrudeBattle_CallBattle( GMEVENT* event, int* seq, void* wk )
{
  EVENT_INTRUDEBATTLE_CALL *eic = wk;
	FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(eic->gsys);
  enum{
    SEQ_BATTLE_BGM,
    SEQ_ENCEFF,
    SEQ_FIELD_CLOSE,
    SEQ_BATTLE_PROC,
    SEQ_BATTLE_AFTER,
    SEQ_FIELD_OPEN,
    SEQ_BGM_POP_WAIT,
    SEQ_FADEIN_WAIT,
  };

  switch( *seq ) {
  case SEQ_BATTLE_BGM:
    // 戦闘用ＢＧＭセット
    GMEVENT_CallEvent(event, EVENT_FSND_PushPlayBattleBGM(eic->gsys, SND_INTRUDE_BATTLE_BGM));
    (*seq)++;
    break;
  case SEQ_ENCEFF:
    ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(fieldWork), event, ENCEFFID_TR_NORMAL);
    (*seq)++;
    break;
  case SEQ_FIELD_CLOSE:
    // フィールドマップ終了
    GMEVENT_CallEvent(event, EVENT_FieldClose(eic->gsys, fieldWork));
    (*seq)++;
    break;
  case SEQ_BATTLE_PROC:
    GMEVENT_CallProc(event, NO_OVERLAY_ID, &IntrudeBattleProcData, eic->p_ibp);
	  (*seq)++;
  	break;
  case SEQ_BATTLE_AFTER:
    GMEVENT_CallEvent(event, EVENT_FSND_PopPlayBGM_fromBattle(eic->gsys));
    (*seq)++;
    break;
  case SEQ_FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(eic->gsys));
    (*seq)++;
    break;
  case SEQ_BGM_POP_WAIT:
    // BGMの復帰待ち
    GMEVENT_CallEvent( event, EVENT_FSND_WaitBGMPop( eic->gsys ) );
    (*seq)++;
    break;
  case SEQ_FADEIN_WAIT:
    GMEVENT_CallEvent( 
      event, EVENT_FieldFadeIn_Black( eic->gsys, fieldWork, FIELD_FADE_WAIT ) );
    (*seq)++;
    break;

  default:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
