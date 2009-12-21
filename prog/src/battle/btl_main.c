//=============================================================================================
/**
 * @file    btl_main.c
 * @brief   ポケモンWB バトルシステムメインモジュール
 * @author  taya
 *
 * @date  2008.09.06  作成
 */
//=============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "tr_tool/trtype_def.h"
#include "tr_tool/tr_tool.h"
#include "gamesystem/btl_setup.h"
#include "battle/battle.h"

#include "btl_common.h"
#include "btl_server.h"
#include "btl_field.h"
#include "btl_client.h"
#include "btl_adapter.h"
#include "btl_pokeparam.h"
#include "btl_net.h"
#include "btl_util.h"

#include "btlv/btlv_core.h"
#include "btlv/btlv_mcss.h"

#include "btl_main.h"



/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  PARENT_HEAP_ID = GFL_HEAPID_APP,

  BTL_COMMITMENT_POKE_MAX = BTL_CLIENT_MAX * TEMOTI_POKEMAX,
};

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BOOL (*pMainLoop)(BTL_MAIN_MODULE*);
typedef BOOL (*pSetupSeq)(BTL_MAIN_MODULE*, int*);

/*--------------------------------------------------------------------------*/
/* Tables                                                                   */
/*--------------------------------------------------------------------------*/
static const u8 ClientBasePokeID[] = {
  0,                  // Client 0 のポケモンID 開始値
  TEMOTI_POKEMAX*2,   // Client 1 のポケモンID 開始値
  TEMOTI_POKEMAX*1,   // Client 2 のポケモンID 開始値
  TEMOTI_POKEMAX*3,   // Client 3 のポケモンID 開始値
};

/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/

struct _BTL_PARTY {
  BTL_POKEPARAM*  member[ TEMOTI_POKEMAX ];
  POKEPARTY*      srcParty;
  u8  memberCount;
  u8  numCoverPos;
};

struct _BTL_POKE_CONTAINER {
  BTL_MAIN_MODULE*  mainModule;
  BTL_PARTY         party[ BTL_CLIENT_MAX ];
  BTL_POKEPARAM*    pokeParam[ BTL_COMMITMENT_POKE_MAX ];
};

typedef struct {
  MYSTATUS*   playerStatus;
  u16         trainerType;
  u16         trainerID;
  STRBUF*     name;
}BTL_TRAINER_DATA;

struct _BTL_MAIN_MODULE {

  BATTLE_SETUP_PARAM* setupParam;

  BTLV_CORE*    viewCore;
  BTL_SERVER*   server;
  BTL_CLIENT*   client[ BTL_CLIENT_MAX ];
  BTL_TRAINER_DATA   trainerParam[ BTL_CLIENT_MAX ];
  const MYSTATUS*    playerStatus;

  // サーバが計算時に書き換えても良い一時使用パラメータ領域と、
  // サーバコマンドを受け取ったクライアントが実際に書き換えるパラメータ領域
  BTL_POKE_CONTAINER    pokeconForClient;
  BTL_POKE_CONTAINER    pokeconForServer;
  POKEPARTY*            srcParty[ BTL_CLIENT_MAX ];
  POKEPARTY*            tmpParty;

  GFL_STD_RandContext   randomContext;
  BTLNET_SERVER_NOTIFY_PARAM  serverNotifyParam;

  u8        posCoverClientID[ BTL_POS_MAX ];
  u32       bonusMoney;


  BTL_PROC    subProc;
  int         subSeq;
  pMainLoop   mainLoop;

  HEAPID      heapID;
  u8          escapeClientID;
  u8          sendClientID;

  u8        numClients;
  u8        myClientID;
  u8        myOrgPos;
  u8        ImServer;
  u8        fWazaEffEnable;
  u8        changeMode;

};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT BTL_PROC_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT BTL_PROC_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT BTL_PROC_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void setSubProcForSetup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param );
static void setSubProcForClanup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param );
static u8 checkBagMode( const BATTLE_SETUP_PARAM* setup );
static void setup_alone_common_ClientID_and_srcParty( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp );
static u8 CheckNumCoverPos( const BTL_MAIN_MODULE* wk, u8 clientID );
static BOOL setup_alone_single( int* seq, void* work );
static BOOL cleanup_common( int* seq, void* work );
static BOOL setup_alone_double( int* seq, void* work );
static BOOL setup_alone_triple( int* seq, void* work );
static BOOL setup_alone_rotation( int* seq, void* work );
static BOOL setup_comm_single( int* seq, void* work );
static BOOL setup_comm_double( int* seq, void* work );
static BOOL setup_comm_rotation( int* seq, void* work );
static BOOL setup_comm_triple( int* seq, void* work );
static BOOL setupseq_comm_determine_server( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_notify_party_data( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_notify_player_data( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_create_server_client_single( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_create_server_client_double( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_create_server_client_triple( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_start_server( BTL_MAIN_MODULE* wk, int* seq );
static BOOL MainLoop_StandAlone( BTL_MAIN_MODULE* wk );
static BOOL MainLoop_Comm_Server( BTL_MAIN_MODULE* wk );
static BOOL MainLoop_Comm_NotServer( BTL_MAIN_MODULE* wk );
static u8 expandPokePos_single( const BTL_MAIN_MODULE* wk, BtlExPos exType, u8 basePos, u8* dst );
static u8 expandPokePos_double( const BTL_MAIN_MODULE* wk, BtlExPos exType, u8 basePos, u8* dst );
static u8 expandPokePos_triple( const BTL_MAIN_MODULE* wk, BtlExPos exType, u8 basePos, u8* dst );
static inline clientID_to_side( u8 clientID );
static inline BtlPokePos getTripleFrontPos( BtlPokePos pos );
static inline u8 GetFriendCrientID( u8 clientID );
static inline u8 btlPos_to_clientID( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos );
static inline void btlPos_to_cliendID_and_posIdx( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos, u8* clientID, u8* posIdx );
static inline u8 btlPos_to_sidePosIdx( BtlPokePos pos );
static inline u8 PokeID_to_ClientID( u8 pokeID );
static void PokeCon_Init( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* mainModule );
static BOOL PokeCon_IsExistClient( const BTL_POKE_CONTAINER* wk, u32 clientID );
static void PokeCon_AddParty( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* wk, u8 clientID );
static void PokeCon_Release( BTL_POKE_CONTAINER* pokecon );
static int PokeCon_FindPokemon( const BTL_POKE_CONTAINER* pokecon, u8 clientID, u8 pokeID );
static void BTL_PARTY_Initialize( BTL_PARTY* party );
static void BTL_PARTY_AddMember( BTL_PARTY* party, BTL_POKEPARAM* member );
static void BTL_PARTY_SetSrcParty( BTL_PARTY* party, POKEPARTY* srcParty );
static void trainerParam_Init( BTL_MAIN_MODULE* wk );
static void trainerParam_Clear( BTL_MAIN_MODULE* wk );
static void trainerParam_StorePlayer( BTL_TRAINER_DATA* dst, HEAPID heapID, const MYSTATUS* playerData );
static void trainerParam_StoreNPCTrainer( BTL_TRAINER_DATA* dst, const BSP_TRAINER_DATA* trData );
static void srcParty_Init( BTL_MAIN_MODULE* wk );
static void srcParty_Quit( BTL_MAIN_MODULE* wk );
static void srcParty_Set( BTL_MAIN_MODULE* wk, u8 clientID, const POKEPARTY* party );
static POKEPARTY* srcParty_Get( BTL_MAIN_MODULE* wk, u8 clientID );
static void srcParty_RefrectBtlParty( BTL_MAIN_MODULE* wk, u8 clientID );
static void srcParty_RefrectBtlPartyStartOrder( BTL_MAIN_MODULE* wk, u8 clientID );
static void reflectPartyData( BTL_MAIN_MODULE* wk );
static void checkWinner( BTL_MAIN_MODULE* wk );
static void Bspstore_Party( BTL_MAIN_MODULE* wk, u8 clientID, const POKEPARTY* party );
static void Bspstore_PlayerStatus( BTL_MAIN_MODULE* wk, u8 clientID, const MYSTATUS* playerStatus );
static void Bspstore_RecordData( BTL_MAIN_MODULE* wk );
static u8 CommClientRelation( u8 myClientID, u8 targetClientID );


//--------------------------------------------------------------
/**
 *  Proc Data
 */
//--------------------------------------------------------------
const GFL_PROC_DATA BtlProcData = {
  BTL_PROC_Init,
  BTL_PROC_Main,
  BTL_PROC_Quit,
};



static GFL_PROC_RESULT BTL_PROC_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  switch( *seq ){
  case 0:
    {
      BTL_MAIN_MODULE* wk;
      BATTLE_SETUP_PARAM* setup_param = pwk;

      GFL_HEAP_CreateHeap( PARENT_HEAP_ID, HEAPID_BTL_SYSTEM, 0x28000 );
      GFL_HEAP_CreateHeap( PARENT_HEAP_ID, HEAPID_BTL_NET,     0x8000 );
      GFL_HEAP_CreateHeap( PARENT_HEAP_ID, HEAPID_BTL_VIEW,   0xa0000 );

      wk = GFL_PROC_AllocWork( proc, sizeof(BTL_MAIN_MODULE), HEAPID_BTL_SYSTEM );
      GFL_STD_MemClear32( wk, sizeof(BTL_MAIN_MODULE) );

      wk->heapID = HEAPID_BTL_SYSTEM;
      wk->setupParam = setup_param;
      wk->setupParam->capturedPokeIdx = TEMOTI_POKEMAX;
      wk->playerStatus = wk->setupParam->playerStatus[ BTL_CLIENT_PLAYER ];

      if( !(wk->setupParam->fRecordPlay) ){
        GFL_STD_RandGeneralInit( &wk->randomContext );
      }else{
        wk->randomContext = setup_param->recRandContext;
      }
      BTL_CALC_InitRandSys( &wk->randomContext );

      wk->escapeClientID = BTL_CLIENTID_NULL;
      wk->fWazaEffEnable = (CONFIG_GetWazaEffectMode(setup_param->configData) == WAZAEFF_MODE_ON);
      wk->changeMode = (CONFIG_GetBattleRule(setup_param->configData) == BATTLERULE_IREKAE)?
          BTL_CHANGEMODE_IREKAE : BTL_CHANGEMODE_KATINUKI;

      BTL_NET_InitSystem( setup_param->netHandle, HEAPID_BTL_NET );
      BTL_CALC_ITEM_InitSystem( HEAPID_BTL_SYSTEM );
      (*seq)++;
    }
    break;

  case 1:
    if( BTL_NET_IsInitialized() )
    {
      BTL_MAIN_MODULE* wk = mywk;
      BTL_ADAPTERSYS_Init( wk->setupParam->commMode );
      BTL_FIELD_Init( BTL_WEATHER_NONE );
      setSubProcForSetup( &wk->subProc, wk, wk->setupParam );
      wk->subSeq = 0;
      trainerParam_Init( wk );
      srcParty_Init( wk );
      (*seq)++;
    }
    break;

   case 2:
   {
      BTL_MAIN_MODULE* wk = mywk;
      if( BTL_UTIL_CallProc(&wk->subProc) )
      {
        BTL_Printf("セットアップ終了\n");
        return GFL_PROC_RES_FINISH;
      }
   }
   break;

  }

  return GFL_PROC_RES_CONTINUE;
}

static GFL_PROC_RESULT BTL_PROC_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  BTL_MAIN_MODULE* wk = mywk;

  if( wk->mainLoop( wk ) )
  {
    BTL_Printf("バトルメインプロセス終了します\n");
    checkWinner( wk );
    Bspstore_RecordData( wk );
    reflectPartyData( wk );
    return GFL_PROC_RES_FINISH;
  }


  return GFL_PROC_RES_CONTINUE;

}

static GFL_PROC_RESULT BTL_PROC_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  BTL_MAIN_MODULE* wk = mywk;

  switch( *seq ){
  case 0:
    wk->subSeq = 0;
    (*seq)++;
    /* fallthru */
  case 1:
    if( wk->subSeq > -16 ){
      wk->subSeq--;
      GX_SetMasterBrightness( wk->subSeq );
      GXS_SetMasterBrightness( wk->subSeq );
    }else{
      (*seq)++;
    }
    break;
  case 2:
    BTL_Printf("クリーンアッププロセス１\n");
    srcParty_Quit( wk );
    trainerParam_Clear( wk );
    setSubProcForClanup( &wk->subProc, wk, wk->setupParam );
    (*seq)++;
    break;
  case 3:
    if( BTL_UTIL_CallProc(&wk->subProc) )
    {
      BTL_Printf("クリーンアッププロセス２－１\n");
      BTL_ADAPTERSYS_Quit();
      BTL_Printf("クリーンアッププロセス２－２\n");
      BTL_NET_QuitSystem();
      BTL_Printf("クリーンアッププロセス２－３\n");
      BTL_CALC_ITEM_QuitSystem();
      (*seq)++;
    }
    break;
  case 4:
    BTL_Printf("クリーンアッププロセス３\n");
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_BTL_VIEW );
    GFL_HEAP_DeleteHeap( HEAPID_BTL_NET );
    GFL_HEAP_DeleteHeap( HEAPID_BTL_SYSTEM );
    BTL_Printf("クリーンアッププロセス終了\n");
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

//======================================================================================================
//======================================================================================================

// 各種モジュール生成＆関連付けルーチンを設定
static void setSubProcForSetup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param )
{
  int i;

  // 位置担当クライアントIDを無効値で初期化しておく
  for(i=0; i<NELEMS(wk->posCoverClientID); ++i)
  {
    wk->posCoverClientID[i] = BTL_CLIENT_MAX;
  }

  // @@@ 本来は setup_param を参照して各種初期化処理ルーチンを決定する
  if( setup_param->commMode == BTL_COMM_NONE )
  {
    BTL_UTIL_SetPrintType( BTL_PRINTTYPE_STANDALONE );

    switch( setup_param->rule ){
    case BTL_RULE_SINGLE:
      BTL_UTIL_SetupProc( bp, wk, setup_alone_single, NULL );
      break;
    case BTL_RULE_DOUBLE:
      BTL_UTIL_SetupProc( bp, wk, setup_alone_double, NULL );
      break;
    case BTL_RULE_TRIPLE:
      BTL_UTIL_SetupProc( bp, wk, setup_alone_triple, NULL );
      break;
    case BTL_RULE_ROTATION:
      BTL_UTIL_SetupProc( bp, wk, setup_alone_rotation, NULL );
      break;
    default:
      GF_ASSERT(0);
      BTL_UTIL_SetupProc( bp, wk, setup_alone_single, NULL );
      break;
    }
  }
  else
  {
    switch( setup_param->rule ){
    case BTL_RULE_SINGLE:
      BTL_UTIL_SetupProc( bp, wk, setup_comm_single, NULL );
      break;
    case BTL_RULE_DOUBLE:
      BTL_UTIL_SetupProc( bp, wk, setup_comm_double, NULL );
      break;
    case BTL_RULE_TRIPLE:
      BTL_UTIL_SetupProc( bp, wk, setup_comm_triple, NULL );
      break;
    case BTL_RULE_ROTATION:
      BTL_UTIL_SetupProc( bp, wk, setup_comm_rotation, NULL );
      break;
    default:
      GF_ASSERT(0);
      BTL_UTIL_SetupProc( bp, wk, setup_alone_single, NULL );
      break;
    }
  }
}

// 各種モジュール解放＆削除処理ルーチンを設定
static void setSubProcForClanup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param )
{
  BTL_UTIL_SetupProc( bp, wk, cleanup_common, NULL );
}

/**
 *  セットアップパラメータからバッグモードを決定
 */
static u8 checkBagMode( const BATTLE_SETUP_PARAM* setup )
{
  switch( setup->competitor ){
  case BTL_COMPETITOR_WILD:
    return BBAG_MODE_NORMAL;

  case BTL_COMPETITOR_TRAINER:
    return BBAG_MODE_NORMAL;

  case BTL_COMPETITOR_SUBWAY:
    return BBAG_MODE_SHOOTER;

  case BTL_COMPETITOR_COMM:
    return BBAG_MODE_SHOOTER;
  }

  return BBAG_MODE_NORMAL;
}

/**
 *  スタンドアロンモードでプレイヤーのクライアントID、基本立ち位置
 *  各クライアントのパーティデータを生成（録画再生に対応）
 */
static void setup_alone_common_ClientID_and_srcParty( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp )
{
  wk->myClientID = sp->commPos;
  wk->myOrgPos = BTL_MAIN_GetClientPokePos( wk, wk->myClientID, 0 );
  {
    u8 relation_0 = CommClientRelation( wk->myClientID, 0 );
    u8 relation_1 = CommClientRelation( wk->myClientID, 1 );

    // バトル用ポケモンパラメータ＆パーティデータを生成
    srcParty_Set( wk, 0, sp->party[ relation_0 ] );
    srcParty_Set( wk, 1, sp->party[ relation_1 ] );
  }
  BTL_Printf("デバッグフラグbit=%04x\n", sp->DebugFlagBit);
}

/**
 *  クライアントが管理する位置数を取得
 */
static u8 CheckNumCoverPos( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  switch( wk->setupParam->rule ){
  case BTL_RULE_SINGLE:
    return 1;
  case BTL_RULE_ROTATION:
    return 2;
  case BTL_RULE_TRIPLE:
    return 3;
  case BTL_RULE_DOUBLE:
    if( wk->setupParam->multiMode == FALSE ){
      return 2;
    }else{
      if( wk->setupParam->competitor == BTL_COMPETITOR_COMM ){
        return 1;
      }
      else
      {
        // マルチで非通信時、相方のパーティデータがあるなら自分の管理領域は１
        u8 friendClientID = GetFriendCrientID( clientID );
        if(  (wk->setupParam->party[friendClientID] != NULL)
        &&   (PokeParty_GetPokeCount( wk->setupParam->party[friendClientID] ) > 0)
        ){
          return 1;
        }
        return 2;
      }
    }
  }
  GF_ASSERT(0);
  return 1;
}

//--------------------------------------------------------------------------
/**
 * スタンドアローン／シングルバトル
 */
//--------------------------------------------------------------------------
static BOOL setup_alone_single( int* seq, void* work )
{
  // server*1, client*2
  BTL_MAIN_MODULE* wk = work;
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  u8 bagMode = checkBagMode( sp );

  wk->numClients = 2;
  wk->posCoverClientID[BTL_POS_1ST_0] = 0;
  wk->posCoverClientID[BTL_POS_2ND_0] = 1;

  setup_alone_common_ClientID_and_srcParty( wk, sp );

  PokeCon_Init( &wk->pokeconForClient, wk );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 1 );

  PokeCon_Init( &wk->pokeconForServer, wk );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 1 );

  // Server 作成
  wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
  wk->ImServer = TRUE;

  trainerParam_StorePlayer( &wk->trainerParam[0], wk->heapID, wk->playerStatus );
  if( sp->fRecordPlay ){
    trainerParam_StorePlayer( &wk->trainerParam[1], wk->heapID, sp->playerStatus[BTL_CLIENT_ENEMY1] );
  }else{
    trainerParam_StoreNPCTrainer( &wk->trainerParam[1], sp->tr_data[BTL_CLIENT_ENEMY1] );
  }

  // Client 作成
  wk->client[0] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 0, 1,
    (sp->fRecordPlay)? BTL_CLIENT_TYPE_REC : BTL_CLIENT_TYPE_UI, bagMode, wk->heapID );

  wk->client[1] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 1, 1,
    (sp->fRecordPlay)? BTL_CLIENT_TYPE_REC : BTL_CLIENT_TYPE_AI, bagMode, wk->heapID );

  if( sp->fRecordPlay ){
    BTL_CLIENT_SetRecordPlayType( wk->client[0], sp->recBuffer, sp->recDataSize );
    BTL_CLIENT_SetRecordPlayType( wk->client[1], sp->recBuffer, sp->recDataSize );
  }

  // 描画エンジン生成、プレイヤークライアントに関連付ける
  {
    BTL_CLIENT* uiClient = (sp->fRecordPlay)? wk->client[sp->commPos] : wk->client[0];
    BTL_Printf("viewClient = %d\n", sp->commPos);
    wk->viewCore = BTLV_Create( wk, uiClient, &wk->pokeconForClient, HEAPID_BTL_VIEW );
    BTL_CLIENT_AttachViewCore( uiClient, wk->viewCore );
  }

  // Server に Client を接続
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[0]), 0, 1 );
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[1]), 1, 1 );

  // Server 始動
  BTL_SERVER_Startup( wk->server );

  wk->mainLoop = MainLoop_StandAlone;

  return TRUE;
}
//----------------------------------------------------------------------------------
/**
 * 上記セットアップ処理で生成したモジュールを全て解放する共通クリーンアップ処理
 *
 * @param   seq
 * @param   work
 *
 * @retval  BOOL    TRUEで終了
 */
//----------------------------------------------------------------------------------
static BOOL cleanup_common( int* seq, void* work )
{
  BTL_MAIN_MODULE* wk = work;
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  int i;

  BTL_Printf("クリーンアップ 1-1\n");

  BTLV_Delete( wk->viewCore );

  BTL_Printf("クリーンアップ 1-2\n");

  for(i=0; i<NELEMS(wk->client); ++i)
  {
    if( wk->client[i] )
    {
      BTL_CLIENT_Delete( wk->client[i] );
    }
  }

  BTL_Printf("クリーンアップ 1-3\n");
  PokeCon_Release( &wk->pokeconForClient );
  PokeCon_Release( &wk->pokeconForServer );

  if( wk->server )
  {
    BTL_SERVER_Delete( wk->server );
  }

  BTL_Printf("クリーンアップ 1-4\n");

  return TRUE;
}
//--------------------------------------------------------------------------
/**
 * スタンドアローン／ダブルバトル：セットアップ
 */
//--------------------------------------------------------------------------
static BOOL setup_alone_double( int* seq, void* work )
{
  // server*1, client*2
  BTL_MAIN_MODULE* wk = work;
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  u8 bagMode = checkBagMode( sp );

  wk->myClientID = 0;
  wk->numClients = 2;
  wk->posCoverClientID[BTL_POS_1ST_0] = 0;
  wk->posCoverClientID[BTL_POS_2ND_0] = 1;
  wk->posCoverClientID[BTL_POS_1ST_1] = 0;
  wk->posCoverClientID[BTL_POS_2ND_1] = 1;

  setup_alone_common_ClientID_and_srcParty( wk, sp );

  PokeCon_Init( &wk->pokeconForClient, wk );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 1 );

  PokeCon_Init( &wk->pokeconForServer, wk );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 1 );

  // Server 作成
  wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

  trainerParam_StorePlayer( &wk->trainerParam[0], wk->heapID, wk->playerStatus );
  trainerParam_StoreNPCTrainer( &wk->trainerParam[1], sp->tr_data[BTL_CLIENT_ENEMY1] );

  // Client 作成
  wk->client[0] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 0, 2,
    (sp->fRecordPlay)? BTL_CLIENT_TYPE_REC : BTL_CLIENT_TYPE_UI, bagMode, wk->heapID );

  wk->client[1] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 1, 2,
    (sp->fRecordPlay)? BTL_CLIENT_TYPE_REC : BTL_CLIENT_TYPE_AI, bagMode, wk->heapID );

  if( sp->fRecordPlay ){
    BTL_Printf("再生タイプ初期化 ... dataSize=%d\n", sp->recDataSize);
    BTL_CLIENT_SetRecordPlayType( wk->client[0], sp->recBuffer, sp->recDataSize );
    BTL_CLIENT_SetRecordPlayType( wk->client[1], sp->recBuffer, sp->recDataSize );
  }


  // 描画エンジン生成
  wk->viewCore = BTLV_Create( wk, wk->client[0], &wk->pokeconForClient, HEAPID_BTL_VIEW );

  // プレイヤークライアントに描画エンジンを関連付ける
  BTL_CLIENT_AttachViewCore( wk->client[0], wk->viewCore );

  // Server に Client を接続
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[0]), 0, 2 );
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[1]), 1, 2 );

  // Server 始動
  BTL_SERVER_Startup( wk->server );

  wk->mainLoop = MainLoop_StandAlone;

  return TRUE;
}
//--------------------------------------------------------------------------
/**
 * スタンドアローン／トリプルバトル：セットアップ
 */
//--------------------------------------------------------------------------
static BOOL setup_alone_triple( int* seq, void* work )
{
  // server*1, client*2
  BTL_MAIN_MODULE* wk = work;
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  u8 bagMode = checkBagMode( sp );

  wk->numClients = 2;
  wk->ImServer = TRUE;
  wk->posCoverClientID[BTL_POS_1ST_0] = 0;
  wk->posCoverClientID[BTL_POS_2ND_0] = 1;
  wk->posCoverClientID[BTL_POS_1ST_1] = 0;
  wk->posCoverClientID[BTL_POS_2ND_1] = 1;
  wk->posCoverClientID[BTL_POS_1ST_2] = 0;
  wk->posCoverClientID[BTL_POS_2ND_2] = 1;

  setup_alone_common_ClientID_and_srcParty( wk, sp );

  PokeCon_Init( &wk->pokeconForClient, wk );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 1 );

  PokeCon_Init( &wk->pokeconForServer, wk );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 1 );

  // Server 作成
  wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

  trainerParam_StorePlayer( &wk->trainerParam[0], wk->heapID, wk->playerStatus );
  trainerParam_StoreNPCTrainer( &wk->trainerParam[1], sp->tr_data[BTL_CLIENT_ENEMY1] );

  // Client 作成
  wk->client[0] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 0, 3,
    (sp->fRecordPlay)? BTL_CLIENT_TYPE_REC : BTL_CLIENT_TYPE_UI, bagMode, wk->heapID );

  wk->client[1] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 1, 3,
    (sp->fRecordPlay)? BTL_CLIENT_TYPE_REC : BTL_CLIENT_TYPE_AI, bagMode, wk->heapID );

  if( sp->fRecordPlay ){
    BTL_CLIENT_SetRecordPlayType( wk->client[0], sp->recBuffer, sp->recDataSize );
    BTL_CLIENT_SetRecordPlayType( wk->client[1], sp->recBuffer, sp->recDataSize );
  }


  // 描画エンジン生成
  wk->viewCore = BTLV_Create( wk, wk->client[0], &wk->pokeconForClient, HEAPID_BTL_VIEW );

  // プレイヤークライアントに描画エンジンを関連付ける
  BTL_CLIENT_AttachViewCore( wk->client[0], wk->viewCore );

  // Server に Client を接続
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[0]), 0, 3 );
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[1]), 1, 3 );

  // Server 始動
  BTL_SERVER_Startup( wk->server );

  wk->mainLoop = MainLoop_StandAlone;

  return TRUE;
}
//--------------------------------------------------------------------------
/**
 * スタンドアローン／ローテーションバトル：セットアップ
 */
//--------------------------------------------------------------------------
static BOOL setup_alone_rotation( int* seq, void* work )
{
  // server*1, client*2
  BTL_MAIN_MODULE* wk = work;
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  u8 bagMode = checkBagMode( sp );

  wk->numClients = 2;
  wk->posCoverClientID[BTL_POS_1ST_0] = 0;
  wk->posCoverClientID[BTL_POS_2ND_0] = 1;
  wk->posCoverClientID[BTL_POS_1ST_1] = 0;
  wk->posCoverClientID[BTL_POS_2ND_1] = 1;
//  wk->posCoverClientID[BTL_POS_1ST_2] = 0;
//  wk->posCoverClientID[BTL_POS_2ND_2] = 1;

  setup_alone_common_ClientID_and_srcParty( wk, sp );


  PokeCon_Init( &wk->pokeconForClient, wk );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 1 );

  PokeCon_Init( &wk->pokeconForServer, wk );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 1 );

  // Server 作成
  wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

  trainerParam_StorePlayer( &wk->trainerParam[0], wk->heapID, wk->playerStatus );
  trainerParam_StoreNPCTrainer( &wk->trainerParam[1], sp->tr_data[BTL_CLIENT_ENEMY1] );

  // Client 作成
  wk->client[0] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 0, 2,
    BTL_CLIENT_TYPE_UI, bagMode, wk->heapID );
  wk->client[1] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 1, 2,
    BTL_CLIENT_TYPE_AI, bagMode, wk->heapID );

  // 描画エンジン生成
  wk->viewCore = BTLV_Create( wk, wk->client[0], &wk->pokeconForClient, HEAPID_BTL_VIEW );

  // プレイヤークライアントに描画エンジンを関連付ける
  BTL_CLIENT_AttachViewCore( wk->client[0], wk->viewCore );

  // Server に Client を接続
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[0]), 0, 2 );
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[1]), 1, 2 );

  // Server 始動
  BTL_SERVER_Startup( wk->server );

  wk->mainLoop = MainLoop_StandAlone;

  return TRUE;
}

//--------------------------------------------------------------------------
/**
 * 通信／シングルバトル
 */
//--------------------------------------------------------------------------
static BOOL setup_comm_single( int* seq, void* work )
{
  static const pSetupSeq funcs[] = {
    NULL,
    setupseq_comm_determine_server,
    setupseq_comm_notify_party_data,
    setupseq_comm_notify_player_data,
    setupseq_comm_create_server_client_single,
    setupseq_comm_start_server,
  };

  BTL_MAIN_MODULE* wk = work;
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;

  if( (*seq) == 0 )
  {
    wk->numClients = 2;
    wk->posCoverClientID[BTL_POS_1ST_0] = 0;
    wk->posCoverClientID[BTL_POS_2ND_0] = 1;
    wk->myClientID = wk->setupParam->commPos;
    wk->myOrgPos = BTL_MAIN_GetClientPokePos( wk, wk->myClientID, 0 );
    (*seq)++;
    return FALSE;
  }
  else if( (*seq) < NELEMS(funcs) ){
    if( funcs[ *seq ]( wk, &wk->subSeq ) ){
      wk->subSeq = 0;
      ++(*seq);
    }
    return FALSE;
  }

  return TRUE;
}
//--------------------------------------------------------------------------
/**
 * 通信／ダブルバトル：セットアップ
 */
//--------------------------------------------------------------------------
static BOOL setup_comm_double( int* seq, void* work )
{
  static const pSetupSeq funcs[] = {
    NULL,
    setupseq_comm_determine_server,
    setupseq_comm_notify_party_data,
    setupseq_comm_notify_player_data,
    setupseq_comm_create_server_client_double,
    setupseq_comm_start_server,
  };

  BTL_MAIN_MODULE* wk = work;

  if( (*seq) == 0 )
  {
    const BATTLE_SETUP_PARAM* sp = wk->setupParam;
    if( sp->multiMode == 0 )
    {
      wk->numClients = 2;
      wk->posCoverClientID[BTL_POS_1ST_0] = 0;
      wk->posCoverClientID[BTL_POS_2ND_0] = 1;
      wk->posCoverClientID[BTL_POS_1ST_1] = 0;
      wk->posCoverClientID[BTL_POS_2ND_1] = 1;
    }
    else
    {
      wk->numClients = 4;
      wk->posCoverClientID[BTL_POS_1ST_0] = 0;
      wk->posCoverClientID[BTL_POS_2ND_0] = 1;
      wk->posCoverClientID[BTL_POS_1ST_1] = 2;
      wk->posCoverClientID[BTL_POS_2ND_1] = 3;
    }
    wk->myClientID = wk->setupParam->commPos;
    wk->myOrgPos = BTL_MAIN_GetClientPokePos( wk, wk->myClientID, 0 );
    (*seq)++;
    return FALSE;
  }
  else if( (*seq) < NELEMS(funcs) ){
    if( funcs[ *seq ]( wk, &wk->subSeq ) ){
      wk->subSeq = 0;
      ++(*seq);
    }
    return FALSE;
  }
  return TRUE;
}
//--------------------------------------------------------------------------
/**
 * 通信／ローテーション：セットアップ
 */
//--------------------------------------------------------------------------
static BOOL setup_comm_rotation( int* seq, void* work )
{
  static const pSetupSeq funcs[] = {
    NULL,
    setupseq_comm_determine_server,
    setupseq_comm_notify_party_data,
    setupseq_comm_notify_player_data,
    setupseq_comm_create_server_client_double,
    setupseq_comm_start_server,
  };

  BTL_MAIN_MODULE* wk = work;

  if( (*seq) == 0 )
  {
    wk->numClients = 2;
    wk->posCoverClientID[BTL_POS_1ST_0] = 0;
    wk->posCoverClientID[BTL_POS_2ND_0] = 1;
    wk->posCoverClientID[BTL_POS_1ST_1] = 0;
    wk->posCoverClientID[BTL_POS_2ND_1] = 1;
    wk->myClientID = wk->setupParam->commPos;
    wk->myOrgPos = BTL_MAIN_GetClientPokePos( wk, wk->myClientID, 0 );
    (*seq)++;
    return FALSE;
  }
  else if( (*seq) < NELEMS(funcs) ){
    if( funcs[ *seq ]( wk, &wk->subSeq ) ){
      wk->subSeq = 0;
      ++(*seq);
    }
    return FALSE;
  }
  return TRUE;
}
//--------------------------------------------------------------------------
/**
 * 通信／トリプルバトル：セットアップ
 */
//--------------------------------------------------------------------------
static BOOL setup_comm_triple( int* seq, void* work )
{
  static const pSetupSeq funcs[] = {
    NULL,
    setupseq_comm_determine_server,
    setupseq_comm_notify_party_data,
    setupseq_comm_notify_player_data,
    setupseq_comm_create_server_client_triple,
    setupseq_comm_start_server,
  };

  BTL_MAIN_MODULE* wk = work;
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;

  if( (*seq) == 0 )
  {
    wk->numClients = 2;
    wk->posCoverClientID[BTL_POS_1ST_0] = 0;
    wk->posCoverClientID[BTL_POS_2ND_0] = 1;
    wk->posCoverClientID[BTL_POS_1ST_1] = 0;
    wk->posCoverClientID[BTL_POS_2ND_1] = 1;
    wk->posCoverClientID[BTL_POS_1ST_2] = 0;
    wk->posCoverClientID[BTL_POS_2ND_2] = 1;
    wk->myClientID = wk->setupParam->commPos;
    wk->myOrgPos = BTL_MAIN_GetClientPokePos( wk, wk->myClientID, 0 );
    (*seq)++;
    return FALSE;
  }
  else if( (*seq) < NELEMS(funcs) ){
    if( funcs[ *seq ]( wk, &wk->subSeq ) ){
      wk->subSeq = 0;
      ++(*seq);
    }
    return FALSE;
  }

  return TRUE;
}


//----------------------------------------------------------------------------------
/**
 * 通信対戦セットアップシーケンス：サーバ＆クライアントIDの確定
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL    終了時にTRUEを返す
 */
//----------------------------------------------------------------------------------
static BOOL setupseq_comm_determine_server( BTL_MAIN_MODULE* wk, int* seq )
{
  switch( *seq ){
  case 0:
    if( BTL_NET_IsServerDetermained() )
    {
      wk->sendClientID = 0;
      wk->ImServer = BTL_NET_IsServer();

      if( BTL_NET_IsServer() )
      {
        BTL_UTIL_SetPrintType( BTL_PRINTTYPE_SERVER );
        (*seq) = 1;
      }
      else
      {
        BTL_UTIL_SetPrintType( BTL_PRINTTYPE_CLIENT );
        (*seq) = 2;
      }
    }
    break;

  case 1:
    // サーバマシンは各クライアントにデバッグパラメータを通知する
    wk->serverNotifyParam.randomContext = wk->randomContext;
    wk->serverNotifyParam.debugFlagBit = wk->setupParam->DebugFlagBit;
    if( BTL_NET_NotifyServerParam(&wk->serverNotifyParam) ){
      ++(*seq);
    }
    break;

  case 2:
    // デバッグパラメータ受信完了
    if( BTL_NET_IsServerParamReceived(&wk->serverNotifyParam) )
    {
      BATTLE_SETUP_PARAM* sp = wk->setupParam;
      sp->DebugFlagBit = wk->serverNotifyParam.debugFlagBit;
      sp->recRandContext = wk->serverNotifyParam.randomContext;
      wk->randomContext  = wk->serverNotifyParam.randomContext;
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 通信対戦セットアップシーケンス：各クライアントのパーティデータを相互に送受信
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL    終了時にTRUEを返す
 */
//----------------------------------------------------------------------------------
static BOOL setupseq_comm_notify_party_data( BTL_MAIN_MODULE* wk, int* seq )
{
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;

  switch( *seq ){
  case 0:
    BTL_NET_TimingSyncStart( BTL_NET_TIMING_NOTIFY_PARTY_DATA );
    (*seq)++;
    break;
  case 1:
    if( BTL_NET_IsTimingSync(BTL_NET_TIMING_NOTIFY_PARTY_DATA) ){
      (*seq)++;
    }
    break;
  case 2:
    if( BTL_NET_StartNotifyPartyData(sp->party[ BTL_CLIENT_PLAYER ]) ){
      (*seq)++;
    }
    break;
  case 3:
    // パーティデータ相互受信を完了
    if( BTL_NET_IsCompleteNotifyPartyData() )
    {
      u32 i;

      BTL_Printf("パーティデータ相互受信できました。\n");

      PokeCon_Init( &wk->pokeconForClient, wk );

      // @@@ 例えばnumCliens=3だったら、ClientID 割り振りは0～2という想定でいいのか？たぶんダメ。
      //     ... でも通信対戦で numClients==3 ということは現状、無い。あるとしたらスタンドアロン。
      for(i=0; i<wk->numClients; ++i)
      {
        srcParty_Set( wk, i, BTL_NET_GetPartyData(i) );
        Bspstore_Party( wk, i, BTL_NET_GetPartyData(i) );
        PokeCon_AddParty( &wk->pokeconForClient, wk, i );
      }
      (*seq)++;
    }
    break;
  case 4:
    if( wk->ImServer )
    {
      u32 i;
      PokeCon_Init( &wk->pokeconForServer, wk );
      // @@@ 例えばnumCliens=3だったら、ClientID 割り振りは0～2という想定でいいのか？たぶんダメ。
      //     ... でも通信対戦で numClients==3 ということは現状、無い。あるとしたらスタンドアロン。
      for(i=0; i<wk->numClients; ++i){
        PokeCon_AddParty( &wk->pokeconForServer, wk, i );
      }
    }
    (*seq)++;
    break;
  case 5:
    BTL_NET_EndNotifyPartyData();
    (*seq)++;
    break;
  default:
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 通信対戦セットアップシーケンス：各クライアントのプレイヤーデータを相互に送受信
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL    終了時にTRUEを返す
 */
//----------------------------------------------------------------------------------
static BOOL setupseq_comm_notify_player_data( BTL_MAIN_MODULE* wk, int* seq )
{
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;

  switch( *seq ){
  case 0:
    BTL_NET_TimingSyncStart( BTL_NET_TIMING_NOTIFY_PLAYER_DATA );
    (*seq)++;
    break;
  case 1:
    if( BTL_NET_IsTimingSync(BTL_NET_TIMING_NOTIFY_PLAYER_DATA) ){
      (*seq)++;
    }
    break;
  case 2:
    if( BTL_NET_StartNotifyPlayerData(wk->playerStatus) ){
      (*seq)++;
    }
    break;
  case 3:
    if( BTL_NET_IsCompleteNotifyPlayerData() )
    {
      const MYSTATUS* playerStatus;
      u32 i;
      for(i=0; i<wk->numClients; ++i){
        playerStatus = BTL_NET_GetPlayerData( i );
        trainerParam_StorePlayer( &wk->trainerParam[i], wk->heapID, playerStatus );
        Bspstore_PlayerStatus( wk, i, playerStatus );
      }
      BTL_Printf("プレイヤーデータ相互受信できました。\n");
      (*seq)++;
    }
    break;
  case 4:
    BTL_NET_EndNotifyPlayerData();
    (*seq)++;
    break;
  default:
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 通信対戦セットアップシーケンス：サーバ・クライアントモジュール構築（シングル用）
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL    終了時にTRUEを返す
 */
//----------------------------------------------------------------------------------
static BOOL setupseq_comm_create_server_client_single( BTL_MAIN_MODULE* wk, int* seq )
{
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  u8 netID = sp->commPos;
  u8 bagMode = checkBagMode( sp );

  // 自分がサーバ
  if( wk->ImServer )
  {
    wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

    wk->client[netID] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        netID, 1, BTL_CLIENT_TYPE_UI, bagMode, wk->heapID );
    BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[netID]), netID, 1 );
    BTL_SERVER_ReceptionNetClient( wk->server, sp->commMode, sp->netHandle, !netID, 1 );
  }
  // 自分がサーバではない
  else
  {
    wk->client[ netID ] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle, netID, 1,
    BTL_CLIENT_TYPE_UI, bagMode, wk->heapID  );
  }

  return TRUE;
}
//----------------------------------------------------------------------------------
/**
 * 通信対戦セットアップシーケンス：サーバ・クライアントモジュール構築（ダブル用）
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL    終了時にTRUEを返す
 */
//----------------------------------------------------------------------------------
static BOOL setupseq_comm_create_server_client_double( BTL_MAIN_MODULE* wk, int* seq )
{
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  u8 netID = GFL_NET_GetNetID( sp->netHandle );
  u8 bagMode = checkBagMode( sp );

  // 自分がサーバ
  if( wk->ImServer )
  {
    u8 numCoverPos, i;

    numCoverPos = (sp->multiMode==0)? 2 : 1;

    wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

    BTL_Printf("myClientID=%d\n", netID);
    wk->client[netID] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        netID, numCoverPos, BTL_CLIENT_TYPE_UI, bagMode, wk->heapID );
    BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[netID]), netID, numCoverPos );

    for(i=0; i<wk->numClients; ++i)
    {
      if(i==netID){ continue; }
      BTL_SERVER_ReceptionNetClient( wk->server, sp->commMode, sp->netHandle, i, numCoverPos );
    }
  }
  // 自分がサーバではない
  else
  {
    u8 netID, numCoverPos;

    netID = GFL_NET_GetNetID( sp->netHandle );
    numCoverPos = (sp->multiMode==0)? 2 : 1;

    wk->client[ netID ] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        netID, numCoverPos, BTL_CLIENT_TYPE_UI, bagMode, wk->heapID  );

  }
  return TRUE;
}
//----------------------------------------------------------------------------------
/**
 * 通信対戦セットアップシーケンス：サーバ・クライアントモジュール構築（トリプル用）
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL    終了時にTRUEを返す
 */
//----------------------------------------------------------------------------------
static BOOL setupseq_comm_create_server_client_triple( BTL_MAIN_MODULE* wk, int* seq )
{
  enum {
    NUM_COVERPOS = 3,
  };

  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  u8 netID = GFL_NET_GetNetID( sp->netHandle );
  u8 bagMode = checkBagMode( sp );

  // 自分がサーバ
  if( wk->ImServer )
  {
    wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

    wk->client[netID] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        netID, NUM_COVERPOS, BTL_CLIENT_TYPE_UI, bagMode, wk->heapID );
    BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[netID]), netID, NUM_COVERPOS );
    BTL_SERVER_ReceptionNetClient( wk->server, sp->commMode, sp->netHandle, !netID, NUM_COVERPOS );
  }
  // 自分がサーバではない
  else
  {
    wk->client[ netID ] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle, netID, NUM_COVERPOS,
    BTL_CLIENT_TYPE_UI, bagMode, wk->heapID  );
  }

  return TRUE;
}
//----------------------------------------------------------------------------------
/**
 * 通信対戦セットアップシーケンス：描画エンジン生成、メインループ始動
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL setupseq_comm_start_server( BTL_MAIN_MODULE* wk, int* seq )
{
  switch( *seq ){
  case 0:
    {
      const BATTLE_SETUP_PARAM* sp = wk->setupParam;
      u32 netID = GFL_NET_GetNetID( sp->netHandle );

      // 描画エンジン生成
      wk->viewCore = BTLV_Create( wk, wk->client[netID], &wk->pokeconForClient, HEAPID_BTL_VIEW );
      BTL_CLIENT_AttachViewCore( wk->client[netID], wk->viewCore );

      if( wk->ImServer ){
        // Server 始動
        wk->mainLoop = MainLoop_Comm_Server;
        BTL_SERVER_Startup( wk->server );
      }else{
        wk->mainLoop = MainLoop_Comm_NotServer;
      }
      (*seq)++;
    }
    break;
  default:
    return TRUE;
  }
  return FALSE;
}

//======================================================================================================
//======================================================================================================


static BOOL MainLoop_StandAlone( BTL_MAIN_MODULE* wk )
{
  BOOL quitFlag = FALSE;
  int i;

  BTL_SERVER_Main( wk->server );

  for(i=0; i<2; i++)
  {
    if( BTL_CLIENT_Main(wk->client[i]) )
    {
      wk->escapeClientID = BTL_CLIENT_GetEscapeClientID( wk->client[i] );
      quitFlag = TRUE;
    }
  }

  BTLV_CORE_Main( wk->viewCore );

  return quitFlag;
}


static BOOL MainLoop_Comm_Server( BTL_MAIN_MODULE* wk )
{
  BOOL quitFlag = FALSE;
  int i;

  BTL_SERVER_Main( wk->server );

  for(i=0; i<wk->numClients; i++)
  {
    if( wk->client[i] )
    {
      if( BTL_CLIENT_Main(wk->client[i]) )
      {
        wk->escapeClientID = BTL_CLIENT_GetEscapeClientID( wk->client[i] );
        quitFlag = TRUE;
      }
    }
  }

  BTLV_CORE_Main( wk->viewCore );

  return quitFlag;
}

static BOOL MainLoop_Comm_NotServer( BTL_MAIN_MODULE* wk )
{
  BOOL quitFlag = FALSE;
  int i;

  for(i=0; i<wk->numClients; i++)
  {
    if( wk->client[i] )
    {
      if( BTL_CLIENT_Main(wk->client[i]) )
      {
        wk->escapeClientID = BTL_CLIENT_GetEscapeClientID( wk->client[i] );
        quitFlag = TRUE;
      }
    }
  }

  BTLV_CORE_Main( wk->viewCore );

  return quitFlag;
}




//======================================================================================================
// クライアント等からの問い合わせ対応
//======================================================================================================

//=============================================================================================
/**
 * 戦闘のルール指定子を返す
 *
 * @param   wk
 *
 * @retval  BtlRule
 */
//=============================================================================================
BtlRule BTL_MAIN_GetRule( const BTL_MAIN_MODULE* wk )
{
  return wk->setupParam->rule;
}
//=============================================================================================
/**
 * ワザエフェクト表示が有効かチェック
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsWazaEffectEnable( const BTL_MAIN_MODULE* wk )
{
  return wk->fWazaEffEnable;
}
//=============================================================================================
/**
 * 経験値取得シーケンスが有効な対戦かどうか判定
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsExpSeqEnable( const BTL_MAIN_MODULE* wk )
{
  if( (wk->setupParam->competitor == BTL_COMPETITOR_WILD)
  ||  (wk->setupParam->competitor == BTL_COMPETITOR_TRAINER)
  ){
    return TRUE;
  }
  return FALSE;
}
//=============================================================================================
/**
 * 通信プレイヤーサポートハンドラを返す
 *
 * @param   wk
 *
 * @retval  COMM_PLAYER_SUPPORT*
 */
//=============================================================================================
COMM_PLAYER_SUPPORT* BTL_MAIN_GetCommSupportHandle( const BTL_MAIN_MODULE* wk )
{
  if( (wk->setupParam->competitor == BTL_COMPETITOR_WILD)
  ||  (wk->setupParam->competitor == BTL_COMPETITOR_TRAINER)
  ){
    return wk->setupParam->commSupport;
  }
  return NULL;
}

//=============================================================================================
/**
 * 現在のルールに応じて、有効になる位置指定子の最大値を返す
 *
 * @param   wk
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_MAIN_GetEnablePosEnd( const BTL_MAIN_MODULE* wk )
{
  switch( wk->setupParam->rule ){
  case BTL_RULE_SINGLE:
    return BTL_POS_2ND_0;

  case BTL_RULE_DOUBLE:
    return BTL_POS_2ND_1;

  case BTL_RULE_TRIPLE:
    return BTL_POS_2ND_2;

  case BTL_RULE_ROTATION:
    return BTL_POS_2ND_1;

  default:
    GF_ASSERT(0);
    return BTL_POS_2ND_2;
  }
}

//=============================================================================================
/**
 * 対戦相手タイプを返す
 *
 * @param   wk
 *
 * @retval  BtlCompetitor
 */
//=============================================================================================
BtlCompetitor BTL_MAIN_GetCompetitor( const BTL_MAIN_MODULE* wk )
{
  return wk->setupParam->competitor;
}
//=============================================================================================
/**
 * 通信タイプを返す
 *
 * @param   wk
 *
 * @retval  BtlCommMode
 */
//=============================================================================================
BtlCommMode BTL_MAIN_GetCommMode( const BTL_MAIN_MODULE* wk )
{
  return wk->setupParam->commMode;
}
//=============================================================================================
/**
 * マルチモードかどうかを返す
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsMultiMode( const BTL_MAIN_MODULE * wk )
{
  return wk->setupParam->multiMode;
}
//=============================================================================================
/**
 * 「にげる」を選択した時の反応タイプ
 *
 * @param   wk
 *
 * @retval  BtlEscapeMode   反応タイプ
 */
//=============================================================================================
BtlEscapeMode BTL_MAIN_GetEscapeMode( const BTL_MAIN_MODULE * wk )
{
  // @@@ いまのところ常に許可する
  return BTL_ESCAPE_MODE_OK;
}
//=============================================================================================
/**
 * フィールドシチュエーションデータポインタを返す
 *
 * @param   wk
 *
 * @retval  const BTL_FIELD_SITUATION*
 */
//=============================================================================================
const BTL_FIELD_SITUATION* BTL_MAIN_GetFieldSituation( const BTL_MAIN_MODULE* wk )
{
  return &wk->setupParam->fieldSituation;
}

//=============================================================================================
/**
 * プレイヤー情報（セーブデータ）ハンドラを返す
 *
 * @param   wk
 *
 * @retval  const MYSTATUS*
 */
//=============================================================================================
const MYSTATUS* BTL_MAIN_GetPlayerStatus( const BTL_MAIN_MODULE * wk )
{
  return wk->playerStatus;
}
//=============================================================================================
/**
 * 図鑑に登録されているポケモンかどうか判定
 *
 * @param   wk
 * @param   bpp
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsZukanRegistered( const BTL_MAIN_MODULE* wk, const BTL_POKEPARAM* bpp )
{
  u16 monsno = BPP_GetMonsNo( bpp );
  return ZUKANSAVE_GetPokeGetFlag( wk->setupParam->zukanData, monsno );
}

//=============================================================================================
/**
 * 特殊ポケ位置指定子を、実際のポケ位置に変換
 *
 * @param   wk            メインモジュールハンドラ
 * @param   exPos         特殊ポケモン位置指定子
 * @param   dst           [out] 対象ポケ位置を格納するバッファ
 *
 * @retval  u8    対象クライアント数
 */
//=============================================================================================
u8 BTL_MAIN_ExpandBtlPos( const BTL_MAIN_MODULE* wk, BtlExPos exPos, u8* dst )
{
  u8 exType = EXPOS_GET_TYPE( exPos );
  u8 basePos = EXPOS_GET_BASEPOS( exPos );

  if( exType == BTL_EXPOS_DEFAULT )
  {
    dst[0] = basePos;
    return 1;
  }

  switch( wk->setupParam->rule ){
  case BTL_RULE_SINGLE:
  default:
    return expandPokePos_single( wk, exType, basePos, dst );
  case BTL_RULE_DOUBLE:
    return expandPokePos_double( wk, exType, basePos, dst );
  case BTL_RULE_TRIPLE:
    return expandPokePos_triple( wk, exType, basePos, dst );
  case BTL_RULE_ROTATION:
    return expandPokePos_double( wk, exType, basePos, dst );
  }
}
// シングル用
static u8 expandPokePos_single( const BTL_MAIN_MODULE* wk, BtlExPos exType, u8 basePos, u8* dst )
{
  switch( exType ){
  default:
    return 0;
    /* fallthru */
  case BTL_EXPOS_AREA_ENEMY:
  case BTL_EXPOS_AREA_OTHERS:
  case BTL_EXPOS_FULL_ENEMY:
    dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 0 );
    break;
  case BTL_EXPOS_AREA_MYTEAM:
    dst[0] = basePos;
    break;
  case BTL_EXPOS_AREA_ALL:
  case BTL_EXPOS_FULL_ALL:
    dst[0] = BTL_POS_1ST_0;
    dst[1] = BTL_POS_2ND_0;
    return 2;
  }
  return 1;
}
// ダブル用
static u8 expandPokePos_double( const BTL_MAIN_MODULE* wk, BtlExPos exType, u8 basePos, u8* dst )
{
  switch( exType ){
  default:
    GF_ASSERT(0);
    return 0;
    /* fallthru */
  case BTL_EXPOS_AREA_ENEMY:
  case BTL_EXPOS_FULL_ENEMY:
    dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 0 );
    dst[1] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
    return 2;
  case BTL_EXPOS_AREA_OTHERS:
    dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 0 );
    dst[1] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
    dst[2] = BTL_MAIN_GetNextPokePos( wk, basePos );
    return 3;
  case BTL_EXPOS_AREA_MYTEAM:
    dst[0] = basePos;
    dst[1] = BTL_MAIN_GetNextPokePos( wk, basePos );
    if( dst[0] > dst[1] )
    {
      u8 tmp = dst[0];
      dst[0] = dst[1];
      dst[1] = tmp;
    }
    return 2;
  case BTL_EXPOS_AREA_FRIENDS:
    dst[0] = BTL_MAIN_GetNextPokePos( wk, basePos );
    return 1;
  case BTL_EXPOS_AREA_ALL:
  case BTL_EXPOS_FULL_ALL:
    dst[0] = BTL_POS_1ST_0;
    dst[1] = BTL_POS_2ND_0;
    dst[2] = BTL_POS_1ST_1;
    dst[3] = BTL_POS_2ND_1;
    return 4;
  }
}
// トリプル用
static u8 expandPokePos_triple( const BTL_MAIN_MODULE* wk, BtlExPos exType, u8 basePos, u8* dst )
{
  u8 fCenter = BTL_MAINUTIL_IsTripleCenterPos( basePos );
  u8 posIdx = btlPos_to_sidePosIdx( basePos );


  switch( exType ){
  default:
    GF_ASSERT(0);
    /* fallthru */
  case BTL_EXPOS_AREA_ENEMY:
    if( fCenter )
    {
      dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 0 );
      dst[1] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
      dst[2] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 2 );
      return 3;
    }
    else
    {
      dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
      dst[1] = getTripleFrontPos( basePos );
      return 2;
    }

  case BTL_EXPOS_FULL_ENEMY:
    dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 0 );
    dst[1] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
    dst[2] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 2 );
    return 3;

  case BTL_EXPOS_FULL_ALL:
    dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 0 );
    dst[1] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
    dst[2] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 2 );
    dst[3] = BTL_MAINUTIL_GetFriendPokePos( basePos, 0 );
    dst[4] = BTL_MAINUTIL_GetFriendPokePos( basePos, 1 );
    dst[5] = BTL_MAINUTIL_GetFriendPokePos( basePos, 1 );
    return 3;


  case BTL_EXPOS_AREA_OTHERS:
    if( fCenter )
    {
      dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 0 );
      dst[1] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
      dst[2] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 2 );
      dst[3] = BTL_MAINUTIL_GetFriendPokePos( basePos, 0 );
      dst[4] = BTL_MAINUTIL_GetFriendPokePos( basePos, 2 );
      return 5;
    }
    else
    {
      dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
      dst[1] = getTripleFrontPos( basePos );
      dst[2] = BTL_MAINUTIL_GetFriendPokePos( basePos, 1 );
      return 3;
    }

  case BTL_EXPOS_AREA_MYTEAM:
    if( fCenter )
    {
      dst[0] = basePos;
      dst[1] = BTL_MAINUTIL_GetFriendPokePos( basePos, 0 );
      dst[2] = BTL_MAINUTIL_GetFriendPokePos( basePos, 2 );
      return 3;
    }
    else
    {
      dst[0] = basePos;
      dst[1] = BTL_MAINUTIL_GetFriendPokePos( basePos, 1 );
      return 2;
    }
    break;

  case BTL_EXPOS_AREA_FRIENDS:
    if( fCenter )
    {
      dst[0] = BTL_MAINUTIL_GetFriendPokePos( basePos, 0 );
      dst[1] = BTL_MAINUTIL_GetFriendPokePos( basePos, 2 );
      return 2;
    }
    else
    {
      dst[0] = BTL_MAINUTIL_GetFriendPokePos( basePos, 1 );
      return 1;
    }
    break;

  case BTL_EXPOS_AREA_ALL:
    if( fCenter )
    {
      dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 0 );
      dst[1] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
      dst[2] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 2 );
      dst[3] = BTL_MAINUTIL_GetFriendPokePos( basePos, 0 );
      dst[4] = BTL_MAINUTIL_GetFriendPokePos( basePos, 2 );
      dst[5] = basePos;
      return 6;
    }
    else
    {
      dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
      dst[1] = getTripleFrontPos( basePos );
      dst[2] = BTL_MAINUTIL_GetFriendPokePos( basePos, 1 );
      dst[3] = basePos;
      return 4;
    }


  }
}
//
// クライアントID -> サイドIDに変換
//
static inline clientID_to_side( u8 clientID )
{
  return clientID & 1;
}
//
// トリプルバトル時、目の前の相手位置を返す
//
static inline BtlPokePos getTripleFrontPos( BtlPokePos pos )
{
  u8 idx = btlPos_to_sidePosIdx( pos );
  if( idx == 0 ){
    idx = 2;
  }else if(idx == 2){
    idx = 0;
  }
  return BTL_MAINUTIL_GetOpponentPokePos( BTL_RULE_TRIPLE, pos, idx );
}

//=============================================================================================
/**
 * クライアントIDからサイドIDを返す
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  BtlSide
 */
//=============================================================================================
BtlSide BTL_MAIN_GetClientSide( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  return clientID_to_side( clientID );
}
//=============================================================================================
/**
 * プレイヤーサイドかどうか判定
 *
 * @param   wk
 * @param   side
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsPlayerSide( const BTL_MAIN_MODULE* wk, BtlSide side )
{
  return ( BTL_MAIN_GetClientSide(wk, wk->myClientID) == side );
}


//=============================================================================================
/**
 * クライアントIDからポケモン戦闘位置を返す
 *
 * @param   wk
 * @param   basePos
 * @param   idx
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_MAIN_GetClientPokePos( const BTL_MAIN_MODULE* wk, u8 clientID, u8 posIdx )
{
  u8 i;

  for(i=0; i<NELEMS(wk->posCoverClientID); ++i)
  {
    if( wk->posCoverClientID[i] == clientID )
    {
      if( posIdx == 0 )
      {
        return i;
      }
      --posIdx;
    }
  }
//  GF_ASSERT(0);
  return BTL_POS_MAX;
}
//=============================================================================================
/**
 * 対戦相手側のポケモン位置IDを返す
 *
 * @param   rule
 * @param   basePos
 * @param   idx
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_MAINUTIL_GetOpponentPokePos( BtlRule rule, BtlPokePos basePos, u8 idx )
{
  switch( rule ){
  case BTL_RULE_SINGLE:
    GF_ASSERT(idx<1);
    break;
  case BTL_RULE_DOUBLE:
    GF_ASSERT(idx<2)
    break;
  case BTL_RULE_TRIPLE:
    GF_ASSERT(idx<3);
    break;
  case BTL_RULE_ROTATION:
    GF_ASSERT(idx<2);
    break;
  default:
    GF_ASSERT(0);
    break;
  }
  if( (basePos&1) == BTL_POS_1ST_0 )
  {
    return BTL_POS_2ND_0 + (idx * 2);
  }
  else
  {
    return BTL_POS_1ST_0 + (idx * 2);
  }
}
//=============================================================================================
/**
 * 味方側のポケモン位置IDを返す
 *
 * @param   basePos
 * @param   idx
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_MAINUTIL_GetFriendPokePos( BtlPokePos basePos, u8 idx )
{
  if( (basePos&1) == BTL_POS_1ST_0 )
  {
    return BTL_POS_1ST_0 + (idx * 2);
  }
  else
  {
    return BTL_POS_2ND_0 + (idx * 2);
  }
}
//=============================================================================================
/**
 * 対戦相手方のポケモン戦闘位置を返す
 *
 * @param   wk
 * @param   basePos
 * @param   idx
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_MAIN_GetOpponentPokePos( const BTL_MAIN_MODULE* wk, BtlPokePos basePos, u8 idx )
{
  return BTL_MAINUTIL_GetOpponentPokePos( wk->setupParam->rule, basePos, idx );
}
//=============================================================================================
/**
 * ある位置から見て、目の前（対戦側）にあたる戦闘位置を返す
 *
 * @param   rule
 * @param   pos
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_MAINUTIL_GetFacedPokePos( BtlRule rule, BtlPokePos pos )
{
  switch( rule ){
  case BTL_RULE_SINGLE:
  default:
    return BTL_MAINUTIL_GetOpponentPokePos( rule, pos, 0 );

  case BTL_RULE_DOUBLE:
  case BTL_RULE_ROTATION:
    {
      u8 idx = btlPos_to_sidePosIdx( pos );
      idx ^= 1;
      return BTL_MAINUTIL_GetOpponentPokePos( rule, pos, idx );
    }

  case BTL_RULE_TRIPLE:
    {
      u8 idx = btlPos_to_sidePosIdx( pos );
      if( idx == 0 ){
        idx = 2;
      }else if(idx == 2){
        idx = 0;
      }
      return BTL_MAINUTIL_GetOpponentPokePos( rule, pos, idx );
    }
  }
}
//=============================================================================================
/**
 * ある位置から見て、目の前（対戦側）にあたる戦闘位置を返す
 *
 * @param   wk
 * @param   pos
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_MAIN_GetFacedPokePos( const BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
  return BTL_MAINUTIL_GetFacedPokePos( wk->setupParam->rule, pos );
}
//=============================================================================================
/**
 * 指定位置から見て隣りの戦闘位置を返す
 *
 * @param   wk
 * @param   basePos
 * @param   idx
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_MAIN_GetNextPokePos( const BTL_MAIN_MODULE* wk, BtlPokePos basePos )
{
  switch( wk->setupParam->rule ){
  case BTL_RULE_SINGLE:
    GF_ASSERT(0);
    return basePos;
  case BTL_RULE_DOUBLE:
  case BTL_RULE_ROTATION:
    {
      u8 retPos = (basePos + 2) & 0x03;
      return retPos;
    }
  case BTL_RULE_TRIPLE:
    GF_ASSERT(0);
    return basePos;
  default:
    GF_ASSERT(0);
    return basePos;
  }
}
//=============================================================================================
/**
 * ２つのクライアントIDが対戦相手同士のものかどうかを判別
 *
 * @param   wk
 * @param   clientID1
 * @param   clientID2
 *
 * @retval  BOOL    対戦相手同士ならTRUE
 */
//=============================================================================================
BOOL BTL_MAIN_IsOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID1, u8 clientID2 )
{
  return (clientID1&1) != (clientID2&1);
}

/**
 *  マルチモード時の味方クライアントIDに変換
 */
static inline u8 GetFriendCrientID( u8 clientID )
{
  return (clientID + 2) & 3;
}

//--------------------------------------------------------------------------
/**
 * 戦闘位置->クライアントIDに 変換
 *
 * @param   wk
 * @param   btlPos
 *
 * @retval  inline u8
 */
//--------------------------------------------------------------------------
static inline u8 btlPos_to_clientID( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos )
{
  GF_ASSERT_MSG(btlPos < NELEMS(wk->posCoverClientID), "pos=%d\n", btlPos);
  GF_ASSERT_MSG(wk->posCoverClientID[btlPos] != BTL_CLIENT_MAX, "pos=%d\n", btlPos);

  return wk->posCoverClientID[btlPos];
}
//--------------------------------------------------------------------------
/**
 * 戦闘位置->クライアントID，位置インデクス（クライアントごと0オリジン，連番）に変換
 *
 * @param   wk        [in] メインモジュールのハンドラ
 * @param   btlPos    [in] 戦闘位置
 * @param   clientID  [out]
 * @param   posIdx    [out]
 *
 */
//--------------------------------------------------------------------------
static inline void btlPos_to_cliendID_and_posIdx( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos, u8* clientID, u8* posIdx )
{
  *clientID = btlPos_to_clientID( wk, btlPos );

  {
    u8 idx = 0;
    while( btlPos-- )
    {
      if( wk->posCoverClientID[ btlPos ] == *clientID  )
      {
        ++idx;
      }
    }

    *posIdx = idx;
  }
}
//
// 戦闘位置 -> 自サイド中の位置インデックスへ変換
//
static inline u8 btlPos_to_sidePosIdx( BtlPokePos pos )
{
  return pos / 2;
}
// ポケモンID -> クライアントID変換
static inline u8 PokeID_to_ClientID( u8 pokeID )
{
  u8 i, min, max;
  for(i=0; i<NELEMS(ClientBasePokeID); ++i)
  {
    min = ClientBasePokeID[i];
    max = min + TEMOTI_POKEMAX - 1;
    if( (pokeID >= min) && (pokeID <= max) )
    {
      return i;
    }
  }
  GF_ASSERT_MSG(0, "Illegal PokeID[%d]", pokeID);
  return 0;
}



//=============================================================================================
/**
 * バトルポケモンIDをポケモン戦闘位置に変換
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BtlPokePos    ポケモン戦闘位置（戦闘に出ていなければBTL_POS_MAX）
 */
//=============================================================================================
BtlPokePos BTL_MAIN_PokeIDtoPokePos( const BTL_MAIN_MODULE* wk, const BTL_POKE_CONTAINER* pokeCon, u8 pokeID )
{
  u8 clientID = PokeID_to_ClientID( pokeID );

  int idx = PokeCon_FindPokemon( pokeCon, clientID, pokeID );
  if( idx >= 0 )
  {
    BtlPokePos pos = BTL_MAIN_GetClientPokePos( wk, clientID, idx );
    if( pos != BTL_POS_MAX )
    {
      return pos;
    }
    GF_ASSERT_MSG(0, " not fighting pokemon client=%d, idx=%d, pokeID [%d]", clientID, idx, pokeID );
  }
  GF_ASSERT_MSG(0, " not including pokeID [%d] (clientID=%d)", pokeID, clientID );
  return BTL_POS_MAX;
}
//=============================================================================================
/**
 * バトルポケモンIDから、そのポケモンを管理するクライアントIDを取得
 *
 * @param   pokeID
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_MAINUTIL_PokeIDtoClientID( u8 pokeID )
{
  return PokeID_to_ClientID( pokeID );
}

//=============================================================================================
/**
 * 戦闘位置->クライアントID変換
 *
 * @param   wk
 * @param   pos
 *
 */
//=============================================================================================
u8 BTL_MAIN_BtlPosToClientID( const BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
  return btlPos_to_clientID( wk, pos );
}
//=============================================================================================
/**
 * 戦闘位置->位置インデックス変換
 *
 * @param   wk
 * @param   pos
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_MAIN_BtlPosToPosIdx( const BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
  u8 clientID, posIdx;
  btlPos_to_cliendID_and_posIdx( wk, pos, &clientID, &posIdx );
  return posIdx;
}
//=============================================================================================
/**
 * 戦闘位置 -> クライアントID＆位置インデックス変換
 *
 * @param   wk
 * @param   pos
 * @param   clientID
 * @param   posIdx
 *
 */
//=============================================================================================
void BTL_MAIN_BtlPosToClientID_and_PosIdx( const BTL_MAIN_MODULE* wk, BtlPokePos pos, u8* clientID, u8* posIdx )
{
  btlPos_to_cliendID_and_posIdx( wk, pos, clientID, posIdx );
}



//=============================================================================================
/**
 * 戦闘位置 -> 描画位置変換
 *
 * @param   wk
 * @param   pos
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_MAIN_BtlPosToViewPos( const BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
  u8 isPlayerSide;

  isPlayerSide = ( (wk->myOrgPos&1) == (pos&1) );

  if( wk->setupParam->rule == BTL_RULE_SINGLE )
  {
    return (isPlayerSide)? BTLV_MCSS_POS_AA : BTLV_MCSS_POS_BB;
  }
  else if( wk->setupParam->rule == BTL_RULE_DOUBLE )
  {
    static const u8 vpos[2][BTL_POSIDX_MAX] = {
      { BTLV_MCSS_POS_B, BTLV_MCSS_POS_D  },
      { BTLV_MCSS_POS_A, BTLV_MCSS_POS_C  },
    };
    u8 posIdx = btlPos_to_sidePosIdx( pos );
    return vpos[ isPlayerSide ][ posIdx ];
  }
  else
  {
    static const u8 vpos[2][BTL_POSIDX_MAX] = {
      { BTLV_MCSS_POS_B, BTLV_MCSS_POS_D, BTLV_MCSS_POS_F  },
      { BTLV_MCSS_POS_A, BTLV_MCSS_POS_C, BTLV_MCSS_POS_E  },
    };
    u8 posIdx = btlPos_to_sidePosIdx( pos );
    return vpos[ isPlayerSide ][ posIdx ];
  }
}

//=============================================================================================
/**
 * 描画位置 -> 戦闘位置変換
 *
 * @param   wk
 * @param   vpos
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_MAIN_ViewPosToBtlPos( const BTL_MAIN_MODULE* wk, u8 vpos )
{
  u8 isFarSide = ( vpos & 1 );

  if( wk->setupParam->rule == BTL_RULE_SINGLE )
  {
    if( isFarSide ){
      return !(wk->myOrgPos);
    }else{
      return wk->myOrgPos;
    }
  }
  else
  {
    #if 0
      BTL_POS_1ST_0=0,    ///< スタンドアローンならプレイヤー側、通信対戦ならサーバ側
      BTL_POS_2ND_0,
      BTL_POS_1ST_1,
      BTL_POS_2ND_1,
      BTL_POS_1ST_2,
      BTL_POS_2ND_2,

      BTLV_MCSS_POS_A=2,
      BTLV_MCSS_POS_B,
      BTLV_MCSS_POS_C,
      BTLV_MCSS_POS_D,
      BTLV_MCSS_POS_E,
      BTLV_MCSS_POS_F,
    #endif

    u8 org_pos = (wk->myOrgPos & 1);
    u8 result;

    if( isFarSide )
    {
      result =  (org_pos ^ 1) + ((vpos - BTLV_MCSS_POS_A ) - 1);
    }
    else
    {
      result =  org_pos + (vpos - BTLV_MCSS_POS_A);
    }

    BTL_Printf("myOrgPos=%d, vpos=%d, resultPos=%d\n", wk->myOrgPos, vpos, result);

    return result;
  }
}

//=============================================================================================
/**
 *
 *
 * @param   pokeID
 *
 * @retval  BtlSide
 */
//=============================================================================================
BtlSide BTL_MAINUTIL_PokeIDtoSide( u8 pokeID )
{
  return (pokeID < TEMOTI_POKEMAX*2)? BTL_SIDE_1ST : BTL_SIDE_2ND;
}
//=============================================================================================
/**
 * 味方同士のポケモンIDかどうかを判定する
 *
 * @param   pokeID1
 * @param   pokeID2
 *
 * @retval  BOOL    味方同士ならTRUE
 */
//=============================================================================================
BOOL BTL_MAINUTIL_IsFriendPokeID( u8 pokeID1, u8 pokeID2 )
{
  BtlSide side1 = BTL_MAINUTIL_PokeIDtoSide( pokeID1 );
  BtlSide side2 = BTL_MAINUTIL_PokeIDtoSide( pokeID2 );
  return side1 == side2;
}

//=======================================================================================================
// BTL_POKE_CONTAINER
//=======================================================================================================

static void PokeCon_Init( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* mainModule )
{
  int i;

  pokecon->mainModule = mainModule;

  for(i=0; i<NELEMS(pokecon->party); ++i){
    BTL_PARTY_Initialize( &pokecon->party[i] );
  }
  for(i=0; i<NELEMS(pokecon->pokeParam); ++i){
    pokecon->pokeParam[i] = NULL;
  }
}
static BOOL PokeCon_IsExistClient( const BTL_POKE_CONTAINER* wk, u32 clientID )
{
  GF_ASSERT(clientID < BTL_CLIENT_MAX);

  return (wk->party[ clientID ].memberCount != 0);
}

static void PokeCon_AddParty( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* wk, u8 clientID )
{
  const POKEPARTY* party_src = srcParty_Get( wk, clientID );
  BTL_PARTY* party = &pokecon->party[ clientID ];
  u32 poke_count = PokeParty_GetPokeCount( party_src );
  u8 pokeID = ClientBasePokeID[ clientID ];
  u8  fIllusion = FALSE;

  POKEMON_PARAM* pp;
  u8 i;

  BTL_Printf(" CreateClient(%d)'s party ... numMembers=%d\n", clientID, poke_count);
  for(i=0; i<poke_count; ++i, ++pokeID)
  {
    pp = PokeParty_GetMemberPointer( party_src, i );
    pokecon->pokeParam[ pokeID ] = BTL_POKEPARAM_Create( pp, pokeID, HEAPID_BTL_SYSTEM );

    // １個前のポケがイリュージョン使いなら、自分のSrcPPデータを見せかけ用データとしてセットする
    if( fIllusion ){
      BPP_SetViewSrcData( pokecon->pokeParam[ pokeID-1 ], pp );
    }
    fIllusion = (BPP_GetValue( pokecon->pokeParam[pokeID], BPP_TOKUSEI) == POKETOKUSEI_IRYUUJON);

    BTL_Printf(" Create Client(%d)'s PokeParam ID=%d, adrs=%p\n", clientID, pokeID, pokecon->pokeParam[i]);
    BTL_PARTY_AddMember( party, pokecon->pokeParam[ pokeID ] );
  }

  // 最後の１体がイリュージョン使いなら、先頭のSrcPPを見せかけデータにする
  if( fIllusion && (poke_count>1)){
    pp = PokeParty_GetMemberPointer( party_src, 0 );
    BPP_SetViewSrcData( pokecon->pokeParam[ pokeID-1 ], pp );
  }
}
static void PokeCon_Release( BTL_POKE_CONTAINER* pokecon )
{
  u32 i;
  for(i=0; i<NELEMS(pokecon->pokeParam); ++i)
  {
    if( pokecon->pokeParam[i] )
    {
      BTL_Printf(" Delete PokeParam ID=%d, adrs=%p\n", i, pokecon->pokeParam[i]);
      BTL_POKEPARAM_Delete( pokecon->pokeParam[i] );
      pokecon->pokeParam[i] = NULL;
    }
  }
}
static int PokeCon_FindPokemon( const BTL_POKE_CONTAINER* pokecon, u8 clientID, u8 pokeID )
{
  const BTL_POKEPARAM* bpp;
  const BTL_PARTY* party = &pokecon->party[ clientID ];
  u32 i, max = BTL_PARTY_GetMemberCount( party );

  for(i=0; i<max; ++i)
  {
    bpp = BTL_PARTY_GetMemberDataConst( party, i );
    if( BPP_GetID(bpp) == pokeID )
    {
      return i;
    }
  }
  return -1;
}

//=============================================================================================
/**
 * 戦闘位置からポケモンデータの取得
 *
 * @param   wk    メインモジュールのハンドラ
 * @param   pos   立ち位置
 *
 * @retval  BTL_POKEPARAM*
 */
//=============================================================================================
BTL_POKEPARAM* BTL_POKECON_GetFrontPokeData( BTL_POKE_CONTAINER* wk, BtlPokePos pos )
{
  return (BTL_POKEPARAM*)BTL_POKECON_GetFrontPokeDataConst(wk, pos);
}
//=============================================================================================
/**
 * 戦闘位置からポケモンデータの取得（const）
 *
 * @param   wk    メインモジュールのハンドラ
 * @param   pos   立ち位置
 *
 * @retval  const BTL_POKEPARAM*
 */
//=============================================================================================
const BTL_POKEPARAM* BTL_POKECON_GetFrontPokeDataConst( const BTL_POKE_CONTAINER* wk, BtlPokePos pos )
{
  const BTL_PARTY* party;
  u8 clientID, posIdx;

  BTL_Printf("戦闘位置[%d] = ", pos);

  btlPos_to_cliendID_and_posIdx( wk->mainModule, pos, &clientID, &posIdx );
  party = &wk->party[ clientID ];

  BTL_PrintfSimple("クライアント[%d]の %d 番目のポケを返す\n", clientID, posIdx );

  return BTL_PARTY_GetMemberDataConst( party, posIdx );
}
//=============================================================================================
/**
 * クライアントIDからポケモンデータの取得
 *
 * @param   wk
 * @param   clientID
 * @param   posIdx
 *
 * @retval  const BTL_POKEPARAM*
 */
//=============================================================================================
BTL_POKEPARAM* BTL_POKECON_GetClientPokeData( BTL_POKE_CONTAINER* wk, u8 clientID, u8 posIdx )
{
  BTL_PARTY* party;
  party = &wk->party[ clientID ];
  return BTL_PARTY_GetMemberData( party, posIdx );
}
//=============================================================================================
/**
 * クライアントポケモンデータの取得 ( const )
 *
 * @param   wk
 * @param   clientID
 * @param   memberIdx
 *
 * @retval  const BTL_POKEPARAM*
 */
//=============================================================================================
const BTL_POKEPARAM* BTL_POKECON_GetClientPokeDataConst( const BTL_POKE_CONTAINER* wk, u8 clientID, u8 memberIdx )
{
  const BTL_PARTY* party;
  party = &wk->party[ clientID ];
  return BTL_PARTY_GetMemberDataConst( party, memberIdx );
}
BTL_POKEPARAM* BTL_POKECON_GetPokeParam( BTL_POKE_CONTAINER* wk, u8 pokeID )
{
  GF_ASSERT(pokeID<BTL_COMMITMENT_POKE_MAX);
  GF_ASSERT_MSG(wk->pokeParam[pokeID], "invalid pokeID(%d)", pokeID);
  return wk->pokeParam[ pokeID ];
}
const BTL_POKEPARAM* BTL_POKECON_GetPokeParamConst( const BTL_POKE_CONTAINER* wk, u8 pokeID )
{
  GF_ASSERT(pokeID<BTL_COMMITMENT_POKE_MAX);
  GF_ASSERT_MSG(wk->pokeParam[pokeID], "pokeID=%d", pokeID);

  return wk->pokeParam[ pokeID ];
}

//=============================================================================================
/**
 * 存在するクライアントIDか判定
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_POKECON_IsExsitClient( const BTL_POKE_CONTAINER* wk, u8 clientID )
{
  return PokeCon_IsExistClient( wk, clientID );
}

//=============================================================================================
/**
 * 戦闘に出ているポケモンかどうかを判定
 *
 * @param   pokeCon
 * @param   pokeID
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_CheckFrontPoke( BTL_MAIN_MODULE* wk, const BTL_POKE_CONTAINER* pokeCon, u8 pokeID )
{
  u8 clientID = PokeID_to_ClientID( pokeID );
  int idx = PokeCon_FindPokemon( pokeCon, clientID, pokeID );
  if( idx >= 0 )
  {
    BtlPokePos pos = BTL_MAIN_GetClientPokePos( wk, clientID, idx );
    if( pos != BTL_POS_MAX )
    {
      const BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParamConst( pokeCon, pokeID );
      return !BPP_IsDead( bpp );
    }
  }
  return FALSE;
}
//=============================================================================================
/**
 * クライアントの管理位置数を返す
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_MAIN_GetClientCoverPosCount( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  return CheckNumCoverPos( wk, clientID );
}


//=============================================================================================
/**
 * トリプルバトル時の攻撃有効範囲テーブルを取得
 *
 * @param   pos   基準位置
 *
 * @retval  const BTL_TRIPLE_ATTACK_AREA*
 */
//=============================================================================================
const BTL_TRIPLE_ATTACK_AREA* BTL_MAINUTIL_GetTripleAttackArea( BtlPokePos pos )
{
  static const BTL_TRIPLE_ATTACK_AREA  tbl[] = {
    { 2, 2, { BTL_POS_2ND_2, BTL_POS_2ND_1, BTL_POS_NULL  }, { BTL_POS_1ST_0, BTL_POS_1ST_1, BTL_POS_NULL  } },
    { 2, 2, { BTL_POS_1ST_2, BTL_POS_1ST_1, BTL_POS_NULL  }, { BTL_POS_2ND_0, BTL_POS_2ND_1, BTL_POS_NULL  } },
    { 3, 3, { BTL_POS_2ND_0, BTL_POS_2ND_1, BTL_POS_2ND_2 }, { BTL_POS_1ST_0, BTL_POS_1ST_1, BTL_POS_1ST_2 } },
    { 3, 3, { BTL_POS_1ST_0, BTL_POS_1ST_1, BTL_POS_1ST_2 }, { BTL_POS_2ND_0, BTL_POS_2ND_1, BTL_POS_2ND_2 } },
    { 2, 2, { BTL_POS_2ND_0, BTL_POS_2ND_1, BTL_POS_NULL  }, { BTL_POS_1ST_2, BTL_POS_1ST_1, BTL_POS_NULL  } },
    { 2, 2, { BTL_POS_1ST_0, BTL_POS_1ST_1, BTL_POS_NULL  }, { BTL_POS_2ND_2, BTL_POS_2ND_1, BTL_POS_NULL  } },
  };

  return &tbl[pos];
}

//------------------------------------------------------------------------------
/**
 * 指定クライアントのパーティデータを返す
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  const POKEPARTY*
 */
//------------------------------------------------------------------------------
BTL_PARTY* BTL_POKECON_GetPartyData( BTL_POKE_CONTAINER* wk, u32 clientID )
{
  GF_ASSERT(clientID < BTL_CLIENT_MAX);
  GF_ASSERT_MSG(wk->party[clientID].memberCount!=0, "clientID=%d", clientID);
  return &wk->party[ clientID ];
}
const BTL_PARTY* BTL_POKECON_GetPartyDataConst( const BTL_POKE_CONTAINER* wk, u32 clientID )
{
  GF_ASSERT(clientID < BTL_CLIENT_MAX);
  GF_ASSERT(wk->party[clientID].memberCount!=0);
  return &wk->party[ clientID ];
}


//=======================================================================================================
// BTL_PARTY
//=======================================================================================================

static void BTL_PARTY_Initialize( BTL_PARTY* party )
{
  int i;
  party->memberCount = 0;
  party->numCoverPos = 0;
  party->srcParty = NULL;
  for(i=0; i<TEMOTI_POKEMAX; i++)
  {
    party->member[i] = NULL;
  }
}

static void BTL_PARTY_AddMember( BTL_PARTY* party, BTL_POKEPARAM* member )
{
  GF_ASSERT(party->memberCount < TEMOTI_POKEMAX);
  party->member[party->memberCount++] = member;
}
static void BTL_PARTY_SetSrcParty( BTL_PARTY* party, POKEPARTY* srcParty )
{
  if( party->srcParty == NULL ){
    party->srcParty = srcParty;
  }
}
u8 BTL_PARTY_GetMemberCount( const BTL_PARTY* party )
{
  return party->memberCount;
}

u8 BTL_PARTY_GetAliveMemberCount( const BTL_PARTY* party )
{
  int i, cnt;

  for(i=0, cnt=0; i<party->memberCount; i++)
  {
    if( BPP_GetValue(party->member[i], BPP_HP) ){
      cnt++;
    }
  }
  BTL_Printf("パーティ[%p]の生きてる数カウント %d/%d\n", party, cnt, party->memberCount);
  return cnt;
}

u8 BTL_PARTY_GetAliveMemberCountRear( const BTL_PARTY* party, u8 startIdx )
{
  int i, cnt;
  for(i=startIdx, cnt=0; i<party->memberCount; i++)
  {
    if( BPP_GetValue(party->member[i], BPP_HP) ){
      cnt++;
    }
  }
  return cnt;
}

BTL_POKEPARAM* BTL_PARTY_GetMemberData( BTL_PARTY* party, u8 idx )
{
  return party->member[ idx ];
}

const BTL_POKEPARAM* BTL_PARTY_GetMemberDataConst( const BTL_PARTY* party, u8 idx )
{
  return party->member[ idx ];
}

void BTL_PARTY_SwapMembers( BTL_PARTY* party, u8 idx1, u8 idx2 )
{
  GF_ASSERT(idx1<party->memberCount);
  GF_ASSERT(idx2<party->memberCount);
  {
    BTL_POKEPARAM* tmp = party->member[ idx1 ];
    party->member[ idx1 ] = party->member[ idx2 ];
    party->member[ idx2 ] = tmp;

    BTL_Printf("パーティメンバー入れ替え %d <-> %d\n", idx1, idx2);

    #if 0
    if( party->srcParty ){
      BTL_Printf("オリジナルパーティも入れ替え %d <-> %d\n", idx1, idx2);
      PokeParty_ExchangePosition( party->srcParty, idx1, idx2, HEAPID_BTL_SYSTEM );
    }
    #endif
  }
}

void BTL_PARTY_RotateMembers( BTL_PARTY* party, BtlRotateDir dir, BTL_POKEPARAM** outPoke, BTL_POKEPARAM** inPoke )
{
  GF_ASSERT(party->memberCount>=3);

  {
    BTL_POKEPARAM* tmp = party->member[0];

    if( dir == BTL_ROTATEDIR_R )
    {
      party->member[0] = party->member[2];
      party->member[2] = party->member[1];
      party->member[1] = tmp;
      if( outPoke != NULL ){
        *outPoke = party->member[2];
      }
      if( inPoke != NULL ){
        *inPoke = party->member[0];
      }
    }
    else if( dir == BTL_ROTATEDIR_L )
    {
      party->member[0] = party->member[1];
      party->member[1] = party->member[2];
      party->member[2] = tmp;
      if( outPoke != NULL ){
        *outPoke = party->member[2];
      }
      if( inPoke != NULL ){
        *inPoke = party->member[1];
      }
    }
  }
}

s16 BTL_PARTY_FindMember( const BTL_PARTY* party, const BTL_POKEPARAM* param )
{
  int i;
  for(i=0; i<party->memberCount; ++i)
  {
    if( party->member[i] == param )
    {
      return i;
    }
  }
  return -1;
}
//----------------------------------------------------------------------
/**
 * とくせい「イリュージョン」を持つメンバーの参照ポケデータ更新
 *
 * @param   party
 * @param   memberIdx
 */
//----------------------------------------------------------------------
void BTL_PARTY_SetFakeSrcMember( BTL_PARTY* party, u8 memberIdx )
{
  if( (party->memberCount > memberIdx)
  &&  (party->memberCount > 1)
  ){
    BTL_POKEPARAM* bpp = party->member[ memberIdx ];
    if( BPP_GetValue(bpp, BPP_TOKUSEI) == POKETOKUSEI_IRYUUJON )
    {
      BTL_POKEPARAM* refPoke;
      u8 idx = memberIdx + 1;
      if( idx >= party->memberCount ){
        idx = 0;
      }
      refPoke = party->member[idx];
      BPP_SetViewSrcData( bpp, BPP_GetSrcData(refPoke) );
      BTL_Printf("%d番目にいるイリュージョン持ちポケモン[%d]の参照ポケを\n", memberIdx, BPP_GetID(bpp));
      BTL_PrintfSimple("     %d番目のポケ[%d]に変更", idx, BPP_GetID(refPoke));
    }
  }
}


POKEPARTY* BTL_MAIN_GetPlayerPokeParty( BTL_MAIN_MODULE* wk )
{
  srcParty_RefrectBtlParty( wk, wk->myClientID );
  return srcParty_Get( wk, wk->myClientID );
}
POKEPARTY* BTL_MAIN_GetMultiPlayerPokeParty( BTL_MAIN_MODULE* wk )
{
  if( BTL_MAIN_IsMultiMode(wk) )
  {
    u8 friendClientID = GetFriendCrientID( wk->myClientID );
    srcParty_RefrectBtlParty( wk, friendClientID );
    return srcParty_Get( wk, friendClientID );
  }
  else
  {
    return NULL;
  }
}

u8 BTL_MAIN_GetPlayerClientID( const BTL_MAIN_MODULE* wk )
{
  return wk->myClientID;
}

u32 BTL_MAIN_GetOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID, u8 idx )
{
  BtlPokePos pos = BTL_MAIN_GetClientPokePos( wk, clientID, 0 );
  pos = BTL_MAIN_GetOpponentPokePos( wk, pos, idx );
  return BTL_MAIN_BtlPosToClientID( wk, pos );
}


//=============================================================================================
/**
 * クライアント計算結果完了後のデータを、サーバ計算用データに上書きする
 *
 * @param   wk
 *
 */
//=============================================================================================
void BTL_MAIN_SyncServerCalcData( BTL_MAIN_MODULE* wk )
{
  #if 0
  u32 i;
  for(i=0; i<NELEMS(wk->pokeconForServer.pokeParam); ++i)
  {
    if( wk->pokeconForServer.pokeParam[i] != NULL )
    {
      BTL_POKEPARAM_Copy(
        wk->pokeconForServer.pokeParam[i],
        wk->pokeconForClient.pokeParam[i] );
    }
  }
  #endif
}

//=============================================================================================
/**
 * プレイヤー使用アイテムをバッグから減らす
 *
 * @param   wk
 * @param   clientID
 * @param   itemID
 */
//=============================================================================================
void  BTL_MAIN_DecrementPlayerItem( const BTL_MAIN_MODULE* wk, u8 clientID, u16 itemID )
{
  if( clientID == wk->myClientID )
  {
    MYITEM_SubItem( (MYITEM_PTR)(wk->setupParam->itemData), itemID, 1, wk->heapID );
  }
}
//=============================================================================================
/**
 * アイテムデータポインタ取得
 *
 * @param   wk
 *
 * @retval  MYITEM_PTR
 */
//=============================================================================================
MYITEM_PTR BTL_MAIN_GetItemDataPtr( BTL_MAIN_MODULE* wk )
{
  return wk->setupParam->itemData;
}
//=============================================================================================
/**
 * バッグカーソルデータ取得
 *
 * @param   wk
 *
 * @retval  BAG_CURSOR*
 */
//=============================================================================================
BAG_CURSOR* BTL_MAIN_GetBagCursorData( BTL_MAIN_MODULE* wk )
{
  return wk->setupParam->bagCursor;
}
//=============================================================================================
/**
 * 録画モード有効かチェック
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsRecordEnable( const BTL_MAIN_MODULE* wk )
{
  if( (wk->setupParam->recBuffer != NULL)
  &&  (wk->setupParam->fRecordPlay == FALSE)
  ){
    return TRUE;
  }
  return FALSE;
}


//=============================================================================================
/**
 * デバッグフラグ取得
 *
 * @param   wk
 * @param   flag
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_GetDebugFlag( const BTL_MAIN_MODULE* wk, BtlDebugFlag flag )
{
#ifdef PM_DEBUG
  return BTL_SETUP_GetDebugFlag( wk->setupParam, flag );
#endif
  return FALSE;
}

//=============================================================================================
/**
 * 捕獲したポケモン位置の通知を受け付け
 *
 * @param   wk
 * @param   pos
 */
//=============================================================================================
void BTL_MAIN_NotifyCapturedPokePos( BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
  if( wk->setupParam->capturedPokeIdx == TEMOTI_POKEMAX )
  {
    u8 clientID, posIdx;
    btlPos_to_cliendID_and_posIdx( wk, pos, &clientID, &posIdx );
    wk->setupParam->capturedPokeIdx = posIdx;
    BTL_Printf("捕獲ポケ位置=%d, Index=%d\n", pos, posIdx );
  }
}
//=============================================================================================
/**
 * おこずかいボーナス額の受け付け
 *
 * @param   wk
 * @param   volume
 */
//=============================================================================================
void BTL_MAIN_AddBonusMoney( BTL_MAIN_MODULE* wk, u32 volume )
{
  wk->bonusMoney += volume;
  if( wk->bonusMoney > BTL_BONUS_MONEY_MAX ){
    wk->bonusMoney = BTL_BONUS_MONEY_MAX;
  }
}

//----------------------------------------------------------------------------------------------
// トレーナーパラメータ関連
//----------------------------------------------------------------------------------------------
static void trainerParam_Init( BTL_MAIN_MODULE* wk )
{
  u32 i;
  for(i=0; i<NELEMS(wk->trainerParam); ++i){
    wk->trainerParam[i].playerStatus = NULL;
    wk->trainerParam[i].trainerID = TRID_NULL;
  }
}
static void trainerParam_Clear( BTL_MAIN_MODULE* wk )
{
  u32 i;
  for(i=0; i<NELEMS(wk->trainerParam); ++i)
  {
    if( wk->trainerParam[i].playerStatus ){
      GFL_HEAP_FreeMemory( wk->trainerParam[i].playerStatus );
      GFL_HEAP_FreeMemory( wk->trainerParam[i].name );
    }
  }
}
static void trainerParam_StorePlayer( BTL_TRAINER_DATA* dst, HEAPID heapID, const MYSTATUS* playerData )
{
  dst->playerStatus = MyStatus_AllocWork( heapID );
  MyStatus_Copy( playerData, dst->playerStatus );

  dst->trainerID = TRID_NULL;
  dst->trainerType = (MyStatus_GetMySex(dst->playerStatus) == PTL_SEX_MALE)? TRTYPE_HERO : TRTYPE_HEROINE;
  dst->name = MyStatus_CreateNameString( dst->playerStatus, HEAPID_BTL_SYSTEM );
}
static void trainerParam_StoreNPCTrainer( BTL_TRAINER_DATA* dst, const BSP_TRAINER_DATA* trData )
{
  dst->playerStatus = NULL;

  if( trData )
  {
    dst->trainerID = trData->tr_id;
    dst->trainerType = trData->tr_type;
    dst->name = trData->name;
  }
  else{
    dst->trainerID = TRID_NULL;
    dst->trainerType = 0;
    dst->name = NULL;
  }
}
BOOL BTL_MAIN_IsClientNPC( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  return ( wk->trainerParam[clientID].playerStatus == NULL );
}
// string x 2
u32 BTL_MAIN_GetClientTrainerID( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  if( clientID != BTL_CLIENTID_COMM_SUPPORT ){
    return wk->trainerParam[clientID].trainerID;
  }else{
    return TRID_NULL;
  }
}
// scu x 2
u16 BTL_MAIN_GetClientTrainerType( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  const BTL_TRAINER_DATA* trData = &wk->trainerParam[ clientID ];
  return trData->trainerType;
}

const MYSTATUS* BTL_MAIN_GetClientPlayerData( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  if( clientID != BTL_CLIENTID_COMM_SUPPORT ){
    return wk->trainerParam[ clientID ].playerStatus;
  }else{
    return COMM_PLAYER_SUPPORT_GetMyStatus( wk->setupParam->commSupport );
  }
}

//----------------------------------------------------------------------------------------------
// オリジナルパーティデータ管理
//----------------------------------------------------------------------------------------------
// 管理ワーク初期化
static void srcParty_Init( BTL_MAIN_MODULE* wk )
{
  u32 i;
  for(i=0; i<NELEMS(wk->srcParty); ++i){
    wk->srcParty[i] = NULL;
  }
  wk->tmpParty = PokeParty_AllocPartyWork( HEAPID_BTL_SYSTEM );
}
// 管理ワーククリア
static void srcParty_Quit( BTL_MAIN_MODULE* wk )
{
  u32 i;
  for(i=0; i<NELEMS(wk->srcParty); ++i)
  {
    if( wk->srcParty[i] ){
      GFL_HEAP_FreeMemory( wk->srcParty[i] );
      wk->srcParty[i] = NULL;
    }
  }
  if( wk->tmpParty ){
    GFL_HEAP_FreeMemory( wk->tmpParty );
    wk->tmpParty = NULL;
  }
}
// パーティデータ領域確保
static void srcParty_Set( BTL_MAIN_MODULE* wk, u8 clientID, const POKEPARTY* party )
{
  if( wk->srcParty[clientID] == NULL )
  {
    wk->srcParty[clientID] = PokeParty_AllocPartyWork( HEAPID_BTL_SYSTEM );
    PokeParty_Copy( party, wk->srcParty[clientID] );
  }
}
// パーティデータ取得
static POKEPARTY* srcParty_Get( BTL_MAIN_MODULE* wk, u8 clientID )
{
  GF_ASSERT( wk->srcParty[clientID] );
  return wk->srcParty[ clientID ];
}
// バトルパーティの内容をオリジナルパーティデータに反映させる
static void srcParty_RefrectBtlParty( BTL_MAIN_MODULE* wk, u8 clientID )
{
  POKEPARTY* srcParty = srcParty_Get( wk, clientID );
  BTL_PARTY* btlParty = BTL_POKECON_GetPartyData( &wk->pokeconForClient, clientID );
  u32 memberCount = PokeParty_GetPokeCount( srcParty );
  const POKEMON_PARAM* pp;
  BTL_POKEPARAM* bpp;
  u32 i;

  PokeParty_InitWork( wk->tmpParty );

  for(i=0; i<memberCount; ++i)
  {
    bpp = BTL_PARTY_GetMemberData( btlParty, i );
    BPP_ReflectToPP( bpp );
    pp = BPP_GetSrcData( bpp );
    PokeParty_Add( wk->tmpParty, pp );
  }

  PokeParty_Copy( wk->tmpParty, srcParty );

  for(i=0; i<memberCount; ++i)
  {
    bpp = BTL_PARTY_GetMemberData( btlParty, i );
    BPP_SetSrcPP( bpp, PokeParty_GetMemberPointer(srcParty, i) );
  }
}
// バトルパーティの内容を、バトル開始時の順に並べ直した上でオリジナルパーティデータに反映させる
static void srcParty_RefrectBtlPartyStartOrder( BTL_MAIN_MODULE* wk, u8 clientID )
{
  POKEPARTY* srcParty = srcParty_Get( wk, clientID );
  BTL_PARTY* btlParty = BTL_POKECON_GetPartyData( &wk->pokeconForClient, clientID );
  u32 memberCount = PokeParty_GetPokeCount( srcParty );
  const POKEMON_PARAM* pp;
  BTL_POKEPARAM* bpp;
  u32 orgPokeID, i, j;

  PokeParty_InitWork( wk->tmpParty );
  orgPokeID = ClientBasePokeID[ clientID ];

  for(i=0; i<memberCount; ++i)
  {
    for(j=0; j<memberCount; ++j){
      bpp = BTL_PARTY_GetMemberData( btlParty, j );
      if( BPP_GetID(bpp) == (orgPokeID+i) )
      {
        BPP_ReflectToPP( bpp );
        pp = BPP_GetSrcData( bpp );
        PokeParty_Add( wk->tmpParty, pp );
        break;
      }
    }
  }

  PokeParty_Copy( wk->tmpParty, srcParty );
}
//----------------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------------

//=============================================================================================
/**
 * クライアント側でレベルアップ処理を行った後、サーバ側パラメータに書き戻す
 * ※スタンドアロン実行時のみ有効
 *
 * @param   wk
 * @param   pokeID
 */
//=============================================================================================
void BTL_MAIN_ClientPokemonReflectToServer( BTL_MAIN_MODULE* wk, u8 pokeID )
{
  GF_ASSERT(wk->setupParam->competitor != BTL_COMPETITOR_COMM);
  GF_ASSERT(wk->setupParam->competitor != BTL_COMPETITOR_SUBWAY);
  {
    BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( &wk->pokeconForServer, pokeID );
    BPP_ReflectByPP( bpp );
  }
}
//----------------------------------------------------------------------------------
/**
 * バトル内パーティデータを、結果としてセットアップパラメータに書き戻す
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void reflectPartyData( BTL_MAIN_MODULE* wk )
{
  // 野生or通常トレーナー戦：経験値・レベルアップ反映のため
  if( (wk->setupParam->competitor != BTL_COMPETITOR_COMM)
  &&  (wk->setupParam->competitor != BTL_COMPETITOR_SUBWAY)
  ){
    POKEPARTY* srcParty;

    srcParty_RefrectBtlPartyStartOrder( wk, wk->myClientID );
    srcParty = srcParty_Get( wk, wk->myClientID );
    BTL_Printf("%p の結果を %p に書き戻し\nsrc_member..\n", srcParty, wk->setupParam->party[ BTL_CLIENT_PLAYER ]);
    PokeParty_Copy( srcParty, wk->setupParam->party[ BTL_CLIENT_PLAYER ] );
    {
      u32 i;
      POKEMON_PARAM* pp;
      for(i=0; i<PokeParty_GetPokeCount(srcParty); ++i){
        pp = PokeParty_GetMemberPointer(srcParty, i);
        BTL_Printf("  %p's exp=%d\n", pp, PP_Get(pp,ID_PARA_exp,NULL));
      }
    }
  }

  // 野生戦：捕獲ポケモン情報のため
  if( wk->setupParam->competitor == BTL_COMPETITOR_WILD )
  {
    u8 clientID = BTL_MAIN_GetOpponentClientID( wk, wk->myClientID, 0 );
    POKEPARTY* srcParty;

    srcParty_RefrectBtlParty( wk, clientID );
    srcParty = srcParty_Get( wk, clientID );
    PokeParty_Copy( srcParty, wk->setupParam->party[ BTL_CLIENT_ENEMY1 ] );
  }
}


//----------------------------------------------------------------------------------------------
// 勝敗判定
//----------------------------------------------------------------------------------------------
static void checkWinner( BTL_MAIN_MODULE* wk )
{
  // 種々メッセージのタグ解釈不備を解消すべし
  BtlResult result;

  if( wk->setupParam->capturedPokeIdx != TEMOTI_POKEMAX )
  {
    result = BTL_RESULT_CAPTURE;
  }
  else if( wk->escapeClientID != BTL_CLIENTID_NULL )
  {
    result = (wk->escapeClientID == wk->myClientID)? BTL_RESULT_RUN : BTL_RESULT_RUN_ENEMY;
    BTL_Printf("逃げたクライアント=%d, 自分=%d\n", wk->escapeClientID, wk->myClientID);
  }
  else
  {
    BTL_POKE_CONTAINER* container;

    u32 i;
    u8 restPokeCnt[2];

    GFL_STD_MemClear( restPokeCnt, sizeof(restPokeCnt) );
    container = &wk->pokeconForClient;

    BTL_Printf("*** 勝敗チェック ***\n");
    for(i=0; i<BTL_CLIENT_MAX; ++i)
    {
      if( PokeCon_IsExistClient( container, i ) )
      {
        BTL_PARTY* party = BTL_POKECON_GetPartyData( container, i );
        if( BTL_PARTY_GetMemberCount(party) )
        {
          u8 side = BTL_MAIN_GetClientSide( wk, i );
          u8 aliveCnt = BTL_PARTY_GetAliveMemberCount( party );
          BTL_Printf("クライアント(%d)の生き残りポケモン数=%d\n", i, aliveCnt);
          restPokeCnt[side] += aliveCnt;
        }
      }
    }

    BTL_Printf("残りポケ数  side0=%d, side1=%d\n", restPokeCnt[0], restPokeCnt[1]);

    if( restPokeCnt[0] == restPokeCnt[1] )
    {
      result = BTL_RESULT_DRAW;
    }
    else
    {
      u8 winSide = (restPokeCnt[0] > restPokeCnt[1])? 0 : 1;
      result = (winSide == BTL_MAIN_GetClientSide(wk, wk->myClientID))?
            BTL_RESULT_WIN : BTL_RESULT_LOSE;
    }
  }

  wk->setupParam->result = result;
}

//----------------------------------------------------------------------------------------------
// バトルパラメータへ書き戻し
//----------------------------------------------------------------------------------------------

/**
 *  通信対戦相手のパーティデータ
 */
static void Bspstore_Party( BTL_MAIN_MODULE* wk, u8 clientID, const POKEPARTY* party )
{
  u8 relation = CommClientRelation( wk->myClientID, clientID );
  POKEPARTY* dstParty = wk->setupParam->party[ relation ];
  if( (dstParty != NULL)
  &&  (PokeParty_GetPokeCount(dstParty) == 0)
  ){
    PokeParty_Copy( party, dstParty );
  }
}
/**
 *  通信対戦相手のプレイヤーデータ
 */
static void Bspstore_PlayerStatus( BTL_MAIN_MODULE* wk, u8 clientID, const MYSTATUS* playerStatus )
{
  u8 relation = CommClientRelation( wk->myClientID, clientID );
  MYSTATUS* dst = (MYSTATUS*)(wk->setupParam->playerStatus[ relation ]);
  if( dst != NULL){
    MyStatus_Copy( playerStatus, dst );
  }
}
/**
 *  録画用操作データがあれば格納
 */
static void Bspstore_RecordData( BTL_MAIN_MODULE* wk )
{
  if( BTL_MAIN_IsRecordEnable(wk) )
  {
    BTL_CLIENT* myClient = wk->client[ wk->myClientID ];
    const void* recData;
    u32 dataSize;

    recData = BTL_CLIENT_GetRecordData( myClient, &dataSize );
    if( recData )
    {
      BTL_Printf("録画データ %d bytes 書き戻し\n", dataSize );
      GFL_STD_MemCopy( recData, wk->setupParam->recBuffer, dataSize );
      wk->setupParam->recDataSize = dataSize;
      wk->setupParam->recRandContext = wk->randomContext;
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * 通信相手のクライアントIDと自分のクライアントIDを比較し、関係性を返す
 *
 * @param   myClientID
 * @param   targetClientID
 *
 * @retval  u8
 */
//----------------------------------------------------------------------------------
static u8 CommClientRelation( u8 myClientID, u8 targetClientID )
{
  if( myClientID == targetClientID ){
    return BTL_CLIENT_PLAYER;
  }
  else{
    BtlSide  mySide = clientID_to_side( myClientID );
    BtlSide  targetSide = clientID_to_side( targetClientID );

    if( mySide == targetSide ){
      return BTL_CLIENT_PARTNER;
    }else{
      return (targetClientID <= 1)? BTL_CLIENT_ENEMY1 : BTL_CLIENT_ENEMY2;
    }
  }
}

