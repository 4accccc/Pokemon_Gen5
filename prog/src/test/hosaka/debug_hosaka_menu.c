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
#include "app/pms_select.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static BOOL TESTMODE_ITEM_SelectPmsSelect( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectMicTest( TESTMODE_WORK *work , const int idx );

static TESTMODE_MENU_LIST menuHosaka[] = 
{
	{L"かんい会話選択",TESTMODE_ITEM_SelectPmsSelect },
	{L"マイクテスト",TESTMODE_ITEM_SelectMicTest },
	
	{L"もどる"				,TESTMODE_ITEM_BackTopMenu },
};

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================




//=============================================================================
/**
 *								static関数
 */
//=============================================================================

// 簡易会話 選択
FS_EXTERN_OVERLAY(pmsinput);
extern const GFL_PROC_DATA ProcData_PMSSelect;
static BOOL TESTMODE_ITEM_SelectPmsSelect( TESTMODE_WORK *work , const int idx )
{
  PMS_SELECT_PARAM* initParam;

  initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(PMS_SELECT_PARAM) );
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
