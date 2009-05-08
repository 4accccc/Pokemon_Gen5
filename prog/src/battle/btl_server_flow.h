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

#include "btl_main.h"
#include "btl_adapter.h"
#include "btl_server.h"
#include "btl_server_cmd.h"
#include "btl_event.h"
#include "btl_string.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
typedef enum {
  SVFLOW_RESULT_DEFAULT = 0,  ///< そのまま続行
  SVFLOW_RESULT_POKE_CHANGE,  ///< ポケモン入れ替えフェーズ->バトル継続
  SVFLOW_RESULT_BTL_QUIT,     ///< バトル終了
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
  BTL_POKEPARAM*  member[ TEMOTI_POKEMAX ];
  BTL_POKEPARAM*  frontMember[ BTL_POSIDX_MAX ];
  u8        memberCount;
  u8        numCoverPos;
  u8        isLocalClient;
  u8        myID;
};

/**
 *  サーバフロー＆ハンドラから参照するワザパラメータ
 */

typedef struct {

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
  BTL_SERVER_CMD_QUE* que, u32 numClient, HEAPID heapID );

extern void BTL_SVFLOW_QuitSystem( BTL_SVFLOW_WORK* wk );

extern SvflowResult BTL_SVFLOW_Start( BTL_SVFLOW_WORK* wk );
extern SvflowResult BTL_SVFLOW_Start_AfterPokemonIn( BTL_SVFLOW_WORK* wk );
extern SvflowResult BTL_SVFLOW_StartAfterPokeSelect( BTL_SVFLOW_WORK* wk );



// server_flow -> server
extern SVCL_WORK* BTL_SERVER_GetClientWork( BTL_SERVER* server, u8 clientID );
extern SVCL_WORK* BTL_SERVER_GetClientWorkIfEnable( BTL_SERVER* server, u8 clientID );


extern u8 BTL_SVCL_GetNumActPoke( SVCL_WORK* clwk );
extern u8 BTL_SVCL_GetNumCoverPos( SVCL_WORK* clwk );
extern const BTL_ACTION_PARAM* BTL_SVCL_GetPokeAction( SVCL_WORK* clwk, u8 posIdx );
extern BTL_POKEPARAM* BTL_SVCL_GetFrontPokeData( SVCL_WORK* clwk, u8 posIdx );


//------------------------------------------------
// イベントハンドラとの連絡関数
//------------------------------------------------
extern BtlPokePos BTL_SVFLOW_CheckExistFrontPokeID( BTL_SVFLOW_WORK* server, u8 pokeID );
extern BOOL BTL_SVFLOW_RECEPT_CheckExistTokuseiPokemon( BTL_SVFLOW_WORK* wk, PokeTokusei tokusei );
extern const BTL_POKEPARAM* BTL_SVFLOW_RECEPT_GetPokeParam( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern u8 BTL_SVFLOW_RECEPT_GetAllFrontPokeID( BTL_SVFLOW_WORK* wk, u8* dst );
extern u8 BTL_SVFLOW_RECEPT_GetAllOpponentFrontPokeID( BTL_SVFLOW_WORK* wk, u8 basePokeID, u8* dst );
extern u16 BTL_SVFLOW_GetTurnCount( BTL_SVFLOW_WORK* wk );

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
extern void BTL_SVFLOW_RECEPT_AddSick( BTL_SVFLOW_WORK* wk, u8 targetPokeID, u8 attackPokeID,
  WazaSick sick, BPP_SICK_CONT contParam, BOOL fAlmost );
extern void BTL_SVFLOW_RECEPT_ChangePokeType( BTL_SVFLOW_WORK* wk, u8 pokeID, PokeType type );
extern void BTL_SVFLOW_RECEPT_ChangePokeForm( BTL_SVFLOW_WORK* wk, u8 pokeID, u8 formNo );
extern void BTL_SVFLOW_RECEPT_CantEscapeAdd( BTL_SVFLOW_WORK* wk, u8 pokeID, BtlCantEscapeCode code );
extern void BTL_SVFLOW_RECEPT_CantEscapeSub( BTL_SVFLOW_WORK* wk, u8 pokeID, BtlCantEscapeCode code );
extern void BTL_SVFLOW_RECEPT_TraceTokusei( BTL_SVFLOW_WORK* wk, u8 pokeID, u8 targetPokeID );
extern HEAPID BTL_SVFLOW_RECEPT_GetHeapID( BTL_SVFLOW_WORK* wk );


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
  BTL_HANDEX_MESSAGE_STD,   ///< STDメッセージ表示
  BTL_HANDEX_MESSAGE_SET,   ///< SETメッセージ表示
  BTL_HANDEX_RECOVER_HP,    ///< HP回復
  BTL_HANDEX_RECOVER_PP,    ///< PP回復
  BTL_HANDEX_CURE_SICK,     ///< 状態異常を回復
  BTL_HANDEX_ADD_SICK,      ///< 状態異常にする
  BTL_HANDEX_RANK_EFFECT,   ///< ランク増減効果
  BTL_HANDEX_RECOVER_RANK,  ///< マイナスランクをフラットに戻す
  BTL_HANDEX_DAMAGE,        ///< ダメージを与える

  BTL_HANDEX_MAX,

}BtlEventHandlerExhibition;

/**
 *  ハンドラ挙動ワーク共有ヘッダ
 */
typedef struct {
  u8    equip;            ///< ハンドラ挙動（BtlEventHandlerEquip）
  u8    userPokeID;       ///< ハンドラ主体のポケモンID
  u8    size;             ///< 構造体サイズ
  u8    tokwin_flag  : 1; ///< 主体ポケモンのとくせい効果であることを表示する
  u8    _padding     : 7; ///< パディング
}BTL_HANDEX_PARAM_HEADER;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
}BTL_HANDEX_PARAM_USE_ITEM;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
}BTL_HANDEX_PARAM_TOKWIN_INOUT;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 int  args[ BTL_STR_ARG_MAX ];
 u16  strID;
}BTL_HANDEX_PARAM_MESSAGE;


typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u16  recoverHP;                     ///< 回復HP量
 u8   pokeID;                        ///< 対象ポケモンID
}BTL_HANDEX_PARAM_RECOVER_HP;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u8   recoverPP;                     ///< 回復PP量
 u8   pokeID;                        ///< 対象ポケモンID
 u8   wazaIdx;                       ///< 対象ワザインデックス
}BTL_HANDEX_PARAM_RECOVER_PP;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 WazaSick   sickID;                  ///< 対応する状態異常ID（WAZASICK_NULLでオールマイティ）
 u8   pokeID;                        ///< 対象ポケモンID
}BTL_HANDEX_PARAM_CURE_SICK;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 WazaSick       sickID;              ///< 状態異常ID
 BPP_SICK_CONT  sickCont;            ///< 状態異常継続パラメータ
 u8   fAlmost;                       ///< ほぼ確定フラグ（特殊要因で失敗した場合に原因メッセージを表示する）
 u8   poke_cnt;                      ///< 対象ポケモン数
 u8   pokeID[ BTL_POS_MAX ];         ///< 対象ポケモンID
 u8   fExMsg;                        ///< 成功時、特殊メッセージ表示
 u8   exStrArgCnt;                   ///< 成功時の特殊メッセージ引数の数
 u16  exStrID;                       ///< 成功時の特殊メッセージID
 int  exStrArgs[ BTL_STR_ARG_MAX ];  ///< 成功時の特殊メッセージ引数
}BTL_HANDEX_PARAM_ADD_SICK;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 WazaRankEffect rankType;            ///< ランク増減種類
 s8   rankVolume;                    ///< ランク増減値
 u8   fAlmost;                       ///< ほぼ確定フラグ（特殊要因で失敗した場合に原因メッセージを表示する）
 u8   poke_cnt;                      ///< 対象ポケモン数
 u8   pokeID[ BTL_POS_MAX ];         ///< 対象ポケモンID
}BTL_HANDEX_PARAM_RANK_EFFECT;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u8 pokeID;                          ///< 対象ポケモンID
}BTL_HANDEX_PARAM_RECOVER_RANK;

typedef struct {
 BTL_HANDEX_PARAM_HEADER   header;   ///< 共有ヘッダ
 u8   poke_cnt;                      ///< 対象ポケモン数
 u8   pokeID[ BTL_POS_MAX ];         ///< 対象ポケモンID
 u16  damage[ BTL_POS_MAX ];         ///< ダメージ量
}BTL_HANDEX_PARAM_DAMAGE;

extern void* BTL_SVFLOW_HANDLERWORK_Push( BTL_SVFLOW_WORK* wk, BtlEventHandlerExhibition eq_type, u8 userPokeID );



#endif
