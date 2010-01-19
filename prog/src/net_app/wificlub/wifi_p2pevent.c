//============================================================================================
/**
 * @file  wifi_p2pevent.c
 * @bfief WIFIP2Pイベント制御
 * @author  k.ohno
 * @date  06.04.14
 */
//============================================================================================

#include <gflib.h>
#include "pm_define.h"
#include "net/network_define.h"

#include "gamesystem/game_event.h"
#include "gamesystem/btl_setup.h"

#include "net_app/wificlub/wifi_p2pmatch.h"
//#include "net_app/wificlub/wifi_p2pmatchfour.h"
#include "net_app/pokemontrade.h"
#include "wifi_p2pmatch_local.h"

#include "net/dwc_rap.h"
#include "net/dwc_raputil.h"
#include "net/dwc_rapcommon.h"
#include "savedata/wifilist.h"
#include "system/main.h"
#include "battle/battle.h"  //BTL_RULE_SINGLE
#include "sound/pm_sndsys.h"

#include "field/event_wificlub.h"

#include "net_app/comm_tvt_sys.h"  //TVトランシーバ
#include "wifi_status.h"
#include "net_app/wifi_login.h"

#include "app/pokelist.h"

//#include "net_app/balloon.h"

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT WifiClubProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT WifiClubProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT WifiClubProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================
//  データ
//==============================================================================
const GFL_PROC_DATA WifiClubProcData = {
  WifiClubProcInit,
  WifiClubProcMain,
  WifiClubProcEnd,
};


FS_EXTERN_OVERLAY(wifi2dmap);
FS_EXTERN_OVERLAY(battle);
FS_EXTERN_OVERLAY(wificlub);
FS_EXTERN_OVERLAY(pokelist);
FS_EXTERN_OVERLAY(pokemon_trade);
#define _LOCALMATCHNO (100)
//----------------------------------------------------------------
// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];
//----------------------------------------------------------------


typedef struct{
  PLIST_DATA PokeList;
  COMM_TVT_INIT_WORK aTVT;
  WIFIP2PMATCH_PROC_PARAM* pMatchParam;
  POKEMONTRADE_PARAM aPokeTr;
  GAMEDATA* pGameData;
  WIFI_LIST* pWifiList;
  GAMESYS_WORK * gsys;
  WIFILOGIN_PARAM     login;
  BATTLE_SETUP_PARAM para;
  POKEPARTY* pPokeParty;   //バトルに渡すPokeParty
  int seq;
  u16* ret;
  u8 bSingle;
  u8 dummy;
}EV_P2PEVENT_WORK;

enum{
  P2P_INIT,
  P2P_LOGIN,
  P2P_MATCH_BOARD,
  P2P_SELECT,
  P2P_BATTLE,
  P2P_BATTLE2,
  P2P_TIMING_SYNC_CALL_BATTLE,
  P2P_BATTLE_START,
  P2P_BATTLE_END,
  P2P_TRADE,
  P2P_TRADE_END,
  P2P_TVT,
  P2P_TVT_END,
  P2P_EXIT,
  P2P_FREE,
  P2P_SETEND,

  P2P_NOP
};



#define _BLOCK (0)

//-------------------------------------
#if _BLOCK
/// バトルルームの実験のためプロセス変更関数を作成
static void P2P_FourWaitInit( EV_P2PEVENT_WORK* p_wk, GMEVENT* fsys, u32 heapID, u32 type );
static u32 P2P_FourWaitEnd( EV_P2PEVENT_WORK* p_wk );
#else
static void P2P_FourWaitInit( EV_P2PEVENT_WORK* p_wk, GMEVENT* fsys, u32 heapID, u32 type ){}
static u32 P2P_FourWaitEnd( EV_P2PEVENT_WORK* p_wk ){ return P2P_MATCH_BOARD; }
#endif

static void _pokmeonListWorkFree(EV_P2PEVENT_WORK* ep2p);

typedef struct {
  u8 bSingle;
  u16 kind;
} NextMatchKindTbl;

NextMatchKindTbl aNextMatchKindTbl[] = {
  { 0, P2P_EXIT},
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


static void _battleSetting(EVENT_WIFICLUB_WORK* pClub,EV_P2PEVENT_WORK * ep2p,int gamemode)
{
  GAMEDATA* gamedata = GAMESYSTEM_GetGameData(pClub->gsys);
  
  switch(gamemode){
  case WIFI_GAME_BATTLE_SINGLE_ALL:
  case WIFI_GAME_BATTLE_SINGLE_FLAT:
    BTL_SETUP_Single_Comm(
      &pClub->para , gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
    break;
  case WIFI_GAME_BATTLE_DOUBLE_ALL:
  case WIFI_GAME_BATTLE_DOUBLE_FLAT:
    BTL_SETUP_Double_Comm( &pClub->para , gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
    break;
  case WIFI_GAME_BATTLE_TRIPLE_ALL:
  case WIFI_GAME_BATTLE_TRIPLE_FLAT:
    BTL_SETUP_Triple_Comm( &pClub->para , gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
    break;
  case WIFI_GAME_BATTLE_ROTATION_ALL:
  case WIFI_GAME_BATTLE_ROTATION_FLAT:
    BTL_SETUP_Rotation_Comm( &pClub->para , gamedata , GFL_NET_HANDLE_GetCurrentHandle() , BTL_COMM_DS, HEAPID_PROC );
    break;
  }
  BATTLE_PARAM_SetPokeParty( &pClub->para, ep2p->pPokeParty, BTL_CLIENT_PLAYER );

}

//==============================================================================
//  WIFIクラブからポケモンリスト用ワーク作成
//==============================================================================

static void _pokeListWorkMake(EV_P2PEVENT_WORK * ep2p,GAMEDATA* pGameData,u32 gamemode)
{
  int my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  PLIST_DATA* plist = &ep2p->PokeList;
  MYSTATUS* pTarget = GAMEDATA_GetMyStatusPlayer(pGameData,1-my_net_id);
  
  plist->reg = ep2p->pMatchParam->pRegulation;
  plist->pp = ep2p->pMatchParam->pPokeParty[my_net_id];
  plist->mode = PL_MODE_BATTLE;
  plist->myitem = GAMEDATA_GetMyItem(pGameData);
  plist->cfg = SaveData_GetConfig(GAMEDATA_GetSaveControlWork(pGameData));
  plist->is_disp_party = TRUE;
  plist->use_tile_limit = FALSE;
  plist->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].pp = ep2p->pMatchParam->pPokeParty[1-my_net_id];
  plist->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].name = MyStatus_GetMyName(pTarget);
  plist->comm_battle[PL_COMM_PLAYER_TYPE_ENEMY_A].sex = MyStatus_GetMySex(pTarget);
  plist->comm_type = PL_COMM_SINGLE;

  switch(gamemode){
  case WIFI_GAME_BATTLE_SINGLE_ALL:
  case WIFI_GAME_BATTLE_SINGLE_FLAT:
    plist->type = PL_TYPE_SINGLE;
    break;
  case WIFI_GAME_BATTLE_DOUBLE_ALL:
  case WIFI_GAME_BATTLE_DOUBLE_FLAT:
    plist->type = PL_TYPE_DOUBLE;
    break;
  case WIFI_GAME_BATTLE_TRIPLE_ALL:
  case WIFI_GAME_BATTLE_TRIPLE_FLAT:
    plist->type = PL_TYPE_TRIPLE;
    break;
  case WIFI_GAME_BATTLE_ROTATION_ALL:
  case WIFI_GAME_BATTLE_ROTATION_FLAT:
    plist->type = PL_TYPE_ROTATION;
    break;
  }
}

//==============================================================================
//  ポケモンリストからバトル用ポケパーティー作成
//==============================================================================
static void _pokeListWorkOut(EV_P2PEVENT_WORK * ep2p,GAMEDATA* pGameData,u32 gamemode)
{
  int entry_no;
  int my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  PLIST_DATA* plist = &ep2p->PokeList;
  MYSTATUS* pTarget = GAMEDATA_GetMyStatusPlayer(pGameData,1-my_net_id);

  PokeParty_InitWork(ep2p->pPokeParty);

  for(entry_no = 0; entry_no < TEMOTI_POKEMAX; entry_no++){
    if(plist->in_num[entry_no] == 0){
      break;
    }
    PokeParty_Add(ep2p->pPokeParty, 
                  PokeParty_GetMemberPointer(plist->pp, plist->in_num[entry_no] - 1));
  }
}



//==============================================================================
//  WIFI通信入り口
//==============================================================================
static GFL_PROC_RESULT WifiClubProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  int len;
  EVENT_WIFICLUB_WORK* pClub = (void*)pwk;
  EV_P2PEVENT_WORK * ep2p = pClub->pWork;
  GAMESYS_WORK * gsys = pClub->gsys;

  switch (ep2p->seq) {
  case P2P_INIT:
    ep2p->seq = P2P_LOGIN;
    { //WIFI引数の設定
      GFL_STD_MemClear( &ep2p->login, sizeof(WIFILOGIN_PARAM) );
      ep2p->login.gamedata = GAMESYSTEM_GetGameData(pClub->gsys);
    }
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, &ep2p->login);
    break;
  case P2P_LOGIN:
      if( ep2p->login.result == WIFILOGIN_RESULT_LOGIN ){
        ep2p->seq = P2P_MATCH_BOARD;
      }
      else{ 
        ep2p->seq = P2P_EXIT;
      }
    break;
  case P2P_MATCH_BOARD:
    GFL_OVERLAY_Load(FS_OVERLAY_ID(wificlub));
    WIFI_STATUS_SetMyMac(ep2p->pMatchParam->pMatch);

    ep2p->pMatchParam->pPokeParty[0] = PokeParty_AllocPartyWork(GFL_HEAPID_APP);   //お互いのPartyを受信
    ep2p->pMatchParam->pPokeParty[1] = PokeParty_AllocPartyWork(GFL_HEAPID_APP);   //お互いのPartyを受信
    ep2p->pMatchParam->pRegulation = Regulation_AllocWork(GFL_HEAPID_APP);

    GFL_PROC_SysCallProc(FS_OVERLAY_ID(wifi2dmap), &WifiP2PMatchProcData, ep2p->pMatchParam);
    ep2p->seq ++;
    break;
  case P2P_SELECT:
    ep2p->seq = aNextMatchKindTbl[ep2p->pMatchParam->seq].kind;
    NET_PRINT("P2P_SELECT %d %d\n",ep2p->seq,ep2p->pMatchParam->seq);
    if(P2P_BATTLE != ep2p->seq){  // バトル以外はいらない
      _pokmeonListWorkFree(ep2p);      // ポケモンリストが終わったら要らない
    }
    ep2p->bSingle = aNextMatchKindTbl[ep2p->pMatchParam->seq].bSingle;
    GFL_OVERLAY_Unload(FS_OVERLAY_ID(wificlub));
    break;
  case P2P_BATTLE:
    {
      _pokeListWorkMake(ep2p,GAMESYSTEM_GetGameData(pClub->gsys),ep2p->seq );
      GFL_PROC_SysCallProc(FS_OVERLAY_ID(pokelist), &PokeList_ProcData, &ep2p->PokeList);
      ep2p->seq++;
    }
    break;
  case P2P_BATTLE2:
    {
      _pokeListWorkOut(ep2p,GAMESYSTEM_GetGameData(pClub->gsys),ep2p->seq );
      
      _pokmeonListWorkFree(ep2p);      // ポケモンリストが終わったら要らない
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle ) );
      GFL_NET_AddCommandTable(GFL_NET_CMD_BATTLE, BtlRecvFuncTable, BTL_NETFUNCTBL_ELEMS, NULL);
      GFL_NET_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_LOCALMATCHNO);
      ep2p->seq++;
    }
    break;
  case P2P_TIMING_SYNC_CALL_BATTLE:
    if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_LOCALMATCHNO)){
      ep2p->seq++;
    }
    break;
  case P2P_BATTLE_START:

    _battleSetting(pClub, ep2p ,ep2p->pMatchParam->seq);
    PMSND_PlayBGM(pClub->para.musicDefault);

    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 1);
    GAMESYSTEM_CallProc(ep2p->gsys, NO_OVERLAY_ID, &BtlProcData, &pClub->para);
    //        GFL_PROC_SysCallProc(NO_OVERLAY_ID, GMEVENT_Sub_BattleProc, battle_param);
    ep2p->seq++;
    break;
  case P2P_BATTLE_END:
    ep2p->seq = P2P_MATCH_BOARD;
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );
    break;
  case P2P_TRADE:
    ep2p->aPokeTr.gamedata = GAMESYSTEM_GetGameData(pClub->gsys);
    ep2p->aPokeTr.type = POKEMONTRADE_WIFICLUB;
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(pokemon_trade), &PokemonTradeProcData, &ep2p->aPokeTr);
    ep2p->seq++;
    break;
  case P2P_TRADE_END:
    ep2p->seq = P2P_MATCH_BOARD;
    break;
  case P2P_TVT:
    ep2p->aTVT.gameData = GAMESYSTEM_GetGameData(ep2p->gsys);
    ep2p->aTVT.mode = CTM_WIFI;
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(comm_tvt), &COMM_TVT_ProcData, &ep2p->aTVT);
    ep2p->seq++;
    break;
  case P2P_TVT_END:
    ep2p->seq = P2P_MATCH_BOARD;
    break;

  case P2P_EXIT:
  case P2P_SETEND:
  case P2P_FREE:
    return TRUE;
  default:
    return TRUE;
  }
  return FALSE;
}


static void _pokmeonListWorkFree(EV_P2PEVENT_WORK* ep2p)
{
  GFL_HEAP_FreeMemory(ep2p->pMatchParam->pPokeParty[0]);
  GFL_HEAP_FreeMemory(ep2p->pMatchParam->pPokeParty[1]);
  GFL_HEAP_FreeMemory(ep2p->pMatchParam->pRegulation);
}


static GFL_PROC_RESULT WifiClubProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_WIFICLUB_WORK* pClub = pwk;
  EV_P2PEVENT_WORK* ep2p = GFL_PROC_AllocWork(proc, sizeof(EV_P2PEVENT_WORK),GetHeapLowID(HEAPID_PROC));

  GFL_STD_MemClear(ep2p, sizeof(EV_P2PEVENT_WORK));
  ep2p->pMatchParam = GFL_HEAP_AllocClearMemory(GetHeapLowID(HEAPID_PROC), sizeof(WIFIP2PMATCH_PROC_PARAM));
  ep2p->pMatchParam->pMatch = GFL_HEAP_AllocClearMemory(GetHeapLowID(HEAPID_PROC), sizeof( WIFI_STATUS ));
  ep2p->pMatchParam->pSaveData = pClub->ctrl;
  ep2p->pMatchParam->pGameData = GAMESYSTEM_GetGameData(pClub->gsys);
  ep2p->pMatchParam->vchatMain = TRUE;

  ep2p->pGameData =  GAMESYSTEM_GetGameData(pClub->gsys);
  ep2p->pWifiList = GAMEDATA_GetWiFiList(ep2p->pGameData);
  ep2p->pMatchParam->seq = WIFI_GAME_NONE;
  ep2p->gsys = pClub->gsys;
  ep2p->pPokeParty = PokeParty_AllocPartyWork(GetHeapLowID(HEAPID_PROC));

  pClub->pWork = ep2p;

  // サウンドテスト
  // ＢＧＭ一時停止→退避
  PMSND_PauseBGM(TRUE);
  PMSND_PushBGM();


  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT WifiClubProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_WIFICLUB_WORK* pClub = pwk;
  EV_P2PEVENT_WORK* ep2p = pClub->pWork;

  GFL_HEAP_FreeMemory(ep2p->pPokeParty);
  GFL_HEAP_FreeMemory(ep2p->pMatchParam->pMatch);
  GFL_HEAP_FreeMemory(ep2p->pMatchParam);
  GFL_PROC_FreeWork(proc);

  // サウンドテスト
  // ＢＧＭ取り出し→再開
  PMSND_PopBGM();
  PMSND_PauseBGM(FALSE);
  PMSND_FadeInBGM(60);


  return GFL_PROC_RES_FINISH;
}




