//=============================================================================================
/**
 * @file	btl_pre_sc.c
 * @brief	ポケモンWB バトルシステム	プレサーバコマンド生成
 * @author	taya
 *
 * @date	2009.04.07	作成
 */
//=============================================================================================
#include <stdarg.h>

#include "btl_common.h"
#include "btl_server_cmd.h"

#include "btl_pre_sc.h"


static inline u16 compFormat( PreScElem preSc, PreScFormat fmt )
{
	GF_ASSERT( fmt < PRESC_FMT_MAX );
	GF_ASSERT( preSc < PRESC_FMT_MAX );

	return ((fmt&0x03)<<14) | (preSc&0x3fff);
}
static inline void putVal( PRE_SC_BUF* buf, u16 val )
{
	GF_ASSERT(buf->pointer < NELEMS(buf->code) );
	buf->code[ buf->pointer++ ] = val;
}


void PreSC_Put( PRE_SC_BUF* buf, PreScElem preSc, PreScFormat fmt, u32 numArgs, ... )
{
	va_list   list;
	u32 i;
	u16 arg;

	preSc = compFormat( preSc, fmt );
	putVal( buf, preSc );
	putVal( buf, numArgs );

	va_start( list, numArgs );
	for(i=0; i<numArgs; ++i)
	{
		arg = va_arg( list, int );
		putVal( buf, arg );
	}
	va_end( list );
}

