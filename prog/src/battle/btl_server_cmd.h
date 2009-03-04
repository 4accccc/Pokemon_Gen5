//=============================================================================================
/**
 * @file	btl_server_cmd.h
 * @brief	ポケモンWB バトルシステム	サーバコマンド生成，解釈処理
 * @author	taya
 *
 * @date	2008.10.30	作成
 */
//=============================================================================================
#ifndef __BTL_SERVER_CMD_H__
#define __BTL_SERVER_CMD_H__

#include "btl_string.h"

enum {
	BTL_SERVER_CMD_QUE_SIZE = 384,
	BTL_SERVERCMD_ARG_MAX = 8,
};




//--------------------------------------------------------------
/**
 *		サーバコマンド
 */
//--------------------------------------------------------------
typedef enum {
	SC_NULL = 0,

	SC_OP_HP_MINUS,			///< 【計算】HPマイナス  [ClientID, マイナス量]
	SC_OP_HP_PLUS,			///< 【計算】HPプラス    [ClientID, プラス量]
	SC_OP_PP_MINUS,			///< 【計算】PPマイナス  [ClientID, マイナス量]
	SC_OP_PP_PLUS,			///< 【計算】PPプラス    [ClientID, プラス量]
	SC_OP_RANK_UP,			///< 【計算】ステータスランクアップ  [ClientID, StatusType, プラス量]
	SC_OP_RANK_DOWN,		///< 【計算】ステータスランクダウン  [ClientID, StatusType, マイナス量]
	SC_OP_SICK_SET,			///< 【計算】状態異常 [PokeID, Sick, Turn]
	SC_OP_WAZASICK_TURNCHECK,
	SC_ACT_WAZA_EFFECT,
	SC_ACT_WAZA_DMG,		///< 【ワザ発動：ダメージ】[ AtClient, DefClient, wazaIdx, Affinity ]
	SC_ACT_WAZA_DMG_DBL,
	SC_ACT_RANKUP,			///< 【ランクアップ効果】 ○○の×××があがった！[ ClientID, statusType, volume ]
	SC_ACT_RANKDOWN,		///< 【ランクダウン効果】 ○○の×××がさがった！[ ClientID, statusType, volume ]
	SC_ACT_DEAD,				///< 【ポケモンひんし】[ ClientID ]
	SC_ACT_MEMBER_OUT,	///< 【ポケモン退場】[ ClientID, memberIdx ]
	SC_ACT_MEMBER_IN,		///< 【ポケモンイン】[ ClientID, posIdx, memberIdx ]
	SC_ACT_SICK_DMG,		///<  アクション／ターンチェック時の状態異常ダメージ
	SC_TOKWIN_IN,				///< とくせいウィンドウ表示イン [ClientID]
	SC_TOKWIN_OUT,			///< とくせいウィンドウ表示アウト [ClientID]

	SC_MSG_WAZA,				///< ワザメッセージ表示[ ClientID, wazaIdx ]
	SC_MSG_STD,					///< メッセージ表示 [MsgID, ClientID, numArgs, arg1, arg2, ... ]
	SC_MSG_SET,					///< メッセージ表示 [MsgID, ClientID, numArgs, arg1, arg2, ... ]

	SC_MAX,
}ServerCmd;

//--------------------------------------------------------------
/**
 *		サーバキュー構造
 */
//--------------------------------------------------------------
typedef struct {
	u16		writePtr;
	u16		readPtr;
	u8		buffer[BTL_SERVER_CMD_QUE_SIZE];
}BTL_SERVER_CMD_QUE;

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
static inline void SCQUE_Init( BTL_SERVER_CMD_QUE* que )
{
	que->writePtr = 0;
	que->readPtr = 0;
}
static inline void SCQUE_Setup( BTL_SERVER_CMD_QUE* que, const void* data, u16 dataLength )
{
	GF_ASSERT(dataLength<BTL_SERVER_CMD_QUE_SIZE);

	GFL_STD_MemCopy32( data, que->buffer, dataLength );
	que->writePtr = dataLength;
	que->readPtr = 0;
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
static inline void scque_put1byte( BTL_SERVER_CMD_QUE* que, u8 data )
{
	GF_ASSERT(que->writePtr < BTL_SERVER_CMD_QUE_SIZE);
	que->buffer[ que->writePtr++ ] = data;
}
static inline void scque_put2byte( BTL_SERVER_CMD_QUE* que, u16 data )
{
	GF_ASSERT(que->writePtr < (BTL_SERVER_CMD_QUE_SIZE-1));
	que->buffer[ que->writePtr++ ] = (data >> 8)&0xff;
	que->buffer[ que->writePtr++ ] = (data & 0xff);
}
static inline void scque_put3byte( BTL_SERVER_CMD_QUE* que, u32 data )
{
	GF_ASSERT(que->writePtr < (BTL_SERVER_CMD_QUE_SIZE-2));
	que->buffer[ que->writePtr++ ] = (data >> 16)&0xff;
	que->buffer[ que->writePtr++ ] = (data >> 8)&0xff;
	que->buffer[ que->writePtr++ ] = (data & 0xff);
}
static inline u8 scque_read1byte( BTL_SERVER_CMD_QUE* que )
{
	GF_ASSERT(que->readPtr < que->writePtr);
	return que->buffer[ que->readPtr++ ];
}
static inline u16 scque_read2byte( BTL_SERVER_CMD_QUE* que )
{
	GF_ASSERT(que->readPtr < (que->writePtr-1));
	{
		u16 data = ( (que->buffer[que->readPtr] << 8) | que->buffer[que->readPtr+1] );
		que->readPtr += 2;
		return data;
	}
}
static inline u32 scque_read3byte( BTL_SERVER_CMD_QUE* que )
{
	GF_ASSERT(que->readPtr < (que->writePtr-2));
	{
		u32 data = ( (que->buffer[que->readPtr]<<16) | (que->buffer[que->readPtr+1]<<8) | (que->buffer[que->readPtr]) );
		que->readPtr += 3;
		return data;
	}
}

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
static inline BOOL SCQUE_IsFinishRead( const BTL_SERVER_CMD_QUE* que )
{
	return que->readPtr == que->writePtr;
}


//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
extern void SCQUE_PUT_Common( BTL_SERVER_CMD_QUE* que, ServerCmd cmd, ... );

//---------------------------------------------
static inline void SCQUE_PUT_OP_HpMinus( BTL_SERVER_CMD_QUE* que, u8 pokeID, u16 value )
{
	SCQUE_PUT_Common( que, SC_OP_HP_MINUS, pokeID, value );
}

static inline void SCQUE_PUT_OP_HpPlus( BTL_SERVER_CMD_QUE* que, u8 pokeID, u16 value )
{
	SCQUE_PUT_Common( que, SC_OP_HP_PLUS, pokeID, value );
}

static inline void SCQUE_PUT_OP_PPMinus( BTL_SERVER_CMD_QUE* que, u8 pokeID, u8 wazaIdx, u8 value )
{
	SCQUE_PUT_Common( que, SC_OP_PP_MINUS, pokeID, wazaIdx, value );
}

static inline void SCQUE_PUT_OP_PPPlus( BTL_SERVER_CMD_QUE* que, u8 pokeID, u8 wazaIdx, u8 value )
{
	SCQUE_PUT_Common( que, SC_OP_PP_PLUS, pokeID, wazaIdx, value );
}

static inline void SCQUE_PUT_OP_RankUp( BTL_SERVER_CMD_QUE* que, u8 pokeID, u8 statusType, u8 volume )
{
	SCQUE_PUT_Common( que, SC_OP_RANK_UP, pokeID, statusType, volume );
}

static inline void SCQUE_PUT_OP_RankDown( BTL_SERVER_CMD_QUE* que, u8 pokeID, u8 statusType, u8 volume )
{
	SCQUE_PUT_Common( que, SC_OP_RANK_DOWN, pokeID, statusType, volume );
}

static inline void SCQUE_PUT_OP_SetSick( BTL_SERVER_CMD_QUE* que, u8 pokeID, u16 sick, u8 turn )
{
	SCQUE_PUT_Common( que, SC_OP_SICK_SET, pokeID, turn, sick );
}

static inline void SCQUE_PUT_OP_WazaSickTurnCheck( BTL_SERVER_CMD_QUE* que, u8 pokeID )
{
	SCQUE_PUT_Common( que, SC_OP_WAZASICK_TURNCHECK, pokeID );
}
//---------------------------------------------
static inline void SCQUE_PUT_ACT_WazaEffect( BTL_SERVER_CMD_QUE* que, u8 atPokeID, u8 defPokeID, u16 waza )
{
	SCQUE_PUT_Common( que, SC_ACT_WAZA_EFFECT, atPokeID, defPokeID, waza );
}

static inline void SCQUE_PUT_ACT_WazaDamage( BTL_SERVER_CMD_QUE* que, u8 defPokeID, u8 affinity, u16 damage )
{
	SCQUE_PUT_Common( que, SC_ACT_WAZA_DMG, defPokeID, affinity, damage );
}

static inline void SCQUE_PUT_ACT_WazaDamageDbl( BTL_SERVER_CMD_QUE* que, u8 defPokeID1, u8 defPokeID2, u16 damage1, u16 damage2, u8 aff  )
{
	SCQUE_PUT_Common( que, SC_ACT_WAZA_DMG_DBL, defPokeID1, defPokeID2, aff, damage1, damage2 );
}

static inline void SCQUE_PUT_ACT_RankUp( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 statusType, u8 volume )
{
	SCQUE_PUT_Common( que, SC_ACT_RANKUP, clientID, statusType, volume );
}

static inline void SCQUE_PUT_ACT_RankDown( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 statusType, u8 volume )
{
	SCQUE_PUT_Common( que, SC_ACT_RANKDOWN, clientID, statusType, volume );
}

static inline void SCQUE_PUT_ACT_Dead( BTL_SERVER_CMD_QUE* que, u8 pokeID )
{
	SCQUE_PUT_Common( que, SC_ACT_DEAD, pokeID );
}
// メンバー入れ替えデータ
static inline void SCQUE_PUT_ACT_MemberOut( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 memberIdx )
{
	SCQUE_PUT_Common( que, SC_ACT_MEMBER_OUT, clientID, memberIdx );
}
static inline void SCQUE_PUT_ACT_MemberIn( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 posIdx, u8 memberIdx )
{
	SCQUE_PUT_Common( que, SC_ACT_MEMBER_IN, clientID, posIdx, memberIdx );
}

static inline void SCQUE_PUT_SickDamage( BTL_SERVER_CMD_QUE* que, u8 pokeID, u8 sick, u8 damage )
{
	SCQUE_PUT_Common( que, SC_ACT_SICK_DMG, pokeID, sick, damage );
}

static inline void SCQUE_PUT_TOKWIN_IN( BTL_SERVER_CMD_QUE* que, u8 btlPos )
{
	SCQUE_PUT_Common( que, SC_TOKWIN_IN, btlPos );
}
static inline void SCQUE_PUT_TOKWIN_OUT( BTL_SERVER_CMD_QUE* que, u8 btlPos )
{
	SCQUE_PUT_Common( que, SC_TOKWIN_OUT, btlPos );
}

static inline void SCQUE_PUT_MSG_WAZA( BTL_SERVER_CMD_QUE* que, u8 btlPos, u8 wazaIdx )
{
	SCQUE_PUT_Common( que, SC_MSG_WAZA, btlPos, wazaIdx );
}

//=====================================================
typedef u8 ScMsgArg;
enum { 
	MSGARG_TERMINATOR = 0xff,
};

#include <stdarg.h>

// ↓ 可変部分の最初の引数には必ず文字列IDを渡す。
extern void SCQUE_PUT_MsgImpl( BTL_SERVER_CMD_QUE* que, u8 scType, ... );
#define SCQUE_PUT_MSG_STD(que, ...)	SCQUE_PUT_MsgImpl( que, SC_MSG_STD, __VA_ARGS__, MSGARG_TERMINATOR )
#define SCQUE_PUT_MSG_SET(que, ...)	SCQUE_PUT_MsgImpl( que, SC_MSG_SET, __VA_ARGS__, MSGARG_TERMINATOR )


//=====================================================

extern ServerCmd SCQUE_Read( BTL_SERVER_CMD_QUE* que, int* args );

#endif
