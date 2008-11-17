//=============================================================================================
/**
 * @file	btl_util.h
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

