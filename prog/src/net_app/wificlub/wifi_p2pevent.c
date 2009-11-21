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
  WIFIP2PMATCH_PROC_PARAM* pMatchParam;
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
  P2P_UTIL,
  P2P_EXIT,
  P2P_FREE,
  P2P_SETEND,
  P2P_POFIN_WAIT,
  P2P_POFIN_WAIT_END,
  P2P_POFIN,
  P2P_POFIN_END,
  P2P_FRONTIER,
  P2P_FRONTIER_END,

#ifdef WFP2P_DEBUG_EXON
  P2P_BATTLEROOM,
  P2P_BATTLEROOM_END,
#endif
  P2P_BUCKET_WAIT,
  P2P_BUCKET_WAIT_END,
  P2P_BUCKET,
  P2P_BUCKET_END,

  P2P_BALANCE_BALL_WAIT,
  P2P_BALANCE_BALL_WAIT_END,
  P2P_BALANCE_BALL,
  P2P_BALANCE_BALL_END,

  P2P_BALLOON_WAIT,
  P2P_BALLOON_WAIT_END,
  P2P_BALLOON,
  P2P_BALLOON_END,

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

//  たまいれ設定
//static BUCKET_PROC_WORK* BCT_ProcSet( FIELDSYS_WORK* fsys, u32 heapID, u32 vchat );
//static void BCT_ProcEnd( BUCKET_PROC_WORK* p_wk );

// たまのり設定
//static BB_PROC_WORK* BB_ProcSet( FIELDSYS_WORK* fsys, u32 heapID, u32 vchat );
//static void BB_ProcEnd( BB_PROC_WORK* p_wk );

#if _BLOCK
// ふうせんわり設定
//static BALLOON_PROC_WORK* BL_ProcSet( FIELD_MAIN_WORK* fsys, u32 heapID, u32 vchat );
static BALLOON_PROC_WORK* BL_ProcSet( GMEVENT* fsys, u32 heapID, u32 vchat );
static void BL_ProcEnd( BALLOON_PROC_WORK* p_wk );
#else
//static BALLOON_PROC_WORK* BL_ProcSet( GMEVENT* fsys, u32 heapID, u32 vchat ){}
//static void BL_ProcEnd( BALLOON_PROC_WORK* p_wk ){}
#endif

// レコード ミニゲーム遊んだスコア設定
//static void P2P_Record_Minigame( FIELDSYS_WORK* fsys );


#ifdef WFP2P_DEBUG
//static WIFI_BATTLEROOM* WBR_ProcSet( FIELDSYS_WORK* fsys, u32 heapID );
//static void WBR_ProcEnd( WIFI_BATTLEROOM* p_wk );

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
  {0,0, P2P_EXIT},   // 通信切断してます。終了します。
  {0,0, P2P_UTIL},   ////WIFIUTILへ飛びます
  {0,0, P2P_SETEND}, //WIFI_P2PMATCH_DPW_END:  //DPWへいく場合
  {0,0, P2P_FRONTIER},   //WIFI_P2PMATCH_FRONTIER:   // フロンティア呼び出し
  {0,0, P2P_BUCKET_WAIT},   //WIFI_P2PMATCH_BUCKET:   // バケット呼び出し
  {0,0, P2P_BALANCE_BALL_WAIT},   //WIFI_P2PMATCH_BALANCEBALL:   // 玉乗り呼び出し
  {0,0, P2P_BALLOON_WAIT},   //WIFI_P2PMATCH_BALLOON:   // ふうせん呼び出し
#ifdef WFP2P_DEBUG_EXON //
  {0, 0, P2P_BATTLEROOM},   //WIFI_P2PMATCH_BALLOON:   // ふうせん呼び出し
  {0, WIFI_BATTLEFLAG_MULTI, P2P_BATTLE},   //MULTI フリー対戦
#endif
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
    pClub->para.netID = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
    pClub->para.commPos = pClub->para.netID;
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
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(pokemon_trade), &PokemonTradeProcData, ep2p->gsys);
    ep2p->seq++;
    break;
  case P2P_TRADE_END:
    ep2p->seq = P2P_MATCH_BOARD;
    break;
  case P2P_UTIL:
    GFL_PROC_SysCallProc(FS_OVERLAY_ID(wifi_util), &WifiUtilProcData, NULL);
    break;
  case P2P_EXIT:
  case P2P_SETEND:
  case P2P_FREE:
    return TRUE;

  case P2P_POFIN_WAIT:
    ep2p->seq++;
    break;

  case P2P_POFIN_WAIT_END:
    break;

  case P2P_POFIN:

    //        ep2p->work = (void*)FieldNutMixer_InitCallWiFi(fsys,HEAPID_WORLD,ep2p->vchat);
    ep2p->seq++;
    break;
  case P2P_POFIN_END:
    //        if( !FieldEvent_Cmd_WaitSubProcEnd(fsys) ){   //サブプロセス終了待ち
    //          sys_FreeMemoryEz(ep2p->work);
    //        ep2p->seq = P2P_MATCH_BOARD;
    //  }
    break;

  case P2P_FRONTIER:
  GFL_NET_SetWifiBothNet(FALSE);
//  ep2p->work = EvCmdFrontierSystemCall( fsys,  NULL );
    ep2p->seq++;
    break;
  case P2P_FRONTIER_END:
    //        if( !FieldEvent_Cmd_WaitSubProcEnd(fsys) ){   //サブプロセス終了待ち
    //            sys_FreeMemoryEz(ep2p->work);
    //            ep2p->seq = P2P_MATCH_BOARD;
    //        }
    break;

#ifdef WFP2P_DEBUG_EXON
  case P2P_BATTLEROOM:
    //        ep2p->work = WBR_ProcSet(fsys,HEAPID_WORLD);
    ep2p->seq++;
    break;

  case P2P_BATTLEROOM_END:
    //      if( WBRSys_EndCheck( ep2p->work ) == TRUE ){    //サブプロセス終了待ち
    //        ep2p->seq = P2P_MATCH_BOARD;
    //    WBR_ProcEnd( ep2p->work );
    //  }
    break;
#endif

  case P2P_BUCKET_WAIT:
    //    P2P_FourWaitInit( ep2p, fsys, HEAPID_WORLD, WFP2PMF_TYPE_BUCKET );
    //      ep2p->seq++;
    break;

  case P2P_BUCKET_WAIT_END:
    //    if( !FieldEvent_Cmd_WaitSubProcEnd(fsys) ){   //サブプロセス終了待ち
    //      ep2p->seq = P2P_FourWaitEnd( ep2p );
    //}
    break;

  case P2P_BUCKET:
    // ミニゲームスコア設定
    //    P2P_Record_Minigame( fsys );
    //        ep2p->work = BCT_ProcSet(fsys,HEAPID_WORLD,ep2p->vchat);
    //        ep2p->seq++;
    break;

  case P2P_BUCKET_END:
    //        if( !FieldEvent_Cmd_WaitSubProcEnd(fsys) ){   //サブプロセス終了待ち
    //            ep2p->seq = P2P_MATCH_BOARD;
    //      BCT_ProcEnd( ep2p->work );
    //        }
    break;

  case P2P_BALANCE_BALL_WAIT:
    //    P2P_FourWaitInit( ep2p, fsys, HEAPID_WORLD, WFP2PMF_TYPE_BALANCE_BALL );
    //        ep2p->seq++;
    break;

  case P2P_BALANCE_BALL_WAIT_END:
    //        if( !FieldEvent_Cmd_WaitSubProcEnd(fsys) ){   //サブプロセス終了待ち
    //            ep2p->seq = P2P_FourWaitEnd( ep2p );
    //        }
    break;

  case P2P_BALANCE_BALL:
    // ミニゲームスコア設定
    //    P2P_Record_Minigame( fsys );
    //        ep2p->work = BB_ProcSet(fsys,HEAPID_WORLD,ep2p->vchat);
    //        ep2p->seq++;
    break;

  case P2P_BALANCE_BALL_END:
    //       if( !FieldEvent_Cmd_WaitSubProcEnd(fsys) ){    //サブプロセス終了待ち
    //           ep2p->seq = P2P_MATCH_BOARD;
    //      BB_ProcEnd( ep2p->work );
    //      }
    break;

  case P2P_BALLOON_WAIT:
//    P2P_FourWaitInit( ep2p, pClub->event , HEAPID_NETWORK, WFP2PMF_TYPE_BALLOON );
    ep2p->seq++;
    break;

  case P2P_BALLOON_WAIT_END:

    if( !GAMESYSTEM_EVENT_IsExists(pClub->gsys) ){    //サブプロセス終了待ち
      //     if( !FieldEvent_Cmd_WaitSubProcEnd(fsys) ){    //サブプロセス終了待ち
      ep2p->seq = P2P_FourWaitEnd( ep2p );
      GFL_OVERLAY_Unload(FS_OVERLAY_ID(wificlub));

    }
    break;

  case P2P_BALLOON:
    // ミニゲームスコア設定
    //    P2P_Record_Minigame( fsys );
#if _BLOCK
    ep2p->work = BL_ProcSet(pClub->event,HEAPID_NETWORK,ep2p->vchat);
#endif
    ep2p->seq++;
    break;

  case P2P_BALLOON_END:
    if( !GAMESYSTEM_EVENT_IsExists(pClub->gsys) ){    //サブプロセス終了待ち
      //        if( !FieldEvent_Cmd_WaitSubProcEnd(fsys) ){   //サブプロセス終了待ち
      ep2p->seq = P2P_MATCH_BOARD;
#if _BLOCK
      BL_ProcEnd( ep2p->work );
#endif
    }
    break;

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
  NET_PRINT("%x %x\n",(int)ep2p->pMatchParam->pMatch,(int)pClub->ctrl);
  ep2p->pWifiList = SaveData_GetWifiListData(pClub->ctrl); //クラブに必要な物を移し変え
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


#if 0
//------------------------------------------------------------------
/*
   @title WIFIクラブイベントを開始する
   @param GAMESYS_WORK
   @param kind
 */
//------------------------------------------------------------------
GMEVENT * EVENT_CreateWifiClub(GAMESYS_WORK *gsys, int kind)
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldOpenEvent, sizeof(EV_P2PEVENT_WORK));
  EV_P2PEVENT_WORK * ep2p = GMEVENT_GetEventWork(event);
  SAVE_CONTROL_WORK *saveWork = SaveControl_GetPointer();

  ep2p->pMatchParam = GFL_HEAP_AllocMemoryLo(HEAPID_BASE_APP, sizeof(WIFIP2PMATCH_PROC_PARAM));
  MI_CpuClear8(ep2p->pMatchParam, sizeof(WIFIP2PMATCH_PROC_PARAM));

  ep2p->pWifiList = SaveData_GetWifiListData(saveWork); //クラブに必要な物を移し変え
  ep2p->pMatchParam->seq = kind;
  return event;
}
#endif



#if _BLOCK
//----------------------------------------------------------------------------
/**
 *  @brief  ４にん待合室初期化
 *
 *  @param  p_wk    ワーク
 *  @param  fsys    フィールドシステム
 *  @param  heapID    ヒープ
 */
//-----------------------------------------------------------------------------
static void P2P_FourWaitInit( EV_P2PEVENT_WORK* p_wk, GMEVENT* fsys, u32 heapID, u32 type )
{
  WFP2PMF_INIT* p_work;

  p_work = GFL_HEAP_AllocMemory( heapID, sizeof(WFP2PMF_INIT) );

  p_work->type = type;
  p_work->comm_min = 2;
//  p_work->comm_max = sc_P2P_FOUR_MATCH_MAX[ type ];
  p_work->result = FALSE;
  p_work->vchat = FALSE;
  //  p_work->p_savedata = fsys->savedata;
  //    p_work->p_fnote = ;
  p_work->wintype = CONFIG_GetWindowType(SaveData_GetConfig(SaveControl_GetPointer()));

  p_wk->work = p_work;

  GFL_OVERLAY_Load(FS_OVERLAY_ID(wificlub));
  GFL_PROC_SysCallProc(FS_OVERLAY_ID(wifi2dmap), &WifiP2PMatchFourProcData, p_work);
//  EventCmd_CallSubProc(fsys, &WifiP2PMatchFourProcData, p_work);
}
//----------------------------------------------------------------------------
/**
 *  @brief  ４にん待合室募集終了
 *
 *  @param  p_wk  ワーク
 *
 *  @retval 次進んでほしいシーケンス
 */
//-----------------------------------------------------------------------------
static u32 P2P_FourWaitEnd( EV_P2PEVENT_WORK* p_wk )
{
  WFP2PMF_INIT* p_work = p_wk->work;

  if( p_work->result == TRUE ){
  switch( p_work->type ){
      // ポフィン
  case WFP2PMF_TYPE_POFIN:
    p_wk->seq = P2P_POFIN;
    break;
      // たまいれ
    case WFP2PMF_TYPE_BUCKET:
      p_wk->seq = P2P_BUCKET;
      break;
      // たまのり
    case WFP2PMF_TYPE_BALANCE_BALL:
      p_wk->seq = P2P_BALANCE_BALL;
      break;
      // ふうせんわり
    case WFP2PMF_TYPE_BALLOON:
      p_wk->seq = P2P_BALLOON;
      break;
    }
  }else{
    p_wk->seq = P2P_MATCH_BOARD;
  }
  p_wk->vchat = p_work->vchat;
  GFL_HEAP_FreeMemory( p_wk->work );

  return p_wk->seq;
}
#endif  //_BLOCK

#if 0 //まだ移植してないから封印

//----------------------------------------------------------------------------
/**
 *  @brief  バケットゲーム開始
 *
 *  @param  fsys    システムワーク
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static BUCKET_PROC_WORK* BCT_ProcSet( FIELDSYS_WORK* fsys, u32 heapID, u32 vchat )
{
  BUCKET_PROC_WORK* p_work;

  FS_EXTERN_OVERLAY(bucket);
  FS_EXTERN_OVERLAY(minigame_common);
  {
    // プロセス定義データ
    static const PROC_DATA BCTProcData = {
      BucketProc_Init,
      BucketProc_Main,
      BucketProc_End,
      FS_OVERLAY_ID(bucket),
    };
    p_work = sys_AllocMemory( heapID, sizeof(BUCKET_PROC_WORK) );
    memset( p_work, 0, sizeof(BUCKET_PROC_WORK) );
    p_work->vchat   = vchat;
    p_work->wifi_lobby  = FALSE;
    p_work->p_save    = SaveControl_GetPointer();

    // ミニゲーム共通オーバーレイを読み込む
    GFL_OVERLAY_Load( FS_OVERLAY_ID(minigame_common));

    EventCmd_CallSubProc(fsys, &BCTProcData, p_work);
  }
  return p_work;
}

//----------------------------------------------------------------------------
/**
 *  @brief  バケットゲーム終了
 *
 *  @param  p_wk
 */
//-----------------------------------------------------------------------------
static void BCT_ProcEnd( BUCKET_PROC_WORK* p_wk )
{
  FS_EXTERN_OVERLAY(minigame_common);

  sys_FreeMemoryEz( p_wk );

  Overlay_UnloadID( FS_OVERLAY_ID(minigame_common) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  バランスボールプロセス設定
 *
 *  @param  fsys    ワーク
 *  @param  heapID    ヒープ
 *  @param  vchat   VCHAT
 */
//-----------------------------------------------------------------------------
static BB_PROC_WORK* BB_ProcSet( FIELDSYS_WORK* fsys, u32 heapID, u32 vchat )
{
  BB_PROC_WORK* p_work;

  FS_EXTERN_OVERLAY(balance_ball);
  FS_EXTERN_OVERLAY(minigame_common);
  {
    // プロセス定義データ
    static const PROC_DATA BBProcData = {
      BalanceBallProc_Init,
      BalanceBallProc_Main,
      BalanceBallProc_Exit,
      FS_OVERLAY_ID(balance_ball),
    };
    p_work = sys_AllocMemory( heapID, sizeof(BB_PROC_WORK) );
    memset( p_work, 0, sizeof(BB_PROC_WORK) );
    p_work->vchat = vchat;
    p_work->wifi_lobby  = FALSE;
    p_work->p_save= fsys->savedata;

    // ミニゲーム共通オーバーレイを読み込む
    GFL_OVERLAY_Load( FS_OVERLAY_ID(minigame_common));

    EventCmd_CallSubProc(fsys, &BBProcData, p_work);
  }
  return p_work;
}

//----------------------------------------------------------------------------
/**
 *  @brief  バランスボールプロセス破棄
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void BB_ProcEnd( BB_PROC_WORK* p_wk )
{
  FS_EXTERN_OVERLAY(minigame_common);

  sys_FreeMemoryEz( p_wk );

  Overlay_UnloadID( FS_OVERLAY_ID(minigame_common) );
}
#endif

#if _BLOCK
//----------------------------------------------------------------------------
/**
 *  @brief  ふうせんわりプロセス設定
 *
 *  @param  fsys    ワーク
 *  @param  heapID    ヒープ
 *  @param  vchat   VCHAT
 */
//-----------------------------------------------------------------------------
static BALLOON_PROC_WORK* BL_ProcSet( GMEVENT* fsys, u32 heapID, u32 vchat )
{
#if 0
  BALLOON_PROC_WORK* p_work;

  FS_EXTERN_OVERLAY(balloon);
  {
    p_work = GFL_HEAP_AllocMemory( heapID, sizeof(BALLOON_PROC_WORK) );
    MI_CpuClear8(p_work, sizeof(BALLOON_PROC_WORK));
    p_work->vchat   = vchat;
    p_work->wifi_lobby  = FALSE;
    p_work->p_save    = SaveControl_GetPointer();
    p_work->game_comm = GAMESYSTEM_GetGameCommSysPtr(GMEVENT_GetGameSysWork(fsys));

    // ミニゲーム共通オーバーレイを読み込む
    //GFL_OVERLAY_Load( FS_OVERLAY_ID(minigame_common));

  GFL_PROC_SysCallProc(FS_OVERLAY_ID(balloon), &BalloonProcData, p_work);
  //    EventCmd_CallSubProc(fsys, &BCTProcData, p_work);
  }
  return p_work;
#else //Wi-Fi広場Fix
  return NULL;
#endif
}

//----------------------------------------------------------------------------
/**
 *  @brief  ふうせんわりプロセス破棄
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void BL_ProcEnd( BALLOON_PROC_WORK* p_wk )
{
  //FS_EXTERN_OVERLAY(minigame_common);

  GFL_HEAP_FreeMemory( p_wk );

  //GFL_OVERLAY_Unload( FS_OVERLAY_ID(minigame_common) );
}
#endif  //_BLOCK
#if 0
//----------------------------------------------------------------------------
/**
 *  @brief  ミニゲームで遊んだ  スコア計算
 *
 *  @param  fsys
 */
//-----------------------------------------------------------------------------
static void P2P_Record_Minigame( FIELDSYS_WORK* fsys )
{
  RECORD* p_rec;

  p_rec = SaveData_GetRecord( fsys->savedata );
  RECORD_Score_Add( p_rec, SCORE_ID_CLUB_MINIGAME );
}








#ifdef WFP2P_DEBUG
// オーバーレイID宣言
FS_EXTERN_OVERLAY(wifi_battleroom);
//----------------------------------------------------------------------------
/**
 *  @brief  バトルルームプロセス設定
 *
 *  @param  fsys    フィールドシステム
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static WIFI_BATTLEROOM* WBR_ProcSet( FIELDSYS_WORK* fsys, u32 heapID )
{
  WBR_INIT init;

  // オーバーレイロード
  GFL_OVERLAY_Load( FS_OVERLAY_ID(wifi_battleroom));

  init.p_fsys = fsys;

  return WBRSys_Init( &init, heapID );
}
static void WBR_ProcEnd( WIFI_BATTLEROOM* p_wk )
{
  WBRSys_Exit( p_wk );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(wifi_battleroom) );
}


#endif
#endif //0 //まだ移植してないから封印
