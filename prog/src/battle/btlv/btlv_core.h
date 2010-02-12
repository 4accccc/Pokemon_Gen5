//=============================================================================================
/**
 * @file  btlv_core.h
 * @brief ポケモンWB バトル描画担当メインモジュール
 * @author  taya
 *
 * @date  2008.10.02  作成
 */
//=============================================================================================
#ifndef __BTLV_CORE_H__
#define __BTLV_CORE_H__


#include "btlv_core_def.h"

//----------------------------------------------------------------------
// 参照ヘッダ include
//----------------------------------------------------------------------
#include "../app/b_bag.h"
#include "../app/b_plist.h"

#include "../btl_string.h"
#include "../btl_main_def.h"
#include "../btl_client.h"
#include "../btl_action.h"
#include "../btl_calc.h"
#include "../btl_pokeselect.h"


#include "btlv_effect.h"
#include "btlv_common.h"

//----------------------------------------------------------------------
/**
 *  描画コマンド
 */
//----------------------------------------------------------------------
typedef enum {

  BTLV_CMD_SETUP,
  BTLV_CMD_SETUP_DEMO,    ///< 捕獲デモ
  BTLV_CMD_SELECT_ACTION,
  BTLV_CMD_MSG,

  BTLV_CMD_MAX,
  BTLV_CMD_NULL = BTLV_CMD_MAX,

}BtlvCmd;

//----------------------------------------------------------------------
/**
 *  戻り値チェック
 */
//----------------------------------------------------------------------
typedef enum {

  BTLV_RESULT_NONE = 0,
  BTLV_RESULT_DONE,
  BTLV_RESULT_CANCEL,

}BtlvResult;


//--------------------------------------------------------------
/**
 *  メッセージウェイト指定
 */
//--------------------------------------------------------------
enum {

  BTLV_MSGWAIT_NONE = 0,
  BTLV_MSGWAIT_STD = 80,

};

//----------------------------------------------------------------------
/**
 *  文字出力パラメータ
 */
//----------------------------------------------------------------------
typedef struct {
  u16   strID;          ///< 基本文字列ID
  u8    wait;           ///< 改行・表示終了時ウェイト
  u8    strType : 4;    ///< BtlStrType
  u8    argCnt  : 4;    ///< 引数の数
  int   args[BTL_STR_ARG_MAX];  ///< 引数
}BTLV_STRPARAM;

static inline void BTLV_STRPARAM_Setup( BTLV_STRPARAM* sp, BtlStrType strType, u16 strID )
{
  int i;
  for(i=0; i<NELEMS(sp->args); ++i){
    sp->args[i] = 0;
  }
  sp->argCnt = 0;
  sp->strID = strID;
  sp->strType = strType;
  sp->wait = BTLV_MSGWAIT_STD;
}
static inline void BTLV_STRPARAM_AddArg( BTLV_STRPARAM* sp, int arg )
{
  if( sp->argCnt < NELEMS(sp->args) ){
    sp->args[ sp->argCnt++ ] = arg;
  }
}
static inline void BTLV_STRPARAM_SetWait( BTLV_STRPARAM* sp, u8 wait )
{
  sp->wait = wait;
}



//=============================================================================================
/**
 * 描画メインモジュールの生成
 *
 * @param   mainModule    システムメインモジュールのハンドラ
 * @param   heapID      生成先ヒープID
 *
 * @retval  BTLV_CORE*    生成された描画メインモジュールのハンドラ
 */
//=============================================================================================
extern BTLV_CORE*  BTLV_Create( BTL_MAIN_MODULE* mainModule, const BTL_CLIENT* client, const BTL_POKE_CONTAINER* pokeCon, HEAPID heapID );

//=============================================================================================
/**
 * 描画メインモジュールの破棄
 *
 * @param   core    描画メインモジュールのハンドラ
 */
//=============================================================================================
extern void BTLV_Delete( BTLV_CORE* core );

//=============================================================================================
/**
 * 描画メインループ
 *
 * @param   core    描画メインモジュールのハンドラ
 */
//=============================================================================================
extern void BTLV_CORE_Main( BTLV_CORE* core );

//=============================================================================================
/**
 * BTLV_StartCommand で受け付けた描画コマンドの終了を待つ
 *
 * @param   wk      描画メインモジュールのハンドラ
 *
 * @retval  BOOL    終了していたらTRUE
 */
//=============================================================================================
extern void BTLV_StartCommand( BTLV_CORE* btlv, BtlvCmd cmd );

//=============================================================================================
/**
 * BTLV_StartCommand で受け付けた描画コマンドの終了を待つ
 *
 * @param   wk      描画メインモジュールのハンドラ
 *
 * @retval  BOOL    終了していたらTRUE
 */
//=============================================================================================
extern BOOL BTLV_WaitCommand( BTLV_CORE* btlv );



extern void BTLV_UI_SelectAction_Start( BTLV_CORE* core, const BTL_POKEPARAM* bpp, BOOL fReturnable, BTL_ACTION_PARAM* dest );
extern BtlAction  BTLV_UI_SelectAction_Wait( BTLV_CORE* core );
extern void BTLV_UI_SelectAction_ForceQuit( BTLV_CORE* wk );

extern void BTLV_UI_SelectWaza_Start( BTLV_CORE* core, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* dest );
extern BOOL BTLV_UI_SelectWaza_Wait( BTLV_CORE* core );
extern void BTLV_UI_SelectWaza_ForceQuit( BTLV_CORE* wk );


extern void BTLV_UI_SelectTarget_Start( BTLV_CORE* core, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* dest );
extern BtlvResult BTLV_UI_SelectTarget_Wait( BTLV_CORE* core );
extern void BTLV_UI_SelectTarget_ForceQuit( BTLV_CORE* core );


extern void BTLV_UI_Restart( BTLV_CORE* wk );


extern void BTLV_StartMemberChangeAct( BTLV_CORE* wk, BtlPokePos pos, u8 clientID, u8 memberIdx );
extern BOOL BTLV_WaitMemberChangeAct( BTLV_CORE* wk );


//=============================================================================================
/**
 * ポケモン選択処理を開始
 *
 * @param   core            [in]  描画メインモジュールハンドラ
 * @param   param           [in]  選択処理パラメータポインタ
 * @param   outMemberIndex  [in]  「ポケモン」メニューからのメンバー交替時、退出するメンバーのIndex / それ以外-1
 * @param   fCantEsc        [in]
 * @param   result          [out] 選択結果格納構造体ポインタ
 */
//=============================================================================================
extern void BTLV_StartPokeSelect( BTLV_CORE* core, const BTL_POKESELECT_PARAM* param, int outMemberIndex, BOOL fCantEsc, BTL_POKESELECT_RESULT* result );

//=============================================================================================
/**
 * ポケモン選択処理の終了待ち
 *
 * @param   core      [in]  描画メインモジュールハンドラ
 *
 * @retval  BOOL      終了したらTRUE
 */
//=============================================================================================
extern BOOL BTLV_WaitPokeSelect( BTLV_CORE* core );

/**
 *  ポケモン選択処理の強制終了（コマンド制限時間による）
 */
extern void BTLV_ForceQuitPokeSelect( BTLV_CORE* wk );



//=============================================================================================
/**
 *
 *
 * @param   wk
 * @param   strID
 * @param   clientID
 *
 * @retval  extern void
 */
//=============================================================================================
extern void BTLV_StartMsg( BTLV_CORE* wk, const BTLV_STRPARAM* param );
extern void BTLV_StartMsgWaza( BTLV_CORE* wk, u8 pokeID, u16 waza );

extern void BTLV_StartMsgStd( BTLV_CORE* wk, u16 strID, const int* args );
extern void BTLV_StartMsgSet( BTLV_CORE* wk, u16 strID, const int* args );


extern BOOL BTLV_WaitMsg( BTLV_CORE* wk );
extern BOOL BTLV_IsJustDoneMsg( BTLV_CORE* wk );


extern void BTLV_ACT_WazaEffect_Start( BTLV_CORE* wk, BtlPokePos atPokePos, BtlPokePos defPokePos, WazaID waza,
  BtlvWazaEffect_TurnType turnType, u8 continueCount );

extern BOOL BTLV_ACT_WazaEffect_Wait( BTLV_CORE* wk );

extern void BTLV_ACT_DamageEffectSingle_Start( BTLV_CORE* wk, WazaID waza, BtlPokePos defPokePos, u16 damage, BtlTypeAff aff );
extern BOOL BTLV_ACT_DamageEffectSingle_Wait( BTLV_CORE* wk );


//=============================================================================================
/**
 * 複数体一斉ダメージ処理 開始
 *
 * @param   wk
 * @param   pokeCnt   ポケモン数
 * @param   affifnity 相性ID
 * @param   pokeID    対象となるポケモンID配列
 */
//=============================================================================================
extern void BTLV_ACT_DamageEffectPlural_Start( BTLV_CORE* wk, u32 pokeCnt, BtlTypeAff affifnity, const u8* pokeID, WazaID waza );
extern BOOL BTLV_ACT_DamageEffectPlural_Wait( BTLV_CORE* wk );


extern void BTLV_StartDeadAct( BTLV_CORE* wk, BtlPokePos pokePos );
extern BOOL BTLV_WaitDeadAct( BTLV_CORE* wk );
extern void BTLV_StartReliveAct( BTLV_CORE* wk, BtlPokePos pos );
extern BOOL BTLV_WaitReliveAct( BTLV_CORE* wk );
extern void BTLV_ACT_MemberOut_Start( BTLV_CORE* wk, BtlvMcssPos vpos );
extern BOOL BTLV_ACT_MemberOut_Wait( BTLV_CORE* wk );
extern void BTLV_ACT_SimpleHPEffect_Start( BTLV_CORE* wk, BtlPokePos pokePos );
extern BOOL BTLV_ACT_SimpleHPEffect_Wait( BTLV_CORE* wk );
extern void BTLV_AddEffectByPos( BTLV_CORE* wk, BtlvMcssPos vpos, u16 effectNo );
extern BOOL BTLV_WaitEffectByPos( BTLV_CORE* wk, BtlvMcssPos vpos );
extern void BTLV_AddEffectByDir( BTLV_CORE* wk, BtlvMcssPos vpos_from, BtlvMcssPos vpos_to, u16 effectNo );
extern BOOL BTLV_WaitEffectByDir( BTLV_CORE* wk );


extern void BTLV_TokWin_DispStart( BTLV_CORE* wk, BtlPokePos pos, BOOL fFlash );
extern BOOL BTLV_TokWin_DispWait( BTLV_CORE* wk, BtlPokePos pos );
extern void BTLV_QuitTokWin( BTLV_CORE* wk, BtlPokePos pos );
extern BOOL BTLV_QuitTokWinWait( BTLV_CORE* wk, BtlPokePos pos );
extern void BTLV_TokWin_Renew_Start( BTLV_CORE* wk, BtlPokePos pos );
extern BOOL BTLV_TokWin_Renew_Wait( BTLV_CORE* wk, BtlPokePos pos );

extern void BTLV_StartRankDownEffect( BTLV_CORE* wk, u8 vpos );
extern void BTLV_StartRankUpEffect( BTLV_CORE* wk, u8 vpos );
extern BOOL BTLV_WaitRankEffect( BTLV_CORE* wk, u8 vpos );


extern void BTLV_StartCommWait( BTLV_CORE* wk );
extern BOOL BTLV_WaitCommWait( BTLV_CORE* wk );
extern void BTLV_ResetCommWaitInfo( BTLV_CORE* wk );

extern void BTLV_KinomiAct_Start( BTLV_CORE* wk, BtlPokePos pos );
extern BOOL BTLV_KinomiAct_Wait( BTLV_CORE* wk, BtlPokePos pos );
extern void BTLV_FakeDisable_Start( BTLV_CORE* wk, BtlPokePos pos );
extern BOOL BTLV_FakeDisable_Wait( BTLV_CORE* wk );

extern void BTLV_ChangeForm_Start( BTLV_CORE* wk, BtlvMcssPos vpos );
extern BOOL BTLV_ChangeForm_Wait( BTLV_CORE* wk );

extern void BTLV_ACT_MoveMember_Start( BTLV_CORE* wk, u8 clientID, u8 vpos1, u8 vpos2, u8 posIdx1, u8 posIdx2 );
extern BOOL BTLV_ACT_MoveMember_Wait( BTLV_CORE* wk );


extern void BTLV_ITEMSELECT_Start( BTLV_CORE* wk, u8 bagMode, u8 energy, u8 reserved_energy );
extern BOOL BTLV_ITEMSELECT_Wait( BTLV_CORE* wk );
extern void BTLV_ITEMSELECT_ForceQuit( BTLV_CORE* wk );

extern u16 BTLV_ITEMSELECT_GetItemID( BTLV_CORE* wk );
extern u8  BTLV_ITEMSELECT_GetCost( BTLV_CORE* wk );
extern u8  BTLV_ITEMSELECT_GetTargetIdx( BTLV_CORE* wk );
extern void BTLV_ITEMSELECT_ReflectUsedItem( BTLV_CORE* wk );


extern void BTLV_UI_SelectRotation_Start( BTLV_CORE* wk, BtlRotateDir prevDir );
extern BOOL BTLV_UI_SelectRotation_Wait( BTLV_CORE* wk, BtlRotateDir* result );
extern void BTLV_UI_SelectRotation_ForceQuit( BTLV_CORE* wk );

extern void BTLV_RotationMember_Start( BTLV_CORE* wk, u8 clientID, BtlRotateDir dir );
extern BOOL BTLV_RotationMember_Wait( BTLV_CORE* wk );

extern void BTLV_YESNO_Start( BTLV_CORE* wk, BTLV_STRPARAM* yes_msg, BTLV_STRPARAM* no_msg );
extern BOOL BTLV_YESNO_Wait( BTLV_CORE* wk, BtlYesNo* result );

extern void BTLV_WAZAWASURE_Start( BTLV_CORE* wk, u8 pos, WazaID waza );
extern BOOL BTLV_WAZAWASURE_Wait( BTLV_CORE* wk, u8* result );

extern void BTLV_StartSelectChangeOrEscape( BTLV_CORE* wk );
extern BOOL BTLV_WaitSelectChangeOrEscape( BTLV_CORE* wk, u8* fSelect );

//=============================================================================================
//  時間制限によるアクション選択の強制終了対応
//=============================================================================================
extern void BTLV_ForceQuitInput_Notify( BTLV_CORE* wk );
extern BOOL BTLV_ForceQuitInput_Wait( BTLV_CORE* wk );

//-----------------------------------------------------------
// 各種下請けから呼び出される関数

extern u8 BTLV_CORE_GetPlayerClientID( const BTLV_CORE* core );

#endif
