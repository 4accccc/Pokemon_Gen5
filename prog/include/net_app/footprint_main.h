//==============================================================================
/**
 * @file	footprint_main.h
 * @brief	足跡ボードのメインヘッダ
 * @author	matsuda
 * @date	2008.01.19(土)
 */
//==============================================================================
#ifndef __FOOTPRINT_MAIN_H__
#define __FOOTPRINT_MAIN_H__

#include "net_app/wifi_lobby/wflby_system_def.h"

//==============================================================================
//	定数定義
//==============================================================================
///ボードの種類
enum{
	FOOTPRINT_BOARD_TYPE_WHITE,		///<白ボード
	FOOTPRINT_BOARD_TYPE_BLACK,		///<黒ボード

	FOOTPRINT_BOARD_TYPE_MAX,
};


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	WFLBY_SYSTEM *wflby_sys;		///<WIFIロビーシステムへのポインタ
	u8 board_type;					///<ボードの種類(FOOTPRINT_BOARD_TYPE_???)
	
	u8 padding[3];
	
#ifdef PM_DEBUG
	SAVE_CONTROL_WORK *debug_sv;
#endif
}FOOTPRINT_PARAM;


//==============================================================================
//	外部関数宣言
//==============================================================================
extern GFL_PROC_RESULT FootPrintProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
extern GFL_PROC_RESULT FootPrintProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
extern GFL_PROC_RESULT FootPrintProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );


#endif	//__FOOTPRINT_MAIN_H__

