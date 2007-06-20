//==============================================================================================
/**
 * @file	assert.c
 * @date	2007.01.19
 * @author	tamada GAME FREAK inc.
 */
//==============================================================================================

#include "gflib.h"

//==============================================================================================
//==============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ASSERT処理用表示メイン
 * @param	fmt
 */
//------------------------------------------------------------------
static void AssertPrintf(const char * fmt, ... )
{
    va_list vlist;

    va_start(vlist, fmt);
    OS_TVPrintf(fmt, vlist);
    va_end(vlist);
}

//------------------------------------------------------------------
/**
 * @brief	アサート失敗箇所のソース情報を出力
 */
//------------------------------------------------------------------
static void AssertPrintTitle(const char * filename, unsigned int line_no)
{
	AssertPrintf("  **** ASSERTION FAILED ! **** \n");
	AssertPrintf("%s(%d)\n", filename, line_no);
}

//------------------------------------------------------------------
/**
 * @brief	ASSERT処理簡易版（条件式と発生箇所の表示）
 */
//------------------------------------------------------------------
void GFL_Assert(const char * filename, unsigned int line_no, const char * exp)
{
	AssertPrintTitle(filename, line_no);

	AssertPrintf("%s\n", exp);

	OS_Terminate();
}

//------------------------------------------------------------------
/**
 * @brief	ASSERT処理（条件式とユーザー指定の文字列表示）
 */
//------------------------------------------------------------------
void GFL_AssertMsg(const char * filename, unsigned int line_no, const char * fmt, ... )
{
	va_list vlist;
	AssertPrintTitle(filename, line_no);
	va_start(vlist, fmt);
	OS_TVPrintf(fmt, vlist);
	va_end(vlist);

	OS_Terminate();
}

#if 0
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void GFL_AssertPrintf(const char * fmt, ... )
{
}
#endif

