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
static BOOL TESTMODE_ITEM_SelectDemo3D( TESTMODE_WORK *work, const int idx );
static BOOL TESTMODE_ITEM_SelectPmsSelect( TESTMODE_WORK *work , const int idx );
static BOOL TESTMODE_ITEM_SelectMicTest( TESTMODE_WORK *work , const int idx );

// メニューリスト
static TESTMODE_MENU_LIST menuHosaka[] = 
{
	{L"3Dデモさいせい",TESTMODE_ITEM_SelectDemo3D },
	{L"かんい会話選択",TESTMODE_ITEM_SelectPmsSelect },
	{L"マイクテスト",TESTMODE_ITEM_SelectMicTest },
	
	{L"もどる"				,TESTMODE_ITEM_BackTopMenu },
};

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

// 3Dデモ再生アプリ
#include "demo/demo3d.h"
static BOOL TESTMODE_ITEM_SelectDemo3D( TESTMODE_WORK *work, const int idx )
{
  DEMO3D_PARAM* initParam;
  
  initParam = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(DEMO3D_PARAM) );
  initParam->demo_id = DEMO3D_ID_C_CRUISER;
  initParam->start_frame = 1000; //@TODO
	TESTMODE_COMMAND_ChangeProc(work,FS_OVERLAY_ID(demo3d), &Demo3DProcData, initParam );
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

