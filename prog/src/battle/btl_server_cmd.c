//=============================================================================================
/**
 * @file	btl_server_cmd.c
 * @brief	ポケモンWB バトルシステム	サーバコマンド生成，解釈処理
 * @author	taya
 *
 * @date	2008.10.30	作成
 */
//=============================================================================================
#include <stdarg.h>

#include "btl_common.h"
#include "btl_server_cmd.h"

//--------------------------------------------------------------
/**
 *		サーバコマンド引数型生成マクロ
 */
//--------------------------------------------------------------
// 引数の数, 引数の数ごとのインデックス:両方4bit
#define SC_ARGFMT(cnt, idx)		((((idx)&0x0f)<<4)|(cnt&0x0f))

// 引数型から引数の数を取得
static inline SC_ARGFMT_GetArgCount( u8 fmt )
{
	return fmt & 0x0f;
}

//--------------------------------------------------------------
/**
 *		サーバコマンド引数型定義
 */
//--------------------------------------------------------------
typedef enum {
	// 引数１個の型
	SC_ARGFMT_1byte = SC_ARGFMT(1,0),
	SC_ARGFMT_2byte = SC_ARGFMT(1,1),

	// 引数２個の型
	SC_ARGFMT_12byte = SC_ARGFMT(2,0),
	SC_ARGFMT_4_4bit = SC_ARGFMT(2,1),
	SC_ARGFMT_5_3bit = SC_ARGFMT(2,2),

	// 引数３個の型
	SC_ARGFMT_53bit_1byte = SC_ARGFMT(3,0),
	SC_ARGFMT_5_5_5bit    = SC_ARGFMT(3,1),
	SC_ARGFMT_5_5_14bit   = SC_ARGFMT(3,2),

	// 引数５個の型
	SC_ARGFMT_5_5_5bit_22byte = SC_ARGFMT(5,0),

	// メッセージ型（可変引数）
	SC_ARGFMT_MSG = SC_ARGFMT(15,15),

}ScArgFormat;


//--------------------------------------------------------------
/**
 *		サーバコマンドから引数の型を取得するためのテーブル
 *		（サーバコマンドと順序を一致させる必要がある）
 */
//--------------------------------------------------------------
static const u8 ServerCmdToFmtTbl[] = {
	0,
	SC_ARGFMT_12byte,						// SC_OP_HP_MINUS
	SC_ARGFMT_12byte,						// SC_OP_HP_PLUS
	SC_ARGFMT_53bit_1byte,			// SC_OP_PP_MINUS
	SC_ARGFMT_53bit_1byte,			// SC_OP_PP_PLUS
	SC_ARGFMT_53bit_1byte,			// SC_OP_RANK_UP
	SC_ARGFMT_53bit_1byte,			// SC_OP_RANK_DOWN
	SC_ARGFMT_5_5_14bit,				// SC_OP_SICK_SET
	SC_ARGFMT_1byte,						// SC_OP_WAZASICK_TURNCHECK
	SC_ARGFMT_5_5_14bit,				// SC_ACT_WAZA_EFFECT
	SC_ARGFMT_5_5_14bit,				// SC_ACT_WAZA_DMG
	SC_ARGFMT_5_5_5bit_22byte,	// SC_ACT_WAZA_DMG_DBL
	SC_ARGFMT_5_5_5bit,					// SC_ACT_RANKUP
	SC_ARGFMT_5_5_5bit,					// SC_ACT_RANKDOWN
	SC_ARGFMT_1byte,						// SC_ACT_DEAD
	SC_ARGFMT_4_4bit,						// SC_ACT_MEMBER_OUT
	SC_ARGFMT_53bit_1byte,			// SC_ACT_MEMBER_IN
	SC_ARGFMT_5_5_14bit,				// SC_ACT_SICK_DMG
	SC_ARGFMT_4_4bit,						// SC_ACT_WEATHER_DMG,
	SC_ARGFMT_1byte,						// SC_ACT_WEATHER_END,
	SC_ARGFMT_1byte,						// SC_TOKWIN_IN
	SC_ARGFMT_1byte,						// SC_TOKWIN_OUT
	SC_ARGFMT_5_3bit,						// SC_MSG_WAZA

	SC_ARGFMT_MSG,		// SC_MSG_STD
	SC_ARGFMT_MSG,		// SC_MSG_SET
};

//------------------------------------------------------------------------
// globals
//------------------------------------------------------------------------
static int ArgBuffer[ 8 ];	// 可変個引数を全て格納するための一時バッファ

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static inline SC_ARGFMT_GetArgCount( u8 fmt );
static inline u8 pack1_2args( int arg1, int arg2, int bits1, int bits2 );
static inline void unpack1_2args( u8 pack, int bits1, int bits2, int* args, int idx_start );
static inline u32 pack_3args( int bytes, int arg1, int arg2, int arg3, int bits1, int bits2, int bits3 );
static inline void unpack_3args( int bytes, u32 pack, int bits1, int bits2, int bits3, int* args, int idx_start );
static void put_core( BTL_SERVER_CMD_QUE* que, ServerCmd cmd, ScArgFormat fmt, const int* args );
static void read_core( BTL_SERVER_CMD_QUE* que, ScArgFormat fmt, int* args );
static void read_core_msg( BTL_SERVER_CMD_QUE* que, int* args );



static inline u8 pack1_2args( int arg1, int arg2, int bits1, int bits2 )
{
	GF_ASSERT(bits1+bits2<=8);
	GF_ASSERT(bits1!=0);
	GF_ASSERT(bits2!=0);
	{
		u8 mask1, mask2;

		mask1 = (1 << bits1) - 1;
		mask2 = (1 << bits2) - 1;

		GF_ASSERT( arg1 <= mask1 );
		GF_ASSERT( arg2 <= mask2 );

		return ((arg1 & mask1) << bits2) | (arg2 & mask2);
	}
}
static inline void unpack1_2args( u8 pack, int bits1, int bits2, int* args, int idx_start )
{
	GF_ASSERT(bits1+bits2<=8);
	GF_ASSERT(bits1!=0);
	GF_ASSERT(bits2!=0);

	{
		u8 mask1 = (1 << bits1) - 1;
		u8 mask2 = (1 << bits2) - 1;

		BTL_Printf(" ** pack=%02x, bits1=%d, bits2=%d, mask1=%02x, mask2=%02x\n",
				pack, bits1, bits2, mask1, mask2 );

		args[ idx_start++ ] = (pack >> bits2) & mask1;
		args[ idx_start ] =  pack & mask2;
	}
}

static inline u32 pack_3args( int bytes, int arg1, int arg2, int arg3, int bits1, int bits2, int bits3 )
{
	GF_ASSERT(bits1+bits2+bits3<=(bytes*8));
	GF_ASSERT(bits1!=0);
	GF_ASSERT(bits2!=0);
	GF_ASSERT(bits3!=0);
	{
		u16 mask1 = (1 << bits1) - 1;
		u16 mask2 = (1 << bits2) - 1;
		u16 mask3 = (1 << bits3) - 1;

		GF_ASSERT(arg1 <= mask1);
		GF_ASSERT(arg2 <= mask2);
		GF_ASSERT(arg3 <= mask3);

		return ( ((arg1&mask2)<<(bits2+bits3)) | ((arg2&mask2)<<bits3) | (arg3&mask3) );
	}
}
static inline void unpack_3args( int bytes, u32 pack, int bits1, int bits2, int bits3, int* args, int idx_start )
{
	GF_ASSERT(bits1+bits2+bits3<=(bytes*8));
	GF_ASSERT(bits1!=0);
	GF_ASSERT(bits2!=0);
	GF_ASSERT(bits3!=0);
	{
		u16 mask1 = (1 << bits1) - 1;
		u16 mask2 = (1 << bits2) - 1;
		u16 mask3 = (1 << bits3) - 1;

		args[ idx_start++ ] = (pack >> (bits2+bits3)) & mask1;
		args[ idx_start++ ] = (pack >> bits3) & mask2;
		args[ idx_start ] = pack & mask3;
	}
}


//--------------------------------------------------------------------------
/**
 * コマンドナンバ、引数型に応じて全引数をQUEに格納
 *
 * @param   que		
 * @param   cmd		
 * @param   fmt		
 * @param   args		
 *
 */
//--------------------------------------------------------------------------
static void put_core( BTL_SERVER_CMD_QUE* que, ServerCmd cmd, ScArgFormat fmt, const int* args )
{
	scque_put2byte( que, cmd );

	switch( fmt ) {
	case SC_ARGFMT_1byte:
		scque_put1byte( que, args[0] );
		break;
	case SC_ARGFMT_12byte:
		scque_put1byte( que, args[0] );
		scque_put2byte( que, args[1] );
		break;
	case SC_ARGFMT_4_4bit:
		scque_put1byte( que, pack1_2args(args[0], args[1], 4, 4) );
		break;
	case SC_ARGFMT_5_3bit:
		scque_put1byte( que, pack1_2args(args[0], args[1], 5, 3) );
		break;
	case SC_ARGFMT_53bit_1byte:
		scque_put1byte( que, pack1_2args(args[0], args[1], 5, 3) );
		scque_put1byte( que, args[2] );
		break;
	case SC_ARGFMT_5_5_5bit:
		{
			u16 pack = pack_3args( 2, args[0],args[1],args[2], 5,5,5 );
			scque_put2byte( que, pack );
		}
		break;
	case SC_ARGFMT_5_5_14bit:
		{
			u32 pack = pack_3args( 3, args[0],args[1],args[2], 5,5,14 );
			scque_put3byte( que, pack );
		}
		break;
	case SC_ARGFMT_5_5_5bit_22byte:
		{
			u16 pack = pack_3args( 2, args[0],args[1],args[2], 5,5,5 );
			scque_put2byte( que, pack );
			scque_put2byte( que, args[3] );
			scque_put2byte( que, args[4] );
		}
		break;
	default:
		GF_ASSERT(0);
		break;
	}/* switch(fmt) */
}
//--------------------------------------------------------------------------
/**
 * 引数型に応じて全引数を配列に読み出し
 *
 * @param   que		
 * @param   fmt		
 * @param   args		
 *
 */
//--------------------------------------------------------------------------
static void read_core( BTL_SERVER_CMD_QUE* que, ScArgFormat fmt, int* args )
{
	switch( fmt ) {
	case SC_ARGFMT_1byte:
		args[0] = scque_read1byte( que );
		break;
	case SC_ARGFMT_12byte:
		args[0] = scque_read1byte( que );
		args[1] = scque_read2byte( que );
		break;
	case SC_ARGFMT_4_4bit:
		{
			u8 pack = scque_read1byte( que );
			unpack1_2args( pack, 4, 4, args, 0 );
		}
		break;
	case SC_ARGFMT_5_3bit:
		{
			u8 pack = scque_read1byte( que );
			unpack1_2args( pack, 5, 3, args, 0 );
		}
		break;
	case SC_ARGFMT_53bit_1byte:
		{
			u8 pack = scque_read1byte( que );
			unpack1_2args( pack, 5, 3, args, 0 );
			args[2] = scque_read1byte( que );
		}
		break;
	case SC_ARGFMT_5_5_5bit:
		{
			u16 pack = scque_read2byte( que );
			unpack_3args( 2, pack, 5,5,5, args, 0 );
		}
		break;
	case SC_ARGFMT_5_5_14bit:
		{
			u32 pack = scque_read3byte( que );
			unpack_3args( 3, pack, 5,5,14, args, 0 );
		}
		break;
	case SC_ARGFMT_5_5_5bit_22byte:
		{
			u16 pack = scque_read2byte( que );
			unpack_3args( 2, pack, 5, 5, 5, args, 0 );
			args[3] = scque_read2byte( que );
			args[4] = scque_read2byte( que );
		}
		break;
	default:
		GF_ASSERT(0);
		break;
	}/* switch(fmt) */
}


void SCQUE_PUT_Common( BTL_SERVER_CMD_QUE* que, ServerCmd cmd, ... )
{
	GF_ASSERT( cmd < NELEMS(ServerCmdToFmtTbl) );

	{
		va_list   list;
		u32 i, arg_cnt;
		u8 fmt;

		fmt = ServerCmdToFmtTbl[ cmd ];
		arg_cnt = SC_ARGFMT_GetArgCount( fmt );

		va_start( list, cmd );
		for(i=0; i<arg_cnt; ++i)
		{
			ArgBuffer[i] = va_arg( list, int );
		}
		va_end( list );

		#ifdef BTL_PRINT_SYSTEM_ENABLE
		BTL_Printf("PutCmd=%d, Format=%02x, argCnt=%d, args=", cmd, fmt, arg_cnt);
		for(i=0; i<arg_cnt; ++i)
		{
			TAYA_Printf("%d,", ArgBuffer[i]);
		}
		TAYA_Printf("\n");
		#endif

		put_core( que, cmd, fmt, ArgBuffer );
	}
}

ServerCmd SCQUE_Read( BTL_SERVER_CMD_QUE* que, int* args )
{
	ServerCmd cmd = scque_read2byte( que );

	GF_ASSERT( cmd < NELEMS(ServerCmdToFmtTbl) );

	{
		u8 fmt = ServerCmdToFmtTbl[ cmd ];

		if( fmt != SC_ARGFMT_MSG )
		{
			read_core( que, fmt, args );

			#ifdef BTL_PRINT_SYSTEM_ENABLE
			{
				u8 arg_cnt = SC_ARGFMT_GetArgCount( fmt );
				u8 i;
				BTL_Printf("ReadCmd=%d, Format=%02x, argCnt=%d, args=", cmd, fmt, arg_cnt);
				for(i=0; i<arg_cnt; ++i)
				{
					TAYA_Printf("%d,", args[i]);
				}
				TAYA_Printf("\n");
			}
			#endif
		}
		else
		{
			read_core_msg( que, args );
		}
	}

	return cmd;
}

//=============================================================================================
/**
 * 特定サーバコマンドの直後、１バイト単位で引数を渡す（可変引数に対処）
 *
 * @param   que		
 * @param   arg		
 *
 */
//=============================================================================================
void SCQUE_PUT_ArgOnly( BTL_SERVER_CMD_QUE* que, u8 arg )
{
	scque_put1byte( que, arg );
}
//=============================================================================================
/**
 * 特定サーバコマンドの直後、１バイト単位で渡された引数を読み出す
 *
 * @param   que		
 *
 */
//=============================================================================================
u8 SCQUE_READ_ArgOnly( BTL_SERVER_CMD_QUE* que )
{
	return scque_read1byte( que );
}



void SCQUE_PUT_MsgImpl( BTL_SERVER_CMD_QUE* que, u8 scType, ... )
{
	{
		va_list   list;
		ScMsgArg   arg;
		u16 strID;

		va_start( list, scType );
		strID = va_arg( list, int );

		scque_put2byte( que, scType );
		scque_put2byte( que, strID );

		BTL_Printf(" PUT MSG SC=%d, StrID=%d\n arg= ", scType, strID);

		do {
			arg = va_arg( list, int );
			scque_put1byte( que, arg );
			TAYA_Printf("%d ", arg);
		}while( arg != MSGARG_TERMINATOR );

		TAYA_Printf("\n");

		va_end( list );
	}
}

static void read_core_msg( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read2byte( que );

	BTL_Printf("READ MSG strID=%d\n", args[0]);

	{
		int i = 0;

		do {
			++i;
			if( i < BTL_SERVERCMD_ARG_MAX )
			{
				args[i] = scque_read1byte( que );
				BTL_Printf(" msg arg[%d] = %d\n", i-1, args[i]);
			}
			else
			{
				GF_ASSERT(0);
				break;
			}
		}while( args[i] != MSGARG_TERMINATOR );
	}
}


