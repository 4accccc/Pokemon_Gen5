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
	BTL_SERVER_CMD_QUE_SIZE = 1024,
	BTL_SERVERCMD_ARG_MAX = 8,
};

//--------------------------------------------------------------
/**
 *		サーバコマンド
 */
//--------------------------------------------------------------
typedef enum {

	SC_OP_HP_MINUS=0,		///< 【計算】HPマイナス  [ClientID, マイナス量]
	SC_OP_HP_PLUS,			///< 【計算】HPプラス    [ClientID, プラス量]
	SC_OP_PP_MINUS,			///< 【計算】PPマイナス  [ClientID, マイナス量]
	SC_OP_PP_PLUS,			///< 【計算】PPプラス    [ClientID, プラス量]
	SC_OP_RANK_UP,			///< 【計算】ステータスランクアップ  [ClientID, StatusType, プラス量]
	SC_OP_RANK_DOWN,		///< 【計算】ステータスランクダウン  [ClientID, StatusType, マイナス量]

	SC_DATA_WAZA_EXE,		///< 【データセット】ワザ発動 [ AtClientID, wazaIdx, DefPokeNum, DefClientID1, ... ]
	SC_DATA_MEMBER_OUT,		///< 【データセット】フロントメンバーアウト[ CliendID ]
	SC_DATA_MEMBER_IN,		///< 【データセット】フロントメンバーイン[ ClientID, pokeIdx ]

	SC_ACT_WAZA_DMG,		///< 【ワザ発動：ダメージ】[ AtClient, DefClient, wazaIdx, Affinity ]
	SC_ACT_RANKUP,			///< 【ランクアップ効果】 ○○の×××があがった！[ ClientID, statusType, volume ]
	SC_ACT_RANKDOWN,		///< 【ランクダウン効果】 ○○の×××がさがった！[ ClientID, statusType, volume ]
	SC_ACT_DEAD,			///< 【ポケモンひんし】[ ClientID ]
	SC_ACT_MEMBER_IN,		///< 【ポケモンイン】[ ClientID, pokeIdx ]

	SC_TOKWIN_IN,			///< とくせいウィンドウ表示イン [ClientID]
	SC_TOKWIN_OUT,			///< とくせいウィンドウ表示アウト [ClientID]
	SC_MSG_STD,				///< メッセージ表示 [MsgID, ClientID, numArgs, arg1, arg2, ... ]
	SC_MSG_SET,				///< メッセージ表示 [MsgID, ClientID, numArgs, arg1, arg2, ... ]
	SC_MSG_WAZA,			///< ワザメッセージ表示[ ClientID, wazaIdx ]

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

//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
static inline BOOL SCQUE_IsFinishRead( const BTL_SERVER_CMD_QUE* que )
{
	return que->readPtr == que->writePtr;
}


//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------


//---------------------------------------------
static inline void SCQUE_PUT_OP_HpMinus( BTL_SERVER_CMD_QUE* que, u8 clientID, u16 value )
{
	scque_put2byte( que, SC_OP_HP_MINUS );
	scque_put1byte( que, clientID );
	scque_put2byte( que, value );
}
static inline void SCQUE_READ_OP_HpMinus( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read2byte( que );
}

static inline void SCQUE_PUT_OP_RankUp( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 statusType, u8 volume )
{
	scque_put2byte( que, SC_OP_RANK_UP );
	scque_put1byte( que, clientID );
	scque_put1byte( que, statusType );
	scque_put1byte( que, volume );
}
static inline void SCQUE_READ_OP_RankUp( BTL_SERVER_CMD_QUE* que, int *args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read1byte( que );
	args[2] = scque_read1byte( que );
}

static inline void SCQUE_PUT_OP_RankDown( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 statusType, u8 volume )
{
	scque_put2byte( que, SC_OP_RANK_DOWN );
	scque_put1byte( que, clientID );
	scque_put1byte( que, statusType );
	scque_put1byte( que, volume );
}
static inline void SCQUE_READ_OP_RankDown( BTL_SERVER_CMD_QUE* que, int *args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read1byte( que );
	args[2] = scque_read1byte( que );
}


static inline void SCQUE_PUT_DATA_WazaExe( BTL_SERVER_CMD_QUE* que, u8 atClientID, u8 wazaIdx, u8 numDefClients, u8 defClientID1, u8 defClientID2, u8 defClientID3 )
{
	scque_put2byte( que, SC_DATA_WAZA_EXE );
	scque_put1byte( que, atClientID );
	scque_put1byte( que, wazaIdx );
	scque_put1byte( que, numDefClients );
	scque_put1byte( que, defClientID1 );
	scque_put1byte( que, defClientID2 );
	scque_put1byte( que, defClientID3 );
}
static inline void SCQUE_READ_DATA_WazaExe( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read1byte( que );
	args[2] = scque_read1byte( que );
	args[3] = scque_read1byte( que );
	args[4] = scque_read1byte( que );
	args[5] = scque_read1byte( que );
}
// メンバー入れ替えデータ
static inline void SCQUE_PUT_DATA_MemberIn( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 memberIdx )
{
	scque_put2byte( que, SC_DATA_MEMBER_IN );
	scque_put1byte( que, clientID );
	scque_put1byte( que, memberIdx );
}
static inline void SCQUE_READ_DATA_MemberIn( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read1byte( que );
}

//---------------------------------------------
static inline void SCQUE_PUT_ACT_WazaDamage( BTL_SERVER_CMD_QUE* que, u8 atClientID, u8 defClientID, u16 damage, u8 wazaIdx, u8 affinity )
{
	scque_put2byte( que, SC_ACT_WAZA_DMG );
	scque_put1byte( que, atClientID );
	scque_put1byte( que, defClientID );
	scque_put2byte( que, damage );
	scque_put1byte( que, wazaIdx );
	scque_put1byte( que, affinity );
}
static inline void SCQUE_READ_ACT_WazaDamage( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read1byte( que );
	args[2] = scque_read2byte( que );
	args[3] = scque_read1byte( que );
	args[4] = scque_read1byte( que );
}
static inline void SCQUE_PUT_ACT_RankUp( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 statusType, u8 volume )
{
	scque_put2byte( que, SC_ACT_RANKUP );
	scque_put1byte( que, clientID );
	scque_put1byte( que, statusType );
	scque_put1byte( que, volume );
}
static inline void SCQUE_READ_ACT_RankUp( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read1byte( que );
	args[2] = scque_read1byte( que );
}
static inline void SCQUE_PUT_ACT_RankDown( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 statusType, u8 volume )
{
	scque_put2byte( que, SC_ACT_RANKDOWN );
	scque_put1byte( que, clientID );
	scque_put1byte( que, statusType );
	scque_put1byte( que, volume );
}
static inline void SCQUE_READ_ACT_RankDown( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read1byte( que );
	args[2] = scque_read1byte( que );
}

static inline void SCQUE_PUT_ACT_Dead( BTL_SERVER_CMD_QUE* que, u8 clientID )
{
	scque_put2byte( que, SC_ACT_DEAD );
	scque_put1byte( que, clientID );
}
static inline void SCQUE_READ_ACT_Dead( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
}

static inline void SCQUE_PUT_ACT_MemberIn( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 pokeIdx )
{
	scque_put2byte( que, SC_ACT_MEMBER_IN );
	scque_put1byte( que, clientID );
	scque_put1byte( que, pokeIdx );
}
static inline void SCQUE_READ_ACT_MemberIn( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read1byte( que );
}



static inline void SCQUE_PUT_TOKWIN_IN( BTL_SERVER_CMD_QUE* que, u8 clientID )
{
	scque_put2byte( que, SC_TOKWIN_IN );
	scque_put1byte( que, clientID );
}
static inline void SCQUE_READ_TOKWIN( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
}
static inline void SCQUE_PUT_TOKWIN_OUT( BTL_SERVER_CMD_QUE* que, u8 clientID )
{
	scque_put2byte( que, SC_TOKWIN_OUT );
	scque_put1byte( que, clientID );
}
//=====================================================
typedef u8 ScMsgArg;
enum { 
	MSGARG_TERMINATOR = 0xff,
};

#include <stdarg.h>

// ↓こいつらはいずれ消す
extern void SCQUE_PUT_MsgSpecial( BTL_SERVER_CMD_QUE* que, u16 strID, u8 arg1, u8 arg2 );
extern void SCQUE_PUT_MsgBody( BTL_SERVER_CMD_QUE* que, u16 strID, ... );
#define SCQUE_PUT_Msg(que, ...)	SCQUE_PUT_MsgBody(que, __VA_ARGS__, MSGARG_TERMINATOR)

// ↓ 可変部分の最初の引数には必ず文字列IDを渡す。
extern void SCQUE_PUT_MsgImpl( BTL_SERVER_CMD_QUE* que, u8 scType, ... );
#define SCQUE_PUT_MSG_STD(que, ...)	SCQUE_PUT_MsgImpl( que, SC_MSG_STD, __VA_ARGS__, MSGARG_TERMINATOR )
#define SCQUE_PUT_MSG_SET(que, ...)	SCQUE_PUT_MsgImpl( que, SC_MSG_SET, __VA_ARGS__, MSGARG_TERMINATOR )

extern void SCQUE_READ_Msg( BTL_SERVER_CMD_QUE* que, int* args );

static inline void SCQUE_PUT_MSG_WAZA( BTL_SERVER_CMD_QUE* que, u8 clientID, u8 wazaIdx )
{
	scque_put2byte( que, SC_MSG_WAZA );
	scque_put1byte( que, clientID );
	scque_put1byte( que, wazaIdx );
}

static inline void SCQUE_READ_MSG_WAZA( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read1byte( que );
	args[1] = scque_read1byte( que );
}

//=====================================================

static inline ServerCmd SCQUE_Read( BTL_SERVER_CMD_QUE* que, int* args )
{
	ServerCmd cmd = scque_read2byte( que );

	switch( cmd ){
	case SC_OP_HP_MINUS:		SCQUE_READ_OP_HpMinus( que, args ); break;
	case SC_OP_RANK_UP:			SCQUE_READ_OP_RankUp( que, args ); break;
	case SC_OP_RANK_DOWN:		SCQUE_READ_OP_RankDown( que, args ); break;
	case SC_DATA_WAZA_EXE:		SCQUE_READ_DATA_WazaExe( que, args ); break;
	case SC_DATA_MEMBER_IN:		SCQUE_READ_DATA_MemberIn( que, args ); break;
	case SC_ACT_WAZA_DMG:		SCQUE_READ_ACT_WazaDamage( que, args ); break;
	case SC_ACT_RANKUP:			SCQUE_READ_ACT_RankUp( que, args ); break;
	case SC_ACT_RANKDOWN:		SCQUE_READ_ACT_RankDown( que, args ); break;
	case SC_ACT_DEAD:			SCQUE_READ_ACT_Dead( que, args ); break;
	case SC_ACT_MEMBER_IN:		SCQUE_READ_ACT_MemberIn( que, args ); break;
	case SC_TOKWIN_IN:			SCQUE_READ_TOKWIN( que, args ); break;
	case SC_TOKWIN_OUT:			SCQUE_READ_TOKWIN( que, args ); break;
	case SC_MSG_STD:			SCQUE_READ_Msg( que, args ); break;
	case SC_MSG_SET:			SCQUE_READ_Msg( que, args ); break;
	case SC_MSG_WAZA:			SCQUE_READ_MSG_WAZA( que, args ); break;
	default:
		args[0] = scque_read1byte( que );
		break;
	}

	return cmd;
}

#endif
