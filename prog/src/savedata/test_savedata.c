//==============================================================================
/**
 * @file	test_savedata.c
 * @brief	テスト用セーブ
 * @author	matsuda
 * @date	2008.08.28(木)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/test_savedata.h"


//--------------------------------------------------------------
/**
 * @brief	松田デバッグ用セーブのサイズを取得
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
u32	DebugMatsuSave_GetWorkSize(void)
{
	return sizeof(int);
}

//---------------------------------------------------------------------------
/**
 * @brief	松田デバッグ用セーブデータの初期化処理
 * @param	condata		コンテストセーブデータへのポインタ
 */
//---------------------------------------------------------------------------
void DebugMatsuSave_Init(int *work)
{
	*work = 0;
}

u32	DebugMatsuSave_GetWorkSize_0(void)
{
	return 0x64;
}
u32	DebugMatsuSave_GetWorkSize_1(void)
{
	return 0x34;
}
u32	DebugMatsuSave_GetWorkSize_2(void)
{
	return 0x598;
}
u32	DebugMatsuSave_GetWorkSize_3(void)
{
	return 0x77c;
}
u32	DebugMatsuSave_GetWorkSize_4(void)
{
	return 0x3b4;
}
u32	DebugMatsuSave_GetWorkSize_5(void)
{
	return 0xa8;
}
u32	DebugMatsuSave_GetWorkSize_6(void)
{
	return 0x32c;
}
u32	DebugMatsuSave_GetWorkSize_7(void)
{
	return 0x1408;
}
u32	DebugMatsuSave_GetWorkSize_8(void)
{
	return 0x110;
}
u32	DebugMatsuSave_GetWorkSize_9(void)
{
	return 0x468;
}
u32	DebugMatsuSave_GetWorkSize_a(void)
{
	return 0x628;
}
u32	DebugMatsuSave_GetWorkSize_b(void)
{
	return 0x1c4;
}
u32	DebugMatsuSave_GetWorkSize_c(void)
{
	return 0x15c;
}
u32	DebugMatsuSave_GetWorkSize_d(void)
{
	return 0xbd0;
}
u32	DebugMatsuSave_GetWorkSize_e(void)
{
	return 0x243a;
}
