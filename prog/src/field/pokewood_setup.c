//============================================================================================
/**
 * @file  pokewood_setup.c
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.07.04
 */
//============================================================================================

#include <gflib.h>

#include "pokewood_setup.h"

//============================================================================================
//
//  PKWD COUNT画面関連
//
//============================================================================================
//------------------------------------------------------------------
///  PKWD_COUNT用パラメータOBJECT生成
//------------------------------------------------------------------
PKWD_COUNT_PARAM * PKWD_COUNT_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work )
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	PKWD_COUNT_PARAM 	*pkwd_param;
	
	pkwd_param 			= GFL_HEAP_AllocClearMemory( heapID, sizeof(PKWD_COUNT_PARAM) );
	// 初期化
	pkwd_param->title_no	  = 0;
	pkwd_param->scenario_no = 0;
	pkwd_param->message		  = 0;
	//
	return pkwd_param;
}

//------------------------------------------------------------------
///  PKWD COUNT用パラメータOBJECT削除
//------------------------------------------------------------------
void PKWD_COUNT_PARAM_Delete( PKWD_COUNT_PARAM * pkwd_param )
{
	// 必要であれば削除を追加
	GFL_HEAP_FreeMemory( pkwd_param );
}

#ifdef  PM_DEBUG
//------------------------------------------------------------------
/**
 * @brief PKWD COUNT用パラメータのセットアップ（デバッグ用）
 *
 */
//------------------------------------------------------------------
void PKWD_COUNT_PARAM_Setup_Debug( GAMESYS_WORK * gsys, PKWD_COUNT_PARAM * pkwd_param )
{
	// 必要であれば、ここでパラメータのセットをするらしい
	
}
#endif

//============================================================================================
//
//  PKWD BOOK画面関連
//
//============================================================================================
//------------------------------------------------------------------
///  PKWD_BOOK用パラメータOBJECT生成
//------------------------------------------------------------------
PKWD_BOOK_PARAM * PKWD_BOOK_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work )
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	MYSTATUS * mystatus = GAMEDATA_GetMyStatus( gamedata );
	MISC * misc         = GAMEDATA_GetMiscWork( gamedata );
	PKWD_BOOK_PARAM 	*pkwd_param;
	
	pkwd_param 			= GFL_HEAP_AllocClearMemory( heapID, sizeof(PKWD_BOOK_PARAM) );
	// 初期化
//  pkwd_param->gamesys = gsys;
  pkwd_param->mystatus = mystatus;
  pkwd_param->misc     = misc;
  pkwd_param->result = (s16*)res_work;
	//
	return pkwd_param;
}

//------------------------------------------------------------------
///  PKWD BOOK用パラメータOBJECT削除
//------------------------------------------------------------------
void PKWD_BOOK_PARAM_Delete( PKWD_BOOK_PARAM * pkwd_param )
{
	// 必要であれば削除を追加
	GFL_HEAP_FreeMemory( pkwd_param );
}


//============================================================================================
//
//  PKWD SELECT SCREEN画面関連
//
//============================================================================================
//------------------------------------------------------------------
///  PKWD_BOOK用パラメータOBJECT生成
//------------------------------------------------------------------
PKWD_SELECT_SCREEN_PARAM * PKWD_SELECT_SCREEN_PARAM_Create( HEAPID heapID, GAMESYS_WORK * gsys, u16 * res_work )
{
	GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	MYSTATUS * mystatus = GAMEDATA_GetMyStatus( gamedata );
	MISC * misc         = GAMEDATA_GetMiscWork( gamedata );
	PKWD_SELECT_SCREEN_PARAM 	*param;
	
	param 			= GFL_HEAP_AllocClearMemory( heapID, sizeof(PKWD_SELECT_SCREEN_PARAM) );
	// 初期化
  param->gamesys = gsys;
//  param->mystatus = mystatus;
//  param->misc     = misc;
  param->result = (s16*)res_work;
	//
	return param;
}

//------------------------------------------------------------------
///  PKWD BOOK用パラメータOBJECT削除
//------------------------------------------------------------------
void PKWD_SELECT_SCREEN_PARAM_Delete( PKWD_SELECT_SCREEN_PARAM * param )
{
	// 必要であれば削除を追加
	GFL_HEAP_FreeMemory( param );
}

