//=============================================================================================
/**
 * @file	btl_server.h
 * @brief	ポケモンWB バトルシステム	サーバモジュール
 * @author	taya
 *
 * @date	2008.09.06	作成
 */
//=============================================================================================
#ifndef __BTL_SERVER_H__
#define __BTL_SERVER_H__

#include "btl_common.h"
#include "btl_main.h"
#include "btl_adapter.h"

typedef struct _BTL_SERVER		BTL_SERVER;


typedef enum {

	SC_RES_CONTINUE,		///< 次のターンへ。
	SC_RES_WAIT_CHANGE,		///< 入れ替え待ち
	SC_RES_ESCAPE,			///< どちらかが逃げて終了
	SC_RES_CLOSE,			///< どちらかが勝って終了


}ServerCmdResult;


extern BTL_SERVER* BTL_SERVER_Create( BTL_MAIN_MODULE* mainWork, HEAPID heapID );
extern void BTL_SERVER_Delete( BTL_SERVER* wk );


extern void BTL_SERVER_AttachLocalClient( BTL_SERVER* server, BTL_ADAPTER* adapter, BTL_PARTY* party, u8 clientID, u8 numCoverPos );
extern void BTL_SERVER_ReceptionNetClient( BTL_SERVER* server, BtlCommMode commMode, GFL_NETHANDLE* netHandle, BTL_PARTY* party, u8 clientID, u8 numCoverPos );
extern void BTL_SERVER_Main( BTL_SERVER* server );
extern BtlPokePos BTL_SERVER_CheckExistFrontPokeID( BTL_SERVER* server, u8 pokeID );


// ワザ失敗理由
typedef enum {

	SCARG_WAZAFAIL_OTHER = 0,

	SCARG_WAZAFAIL_MAHI,
	SCARG_WAZAFAIL_HIRUMI,
	SCARG_WAZAFAIL_MEROMERO,
	SCARG_WAZAFAIL_CHOUHATU,


}ServerArgWazaFailReason;





static inline u8 BTL_SERVER_MakeArg8_44( u8 arg1, u8 arg2 )
{
	GF_ASSERT((arg1&0x0f)==0);
	GF_ASSERT((arg2&0x0f)==0);

	return ((arg1&0x0f)<<4) | (arg2&0x0f);
}



//------------------------------------------------
// イベントハンドラとの連絡関数
//------------------------------------------------

extern int BTL_SERVER_RECEPT_GetEventArg( const BTL_SERVER* server, u8 idx );
extern void BTL_SERVER_RECEPT_SetEventArg( BTL_SERVER* server, u8 idx, int value );

extern void BTL_SERVER_RECTPT_SetMessage( BTL_SERVER* server, u16 msgID, BtlPokePos pos );
extern void BTL_SERVER_RECEPT_TokuseiWinIn( BTL_SERVER* server, u8 pokeID );
extern void BTL_SERVER_RECEPT_TokuseiWinOut( BTL_SERVER* server, u8 pokeID );

extern void BTL_SERVER_RECEPT_RankDownEffect( BTL_SERVER* server, BtlExPos exPos, BppValueID statusType, u8 volume );
extern void BTL_SERVER_RECEPT_SetDecrementPP( BTL_SERVER* server, u8 volume );

#endif
