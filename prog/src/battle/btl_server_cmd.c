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

void SCQUE_PUT_MsgSpecial( BTL_SERVER_CMD_QUE* que, u16 strID, u8 arg1, u8 arg2 )
{
	scque_put2byte( que, SC_MSG );
	scque_put2byte( que, strID );
	scque_put1byte( que, arg1 );
	scque_put1byte( que, arg2 );
	scque_put1byte( que, MSGARG_TERMINATOR );
}


void SCQUE_PUT_MsgBody( BTL_SERVER_CMD_QUE* que, u16 strID, ... )
{
	{
		va_list   list;
		ScMsgArg   arg;

		va_start( list, strID );

		scque_put2byte( que, SC_MSG );
		scque_put2byte( que, strID );

		do {
			arg = va_arg( list, int );
			scque_put1byte( que, arg );
		}while( arg != MSGARG_TERMINATOR );

		va_end( list );
	}
}

void SCQUE_READ_Msg( BTL_SERVER_CMD_QUE* que, int* args )
{
	args[0] = scque_read2byte( que );

	{
		int i = 0;
		do {
			++i;
			if( i < BTL_SERVERCMD_ARG_MAX )
			{
				args[i] = scque_read1byte( que );
			}
			else
			{
				GF_ASSERT(0);
				break;
			}
		}while( args[i] != MSGARG_TERMINATOR );
	}
}

