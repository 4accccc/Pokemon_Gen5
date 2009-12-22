//============================================================================================
/**
 * @file  wifi_p2pevent.c
 * @bfief WIFIP2Pイベント制御
 * @author  k.ohno
 * @date  06.04.14
 */
//============================================================================================

#include <gflib.h>
#include "net/network_define.h"

#include "gamesystem/game_event.h"

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
FS_EXTERN_OVERLAY(pokemon_trade);
#define _LOCALMATCHNO (100)
//----------------------------------------------------------------
// バトル用定義
extern const NetRecvFuncTable BtlRecvFuncTable[];
//----------------------------------------------------------------


typedef struct{
  COMM_TVT_INIT_WORK aTVT;
  WIFIP2PMATCH_PROC_PARAM* pMatchParam;
  POKEMONTRADE_PARAM aPokeTr;
  GAMEDATA* pGameData;
  WIFI_LIST* pWifiList;
  GAMESYS_WORK * gsys;
  BATTLE_SETUP_PARAM para;
  int seq;
  u16* ret;
  u8 lvLimit;
  u8 bSingle;
  void* work;   // wifiPofin用ワーク
  u32 vchat;
}EV_P2PEVENT_WORK;

enum{
  P2P_INIT,
  P2P_MATCH_BOARD,
  P2P_SELECT,
  P2P_BATTLE,
  P2P_TIMING_SYNC_CALL_BATTLE,
  P2P_BATTLE_START,
  P2P_BATTLE_END,
  P2P_TRADE,
  P2P_TRADE_END,
  P2P_TVT,
  P2P_TVT_END,
  P2P_UTIL,
  P2P_EXIT,
  P2P_FREE,
  P2P_SETEND,

  P2P_NOP
};


// WiFiP2PMatrch ４人接続画面人数制限定義
//static const u8 sc_P2P_FOUR_MATCH_MAX[ WFP2PMF_TYPE_NUM ] = {
//  3, 4, 4, 4
//};


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


typedef struct {
  u8 lvLimit;
  u8 bSingle;
  u16 kind;
} NextMatchKindTbl;

NextMatchKindTbl aNextMatchKindTbl[] = {
  {0, 0, 0},
  {50,WIFI_BATTLEFLAG_SINGLE, P2P_BATTLE},   // WIFI_P2PMATCH_SBATTLE50:   // 通信対戦呼び出し
  {100, WIFI_BATTLEFLAG_SINGLE, P2P_BATTLE}, // WIFI_P2PMATCH_SBATTLE100:   // 通信対戦呼び出し
  {0, WIFI_BATTLEFLAG_SINGLE, P2P_BATTLE}, // WIFI_P2PMATCH_SBATTLE_FREE:   // 通信対戦呼び出し
  {50,WIFI_BATTLEFLAG_DOUBLE, P2P_BATTLE}, //WIFI_P2PMATCH_DBATTLE50:   // 通信対戦呼び出し
  {100,WIFI_BATTLEFLAG_DOUBLE, P2P_BATTLE}, //WIFI_P2PMATCH_DBATTLE100:   // 通信対戦呼び出し
  {0,WIFI_BATTLEFLAG_DOUBLE, P2P_BATTLE}, //WIFI_P2PMATCH_DBATTLE_FREE:   // 通信対戦呼び出し
  {0,0, P2P_TRADE}, //WIFI_P2PMATCH_TRADE:   // ポケモントレード呼び出し
  {0,0, P2P_TVT}, //WIFI_P2PMATCH_TRADE:   // ポケモントレード呼び出し
  {0,0, P2P_EXIT},   // 通信切断してます。終了します。
  {0,0, P2P_UTIL},   ////WIFIUTILへ飛びます
  {0,0, P2P_SETEND}, //WIFI_P2PMATCH_DPW_END:  //DPWへいく場合
};



//==============================================================================
//  WIFI通信入り口
//==============================================================================
static GFL_PROC_RESULT WifiClubProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  int len;
  EVENT_WIFICLUB_WORK* pClub = (void*)pwk;
  EV_P2PEVENT_WORK * ep2p = pClub->pWork;

  switch (ep2p->seq) {
  case P2P_INIT:

    ep2p->seq = P2P_MATCH_BOARD;
    if(ep2p->pMatchParam->seq == WIFI_P2PMATCH_DPW){
      if( mydwc_checkMyGSID() ){
        ep2p->seq = P2P_FREE;  //コード取得済みの場合なにもしない
        *(ep2p->ret) = 0;
      }
    }
    break;
  case P2P_MATCH_BOARD:
    GFL_OVERLAY_Load(FS_OVERLAY_ID(wificlub));
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(wifi2dmap), &WifiP2PMatchProcData, ep2p->pMatchParam);
    ep2p->seq ++;
    break;
  case P2P_SELECT:
    if(GFL_NET_IsInit()){
      if( mydwc_checkMyGSID() ){  // コード取得に成功
        // きっと置きなおす
        //            SysFlag_WifiUseSet(SaveData_GetEventWork(fsys->savedata));
      }
    }
    ep2p->seq = aNextMatchKindTbl[ep2p->pMatchParam->seq].kind;
    NET_PRINT("P2P_SELECT %d %d\n",ep2p->seq,ep2p->pMatchParam->seq);
    ep2p->lvLimit = aNextMatchKindTbl[ep2p->pMatchParam->seq].lvLimit;
    ep2p->bSingle = aNextMatchKindTbl[ep2p->pMatchParam->seq].bSingle;
    GFL_OVERLAY_Unload(FS_OVERLAY_ID(wificlub));
    switch(ep2p->pMatchParam->seq){
    case WIFI_P2PMATCH_DPW_END:  //DPWへいく場合
      *(ep2p->ret) = 1;
      ep2p->seq = P2P_SETEND;
      break;
    }
    break;
  case P2P_BATTLE:
    {
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
    pClub->para.rule = BTL_RULE_SINGLE;
    pClub->para.netHandle = GFL_NET_HANDLE_GetCurrentHandle();
    pClub->para.commPos = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
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
    ep2p->aPokeTr.gsys=ep2p->gsys;
    ep2p->aPokeTr.type = POKEMONTRADE_WIFICLUB;
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(pokemon_trade), &PokemonTradeProcData, &ep2p->aPokeTr);
    ep2p->seq++;
    break;
  case P2P_TRADE_END:
    ep2p->seq = P2P_MATCH_BOARD;
    break;
/*
お疲れ様です、有泉です。

通信TVTの呼び出しです。
ヘッダファイル include/net_app/comm_tvt_sys.h
extern GFL_PROC_DATA COMM_TVT_ProcData;

COMM_TVT_INIT_WORK の mode に CTM_WIFI を渡して起動してください。
よろしくお願いいたします。
*/


  case P2P_TVT:
    ep2p->aTVT.gameData = GAMESYSTEM_GetGameData(ep2p->gsys);
    ep2p->aTVT.mode = CTM_WIFI;
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(comm_tvt), &COMM_TVT_ProcData, &ep2p->aTVT);
    ep2p->seq++;
    break;
  case P2P_TVT_END:
    ep2p->seq = P2P_MATCH_BOARD;
    break;

  case P2P_UTIL:
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(wifi_util), &WifiUtilProcData, NULL);
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

static GFL_PROC_RESULT WifiClubProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_WIFICLUB_WORK* pClub = pwk;
  EV_P2PEVENT_WORK* ep2p = GFL_PROC_AllocWork(proc, sizeof(EV_P2PEVENT_WORK),GetHeapLowID(HEAPID_PROC));

  GFL_STD_MemClear(ep2p, sizeof(EV_P2PEVENT_WORK));
  ep2p->pMatchParam = GFL_HEAP_AllocClearMemory(GetHeapLowID(HEAPID_PROC), sizeof(WIFIP2PMATCH_PROC_PARAM));
  ep2p->pMatchParam->pMatch = GFL_HEAP_AllocClearMemory(GetHeapLowID(HEAPID_PROC), WIFI_STATUS_GetSize());
  WIFI_STATUS_SetMyMac(ep2p->pMatchParam->pMatch);
  ep2p->pMatchParam->pSaveData = pClub->ctrl;
  ep2p->pMatchParam->pGameData = GAMESYSTEM_GetGameData(pClub->gsys);
  NET_PRINT("%x %x\n",(int)ep2p->pMatchParam->pMatch,(int)pClub->ctrl);

  ep2p->pGameData =  GAMESYSTEM_GetGameData(pClub->gsys);
  ep2p->pWifiList = GAMEDATA_GetWiFiList(ep2p->pGameData);
  ep2p->pMatchParam->seq = P2P_INIT;
  ep2p->gsys = pClub->gsys;

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




