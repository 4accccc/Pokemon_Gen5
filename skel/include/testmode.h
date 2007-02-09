//============================================================================================
/**
 * @file	testmode.h
 * @brief	デバッグ＆テストアプリ呼び出し用ヘッダ
 * @date	2007.01.30
 */
//============================================================================================

#ifndef	__TESTMODE_H__
#define	__TESTMODE_H__

#include "procsys.h"

extern void TestModeSet(void);
extern const GFL_PROC_DATA TestMainProcData;

//------------------------------------------------------------------
//
//	各ユーザー毎のプロセスデータの外部参照宣言
//
//------------------------------------------------------------------
extern const GFL_PROC_DATA DebugWatanabeMainProcData;
extern const GFL_PROC_DATA DebugTamadaMainProcData;
extern const GFL_PROC_DATA DebugSogabeMainProcData;
extern const GFL_PROC_DATA DebugOhnoMainProcData;
extern const GFL_PROC_DATA DebugMitsuharaMainProcData;


#endif	/*	__TESTMODE_H__ */

