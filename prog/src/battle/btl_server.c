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
#include "waza_tool\wazadata.h"
#include "gamesystem\game_beacon.h"
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
 *
 */
//--------------------------------------------------------------
struct _BTL_SERVER {
  ServerMainProc    mainProc;
  int               seq;

  BTL_MAIN_MODULE*      mainModule;
  BTL_POKE_CONTAINER*   pokeCon;
  SVCL_WORK             client[ BTL_CLIENT_MAX ];
  BTL_SVFLOW_WORK*      flowWork;
  SvflowResult          flowResult;
  BtlBagMode            bagMode;
  GFL_STD_RandContext   randContext;
  BTL_RECTOOL           recTool;
  STRBUF*               strbuf;

  u8          quitStep;
  u32         escapeClientID;
  u32         exitTimer;

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
static BOOL ServerMain_SelectRotation( BTL_SERVER* server, int* seq );
static BOOL ServerMain_SelectAction( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ConfirmChangeOrEscape( BTL_SERVER* server, int* seq );
static BOOL ServerMain_SelectPokemonIn( BTL_SERVER* server, int* seq );
static BOOL ServerMain_SelectPokemonChange( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ExitBattle( BTL_SERVER* server, int* seq );
static BOOL ServerMain_ExitBattle_ForTrainer( BTL_SERVER* server, int* seq );
static BOOL SendActionRecord( BTL_SERVER* server );
static BOOL SendRotateRecord( BTL_SERVER* server );
static void* MakeSelectActionRecord( BTL_SERVER* server, u32* dataSize );
static void* MakeRotationRecord( BTL_SERVER* server, u32* dataSize );
static void SetAdapterCmd( BTL_SERVER* server, BtlAdapterCmd cmd );
static void SetAdapterCmdEx( BTL_SERVER* server, BtlAdapterCmd cmd, const void* sendData, u32 dataSize );
static BOOL WaitAdapterCmd( BTL_SERVER* server );
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


  {
    int i;
    for(i=0; i<BTL_CLIENT_MAX; ++i){
      Svcl_Clear( &sv->client[i] );
    }
  }

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
    adapter = BTL_ADAPTER_Create( netHandle, server->heapID, clientID );

    client = &server->client[ clientID ];
    Svcl_Setup( client, clientID, adapter, FALSE );
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
  GF_ASSERT(server->flowWork==NULL);
  server->flowWork = BTL_SVFLOW_InitSystem( server, server->mainModule, server->pokeCon,
        &server->queBody, server->bagMode, server->heapID );
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
void BTL_SERVER_Main( BTL_SERVER* sv )
{
  if( sv->quitStep != QUITSTEP_CORE )
  {
    if( sv->mainProc( sv, &sv->seq ) )
    {
      sv->escapeClientID = BTL_SVFLOW_GetEscapeClientID( sv->flowWork );
      SetAdapterCmdEx( sv, BTL_ACMD_QUIT_BTL, &sv->escapeClientID, sizeof(sv->escapeClientID) );
      sv->quitStep = QUITSTEP_CORE;
    }
  }
  else
  {
    WaitAdapterCmd( sv );
  }
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
  // ローテーションバトル＆初回ターン以外なら、アクションの前にローテーション選択する
  if( (BTL_MAIN_GetRule(server->mainModule) == BTL_RULE_ROTATION)
  &&  (BTL_SVFTOOL_GetTurnCount(server->flowWork) != 0 )
  ){
    setMainProc( server, ServerMain_SelectRotation );
  }else{
    setMainProc( server, ServerMain_SelectAction );
  }
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
    BTL_EVENT_InitSystem();
    SetAdapterCmd( server, BTL_ACMD_WAIT_SETUP );
    (*seq)++;
    /* fallthru */
  case 1:
    if( WaitAdapterCmd(server) )
    {
      ResetAdapterCmd( server );
      BTL_SVFLOW_Start_AfterPokemonIn( server->flowWork );
      if( server->que->writePtr )
      {
        SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
        (*seq)++;
      }
      else
      {
        (*seq)+=2;
      }
    }
    break;
  case 2:
    if( WaitAdapterCmd(server) )
    {
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;
  case 3:
    setMainProc_Root( server );
    break;
  }
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * サーバメインループ：ローテーション選択（ローテーションバトルのみ）
 *
 * @param   server
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL ServerMain_SelectRotation( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    SetAdapterCmd( server, BTL_ACMD_SELECT_ROTATION );
    (*seq)++;
    break;

  case 1:
    if( WaitAdapterCmd(server) )
    {
      const BtlRotateDir  *dir;
      u32 i;

      ResetAdapterCmd( server );
      SCQUE_Init( server->que );
      for(i=0; i<BTL_CLIENT_MAX; ++i)
      {
        if( Svcl_IsEnable(&server->client[i]) ){
          dir = BTL_ADAPTER_GetReturnData( server->client[i].adapter, NULL );
          BTL_SVFLOW_CreateRotationCommand( server->flowWork, i, *dir );
        }
      }

      if( SendRotateRecord(server) ){
        (*seq)++;
      }else{
        (*seq) += 2;
      }
    }
    break;

  case 2:
    if( WaitAdapterCmd(server) )
    {
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;

  case 3:
    SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
    (*seq)++;
    break;

  case 4:
    if( WaitAdapterCmd(server) )
    {
      ResetAdapterCmd( server );
      setMainProc( server, ServerMain_SelectAction );
    }
    break;
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
    if( WaitAdapterCmd(server) )
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
    if( WaitAdapterCmd(server) )
    {
      BTL_N_Printf( DBGSTR_SERVER_ActionSelectDoneAll );
      ResetAdapterCmd( server );
      server->flowResult = BTL_SVFLOW_StartTurn( server->flowWork );
      BTL_N_Printf( DBGSTR_SERVER_FlowResult, server->flowResult);

      if( SendActionRecord(server) ){
        (*seq)++;
      }else{
        (*seq) += 2;  /// 何らかの理由で録画データ生成に失敗したらスキップ
      }
    }
    break;

  case 4:
    if( WaitAdapterCmd(server) )
    {
      BTL_N_Printf( DBGSTR_SVFL_RecDataSendComped );
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;

  case 5:
      BTL_N_Printf( DBGSTR_SVFL_SendServerCmd, server->flowResult);
      SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
      (*seq)++;
      break;

  case 6:
    if( WaitAdapterCmd(server) )
    {
      BTL_N_Printf( DBGSTR_SVFL_AllClientCmdPlayComplete, server->flowResult);
      BTL_MAIN_SyncServerCalcData( server->mainModule );
      ResetAdapterCmd( server );

      switch( server->flowResult ){
      case SVFLOW_RESULT_DEFAULT:
        setMainProc_Root( server );
        break;
      case SVFLOW_RESULT_POKE_COVER:
        BTL_N_Printf( DBGSTR_SV_PokeInReqForEmptyPos );
        {
          BtlCompetitor competitor = BTL_MAIN_GetCompetitor( server->mainModule );
          BtlRule rule = BTL_MAIN_GetRule( server->mainModule );
          if( (competitor == BTL_COMPETITOR_WILD) &&  (rule == BTL_RULE_SINGLE) ){
            setMainProc( server, ServerMain_ConfirmChangeOrEscape );
          }else{
            setMainProc( server, ServerMain_SelectPokemonIn );
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
        setMainProc( server, ServerMain_ExitBattle );
        return FALSE;
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
    if( WaitAdapterCmd(server) ){
      u8 clientID = BTL_MAIN_GetPlayerClientID( server->mainModule );
      const u8* result;

      ResetAdapterCmd( server );
      result = BTL_ADAPTER_GetReturnData( server->client[clientID].adapter, NULL );
      if( (*result) == BTL_CLIENTASK_CHANGE ){
        setMainProc( server, ServerMain_SelectPokemonIn );
      }else{
        server->flowResult = BTL_SVFLOW_CreatePlayerEscapeCommand( server->flowWork );
        SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
        (*seq)++;
      }
    }
    break;
  case 2:
    if( WaitAdapterCmd(server) ){
      ResetAdapterCmd( server );

      if( server->flowResult ){
        return TRUE;
      }else{
        setMainProc( server, ServerMain_SelectPokemonIn );
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
static BOOL ServerMain_SelectPokemonIn( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    BTL_Printf("入場ポケモン選択へ  交替されるポケ数=%d\n", server->changePokeCnt);
    SetAdapterCmdEx( server, BTL_ACMD_SELECT_POKEMON_FOR_COVER, server->changePokePos,
        server->changePokeCnt*sizeof(server->changePokePos[0]) );
    (*seq)++;
    break;

  case 1:
    if( WaitAdapterCmd(server) )
    {
      BTL_Printf("入場ポケモン選択後\n");
      ResetAdapterCmd( server );
      SCQUE_Init( server->que );
      server->flowResult = BTL_SVFLOW_StartAfterPokeIn( server->flowWork );
      BTL_N_Printf( DBGSTR_SERVER_FlowResult, server->flowResult );

      if( SendActionRecord(server) ){
        (*seq)++;
      }else{
        (*seq) += 2;  /// 何らかの理由で録画データ生成に失敗したらスキップ
      }
    }
    break;

  case 2:
    if( WaitAdapterCmd(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;

  case 3:
    SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
    (*seq)++;
    break;

  case 4:
    if( WaitAdapterCmd(server) )
    {
      ResetAdapterCmd( server );
      BTL_MAIN_SyncServerCalcData( server->mainModule );

      switch( server->flowResult ){
      case SVFLOW_RESULT_POKE_COVER:
        (*seq) = 0;
        break;
      case SVFLOW_RESULT_BTL_SHOWDOWN:
        setMainProc( server, ServerMain_ExitBattle );
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
    if( WaitAdapterCmd(server) )
    {
      ResetAdapterCmd( server );
      SCQUE_Init( server->que );
      server->flowResult = BTL_SVFLOW_ContinueAfterPokeChange( server->flowWork );
      BTL_N_Printf( DBGSTR_SERVER_FlowResult, server->flowResult );

      if( SendActionRecord(server) ){
        (*seq)++;
      }else{
        (*seq) += 2;  /// 何らかの理由で録画データ生成に失敗したらスキップ
      }
    }
    break;

  case 2:
    if( WaitAdapterCmd(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;

  case 3:
    SetAdapterCmdEx( server, BTL_ACMD_SERVER_CMD, server->que->buffer, server->que->writePtr );
    (*seq)++;
    break;

  case 4:
    if( WaitAdapterCmd(server) )
    {
      ResetAdapterCmd( server );
      BTL_MAIN_SyncServerCalcData( server->mainModule );

      switch( server->flowResult ){
      case SVFLOW_RESULT_POKE_CHANGE:
        (*seq) = 0;
        break;

      case SVFLOW_RESULT_POKE_COVER:
        setMainProc( server, ServerMain_SelectPokemonIn );
        break;
      case SVFLOW_RESULT_BTL_SHOWDOWN:
        setMainProc( server, ServerMain_ExitBattle );
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
  // @todo 本来は勝ちor負け、野生orトレーナー戦などで分岐する
  switch( *seq ){
  case 0:
    if( BTL_MAIN_ChecBattleResult(server->mainModule) == BTL_RESULT_WIN )
    {
      PMSND_PlayBGM( SEQ_BGM_WIN1 );
      if( BTL_MAIN_GetCompetitor(server->mainModule) == BTL_COMPETITOR_TRAINER ){
        setMainProc( server, ServerMain_ExitBattle_ForTrainer );
      }else{
        (*seq)++;
      }
    }
    else
    {
      (*seq)++;
    }
    break;
  case 1:
    {
      u8 touch = FALSE;
      if( (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B))
      ||  (GFL_UI_TP_GetTrg())
      ){
        touch = TRUE;
      }
      if( touch ){
        return TRUE;
      }
    }
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
static BOOL ServerMain_ExitBattle_ForTrainer( BTL_SERVER* server, int* seq )
{
  switch( *seq ){
  case 0:
    SetAdapterCmd( server, BTL_ACMD_EXIT_WIN_TRAINER );
    (*seq)++;
    break;
  case 1:
    if( WaitAdapterCmd(server) ){
      ResetAdapterCmd( server );
      (*seq)++;
    }
    break;
  case 2:
    {
      u8 touch = FALSE;
      if( (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B))
      ||  (GFL_UI_TP_GetTrg())
      ){
        touch = TRUE;
      }
      if( touch ){
        return TRUE;
      }
    }
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------


/**
 *  アクション記録データ送信開始
 */
static BOOL SendActionRecord( BTL_SERVER* server )
{
  void* recData;
  u32   recDataSize;
  recData = MakeSelectActionRecord( server, &recDataSize );
  if( recData != NULL ){
//    BTL_Printf("アクション記録データを送信する (%dbytes)\n", recDataSize);
    SetAdapterCmdEx( server, BTL_ACMD_RECORD_DATA, recData, recDataSize );
    return TRUE;
  }
  return FALSE;
}

/**
 *  ローテーション記録データ送信開始
 */
static BOOL SendRotateRecord( BTL_SERVER* server )
{
  void* recData;
  u32   recDataSize;
  recData = MakeRotationRecord( server, &recDataSize );
  if( recData != NULL ){
//    BTL_Printf("ローテーション記録データを送信する (%dbytes)\n", recDataSize);
    SetAdapterCmdEx( server, BTL_ACMD_RECORD_DATA, recData, recDataSize );
    return TRUE;
  }
  return FALSE;
}

/**
 * アクション記録データを生成
 *
 * @param   server
 * @param   dataSize   [out] 生成されたデータサイズ
 *
 * @retval  void*   正しく生成できたら送信データポインタ / できない場合NULL
 */
static void* MakeSelectActionRecord( BTL_SERVER* server, u32* dataSize )
{
  u32 ID;

  BTL_RECTOOL_Init( &server->recTool );

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

  return BTL_RECTOOL_FixSelActionData( &server->recTool, dataSize );
}
/**
 * ローテーション記録データを生成
 *
 * @param   server
 * @param   dataSize   [out] 生成されたデータサイズ
 *
 * @retval  void*   正しく生成できたら送信データポインタ / できない場合NULL
 */
static void* MakeRotationRecord( BTL_SERVER* server, u32* dataSize )
{
  u32 ID;

  BTL_RECTOOL_Init( &server->recTool );

  for(ID=0; ID<BTL_CLIENT_MAX; ++ID)
  {
    if( server->client[ID].myID != CLIENT_DISABLE_ID )
    {
      const BtlRotateDir *pDir = BTL_ADAPTER_GetReturnData( server->client[ID].adapter, NULL );
      BTL_RECTOOL_PutRotationData( &server->recTool, ID, *pDir );
    }
  }

  return BTL_RECTOOL_FixRotationData( &server->recTool, dataSize );
}


static void SetAdapterCmd( BTL_SERVER* server, BtlAdapterCmd cmd )
{
  SetAdapterCmdEx( server, cmd, NULL, 0 );
}

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

static BOOL WaitAdapterCmd( BTL_SERVER* server )
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
  PP_Get( pp, ID_PARA_nickname, server->strbuf );
  GAMEBEACON_Set_PokemonGet( server->strbuf );
}

void BTL_SERVER_AddBonusMoney( BTL_SERVER* server, u32 volume )
{
  BTL_MAIN_AddBonusMoney( server->mainModule, volume );
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
