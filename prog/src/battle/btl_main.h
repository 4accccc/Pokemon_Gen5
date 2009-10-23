#ifndef __BTL_MAIN_H__
#define __BTL_MAIN_H__

#include "battle/battle.h"

#include "btl_common.h"
#include "btl_pokeparam.h"

typedef struct _BTL_MAIN_MODULE   BTL_MAIN_MODULE;

typedef struct _BTL_PARTY           BTL_PARTY;
typedef struct _BTL_POKE_CONTAINER  BTL_POKE_CONTAINER;

extern BtlRule BTL_MAIN_GetRule( const BTL_MAIN_MODULE* wk );
extern BtlCompetitor BTL_MAIN_GetCompetitor( const BTL_MAIN_MODULE* wk );
extern BtlCommMode BTL_MAIN_GetCommMode( const BTL_MAIN_MODULE* wk );
extern BtlPokePos BTL_MAIN_GetClientPokePos( const BTL_MAIN_MODULE* wk, u8 clientID, u8 posIdx );
extern BtlPokePos BTL_MAIN_GetOpponentPokePos( const BTL_MAIN_MODULE* wk, BtlPokePos basePos, u8 idx );
extern BtlPokePos BTL_MAIN_GetNextPokePos( const BTL_MAIN_MODULE* wk, BtlPokePos basePos );
extern BtlPokePos BTL_MAIN_PokeIDtoPokePos( const BTL_MAIN_MODULE* wk, const BTL_POKE_CONTAINER* pokeCon, u8 pokeID );
extern BOOL BTL_MAIN_IsOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID1, u8 clientID2 );
extern u8 BTL_MAIN_BtlPosToClientID( const BTL_MAIN_MODULE* wk, BtlPokePos pos );
extern u8 BTL_MAIN_BtlPosToViewPos( const BTL_MAIN_MODULE* wk, BtlPokePos pos );
extern BtlSide BTL_MAIN_GetClientSide( const BTL_MAIN_MODULE* wk, u8 clientID );
extern BtlEscapeMode BTL_MAIN_GetEscapeMode( const BTL_MAIN_MODULE * wk );
extern BtlLandForm BTL_MAIN_GetLandForm( const BTL_MAIN_MODULE* wk );
extern BOOL BTL_MAIN_IsFriendPokeID( const BTL_MAIN_MODULE* wk, u8 pokeID1, u8 pokeID2 );
extern u8 BTL_MAINUTIL_PokeIDtoClientID( u8 pokeID );
extern const MYSTATUS* BTL_MAIN_GetPlayerStatus( const BTL_MAIN_MODULE* wk );
extern BOOL BTL_MAIN_IsServerMachine( BTL_MAIN_MODULE * wk );
extern BOOL BTL_MAIN_IsMultiMode( const BTL_MAIN_MODULE * wk );
extern BtlPokePos BTL_MAIN_GetEnablePosEnd( const BTL_MAIN_MODULE* wk );
extern POKEPARTY* BTL_MAIN_GetPlayerPokeParty( BTL_MAIN_MODULE* wk );
extern u8 BTL_MAIN_GetPlayerClientID( const BTL_MAIN_MODULE* wk );
extern u16 BTL_MAIN_GetClientTrainerType( const BTL_MAIN_MODULE* wk, u8 clientID );
extern void BTL_MAIN_NotifyCapturedPokePos( BTL_MAIN_MODULE* wk, BtlPokePos pos );

//-------------------------
extern void BTL_MAIN_AddBonusMoney( BTL_MAIN_MODULE* wk, u32 volume );


//-------------------------


//=========================================================
// for server only
//=========================================================
extern void BTL_MAIN_BtlPosToClientID_and_PosIdx( const BTL_MAIN_MODULE* wk, BtlPokePos pos, u8* clientID, u8* posIdx );
extern void BTL_MAIN_SyncServerCalcData( BTL_MAIN_MODULE* wk );

//=========================================================

extern u8 BTL_PARTY_GetMemberCount( const BTL_PARTY* party );
extern u8 BTL_PARTY_GetAliveMemberCount( const BTL_PARTY* party );
extern u8 BTL_PARTY_GetAliveMemberCountRear( const BTL_PARTY* party, u8 startIdx );

extern s16 BTL_PARTY_FindMember( const BTL_PARTY* party, const BTL_POKEPARAM* param );
//-------------------------
extern BTL_POKEPARAM* BTL_PARTY_GetMemberData( BTL_PARTY* party, u8 idx );
extern const BTL_POKEPARAM* BTL_PARTY_GetMemberDataConst( const BTL_PARTY* party, u8 idx );

extern void BTL_PARTY_SwapMembers( BTL_PARTY* party, u8 idx1, u8 idx2 );

//-------------------------------------------------------------------------------
/**
 *  POKEMON_CONTAINER 関連
 */
//-------------------------------------------------------------------------------
extern BTL_POKEPARAM* BTL_POKECON_GetFrontPokeData( BTL_POKE_CONTAINER* wk, BtlPokePos pos );
extern const BTL_POKEPARAM* BTL_POKECON_GetFrontPokeDataConst( const BTL_POKE_CONTAINER* wk, BtlPokePos pos );

extern BTL_POKEPARAM* BTL_POKECON_GetClientPokeData( BTL_POKE_CONTAINER* wk, u8 clientID, u8 memberIdx );
extern const BTL_POKEPARAM* BTL_POKECON_GetClientPokeDataConst( const BTL_POKE_CONTAINER* wk, u8 clientID, u8 memberIdx );

extern BTL_POKEPARAM* BTL_POKECON_GetPokeParam( BTL_POKE_CONTAINER* wk, u8 pokeID );
extern const BTL_POKEPARAM* BTL_POKECON_GetPokeParamConst( const BTL_POKE_CONTAINER* wk, u8 pokeID );

extern BTL_PARTY* BTL_POKECON_GetPartyData( BTL_POKE_CONTAINER* wk, u32 clientID );
extern const BTL_PARTY* BTL_POKECON_GetPartyDataConst( const BTL_POKE_CONTAINER* wk, u32 clientID );

extern BtlPokePos BTL_MAIN_ViewPosToBtlPos( const BTL_MAIN_MODULE* wk, u8 vpos );


//-------------------------------------------------------------------------------
/**
 *  サイドID関連
 */
//-------------------------------------------------------------------------------

extern BOOL BTL_MAINUTIL_IsFriendPokeID( u8 pokeID1, u8 pokeID2 );
extern BtlSide BTL_MAINUTIL_PokeIDtoSide( u8 pokeID );
extern BtlPokePos BTL_MAINUTIL_GetFriendPokePos( BtlPokePos basePos, u8 idx );
extern BtlPokePos BTL_MAINUTIL_GetOpponentPokePos( BtlRule rule, BtlPokePos basePos, u8 idx );
extern u32 BTL_MAIN_GetOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID, u8 idx );


static inline BtlSide BTL_MAINUTIL_GetOpponentSide( BtlSide side )
{
  GF_ASSERT(side < BTL_SIDE_MAX);
  return !side;
}

static inline BtlPokePos BTL_MAINUTIL_GetSidePos( BtlSide side, u8 idx )
{
  GF_ASSERT(side < BTL_SIDE_MAX);
  GF_ASSERT(idx < BTL_POSIDX_MAX);

  return (side&1) + idx*2;
}

static inline BtlSide BTL_MAINUTIL_PokeIDtoOpponentSide( u8 pokeID )
{
  BtlSide  side = BTL_MAINUTIL_PokeIDtoSide( pokeID );
  return BTL_MAINUTIL_GetOpponentSide( side );
}

//-------------------------------------------------------------------------------
/**
 *  特殊位置ID指定子
 */
//-------------------------------------------------------------------------------
typedef enum {

  BTL_EXPOS_DEFAULT = 0,    ///< 自分だけ
  BTL_EXPOS_ENEMY_ALL,      ///< 相手全部
  BTL_EXPOS_WITHOUT_ME,     ///< 自分以外全部
  BTL_EXPOS_MYSIDE_ALL,     ///< 味方全部
  BTL_EXPOS_ALL,            ///< 敵・味方全部

//  BTL_EXPOS_ENEMY_ALL

}BtlExPosType;

typedef u16 BtlExPos;

static inline BtlExPos  EXPOS_MAKE( BtlExPosType type, u8 basePos )
{
  return (type<<8) | basePos;
}

static inline u8 EXPOS_GET_TYPE( BtlExPos exPos )
{
  return (exPos >> 8) & 0xff;
}

static inline u8 EXPOS_GET_BASEPOS( BtlExPos exPos )
{
  return exPos & 0xff;
}

//-------------------------------------------------------------------------------
/**
 *  トリプルバトル専用
 */
//-------------------------------------------------------------------------------
/**
 *  トリプルバトル時の通常攻撃有効範囲
 */
typedef struct {
  u16  numEnemys;   ///< 範囲内にある敵の位置数
  u16  numFriends;  ///< 範囲内にある味方の位置数
  BtlPokePos  enemyPos[ BTL_POSIDX_MAX ];   ///< 敵位置ID（ numEnemys 分有効）
  BtlPokePos  friendPos[ BTL_POSIDX_MAX ];  ///< 味方位置ID（ numFriends 分有効／先頭は常に基準位置）
}BTL_TRIPLE_ATTACK_AREA;

extern const BTL_TRIPLE_ATTACK_AREA* BTL_MAINUTIL_GetTripleAttackArea( BtlPokePos pos );

/**
 *  トリプルバトル時にセンター位置かどうかを判定
 */
static inline BOOL BTL_MAINUTIL_IsTripleCenterPos( BtlPokePos pos )
{
  return (pos == BTL_POS_1ST_1) || (pos == BTL_POS_2ND_1);
}

//----------------------------------------------------------------------------------------------
/*
 *  セーブデータアクセス
 */
//----------------------------------------------------------------------------------------------
extern MYITEM_PTR BTL_MAIN_GetItemDataPtr( BTL_MAIN_MODULE* wk );
extern BAG_CURSOR* BTL_MAIN_GetBagCursorData( BTL_MAIN_MODULE* wk );



//=============================================================================================
/**
 * 特殊位置指定子を、実際の位置IDに変換
 *
 * @param   wk            メインモジュールハンドラ
 * @param   exPos         特殊ポケモン位置指定子
 * @param   dst           [out] 対象ポケモン位置を格納するバッファ（BTL_POSIDX_MAX分が確保されていること）
 *
 * @retval  u8    対象ポケモン数
 */
//=============================================================================================
extern u8 BTL_MAIN_ExpandBtlPos( const BTL_MAIN_MODULE* wk, BtlExPos exPos, u8* dst );


extern TrainerID BTL_MAIN_GetTrainerID( const BTL_MAIN_MODULE* wk );

extern u32 BTL_MAIN_GetClientTrainerID( const BTL_MAIN_MODULE* wk, u8 clientID );
extern const MYSTATUS* BTL_MAIN_GetClientPlayerData( const BTL_MAIN_MODULE* wk, u8 clientID );


#endif
