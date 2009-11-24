//=============================================================================
/**
 *
 *	@file		debug_hosaka_menu.c
 *	@brief  保坂デバッグメニュー
 *	@author		hosaka genya
 *	@data		2009.10.21
 *
 */
//=============================================================================

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static BOOL TESTMODE_ITEM_SelectDemoEngine( TESTMODE_WORK *work, const int idx );
static BOOL TESTMODE_ITEM_SelectPmsSelect( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectMicTest( TESTMODE_WORK *work , const int idx );

// メニューリスト
static TESTMODE_MENU_LIST menuHosaka[] = 
{
	{L"デモエンジン",TESTMODE_ITEM_SelectDemoEngine },
	{L"かんい会話選択",TESTMODE_ITEM_SelectPmsSelect },
	{L"マイクテスト",TESTMODE_ITEM_SelectMicTest },
	
	{L"もどる"				,TESTMODE_ITEM_BackTopMenu },
};

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

// 3Dデモエンジン
#include "debug_3d_demo.h"
FS_EXTERN_OVERLAY(debug_hosaka);
static BOOL TESTMODE_ITEM_SelectDemoEngine( TESTMODE_WORK *work, const int idx )
{
  TESTMODE_COMMAND_ChangeProc( work, FS_OVERLAY_ID(debug_hosaka), &Debug3DDemoProcData, NULL );
//	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(demo_engine), &TitleMicTestProcData, NULL );
  return TRUE;
}

// 簡易会話 選択
#include "app/pms_select.h"
FS_EXTERN_OVERLAY(pmsinput);

static BOOL TESTMODE_ITEM_SelectPmsSelect( TESTMODE_WORK *work , const int idx )
{
  PMS_SELECT_PARAM* initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(PMS_SELECT_PARAM) );

  initParam->save_ctrl = SaveControl_GetPointer();

	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(pmsinput), &ProcData_PMSSelect, initParam);
	return TRUE;
}

// マイクテスト
FS_EXTERN_OVERLAY(mictest);
extern const GFL_PROC_DATA TitleMicTestProcData;
static BOOL TESTMODE_ITEM_SelectMicTest( TESTMODE_WORK *work , const int idx )
{
	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(mictest), &TitleMicTestProcData, NULL );
	return TRUE;
}

