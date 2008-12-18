//=============================================================================================
/**
 * @file	btl_util.c
 * @brief	ポケモンWB バトルシステム 共有ツール群
 * @author	taya
 *
 * @date	2008.09.25	作成
 */
//=============================================================================================

#include "btl_common.h"
#include "btl_util.h"


void BTL_UTIL_Printf( const char* filename, int line, const char* fmt, ... )
{
//	AssertPrintTitle(filename, line_no);
	{
	    va_list vlist;
	    va_start( vlist, fmt );
	    OS_TVPrintf( fmt, vlist );
	    va_end( vlist );
	}

}

void BTL_UTIL_DumpPrintf( const char* caption, const void* data, u32 size )
{
	OS_TPrintf( "[DUMP] %s (%dbytes)\n", caption, size );

	if( size > 8 ){ size = 8; }

	{
		const u8* p = data;
		u32 i;
		for(i=0; i<size; i++)
		{
			OS_TPrintf("%02x ", p[i]);
		}
		OS_TPrintf("\n");
	}
}

