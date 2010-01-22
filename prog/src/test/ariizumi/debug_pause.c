//======================================================================
/**
 * @file	debug_pause.c
 * @brief	コマ送り機能
 * @author	ariizumi
 * @data	09/03/18
 */
//======================================================================
#include <gflib.h>

#include "test/debug_pause.h"

#ifdef PM_DEBUG

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
	BOOL isEnable;
}DEBUG_PAUSE_WORK;
//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

DEBUG_PAUSE_WORK debPauseWork;

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------

void DEBUG_PAUSE_Init( void )
{
	debPauseWork.isEnable = FALSE;
	
#if DEBUG_ONLY_FOR_ariizumi_nobuhiko
	//debPauseWork.isEnable = TRUE;
#endif

}

const BOOL DEBUG_PAUSE_Update( void )
{
	if( debPauseWork.isEnable == FALSE )
	{
		return TRUE;
	}
	
	//Lが押し続けられていてRが押された瞬間ではないとき
	if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_L)  &&
		(GFL_UI_KEY_GetTrg()  & PAD_BUTTON_R) == 0 )
	{
		return FALSE;
	}
	return TRUE;
}

//有効の設定・取得
void DEBUG_PAUSE_SetEnable( const BOOL isEnable )
{
	debPauseWork.isEnable = isEnable;
}

const BOOL DEBUG_PAUSE_GetEnable( void )
{
	return debPauseWork.isEnable;
}


#endif //PM_DEBUG
