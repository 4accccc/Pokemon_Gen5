//==============================================================================================
/**
 * @file	assert.c
 * @date	2007.01.19
 * @author	tamada GAME FREAK inc.
 */
//==============================================================================================

#include "gflib.h"


//==============================================================================================
// ユーザ定義の表示関数群を管理するためのポインタ
//==============================================================================================
static GFL_ASSERT_DispInitFunc		DispInitFunc = NULL;
static GFL_ASSERT_DispMainFunc		DispMainFunc = NULL;
static GFL_ASSERT_DispFinishFunc	DispFinishFunc = NULL;

//==============================================================================================
//==============================================================================================


//------------------------------------------------------------------
/**
 * アサート情報出力処理関数をユーザ設定する
 *
 * @param   initFunc		表示初期化時に呼ばれるコールバック関数
 * @param   mainFunc		文字出力ごとに呼ばれるコールバック関数
 * @param   finishFunc		表示終了時に呼ばれるコールバック関数
 *
 */
//------------------------------------------------------------------
void GFL_ASSERT_SetDisplayFunc( GFL_ASSERT_DispInitFunc initFunc, GFL_ASSERT_DispMainFunc mainFunc, GFL_ASSERT_DispFinishFunc finishFunc )
{
	DispInitFunc = initFunc;
	DispMainFunc = mainFunc;
	DispFinishFunc = finishFunc;
}


//------------------------------------------------------------------
/**
 * アサート情報出力前処理
 *
 * @param   none		
 *
 */
//------------------------------------------------------------------
static void InitAssertDisp( void )
{
	if( DispInitFunc != NULL )
	{
		DispInitFunc();
	}
}

//------------------------------------------------------------------
/**
 * アサート情報出力メイン処理
 *
 * @param   fmt			書式付き文字列
 * @param   vlist		書式情報
 *
 */
//------------------------------------------------------------------
static void AssertVPrintf( const char* fmt, va_list vlist )
{
	if( DispMainFunc != NULL )
	{
		enum {
			TMP_BUFFER_SIZE = 512,
		};

		static char buf[ TMP_BUFFER_SIZE ];

		OS_VSNPrintf( buf, TMP_BUFFER_SIZE, fmt, vlist );
		DispMainFunc( buf );
	}
	else
	{
		OS_TVPrintf(fmt, vlist);
	}
}

//------------------------------------------------------------------
/**
 * アサート情報出力終了処理
 */
//------------------------------------------------------------------
static void FinishAssertDisp( void )
{
	if( DispFinishFunc != NULL )
	{
		DispFinishFunc();
	}
	else
	{
		OS_Terminate();
	}
}

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
    AssertVPrintf( fmt, vlist );
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
	InitAssertDisp();

	AssertPrintTitle(filename, line_no);
	AssertPrintf("%s\n", exp);

	FinishAssertDisp();
}

//------------------------------------------------------------------
/**
 * @brief	ASSERT処理（条件式とユーザー指定の文字列表示）
 */
//------------------------------------------------------------------
void GFL_AssertMsg(const char * filename, unsigned int line_no, const char * fmt, ... )
{
	InitAssertDisp();

	AssertPrintTitle(filename, line_no);
	{
	    va_list vlist;
	    va_start(vlist, fmt);
		AssertVPrintf( fmt, vlist );
	    va_end(vlist);
	}
	FinishAssertDisp();
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

