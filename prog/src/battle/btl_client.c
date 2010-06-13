//=============================================================================================
/**
 * @file  btl_client.c
 * @brief ポケモンWB バトルシステム クライアントモジュール
 * @author  taya
 *
 * @date  2008.09.06  作成
 */
//=============================================================================================
#include <gflib.h>

#include "sound\pm_sndsys.h"
#include "poke_tool\pokeparty.h"
#include  "tr_tool/tr_tool.h"
#include  "tr_tool/trtype_def.h"
#include  "btlv/btlv_gauge.h"
#include  "item\item.h"
#include  "item\itemtype_def.h"

#include  "btlv/btlv_effect.h"
#include  "btlv/btlv_gauge.h"

#include "battle\battle.h"
#include "btl_common.h"
#include "btl_main.h"
#include "btl_adapter.h"
#include "btl_action.h"
#include "btl_field.h"
#include "btl_pokeselect.h"
#include "btl_rec.h"
#include "btl_server_cmd.h"
#include "btl_net.h"
#include "btl_calc.h"

#include "app\b_bag.h"
#include "app\b_plist.h"
#include "btlv\btlv_core.h"
#include "btlv\btlv_effect.h"
#include "btlv\btlv_timer.h"
#include "btlv\btlv_input.h"

#include "tr_ai/tr_ai.h"
#include "tr_tool/tr_tool.h"

#include "btl_tables.h"
#include "btl_client.h"

#include "debug/debug_hudson.h"

enum {
  PRINT_FLG = FALSE,
};

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/

enum {
  CANTESC_COUNT_MAX = 8,    ///< とりあえず少なめギリギリなとこでASSERTかけてみる
  AI_ITEM_MAX = 4,          ///< AIトレーナーが使用するアイテム数

  EMPTY_POS_NONE = -1,

  CLIENT_FLDEFF_BITFLAG_SIZE = BTL_CALC_BITFLAG_BUFSIZE( BTL_FLDEFF_MAX ) + 1,
};


/**
 *  AIトレーナーのメッセージ種類
 */
enum {
  AITRAINER_MSG_HP_HALF,        ///< HP半分以下
  AITRAINER_MSG_FIRST_DAMAGE,   ///< 最初のダメージ後
  AITRAINER_MSG_LAST,           ///< 最後の１体登場直後
  AITRAINER_MSG_LAST_HP_HALF,   ///< 最後の１体ＨＰ半分以下
  AITRAINER_MSG_MAX,
};



/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BOOL (*ClientMainProc)( BTL_CLIENT* );
typedef BOOL (*ClientSubProc)( BTL_CLIENT*, int* );
typedef BOOL (*ServerCmdProc)( BTL_CLIENT*, int*, const int* );

/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/


/**
 *  録画データコントロールコード
 */
typedef enum {
  RECCTRL_NONE = 0,   ///< 操作無し
  RECCTRL_QUIT,       ///< 再生終了
  RECCTRL_CHAPTER,    ///< チャプター移動
}RecCtrlCode;



/**
 *  録画データ再生コントロール構造体
 */
typedef struct {
  u8   seq;
  u8   ctrlCode;
  u8   fChapterSkip    : 1;
  u8   fFadeOutStart   : 1;
  u8   fFadeOutDone    : 1;
  u8   fTurnIncrement  : 1;
  u8   fLock           : 1;
  u16  handlingTimer;
  u16  turnCount;
  u16  nextTurnCount;
  u16  maxTurnCount;
  u16  skipTurnCount;
}RECPLAYER_CONTROL;

/*--------------------------------------------------------------------------*/
/* Globals                                                                  */
/*--------------------------------------------------------------------------*/
#ifdef PM_DEBUG
static BTLV_CORE* GViewCore = NULL;
static int GControlableAIClientID = -1;
static int GYubiCtrlPos;
#endif


//--------------------------------------------------------------
/**
 *  クライアントモジュール構造定義
 */
//--------------------------------------------------------------
struct _BTL_CLIENT {

  BTL_MAIN_MODULE*      mainModule;
  BTL_POKE_CONTAINER*   pokeCon;
  const BTL_POKEPARAM*  procPoke;
  BTL_ACTION_PARAM*     procAction;
  BTL_REC*              btlRec;
  BTL_RECREADER*        btlRecReader;
  RECPLAYER_CONTROL     recPlayer;
  ClientMainProc        mainProc;
  BTL_ESCAPEINFO        escapeInfo;

  BTL_ADAPTER*    adapter;
  BTLV_CORE*      viewCore;
  BTLV_STRPARAM   strParam;
  BTLV_STRPARAM   strParamSub;
  BTL_SERVER*     cmdCheckServer;
  BTLV_ROTATION_WAZASEL_PARAM  rotWazaSelParam;
  BtlRotateDir    prevRotateDir;

  ClientSubProc   subProc;
  int             subSeq;
  ClientSubProc   selActProc;
  int             selActSeq;

  const void*    returnDataPtr;
  u32            returnDataSize;
  u32            dummyReturnData;
  u16            cmdLimitTime;
  u16            gameLimitTime;

  u16                   AIItem[ AI_ITEM_MAX ];
  VMHANDLE*             AIHandle;
  GFL_STD_RandContext   AIRandContext;

  s8             AIChangeIndex[ BTL_PARTY_MEMBER_MAX ];
  u8             AITrainerMsgCheckedFlag[ AITRAINER_MSG_MAX ];


  const BTL_PARTY*  myParty;
  u8                numCoverPos;    ///< 担当する戦闘ポケモン数
  u8                procPokeIdx;    ///< 処理中ポケモンインデックス
  s8                prevPokeIdx;    ///< 前回の処理ポケモンインデックス
  u8                firstPokeIdx;   ///< アクション指示できる最初のポケモンインデックス
  u8                fStdMsgChanged; ///< 標準メッセージ内容を書き換えたフラグ
  BtlPokePos        basePos;        ///< 戦闘ポケモンの位置ID

  BTL_ACTION_PARAM     actionParam[ BTL_POSIDX_MAX ];
  u8                   shooterCost[ BTL_POSIDX_MAX ];
  BTL_SERVER_CMD_QUE*  cmdQue;
  int                  cmdArgs[ BTL_SERVERCMD_ARG_MAX ];
  ServerCmd            serverCmd;
  ServerCmdProc        scProc;
  int                  scSeq;

  BTL_POKESELECT_PARAM    pokeSelParam;
  BTL_POKESELECT_RESULT   pokeSelResult;

  HEAPID heapID;
  u16  EnemyPokeHPBase;
  u16  AITrainerMsgID;
  u16  selItemWork[ BTL_POSIDX_MAX ];
  u8   myID;
  u8   myType;
  u8   myState;
  u8   commWaitInfoOn;
  u8   bagMode;
  u8   shooterEnergy;
  u8   change_escape_code;
  u8   fForceQuitSelAct;
  u8   cmdCheckTimingCode;
  u8   actionAddCount;
  u8   wazaInfoPokeIdx;
  u8   wazaInfoWazaIdx;

  u8   fAITrainerBGMChanged : 1;
  u8   fCommError           : 1;
  u8   fBallSelected        : 1;

  u8          myChangePokeCnt;
  u8          myPuttablePokeCnt;
  BtlPokePos  myChangePokePos[ BTL_POSIDX_MAX ];

  u8          fieldEffectFlag[ CLIENT_FLDEFF_BITFLAG_SIZE ];

  #ifdef PM_DEBUG
  const BTL_CLIENT* viewOldClient;
  int               yubifuruDebugRetSeq;
  #endif
};



/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void ChangeMainProc( BTL_CLIENT* wk, ClientMainProc proc );
static BOOL ClientMain_Normal( BTL_CLIENT* wk );
static BOOL ClientMain_ChapterSkip( BTL_CLIENT* wk );
static void setDummyReturnData( BTL_CLIENT* wk );
static ClientSubProc getSubProc( BTL_CLIENT* wk, BtlAdapterCmd cmd, BOOL* fRecCtrlLock );
static BOOL SubProc_UI_Setup( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_REC_Setup( BTL_CLIENT* wk, int* seq );
static void EnemyPokeHPBase_Update( BTL_CLIENT* wk );
static u32 EnemyPokeHPBase_CheckRatio( BTL_CLIENT* wk );
static const BTL_POKEPARAM* EnemyPokeHPBase_GetTargetPoke( BTL_CLIENT* wk );
static BOOL SubProc_UI_SelectRotation( BTL_CLIENT* wk, int* seq );
static BtlRotateDir decideNextDirRandom( BtlRotateDir prevDir );
static BOOL SubProc_REC_SelectRotation( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_SelectRotation( BTL_CLIENT* wk, int* seq );
static void CmdLimit_Start( BTL_CLIENT* wk );
static BOOL CmdLimit_CheckOver( BTL_CLIENT* wk );
static BOOL CheckSelactForceQuit( BTL_CLIENT* wk, ClientSubProc nextProc );
static void CmdLimit_End( BTL_CLIENT* wk );
static void SetupSelectStartStr( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, BTLV_STRPARAM* strParam );
static void ClientSubProc_Set( BTL_CLIENT* wk, ClientSubProc proc );
static BOOL ClientSubProc_Call( BTL_CLIENT* wk );
static BOOL SubProc_UI_SelectAction( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_REC_SelectAction( BTL_CLIENT* wk, int* seq );
static void setNullActionRecplay( BTL_CLIENT* wk );
static BOOL selact_Start( BTL_CLIENT* wk, int* seq );
static void selact_startMsg( BTL_CLIENT* wk, const BTLV_STRPARAM* strParam );
static BOOL selact_ForceQuit( BTL_CLIENT* wk, int* seq );
static BOOL selact_YubiFuruDebug( BTL_CLIENT* wk, int* seq );
static BOOL selact_Root( BTL_CLIENT* wk, int* seq );
static BOOL selact_TrainerMessage( BTL_CLIENT* wk, int* seq );
static  BOOL  check_tr_message( BTL_CLIENT* wk, u16* msgID );
static BOOL selact_Fight( BTL_CLIENT* wk, int* seq );
static void setupRotationParams( BTL_CLIENT* wk, BTLV_ROTATION_WAZASEL_PARAM* param );
static BOOL selact_WazaInfoView( BTL_CLIENT* wk, int* seq );
static BOOL selact_SelectChangePokemon( BTL_CLIENT* wk, int* seq );
static BOOL selact_Item( BTL_CLIENT* wk, int* seq );
static BOOL selact_Escape( BTL_CLIENT* wk, int* seq );
static BOOL selact_CheckFinish( BTL_CLIENT* wk, int* seq );
static BOOL selact_Finish( BTL_CLIENT* wk, int* seq );
static void selItemWork_Init( BTL_CLIENT* wk );
static void selItemWork_Reserve( BTL_CLIENT* wk, u8 pokeIdx, u16 itemID );
static void selItemWork_Restore( BTL_CLIENT* wk, u8 pokeIdx );
static void selItemWork_Quit( BTL_CLIENT* wk );
static void shooterCost_Init( BTL_CLIENT* wk );
static void shooterCost_Save( BTL_CLIENT* wk, u8 procPokeIdx, u8 cost );
static u8 shooterCost_Get( BTL_CLIENT* wk, u8 procPokeIdx );
static u8 shooterCost_GetSum( BTL_CLIENT* wk );
static BOOL checkActionForceSet( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* action );
static BOOL checkWazaForceSet( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* action );
static void setWaruagakiAction( BTL_ACTION_PARAM* dst, BTL_CLIENT* wk, const BTL_POKEPARAM* bpp );
static BOOL is_unselectable_waza( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, WazaID waza, BTLV_STRPARAM* strParam );
static u8 StoreSelectableWazaFlag( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, u8* dst );
static BOOL IsItemEffective( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp );
static BtlCantEscapeCode isForbidPokeChange( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, u8* pokeID, u16* tokuseiID );
static BtlCantEscapeCode isForbidEscape( BTL_CLIENT* wk, u8* pokeID, u16* tokuseiID );
static BtlCantEscapeCode checkForbidChangeEscapeCommon( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, u8* pokeID, u16* tokuseiID );
static BOOL checkForbitEscapeEffective_Kagefumi( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke );
static BOOL checkForbitEscapeEffective_Arijigoku( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke );
static BOOL checkForbitEscapeEffective_Jiryoku( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke );
static void ChangeAI_InitWork( BTL_CLIENT* wk );
static BOOL ChangeAI_CheckReserve( BTL_CLIENT* wk, u8 index );
static void ChangeAI_SetReserve( BTL_CLIENT* wk, u8 outPokeIdx, u8 inPokeIdx );
static BOOL ChangeAI_Root( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, u8 procPokeIndex, u8* targetIndex );
static BTL_POKEPARAM* ChangeAI_DecideTarget( BTL_CLIENT* wk, BtlPokePos basePos );
static BOOL ChangeAI_CheckHorobi( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke );
static BOOL ChangeAI_CheckFusigiNaMamori( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, const BTL_POKEPARAM* targetPoke );
static BOOL ChangeAI_CheckNoEffectWaza( BTL_CLIENT* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender );
static BOOL ChangeAI_CheckKodawari( BTL_CLIENT* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender );
static BOOL ChangeAI_CheckUkeTokusei( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, const BTL_POKEPARAM* targetPoke, u8* changeIndex );
static BOOL AI_ChangeProcSub_SizenKaifuku( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, u8* changeIndex );
static BOOL AI_ChangeProcSub_WazaAff( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, const BTL_POKEPARAM* targetPoke, u8* changeIndex );
static BOOL AI_ChangeProcSub_CheckTokHikae( BTL_CLIENT* wk, u16 tokusei, u8* index );
static BOOL AI_ChangeProcSub_HikaeWazaAff( BTL_CLIENT* wk, const BTL_POKEPARAM* target, BtlTypeAff affMin );
static BOOL AI_ChangeProcSub_HikaePokeAff( BTL_CLIENT* wk, PokeType wazaType, BtlTypeAff affMax, u8* pokeIndex );
static BOOL AI_ChangeProcSub_CheckWazaAff( BTL_CLIENT* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BtlTypeAff affMin );
static BtlRotateDir RotAI_CheckRotation( BTL_CLIENT* wk );
static BOOL SubProc_AI_SelectAction( BTL_CLIENT* wk, int* seq );
static u8 calcPuttablePokemons( BTL_CLIENT* wk, u8* list );
static void sortPuttablePokemonList( BTL_CLIENT* wk, u8* list, u8 numPoke, const BTL_POKEPARAM* target );
static void setupPokeSelParam( BTL_CLIENT* wk, u8 mode, u8 numSelect, BTL_POKESELECT_PARAM* param, BTL_POKESELECT_RESULT* result );
static void storePokeSelResult( BTL_CLIENT* wk, const BTL_POKESELECT_RESULT* res );
static void storePokeSelResult_ForceQuit( BTL_CLIENT* wk );
static u8 storeMyChangePokePos( BTL_CLIENT* wk, BtlPokePos* myCoverPos );
static BOOL SubProc_UI_SelectChangeOrEscape( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_SelectPokemonForCover( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_SelectPokemonForChange( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_AI_SelectPokemon( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_REC_SelectPokemon( BTL_CLIENT* wk, int* seq );
static BOOL SelectPokemonUI_Core( BTL_CLIENT* wk, int* seq, u8 mode );
static u32 SetCoverRotateAction( BTL_CLIENT* wk, BTL_ACTION_PARAM* resultAction );
static BOOL SubProc_UI_ConfirmIrekae( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_RecordData( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_REC_ExitCommTrainer( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_ExitCommTrainer( BTL_CLIENT* wk, int* seq );
static BtlResult expandServerResult( BTL_CLIENT* wk );
static inline BOOL IsEnemyClientDouble( BTL_CLIENT* wk );
static inline void TrainerGraphicIn( BTL_CLIENT* wk, int client_idx );
static inline void MsgWinningTrainerStart( BTL_CLIENT* wk );
static BOOL SubProc_REC_ExitForNPC( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_ExitForNPC( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_REC_ExitForSubwayTrainer( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_ExitForSubwayTrainer( BTL_CLIENT* wk, int* seq );
static void setupSubwayTrainerMsg( BTL_CLIENT* wk, BtlResult result, u8 client_idx );
static BOOL SubProc_UI_WinWild( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_LoseWild( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_NotifyTimeUp( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_REC_ServerCmd( BTL_CLIENT* wk, int* seq );
static BOOL SubProc_UI_ServerCmd( BTL_CLIENT* wk, int* seq );
static BOOL scProc_ACT_MemberOutMsg( BTL_CLIENT* wk, int* seq, const int* args );
static u16 CheckMemberOutStrID( BTL_CLIENT* wk, u8 clientID, BOOL* fClientArg );
static BOOL scProc_ACT_MemberOut( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_MemberIn( BTL_CLIENT* wk, int* seq, const int* args );
static u16 CheckMemberPutStrID( BTL_CLIENT* wk );
static BOOL scProc_MSG_Std( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_MSG_StdSE( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_MSG_Set( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_MSG_Waza( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_WazaEffect( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_TameWazaHide( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_WazaDmg( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_WazaDmg_Plural( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_WazaIchigeki( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_SickIcon( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_ConfDamage( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_Dead( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_Relive( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_RankDown( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_RankUp( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_WeatherDmg( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_WeatherStart( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_WeatherEnd( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_SimpleHP( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_Kinomi( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_Kill( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_Move( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_ResetMove( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_MigawariCreate( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_MigawariDelete( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_Hensin( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_MigawariDamage( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_PlayBGM( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_MsgWinHide( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_Exp( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL msgPokanCallback( u32 arg );
static BOOL wazaOboeSeq( BTL_CLIENT* wk, int* seq, BTL_POKEPARAM* bpp );
static BOOL scProc_ACT_BallThrow( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_BallThrowTrainer( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_Rotation( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_ChangeTokusei( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_SwapTokusei( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_FakeDisable( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_EffectSimple( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_EffectByPos( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_EffectByVector( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_ACT_ChangeForm( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_TOKWIN_In( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_TOKWIN_Out( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_HpMinus( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_HpPlus( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_PPMinus( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_PPMinus_Org( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_WazaUsed( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_HpZero( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_PPPlus( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_PPPlus_Org( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_RankUp( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_RankDown( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_RankSet7( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_RankRecover( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_RankReset( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_AddCritical( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_SickSet( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_CurePokeSick( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_CureWazaSick( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_MemberIn( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_ChangePokeType( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_WSTurnCheck( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_ConsumeItem( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_UpdateUseWaza( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_SetContFlag( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_ResetContFlag( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_SetTurnFlag( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_ResetTurnFlag( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_ChangeTokusei( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_SetItem( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_UpdateWazaNumber( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_OutClear( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_AddFldEff( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_AddFldEffDepend( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_DelFldEffDepend( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_RemoveFldEff( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_SetPokeCounter( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_BatonTouch( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_MigawariCreate( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_MigawariDelete( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_ShooterCharge( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_SetFakeSrc( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_ClearConsumedItem( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_CureSickDependPoke( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_AddWazaDamage( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_TurnCheck( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_SetStatus( BTL_CLIENT* wk, int* seq, const int* args );
static BOOL scProc_OP_SetWeight( BTL_CLIENT* wk, int* seq, const int* args );
static u8 countFrontPokeTokusei( BTL_CLIENT* wk, PokeTokusei tokusei );
static u8 countFrontPokeType( BTL_CLIENT* wk, PokeType type );
static void AIItem_Setup( BTL_CLIENT* wk );
static u16 AIItem_CheckUse( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, const BTL_PARTY* party );
static BOOL check_status_up_item( u16 itemID, const BTL_POKEPARAM* bpp );
static BOOL check_cure_sick_item( u16 itemID, const BTL_POKEPARAM* bpp );
static void RecPlayer_Init( RECPLAYER_CONTROL* ctrl );
static void RecPlayer_Setup( RECPLAYER_CONTROL* ctrl, u32 turnCnt );
static BOOL RecPlayer_CheckBlackOut( const RECPLAYER_CONTROL* ctrl );
static void RecPlayer_TurnIncReq( RECPLAYER_CONTROL* ctrl );
static RecCtrlCode RecPlayer_GetCtrlCode( const RECPLAYER_CONTROL* ctrl );
static void RecPlayer_ChapterSkipOn( RECPLAYER_CONTROL* ctrl, u32 nextTurnNum );
static void RecPlayer_ChapterSkipOff( RECPLAYER_CONTROL* ctrl );
static BOOL RecPlayer_CheckChapterSkipEnd( const RECPLAYER_CONTROL* ctrl );
static u32 RecPlayer_GetNextTurn( const RECPLAYER_CONTROL* ctrl );
static BOOL RecPlayerCtrl_Lock( RECPLAYER_CONTROL* ctrl );
static void RecPlayerCtrl_Unlock( RECPLAYER_CONTROL* ctrl );
static void RecPlayerCtrl_Main( BTL_CLIENT* wk, RECPLAYER_CONTROL* ctrl );
static void AICtrl_Init( void );
static void AICtrl_Delegate( BTL_CLIENT* wk );
static void aictrl_RestoreViewClient( BTL_CLIENT* wk );
static BOOL AICtrl_IsMyFase( BTL_CLIENT* wk );


//=============================================================================================
/**
 * システム初期化
 *
 * @param   none
 */
//=============================================================================================
void BTL_CLIENTSYSTEM_Init( void )
{
  #ifdef PM_DEBUG
  AICtrl_Init();
  #endif
}


//=============================================================================================
/**
 * クライアント生成
 *
 * @param   mainModule
 * @param   pokecon
 * @param   commMode
 * @param   netHandle
 * @param   clientID
 * @param   numCoverPos
 * @param   clientType
 * @param   bagMode
 * @param   fRecPlayMode
 * @param   randContext
 * @param   heapID
 *
 * @retval  BTL_CLIENT*
 */
//=============================================================================================
BTL_CLIENT* BTL_CLIENT_Create(
  BTL_MAIN_MODULE* mainModule, BTL_POKE_CONTAINER* pokecon, BtlCommMode commMode,
  GFL_NETHANDLE* netHandle, u16 clientID, u16 numCoverPos, BtlClientType clientType, BtlBagMode bagMode, BOOL fRecPlayMode,
  const GFL_STD_RandContext* randContext, HEAPID heapID )
{
  BTL_CLIENT* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_CLIENT) );
  int i;
  BOOL fComm = ((commMode != BTL_COMM_NONE) && (clientType == BTL_CLIENT_TYPE_UI));

  wk->myID = clientID;
  wk->heapID = heapID;
  wk->myType = clientType;
  wk->adapter = BTL_ADAPTER_Create( netHandle, clientID, fComm, heapID );
  wk->myParty = BTL_POKECON_GetPartyDataConst( pokecon, clientID );
  wk->mainModule = mainModule;
  wk->pokeCon = pokecon;
  wk->numCoverPos = numCoverPos;
  wk->procPokeIdx = 0;
  wk->basePos = clientID;
  wk->prevRotateDir = BTL_ROTATEDIR_NONE;
  wk->viewCore = NULL;
  wk->EnemyPokeHPBase = 0;
  wk->cmdQue = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTL_SERVER_CMD_QUE) );
  wk->mainProc = ClientMain_Normal;
  wk->myState = 0;
  wk->cmdCheckServer = NULL;
  wk->cmdCheckTimingCode = BTL_RECTIMING_None;

  wk->commWaitInfoOn = FALSE;
  wk->shooterEnergy = 0;
  wk->cmdLimitTime = 0;
  wk->gameLimitTime = 0;
  wk->fForceQuitSelAct = FALSE;
  wk->fAITrainerBGMChanged = FALSE;
  wk->fCommError = FALSE;
//  wk->shooterEnergy = BTL_SHOOTER_ENERGY_MAX;

  wk->bagMode = bagMode;
  BTL_ESCAPEINFO_Clear( &wk->escapeInfo );

  RecPlayer_Init( &wk->recPlayer );

  BTL_CALC_BITFLG_Construction( wk->fieldEffectFlag, sizeof(wk->fieldEffectFlag) );

  AIItem_Setup( wk );
  wk->AIRandContext = *randContext;
  if( (wk->myType == BTL_CLIENT_TYPE_AI) && (!fRecPlayMode) )
  {
    u32 ai_bit = BTL_MAIN_GetClientAIBit( wk->mainModule, wk->myID );
    BTL_SVFLOW_WORK* svf_work = BTL_MAIN_GetSVFWorkForAI( wk->mainModule );
    wk->AIHandle = TR_AI_Init( wk->mainModule, svf_work, wk->pokeCon, ai_bit, wk->heapID );
  }
  else
  {
    wk->AIHandle = NULL;
  }

  for(i=0; i<NELEMS(wk->AITrainerMsgCheckedFlag); ++i){
    wk->AITrainerMsgCheckedFlag[ i ] = FALSE;
  }

  if( (wk->myType == BTL_CLIENT_TYPE_UI)
  &&  (BTL_MAIN_IsRecordEnable(wk->mainModule))
  ){
    wk->btlRec = BTL_REC_Create( heapID );
  }else{
    wk->btlRec = NULL;
  }

  #ifdef PM_DEBUG
  wk->viewOldClient = NULL;
  #endif

  return wk;
}

void BTL_CLIENT_Delete( BTL_CLIENT* wk )
{
  if( wk->btlRec ){
    BTL_REC_Delete( wk->btlRec );
  }
  if( wk->AIHandle ){
    TR_AI_Exit( wk->AIHandle );
  }
  GFL_HEAP_FreeMemory( wk->cmdQue );
  BTL_ADAPTER_Delete( wk->adapter );

  GFL_HEAP_FreeMemory( wk );
}

void BTL_CLIENT_NotifyCommError( BTL_CLIENT* wk )
{
  wk->fCommError = TRUE;
}

/**
 *  録画記録データを取得
 */
const void* BTL_CLIENT_GetRecordData( BTL_CLIENT* wk, u32* size )
{
  if( wk->btlRec ){
    return BTL_REC_GetDataPtr( wk->btlRec, size );
  }
  return NULL;
}


void BTL_CLIENT_SetRecordPlayerMode( BTL_CLIENT* wk, BTL_RECREADER* recReader )
{
  u32 turnCnt;

  wk->myType = BTL_CLIENT_TYPE_RECPLAY;
  wk->btlRecReader = recReader;
  turnCnt = BTL_RECREADER_GetTurnCount( wk->btlRecReader );

  RecPlayer_Setup( &wk->recPlayer, turnCnt );
}



//=============================================================================================
/**
 * 描画処理モジュールをアタッチ（UIクライアントのみ）
 *
 * @param   wk
 * @param   viewCore
 *
 */
//=============================================================================================
void BTL_CLIENT_AttachViewCore( BTL_CLIENT* wk, BTLV_CORE* viewCore )
{
  wk->viewCore = viewCore;
}

//=============================================================================================
/**
 * コマンド整合性チェク用のサーバモジュールをアタッチ
 *
 * @param   wk
 * @param   server
 */
//=============================================================================================
void BTL_CLIENT_AttachCmeCheckServer( BTL_CLIENT* wk, BTL_SERVER* server )
{
  wk->cmdCheckServer = server;
}



//=============================================================================================
/**
 * 生成したアダプタを返す
 *
 * @param   wk
 *
 * @retval  BTL_ADAPTER*
 */
//=============================================================================================
BTL_ADAPTER* BTL_CLIENT_GetAdapter( BTL_CLIENT* wk )
{
  GF_ASSERT(wk);
  return wk->adapter;
}

//=============================================================================================
/**
 * クライアントメインループ
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_CLIENT_Main( BTL_CLIENT* wk )
{
  return wk->mainProc( wk );
}

//=============================================================================================
/**
 * チャプタスキップ
 *
 * @param   wk
 * @param   fChapterSkipMode
 */
//=============================================================================================
void BTL_CLIENT_SetChapterSkip( BTL_CLIENT* wk, u32 nextTurnNum )
{
  BTL_RECREADER_Reset( wk->btlRecReader );

  RecPlayer_ChapterSkipOn( &wk->recPlayer, nextTurnNum );
  ChangeMainProc( wk, ClientMain_ChapterSkip );
}

void BTL_CLIENT_StopChapterSkip( BTL_CLIENT* wk )
{
//  ChangeMainProc( wk, ClientMain_Normal );
  RecPlayer_ChapterSkipOff( &wk->recPlayer );
  ChangeMainProc( wk, ClientMain_Normal );
}

//=============================================================================================
/**
 * クライアントが録画再生モードで動いているか判定
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_CLIENT_IsRecPlayerMode( const BTL_CLIENT* wk )
{
  return wk->myType == BTL_CLIENT_TYPE_RECPLAY;
}
//=============================================================================================
/**
 *
 *
 * @param   wk
 *
 * @retval  u32
 */
//=============================================================================================
u32 BTL_CLIENT_GetRecPlayerMaxChapter( const BTL_CLIENT* wk )
{
  return wk->recPlayer.maxTurnCount;
}

//=============================================================================================
/**
 * チャプタースキップモードで動作しているか判定
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_CLIENT_IsChapterSkipMode( const BTL_CLIENT* wk )
{
  return (wk->mainProc == ClientMain_ChapterSkip);
}




/**
 *  メインループ関数を差し替える
 */
static void ChangeMainProc( BTL_CLIENT* wk, ClientMainProc proc )
{
  wk->mainProc = proc;
  wk->myState = 0;
}

/**
 *  メインループ：通常時
 */
static BOOL ClientMain_Normal( BTL_CLIENT* wk )
{
  enum {
    SEQ_READ_ACMD = 0,
    SEQ_EXEC_CMD,
    SEQ_RETURN_TO_SV,
    SEQ_RETURN_TO_SV_QUIT,
    SEQ_RECPLAY_CTRL,
    SEQ_BGM_FADEOUT,
    SEQ_WAIT_RECPLAY_FADEOUT,
    SEQ_QUIT,
  };

  RecPlayerCtrl_Main( wk, &wk->recPlayer );

  switch( wk->myState ){
  case SEQ_READ_ACMD:
    {
      BtlAdapterCmd  cmd = BTL_ADAPTER_RecvCmd(wk->adapter);

      setDummyReturnData( wk );
      if( cmd == BTL_ACMD_QUIT_BTL )
      {
        const BTL_ESCAPEINFO* p;
        BTL_ADAPTER_GetRecvData( wk->adapter, (const void**)&p );
        wk->escapeInfo = *p;
        BTL_N_Printf( DBGSTR_CLIENT_RecvedQuitCmd, wk->myID );
        wk->subSeq = 0;
        wk->myState = SEQ_RETURN_TO_SV_QUIT;
        break;
      }
      if( cmd != BTL_ACMD_NONE )
      {
        BOOL fRecCtrlLock;
        wk->subProc = getSubProc( wk, cmd, &fRecCtrlLock );
        if( fRecCtrlLock ){
          if( !RecPlayerCtrl_Lock(&wk->recPlayer) ){  // 既にフェード中でロックできない->フェード待ちへ
            wk->myState = SEQ_WAIT_RECPLAY_FADEOUT;
            break;
          }
        }else{
          RecPlayerCtrl_Unlock( &wk->recPlayer );
        }
        if( wk->subProc != NULL )
        {
          BTL_N_Printf( DBGSTR_CLIENT_StartCmd, wk->myID, cmd );
          wk->myState = SEQ_EXEC_CMD;
          wk->subSeq = 0;
        }
        else
        {
          wk->subSeq = 0;
          wk->myState = SEQ_RETURN_TO_SV;
        }
      }
      else
      {
        if( wk->fCommError ){
          BTL_N_Printf( DBGSTR_SV_CommError, __LINE__ );
          return TRUE;
        }
      }
    }
    break;

  case SEQ_EXEC_CMD:
    if( wk->subProc(wk, &wk->subSeq) )
    {
      if( wk->fCommError ){
        BTL_N_Printf( DBGSTR_SV_CommError, __LINE__ );
        return TRUE;
      }

      // １コマンド終了毎に、録画プレイヤーがブラックアウトさせているかチェック
      if( RecPlayer_CheckBlackOut(&wk->recPlayer) )
      {
        wk->myState = SEQ_RECPLAY_CTRL;
      }
      else
      {
        BTL_N_Printf( DBGSTR_CLIENT_RETURN_CMD_START, wk->myID );
        wk->myState = SEQ_RETURN_TO_SV;
      }
    }
    break;

  case SEQ_RETURN_TO_SV:
    if( BTL_ADAPTER_ReturnCmd(wk->adapter, wk->returnDataPtr, wk->returnDataSize) ){
      wk->myState = SEQ_READ_ACMD;
      BTL_N_Printf( DBGSTR_CLIENT_RETURN_CMD_DONE, wk->myID, wk->returnDataSize );
    }
    if( wk->fCommError ){
      BTL_N_Printf( DBGSTR_SV_CommError, __LINE__ );
      return TRUE;
    }
    break;

  case SEQ_RETURN_TO_SV_QUIT:
    if( BTL_ADAPTER_ReturnCmd(wk->adapter, wk->returnDataPtr, wk->returnDataSize) ){
      wk->myState = SEQ_QUIT;
      BTL_N_Printf( DBGSTR_CLIENT_ReplyToQuitCmd, wk->myID );
    }
    if( wk->fCommError ){
      BTL_N_Printf( DBGSTR_SV_CommError, __LINE__ );
      return TRUE;
    }
    break;

  // 録画再生コントロール：ブラックアウト待ち
  case SEQ_WAIT_RECPLAY_FADEOUT:
    if( RecPlayer_CheckBlackOut(&wk->recPlayer) ){
      wk->myState = SEQ_RECPLAY_CTRL;
    }
    break;

  // 録画再生コントロール：ブラックアウト後
  case SEQ_RECPLAY_CTRL:
    if( RecPlayer_GetCtrlCode(&wk->recPlayer) == RECCTRL_QUIT )
    {
      wk->myState = SEQ_QUIT;
    }
    else
    {
      if( wk->viewCore ){
        PMSND_FadeOutBGM( 30 );
      }
      wk->myState = SEQ_BGM_FADEOUT;
    }
    break;
  case SEQ_BGM_FADEOUT:
    if( !PMSND_CheckFadeOnBGM() )
    {
      // nextTurn を引数にして全クライアントの BTL_CLIENT_SetChapterSkip がコールバックされる
      u32 nextTurn = RecPlayer_GetNextTurn( &wk->recPlayer );

      PMSND_AllPlayerVolumeEnable( FALSE, PMSND_MASKPL_BGM );
      BTL_MAIN_ResetForRecPlay( wk->mainModule, nextTurn );
    }
    break;

  case SEQ_QUIT:
    return TRUE;
  }
  return FALSE;
}
/**
 *  メインループ：チャプタスキップ時
 */
static BOOL ClientMain_ChapterSkip( BTL_CLIENT* wk )
{
  enum {
    SEQ_RECPLAY_START = 0,
    SEQ_RECPLAY_READ_ACMD,
    SEQ_RECPLAY_EXEC_CMD,
    SEQ_RECPLAY_RETURN_TO_SV,
    SEQ_RECPLAY_FADEIN,

    SEQ_RECPLAY_QUIT,
  };

  RecPlayerCtrl_Main( wk, &wk->recPlayer );

  switch( wk->myState ){
  case SEQ_RECPLAY_START:
    if( wk->viewCore ){
//      BTLV_RecPlayer_StartSkip( wk->viewCore, RecPlayer_GetNextTurn(&wk->recPlayer) );
    }
    wk->myState = SEQ_RECPLAY_READ_ACMD;
    /* fallthru */

  case SEQ_RECPLAY_READ_ACMD:
    {
      BOOL fSkipEnd = RecPlayer_CheckChapterSkipEnd( &wk->recPlayer );
      if( !fSkipEnd )
      {
        BtlAdapterCmd  cmd = BTL_ADAPTER_RecvCmd( wk->adapter );
        if( cmd != BTL_ACMD_NONE )
        {
          wk->subProc = getSubProc( wk, cmd, NULL );
          if( wk->subProc != NULL ){
            wk->myState = SEQ_RECPLAY_EXEC_CMD;
            wk->subSeq = 0;
          }
          else
          {
            setDummyReturnData( wk );
            wk->myState = SEQ_RECPLAY_RETURN_TO_SV;
            wk->subSeq = 0;
          }
        }
      }
      else
      {
        BTL_N_Printf( DBGSTR_CLIENT_RecPlay_ChapterSkipped, wk->myID, wk->recPlayer.nextTurnCount);
        if( wk->viewCore ){
          PMSND_AllPlayerVolumeEnable( TRUE, PMSND_MASKPL_BGM );
          BTLV_RecPlayFadeIn_Start( wk->viewCore );
          PMSND_FadeInBGM( 30 );
        }
        wk->myState = SEQ_RECPLAY_FADEIN;
      }
    }
    break;

  case SEQ_RECPLAY_EXEC_CMD:
    if( wk->subProc(wk, &wk->subSeq) )
    {
      wk->myState = SEQ_RECPLAY_RETURN_TO_SV;
      wk->subSeq = 0;
    }
    break;

  case SEQ_RECPLAY_RETURN_TO_SV:
    if( BTL_ADAPTER_ReturnCmd(wk->adapter, wk->returnDataPtr, wk->returnDataSize) ){
      wk->myState = SEQ_RECPLAY_READ_ACMD;
      BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CLIENT_RETURN_CMD_DONE, wk->myID );
    }
    break;

  case SEQ_RECPLAY_FADEIN:
    if( wk->viewCore != NULL )
    {
      if( BTLV_RecPlayFadeIn_Wait(wk->viewCore)
      &&  !PMSND_CheckFadeOnBGM()
      ){
        u16 currentChapter = RecPlayer_GetNextTurn( &wk->recPlayer );
        BTLV_UpdateRecPlayerInput( wk->viewCore, currentChapter, currentChapter );
        BTL_MAIN_NotifyChapterSkipEnd( wk->mainModule );
      }
    }
    break;

  case SEQ_RECPLAY_QUIT:
    return TRUE;

  }
  return FALSE;
}
/**
 *  ダミー返信データ（サーバに返信する必要があるが内容は問われないケース）を作成
 */
static void setDummyReturnData( BTL_CLIENT* wk )
{
  wk->dummyReturnData = 0;
  wk->returnDataPtr = &(wk->dummyReturnData);
  wk->returnDataSize = sizeof(wk->dummyReturnData);
}

static ClientSubProc getSubProc( BTL_CLIENT* wk, BtlAdapterCmd cmd, BOOL* fRecCtrlLock )
{
  static const struct {
    BtlAdapterCmd   cmd;                            ///< コマンド
    u8              fRecCtrlLock;                   ///< 録画再生コントロールをロックする（コマンド処理中）
    ClientSubProc   proc[ BTL_CLIENT_TYPE_MAX ];    ///< コマンド処理関数ポインタテーブル
  }procTbl[] = {

    { BTL_ACMD_WAIT_SETUP,        TRUE,
      { SubProc_UI_Setup,         NULL,                      SubProc_REC_Setup          } },

    { BTL_ACMD_SELECT_ROTATION,    FALSE,
      { SubProc_UI_SelectRotation, SubProc_AI_SelectRotation, SubProc_REC_SelectRotation } },

#if 1
    { BTL_ACMD_SELECT_ACTION,     FALSE,
     { SubProc_UI_SelectAction,   SubProc_AI_SelectAction,   SubProc_REC_SelectAction   } },
#else
// AIにテスト駆動させる
    { BTL_ACMD_SELECT_ACTION,   FALSE,
       { SubProc_AI_SelectAction,  SubProc_AI_SelectAction,   SubProc_REC_SelectAction   } },
#endif

    { BTL_ACMD_SELECT_CHANGE_OR_ESCAPE,   FALSE,
       { SubProc_UI_SelectChangeOrEscape, NULL,  NULL  }
    },

    { BTL_ACMD_SELECT_POKEMON_FOR_COVER,    FALSE,
       { SubProc_UI_SelectPokemonForCover,  SubProc_AI_SelectPokemon,   SubProc_REC_SelectPokemon  }
    },

    { BTL_ACMD_SELECT_POKEMON_FOR_CHANGE,   FALSE,
       { SubProc_UI_SelectPokemonForChange, SubProc_AI_SelectPokemon,  SubProc_REC_SelectPokemon  } },

    { BTL_ACMD_CONFIRM_IREKAE,          FALSE,
       { SubProc_UI_ConfirmIrekae,      NULL,  SubProc_REC_SelectPokemon  } },

    { BTL_ACMD_SERVER_CMD,        FALSE,
       { SubProc_UI_ServerCmd,    NULL,                      SubProc_REC_ServerCmd      } },

    { BTL_ACMD_RECORD_DATA,       FALSE,
       { SubProc_UI_RecordData,   NULL,                      NULL                       } },

    { BTL_ACMD_EXIT_NPC,          TRUE,
      { SubProc_UI_ExitForNPC,    NULL,     SubProc_REC_ExitForNPC }, },

    { BTL_ACMD_EXIT_SUBWAY_TRAINER,         TRUE,
        { SubProc_UI_ExitForSubwayTrainer,  NULL,   SubProc_REC_ExitForSubwayTrainer }, },

    { BTL_ACMD_EXIT_WIN_WILD,     TRUE,
      { SubProc_UI_WinWild,   NULL,  NULL }, },

    { BTL_ACMD_EXIT_LOSE_WILD,    TRUE,
      { SubProc_UI_LoseWild,   NULL,  NULL }, },

    { BTL_ACMD_EXIT_COMM,         TRUE,
      { SubProc_UI_ExitCommTrainer,   NULL,  SubProc_REC_ExitCommTrainer }, },

    { BTL_ACMD_NOTIFY_TIMEUP,     TRUE,
      { SubProc_UI_NotifyTimeUp,  NULL,  NULL }, },
  };

  int i;

  for(i=0; i<NELEMS(procTbl); i++)
  {
    if( procTbl[i].cmd == cmd )
    {
      if( fRecCtrlLock != NULL ){
        *fRecCtrlLock = procTbl[i].fRecCtrlLock;
      }
      return procTbl[i].proc[ wk->myType ];
    }
  }

  GF_ASSERT(0);
  return NULL;
}



//=============================================================================================
/**
 * 逃げたクライアントのIDを取得（サーバコマンドを介して送られてくる）
 *
 * @param   wk
 *
 * @retval  u8
 */
//=============================================================================================
void BTL_CLIENT_GetEscapeInfo( const BTL_CLIENT* wk, BTL_ESCAPEINFO* dst )
{
  *dst = wk->escapeInfo;
}

//=============================================================================================
/**
 * ゲーム制限時間の終了チェック
 *
 * @param   wk
 *
 * @retval  BOOL    ゲームに制限時間が設定されており、かつそれが終わっている場合にTRUE
 */
//=============================================================================================
BOOL BTL_CLIENT_IsGameTimeOver( const BTL_CLIENT* wk )
{
  if( wk->gameLimitTime )
  {
    return BTLV_EFFECT_IsZero( BTLV_TIMER_TYPE_GAME_TIME );
  }
  return FALSE;
}

//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------



static BOOL SubProc_UI_Setup( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    if( BTL_MAIN_GetCompetitor(wk->mainModule) != BTL_COMPETITOR_DEMO_CAPTURE ){
      BTLV_StartCommand( wk->viewCore, BTLV_CMD_SETUP );
    }else{
      BTLV_StartCommand( wk->viewCore, BTLV_CMD_SETUP_DEMO );
    }
    (*seq)++;
    break;
  case 1:
    if( BTLV_WaitCommand(wk->viewCore) )
    {
      wk->cmdLimitTime  = BTL_MAIN_GetCommandLimitTime( wk->mainModule );
      wk->gameLimitTime = BTL_MAIN_GetGameLimitTime( wk->mainModule );
      if( wk->cmdLimitTime || wk->gameLimitTime )
      {
        BTLV_EFFECT_CreateTimer( wk->gameLimitTime, wk->cmdLimitTime );
        if( wk->gameLimitTime ){
          BTLV_EFFECT_DrawEnableTimer( BTLV_TIMER_TYPE_GAME_TIME, TRUE, TRUE );
        }
      }

      EnemyPokeHPBase_Update( wk );

      // 録画スキップ状態の場合、スクリーン設定
      if( BTL_CLIENT_IsChapterSkipMode(wk) )
      {
        BTLV_RecPlayer_StartSkip( wk->viewCore, RecPlayer_GetNextTurn(&wk->recPlayer) );
      }

      return TRUE;
    }
    break;
  }
  return FALSE;
}

static BOOL SubProc_REC_Setup( BTL_CLIENT* wk, int* seq )
{
  if( wk->viewCore )
  {
    BOOL fDone = SubProc_UI_Setup( wk, seq );
    if( fDone )
    {
      if( BTL_RECREADER_CheckBtlInChapter(wk->btlRecReader, wk->myID) )
      {
        TAYA_Printf("バトル開始チャプタあり\n");
        RecPlayer_TurnIncReq( &wk->recPlayer );
      }
    }
    return fDone;
  }

  BTL_RECREADER_CheckBtlInChapter( wk->btlRecReader, wk->myID );
  return TRUE;
}
//------------------------------------------------------------------------------------------------------
// 敵先頭ポケモンの基準HP値を記録（ポケモン引っ込めメッセージ用：非通信、シングル戦のみ）
//------------------------------------------------------------------------------------------------------
static void EnemyPokeHPBase_Update( BTL_CLIENT* wk )
{
  const BTL_POKEPARAM* bpp = EnemyPokeHPBase_GetTargetPoke( wk );
  if( bpp ){
    wk->EnemyPokeHPBase = BPP_GetValue( bpp, BPP_HP );
    BTL_N_Printf( DBGSTR_CLIENT_UpdateEnemyBaseHP, wk->EnemyPokeHPBase, BPP_GetID(bpp) );
  }
}
static u32 EnemyPokeHPBase_CheckRatio( BTL_CLIENT* wk )
{
  const BTL_POKEPARAM* bpp = EnemyPokeHPBase_GetTargetPoke( wk );
  if( bpp ){
    u32 hp = BPP_GetValue( bpp, BPP_HP );
    if( hp >= wk->EnemyPokeHPBase ){
      return 0;
    }
    return ((wk->EnemyPokeHPBase - hp) * 100) / hp;
  }
  return 0;
}
static const BTL_POKEPARAM* EnemyPokeHPBase_GetTargetPoke( BTL_CLIENT* wk )
{
  if( (BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_SINGLE)
  &&  (BTL_MAIN_GetCommMode(wk->mainModule) == BTL_COMM_NONE)
  ){
    u8 clientID = BTL_MAIN_GetOpponentClientID( wk->mainModule, wk->myID, 0 );
    return BTL_POKECON_GetClientPokeDataConst( wk->pokeCon, clientID, 0 );
  }
  return NULL;
}



//------------------------------------------------------------------------------------------------------
// ローテーション選択
//------------------------------------------------------------------------------------------------------

static BOOL SubProc_UI_SelectRotation( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    {
      BTLV_UI_SelectRotation_Start( wk->viewCore, wk->prevRotateDir );
      CmdLimit_Start( wk );
      (*seq)++;
    }
    break;

  case 1:
    if( CmdLimit_CheckOver(wk) ){
      BTLV_UI_SelectRotation_ForceQuit( wk->viewCore );
      (*seq) = 2;
      break;
    }

    {
      BtlRotateDir  dir;
      if( BTLV_UI_SelectRotation_Wait(wk->viewCore, &dir) ){
        wk->prevRotateDir = dir;
        (*seq) = 3;
      }
    }
    break;

  // 強制終了時のランダム決定処理
  case 2:
    {
      wk->prevRotateDir = decideNextDirRandom( wk->prevRotateDir );
      (*seq)++;
    }
    break;

  case 3:
    CmdLimit_End( wk );
    (*seq)++;
    break;
  case 4:
    wk->returnDataPtr = &wk->prevRotateDir;
    wk->returnDataSize = sizeof(wk->prevRotateDir);
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 制限時間による強制終了時、ランダムで方向を決定する
 *
 * @param   prevDir
 *
 * @retval  BtlRotateDir
 */
//----------------------------------------------------------------------------------
static BtlRotateDir decideNextDirRandom( BtlRotateDir prevDir )
{
  u32 rnd = GFL_STD_MtRand( 100 );

  if( prevDir == BTL_ROTATEDIR_NONE )
  {
    if( rnd < 30 ){
      prevDir = BTL_ROTATEDIR_L;
    }else if( rnd < 60 ){
      prevDir = BTL_ROTATEDIR_R;
    }else{
      prevDir = BTL_ROTATEDIR_STAY;
    }
  }
  else
  {
    switch( prevDir ){
    case BTL_ROTATEDIR_STAY:
    default:
      prevDir = (rnd < 50)? BTL_ROTATEDIR_R : BTL_ROTATEDIR_L;
      break;

    case BTL_ROTATEDIR_L:
      prevDir = (rnd < 40)? BTL_ROTATEDIR_R : BTL_ROTATEDIR_STAY;
      break;

    case BTL_ROTATEDIR_R:
      prevDir = (rnd < 40)? BTL_ROTATEDIR_L : BTL_ROTATEDIR_STAY;
      break;
    }
  }

  return prevDir;
}


static BOOL SubProc_REC_SelectRotation( BTL_CLIENT* wk, int* seq )
{
  return TRUE;
}

static BOOL SubProc_AI_SelectRotation( BTL_CLIENT* wk, int* seq )
{
  wk->prevRotateDir = decideNextDirRandom( wk->prevRotateDir );
  wk->returnDataPtr = &wk->prevRotateDir;
  wk->returnDataSize = sizeof(wk->prevRotateDir);

  return TRUE;
}

//----------------------------------------------------------------------------------
/**
 * 制限時間 表示＆カウント開始
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void CmdLimit_Start( BTL_CLIENT* wk )
{
  if( wk->cmdLimitTime )
  {
    BTL_N_Printf( DBGSTR_CLIENT_CmdLimitTimeEnable, wk->cmdLimitTime );
    BTLV_EFFECT_DrawEnableTimer( BTLV_TIMER_TYPE_COMMAND_TIME, TRUE, TRUE );
    wk->fForceQuitSelAct = FALSE;
  }
}
//----------------------------------------------------------------------------------
/**
 * 制限時間 タイムオーバーチェック
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL CmdLimit_CheckOver( BTL_CLIENT* wk )
{
  if( wk->cmdLimitTime )
  {
    if( wk->fForceQuitSelAct == FALSE )
    {
      if( BTLV_EFFECT_IsZero( BTLV_TIMER_TYPE_COMMAND_TIME ) ){
        BTL_N_Printf( DBGSTR_CLIENT_CmdLimitTimeOver );
        wk->fForceQuitSelAct = TRUE;
      }
    }
    return wk->fForceQuitSelAct;
  }
  return FALSE;
}

/**
 *  コマンド時間制限による強制終了があれば、メインプロセスを切り替えてTRUEを返す
 */
static BOOL CheckSelactForceQuit( BTL_CLIENT* wk, ClientSubProc nextProc )
{
  if( CmdLimit_CheckOver(wk) )
  {
    if( nextProc ){
      ClientSubProc_Set( wk, nextProc );
    }
    return TRUE;
  }
  return FALSE;
}
/**
 *  コマンド時間制限表示を終了
 */
static void CmdLimit_End( BTL_CLIENT* wk )
{
  if( wk->cmdLimitTime )
  {
    BTLV_EFFECT_DrawEnableTimer( BTLV_TIMER_TYPE_COMMAND_TIME, FALSE, FALSE );
  }
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 *  ○○はどうする？　文字列ID取得
 */
static void SetupSelectStartStr( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, BTLV_STRPARAM* strParam )
{
  if( BTL_MAIN_GetRule(wk->mainModule) != BTL_RULE_ROTATION )
  {
    BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_STD, BTL_STRID_STD_SelectAction );
    BTLV_STRPARAM_AddArg( strParam, BPP_GetID(procPoke) );
    BTLV_STRPARAM_SetWait( strParam, 0 );
  }
  else
  {
    BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_STD, BTL_STRID_STD_SelectRotation );
    BTLV_STRPARAM_AddArg( strParam, wk->myID );
    BTLV_STRPARAM_SetWait( strParam, 0 );
  }
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 *  サブプロセス設定
 */
static void ClientSubProc_Set( BTL_CLIENT* wk, ClientSubProc proc )
{
  wk->selActProc = proc;
  wk->selActSeq = 0;
}
/**
 *  サブプロセスコール
 */
static BOOL ClientSubProc_Call( BTL_CLIENT* wk )
{
  return wk->selActProc( wk, &wk->selActSeq );
}



static BOOL SubProc_UI_SelectAction( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    ClientSubProc_Set( wk, selact_Start );
    CmdLimit_Start( wk );
    (*seq)++;
    break;

  case 1:
    CmdLimit_CheckOver( wk );
    if( ClientSubProc_Call(wk) )
    {
      #ifdef PM_DEBUG
      AICtrl_Delegate( wk );
      #endif
      return TRUE;
    }
    break;
  }

  return FALSE;
}

static BOOL SubProc_REC_SelectAction( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    {
      u8 numAction, fChapter;

      const BTL_ACTION_PARAM* act = BTL_RECREADER_ReadAction( wk->btlRecReader, wk->myID, &numAction, &fChapter );
      if( fChapter ){
        RecPlayer_TurnIncReq( &wk->recPlayer );
      }

      wk->returnDataPtr = act;
      wk->returnDataSize = numAction * sizeof(BTL_ACTION_PARAM);

      // 再生時タイムオーバー検出処理
      if( act->gen.cmd == BTL_ACTION_RECPLAY_TIMEOVER )
      {
        BTL_N_Printf( DBGSTR_CLIENT_ReadRecTimerOver, wk->myID );

        // 描画クライアントなのでメッセージ表示へ
        if( wk->viewCore )
        {
          BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_BattleTimeOver );
          BTLV_StartMsg( wk->viewCore, &wk->strParam );

          (*seq)++;
        // 描画クライアントではないので終了
        }else{
          setNullActionRecplay( wk );
          return TRUE;
        }
      }
      // 読み込みエラーなら何も言わず終了
      else if( act->gen.cmd == BTL_ACTION_RECPLAY_ERROR )
      {
        BTL_N_Printf( DBGSTR_CLIENT_ReadRecError, wk->myID );
        setNullActionRecplay( wk );
        return TRUE;
      }
      else
      {
        BTL_N_Printf( DBGSTR_CLIENT_ReadRecAct, wk->myID, numAction);
        if( act->gen.cmd  == BTL_ACTION_FIGHT ){
          BTL_N_Printf( DBGSTR_CLIENT_ReadRecAct_Fight, act->fight.waza);
        }
        if( act->gen.cmd  == BTL_ACTION_CHANGE ){
          BTL_N_Printf( DBGSTR_CLIENT_ReadRecAct_Change, act->change.memberIdx);
        }
        if( act->gen.cmd  == BTL_ACTION_MOVE ){
          BTL_N_Printf( DBGSTR_CLIENT_ReadRecAct_Move );
        }
        return TRUE;
      }
    }
    break;

  case 1:
    if( BTLV_WaitMsg(wk->viewCore) ){
      setNullActionRecplay( wk );
      return TRUE;
    }
    break;
  }
  return FALSE;
}
/**
 *  タイムオーバー検出時などの無効アクションパラメータ返信設定
 */
static void setNullActionRecplay( BTL_CLIENT* wk )
{
  wk->procAction = &wk->actionParam[ 0 ];
  BTL_ACTION_SetNULL( wk->procAction );
  wk->returnDataPtr = wk->procAction;
  wk->returnDataSize = sizeof(BTL_ACTION_PARAM);
}



//----------------------------------------------------------------------
/**
 *  アクション選択ワーク初期化
 */
//----------------------------------------------------------------------
static BOOL selact_Start( BTL_CLIENT* wk, int* seq )
{
  wk->procPokeIdx = 0;
  wk->prevPokeIdx = -1;
  wk->firstPokeIdx = 0;
  wk->fBallSelected = FALSE;

  // ダブル以上の時、「既に選ばれているポケモン」を記録するために初期化をここで行う
  setupPokeSelParam( wk, BPL_MODE_NORMAL, wk->numCoverPos, &wk->pokeSelParam, &wk->pokeSelResult  );

  shooterCost_Init( wk );
  selItemWork_Init( wk );

  {
    const BTL_POKEPARAM* bpp;
    u32 i;

    for(i=0; i<NELEMS(wk->actionParam); ++i){
      BTL_ACTION_SetNULL( &wk->actionParam[i] );
    }

    for(i=0; i<wk->numCoverPos; ++i)
    {
      bpp = BTL_POKECON_GetClientPokeData( wk->pokeCon, wk->myID, i );
      if( !checkActionForceSet(wk, bpp,  NULL) ){
        wk->firstPokeIdx = i;
        break;
      }
    }
  }


  ClientSubProc_Set( wk, selact_Root );

  return FALSE;
}
/**
 *  アクション選択途中の標準メッセージウィンドウ文字出力
 */
static void selact_startMsg( BTL_CLIENT* wk, const BTLV_STRPARAM* strParam )
{
  BTLV_StartMsg( wk->viewCore, strParam );
  wk->fStdMsgChanged = TRUE; // 「○○はどうする？」メッセージを書き換えたフラグON
}
//----------------------------------------------------------------------
/**
 *  アクション選択の強制終了
 */
//----------------------------------------------------------------------
static BOOL selact_ForceQuit( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    BTLV_ForceQuitInput_Notify( wk->viewCore );
    (*seq)++;
    break;
  case 1:
    if( BTLV_ForceQuitInput_Wait(wk->viewCore) )
    {
      while( wk->procPokeIdx < wk->numCoverPos )
      {
        wk->procPoke = BTL_POKECON_GetClientPokeData( wk->pokeCon, wk->myID, wk->procPokeIdx );
        wk->procAction = &wk->actionParam[ wk->procPokeIdx ];
        BTL_ACTION_SetNULL( wk->procAction );
        if( !checkActionForceSet(wk, wk->procPoke,  wk->procAction) )
        {
          u8 usableWazaFlag[ PTL_WAZA_MAX ];
          u8 wazaIdx = StoreSelectableWazaFlag( wk, wk->procPoke, usableWazaFlag );
          if( wazaIdx < PTL_WAZA_MAX )
          {
            WazaID waza = BPP_WAZA_GetID( wk->procPoke, wazaIdx );
            BtlPokePos  targetPos = BTL_CALC_DecideWazaTargetAuto( wk->mainModule, wk->pokeCon, wk->procPoke, waza );
            BTL_ACTION_SetFightParam( wk->procAction, waza, targetPos );
          }else{
            setWaruagakiAction( wk->procAction, wk, wk->procPoke );
          }
        }
        wk->procPokeIdx++;
      }
      (*seq)++;
    }
    break;
  case 2:
    wk->returnDataPtr = &(wk->actionParam[0]);
    wk->returnDataSize = sizeof(wk->actionParam[0]) * wk->numCoverPos;
    ClientSubProc_Set( wk, selact_Finish );
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------
/**
 *  ゆびをふるデバッグ
 */
//----------------------------------------------------------------------
#ifdef PM_DEBUG
static BOOL selact_YubiFuruDebug( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    GYubiCtrlPos = 0;
    wk->fStdMsgChanged = FALSE;
    (*seq)++;
    /* fallthru */
  case 1:
    BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_YubiFuruDebug );
    BTLV_STRPARAM_AddArg( &wk->strParam, GYubiCtrlPos );
    BTLV_STRPARAM_AddArg( &wk->strParam, GYubiFuruDebugNumber[GYubiCtrlPos] );
    BTLV_STRPARAM_AddArg( &wk->strParam, GYubiFuruDebugNumber[GYubiCtrlPos] );
    BTLV_PrintMsgAtOnce( wk->viewCore, &wk->strParam );
    (*seq)++;
    break;
  case 2:
    if( !BTLV_WaitMsg(wk->viewCore) ){
      (*seq)++;
    }
    break;
  case 3:
    {
      u16 key = GFL_UI_KEY_GetRepeat();
      u8 fUpdate = TRUE;

      do  {
        if( key & PAD_KEY_LEFT ){
          GYubiCtrlPos = (GYubiCtrlPos)? (GYubiCtrlPos-1) : (BTL_POS_MAX-1);
          break;
        }
        if( key & PAD_KEY_RIGHT ){
          if( ++GYubiCtrlPos >= (BTL_POS_MAX) ){ GYubiCtrlPos = 0; }
          break;
        }
        if( key & PAD_KEY_UP ){
          GYubiFuruDebugNumber[ GYubiCtrlPos ]++;
          break;
        }
        if( key & PAD_KEY_DOWN ){
          GYubiFuruDebugNumber[ GYubiCtrlPos ]--;
          break;
        }
        if( key & PAD_BUTTON_X ){
          GYubiFuruDebugNumber[ GYubiCtrlPos ] += 10;
          break;
        }
        if( key & PAD_BUTTON_Y ){
          GYubiFuruDebugNumber[ GYubiCtrlPos ] -= 10;
          break;
        }
        if( key & PAD_BUTTON_R ){
          GYubiFuruDebugNumber[ GYubiCtrlPos ] += 100;
          break;
        }
        if( key & PAD_BUTTON_L ){
          GYubiFuruDebugNumber[ GYubiCtrlPos ] -= 100;
          break;
        }

        fUpdate = FALSE;

      }while(0);

      if( fUpdate )
      {
        if( GYubiFuruDebugNumber[ GYubiCtrlPos ] >= WAZANO_MAX ){
          GYubiFuruDebugNumber[ GYubiCtrlPos ] = 0;
        }
        else if( GYubiFuruDebugNumber[ GYubiCtrlPos ] < 0 ){
          GYubiFuruDebugNumber[ GYubiCtrlPos ] = WAZANO_MAX - 1;
        }
        (*seq) = 1;
        break;
      }
      else
      {
        if( key & PAD_BUTTON_A ){
          BTL_TABLES_YubifuruDebugSetEnd();
//          BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, SelActStrID(wk) );
//          BTLV_STRPARAM_AddArg( &wk->strParam, BPP_GetID(wk->procPoke) );
//          BTLV_STRPARAM_SetWait( &wk->strParam, 0 );
          SetupSelectStartStr( wk, wk->procPoke, &wk->strParam );
          BTLV_PrintMsgAtOnce( wk->viewCore, &wk->strParam );
          ClientSubProc_Set( wk, selact_Root );
          (*seq) = wk->yubifuruDebugRetSeq;
        }
      }
    }
  }
  return FALSE;
}
#endif

//----------------------------------------------------------------------
/**
 *  アクション選択ルート
 */
//----------------------------------------------------------------------
static BOOL selact_Root( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    wk->procPoke = BTL_POKECON_GetClientPokeData( wk->pokeCon, wk->myID, wk->procPokeIdx );
    wk->procAction = &wk->actionParam[ wk->procPokeIdx ];
    wk->actionAddCount = 0;
    BTL_ACTION_SetNULL( wk->procAction );

    BTL_N_Printf( DBGSTR_CLIENT_SelectActionRoot, wk->procPokeIdx, BPP_GetID(wk->procPoke), wk->procAction );

    if( checkActionForceSet(wk, wk->procPoke,  wk->procAction) ){
      BTL_N_Printf( DBGSTR_CLIENT_SelectActionSkip, wk->procPokeIdx );
      ClientSubProc_Set( wk, selact_CheckFinish );
    }
    else
    {
      if( check_tr_message( wk, &wk->AITrainerMsgID ) == TRUE )
      {
        ClientSubProc_Set( wk, selact_TrainerMessage );
      }
      else
      {
        (*seq)++;
      }
    }
    break;

  case 1:
    // 「○○はどうする？」表示
    if( (wk->prevPokeIdx != wk->procPokeIdx)
    ||  (wk->fStdMsgChanged)
    ){
      SetupSelectStartStr( wk, wk->procPoke, &wk->strParam );
      BTLV_PrintMsgAtOnce( wk->viewCore, &wk->strParam );
      wk->fStdMsgChanged = FALSE;
      wk->prevPokeIdx = wk->procPokeIdx;
      (*seq)++;
    }
    else{
      (*seq) += 2;
    }
    break;
  case 2:
    if( !BTLV_WaitMsg(wk->viewCore) ){
      return FALSE;
    }
    (*seq)++;
    /* fallthru */

  case 3:
    // アクション選択開始
    BTL_N_Printf( DBGSTR_CLIENT_SelectActionStart, wk->procPokeIdx, BPP_GetID(wk->procPoke), wk->firstPokeIdx );
    BTLV_UI_SelectAction_Start( wk->viewCore, wk->procPoke, (wk->procPokeIdx>wk->firstPokeIdx), wk->procAction );
    (*seq)++;
    break;

  case 4:

    // デバッグ用Print制御
    #ifdef PM_DEBUG
    if( OS_GetConsoleType() == OS_CONSOLE_TWLDEBUGGER ){
      BTL_DEBUGPRINT_Ctrl();
    }
    #endif

    // デバッグ用ゆびをふる制御
    #ifdef PM_DEBUG
    if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_R) && (GFL_UI_KEY_GetCont() & PAD_BUTTON_Y) )
    {
      if( BTL_MAIN_CheckImServerMachine(wk->mainModule) ){
        wk->yubifuruDebugRetSeq = (*seq);
        ClientSubProc_Set( wk, selact_YubiFuruDebug );
        return FALSE;
      }
    }
    #endif

    if( CheckSelactForceQuit(wk, selact_ForceQuit) )
    {
      BTL_N_Printf( DBGSTR_CLIENT_ForceQuitByTimeLimit, wk->myID );
      BTLV_UI_SelectAction_ForceQuit( wk->viewCore );
      return FALSE;
    }

    switch( BTLV_UI_SelectAction_Wait(wk->viewCore) ){

    // 入れ替えポケモン選択の場合はまだアクションパラメータが不十分->ポケモン選択へ
    case BTL_ACTION_CHANGE:
      BTL_N_Printf( DBGSTR_CLIENT_SelectAction_Pokemon );
      shooterCost_Save( wk, wk->procPokeIdx, 0 );
      ClientSubProc_Set( wk, selact_SelectChangePokemon );
      break;

    // 「たたかう」を選んだ
    case BTL_ACTION_FIGHT:
      BTL_N_Printf( DBGSTR_CLIENT_SelectAction_Fight );
      shooterCost_Save( wk, wk->procPokeIdx, 0 );
      ClientSubProc_Set( wk, selact_Fight );
      break;

    // 「どうぐ」を選んだ
    case BTL_ACTION_ITEM:
      // シューター使えない設定チェック
      if( wk->bagMode == BBAG_MODE_SHOOTER )
      {
        if( !BTL_MAIN_IsShooterEnable(wk->mainModule) )
        {
          BTLV_UI_Restart( wk->viewCore );
          (*seq) = 5;
          break;
        }
      }

      ClientSubProc_Set( wk, selact_Item );
      break;

    // 「にげる」or「もどる」
    case BTL_ACTION_ESCAPE:
      // 先頭のポケなら「にげる」として処理
      if( wk->procPokeIdx == wk->firstPokeIdx ){
        shooterCost_Save( wk, wk->procPokeIdx, 0 );
        ClientSubProc_Set( wk, selact_Escape );
      // ２体目以降は「もどる」として処理
      }else{
        BTL_POKEPARAM* bpp;
        while( wk->procPokeIdx )
        {
          wk->procPokeIdx--;
          bpp = BTL_POKECON_GetClientPokeData( wk->pokeCon, wk->myID, wk->procPokeIdx );
          if( !checkActionForceSet(wk, bpp, NULL) )
          {
            wk->shooterEnergy += shooterCost_Get( wk, wk->procPokeIdx );

            // 「もどる」先のポケモンが、既に「ポケモン」で交換対象を選んでいた場合はその情報をPopする
            if( BTL_ACTION_GetAction( &wk->actionParam[wk->procPokeIdx] ) == BTL_ACTION_CHANGE ){
              BTL_POKESELECT_RESULT_Pop( &wk->pokeSelResult );
            }
            // 「もどる」先のポケモンが、既に「どうぐ」で道具を選んでいた場合は以下同文
            if( BTL_ACTION_GetAction( &wk->actionParam[wk->procPokeIdx] ) == BTL_ACTION_ITEM ){
              selItemWork_Restore( wk, wk->procPokeIdx );
            }
            ClientSubProc_Set( wk, selact_Root );
            return FALSE;
          }
        }
        GF_ASSERT(0);
      }
      break;
    }
    break;

  case 5:
    if( BTLV_UI_WaitRestart(wk->viewCore) ){
      (*seq) = 3;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------
/**
 *  アクション選択直前のトレーナーメッセージ表示処理
 */
//----------------------------------------------------------------------
static BOOL selact_TrainerMessage( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    {
      u8 clientID = BTL_MAIN_GetEnemyClientID( wk->mainModule, 0 );
      u32 trainerID = BTL_MAIN_GetClientTrainerID( wk->mainModule, clientID );
      int trtype = BTL_MAIN_GetClientTrainerType( wk->mainModule, clientID );

      BTLV_EFFECT_SetTrainer( trtype, BTLV_MCSS_POS_TR_BB, 0, 0, 0 );
      BTLV_EFFECT_Add( BTLEFF_TRAINER_IN );
      BTLV_StartMsgTrainer( wk->viewCore, trainerID, wk->AITrainerMsgID );

      if( ((wk->AITrainerMsgID==TRMSG_FIGHT_POKE_LAST) || (wk->AITrainerMsgID==TRMSG_FIGHT_POKE_LAST_HP_HALF))
      ){
        u16 trType = BTL_MAIN_GetClientTrainerType( wk->mainModule, clientID );
        if( BTL_CALC_IsTrtypeGymLeader(trType) && ( wk->fAITrainerBGMChanged == FALSE ) ){
          BTLV_EFFECT_SetTrainerBGMChangeFlag( SEQ_BGM_BATTLESUPERIOR );
          if( BTLV_EFFECT_GetPinchBGMFlag() == 0 )
          {
            PMSND_FadeOutBGM( 8 );
            (*seq) = 1;
            break;
          }
        }
      }
      (*seq) = 2;
    }
    break;
  case 1:
    if( PMSND_CheckFadeOnBGM() == FALSE )
    {
      PMSND_PlayBGM( SEQ_BGM_BATTLESUPERIOR );
      wk->fAITrainerBGMChanged = TRUE;
      (*seq)++;
    }
    break;
  case 2:
    if( !BTLV_EFFECT_CheckExecute()
    &&  BTLV_WaitMsg(wk->viewCore)
    ){
      BTLV_EFFECT_Add( BTLEFF_TRAINER_OUT );
      (*seq)++;
    }
    break;
  case 3:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      ClientSubProc_Set( wk, selact_Root );
    }
    break;
  }

  return FALSE;
}
/**
 *  トレーナーメッセージ表示タイミングかどうか判定
 */
static  BOOL  check_tr_message( BTL_CLIENT* wk, u16* msgID )
{
  if( BTL_MAIN_GetCompetitor(wk->mainModule) == BTL_COMPETITOR_TRAINER )
  {
    static const u16 TRMsgTbl[] = {
      TRMSG_FIGHT_POKE_HP_HALF,       ///< HP半分以下（初回）
      TRMSG_FIGHT_FIRST_DAMAGE,       ///< HP満タンでない（初回）
      TRMSG_FIGHT_POKE_LAST,          ///< 最後の１体登場
      TRMSG_FIGHT_POKE_LAST_HP_HALF,  ///< 最後の１体がHP半分以下
    };

    u8 clientID = BTL_MAIN_GetEnemyClientID( wk->mainModule, 0 );
    u32 trainerID = BTL_MAIN_GetClientTrainerID( wk->mainModule, clientID );

    if( trainerID != TRID_NULL )
    {
      BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
      const BTL_POKEPARAM* bpp = BTL_POKECON_GetClientPokeData( wk->pokeCon, clientID, 0 );
      u32 i;
      int enableIndex;

      // 場に出ているポケで生きているヤツを探しておく
      if( BPP_IsDead(bpp) )
      {
        u32 numCoverPos = BTL_MAIN_GetClientFrontPosCount( wk->mainModule, clientID );
        for(i=1; i<numCoverPos; ++i){
          bpp = BTL_POKECON_GetClientPokeData( wk->pokeCon, clientID, i );
          if( !BPP_IsDead(bpp) ){
            break;
          }
        }
      }

      enableIndex = -1;
      for(i=0; i<NELEMS(TRMsgTbl); ++i)
      {
        if( wk->AITrainerMsgCheckedFlag[i] == FALSE )
        {
          if( TT_TrainerMessageCheck(trainerID, TRMsgTbl[i], wk->heapID ) )
          {
            switch( TRMsgTbl[i] ){
            case TRMSG_FIGHT_POKE_HP_HALF:
              if( BPP_GetValue(bpp,BPP_HP) <= BTL_CALC_QuotMaxHP_Zero(bpp,2) ){
                enableIndex = i;
                wk->AITrainerMsgCheckedFlag[ i ] = TRUE;
              }
              break;

            case TRMSG_FIGHT_FIRST_DAMAGE:       ///< HP満タンでない（初回）
              if( !BPP_IsHPFull(bpp) ){
                enableIndex = i;
                wk->AITrainerMsgCheckedFlag[ i ] = TRUE;
              }
              break;

            case TRMSG_FIGHT_POKE_LAST:          ///< 最後の１体登場
              if( (BTL_PARTY_GetMemberCount(party) > 1)
              &&  (BTL_PARTY_GetAliveMemberCount(party) == 1)
              ){
                enableIndex = i;
                wk->AITrainerMsgCheckedFlag[ i ] = TRUE;
              }
              break;

            case TRMSG_FIGHT_POKE_LAST_HP_HALF:  ///< 最後の１体がHP半分以下
              if( (BTL_PARTY_GetMemberCount(party) > 1)
              &&  (BTL_PARTY_GetAliveMemberCount(party) == 1)
              &&  (BPP_GetValue(bpp,BPP_HP) <= BTL_CALC_QuotMaxHP_Zero(bpp,2) )
              ){
                enableIndex = i;
                wk->AITrainerMsgCheckedFlag[ i ] = TRUE;
              }
            }
          }
          else
          {
            wk->AITrainerMsgCheckedFlag[i] = TRUE;
          }
        }
      } /* for( i<NELEMS(TRMsgTbl) ) */

      if( enableIndex >= 0 )
      {
        *msgID = TRMsgTbl[ enableIndex ];
        return TRUE;
      }

    } /* if( trainerID != TRID_NULL ) */
  }

  return FALSE;
}



//----------------------------------------------------------------------
/**
 *  「たたかう」選択後の分岐
 */
//----------------------------------------------------------------------
static BOOL selact_Fight( BTL_CLIENT* wk, int* seq )
{
  enum {
    SEQ_START = 0,
    SEQ_SELECT_WAZA_START,
    SEQ_SELECT_ROTATION_WAZA_START,
    SEQ_SELECT_WAZA_WAIT,
    SEQ_CHECK_WAZA_TARGET,
    SEQ_SELECT_TARGET_START,
    SEQ_SELECT_TARGET_WAIT,
    SEQ_WAIT_UNSEL_WAZA_MSG,
  };

  switch( *seq ){
  case SEQ_START:
    if( BTL_MAIN_GetRule(wk->mainModule) != BTL_RULE_ROTATION )
    {
      if( checkWazaForceSet( wk, wk->procPoke, wk->procAction ) ){
        ClientSubProc_Set( wk, selact_CheckFinish );
      }else{
        (*seq) = SEQ_SELECT_WAZA_START;
      }
    }
    else
    {
      #ifdef ROTATION_NEW_SYSTEM
      setupRotationParams( wk, &wk->rotWazaSelParam );
      (*seq) = SEQ_SELECT_ROTATION_WAZA_START;
      #else
      if( checkWazaForceSet( wk, wk->procPoke, wk->procAction ) ){
        ClientSubProc_Set( wk, selact_CheckFinish );
      }else{
        (*seq) = SEQ_SELECT_WAZA_START;
      }
      #endif
    }
    break;

  case SEQ_SELECT_WAZA_START:
    BTLV_UI_SelectWaza_Start( wk->viewCore, wk->procPoke, wk->procAction );
    (*seq) = SEQ_SELECT_WAZA_WAIT;
    break;

  case SEQ_SELECT_ROTATION_WAZA_START:
    BTLV_UI_SelectRotationWaza_Start( wk->viewCore, &wk->rotWazaSelParam, wk->procAction );
    (*seq) = SEQ_SELECT_WAZA_WAIT;
    break;

  case SEQ_SELECT_WAZA_WAIT:
    if( CheckSelactForceQuit(wk, selact_ForceQuit) )
    {
      BTL_N_Printf( DBGSTR_CLIENT_ForceQuitByTimeLimit, wk->myID );
      BTLV_UI_SelectWaza_ForceQuit( wk->viewCore );
      return FALSE;
    }

    if( BTLV_UI_SelectWaza_Wait(wk->viewCore) )
    {
      BtlAction action = BTL_ACTION_GetAction( wk->procAction );

      if( action == BTL_ACTION_NULL ){
        ClientSubProc_Set( wk, selact_Root );
      }
      else if( action == BTL_ACTION_MOVE ){
        ClientSubProc_Set( wk, selact_CheckFinish );
      }
      else
      {
        const BTL_POKEPARAM* procPoke = wk->procPoke;
        BTL_ACTION_PARAM*    procAction = wk->procAction;
        BtlRotateDir dir = BTL_ROTATEDIR_NONE;
        u8 pokeIdx = wk->procPokeIdx;

        #ifdef ROTATION_NEW_SYSTEM
        if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION )
        {
          dir = wk->rotWazaSelParam.actRotation.rotation.dir;
          pokeIdx = BTL_MAINUTIL_GetRotateInPosIdx( dir );
          procPoke = wk->rotWazaSelParam.poke[ pokeIdx ].bpp;
          BTL_N_Printf( DBGSTR_CLIENT_ROT_Determine, dir, pokeIdx, BPP_GetID(procPoke) );
        }
        #endif

        // ワザ説明ショートカットへ
        if( BTL_ACTION_IsWazaInfoMode(procAction) )
        {
          wk->wazaInfoPokeIdx = pokeIdx;
          wk->wazaInfoWazaIdx = BPP_WAZA_SearchIdx( procPoke, procAction->fight.waza );
          if( wk->wazaInfoWazaIdx != PTL_WAZA_MAX ){
            ClientSubProc_Set( wk, selact_WazaInfoView );
            return FALSE;
          }
        }

        if( is_unselectable_waza(wk, procPoke, procAction->fight.waza, &wk->strParam) )
        {
          selact_startMsg( wk, &wk->strParam );
          (*seq) = SEQ_WAIT_UNSEL_WAZA_MSG;
        }
        else
        {
          // アクションパラメータにローテ方向もセットする仮動作
          #ifdef ROTATION_NEW_SYSTEM
          BtlRule rule = BTL_MAIN_GetRule( wk->mainModule );
          if( rule == BTL_RULE_ROTATION )
          {
            if( (dir != BTL_ROTATEDIR_NONE) && (dir != BTL_ROTATEDIR_STAY) )
            {
              if( !BPP_IsDead(procPoke) )
              {
                WazaID waza = BTL_ACTION_GetWazaID( &(wk->rotWazaSelParam.actWaza) );
                BTL_ACTION_SetRotation( wk->procAction++, dir );
                BTL_ACTION_SetFightParam( wk->procAction, waza, BTL_POS_NULL );
                wk->actionAddCount++;
              }
            }
          }
          #endif
          (*seq) = SEQ_CHECK_WAZA_TARGET;
        }
      }
    }
    break;

  case SEQ_CHECK_WAZA_TARGET:
    // シングルなら対象選択なし
    {
      BtlRule rule = BTL_MAIN_GetRule( wk->mainModule );
      if( BTL_RULE_IsNeedSelectTarget(rule) ){
        (*seq) = SEQ_SELECT_TARGET_START;
      }else{
        ClientSubProc_Set( wk, selact_CheckFinish );
      }
    }
    break;

  case SEQ_SELECT_TARGET_START:
    BTL_N_Printf( DBGSTR_CLIENT_StartWazaTargetSel );
    BTLV_UI_SelectTarget_Start( wk->viewCore, wk->procPoke, wk->procAction );
    (*seq) = SEQ_SELECT_TARGET_WAIT;
    break;

  case SEQ_SELECT_TARGET_WAIT:
    if( CheckSelactForceQuit(wk, selact_ForceQuit) ){
      BTLV_UI_SelectTarget_ForceQuit( wk->viewCore );
      return FALSE;
    }

    {
      BtlvResult result = BTLV_UI_SelectTarget_Wait( wk->viewCore );
      if( result == BTLV_RESULT_DONE ){
        ClientSubProc_Set( wk, selact_CheckFinish );
      }else if( result == BTLV_RESULT_CANCEL ){
        (*seq) = SEQ_SELECT_WAZA_START;
      }
    }
    break;

  case SEQ_WAIT_UNSEL_WAZA_MSG:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      BTLV_UI_SelectWaza_Restart( wk->viewCore );
      (*seq) = SEQ_SELECT_WAZA_WAIT;
    }
    break;
  }

  return FALSE;
}
/**
 *  ローテーションワザ選択用パラメータ初期化
 */
static void setupRotationParams( BTL_CLIENT* wk, BTLV_ROTATION_WAZASEL_PARAM* param )
{
  const BTL_POKEPARAM* bpp;
  u32 i, j;

  for(i=0; i<BTL_ROTATE_NUM; ++i)
  {
    bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
    param->poke[ i ].bpp = bpp;

    for(j=0; j<PTL_WAZA_MAX; ++j){
      param->poke[ i ].fWazaUsable[ j ] = FALSE;
    }

    if( !BPP_IsDead(bpp) )
    {
      u32 wazaCnt = BPP_WAZA_GetCount( bpp );

      for(j=0; j<wazaCnt; ++j)
      {
        if( (BPP_WAZA_GetPP(bpp, j) == 0)
        ||  (is_unselectable_waza(wk, bpp, BPP_WAZA_GetID(bpp,j), NULL))
        ){
          param->poke[ i ].fWazaUsable[ j ] = FALSE;
        }
        else{
          param->poke[ i ].fWazaUsable[ j ] = TRUE;
        }
      }
    }
  }
}
//----------------------------------------------------------------------
/**
 *  ワザ説明画面（ショートカットモード）
 */
//----------------------------------------------------------------------
static BOOL selact_WazaInfoView( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    BTLV_StartWazaInfoView( wk->viewCore, wk->wazaInfoPokeIdx, wk->wazaInfoWazaIdx );
    (*seq)++;
    break;
  case 1:
    // 制限時間による強制終了（ポケモン選択画面の終了通知）
    if( CheckSelactForceQuit(wk, NULL) ){
      BTLV_ForceQuitPokeSelect( wk->viewCore );
      (*seq) = 2;
      return FALSE;
    }

    if( BTLV_WaitPokeSelect(wk->viewCore) )
    {
      ClientSubProc_Set( wk, selact_Fight );
    }
    break;

  case 2:
    if( BTLV_WaitPokeSelect(wk->viewCore) ){
      ClientSubProc_Set( wk, selact_ForceQuit );
    }
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------
/**
 *  「ポケモン」選択後のいれかえポケモン選択
 */
//----------------------------------------------------------------------
static BOOL selact_SelectChangePokemon( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    {
      u16 tokuseiID;
      u8  code, pokeID, fCantEsc = FALSE;
      code = isForbidPokeChange( wk, wk->procPoke, &pokeID, &tokuseiID );
      if( (code != BTL_CANTESC_NULL)
      ||  (BPP_GetID(wk->procPoke) == pokeID)
      ){
        fCantEsc = TRUE;
      }
      BTLV_StartPokeSelect( wk->viewCore, &wk->pokeSelParam, wk->procPokeIdx, fCantEsc, &wk->pokeSelResult );
      (*seq)++;
    }
    break;

  case 1:
    // 制限時間による強制終了（ポケモン選択画面の終了通知）
    if( CheckSelactForceQuit(wk, NULL) ){
      BTLV_ForceQuitPokeSelect( wk->viewCore );
      (*seq) = 2;
      return FALSE;
    }

    if( BTLV_WaitPokeSelect(wk->viewCore) )
    {
      if( !BTL_POKESELECT_IsCancel(&wk->pokeSelResult) )
      {
        u8 idx = BTL_POKESELECT_RESULT_GetLast( &wk->pokeSelResult );
        if( idx < BTL_PARTY_MEMBER_MAX )
        {
          BTL_N_Printf( DBGSTR_CLIENT_SelectChangePoke, idx);
          BTL_ACTION_SetChangeParam( wk->procAction, wk->procPokeIdx, idx );
          ClientSubProc_Set( wk, selact_CheckFinish );
          break;
        }
      }

      BTL_N_Printf( DBGSTR_CLIENT_SelectChangePokeCancel );
      ClientSubProc_Set( wk, selact_Root );
    }
    break;

  // 制限時間による強制終了（ポケモン選択画面の終了待ち後）
  case 2:
    if( BTLV_WaitPokeSelect(wk->viewCore) ){
      ClientSubProc_Set( wk, selact_ForceQuit );
    }
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------
/**
 *  「どうぐ」選択
 */
//----------------------------------------------------------------------
static BOOL selact_Item( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    {
      u8 cost_sum = shooterCost_GetSum( wk );
      u8 fFirstPokemon = (wk->procPokeIdx == wk->firstPokeIdx );
      BTLV_ITEMSELECT_Start( wk->viewCore, wk->bagMode, wk->shooterEnergy, cost_sum, fFirstPokemon );
      (*seq)++;
    }
    break;

  case 1:
    // 制限時間による強制終了通知
    if( CheckSelactForceQuit(wk, NULL) ){
      BTLV_ITEMSELECT_ForceQuit( wk->viewCore );
      (*seq) = 2;
      return FALSE;
    }

    if( BTLV_ITEMSELECT_Wait(wk->viewCore) )
    {
      u16 itemID, targetIdx;
      itemID = BTLV_ITEMSELECT_GetItemID( wk->viewCore );
      targetIdx = BTLV_ITEMSELECT_GetTargetIdx( wk->viewCore );
      if( (itemID != ITEM_DUMMY_DATA) && (targetIdx != BPL_SEL_EXIT) )
      {
        u8 cost = BTLV_ITEMSELECT_GetCost( wk->viewCore );
        u16 wazaIdx = BTLV_ITEMSELECT_GetWazaIdx( wk->viewCore );
        if( wk->shooterEnergy >= cost ){
          wk->shooterEnergy -= cost;
        }else{
          GF_ASSERT_MSG(0, "Energy=%d, item=%d, cost=%d\n", wk->shooterEnergy, itemID, cost );
          wk->shooterEnergy = 0;
          cost = wk->shooterEnergy;
        }
        shooterCost_Save( wk, wk->procPokeIdx, cost );
        BTL_ACTION_SetItemParam( wk->procAction, itemID, targetIdx, wazaIdx );
        BTLV_ITEMSELECT_ReflectUsedItem( wk->viewCore );

        // ボールを選択した場合、残りポケモンのアクション選択は全てスキップさせる
        if( BTL_CALC_ITEM_GetParam(itemID, ITEM_PRM_ITEM_TYPE) == ITEMTYPE_BALL )
        {
          wk->fBallSelected = TRUE;
        }
        selItemWork_Reserve( wk, wk->procPokeIdx, itemID );
        ClientSubProc_Set( wk, selact_CheckFinish );
      }else{
//      (*seq)=SEQ_SELECT_ACTION;
        ClientSubProc_Set( wk, selact_Root );
      }
    }
    break;

  // 制限時間による強制終了待ち後
  case 2:
    if( BTLV_ITEMSELECT_Wait(wk->viewCore) ){
      ClientSubProc_Set( wk, selact_ForceQuit );
    }
    break;

  }
  return FALSE;
}
//----------------------------------------------------------------------
/**
 *  「にげる」選択
 */
//----------------------------------------------------------------------
static BOOL selact_Escape( BTL_CLIENT* wk, int* seq )
{
  enum {
    SEQ_INIT = 0,
    SEQ_WAIT_MSG_RETURN,
    SEQ_WAIT_MSG_RETURN_END,

    SEQ_WAIT_MSG_CONFIRM,
    SEQ_WAIT_CONFIRM_YESNO,

    SEQ_RETURN_ESCAPE,
  };

  switch( *seq ){
  case SEQ_INIT:
  {
    // 戦闘モード等による禁止チェック
    BtlEscapeMode esc = BTL_MAIN_GetEscapeMode( wk->mainModule );
    switch( esc ){
    case BTL_ESCAPE_MODE_WILD:  // 素早さなど条件クリアすれば逃げられる（野生）
    default:
      {
        BtlCantEscapeCode  code;
        u16 tokuseiID;
        u8 pokeID;
        code = isForbidEscape( wk, &pokeID, &tokuseiID );

        // とくせい、ワザ効果等による禁止チェック -> 何もない
        if( code == BTL_CANTESC_NULL )
        {
          (*seq) = SEQ_RETURN_ESCAPE;
        }
        // とくせい、ワザ効果等による禁止チェック -> 引っかかって逃げられない
        else
        {
          if( tokuseiID != POKETOKUSEI_NULL ){
            BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_SET, BTL_STRID_SET_CantEscTok );
            BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
            BTLV_STRPARAM_AddArg( &wk->strParam, tokuseiID );
          }else{
            BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_EscapeProhibit );
          }
          selact_startMsg( wk, &wk->strParam );
          (*seq) = SEQ_WAIT_MSG_RETURN;
        }
      }
      break;

    case BTL_ESCAPE_MODE_NG:  // 逃げること自体が禁止されている（通常トレーナー戦）
      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_EscapeCant );
      selact_startMsg( wk, &wk->strParam );
      (*seq) = SEQ_WAIT_MSG_RETURN;
      break;

    case BTL_ESCAPE_MODE_CONFIRM: // 逃げると負けになるのを確認させ、確実に逃げられる（通信対戦＆サブウェイ）
      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_EscapeCheck );
      selact_startMsg( wk, &wk->strParam );
      (*seq) = SEQ_WAIT_MSG_CONFIRM;
    }
  }
  break;

  case SEQ_WAIT_MSG_RETURN:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
//      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, SelActStrID(wk) );
//      BTLV_STRPARAM_AddArg( &wk->strParam, BPP_GetID(wk->procPoke) );
//      BTLV_STRPARAM_SetWait( &wk->strParam, 0 );
      SetupSelectStartStr( wk, wk->procPoke, &wk->strParam );
      BTLV_PrintMsgAtOnce( wk->viewCore, &wk->strParam );
      (*seq) = SEQ_WAIT_MSG_RETURN_END;
    }
    break;

  case SEQ_WAIT_MSG_RETURN_END:
    if( BTLV_WaitMsg(wk->viewCore) ){
      ClientSubProc_Set( wk, selact_Root );
    }
    break;

  // 逃げると負けるけどいいですか？
  case SEQ_WAIT_MSG_CONFIRM:
    BTLV_WaitMsg( wk->viewCore );
    if( BTLV_IsJustDoneMsg(wk->viewCore) )
    {
      BTLV_STRPARAM_Setup( &wk->strParam,    BTL_STRTYPE_UI, BTLMSG_UI_SEL_YES );
      BTLV_STRPARAM_Setup( &wk->strParamSub, BTL_STRTYPE_UI, BTLMSG_UI_SEL_NO );
      BTLV_YESNO_Start( wk->viewCore, &wk->strParam, &wk->strParamSub, TRUE );
      (*seq) = SEQ_WAIT_CONFIRM_YESNO;
    }
    break;

  case SEQ_WAIT_CONFIRM_YESNO:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      BtlYesNo result;
      if( BTLV_YESNO_Wait(wk->viewCore, &result) )
      {
        if( result == BTL_YESNO_YES )
        {
          (*seq) = SEQ_RETURN_ESCAPE;
        }
        else
        {
          ClientSubProc_Set( wk, selact_Root );
        }
      }
      // コマンド選択時間切れ対処
      else if( CmdLimit_CheckOver(wk) )
      {
        ClientSubProc_Set( wk, selact_ForceQuit );
      }
    }
    break;

  case SEQ_RETURN_ESCAPE:
    BTL_ACTION_SetEscapeParam( wk->procAction );
    wk->returnDataPtr = &(wk->actionParam[0]);
    wk->returnDataSize = sizeof(BTL_ACTION_PARAM) * (wk->procPokeIdx + 1);
    ClientSubProc_Set( wk, selact_Finish );
    break;

  }
  return FALSE;
}
//----------------------------------------------------------------------
/**
 *  全アクション選択終了チェック
 */
//----------------------------------------------------------------------
static BOOL selact_CheckFinish( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    BTLV_UI_Restart( wk->viewCore );
    (*seq)++;
    break;
  case 1:
    if( BTLV_UI_WaitRestart(wk->viewCore) )
    {
      wk->procPokeIdx++;

      if( wk->procPokeIdx < wk->numCoverPos )
      {
        if( wk->fBallSelected == FALSE ){
          BTL_N_Printf( DBGSTR_CLIENT_SelectActionBacktoRoot, wk->procPokeIdx );
          ClientSubProc_Set( wk, selact_Root );
          break;
        }
        else
        {
          while( wk->procPokeIdx < wk->numCoverPos )
          {
            BTL_ACTION_SetNULL( &(wk->actionParam[wk->procPokeIdx++]) );
          }
        }
      }

      if( wk->procPokeIdx >= wk->numCoverPos )
      {
        u8 actionCnt = wk->numCoverPos + wk->actionAddCount;
        BTL_N_Printf( DBGSTR_CLIENT_SelectActionDone, wk->numCoverPos);

        wk->actionAddCount = 0;
        wk->returnDataPtr = &(wk->actionParam[0]);
        wk->returnDataSize = sizeof(wk->actionParam[0]) * actionCnt;
        ClientSubProc_Set( wk, selact_Finish );
      }
    }
    break;
  }

  return FALSE;
}
//----------------------------------------------------------------------
/**
 *  全アクション選択終了->サーバ返信へ
 */
//----------------------------------------------------------------------
static BOOL selact_Finish( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    selItemWork_Quit( wk );

    if( BTL_MAIN_GetCommMode(wk->mainModule) != BTL_COMM_NONE )
    {
      wk->commWaitInfoOn = TRUE;
      BTLV_StartCommWait( wk->viewCore );
      (*seq)++;
    }
    else
    {
      (*seq)+=2;
    }
    break;

  case 1:
    if( BTLV_WaitCommWait(wk->viewCore) ){
      (*seq)++;
    }
    break;

  case 2:
    if( !BTLV_EFFECT_CheckExecute() ){
      BTL_N_Printf( DBGSTR_CLIENT_ReturnSeqDone );

      CmdLimit_End( wk );
      (*seq)++;
      return TRUE;
    }
    break;
  default:
    return TRUE;
  }
  return FALSE;
}

/*============================================================================================*/
/* 選択済みアイテム保持用ワーク                                                               */
/*============================================================================================*/
// 初期化
static void selItemWork_Init( BTL_CLIENT* wk )
{
  u32 i;
  for(i=0; i<NELEMS(wk->selItemWork); ++i){
    wk->selItemWork[ i ] = ITEM_DUMMY_DATA;
  }
}
// アイテム１つ使う予約
static void selItemWork_Reserve( BTL_CLIENT* wk, u8 pokeIdx, u16 itemID )
{
  if( wk->bagMode == BBAG_MODE_NORMAL )
  {
    if( wk->myID == BTL_MAIN_GetPlayerClientID(wk->mainModule) )
    {
      TAYA_Printf("ItemID=%dを使う予定\n", itemID);
      wk->selItemWork[ pokeIdx ] = itemID;
      BTL_MAIN_DecrementPlayerItem( wk->mainModule, wk->myID, itemID );
    }
  }
}
// 使う予約していたアイテム情報をキャンセル
static void selItemWork_Restore( BTL_CLIENT* wk, u8 pokeIdx )
{
  if( wk->bagMode == BBAG_MODE_NORMAL )
  {
    if( wk->myID == BTL_MAIN_GetPlayerClientID(wk->mainModule) )
    {
      u16 itemID = wk->selItemWork[ pokeIdx ];
      if( itemID != ITEM_DUMMY_DATA )
      {
        TAYA_Printf("ItemID=%dを戻す\n", itemID);
        wk->selItemWork[ pokeIdx ] = ITEM_DUMMY_DATA;
        BTL_MAIN_AddItem( wk->mainModule, wk->myID, itemID );
      }
    }
  }
}
// 予約情報を全クリア
static void selItemWork_Quit( BTL_CLIENT* wk )
{
  if( wk->bagMode == BBAG_MODE_NORMAL )
  {
    u32 i;
    for(i=0; i<NELEMS(wk->selItemWork); ++i)
    {
      selItemWork_Restore( wk, i );
    }
  }
}

/*============================================================================================*/
/* シューターコスト計算用ワーク                                                               */
/*============================================================================================*/
// 初期化
static void shooterCost_Init( BTL_CLIENT* wk )
{
  GFL_STD_MemClear( wk->shooterCost, sizeof(wk->shooterCost) );
}
// 使用アイテムコストを記録
static void shooterCost_Save( BTL_CLIENT* wk, u8 procPokeIdx, u8 cost )
{
  wk->shooterCost[ procPokeIdx ] = cost;
}
static u8 shooterCost_Get( BTL_CLIENT* wk, u8 procPokeIdx )
{
  return wk->shooterCost[ procPokeIdx ];
}
// 使用予定の合計値を取得
static u8 shooterCost_GetSum( BTL_CLIENT* wk )
{
  u32 i, sum = 0;
  for(i=0; i<wk->procPokeIdx; ++i){
    sum += wk->shooterCost[i];
  }
  return sum;
}

//----------------------------------------------------------------------------------
/**
 * アクション選択不可状態のポケモンかどうか判定
 * 不可状態であれば、アクション内容を自動生成する
 *
 * @param   wk
 * @param   bpp       判定対象ポケモンパラメータ
 * @param   action    [out] アクション内容生成先
 *
 * @retval  BOOL      不可状態ならTRUE
 */
//----------------------------------------------------------------------------------
static BOOL checkActionForceSet( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* action )
{
  // 死んでたらNULLデータを返す
  if( BPP_IsDead(bpp) )
  {
    if( action ){
      // 死んでる状態のアクション内容セット
      BTL_ACTION_SetNULL( action );
    }
    return TRUE;
  }
  // アクション選択できない（攻撃の反動など）場合はスキップ
  if( BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_CANT_ACTION) )
  {
    if( action ){
      BTL_ACTION_SetSkip( action );
    }
    return TRUE;
  }
  // ワザロック状態（記録ワザをそのまま使う）
  if( BPP_CheckSick(bpp, WAZASICK_WAZALOCK) )
  {
    BPP_SICK_CONT  cont = BPP_GetSickCont( bpp, WAZASICK_WAZALOCK );
    WazaID waza = BPP_SICKCONT_GetParam( cont );
    BtlPokePos pos = BPP_GetPrevTargetPos( bpp );

    GF_ASSERT(waza!=WAZANO_NULL);
    GF_ASSERT(waza<WAZANO_MAX);

    BTL_N_PrintfEx( PRINT_FLG, DBGSTR_CLIENT_WazaLockInfo, wk->myID, waza, pos);

    if( action ){
      BTL_ACTION_SetFightParam( action, waza, pos );
    }
    return TRUE;
  }

  // 溜めロック状態（記録ワザをそのまま使う）
  if( BPP_CheckSick(bpp, WAZASICK_TAMELOCK) )
  {
    WazaID waza = BPP_GetPrevWazaID( bpp );
    BtlPokePos pos = BPP_GetPrevTargetPos( bpp );
    u8 waza_idx = BPP_WAZA_SearchIdx( bpp, waza );

    // PP ゼロでも実行
    if( action ){
      BTL_ACTION_SetFightParam( action, waza, pos );
    }
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * ワザ選択不可状態のポケモンかどうか判定
 * ワザ選択不可状態であれば、アクション内容を自動生成する
 *
 * @param   wk
 * @param   bpp         判定対象ポケモンパラメータ
 * @param   fUIMode     UI処理モード（出せるワザが１つでもあればアクション内容生成しない）
 * @param   action      [out] アクション内容生成先
 *
 * @retval  BOOL      ワザ選択不可状態ならTRUE
 */
//----------------------------------------------------------------------------------
static BOOL checkWazaForceSet( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* action )
{
  u32 wazaCount = BPP_WAZA_GetCount( bpp );
  u32 selectableCount, i;

  // 使えるワザのPPが全て0なら「わるあがき」
  selectableCount = 0;
  for(i=0; i<wazaCount; ++i)
  {
    if( is_unselectable_waza( wk, bpp, BPP_WAZA_GetID(bpp,i), NULL ) ){
      continue;
    }
    if( BPP_WAZA_GetPP(bpp, i) == 0 ){
      continue;
    }
    ++selectableCount;
  }

  if( selectableCount == 0 )
  {
    setWaruagakiAction( action, wk, bpp );
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 「わるあがき」用アクションパラメータセット
 *
 * @param   dst
 * @param   wk
 * @param   bpp
 */
//----------------------------------------------------------------------------------
static void setWaruagakiAction( BTL_ACTION_PARAM* dst, BTL_CLIENT* wk, const BTL_POKEPARAM* bpp )
{
  BtlPokePos targetPos = BTL_CALC_DecideWazaTargetAuto( wk->mainModule, wk->pokeCon, bpp, WAZANO_WARUAGAKI );
  BTL_ACTION_SetFightParam( dst, WAZANO_WARUAGAKI, targetPos );
}

//----------------------------------------------------------------------------------
/**
 * 使用できないワザが選ばれたかどうか判定
 *
 * @param   wk
 * @param   bpp       対象ポケモン
 * @param   waza      ワザ
 * @param   strParam  [out]警告メッセージパラメータ出力先
 *
 * @retval  BOOL    使用できないワザの場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL is_unselectable_waza( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, WazaID waza, BTLV_STRPARAM* strParam )
{
  // こだわりアイテム効果（最初に使ったワザしか選べない／ただしマジックルーム非発動時のみ）
  if( IsItemEffective(wk, bpp) )
  {
    if( BPP_CheckSick(bpp, WAZASICK_KODAWARI) )
    {
      BPP_SICK_CONT  cont = BPP_GetSickCont( bpp, WAZASICK_KODAWARI );
      WazaID  kodawariWaza = BPP_SICKCONT_GetParam( cont );

      TAYA_Printf("こだわり状態です。waza=%d\n", kodawariWaza);

      // こだわり対象のワザを覚えていて、それ以外のワザを使おうとしたらダメ
      if( (BPP_WAZA_IsUsable(bpp, kodawariWaza))
      &&  (kodawariWaza != waza )
      ){
        GF_ASSERT(kodawariWaza != WAZANO_NULL);
        GF_ASSERT(kodawariWaza < WAZANO_MAX);
        if( strParam != NULL )
        {
          BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_STD, BTL_STRID_STD_KodawariLock );
          BTLV_STRPARAM_AddArg( strParam, BPP_GetItem(bpp) );
          BTLV_STRPARAM_AddArg( strParam, kodawariWaza );
        }
        return TRUE;
      }
    }
  }

  // アンコール効果（前回と同じワザしか出せない）
  if( BPP_CheckSick(bpp, WAZASICK_ENCORE) )
  {
    WazaID prevWaza = BPP_SICKCONT_GetParam( BPP_GetSickCont(bpp, WAZASICK_ENCORE) );
    if( waza != prevWaza ){
      if( strParam != NULL )
      {
        BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_STD, BTL_STRID_STD_WazaLock );
        BTLV_STRPARAM_AddArg( strParam, BPP_GetID(bpp) );
        BTLV_STRPARAM_AddArg( strParam, prevWaza );
      }
      return TRUE;
    }
  }

  // ちょうはつ状態（ダメージワザしか選べない）
  if( BPP_CheckSick(bpp, WAZASICK_TYOUHATSU) )
  {
    if( !WAZADATA_IsDamage(waza) )
    {
      if( strParam != NULL )
      {
        BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_SET, BTL_STRID_SET_ChouhatuWarn );
        BTLV_STRPARAM_AddArg( strParam, BPP_GetID(bpp) );
        BTLV_STRPARAM_AddArg( strParam, waza );
      }
      return TRUE;
    }
  }

  // いちゃもん状態（２ターン続けて同じワザを選べない）
  if( BPP_CheckSick(bpp, WAZASICK_ICHAMON) )
  {
    if( BPP_GetPrevOrgWazaID(bpp) == waza )
    {
      if( strParam != NULL )
      {
        BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_SET, BTL_STRID_SET_IchamonWarn );
        BTLV_STRPARAM_AddArg( strParam, BPP_GetID(bpp) );
        BTLV_STRPARAM_AddArg( strParam, waza );
      }
      return TRUE;
    }
  }

  // かなしばり状態（かなしばり直前に出していたワザを選べない）
  if( BPP_CheckSick(bpp, WAZASICK_KANASIBARI) )
  {
    u16 kanasibariWaza = BPP_GetSickParam( bpp, WAZASICK_KANASIBARI );
    BTL_Printf("かなしばりチェック waza=%d\n", kanasibariWaza);
    if( (waza == kanasibariWaza) && (waza != WAZANO_WARUAGAKI) )
    {
      if( strParam != NULL )
      {
        BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_SET, BTL_STRID_SET_KanasibariWarn );
        BTLV_STRPARAM_AddArg( strParam, BPP_GetID(bpp) );
        BTLV_STRPARAM_AddArg( strParam, waza );
      }
      return TRUE;
    }
  }

  // かいふくふうじ状態（回復ワザを選べない）
  if( BPP_CheckSick(bpp, WAZASICK_KAIHUKUHUUJI) )
  {
    if( WAZADATA_GetFlag(waza, WAZAFLAG_KaifukuHuuji) )
    {
      if( strParam )
      {
        BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_SET, BTL_STRID_SET_KaifukuFujiWarn );
        BTLV_STRPARAM_AddArg( strParam, BPP_GetID(bpp) );
        BTLV_STRPARAM_AddArg( strParam, waza );
      }
      return TRUE;
    }
  }

// ふういんチェック（ふういんをかけたポケが持ってるワザを出せない）
  if( BTL_FIELD_CheckEffect(BTL_FLDEFF_FUIN) )
  {
    TAYA_Printf("ふういんが効いてるからチェックします\n");
    if( BTL_FIELD_CheckFuin(wk->pokeCon, bpp, waza) )
    {
      if( strParam != NULL )
      {
        BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_SET, BTL_STRID_SET_FuuinWarn );
        BTLV_STRPARAM_AddArg( strParam, BPP_GetID(bpp) );
        BTLV_STRPARAM_AddArg( strParam, waza );
      }
      return TRUE;
    }
  }

  // じゅうりょくチェック
  if( BTL_FIELD_CheckEffect(BTL_FLDEFF_JURYOKU) )
  {
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Flying) )
    {
      if( strParam != NULL )
      {
        BTLV_STRPARAM_Setup( strParam, BTL_STRTYPE_SET, BTL_STRID_SET_JyuryokuWazaFail );
        BTLV_STRPARAM_AddArg( strParam, BPP_GetID(bpp) );
        BTLV_STRPARAM_AddArg( strParam, waza );
      }
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 選択できるワザのIndexを全て列挙
 *
 * @param   wk
 * @param   bpp
 * @param   dst   [out] u8 x 4 の配列。選択できるワザなら対応Indexが1, それ以外は0になる
 *
 * @retval  u8    選択できる最初のワザIndex。選択できるワザがない場合、PTL_WAZA_MAX
 */
//----------------------------------------------------------------------------------
static u8 StoreSelectableWazaFlag( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, u8* dst )
{
  u8 wazaCount = BPP_WAZA_GetCount( bpp );
  u8 firstIdx = PTL_WAZA_MAX;
  u8 i;

  for(i=0; i<wazaCount; ++i )
  {
    if( BPP_WAZA_GetPP(bpp, i) )
    {
      WazaID  waza = BPP_WAZA_GetID( bpp, i );
      dst[i] = !is_unselectable_waza( wk, bpp, waza, NULL );
      if( (firstIdx==PTL_WAZA_MAX) && (dst[i]) ){
        firstIdx = i;
      }
    }
    else{
      dst[i] = 0;
    }
  }
  for( ; i<PTL_WAZA_MAX; ++i ){
    dst[i] = 0;
  }
  return firstIdx;
}

/**
 *  アイテム装備効果有効かチェック
 */
static BOOL IsItemEffective( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp )
{
  if( BTL_FIELD_CheckEffect(BTL_FLDEFF_MAGICROOM) ){
//    TAYA_Printf("マジックルーム効いてるから道具効果なし\n");
    return FALSE;
  }
  if( BPP_CheckSick(bpp, WAZASICK_SASIOSAE) ){
//    TAYA_Printf("さしおさえだから道具効果なし\n");
    return FALSE;
  }
  if( BPP_GetValue(bpp, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_BUKIYOU ){
//    TAYA_Printf("ぶきようだから道具効果なし\n");
    return FALSE;
  }
  return TRUE;

}

//----------------------------------------------------------------------------------
/**
 * 交換禁止チェック
 *
 * @param   wk
 * @param   procPoke
 * @param   pokeID
 * @param   tokuseiID   [out] 交換できない原因が相手とくせいの場合、そのとくせいID（それ以外POKETOKUSEI_NULL）
 *
 * @retval  BtlCantEscapeCode
 */
//----------------------------------------------------------------------------------
static BtlCantEscapeCode isForbidPokeChange( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, u8* pokeID, u16* tokuseiID )
{
  *pokeID = BTL_POKEID_NULL;
  *tokuseiID = POKETOKUSEI_NULL;

  // 「きれいなぬけがら」を持っていたら確実にOK
  if( IsItemEffective(wk, procPoke) )
  {
    if( BPP_GetItem(procPoke) == ITEM_KIREINANUKEGARA ){
      return BTL_CANTESC_NULL;
    }
  }

  // 逃げ・交換禁止チェック共通部分
  {
    BtlCantEscapeCode code = checkForbidChangeEscapeCommon( wk, procPoke, pokeID, tokuseiID );
    if( code != BTL_CANTESC_NULL ){
      return code;
    }
  }

  return BTL_CANTESC_NULL;
}
//----------------------------------------------------------------------------------
/**
 * 逃げ禁止チェック
 *
 * @param   wk
 * @param   procPoke
 * @param   pokeID      [out] 逃げ・交換できない原因となるポケID（敵・味方ともあり得る）
 * @param   tokuseiID   [out] 逃げ・交換できない原因がとくせいの場合、そのとくせいID（それ以外POKETOKUSEI_NULL）
 *
 * @retval  BtlCantEscapeCode
 */
//----------------------------------------------------------------------------------
static BtlCantEscapeCode isForbidEscape( BTL_CLIENT* wk, u8* pokeID, u16* tokuseiID )
{
  const BTL_POKEPARAM* procPoke;
  u32 i;

  #ifdef PM_DEBUG
  if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_L|PAD_BUTTON_R) ){
    return BTL_CANTESC_NULL;
  }
  #endif

  #ifdef DEBUG_ONLY_FOR_hudson
  if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA ) )
  {
    return BTL_CANTESC_NULL;
  }
  #endif

  for(i=0; i<wk->numCoverPos; ++i)
  {
    procPoke = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
    if( BPP_IsDead(procPoke) ){
      continue;
    }

    // 「けむりだま」を持っていたら確実にOK
    if( IsItemEffective(wk, procPoke) )
    {
      if( BPP_GetItem(procPoke) == ITEM_KEMURIDAMA ){
        return BTL_CANTESC_NULL;
      }
    }
    // とくせい「にげあし」で確実にOK
    if( BPP_GetValue(procPoke, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_NIGEASI ){
      return BTL_CANTESC_NULL;
    }
  }
  for(i=0; i<wk->numCoverPos; ++i)
  {
    // 逃げ・交換禁止チェック共通部分
    {
      BtlCantEscapeCode code = checkForbidChangeEscapeCommon( wk, procPoke, pokeID, tokuseiID );
      if( code != BTL_CANTESC_NULL ){
        return code;
      }
    }
  }

  return BTL_CANTESC_NULL;
}
/**
 * 逃げ・交換禁止チェック共通部分
 */
static BtlCantEscapeCode checkForbidChangeEscapeCommon( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, u8* pokeID, u16* tokuseiID )
{
  BtlExPos    exPos;
  u16 checkTokusei;
  BtlPokePos  myPos;
  u8 procPokeID;
  u8 checkPokeCnt, checkPokeID, i;
  const BTL_POKEPARAM* bpp;
  u8 pokeIDAry[ BTL_POSIDX_MAX ];

  procPokeID = BPP_GetID( procPoke );
  myPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, procPokeID );
  exPos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, myPos );

  checkPokeCnt = BTL_MAIN_ExpandExistPokeID( wk->mainModule, wk->pokeCon, exPos, pokeIDAry );
  for(i=0; i<checkPokeCnt; ++i)
  {
    // 相手側とくせい「かげふみ」チェック
    bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeIDAry[i] );
    checkTokusei = BPP_GetValue( bpp, BPP_TOKUSEI_EFFECTIVE );
    checkPokeID = BPP_GetID( bpp );
    if( checkTokusei == POKETOKUSEI_KAGEFUMI )
    {
      BTL_N_Printf( DBGSTR_CLIENT_ForbidEscape_Kagefumi_Chk, checkPokeID );
      if( checkForbitEscapeEffective_Kagefumi(wk, procPoke) ){
        BTL_N_Printf( DBGSTR_CLIENT_ForbidEscape_Kagefumi_Enable );
        *pokeID = BPP_GetID( bpp );
        *tokuseiID = checkTokusei;
        return BTL_CANTESC_KAGEFUMI;
      }
    }
    // 相手側とくせい「ありじごく」チェック
    if( checkTokusei == POKETOKUSEI_ARIJIGOKU )
    {
      BTL_N_Printf( DBGSTR_CLIENT_ForbidEscape_Arijigoku_Chk, checkPokeID );
      if( checkForbitEscapeEffective_Arijigoku(wk, procPoke) ){
        BTL_N_Printf( DBGSTR_CLIENT_ForbidEscape_Arijigoku_Enable );
        *pokeID = BPP_GetID( bpp );
        *tokuseiID = checkTokusei;
        return BTL_CANTESC_KAGEFUMI;
      }
    }
    // 相手側とくせい「じりょく」チェック
    if( checkTokusei == POKETOKUSEI_JIRYOKU )
    {
      BTL_N_Printf( DBGSTR_CLIENT_ForbidEscape_Jiryoku_Chk, checkPokeID );
      if( checkForbitEscapeEffective_Jiryoku(wk, procPoke) ){
        BTL_N_Printf( DBGSTR_CLIENT_ForbidEscape_Jiryoku_Enable );
        *pokeID = BPP_GetID( bpp );
        *tokuseiID = checkTokusei;
        return BTL_CANTESC_KAGEFUMI;
      }
    }
  }

  // こちらのポケモン状態異常による逃げ・交換禁止チェック
  if( BPP_CheckSick( procPoke, WAZASICK_TOOSENBOU )
  ||  BPP_CheckSick( procPoke, WAZASICK_BIND )
  ||  BPP_CheckSick( procPoke, WAZASICK_NEWOHARU )
  ){
     *pokeID = BPP_GetID( procPoke );
     return BTL_CANTESC_TOOSENBOU;
  }

  return BTL_CANTESC_NULL;
}

/**
 *  逃げ・交換禁止チェック：自分に「かげふみ」が効くか
 */
static BOOL checkForbitEscapeEffective_Kagefumi( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke )
{
  if( BPP_GetValue(procPoke, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_KAGEFUMI ){
    return FALSE;
  }
  return TRUE;
}
/**
 *  逃げ・交換禁止チェック：自分に「ありじごく」が効くか
 */
static BOOL checkForbitEscapeEffective_Arijigoku( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke )
{
  BOOL fItemEffective = IsItemEffective( wk, procPoke );

  if( BTL_FIELD_CheckEffect(BTL_FLDEFF_JURYOKU) ){
    return TRUE;
  }
  if( BPP_CheckSick(procPoke, WAZASICK_FLYING_CANCEL) ){
    return TRUE;
  }
  if( BPP_CheckSick(procPoke, WAZASICK_NEWOHARU) ){
    return TRUE;
  }
  if( fItemEffective && (BPP_GetItem(procPoke) == ITEM_KUROITEKKYUU) ){
    return TRUE;
  }

  if( BPP_GetValue(procPoke, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_FUYUU ){
    return FALSE;
  }
  if( BPP_IsMatchType(procPoke, POKETYPE_HIKOU) ){
    return FALSE;
  }
  if( BPP_CheckSick(procPoke, WAZASICK_FLYING) ){
    return FALSE;
  }
  if( BPP_CheckSick(procPoke, WAZASICK_TELEKINESIS) ){
    return FALSE;
  }
  if( fItemEffective && (BPP_GetItem(procPoke) == ITEM_HUUSEN) ){
    return FALSE;
  }

  return TRUE;
}

/**
 *  逃げ・交換禁止チェック：自分に「じりょく」が効くか
 */
static BOOL checkForbitEscapeEffective_Jiryoku( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke )
{
  if( BPP_IsMatchType(procPoke, POKETYPE_HAGANE) ){
    return TRUE;
  }
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * AI 入れ替え判定用ワーク初期化
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static void ChangeAI_InitWork( BTL_CLIENT* wk )
{
  u32 i;
  for(i=0; i<NELEMS(wk->AIChangeIndex); ++i){
    wk->AIChangeIndex[ i ] = -1;
  }
}
static BOOL ChangeAI_CheckReserve( BTL_CLIENT* wk, u8 index )
{
  u32 i;

  for(i=0; i<NELEMS(wk->AIChangeIndex); ++i)
  {
    if( wk->AIChangeIndex[i] == index ){
      return TRUE;
    }
  }

  return FALSE;
}
static void ChangeAI_SetReserve( BTL_CLIENT* wk, u8 outPokeIdx, u8 inPokeIdx )
{
  if( outPokeIdx < NELEMS(wk->AIChangeIndex) )
  {
    wk->AIChangeIndex[ outPokeIdx ] = inPokeIdx;
  }
}

//----------------------------------------------------------------------------------
/**
 * AI 入れ替え判定
 *
 * @param   wk
 * @param   procPoke        行動チェック中ポケモンパラメータ
 * @param   procPokeIndex   行動チェック中ポケモンのパーティ内Index
 * @param   targetIndex     [out]入れ替える場合、対象ポケモンのパーティ内Index
 *
 * @retval  BOOL    入れ替える場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL ChangeAI_Root( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, u8 procPokeIndex, u8* targetIndex )
{
  BOOL fChange = FALSE;
  u8   changeIndex = BTL_PARTY_MEMBER_MAX;
  BTL_POKEPARAM* targetPoke;

  // 逃げ・交換禁止状態のチェック
  {
    u8  prohibit_pokeID;
    u16 prohibit_tokID;

    if( isForbidPokeChange(wk, procPoke, &prohibit_pokeID, &prohibit_tokID ) != BTL_CANTESC_NULL ){
      return FALSE;
    }
  }

  // もう交換できるメンバーが居ない
  if( BTL_PARTY_GetAliveMemberCount(wk->myParty) <= wk->numCoverPos ){
    return FALSE;
  }

  do {

    if( ChangeAI_CheckHorobi(wk, procPoke) ){  // ほろびのうたチェック
      BTL_N_Printf( DBGSTR_CLIENT_CHGAI_HOROBI );
      fChange = TRUE;
      break;
    }

    {
      BtlPokePos  basePos = BTL_MAIN_PokeIDtoPokePos(wk->mainModule, wk->pokeCon, BPP_GetID(procPoke) );
      targetPoke = ChangeAI_DecideTarget( wk, basePos );
      if( targetPoke == NULL ){
        break;
      }
    }

    if( ChangeAI_CheckFusigiNaMamori(wk, procPoke, targetPoke) ){  // ふしぎなまもりチェック
      BTL_N_Printf( DBGSTR_CLIENT_CHGAI_FusigiNaMamori );
      fChange = TRUE;
      break;
    }

    if( ChangeAI_CheckNoEffectWaza(wk, procPoke, targetPoke) ){  // 相性無効ワザチェック
      BTL_N_Printf( DBGSTR_CLIENT_CHGAI_NoEffWaza );
      fChange = TRUE;
      break;
    }

    if( ChangeAI_CheckKodawari(wk, procPoke, targetPoke) ){  // こだわりワザチェック
      BTL_N_Printf( DBGSTR_CLIENT_CHGAI_Kodawari );
      fChange = TRUE;
      break;
    }

    if( ChangeAI_CheckUkeTokusei(wk, procPoke, targetPoke, &changeIndex) ){ // 受けとくせいチェック
      BTL_N_Printf( DBGSTR_CLIENT_CHGAI_UkeTok );
      fChange = TRUE;
      break;
    }

    if( AI_ChangeProcSub_SizenKaifuku(wk, procPoke, &changeIndex) ){ // しぜんかいふくチェック
      BTL_N_Printf( DBGSTR_CLIENT_CHGAI_SizenKaifuku );
      fChange = TRUE;
      break;
    }

    if( AI_ChangeProcSub_WazaAff(wk, procPoke, targetPoke, &changeIndex) ){ // ワザ相性チェック
      BTL_N_Printf( DBGSTR_CLIENT_CHGAI_WazaEff );
      fChange = TRUE;
      break;
    }

    return FALSE;

  }while(0);

  if( !fChange ){ return FALSE; }

  if( changeIndex == BTL_PARTY_MEMBER_MAX )
  {
    u8 puttableList[ BTL_PARTY_MEMBER_MAX ];
    u32 cnt, i;

    cnt = calcPuttablePokemons( wk, puttableList );

    sortPuttablePokemonList( wk, puttableList, cnt, targetPoke );
    for(i=0; i<cnt; ++i)
    {
      if( !ChangeAI_CheckReserve(wk, puttableList[i]) )
      {
        changeIndex = puttableList[ i ];
        break;
      }
    }
  }

  if( changeIndex == BTL_PARTY_MEMBER_MAX ){
    return FALSE;
  }

  ChangeAI_SetReserve( wk, procPokeIndex, changeIndex );
  *targetIndex = changeIndex;
  return TRUE;
}

/**
 *  交換チェック時に考慮する「相手」ポケモンの決定
 */
static BTL_POKEPARAM* ChangeAI_DecideTarget( BTL_CLIENT* wk, BtlPokePos basePos )
{
  BtlExPos    exPos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, basePos );
  u8 aryPokeID[ BTL_POSIDX_MAX ];
  u8 cnt, i;

  cnt = BTL_MAIN_ExpandExistPokeID( wk->mainModule, wk->pokeCon, exPos, aryPokeID );
  if( cnt )
  {
    u8 idx = GFL_STD_Rand( &wk->AIRandContext, cnt );
    return BTL_POKECON_GetPokeParam( wk->pokeCon, aryPokeID[idx] );
  }

  return NULL;
}
/**
 *  交換チェック：ほろびのうた
 */
static BOOL ChangeAI_CheckHorobi( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke )
{
  if( BPP_CheckSick(procPoke, WAZASICK_HOROBINOUTA) )
  {
    BPP_SICK_CONT  cont = BPP_GetSickCont( procPoke, WAZASICK_HOROBINOUTA );
    u8 turnMax = BPP_SICCONT_GetTurnMax( cont );
    u8 turnNow = BPP_GetSickTurnCount( procPoke, WAZASICK_HOROBINOUTA );
    if( (turnNow+1) == turnMax ){
      return TRUE;
    }
  }
  return FALSE;
}
/**
 *  交換チェック：ふしぎなまもり
 */
static BOOL ChangeAI_CheckFusigiNaMamori( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, const BTL_POKEPARAM* targetPoke )
{
  if( BTL_MAIN_GetRule(wk->mainModule) != BTL_RULE_SINGLE )
  {
    return FALSE;
  }

  if( BPP_GetValue(targetPoke, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_FUSIGINAMAMORI )
  {
    // 自分は効果バツグンを持ってない＆控えは持っている
    if( !AI_ChangeProcSub_CheckWazaAff(wk, procPoke, targetPoke, BTL_TYPEAFF_200) )
    {
      if( AI_ChangeProcSub_HikaeWazaAff(wk, targetPoke, BTL_TYPEAFF_200) )
      {
        if( GFL_STD_Rand(&wk->AIRandContext, 3) < 2 ){
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}
/**
 *  交換チェック：効果なしワザしか持ってない
 */
static BOOL ChangeAI_CheckNoEffectWaza( BTL_CLIENT* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender )
{
  u32 i;
  WazaID waza;
  PokeType wazaType;
  PokeTypePair targetPokeType = BPP_GetPokeType( defender );
  u32 waza_cnt = BPP_WAZA_GetCount( attacker );

  u32 dmgWazaCnt = 0;
  u32 noEffCnt = 0;

  for(i=0; i<waza_cnt; ++i)
  {
    waza = BPP_WAZA_GetID( attacker, i );
    if( WAZADATA_IsDamage(waza) )
    {
      // 効果のあるダメージワザを持っていたらFALSE
      wazaType = WAZADATA_GetType( waza );
      if( BTL_CALC_TypeAffPair(wazaType, targetPokeType) != BTL_TYPEAFF_0 ){
        return FALSE;
      }
      ++dmgWazaCnt;
    }
  }

  // 効果なしダメージワザのみ、２つ以上所有していたら
  if( dmgWazaCnt >= 2 )
  {
    if( AI_ChangeProcSub_HikaeWazaAff(wk, defender, BTL_TYPEAFF_200) )
    {
      if( GFL_STD_Rand(&wk->AIRandContext, 3) < 2 ){
        return TRUE;
      }
      return FALSE;
    }
    if( AI_ChangeProcSub_HikaeWazaAff(wk, defender, BTL_TYPEAFF_100) )
    {
      if( GFL_STD_Rand(&wk->AIRandContext, 2) < 1 ){
        return TRUE;
      }
      return FALSE;
    }
  }
  return FALSE;
}
/**
 *  交換チェック：こだわり効果発動して効果なし
 */
static BOOL ChangeAI_CheckKodawari( BTL_CLIENT* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender )
{
  if( BPP_CheckSick(attacker, WAZASICK_KODAWARI) )
  {
    BPP_SICK_CONT cont = BPP_GetSickCont( attacker, WAZASICK_KODAWARI );
    WazaID  kodawariWaza = BPP_SICKCONT_GetParam( cont );
    if( kodawariWaza != WAZANO_NULL )
    {
      BOOL fDamageWaza = WAZADATA_IsDamage( kodawariWaza );
      PokeType  wazaType = WAZADATA_GetType( kodawariWaza );
      PokeTypePair targetType = BPP_GetPokeType( defender );

      if( (BTL_CALC_TypeAffPair(wazaType, targetType)  == BTL_TYPEAFF_0)
      &&  (fDamageWaza)
      ){
        if( AI_ChangeProcSub_HikaeWazaAff(wk, defender, BTL_TYPEAFF_200) )
        {
          return ( GFL_STD_Rand(&wk->AIRandContext, 3) < 2 );
        }
        if( AI_ChangeProcSub_HikaeWazaAff(wk, defender, BTL_TYPEAFF_100) )
        {
          return ( GFL_STD_Rand(&wk->AIRandContext, 2) < 1 );
        }
      }
      if( !fDamageWaza )
      {
        return ( GFL_STD_Rand(&wk->AIRandContext, 2) < 1 );
      }
    }
  }
  return FALSE;
}
/**
 *  交換チェック：前ターンに受けたワザの受け特性を持っている控えポケチェック
 */
static BOOL ChangeAI_CheckUkeTokusei( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, const BTL_POKEPARAM* targetPoke, u8* changeIndex )
{
  enum {
    CHECK_TOK_MAX = 4,  // チェックするとくせい最大数
  };

  static const struct {
    u8   wazaType;
    u16  tokusei[ CHECK_TOK_MAX ];
  }CheckTable[] = {
    { POKETYPE_MIZU,  { POKETOKUSEI_TYOSUI,   POKETOKUSEI_YOBIMIZU,   POKETOKUSEI_KANSOUHADA, POKETOKUSEI_NULL } },
    { POKETYPE_DENKI, { POKETOKUSEI_TIKUDEN,  POKETOKUSEI_DENKIENJIN, POKETOKUSEI_HIRAISIN,   POKETOKUSEI_NULL } },
    { POKETYPE_KUSA,  { POKETOKUSEI_SINRYOKU, POKETOKUSEI_NULL,       POKETOKUSEI_NULL,       POKETOKUSEI_NULL } },
    { POKETYPE_HONOO, { POKETOKUSEI_MORAIBI,  POKETOKUSEI_NULL,       POKETOKUSEI_NULL,       POKETOKUSEI_NULL } },
 };

  BPP_WAZADMG_REC rec;
  u32 i;
  u8 idx = 0;

  // 効果バツグンを持ってる場合、1/3の確率でチェックを行わない
  if( AI_ChangeProcSub_CheckWazaAff(wk, procPoke,  targetPoke, BTL_TYPEAFF_200) )
  {
    if( GFL_STD_Rand(&wk->AIRandContext, 3) == 0 ){
      return FALSE;
    }
  }


  while( BPP_WAZADMGREC_Get(procPoke, 1, idx++, &rec) )
  {
    BTL_N_Printf( DBGSTR_CLIENT_CHGAI_UkeTokStart, rec.wazaType );

    for(i=0; i<NELEMS(CheckTable); ++i)
    {
      if( CheckTable[i].wazaType == rec.wazaType )
      {
        u32 t;
        u8  hikae_idx;
        for(t=0; t<CHECK_TOK_MAX; ++t)
        {
          if( CheckTable[i].tokusei[t] == POKETOKUSEI_NULL ){ break; }
          if( AI_ChangeProcSub_CheckTokHikae(wk, CheckTable[i].tokusei[t], &hikae_idx) )
          {
            BTL_N_Printf( DBGSTR_CLIENT_CHGAI_UkeTokFind, hikae_idx, rec.wazaType );

            if( GFL_STD_Rand(&wk->AIRandContext, 2) == 0 )
            {
              BTL_N_Printf( DBGSTR_CLIENT_CHGAI_UkeTokDecide );

              *changeIndex = hikae_idx;
              return TRUE;
            }
          }
        }
      }
    }
  }
  return FALSE;
}
/**
 *  交換チェック：しぜんかいふくチェック
 */
static BOOL AI_ChangeProcSub_SizenKaifuku( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, u8* changeIndex )
{
  if( (BPP_GetValue(procPoke, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_SIZENKAIFUKU)
  &&  (BPP_CheckSick(procPoke, WAZASICK_NEMURI) || BPP_CheckSick(procPoke, WAZASICK_KOORI) )
  &&  (BPP_GetHPRatio(procPoke) >= FX32_CONST(50) )
  ){
    BPP_WAZADMG_REC rec;
    if( BPP_WAZADMGREC_Get(procPoke, 1, 0, &rec) )
    {
      u8 index;
      if( AI_ChangeProcSub_HikaePokeAff(wk, rec.wazaType, BTL_TYPEAFF_50, &index) )
      {
        *changeIndex = index;
        return TRUE;
      }
    }
    else
    {
      return (GFL_STD_Rand(&wk->AIRandContext, 2) == 0);
    }
  }
  return FALSE;
}
/**
 *  交換チェック：通常ワザ相性チェック
 */
static BOOL AI_ChangeProcSub_WazaAff( BTL_CLIENT* wk, const BTL_POKEPARAM* procPoke, const BTL_POKEPARAM* targetPoke, u8* changeIndex )
{
  if( AI_ChangeProcSub_CheckWazaAff(wk, procPoke, targetPoke, BTL_TYPEAFF_200) )
  {
    if( GFL_STD_Rand(&wk->AIRandContext, 10) != 0 ){
      return FALSE;
    }
  }



  {
    u32 i, rankCnt = 0;
    BPP_WAZADMG_REC rec;


    for(i=BPP_RANKVALUE_START; i<=BPP_RANKVALUE_END; ++i)
    {
      rankCnt += BPP_GetValue( procPoke, i );
    }
    if( rankCnt >= 4 ){
      return FALSE;
    }

    if( BPP_WAZADMGREC_Get(procPoke, 1, 0, &rec) )
    {
      u32 i, hikaeStart, memberCnt;

      memberCnt = BTL_PARTY_GetMemberCount( wk->myParty );
      hikaeStart = BTL_RULE_GetNumFrontPos( BTL_MAIN_GetRule(wk->mainModule) );

      for(i=hikaeStart; i<memberCnt; ++i)
      {
        if( !ChangeAI_CheckReserve(wk, i) )
        {
          const BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
          if( AI_ChangeProcSub_CheckWazaAff(wk, bpp, targetPoke, BTL_TYPEAFF_200) )
          {
            BtlTypeAff  aff = BTL_CALC_TypeAffPair( rec.wazaType, BPP_GetPokeType(bpp) );
            if( aff == BTL_TYPEAFF_0 )
            {
              if( GFL_STD_Rand(&wk->AIRandContext, 2) == 0 )
              {
                *changeIndex = i;
                return TRUE;
              }
            }
            else if( aff < BTL_TYPEAFF_100 )
            {
              if( GFL_STD_Rand(&wk->AIRandContext, 3) == 0 )
              {
                *changeIndex = i;
                return TRUE;
              }
            }
          }
        }
      }
    }
  }
  return FALSE;
}


/**
 *  交換チェック：控えポケモンの中に、指定とくせいを持ったポケモンがいるかチェック
 */
static BOOL AI_ChangeProcSub_CheckTokHikae( BTL_CLIENT* wk, u16 tokusei, u8* index )
{
  u32 i, hikaeStart, memberCnt;

  memberCnt = BTL_PARTY_GetMemberCount( wk->myParty );
  hikaeStart = BTL_RULE_HandPokeIndex( BTL_MAIN_GetRule(wk->mainModule), wk->numCoverPos );

  for(i=hikaeStart; i<memberCnt; ++i)
  {
    if( !ChangeAI_CheckReserve(wk, i) )
    {
      const BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
      if( !BPP_IsDead(bpp) && (BPP_GetValue(bpp, BPP_TOKUSEI_EFFECTIVE) == tokusei) )
      {
        *index = i;
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 *  交換チェック：控えポケモンの中に、対象ポケモンに対して相性affMin以上のダメージワザを持つポケモンがいるか
 */
static BOOL AI_ChangeProcSub_HikaeWazaAff( BTL_CLIENT* wk, const BTL_POKEPARAM* target, BtlTypeAff affMin )
{
  u32 i, hikaeStart, memberCnt;

  memberCnt = BTL_PARTY_GetMemberCount( wk->myParty );
  hikaeStart = BTL_RULE_GetNumFrontPos( BTL_MAIN_GetRule(wk->mainModule) );

  for(i=hikaeStart; i<memberCnt; ++i)
  {
    if( !ChangeAI_CheckReserve(wk, i) )
    {
      const BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
      if( AI_ChangeProcSub_CheckWazaAff(wk, bpp, target, affMin) ){
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 *  交換チェック：控えポケモンの中に、対象タイプのワザに対してaffMax以下のタイプであるポケモンがいるか
 */
static BOOL AI_ChangeProcSub_HikaePokeAff( BTL_CLIENT* wk, PokeType wazaType, BtlTypeAff affMax, u8* pokeIndex )
{
  u32 i, hikaeStart, memberCnt;

  memberCnt = BTL_PARTY_GetMemberCount( wk->myParty );
  hikaeStart = BTL_RULE_GetNumFrontPos( BTL_MAIN_GetRule(wk->mainModule) );

  for(i=hikaeStart; i<memberCnt; ++i)
  {
    if( !ChangeAI_CheckReserve(wk, i) )
    {
      const BTL_POKEPARAM* bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
      PokeTypePair  pokeType = BPP_GetPokeType( bpp );
      if( BTL_CALC_TypeAffPair(wazaType, pokeType) <= affMax )
      {
        *pokeIndex = i;
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 *  交換チェック：attacker が defender に対し効果 affMin 以上のダメージワザを持っているか判定
 */
static BOOL AI_ChangeProcSub_CheckWazaAff( BTL_CLIENT* wk, const BTL_POKEPARAM* attacker, const BTL_POKEPARAM* defender, BtlTypeAff affMin )
{
  if( !BPP_IsDead(attacker) )
  {
    u32 i;
    WazaID waza;
    PokeType wazaType;
    PokeTypePair targetPokeType = BPP_GetPokeType( defender );
    u32 waza_cnt = BPP_WAZA_GetCount( attacker );

    for(i=0; i<waza_cnt; ++i)
    {
      waza = BPP_WAZA_GetID( attacker, i );
      if( BPP_WAZA_GetPP(attacker, i)
      &&  WAZADATA_IsDamage(waza)
      &&  !is_unselectable_waza(wk, attacker, waza, NULL)
      ){
        wazaType = WAZADATA_GetType( waza );
        if( BTL_CALC_TypeAffPair(wazaType, targetPokeType) >= affMin ){
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

/**
 *  ローテーションチェック：
 */
static BtlRotateDir RotAI_CheckRotation( BTL_CLIENT* wk )
{
  const BTL_POKEPARAM* bpp;
  BtlRotateDir  dirAry[ BTL_ROTATE_NUM ];
  u32 i, cnt, r;

  dirAry[ 0 ] = BTL_ROTATEDIR_STAY;
  cnt = 0;

  i = BTL_MAINUTIL_GetRotateInPosIdx( BTL_ROTATEDIR_R );
  bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
  if( !BPP_IsDead(bpp) ){
    dirAry[ cnt++ ] = BTL_ROTATEDIR_R;
  }

  i = BTL_MAINUTIL_GetRotateInPosIdx( BTL_ROTATEDIR_L );
  bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
  if( !BPP_IsDead(bpp) ){
    dirAry[ cnt++ ] = BTL_ROTATEDIR_L;
  }

  if( cnt > 1 )
  {
    r = GFL_STD_Rand( &wk->AIRandContext, cnt );
    return dirAry[ r ];
  }

  return BTL_ROTATEDIR_STAY;
}

/**
 *  アクション選択：AI
 */
static BOOL SubProc_AI_SelectAction( BTL_CLIENT* wk, int* seq )
{
  enum {
    SEQ_INIT = 0,
    SEQ_ROOT,
    SEQ_WAZA_AI,
    SEQ_INC,
  };

  GF_ASSERT(wk->AIHandle);

  #ifdef PM_DEBUG
  if( BTL_MAIN_GetDebugFlag(wk->mainModule, BTL_DEBUGFLAG_AI_CTRL) )
  {
    if( AICtrl_IsMyFase(wk) ){
      return SubProc_UI_SelectAction( wk, seq );
    }
    return FALSE;
  }
  #endif

  switch( *seq ){
  case SEQ_INIT:
    ChangeAI_InitWork( wk );
    wk->procPokeIdx = 0;
    wk->actionAddCount = 0;
    (*seq) = SEQ_ROOT;
    /* fallthru */
  case SEQ_ROOT:
    if( wk->procPokeIdx < wk->numCoverPos )
    {
      wk->procPoke = BTL_PARTY_GetMemberDataConst( wk->myParty, wk->procPokeIdx );
      wk->procAction = &wk->actionParam[wk->procPokeIdx];

      if( BPP_IsDead(wk->procPoke) )
      {
        BTL_ACTION_SetNULL( wk->procAction );
        (*seq) = SEQ_INC;
        break;
      }
      else
      {
        // 行動選択できないチェック
        if( checkActionForceSet(wk, wk->procPoke,  wk->procAction) ){
          (*seq) = SEQ_INC;
          break;
        }

        // アイテム使用チェック
        {
          u16 itemID = AIItem_CheckUse( wk, wk->procPoke, wk->myParty );
          if( itemID != ITEM_DUMMY_DATA ){
            BTL_ACTION_SetItemParam( wk->procAction, itemID, wk->procPokeIdx, 0 );
            (*seq) = SEQ_INC;
            break;
          }
        }

        // 入れ替えチェック
        if( BTL_MAIN_GetCompetitor(wk->mainModule) != BTL_COMPETITOR_WILD )
        {
          u8 targetIndex;
          if( ChangeAI_Root(wk, wk->procPoke, wk->procPokeIdx, &targetIndex) ){
            BTL_ACTION_SetChangeParam( wk->procAction, wk->procPokeIdx, targetIndex );
            (*seq) = SEQ_INC;
            break;
          }
        }

        // ローテーションランダム決定
        #ifdef ROTATION_NEW_SYSTEM
        if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION )
        {
          BtlRotateDir dir = RotAI_CheckRotation( wk );
          if( dir != BTL_ROTATEDIR_STAY ){
            u8  pokeIdx = BTL_MAINUTIL_GetRotateInPosIdx( dir );

            BTL_ACTION_SetRotation( wk->procAction, dir );
            wk->procPoke = BTL_PARTY_GetMemberDataConst( wk->myParty, pokeIdx );
            wk->actionAddCount = 1;
            wk->procAction++;
          }
        }
        #endif

        // ワザロック状態
        if( BPP_CheckSick(wk->procPoke, WAZASICK_WAZALOCK) ){
          WazaID waza = BPP_SICKCONT_GetParam(BPP_GetSickCont(wk->procPoke, WAZASICK_WAZALOCK));
          BtlPokePos pos = BTL_CALC_DecideWazaTargetAuto( wk->mainModule, wk->pokeCon, wk->procPoke, waza );
          BTL_ACTION_SetFightParam( wk->procAction, waza, pos );
          (*seq) = SEQ_INC;
          break;
        }

        // わるあがきしか出せない判定
        if( checkWazaForceSet(wk, wk->procPoke, wk->procAction) ){
          (*seq) = SEQ_INC;
        }

        // ワザ選択AI
        {
          u8 wazaCount, wazaIdx, targetPos;
          u8 usableWazaFlag[ PTL_WAZA_MAX ];

          wazaCount = BPP_WAZA_GetCount( wk->procPoke );

          if( StoreSelectableWazaFlag(wk, wk->procPoke, usableWazaFlag) != PTL_WAZA_MAX )
          {
//            u8  mypos = BTL_MAIN_GetClientPokePos( wk->mainModule, wk->myID, wk->procPokeIdx );
            u8  mypos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(wk->procPoke) );

            TR_AI_Start( wk->AIHandle, usableWazaFlag, mypos );
            (*seq) = SEQ_WAZA_AI;
            break;
          }
          else
          {
            setWaruagakiAction( wk->procAction, wk, wk->procPoke );
            (*seq) = SEQ_INC;
            break;
          }
        }
      }
    }
    break;

  case SEQ_WAZA_AI:
    if( !TR_AI_Main(wk->AIHandle) )
    {
      u8 wazaIdx = TR_AI_GetSelectWazaPos( wk->AIHandle );
      if( wazaIdx != AI_ENEMY_ESCAPE )
      {
        u8 targetPos = TR_AI_GetSelectWazaDir( wk->AIHandle );

        WazaID  waza = BPP_WAZA_GetID( wk->procPoke, wazaIdx );
        BTL_ACTION_SetFightParam( wk->procAction, waza, targetPos );
        (*seq) = SEQ_INC;
      }
      else
      {
        BTL_ACTION_SetEscapeParam( wk->procAction );
        (*seq) = SEQ_INC;
      }
    }
    break;

  case SEQ_INC:
    wk->procPokeIdx++;
    if( wk->procPokeIdx >= wk->numCoverPos )
    {
      u32 dataSize = (wk->numCoverPos + wk->actionAddCount) * sizeof(wk->actionParam[0]);

      wk->returnDataPtr = &(wk->actionParam[0]);
      wk->returnDataSize = dataSize;
      wk->actionAddCount = 0;
      return TRUE;
    }
    else{
      (*seq) = SEQ_ROOT;
    }
    break;

  }
  return FALSE;
}
//--------------------------------------------------------------------------
/**
 * 死亡時の選択可能ポケモン（場に出ているポケモン以外で、生きているポケモン）の数
 *
 * @param   wk
 * @param   list    [out] 選択可能ポケモンのパーティ内インデックスを格納する配列
 *
 * @retval  u8    選択可能ポケモンの数
 */
//--------------------------------------------------------------------------
static u8 calcPuttablePokemons( BTL_CLIENT* wk, u8* list )
{
  const BTL_POKEPARAM* bpp;
  u8 cnt, numMembers, numFront, i, j;

  numMembers = BTL_PARTY_GetMemberCount( wk->myParty );
  numFront = BTL_RULE_HandPokeIndex( BTL_MAIN_GetRule(wk->mainModule), wk->numCoverPos );

  for(i=numFront, cnt=0; i<numMembers; i++)
  {
    bpp = BTL_PARTY_GetMemberDataConst(wk->myParty, i);
    if( BPP_IsFightEnable(bpp) )
    {
      if( list ){
        list[cnt] = i;
      }
      ++cnt;
    }
  }
  return cnt;
}
//----------------------------------------------------------------------------------
/**
 * calcPuttablePokemonsで生成した list を出すべき順に並べ替え（相手との相性による）
 *
 * @param   wk
 * @param   list
 * @param   numList
 *
 * @retval  u8
 */
//----------------------------------------------------------------------------------
static void sortPuttablePokemonList( BTL_CLIENT* wk, u8* list, u8 numPoke, const BTL_POKEPARAM* target )
{
  u16 affVal[ BTL_PARTY_MEMBER_MAX ];
  const BTL_POKEPARAM* bpp;
  PokeTypePair targetType;
  u8  i, j;

  targetType = BPP_GetPokeType( target );

  for(i=0; i<numPoke; ++i)
  {
    affVal[i] = 0;
    bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, list[i] );
    if( !BPP_IsDead(bpp) )
    {
      u8 wazaCnt = BPP_WAZA_GetCount( bpp );
      for(j=0; j<wazaCnt; ++j)
      {
        if( BPP_WAZA_GetPP(bpp, j) )
        {
          WazaID waza = BPP_WAZA_GetID( bpp, j );
          if( WAZADATA_IsDamage(waza) )
          {
            PokeType wazaType = WAZADATA_GetType( waza );
            u16 pow = WAZADATA_GetPower( waza );
            if( pow < 10 ){ // 10未満は特殊設定として大体60くらいに…
              pow = 60;
            }
            switch( BTL_CALC_TypeAffPair(wazaType, targetType) ){
            case BTL_TYPEAFF_400:  pow *= 4; break;
            case BTL_TYPEAFF_200:  pow *= 2; break;
            case BTL_TYPEAFF_50:   pow /= 2; break;
            case BTL_TYPEAFF_25:   pow /= 4; break;
            case BTL_TYPEAFF_0:    pow = 0; break;
            }

            if( affVal[i] < pow ){
              affVal[ i ] = pow;
            }
          }
        }
      }
    }
  }

  BTL_N_Printf( DBGSTR_CLIENT_CHGAI_PreSortList );
  for(i=0; i<numPoke; ++i){
    BTL_N_PrintfSimple( DBGSTR_CLIENT_CHGAI_SortListVal, list[i], affVal[i]);
  }
  BTL_N_PrintfSimple( DBGSTR_LF );

  for(i=0; i<numPoke; ++i)
  {
    for(j=i+1; j<numPoke; ++j)
    {
      if( affVal[i] < affVal[j] )
      {
        u16 tmp = affVal[i];
        affVal[i] = affVal[j];
        affVal[j] = tmp;

        tmp = list[i];
        list[i] = list[j];
        list[j] = tmp;
      }
    }
  }


  BTL_N_Printf( DBGSTR_CLIENT_CHGAI_AfterSortList );
  for(i=0; i<numPoke; ++i){
    BTL_N_PrintfSimple( DBGSTR_CLIENT_CHGAI_SortListVal, list[i], affVal[i]);
  }
  BTL_N_PrintfSimple( DBGSTR_LF );

}

//--------------------------------------------------------------------------
/**
 * ポケモン選択画面用パラメータセット
 *
 * @param   wk          クライアントモジュールハンドラ
 * @param   mode
 * @param   numSelect
 * @param   param       [out] 選択画面用パラメータを格納する
 * @param   result      [out] 選択画面結果データ（初期化される）
 *
 */
//--------------------------------------------------------------------------
static void setupPokeSelParam( BTL_CLIENT* wk, u8 mode, u8 numSelect, BTL_POKESELECT_PARAM* param, BTL_POKESELECT_RESULT* result )
{
  BTL_POKESELECT_PARAM_Init( param, wk->myParty, numSelect, mode );
  BTL_POKESELECT_PARAM_SetProhibitFighting( param, wk->numCoverPos );
  BTL_POKESELECT_RESULT_Init( result, param );
}

//-----------------------------------------------------------------------------
/**
 * ポケモン選択画面結果 -> 決定アクションパラメータに変換
 */
//-----------------------------------------------------------------------------
static void storePokeSelResult( BTL_CLIENT* wk, const BTL_POKESELECT_RESULT* res )
{
  // res->cnt=選択されたポケモン数,  wk->myChangePokeCnt=自分が担当する入れ替えリクエストポケモン数
  GF_ASSERT_MSG(res->cnt <= wk->myChangePokeCnt, "selCnt=%d, changePokeCnt=%d\n", res->cnt, wk->myChangePokeCnt);

  {
    u8 clientID, selIdx, outIdx, i;

    BTL_N_Printf( DBGSTR_CLIENT_PokeSelCnt, res->cnt);

    for(i=0; i<res->cnt; i++)
    {
      selIdx = res->selIdx[i];
      outIdx = res->outPokeIdx[i];

      BTL_N_Printf( DBGSTR_CLIENT_PokeChangeIdx, outIdx, selIdx );
      BTL_ACTION_SetChangeParam( &wk->actionParam[i], outIdx, selIdx );
    }
    wk->returnDataPtr = &(wk->actionParam[0]);
    wk->returnDataSize = sizeof(wk->actionParam[0]) * res->cnt;
  }
}
//-----------------------------------------------------------------------------
/**
 *  強制終了時のポケモン選択自動決定処理
 */
//-----------------------------------------------------------------------------
static void storePokeSelResult_ForceQuit( BTL_CLIENT* wk )
{
  const BTL_POKEPARAM* bpp;
  u8 clientID, posIdx, selIdx, memberMax, prevMemberIdx, selectCnt=0;
  u32 i, j;

  memberMax = BTL_PARTY_GetMemberCount( wk->myParty );
  prevMemberIdx = wk->numCoverPos;

  TAYA_Printf("時間切れだから強制的に選ぶ。%d体選択しなきゃならなくて、生きてるのは%d体だよ\n",
        wk->myChangePokeCnt, BTL_PARTY_GetAliveMemberCount(wk->myParty) );

  for(i=0; i<wk->myChangePokeCnt; ++i)
  {
    BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, wk->myChangePokePos[i], &clientID, &posIdx );
    TAYA_Printf("%d件目 .. 位置Idx=%d\n", i+1, posIdx);
    for(j=prevMemberIdx; j<memberMax; ++j)
    {
      bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, j );
      TAYA_Printf("Idx=%d, ポケ=%d, HP=%d\n", j, BPP_GetID(bpp), BPP_GetValue(bpp, BPP_HP));
      if( !BPP_IsDead(bpp) )
      {
        BTL_N_Printf( DBGSTR_CLIENT_ForcePokeChange, selectCnt+1, wk->myChangePokeCnt, posIdx, j );
        BTL_ACTION_SetChangeParam( &wk->actionParam[selectCnt++], posIdx, j );
        prevMemberIdx = j+1;
        break;
      }
      else{
        TAYA_Printf("たたかえない\n");
      }
    }
  }

  GF_ASSERT(selectCnt);

  wk->returnDataPtr = &(wk->actionParam[0]);
  wk->returnDataSize = sizeof(wk->actionParam[0]) * selectCnt;
}

//----------------------------------------------------------------------------------
/**
 * サーバから送られてきた交替リクエストポケモン位置から、自分の担当する位置のみを抜き出してバッファに格納
 *
 * @param   wk
 * @param   myCoverPos      [out] changePokePos中、自分が担当する位置を格納するバッファ
 *
 * @retval  u8    myCoverPos配列に格納した、自分が担当する位置の数.
 */
//----------------------------------------------------------------------------------
static u8 storeMyChangePokePos( BTL_CLIENT* wk, BtlPokePos* myCoverPos )
{
  u8 numChangePoke;
  const u8 *changePokePos;
  u32 i, cnt;

  numChangePoke = BTL_ADAPTER_GetRecvData( wk->adapter, (const void*)&changePokePos );
  BTL_N_Printf( DBGSTR_CLIENT_NumChangePokeBegin, numChangePoke);
  for(i=0; i<numChangePoke; ++i){
    BTL_N_PrintfSimple( DBGSTR_csv, changePokePos[i]);
  }
  BTL_N_PrintfSimple( DBGSTR_LF );

  for(i=0, cnt=0; i<numChangePoke; ++i)
  {
    if( BTL_MAIN_BtlPosToClientID(wk->mainModule, changePokePos[i]) == wk->myID ){
      myCoverPos[ cnt++ ] = changePokePos[ i ];
    }
  }
  BTL_N_Printf( DBGSTR_CLIENT_NumChangePokeResult, wk->myID, cnt);

  return cnt;
}

//----------------------------------------------------------------------------------
/**
 * 野生・シングル戦で自分のポケモンが倒れた時の「逃げる」「入れ替え」選択処理
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL SubProc_UI_SelectChangeOrEscape( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    BTLV_STRPARAM_Setup( &wk->strParam,    BTL_STRTYPE_UI, BTLMSG_UI_SEL_NEXT_POKE );
    BTLV_STRPARAM_Setup( &wk->strParamSub, BTL_STRTYPE_UI, BTLMSG_UI_SEL_ESCAPE );
    BTLV_YESNO_Start( wk->viewCore, &wk->strParam, &wk->strParamSub, FALSE );
    (*seq)++;
    break;

  case 1:
    {
      BtlYesNo result;
      if( BTLV_YESNO_Wait(wk->viewCore, &result) )
      {
        wk->change_escape_code = (result == BTL_YESNO_YES)? BTL_CLIENTASK_CHANGE : BTL_CLIENTASK_ESCAPE;
        wk->returnDataPtr = &(wk->change_escape_code);
        wk->returnDataSize = sizeof(wk->change_escape_code);
        return TRUE;
      }
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 【ポケモン選択】ひん死・生き返りで次に出すポケモンを選択
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL SubProc_UI_SelectPokemonForCover( BTL_CLIENT* wk, int* seq )
{
  return SelectPokemonUI_Core( wk, seq, BPL_MODE_CHG_DEAD );
}
//----------------------------------------------------------------------------------
/**
 * 【ポケモン選択】バトンタッチなど生きているポケモンをターン途中に入れ替える
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL SubProc_UI_SelectPokemonForChange( BTL_CLIENT* wk, int* seq )
{
  return SelectPokemonUI_Core( wk, seq, BPL_MODE_NO_CANCEL );
}
//----------------------------------------------------------------------------------
/**
 * 【ポケモン選択】AI
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL SubProc_AI_SelectPokemon( BTL_CLIENT* wk, int* seq )
{
  wk->myChangePokeCnt = storeMyChangePokePos( wk, wk->myChangePokePos );
  if( wk->myChangePokeCnt )
  {
    u8 puttableList[ BTL_PARTY_MEMBER_MAX ];
    u8 numPuttable;

    numPuttable = calcPuttablePokemons( wk, puttableList );
    if( numPuttable )
    {
      u8 numSelect = wk->myChangePokeCnt;
      u8 posIdx, clientID, i;

      {
        const BTL_POKEPARAM* target = ChangeAI_DecideTarget( wk, wk->myChangePokePos[0] );
        if( target ){
          sortPuttablePokemonList( wk, puttableList, numPuttable, target );
        }
      }

      // 生きてるポケが出さなければいけない数に足りない場合、そこを諦める
      if( numSelect > numPuttable )
      {
        #ifdef ROTATION_NEW_SYSTEM
        if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION ){
          // 新ローテーションの場合、強制的に生きているポケモンを前に出す回転を行う
          wk->returnDataSize = SetCoverRotateAction( wk, &(wk->actionParam[0]) );
          wk->returnDataPtr = &(wk->actionParam[0]);
          return TRUE;
        }
        #endif

        numSelect = numPuttable;
      }
      BTL_N_Printf( DBGSTR_CLIENT_AI_PutPokeStart, wk->myID, numSelect);
      for(i=0; i<numSelect; i++)
      {
        BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, wk->myChangePokePos[i], &clientID, &posIdx );
        BTL_ACTION_SetChangeParam( &wk->actionParam[i], posIdx, puttableList[i] );
        BTL_Printf( DBGSTR_CLIENT_AI_PutPokeDecide, puttableList[i] );
      }
      wk->returnDataPtr = &(wk->actionParam[0]);
      wk->returnDataSize = sizeof(wk->actionParam[0]) * numSelect;
    }
    else
    {
      #ifdef ROTATION_NEW_SYSTEM
      if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION )
      {
        // 新ローテーションの場合、強制的に生きているポケモンを前に出す回転を行う
        u32 sendDataSize = SetCoverRotateAction( wk, &(wk->actionParam[0]) );
        if( sendDataSize ){
          wk->returnDataSize = sendDataSize;
          wk->returnDataPtr = &(wk->actionParam[0]);
          return TRUE;
        }
      }
      #endif

      BTL_N_Printf( DBGSTR_CLIENT_NoMorePuttablePoke, wk->myID);
      BTL_ACTION_SetChangeDepleteParam( &wk->actionParam[0] );
      wk->returnDataPtr = &(wk->actionParam[0]);
      wk->returnDataSize = sizeof(wk->actionParam[0]);
    }
  }
  else
  {
     BTL_N_Printf( DBGSTR_CLIENT_NotDeadMember, wk->myID);
     BTL_ACTION_SetNULL( &wk->actionParam[0] );
     wk->returnDataPtr = &(wk->actionParam[0]);
     wk->returnDataSize = sizeof(wk->actionParam[0]);
  }
  return TRUE;
}
//----------------------------------------------------------------------------------
/**
 * 【ポケモン選択】録画再生
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL SubProc_REC_SelectPokemon( BTL_CLIENT* wk, int* seq )
{
  return SubProc_REC_SelectAction( wk, seq );
}

/**
 *  UIポケモン入れ替えコア処理
 */
static BOOL SelectPokemonUI_Core( BTL_CLIENT* wk, int* seq, u8 mode )
{
  switch( *seq ){
  case 0:
  {
    wk->myChangePokeCnt = storeMyChangePokePos( wk, wk->myChangePokePos );

    if( wk->myChangePokeCnt )
    {
      u8 puttableList[ BTL_PARTY_MEMBER_MAX ];
      u8 numPuttable = calcPuttablePokemons( wk, puttableList );

      // 控えに出せるポケモンが居る
      if( numPuttable )
      {
        u8 numSelect = wk->myChangePokeCnt;
        u8 clientID, outPokeIdx;

        // 生きてるポケが出さなければいけない数に足りない場合、そこを諦める
        if( numSelect > numPuttable )
        {
          numSelect = numPuttable;
        }

        // 新ローテーションの場合、先頭が空いている＆繰り出せる控え１体の時に自動決定
        #ifdef ROTATION_NEW_SYSTEM
        if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION )
        {
          const BTL_POKEPARAM* topPoke = BTL_PARTY_GetMemberDataConst( wk->myParty, 0 );
          if( (numPuttable == 1) && BPP_IsDead(topPoke) )
          {
            BTL_ACTION_SetChangeParam( &(wk->actionParam[0]), 0, puttableList[0] );
            wk->returnDataPtr = &(wk->actionParam[0]);
            wk->returnDataSize = sizeof(wk->actionParam[0]);
            return TRUE;
          }
        }
        #endif


        BTL_MAIN_BtlPosToClientID_and_PosIdx( wk->mainModule, wk->myChangePokePos[0], &clientID, &outPokeIdx );

        BTL_N_Printf( DBGSTR_CLIENT_ChangePokeCmdInfo, wk->myID, numSelect, outPokeIdx, mode );
        setupPokeSelParam( wk, mode, numSelect, &wk->pokeSelParam, &wk->pokeSelResult );
        BTLV_StartPokeSelect( wk->viewCore, &wk->pokeSelParam, outPokeIdx, FALSE, &wk->pokeSelResult );

        CmdLimit_Start( wk );
        (*seq)++;
      }
      // 控えに出せるポケモンがもう居ない
      else
      {
        #ifdef ROTATION_NEW_SYSTEM
        // 新ローテーションの場合、強制的に生きているポケモンを前に出す回転を行う
        if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_ROTATION )
        {
          const BTL_POKEPARAM* topPoke = BTL_PARTY_GetMemberDataConst( wk->myParty, 0 );
          if( BPP_IsDead(topPoke) )
          {
            u32 sendDataSize = SetCoverRotateAction( wk, &(wk->actionParam[0]) );
            if( sendDataSize ){
              wk->returnDataSize = sendDataSize;
              wk->returnDataPtr = &(wk->actionParam[0]);
              return TRUE;
            }
          }
        }
        #endif

        BTL_N_Printf( DBGSTR_CLIENT_NoMorePuttablePoke, wk->myID);
        BTL_ACTION_SetChangeDepleteParam( &wk->actionParam[0] );
        wk->returnDataPtr = &(wk->actionParam[0]);
        wk->returnDataSize = sizeof(wk->actionParam[0]);
        return TRUE;
      }
    }
    // 自分は空きが出来ていないので何も選ぶ必要がない
    else
    {
      BTL_N_Printf( DBGSTR_CLIENT_NotDeadMember, wk->myID);
      BTL_ACTION_SetNULL( &wk->actionParam[0] );
      wk->returnDataPtr = &(wk->actionParam[0]);
      wk->returnDataSize = sizeof(wk->actionParam[0]);
      return TRUE;
    }
  }
  break;

  case 1:
    if( CmdLimit_CheckOver(wk) ){
      BTLV_ForceQuitPokeSelect( wk->viewCore );
      (*seq) = 2;
      break;
    }

    if( BTLV_WaitPokeSelect(wk->viewCore) )
    {
      storePokeSelResult( wk, &wk->pokeSelResult );
      (*seq) = 3;
    }
    break;

  // コマンド制限時間による強制終了処理
  case 2:
    if( BTLV_WaitPokeSelect(wk->viewCore) )
    {
      storePokeSelResult_ForceQuit( wk );
      (*seq) = 3;
    }
    break;

  case 3:
    CmdLimit_End( wk );
    return TRUE;

  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * ローテーションバトルで、前衛が死んで控えが居ない時に回転を行う（後衛を前衛に出すため）
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static u32 SetCoverRotateAction( BTL_CLIENT* wk, BTL_ACTION_PARAM* resultAction )
{
  BtlRotateDir  dir = BTL_ROTATEDIR_R;
  const BTL_POKEPARAM* bpp;
  u32 pokeIdx;

  pokeIdx = BTL_MAINUTIL_GetRotateInPosIdx( dir );
  bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, pokeIdx );
  if( !BPP_IsDead(bpp) )
  {
    BTL_ACTION_SetRotation( resultAction, dir );
    BTL_N_Printf( DBGSTR_CLIENT_CoverRotateDecide, wk->myID, dir );
    return sizeof( BTL_ACTION_PARAM );
  }

  dir = BTL_ROTATEDIR_L;
  pokeIdx = BTL_MAINUTIL_GetRotateInPosIdx( dir );
  bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, pokeIdx );
  if( !BPP_IsDead(bpp) )
  {
    BTL_ACTION_SetRotation( resultAction, dir );
    BTL_N_Printf( DBGSTR_CLIENT_CoverRotateDecide, wk->myID, dir );
    return sizeof( BTL_ACTION_PARAM );
  }

  return 0;
}

//----------------------------------------------------------------------------------
/**
 * スタンドアロン入れ替え戦の入れ替え確認処理
 *
 * @param   wk
 * @param   seq
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL SubProc_UI_ConfirmIrekae( BTL_CLIENT* wk, int* seq )
{
  enum {
    SEQ_INIT = 0,
    SEQ_START_CONFIRM,
    SEQ_WAIT_CONFIRM,
    SEQ_WAIT_POKESELECT,
    SEQ_DONT_CHANGE,
    SEQ_RETURN,
  };

  switch( *seq ){
  case 0:
    {
      const u8* enemyPutPokeID;
      BTL_ADAPTER_GetRecvData( wk->adapter, (const void**)(&enemyPutPokeID) );
      if( (*enemyPutPokeID) != BTL_POKEID_NULL )
      {
        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_IrekaeConfirm );
        BTLV_STRPARAM_AddArg( &wk->strParam, BTL_CLIENTID_SA_ENEMY1 );
        BTLV_STRPARAM_AddArg( &wk->strParam, *enemyPutPokeID );
        BTLV_StartMsg( wk->viewCore, &wk->strParam );
        (*seq) = SEQ_START_CONFIRM;
      }
      else
      {
        (*seq) = SEQ_DONT_CHANGE;
      }
    }
    break;

  case SEQ_START_CONFIRM:
    BTLV_WaitMsg( wk->viewCore );
    if( BTLV_IsJustDoneMsg(wk->viewCore) )
    {
      BTLV_STRPARAM_Setup( &wk->strParam,    BTL_STRTYPE_UI, BTLMSG_UI_SEL_IREKAE_YES );
      BTLV_STRPARAM_Setup( &wk->strParamSub, BTL_STRTYPE_UI, BTLMSG_UI_SEL_IREKAE_NO );
      BTLV_YESNO_Start( wk->viewCore, &wk->strParam, &wk->strParamSub, TRUE );
      (*seq) = SEQ_WAIT_CONFIRM;
    }
    break;

  case SEQ_WAIT_CONFIRM:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      BtlYesNo result;
      if( BTLV_YESNO_Wait(wk->viewCore, &result) )
      {
        if( result == BTL_YESNO_YES )
        {
          setupPokeSelParam( wk, BPL_MODE_NORMAL, 1, &wk->pokeSelParam, &wk->pokeSelResult );
          BTLV_StartPokeSelect( wk->viewCore, &wk->pokeSelParam, 0, FALSE, &wk->pokeSelResult );
          (*seq) = SEQ_WAIT_POKESELECT;
        }
        else
        {
          (*seq) = SEQ_DONT_CHANGE;
        }
      }
    }
    break;

  case SEQ_WAIT_POKESELECT:
    if( BTLV_WaitPokeSelect(wk->viewCore) )
    {
      if( !BTL_POKESELECT_IsCancel(&wk->pokeSelResult) )
      {
        u8 idx = BTL_POKESELECT_RESULT_GetLast( &wk->pokeSelResult );
        if( (idx!=0) && (idx < BTL_PARTY_MEMBER_MAX) )
        {
          BTL_ACTION_SetChangeParam( &wk->actionParam[0], 0, idx );
          (*seq) = SEQ_RETURN;
          break;
        }
      }
      (*seq) = SEQ_DONT_CHANGE;
    }
    break;

  case SEQ_DONT_CHANGE:
    BTL_ACTION_SetNULL( &wk->actionParam[0] );
    (*seq) = SEQ_RETURN;
    /* fallthru */
  case SEQ_RETURN:
    wk->returnDataPtr = &(wk->actionParam[0]);
    wk->returnDataSize = sizeof(wk->actionParam[0]);
    return TRUE;

  }
  return FALSE;
}




//---------------------------------------------------
// 操作記録データ受信->保存
//---------------------------------------------------
static BOOL SubProc_UI_RecordData( BTL_CLIENT* wk, int* seq )
{
  u32 dataSize;
  const void* dataBuf;

  dataSize = BTL_ADAPTER_GetRecvData( wk->adapter, &dataBuf );

  if( wk->btlRec )
  {
    BTL_REC_Write( wk->btlRec, dataBuf, dataSize );
  }

  if( wk->cmdCheckServer )
  {
    BTL_SERVER_CMDCHECK_RestoreActionData( wk->cmdCheckServer, dataBuf, dataSize );
    wk->cmdCheckTimingCode = BTL_REC_GetTimingCode( dataBuf );;
//    TAYA_Printf( "整合性チェック TimingCode=%d\n", wk->cmdCheckTimingCode);
  }

  return TRUE;
}
//---------------------------------------------------
// 通信対戦終了
//---------------------------------------------------
static BOOL SubProc_REC_ExitCommTrainer( BTL_CLIENT* wk, int* seq )
{
  if( wk->viewCore )
  {
    BOOL fDone = SubProc_UI_ExitCommTrainer( wk, seq );

    //  録画コマンドを最後まで読み取ったかどうかで完遂チェックすることにしたので
    //  クライアントから通知が不要になった
    #if 0
    if( fDone ){
      BTL_MAIN_NotifyRecPlayComplete( wk->mainModule );
    }
    #endif

    return fDone;
  }
  return TRUE;
}
static BOOL SubProc_UI_ExitCommTrainer( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    {
      BtlResult  result;
      u16 strID;
      u8  fMulti;
      u8 clientID = BTL_MAIN_GetEnemyClientID( wk->mainModule, 0 );

      result = expandServerResult( wk );
      fMulti = BTL_MAIN_IsMultiMode( wk->mainModule );

      // 自分が非サーバである場合のため、Mainにサーバ計算した勝敗結果を渡しておく
      BTL_MAIN_NotifyBattleResult( wk->mainModule, result );

      switch( result ){
      case BTL_RESULT_WIN:
        strID = (fMulti)? BTL_STRID_STD_WinCommMulti : BTL_STRID_STD_WinComm;
        break;
      case BTL_RESULT_LOSE:
        strID = (fMulti)? BTL_STRID_STD_LoseCommMulti : BTL_STRID_STD_LoseComm;
        break;
      case BTL_RESULT_DRAW:
        strID = (fMulti)? BTL_STRID_STD_DrawCommMulti : BTL_STRID_STD_DrawComm;
        break;
      default:
        return TRUE;
      }

      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, strID );
      BTLV_STRPARAM_AddArg( &wk->strParam, clientID );
      if( fMulti ){
        u8 clientID_2 = BTL_MAIN_GetEnemyClientID( wk->mainModule, 1 );
        BTLV_STRPARAM_AddArg( &wk->strParam, clientID_2);
      }
      BTLV_StartMsg( wk->viewCore, &wk->strParam );

      if( result == BTL_RESULT_WIN ){
        u32 winBGMNo = BTL_MAIN_GetWinBGMNo( wk->mainModule );
        PMSND_PlayBGM( winBGMNo );
        (*seq)++;
      }else{
        PMSND_FadeOutBGM( BTL_BGM_FADEOUT_FRAMES );
        (*seq)+=2;
      }
    }
    break;

  //
  case 1:
    if( BTLV_WaitMsg(wk->viewCore) ){
      return TRUE;
    }
    break;

  case 2:
    if( BTLV_WaitMsg(wk->viewCore) ){
      (*seq)++;
    }
    break;
  case 3:
    if( !PMSND_CheckFadeOnBGM() ){
      PMSND_StopBGM();
      return TRUE;
    }
  }
  return FALSE;
}

/**
 *  サーバが送ってきた勝敗パラメータを解釈->自分が勝ったか負けたか判定
 */
static BtlResult expandServerResult( BTL_CLIENT* wk )
{
  const BTL_RESULT_CONTEXT* resultContext;

  BTL_ADAPTER_GetRecvData( wk->adapter, (const void*)(&resultContext) );

  switch( resultContext->resultCode ){
  case BTL_RESULT_LOSE:
    if( !BTL_MAINUTIL_IsFriendClientID(resultContext->clientID, wk->myID) ){
      return BTL_RESULT_WIN;
    }
    break;

  case BTL_RESULT_WIN:
    if( !BTL_MAINUTIL_IsFriendClientID(resultContext->clientID, wk->myID) ){
      return BTL_RESULT_LOSE;
    }
    break;

  }
  return resultContext->resultCode;
}

/**
 *  相手クライアントが２人かどうか判定
 */
static inline BOOL IsEnemyClientDouble( BTL_CLIENT* wk )
{
  u8 clientID_1 = BTL_MAIN_GetEnemyClientID( wk->mainModule, 0 );
  u8 clientID_2 = BTL_MAIN_GetEnemyClientID( wk->mainModule, 1 );

  return (clientID_1 != clientID_2);
}
/**
 *  トレーナー（ゲーム内・サブウェイ共通）グラフィックスライドイン
 */
static inline void TrainerGraphicIn( BTL_CLIENT* wk, int client_idx )
{
  u8 clientID = BTL_MAIN_GetEnemyClientID( wk->mainModule, client_idx );
  int trtype = BTL_MAIN_GetClientTrainerType( wk->mainModule, clientID );

  BTLV_EFFECT_SetTrainer( trtype, BTLV_MCSS_POS_TR_BB, 0, 0, 0 );
  BTLV_EFFECT_Add( BTLEFF_TRAINER_IN );
}
/**
 *  トレーナー（ゲーム内・サブウェイ共通）に勝った時の勝利メッセージプリント開始
 */
static inline void MsgWinningTrainerStart( BTL_CLIENT* wk )
{
  u8 clientID_1 = BTL_MAIN_GetEnemyClientID( wk->mainModule, 0 );
  u8 clientID_2 = BTL_MAIN_GetEnemyClientID( wk->mainModule, 1 );

  // 相手クライアントが１人
  if( clientID_1 == clientID_2 )
  {
    BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_WinTrainer );
    BTLV_STRPARAM_AddArg( &wk->strParam, clientID_1 );
  }
  // 相手クライアントが２人
  else
  {
    BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_WinTrainer2 );
    BTLV_STRPARAM_AddArg( &wk->strParam, clientID_1 );
    BTLV_STRPARAM_AddArg( &wk->strParam, clientID_2 );
  }

  BTLV_StartMsg( wk->viewCore, &wk->strParam );
}


//---------------------------------------------------
// ゲーム内トレーナー戦を終了
//---------------------------------------------------
static BOOL SubProc_REC_ExitForNPC( BTL_CLIENT* wk, int* seq )
{
  if( wk->viewCore )
  {
    BOOL result = SubProc_UI_ExitForNPC( wk, seq );

    // 録画コマンドを最後まで読み取ったかどうかで完遂チェックすることにしたので
    // クライアントから通知が不要になった
    #if 0
    if( result ){
      BTL_MAIN_NotifyRecPlayComplete( wk->mainModule );
    }
    #endif

    return result;

  }
  return TRUE;
}
static BOOL SubProc_UI_ExitForNPC( BTL_CLIENT* wk, int* seq )
{
  enum {
    SEQ_INIT = 0,
    SEQ_WIN_START,
    SEQ_WIN_WAIT_TR1_IN,
    SEQ_WIN_WAIT_TR1_MSG,
    SEQ_WIN_WAIT_TR1_OUT,
    SEQ_WIN_WAIT_TR2_IN,
    SEQ_WIN_WAIT_TR2_MSG,
    SEQ_WIN_GET_MONEY,
    SEQ_WIN_BONUS_MONEY,

    SEQ_LOSE_START,
    SEQ_LOSE_WAIT_MSG1,

    SEQ_END,
  };


  switch( *seq ){
  case SEQ_INIT:
    {
      BtlResult resultCode = expandServerResult( wk );

      if( resultCode == BTL_RESULT_WIN )
      {
        PMSND_PlayBGM( BTL_MAIN_GetWinBGMNo( wk->mainModule ) );
        MsgWinningTrainerStart( wk );
        (*seq) = SEQ_WIN_START;
      }
      else if( BTL_MAIN_GetSetupStatusFlag(wk->mainModule, BTL_STATUS_FLAG_NO_LOSE) == FALSE )
      {
        u8 clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );

        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_LoseStart );
        BTLV_STRPARAM_AddArg( &wk->strParam, clientID );
        BTLV_STRPARAM_AddArg( &wk->strParam, clientID );
        BTLV_StartMsg( wk->viewCore, &wk->strParam );
        (*seq) = SEQ_LOSE_START;
      }
      else{
        return TRUE;
      }
    }
    break;
  case SEQ_WIN_START:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      TrainerGraphicIn( wk, 0 );
      (*seq) = SEQ_WIN_WAIT_TR1_IN;
    }
    break;
  case SEQ_WIN_WAIT_TR1_IN:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u8 clientID = BTL_MAIN_GetEnemyClientID( wk->mainModule, 0 );
      u32 trainerID = BTL_MAIN_GetClientTrainerID( wk->mainModule, clientID );
      BTLV_StartMsgTrainer( wk->viewCore, trainerID, TRMSG_FIGHT_LOSE );
      (*seq) = SEQ_WIN_WAIT_TR1_MSG;
    }
    break;
  case SEQ_WIN_WAIT_TR1_MSG:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      if( IsEnemyClientDouble(wk) )
      {
        BTLV_EFFECT_Add( BTLEFF_TRAINER_OUT );
        (*seq) = SEQ_WIN_WAIT_TR1_OUT;
      }
      else
      {
        (*seq) = SEQ_WIN_GET_MONEY;
      }
    }
    break;
  case SEQ_WIN_WAIT_TR1_OUT:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      TrainerGraphicIn( wk, 1 );
      (*seq) = SEQ_WIN_WAIT_TR2_IN;
    }
    break;
  case SEQ_WIN_WAIT_TR2_IN:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u8 clientID = BTL_MAIN_GetEnemyClientID( wk->mainModule, 1 );
      u32 trainerID = BTL_MAIN_GetClientTrainerID( wk->mainModule, clientID );
      BTLV_StartMsgTrainer( wk->viewCore, trainerID, TRMSG_FIGHT_LOSE );
      (*seq) = SEQ_WIN_WAIT_TR2_MSG;
    }
    break;
  case SEQ_WIN_WAIT_TR2_MSG:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      (*seq) = SEQ_WIN_GET_MONEY;
    }
    break;
  case SEQ_WIN_GET_MONEY:
    {
      u32 getMoney = BTL_MAIN_FixRegularMoney( wk->mainModule );
      if( getMoney )
      {
        u8 clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );

        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_GetMoney );
        BTLV_STRPARAM_AddArg( &wk->strParam, clientID );
        BTLV_STRPARAM_AddArg( &wk->strParam, getMoney );
        BTLV_StartMsg( wk->viewCore, &wk->strParam );
      }

      if( BTL_MAIN_GetBonusMoney(wk->mainModule) == 0 ){
        (*seq) = SEQ_END;
      }else{
        (*seq) = SEQ_WIN_BONUS_MONEY;
      }
    }
    break;
  case SEQ_WIN_BONUS_MONEY:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      u32 bonus = BTL_MAIN_GetBonusMoney( wk->mainModule );
      if( bonus )
      {
        u8 clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );

        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_PickMoney );
        BTLV_STRPARAM_AddArg( &wk->strParam, clientID );
        BTLV_STRPARAM_AddArg( &wk->strParam, bonus );
        BTLV_StartMsg( wk->viewCore, &wk->strParam );
      }
      (*seq) = SEQ_END;
    }
    break;

  case SEQ_LOSE_START:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      u32 loseMoney = BTL_MAIN_FixLoseMoney( wk->mainModule );
      if( loseMoney )
      {
        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_LoseMoneyTrainer );
        BTLV_STRPARAM_AddArg( &wk->strParam, wk->myID );
        BTLV_STRPARAM_AddArg( &wk->strParam, loseMoney );
        BTLV_StartMsg( wk->viewCore, &wk->strParam );
      }
      (*seq) = SEQ_LOSE_WAIT_MSG1;
    }
    break;
  case SEQ_LOSE_WAIT_MSG1:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_LoseEnd );
      BTLV_STRPARAM_AddArg( &wk->strParam, wk->myID );
      BTLV_StartMsg( wk->viewCore, &wk->strParam );
      (*seq) = SEQ_END;
    }
    break;

  case SEQ_END:
    if( BTLV_WaitMsg(wk->viewCore) ){
      return TRUE;
    }
    break;

  default:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------
// サブウェイトレーナー戦を終了
//---------------------------------------------------
static BOOL SubProc_REC_ExitForSubwayTrainer( BTL_CLIENT* wk, int* seq )
{
  if( wk->viewCore )
  {
    BOOL result = SubProc_UI_ExitForSubwayTrainer( wk, seq );

    // 録画コマンドを最後まで読み取ったかどうかで完遂チェックすることにしたので
    // クライアントから通知が不要になった
    #if 0
    if( result ){
      BTL_MAIN_NotifyRecPlayComplete( wk->mainModule );
    }
    #endif

    return result;
  }
  return TRUE;
}
static BOOL SubProc_UI_ExitForSubwayTrainer( BTL_CLIENT* wk, int* seq )
{
  enum {
    SEQ_INIT,
    SEQ_WAIT_TRAINER_IN,
    SEQ_WAIT_MSG,
    SEQ_WAIT_TRAINER_OUT,
    SEQ_WAIT_TRAINER2_IN,
    SEQ_WAIT_MSG2,
  };

  switch( *seq ){
  case SEQ_INIT:
    {
      BtlResult result = expandServerResult( wk );

      // サブウェイはAIマルチなどもあり得るので
      // 自分が非サーバである場合のため、Mainにサーバ計算した勝敗結果を渡しておく
      BTL_MAIN_NotifyBattleResult( wk->mainModule, result );

      if( result == BTL_RESULT_WIN )
      {
        PMSND_PlayBGM( BTL_MAIN_GetWinBGMNo( wk->mainModule ) );
      }
      if( (result == BTL_RESULT_WIN) || (result == BTL_RESULT_LOSE) ){
        TrainerGraphicIn( wk, 0 );
        setupSubwayTrainerMsg( wk, result, 0 );
        (*seq) = SEQ_WAIT_TRAINER_IN;
      }
      else{
        return TRUE;
      }
    }
    break;

  case SEQ_WAIT_TRAINER_IN:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTLV_StartMsgInBuffer( wk->viewCore );
      (*seq) = SEQ_WAIT_MSG;
    }
    break;

  case SEQ_WAIT_MSG:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      if( !IsEnemyClientDouble(wk) ){
        return TRUE;
      }else{
        BTLV_EFFECT_Add( BTLEFF_TRAINER_OUT );
        (*seq) = SEQ_WAIT_TRAINER_OUT;
      }
    }
    break;

  case SEQ_WAIT_TRAINER_OUT:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BtlResult result = expandServerResult( wk );

      TrainerGraphicIn( wk, 1 );
      setupSubwayTrainerMsg( wk, result, 1 );
      (*seq) = SEQ_WAIT_TRAINER2_IN;
    }
    break;

  case SEQ_WAIT_TRAINER2_IN:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTLV_StartMsgInBuffer( wk->viewCore );
      (*seq) = SEQ_WAIT_MSG2;
    }
    break;

  case SEQ_WAIT_MSG2:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * サブウェイトレーナーの勝敗メッセージをBTLV内部に生成しておく
 *
 * @param   wk
 * @param   result
 */
//----------------------------------------------------------------------------------
static void setupSubwayTrainerMsg( BTL_CLIENT* wk, BtlResult result, u8 client_idx )
{
  u8 clientID = BTL_MAIN_GetEnemyClientID( wk->mainModule, client_idx );
  const PMS_DATA* pms;

  pms = BTL_MAIN_GetClientPMSData( wk->mainModule, clientID, result );

  if( (pms != NULL) && PMSDAT_IsEnabled(pms) )
  {
    STRBUF* tmpbuf = PMSDAT_ToString( pms, GFL_HEAP_LOWID(wk->heapID) );

    BTLV_CopyMsgToBuffer( wk->viewCore, tmpbuf );

    GFL_STR_DeleteBuffer( tmpbuf );
  }
  else
  {
    u32 subwayTrID = BTL_MAIN_GetClientTrainerID( wk->mainModule, clientID );

    if( subwayTrID )
    {
      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_SUBWAY_LOCALTRAINER, 0 );
      BTLV_STRPARAM_AddArg( &wk->strParam, subwayTrID );
      BTLV_STRPARAM_AddArg( &wk->strParam, result );
      BTLV_SetMsgToBuffer( wk->viewCore, &wk->strParam );
    }
  }
}
//---------------------------------------------------
// 野生戦で勝った
//---------------------------------------------------
static BOOL SubProc_UI_WinWild( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    // シナリオのシン・ム戦では落ちてるお金を拾わない
    if( !BTL_MAIN_GetSetupStatusFlag(wk->mainModule, BTL_STATUS_FLAG_LEGEND_EX) )
    {
      u32 bonus = BTL_MAIN_GetBonusMoney( wk->mainModule );
      if( bonus )
      {
        u8 clientID = BTL_MAIN_GetPlayerClientID( wk->mainModule );
        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_PickMoney );
        BTLV_STRPARAM_AddArg( &wk->strParam, clientID );
        BTLV_STRPARAM_AddArg( &wk->strParam, bonus );
        BTLV_StartMsg( wk->viewCore, &wk->strParam );
      }
    }
    (*seq)++;
  case 1:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      return TRUE;
    }
    break;

  }
  return FALSE;
}
//---------------------------------------------------
// 野生戦で負けた
//---------------------------------------------------
static BOOL SubProc_UI_LoseWild( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_LoseStart );
    BTLV_STRPARAM_AddArg( &wk->strParam, wk->myID );
    BTLV_StartMsg( wk->viewCore, &wk->strParam );
    (*seq)++;
    break;

  case 1:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      u32 loseMoney = BTL_MAIN_FixLoseMoney( wk->mainModule );
      if( loseMoney )
      {
        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_LoseMoneyWild );
        BTLV_STRPARAM_AddArg( &wk->strParam, wk->myID );
        BTLV_STRPARAM_AddArg( &wk->strParam, loseMoney );
        BTLV_StartMsg( wk->viewCore, &wk->strParam );
      }
      (*seq)++;
    }
    break;

  case 2:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_LoseEnd );
      BTLV_STRPARAM_AddArg( &wk->strParam, wk->myID );
      BTLV_StartMsg( wk->viewCore, &wk->strParam );
      (*seq)++;
    }
    break;

  case 3:
    if( BTLV_WaitMsg(wk->viewCore) ){
      (*seq)++;
    }
    break;

  default:
    return TRUE;
  }

  return FALSE;
}


//---------------------------------------------------
// 制限時間終了
//---------------------------------------------------
static BOOL SubProc_UI_NotifyTimeUp( BTL_CLIENT* wk, int* seq )
{
  switch( *seq ){
  case 0:
    BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_BattleTimeOver );
    BTLV_StartMsg( wk->viewCore, &wk->strParam );
    (*seq)++;
    break;
  case 1:
    if( BTLV_WaitMsg(wk->viewCore) ){
      (*seq)++;
    }
    break;
  case 2:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------
// サーバコマンド処理
//---------------------------------------------------
static BOOL SubProc_REC_ServerCmd( BTL_CLIENT* wk, int* seq )
{
  if( wk->viewCore )
  {
    return SubProc_UI_ServerCmd( wk, seq );
  }
  return TRUE;
}
static BOOL SubProc_UI_ServerCmd( BTL_CLIENT* wk, int* seq )
{
  static const struct {
    u32       cmd;
    ServerCmdProc proc;
  }scprocTbl[] = {
    { SC_MSG_STD,               scProc_MSG_Std            },
    { SC_MSG_STD_SE,            scProc_MSG_StdSE          },
    { SC_MSG_SET,               scProc_MSG_Set            },
    { SC_MSG_WAZA,              scProc_MSG_Waza           },
    { SC_ACT_WAZA_EFFECT,       scProc_ACT_WazaEffect     },
    { SC_ACT_TAMEWAZA_HIDE,     scProc_ACT_TameWazaHide   },
    { SC_ACT_WAZA_DMG,          scProc_ACT_WazaDmg        },
    { SC_ACT_WAZA_DMG_PLURAL,   scProc_ACT_WazaDmg_Plural },
    { SC_ACT_WAZA_ICHIGEKI,     scProc_ACT_WazaIchigeki   },
    { SC_ACT_SICK_ICON,         scProc_ACT_SickIcon       },
    { SC_ACT_CONF_DMG,          scProc_ACT_ConfDamage     },
    { SC_ACT_DEAD,              scProc_ACT_Dead           },
    { SC_ACT_RELIVE,            scProc_ACT_Relive         },
    { SC_ACT_MEMBER_OUT_MSG,    scProc_ACT_MemberOutMsg   },
    { SC_ACT_MEMBER_OUT,        scProc_ACT_MemberOut      },
    { SC_ACT_MEMBER_IN,         scProc_ACT_MemberIn       },
    { SC_ACT_RANKUP,            scProc_ACT_RankUp         },
    { SC_ACT_RANKDOWN,          scProc_ACT_RankDown       },
    { SC_ACT_WEATHER_DMG,       scProc_ACT_WeatherDmg     },
    { SC_ACT_WEATHER_START,     scProc_ACT_WeatherStart   },
    { SC_ACT_WEATHER_END,       scProc_ACT_WeatherEnd     },
    { SC_ACT_SIMPLE_HP,         scProc_ACT_SimpleHP       },
    { SC_ACT_KINOMI,            scProc_ACT_Kinomi         },
    { SC_TOKWIN_IN,             scProc_TOKWIN_In          },
    { SC_TOKWIN_OUT,            scProc_TOKWIN_Out         },
    { SC_OP_HP_MINUS,           scProc_OP_HpMinus         },
    { SC_OP_HP_PLUS,            scProc_OP_HpPlus          },
    { SC_OP_HP_ZERO,            scProc_OP_HpZero          },
    { SC_OP_PP_MINUS,           scProc_OP_PPMinus         },
    { SC_OP_PP_MINUS_ORG,       scProc_OP_PPMinus_Org     },
    { SC_OP_WAZA_USED,          scProc_OP_WazaUsed        },
    { SC_OP_PP_PLUS,            scProc_OP_PPPlus          },
    { SC_OP_PP_PLUS_ORG,        scProc_OP_PPPlus_Org      },
    { SC_OP_RANK_UP,            scProc_OP_RankUp          },
    { SC_OP_RANK_DOWN,          scProc_OP_RankDown        },
    { SC_OP_RANK_SET7,          scProc_OP_RankSet7        },
    { SC_OP_RANK_RECOVER,       scProc_OP_RankRecover     },
    { SC_OP_RANK_RESET,         scProc_OP_RankReset       },
    { SC_OP_ADD_CRITICAL,       scProc_OP_AddCritical     },
    { SC_OP_SICK_SET,           scProc_OP_SickSet         },
    { SC_OP_CURE_POKESICK,      scProc_OP_CurePokeSick    },
    { SC_OP_CURE_WAZASICK,      scProc_OP_CureWazaSick    },
    { SC_OP_MEMBER_IN,          scProc_OP_MemberIn        },
    { SC_OP_SET_STATUS,         scProc_OP_SetStatus       },
    { SC_OP_SET_WEIGHT,         scProc_OP_SetWeight       },
    { SC_OP_CHANGE_POKETYPE,    scProc_OP_ChangePokeType  },
    { SC_OP_WAZASICK_TURNCHECK, scProc_OP_WSTurnCheck     },
    { SC_OP_CONSUME_ITEM,       scProc_OP_ConsumeItem     },
    { SC_OP_UPDATE_USE_WAZA,    scProc_OP_UpdateUseWaza   },
    { SC_OP_SET_CONTFLAG,       scProc_OP_SetContFlag     },
    { SC_OP_RESET_CONTFLAG,     scProc_OP_ResetContFlag   },
    { SC_OP_SET_TURNFLAG,       scProc_OP_SetTurnFlag     },
    { SC_OP_RESET_TURNFLAG,     scProc_OP_ResetTurnFlag   },
    { SC_OP_CHANGE_TOKUSEI,     scProc_OP_ChangeTokusei   },
    { SC_OP_SET_ITEM,           scProc_OP_SetItem         },
    { SC_OP_UPDATE_WAZANUMBER,  scProc_OP_UpdateWazaNumber},
    { SC_OP_OUTCLEAR,           scProc_OP_OutClear        },
    { SC_OP_ADD_FLDEFF,         scProc_OP_AddFldEff       },
    { SC_OP_ADD_FLDEFF_DEPEND,  scProc_OP_AddFldEffDepend },
    { SC_OP_DEL_FLDEFF_DEPEND,  scProc_OP_DelFldEffDepend },
    { SC_OP_REMOVE_FLDEFF,      scProc_OP_RemoveFldEff    },
    { SC_OP_SET_POKE_COUNTER,   scProc_OP_SetPokeCounter  },
    { SC_OP_BATONTOUCH,         scProc_OP_BatonTouch      },
    { SC_OP_MIGAWARI_CREATE,    scProc_OP_MigawariCreate  },
    { SC_OP_MIGAWARI_DELETE,    scProc_OP_MigawariDelete  },
    { SC_OP_SHOOTER_CHARGE,     scProc_OP_ShooterCharge   },
    { SC_OP_SET_ILLUSION,       scProc_OP_SetFakeSrc      },
    { SC_OP_CLEAR_CONSUMED_ITEM,scProc_OP_ClearConsumedItem },
    { SC_OP_CURESICK_DEPEND_POKE,scProc_OP_CureSickDependPoke},
    { SC_OP_WAZADMG_REC,        scProc_OP_AddWazaDamage   },
    { SC_OP_TURN_CHECK,         scProc_OP_TurnCheck       },
    { SC_ACT_KILL,              scProc_ACT_Kill           },
    { SC_ACT_MOVE,              scProc_ACT_Move           },
    { SC_ACT_EXP,               scProc_ACT_Exp            },
    { SC_ACT_BALL_THROW,        scProc_ACT_BallThrow      },
    { SC_ACT_BALL_THROW_TR,     scProc_ACT_BallThrowTrainer },
    { SC_ACT_ROTATION,          scProc_ACT_Rotation       },
    { SC_ACT_CHANGE_TOKUSEI,    scProc_ACT_ChangeTokusei  },
    { SC_ACT_SWAP_TOKUSEI,      scProc_ACT_SwapTokusei    },
    { SC_ACT_FAKE_DISABLE,      scProc_ACT_FakeDisable    },
    { SC_ACT_EFFECT_SIMPLE,     scProc_ACT_EffectSimple   },
    { SC_ACT_EFFECT_BYPOS,      scProc_ACT_EffectByPos    },
    { SC_ACT_EFFECT_BYVECTOR,   scProc_ACT_EffectByVector },
    { SC_ACT_CHANGE_FORM,       scProc_ACT_ChangeForm     },
    { SC_ACT_RESET_MOVE,        scProc_ACT_ResetMove      },
    { SC_ACT_MIGAWARI_CREATE,   scProc_ACT_MigawariCreate },
    { SC_ACT_MIGAWARI_DELETE,   scProc_ACT_MigawariDelete },
    { SC_ACT_HENSIN,            scProc_ACT_Hensin         },
    { SC_ACT_MIGAWARI_DAMAGE,   scProc_ACT_MigawariDamage },
    { SC_ACT_WIN_BGM,           scProc_ACT_PlayBGM        },
    { SC_ACT_MSGWIN_HIDE,       scProc_ACT_MsgWinHide     },
  };

restart:

  switch( *seq ){
  case 0:
    {
      u32 cmdSize;
      const void* cmdBuf;

      cmdSize = BTL_ADAPTER_GetRecvData( wk->adapter, &cmdBuf );

      if( (wk->cmdCheckServer != NULL)
      &&  (wk->cmdCheckTimingCode != BTL_RECTIMING_None)
      ){
        if( BTL_SERVER_CMDCHECK_Make(wk->cmdCheckServer, wk->cmdCheckTimingCode, cmdBuf, cmdSize) ){
          BTL_MAIN_NotifyCmdCheckError( wk->mainModule );
          OS_TPrintf("!!!! recvedCmd=%p, %02x, %02x, %02x, ...\n", cmdBuf,
              ((const u8*)cmdBuf)[0], ((const u8*)cmdBuf)[1], ((const u8*)cmdBuf)[2] );
        }
        wk->cmdCheckTimingCode = BTL_RECTIMING_None;
      }

      SCQUE_Setup( wk->cmdQue, cmdBuf, cmdSize );

      if( wk->commWaitInfoOn )
      {
        wk->commWaitInfoOn = FALSE;
        BTLV_ResetCommWaitInfo( wk->viewCore );
      }
      (*seq)++;
    }
    /* fallthru */
  case 1:
    if( SCQUE_IsFinishRead(wk->cmdQue) )
    {
      BTL_Printf("サーバコマンド読み終わり\n");
      (*seq) = 4;
      return TRUE;
    }
    // 録画再生処理チェック
    else if( RecPlayer_CheckBlackOut(&wk->recPlayer) )
    {
      BTL_N_Printf( DBGSTR_CLIENT_RecPlayerBlackOut );
      return TRUE;
    }
    (*seq)++;
    /* fallthru */
  case 2:
    {
      int i;

      wk->serverCmd = SCQUE_Read( wk->cmdQue, wk->cmdArgs );

      for(i=0; i<NELEMS(scprocTbl); ++i)
      {
        if( scprocTbl[i].cmd == wk->serverCmd ){ break; }
      }

      if( i == NELEMS(scprocTbl) )
      {
        BTL_N_Printf( DBGSTR_CLIENT_UnknownServerCmd, wk->serverCmd);
        (*seq) = 1;
        goto restart;
      }

      BTL_Printf( "serverCmd=%d\n", wk->serverCmd );
      wk->scProc = scprocTbl[i].proc;
      wk->scSeq = 0;
      (*seq)++;
    }
    /* fallthru */
  case 3:
    if( wk->scProc(wk, &wk->scSeq, wk->cmdArgs) )
    {
      (*seq) = 1;
      goto restart;
    }
    break;

  case 4:
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * メンバー引き上げメッセージ表示
 *
 * @param   wk
 * @param   seq
 * @param   args    [0]:ClientID, [1]:pokeID
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL scProc_ACT_MemberOutMsg( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BOOL fClientArg;
      u16  strID;
      strID = CheckMemberOutStrID( wk, args[0], &fClientArg );

      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, strID );
      if( fClientArg ){
        BTLV_STRPARAM_AddArg( &wk->strParam, args[0] );
      }
      BTLV_STRPARAM_AddArg( &wk->strParam, args[1] );

      BTLV_StartMsg( wk->viewCore, &wk->strParam );
      ++(*seq);
    }
    break;

  case 1:
    if( BTLV_WaitMsg(wk->viewCore) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * メンバー引き上げメッセージ（●●　もどれ！など）のIDを取得
 *
 * @param   wk
 * @param   clientID
 * @param   fClientArg    [out] 文字列の引数としてクライアントIDを使うケースでTRUEが入る
 *
 * @retval  u16   文字列ID
 */
//----------------------------------------------------------------------------------
static u16 CheckMemberOutStrID( BTL_CLIENT* wk, u8 clientID, BOOL* fClientArg )
{
  // 自分が対象
  if( clientID == wk->myID )
  {
    *fClientArg = FALSE;

    // シングル＆非通信なら相手に与えたダメージ量に応じてメッセージを変える
    if( (BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_SINGLE)
    &&  (BTL_MAIN_GetCommMode(wk->mainModule) == BTL_COMM_NONE)
    ){
      u32 dmg_per = EnemyPokeHPBase_CheckRatio( wk );
      if( dmg_per >= 75 ){
        return BTL_STRID_STD_BackChange5;
      }
      if( dmg_per > 50 ){
        return BTL_STRID_STD_BackChange4;
      }
      if( dmg_per > 25 ){
        return BTL_STRID_STD_BackChange3;
      }
      if( dmg_per > 0 ){
        return BTL_STRID_STD_BackChange2;
      }
      return BTL_STRID_STD_BackChange1;
    }
    else
    {
      return BTL_STRID_STD_BackChange2;
    }
  }
  // 自分以外が対象
  else
  {
    *fClientArg = TRUE;

    if( BTL_MAIN_IsClientNPC(wk->mainModule, clientID) ){
      return BTL_STRID_STD_BackChange_NPC;
    }else{
      return BTL_STRID_STD_BackChange_Player;
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * メンバー退場アクション
 *
 * args[0]: pos  args[1]: effectNo
 */
//----------------------------------------------------------------------------------
static BOOL scProc_ACT_MemberOut( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlvMcssPos  vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, args[0] );

      BTLV_ACT_MemberOut_Start( wk->viewCore,  vpos, args[1] );
      (*seq)++;
    }
    break;

  case 1:
    if( BTLV_ACT_MemberOut_Wait(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * メンバー入場アクション
 *
 * @param   wk
 * @param   seq
 * @param   args
 *
 * @retval  BOOL
 */
//----------------------------------------------------------------------------------
static BOOL scProc_ACT_MemberIn( BTL_CLIENT* wk, int* seq, const int* args )
{
  u8 clientID  = args[0];
  u8 posIdx    = args[1];
  u8 memberIdx = args[2];
  u8 fPutMsg   = args[3];

  switch( *seq ){
  case 0:
    if( fPutMsg )
    {
      const BTL_POKEPARAM* bpp = BTL_POKECON_GetClientPokeDataConst( wk->pokeCon, clientID, posIdx );
      u8 pokeID = BPP_GetID( bpp );

      if( !BTL_MAIN_IsOpponentClientID(wk->mainModule, wk->myID, clientID) )
      {
        if( wk->myID == clientID )
        {
          u16 strID = CheckMemberPutStrID( wk );
          BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, strID );
          BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
        }
        else
        {
          BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_PutSingle_Player );
          BTLV_STRPARAM_AddArg( &wk->strParam, clientID );
          BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
        }
      }
      else
      {
        // 相手が入れ替え
//      if( BTL_MAIN_GetCompetitor(wk->mainModule) == BTL_COMPETITOR_TRAINER )
        if( BTL_MAIN_IsClientNPC(wk->mainModule, clientID) )
        {
          BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_PutSingle_NPC );
          BTLV_STRPARAM_AddArg( &wk->strParam, clientID );
          BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
        }
        else
        {
          BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_PutSingle_Player );
          BTLV_STRPARAM_AddArg( &wk->strParam, clientID );
          BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
        }
      }
      BTLV_StartMsg( wk->viewCore, &wk->strParam );
    }
    (*seq)++;
    break;

  case 1:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      BtlPokePos  pokePos = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx );

      BTL_Printf("メンバーIN ACT client=%d, posIdx=%d, pos=%d, memberIdx=%d\n",
          clientID, posIdx, pokePos, memberIdx );

      BTLV_StartMemberChangeAct( wk->viewCore, pokePos, clientID, posIdx );
      (*seq)++;
    }
    break;
  case 2:
    if( BTLV_WaitMemberChangeAct(wk->viewCore) )
    {
      EnemyPokeHPBase_Update( wk );
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * メンバー繰り出しメッセージ（ゆけっ！●●など）のIDを取得
 *
 * @param   wk
 *
 * @retval  u16
 */
//----------------------------------------------------------------------------------
static u16 CheckMemberPutStrID( BTL_CLIENT* wk )
{
  if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_SINGLE )
  {
    u8 opponentClientID = BTL_MAIN_GetOpponentClientID( wk->mainModule, wk->myID, 0 );
    const BTL_POKEPARAM* bpp = BTL_POKECON_GetClientPokeDataConst( wk->pokeCon, opponentClientID, 0 );

    if( BPP_IsDead(bpp) ){
      return BTL_STRID_STD_PutSingle;
    }
    else
    {
      fx32 hp_ratio = BPP_GetHPRatio( bpp );

      if( hp_ratio >= FX32_CONST(77.5) ){
        return BTL_STRID_STD_PutSingle;
      }
      if( hp_ratio >= FX32_CONST(55) ){
        return BTL_STRID_STD_PutChange1;
      }
      if( hp_ratio >= FX32_CONST(32.5) ){
        return BTL_STRID_STD_PutChange2;
      }
      if( hp_ratio >= FX32_CONST(10) ){
        return BTL_STRID_STD_PutChange3;
      }
      return BTL_STRID_STD_PutChange4;
    }
  }
  // シングル戦以外は共通
  return BTL_STRID_STD_PutSingle;
}

static BOOL scProc_MSG_Std( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    BTLV_StartMsgStd( wk->viewCore, args[0], &args[1] );
    (*seq)++;
    break;
  case 1:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      return TRUE;
    }
  }
  return FALSE;
}
static BOOL scProc_MSG_StdSE( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    BTLV_StartMsgStd( wk->viewCore, args[0], &args[2] );
    (*seq)++;
    break;
  case 1:
    if( BTLV_IsJustDoneMsg(wk->viewCore) ){
      PMSND_PlaySE( args[1] );
    }
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      return TRUE;
    }
  }
  return FALSE;
}


static BOOL scProc_MSG_Set( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    BTLV_StartMsgSet( wk->viewCore, args[0], &args[1] );
    (*seq)++;
    break;
  case 1:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL scProc_MSG_Waza( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    BTLV_StartMsgWaza( wk->viewCore, args[0], args[1] );
    (*seq)++;
    break;
  case 1:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 *  ワザエフェクト
 *  args  [0]:攻撃ポケ位置  [1]:防御ポケ位置  [2]:ワザID
 */
static BOOL scProc_ACT_WazaEffect( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ) {
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    {
      if( BTL_MAIN_IsWazaEffectEnable(wk->mainModule) )
      {
        WazaID waza;
        u8 atPokePos, defPokePos, turnType;
        const BTL_PARTY* party;
        const BTL_POKEPARAM* poke;

        atPokePos  = args[0];
        defPokePos = args[1];
        waza       = args[2];
        turnType   = args[3];

        BTLV_ACT_WazaEffect_Start( wk->viewCore, atPokePos, defPokePos, waza, turnType, 0 );
        (*seq)++;
      }
      else{
        return TRUE;
      }
    }
    break;

  case 1:
    if( BTLV_ACT_WazaEffect_Wait(wk->viewCore) ){
      return TRUE;
    }
    break;
  }

  return FALSE;
}
/**
 *  溜めワザ（そらをとぶ等）による、表示オン／オフ
 *  args  [0]:対象ポケID  [1]:TRUE=非表示, FALSE=表示
 */
static BOOL scProc_ACT_TameWazaHide( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ) {
  case 0:
    {
      u8 pokeID = args[0];
      BtlvMcssPos vpos = BTL_MAIN_PokeIDtoViewPos( wk->mainModule, wk->pokeCon, pokeID );

      BTLV_MCSS_VANISH_FLAG  flag = (args[1])? BTLV_MCSS_VANISH_ON : BTLV_MCSS_VANISH_OFF;
      BTLV_ACT_TameWazaHide( wk->viewCore, vpos, flag );
      (*seq)++;
    }
    break;

  case 1:
    return TRUE;
  }
  return FALSE;
}
/**
 * 【アクション】単体ダメージ演出  [0]:pokeID, [1]:affAbout [2]:wazaID
 */
static BOOL scProc_ACT_WazaDmg( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ) {
  case 0:
  {
    WazaID waza;
    u8 atPokePos, defPokePos, affAbout;
    u16 damage;
    const BTL_PARTY* party;
    const BTL_POKEPARAM* poke;

    defPokePos  = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
    affAbout  = args[1];
    waza      = args[2];

    BTLV_ACT_DamageEffectSingle_Start( wk->viewCore, waza, defPokePos, affAbout );
    (*seq)++;
  }
  break;

  case 1:
    if( BTLV_ACT_DamageEffectSingle_Wait(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
/**
 * 【アクション】複数体一斉ダメージ処理
 */
static BOOL scProc_ACT_WazaDmg_Plural( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ) {
  case 0:
  {
    u16               poke_cnt  = args[0];
    BtlTypeAffAbout   aff_about = args[1];
    WazaID            waza      = args[2];

    u8 pokeID[ BTL_POS_MAX ];
    u8 i;
    for(i=0; i<poke_cnt; ++i){
      pokeID[i] = SCQUE_READ_ArgOnly( wk->cmdQue );
    }
    BTLV_ACT_DamageEffectPlural_Start( wk->viewCore, poke_cnt, aff_about, pokeID, waza );
    (*seq)++;
  }
  break;

  case 1:
    if( BTLV_ACT_DamageEffectPlural_Wait(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
/**
 * 【アクション】一撃必殺ワザ処理
 */
static BOOL scProc_ACT_WazaIchigeki( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ) {
  case 0:
  {
    BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );

//    BTLV_EFFECT_Damage( BTL_MAIN_BtlPosToViewPos(wk->mainModule, pos), wazaID );

    BTLV_ACT_SimpleHPEffect_Start( wk->viewCore, pos );
    (*seq)++;
  }
  break;

  case 1:
    if( BTLV_ACT_SimpleHPEffect_Wait(wk->viewCore) )
    {
      BTLV_StartMsgStd( wk->viewCore, BTL_STRID_STD_Ichigeki, args );
      (*seq)++;
    }
    break;

  case 2:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
/**
 * 【アクション】状態異常アイコン  args[0]:pokeID  args[1]:状態異常コード
 */
static BOOL scProc_ACT_SickIcon( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ) {
  case 0:
  {
    BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
    BtlvMcssPos vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos );
    BTLV_EFFECT_SetGaugeStatus( args[1], vpos );
    (*seq)++;
  }
  break;

  case 1:
    return TRUE;
  }

  return FALSE;
}
/**
 * 【アクション】こんらん自爆ダメージ
 */
static BOOL scProc_ACT_ConfDamage( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ) {
  case 0:
    if( !BTL_CLIENT_IsChapterSkipMode(wk) )
    {
      BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
      BTLV_ACT_WazaEffect_Start( wk->viewCore, pos, pos, WAZANO_HATAKU, BTLV_WAZAEFF_INDEX_DEFAULT, 0 );
    }
    (*seq)++;
    break;
  case 1:
    if( BTLV_ACT_WazaEffect_Wait(wk->viewCore) )
    {
      BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );

      if( !BTL_CLIENT_IsChapterSkipMode(wk) ){
        PMSND_PlaySE( SEQ_SE_KOUKA_M );
      }

      BTLV_ACT_DamageEffectSingle_Start( wk->viewCore, WAZANO_HATAKU, pos, BTL_TYPEAFF_ABOUT_NORMAL );
//      BTLV_ACT_SimpleHPEffect_Start( wk->viewCore, pos );
      (*seq)++;
    }
    break;
  case 2:
//    if( BTLV_ACT_SimpleHPEffect_Wait(wk->viewCore) )
    if( BTLV_ACT_DamageEffectSingle_Wait(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
/**
 * 【アクション】ポケモンひんし
 */
static BOOL scProc_ACT_Dead( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
      BTLV_StartDeadAct( wk->viewCore, pos );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_WaitDeadAct(wk->viewCore) )
    {
      BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
      BPP_Clear_ForDead( bpp );
      return TRUE;
    }
    break;
  }
  return FALSE;
}
/**
 * 【アクション】ポケモン生き返り
 */
static BOOL scProc_ACT_Relive( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
      BTLV_StartReliveAct( wk->viewCore, pos );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_WaitReliveAct(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}

/**
 * 【アクション】能力ランクダウン
 */
static BOOL scProc_ACT_RankDown( BTL_CLIENT* wk, int* seq, const int* args )
{
  BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
  u8 vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule,  pos );

  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    BTLV_StartRankDownEffect( wk->viewCore, vpos );
    (*seq)++;
    break;
  case 1:
    if( BTLV_WaitRankEffect(wk->viewCore, vpos) ){
      return TRUE;
    }
  }
  return FALSE;
}
/**
 * 【アクション】能力ランクアップ
 */
static BOOL scProc_ACT_RankUp( BTL_CLIENT* wk, int* seq, const int* args )
{
  BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
  u8 vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos );

  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    BTLV_StartRankUpEffect( wk->viewCore, vpos );
    (*seq)++;
    break;
  case 1:
    if( BTLV_WaitRankEffect(wk->viewCore, vpos) ){
      return TRUE;
    }
  }
  return FALSE;
}
/**
 *  【アクション】天候による一斉ダメージ処理
 */
static BOOL scProc_ACT_WeatherDmg( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlWeather weather = args[0];
      u8 pokeCnt = args[1];
      u16 msgID;
      u8 pokeID, pokePos, i;

      switch( weather ){
      case BTL_WEATHER_SAND:
        BTLV_StartMsgStd( wk->viewCore, BTL_STRID_STD_SandAttack, NULL );
        break;
      case BTL_WEATHER_SNOW:
        BTLV_StartMsgStd( wk->viewCore, BTL_STRID_STD_SnowAttack, NULL );
        break;
      default:
        break;
      }

      for(i=0; i<pokeCnt; ++i)
      {
        pokeID = SCQUE_READ_ArgOnly( wk->cmdQue );
        pokePos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, pokeID );
        BTLV_ACT_SimpleHPEffect_Start( wk->viewCore, pokePos );
      }
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_WaitMsg(wk->viewCore)
    &&  BTLV_ACT_SimpleHPEffect_Wait(wk->viewCore)
    ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  天候変化開始
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_WeatherStart( BTL_CLIENT* wk, int* seq, const int* args )
{
  static const struct {
    u16 strID;
    u16 effectID;
  }ParamTbl[] = {
    { BTL_STRID_STD_ShineStart,   BTLEFF_WEATHER_HARE       },
    { BTL_STRID_STD_ShineStart,   BTLEFF_WEATHER_HARE       },
    { BTL_STRID_STD_RainStart,    BTLEFF_WEATHER_AME        },
    { BTL_STRID_STD_SnowStart,    BTLEFF_WEATHER_ARARE      },
    { BTL_STRID_STD_StormStart,   BTLEFF_WEATHER_SUNAARASHI },
  };

  BtlWeather weather = args[0];

  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    if( weather < NELEMS(ParamTbl) )
    {
      BTLV_EFFECT_Add( ParamTbl[weather].effectID );
    }
    (*seq)++;
    break;

  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTLV_StartMsgStd( wk->viewCore, ParamTbl[weather].strID, NULL );
      (*seq)++;
    }
    break;

  case 2:
    if( BTLV_WaitMsg(wk->viewCore) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  ターンチェックによる天候の終了処理
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_WeatherEnd( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlWeather weather = args[0];
      u16  msgID;
      switch( weather ){
      case BTL_WEATHER_SHINE: msgID = BTL_STRID_STD_ShineEnd; break;
      case BTL_WEATHER_RAIN:  msgID = BTL_STRID_STD_RainEnd; break;
      case BTL_WEATHER_SAND:  msgID = BTL_STRID_STD_StormEnd; break;
      case BTL_WEATHER_SNOW:  msgID = BTL_STRID_STD_SnowEnd; break;
//      case BTL_WEATHER_MIST:  msgID = BTL_STRID_STD_MistEnd; break;
      default:
        GF_ASSERT(0); // おかしな天候ID
        return TRUE;
      }
      BTLV_StartMsgStd( wk->viewCore, msgID, NULL );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  シンプルなHPゲージ増減処理
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_SimpleHP( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
      TAYA_Printf("PokeID=%d, Pos=%d\n", args[0], pos);
      BTLV_ACT_SimpleHPEffect_Start( wk->viewCore, pos );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_ACT_SimpleHPEffect_Wait(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  装備アイテム使用エフェクト
 *  args .. [0]:対象ポケID
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_Kinomi( BTL_CLIENT* wk, int* seq, const int* args )
{
  u8 pokeID = args[0];
  BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, pokeID );

  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }
    else
    {
      BTLV_KinomiAct_Start( wk->viewCore, pos );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_KinomiAct_Wait( wk->viewCore, pos ) )
    {
      return TRUE;
    }
    break;
  }

  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  強制ひんし演出
 *  args .. [0]:対象ポケID  [1]:エフェクトタイプ
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_Kill( BTL_CLIENT* wk, int* seq, const int* args )
{
  // @@@ 今は単純なHP減エフェクトで代用
  u8 pokeID = args[0];
  u8 effType = args[1];

  switch( *seq ){
  case 0:
    {
      BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, pokeID );
      BTLV_ACT_SimpleHPEffect_Start( wk->viewCore, pos );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_ACT_SimpleHPEffect_Wait(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  ムーブ処理
 *  args .. [0]:対象クライアントID  [1]:対象ポケモン位置1  [2]:対象ポケモン位置2
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_Move( BTL_CLIENT* wk, int* seq, const int* args )
{

  switch( *seq ){
  case 0:
    {
      u8 clientID = args[0];
      u8 pos1 = args[1];
      u8 pos2 = args[2];
      u8 posIdx1 = BTL_MAIN_BtlPosToPosIdx( wk->mainModule, pos1 );
      u8 posIdx2 = BTL_MAIN_BtlPosToPosIdx( wk->mainModule, pos2 );
      u8 vpos1 = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos1 );
      u8 vpos2 = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos2 );

      BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
      BTL_PARTY_SwapMembers( party, posIdx1, posIdx2 );

      BTL_N_Printf( DBGSTR_CLIENT_MoveAction, pos1, pos2, vpos1, vpos2 );

      BTLV_ACT_MoveMember_Start( wk->viewCore, clientID, vpos1, vpos2, posIdx1, posIdx2 );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_ACT_MoveMember_Wait(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  トリプルバトル・リセットムーブ動作
 *  args .. [0]:clientID_1, [1]:posIdx_1, [2]:clientID_2, [3]:posIdx_2
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_ResetMove( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
//    BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_ResetMove );
//    BTLV_StartMsg( wk->viewCore, &wk->strParam );
    BTLV_EFFECT_Add( BTLEFF_RESET_MOVE );
    (*seq)++;
    break;
  case 1:
//    if( BTLV_WaitMsg(wk->viewCore) )
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u8 clientID = args[0];
      u8 posIdx1 = args[1];
      u8 posIdx2 = 1;
      u8 pos1  = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx1 );
      u8 pos2  = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx2 );
      u8 vpos1 = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos1 );
      u8 vpos2 = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos2 );

      BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
      BTL_PARTY_SwapMembers( party, posIdx1, posIdx2 );

      BTLV_ACT_MoveMember_Start( wk->viewCore, clientID, vpos1, vpos2, posIdx1, posIdx2 );
      (*seq)++;
    }
    break;
  case 2:
    if( BTLV_ACT_MoveMember_Wait(wk->viewCore) )
    {
      u8 clientID = args[2];
      u8 posIdx1 = args[3];
      u8 posIdx2 = 1;
      u8 pos1  = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx1 );
      u8 pos2  = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx2 );
      u8 vpos1 = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos1 );
      u8 vpos2 = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos2 );

      BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
      BTL_PARTY_SwapMembers( party, posIdx1, posIdx2 );

      BTLV_ACT_MoveMember_Start( wk->viewCore, clientID, vpos1, vpos2, posIdx1, posIdx2 );
      (*seq)++;
    }
    break;
  case 3:
    if( BTLV_ACT_MoveMember_Wait(wk->viewCore) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  みがわり作成
 *  args .. [0]:pos
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_MigawariCreate( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlvMcssPos  vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, args[0] );
      BTLV_EFFECT_CreateMigawari( vpos );
      (*seq)++;
    }
    break;
  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      return TRUE;
    }
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  みがわり削除
 *  args .. [0]:pos
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_MigawariDelete( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlvMcssPos  vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, args[0] );
      BTLV_EFFECT_DeleteMigawari( vpos );
      (*seq)++;
    }
    break;
  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * へんしん
 *
 * args ..[0]: へんしんするポケID   [1]: へんしん対象のポケID
 */
//----------------------------------------------------------------------------------
static BOOL scProc_ACT_Hensin( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BTL_POKEPARAM* atkPoke = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
      BTL_POKEPARAM* tgtPoke = BTL_POKECON_GetPokeParam( wk->pokeCon, args[1] );
      BtlvMcssPos  atkVpos = BTL_MAIN_PokeIDtoViewPos( wk->mainModule, wk->pokeCon, args[0] );
      BtlvMcssPos  tgtVpos = BTL_MAIN_PokeIDtoViewPos( wk->mainModule, wk->pokeCon, args[1] );

//      TAYA_Printf("PokeSick=%d, line=%d\n", BPP_GetPokeSick(atkPoke), __LINE__);
      BPP_HENSIN_Set( atkPoke, tgtPoke );
//      TAYA_Printf("PokeSick=%d, line=%d\n", BPP_GetPokeSick(atkPoke), __LINE__);


      BTLV_Hensin_Start( wk->viewCore, atkVpos, tgtVpos );
      (*seq)++;
    }
    break;

  case 1:
    if( BTLV_Hensin_Wait(wk->viewCore) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * みがわりダメージ
 *
 * args ..[0]: みがわりを出しているポケID   [1]: ダメージ相性  [2]:ワザID
 */
//----------------------------------------------------------------------------------
static BOOL scProc_ACT_MigawariDamage( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlPokePos  tgtPos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
      BtlTypeAffAbout  affAbout = BTL_CALC_TypeAffAbout( args[1] );

      BTLV_ACT_MigawariDamageEffect_Start( wk->viewCore, args[2], tgtPos, affAbout );
      (*seq)++;
    }
    break;

  case 1:
    if( BTLV_ACT_MigawariDamageEffect_Wait(wk->viewCore) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}


//---------------------------------------------------------------------------------------
/**
 *  PlayBGM
 *  args .. [0]:BGMナンバー
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_PlayBGM( BTL_CLIENT* wk, int* seq, const int* args )
{
  PMSND_PlayBGM( args[0] );
  return TRUE;
}
//---------------------------------------------------------------------------------------
/**
 *  MsgWindow フェードアウト
 *  args .. [0]:dummy
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_MsgWinHide( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    BTLV_MsgWinHide_Start( wk->viewCore );
    (*seq)++;
    break;
  case 1:
    if( BTLV_MsgWinHide_Wait(wk->viewCore) ){
      return TRUE;
    }
  }
  return FALSE;
}

//---------------------------------------------------------------------------------------
/**
 *  経験値加算処理
 *  args .. [0]:対象ポケモンID  [1]:経験値加算分
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_Exp( BTL_CLIENT* wk, int* seq, const int* args )
{
  enum {
    SEQ_INIT = 0,
    SEQ_ADD_ROOT,
    SEQ_GAUGE_NORMAL_WAIT,
    SEQ_LVUP_ROOT,
    SEQ_LVUP_GAUGE_WAIT,
    SEQ_LVUP_EFFECT_WAIT,
    SEQ_LVUP_INFO_START,
    SEQ_LVUP_INFO_MSG_WAIT,
    SEQ_LVUP_INFO_PARAM_START,
    SEQ_LVUP_INFO_PARAM_SEQ1,
    SEQ_LVUP_INFO_PARAM_SEQ2,
    SEQ_LVUP_INFO_PARAM_END,
    SEQ_LVUP_WAZAOBOE_WAIT,
    SEQ_END,
  };

  static BTL_LEVELUP_INFO  lvupInfo;
  static u32  addExp;
  static int  subSeq;

  u8 pokeID = args[0];
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
  BtlvMcssPos vpos = BTL_MAIN_PokeIDtoViewPos( wk->mainModule, wk->pokeCon, pokeID );
  BOOL fGaugeExist = ((vpos != BTLV_MCSS_POS_ERROR) && BTLV_EFFECT_CheckExistGauge(vpos) );

  switch( *seq ){
  case SEQ_INIT:
    addExp = args[1];
    subSeq = 0;
    (*seq) = SEQ_ADD_ROOT;
    /* fallthru */
  case SEQ_ADD_ROOT:
    if( addExp )
    {
      u32 expRest = addExp;

      if( BPP_AddExp(bpp, &expRest, &lvupInfo) )
      {
        (*seq) = SEQ_LVUP_ROOT;
      }
      else
      {
        if( fGaugeExist ){
          BTLV_EFFECT_CalcGaugeEXP( vpos, addExp );
          (*seq) = SEQ_GAUGE_NORMAL_WAIT;
        }
        else {
          (*seq) = SEQ_END;
        }
        expRest = 0;
      }
      addExp = expRest;
    }
    else
    {
      (*seq) = SEQ_END;
    }
    break;

  // 通常の（レベルアップしない）ゲージ演出の終了待ち
  case SEQ_GAUGE_NORMAL_WAIT:
    if( !BTLV_EFFECT_CheckExecuteGauge() ){
      (*seq) = SEQ_END;
    }
    break;

  // レベルアップ処理ルート
  case SEQ_LVUP_ROOT:
    BTL_N_Printf( DBGSTR_CLIENT_HPCheckByLvup, __LINE__, BPP_GetID(bpp), BPP_GetValue(bpp,BPP_HP));
    BPP_ReflectByPP( bpp );
    BTL_N_Printf( DBGSTR_CLIENT_HPCheckByLvup, __LINE__, BPP_GetID(bpp), BPP_GetValue(bpp,BPP_HP));
    {
       // 場に出ているならゲージ演出
       if( fGaugeExist )
       {
         BTLV_EFFECT_CalcGaugeEXPLevelUp( vpos, bpp );
         (*seq) = SEQ_LVUP_GAUGE_WAIT;
       }
       // 出ていないなら能力上昇表示までジャンプ
       else{
         (*seq) = SEQ_LVUP_INFO_START;
       }
    }
    break;

  case SEQ_LVUP_GAUGE_WAIT:
    if( (!BTLV_EFFECT_CheckExecuteGauge())
    &&  (!PMSND_CheckPlaySE())
    ){
      BTLV_AddEffectByPos( wk->viewCore, vpos, BTLEFF_LVUP );
      (*seq) = SEQ_LVUP_EFFECT_WAIT;
    }
    break;
  case SEQ_LVUP_EFFECT_WAIT:
    if( BTLV_WaitEffectByPos(wk->viewCore) )
    {
      (*seq) = SEQ_LVUP_INFO_START;
    }
    break;

  case SEQ_LVUP_INFO_START:
    BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_LevelUp );
    BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
    BTLV_STRPARAM_AddArg( &wk->strParam, lvupInfo.level );
    BTLV_StartMsg( wk->viewCore, &wk->strParam );
    (*seq) = SEQ_LVUP_INFO_MSG_WAIT;
    break;

  case SEQ_LVUP_INFO_MSG_WAIT:
    if( BTLV_IsJustDoneMsg(wk->viewCore) ){
      PMSND_PauseBGM( TRUE );
      PMSND_PushBGM();
      PMSND_PlayBGM( SEQ_ME_LVUP );
    }
    if( BTLV_WaitMsg(wk->viewCore) && !PMSND_CheckPlayBGM() ){
      PMSND_PopBGM();
      PMSND_PauseBGM( FALSE );
      subSeq = 0;
      (*seq) = SEQ_LVUP_INFO_PARAM_START;
    }
    break;

  case SEQ_LVUP_INFO_PARAM_START:
    ++subSeq;
    if( (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B))
    ||  (GFL_UI_TP_GetTrg())
    ||  (subSeq > 80)
    ){
      subSeq = 0;
      BTL_MAIN_NotifyPokemonLevelup( wk->mainModule, bpp );
      BTLV_LvupWin_StartDisp( wk->viewCore, bpp, &lvupInfo );
      (*seq) = SEQ_LVUP_INFO_PARAM_SEQ1;
    }
    break;

  case SEQ_LVUP_INFO_PARAM_SEQ1:
    if( BTLV_LvupWin_WaitDisp(wk->viewCore) )
    {
      if( (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B))
      ||  (GFL_UI_TP_GetTrg())
      ){
        BTLV_LvupWin_StepFwd( wk->viewCore );
        (*seq) = SEQ_LVUP_INFO_PARAM_SEQ2;
      }
    }
    break;

  case SEQ_LVUP_INFO_PARAM_SEQ2:
    if( BTLV_LvupWin_WaitFwd(wk->viewCore) )
    {
      if( (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B))
      ||  (GFL_UI_TP_GetTrg())
      ){
        BTLV_LvupWin_StartHide( wk->viewCore );
        (*seq) = SEQ_LVUP_INFO_PARAM_END;
      }
    }
    break;

  case SEQ_LVUP_INFO_PARAM_END:
    if( BTLV_LvupWin_WaitHide(wk->viewCore) ){
      subSeq = 0;
      (*seq) = SEQ_LVUP_WAZAOBOE_WAIT;
    }
    break;

  case SEQ_LVUP_WAZAOBOE_WAIT:
    if( wazaOboeSeq(wk, &subSeq, bpp) ){
      (*seq) = SEQ_ADD_ROOT;
    }
    break;

  case SEQ_END:
    BTL_MAIN_ReflectPokeWazaOboe( wk->mainModule, pokeID );
    return TRUE;

  }
  return FALSE;
}
/**
 *  １、２のポカンSE管理コールバック
 */
static BOOL msgPokanCallback( u32 arg )
{
  switch( arg ){
  case 3: PMSND_PlaySE( SEQ_SE_KON ); break;
  case 5:
    if( PMSND_CheckPlayingSEIdx(SEQ_SE_KON) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}
/**
 * ワザ覚え処理
 *  （経験値加算処理 scProc_ACT_Exp のサブシーケンス）
 */
static BOOL wazaOboeSeq( BTL_CLIENT* wk, int* seq, BTL_POKEPARAM* bpp )
{
  enum {
    SEQ_INIT = 0,
    SEQ_CHECK_ROOT,
    SEQ_OBOETA_MSG_START,
    SEQ_OBOETA_MSG_WAIT,
    SEQ_OBOETA_ME_WAIT,

    SEQ_WASURE_ROOT,
    SEQ_WASURE_WAIT_MSG_1ST,
    SEQ_WASURE_YESNO_WAIT,

    SEQ_WASURE_SELECT,
    SEQ_WASURE_DECIDE,

    SEQ_AKIRAME_ROOT,
    SEQ_AKIRAME_YESNO_START,
    SEQ_AKIRAME_YESNO_WAIT,
    SEQ_AKIRAME_END,
  };

  static int wazaoboe_index;
  static WazaID wazaoboe_no;

  u8 pokeID = BPP_GetID( bpp );

  switch( *seq ){
  case SEQ_INIT:
    wazaoboe_index = 0;
    wazaoboe_no = 0;
    (*seq) = SEQ_CHECK_ROOT;
    /* fallthru */

  case SEQ_CHECK_ROOT:
    //技覚えチェック
    {
      const POKEMON_PARAM* pp = BPP_GetSrcData( bpp );
      wazaoboe_no = PP_CheckWazaOboe( ( POKEMON_PARAM* )pp, &wazaoboe_index, wk->heapID );
      if( wazaoboe_no == PTL_WAZAOBOE_NONE )
      {
        return TRUE;
      }
      else if( wazaoboe_no == PTL_WAZASET_SAME )
      {
        break;
      }
      else if( wazaoboe_no & PTL_WAZAOBOE_FULL )
      {
        wazaoboe_no &= ( PTL_WAZAOBOE_FULL ^ 0xffff );
        (*seq) = SEQ_WASURE_ROOT;
      }
      else
      {
        (*seq) = SEQ_OBOETA_MSG_START;
      }
    }
    break;
  case SEQ_OBOETA_MSG_START:
    {
      BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
      const POKEMON_PARAM* pp = BPP_GetSrcData( bpp );
      BPP_ReflectByPP( bpp );
      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_WAZAOBOE, msg_waza_oboe_04 );
      BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
      BTLV_STRPARAM_AddArg( &wk->strParam, wazaoboe_no );
      BTLV_StartMsg( wk->viewCore, &wk->strParam );
      (*seq) = SEQ_OBOETA_MSG_WAIT;
    }
    /* fallthru */
  case SEQ_OBOETA_MSG_WAIT:
    //技覚え処理
    if( BTLV_IsJustDoneMsg(wk->viewCore) ){
      PMSND_PauseBGM( TRUE );
      PMSND_PushBGM();
      PMSND_PlayBGM( SEQ_ME_LVUP );
    }
    if( BTLV_WaitMsg( wk->viewCore ) ){
      (*seq) = SEQ_OBOETA_ME_WAIT;
    }
    break;
  case SEQ_OBOETA_ME_WAIT:
    if( !PMSND_CheckPlayBGM() ){
      PMSND_PopBGM();
      PMSND_PauseBGM( FALSE );
      (*seq) = SEQ_CHECK_ROOT;
    }
    break;

  case SEQ_WASURE_ROOT:
    //技忘れ確認処理「○○覚えるために他のワザを忘れさせますか？」
    BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_WAZAOBOE, msg_waza_oboe_05 );
    BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
    BTLV_STRPARAM_AddArg( &wk->strParam, wazaoboe_no );
    BTLV_StartMsg( wk->viewCore, &wk->strParam );
    (*seq) = SEQ_WASURE_WAIT_MSG_1ST;
    /* fallthru */
  case SEQ_WASURE_WAIT_MSG_1ST:
    if( BTLV_IsJustDoneMsg(wk->viewCore) )
    {
      BTLV_STRPARAM   yesParam;
      BTLV_STRPARAM   noParam;
      BTLV_STRPARAM_Setup( &yesParam, BTL_STRTYPE_YESNO, msgid_yesno_wazawasureru );
      BTLV_STRPARAM_Setup( &noParam, BTL_STRTYPE_YESNO, msgid_yesno_wazawasurenai );
      BTLV_YESNO_Start( wk->viewCore, &yesParam, &noParam, TRUE );
    }
    if( BTLV_WaitMsg(wk->viewCore) ){
      (*seq) = SEQ_WASURE_YESNO_WAIT;
    }
    break;
  case SEQ_WASURE_YESNO_WAIT:
    {
      BtlYesNo result;

      if( BTLV_YESNO_Wait( wk->viewCore, &result ) )
      {
        if( result == BTL_YESNO_YES )
        {
          const BTL_PARTY* party = BTL_POKECON_GetPartyDataConst( wk->pokeCon, wk->myID );
          int index = BTL_PARTY_FindMemberByPokeID( party, pokeID );
          BTLV_WAZAWASURE_Start( wk->viewCore, index, wazaoboe_no );
          (*seq) = SEQ_WASURE_SELECT;
        }
        else
        {
          (*seq) = SEQ_AKIRAME_ROOT;
        }
      }
    }
    break;
  case SEQ_WASURE_SELECT:
    //技忘れ選択処理
    {
      u8 result;

      if( BTLV_WAZAWASURE_Wait( wk->viewCore, &result ) )
      {
        if( result == BPL_SEL_WP_CANCEL )
        {
          (*seq) = SEQ_WASURE_ROOT;
        }
        else
        {
          // １、２のポカン
          BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
          const POKEMON_PARAM* pp = BPP_GetSrcData( bpp );
          WazaID forget_wazano = PP_Get( pp, ID_PARA_waza1 + result, NULL );
          PP_SetWazaPos( ( POKEMON_PARAM* )pp, wazaoboe_no, result );
          BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_WAZAOBOE, msg_waza_oboe_06 );
          BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
          BTLV_STRPARAM_AddArg( &wk->strParam, forget_wazano );
          BTLV_StartMsgCallback( wk->viewCore, &wk->strParam, msgPokanCallback );
          (*seq) = SEQ_WASURE_DECIDE;
        }
      }
    }
    break;
  case SEQ_WASURE_DECIDE:
    //技忘れ処理
    if( BTLV_WaitMsg(wk->viewCore) ){
      (*seq) = SEQ_OBOETA_MSG_START;
    }
    break;

  case SEQ_AKIRAME_ROOT:
    BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_WAZAOBOE, msg_waza_oboe_08 );
    BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
    BTLV_STRPARAM_AddArg( &wk->strParam, wazaoboe_no );
    BTLV_StartMsg( wk->viewCore, &wk->strParam );
    (*seq) = SEQ_AKIRAME_YESNO_START;
    break;

  case SEQ_AKIRAME_YESNO_START:
    //技忘れあきらめ確認処理「では○○を覚えるのをあきらめますか？」
    if( BTLV_IsJustDoneMsg(wk->viewCore) ){
      BTLV_STRPARAM   yesParam;
      BTLV_STRPARAM   noParam;
      BTLV_STRPARAM_Setup( &yesParam, BTL_STRTYPE_YESNO, msgid_yesno_wazaakirameru );
      BTLV_STRPARAM_AddArg( &yesParam, wazaoboe_no );
      BTLV_STRPARAM_Setup( &noParam, BTL_STRTYPE_YESNO, msgid_yesno_wazaakiramenai );
      BTLV_STRPARAM_AddArg( &noParam, wazaoboe_no );
      BTLV_YESNO_Start( wk->viewCore, &yesParam, &noParam, TRUE );
    }
    if( BTLV_WaitMsg(wk->viewCore) ){
      (*seq) = SEQ_AKIRAME_YESNO_WAIT;
    }
    break;
  case SEQ_AKIRAME_YESNO_WAIT:
    {
      BtlYesNo result;
      if( BTLV_YESNO_Wait( wk->viewCore, &result ) )
      {
        // あきらめる
        if( result == BTL_YESNO_YES )
        {
          BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_WAZAOBOE, msg_waza_oboe_09 );
          BTLV_STRPARAM_AddArg( &wk->strParam, pokeID );
          BTLV_STRPARAM_AddArg( &wk->strParam, wazaoboe_no );
          BTLV_StartMsg( wk->viewCore, &wk->strParam );
          (*seq) = SEQ_AKIRAME_END;
        }
        // あきらめない
        else
        {
          (*seq) = SEQ_WASURE_ROOT;
        }
      }
    }
    break;
  case SEQ_AKIRAME_END:
    if( BTLV_WaitMsg(wk->viewCore) ){
      (*seq) = SEQ_CHECK_ROOT;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  モンスターボール投げつけ（野生）
 *  args .. [0]:対象ポケ位置  [1]:ゆれ回数  [2]:捕獲成功フラグ [3]: 図鑑登録フラグ [4]:クリティカルフラグ
 *           [5]:ボールのアイテムナンバー
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_BallThrow( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      u8 vpos;

      vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, args[0] );
      BTLV_EFFECT_BallThrow( vpos, args[5], args[1], args[2], args[4] );
      (*seq)++;
    }
    break;
  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      u16 strID;

      // 捕獲成功メッセージ
      if( args[2] )
      {
        const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, args[0] );
        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_BallThrowS );
        BTLV_STRPARAM_AddArg( &wk->strParam, BPP_GetID( bpp ) );
        //ピンチBGMが鳴っていたときは、戦闘BGMがPushされているのでチェックしてからSEQ_ME_POKEGETを鳴らす
        BTLV_EFFECT_SetBGMNoCheckPush( SEQ_ME_POKEGET );
      }
      // 捕獲失敗メッセージ
      else
      {
        static const u16 strTbl[] = {
          BTL_STRID_STD_BallThrow0, BTL_STRID_STD_BallThrow1, BTL_STRID_STD_BallThrow2, BTL_STRID_STD_BallThrow3,
        };

        if( args[1] < NELEMS(strTbl) ){
          strID = strTbl[ args[1] ];
        }else{
          strID = strTbl[ 0 ];
        }

        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, strID );
      }

      BTLV_StartMsg( wk->viewCore, &wk->strParam );
      (*seq)++;
    }
    break;
  case 2:
    if( BTLV_WaitMsg(wk->viewCore) )
    {
      if( args[2] )
      {
        const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, args[0] );
        BTL_MAIN_NotifyPokemonGetToGameSystem( wk->mainModule, bpp );
        (*seq)++;
      }else{
        return TRUE;
      }
    }
    break;
  case 3:
    if( !PMSND_CheckPlayBGM() )
    {
      //戦闘エフェクト一時停止を解除
      BTLV_EFFECT_Restart();
      PMSND_PlayBGM( SEQ_BGM_WIN1 );
      BTL_MAIN_BGMFadeOutDisable( wk->mainModule );
      // 図鑑登録メッセージ
      if( args[3] )
      {
        const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, args[0] );
        BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_ZukanRegister );
        BTLV_STRPARAM_AddArg( &wk->strParam, BPP_GetID( bpp ) );
        BTLV_StartMsg( wk->viewCore, &wk->strParam );
        (*seq)++;
      }
      else{
        return TRUE;
      }
    }
    break;
  case 4:
    if( BTLV_WaitMsg(wk->viewCore) ){
      (*seq)++;
    }
    break;
  default:
      return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  モンスターボール投げつけ（トレーナー戦）
 *  args .. [0]:対象ポケ位置  [1]:ボールのアイテムナンバー
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_BallThrowTrainer( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      u8 vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, args[0] );
      BTLV_EFFECT_BallThrowTrainer( vpos, args[1] );
      (*seq)++;
    }
    break;
  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTLV_STRPARAM_Setup( &wk->strParam, BTL_STRTYPE_STD, BTL_STRID_STD_BallThrowTrainer );
      BTLV_StartMsg( wk->viewCore, &wk->strParam );
      (*seq)++;
    }
    break;
  case 2:
    if( BTLV_WaitMsg(wk->viewCore) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//---------------------------------------------------------------------------------------
/**
 *  ローテーション動作
 *  args .. [0]:対象クライアントID  [1]:ローテーション方向 (BtlRotateDir)
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_Rotation( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      u8 clientID = args[0];
      BtlRotateDir dir = args[1];
      BTL_PARTY*  party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );

      BTL_N_Printf( DBGSTR_CLIENT_StartRotAct, clientID, dir );

      BTL_PARTY_RotateMembers( party, dir, NULL, NULL );
      BTLV_RotationMember_Start( wk->viewCore, clientID, dir );
      (*seq)++;
    }
    break;

  case 1:
    if( BTLV_RotationMember_Wait(wk->viewCore) )
    {
      BtlRotateDir dir = args[1];

      BTL_N_Printf( DBGSTR_CLIENT_EndRotAct, args[0], args[1] );
      if( dir == BTL_ROTATEDIR_STAY ){
        return TRUE;
      }
      else
      {
        u8  clientID = args[0];
        u8  fNPC = BTL_MAIN_IsClientNPC( wk->mainModule, clientID );
        u16 strID;

        if( dir == BTL_ROTATEDIR_L ){
          strID = (fNPC)? BTL_STRID_STD_RotateL_NPC : BTL_STRID_STD_RotateL_Player;
        }else{
          strID = (fNPC)? BTL_STRID_STD_RotateR_NPC : BTL_STRID_STD_RotateR_Player;
        }

        BTLV_StartMsgStd( wk->viewCore, strID, args );
        (*seq)++;
      }
    }
    break;

  case 2:
    if( BTLV_WaitMsg(wk->viewCore) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  とくせい書き換えアクション
 *  args .. [0]:対象のポケモンID [1]:書き換え後のとくせい
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_ChangeTokusei( BTL_CLIENT* wk, int* seq, const int* args )
{
  u8 pokeID = args[0];
  BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, pokeID );

  switch( *seq ){
  case 0:
    {
      BTLV_TokWin_DispStart( wk->viewCore, pos, FALSE );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_TokWin_DispWait( wk->viewCore, pos ) )
    {
      BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
      BPP_ChangeTokusei( bpp, args[1] );
      BTLV_TokWin_Renew_Start( wk->viewCore, pos );
      (*seq)++;
    }
    break;
  case 2:
    if( BTLV_TokWin_Renew_Wait( wk->viewCore, pos ) )
    {
      (*seq)++;
    }
    break;
  default:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  とくせい入れ替え演出  args [0]:pokeID_1, [1]:pokeID_2 [2]:tokID_1, [3]:tokID_2
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_SwapTokusei( BTL_CLIENT* wk, int* seq, const int* args )
{
  // 現状、描画側未実装、未使用。
  // 本来はbpp書き換え処理もここで行わないと難しい。
  static u32 timer = 0;

  u8 poke1_ID = args[0];
  u8 poke2_ID = args[1];
  u16 poke1_nextTokID = args[2];
  u16 poke2_nextTokID = args[3];

  BtlPokePos pos1 = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, poke1_ID );
  BtlPokePos pos2 = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, poke2_ID );

  switch( (*seq) ){
  case 0:
    if( BTL_MAINUTIL_IsFriendPokeID(poke1_ID, poke2_ID) )
    {
      BTL_POKEPARAM* bpp1 = BTL_POKECON_GetPokeParam( wk->pokeCon, poke1_ID );
      BTL_POKEPARAM* bpp2 = BTL_POKECON_GetPokeParam( wk->pokeCon, poke2_ID );
      BPP_ChangeTokusei( bpp1, poke1_nextTokID );
      BPP_ChangeTokusei( bpp2, poke2_nextTokID );
      return TRUE;
    }
    else
    {
      BTLV_TokWin_DispStart( wk->viewCore, pos1, FALSE );
      timer = 8;
      (*seq)++;
    }
    break;

  case 1:
    BTLV_TokWin_DispWait( wk->viewCore, pos1 );
    if( timer ){
      --timer;
    }else{
      BTLV_TokWin_DispStart( wk->viewCore, pos2, FALSE );
      (*seq)++;
    }
    break;

  case 2:
    {
      u8 wait1 = BTLV_TokWin_DispWait( wk->viewCore, pos1 ) ;
      u8 wait2 = BTLV_TokWin_DispWait( wk->viewCore, pos2 ) ;
      if( wait1 && wait2 ){
        (*seq)++;
      }
    }
    break;

  case 3:
    {
      BTL_POKEPARAM* bpp1 = BTL_POKECON_GetPokeParam( wk->pokeCon, poke1_ID );
      BTL_POKEPARAM* bpp2 = BTL_POKECON_GetPokeParam( wk->pokeCon, poke2_ID );
      BPP_ChangeTokusei( bpp1, poke1_nextTokID );
      BPP_ChangeTokusei( bpp2, poke2_nextTokID );
      timer = 0;
      (*seq)++;
    }
    break;

  case 4:
    if( ++timer > 8 ){
      BTLV_TokWin_Renew_Start( wk->viewCore, pos1 );
      (*seq)++;
    }
    break;

  case 5:
    if( BTLV_TokWin_Renew_Wait( wk->viewCore, pos1 ) ){
      BTLV_TokWin_Renew_Start( wk->viewCore, pos2 );
      (*seq)++;
    }
    break;

  case 6:
    if( BTLV_TokWin_Renew_Wait( wk->viewCore, pos2 ) ){
      (*seq)++;
    }
    break;

  default:
    return TRUE;
  }

  return FALSE;
}

//---------------------------------------------------------------------------------------
/**
 *  イリュージョン状態を解除
 *  args .. [0]:対象のポケモンID
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_FakeDisable( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      u8 pokeID = args[0];
      BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, pokeID );
      BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );

      BPP_FakeDisable( bpp );
      BTLV_FakeDisable_Start( wk->viewCore, pos );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_FakeDisable_Wait(wk->viewCore) )
    {
      (*seq)++;
    }
    break;
  default:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  位置指定なしのエフェクト発動
 *  args .. [0]:位置指定  [1]:エフェクトナンバー指定
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_EffectSimple( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }
    BTLV_EFFECT_Add( args[0] );
    (*seq)++;
    break;

  default:
    if( !BTLV_EFFECT_CheckExecute() ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  指定位置にエフェクト発動
 *  args .. [0]:位置指定  [1]:エフェクト指定
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_EffectByPos( BTL_CLIENT* wk, int* seq, const int* args )
{
  BtlvMcssPos vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, args[0] );

  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    BTLV_AddEffectByPos( wk->viewCore, vpos, args[1] );
    (*seq)++;
    break;

  default:
    if( BTLV_WaitEffectByPos( wk->viewCore) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  指定位置 -> 指定位置 にエフェクト発動
 *  args .. [0]:開始位置指定  [1]:終端位置指定  [2]:エフェクト指定
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_EffectByVector( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    {
      BtlvMcssPos vpos_from = BTL_MAIN_BtlPosToViewPos( wk->mainModule, args[0] );
      BtlvMcssPos vpos_to   = BTL_MAIN_BtlPosToViewPos( wk->mainModule, args[1] );

      BTLV_AddEffectByDir( wk->viewCore, vpos_from, vpos_to, args[2] );
      (*seq)++;
    }
    break;

  default:
    if( BTLV_WaitEffectByDir(wk->viewCore) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  指定ポケモンのフォルムナンバーチェンジ
 *  args .. [0]:pokeID  [1]:formNo
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_ACT_ChangeForm( BTL_CLIENT* wk, int* seq, const int* args )
{
  switch( *seq ){
  case 0:
    {
      BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
      BtlvMcssPos vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos );
      BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );

      BPP_ChangeForm( bpp, args[1] );
      BTLV_ChangeForm_Start( wk->viewCore, vpos );
      (*seq)++;
    }
    break;
  case 1:
    if( BTLV_ChangeForm_Wait(wk->viewCore) )
    {
      (*seq)++;
    }
    break;
  default:
    return TRUE;

  }
  return FALSE;
}



//---------------------------------------------------------------------------------------
/**
 *  とくせいウィンドウ表示オン
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_TOKWIN_In( BTL_CLIENT* wk, int* seq, const int* args )
{
  BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
  switch( *seq ){
  case 0:
    if( BTL_CLIENT_IsChapterSkipMode(wk) ){
      return TRUE;
    }

    BTLV_TokWin_DispStart( wk->viewCore, pos, TRUE );
    (*seq)++;
    break;

  case 1:
    if( BTLV_TokWin_DispWait(wk->viewCore, pos) ){
      (*seq)++;
    }
    break;

  default:
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------------------
/**
 *  とくせいウィンドウ表示オフ
 */
//---------------------------------------------------------------------------------------
static BOOL scProc_TOKWIN_Out( BTL_CLIENT* wk, int* seq, const int* args )
{
  BtlPokePos pos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, args[0] );
  switch( *seq ){
  case 0:
    BTLV_QuitTokWin( wk->viewCore, pos );
    (*seq)++;
    break;
  case 1:
    if( BTLV_QuitTokWinWait(wk->viewCore, pos) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}



static BOOL scProc_OP_HpMinus( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_HpMinus( pp, args[1] );
  return TRUE;
}


static BOOL scProc_OP_HpPlus( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_HpPlus( bpp, args[1] );
  BTL_N_Printf( DBGSTR_CLIENT_OP_HPPlus, bpp, args[0], args[1], BPP_GetValue(bpp, BPP_HP) );
  return TRUE;
}
static BOOL scProc_OP_PPMinus( BTL_CLIENT* wk, int* seq, const int* args )
{
  u8 pokeID = args[0];
  u8 wazaIdx = args[1];
  u8 value = args[2];

  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
  BPP_WAZA_DecrementPP( pp, wazaIdx, value );

  return TRUE;
}
static BOOL scProc_OP_PPMinus_Org( BTL_CLIENT* wk, int* seq, const int* args )
{
  u8 pokeID = args[0];
  u8 wazaIdx = args[1];
  u8 value = args[2];

  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
  BPP_WAZA_DecrementPP_Org( pp, wazaIdx, value );

  return TRUE;
}
static BOOL scProc_OP_WazaUsed( BTL_CLIENT* wk, int* seq, const int* args )
{
  u8 pokeID = args[0];
  u8 wazaIdx = args[1];

  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, pokeID );
  BPP_WAZA_SetUsedFlag( bpp, wazaIdx );

  return TRUE;
}


static BOOL scProc_OP_HpZero( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_HpZero( pp );
  return TRUE;
}
/**
 *  PP回復  args[0]:pokeID  args[1]:WazaIdx  args[2]:回復PP値
 */
static BOOL scProc_OP_PPPlus( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_WAZA_IncrementPP( bpp, args[1], args[2] );
  return TRUE;
}
static BOOL scProc_OP_PPPlus_Org( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_WAZA_IncrementPP_Org( bpp, args[1], args[2] );
  return TRUE;
}


static BOOL scProc_OP_RankUp( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_RankUp( pp, args[1], args[2] );
  return TRUE;
}
static BOOL scProc_OP_RankDown( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_RankDown( pp, args[1], args[2] );
  return TRUE;
}
static BOOL scProc_OP_RankSet7( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_RankSet( bpp, BPP_ATTACK_RANK,      args[1] );
  BPP_RankSet( bpp, BPP_DEFENCE_RANK,     args[2] );
  BPP_RankSet( bpp, BPP_SP_ATTACK_RANK,   args[3] );
  BPP_RankSet( bpp, BPP_SP_DEFENCE_RANK,  args[4] );
  BPP_RankSet( bpp, BPP_AGILITY_RANK,     args[5] );
  BPP_RankSet( bpp, BPP_HIT_RATIO,        args[6] );
  BPP_RankSet( bpp, BPP_AVOID_RATIO,      args[7] );
  return TRUE;
}
static BOOL scProc_OP_RankRecover( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_RankRecover( bpp );
  return TRUE;
}
static BOOL scProc_OP_RankReset( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_RankReset( bpp );
  return TRUE;
}
static BOOL scProc_OP_AddCritical( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_AddCriticalRank( pp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_SickSet( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_SICK_CONT cont;
  cont.raw = args[2];

  BPP_SetWazaSick( pp, args[1], cont );
  return TRUE;
}
static BOOL scProc_OP_CurePokeSick( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_CurePokeSick( pp );
  return TRUE;
}
static BOOL scProc_OP_CureWazaSick( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_CureWazaSick( pp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_MemberIn( BTL_CLIENT* wk, int* seq, const int* args )
{
  u8 clientID = wk->cmdArgs[0];
  u8 posIdx = wk->cmdArgs[1];
  u8 memberIdx = wk->cmdArgs[2];

  {
    BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );

    BTL_POKEPARAM* bpp;
    if( posIdx != memberIdx ){
      BTL_PARTY_SwapMembers( party, posIdx, memberIdx );
    }
    bpp = BTL_PARTY_GetMemberData( party, posIdx );
    BPP_SetAppearTurn( bpp, args[3] );
    BPP_Clear_ForIn( bpp );
    BTL_N_Printf( DBGSTR_CLIENT_OP_MemberIn, posIdx, memberIdx, BPP_GetID(bpp), BPP_GetValue(bpp,BPP_HP));
  }
  return TRUE;
}
static BOOL scProc_OP_ChangePokeType( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_ChangePokeType( pp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_WSTurnCheck( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_WazaSick_TurnCheck( bpp, NULL, NULL );
  return TRUE;
}
static BOOL scProc_OP_ConsumeItem( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_ConsumeItem( pp );
  return TRUE;
}
static BOOL scProc_OP_UpdateUseWaza( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );

  {
    BTL_N_Printf( DBGSTR_CLIENT_UpdateWazaProcResult, args[0], args[5], args[4], args[3], args[1],  args[2] );
  }

  BPP_UpdateWazaProcResult( bpp, args[1], args[2], args[3], args[4], args[5] );
  return TRUE;
}
/**
 *  継続フラグセット  args[0]=pokeID, args[1]=flagID
 */
static BOOL scProc_OP_SetContFlag( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_CONTFLAG_Set( pp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_ResetContFlag( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_CONTFLAG_Clear( pp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_SetTurnFlag( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_TURNFLAG_Set( pp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_ResetTurnFlag( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_TURNFLAG_ForceOff( pp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_ChangeTokusei( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_ChangeTokusei( pp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_SetItem( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_SetItem( pp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_UpdateWazaNumber( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  // サーバコマンド送信時の都合で引数がヘンな並びになってる…
  BPP_WAZA_UpdateID( pp, args[1], args[4], args[2], args[3] );
  return TRUE;
}
static BOOL scProc_OP_OutClear( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* pp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_Clear_ForOut( pp );
  return TRUE;
}
static BOOL scProc_OP_AddFldEff( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_CALC_BITFLG_Set( wk->fieldEffectFlag, args[0] );
  if( !BTL_MAIN_CheckImServerMachine(wk->mainModule) )
  {
    BPP_SICK_CONT cont;
    cont.raw = (u32)(args[1]);
    BTL_FIELD_AddEffect( args[0], cont );
  }
  return TRUE;
}
static BOOL scProc_OP_AddFldEffDepend( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_CALC_BITFLG_Set( wk->fieldEffectFlag, args[0] );
  if( !BTL_MAIN_CheckImServerMachine(wk->mainModule) )
  {
    BTL_FIELD_AddDependPoke( args[0], args[1] );
  }
  return TRUE;
}
static BOOL scProc_OP_DelFldEffDepend( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_CALC_BITFLG_Set( wk->fieldEffectFlag, args[0] );
  if( !BTL_MAIN_CheckImServerMachine(wk->mainModule) )
  {
    BTL_FIELD_RemoveDependPokeEffect( args[0] );
  }
  return TRUE;
}
static BOOL scProc_OP_RemoveFldEff( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_CALC_BITFLG_Off( wk->fieldEffectFlag, args[0] );
  return TRUE;
}
static BOOL scProc_OP_SetPokeCounter( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_COUNTER_Set( bpp, args[1], args[2] );
  return TRUE;
}
static BOOL scProc_OP_BatonTouch( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* user = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BTL_POKEPARAM* target = BTL_POKECON_GetPokeParam( wk->pokeCon, args[1] );
  BPP_BatonTouchParam( target, user );
  return TRUE;
}
static BOOL scProc_OP_MigawariCreate( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_MIGAWARI_Create( bpp, args[1] );
  return TRUE;
}
static BOOL scProc_OP_MigawariDelete( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_MIGAWARI_Delete( bpp );
  return TRUE;
}
static BOOL scProc_OP_ShooterCharge( BTL_CLIENT* wk, int* seq, const int* args )
{
  u8 clientID = args[0];
  u8 increment = args[1];

  if( clientID == wk->myID )
  {
    wk->shooterEnergy += increment;
    if( wk->shooterEnergy > BTL_SHOOTER_ENERGY_MAX ){
      wk->shooterEnergy = BTL_SHOOTER_ENERGY_MAX;
    }
  }
  return TRUE;
}
static BOOL scProc_OP_SetFakeSrc( BTL_CLIENT* wk, int* seq, const int* args )
{
  u8 clientID = args[0];
  u8 memberIdx = args[1];

  BTL_PARTY* party = BTL_POKECON_GetPartyData( wk->pokeCon, clientID );
//  BTL_MAIN_SetFakeSrcMember( wk->mainModule, party, memberIdx );
  BTL_MAIN_SetIllusionForParty(  wk->mainModule, party, clientID );
  return TRUE;
}
static BOOL scProc_OP_ClearConsumedItem( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_ClearConsumedItem( bpp );
  return TRUE;
}
/**
 *  特定ポケ依存の状態異常を回復  args[0]:回復するポケID, [1]:依存対象ポケID
 */
static BOOL scProc_OP_CureSickDependPoke( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_CureWazaSickDependPoke( bpp, args[1] );
  return TRUE;
}

/**
 *  ワザダメージ記録追加
 *  args [0]:defPokeID [1]:atkPokeID [2]:pokePos [3]:wazaType [4]:wazaID [5]:damage
 */
static BOOL scProc_OP_AddWazaDamage( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_WAZADMG_REC rec;

  BPP_WAZADMG_REC_Setup( &rec, args[1], args[2], args[4], args[3], args[5] );
  BPP_WAZADMGREC_Add( bpp, &rec );

  return TRUE;
}
/**
 *  ターンチェック処理
 *  args [0]:pokeID
 */
static BOOL scProc_OP_TurnCheck( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );

  BPP_TurnCheck( bpp );
  BPP_CombiWaza_ClearParam( bpp );

  return TRUE;
}



/*
 *  能力基本値書き換え  [0]:pokeID, [1]:statusID [2]:value
 */
static BOOL scProc_OP_SetStatus( BTL_CLIENT* wk, int* seq, const int* args )
{
  BppValueID statusID = args[1];

  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_SetBaseStatus( bpp, statusID, args[2] );
  return TRUE;
}
/*
 *  体重値書き換え  [0]:pokeID, [2]:weight
 */
static BOOL scProc_OP_SetWeight( BTL_CLIENT* wk, int* seq, const int* args )
{
  BTL_POKEPARAM* bpp = BTL_POKECON_GetPokeParam( wk->pokeCon, args[0] );
  BPP_SetWeight( bpp, args[1] );
  return TRUE;
}



//------------------------------------------------------------------------------------------------------
// 戦闘中ポケモンの所有とくせい・タイプ等チェック
//------------------------------------------------------------------------------------------------------
static u8 countFrontPokeTokusei( BTL_CLIENT* wk, PokeTokusei tokusei )
{
  const BTL_POKEPARAM* bpp;
  int i, cnt;
  for(i=0, cnt=0; i<wk->numCoverPos; ++i){
    bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
    if( !BPP_IsDead(bpp) )
    {
      if( BPP_GetValue(bpp, BPP_TOKUSEI_EFFECTIVE) == tokusei ){ ++cnt; }
    }
  }
  return cnt;
}
static u8 countFrontPokeType( BTL_CLIENT* wk, PokeType type )
{
  const BTL_POKEPARAM* bpp;
  int i, cnt;
  for(i=0, cnt=0; i<wk->numCoverPos; ++i){
    bpp = BTL_PARTY_GetMemberDataConst( wk->myParty, i );
    if( !BPP_IsDead(bpp) )
    {
      if( BPP_IsMatchType(bpp, type) ){ ++cnt; }
    }
  }
  return cnt;
}


//------------------------------------------------------------------------------------------------------
// 外部モジュールからの情報取得処理
//------------------------------------------------------------------------------------------------------
u8 BTL_CLIENT_GetClientID( const BTL_CLIENT* client )
{
  return client->myID;
}

const BTL_PARTY* BTL_CLIENT_GetParty( const BTL_CLIENT* client )
{
  return client->myParty;
}


//=============================================================================================
/**
 * 現在、行動選択処理中のポケモン位置IDを取得
 *
 * @param   client
 *
 * @retval  BtlPokePos
 */
//=============================================================================================
BtlPokePos BTL_CLIENT_GetProcPokePos( const BTL_CLIENT* client )
{
  return client->basePos + client->procPokeIdx*2;
}
//=============================================================================================
/**
 * シューターエネルギー量を取得
 *
 * @param   client
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTL_CLIENT_GetShooterEnergy( const BTL_CLIENT* client )
{
  return client->shooterEnergy;
}


//------------------------------------------------------------------------------------------------------
// AIアイテム
//------------------------------------------------------------------------------------------------------

/**
 *  ワーク初期化
 */
static void AIItem_Setup( BTL_CLIENT* wk )
{
  u32 i;

  for(i=0; i<NELEMS(wk->AIItem); ++i){
    wk->AIItem[i] = BTL_MAIN_GetClientUseItem( wk->mainModule, wk->myID, i );
  }
}

/**
 *  使用アイテムチェック
 */
static u16 AIItem_CheckUse( BTL_CLIENT* wk, const BTL_POKEPARAM* bpp, const BTL_PARTY* party )
{
  static const u8 borderMembers[] = {
    BTL_PARTY_MEMBER_MAX, 4, 2, 1,
  };
  u32 i, numMembers = BTL_PARTY_GetMemberCount( party );

  for(i=0; i<NELEMS(wk->AIItem); ++i)
  {
    if( borderMembers[i] < numMembers ){
      break;
    }
    if( wk->AIItem[i] != ITEM_DUMMY_DATA )
    {
      BOOL fUse = FALSE;
      // HP回復系
      if( BTL_CALC_ITEM_GetParam(wk->AIItem[i], ITEM_PRM_HP_RCV) )
      {
        u32 hp = BPP_GetValue( bpp, BPP_HP );
        if( hp <= BTL_CALC_QuotMaxHP_Zero(bpp, 4) ){
          fUse = TRUE;
        }
        else{
          continue;
        }
      }
      // 能力ランクアップ系
      else if( check_status_up_item(wk->AIItem[i], bpp) )
      {
        fUse = TRUE;
      }
      // 状態異常回復系
      else if( check_cure_sick_item(wk->AIItem[i], bpp) )
      {
        fUse = TRUE;
      }

      if( fUse )
      {
        u16 useItem = wk->AIItem[i];
        wk->AIItem[i] = ITEM_DUMMY_DATA;
        return useItem;
      }
    }
  }

  return ITEM_DUMMY_DATA;
}
/**
 *  能力ランクアップ系アイテムである＆使って効果がある判定
 */
static BOOL check_status_up_item( u16 itemID, const BTL_POKEPARAM* bpp )
{
  static const struct {
    u8 itemParamID;
    u8 rankID;
  }check_tbl[] = {
    { ITEM_PRM_ATTACK_UP,     BPP_ATTACK_RANK     },   // 攻撃力アップ
    { ITEM_PRM_DEFENCE_UP,    BPP_DEFENCE_RANK    },   // 防御力アップ
    { ITEM_PRM_SP_ATTACK_UP,  BPP_SP_ATTACK_RANK  },   // 特攻アップ
    { ITEM_PRM_SP_DEFENCE_UP, BPP_SP_DEFENCE_RANK },   // 特防アップ
    { ITEM_PRM_AGILITY_UP,    BPP_AGILITY_RANK    },   // 素早さアップ
    { ITEM_PRM_HIT_UP,        BPP_HIT_RATIO       },   // 命中率アップ
  };
  u32 i;

  for(i=0; i<NELEMS(check_tbl); ++i)
  {
    if( BTL_CALC_ITEM_GetParam(itemID, check_tbl[i].itemParamID) )
    {
      if( BPP_IsRankEffectValid(bpp, check_tbl[i].rankID, 1) ){
        return TRUE;
      }
    }
  }

 // クリティカルだけ別判定
  if( BTL_CALC_ITEM_GetParam(itemID, ITEM_PRM_CRITICAL_UP) )
  {
    if( BPP_GetCriticalRank(bpp) == 0 ){
      return TRUE;
    }
  }

  return FALSE;
}

/**
 *  状態異常回復系アイテムである＆使って効果がある判定
 */
static BOOL check_cure_sick_item( u16 itemID, const BTL_POKEPARAM* bpp )
{
  static const struct {
    u8 itemParamID;
    u8 sickID;
  }check_tbl[] = {
    { ITEM_PRM_SLEEP_RCV,     WAZASICK_NEMURI      },   // 眠り回復
    { ITEM_PRM_POISON_RCV,    WAZASICK_DOKU        },   // 毒回復
    { ITEM_PRM_BURN_RCV,      WAZASICK_YAKEDO      },   // 火傷回復
    { ITEM_PRM_ICE_RCV,       WAZASICK_KOORI       },   // 氷回復
    { ITEM_PRM_PARALYZE_RCV,  WAZASICK_MAHI        },   // 麻痺回復
    { ITEM_PRM_PANIC_RCV,     WAZASICK_KONRAN      },   // 混乱回復
    { ITEM_PRM_MEROMERO_RCV,  WAZASICK_MEROMERO    },   // メロメロ回復
  };
  u32 i;

  for(i=0; i<NELEMS(check_tbl); ++i)
  {
    if( BTL_CALC_ITEM_GetParam(itemID, check_tbl[i].itemParamID) )
    {
      if( BPP_CheckSick(bpp, check_tbl[i].sickID) ){
        return TRUE;
      }
    }
  }

  return FALSE;
}

//------------------------------------------------------------------------------------------------------
// 録画データプレイヤー
//------------------------------------------------------------------------------------------------------

/**
 *  領域初期化（再生時でなくても、これを呼び出して初期化する必要がある）
 */
static void RecPlayer_Init( RECPLAYER_CONTROL* ctrl )
{
  ctrl->ctrlCode = RECCTRL_NONE;
  ctrl->seq = 0;
  ctrl->fTurnIncrement = FALSE;
  ctrl->fFadeOutStart = FALSE;
  ctrl->fFadeOutDone = FALSE;
  ctrl->fChapterSkip = FALSE;
  ctrl->fLock = FALSE;
  ctrl->turnCount = 0;
  ctrl->nextTurnCount = 0;
  ctrl->maxTurnCount = 0;
  ctrl->skipTurnCount = 0;
  ctrl->handlingTimer = 0;
}
/**
 *  再生準備処理（再生時にのみ呼び出す）
 */
static void RecPlayer_Setup( RECPLAYER_CONTROL* ctrl, u32 turnCnt )
{
  ctrl->maxTurnCount = turnCnt;
}
/**
 *  停止またはチャプター送り前のブラックアウトが発生したか判定
 */
static BOOL RecPlayer_CheckBlackOut( const RECPLAYER_CONTROL* ctrl )
{
  return ctrl->fFadeOutDone;
}
/**
 *  ターンカウンタインクリメントリクエスト
 */
static void RecPlayer_TurnIncReq( RECPLAYER_CONTROL* ctrl )
{
  ctrl->fTurnIncrement = TRUE;
}

/**
 *  状態取得
 */
static RecCtrlCode RecPlayer_GetCtrlCode( const RECPLAYER_CONTROL* ctrl )
{
  return ctrl->ctrlCode;
}
/**
 *  チャプタースキップモードへ切り替え
 */
static void RecPlayer_ChapterSkipOn( RECPLAYER_CONTROL* ctrl, u32 nextTurnNum )
{
  ctrl->fChapterSkip  = TRUE;
  ctrl->skipTurnCount = 0;
  ctrl->nextTurnCount = nextTurnNum;
  ctrl->fFadeOutStart = FALSE;
  ctrl->fFadeOutDone  = FALSE;
}
/**
 *
 */
static void RecPlayer_ChapterSkipOff( RECPLAYER_CONTROL* ctrl )
{
  ctrl->seq = 0;
  ctrl->fChapterSkip = FALSE;
}
/**
 *  スキップ処理が終了したか判定
 */
static BOOL RecPlayer_CheckChapterSkipEnd( const RECPLAYER_CONTROL* ctrl )
{
  return ( ctrl->skipTurnCount == ctrl->nextTurnCount );
}
/**
 *
 */
static u32 RecPlayer_GetNextTurn( const RECPLAYER_CONTROL* ctrl )
{
  return ctrl->nextTurnCount;
}
/**
 *  キー・タッチパネル操作ロック
 *
 *  @retval BOOL  既にフェード中でロック不可の場合、FALSEを返す
 */
static BOOL RecPlayerCtrl_Lock( RECPLAYER_CONTROL* ctrl )
{
  if( ctrl->fFadeOutStart ){
    return FALSE;
  }
  ctrl->fLock = TRUE;
  return TRUE;
}
/**
 *  キー・タッチパネル操作アンロック
 */
static void RecPlayerCtrl_Unlock( RECPLAYER_CONTROL* ctrl )
{
  ctrl->fLock = FALSE;
}
/**
 *  メインコントロール（キー・タッチパネルに反応、状態遷移する）
 */
static void RecPlayerCtrl_Main( BTL_CLIENT* wk, RECPLAYER_CONTROL* ctrl )
{
  enum {
    CHAPTER_CTRL_FRAMES = 45,
  };

  if( (wk->myType == BTL_CLIENT_TYPE_RECPLAY)
  &&  (ctrl->fLock == FALSE)
  &&  (ctrl->maxTurnCount)
  ){
    enum {
      SEQ_DEFAULT = 0,
      SEQ_FADEOUT,
      SEQ_STAY,
    };

    BOOL fTurnUpdate = FALSE;

    if( ctrl->fTurnIncrement )
    {
      ctrl->fTurnIncrement = FALSE;

      if( ctrl->fChapterSkip == FALSE )
      {
        if( ctrl->turnCount < ctrl->maxTurnCount ){
          ++(ctrl->turnCount);
          if(ctrl->handlingTimer == 0){
            ctrl->nextTurnCount = ctrl->turnCount;
          }
          fTurnUpdate = TRUE;
        }
      }
      else
      {
        if( ctrl->skipTurnCount < ctrl->nextTurnCount )
        {
          ctrl->skipTurnCount++;
          if( ctrl->skipTurnCount == ctrl->nextTurnCount )
          {
            ctrl->turnCount = ctrl->nextTurnCount;
            if( wk->viewCore ){
              BTLV_UpdateRecPlayerInput( wk->viewCore, ctrl->turnCount, ctrl->nextTurnCount );
            }
          }
        }
        return;
      }
    }

    // 最初のチャプタまで何もしない（入場エフェクト待ち）
    if( ctrl->turnCount == 0 ){
//      BTLV_UpdateRecPlayerInput( wk->viewCore, ctrl->turnCount, ctrl->nextTurnCount );
      return;
    }

    // ここから先、描画クライアント以外は何もしない
    if( wk->viewCore == NULL) {
      return;
    }

    switch( ctrl->seq ){
    case SEQ_DEFAULT:
      {
        int result = BTLV_CheckRecPlayerInput( wk->viewCore );
        BOOL fCtrlUpdate = FALSE;

        switch( result ){
        case  BTLV_INPUT_BR_SEL_STOP:     //停止
          ctrl->ctrlCode = RECCTRL_QUIT;
          BTLV_RecPlayFadeOut_Start( wk->viewCore );
          ctrl->fFadeOutStart = TRUE;
          ctrl->seq = SEQ_FADEOUT;
          BTLV_RecPlayer_StartQuit( wk->viewCore, ctrl->turnCount, BTLV_INPUT_BR_STOP_KEY );
          break;

        case BTLV_INPUT_BR_SEL_REW:
          if( ctrl->nextTurnCount > 1 ){
            ctrl->nextTurnCount--;
            fCtrlUpdate = TRUE;
          }
          ctrl->handlingTimer = CHAPTER_CTRL_FRAMES;
          break;

        case BTLV_INPUT_BR_SEL_FF:
          if( ctrl->nextTurnCount < ctrl->maxTurnCount ){
            ctrl->nextTurnCount++;
            fCtrlUpdate = TRUE;
          }
          ctrl->handlingTimer = CHAPTER_CTRL_FRAMES;
          break;
        }

        if( fCtrlUpdate || fTurnUpdate ){
          BTLV_UpdateRecPlayerInput( wk->viewCore, ctrl->turnCount, ctrl->nextTurnCount );
        }

        if( ctrl->handlingTimer )
        {
          if( --(ctrl->handlingTimer) == 0 )
          {
            ctrl->ctrlCode = RECCTRL_CHAPTER;
            BTLV_RecPlayFadeOut_Start( wk->viewCore );
            ctrl->fFadeOutStart = TRUE;
            ctrl->seq = SEQ_FADEOUT;
          }
          break;
        }
      }
      break;

    case SEQ_FADEOUT:
      if( BTLV_RecPlayFadeOut_Wait(wk->viewCore) ){
        ctrl->fFadeOutDone = TRUE;
        ctrl->seq = SEQ_STAY;
      }
      break;

    case SEQ_STAY:
      break;
    }
  }
}







/*============================================================================================*/
/* デバッグ用AIクライアントを操作するためのシステム                                           */
/*============================================================================================*/
#ifdef PM_DEBUG
/**
 *  AI操作システム：初期化
 */
static void AICtrl_Init( void )
{
  GViewCore = NULL;
  GControlableAIClientID = BTL_CLIENTID_NULL;
}
/**
 *  AI操作システム：１クライアント操作終了->次のクライアントへ権限委譲
 */
static void AICtrl_Delegate( BTL_CLIENT* wk )
{
  u8 clientID = wk->myID + 1;
  while( clientID < BTL_CLIENT_MAX)
  {
    if( BTL_MAIN_IsExistClient(wk->mainModule, clientID) )
    {
      GControlableAIClientID = clientID;
      GViewCore = wk->viewCore;
      aictrl_RestoreViewClient( wk );
      return;
    }
    ++clientID;
  }

  GControlableAIClientID = BTL_CLIENTID_NULL;
  aictrl_RestoreViewClient( wk );
}
/**
 *  AI操作システム：描画モジュールの管理クライアントを自分から元に戻す
 */
static void aictrl_RestoreViewClient( BTL_CLIENT* wk )
{
  if( wk->viewOldClient != NULL )
  {
    BTLV_SetTmpClient( wk->viewCore, wk->viewOldClient );
    wk->viewCore = NULL;
    wk->viewOldClient = NULL;
  }
}
/**
 *  AI操作システム：操作権限が自分になっているか判定
 */
static BOOL AICtrl_IsMyFase( BTL_CLIENT* wk )
{
  if( GControlableAIClientID == wk->myID )
  {
    if( wk->viewCore == NULL )
    {
      wk->viewCore = GViewCore;
      wk->viewOldClient = BTLV_SetTmpClient( wk->viewCore, wk );
    }
    return TRUE;
  }
  return FALSE;
}

#endif
