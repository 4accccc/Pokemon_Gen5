//=============================================================================================
/**
 * @file    btl_main.c
 * @brief   ポケモンWB バトルシステムメインモジュール
 * @author  taya
 *
 * @date  2008.09.06  作成
 */
//=============================================================================================
#include <setjmp.h>
#include <gflib.h>


#include "system/main.h"
#include "sound/pm_sndsys.h"
#include "tr_tool/trtype_def.h"
#include "tr_tool/tr_tool.h"
#include "poke_tool/natsuki.h"
#include "gamesystem/msgspeed.h"
#include "gamesystem/g_power.h"

#include "gamesystem/btl_setup.h"
#include "battle/battle.h"

#include "btl_common.h"
#include "btl_server.h"
#include "btl_server_flow.h"
#include "btl_field.h"
#include "btl_client.h"
#include "btl_adapter.h"
#include "btl_pokeparam.h"
#include "btl_net.h"
#include "btl_util.h"

#include "btlv/btlv_core.h"
#include "btlv/btlv_mcss.h"
#include "tr_ai/tr_ai.h"

#include "btl_main.h"

#include "net/dwc_tool.h"

#include "field/ringtone_sys_ext.h"

#include "debug/debug_hudson.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  PARENT_HEAP_ID = GFL_HEAPID_APP,

  BTL_COMMITMENT_POKE_MAX = BTL_CLIENT_MAX * TEMOTI_POKEMAX,

#ifdef ROTATION_NEW_SYSTEM
  ROTATION_NUM_COVER_POS = 1,
#else
  ROTATION_NUM_COVER_POS = 2,
#endif

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
  u8  memberCount;
  u8  numCoverPos;
};

struct _BTL_POKE_CONTAINER {
  BTL_MAIN_MODULE*  mainModule;
  BTL_PARTY         party[ BTL_CLIENT_MAX ];
  POKEPARTY*        srcParty[ BTL_CLIENT_MAX ];
  BTL_POKEPARAM*    pokeParam[ BTL_COMMITMENT_POKE_MAX ];
  u32               fForServer;
};

typedef struct {
  MYSTATUS*   playerStatus;
  STRBUF*     name;

  u16         trainerType;
  u16         trainerID;
  u32         ai_bit;
  u16         useItem[ BSP_TRAINERDATA_ITEM_MAX ];

  PMS_DATA  win_word;   //戦闘終了時勝利メッセージ
  PMS_DATA  lose_word;  //戦闘終了時負けメッセージ


}BTL_TRAINER_DATA;

struct _BTL_MAIN_MODULE {

  BATTLE_SETUP_PARAM* setupParam;

  BTLV_CORE*    viewCore;
  BTL_SERVER*   server;
  BTL_SERVER*   cmdCheckServer;
  BTL_CLIENT*   client[ BTL_CLIENT_MAX ];
  BTL_TRAINER_DATA   trainerParam[ BTL_CLIENT_MAX ];
  const MYSTATUS*    playerStatus;
  u8                 fClientQuit[ BTL_CLIENT_MAX ];

  // サーバが計算時に書き換えても良い一時使用パラメータ領域と、
  // サーバコマンドを受け取ったクライアントが実際に書き換えるパラメータ領域
  BTL_POKE_CONTAINER    pokeconForClient;
  BTL_POKE_CONTAINER    pokeconForServer;
  POKEPARTY*            srcParty[ BTL_CLIENT_MAX ];
  POKEPARTY*            srcPartyForServer[ BTL_CLIENT_MAX ];
  POKEPARTY*            tmpParty;
  POKEMON_PARAM*        ppIllusionZoroArc;
  BTLNET_AIDATA_CONTAINER*  AIDataContainer;
  BTL_RECREADER         recReader;
  PERAPVOICE*           perappVoice[ BTL_CLIENT_MAX ];

  GFL_STD_RandContext   randomContext;
  BTLNET_SERVER_NOTIFY_PARAM  serverNotifyParam;

  u8        posCoverClientID[ BTL_POS_MAX ];
  u32       regularMoney;
  u32       bonusMoney;
  u32       loseMoney;
  int       msgSpeed;
  u16       LimitTimeGame;
  u16       LimitTimeCommand;
  BtlResult serverResult;

  BTL_ESCAPEINFO  escapeInfo;

  BTL_PROC    subProc;
  int         subSeq;
  pMainLoop   mainLoop;

  HEAPID      heapID;
  u8          sendClientID;

  u8        numClients;
  u8        myClientID;
  u8        myOrgPos;
  u8        ImServer;
  u8        changeMode;
  u8        MultiAIDataSeq;
  u8        MultiAIClientNum;
  u8        MultiAIClientID;
  u8        fCommError           : 1;
  u8        fCommErrorMainQuit   : 1;
  u8        fWazaEffectEnable    : 1;
  u8        fGetMoneyFixed       : 1;
  u8        fLoseMoneyFixed      : 1;
  u8        fBGMFadeOutDisable   : 1;
  u8        fMoneyDblUp          : 1;


};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT BTL_PROC_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT BTL_PROC_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static BOOL checkRecReadComplete( BTL_MAIN_MODULE* wk );
static GFL_PROC_RESULT BTL_PROC_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void setSubProcForSetup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param );
static void setSubProcForClanup( BTL_PROC* bp, BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* setup_param );
static u8 checkBagMode( const BATTLE_SETUP_PARAM* setup );
static void setup_alone_common_ClientID_and_srcParty( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp );
static void setup_common_srcParty( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp );
static void trainerParam_SetupForRecPlay( BTL_MAIN_MODULE* wk, u8 clientID );
static u8 CheckNumFrontPos( const BTL_MAIN_MODULE* wk, u8 clientID );
static void setupCommon_TrainerParam( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp );
static void setupCommon_SetRecplayerClientMode( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp );
static void setupCommon_CreateViewModule( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp, u8 bagMode );
static BOOL setup_alone_single( int* seq, void* work );
static BOOL cleanup_common( int* seq, void* work );
static BOOL setup_alone_double( int* seq, void* work );
static BOOL setup_alone_double_multi( int* seq, void* work );
static BOOL setup_alone_triple( int* seq, void* work );
static BOOL setup_alone_rotation( int* seq, void* work );
static BOOL setup_comm_single( int* seq, void* work );
static BOOL setup_comm_double( int* seq, void* work );
static BOOL setup_comm_rotation( int* seq, void* work );
static BOOL setup_comm_triple( int* seq, void* work );
static BOOL setupseq_comm_determine_server( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_notify_party_data( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_notify_player_data( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_notify_perap_voice( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_create_server_client_single( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_create_server_client_double( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_create_server_client_triple( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_create_server_client_rotation( BTL_MAIN_MODULE* wk, int* seq );
static BOOL setupseq_comm_start_server( BTL_MAIN_MODULE* wk, int* seq );
static BOOL MainLoop_StandAlone( BTL_MAIN_MODULE* wk );
static BOOL MainLoop_Comm_Server( BTL_MAIN_MODULE* wk );
static BOOL MainLoop_Comm_NotServer( BTL_MAIN_MODULE* wk );
static void NotifyCommErrorToLocalClient( BTL_MAIN_MODULE* wk );
static BOOL  CheckAllClientQuit( BTL_MAIN_MODULE* wk );
static u8 expandPokePos_single( const BTL_MAIN_MODULE* wk, BtlExPos exType, u8 basePos, u8* dst );
static u8 expandPokePos_double( const BTL_MAIN_MODULE* wk, BtlExPos exType, u8 basePos, u8* dst );
static u8 expandPokePos_triple( const BTL_MAIN_MODULE* wk, BtlExPos exType, u8 basePos, u8* dst );
static inline clientID_to_side( u8 clientID );
static inline BtlPokePos getTripleFrontPos( BtlPokePos pos );
static u32 GetClientCoverPosNum( const BTL_MAIN_MODULE* wk, u8 clientID );
static inline u8 GetFriendCrientID( u8 clientID );
static u8 btlPos_to_clientID( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos );
static inline void btlPos_to_cliendID_and_posIdx( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos, u8* clientID, u8* posIdx );
static inline u8 btlPos_to_sidePosIdx( BtlPokePos pos );
static u8 PokeID_to_ClientID( u8 pokeID );
static void NatsukiPut( const BTL_MAIN_MODULE* wk, BTL_POKEPARAM* bpp, u32 calcID );
static BOOL PokeCon_IsInitialized( const BTL_POKE_CONTAINER* pokeCon );
static void PokeCon_Init( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* mainModule, BOOL fForServer );
static void PokeCon_Clear( BTL_POKE_CONTAINER* pokecon );
static void PokeCon_AddParty( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* wk, u8 clientID );
static int GetIllusionTargetIndex( const POKEPARTY* party );
static void PokeCon_Release( BTL_POKE_CONTAINER* pokecon );
static void PokeCon_RefrectBtlParty( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* wk, u8 clientID, BOOL fDefaultForm );
static void PokeCon_RefrectBtlPartyStartOrder( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* wk, u8 clientID );
static POKEPARTY* PokeCon_GetSrcParty( BTL_POKE_CONTAINER* pokecon, u8 clientID );
static int PokeCon_FindPokemon( const BTL_POKE_CONTAINER* pokecon, u8 clientID, u8 pokeID );
static void BTL_PARTY_Initialize( BTL_PARTY* party );
static void BTL_PARTY_SetNumCoverPos( BTL_PARTY* party, u8 numCoverPos );
static void BTL_PARTY_AddMember( BTL_PARTY* party, BTL_POKEPARAM* member );
static void   BTL_PARTY_MoveAlivePokeToFirst( BTL_PARTY* party );
static void trainerParam_Init( BTL_MAIN_MODULE* wk );
static void trainerParam_Clear( BTL_MAIN_MODULE* wk );
static void trainerParam_StorePlayer( BTL_TRAINER_DATA* dst, HEAPID heapID, const MYSTATUS* playerData );
static void trainerParam_StoreNPCTrainer( BTL_TRAINER_DATA* dst, const BSP_TRAINER_DATA* trData );
static void trainerParam_StoreCommNPCTrainer( BTL_TRAINER_DATA* dst, const BTL_TRAINER_SEND_DATA* trSendData );
static void srcParty_Init( BTL_MAIN_MODULE* wk );
static void srcParty_Quit( BTL_MAIN_MODULE* wk );
static void srcParty_Set( BTL_MAIN_MODULE* wk, u8 clientID, const POKEPARTY* party );
static POKEPARTY* srcParty_Get( BTL_MAIN_MODULE* wk, u8 clientID, BOOL fForServer );
static void reflectPartyData( BTL_MAIN_MODULE* wk );
static BtlResult checkWinner( BTL_MAIN_MODULE* wk );
static void Bspstore_Party( BTL_MAIN_MODULE* wk, u8 clientID, const POKEPARTY* party );
static void Bspstore_PlayerStatus( BTL_MAIN_MODULE* wk, u8 clientID, const MYSTATUS* playerStatus );
static void Bspstore_RecordData( BTL_MAIN_MODULE* wk );
static u8 CommClientRelation( u8 myClientID, u8 targetClientID );
static void Kentei_ClearField( BATTLE_SETUP_PARAM* sp );
static void Bspstore_KenteiData( BTL_MAIN_MODULE* wk );




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



//----------------------------------------------------------------------------------------------
/**
 *  Proc Init
 */
//----------------------------------------------------------------------------------------------
static GFL_PROC_RESULT BTL_PROC_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  BTL_MAIN_MODULE* wk = mywk;

  switch( *seq ){
  case 0:
    {
      BATTLE_SETUP_PARAM* setup_param = pwk;

      GFL_HEAP_CreateHeap( PARENT_HEAP_ID, HEAPID_BTL_SYSTEM, 0x2b000 );
      GFL_HEAP_CreateHeap( PARENT_HEAP_ID, HEAPID_BTL_NET,     0x8000 );
      GFL_HEAP_CreateHeap( PARENT_HEAP_ID, HEAPID_BTL_VIEW,   0xa0000 );

      wk = GFL_PROC_AllocWork( proc, sizeof(BTL_MAIN_MODULE), HEAPID_BTL_SYSTEM );
      GFL_STD_MemClear32( wk, sizeof(BTL_MAIN_MODULE) );

      BTLV_InitSystem( HEAPID_BTL_VIEW );
      BTL_UTIL_PRINTSYS_Init();

      //Sleep時のボリューム操作をマスターボリュームに変更
      GFUser_SetSleepSoundFunc();

      wk->heapID = HEAPID_BTL_SYSTEM;
      wk->setupParam = setup_param;
      wk->setupParam->capturedPokeIdx = TEMOTI_POKEMAX;
      wk->playerStatus = wk->setupParam->playerStatus[ BTL_CLIENT_PLAYER ];
      wk->LimitTimeGame = wk->setupParam->LimitTimeGame;
      wk->LimitTimeCommand = wk->setupParam->LimitTimeCommand;
      wk->fGetMoneyFixed = FALSE;
      wk->fLoseMoneyFixed = FALSE;
      wk->fCommError = FALSE;
      wk->fCommErrorMainQuit = FALSE;
      wk->fBGMFadeOutDisable = FALSE;
      wk->fMoneyDblUp = FALSE;
      wk->MultiAIClientNum = 0;
      wk->mainLoop = NULL;
      wk->viewCore = NULL;
      wk->serverResult = BTL_RESULT_MAX;  // 無効コードとして
      wk->msgSpeed = CONFIG_GetMsgSpeed( wk->setupParam->configData );
      wk->fWazaEffectEnable = (CONFIG_GetWazaEffectMode(setup_param->configData) == WAZAEFF_MODE_ON);


      PokeCon_Init( &wk->pokeconForClient, wk, FALSE );
      PokeCon_Init( &wk->pokeconForServer, wk, TRUE );


      Kentei_ClearField( wk->setupParam );

      wk->ppIllusionZoroArc = NULL;
      wk->AIDataContainer = NULL;

      if( !(wk->setupParam->fRecordPlay) ){
        GFL_STD_RandGeneralInit( &wk->randomContext );
      }else{
        wk->randomContext = setup_param->recRandContext;
      }

      BTL_CALC_InitSys( &wk->randomContext, wk->heapID );
      BTL_CLIENTSYSTEM_Init();

      wk->regularMoney =  BTL_CALC_WinMoney( setup_param );
      wk->bonusMoney = 0;
      wk->loseMoney = 0;


      BTL_ESCAPEINFO_Clear( &wk->escapeInfo );

      wk->changeMode = (CONFIG_GetBattleRule(setup_param->configData) == BATTLERULE_IREKAE)?
          BTL_CHANGEMODE_IREKAE : BTL_CHANGEMODE_KATINUKI;

      BTL_NET_InitSystem( setup_param->netHandle, setup_param->commNetIDBit, HEAPID_BTL_NET );
      BTL_CALC_ITEM_InitSystem( HEAPID_BTL_SYSTEM );
      (*seq)++;
    }
    break;

  case 1:
    if( wk->setupParam->netHandle != NULL )
    {
      if( BTL_NET_CheckError() )
      {
        wk->fCommError = TRUE;
        wk->fCommErrorMainQuit = TRUE;
        return GFL_PROC_RES_FINISH;
      }
    }

    if( BTL_NET_IsInitialized() )
    {
      BTL_ADAPTERSYS_Init( wk->setupParam->commMode );
      setSubProcForSetup( &wk->subProc, wk, wk->setupParam );
      wk->subSeq = 0;
      trainerParam_Init( wk );
      srcParty_Init( wk );
      (*seq)++;
    }
    break;

   case 2:
   {
      if( BTL_UTIL_CallProc(&wk->subProc) )
      {
        BTL_N_Printf( DBGSTR_SETUP_DONE );

        if( wk->fCommError )
        {
          wk->fCommErrorMainQuit = TRUE;
          return GFL_PROC_RES_FINISH;
        }

        //BTS4815 捕獲デモはレコードカウントしない
        if( wk->setupParam->competitor != BTL_COMPETITOR_DEMO_CAPTURE )
        {
          BTL_MAIN_RECORDDATA_Inc( wk, RECID_BATTLE_COUNT );
        }
        return GFL_PROC_RES_FINISH;
      }
   }
   break;

  }

  return GFL_PROC_RES_CONTINUE;
}
//----------------------------------------------------------------------------------------------
/**
 *  Proc Main
 */
//----------------------------------------------------------------------------------------------
static GFL_PROC_RESULT BTL_PROC_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  BTL_MAIN_MODULE* wk = mywk;

  if( wk->mainLoop == NULL ){
    return GFL_PROC_RES_FINISH;
  }

  if( wk->mainLoop( wk ) )
  {
    if( !BTL_MAIN_IsRecordPlayMode(wk) )
    {
      BtlResult result = checkWinner( wk );
      Bspstore_RecordData( wk );
      Bspstore_KenteiData( wk );
      reflectPartyData( wk );

      switch( result ){
      case BTL_RESULT_WIN:
        wk->setupParam->getMoney = (wk->regularMoney + wk->bonusMoney);
        break;
      case BTL_RESULT_LOSE:
      case BTL_RESULT_DRAW:
        wk->setupParam->getMoney = (int)(wk->loseMoney) * -1;
        break;
      }
    }
    // 録画再生時なら最後までデータを読み込んだか判定
    else
    {
      wk->setupParam->recPlayCompleteFlag = checkRecReadComplete( wk );
      TAYA_Printf("録画データ最後まで読み込めたフラグ=%d\n", wk->setupParam->recPlayCompleteFlag);
    }

    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------------
/**
 * 録画再生データを全クライアントが最後まで読んだか判定
 */
//----------------------------------------------------------------------------------
static BOOL checkRecReadComplete( BTL_MAIN_MODULE* wk )
{
  u32 i;
  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( BTL_MAIN_IsExistClient(wk, i) )
    {
      if( !BTL_RECREADER_IsReadComplete( &wk->recReader, i ) ){
        return FALSE;
      }
    }
  }
  return TRUE;
}

//----------------------------------------------------------------------------------------------
/**
 *  Proc Quit
 */
//----------------------------------------------------------------------------------------------
static GFL_PROC_RESULT BTL_PROC_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  BTL_MAIN_MODULE* wk = mywk;

  switch( *seq ){
  case 0:
    if( wk->fBGMFadeOutDisable == FALSE ){
      PMSND_FadeOutBGM( BTL_BGM_FADEOUT_FRAMES );
    }
    wk->subSeq = 0;

    // Main側マスター輝度デフォルト以外であれば通信エラー等の可能性->フェードを開始せず即ブラックアウト
    if( (GX_GetMasterBrightness() != 0) )
    {
      wk->subSeq = -16;
      GX_SetMasterBrightness( -16 );
      GXS_SetMasterBrightness( -16 );
      (*seq) += 2;
    }
    else{
      (*seq)++;
    }
    break;

  case 1:
    if( wk->subSeq > -16 )
    {
      wk->subSeq--;
      //BTS2631対処 by iwasawa 10.06.08
      if( GX_GetMasterBrightness() > wk->subSeq ){
        GX_SetMasterBrightness( wk->subSeq );
      }
      if( GXS_GetMasterBrightness() > wk->subSeq ){
        GXS_SetMasterBrightness( wk->subSeq );
      }
    }else{
      (*seq)++;
    }
    break;
  case 2:
    if( wk->fBGMFadeOutDisable == FALSE )
    {
      if( !PMSND_CheckFadeOnBGM() )
      {
        PMSND_StopBGM();
        (*seq)++;
      }
    }
    else{
      (*seq)++;
    }
    break;
  case 3:
    BTL_Printf("クリーンアッププロセス１\n");
    if( wk->ppIllusionZoroArc ){
      GFL_HEAP_FreeMemory( wk->ppIllusionZoroArc );
      wk->ppIllusionZoroArc = NULL;
    }
    if( wk->AIDataContainer ){
      BTL_NET_AIDC_Delete( wk->AIDataContainer );
      wk->AIDataContainer = NULL;
    }

    BTL_CALC_QuitSys();
    srcParty_Quit( wk );
    trainerParam_Clear( wk );
    setSubProcForClanup( &wk->subProc, wk, wk->setupParam );
    (*seq)++;
    break;
  case 4:
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
  case 5:
    BTL_Printf("クリーンアッププロセス３\n");
    BTLV_QuitSystem();

    // BTS:2360 BTL_Client、録画再生停止処理でのSE停止を元に戻す。
    //BTS:6700
    PMSND_AllPlayerVolumeEnable(TRUE, PMSND_MASKPL_ALLSE);
    PMV_ResetMasterVolume();

    //Sleep時のボリューム操作をプレイヤーボリュームに変更
    RINGTONE_SYS_SetSleepSoundFunc();

    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_BTL_VIEW );
    GFL_HEAP_DeleteHeap( HEAPID_BTL_NET );
    GFL_HEAP_DeleteHeap( HEAPID_BTL_SYSTEM );
    BTL_Printf("クリーンアッププロセス終了\n");
    BTL_UTIL_PRINTSYS_Quit();
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

  if( setup_param->commMode == BTL_COMM_NONE )
  {
    BTL_UTIL_SetPrintType( BTL_PRINTTYPE_STANDALONE );

    switch( setup_param->rule ){
    case BTL_RULE_SINGLE:
      BTL_UTIL_SetupProc( bp, wk, setup_alone_single, NULL );
      break;
    case BTL_RULE_DOUBLE:
      if( setup_param->multiMode == BTL_MULTIMODE_NONE ){
        BTL_UTIL_SetupProc( bp, wk, setup_alone_double, NULL );
      }else{
        BTL_UTIL_SetupProc( bp, wk, setup_alone_double_multi, NULL );
      }
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
  #ifdef PM_DEBUG
  if( BTL_SETUP_GetDebugFlag(setup, BTL_DEBUGFLAG_SHOOTER_MODE) ){
    return BBAG_MODE_SHOOTER;
  }
  #endif

  switch( setup->competitor ){
  case BTL_COMPETITOR_WILD:
    return BBAG_MODE_NORMAL;

  case BTL_COMPETITOR_TRAINER:
    return BBAG_MODE_NORMAL;

  case BTL_COMPETITOR_SUBWAY:
    return BBAG_MODE_NORMAL;

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
  wk->myClientID = sp->commPos;;
  wk->myOrgPos = BTL_MAIN_GetClientPokePos( wk, wk->myClientID, 0 );

  setup_common_srcParty( wk, sp );

  {
    PERAPVOICE* pVoice = SaveData_GetPerapVoice( GAMEDATA_GetSaveControlWork(sp->gameData) );
    wk->perappVoice[ BTL_CLIENT_PLAYER ] = PERAPVOICE_AllocWork( wk->heapID );
    PERAPVOICE_CopyData( wk->perappVoice[ BTL_CLIENT_PLAYER ], pVoice );
  }

  BTL_N_Printf( DBGSTR_DEBUGFLAG_BIT, sp->DebugFlagBit );
}
/**
 *  スタンドアロンモードでパーティデータ生成（録画再生対応）
 */
static void setup_common_srcParty( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp )
{
  u8 relation_0 = CommClientRelation( wk->myClientID, 0 );
  u8 relation_1 = CommClientRelation( wk->myClientID, 1 );

  // バトル用ポケモンパラメータ＆パーティデータを生成
  srcParty_Set( wk, 0, sp->party[ relation_0 ] );
  srcParty_Set( wk, 1, sp->party[ relation_1 ] );

  {
    u8 relation_2 = CommClientRelation( wk->myClientID, 2 );
    u8 relation_3 = CommClientRelation( wk->myClientID, 3 );
    if( sp->party[ relation_2 ] != NULL ){
      srcParty_Set( wk, 2, sp->party[relation_2] );
    }
    if( sp->party[ relation_3 ] != NULL ){
      srcParty_Set( wk, 3, sp->party[relation_3] );
    }
  }
}

/**
 *  録画再生時用のトレーナーパラメータセット
 */
static void trainerParam_SetupForRecPlay( BTL_MAIN_MODULE* wk, u8 clientID )
{
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  BTL_TRAINER_DATA* trParam;
  u8 relation_clientID = CommClientRelation( wk->myClientID, clientID );

  trParam = &(wk->trainerParam[clientID]);

  if( sp->tr_data[relation_clientID]->tr_id != TRID_NULL ){
    trainerParam_StoreNPCTrainer( trParam, sp->tr_data[relation_clientID] );
  }else{
    trainerParam_StorePlayer( trParam, wk->heapID, sp->playerStatus[relation_clientID] );
  }
}


/**
 *  クライアントが管理する位置数を取得
 */
static u8 CheckNumFrontPos( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  switch( wk->setupParam->rule ){
  case BTL_RULE_SINGLE:
    return 1;
  case BTL_RULE_ROTATION:
    return BTL_ROTATION_FRONTPOS_NUM;
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

//----------------------------------------------------------------------------------
/**
 * スタンドアローンセットアップ関数共通（マルチ以外）：トレーナーデータ設定
 */
//----------------------------------------------------------------------------------
static void setupCommon_TrainerParam( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp )
{
  if( sp->fRecordPlay == FALSE ){
    trainerParam_StorePlayer( &wk->trainerParam[0], wk->heapID, wk->playerStatus );
    trainerParam_StoreNPCTrainer( &wk->trainerParam[BTL_CLIENT_ENEMY1], sp->tr_data[BTL_CLIENT_ENEMY1] );
  }else{
    trainerParam_SetupForRecPlay( wk, BTL_CLIENT_PLAYER );
    trainerParam_SetupForRecPlay( wk, BTL_CLIENT_ENEMY1 );
  }
}
//----------------------------------------------------------------------------------
/**
 * スタンドアローンセットアップ関数共通（マルチ以外）：クライアント録画再生モード設定
 */
//----------------------------------------------------------------------------------
static void setupCommon_SetRecplayerClientMode( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp )
{
  if( sp->fRecordPlay )
  {
    BTL_RECREADER_Init( &wk->recReader, sp->recBuffer, sp->recDataSize );
    BTL_CLIENT_SetRecordPlayerMode( wk->client[0], &wk->recReader );
    BTL_CLIENT_SetRecordPlayerMode( wk->client[1], &wk->recReader );
  }
}
//----------------------------------------------------------------------------------
/**
 * スタンドアローンセットアップ関数共通（マルチ含む）
 *  描画エンジン生成＆UIクライアントに関連付ける
 */
//----------------------------------------------------------------------------------
static void setupCommon_CreateViewModule( BTL_MAIN_MODULE* wk, const BATTLE_SETUP_PARAM* sp, u8 bagMode )
{
  // 描画エンジン生成、プレイヤークライアントに関連付ける
  BTL_CLIENT* uiClient = (sp->fRecordPlay)? wk->client[sp->commPos] : wk->client[0];
  wk->viewCore = BTLV_Create( wk, uiClient, &wk->pokeconForClient, bagMode, HEAPID_BTL_VIEW );
  BTL_CLIENT_AttachViewCore( uiClient, wk->viewCore );
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

  PokeCon_Init( &wk->pokeconForClient, wk, FALSE );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 1 );

  PokeCon_Init( &wk->pokeconForServer, wk, TRUE );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 1 );

  // Server 作成
  wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
  wk->ImServer = TRUE;

  // トレーナーデータ設定
  setupCommon_TrainerParam( wk, sp );

  // Client 作成
  wk->client[0] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 0, 1,
              BTL_CLIENT_TYPE_UI, bagMode, sp->fRecordPlay, &wk->randomContext, wk->heapID );

  wk->client[1] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 1, 1,
              BTL_CLIENT_TYPE_AI, bagMode, sp->fRecordPlay, &wk->randomContext, wk->heapID );


  // 録画再生モード設定
  setupCommon_SetRecplayerClientMode( wk, sp );

  // 描画エンジン生成＆UIクライアントに関連付ける
  setupCommon_CreateViewModule( wk, sp, bagMode );

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

  if( wk->viewCore ){
    BTLV_Delete( wk->viewCore );
    wk->viewCore = NULL;
  }

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

  if( wk->server ){
    BTL_SERVER_Delete( wk->server );
  }
  if( wk->cmdCheckServer ){
    BTL_SERVER_Delete( wk->cmdCheckServer );
  }


  BTL_Printf("クリーンアップ 1-4\n");

  return TRUE;
}
//--------------------------------------------------------------------------
/**
 * スタンドアローン／ダブルバトル（通常）：セットアップ
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

  // パーティデータ生成
  PokeCon_Init( &wk->pokeconForClient, wk, FALSE );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 1 );

  PokeCon_Init( &wk->pokeconForServer, wk, TRUE );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 1 );

  // トレーナーデータ生成
  setupCommon_TrainerParam( wk, sp );

  // Server 作成
  wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
  wk->ImServer = TRUE;

  // Client 作成
  wk->client[0] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 0, 2,
            BTL_CLIENT_TYPE_UI, bagMode, sp->fRecordPlay, &wk->randomContext, wk->heapID );

  wk->client[1] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 1, 2,
            BTL_CLIENT_TYPE_AI, bagMode, sp->fRecordPlay, &wk->randomContext, wk->heapID );

  // 録画再生モード設定
  setupCommon_SetRecplayerClientMode( wk, sp );

  // 描画エンジン生成＆UIクライアントに関連付ける
  setupCommon_CreateViewModule( wk, sp, bagMode );

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
 * スタンドアローン／ダブルバトル（マルチ含む）：セットアップ
 */
//--------------------------------------------------------------------------
static BOOL setup_alone_double_multi( int* seq, void* work )
{
  // server*1, client*2
  POKEPARTY* party;
  BTL_MAIN_MODULE* wk = work;
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  u8 bagMode = checkBagMode( sp );
  int i;

  // 基本設定
  wk->myClientID = 0;
  wk->numClients = 2;
  wk->posCoverClientID[BTL_POS_1ST_0] = BTL_CLIENT_PLAYER;
  wk->posCoverClientID[BTL_POS_1ST_1] = BTL_CLIENT_PLAYER;
  wk->posCoverClientID[BTL_POS_2ND_0] = BTL_CLIENT_ENEMY1;
  wk->posCoverClientID[BTL_POS_2ND_1] = BTL_CLIENT_ENEMY1;

  switch( sp->multiMode ){
  case BTL_MULTIMODE_NONE:  // 通常のダブルバトル = 基本設定のままでOK
  default:
    break;

  case BTL_MULTIMODE_PA_AA: // AIマルチ
  case BTL_MULTIMODE_PP_PP: // 通信マルチ
  case BTL_MULTIMODE_PP_AA: // 通信タッグマルチ
    wk->posCoverClientID[BTL_POS_1ST_1] = BTL_CLIENT_PARTNER;
    wk->posCoverClientID[BTL_POS_2ND_1] = BTL_CLIENT_ENEMY2;
    wk->numClients = 4;
    break;

  case BTL_MULTIMODE_P_AA:  // AIタッグマルチ
    wk->posCoverClientID[BTL_POS_2ND_1] = BTL_CLIENT_ENEMY2;
    wk->numClients = 3;
    break;
  }

  setup_alone_common_ClientID_and_srcParty( wk, sp );

  // ポケモンパーティデータ生成
  PokeCon_Init( &wk->pokeconForClient, wk, FALSE );
  PokeCon_Init( &wk->pokeconForServer, wk, TRUE );
  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( BTL_MAIN_IsExistClient(wk, i) ){
      PokeCon_AddParty( &wk->pokeconForClient, wk, i );
      PokeCon_AddParty( &wk->pokeconForServer, wk, i );
    }
  }
  // トレーナーパラメータ設定（通常プレイ用）
  if( sp->fRecordPlay == FALSE )
  {
    trainerParam_StorePlayer( &wk->trainerParam[0], wk->heapID, wk->playerStatus );
    trainerParam_StoreNPCTrainer( &wk->trainerParam[1], sp->tr_data[BTL_CLIENT_ENEMY1] );
    if( BTL_MAIN_IsExistClient(wk, BTL_CLIENT_PARTNER) ){
      trainerParam_StoreNPCTrainer( &wk->trainerParam[BTL_CLIENT_PARTNER], sp->tr_data[BTL_CLIENT_PARTNER] );
    }
    if( BTL_MAIN_IsExistClient(wk, BTL_CLIENT_ENEMY2) ){
      trainerParam_StoreNPCTrainer( &wk->trainerParam[BTL_CLIENT_ENEMY2], sp->tr_data[BTL_CLIENT_ENEMY2] );
    }
  }
  // 録画再生用
  else
  {
    for(i=0; i<BTL_CLIENT_MAX; ++i)
    {
      if( BTL_MAIN_IsExistClient(wk, i) ){
        trainerParam_SetupForRecPlay( wk, i );
      }
    }
  }

  // Server 作成
  wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
  wk->ImServer = TRUE;

  // Client 作成
  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( BTL_MAIN_IsExistClient(wk, i) )
    {
      u8 numCoverPos = GetClientCoverPosNum( wk, i );
      wk->client[i] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, i, numCoverPos,
                (i==BTL_CLIENT_PLAYER)?BTL_CLIENT_TYPE_UI : BTL_CLIENT_TYPE_AI, bagMode,
                sp->fRecordPlay, &wk->randomContext, wk->heapID );
    }
  }

  // 録画再生モード設定
  if( sp->fRecordPlay )
  {
    BTL_RECREADER_Init( &wk->recReader, sp->recBuffer, sp->recDataSize );
    for(i=0; i<BTL_CLIENT_MAX; ++i)
    {
      if( BTL_MAIN_IsExistClient(wk, i) ){
        BTL_CLIENT_SetRecordPlayerMode( wk->client[i], &wk->recReader );
      }
    }
  }


  // 描画エンジン生成＆UIクライアントに関連付ける
  setupCommon_CreateViewModule( wk, sp, bagMode );


  // Server に Client を接続
  for(i=0; i<BTL_CLIENT_MAX; ++i){
    if( BTL_MAIN_IsExistClient(wk, i) )
    {
      u8 numCoverPos = GetClientCoverPosNum( wk, i );
      BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[i]), i, numCoverPos );
    }
  }

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

  // パーティデータ生成
  PokeCon_Init( &wk->pokeconForClient, wk, FALSE );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 1 );

  PokeCon_Init( &wk->pokeconForServer, wk, TRUE );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 1 );

  // トレーナーデータ設定
  setupCommon_TrainerParam( wk, sp );

  // Server 作成
  wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
  wk->ImServer = TRUE;

  // Client 作成
  wk->client[0] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 0, 3,
          BTL_CLIENT_TYPE_UI, bagMode, sp->fRecordPlay, &wk->randomContext, wk->heapID );

  wk->client[1] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 1, 3,
          BTL_CLIENT_TYPE_AI, bagMode, sp->fRecordPlay, &wk->randomContext, wk->heapID );

  // 録画再生モード設定
  setupCommon_SetRecplayerClientMode( wk, sp );

  // 描画エンジン生成＆UIクライアントに関連付ける
  setupCommon_CreateViewModule( wk, sp, bagMode );

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
  wk->posCoverClientID[BTL_POS_1ST_2] = 0;
  wk->posCoverClientID[BTL_POS_2ND_2] = 1;

  setup_alone_common_ClientID_and_srcParty( wk, sp );

  PokeCon_Init( &wk->pokeconForClient, wk, FALSE );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForClient, wk, 1 );

  PokeCon_Init( &wk->pokeconForServer, wk, TRUE );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 0 );
  PokeCon_AddParty( &wk->pokeconForServer, wk, 1 );

  // Server 作成
  wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
  wk->ImServer = TRUE;

  // トレーナーデータ設定
  setupCommon_TrainerParam( wk, sp );

  // Client 作成
  wk->client[0] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 0, ROTATION_NUM_COVER_POS,
    BTL_CLIENT_TYPE_UI, bagMode, sp->fRecordPlay, &wk->randomContext, wk->heapID );
  wk->client[1] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, BTL_COMM_NONE, sp->netHandle, 1, ROTATION_NUM_COVER_POS,
    BTL_CLIENT_TYPE_AI, bagMode, sp->fRecordPlay, &wk->randomContext, wk->heapID );

  // 録画再生モード設定
  setupCommon_SetRecplayerClientMode( wk, sp );

  // 描画エンジン生成＆UIクライアントに関連付ける
  setupCommon_CreateViewModule( wk, sp, bagMode );

  // Server に Client を接続
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[0]), 0, ROTATION_NUM_COVER_POS );
  BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[1]), 1, ROTATION_NUM_COVER_POS );

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
    setupseq_comm_notify_perap_voice,
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
    setupseq_comm_notify_perap_voice,
    setupseq_comm_create_server_client_double,
    setupseq_comm_start_server,
  };

  BTL_MAIN_MODULE* wk = work;

  if( (*seq) == 0 )
  {
    const BATTLE_SETUP_PARAM* sp = wk->setupParam;

    POKEPARTY* party = sp->party[ BTL_CLIENT_ENEMY1 ];

    switch( sp->multiMode ){
    case BTL_MULTIMODE_NONE:
    default:
      wk->numClients = 2;
      wk->posCoverClientID[BTL_POS_1ST_0] = 0;
      wk->posCoverClientID[BTL_POS_2ND_0] = 1;
      wk->posCoverClientID[BTL_POS_1ST_1] = 0;
      wk->posCoverClientID[BTL_POS_2ND_1] = 1;
      break;
    case BTL_MULTIMODE_PP_AA:
      wk->MultiAIClientNum = 2;
      /* fallthru */
    case BTL_MULTIMODE_PP_PP:
      wk->numClients = 4;
      wk->posCoverClientID[BTL_POS_1ST_0] = 0;
      wk->posCoverClientID[BTL_POS_2ND_0] = 1;
      wk->posCoverClientID[BTL_POS_1ST_1] = 2;
      wk->posCoverClientID[BTL_POS_2ND_1] = 3;
      break;
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
    setupseq_comm_notify_perap_voice,
    setupseq_comm_create_server_client_rotation,
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
    setupseq_comm_notify_perap_voice,
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
  if( BTL_NET_CheckError() ){
    wk->fCommError = TRUE;
    return TRUE;
  }

  switch( *seq ){
  case 0:
    if( BTL_NET_DetermineServer(wk->myClientID) ){
      (*seq)++;
    }
    break;
  case 1:
    if( BTL_NET_IsServerDetermained() )
    {
      wk->sendClientID = 0;
      wk->ImServer = BTL_NET_ImServer();
      wk->setupParam->commServerVer = BTL_NET_GetServerVersion();

      if( BTL_NET_ImServer() )
      {
        BTL_UTIL_SetPrintType( BTL_PRINTTYPE_SERVER );
        (*seq) = 2;
      }
      else
      {
        BTL_UTIL_SetPrintType( BTL_PRINTTYPE_CLIENT );
        (*seq) = 3;
      }
    }
    break;

  case 2:
    // サーバマシンは各クライアントにサーバパラメータを通知する
    wk->serverNotifyParam.randomContext = wk->randomContext;
    wk->serverNotifyParam.debugFlagBit = wk->setupParam->DebugFlagBit;
    wk->serverNotifyParam.LimitTimeGame = wk->setupParam->LimitTimeGame;
    wk->serverNotifyParam.LimitTimeCommand = wk->setupParam->LimitTimeCommand;

      // Config Share モードならサーバマシンの設定を引き継ぎ
    if( BTL_MAIN_GetSetupStatusFlag(wk, BTL_STATUS_FLAG_CONFIG_SHARE) )
    {
      wk->serverNotifyParam.msgSpeed = wk->msgSpeed;
      wk->serverNotifyParam.fWazaEffectEnable = wk->fWazaEffectEnable;
    }else{
      wk->serverNotifyParam.msgSpeed = MSGSPEED_FAST;
      wk->serverNotifyParam.fWazaEffectEnable = TRUE;
    }


    if( BTL_NET_NotifyServerParam(&wk->serverNotifyParam) ){
      ++(*seq);
    }
    break;

  case 3:
    // デバッグパラメータ受信完了
    if( BTL_NET_IsServerParamReceived(&wk->serverNotifyParam) )
    {
      BATTLE_SETUP_PARAM* sp = wk->setupParam;

      sp->DebugFlagBit = wk->serverNotifyParam.debugFlagBit;
      sp->recRandContext = wk->serverNotifyParam.randomContext;

      wk->randomContext     = wk->serverNotifyParam.randomContext;
      wk->fWazaEffectEnable = wk->serverNotifyParam.fWazaEffectEnable;
      wk->msgSpeed          = wk->serverNotifyParam.msgSpeed;
      wk->LimitTimeGame     = wk->serverNotifyParam.LimitTimeGame;
      wk->LimitTimeCommand  = wk->serverNotifyParam.LimitTimeCommand;

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

  if( BTL_NET_CheckError() ){
    wk->fCommError = TRUE;
    return TRUE;
  }

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
      BTL_N_Printf( DBGSTR_MAIN_PartyDataNotifyComplete );

      if( wk->MultiAIClientNum == 0 ){
        (*seq) = 10;
      }
      else
      {
        BTL_N_Printf ( DBGSTR_MAIN_SendAIPartyStart, wk->MultiAIClientNum );
        if( wk->ImServer ){
          wk->AIDataContainer = BTL_NET_AIDC_Create( PokeParty_GetWorkSize(), GFL_HEAP_LOWID(wk->heapID) );
        }
        wk->MultiAIDataSeq = 0;
        (*seq)++;
      }
    }
    break;
  case 4:
    wk->MultiAIClientID = (wk->MultiAIDataSeq == 0) ? BTL_CLIENT_ENEMY1 : BTL_CLIENT_ENEMY2;
    if( wk->ImServer )
    {
      BTL_NET_AIDC_SetData( wk->AIDataContainer, (const void*)(sp->party[wk->MultiAIClientID]), wk->MultiAIClientID );
    }
    BTL_NET_TimingSyncStart( BTL_NET_TIMING_NOTIFY_AI_PARTY_1 + wk->MultiAIDataSeq );
    (*seq)++;
    break;
  case 5:
    if( BTL_NET_IsTimingSync( BTL_NET_TIMING_NOTIFY_AI_PARTY_1 + wk->MultiAIDataSeq) )
    {
      (*seq)++;
    }
    break;
  case 6:
    if( wk->ImServer )
    {
      if( !BTL_NET_StartNotify_AI_PartyData(wk->AIDataContainer) ){
        break;
      }
      #ifdef PM_DEBUG
      {
        const POKEPARTY* party = (const POKEPARTY*)(wk->AIDataContainer->data);
        const POKEMON_PARAM* pp;
        u32 cnt, i;
        u8 clientID;

        cnt = PokeParty_GetPokeCount( party );
        clientID = wk->AIDataContainer->clientID;
        BTL_N_Printf( DBGSTR_MAIN_SendAIPartyParam, clientID, cnt );
        for(i=0; i<cnt; ++i){
          pp = PokeParty_GetMemberPointer( party, i );
          BTL_N_PrintfSimple( DBGSTR_csv, PP_Get(pp, ID_PARA_monsno, NULL) );
        }
        BTL_N_PrintfSimple( DBGSTR_LF );
      }
      #endif
    }
    (*seq)++;
    break;

  case 7:
    if( BTL_NET_IsRecved_AI_PartyData(wk->MultiAIClientID) )
    {
      wk->MultiAIDataSeq++;
      if( wk->MultiAIDataSeq >= wk->MultiAIClientNum )
      {
        BTL_N_Printf( DBGSTR_MAIN_AIPartyDataSendComplete );
        if( wk->ImServer ){
          BTL_NET_AIDC_Delete( wk->AIDataContainer );
          wk->AIDataContainer = NULL;
        }
        (*seq) = 10;
      }
      else{
        (*seq) = 4;
      }
    }
    break;

  case 10:
    {
      u32 i;

      PokeCon_Init( &wk->pokeconForClient, wk, FALSE );

      for(i=0; i<BTL_CLIENT_MAX; ++i)
      {
        if( BTL_MAIN_IsExistClient(wk, i) )
        {
          srcParty_Set( wk, i, BTL_NET_GetPartyData(i) );
          Bspstore_Party( wk, i, BTL_NET_GetPartyData(i) );
          PokeCon_AddParty( &wk->pokeconForClient, wk, i );
        }
      }
      (*seq)++;
    }
    break;
  case 11:
//    if( wk->ImServer )  // 整合性チェック用にClientにもサーバーモジュールを作るので
    {
      u32 i;
      PokeCon_Init( &wk->pokeconForServer, wk, TRUE );
      for(i=0; i<BTL_CLIENT_MAX; ++i)
      {
        if( BTL_MAIN_IsExistClient(wk, i) ){
          PokeCon_AddParty( &wk->pokeconForServer, wk, i );
        }
      }
    }
    (*seq)++;
    break;
  case 12:
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

  if( BTL_NET_CheckError() ){
    wk->fCommError = TRUE;
    return TRUE;
  }

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
      for(i=0; i<wk->numClients; ++i)
      {
        playerStatus = BTL_NET_GetPlayerData( i );
        if( playerStatus )
        {
          trainerParam_StorePlayer( &wk->trainerParam[i], wk->heapID, playerStatus );

          //Wifiの対戦相手が不正名だったら名前を置き換え
          //BTS:5210対処　tomoya
          if( ( i != wk->myClientID ) && ( sp->WifiBadNameFlag ) )
          {
            STRCODE name[ BUFLEN_PERSON_NAME ];
            DWC_TOOL_SetBadNickName( name, BUFLEN_PERSON_NAME, wk->heapID );
            MyStatus_SetMyName( wk->trainerParam[ i ].playerStatus, name );
            GFL_STR_DeleteBuffer( wk->trainerParam[ i ].name );
            wk->trainerParam[ i ].name = DWC_TOOL_CreateBadNickName( wk->heapID );
          }

          Bspstore_PlayerStatus( wk, i, wk->trainerParam[ i ].playerStatus ); ///<BadName書き換え後のMyStatusを渡すように変更 tomoya
        }
      }
      BTL_Printf("プレイヤーデータ相互受信できました。\n");
      (*seq)++;
    }
    break;
  case 4:
    if( wk->MultiAIClientNum == 0 ){
      (*seq) = 10;
    }
    else{
      wk->MultiAIDataSeq = 0;
      (*seq)++;
    }
    break;
  // マルチ対戦でAIトレーナーデータの送信（Server->Client）が必要
  case 5:
    BTL_NET_TimingSyncStart( BTL_NET_TIMING_NOTIFY_AI_TRAINER_1 + wk->MultiAIDataSeq );
    (*seq)++;
    break;
  case 6:
    if( BTL_NET_IsTimingSync(BTL_NET_TIMING_NOTIFY_AI_TRAINER_1 + wk->MultiAIDataSeq) ){
      (*seq)++;
    }
    break;
  case 7:
    BTL_N_Printf( DBGSTR_MAIN_MultiAITrainer_SeqStart, wk->MultiAIDataSeq );
    wk->MultiAIClientID = (wk->MultiAIDataSeq == 0) ? BTL_CLIENT_ENEMY1 : BTL_CLIENT_ENEMY2;
    if( wk->ImServer )
    {
      if( !BTL_NET_StartNotify_AI_TrainerData(sp->tr_data[ wk->MultiAIClientID ]) ){
        break;
      }
      BTL_N_Printf( DBGSTR_MAIN_MultiAITrainer_SendDone, wk->MultiAIClientID );
    }
    (*seq)++;
    /* fallthru */
  case 8:
    if( BTL_NET_IsRecved_AI_TrainerData() )
    {
      const BTL_TRAINER_SEND_DATA*  trSendData = BTL_NET_Get_AI_TrainerData();

      BTL_N_Printf( DBGSTR_MAIN_RecvedMultiAITrainer, wk->MultiAIClientID, trSendData->base_data.tr_id );

      trainerParam_StoreCommNPCTrainer( &wk->trainerParam[ wk->MultiAIClientID ], trSendData );
      BTL_NET_Clear_AI_TrainerData();

      wk->MultiAIDataSeq++;
      if( wk->MultiAIDataSeq >= wk->MultiAIClientNum ){
        (*seq) = 10;
      }else{
        (*seq) = 5;
      }
    }
    break;

  case 10:
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
 * 通信対戦セットアップシーケンス：各クライアントのペラップボイスを相互受信
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL    終了時にTRUEを返す
 */
//----------------------------------------------------------------------------------
static BOOL setupseq_comm_notify_perap_voice( BTL_MAIN_MODULE* wk, int* seq )
{
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;

  if( BTL_NET_CheckError() ){
    wk->fCommError = TRUE;
    return TRUE;
  }

  switch( *seq ){
  case 0:
    BTL_NET_SetupPerappVoice();
    BTL_NET_TimingSyncStart( BTL_NET_TIMING_NOTIFY_PERAPP_VOICE );
    (*seq)++;
    break;
  case 1:
    if( BTL_NET_IsTimingSync(BTL_NET_TIMING_NOTIFY_PERAPP_VOICE) ){
      (*seq)++;
    }
    break;
  case 2:
    {
      PERAPVOICE* pVoice = SaveData_GetPerapVoice( GAMEDATA_GetSaveControlWork(sp->gameData) );
      if( BTL_NET_StartNotifyPerappVoice(pVoice) ){
        (*seq)++;
      }
    }
    break;
  case 3:
    // ペラップ相互受信を完了
    if( BTL_NET_IsCompletePerappVoice() )
    {
      u32 i;

      BTL_N_Printf( DBGSTR_MAIN_PerappVoiceComplete );
      for(i=0; i<BTL_CLIENT_MAX; ++i)
      {
        if( BTL_MAIN_IsExistClient(wk, i)
        &&  !BTL_MAIN_IsClientNPC(wk, i)
        ){
          const void* voiceRaw = BTL_NET_GetPerappVoiceRaw( i );
          if( voiceRaw ){
            wk->perappVoice[ i ] = PERAPVOICE_AllocWork( wk->heapID );
            GFL_STD_MemCopy( voiceRaw, (void*)(wk->perappVoice[i]), PERAPVOICE_GetWorkSize() );
            BTL_N_Printf( DBGSTR_MAIN_PerappVoiceAdded, i );
          }
        }
      }

      BTL_NET_QuitNotifyPerappVoice();
      (*seq)++;
    }
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
  u8 clientID = wk->myClientID;
  u8 bagMode = checkBagMode( sp );

  // 通信エラーが起きていたら終了
  if( BTL_NET_CheckError() ){
    wk->fCommError = TRUE;
    return TRUE;
  }

  // 自分がサーバ
  if( wk->ImServer )
  {
    wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

    wk->client[clientID] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        clientID, 1, BTL_CLIENT_TYPE_UI, bagMode, FALSE, &wk->randomContext, wk->heapID );
    BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[clientID]), clientID, 1 );
    BTL_SERVER_ReceptionNetClient( wk->server, sp->commMode, sp->netHandle, !clientID, 1 );
  }
  // 自分がサーバではない
  else
  {
    u32 i;

    wk->client[ clientID ] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle, clientID, 1,
      BTL_CLIENT_TYPE_UI, bagMode, FALSE, &wk->randomContext, wk->heapID  );

    // コマンド整合性チェックのためサーバを作る
    wk->cmdCheckServer = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
    for(i=0; i<BTL_CLIENT_MAX; ++i)
    {
      if( BTL_MAIN_IsExistClient(wk, i) ){
        BTL_SERVER_CmdCheckMode( wk->cmdCheckServer, i, 1 );
      }
    }

    BTL_CLIENT_AttachCmeCheckServer( wk->client[clientID], wk->cmdCheckServer );
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
  u8 clientID = wk->myClientID;
  u8 bagMode = checkBagMode( sp );
  u8 numCoverPos = (sp->multiMode==0)? 2 : 1;
  u32 i;


  // 通信エラーが起きていたら終了
  if( BTL_NET_CheckError() ){
    wk->fCommError = TRUE;
    return TRUE;
  }

  // 自分がサーバ
  if( wk->ImServer )
  {
    wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

    wk->client[clientID] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        clientID, numCoverPos, BTL_CLIENT_TYPE_UI, bagMode, FALSE, &wk->randomContext, wk->heapID );
    BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[clientID]), clientID, numCoverPos );

    // 通信タッグの場合
    // @todo 条件判定はもっと明確に記述できるように整える。
    if( (sp->multiMode == BTL_MULTIMODE_PP_AA) )
    {
      wk->client[BTL_CLIENT_ENEMY1] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
          BTL_CLIENT_ENEMY1, numCoverPos, BTL_CLIENT_TYPE_AI, bagMode, FALSE, &wk->randomContext, wk->heapID );
      BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[BTL_CLIENT_ENEMY1]), BTL_CLIENT_ENEMY1, numCoverPos );

      wk->client[BTL_CLIENT_ENEMY2] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
          BTL_CLIENT_ENEMY2, numCoverPos, BTL_CLIENT_TYPE_AI, bagMode, FALSE, &wk->randomContext, wk->heapID );
      BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[BTL_CLIENT_ENEMY2]), BTL_CLIENT_ENEMY2, numCoverPos );

      BTL_SERVER_ReceptionNetClient( wk->server, sp->commMode, sp->netHandle, BTL_CLIENT_PARTNER, numCoverPos );
    }
    // 通常の通信ダブルの場合
    else
    {
      for(i=0; i<wk->numClients; ++i)
      {
        if(i==clientID){ continue; }
        BTL_SERVER_ReceptionNetClient( wk->server, sp->commMode, sp->netHandle, i, numCoverPos );
      }
    }
  }
  // 自分がサーバではない
  else
  {
    wk->client[ clientID ] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        clientID, numCoverPos, BTL_CLIENT_TYPE_UI, bagMode, FALSE, &wk->randomContext, wk->heapID );

    // コマンド整合性チェックのためサーバを作る
    wk->cmdCheckServer = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
    for(i=0; i<BTL_CLIENT_MAX; ++i)
    {
      if( BTL_MAIN_IsExistClient(wk, i) ){
        BTL_SERVER_CmdCheckMode( wk->cmdCheckServer, i, CheckNumFrontPos(wk, i) );
      }
    }

    BTL_CLIENT_AttachCmeCheckServer( wk->client[clientID], wk->cmdCheckServer );

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
  u8 clientID = wk->myClientID;
  u8 bagMode = checkBagMode( sp );
  u32 i;

  // 通信エラーが起きていたら終了
  if( BTL_NET_CheckError() ){
    wk->fCommError = TRUE;
    return TRUE;
  }

  // 自分がサーバ
  if( wk->ImServer )
  {
    wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

    wk->client[clientID] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        clientID, NUM_COVERPOS, BTL_CLIENT_TYPE_UI, bagMode, FALSE, &wk->randomContext, wk->heapID );

    BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[clientID]), clientID, NUM_COVERPOS );
    BTL_SERVER_ReceptionNetClient( wk->server, sp->commMode, sp->netHandle, !clientID, NUM_COVERPOS );
  }
  // 自分がサーバではない
  else
  {
    wk->client[ clientID ] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle, clientID, NUM_COVERPOS,
    BTL_CLIENT_TYPE_UI, bagMode, FALSE, &wk->randomContext, wk->heapID  );

    // コマンド整合性チェックのためサーバを作る
    wk->cmdCheckServer = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
    for(i=0; i<BTL_CLIENT_MAX; ++i)
    {
      if( BTL_MAIN_IsExistClient(wk, i) ){
        BTL_SERVER_CmdCheckMode( wk->cmdCheckServer, i, CheckNumFrontPos(wk, i) );
      }
    }

    BTL_CLIENT_AttachCmeCheckServer( wk->client[clientID], wk->cmdCheckServer );
  }

  return TRUE;
}
//----------------------------------------------------------------------------------
/**
 * 通信対戦セットアップシーケンス：サーバ・クライアントモジュール構築（ローテーション用）
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL    終了時にTRUEを返す
 */
//----------------------------------------------------------------------------------
static BOOL setupseq_comm_create_server_client_rotation( BTL_MAIN_MODULE* wk, int* seq )
{
  const BATTLE_SETUP_PARAM* sp = wk->setupParam;
  u8 clientID = wk->myClientID;
  u8 bagMode = checkBagMode( sp );
  u8 numCoverPos = ROTATION_NUM_COVER_POS;
  u32 i;

  // 通信エラーが起きていたら終了
  if( BTL_NET_CheckError() ){
    wk->fCommError = TRUE;
    return TRUE;
  }

  // 自分がサーバ
  if( wk->ImServer )
  {
    wk->server = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );

    wk->client[clientID] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        clientID, numCoverPos, BTL_CLIENT_TYPE_UI, bagMode, FALSE, &wk->randomContext, wk->heapID );
    BTL_SERVER_AttachLocalClient( wk->server, BTL_CLIENT_GetAdapter(wk->client[clientID]), clientID, numCoverPos );

    for(i=0; i<wk->numClients; ++i)
    {
      if(i==clientID){ continue; }
      BTL_SERVER_ReceptionNetClient( wk->server, sp->commMode, sp->netHandle, i, numCoverPos );
    }
  }
  // 自分がサーバではない
  else
  {
    wk->client[ clientID ] = BTL_CLIENT_Create( wk, &wk->pokeconForClient, sp->commMode, sp->netHandle,
        clientID, numCoverPos, BTL_CLIENT_TYPE_UI, bagMode, FALSE, &wk->randomContext, wk->heapID );

    // コマンド整合性チェックのためサーバを作る
    wk->cmdCheckServer = BTL_SERVER_Create( wk, &wk->randomContext, &wk->pokeconForServer, bagMode, wk->heapID );
    for(i=0; i<BTL_CLIENT_MAX; ++i)
    {
      if( BTL_MAIN_IsExistClient(wk, i) ){
        BTL_SERVER_CmdCheckMode( wk->cmdCheckServer, i, CheckNumFrontPos(wk, i) );
      }
    }

    BTL_CLIENT_AttachCmeCheckServer( wk->client[clientID], wk->cmdCheckServer );

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
  // 通信エラーが起きていたら終了
  if( BTL_NET_CheckError() ){
    wk->fCommError = TRUE;
    return TRUE;
  }

  switch( *seq ){
  case 0:
    {
      const BATTLE_SETUP_PARAM* sp = wk->setupParam;
      u8 bagMode = checkBagMode( sp );

      // 描画エンジン生成
      wk->viewCore = BTLV_Create( wk, wk->client[wk->myClientID], &wk->pokeconForClient, bagMode, HEAPID_BTL_VIEW );
      BTL_CLIENT_AttachViewCore( wk->client[wk->myClientID], wk->viewCore );

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

  for(i=0; i<BTL_CLIENT_MAX; i++)
  {
    if( wk->client[i] != NULL )
    {
      if( BTL_CLIENT_Main(wk->client[i]) )
      {
        BTL_CLIENT_GetEscapeInfo( wk->client[i], &wk->escapeInfo );
        quitFlag = TRUE;
      }
    }
  }


  BTLV_CORE_Main( wk->viewCore );
  return quitFlag;
}


static BOOL MainLoop_Comm_Server( BTL_MAIN_MODULE* wk )
{
  if( wk->fCommError == FALSE )
  {
    if( BTL_NET_CheckError() )
    {
      BTL_N_Printf( DBGSTR_MAIN_CommError, __LINE__ );
      wk->fCommError = TRUE;
      NotifyCommErrorToLocalClient( wk );
    }
  }
  else if( wk->fCommErrorMainQuit )
  {
    return TRUE;
  }

  {
    BOOL quitFlag = FALSE;
    int i;

    quitFlag = BTL_SERVER_Main( wk->server );
    if( quitFlag ){
      TAYA_Printf("サーバから逃げ情報を取得\n");
      BTL_SERVER_GetEscapeInfo( wk->server, &wk->escapeInfo );
    }

    for(i=0; i<BTL_CLIENT_MAX; i++)
    {
      if( wk->client[i] && (wk->fClientQuit[i] == FALSE) )
      {
        wk->fClientQuit[i] = BTL_CLIENT_Main( wk->client[i] );
      }
    }

    if( CheckAllClientQuit(wk) ){
      TAYA_Printf("クライアントから逃げ情報を取得\n");
      BTL_CLIENT_GetEscapeInfo( wk->client[wk->myClientID], &wk->escapeInfo );
      quitFlag = TRUE;
    }

    BTLV_CORE_Main( wk->viewCore );

    return quitFlag;
  }
}

static BOOL MainLoop_Comm_NotServer( BTL_MAIN_MODULE* wk )
{
  if( wk->fCommError == FALSE )
  {
    if( BTL_NET_CheckError() )
    {
      BTL_N_Printf( DBGSTR_MAIN_CommError, __LINE__ );
      wk->fCommError = TRUE;
      NotifyCommErrorToLocalClient( wk );
    }
  }
  else if( wk->fCommErrorMainQuit )
  {
    return TRUE;
  }

  {
    BOOL quitFlag = FALSE;
    int i;

    for(i=0; i<BTL_CLIENT_MAX; i++)
    {
      if( wk->client[i] )
      {
        if( BTL_CLIENT_Main(wk->client[i]) )
        {
          BTL_CLIENT_GetEscapeInfo( wk->client[i], &wk->escapeInfo );
          quitFlag = TRUE;
        }
      }
    }

    BTLV_CORE_Main( wk->viewCore );

    return quitFlag;
  }
}

static BOOL MainLoop_Comm_Error( BTL_MAIN_MODULE* wk )
{
  return TRUE;
}


/**
 *  同一マシンに存在する全クライアントに通信エラー通知
 */
static void NotifyCommErrorToLocalClient( BTL_MAIN_MODULE* wk )
{
  u32 i;
  for(i=0; i<BTL_CLIENT_MAX; i++)
  {
    if( wk->client[i] ){
      BTL_CLIENT_NotifyCommError( wk->client[i] );
    }
  }
}
/**
 *  同一マシンに存在する全クライアントの終了フラグチェック
 */
static BOOL  CheckAllClientQuit( BTL_MAIN_MODULE* wk )
{
  u32 i;
  for(i=0; i<BTL_CLIENT_MAX; i++)
  {
    if( wk->client[i] )
    {
      if( wk->fClientQuit[i] == FALSE ){
        return FALSE;
      }
    }
  }
  return TRUE;
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
  return wk->fWazaEffectEnable;
}
//=============================================================================================
/**
 * プレイヤー取得済みバッジ数を返す
 *
 * @param   wk
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_MAIN_GetPlayerBadgeCount( const BTL_MAIN_MODULE* wk )
{
  return wk->setupParam->badgeCount;
}
//=============================================================================================
/**
 * 戦闘ルール「いれかえ」を採用するか
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsIrekaeMode( const BTL_MAIN_MODULE* wk )
{
  if( (BTL_MAIN_GetCompetitor(wk) == BTL_COMPETITOR_TRAINER)
  &&  (BTL_MAIN_GetRule(wk) == BTL_RULE_SINGLE)
  &&  (BTL_MAIN_GetCommMode(wk) == BTL_COMM_NONE)
  &&  (CONFIG_GetBattleRule(wk->setupParam->configData) == BATTLERULE_IREKAE)
  ){
    return TRUE;
  }
  return FALSE;
}

//=============================================================================================
/**
 * 文字出力のwait値を返す
 *
 * @param   wk
 *
 * @retval  int
 */
//=============================================================================================
int BTL_MAIN_GetPrintWait( const BTL_MAIN_MODULE* wk )
{
  return MSGSPEED_GetWaitByConfigParam( wk->msgSpeed );
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
    if( !BTL_MAIN_GetSetupStatusFlag(wk, BTL_STATUS_FLAG_LEGEND_EX) ){
      return TRUE;
    }
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
    return BTL_POS_2ND_2;

  default:
    GF_ASSERT(0);
    return BTL_POS_2ND_2;
  }
}
//=============================================================================================
/**
 * 現在のルールに応じて、戦闘の前衛（ワザがあたる場所）の数を返す
 *
 * @param   wk
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_MAIN_GetFrontPosNum( const BTL_MAIN_MODULE* wk )
{
  switch( wk->setupParam->rule ){
  case BTL_RULE_SINGLE:
    return 1;

  case BTL_RULE_DOUBLE:
    return 2;

  case BTL_RULE_TRIPLE:
    return 3;

  case BTL_RULE_ROTATION:
    return BTL_ROTATION_FRONTPOS_NUM;

  default:
    GF_ASSERT(0);
    return 1;
  }
}
//=============================================================================================
/**
 * 指定位置が前衛（ワザがあたる場所）かどうか判定
 *
 * @param   wk
 * @param   pos
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsFrontPos( const BTL_MAIN_MODULE* wk, BtlPokePos pos )
{
  u32 frontNum = BTL_MAIN_GetFrontPosNum( wk );
  return (pos < (frontNum * 2) );
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
  return wk->setupParam->multiMode != BTL_MULTIMODE_NONE;
}
//=============================================================================================
/**
 * 勝利時BGMナンバーを返す
 *
 * @param   wk
 *
 * @retval  u16
 */
//=============================================================================================
u16 BTL_MAIN_GetWinBGMNo( const BTL_MAIN_MODULE* wk )
{
  return wk->setupParam->musicWin;
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
  switch( wk->setupParam->competitor ){
  case BTL_COMPETITOR_WILD:
    return BTL_ESCAPE_MODE_WILD;

  case BTL_COMPETITOR_TRAINER:
    #ifdef PM_DEBUG
    if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_L | PAD_BUTTON_R) ){
      return BTL_ESCAPE_MODE_WILD;
    }
    #endif
    #ifdef DEBUG_ONLY_FOR_hudson
    // 逃げれるようにする
    if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA2 ) ||
        HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_POKE ) ||
        HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA_CAM ) )
    {
      return BTL_ESCAPE_MODE_WILD;
    }
    #endif //DEBUG_ONLY_FOR_hudson
    return BTL_ESCAPE_MODE_NG;

  case BTL_COMPETITOR_SUBWAY:
    #ifdef PM_DEBUG
    if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_L | PAD_BUTTON_R)){
      return BTL_ESCAPE_MODE_WILD;
    }
    #endif
    return BTL_ESCAPE_MODE_CONFIRM;

  case BTL_COMPETITOR_COMM:
    return BTL_ESCAPE_MODE_CONFIRM;


  default:
    return BTL_ESCAPE_MODE_NG;
  }
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
 * 図鑑登録（捕獲）されているポケモン数を返す
 *
 * @param   wk
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_MAIN_GetZukanCapturedCount( const BTL_MAIN_MODULE* wk )
{
  return ZUKANSAVE_GetPokeGetCount( wk->setupParam->zukanData );
}
//=============================================================================================
/**
 * 図鑑セーブデータを返す
 *
 * @param   wk
 *
 * @retval  ZUKAN_SAVEDATA*
 */
//=============================================================================================
ZUKAN_SAVEDATA* BTL_MAIN_GetZukanSaveData( const BTL_MAIN_MODULE* wk )
{
  return wk->setupParam->zukanData;
}
//=============================================================================================
/**
 * GameData取得
 *
 * @param   wk
 *
 * @retval  GAMEDATA*
 */
//=============================================================================================
GAMEDATA* BTL_MAIN_GetGameData( const BTL_MAIN_MODULE* wk )
{
  return (GAMEDATA*)(wk->setupParam->gameData);
}
//=============================================================================================
/**
 *  図鑑「見た」フラグセット
 *
 * @param   wk
 * @param   clientID
 * @param   bpp
 */
//=============================================================================================
void BTL_MAIN_RegisterZukanSeeFlag( const BTL_MAIN_MODULE* wk, u8 clientID, const BTL_POKEPARAM* bpp )
{
  BtlCompetitor competitor = BTL_MAIN_GetCompetitor( wk );
  if( ((competitor == BTL_COMPETITOR_WILD) || (competitor == BTL_COMPETITOR_TRAINER))
  ){
    if( clientID != BTL_CLIENT_PLAYER )
    {
      POKEMON_PARAM* pp = (POKEMON_PARAM*)BPP_GetSrcData( bpp );
      ZUKAN_SAVEDATA* zukan = (ZUKAN_SAVEDATA*)(wk->setupParam->zukanData);

      ZUKANSAVE_SetPokeSee( zukan, pp );
    }
  }
}

//=============================================================================================
/**
 * 特殊ポケ位置指定子を、実際のポケ位置に変換
 *
 * @param   wk            メインモジュールハンドラ
 * @param   exPos         特殊ポケモン位置指定子
 * @param   dst           [out] 対象ポケ位置を格納するバッファ
 *
 * @retval  u8    対象ポケ位置数
 */
//=============================================================================================
u8 BTL_MAIN_ExpandBtlPos( const BTL_MAIN_MODULE* wk, BtlExPos exPos, u8* dst )
{
  u8 exType = EXPOS_GET_TYPE( exPos );
  u8 basePos = EXPOS_GET_BASEPOS( exPos );

  if( exType == BTL_EXPOS_DEFAULT )
  {
    dst[0] = basePos;
    return (dst[0] != BTL_POS_NULL)? 1 : 0;
  }

  if( basePos != BTL_POS_NULL )
  {
    switch( wk->setupParam->rule ){
    case BTL_RULE_SINGLE:
    default:
      return expandPokePos_single( wk, exType, basePos, dst );
    case BTL_RULE_DOUBLE:
      return expandPokePos_double( wk, exType, basePos, dst );
    case BTL_RULE_TRIPLE:
      return expandPokePos_triple( wk, exType, basePos, dst );
    case BTL_RULE_ROTATION:
      #ifdef ROTATION_NEW_SYSTEM
      return expandPokePos_single( wk, exType, basePos, dst );
      #else
      return expandPokePos_double( wk, exType, basePos, dst );
      #endif
    }
  }

  dst[0] = BTL_POS_NULL;
  return 0;
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
  case BTL_EXPOS_NEXT_FRIENDS:
    dst[0] = BTL_MAIN_GetNextPokePos( wk, basePos );
    return 1;

  case BTL_EXPOS_FULL_FRIENDS:
    dst[0] = basePos;
    dst[1] = BTL_MAIN_GetNextPokePos( wk, basePos );
    return 2;

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

  case BTL_EXPOS_FULL_FRIENDS:
#if 0
    {
      u8 i, cnt;
      for(i=0, cnt=0; i<3; ++i){
        if( i == posIdx ){ continue; }
        dst[cnt++] = BTL_MAINUTIL_GetFriendPokePos( basePos, i );
      }
      return cnt;
    }
#endif
    dst[0] = BTL_MAINUTIL_GetFriendPokePos( basePos, 0 );
    dst[1] = BTL_MAINUTIL_GetFriendPokePos( basePos, 1 );
    dst[2] = BTL_MAINUTIL_GetFriendPokePos( basePos, 2 );
    return 3;

  case BTL_EXPOS_FULL_ALL:
    dst[0] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 0 );
    dst[1] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 1 );
    dst[2] = BTL_MAIN_GetOpponentPokePos( wk, basePos, 2 );
    dst[3] = BTL_MAINUTIL_GetFriendPokePos( basePos, 0 );
    dst[4] = BTL_MAINUTIL_GetFriendPokePos( basePos, 1 );
    dst[5] = BTL_MAINUTIL_GetFriendPokePos( basePos, 2 );
    return 6;

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

  case BTL_EXPOS_NEXT_FRIENDS:
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
  TAYA_Printf("btlpos2sidePosIdx .. pos(%d) -> idx(%d)\n", pos, idx);
  if( idx == 0 ){
    idx = 2;
  }else if(idx == 2){
    idx = 0;
  }
  return BTL_MAINUTIL_GetOpponentPokePos( BTL_RULE_TRIPLE, pos, idx );
}
/**
 *  指定クライアントの担当する位置数を返す
 */
static u32 GetClientCoverPosNum( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  u32 cnt, i;
  for(cnt=i=0; i<NELEMS(wk->posCoverClientID); ++i)
  {
    if( wk->posCoverClientID[i] == clientID ){
      ++cnt;
    }
  }
  TAYA_Printf("Client-%d, numCoverPos=%d\n", cnt);
  return cnt;
}

//=============================================================================================
/**
 * 有効なクライアントかどうか判定
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsExistClient( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  if( clientID < BTL_CLIENT_MAX )
  {
    int i;
    for(i=0; i<NELEMS(wk->posCoverClientID); ++i)
    {
      if( wk->posCoverClientID[i] == clientID ){
        return TRUE;
      }
    }
  }
  return FALSE;
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
    GF_ASSERT(idx<BTL_ROTATION_FRONTPOS_NUM);
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

  case BTL_RULE_ROTATION:
    #ifdef ROTATION_NEW_SYSTEM
      return BTL_MAINUTIL_GetOpponentPokePos( rule, pos, 0 );
    #else
    {
      u8 idx = btlPos_to_sidePosIdx( pos );
      idx ^= 1;
      return BTL_MAINUTIL_GetOpponentPokePos( rule, pos, idx );
    }
    #endif
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

  case BTL_RULE_ROTATION:
    #ifdef ROTATION_NEW_SYSTEM
    GF_ASSERT(0);
    return basePos;
    #endif
    /* fallthru */
  case BTL_RULE_DOUBLE:
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
 *  マルチモード時の味方クライアントIDに変換（存在しなくても計算上の値を返す）
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
static u8 btlPos_to_clientID( const BTL_MAIN_MODULE* wk, BtlPokePos btlPos )
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
static u8 PokeID_to_ClientID( u8 pokeID )
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
 * 特殊ポケ位置指定子を、実際のポケ位置に展開後、
 * 実際にそこに存在しているポケモンパラメータをポインタ配列に格納
 *
 * @param   wk
 * @param   pokeCon
 * @param   exPos
 * @param   bppAry
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_MAIN_ExpandExistPokeID( const BTL_MAIN_MODULE* wk, BTL_POKE_CONTAINER* pokeCon, BtlExPos exPos, u8* pokeIDAry )
{
  BTL_POKEPARAM* bpp;
  u8 pokePos[ BTL_POS_MAX ];
  u8 posCnt, existCnt, i;

  posCnt = BTL_MAIN_ExpandBtlPos( wk, exPos, pokePos );
  for(i=0, existCnt=0; i<posCnt; ++i)
  {
    bpp = BTL_POKECON_GetFrontPokeData( pokeCon, pokePos[i] );
    if( (bpp != NULL) && !BPP_IsDead(bpp) )
    {
      pokeIDAry[ existCnt++ ] = BPP_GetID( bpp );
    }
  }
  return existCnt;
}

//=============================================================================================
/**
 * バトルポケモンIDをポケモン戦闘位置に変換
 *
 * @param   wk
 * @param   pokeID
 *
 * @retval  BtlPokePos    ポケモン戦闘位置（戦闘に出ていなければBTL_POS_NULL）
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
  }
  return BTL_POS_NULL;
}
//=============================================================================================
/**
 * バトルポケモンIDをポケモン描画位置に変換
 *
 * @param   wk
 * @param   pokeCon
 * @param   pokeID
 *
 * @retval  u8   （戦闘に出ていない場合 BTLV_MCSS_POS_ERROR )
 */
//=============================================================================================
u8 BTL_MAIN_PokeIDtoViewPos( const BTL_MAIN_MODULE* wk, const BTL_POKE_CONTAINER* pokeCon, u8 pokeID )
{
  BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk, pokeCon, pokeID );
  if( pos != BTL_POS_NULL )
  {
    return BTL_MAIN_BtlPosToViewPos( wk, pos );
  }
  else
  {
    return BTLV_MCSS_POS_ERROR;
  }
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
    u8 result = vpos[ isPlayerSide ][ posIdx ];

    return result;
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

    u8 org_pos = ( wk->myOrgPos & 1 );
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
/**
 *  ポケモンリスト画面をマルチモードで開くかどうか判定
 */
BOOL BTL_MAIN_IsPokeListMultiMode( BTL_MAIN_MODULE* wk )
{
  if( (BTL_MAIN_IsMultiMode(wk)) )
  {
    BtlCompetitor competitor = BTL_MAIN_GetCompetitor( wk );

    if( (competitor != BTL_COMPETITOR_TRAINER)
    &&  (competitor != BTL_COMPETITOR_WILD)
    ){
      return TRUE;
    }
  }
  return FALSE;
}

/**
 *  クライアントの元パーティデータを取得（バトルリスト画面等に使用）
 */
POKEPARTY* BTL_MAIN_GetClientSrcParty( BTL_MAIN_MODULE* wk, u8 clientID )
{
  PokeCon_RefrectBtlParty( &wk->pokeconForClient, wk, clientID, FALSE );
  return PokeCon_GetSrcParty( &wk->pokeconForClient, clientID );
}
/**
 *  マルチバトル時、クライアントの仲間パーティ（いなければNULL）を取得
 */
POKEPARTY* BTL_MAIN_GetClientMultiSrcParty( BTL_MAIN_MODULE* wk, u8 clientID )
{
  if( BTL_MAIN_IsMultiMode(wk) )
  {
    u8 friendClientID = GetFriendCrientID( clientID );
    if( BTL_MAIN_IsExistClient(wk, friendClientID) )
    {
      PokeCon_RefrectBtlParty( &wk->pokeconForClient, wk, friendClientID, FALSE );
      return PokeCon_GetSrcParty( &wk->pokeconForClient, friendClientID );
    }
  }
  return NULL;
}
u8 BTL_MAIN_GetClientMultiPos( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  if( BTL_MAIN_IsMultiMode(wk) )
  {
    return ((clientID & 2) != 0);
  }
  return 0;
}
/**
 * プレイヤーのマルチ立ち位置(0 or 1）を取得
 */
u8 BTL_MAIN_GetPlayerMultiPos( const BTL_MAIN_MODULE* wk )
{
  return BTL_MAIN_GetClientMultiPos( wk, wk->myClientID );
}



u8 BTL_MAIN_GetPlayerClientID( const BTL_MAIN_MODULE* wk )
{
  return wk->myClientID;
}
/**
 *  存在するならプレイヤーの味方クライアントIDを返す（存在しない場合 BTL_CLIENTID_NULL ）
 */
u8 BTL_MAIN_GetPlayerFriendCleintID( const BTL_MAIN_MODULE* wk )
{
  return BTL_MAIN_GetFriendCleintID( wk, wk->myClientID );
}
/**
 *  存在するなら指定クライアントの味方クライアントIDを返す（存在しない場合 BTL_CLIENTID_NULL ）
 */
u8 BTL_MAIN_GetFriendCleintID( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  u8 friendClientID = GetFriendCrientID( clientID );
  if( BTL_MAIN_IsExistClient(wk, friendClientID) ){
    return friendClientID;
  }
  return BTL_CLIENTID_NULL;
}
BOOL BTL_MAINUTIL_IsFriendClientID( u8 clientID_1, u8 clientID_2 )
{
  if( clientID_1 == clientID_2 ){
    return TRUE;
  }
  if( clientID_1 == (GetFriendCrientID(clientID_2)) ){
    return TRUE;
  }
  return FALSE;
}
u8 BTL_MAIN_GetEnemyClientID( const BTL_MAIN_MODULE* wk, u8 idx )
{
  return BTL_MAIN_GetOpponentClientID( wk, wk->myClientID, idx );
}

u32 BTL_MAIN_GetOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID, u8 idx )
{
  clientID &= 1;
  clientID ^= 1;
  if( idx )
  {
    u8 nextClientID = clientID + ((idx & 1) * 2);
    if( BTL_MAIN_IsExistClient(wk, nextClientID) ){
      clientID = nextClientID;
    }
  }

  return clientID;
  /*
  BtlPokePos pos = BTL_MAIN_GetClientPokePos( wk, clientID, 0 );
  pos = BTL_MAIN_GetOpponentPokePos( wk, pos, idx );
  return BTL_MAIN_BtlPosToClientID( wk, pos );
  */
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
  if( wk->setupParam->fRecordPlay == FALSE )
  {
    if( clientID == wk->myClientID )
    {
      MYITEM_SubItem( (MYITEM_PTR)(wk->setupParam->itemData), itemID, 1, wk->heapID );
    }
  }
}
//=============================================================================================
/**
 * プレイヤー使用アイテムを追加（書き戻し）
 *
 * @param   wk
 * @param   clientID
 * @param   itemID
 */
//=============================================================================================
void BTL_MAIN_AddItem( const BTL_MAIN_MODULE* wk, u8 clientID, u16 itemID )
{
  if( wk->setupParam->fRecordPlay == FALSE )
  {
    if( clientID == wk->myClientID )
    {
      MYITEM_AddItem( (MYITEM_PTR)(wk->setupParam->itemData), itemID, 1, wk->heapID );
      TAYA_Printf("戻したアイテム=%d, その個数=%d\n", itemID,
                MYITEM_GetItemNum( (MYITEM_PTR)(wk->setupParam->itemData), itemID, wk->heapID) );
    }
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
 * 録画データ再生モードかチェック
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsRecordPlayMode( const BTL_MAIN_MODULE* wk )
{
  return wk->setupParam->fRecordPlay;
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
 * サーバから捕獲したポケモン位置の通知を受け付け
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
  }
}
//=============================================================================================
/**
 * サーバから勝敗判定結果を受け付け
 *
 * @param   wk
 * @param   result
 */
//=============================================================================================
void BTL_MAIN_NotifyBattleResult( BTL_MAIN_MODULE* wk, BtlResult result )
{
  if( wk->serverResult == BTL_RESULT_MAX ){
    wk->serverResult = result;
  }
}

//=============================================================================================
/**
 *
 *
 * @param   wk
 *
 * @retval  BtlResult
 */
//=============================================================================================
BtlResult BTL_MAIN_GetBattleResult( BTL_MAIN_MODULE* wk )
{
  return checkWinner( wk );
}

//=============================================================================================
/**
 * 通信対戦時コマンド整合性チェックエラー発生通知を受け付け
 *
 * @param   wk
 */
//=============================================================================================
void BTL_MAIN_NotifyCmdCheckError( BTL_MAIN_MODULE* wk )
{
  wk->setupParam->cmdIllegalFlag = TRUE;
}

//=============================================================================================
/**
 * おこずかい額を現在の値で決定する
 *
 * @param   wk
 */
//=============================================================================================
u32 BTL_MAIN_FixRegularMoney( BTL_MAIN_MODULE* wk )
{
  if( wk->fGetMoneyFixed == FALSE )
  {
    if( wk->fMoneyDblUp ){
      wk->regularMoney *= 2;
    }
    wk->regularMoney = GPOWER_Calc_Money( wk->regularMoney );
    wk->fGetMoneyFixed = TRUE;
  }

  return wk->regularMoney;
}
//=============================================================================================
/**
 * ボーナス額の増量受け付け
 *
 * @param   wk
 * @param   volume
 */
//=============================================================================================
void BTL_MAIN_AddBonusMoney( BTL_MAIN_MODULE* wk, u32 volume )
{
  if( wk->fGetMoneyFixed == FALSE )
  {
    wk->bonusMoney += volume;
    if( wk->bonusMoney > BTL_BONUS_MONEY_MAX ){
      wk->bonusMoney = BTL_BONUS_MONEY_MAX;
    }
  }
}
//=============================================================================================
/**
 * ボーナス額を取得
 *
 * @param   wk
 * @param   volume
 */
//=============================================================================================
u32 BTL_MAIN_GetBonusMoney( const BTL_MAIN_MODULE* wk )
{
  u32 bonus = wk->bonusMoney;
  if( wk->fMoneyDblUp ){
    bonus *= 2;
    if( bonus > BTL_BONUS_MONEY_MAX ){
      bonus = BTL_BONUS_MONEY_MAX;
    }
  }
  return bonus;
}
//=============================================================================================
/**
 * おこづかい・ボーナスの２倍化フラグ設定
 *
 * @param   wk
 */
//=============================================================================================
void BTL_MAIN_SetMoneyDblUp( BTL_MAIN_MODULE* wk )
{
  wk->fMoneyDblUp = TRUE;
}

//=============================================================================================
/**
 * 負けた時の支払い金額を確定させる
 *
 * @param   wk
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_MAIN_FixLoseMoney( BTL_MAIN_MODULE* wk )
{
  u32 loseMoney = 0;

  if( wk->fLoseMoneyFixed == FALSE )
  {
    if( (wk->setupParam->competitor == BTL_COMPETITOR_WILD)
    ||  (wk->setupParam->competitor == BTL_COMPETITOR_TRAINER)
    ){
      u32 holdMoney = MISC_GetGold( GAMEDATA_GetMiscWork(wk->setupParam->gameData) );
      const BTL_PARTY* playerParty = BTL_POKECON_GetPartyDataConst( &(wk->pokeconForServer), wk->myClientID );
      loseMoney = BTL_CALC_LoseMoney( wk->setupParam->badgeCount, playerParty );
      if( loseMoney > holdMoney ){
        loseMoney = holdMoney;
      }
      wk->loseMoney = loseMoney;
    }
    wk->fLoseMoneyFixed = TRUE;
  }
  return loseMoney;
}
//=============================================================================================
/**
 * 戦闘終了時、BGMをフェードアウトさせない（捕獲時など）
 *
 * @param   wk
 */
//=============================================================================================
void BTL_MAIN_BGMFadeOutDisable( BTL_MAIN_MODULE* wk )
{
  wk->fBGMFadeOutDisable = TRUE;
}

//=============================================================================================
/**
 * プレイヤーポケモンが死んだ時のなつき度反映
 *
 * @param   wk
 * @param   bpp
 * @param   fLargeDiffLevel
 */
//=============================================================================================
void BTL_MAIN_ReflectNatsukiDead( const BTL_MAIN_MODULE* wk, BTL_POKEPARAM* bpp, BOOL fLargeDiffLevel )
{
  if( (wk->setupParam->competitor == BTL_COMPETITOR_WILD)
  ||  (wk->setupParam->competitor == BTL_COMPETITOR_TRAINER)
  ){
    if( wk->setupParam->fRecordPlay == FALSE)
    {
      u32 calcID = (fLargeDiffLevel)? CALC_NATSUKI_LEVEL30_HINSHI : CALC_NATSUKI_HINSHI;
      NatsukiPut( wk, bpp, calcID );
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * なつき度計算、サーバ・クライアント双方のsrcPPに反映させる
 *
 * @param   wk
 * @param   bpp
 * @param   calcID
 */
//----------------------------------------------------------------------------------
static void NatsukiPut( const BTL_MAIN_MODULE* wk, BTL_POKEPARAM* bpp, u32 calcID )
{
  u8 pokeID = BPP_GetID( bpp );

  const BTL_POKEPARAM* bppServer = BTL_POKECON_GetPokeParamConst( &wk->pokeconForServer, pokeID );
  const BTL_POKEPARAM* bppClient = BTL_POKECON_GetPokeParamConst( &wk->pokeconForClient, pokeID );

  POKEMON_PARAM* ppServer = (POKEMON_PARAM*)BPP_GetSrcData( bppServer );
  POKEMON_PARAM* ppClient = (POKEMON_PARAM*)BPP_GetSrcData( bppClient );

  const BTL_FIELD_SITUATION* sit = BTL_MAIN_GetFieldSituation( wk );

  NATSUKI_Calc( ppServer, calcID, sit->zoneID, GFL_HEAP_LOWID(wk->heapID) );
  NATSUKI_Calc( ppClient, calcID, sit->zoneID, GFL_HEAP_LOWID(wk->heapID) );
}


//=============================================================================================
/**
 * クライアント側で処理したレベルアップ・ワザおぼえの結果をサーバ用パラメータに反映
 *
 * @param   wk
 * @param   pokeID
 */
//=============================================================================================
void BTL_MAIN_ReflectPokeWazaOboe( BTL_MAIN_MODULE* wk, u8 pokeID )
{
  BTL_POKEPARAM  *bppSv, *bppCl;

  bppSv = BTL_POKECON_GetPokeParam( &wk->pokeconForServer, pokeID );
  bppCl = BTL_POKECON_GetPokeParam( &wk->pokeconForClient, pokeID );

  BPP_WAZA_Copy( bppCl, bppSv );
}


//=======================================================================================================
// BTL_POKE_CONTAINER
//=======================================================================================================

static BOOL PokeCon_IsInitialized( const BTL_POKE_CONTAINER* pokeCon )
{
  return (pokeCon->mainModule != NULL);
}


static void PokeCon_Init( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* mainModule, BOOL fForServer )
{
  int i;

  pokecon->mainModule = mainModule;
  pokecon->fForServer = fForServer;

  for(i=0; i<NELEMS(pokecon->party); ++i){
    BTL_PARTY_Initialize( &pokecon->party[i] );
  }
  for(i=0; i<NELEMS(pokecon->pokeParam); ++i){
    pokecon->pokeParam[i] = NULL;
  }
  for(i=0; i<NELEMS(pokecon->srcParty); ++i){
    pokecon->srcParty[i] = NULL;
  }
}
static void PokeCon_Clear( BTL_POKE_CONTAINER* pokecon )
{
  int i;
  for(i=0; i<NELEMS(pokecon->pokeParam); ++i)
  {
    if( pokecon->pokeParam[i] != NULL )
    {
      BTL_POKEPARAM_Delete( pokecon->pokeParam[i] );
      pokecon->pokeParam[i] = NULL;
    }
  }

  for(i=0; i<NELEMS(pokecon->party); ++i){
    BTL_PARTY_Initialize( &pokecon->party[i] );
  }
}


static void PokeCon_AddParty( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* wk, u8 clientID )
{
  POKEPARTY* party_src = srcParty_Get( wk, clientID, pokecon->fForServer );
  BTL_PARTY* party = &pokecon->party[ clientID ];
  u32 poke_count = PokeParty_GetPokeCount( party_src );
  u8 pokeID_Start = ClientBasePokeID[ clientID ];
  u8 pokeID;

  POKEMON_PARAM* pp;
  int i;

  pokecon->srcParty[ clientID ] = party_src;

  pokeID = pokeID_Start;
  for(i=0; i<poke_count; ++i, ++pokeID)
  {
    pp = PokeParty_GetMemberPointer( party_src, i );
    if( i==0 )
    {
      TAYA_Printf("ClientID=%d, 1stPokePP=%d\n", PP_Get(pp, ID_PARA_pp1, NULL));
    }
    pokecon->pokeParam[ pokeID ] = BTL_POKEPARAM_Create( pp, pokeID, HEAPID_BTL_SYSTEM );
    BTL_PARTY_AddMember( party, pokecon->pokeParam[ pokeID ] );
  }

  // 最後以外のポケモンがイリュージョン使いなら、最後のSrcPPを見せかけデータにする
  {
    int lastPokeIndex = GetIllusionTargetIndex( party_src );
    if( lastPokeIndex > 0 )
    {
      pokeID = pokeID_Start;
      for(i=0; i<lastPokeIndex; ++i, ++pokeID)
      {
        if( BPP_GetValue(pokecon->pokeParam[pokeID], BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_IRYUUJON )
        {
          pp = PokeParty_GetMemberPointer( party_src, lastPokeIndex );
          BPP_SetViewSrcData( pokecon->pokeParam[ pokeID ], pp );
          TAYA_Printf("%d番目のポケはイリュージョンね\n", i );
        }
      }
    }
  }

  // 野生ゾロアークのイリュージョンなら特殊処理
  if( (BTL_MAIN_GetCompetitor(wk) == BTL_COMPETITOR_WILD) && (clientID == BTL_CLIENTID_SA_ENEMY1) )
  {
    enum {
      MONSNO_NULL = 0,
    };
    BTL_POKEPARAM* bpp;

    pokeID = ClientBasePokeID[ clientID ];
    for(i=0; i<poke_count; ++i, ++pokeID)
    {
      bpp = pokecon->pokeParam[ pokeID ];
      if( (BPP_GetMonsNo(bpp) == MONSNO_ZOROAAKU)
      &&  (BPP_GetValue(bpp, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_IRYUUJON)
      ){
        /*
         * 自分の手持ちに ライコウ がいれば エンテイ に化ける。
         * 同じく手持ちに エンテイ がいれば スイクン に、
         *                スイクン がいれば ライコウ にそれぞれ化ける
         */
        POKEPARTY* party_player = srcParty_Get( wk, 0, pokecon->fForServer );
        u8 player_poke_cnt = PokeParty_GetPokeCount( party_player );
        u8 p;
        u16 monsno, monsno_illusion = MONSNO_NULL;

        for(p=0; p<player_poke_cnt; ++p)
        {
          pp = PokeParty_GetMemberPointer( party_player, p );
          monsno = PP_Get( pp, ID_PARA_monsno, NULL );
          if( monsno == MONSNO_RAIKOU ){
            monsno_illusion = MONSNO_ENTEI;
            break;
          }
          if( monsno == MONSNO_ENTEI ){
            monsno_illusion = MONSNO_SUIKUN;
            break;
          }
          if( monsno == MONSNO_SUIKUN ){
            monsno_illusion = MONSNO_RAIKOU;
            break;
          }
        }

        if( monsno_illusion != MONSNO_NULL )
        {
          if( wk->ppIllusionZoroArc == NULL )
          {
            u32 level = BPP_GetValue( bpp, BPP_LEVEL );
            wk->ppIllusionZoroArc = PP_Create( monsno_illusion, level, 0, wk->heapID );
          }
          BPP_SetViewSrcData( bpp, wk->ppIllusionZoroArc );
        }
      }
    }
  }

  BTL_PARTY_MoveAlivePokeToFirst( party );
}
static int GetIllusionTargetIndex( const POKEPARTY* party )
{
  int poke_count = PokeParty_GetPokeCount( party );
  const POKEMON_PARAM* pp;
  int i;

  for(i=(poke_count-1); i>=0; --i)
  {
    pp = PokeParty_GetMemberPointer( party, i );
    if( (!PP_Get(pp, ID_PARA_tamago_flag, NULL))  // タマゴ＆ひん死は対象外
    &&  (PP_Get(pp, ID_PARA_hp, NULL) != 0)
    ){
      return i;
    }
  }
  return -1;
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
// バトルパーティの内容をオリジナルパーティデータに反映させる
static void PokeCon_RefrectBtlParty( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* wk, u8 clientID, BOOL fDefaultForm )
{
  POKEPARTY* srcParty = PokeCon_GetSrcParty( pokecon, clientID );
  BTL_PARTY* btlParty = BTL_POKECON_GetPartyData( pokecon, clientID );
  u32 memberCount = PokeParty_GetPokeCount( srcParty );
  POKEMON_PARAM *pp;
  BTL_POKEPARAM* bpp;
  u8  illusionTargetIdx[ BTL_PARTY_MEMBER_MAX ];
  u32 i;

  PokeParty_InitWork( wk->tmpParty );

  // 無効値でクリア
  for(i=0; i<NELEMS(illusionTargetIdx); ++i){
    illusionTargetIdx[ i ] = BTL_PARTY_MEMBER_MAX;
  }

  for(i=0; i<memberCount; ++i)
  {
    bpp = BTL_PARTY_GetMemberData( btlParty, i );
    BPP_ReflectToPP( bpp, fDefaultForm );
    pp = (POKEMON_PARAM*)BPP_GetSrcData( bpp );
    PokeParty_Add( wk->tmpParty, pp );

    // イリュージョン中の場合、自分のイリュージョン対象ポケIndexを記録する
    if( BPP_IsFakeEnable(bpp) )
    {
      const POKEMON_PARAM* ppFake = BPP_GetViewSrcData( bpp );
      u32 j;
      for(j=0; j<memberCount; ++j)
      {
        BTL_POKEPARAM* bppSeek = BTL_PARTY_GetMemberData( btlParty, j );
        if( ppFake == BPP_GetSrcData(bppSeek) )
        {
//          TAYA_Printf("%d番目のポケモンはイリュージョン。対象は%d番目(monsno=%d)\n",
//                      i, j, PP_Get(ppFake, ID_PARA_monsno, 0)  );
          illusionTargetIdx[i] = j;
          break;
        }
      }
    }
  }

  PokeParty_Copy( wk->tmpParty, srcParty );

  // SrcPP ポインタを再設定
  for(i=0; i<memberCount; ++i)
  {
    pp = PokeParty_GetMemberPointer( srcParty, i );
    bpp = BTL_PARTY_GetMemberData( btlParty, i );
    BPP_SetSrcPP( bpp,  pp );

    if( illusionTargetIdx[i] != BTL_PARTY_MEMBER_MAX )
    {
      pp = PokeParty_GetMemberPointer( srcParty, illusionTargetIdx[i] );
      BPP_SetViewSrcData( bpp, pp );
//      TAYA_Printf("%d番目のポケモンはイリュージョン。対象は%d番目(monsno=%d)\n",
//                  i, illusionTargetIdx[i], PP_Get(pp, ID_PARA_monsno, 0)  );
    }
  }
}

static void PokeCon_RefrectBtlPartyStartOrder( BTL_POKE_CONTAINER* pokecon, BTL_MAIN_MODULE* wk, u8 clientID )
{
  POKEPARTY* srcParty = PokeCon_GetSrcParty( pokecon, clientID );
  BTL_PARTY* btlParty = BTL_POKECON_GetPartyData( pokecon, clientID );
  u32 memberCount = PokeParty_GetPokeCount( srcParty );
  const POKEMON_PARAM* pp;
  BTL_POKEPARAM* bpp;
  u32 orgPokeID, i, j;

  PokeParty_InitWork( wk->tmpParty );
  orgPokeID = ClientBasePokeID[ clientID ];

  for(i=0; i<memberCount; ++i)
  {
    for(j=0; j<memberCount; ++j)
    {
      bpp = BTL_PARTY_GetMemberData( btlParty, j );
      if( BPP_GetID(bpp) == (orgPokeID+i) )
      {
        BPP_ReflectToPP( bpp, TRUE );
        pp = BPP_GetSrcData( bpp );
        PokeParty_Add( wk->tmpParty, pp );

        if( BPP_GetBtlInFlag(bpp) )
        {
          wk->setupParam->fightPokeIndex[ i ] = TRUE;
        }
        break;
      }
    }
  }

  PokeParty_Copy( wk->tmpParty, srcParty );
}
static POKEPARTY* PokeCon_GetSrcParty( BTL_POKE_CONTAINER* pokecon, u8 clientID )
{
  return pokecon->srcParty[ clientID ];
}

//----------------------------------------------------------------------------------
/**
 * 指定ポケモンのパーティ内位置を返す
 *
 * @param   pokecon
 * @param   clientID
 * @param   pokeID
 *
 * @retval  int
 */
//----------------------------------------------------------------------------------
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
 * @retval  const BTL_POKEPARAM*（パーティのメンバーが足りない場合はNULL）
 */
//=============================================================================================
const BTL_POKEPARAM* BTL_POKECON_GetFrontPokeDataConst( const BTL_POKE_CONTAINER* wk, BtlPokePos pos )
{
  const BTL_PARTY* party;
  u8 clientID, posIdx;

//  BTL_N_Printf( DBGSTR_MAIN_GetFrontPokeData, pos);

  btlPos_to_cliendID_and_posIdx( wk->mainModule, pos, &clientID, &posIdx );
  party = &wk->party[ clientID ];
  if( posIdx < BTL_PARTY_GetMemberCount(party) ){
//    BTL_N_PrintfSimple( DBGSTR_MAIN_GetFrontPokeDataResult, clientID, posIdx );
    return BTL_PARTY_GetMemberDataConst( party, posIdx );
  }else{
    BTL_N_PrintfSimple( DBGSTR_MAIN_PokeConGetByPos, pos, clientID, posIdx );
    return NULL;
  }
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
 * ポケモンボイスパラメータを設定
 *
 * @param   wk
 * @param   pRef
 *
 * @retval  BOOL    正しく設定できた場合（ペラップ録音データがある場合）TRUE
 */
//=============================================================================================
BOOL BTL_MAIN_SetPmvRef( const BTL_MAIN_MODULE* wk, BtlvMcssPos vpos, PMV_REF* pRef )
{
  BtlPokePos pos = BTL_MAIN_ViewPosToBtlPos( wk, vpos );
  u8 clientID = BTL_MAIN_BtlPosToClientID( wk, pos );

  if( wk->perappVoice[ clientID ] ){
    pRef->perapVoice = wk->perappVoice[ clientID ];
    return TRUE;
  }

  return FALSE;
}
//=============================================================================================
/**
 * ペラップボイスランク取得
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  BOOL
 */
//=============================================================================================
u32 BTL_MAIN_GetPerappVoicePower( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  if( wk->perappVoice[ clientID ] )
  {
    if( PERAPVOICE_GetExistFlag(wk->perappVoice[clientID]) )
    {
      return PERAPVOICE_GetWazaParam( wk->perappVoice[clientID] );
    }
  }
  return 0;
}


//=============================================================================================
/**
 * 自分がサーバマシンか判定
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_CheckImServerMachine( const BTL_MAIN_MODULE* wk )
{
  return wk->ImServer;
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
u8 BTL_MAIN_GetClientFrontPosCount( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  return CheckNumFrontPos( wk, clientID );
}
//=============================================================================================
/**
 * 指定クライアントの控えポケモンIndex開始値を返す
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_MAIN_GetClientBenchPosIndex( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  u8 index = CheckNumFrontPos( wk, clientID );
  if( BTL_MAIN_GetRule(wk) == BTL_RULE_ROTATION )
  {
    index += BTL_ROTATION_BACKPOS_NUM;
  }
  return index;
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
//-------------------------------------------------------------------
/**
 *  トリプルバトル時にセンター位置かどうかを判定
 */
//-------------------------------------------------------------------
BOOL BTL_MAINUTIL_IsTripleCenterPos( BtlPokePos pos )
{
  return (pos == BTL_POS_1ST_1) || (pos == BTL_POS_2ND_1);
}
//-------------------------------------------------------------------
/**
 *  トリプルバトル時に相手の反対端にあたる位置を取得
 *
 * @param   myPos   自分位置
 * @param   farPos  [out] 相手の反対端にあたる位置を格納する変数ポインタ
 *
 * @retval  BOOL    取得できたらTRUE／取得できない（自分位置がCenter）場合 FALSE
 */
//-------------------------------------------------------------------
BOOL BTL_MAINUTIL_GetTripleFarPos( BtlPokePos myPos, BtlPokePos* farPos )
{
  if( !BTL_MAINUTIL_IsTripleCenterPos(myPos) )
  {
    u8 myPosIdx = btlPos_to_sidePosIdx( myPos );
    *farPos = BTL_MAINUTIL_GetOpponentPokePos( BTL_RULE_TRIPLE, myPos, myPosIdx );
    return TRUE;
  }
  return FALSE;
}
//-------------------------------------------------------------------
/**
 * 味方同士のポケモンIDかどうかを判定する
 *
 * @param   pokeID1
 * @param   pokeID2
 *
 * @retval  BOOL    味方同士ならTRUE
 */
//-------------------------------------------------------------------
BOOL BTL_MAINUTIL_IsFriendPokeID( u8 pokeID1, u8 pokeID2 )
{
  BtlSide side1 = BTL_MAINUTIL_PokeIDtoSide( pokeID1 );
  BtlSide side2 = BTL_MAINUTIL_PokeIDtoSide( pokeID2 );
  return side1 == side2;
}
//-------------------------------------------------------------------
/**
 *ポケモンID -> サイドID変換
 *
 * @param   pokeID
 *
 * @retval  BtlSide
 */
//-------------------------------------------------------------------
BtlSide BTL_MAINUTIL_PokeIDtoSide( u8 pokeID )
{
  return (pokeID < TEMOTI_POKEMAX*2)? BTL_SIDE_1ST : BTL_SIDE_2ND;
}
//-------------------------------------------------------------------
/**
 * ポケモンID -> 敵方サイドID変換
 *
 * @param   pokeID
 *
 * @retval  BtlSide
 */
//-------------------------------------------------------------------
BtlSide BTL_MAINUTIL_PokeIDtoOpponentSide( u8 pokeID )
{
  BtlSide side = BTL_MAINUTIL_PokeIDtoSide( pokeID );
  return BTL_MAINUTIL_GetOpponentSide( side );
}
//-------------------------------------------------------------------
/**
 *  敵型サイドID変換
 */
//-------------------------------------------------------------------
BtlSide BTL_MAINUTIL_GetOpponentSide( BtlSide side )
{
  GF_ASSERT(side < BTL_SIDE_MAX);
  return !side;
}
//-------------------------------------------------------------------
/**
 * トリプルバトル時の攻撃可能エリア判定
 *
 * @param   attackerPos
 * @param   targetPos
 *
 * @retval  BOOL
 */
//-------------------------------------------------------------------
BOOL BTL_MAINUTIL_CheckTripleHitArea( BtlPokePos attackerPos, BtlPokePos targetPos )
{
  const BTL_TRIPLE_ATTACK_AREA* area = BTL_MAINUTIL_GetTripleAttackArea( attackerPos );
  u32 i;

  for(i=0; i<area->numEnemys; ++i)
  {
    if( area->enemyPos[i] == targetPos ){ return TRUE; }
  }
  for(i=0; i<area->numFriends; ++i)
  {
    if( area->friendPos[i] == targetPos ){ return TRUE; }
  }

  return FALSE;
}
//-------------------------------------------------------------------
/**
 *  ローテーション回転方向 -> 新しく入ってくるポケモンの元位置Index取得
 */
//-------------------------------------------------------------------
u8 BTL_MAINUTIL_GetRotateInPosIdx( BtlRotateDir dir )
{
  switch( dir ){
  case BTL_ROTATEDIR_R:  return 1;
  case BTL_ROTATEDIR_L:  return 2;
  default:
    return 0;
  }
}
//-------------------------------------------------------------------
/**
 *  ローテーション回転方向 -> 出て行く先のポケモンの位置Index取得
 */
//-------------------------------------------------------------------
u8 BTL_MAINUTIL_GetRotateOutPosIdx( BtlRotateDir dir )
{
  switch( dir ){
  case BTL_ROTATEDIR_R:  return 2;
  case BTL_ROTATEDIR_L:  return 1;
  default:
    return 0;
  }
}
//-------------------------------------------------------------------
/**
 * 指定サイドの位置ID取得
 */
//-------------------------------------------------------------------
BtlPokePos BTL_MAINUTIL_GetSidePos( BtlSide side, u8 idx )
{
  GF_ASSERT(side < BTL_SIDE_MAX);
  GF_ASSERT(idx < BTL_POSIDX_MAX);

  return (side&1) + idx*2;
}
//-------------------------------------------------------------------
/**
 *  位置ID->サイドID
 */
//-------------------------------------------------------------------
BtlSide BTL_MAINUTIL_PosToSide( BtlPokePos pos )
{
  return pos & 1;
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

//=============================================================================================
/**
 * クライアント指定で戦えるポケモン数を取得
 *
 * @param   wk
 * @param   clientID
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_POKECON_GetClientAlivePokeCount( const BTL_POKE_CONTAINER* pokeCon, u8 clientID )
{
  const BTL_PARTY* party = BTL_POKECON_GetPartyDataConst( pokeCon, clientID );
  return BTL_PARTY_GetAliveMemberCount( party );
}


//=======================================================================================================
// BTL_PARTY
//=======================================================================================================

/**
 * 構造体初期化
 */
static void BTL_PARTY_Initialize( BTL_PARTY* party )
{
  int i;
  party->memberCount = 0;
  for(i=0; i<TEMOTI_POKEMAX; i++)
  {
    party->member[i] = NULL;
  }
}
/**
 * メンバー１体追加（初期化後に手持ちメンバー数分だけ行う）
 */
static void BTL_PARTY_AddMember( BTL_PARTY* party, BTL_POKEPARAM* member )
{
  GF_ASSERT(party->memberCount < TEMOTI_POKEMAX);
  party->member[party->memberCount++] = member;
}
/**
 * 戦える（生きていてタマゴでない）ポケモンを先頭に移動（初期化後に１度だけ行う）
 */
static void   BTL_PARTY_MoveAlivePokeToFirst( BTL_PARTY* party )
{
  u32 idx, checkedCount;

  idx = checkedCount = 0;
  while( checkedCount < party->memberCount )
  {
    if( BPP_IsFightEnable(party->member[idx]) ){
      ++idx;
    }
    else{
      BTL_PARTY_MoveLastMember( party, idx );
    }
    ++checkedCount;
  }
}
/**
 * メンバー数を返す
 *
 * @param   party
 *
 * @retval  u8
 */
u8 BTL_PARTY_GetMemberCount( const BTL_PARTY* party )
{
  return party->memberCount;
}
/**
 *  戦えるメンバー数を返す
 *
 * @param   party
 *
 * @retval  u8
 */
u8 BTL_PARTY_GetAliveMemberCount( const BTL_PARTY* party )
{
  int i, cnt;

  for(i=0, cnt=0; i<party->memberCount; i++)
  {
    if( BPP_IsFightEnable(party->member[i]) ){
      ++cnt;
    }
  }
  return cnt;
}
/**
 * 指定Index以降の戦えるメンバー数を返す
 *
 * @param   party
 *
 * @retval  u8
 */
u8 BTL_PARTY_GetAliveMemberCountRear( const BTL_PARTY* party, u8 startIdx )
{
  int i, cnt;
  for(i=startIdx, cnt=0; i<party->memberCount; i++)
  {
    if( BPP_IsFightEnable(party->member[i]) ){
      ++cnt;
    }
  }
  return cnt;
}
/**
 * メンバーIndex からメンバーデータポインタ取得
 *
 * @param   party
 * @param   idx
 *
 * @retval  BTL_POKEPARAM*
 */
BTL_POKEPARAM* BTL_PARTY_GetMemberData( BTL_PARTY* party, u8 idx )
{
  if( idx < party->memberCount ){
    return party->member[ idx ];
  }
  return NULL;
}
/**
 * メンバーIndex からメンバーデータポインタ取得（const版）
 *
 * @param   party
 * @param   idx
 *
 * @retval  BTL_POKEPARAM*
 */
const BTL_POKEPARAM* BTL_PARTY_GetMemberDataConst( const BTL_PARTY* party, u8 idx )
{
  if( idx < party->memberCount ){
    return party->member[ idx ];
  }
  return NULL;
}
/**
 * メンバー入れ替え
 *
 * @param   party
 * @param   idx1
 * @param   idx2
 */
void BTL_PARTY_SwapMembers( BTL_PARTY* party, u8 idx1, u8 idx2 )
{
  GF_ASSERT(idx1<party->memberCount);
  GF_ASSERT(idx2<party->memberCount);
  {
    BTL_POKEPARAM* tmp = party->member[ idx1 ];
    party->member[ idx1 ] = party->member[ idx2 ];
    party->member[ idx2 ] = tmp;

    BTL_N_PrintfEx(TRUE, DBGSTR_MAIN_SwapPartyMember, idx1, idx2);
  }
}
/**
 *  メンバーを最後尾に移動（初期化時に戦えないポケモンに適用)
 */
void BTL_PARTY_MoveLastMember( BTL_PARTY* party, u8 idx )
{
  GF_ASSERT(idx<party->memberCount);

  {
    BTL_POKEPARAM* tmp = party->member[ idx ];
    while( idx < (party->memberCount-1) )
    {
      party->member[ idx ] = party->member[ idx + 1 ];
      ++idx;
    }
    party->member[ idx ] = tmp;
  }

}
/**
 * メンバーローテーション
 *
 * @param   party
 * @param   dir
 * @param   outPoke
 * @param   inPoke
 */
void BTL_PARTY_RotateMembers( BTL_PARTY* party, BtlRotateDir dir, BTL_POKEPARAM** outPoke, BTL_POKEPARAM** inPoke )
{
  GF_ASSERT(party->memberCount>=3);

  #ifdef ROTATION_NEW_SYSTEM
  if( (dir!=BTL_ROTATEDIR_NONE) && (dir!=BTL_ROTATEDIR_STAY) )
  {
    BTL_POKEPARAM* prev_top_poke = party->member[0];
    u8  inIdx, outIdx;

    inIdx  = BTL_MAINUTIL_GetRotateInPosIdx( dir );
    outIdx = BTL_MAINUTIL_GetRotateOutPosIdx( dir );

    party->member[ 0 ]      = party->member[ inIdx ];
    party->member[ inIdx ]  = party->member[ outIdx ];
    party->member[ outIdx ] = prev_top_poke;

    if( outPoke != NULL ){
      *outPoke = prev_top_poke;
    }
    if( inPoke != NULL ){
      *inPoke = party->member[0];
    }

  }
  #else
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
  #endif
}
/**
 * 特定ポケモンのパーティ内Indexを返す（データポインタ指定）
 *
 * @param   party
 * @param   pokeID
 *
 * @retval  int   パーティ内Index（パーティに指定IDのポケモンが存在しない場合 -1）
 */
int BTL_PARTY_FindMember( const BTL_PARTY* party, const BTL_POKEPARAM* param )
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
/**
 * 特定ポケモンのパーティ内Indexを返す（PokeID指定）
 *
 * @param   party
 * @param   pokeID
 *
 * @retval  int   パーティ内Index（パーティに指定IDのポケモンが存在しない場合 -1）
 */
int BTL_PARTY_FindMemberByPokeID( const BTL_PARTY* party, u8 pokeID )
{
  int i;
  for(i=0; i<party->memberCount; i++)
  {
    if( BPP_GetID(party->member[i]) == pokeID ){
      return i;
    }
  }
  return -1;
}
/**
 * 生きていて戦える先頭のポケモンデータを返す
 *
 * @param   party
 *
 * @retval  int   パーティ内Index（存在しない場合 NULL）
 */
BTL_POKEPARAM* BTL_PARTY_GetAliveTopMember( BTL_PARTY* party )
{
  int i;

  for(i=0; i<party->memberCount; ++i)
  {
    if( BPP_IsFightEnable(party->member[i]) ){
      return party->member[i];
    }
  }
  return NULL;
}

//----------------------------------------------------------------------
/**
 * とくせい「イリュージョン」を持つメンバーの参照ポケデータ更新
 *
 * @param   party
 */
//----------------------------------------------------------------------
void BTL_MAIN_SetIllusionForParty( const BTL_MAIN_MODULE* wk, BTL_PARTY* party, u8 clientID )
{
  int lastPokeIdx, visiblePosCnt, i;

  // 最後尾にいて生きてるポケがイリュージョン対象
  for(lastPokeIdx=(party->memberCount-1); lastPokeIdx>0; --lastPokeIdx)
  {
    if(BPP_IsFightEnable(party->member[lastPokeIdx])){
      break;
    }
  }

  visiblePosCnt = (BTL_MAIN_GetRule(wk) != BTL_RULE_ROTATION)?
        GetClientCoverPosNum(wk,clientID) : BTL_ROTATION_VISIBLE_POS_NUM;

  TAYA_Printf("見えている開始位置(%d)まではイリュージョンにしません\n", visiblePosCnt);

  for(i=visiblePosCnt; i<party->memberCount; ++i)
  {
    BTL_POKEPARAM* bpp = party->member[ i ];
    // イリュージョン持ちなら対象を更新
    if( BPP_GetValue(bpp, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_IRYUUJON )
    {
      if( i < lastPokeIdx )
      {
        BTL_POKEPARAM* bppRef = party->member[ lastPokeIdx ];
        BPP_SetViewSrcData( bpp, BPP_GetSrcData(bppRef) );
        BTL_N_Printf( DBGSTR_MAIN_Illusion1st, i, BPP_GetID(bpp));
        BTL_N_PrintfSimple( DBGSTR_MAIN_Illusion2nd, lastPokeIdx, BPP_GetID(bppRef));
      }
      // 自身が最後尾にいる場合、イリュージョンは無効（ただし野生ゾロアークの特殊処理は除外）
      else if( BPP_GetViewSrcData(bpp) != wk->ppIllusionZoroArc )
      {
         BPP_ClearViewSrcData( bpp );
      }
    }
  }

  for(i=0; i<party->memberCount; ++i)
  {
    BTL_POKEPARAM* bpp = party->member[ i ];
    if( BPP_IsFakeEnable(bpp) )
    {
      TAYA_Printf("Member[%d], monsno=%d ... Illusion\n", i, BPP_GetMonsNo(bpp));
    }
    else{
      TAYA_Printf("Member[%d], monsno=%d ... normal\n", i, BPP_GetMonsNo(bpp));
    }
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
    wk->trainerParam[i].ai_bit = 0;
  }
  for(i=0; i<NELEMS(wk->perappVoice); ++i){
    wk->perappVoice[i] = NULL;
  }
}
static void trainerParam_Clear( BTL_MAIN_MODULE* wk )
{
  u32 i;
  for(i=0; i<NELEMS(wk->trainerParam); ++i)
  {
    if( wk->trainerParam[i].playerStatus ){
      GFL_HEAP_FreeMemory( wk->trainerParam[i].playerStatus );
      wk->trainerParam[i].playerStatus = NULL;
    }

    if( wk->trainerParam[i].name ){
      GFL_HEAP_FreeMemory( wk->trainerParam[i].name );
      wk->trainerParam[i].name = NULL;
    }
  }
  for(i=0; i<NELEMS(wk->perappVoice); ++i)
  {
    if( wk->perappVoice[i] != NULL ){
      GFL_HEAP_FreeMemory( wk->perappVoice[i] );
      wk->perappVoice[i] = NULL;
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

  PMSDAT_Clear( &dst->win_word );
  PMSDAT_Clear( &dst->lose_word );
}
static void trainerParam_StoreNPCTrainer( BTL_TRAINER_DATA* dst, const BSP_TRAINER_DATA* trData )
{
  dst->playerStatus = NULL;

  if( trData )
  {
    dst->trainerID = trData->tr_id;
    dst->trainerType = trData->tr_type;
    dst->name = GFL_STR_CreateCopyBuffer( trData->name, HEAPID_BTL_SYSTEM );
    dst->ai_bit = trData->ai_bit;
    GFL_STD_MemCopy( trData->use_item, dst->useItem, sizeof(trData->use_item) );

    PMSDAT_Copy( &dst->win_word, &trData->win_word );
    PMSDAT_Copy( &dst->lose_word, &trData->lose_word );
  }
  else{
    u32 i;
    dst->trainerID = TRID_NULL;
    dst->trainerType = 0;
    dst->name = NULL;
    for(i=0; i<NELEMS(dst->useItem); ++i){
      dst->useItem[i] = ITEM_DUMMY_DATA;
    }
    PMSDAT_Clear( &dst->win_word );
    PMSDAT_Clear( &dst->lose_word );
  }
}
static void trainerParam_StoreCommNPCTrainer( BTL_TRAINER_DATA* dst, const BTL_TRAINER_SEND_DATA* trSendData )
{

  dst->playerStatus = NULL;

  if( trSendData )
  {
    const BSP_TRAINER_DATA* trData =  &trSendData->base_data;

    dst->trainerID = trData->tr_id;
    dst->trainerType = trData->tr_type;
    dst->name = GFL_STR_CreateBuffer( BTL_COMM_TRAINERNAME_MAX, HEAPID_BTL_SYSTEM );
    GFL_STR_SetEncodedString( dst->name, trSendData->trainer_name, trSendData->trainer_name_length+1 );
    dst->ai_bit = trData->ai_bit;
    GFL_STD_MemCopy( trData->use_item, dst->useItem, sizeof(trData->use_item) );

    PMSDAT_Copy( &dst->win_word, &trData->win_word );
    PMSDAT_Copy( &dst->lose_word, &trData->lose_word );
  }
  else{
    u32 i;
    dst->trainerID = TRID_NULL;
    dst->trainerType = 0;
    dst->name = NULL;
    for(i=0; i<NELEMS(dst->useItem); ++i){
      dst->useItem[i] = ITEM_DUMMY_DATA;
    }
    PMSDAT_Clear( &dst->win_word );
    PMSDAT_Clear( &dst->lose_word );
  }
}

BOOL BTL_MAIN_IsClientNPC( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  return ( wk->trainerParam[clientID].playerStatus == NULL );
}
u16 BTL_MAIN_GetClientUseItem( const BTL_MAIN_MODULE* wk, u8 clientID, u8 itemIdx )
{
  if( BTL_MAIN_IsClientNPC(wk, clientID) )
  {
    const BTL_TRAINER_DATA* trData = &wk->trainerParam[clientID];
    if( itemIdx < NELEMS(trData->useItem) ){
      return trData->useItem[ itemIdx ];
    }
  }
  return ITEM_DUMMY_DATA;
}

const STRBUF* BTL_MAIN_GetClientTrainerName( const BTL_MAIN_MODULE* wk, u8 clientID, u32* tr_type )
{
  *tr_type = wk->trainerParam[clientID].trainerType;
  return wk->trainerParam[clientID].name;
}
u32 BTL_MAIN_GetClientAIBit( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  if( BTL_MAIN_GetCompetitor(wk) == BTL_COMPETITOR_WILD )
  {
    if( BTL_MAIN_GetSetupStatusFlag(wk, BTL_STATUS_FLAG_MOVE_POKE) ){
      return AI_THINK_BIT_MV_POKE;
    }
    else if( BTL_MAIN_GetRule(wk) == BTL_RULE_DOUBLE )
    {
      return AI_THINK_BIT_DOUBLE;
    }
  }
  else if( BTL_MAIN_IsClientNPC(wk, clientID) )
  {
    return wk->trainerParam[ clientID ].ai_bit;
  }
  return 0x00;
}

// string x 2
u32 BTL_MAIN_GetClientTrainerID( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  if( BTL_MAIN_IsClientNPC(wk, clientID) ){
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

const PMS_DATA* BTL_MAIN_GetClientPMSData( const BTL_MAIN_MODULE* wk, u8 clientID, BtlResult playerResult )
{
  if( (BTL_MAIN_GetCompetitor(wk) == BTL_COMPETITOR_SUBWAY)
  &&  BTL_MAIN_IsClientNPC(wk, clientID)
  ){
    if( playerResult == BTL_RESULT_WIN ){
      return &wk->trainerParam[ clientID ].lose_word;
    }
    else{
      return &wk->trainerParam[ clientID ].win_word;
    }
  }
  return NULL;

}

const MYSTATUS* BTL_MAIN_GetClientPlayerData( const BTL_MAIN_MODULE* wk, u8 clientID )
{
  if( !BTL_MAIN_IsClientNPC(wk, clientID) )
  {
    return wk->trainerParam[ clientID ].playerStatus;
  }
  GF_ASSERT_MSG(0, "This Client(%d) is not Human Player\n", clientID);
  return NULL;
}
/**
 *  通信サポートプレイヤーのMyStatusを取得
 */
const MYSTATUS* BTL_MAIN_GetCommSuppoortPlayerData( const BTL_MAIN_MODULE* wk )
{
  return COMM_PLAYER_SUPPORT_GetMyStatus( wk->setupParam->commSupport );
}

//----------------------------------------------------------------------------------------------
// オリジナルパーティデータ管理
//----------------------------------------------------------------------------------------------
// 管理ワーク初期化
static void srcParty_Init( BTL_MAIN_MODULE* wk )
{
  u32 i;
  for(i=0; i<NELEMS(wk->srcParty); ++i)
  {
    wk->srcParty[i] = PokeParty_AllocPartyWork( HEAPID_BTL_SYSTEM );
    wk->srcPartyForServer[i] = PokeParty_AllocPartyWork( HEAPID_BTL_SYSTEM );
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
    if( wk->srcPartyForServer[i] ){
      GFL_HEAP_FreeMemory( wk->srcPartyForServer[i] );
      wk->srcPartyForServer[i] = NULL;
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
  if( wk->srcParty[clientID] != NULL )
  {
    PokeParty_InitWork( wk->srcParty[clientID] );
    PokeParty_InitWork( wk->srcPartyForServer[clientID] );
    PokeParty_Copy( party, wk->srcParty[clientID] );
    PokeParty_Copy( party, wk->srcPartyForServer[clientID] );

    if( PokeParty_GetPokeCount(wk->srcPartyForServer[clientID]) )
    {
      POKEMON_PARAM* pp = PokeParty_GetMemberPointer( wk->srcPartyForServer[clientID], 0 );
      TAYA_Printf("SrcParty[%d] Set 1stPoke Adrs=%p, PP=%d\n", clientID, pp, PP_Get(pp, ID_PARA_pp1, NULL) );
    }
  }
}
// パーティデータ取得
static POKEPARTY* srcParty_Get( BTL_MAIN_MODULE* wk, u8 clientID, BOOL fForServer )
{
  GF_ASSERT( wk->srcParty[clientID] );

  if( fForServer )
  {
    if( PokeParty_GetPokeCount(wk->srcPartyForServer[clientID]) )
    {
      POKEMON_PARAM* pp = PokeParty_GetMemberPointer( wk->srcPartyForServer[clientID], 0 );
      TAYA_Printf("SrcParty[%d] Get 1stPoke Adrs=%p, PP=%d\n", clientID, pp, PP_Get(pp, ID_PARA_pp1, NULL) );
    }

    return wk->srcPartyForServer[ clientID ];
  }else{
    return wk->srcParty[ clientID ];
  }
}

//=============================================================================================
/**
 * セットアップパラメータのステータスフラグを取得
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_GetSetupStatusFlag( const BTL_MAIN_MODULE* wk, BTL_STATUS_FLAG flag )
{
  return BATTLE_PARAM_CheckBtlStatusFlag( wk->setupParam, flag );
}

//=============================================================================================
/**
 * 記録データインクリメント
 *
 * @param   wk
 * @param   recID
 */
//=============================================================================================
void BTL_MAIN_RECORDDATA_Inc( const BTL_MAIN_MODULE* wk, int recID )
{
  if( !(wk->setupParam->fRecordPlay) ){
    RECORD_Inc( (RECORD*)(wk->setupParam->recordData), recID );
  }
}
//=============================================================================================
/**
 * 記録データ加算
 *
 * @param   wk
 * @param   recID
 * @param   value
 */
//=============================================================================================
void BTL_MAIN_RECORDDATA_Add( const BTL_MAIN_MODULE* wk, int recID, u32 value )
{
  if( !(wk->setupParam->fRecordPlay) ){
    RECORD_Add( (RECORD*)(wk->setupParam->recordData), recID, value );
  }
}

//=============================================================================================
/**
 *
 *
 * @param   wk
 *
 * @retval  const SHOOTER_ITEM_BIT_WORK*
 */
//=============================================================================================
const SHOOTER_ITEM_BIT_WORK* BTL_MAIN_GetSetupShooterBit( const BTL_MAIN_MODULE* wk )
{
  return &(wk->setupParam->shooterBitWork);
}
//=============================================================================================
/**
 *
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsShooterEnable( const BTL_MAIN_MODULE* wk )
{
  BOOL ret = FALSE;
  if( checkBagMode(wk->setupParam) == BBAG_MODE_SHOOTER )
  {
    const SHOOTER_ITEM_BIT_WORK* shooterBitWork = &(wk->setupParam->shooterBitWork);
    ret = (shooterBitWork->shooter_use != 0);
  }
  return ret;
}
//=============================================================================================
/**
 * アイテム使用できるかどうか判定（使用できないモードならUI表示が暗くなり反応しない）
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_MAIN_IsItemEnable( const BTL_MAIN_MODULE* wk )
{
  if( checkBagMode(wk->setupParam) == BBAG_MODE_SHOOTER )
  {
    return BTL_MAIN_IsShooterEnable( wk );
  }
  else
  {
    BtlCompetitor cometitor = BTL_MAIN_GetCompetitor( wk );
    return (cometitor==BTL_COMPETITOR_WILD) || (cometitor==BTL_COMPETITOR_TRAINER);
  }
}

//----------------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------------


//=============================================================================================
/**
 * ポケモンレベルアップ通知をクライアントから受け取り、必要な処理を行う
 *
 * ->ゲームシステム（ビーコン）に通知
 * ->なつき度計算
 *
 * @param   wk
 */
//=============================================================================================
void BTL_MAIN_NotifyPokemonLevelup( BTL_MAIN_MODULE* wk, BTL_POKEPARAM* bpp )
{
  GF_ASSERT(wk->setupParam->competitor != BTL_COMPETITOR_COMM);
  GF_ASSERT(wk->setupParam->competitor != BTL_COMPETITOR_SUBWAY);
  {
    // レベルアップでなつき度上昇
    NatsukiPut( wk, bpp, CALC_NATSUKI_LEVELUP );

    // ビーコン送信
    BTL_SERVER_NotifyPokemonLevelUp( wk->server, bpp );
  }
}
//=============================================================================================
/**
 * アイテム使用によるなつき度上昇処理
 *
 * @param   wk
 * @param   bpp
 * @param   itemNo
 */
//=============================================================================================
void BTL_MAIN_CalcNatsukiItemUse( const BTL_MAIN_MODULE* wk, BTL_POKEPARAM* bpp, u16 itemNo )
{
  u8 pokeID = BPP_GetID( bpp );

  const BTL_POKEPARAM* bppServer = BTL_POKECON_GetPokeParamConst( &wk->pokeconForServer, pokeID );
  const BTL_POKEPARAM* bppClient = BTL_POKECON_GetPokeParamConst( &wk->pokeconForClient, pokeID );

  POKEMON_PARAM* ppServer = (POKEMON_PARAM*)BPP_GetSrcData( bppServer );
  POKEMON_PARAM* ppClient = (POKEMON_PARAM*)BPP_GetSrcData( bppClient );

  const BTL_FIELD_SITUATION* sit = BTL_MAIN_GetFieldSituation( wk );

  NATSUKI_CalcUseItem( ppServer, itemNo, sit->zoneID, GFL_HEAP_LOWID(wk->heapID) );
  NATSUKI_CalcUseItem( ppClient, itemNo, sit->zoneID, GFL_HEAP_LOWID(wk->heapID) );
}
//=============================================================================================
/**
 * ポケモン捕獲通知をクライアントから受け取り、ゲームシステム（レコード、ビーコン）に通知
 *
 * @param   wk
 * @param   bpp
 */
//=============================================================================================
void BTL_MAIN_NotifyPokemonGetToGameSystem( BTL_MAIN_MODULE* wk, const BTL_POKEPARAM* bpp )
{
  if( wk->server )
  {
    BTL_SERVER_NotifyPokemonCapture( wk->server, bpp );
  }
}


//----------------------------------------------------------------------------------
/**
 * バトル内パーティデータを、結果としてセットアップパラメータに書き戻す
 * ※バトル終了時
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void reflectPartyData( BTL_MAIN_MODULE* wk )
{
  // 録画再生時は何もしない
  if( wk->setupParam->fRecordPlay ){
    return;
  }

  // 野生or通常トレーナー戦：経験値・レベルアップ・なつき度を反映
  if( (wk->setupParam->competitor == BTL_COMPETITOR_WILD)
  ||  (wk->setupParam->competitor == BTL_COMPETITOR_TRAINER)
  ){
    POKEPARTY* srcParty;

    PokeCon_RefrectBtlPartyStartOrder( &wk->pokeconForServer, wk, wk->myClientID );

    srcParty = PokeCon_GetSrcParty( &wk->pokeconForServer, wk->myClientID );

    // ジムリーダー、四天王、チャンピオンならなつき度アップ
    if( wk->setupParam->competitor == BTL_COMPETITOR_TRAINER )
    {
      u32 trType = BTL_MAIN_GetClientTrainerType( wk, BTL_CLIENT_ENEMY1 );
      if( BTL_CALC_IsTrTypeBoss(trType) )
      {
        const BTL_FIELD_SITUATION* sit = BTL_MAIN_GetFieldSituation( wk );
        NATSUKI_CalcBossBattle( srcParty, sit->zoneID, GFL_HEAP_LOWID(wk->heapID) );
      }
    }

    // トレーナー戦は装備アイテムを元に戻す
    if( (wk->setupParam->competitor == BTL_COMPETITOR_TRAINER) )
    {
      POKEMON_PARAM *ppOrg, *ppResult;
      u32 i, pokeCnt = PokeParty_GetPokeCount( srcParty );
      u16 OrgItemNo;
      for(i=0; i<pokeCnt; ++i)
      {
        ppOrg = PokeParty_GetMemberPointer( wk->setupParam->party[ BTL_CLIENT_PLAYER ], i );
        ppResult = PokeParty_GetMemberPointer( srcParty, i );
        OrgItemNo = PP_Get( ppOrg, ID_PARA_item, NULL );
        // 消費アイテムでなければ、ただ元に戻すだけ
        if( !BTL_CALC_ITEM_GetParam(OrgItemNo, ITEM_PRM_ITEM_SPEND) ){
          PP_Put( ppResult, ID_PARA_item, OrgItemNo );
        // 消費アイテムであれば、他のアイテムに変わっていた場合に消す
        }else if( PP_Get(ppResult, ID_PARA_item, NULL) != OrgItemNo ){
          PP_Put( ppResult, ID_PARA_item, ITEM_DUMMY_DATA );
        }
      }
    }

    PokeParty_Copy( srcParty, wk->setupParam->party[ BTL_CLIENT_PLAYER ] );
    {
      u32 i;
      POKEMON_PARAM* pp;
      for(i=0; i<PokeParty_GetPokeCount(srcParty); ++i){
        pp = PokeParty_GetMemberPointer(srcParty, i);
      }
    }
  }

  // 野生戦：捕獲ポケモン情報
  if( wk->setupParam->competitor == BTL_COMPETITOR_WILD )
  {
    u8 clientID = BTL_MAIN_GetOpponentClientID( wk, wk->myClientID, 0 );
    POKEPARTY* srcParty;

    PokeCon_RefrectBtlParty( &wk->pokeconForServer, wk, clientID, TRUE );
    srcParty = PokeCon_GetSrcParty( &wk->pokeconForServer, clientID );
    PokeParty_Copy( srcParty, wk->setupParam->party[ BTL_CLIENT_ENEMY1 ] );
  }

  // デモ以外全て：状態異常コード書き戻し
  if( wk->setupParam->competitor != BTL_COMPETITOR_DEMO_CAPTURE )
  {
    u32 p, id, clientID;
    for(id=0; id<BTL_CLIENT_MAX; ++id)
    {
      clientID = CommClientRelation( wk->myClientID, id );
      for(p=0; p<TEMOTI_POKEMAX; ++p){
        wk->setupParam->party_state[ clientID ][ p ] = BTL_POKESTATE_NORMAL;
      }

//      TAYA_Printf("myClientID=%d, idx=%d , clientID=%d ... \n", wk->myClientID, id, clientID );

      if( BTL_MAIN_IsExistClient(wk, clientID) )
      {
        BTL_PARTY* party = BTL_POKECON_GetPartyData( &wk->pokeconForClient, clientID );
        u32 numMembers = BTL_PARTY_GetMemberCount( party );
        u32 HPSum, MaxHPSum;
        u8 orgPokeID = ClientBasePokeID[ clientID ];
        u8 idx;

        HPSum = MaxHPSum = 0;
        for(p=0; p<numMembers; ++p)
        {
          BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberData( party, p );
          idx = BPP_GetID(bpp) - orgPokeID;
          HPSum += BPP_GetValue( bpp, BPP_HP );
          MaxHPSum += BPP_GetValue( bpp, BPP_MAX_HP );

          if( BPP_IsDead(bpp) ){
            wk->setupParam->party_state[ clientID ][ idx ] = BTL_POKESTATE_DEAD;
          }
          else if( BPP_GetPokeSick(bpp) != POKESICK_NULL ){
            wk->setupParam->party_state[ clientID ][ idx ] = BTL_POKESTATE_SICK;
          }
        }

        wk->setupParam->restHPRatio[ clientID ] = (MaxHPSum * 100) / HPSum;
      }
    }
  }
}


//----------------------------------------------------------------------------------------------
// 勝敗判定
//----------------------------------------------------------------------------------------------
BOOL BTL_MAIN_IsResultStrictMode( const BTL_MAIN_MODULE* wk )
{
  BtlCompetitor competitor = BTL_MAIN_GetCompetitor( wk );

  return (competitor == BTL_COMPETITOR_SUBWAY) || (competitor == BTL_COMPETITOR_COMM);
}

static BtlResult checkWinner( BTL_MAIN_MODULE* wk )
{
  BtlResult result;

  BTL_N_Printf( DBGSTR_MAIN_CheckResultStart );

  // 捕獲デモ
  if( BTL_MAIN_GetCompetitor(wk) == BTL_COMPETITOR_DEMO_CAPTURE )
  {
    result = BTL_RESULT_WIN;
  }
  // 通信エラー
  else if( wk->fCommError )
  {
    BTL_N_Printf( DBGSTR_MAIN_Result_CommError );
    result = BTL_RESULT_COMM_ERROR;
  }
  // 野生ポケモン捕獲
  else if( wk->setupParam->capturedPokeIdx != TEMOTI_POKEMAX )
  {
    BTL_N_Printf( DBGSTR_MAIN_Result_Capture );
    result = BTL_RESULT_CAPTURE;
  }
  // 逃げて終了
  else if( BTL_ESCAPEINFO_GetCount(&wk->escapeInfo) )
  {
    result = BTL_ESCAPEINFO_CheckWinner( &wk->escapeInfo, wk->myClientID, BTL_MAIN_GetCompetitor(wk) );
    BTL_N_Printf( DBGSTR_MAIN_Result_Escape, result );
  }
  // 通常決着
  else
  {
    if( wk->serverResult == BTL_RESULT_MAX )
    {
      // サーバ計算無効のままここに来ることは本来は有り得ない
      GF_ASSERT(0);
      wk->serverResult = BTL_RESULT_WIN;
    }
    result = wk->serverResult;
    BTL_N_Printf( DBGSTR_MAIN_Result_ServerCalc, result );
  }

  wk->setupParam->result = result;
  return result;
}
//----------------------------------------------------------------------------------------------
// 制限時間関連
//----------------------------------------------------------------------------------------------

/**
 *  コマンド選択の制限時間取得（秒）
 */
u32 BTL_MAIN_GetCommandLimitTime( const BTL_MAIN_MODULE* wk )
{
  return wk->LimitTimeCommand;
}
/**
 *  試合全体の制限時間取得（秒）
 */
u32 BTL_MAIN_GetGameLimitTime( const BTL_MAIN_MODULE * wk )
{
  return wk->LimitTimeGame;
}

/**
 *  試合の制限時間が終了したか判定
 */
BOOL BTL_MAIN_CheckGameLimitTimeOver( const BTL_MAIN_MODULE* wk )
{
  if( wk->LimitTimeGame ){
    return BTL_CLIENT_IsGameTimeOver( wk->client[wk->myClientID] );
  }
  return FALSE;
}

//----------------------------------------------------------------------------------------------
// バトルパラメータへ書き戻し
//----------------------------------------------------------------------------------------------

/**
 *  通信対戦相手のパーティデータを書き戻し
 */
static void Bspstore_Party( BTL_MAIN_MODULE* wk, u8 clientID, const POKEPARTY* party )
{
  u8 relation = CommClientRelation( wk->myClientID, clientID );
  POKEPARTY* dstParty = wk->setupParam->party[ relation ];
  if( dstParty != NULL )
  {
    PokeParty_Copy( party, dstParty );
  }
}
/**
 *  通信対戦相手のプレイヤーデータを書き戻し
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
 *  AIトレーナーデータを書き戻し
 */

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

//=============================================================================================
/**
 * 録画再生リセット（パーティデータ、サーバ初期化など）
 *
 * @param   wk
 */
//=============================================================================================
void BTL_MAIN_ResetForRecPlay( BTL_MAIN_MODULE* wk, u32 nextTurnNum )
{
  u32 i;
  u8 bagMode = checkBagMode( wk->setupParam );

  BTL_CALC_ResetSys( &wk->randomContext );
  setup_common_srcParty( wk, wk->setupParam );

  PokeCon_Clear( &wk->pokeconForClient );
  PokeCon_Clear( &wk->pokeconForServer );

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( BTL_MAIN_IsExistClient(wk, i) )
    {
      PokeCon_AddParty( &wk->pokeconForServer, wk, i );
      PokeCon_AddParty( &wk->pokeconForClient, wk, i );

      BTL_CLIENT_SetChapterSkip( wk->client[i], nextTurnNum );
    }
  }

  BTLV_Delete( wk->viewCore );
  BTLV_ResetSystem( HEAPID_BTL_VIEW );
  wk->viewCore = BTLV_Create( wk, wk->client[wk->setupParam->commPos], &wk->pokeconForClient, bagMode, HEAPID_BTL_VIEW );
  BTL_CLIENT_AttachViewCore( wk->client[wk->setupParam->commPos], wk->viewCore );

  // Server 始動
  BTL_SERVER_Startup( wk->server );

}

void BTL_MAIN_NotifyChapterSkipEnd( BTL_MAIN_MODULE* wk )
{
  u32 i;

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( BTL_MAIN_IsExistClient(wk, i) )
    {
      BTL_CLIENT_StopChapterSkip( wk->client[i] );
    }
  }
}

BTL_SVFLOW_WORK* BTL_MAIN_GetSVFWorkForAI( const BTL_MAIN_MODULE* wk )
{
  GF_ASSERT(wk->server);
  return BTL_SERVER_GetFlowWork( wk->server );
}

//----------------------------------------------------------------------------------------------
// バトル検定データ処理
//----------------------------------------------------------------------------------------------
static void Kentei_ClearField( BATTLE_SETUP_PARAM* sp )
{
  sp->TurnNum = 0;       //かかったターン数
  sp->PokeChgNum = 0;    //交代回数

  sp->VoidAtcNum = 0;    //効果がない技を出した回数
  sp->WeakAtcNum = 0;    //ばつぐんの技を出した回数
  sp->ResistAtcNum = 0;  //いまひとつの技を出した回数
  sp->VoidNum = 0;       //効果がない技を受けた回数
  sp->ResistNum = 0;     //いまひとつの技を受けた回数

  sp->WinTrainerNum = 0; //倒したトレーナー数
  sp->WinPokeNum = 0;    //倒したポケモン数
  sp->LosePokeNum = 0;   //倒されたポケモン数
  sp->UseWazaNum = 0;    //使用した技の数
  sp->HPSum = 0;
}
static void Bspstore_KenteiData( BTL_MAIN_MODULE* wk )
{
  if( wk->server )
  {
    BTL_SVFLOW_WORK* svf = BTL_SERVER_GetFlowWork( wk->server );
    BATTLE_SETUP_PARAM* sp = wk->setupParam;

    sp->TurnNum    = BTL_SVFTOOL_GetTurnCount( svf );
    sp->PokeChgNum = BTL_SVFTOOL_GetPokeChangeCount( svf, BTL_CLIENT_PLAYER );

    sp->VoidAtcNum   = BTL_SVF_GetTypeAffCnt_PutVoid( svf );
    sp->WeakAtcNum   = BTL_SVF_GetTypeAffCnt_PutAdvantage( svf );
    sp->ResistAtcNum = BTL_SVF_GetTypeAffCnt_PutDisadvantage( svf );
    sp->VoidNum      = BTL_SVF_GetTypeAffCnt_RecvVoid( svf );
    sp->ResistNum    = BTL_SVF_GetTypeAffCnt_RecvDisadvantage( svf );

    if( sp->result == BTL_RESULT_WIN ){
      sp->WinTrainerNum = 1;
    }

    {
      BTL_PARTY* party = BTL_POKECON_GetPartyData( &wk->pokeconForServer, BTL_CLIENT_PLAYER );
      u32 memberCnt = BTL_PARTY_GetMemberCount( party );
      u32 aliveCnt = BTL_PARTY_GetAliveMemberCount( party );
      u32 i;
      for(i=0; i<memberCnt; ++i){
        BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberData( party, i );
        sp->UseWazaNum += BPP_WAZA_GetUsedCount( bpp );
        sp->HPSum += BPP_GetValue( bpp, BPP_HP );
      }
      sp->LosePokeNum = (memberCnt - aliveCnt);

      party = BTL_POKECON_GetPartyData( &wk->pokeconForServer, BTL_CLIENT_ENEMY1 );
      memberCnt = BTL_PARTY_GetMemberCount( party );
      aliveCnt = BTL_PARTY_GetAliveMemberCount( party );
      sp->WinPokeNum = (memberCnt - aliveCnt);

      if( BTL_MAIN_IsExistClient(wk, BTL_CLIENT_ENEMY2) )
      {
        party = BTL_POKECON_GetPartyData( &wk->pokeconForServer, BTL_CLIENT_ENEMY2 );
        if( party )
        {
          memberCnt = BTL_PARTY_GetMemberCount( party );
          aliveCnt = BTL_PARTY_GetAliveMemberCount( party );
          sp->WinPokeNum += (memberCnt - aliveCnt);
        }
      }
    }
  }
}


