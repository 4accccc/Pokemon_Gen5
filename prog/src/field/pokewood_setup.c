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
	PKWD_BOOK_PARAM 	*pkwd_param;
	
	pkwd_param 			= GFL_HEAP_AllocClearMemory( heapID, sizeof(PKWD_BOOK_PARAM) );
	// 初期化
  pkwd_param->gamesys = gsys;
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

