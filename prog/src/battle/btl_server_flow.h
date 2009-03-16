//=============================================================================================
/**
 * @file	btl_server_flow.h
 * @brief	ポケモンWB バトルシステム	サーバコマンド生成処理
 * @author	taya
 *
 * @date	2009.03.06	作成
 */
//=============================================================================================
#ifndef __BTL_SERVER_FLOW_H__
#define __BTL_SERVER_FLOW_H__

#include "btl_main.h"
#include "btl_adapter.h"
#include "btl_server.h"
#include "btl_server_cmd.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
typedef enum {
	SVFLOW_RESULT_DEFAULT = 0,	///< そのまま続行
	SVFLOW_RESULT_POKE_DEAD,		///< ポケモン死んだので入れ替えフェーズ->バトル継続
	SVFLOW_RESULT_BTL_QUIT,			///< バトル終了
}SvflowResult;


/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/

/**
 *	サーバ本体、サーバフロー双方から参照するクライアントパラメータ
 */

struct _SVCL_WORK {

	BTL_ADAPTER*		adapter;
	BTL_PARTY*			party;
	BTL_POKEPARAM*	member[ TEMOTI_POKEMAX ];
	BTL_POKEPARAM*	frontMember[ BTL_POSIDX_MAX ];
	u8				memberCount;
	u8				numCoverPos;
	u8				isLocalClient;
	u8				myID;
};


/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef struct _BTL_SVFLOW_WORK		BTL_SVFLOW_WORK;
typedef struct _SVCL_WORK					SVCL_WORK;


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
extern const BTL_POKEPARAM* BTL_SVFLOW_RECEPT_GetPokeParam( BTL_SVFLOW_WORK* wk, u8 pokeID );

extern void BTL_SERVER_RECEPT_TokuseiWinIn( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern void BTL_SERVER_RECEPT_TokuseiWinOut( BTL_SVFLOW_WORK* wk, u8 pokeID );
extern void BTL_SERVER_RECTPT_StdMessage( BTL_SVFLOW_WORK* wk, u16 msgID );
extern void BTL_SERVER_RECTPT_SetMessage( BTL_SVFLOW_WORK* wk, u16 msgID, BtlPokePos pokePos );
extern void BTL_SERVER_RECEPT_RankDownEffect( BTL_SVFLOW_WORK* wk, BtlExPos exPos, BppValueID statusType, u8 volume );
extern void BTL_SERVER_RECEPT_RankUpEffect( BTL_SVFLOW_WORK* wk, BtlExPos exPos, BppValueID statusType, u8 volume );
extern void BTL_SVFLOW_RECEPT_ChangeWeather( BTL_SVFLOW_WORK* wk, BtlWeather weather );


#endif
