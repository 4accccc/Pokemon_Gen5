//============================================================================================
/**
 * @file  event_wificlub.c
 * @brief イベント：WiFiクラブ
 * @author  k.ohno
 * @date  2009.03.31
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "sound/pm_sndsys.h"  //SEQ_BA_TEST_250KB

#include "./event_fieldmap_control.h"
#include "field/event_wificlub.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "battle/battle.h"   // BATTLE_SETUP_PARAM
#include "poke_tool/monsno_def.h"
#include "system/main.h"      //GFL_HEAPID_APP参照

#include "poke_tool/pokeparty.h"
#include "poke_tool/poke_tool.h"

//============================================================================================

#include "pm_define.h"
#include "net/network_define.h"

#include "gamesystem/game_event.h"
#include "gamesystem/btl_setup.h"

#include "net_app/wificlub/wifi_p2pmatch.h"
//#include "net_app/wificlub/wifi_p2pmatchfour.h"
#include "net_app/pokemontrade.h"
//#include "wifi_p2pmatch_local.h"

#include "net/dwc_rap.h"
#include "net/dwc_raputil.h"
#include "net/dwc_rapcommon.h"
#include "savedata/wifilist.h"
#include "system/main.h"
#include "battle/battle.h"  //BTL_RULE_SINGLE
#include "sound/pm_sndsys.h"
//#include "sound/pm_wb_voice.h"
//#include "wifi_p2pmatch_se.h"

#include "field/event_wificlub.h"

#include "net_app/comm_tvt_sys.h"  //TVトランシーバ
#include "net_app/wifi_login.h"

#include "app/pokelist.h"
#include "include/demo/comm_btl_demo.h"
#include "field/event_battle_call.h"



//============================================================================================
FS_EXTERN_OVERLAY(wificlub);
FS_EXTERN_OVERLAY(battle);
FS_EXTERN_OVERLAY(ircbattlematch);

FS_EXTERN_OVERLAY(wifi2dmap);
FS_EXTERN_OVERLAY(battle);
FS_EXTERN_OVERLAY(wificlub);
FS_EXTERN_OVERLAY(pokelist);
FS_EXTERN_OVERLAY(ui_common);
FS_EXTERN_OVERLAY(pokemon_trade);
FS_EXTERN_OVERLAY(wificlub_subproc);
#define _LOCALMATCHNO (100)

//extern const GFL_PROC_DATA WifiClubProcData;

//----------------------------------------------------------------
// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];
//----------------------------------------------------------------

#define HEAPID_CORE GFL_HEAPID_APP

enum _EVENT_IRCBATTLE {
  _FIELD_FADE_START,
  _FIELD_CLOSE,
  _CALL_WIFICLUB,
  _WAIT_WIFICLUB,
  _WAIT_NET_END,
  _FIELD_OPEN,
  _FIELD_FADEIN,
  _FIELD_END,


  P2P_INIT,
  P2P_INIT2,
  P2P_LOGIN,
  P2P_MATCH_BOARD,
  P2P_SELECT,
  P2P_BATTLE,
  P2P_BATTLE2,
  P2P_TIMING_SYNC_CALL_BATTLE,
  P2P_BATTLE_START,
  P2P_BATTLE_END,
  P2P_TRADE,
  P2P_TRADE_MAIN,
  P2P_TRADE_END,
  P2P_EVOLUTION,
  P2P_EVOLUTION_END,
  P2P_TVT,
  P2P_TVT_END,
  P2P_EXIT,
  P2P_FREE,
  P2P_SETEND,
  P2P_NETENDCALL,
  P2P_NETEND,


};

static void _FreeMemory(EVENT_WIFICLUB_WORK * pClub);

typedef struct {
  u8 bSingle;
  u16 kind;
} NextMatchKindTbl;

NextMatchKindTbl aNextMatchKindTbl[] = {
  { 0, P2P_EXIT},
  { 0, P2P_INIT2},  //ERR
  { 0, P2P_EXIT},
  { 0, P2P_EXIT},
  { 0, P2P_EXIT},
  { 0, P2P_TVT}, //WIFI_P2PMATCH_TRADE:   // TVトランシーバ
  { 0, P2P_TRADE}, //WIFI_P2PMATCH_TRADE:   // ポケモントレード呼び出し
  { WIFI_GAME_BATTLE_SINGLE_ALL, P2P_BATTLE},   //バトル
  { WIFI_GAME_BATTLE_SINGLE_FLAT, P2P_BATTLE}, 
  { WIFI_GAME_BATTLE_DOUBLE_ALL, P2P_BATTLE}, 
  { WIFI_GAME_BATTLE_DOUBLE_FLAT, P2P_BATTLE}, 
  { WIFI_GAME_BATTLE_TRIPLE_ALL, P2P_BATTLE}, 
  { WIFI_GAME_BATTLE_TRIPLE_FLAT, P2P_BATTLE}, 
  { WIFI_GAME_BATTLE_ROTATION_ALL, P2P_BATTLE}, 
  { WIFI_GAME_BATTLE_ROTATION_FLAT, P2P_BATTLE}, 
  { 0, P2P_EXIT},   // 通信切断してます。終了します。
};


static void _battleSetting(EVENT_WIFICLUB_WORK* pClub,int gamemode)
{
  GAMEDATA* gamedata = GAMESYSTEM_GetGameData(pClub->gsys);
  int no;
  int shooter = pClub->pMatchParam->shooter;

  if(shooter){
    Regulation_SetParam(pClub->pMatchParam->pRegulation, REGULATION_SHOOTER, REGULATION_SHOOTER_VALID);
  }
  else{
    Regulation_SetParam(pClub->pMatchParam->pRegulation, REGULATION_SHOOTER, REGULATION_SHOOTER_INVALID);
  }
  
  switch(gamemode){
  case WIFI_GAME_BATTLE_SINGLE_FLAT:
    no = BATTLE_MODE_COLOSSEUM_SINGLE_50;
    if(shooter){
      no = BATTLE_MODE_COLOSSEUM_SINGLE_50_SHOOTER;
    }
    break;
  case WIFI_GAME_BATTLE_SINGLE_ALL:
    no = BATTLE_MODE_COLOSSEUM_SINGLE_FREE;
    if(shooter){
      no = BATTLE_MODE_COLOSSEUM_SINGLE_FREE_SHOOTER;
    }
    break;
  case WIFI_GAME_BATTLE_DOUBLE_FLAT:
    no = BATTLE_MODE_COLOSSEUM_DOUBLE_50;
    if(shooter){
      no = BATTLE_MODE_COLOSSEUM_DOUBLE_50_SHOOTER;
    }
    break;
  case WIFI_GAME_BATTLE_DOUBLE_ALL:
    no = BATTLE_MODE_COLOSSEUM_DOUBLE_FREE;
    if(shooter){
      no = BATTLE_MODE_COLOSSEUM_DOUBLE_FREE_SHOOTER;
    }
    break;
  case WIFI_GAME_BATTLE_TRIPLE_FLAT:
    no = BATTLE_MODE_COLOSSEUM_TRIPLE_50;
    if(shooter){
      no = BATTLE_MODE_COLOSSEUM_TRIPLE_50_SHOOTER;
    }
    break;
  case WIFI_GAME_BATTLE_TRIPLE_ALL:
    no = BATTLE_MODE_COLOSSEUM_TRIPLE_FREE;
    if(shooter){
      no = BATTLE_MODE_COLOSSEUM_TRIPLE_FREE_SHOOTER;
    }
    break;
  case WIFI_GAME_BATTLE_ROTATION_FLAT:
    no = BATTLE_MODE_COLOSSEUM_ROTATION_50;
    if(shooter){
      no = BATTLE_MODE_COLOSSEUM_ROTATION_50_SHOOTER;
    }
    break;
  case WIFI_GAME_BATTLE_ROTATION_ALL:
    no = BATTLE_MODE_COLOSSEUM_ROTATION_FREE;
    if(shooter){
      no = BATTLE_MODE_COLOSSEUM_ROTATION_FREE_SHOOTER;
    }
    break;
  }
  pClub->prm.battle_mode = no;
  pClub->prm.fight_count = 0;      ///<連勝数

  switch(gamemode){
  case WIFI_GAME_BATTLE_SINGLE_ALL:
  case WIFI_GAME_BATTLE_SINGLE_FLAT:
    BTL_SETUP_Single_Comm(
      pClub->para , gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
    break;
  case WIFI_GAME_BATTLE_DOUBLE_ALL:
  case WIFI_GAME_BATTLE_DOUBLE_FLAT:
    BTL_SETUP_Double_Comm( pClub->para , gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
    break;
  case WIFI_GAME_BATTLE_TRIPLE_ALL:
  case WIFI_GAME_BATTLE_TRIPLE_FLAT:
    BTL_SETUP_Triple_Comm( pClub->para , gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
    break;
  case WIFI_GAME_BATTLE_ROTATION_ALL:
  case WIFI_GAME_BATTLE_ROTATION_FLAT:
    BTL_SETUP_Rotation_Comm( pClub->para , gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
    break;
  }
  BTL_SETUP_AllocRecBuffer( pClub->para, GFL_HEAPID_APP); //録画バッファ確保 BTL_SETUPの後に必ず呼ばないといけない
  BATTLE_PARAM_SetRegulation( pClub->para, pClub->pMatchParam->pRegulation, HEAPID_PROC);

  //レベル補正を設定
  PokeRegulation_ModifyLevelPokeParty( pClub->pMatchParam->pRegulation, pClub->pMatchParam->pPokeParty[0]);
  PokeRegulation_ModifyLevelPokeParty( pClub->pMatchParam->pRegulation, pClub->pMatchParam->pPokeParty[1]);
  pClub->para->recordData = GAMEDATA_GetRecordPtr(GAMESYSTEM_GetGameData(pClub->gsys));
}


//==============================================================================
//  WIFIクラブからポケモンリスト用ワーク作成
//==============================================================================

static void _pokeListWorkMake(EVENT_WIFICLUB_WORK * ep2p,GAMEDATA* pGameData,u32 gamemode)
{
  int my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  MYSTATUS* pTarget = GAMEDATA_GetMyStatusPlayer(pGameData,1-my_net_id);
  WIFICLUB_BATTLE_SUBPROC_PARAM *initWork = &ep2p->subProcParam;
  
  initWork->regulation = ep2p->pMatchParam->pRegulation;
  initWork->selfPokeParty = ep2p->pMatchParam->pPokeParty[my_net_id];
  initWork->enemyName = MyStatus_GetMyName(pTarget);
  initWork->enemySex = MyStatus_GetMySex(pTarget);
  initWork->enemyPokeParty = ep2p->pMatchParam->pPokeParty[1-my_net_id];
  initWork->gameData = pGameData;
  initWork->comm_selected_num = 0;
  
  //以下[out]
  PokeParty_InitWork(ep2p->pPokeParty);
  initWork->p_party = ep2p->pPokeParty;
  initWork->netParty[0] = ep2p->pMatchParam->pPokeParty[0];
  initWork->netParty[1] = ep2p->pMatchParam->pPokeParty[1];

}

//==============================================================================
//  ポケモンリストからバトル用ポケパーティー作成
//==============================================================================
static void _pokeListWorkOut(EVENT_WIFICLUB_WORK * ep2p,GAMEDATA* pGameData,u32 gamemode)
{

  BATTLE_PARAM_SetPokeParty( ep2p->para, ep2p->pPokeParty, BTL_CLIENT_PLAYER );
}





//============================================================================================
//
//    サブイベント
//
//============================================================================================
static GMEVENT_RESULT EVENT_WiFiClubMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_WIFICLUB_WORK * pClub = work;
  EVENT_WIFICLUB_WORK * ep2p = work;
  GAMESYS_WORK * gsys = pClub->gsys;

  switch (*seq) {
  case _FIELD_FADE_START:
    
    pClub->soundNo = PMSND_GetBGMsoundNo();
    PMSND_FadeOutBGM(6);
    (*seq) ++;
    break;
  case _FIELD_CLOSE:

    if(GAME_COMM_NO_NULL == GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys)))
    {
      pClub->isEndProc = FALSE;
      GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, pClub->fieldmap));
      (*seq)++;
    }
    break;
  case _CALL_WIFICLUB:
    (*seq) = P2P_INIT;
    
    break;
  case _WAIT_NET_END:
    if(GFL_NET_IsExit()){
      (*seq) ++;
      _FreeMemory(ep2p);
      PMV_ResetMasterVolume();
      PMSND_PlayBGM( ep2p->soundNo );
      PMSND_FadeInBGM(PMSND_FADE_NORMAL);
    }
    break;
  case _FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case _FIELD_FADEIN:
    if(pClub->bFieldEnd){
      GMEVENT_CallEvent(event,EVENT_FieldFadeIn_Black(gsys, pClub->fieldmap, FIELD_FADE_WAIT));
    }
    (*seq) ++;
    break;
  case _FIELD_END:
    return GMEVENT_RES_FINISH;
    break;

  case P2P_INIT:
    GFL_STD_MemClear( &ep2p->login, sizeof(WIFILOGIN_PARAM) );
    (*seq)  = P2P_INIT2;
    ep2p->login.mode = WIFILOGIN_MODE_NORMAL;
    break;
  case P2P_INIT2:
    { //WIFI引数の設定
      ep2p->login.gamedata = GAMESYSTEM_GetGameData(pClub->gsys);
      ep2p->login.display = WIFILOGIN_DISPLAY_UP;
      ep2p->login.bg = WIFILOGIN_BG_NORMAL;
      ep2p->login.nsid = WB_NET_WIFICLUB;
    }
    ep2p->localProc = GFL_PROC_LOCAL_boot(HEAPID_PROC);
    GFL_PROC_LOCAL_CallProc(ep2p->localProc,
                            FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, &ep2p->login);
    ep2p->pMatchParam->seq = WIFI_GAME_NONE;
    (*seq)  = P2P_LOGIN;
    break;
  case P2P_LOGIN:
    if( GFL_PROC_LOCAL_Main(ep2p->localProc) != GFL_PROC_MAIN_NULL ){
      NetErr_DispCall(FALSE);
      return FALSE;
    }
    GFL_PROC_LOCAL_Exit(ep2p->localProc);
    
    if(WifiList_GetFriendDataNum(GAMEDATA_GetWiFiList(ep2p->pGameData)) == 0){
      (*seq)  = P2P_NETENDCALL;
    }
    else if( ep2p->login.result == WIFILOGIN_RESULT_LOGIN ){
      (*seq)  = P2P_MATCH_BOARD;
    }
    else{ 
      (*seq)  = P2P_EXIT;
    }
    break;
  case P2P_MATCH_BOARD:
    GFL_OVERLAY_Load(FS_OVERLAY_ID(wificlub));
    GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
    WIFI_STATUS_SetMyMac(ep2p->pMatchParam->pMatch);
    if(ep2p->para){
      BATTLE_PARAM_Delete(ep2p->para);
      ep2p->para = NULL;
    }
    GMEVENT_CallProc(pClub->event,FS_OVERLAY_ID(wifi2dmap), &WifiP2PMatchProcData, ep2p->pMatchParam);
    (*seq)  ++;
    break;
  case P2P_SELECT:
    (*seq) = aNextMatchKindTbl[ep2p->pMatchParam->seq].kind;
    if(ep2p->pMatchParam->seq == WIFI_GAME_ERROR){
       ep2p->login.mode = WIFILOGIN_MODE_ERROR;
    }
    ep2p->bSingle = aNextMatchKindTbl[ep2p->pMatchParam->seq].bSingle;
    GFL_OVERLAY_Unload(FS_OVERLAY_ID(ui_common));
    GFL_OVERLAY_Unload(FS_OVERLAY_ID(wificlub));
    break;
  case P2P_BATTLE:
    {
      _pokeListWorkMake(ep2p,GAMESYSTEM_GetGameData(pClub->gsys),(*seq)  );

      ep2p->para =BATTLE_PARAM_Create(HEAPID_PROC);
      _battleSetting(pClub, ep2p->pMatchParam->seq);

      GMEVENT_CallProc(pClub->event,FS_OVERLAY_ID(wificlub_subproc), &WifiClubBattle_Sub_ProcData, &ep2p->subProcParam);
      (*seq) ++;
    }
    break;
  case P2P_BATTLE2:
    if(ep2p->subProcParam.result == WIFICLUB_BATTLE_SUBPROC_RESULT_ERROR_NEXT_LOGIN){
      if(GFL_NET_IsInit()){
        pClub->pMatchParam->bTalk = WIFIP2PMATCH_STATE_NONE;
        (*seq) = P2P_MATCH_BOARD;
      }
      else{
        ep2p->login.mode = WIFILOGIN_MODE_ERROR;
        (*seq)  = P2P_INIT2;
      }
      break;
    }
    else{
      _pokeListWorkOut(ep2p,GAMESYSTEM_GetGameData(pClub->gsys),(*seq)  );
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );
      GFL_NET_AddCommandTable(GFL_NET_CMD_BATTLE, BtlRecvFuncTable, BTL_NETFUNCTBL_ELEMS, NULL);
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_LOCALMATCHNO,WB_NET_WIFICLUB);
      (*seq) ++;
    }
    break;
  case P2P_TIMING_SYNC_CALL_BATTLE:
    if( !GFL_NET_IsInit() || (NET_ERR_CHECK_NONE != NetErr_App_CheckError())){
      NetErr_ExitNetSystem();
      NetErr_DispCallPushPop();
      ep2p->login.mode = WIFILOGIN_MODE_ERROR;
      (*seq)  = P2P_INIT2;
    }
    else if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),_LOCALMATCHNO, WB_NET_WIFICLUB)){
      (*seq) ++;
    }
    break;
  case P2P_BATTLE_START:

    PMSND_PlayBGM(SEQ_BGM_VS_TRAINER_WIFI);
    PMSND_FadeInBGM(PMSND_FADE_NORMAL);

    //GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 1);
#if 0
    GAMESYSTEM_CallProc(ep2p->gsys, NO_OVERLAY_ID, &BtlProcData, ep2p->para);
#else
    {
      int i;

      ep2p->demo_prm.type = COMM_BTL_DEMO_TYPE_NORMAL_START;
      
      if( GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() )== 0 ){
        for( i=0;i<2;i++){
          ep2p->demo_prm.trainer_data[i].mystatus = GAMEDATA_GetMyStatusPlayer( GAMESYSTEM_GetGameData( gsys ),i );
          ep2p->demo_prm.trainer_data[i].party = ep2p->pMatchParam->pPokeParty[i];
        }
      }
      else{
        for( i=0;i<2;i++){
          u8 buff[]={1,0};
          ep2p->demo_prm.trainer_data[buff[i]].mystatus = GAMEDATA_GetMyStatusPlayer( GAMESYSTEM_GetGameData( gsys ),i );
          ep2p->demo_prm.trainer_data[buff[i]].party = ep2p->pMatchParam->pPokeParty[i];
        }
      }
      {
        ep2p->prm.gdata = GAMESYSTEM_GetGameData(pClub->gsys);
        ep2p->prm.btl_setup_prm = ep2p->para;
        ep2p->prm.demo_prm = &ep2p->demo_prm;
        ep2p->prm.error_auto_disp = TRUE;
        ep2p->demo_prm.record = GAMEDATA_GetRecordPtr(GAMESYSTEM_GetGameData(pClub->gsys));
        GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
        GMEVENT_CallProc(pClub->event, FS_OVERLAY_ID(event_battlecall), &CommBattleCommProcData, &ep2p->prm);
      }
    }
#endif
    //        GFL_PROC_SysCallProc(NO_OVERLAY_ID, GMEVENT_Sub_BattleProc, battle_param);
    (*seq) ++;
    break;
  case P2P_BATTLE_END:
    GF_ASSERT((pClub->pMatchParam->friendNo-1) >= 0);
    
    if(ep2p->demo_prm.result == COMM_BTL_DEMO_RESULT_WIN){
      // バトルの記録
      WifiList_SetResult(GAMEDATA_GetWiFiList(GAMESYSTEM_GetGameData(pClub->gsys)),
                         pClub->pMatchParam->friendNo-1,1,0,0);
    }
    else if(ep2p->demo_prm.result == COMM_BTL_DEMO_RESULT_LOSE){
      WifiList_SetResult(GAMEDATA_GetWiFiList(GAMESYSTEM_GetGameData(pClub->gsys)),
                         pClub->pMatchParam->friendNo-1,0,1,0);
    }
    GFL_NET_DelCommandTable(GFL_NET_CMD_BATTLE);
    (*seq)  = P2P_MATCH_BOARD;
    break;
  case P2P_TRADE:
    ep2p->aPokeTr.ret = POKEMONTRADE_MOVE_START;
    ep2p->aPokeTr.friendNo = pClub->pMatchParam->friendNo;
    (*seq) ++;
    break;
  case P2P_TRADE_MAIN:
    ep2p->aPokeTr.gamedata = GAMESYSTEM_GetGameData(pClub->gsys);

    GMEVENT_CallProc(pClub->event,FS_OVERLAY_ID(pokemon_trade), &PokemonTradeClubProcData, &ep2p->aPokeTr);
    (*seq) ++;
    break;
  case P2P_TRADE_END:
    switch(ep2p->aPokeTr.ret){
    case POKEMONTRADE_MOVE_EVOLUTION:
      (*seq)  = P2P_EVOLUTION;
      break;
    case POKEMONTRADE_MOVE_ERROR:
      ep2p->pMatchParam->seq = WIFI_GAME_NONE;
      (*seq)  = P2P_MATCH_BOARD;
      break;
    default:
      (*seq)  = P2P_MATCH_BOARD;
      break;
    }
    if(!GFL_NET_IsInit()){
      ep2p->login.mode = WIFILOGIN_MODE_ERROR;
      (*seq)  = P2P_INIT2;
    }
    break;
  case P2P_EVOLUTION:
    {
      SHINKA_DEMO_PARAM* sdp = GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof( SHINKA_DEMO_PARAM ) );
      sdp->gamedata          = GAMESYSTEM_GetGameData(pClub->gsys);
      sdp->ppt               = ep2p->aPokeTr.pParty;
      sdp->after_mons_no     = ep2p->aPokeTr.after_mons_no;
      sdp->shinka_pos        = 0;
      sdp->shinka_cond       = ep2p->aPokeTr.cond;
      sdp->b_field           = TRUE;
      sdp->b_enable_cancel   = FALSE;
      ep2p->aPokeTr.shinka_param = sdp;
    }
    GMEVENT_CallProc(pClub->event, FS_OVERLAY_ID(shinka_demo), &ShinkaDemoProcData, ep2p->aPokeTr.shinka_param );
    (*seq)  = P2P_EVOLUTION_END;
    break;
  case P2P_EVOLUTION_END:
    {
      SHINKA_DEMO_PARAM* sdp = ep2p->aPokeTr.shinka_param;
      GFL_HEAP_FreeMemory( sdp );
    }
    ep2p->aPokeTr.ret = POKEMONTRADE_MOVE_EVOLUTION;
    (*seq)  = P2P_TRADE_MAIN;
    break;
  case P2P_TVT:
    ep2p->aTVT.gameData = GAMESYSTEM_GetGameData(ep2p->gsys);
    ep2p->aTVT.mode = CTM_WIFI;
    GMEVENT_CallProc(pClub->event, FS_OVERLAY_ID(comm_tvt), &COMM_TVT_ProcData, &ep2p->aTVT);
    (*seq) ++;
    break;
  case P2P_TVT_END:
    if(!GFL_NET_IsInit()){
      ep2p->login.mode = WIFILOGIN_MODE_ERROR;
      (*seq)  = P2P_INIT2;
    }
    else{
      (*seq) = P2P_MATCH_BOARD;
    }
    break;
  case P2P_NETENDCALL:
    GFL_NET_Exit(NULL);
    (*seq) = P2P_NETEND;
    break;
  case P2P_NETEND:
    if(GFL_NET_IsResetEnable()){
      (*seq) = _WAIT_NET_END;
    }
    break;
  case P2P_EXIT:
  case P2P_SETEND:
  case P2P_FREE:
    (*seq) = _WAIT_NET_END;
    break;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}


/*
開放処理
 */

static void _FreeMemory(EVENT_WIFICLUB_WORK * pClub)
{
  if(pClub->para){
    BATTLE_PARAM_Delete(pClub->para);
    pClub->para = NULL;
  }
  
  GFL_HEAP_FreeMemory(pClub->pMatchParam->pPokeParty[0]);
  GFL_HEAP_FreeMemory(pClub->pMatchParam->pPokeParty[1]);
  GFL_HEAP_FreeMemory(pClub->pMatchParam->pRegulation);

  GFL_HEAP_FreeMemory(pClub->pPokeParty);
  GFL_HEAP_FreeMemory(pClub->pMatchParam->pMatch);
  GFL_HEAP_FreeMemory(pClub->pMatchParam);
}


/*
 *  @brief  WiFiクラブ呼び出しパラメータセット
 */
static void wifi_SetEventParam( GMEVENT* event, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap,BOOL bFieldEnd )
{
  BATTLE_SETUP_PARAM * para;
  EVENT_WIFICLUB_WORK * pClub;

  if(GAME_COMM_NO_NULL!= GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys))){
    GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(gsys));
  }

  pClub = GMEVENT_GetEventWork(event);
  pClub->gsys = gsys;
  pClub->fieldmap = fieldmap;
  pClub->event = event;
  pClub->bFieldEnd = bFieldEnd;
  
  pClub->pMatchParam = GFL_HEAP_AllocClearMemory(GetHeapLowID(HEAPID_PROC), sizeof(WIFIP2PMATCH_PROC_PARAM));
  pClub->pMatchParam->pMatch = GFL_HEAP_AllocClearMemory(GetHeapLowID(HEAPID_PROC), sizeof( WIFI_STATUS ));
  pClub->pMatchParam->pGameData = GAMESYSTEM_GetGameData(pClub->gsys);
  pClub->pMatchParam->pSaveData = GAMEDATA_GetSaveControlWork(pClub->pMatchParam->pGameData);
  pClub->pMatchParam->vchatMain = TRUE;
  pClub->pMatchParam->bTalk = WIFIP2PMATCH_STATE_NONE;  //これは必要なので削ってはいけません

  pClub->pGameData =  pClub->pMatchParam->pGameData;
  pClub->pWifiList = GAMEDATA_GetWiFiList(pClub->pGameData);
  pClub->pMatchParam->seq = WIFI_GAME_NONE;
  pClub->gsys = pClub->gsys;
  pClub->pPokeParty = PokeParty_AllocPartyWork(GetHeapLowID(HEAPID_PROC));
  pClub->aPokeTr.pParty = pClub->pPokeParty; // 同じ物を使う
  
  pClub->pWork = pClub;
  pClub->soundNo = PMSND_GetBGMsoundNo();

  pClub->pMatchParam->pPokeParty[0] = PokeParty_AllocPartyWork(GFL_HEAPID_APP);   //お互いのPartyを受信
  pClub->pMatchParam->pPokeParty[1] = PokeParty_AllocPartyWork(GFL_HEAPID_APP);   //お互いのPartyを受信
  pClub->pMatchParam->pRegulation = Regulation_AllocWork(GFL_HEAPID_APP);
  pClub->pMatchParam->battleBoard.shooterSelect = 1;
}

//------------------------------------------------------------------
/*
 *  @brief  WiFiクラブイベントコール
 */
//------------------------------------------------------------------
static GMEVENT* EVENT_WiFiClub( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, BOOL bFieldEnd )
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, EVENT_WiFiClubMain, sizeof(EVENT_WIFICLUB_WORK));
  wifi_SetEventParam( event, gsys, fieldmap, bFieldEnd );

  return event;
}


//----------------------------------------------------------------------------
/**
 *	@brief  Wifiクラブイベントを生成
///	GMEVENT_CreateOverlayEventCall関数用コールバック関数
 */
//-----------------------------------------------------------------------------
GMEVENT* EVENT_CallWiFiClub( GAMESYS_WORK * gsys, void * work )
{
  EV_WIFICLUB_PARAM* param = work;

  return EVENT_WiFiClub( gsys, param->fieldmap, param->bFieldEnd );
}



