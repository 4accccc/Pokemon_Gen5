#ifndef __BTL_MAIN_H__
#define __BTL_MAIN_H__

#include "battle/battle.h"

#include "btl_common.h"
#include "btl_pokeparam.h"

typedef struct _BTL_MAIN_MODULE		BTL_MAIN_MODULE;

typedef struct _BTL_PARTY		BTL_PARTY;


extern BtlRule BTL_MAIN_GetRule( const BTL_MAIN_MODULE* wk );
extern u8 BTL_MAIN_GetFrontMemberIdx( const BTL_MAIN_MODULE* wk, u8 clientID );
extern void BTL_MAIN_SetFrontMemberIdx( BTL_MAIN_MODULE* wk, u8 clientID, u8 memberIdx );
extern BTL_PARTY* BTL_MAIN_GetPartyData( BTL_MAIN_MODULE* wk, u32 clientID );
extern const BTL_PARTY* BTL_MAIN_GetPartyDataConst( const BTL_MAIN_MODULE* wk, u32 clientID );
extern u8 BTL_MAIN_GetOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID, u8 idx );
extern u8 BTL_MAIN_GetFriendClientID( const BTL_MAIN_MODULE* wk, u8 cliendID, u8 idx );
extern const BTL_POKEPARAM* BTL_MAIN_GetPokeParam( const BTL_MAIN_MODULE* wk, u8 pokeID );
extern const BTL_POKEPARAM* BTL_MAIN_GetFrontPokeDataConst( const BTL_MAIN_MODULE* wk, u8 clientID );
extern BTL_POKEPARAM* BTL_MAIN_GetFrontPokeData( BTL_MAIN_MODULE* wk, u8 clientID );
extern void BTL_MAIN_SyncServerCalcData( BTL_MAIN_MODULE* wk );


//-------------------------
extern u8 BTL_PARTY_GetMemberCount( const BTL_PARTY* party );
extern u8 BTL_PARTY_GetAliveMemberCount( const BTL_PARTY* party );
extern BTL_POKEPARAM* BTL_PARTY_GetMemberData( BTL_PARTY* party, u8 idx );
extern const BTL_POKEPARAM* BTL_PARTY_GetMemberDataConst( const BTL_PARTY* party, u8 idx );


//-------------------------------------------------------------------------------
/**
 *	特殊クライアントID指定子
 */
//-------------------------------------------------------------------------------
typedef enum {

	BTL_EXCLIENT_DEFAULT = 0,		///< 自分だけ
	BTL_EXCLIENT_ENEMY_ALL,			///< 相手全部
	BTL_EXCLIENT_ENEMY_RANDOM,		///< 相手ランダム１体
	BTL_EXCLIENT_WITHOUT_ME,		///< 自分以外全部

}BtlExClientType;

typedef u16 BtlExClientID;

static inline BtlExClientID  EXID_MAKE( BtlExClientType type, u8 clientID )
{
	return (type<<8) | clientID;
}

static inline u8 EXID_GET_TYPE( BtlExClientID exID )
{
	return (exID >> 8) & 0xff;
}

static inline u8 EXID_GET_CLIENTID( BtlExClientID exID )
{
	return exID & 0xff;
}



//=============================================================================================
/**
 * 特殊クライアントID指定子を、実際の対象クライアントIDに変換
 *
 * @param   wk						メインモジュールハンドラ
 * @param   exID					特殊クライアントID指定子
 * @param   dst						[out] 対象クライアントIDを格納するバッファ（BTL_CLIENT_MAX分が確保されていること）
 *
 * @retval  u8		対象クライアント数
 */
//=============================================================================================
extern u8 BTL_MAIN_ExpandClientID( const BTL_MAIN_MODULE* wk, BtlExClientID exID, u8* dst );




#endif
