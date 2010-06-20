//=============================================================================================
/**
 * @file  btl_server.c
 * @brief ポケモンWB バトルシステム サーバモジュール
 * @author  taya
 *
 * @date  2008.09.06  作成
 */
//=============================================================================================
#include <gflib.h>
#include <isdbglib.h>

#include "gamesystem\game_beacon.h"
#include "waza_tool\wazadata.h"
#include "sound\pm_sndsys.h"

#include "btl_common.h"
#include "btl_client.h"
#include "btl_adapter.h"
#include "btl_action.h"
#include "btl_pokeparam.h"
#include "btl_calc.h"
#include "btl_event.h"
#include "btl_util.h"
#include "btl_string.h"
#include "btl_rec.h"

#include "btl_server_cmd.h"
#include "btl_field.h"
#include "btl_server_flow.h"
#include "btl_server_local.h"
#include "handler\hand_tokusei.h"

#include "btl_server.h"

#include  "btlv/btlv_effect.h"



/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  SERVER_CMD_SIZE_MAX = 1024,
  CLIENT_DISABLE_ID = 0xff,
  SERVER_STRBUF_SIZE = 32,
};

enum {
  QUITSTEP_NONE = 0,    ///
  QUITSTEP_REQ,
  QUITSTEP_CORE,
};



//--------------------------------------------------------------
/**
 *  サーバメインループ関数型  - バトル終了時のみ TRUE を返す -
 */
//--------------------------------------------------------------
typedef BOOL (*ServerMainProc)(BTL_SERVER*, int*);


//--------------------------------------------------------------
/**
 *  クライアントアクション格納構造体
 */
//--------------------------------------------------------------
struct _BTL_SVCL_ACTION {
  BTL_ACTION_PARAM     param[ BTL_CLIENT_MAX ][ BTL_POSIDX_MAX ];
  u8                   count[ BTL_CLIENT_MAX ];
};

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
struct _BTL_SERVER {
  ServerMainProc    mainProc;
  int               seq;
  int               intrSeq;

  BTL_MAIN_MODULE*        mainModule;
  BTL_POKE_CONTAINER*     pokeCon;
  SVCL_WORK               client[ BTL_CLIENT_MAX ];
  BTL_SVFLOW_WORK*        flowWork;
  SvflowResult            flowResult;
  BtlBagMode              bagMode;
  GFL_STD_RandContext     randContext;
  BTL_RECTOOL             recTool;
  BTL_RESULT_CONTEXT      btlResultContext;
  STRBUF*                 strbuf;
  const BTL_ESCAPEINFO*   escapeInfo;

  BTL_SVCL_ACTION         clientAction;

  u32         exitTimer;
  u8          enemyPutPokeID;
  u8          quitStep;


  BTL_SERVER_CMD_QUE  queBody;
  BTL_SERVER_CMD_QUE* que;

  u8                  changePokePos[ BTL_POS_MAX ];
  u8                  changePokeCnt;
  u8                  giveupClientID[ BTL_CLIENT_MAX ];
  u8                  giveupClientCnt;

  HEAPID        heapID;
};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void setMainProc( BTL_SERVER* sv, ServerMainProc mainProc );
static void setMainProc_Root( BTL_SERVER* server );
static BOOL ServerMain_WaitReady( BTL_SERVER* server, int* seq );
static BOOL ServerMain_SelectAction( BTL_SERVER* server, int* seq );
static BOOL check_action_chapter( BTL_SERVER* server );
static BOOL ServerMain_ConfirmChangeOrEscape( BTL_SERVER* server, int* seq );
static BOOL ServerMain_SelectPokemonCover( BTL_SERVER* server, int* seq );
static BOOL Irekae_IsNeedConfirm( BTL_SERVER* server );
static u8 Irekae_GetEnemyPutPokeID( BTL_SERVER* server );
static BOOL ServerMain_SelectPokemonChange( BTL_SERVER* server, int* seq );
static BOOL ServerMain_BattleTimeOver( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ExitBattle( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ExitBattle_KeyWait( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ExitBattle_WinWild( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ExitBattle_LoseWild( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ExitBattle_ForCommPlayer( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ExitBattle_ForNPC( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ExitBattle_ForSubwayTrainer( BTL_SERVER* server, int* seq );
static BOOL SendBtlInChapterRecord( BTL_SERVER* server );
static void print_client_action( const BTL_SVCL_ACTION* clientAction );
static void print_que_info( BTL_SERVER_CMD_QUE* que, const char* caption );
static BOOL SendActionRecord( BTL_SERVER* server, BtlRecTiming timingCode, BOOL fChapter );
static void* MakeSelectActionRecord( BTL_SERVER* server, BtlRecTiming timingCode, BOOL fChapter, u32* dataSize );
static BOOL SendRotateRecord( BTL_SERVER* server, const BtlRotateDir* dirAry );
static void storeClientAction( BTL_SERVER* server );
static void SetAdapterCmd( BTL_SERVER* server, BtlAdapterCmd cmd );
static void SetAdapterCmdEx( BTL_SERVER* server, BtlAdapterCmd cmd, const void* sendData, u32 dataSize );
static void SetAdapterCmdSingle( BTL_SERVER* server, BtlAdapterCmd cmd, u8 clientID, const void* sendData, u32 dataSize );
static BOOL WaitAllAdapterReply( BTL_SERVER* server );
static void ResetAdapterCmd( BTL_SERVER* server );
static void Svcl_Clear( SVCL_WORK* clwk );
static BOOL Svcl_IsEnable( const SVCL_WORK* clwk );
static void Svcl_Setup( SVCL_WORK* clwk, u8 clientID, BTL_ADAPTER* adapter, BOOL fLocalClient );
static void Svcl_SetParty( SVCL_WORK* client, BTL_PARTY* party, u8 numCoverPos );




//=============================================================================================
/**
 * サーバ生成
 *
 * @param   mainModule   メインモジュールのハンドラ
 * @param   randContext  乱数コンテキスト（録画データ作成用にクライアントに通知する。乱数初期化は既に行われているのでサーバが行う必要は無い。）
 * @param   pokeCon      ポケモンデータコンテナハンドラ
 * @param   bagMode      バッグモード
 * @param   heapID       ヒープID
 *
 * @retval  BTL_SERVER*
 */
//=============================================================================================
BTL_SERVER* BTL_SERVER_Create( BTL_MAIN_MODULE* mainModule, const GFL_STD_RandContext* randContext,
  BTL_POKE_CONTAINER* pokeCon, BtlBagMode bagMode, HEAPID heapID )
{
  BTL_SERVER* sv = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_SERVER) );

  sv->mainModule = mainModule;
  sv->pokeCon = pokeCon;
  sv->heapID = heapID;
  sv->que = &sv->queBody;
  sv->quitStep = QUITSTEP_NONE;
  sv->flowWork = NULL;
  sv->changePokeCnt = 0;
  sv->giveupClientCnt = 0;
  sv->bagMode = bagMode;
  sv->randContext = *randContext;
  sv->strbuf = GFL_STR_CreateBuffer( SERVER_STRBUF_SIZE, heapID );

  BTL_CALC_ResetSys( randContext );


  {
    int i;
    for(i=0; i<BTL_CLIENT_MAX; ++i){
      Svcl_Clear( &sv->client[i] );
    }
  }

  sv->flowWork = BTL_SVFLOW_InitSystem( sv, sv->mainModule, sv->pokeCon,
        &sv->queBody, sv->bagMode, sv->heapID );

  sv->escapeInfo = BTL_SVFLOW_GetEscapeInfoPointer( sv->flowWork );

  setMainProc( sv, ServerMain_WaitReady );

  return sv;
}

//--------------------------------------------------------------------------------------
/**
 * サーバと同一マシン上にあるクライアントとのアダプタを接続する
 *
 * @param   server      サーバハンドラ
 * @param   adapter     アダプタ（既にクライアントに関連付けられている）
 * @param   clientID    クライアントID
 * @param   numCoverPos クライアントが受け持つ戦闘中ポケモン数
 *
 */
//--------------------------------------------------------------------------------------
void BTL_SERVER_AttachLocalClient( BTL_SERVER* server, BTL_ADAPTER* adapter, u8 clientID, u8 numCoverPos )
{
  GF_ASSERT(clientID < BTL_CLIENT_MAX);
  GF_ASSERT(server->client[clientID].adapter==NULL);

  {
    SVCL_WORK* client;
    BTL_PARTY* party;

    party = BTL_POKECON_GetPartyData( server->pokeCon, clientID );
    client = &server->client[ clientID ];

    Svcl_Setup( client, clientID, adapter, TRUE );
    Svcl_SetParty( client, party, numCoverPos );
  }
}
//--------------------------------------------------------------------------------------
/**
 * サーバと異なるマシン上にあるクライアントとのアダプタを生成する
 *
 * @param   server      サーバハンドラ
 * @param   commMode    通信モード
 * @param   netHandle   ネットワークハンドラ
 * @param   clientID    クライアントID
 * @param   numCoverPos クライアントが受け持つ戦闘中ポケモン数
 *
 */
//--------------------------------------------------------------------------------------
void BTL_SERVER_ReceptionNetClient( BTL_SERVER* server, BtlCommMode commMode, GFL_NETHANDLE* netHandle, u8 clientID, u8 numCoverPos )
{
  GF_ASSERT(clientID < BTL_CLIENT_MAX);
  GF_ASSERT(server->client[clientID].adapter==NULL);

  {
    SVCL_WORK* client;
    BTL_PARTY* party;
    BTL_ADAPTER* adapter;

    party = BTL_POKECON_GetPartyData( server->pokeCon, clientID );
    adapter = BTL_ADAPTER_Create( netHandle, clientID, TRUE, server->heapID );

    client = &server->client[ clientID ];
    Svcl_Setup( client, clientID, adapter, FALSE );
    Svcl_SetParty( client, party, numCoverPos );
  }
}

//=============================================================================================
/**
 * コマンド整合性チェックモードでクライアントをセットアップ
 *
 * @param   server
 * @param   clientID
 * @param   numCoverPos
 */
//=============================================================================================
void BTL_SERVER_CmdCheckMode( BTL_SERVER* server, u8 clientID, u8 numCoverPos )
{
  GF_ASSERT(clientID < BTL_CLIENT_MAX);
  GF_ASSERT(server->client[clientID].adapter==NULL);

  {
    SVCL_WORK* client;
    BTL_PARTY* party;
    BTL_ADAPTER* adapter;

    party = BTL_POKECON_GetPartyData( server->pokeCon, clientID );

    client = &server->client[ clientID ];

    Svcl_Setup( client, clientID, NULL, FALSE );
    Svcl_SetParty( client, party, numCoverPos );
  }
}

//--------------------------------------------------------------------------------------
/**
 * 全クライアント生成・接続完了後のスタートアップ処理
 *
 * @param   server
 *
 */
//--------------------------------------------------------------------------------------
void BTL_SERVER_Startup( BTL_SERVER* server )
{
  setMainProc( server, ServerMain_WaitReady );
  ResetAdapterCmd( server );
  BTL_SVFLOW_ResetSystem( server->flowWork );
}
//--------------------------------------------------------------------------------------
/**
 * サーバ削除
 *
 * @param   wk
 *
 */
//--------------------------------------------------------------------------------------
void BTL_SERVER_Delete( BTL_SERVER* wk )
{
  int i;

  GFL_STR_DeleteBuffer( wk->strbuf );

  BTL_SVFLOW_QuitSystem( wk->flowWork );

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( (wk->client[i].myID != CLIENT_DISABLE_ID)
    &&  (wk->client[i].adapter != NULL)
    &&  (wk->client[i].isLocalClient == FALSE)
    ){
      // 同一マシン上にあるクライアントでなければ、
      // サーバがアダプタを用意したハズなので自分で削除
      BTL_ADAPTER_Delete( wk->client[i].adapter );
    }
  }
  GFL_HEAP_FreeMemory( wk );
}
//--------------------------------------------------------------------------------------
/**
 * サーバメインループ
 *
 * @param   sv      サーバハンドラ
 *
 * @retval  BOOL    終了時 TRUE を返す
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SERVER_Main( BTL_SERVER* sv )
{
  if( sv->quitStep != QUITSTEP_CORE )
  {
    if( sv->mainProc( sv, &sv->seq ) )
    {
      SetAdapterCmdEx( sv, BTL_ACMD_QUIT_BTL, sv->escapeInfo, sizeof(BTL_ESCAPEINFO) );
      BTL_N_Printf( DBGSTR_SV_SendQuitACmad, BTL_ACMD_QUIT_BTL );
      sv->quitStep = QUITSTEP_CORE;
    }
  }
  else
  {
    if( WaitAllAdapterReply(sv) ){
      BTL_N_Printf( DBGSTR_SV_ReplyQuitACmad );
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 終了時、逃げたクライアントのIDを取得
 *
 * @param   sv
 *
 * @retval  u8    逃げたクライアントID（誰も逃げていないならBTL_CLIENTID_NULL）
 */
//----------------------------------------------------------------------------------
void BTL_SERVER_GetEscapeInfo( const BTL_SERVER* sv, BTL_ESCAPEINFO* dst )
{
  *dst = *(sv->escapeInfo);
}
//----------------------------------------------------------------------------------
/**
 * メインループ関数切り替え
 *
 * @param   sv
 * @param   mainProc
 */
//----------------------------------------------------------------------------------
static void setMainProc( BTL_SERVER* sv, ServerMainProc mainProc )
{
  sv->mainProc = mainProc;
  sv->seq = 0;
}
//----------------------------------------------------------------------------------
/**
 * 状況に応じてルートメインループ関数切り分け
 *
 * @param   sv
 */
//----------------------------------------------------------------------------------
static void setMainProc_Root( BTL_SERVER* server )
{
  setMainProc( server, ServerMain_SelectAction );
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
/**
 * サーバメインループ：画面構築、初期ポケ入場エフェクトまで終了待ち
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_WaitReady( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    SetAdapterCmd( server, BTL_ACMD_WAIT_SETUP );
    (*seq)++;
    /* fallthru */
  case 1:
    if( WaitAllAdapterReply(server) )
    {
      ResetAdapterCmd( server );

      // 入場演出処理後、捕獲デモ以外の処理
      if( BTL_MAIN_GetCompetitor(server->mainModule) != BTL_COMPETITOR_DEMO_CAPTURE )
      {
        BOOL fAnyEvent = BTL_SVFLOW_StartBtlIn( server->flowWork );
        if( fAnyEvent )
        {
          TAYA_Printf("wp=%d\n", server->que->writePtr );
          SendBtlInChapterRecord( server );
          (*seq)++;
          break;
        }
        else
        {
          setMainProc_Root( server );
          break;
        }
      }
      // 捕獲デモなら
      else
      {
        setMainProc( server, ServerMain_ExitBattle );
      }
    }
    break;

  case 2:
    if( WaitAllAdapterReply(server) )
    {
      ResetAdapterCmd( server );
      SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
      (*seq)++;
    }
    break;
  case 3:
    if( WaitAllAdapterReply(server) )
    {
      ResetAdapterCmd( server );
      setMainProc_Root( server );
    }
    break;
  }
  return FALSE;
}


void BTL_SERVER_IntrLevelup_Start( BTL_SERVER* server )
{
  server->intrSeq = 0;
}
BOOL BTL_SERVER_IntrLevelup_Main( BTL_SERVER* server )
{
  switch( server->intrSeq ){
  case 0:
    SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
    server->intrSeq++;
    break;
  case 1:
    if( WaitAllAdapterReply(server) )
    {
      ResetAdapterCmd( server );
      SCQUE_Init( server->que );
      server->intrSeq++;
    }
    break;
  case 2:
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * サーバメインループ：アクション選択
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_SelectAction( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    if( BTL_SVFLOW_MakeShooterChargeCommand(server->flowWork) ){
      BTL_N_Printf( DBGSTR_SERVER_SendShooterChargeCmd );
      SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
      (*seq)++;
    }else{
      (*seq) += 2;
    }
    break;

  case 1:
    if( WaitAllAdapterReply(server) )
    {
      BTL_Printf( DBGSTR_SERVER_ShooterChargeCmdDoneAll );
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;

  case 2:
    BTL_N_Printf( DBGSTR_SERVER_SendActionSelectCmd );
    SetAdapterCmd( server, BTL_ACMD_SELECT_ACTION );
    (*seq)++;
    break;

  case 3:
    if( WaitAllAdapterReply(server) )
    {
      BTL_N_Printf( DBGSTR_SERVER_ActionSelectDoneAll );
      ResetAdapterCmd( server );

      // 試合制限時間切れのチェック
      if( BTL_MAIN_CheckGameLimitTimeOver(server->mainModule) )
      {
        setMainProc( server, ServerMain_BattleTimeOver );
        break;
      }

      storeClientAction( server );
      if( SendActionRecord(server, BTL_RECTIMING_StartTurn, check_action_chapter(server)) ){
        (*seq)++;
      }else{
        TAYA_Printf("録画データ生成失敗！！！\n");
        (*seq) += 2;  /// 何らかの理由で録画データ生成に失敗したらスキップ
      }
    }
    break;

  case 4:
    if( WaitAllAdapterReply(server) )
    {
      BTL_N_Printf( DBGSTR_SVFL_RecDataSendComped );
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;

  case 5:
    BTL_SVFLOW_StartTurn_Boot( server->flowWork );
    ResetAdapterCmd( server );
    (*seq)++;

    /* fallthru */

  case 6:
    server->flowResult = BTL_SVFLOW_StartTurn( server->flowWork, &server->clientAction );
    BTL_N_Printf( DBGSTR_SERVER_FlowResult, server->flowResult);

    if( BTL_MAIN_GetCommMode(server->mainModule) != BTL_COMM_NONE ){
      print_client_action( &server->clientAction );
      print_que_info( server->que, "Server Send ... ");
    }

    SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
    (*seq)++;
    break;

  case 7:
    if( WaitAllAdapterReply(server) )
    {
      BTL_N_Printf( DBGSTR_SVFL_AllClientCmdPlayComplete, server->flowResult);
      ResetAdapterCmd( server );

      switch( server->flowResult ){
      case SVFLOW_RESULT_DEFAULT:
        setMainProc_Root( server );
        break;
      case SVFLOW_RESULT_LEVELUP:
        (*seq) = 6;
        break;
      case SVFLOW_RESULT_POKE_COVER:
        BTL_N_Printf( DBGSTR_SV_PokeInReqForEmptyPos );
        {
          BtlCompetitor competitor = BTL_MAIN_GetCompetitor( server->mainModule );
          BtlRule rule = BTL_MAIN_GetRule( server->mainModule );
          if( (competitor == BTL_COMPETITOR_WILD) &&  (rule == BTL_RULE_SINGLE) ){
            setMainProc( server, ServerMain_ConfirmChangeOrEscape );
          }else{
            setMainProc( server, ServerMain_SelectPokemonCover );
          }
        }
        break;
      case SVFLOW_RESULT_POKE_CHANGE:
        BTL_N_Printf( DBGSTR_SV_ChangePokeOnTheTurn );
        GF_ASSERT( server->changePokeCnt );
        setMainProc( server, ServerMain_SelectPokemonChange );
        break;
      case SVFLOW_RESULT_POKE_GET:
        {
          BtlPokePos pos = BTL_SVFLOW_GetCapturedPokePos( server->flowWork );
          BTL_MAIN_NotifyCapturedPokePos( server->mainModule, pos );
          setMainProc( server, ServerMain_ExitBattle );
          return FALSE;
        }
      case SVFLOW_RESULT_BTL_SHOWDOWN:
        {
          BtlResult result = BTL_SVFLOW_ChecBattleResult( server->flowWork );
          BTL_MAIN_NotifyBattleResult( server->mainModule, result );
          setMainProc( server, ServerMain_ExitBattle );
          return FALSE;
        }
      default:
        GF_ASSERT(0);
        /* fallthru */
      case SVFLOW_RESULT_BTL_QUIT:
        BTL_N_Printf( DBGSTR_SVFL_GotoQuit );
        return TRUE;
      }
    }
    break;
  }

  return FALSE;
}
/**
 *  録画データ（アクションコマンド用）のチャプターを打つか判定
 */
static BOOL check_action_chapter( BTL_SERVER* server )
{
  #ifdef ROTATION_NEW_SYSTEM
  // 新ローテーションシステムにはローテーション選択シーケンスは無いので常にTRUE
  return TRUE;
  #else
  // ローテーション以外なら常にチャプター打つ
  if( BTL_MAIN_GetRule(server->mainModule) != BTL_RULE_ROTATION ){
    return TRUE;
  }
  // ローテーションは最初のターンのみ打つ
  // （２ターン目以降はローテーションコマンドにチャプターを打つため）
  else
  {
    if( BTL_SVFTOOL_GetTurnCount(server->flowWork) == 0 ){
      return TRUE;
    }
    return FALSE;
  }
  #endif
}

//----------------------------------------------------------------------------------
/**
 * サーバメインループ：自分のポケモンが倒れた時、逃げるか入れ替えるかを選択（野生シングルのみ）
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_ConfirmChangeOrEscape( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    SetAdapterCmd( server, BTL_ACMD_SELECT_CHANGE_OR_ESCAPE );
    (*seq)++;
    break;
  case 1:
    if( WaitAllAdapterReply(server) )
    {
      u8 clientID = BTL_MAIN_GetPlayerClientID( server->mainModule );
      const u8* result;

      ResetAdapterCmd( server );
      result = BTL_ADAPTER_GetReturnData( server->client[clientID].adapter, NULL );
      if( (*result) == BTL_CLIENTASK_CHANGE ){
        setMainProc( server, ServerMain_SelectPokemonCover );
      }else{
        server->flowResult = BTL_SVFLOW_MakePlayerEscapeCommand( server->flowWork );
        SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
        (*seq)++;
      }
    }
    break;
  case 2:
    if( WaitAllAdapterReply(server) ){
      ResetAdapterCmd( server );

      if( server->flowResult ){
        return TRUE;
      }else{
        setMainProc( server, ServerMain_SelectPokemonCover );
      }
    }
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サーバメインループ：死亡・生き返りで空き位置にポケモンを投入する
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_SelectPokemonCover( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    BTL_Printf("入場ポケモン選択へ  交替されるポケ数=%d\n", server->changePokeCnt);
    SetAdapterCmdEx( server, BTL_ACMD_SELECT_POKEMON_FOR_COVER, server->changePokePos,
        server->changePokeCnt*sizeof(server->changePokePos[0]) );
    (*seq)++;
    break;

  case 1:
    if( WaitAllAdapterReply(server) )
    {
      if( Irekae_IsNeedConfirm(server) )
      {
        server->enemyPutPokeID = Irekae_GetEnemyPutPokeID( server );

        SetAdapterCmdSingle( server, BTL_ACMD_CONFIRM_IREKAE, BTL_CLIENTID_SA_PLAYER,
                &server->enemyPutPokeID, sizeof(server->enemyPutPokeID) );
      }
      (*seq)++;
    }
    break;

  case 2:
    if( WaitAllAdapterReply(server) )
    {
      ResetAdapterCmd( server );

      storeClientAction( server );
      if( SendActionRecord(server, BTL_RECTIMING_PokeInCover, FALSE) ){
        (*seq)++;
      }else{
        BTL_N_Printf( DBGSTR_SV_SendActRecordFailed );
        (*seq) += 2;  /// 何らかの理由で録画データ生成に失敗したらスキップ
      }
    }
    break;

  case 3:
    if( WaitAllAdapterReply(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;

  case 4:
    ResetAdapterCmd( server );
    BTL_SVFLOW_StartAfterPokeIn_Boot( server->flowWork );
    (*seq)++;
    /* fallthru */
  case 5:
//    SCQUE_Init( server->que );
    server->flowResult = BTL_SVFLOW_StartAfterPokeIn( server->flowWork, &server->clientAction );

    BTL_N_Printf( DBGSTR_SERVER_FlowResult, server->flowResult );

    SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
    (*seq)++;
    break;

  case 6:
    if( WaitAllAdapterReply(server) )
    {
      ResetAdapterCmd( server );

      switch( server->flowResult ){
      case SVFLOW_RESULT_POKE_COVER:
        (*seq) = 0;
        break;
      case SVFLOW_RESULT_LEVELUP:
        (*seq) = 5;
        break;
      case SVFLOW_RESULT_BTL_SHOWDOWN:
        {
          BtlResult result = BTL_SVFLOW_ChecBattleResult( server->flowWork );
          BTL_MAIN_NotifyBattleResult( server->mainModule, result );
          setMainProc( server, ServerMain_ExitBattle );
        }
        break;
      case SVFLOW_RESULT_BTL_QUIT:
        BTL_N_Printf( DBGSTR_SVFL_GotoQuit );
        return TRUE;
      default:
        setMainProc_Root( server );
        break;
      }
    }
    break;
  }

  return FALSE;
}

/**
 *  スタンドアロン入れ替え戦にて、プレイヤー側に入れ替えを確認する必要があるか判定
 */
static BOOL Irekae_IsNeedConfirm( BTL_SERVER* server )
{
  if( BTL_MAIN_IsIrekaeMode(server->mainModule) )
  {
    if( server->changePokeCnt )
    {
      u32 i;
      for(i=0; i<server->changePokeCnt; ++i)
      {
        // プレイヤー側も瀕死リクエストがある場合、入れ替え確認の必要なし
        if( BTL_MAIN_BtlPosToClientID(server->mainModule, server->changePokePos[i]) == BTL_CLIENTID_SA_PLAYER){
          return FALSE;
        }
      }

      // 交替できるポケモンがいない場合は確認の必要なし
      {
        BTL_PARTY* party = BTL_POKECON_GetPartyData( server->pokeCon, BTL_MAIN_GetPlayerClientID(server->mainModule) );
        if( BTL_PARTY_GetAliveMemberCount(party) < 2 ){
          return FALSE;
        }
      }

      return TRUE;
    }
  }
  return FALSE;
}
/**
 *  スタンドアロン入れ替え戦にて、敵側が次にくり出すポケモンIDを取得
 */
static u8 Irekae_GetEnemyPutPokeID( BTL_SERVER* server )
{
  SVCL_WORK*  svcl = &server->client[ BTL_CLIENTID_SA_ENEMY1 ];

  if( Svcl_IsEnable(svcl) )
  {
    const BTL_ACTION_PARAM* actParam = BTL_ADAPTER_GetReturnData( svcl->adapter, NULL );
    if( BTL_ACTION_GetAction(actParam) == BTL_ACTION_CHANGE )
    {
      const BTL_POKEPARAM* bpp = BTL_POKECON_GetClientPokeDataConst(
                  server->pokeCon, BTL_CLIENTID_SA_ENEMY1, actParam->change.memberIdx );
      return BPP_GetID( bpp );
    }
  }
  return BTL_POKEID_NULL;
}

//----------------------------------------------------------------------------------
/**
 * サーバメインループ：ターン途中で入れ替えが生じた後の処理
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_SelectPokemonChange( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    BTL_N_Printf( DBGSTR_SV_StartChangePokeInfo, server->changePokeCnt);
    SetAdapterCmdEx( server, BTL_ACMD_SELECT_POKEMON_FOR_CHANGE, server->changePokePos,
        server->changePokeCnt*sizeof(server->changePokePos[0]) );
    (*seq)++;
    break;

  case 1:
    if( WaitAllAdapterReply(server) )
    {
      ResetAdapterCmd( server );

      storeClientAction( server );
      if( SendActionRecord(server, BTL_RECTIMING_PokeInChange, FALSE) ){
        (*seq)++;
      }else{
        (*seq) += 2;  /// 何らかの理由で録画データ生成に失敗したらスキップ
      }
    }
    break;

  case 2:
    if( WaitAllAdapterReply(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;

  case 3:
    ResetAdapterCmd( server );
    BTL_SVFLOW_ContinueAfterPokeChange_Boot( server->flowWork );
    (*seq)++;
    /* fallthru */
  case 4:
    SCQUE_Init( server->que );
    server->flowResult = BTL_SVFLOW_ContinueAfterPokeChange( server->flowWork, &server->clientAction );
    BTL_N_Printf( DBGSTR_SERVER_FlowResult, server->flowResult );
    SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
    (*seq)++;
    break;

  case 5:
    if( WaitAllAdapterReply(server) )
    {
      ResetAdapterCmd( server );

      switch( server->flowResult ){
      case SVFLOW_RESULT_POKE_CHANGE:
        (*seq) = 0;
        break;
      case SVFLOW_RESULT_LEVELUP:
        (*seq) = 4;
        break;
      case SVFLOW_RESULT_POKE_COVER:
        setMainProc( server, ServerMain_SelectPokemonCover );
        break;
      case SVFLOW_RESULT_BTL_SHOWDOWN:
        {
          BtlResult result = BTL_SVFLOW_ChecBattleResult( server->flowWork );
          BTL_MAIN_NotifyBattleResult( server->mainModule, result );
          setMainProc( server, ServerMain_ExitBattle );
        }
        break;
      case SVFLOW_RESULT_BTL_QUIT:
        BTL_N_Printf( DBGSTR_SVFL_GotoQuit );
        return TRUE;
      default:
        setMainProc_Root( server );
        break;
      }
    }
    break;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サーバメインループ：制限時間終了
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_BattleTimeOver( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    {
      void* recData;
      u32   recDataSize;
      recData = BTL_RECTOOL_PutTimeOverData( &server->recTool, &recDataSize );
      TAYA_Printf("時間制限による終了コード[%x][%x] %d byte 送信\n", ((u8*)(recData))[0], ((u8*)(recData))[1], recDataSize);
      SetAdapterCmdEx( server, BTL_ACMD_RECORD_DATA, recData, recDataSize );
      (*seq)++;
    }
    break;
  case 1:
    if( WaitAllAdapterReply(server) ){
      ResetAdapterCmd( server );
      SetAdapterCmd( server, BTL_ACMD_NOTIFY_TIMEUP );
      (*seq)++;
    }
    break;
  case 2:
    if( WaitAllAdapterReply(server) )
    {
      ResetAdapterCmd( server );

      {
        BtlResult result = BTL_SVFLOW_ChecBattleResult( server->flowWork );
        BTL_MAIN_NotifyBattleResult( server->mainModule, result );
        setMainProc( server, ServerMain_ExitBattle );
      }
    }
    break;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サーバメインループ：バトル終了処理
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_ExitBattle( BTL_SERVER* server, int* seq )
{
  BtlResult result = BTL_MAIN_GetBattleResult( server->mainModule );

  server->btlResultContext.resultCode = result;
  server->btlResultContext.clientID = BTL_MAIN_GetPlayerClientID( server->mainModule );

  {
    BtlCompetitor  competitor = BTL_MAIN_GetCompetitor( server->mainModule );

    BTL_N_Printf( DBGSTR_SV_ExitBattle, competitor);

    switch( competitor ){
    case BTL_COMPETITOR_TRAINER:
      setMainProc( server, ServerMain_ExitBattle_ForNPC );
      break;

    case BTL_COMPETITOR_SUBWAY:
      setMainProc( server, ServerMain_ExitBattle_ForSubwayTrainer );
      break;

    case BTL_COMPETITOR_COMM:
      setMainProc( server, ServerMain_ExitBattle_ForCommPlayer );
      break;

    case BTL_COMPETITOR_WILD:
      if( result == BTL_RESULT_WIN )
      {
        setMainProc( server, ServerMain_ExitBattle_WinWild );
      }
      else if( (result == BTL_RESULT_LOSE) || (result == BTL_RESULT_DRAW) ){
        setMainProc( server, ServerMain_ExitBattle_LoseWild );
      }
      else{
        setMainProc( server, ServerMain_ExitBattle_KeyWait );
      }
      break;

    default:
      if( result == BTL_RESULT_WIN ){
        u16 winBGM = BTL_MAIN_GetWinBGMNo( server->mainModule );
        PMSND_PlayBGM( winBGM );
      }
      setMainProc( server, ServerMain_ExitBattle_KeyWait );
      break;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サーバメインループ：バトル終了処理時キー待ち
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_ExitBattle_KeyWait( BTL_SERVER* server, int* seq )
{
  switch( *seq ) {
  case 0:
    {
      u8 touch = FALSE;

      if( !BTL_MAIN_IsRecordPlayMode(server->mainModule) )
      {
        if( (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B))
        ||  (GFL_UI_TP_GetTrg())
        ){
          touch = TRUE;
        }
      }
      else{
        touch = TRUE;
      }

      if( touch ){
        ++(*seq);
        return TRUE;
      }
    }
    break;

  case 1:
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サーバメインループ：バトル終了（野生に勝ち）
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_ExitBattle_WinWild( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    SetAdapterCmdEx( server, BTL_ACMD_EXIT_WIN_WILD, &server->btlResultContext, sizeof(server->btlResultContext) );
    (*seq)++;
    break;
  case 1:
    if( WaitAllAdapterReply(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;
  case 2:
    setMainProc( server, ServerMain_ExitBattle_KeyWait );
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サーバメインループ：バトル終了（野生に負け）
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_ExitBattle_LoseWild( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    SetAdapterCmdEx( server, BTL_ACMD_EXIT_LOSE_WILD, &server->btlResultContext, sizeof(server->btlResultContext) );
    (*seq)++;
    break;
  case 1:
    if( WaitAllAdapterReply(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;
  case 2:
    setMainProc( server, ServerMain_ExitBattle_KeyWait );
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サーバメインループ：バトル終了（通信対戦トレーナーとの対戦）
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_ExitBattle_ForCommPlayer( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    SetAdapterCmdEx( server, BTL_ACMD_EXIT_COMM, &server->btlResultContext, sizeof(server->btlResultContext) );
    (*seq)++;
    break;
  case 1:
    if( WaitAllAdapterReply(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;
  case 2:
//    setMainProc( server, ServerMain_ExitBattle_KeyWait );
    return TRUE;
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サーバメインループ：バトル終了（ゲーム内通常トレーナーとの対戦に勝利）
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_ExitBattle_ForNPC( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    SetAdapterCmdEx( server, BTL_ACMD_EXIT_NPC, &server->btlResultContext, sizeof(server->btlResultContext) );
    (*seq)++;
    break;
  case 1:
    if( WaitAllAdapterReply(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;
  case 2:
    setMainProc( server, ServerMain_ExitBattle_KeyWait );
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サーバメインループ：バトル終了（サブウェイトレーナーとの対戦終了）
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_ExitBattle_ForSubwayTrainer( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    SetAdapterCmdEx( server, BTL_ACMD_EXIT_SUBWAY_TRAINER, &server->btlResultContext, sizeof(server->btlResultContext) );
    (*seq)++;
    break;
  case 1:
    if( WaitAllAdapterReply(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;
  case 2:
    setMainProc( server, ServerMain_ExitBattle_KeyWait );
    break;
  }
  return FALSE;
}


//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

void BTL_SERVER_CMDCHECK_RestoreActionData( BTL_SERVER* server, const void* recData, u32 recDataSize )
{
  u32 rp = 0;
  BTL_ACTION_PARAM action[ BTL_POSIDX_MAX ];
  u8 clientID, numAction, i;

  for(i=0; i<BTL_CLIENT_MAX; ++i){
    server->clientAction.count[ i ] = 0;
  }

  BTL_RECTOOL_RestoreStart( &server->recTool, recData, recDataSize );
  while( BTL_RECTOOL_RestoreFwd(&server->recTool, &rp, &clientID, &numAction, action) )
  {
    BTL_N_Printf( DBGSTR_SV_RestoreAction, clientID, numAction );
    for(i=0; i<numAction; ++i){
      server->clientAction.param[ clientID ][ i ] = action[ i ];
    }
    server->clientAction.count[ clientID ] = numAction;
  }
}

static void print_client_action( const BTL_SVCL_ACTION* clientAction )
{
  #ifdef PM_DEBUG
  u32 i;
  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    if( clientAction->count[i] )
    {
      u32 j;
      OS_TPrintf("  Client(%d) action cnt=%d\n", i, clientAction->count[ i ] );
      for(j=0; j<clientAction->count[i]; ++j)
      {
        const BTL_ACTION_PARAM* action = &(clientAction->param[i][j]);

        OS_TPrintf("   Action(%d) = %d:", j, action->gen.cmd );
        if( action->gen.cmd == BTL_ACTION_FIGHT )
        {
          OS_TPrintf(" waza=%d, targetPos=%d\n", action->fight.waza, action->fight.targetPos );
        }
        OS_TPrintf("\n");
      }
    }
  }
  #endif
}

static void print_que_info( BTL_SERVER_CMD_QUE* que, const char* caption )
{
  #ifdef PM_DEBUG
  // TWL-DEBUGGER 上でないと文字列モジュールが正しく動かないので…
  if( OS_GetConsoleType() == OS_CONSOLE_TWLDEBUGGER )
  {
    OS_TPrintf("  * %s : %d bytes\n", caption, que->writePtr );
    {
      static int args[ BTL_SERVERCMD_ARG_MAX ];
      ServerCmd  cmd;
      int i;

      for(i=0; i<que->writePtr; ++i)
      {
        OS_TPrintf("%02x,", que->buffer[i] );
        if( (i+1)%16==0 ){ OS_TPrintf("\n"); }
      }
      OS_TPrintf("\n");

      que->readPtr = 0;

      do {
        const char* scname;
        for(i=0; i<NELEMS(args); ++i){
          args[ i ] = -1;
        }

        cmd = SCQUE_Read( que, args );
        scname = BTL_DEBUGPRINT_GetServerCmdName(cmd);
        if( scname == NULL ){
          OS_TPrintf(" UnKnown Command [%d]  \n", cmd );
          return;
        }
        OS_TPrintf(" cmd=%04x ", cmd);
        OS_TPrintf( scname );
        OS_TPrintf(" args=" );
        for(i=0; i<NELEMS(args); ++i){
          OS_TPrintf(" %d,", args[i]);
        }
        // 追加引数のあるコマンド処理
        {
          u32 j, exArgCnt = 0;
          switch( cmd ){
          case SC_ACT_WAZA_DMG_PLURAL:  exArgCnt = args[0]; break;
          case SC_ACT_WEATHER_DMG:      exArgCnt = args[1]; break;
          }
          if( exArgCnt )
          {
            OS_TPrintf("\n   ExArg=");
            for(j=0; j<exArgCnt; ++j){
              OS_TPrintf("%d, ", SCQUE_READ_ArgOnly(que) );
            }
            OS_TPrintf("\n");
          }
        }
        OS_TPrintf( "\n" );

      } while( !SCQUE_IsFinishRead(que) );

    }
  }
  #endif
}

BOOL BTL_SERVER_CMDCHECK_Make( BTL_SERVER* server, BtlRecTiming timingCode, const void* recvedCmd, u32 cmdSize )
{
  SCQUE_Init( server->que );

  switch( timingCode ){
  case BTL_RECTIMING_BtlIn:
    BTL_SVFLOW_StartBtlIn( server->flowWork );
    break;

  case BTL_RECTIMING_StartTurn:
    BTL_SVFLOW_StartTurn_Boot( server->flowWork );
    BTL_SVFLOW_StartTurn( server->flowWork, &server->clientAction );
    break;

  case BTL_RECTIMING_PokeInCover:
    BTL_SVFLOW_StartAfterPokeIn_Boot( server->flowWork );
    BTL_SVFLOW_StartAfterPokeIn( server->flowWork, &server->clientAction );
    break;

  case BTL_RECTIMING_PokeInChange:
    BTL_SVFLOW_ContinueAfterPokeChange_Boot( server->flowWork );
    BTL_SVFLOW_ContinueAfterPokeChange( server->flowWork, &server->clientAction );
    break;

  default:
    break;
  }

  if( server->que->writePtr == cmdSize )
  {
    if( GFL_STD_MemComp(server->que->buffer, recvedCmd, cmdSize) == 0 ){
      BTL_N_Printf( DBGSTR_SV_CmdCheckOK );
      return FALSE;
    }
  }

  // 以下、コマンド内容が完全一致しなかった場合のデバッグ出力
  #ifdef PM_DEBUG
  ISDPrintSetBlockingMode( 1 );
  {
    OS_TPrintf( "****** コマンド整合性 NG timing=%d  ****** \n", timingCode );

    print_client_action( &server->clientAction );

    print_que_info( server->que, "cmd by check_server" );
//    SCQUE_Init( server->que );
    SCQUE_Setup( server->que, recvedCmd, cmdSize );
    OS_TPrintf("  True Server Cmd = %d bytes, adrs=%p, %02x, %02x, %02x, ... %02x, %02x, %02x\n", cmdSize, recvedCmd,
        ((const u8*)recvedCmd)[0], ((const u8*)recvedCmd)[1], ((const u8*)recvedCmd)[2],
        server->que->buffer[0], server->que->buffer[1], server->que->buffer[2] );
    print_que_info( server->que, "cmd by true_server" );
  }
  ISDPrintSetBlockingMode( 0 );
  #endif

  return TRUE;
}


/**
 *  バトル開始チャプタ記録データ送信開始
 */
static BOOL SendBtlInChapterRecord( BTL_SERVER* server )
{
  void* recData;
  u32   recDataSize;

  BTL_RECTOOL_Init( &server->recTool, TRUE );

  recData = BTL_RECTOOL_PutBtlInChapter( &server->recTool, &recDataSize );
  SetAdapterCmdEx( server, BTL_ACMD_RECORD_DATA, recData, recDataSize );
  TAYA_Printf("チャプタ記録データ送信\n");
  return TRUE;
}
/**
 *  アクション記録データ送信開始
 */
static BOOL SendActionRecord( BTL_SERVER* server, BtlRecTiming timingCode, BOOL fChapter )
{
  void* recData;
  u32   recDataSize;
  recData = MakeSelectActionRecord( server, timingCode, fChapter, &recDataSize );
  if( recData != NULL )
  {
    BTL_N_Printf( DBGSTR_SV_SendActRecord, recDataSize);
    SetAdapterCmdEx( server, BTL_ACMD_RECORD_DATA, recData, recDataSize );
    return TRUE;
  }
  return FALSE;
}
/**
 * アクション記録データを生成
 *
 * @param   server
 * @param   fChapter   TRUEならターン区切りコードを記録データに埋め込む
 * @param   dataSize   [out] 生成されたデータサイズ
 *
 * @retval  void*   正しく生成できたら送信データポインタ / できない場合NULL
 */
static void* MakeSelectActionRecord( BTL_SERVER* server, BtlRecTiming timingCode, BOOL fChapter, u32* dataSize )
{
  u32 ID;

  BTL_RECTOOL_Init( &server->recTool, fChapter );

  for(ID=0; ID<BTL_CLIENT_MAX; ++ID)
  {
    if( server->client[ID].myID != CLIENT_DISABLE_ID )
    {
      u32 returnDataSize, numAction;
      const BTL_ACTION_PARAM* action = BTL_ADAPTER_GetReturnData( server->client[ID].adapter, &returnDataSize );

      numAction = returnDataSize / sizeof(BTL_ACTION_PARAM);
      BTL_RECTOOL_PutSelActionData( &server->recTool, ID, action, numAction );
    }
  }

  return BTL_RECTOOL_FixSelActionData( &server->recTool, timingCode, dataSize );
}
/**
 *  各クライアントから返信のあったアクション内容を構造体に格納
 */
static void storeClientAction( BTL_SERVER* server )
{
  SVCL_WORK* clwk;
  u32 i, j;

  for(i=0; i<BTL_CLIENT_MAX; ++i)
  {
    clwk = &server->client[i];
    server->clientAction.count[i] = 0;
    if( Svcl_IsEnable(clwk) )
    {
      u32 returnDataSize, numAction;
      const BTL_ACTION_PARAM* action = BTL_ADAPTER_GetReturnData( clwk->adapter, &returnDataSize );
      numAction = returnDataSize / sizeof(BTL_ACTION_PARAM);
      for(j=0; j<numAction; ++j){
        server->clientAction.param[i][j] = *action;
        server->clientAction.count[i]++;
        ++action;
      }
    }
  }
}


//=================================================================================================
//=================================================================================================

//----------------------------------------------------------------------------------
/**
 * 全クライアントへコマンドリクエスト
 *
 * @param   server
 * @param   cmd
 */
//----------------------------------------------------------------------------------
static void SetAdapterCmd( BTL_SERVER* server, BtlAdapterCmd cmd )
{
  SetAdapterCmdEx( server, cmd, NULL, 0 );
}
//----------------------------------------------------------------------------------
/**
 * 全クライアントへコマンドリクエスト（拡張情報あり）
 *
 * @param   server
 * @param   cmd
 * @param   sendData
 * @param   dataSize
 */
//----------------------------------------------------------------------------------
static void SetAdapterCmdEx( BTL_SERVER* server, BtlAdapterCmd cmd, const void* sendData, u32 dataSize )
{
  int i;

  BTL_ADAPTERSYS_BeginSetCmd();

  for(i=0; i<BTL_CLIENT_MAX; i++)
  {
    if( Svcl_IsEnable(&server->client[i]) ){
      BTL_ADAPTER_SetCmd( server->client[i].adapter, cmd, sendData, dataSize );
    }
  }

  BTL_ADAPTERSYS_EndSetCmd();
}
//----------------------------------------------------------------------------------
/**
 * 指定クライアントへのみコマンドリクエスト
 *
 * @param   server
 * @param   cmd
 * @param   clientID
 */
//----------------------------------------------------------------------------------
static void SetAdapterCmdSingle( BTL_SERVER* server, BtlAdapterCmd cmd, u8 clientID, const void* sendData, u32 dataSize )
{
  SVCL_WORK* svcl = &server->client[ clientID ];

  if( Svcl_IsEnable(svcl) ){
    BTL_ADAPTER_ResetCmd( svcl->adapter );
    BTL_ADAPTER_SetCmd( svcl->adapter, cmd, sendData, dataSize );
  }
  else{
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------------
/**
 * 接続している全クライアントからの返信を待つ
 *
 * @param   server
 *
 * @retval  BOOL    全クライアントからの返信が揃ったらTRUE
 */
//----------------------------------------------------------------------------------
static BOOL WaitAllAdapterReply( BTL_SERVER* server )
{
  int i;
  BOOL ret = TRUE;

  for(i=0; i<BTL_CLIENT_MAX; i++)
  {
    if( Svcl_IsEnable(&server->client[i]) )
    {
      if( !BTL_ADAPTER_WaitCmd( server->client[i].adapter ) ){
        ret = FALSE;
      }
    }
  }

  return ret;
}
//----------------------------------------------------------------------------------
/**
 * コマンドリクエスト処理リセット
 *
 * @param   server
 */
//----------------------------------------------------------------------------------
static void ResetAdapterCmd( BTL_SERVER* server )
{
  int i;
  for(i=0; i<BTL_CLIENT_MAX; i++)
  {
    if( Svcl_IsEnable(&server->client[i]) ){
      BTL_ADAPTER_ResetCmd( server->client[i].adapter );
    }
  }
}


//=================================================================================================
//=================================================================================================

//----------------------------------------------------------------------------------
/**
 * サーバ保持用クライアントワーク：初期化
 *
 * @param   clwk
 */
//----------------------------------------------------------------------------------
static void Svcl_Clear( SVCL_WORK* clwk )
{
  clwk->myID = CLIENT_DISABLE_ID;
}
//----------------------------------------------------------------------------------
/**
 * サーバ保持用クライアントワーク：有効判定
 *
 * @param   clwk
 */
//----------------------------------------------------------------------------------
static BOOL Svcl_IsEnable( const SVCL_WORK* clwk )
{
  return clwk->myID != CLIENT_DISABLE_ID;
}
//----------------------------------------------------------------------------------
/**
 * サーバ保持用クライアントワーク：基本設定
 *
 * @param   clwk
 * @param   adapter
 * @param   fLocalClient
 */
//----------------------------------------------------------------------------------
static void Svcl_Setup( SVCL_WORK* clwk, u8 clientID, BTL_ADAPTER* adapter, BOOL fLocalClient )
{
  clwk->myID = clientID;
  clwk->adapter = adapter;
  clwk->isLocalClient = fLocalClient;
}
//--------------------------------------------------------------------------
/**
 * サーバ保持用クライアントワーク：パーティ設定
 *
 * @param   client        クライアントワーク
 * @param   party         パーティデータ
 * @param   numCoverPos   クライアントが受け持つ戦闘中ポケモン数
 */
//--------------------------------------------------------------------------
static void Svcl_SetParty( SVCL_WORK* client, BTL_PARTY* party, u8 numCoverPos )
{
  u16 i;

  client->party = party;
  client->memberCount = BTL_PARTY_GetMemberCount( party );
  client->numCoverPos = numCoverPos;
}

//--------------------------------------------------------------------------------------
/**
 * サーバ保持用クライアントワークへのポインタを返す（存在しないとASSERT失敗）
 *
 * @param   server
 * @param   clientID
 *
 * @retval  BTL_SERVER_CLWORK*
 */
//--------------------------------------------------------------------------------------
SVCL_WORK* BTL_SERVER_GetClientWork( BTL_SERVER* server, u8 clientID )
{
  if( (clientID < NELEMS(server->client))
  &&  (server->client[clientID].myID != CLIENT_DISABLE_ID)
  ){
    return &server->client[ clientID ];
  }
  return NULL;
}
//--------------------------------------------------------------------------------------
/**
 * サーバ保持用クライアントワークへのポインタを返す（存在しなければNULLを返す）
 *
 * @param   server
 * @param   clientID
 *
 * @retval  SVCL_WORK*
 */
//--------------------------------------------------------------------------------------
SVCL_WORK* BTL_SERVER_GetClientWorkIfEnable( BTL_SERVER* server, u8 clientID )
{
  if( server->client[clientID].myID != CLIENT_DISABLE_ID )
  {
    return &(server->client[clientID]);
  }
  return NULL;
}
//--------------------------------------------------------------------------------------
/**
 * 指定IDのクライアントが存在するか判定
 *
 * @param   server
 * @param   clientID
 *
 * @retval  BOOL    存在すればTRUE
 */
//--------------------------------------------------------------------------------------
BOOL BTL_SERVER_IsClientEnable( const BTL_SERVER* server, u8 clientID )
{
  if( clientID < BTL_CLIENT_MAX ){
    return Svcl_IsEnable( &server->client[ clientID ] );
  }
  return FALSE;
}





u8 BTL_SVCL_GetNumActPoke( SVCL_WORK* clwk )
{
  GF_ASSERT(clwk->adapter);
  {
    u32 dataSize = BTL_ADAPTER_GetReturnDataSize( clwk->adapter );
    GF_ASSERT((dataSize%sizeof(BTL_ACTION_PARAM))==0);
    return dataSize / sizeof(BTL_ACTION_PARAM);
  }
}
const BTL_ACTION_PARAM* BTL_SVCL_GetPokeAction( SVCL_WORK* clwk, u8 posIdx )
{
  GF_ASSERT(clwk->adapter);
  GF_ASSERT_MSG(posIdx<clwk->numCoverPos, "posIdx=%d, numCoverPos=%d", posIdx, clwk->numCoverPos);

  {
    const BTL_ACTION_PARAM* action = BTL_ADAPTER_GetReturnData( clwk->adapter, NULL );
    action += posIdx;
    return action;
  }
}

u8 BTL_SVCL_ACTION_GetNumActPoke( const BTL_SVCL_ACTION* clientAction, u8 clientID )
{
  GF_ASSERT(clientID<BTL_CLIENT_MAX);
  return clientAction->count[ clientID ];
}

BTL_ACTION_PARAM BTL_SVCL_ACTION_Get( const BTL_SVCL_ACTION* clientAction, u8 clientID, u8 posIdx )
{
  GF_ASSERT(clientID<BTL_CLIENT_MAX);
  GF_ASSERT(posIdx<BTL_POSIDX_MAX);
  return clientAction->param[ clientID ][ posIdx ];
}


//----------------------------------------------
void BTL_SERVER_NotifyPokemonLevelUp( BTL_SERVER* server, const BTL_POKEPARAM* bpp )
{
  const POKEMON_PARAM* pp;
  pp = BPP_GetSrcData( bpp );
  PP_Get( pp, ID_PARA_nickname, server->strbuf );
  GAMEBEACON_Set_PokemonLevelUp( server->strbuf );
}
void BTL_SERVER_NotifyPokemonCapture( BTL_SERVER* server, const BTL_POKEPARAM* bpp )
{
  const POKEMON_PARAM* pp;
  pp = BPP_GetSrcData( bpp );
  GAMEBEACON_Set_PokemonGet( PP_Get( pp, ID_PARA_monsno, NULL ) );

  BTL_MAIN_RECORDDATA_Inc( server->mainModule, RECID_CAPTURE_POKE );
  BTL_MAIN_RECORDDATA_Inc( server->mainModule, RECID_DAYCNT_CAPTURE );
}

void BTL_SERVER_AddBonusMoney( BTL_SERVER* server, u32 volume )
{
  BTL_MAIN_AddBonusMoney( server->mainModule, volume );
}
void BTL_SERVER_SetMoneyDblUp( BTL_SERVER* server )
{
  BTL_MAIN_SetMoneyDblUp( server->mainModule );
}

void BTL_SERVER_InitChangePokemonReq( BTL_SERVER* server )
{
  server->changePokeCnt = 0;
  server->giveupClientCnt = 0;
}

void BTL_SERVER_RequestChangePokemon( BTL_SERVER* server, BtlPokePos pos )
{
  GF_ASSERT(server->changePokeCnt < BTL_POS_MAX);
  {
    u32 i;
    for(i=0; i<server->changePokeCnt; ++i){
      if( server->changePokePos[i] == pos ){
        return;
      }
    }
    server->changePokePos[ server->changePokeCnt++ ] = pos;
  }
}



BTL_SVFLOW_WORK* BTL_SERVER_GetFlowWork( BTL_SERVER* server )
{
  return server->flowWork;
}

