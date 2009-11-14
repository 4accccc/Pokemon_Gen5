//=============================================================================================
/**
 * @file  btl_server_flow.h
 * @brief ポケモンWB バトルシステム サーバコマンド生成処理
 * @author  taya
 *
 * @date  2009.03.06  作成
 */
//=============================================================================================
#ifndef __BTL_SERVER_FLOW_H__
#define __BTL_SERVER_FLOW_H__

#include "waza_tool/wazadata.h"

#include "btl_main.h"
#include "btl_adapter.h"
#include "btl_server.h"
#include "btl_server_cmd.h"
#include "btl_string.h"
#include "btl_sideeff.h"
#include "btl_poseff.h"
#include "btl_field.h"
#include "app/b_bag.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
typedef enum {
  SVFLOW_RESULT_DEFAULT = 0,  ///< そのまま続行
  SVFLOW_RESULT_POKE_CHANGE,  ///< ポケモン入れ替えフェーズ->バトル継続
  SVFLOW_RESULT_POKE_IN_REQ,  ///< ポケモン死亡->新ポケ選択へ
  SVFLOW_RESULT_BTL_QUIT,     ///< バトル終了（決着）
  SVFLOW_RESULT_POKE_GET,     ///< バトル終了（ポケモン捕獲）
}SvflowResult;


/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/

/**
 *  サーバ本体、サーバフロー双方から参照するクライアントパラメータ
 */

struct _SVCL_WORK {

  BTL_ADAPTER*    adapter;
  BTL_PARTY*      party;
  u8        memberCount;
  u8        numCoverPos;
  u8        isLocalClient;
  u8        myID;
};

/**
 *  サーバフロー＆ハンドラから参照するワザパラメータ
 */

typedef struct {

  WazaID        wazaID;
  PokeTypePair  userType;
  PokeType      wazaType;

}SVFL_WAZAPARAM;

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef struct _BTL_SVFLOW_WORK   BTL_SVFLOW_WORK;
typedef struct _SVCL_WORK         SVCL_WORK;


// server -> server_flow
extern BTL_SVFLOW_WORK* BTL_SVFLOW_InitSystem(
  BTL_SERVER* server, BTL_MAIN_MODULE* mainModule, BTL_POKE_CONTAINER* pokeCon,
  BTL_SERVER_CMD_QUE* que, u32 numClient, BtlBagMode bagMode, HEAPID heapID );

extern void BTL_SVFLOW_QuitSystem( BTL_SVFLOW_WORK* wk );

extern SvflowResult BTL_SVFLOW_Start( BTL_SVFLOW_WORK* wk );
extern SvflowResult BTL_SVFLOW_Start_AfterPokemonIn( BTL_SVFLOW_WORK* wk );
extern SvflowResult BTL_SVFLOW_StartAfterPokeIn( BTL_SVFLOW_WORK* wk );
extern SvflowResult BTL_SVFLOW_StartAfterPokeChange( BTL_SVFLOW_WORK* wk );
extern void BTL_SVFLOW_CreateRotationCommand( BTL_SVFLOW_WORK* wk, u8 clientID, BtlRotateDir dir );


// server_flow -> server
extern SVCL_WORK* BTL_SERVER_GetClientWork( BTL_SERVER* server, u8 clientID );
extern SVCL_WORK* BTL_SERVER_GetClientWorkIfEnable( BTL_SERVER* server, u8 clientID );
extern u8 BTL_SVFLOW_GetEscapeClientID( const BTL_SVFLOW_WORK* wk );

extern u8 BTL_SVCL_GetNumActPoke( SVCL_WORK* clwk );
extern const BTL_ACTION_PARAM* BTL_SVCL_GetPokeAction( SVCL_WORK* clwk, u8 posIdx );


//------------------------------------------------
// イベントハンドラとの連絡関数
//------------------------------------------------
extern BtlRule BTL_SVFLOW_GetRule( BTL_SVFLOW_WORK* wk );
extern BtlCompetitor BTL_SVFLOW_GetCompetitor( BTL_SVFLOW_WORK* wk );
extern BtlPokePos BTL_SVFLOW_CheckExistFrontPokeID( BTL_SVFLOW_WORK* server, u8 pokeID );
extern BOOL BTL_SVFLOW_RECEPT_CheckExistTokuseiPokemon( BTL_SVFLOW_WORK* wk, PokeTokusei tokusei );
extern const BTL_POKEPARAM* BTL_SVFLOW_RECEPT_GetPokeParam( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern u8 BTL_SVFLOW_RECEPT_GetAllFrontPokeID( BTL_SVFLOW_WORK* wk, u8* dst );
extern u8 BTL_SVFLOW_RECEPT_GetAllOpponentFrontPokeID( BTL_SVFLOW_WORK* wk, u8 basePokeID, u8* dst );
extern u16 BTL_SVFLOW_GetTurnCount( BTL_SVFLOW_WORK* wk );
extern u8 BTL_SVFLOW_GetClientCoverPosCount( BTL_SVFLOW_WORK* wk, u8 pokeID );

extern void BTL_SERVER_RECEPT_TokuseiWinIn( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern void BTL_SERVER_RECEPT_TokuseiWinOut( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern void BTL_SERVER_RECTPT_StdMessage( BTL_SVFLOW_WORK* wk, u16 msgID );
extern void BTL_SERVER_RECTPT_SetMessage( BTL_SVFLOW_WORK* wk, u16 msgID, u8 pokeID );
extern void BTL_SERVER_RECTPT_SetMessageEx( BTL_SVFLOW_WORK* wk, u16 msgID, u8 pokeID, int arg );
extern void BTL_SERVER_RECEPT_RankDownEffect( BTL_SVFLOW_WORK* wk, BtlExPos exPos, BppValueID statusType, u8 volume, BOOL fAlmost );
extern void BTL_SERVER_RECEPT_RankUpEffect( BTL_SVFLOW_WORK* wk, BtlExPos exPos, BppValueID statusType, u8 volume, BOOL fAlmost );
extern void BTL_SERVER_RECEPT_HP_Add( BTL_SVFLOW_WORK* wk, u8 PokeID, int value );
extern void BTL_SVFLOW_RECEPT_ChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather );
extern void BTL_SVFLOW_RECEPT_CurePokeSick( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern void BTL_SVFLOW_RECEPT_CureWazaSick( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaSick sick );
extern void BTL_SVFLOW_RECEPT_CantEscapeAdd( BTL_SVFLOW_WORK* wk, u8 pokeID, BtlCantEscapeCode code );
extern void BTL_SVFLOW_RECEPT_CantEscapeSub( BTL_SVFLOW_WORK* wk, u8 pokeID, BtlCantEscapeCode code );
extern void BTL_SVFLOW_RECEPT_TraceTokusei( BTL_SVFLOW_WORK* wk, u8 pokeID, u8 targetPokeID );
extern HEAPID BTL_SVFLOW_RECEPT_GetHeapID( BTL_SVFLOW_WORK* wk );
extern u32 BTL_SVFLOW_SimulationDamage( BTL_SVFLOW_WORK* flowWk, u8 atkPokeID, u8 defPokeID, WazaID waza, BOOL fAffinity, BOOL fCritical );
extern BtlLandForm BTL_SVFLOW_GetLandForm( BTL_SVFLOW_WORK* wk );
extern const BTL_PARTY* BTL_SVFLOW_GetPartyData( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern BtlPokePos BTL_SVFLOW_PokeIDtoPokePos( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern BtlPokePos BTL_SVFLOW_ReqWazaTargetAuto( BTL_SVFLOW_WORK* wk, u8 pokeID, WazaID waza );
extern u8 BTL_SVFLOW_PokePosToPokeID( BTL_SVFLOW_WORK* wk, u8 pokePos );
extern BOOL BTL_SVFLOW_GetThisTurnAction( BTL_SVFLOW_WORK* wk, u8 pokeID, BTL_ACTION_PARAM* dst );
extern void* BTL_SVFLOW_GetHandlerTmpWork( BTL_SVFLOW_WORK* wk );
extern WazaID BTL_SVFLOW_GetPrevExeWaza( BTL_SVFLOW_WORK* wk );

//----------------------------
extern void BTL_SVFLOW_AddBonusMoney( BTL_SVFLOW_WORK* wk, u32 volume );
extern BOOL BTL_SVFLOW_IsStandAlonePlayerSide( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern BtlPokePos BTL_SVFLOW_GetCapturedPokePos( const BTL_SVFLOW_WORK* wk );


/**
 * 特殊優先順コードA（ワザ優先順より上位：0～7に収まるように！）
 */
typedef enum {

  BTL_SPPRI_A_LOW     = 0,
  BTL_SPPRI_A_DEFAULT = 1,
  BTL_SPPRI_A_HIGH    = 2,

}BtlSpecialPriA;

/**
 * 特殊優先順コードB（ワザ優先順より下位：：0～7に収まるように！）
 */
typedef enum {

  BTL_SPPRI_B_LOW     = 0,
  BTL_SPPRI_B_DEFAULT = 1,
  BTL_SPPRI_B_HIGH    = 2,

}BtlSpPriA;

/**
 *  状態異常指定の拡張コード
 */
typedef enum {

  // 全ての WazaSick コードも対象となります

  WAZASICK_EX_POKEFULL = WAZASICK_MAX,  ///< 全ポケモン系異常が対象
  WAZASICK_EX_POKEFULL_PLUS,            ///< 全ポケモン系異常＋こんらんも対象

}BtlWazaSickEx;

/**
 *  状態異常化失敗コード
 */
typedef enum {

  BTL_ADDSICK_FAIL_NULL = 0,
  BTL_ADDSICK_FAIL_ALREADY,
  BTL_ADDSICK_FAIL_OTHER,

}BtlAddSickFailCode;


/**
 *  ハンドラ挙動（効果が表出するもの）
 */
typedef enum {

  BTL_HANDEX_USE_ITEM,      ///< アイテム使用

  /*--- 以下、アイテム使用以外の効果に使う ---*/
  BTL_HANDEX_TOKWIN_IN,     ///< とくせいウィンドウイン
  BTL_HANDEX_TOKWIN_OUT,    ///< とくせいウィンドウアウト
  BTL_HANDEX_MESSAGE,       ///< メッセージ表示
  BTL_HANDEX_RECOVER_HP,    ///< HP回復
  BTL_HANDEX_DRAIN,         ///< HP回復（ドレイン系）
  BTL_HANDEX_DAMAGE,        ///< ダメージを与える
  BTL_HANDEX_SHIFT_HP,      ///< HPを変更（ダメージ・回復として扱わない）
  BTL_HANDEX_RECOVER_PP,    ///< PP回復
  BTL_HANDEX_DECREMENT_PP,  ///< PP減少
  BTL_HANDEX_CURE_SICK,     ///< 状態異常を回復
  BTL_HANDEX_ADD_SICK,      ///< 状態異常にする
  BTL_HANDEX_RANK_EFFECT,   ///< ランク増減効果
  BTL_HANDEX_SET_RANK,      ///< ランクを指定地に強制書き換え
  BTL_HANDEX_RESET_RANK,    ///< ランク効果を全てフラットに
  BTL_HANDEX_RECOVER_RANK,  ///< マイナスランク効果のみをフラットに
  BTL_HANDEX_KILL,          ///< 瀕死にする
  BTL_HANDEX_CHANGE_TYPE,   ///< ポケモンのタイプを変える
  BTL_HANDEX_SET_TURNFLAG,  ///< ターンフラグセット
  BTL_HANDEX_RESET_TURNFLAG,///< ターンフラグ強制リセット
  BTL_HANDEX_SET_CONTFLAG,  ///< 継続フラグセット
  BTL_HANDEX_RESET_CONTFLAG,///< 継続フラグリセット
  BTL_HANDEX_SIDEEFF_REMOVE, ///< サイドエフェクト削除
  BTL_HANDEX_ADD_FLDEFF,    ///< フィールドエフェクト追加
  BTL_HANDEX_REMOVE_FLDEFF, ///< フィールドエフェクト追加
  BTL_HANDEX_POSEFF_ADD,    ///< 位置エフェクト追加
  BTL_HANDEX_CHANGE_TOKUSEI,///< とくせい書き換え
  BTL_HANDEX_SET_ITEM,      ///< アイテム書き換え
  BTL_HANDEX_EQUIP_ITEM,    ///< アイテム効果発動
  BTL_HANDEX_CONSUME_ITEM,  ///< 自分でアイテムを消費
  BTL_HANDEX_SWAP_ITEM,     ///< アイテム入れ替え
  BTL_HANDEX_UPDATE_WAZA,   ///< ワザ書き換え
  BTL_HANDEX_COUNTER,       ///< ポケモンカウンタ値書き換え
  BTL_HANDEX_DELAY_WAZADMG, ///< 時間差ワザダメージ
  BTL_HANDEX_QUIT_BATTLE,   ///< バトル離脱
  BTL_HANDEX_CHANGE_MEMBER, ///< メンバー交換
  BTL_HANDEX_BATONTOUCH,    ///< バトンタッチ（ランク効果等の引き継ぎ）
  BTL_HANDEX_ADD_SHRINK,    ///< ひるませる
  BTL_HANDEX_RELIVE,        ///< 生き返らせる

  BTL_HANDEX_MAX,

}BtlEventHandlerExhibition;

/**
 *  ハンドラ挙動ワーク共有ヘッダ
 */
typedef struct {
  u8    equip;             ///< ハンドラ挙動（BtlEventHandlerEquip）
  u8    userPokeID;        ///< ハンドラ主体のポケモンID
  u8    size;              ///< 構造体サイズ
  u8    tokwin_flag  : 1;  ///< 主体ポケモンのとくせい効果であることを表示する
  u8    failSkipFlag : 1;  ///< 直前の命令が失敗したらスキップされる
  u8    padding : 6;       ///< パディング
}BTL_HANDEX_PARAM_HEADER;

/**
 *  ハンドラ用メッセージ出力パラメータ
 */
typedef struct {

  u8    type;    ///< 文字列タイプ（ BtlStrType  (btl_string.h) )
  u8    argCnt;  ///< 引数の数
  u16   ID;      ///< 文字列ID
  int   args[ BTL_STR_ARG_MAX ];  ///< 引数

}BTL_HANDEX_STR_PARAMS;

static inline void HANDEX_STR_Setup( BTL_HANDEX_STR_PARAMS* param, BtlStrType type, u16 strID )
{
  param->type = type;
  param->ID = strID;
  param->argCnt = 0;
}
static inline void HANDEX_STR_AddArg( BTL_HANDEX_STR_PARAMS* param, int arg )
{
  if( param->argCnt < BTL_STR_ARG_MAX )
  {
    param->args[ param->argCnt++ ] = arg;
  }
}


typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
}BTL_HANDEX_PARAM_USE_ITEM;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
}BTL_HANDEX_PARAM_TOKWIN_INOUT;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 BTL_HANDEX_STR_PARAMS  str;
}BTL_HANDEX_PARAM_MESSAGE;


/**
 * HP回復処理
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u16  recoverHP;                     ///< 回復HP量
 u8   pokeID;                        ///< 対象ポケモンID
 BTL_HANDEX_STR_PARAMS     exStr;    ///< 成功時メッセージ
}BTL_HANDEX_PARAM_RECOVER_HP;

/**
 * HP吸収
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u16  recoverHP;                     ///< 回復HP量
 u8   recoverPokeID;                 ///< 回復対象ポケモンID
 u8   damagedPokeID;                 ///< ダメージを受けた側のポケモンID
 BTL_HANDEX_STR_PARAMS     exStr;    ///< 成功時メッセージ
}BTL_HANDEX_PARAM_DRAIN;

/**
 * ダメージを与える
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u8   poke_cnt;                      ///< 対象ポケモン数
 u8   pokeID[ BTL_POS_MAX ];         ///< 対象ポケモンID
 u16  damage[ BTL_POS_MAX ];         ///< ダメージ量
 BTL_HANDEX_STR_PARAMS     exStr;    ///< 成功時メッセージ
}BTL_HANDEX_PARAM_DAMAGE;

/**
 * HPを増減（ダメージ・回復とは見なされない）
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
  u8     poke_cnt;
  u8     pokeID[ BTL_POS_MAX ];
  int    volume[ BTL_POS_MAX ];
}BTL_HANDEX_PARAM_SHIFT_HP;

/**
 * PPを回復
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u8   volume;                        ///< PP量
 u8   pokeID;                        ///< 対象ポケモンID
 u8   wazaIdx;                       ///< 対象ワザインデックス
 BTL_HANDEX_STR_PARAMS     exStr;    ///< 成功時メッセージ
}BTL_HANDEX_PARAM_PP;

/**
 * 対象の状態異常を回復
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 BtlWazaSickEx   sickCode;           ///< 対応する状態異常コード（拡張可）
 u8   pokeID[ BTL_POS_MAX ];         ///< 対象ポケモンID
 u8   poke_cnt;                      ///< 対象ポケモン数
 u8   fStdMsgDisable;                ///< 成功時、標準メッセージを出力しない
 u8   fExMsg;                        ///< 成功時、指定メッセージを出力する
 u16  exStrID;
}BTL_HANDEX_PARAM_CURE_SICK;

/**
 * 対象を状態異常にする
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 WazaSick       sickID;              ///< 状態異常ID
 BPP_SICK_CONT  sickCont;            ///< 状態異常継続パラメータ
 u8   fAlmost;                       ///< ほぼ確定フラグ（特殊要因で失敗した場合に原因メッセージを表示する）
 u8   poke_cnt;                      ///< 対象ポケモン数
 u8   pokeID[ BTL_POS_MAX ];         ///< 対象ポケモンID
 u8   fExMsg;                        ///< 成功時、特殊メッセージ表示
 BTL_HANDEX_STR_PARAMS  exStr;       ///< 特殊メッセージ詳細
}BTL_HANDEX_PARAM_ADD_SICK;

/**
 * ランク効果（上昇、下降）
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 WazaRankEffect rankType;            ///< ランク増減種類
 s8   rankVolume;                    ///< ランク増減値(=0だと強制リセット）
 u8   fStdMsgDisable;                ///< 標準メッセージを表示しないフラグ
 u8   fAlmost;                       ///< ほぼ確定フラグ（特殊要因で失敗した場合に原因メッセージを表示する）
 u8   poke_cnt;                      ///< 対象ポケモン数
 u8   pokeID[ BTL_POS_MAX ];         ///< 対象ポケモンID
}BTL_HANDEX_PARAM_RANK_EFFECT;

/**
 * ランク効果を特定値にセット
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u8 pokeID;                          ///< 対象ポケモンID
 s8 attack;
 s8 defence;
 s8 sp_attack;
 s8 sp_defence;
 s8 agility;
}BTL_HANDEX_PARAM_SET_RANK;

/**
 * 対象の下がっているランク効果をフラットに戻す
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u8 pokeID;                          ///< 対象ポケモンID
}BTL_HANDEX_PARAM_RECOVER_RANK;

/**
 * 対象の全てのランク効果をフラットに戻す
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u8   poke_cnt;                      ///< 対象ポケモン数
 u8   pokeID[ BTL_POS_MAX ];         ///< 対象ポケモンID
}BTL_HANDEX_PARAM_RESET_RANK;

/**
 * 対象をひん死にする
 */
typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u32   pokeID;
}BTL_HANDEX_PARAM_KILL;

/**
 * タイプ変更処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  PokeTypePair    next_type;
  u8              pokeID;
}BTL_HANDEX_PARAM_CHANGE_TYPE;

/**
 * ターンフラグセット処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  BppTurnFlag     flag;
  u8              pokeID;
}BTL_HANDEX_PARAM_TURNFLAG;

/**
 * 永続フラグセット処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  BppContFlag     flag;
  u8              pokeID;
}BTL_HANDEX_PARAM_SET_CONTFLAG;

/**
 * サイドエフェクト追加処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  BtlSideEffect   effect;
  BPP_SICK_CONT   cont;
  u8              pokeID;
}BTL_HANDEX_PARAM_SIDE_EFFECT;

/**
 * サイドエフェクト除去処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u8              flags[ BTL_SIDEEFF_BITFLG_BYTES ];
  u8              side;
  u8              fExMsg;
  u16             exStrID;
}BTL_HANDEX_PARAM_SIDEEFF_REMOVE;

/**
 * フィールドエフェクト追加処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  BtlFieldEffect           effect;
  BPP_SICK_CONT            cont;
  u8                       sub_param;

  BTL_HANDEX_STR_PARAMS    exStr;

}BTL_HANDEX_PARAM_ADD_FLDEFF;

/**
 * フィールドエフェクト除去処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  BtlFieldEffect           effect;
}BTL_HANDEX_PARAM_REMOVE_FLDEFF;

/**
 * 位置エフェクト追加
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  BtlPosEffect             effect;
  BtlPokePos               pos;
  int                      param[ BTL_POSEFF_PARAM_MAX ];
  u8                       param_cnt;
}BTL_HANDEX_PARAM_POSEFF_ADD;

/**
 *  とくせい書き換え処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u16             tokuseiID;    ///< 書き換え後のとくせい（POKETOKUSEI_NULLならとくせいを消す）
  u8              pokeID;       ///< 対象ポケモンID
  BTL_HANDEX_STR_PARAMS  exStr; ///< 成功時メッセージ
}BTL_HANDEX_PARAM_CHANGE_TOKUSEI;

typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u16             itemID;       ///< 書き換え後のアイテム（ITEM_DUMMY_DATA ならアイテムを消す）
  u8              pokeID;       ///< 対象ポケモンID
  BTL_HANDEX_STR_PARAMS  exStr; ///< 成功時メッセージ
}BTL_HANDEX_PARAM_SET_ITEM;

/**
 *  アイテム入れ替え（ヘッダの使用ポケモンと）
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u8              pokeID;            ///< 対象ポケモンID
  u8              fSucceedMsg;
  u8              succeedStrArgCnt;
  u16             succeedStrID;
  int             succeedStrArgs[ BTL_STR_ARG_MAX ];
}BTL_HANDEX_PARAM_SWAP_ITEM;

/**
 *  アイテム効果を強制発動
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u8              pokeID;            ///< 対象ポケモンID
  u16             itemID;
}BTL_HANDEX_PARAM_EQUIP_ITEM;

/**
 * ワザ書き換え処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u8              pokeID;            ///< 対象ポケモンID
  u8              wazaIdx;
  u8              ppMax;             ///< PPMax値:0ならデフォルト
  u8              fPermanent;        ///< バトル後まで書き換えが残るフラグ
  WazaID          waza;
}BTL_HANDEX_PARAM_UPDATE_WAZA;

/**
 * カウンタ書き換え処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u8              pokeID;            ///< 対象ポケモンID
  u8              counterID;         ///< カウンタID（BppCounter)
  u8              value;             ///< カウンタに設定する値
}BTL_HANDEX_PARAM_COUNTER;

/**
 * 時間差ワザダメージ処理用ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u8                       attackerPokeID;
  u8                       targetPokeID;
  WazaID                   wazaID;
}BTL_HANDEX_PARAM_DELAY_WAZADMG;

/**
 * メンバー入れ替え処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  BTL_HANDEX_STR_PARAMS    exStr;   ///< 成功時メッセージ
  u8                       pokeID;
}BTL_HANDEX_PARAM_CHANGE_MEMBER;

/**
 * バトンタッチ処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u8  userPokeID;     ///< 状態を引き継がせるポケID
  u8  targetPokeID;   ///< 状態を引き継ぐポケID
}BTL_HANDEX_PARAM_BATONTOUCH;

/**
 * ひるませる処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u8  pokeID;     ///< 対象ポケID
  u8  per;        ///< 確率
}BTL_HANDEX_PARAM_ADD_SHRINK;

/**
 * 生き返らせる処理ワーク
 */
typedef struct {
  BTL_HANDEX_PARAM_HEADER  header;
  u8   pokeID;        ///< 対象ポケID
  u16  recoverHP;     ///< 回復HP量
  BTL_HANDEX_STR_PARAMS  exStr; ///< 成功時メッセージ
}BTL_HANDEX_PARAM_RELIVE;

extern void* BTL_SVFLOW_HANDLERWORK_Push( BTL_SVFLOW_WORK* wk, BtlEventHandlerExhibition eq_type, u8 userPokeID );
extern u8 BTL_SERVERFLOW_RECEPT_GetTargetPokeID( BTL_SVFLOW_WORK* wk, BtlExPos exPos, u8* dst_pokeID );




#endif
