//==============================================================================================
/**
 * @file	debug_print.c
 * @date	2007.02.13
 * @author	taya GAME FREAK inc.
 */
//==============================================================================================

#include "system/debug_print.h"


// 一時的に別環境でも出力したい情報がある場合、そのPrintf記述者のIDを定義する
#define REGULAR_AUTHOR_ID	(AUTHOR_FREE)


#ifdef DEBUG_PRINT_ENABLE

//==============================================================================================
/**
 * 特定ユーザの環境でコンパイルした場合のみ動作するPrintf関数
 *
 * @param   author_id		ユーザID（debug_print.hに記述）
 * @param   fmt				OS_TPrintfに渡す書式付き文字列
 *
 */
//==============================================================================================
void DEBUG_Printf( AUTHOR_ID author_id, const char * fmt, ... )
{
	if(	(author_id==AUTHOR_FREE)
	||	(author_id==DEFINED_AUTHOR_ID)
	||	(author_id==REGULAR_AUTHOR_ID)
	){
		va_list vlist;

		va_start(vlist, fmt);
		OS_TVPrintf(fmt, vlist);
		va_end(vlist);
	}
}

#endif
