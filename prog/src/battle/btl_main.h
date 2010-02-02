#ifndef __BTL_MAIN_H__
#define __BTL_MAIN_H__

#include "savedata/perapvoice.h"

#include "btl_main_def.h"

#include "battle/battle.h"

#include "btl_common.h"
#include "btl_pokeparam.h"


/*------------------------------------------------------------------------------*/
/* ルールなど基本情報                                                           */
/*------------------------------------------------------------------------------*/
extern BtlRule BTL_MAIN_GetRule( const BTL_MAIN_MODULE* wk );
extern BtlCompetitor BTL_MAIN_GetCompetitor( const BTL_MAIN_MODULE* wk );
extern BtlCommMode BTL_MAIN_GetCommMode( const BTL_MAIN_MODULE* wk );
extern BOOL BTL_MAIN_IsWazaEffectEnable( const BTL_MAIN_MODULE* wk );
extern BOOL BTL_MAIN_IsExpSeqEnable( const BTL_MAIN_MODULE* wk );
extern BtlEscapeMode BTL_MAIN_GetEscapeMode( const BTL_MAIN_MODULE * wk );
extern const MYSTATUS* BTL_MAIN_GetPlayerStatus( const BTL_MAIN_MODULE* wk );
extern BOOL BTL_MAIN_IsZukanRegistered( const BTL_MAIN_MODULE* wk, const BTL_POKEPARAM* bpp );
extern POKEPARTY* BTL_MAIN_GetPlayerPokeParty( BTL_MAIN_MODULE* wk );
extern POKEPARTY* BTL_MAIN_GetMultiPlayerPokeParty( BTL_MAIN_MODULE* wk );
extern const BTL_FIELD_SITUATION* BTL_MAIN_GetFieldSituation( const BTL_MAIN_MODULE* wk );
extern BOOL BTL_MAIN_IsClientNPC( const BTL_MAIN_MODULE* wk, u8 clientID );
extern BOOL BTL_MAIN_IsRecordEnable( const BTL_MAIN_MODULE* wk );
extern BOOL BTL_MAIN_GetDebugFlag( const BTL_MAIN_MODULE* wk, BtlDebugFlag flag );

/*------------------------------------------------------------------------------*/
/* クライアントID，ポケモンID，位置IDなどの情報＆相互変換                       */
/*------------------------------------------------------------------------------*/
extern BOOL BTL_MAIN_IsExistClient( const BTL_MAIN_MODULE* wk, u8 clientID );
extern BtlPokePos BTL_MAIN_GetClientPokePos( const BTL_MAIN_MODULE* wk, u8 clientID, u8 posIdx );
extern BtlPokePos BTL_MAIN_GetOpponentPokePos( const BTL_MAIN_MODULE* wk, BtlPokePos basePos, u8 idx );
extern BtlPokePos BTL_MAIN_GetNextPokePos( const BTL_MAIN_MODULE* wk, BtlPokePos basePos );
extern BtlPokePos BTL_MAIN_PokeIDtoPokePos( const BTL_MAIN_MODULE* wk, const BTL_POKE_CONTAINER* pokeCon, u8 pokeID );
extern BOOL BTL_MAIN_IsOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID1, u8 clientID2 );
extern u8 BTL_MAIN_BtlPosToClientID( const BTL_MAIN_MODULE* wk, BtlPokePos pos );
extern u32 BTL_MAIN_GetOpponentClientID( const BTL_MAIN_MODULE* wk, u8 clientID, u8 idx );
extern u8 BTL_MAIN_BtlPosToViewPos( const BTL_MAIN_MODULE* wk, BtlPokePos pos );
extern u8 BTL_MAIN_PokeIDtoViewPos( const BTL_MAIN_MODULE* wk, const BTL_POKE_CONTAINER* pokeCon, u8 pokeID );
extern BtlSide BTL_MAIN_GetClientSide( const BTL_MAIN_MODULE* wk, u8 clientID );
extern u8 BTL_MAINUTIL_PokeIDtoClientID( u8 pokeID );
extern BtlPokePos BTL_MAIN_GetFacedPokePos( const BTL_MAIN_MODULE* wk, BtlPokePos basePos );
extern BtlPokePos BTL_MAINUTIL_GetFacedPokePos( BtlRule rule, BtlPokePos pos );
extern u8 BTL_MAIN_GetClientCoverPosCount( const BTL_MAIN_MODULE* wk, u8 clientID );
extern u8 BTL_MAIN_GetPlayerMultiPos( const BTL_MAIN_MODULE* wk );

extern BtlPokePos BTL_MAIN_GetEnablePosEnd( const BTL_MAIN_MODULE* wk );
extern u16 BTL_MAIN_GetClientTrainerType( const BTL_MAIN_MODULE* wk, u8 clientID );
extern u8 BTL_MAIN_GetPlayerClientID( const BTL_MAIN_MODULE* wk );
extern u8 BTL_MAIN_GetEnemyClientID( const BTL_MAIN_MODULE* wk, u8 idx );
extern BtlSide BTL_MAIN_GetPlayerSide( const BTL_MAIN_MODULE* wk );

//-------------------------------------------------------------------------------
/**
 *  サイドID関連
 */
//-------------------------------------------------------------------------------
extern BOOL BTL_MAINUTIL_IsFriendPokeID( u8 pokeID1, u8 pokeID2 );
extern BtlSide BTL_MAINUTIL_PokeIDtoSide( u8 pokeID );
extern BtlPokePos BTL_MAINUTIL_GetFriendPokePos( BtlPokePos basePos, u8 idx );
extern BtlPokePos BTL_MAINUTIL_GetOpponentPokePos( BtlRule rule, BtlPokePos basePos, u8 idx );
extern BOOL BTL_MAIN_IsPlayerSide( const BTL_MAIN_MODULE* wk, BtlSide side );

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
static inline BtlSide BTL_MAINUTIL_PosToSide( BtlPokePos pos )
{
  return pos & 1;
}

/*------------------------------------------------------------------------------*/
/* フィールドへ引き継ぐ情報の取得・通知                                         */
/*------------------------------------------------------------------------------*/
extern void BTL_MAIN_NotifyCapturedPokePos( BTL_MAIN_MODULE* wk, BtlPokePos pos );
extern void BTL_MAIN_AddBonusMoney( BTL_MAIN_MODULE* wk, u32 volume );
extern void BTL_MAIN_ClientPokemonReflectToServer( BTL_MAIN_MODULE* wk, u8 pokeID );
extern u32 BTL_MAIN_GetBonusMoney( const BTL_MAIN_MODULE* wk );
extern BtlResult BTL_MAIN_ChecBattleResult( BTL_MAIN_MODULE* wk );


/*------------------------------------------------------------------------------*/
/* 通信・CONFIG等のシステム情報                                                 */
/*------------------------------------------------------------------------------*/
extern BOOL BTL_MAIN_IsMultiMode( const BTL_MAIN_MODULE * wk );
extern COMM_PLAYER_SUPPORT* BTL_MAIN_GetCommSupportHandle( const BTL_MAIN_MODULE* wk );
extern int BTL_MAIN_GetPrintWait( const BTL_MAIN_MODULE* wk );

//-------------------------



//=========================================================
// for server only
//=========================================================
extern void BTL_MAIN_BtlPosToClientID_and_PosIdx( const BTL_MAIN_MODULE* wk, BtlPokePos pos, u8* clientID, u8* posIdx );
extern u8   BTL_MAIN_BtlPosToPosIdx( const BTL_MAIN_MODULE* wk, BtlPokePos pos );
extern void BTL_MAIN_SyncServerCalcData( BTL_MAIN_MODULE* wk );

//=========================================================

extern u8 BTL_PARTY_GetMemberCount( const BTL_PARTY* party );
extern u8 BTL_PARTY_GetAliveMemberCount( const BTL_PARTY* party );
extern u8 BTL_PARTY_GetAliveMemberCountRear( const BTL_PARTY* party, u8 startIdx );

extern int BTL_PARTY_FindMember( const BTL_PARTY* party, const BTL_POKEPARAM* param );
extern int BTL_PARTY_FindMemberByPokeID( const BTL_PARTY* party, u8 pokeID );
extern void BTL_PARTY_SetFakeSrcMember( BTL_PARTY* party, u8 memberIdx );
//-------------------------
extern BTL_POKEPARAM* BTL_PARTY_GetMemberData( BTL_PARTY* party, u8 idx );
extern const BTL_POKEPARAM* BTL_PARTY_GetMemberDataConst( const BTL_PARTY* party, u8 idx );

extern void BTL_PARTY_SwapMembers( BTL_PARTY* party, u8 idx1, u8 idx2 );
extern void BTL_PARTY_RotateMembers( BTL_PARTY* party, BtlRotateDir dir, BTL_POKEPARAM** outPoke, BTL_POKEPARAM** inPoke );

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
extern BOOL BTL_MAIN_CheckFrontPoke( BTL_MAIN_MODULE* wk, const BTL_POKE_CONTAINER* pokeCon, u8 pokeID );


//-------------------------------------------------------------------------------
/**
 *  特殊位置ID指定子
 */
//-------------------------------------------------------------------------------
typedef enum {

  BTL_EXPOS_DEFAULT = 0,      ///< 自分だけ

  BTL_EXPOS_AREA_ENEMY,       ///< （攻撃範囲内の）相手全部
  BTL_EXPOS_AREA_OTHERS,      ///< （攻撃範囲内の）自分以外全部
  BTL_EXPOS_AREA_MYTEAM,      ///< （攻撃範囲内の）味方全部
  BTL_EXPOS_AREA_FRIENDS,     ///< （攻撃範囲内の）自分以外の味方全部
  BTL_EXPOS_AREA_ALL,         ///< （攻撃範囲内の）敵・味方全部

  BTL_EXPOS_FULL_ENEMY,       ///< （全フィールドの）相手ポケ全部
  BTL_EXPOS_FULL_FRIENDS,     ///< （全フィールドの）自分以外の味方ポケ全部
  BTL_EXPOS_FULL_ALL,         ///< （全フィールドの）ポケ全部


//  BTL_EXPOS_AREA_ENEMY

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

extern u8 BTL_MAIN_ExpandExistPokeID( const BTL_MAIN_MODULE* wk, BTL_POKE_CONTAINER* pokeCon, BtlExPos exPos,
     u8* pokeIDAry );

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
extern BOOL BTL_MAINUTIL_CheckTripleHitArea( BtlPokePos attackerPos, BtlPokePos targetPos );


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
extern void  BTL_MAIN_DecrementPlayerItem( const BTL_MAIN_MODULE* wk, u8 clientID, u16 itemID );
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



extern u32 BTL_MAIN_GetClientTrainerID( const BTL_MAIN_MODULE* wk, u8 clientID );
extern const MYSTATUS* BTL_MAIN_GetClientPlayerData( const BTL_MAIN_MODULE* wk, u8 clientID );


//----------------------------------------------------------------------------------------------
/*
 *  制限時間関連
 */
//----------------------------------------------------------------------------------------------
extern u32 BTL_MAIN_GetCommandLimitTime( const BTL_MAIN_MODULE* wk );
extern u32 BTL_MAIN_GetGameLimitTime( const BTL_MAIN_MODULE * wk );
extern BOOL BTL_MAIN_CheckGameLimitTimeOver( const BTL_MAIN_MODULE* wk );


#endif
