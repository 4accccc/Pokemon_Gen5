////=============================================================================
/**
 *
 *	@file		pmsiv_menu.c
 *	@brief  メニュー管理モジュール
 *	@author	hosaka genya
 *	@data		2009.11.04
 *
 */
//=============================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system\main.h"
#include "system\pms_word.h"
#include "print\printsys.h"
#include "msg\msg_pms_input_button.h"
#include "msg\msg_pms_input.h"
#include "message.naix"

#include "pms_input_prv.h"
#include "pms_input_view.h"

//タッチバー
#include "ui/touchbar.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  // エディットモードの配置
  TASKMENU_WIN_EDIT_H = APP_TASKMENU_PLATE_HEIGHT,
  TASKMENU_WIN_EDIT_W = 9,
  TASKMENU_WIN_EDIT_X = 24,
  TASKMENU_WIN_EDIT_Y = 18,

  // カテゴリ・イニシャルモードの配置
  TASKMENU_WIN_INITIAL_H = APP_TASKMENU_PLATE_HEIGHT,
  TASKMENU_WIN_INITIAL_W = 9,
  TASKMENU_WIN_INITIAL_X = 6,
  TASKMENU_WIN_INITIAL_Y = 21,
};

//--------------------------------------------------------------
///	タスクメニューウィンドウ
//==============================================================
typedef enum {
  // けってい・やめる
  TASKMENU_WIN_EDIT_DECIDE = 0,
  TASKMENU_WIN_EDIT_CANCEL,
  TASKMENU_WIN_EDIT_MAX,

  // えらぶ・けす・やめる
  TASKMENU_WIN_INITIAL_SELECT = 0,
  TASKMENU_WIN_INITIAL_DELETE,
  TASKMENU_WIN_INITIAL_CANCEL,
  TASKMENU_WIN_INITIAL_MAX,

  // ワークの最大数
  TASKMENU_WIN_MAX = TASKMENU_WIN_INITIAL_MAX,

} TASKMENU_WIN;

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
///	メニュー管理モジュール メインワーク
//==============================================================
struct _PMSIV_MENU {
  // [IN]
  PMS_INPUT_VIEW* vwk;
  const PMS_INPUT_WORK* mwk;
  const PMS_INPUT_DATA* dwk;
  // [PRIVATE]
  TOUCHBAR_WORK*          touchbar;
	GFL_MSGDATA*            msgman;
  APP_TASKMENU_RES*       menu_res;
  APP_TASKMENU_ITEMWORK   menu_item[ TASKMENU_WIN_MAX ];
  APP_TASKMENU_WIN_WORK*  menu_win[ TASKMENU_WIN_MAX ];
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static TOUCHBAR_WORK* touchbar_init( GFL_CLUNIT* clunit, HEAPID heap_id );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  メニュー管理モジュール生成
 *
 *	@param	PMS_INPUT_VIEW* vwk
 *	@param	PMS_INPUT_WORK* mwk
 *	@param	PMS_INPUT_DATA* dwk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
PMSIV_MENU* PMSIV_MENU_Create( PMS_INPUT_VIEW* vwk, const PMS_INPUT_WORK* mwk, const PMS_INPUT_DATA* dwk )
{
  PMSIV_MENU* wk = GFL_HEAP_AllocClearMemory( HEAPID_PMS_INPUT_VIEW, sizeof(PMSIV_MENU) );

	wk->vwk = vwk;
	wk->mwk = mwk;
	wk->dwk = dwk;
  
  wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_pms_input_dat, HEAPID_PMS_INPUT_VIEW );
    
  // リソース展開
  wk->menu_res = APP_TASKMENU_RES_Create( 
        FRM_MAIN_TASKMENU, PALNUM_MAIN_TASKMENU, 
        PMSIView_GetFontHandle(wk->vwk),
        PMSIView_GetPrintQue(wk->vwk),
        HEAPID_PMS_INPUT_VIEW );

  // タッチバー
  wk->touchbar = touchbar_init( PMSIView_GetCellUnit(wk->vwk), HEAPID_PMS_INPUT_VIEW );

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メニュー管理モジュール削除
 *
 *	@param	PMSIV_MENU* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSIV_MENU_Delete( PMSIV_MENU* wk )
{
  // タッチバー開放
  TOUCHBAR_Exit( wk->touchbar );

  // タスクメニュー開放
  PMSIV_MENU_Clear( wk );

  // タスクメニュー リソース開放
  APP_TASKMENU_RES_Delete( wk->menu_res );

  GFL_MSG_Delete( wk->msgman );

  GFL_HEAP_FreeMemory( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  全てのタスクメニューを開放
 *
 *	@param	PMSIV_MENU* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSIV_MENU_Clear( PMSIV_MENU* wk )
{
  int i;

  for( i=0; i<TASKMENU_WIN_MAX; i++ )
  {
    if( wk->menu_item[i].str != NULL )
    {
      GFL_STR_DeleteBuffer( wk->menu_item[i].str );
      wk->menu_item[i].str = NULL;
    }
    if( wk->menu_win[i] != NULL )
    {

      APP_TASKMENU_WIN_Delete( wk->menu_win[i] );
      wk->menu_win[i] = NULL;
      HOSAKA_Printf("dell win[%d] \n",i);
    }
  }
 
  GFL_BG_LoadScreenReq( FRM_MAIN_TASKMENU );
  
  HOSAKA_Printf("PMSIV_MENU_Clear\n");
}

//-----------------------------------------------------------------------------
/**
 *	@brief  EDITシーン用の配置でセットアップ
 *
 *	@param	PMS_INPUT_VIEW* vwk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSIV_MENU_SetupEdit( PMSIV_MENU* wk )
{
  int i;
    
  PMSIV_MENU_Clear( wk );
    
  for( i=0; i<TASKMENU_WIN_EDIT_MAX; i++ )
  {
    if( wk->menu_item[i].str != NULL )
    {
      GFL_STR_DeleteBuffer( wk->menu_item[i].str );
      wk->menu_item[i].str = NULL;
    }

		wk->menu_item[i].str			= GFL_MSG_CreateString( wk->msgman, str_decide + i );
		wk->menu_item[i].msgColor	= APP_TASKMENU_ITEM_MSGCOLOR;
		wk->menu_item[i].type			= APP_TASKMENU_WIN_TYPE_NORMAL + (i==TASKMENU_WIN_EDIT_CANCEL);

    if( wk->menu_win[i] != NULL )
    {
      APP_TASKMENU_WIN_Delete( wk->menu_win[i] );
      wk->menu_win[i] = NULL;
    }

    wk->menu_win[i] = APP_TASKMENU_WIN_Create( wk->menu_res, &wk->menu_item[i], 
        TASKMENU_WIN_EDIT_X,
        TASKMENU_WIN_EDIT_Y + TASKMENU_WIN_EDIT_H * i, 
        TASKMENU_WIN_EDIT_W,
        HEAPID_PMS_INPUT_VIEW );
      
    HOSAKA_Printf("create win[%d] \n",i);
  }

  GFL_BG_LoadScreenReq( FRM_MAIN_TASKMENU );

  HOSAKA_Printf("PMSIV_MENU_SetupEdit\n");
}

//-----------------------------------------------------------------------------
/**
 *	@brief  カテゴリーモードのセットアップ
 *
 *	@param	PMSIV_MENU* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMSIV_MENU_SetupCategory( PMSIV_MENU* wk )
{
  int i;
    
  PMSIV_MENU_Clear( wk );
	  
  if( PMSI_GetCategoryMode(wk->mwk) == CATEGORY_MODE_GROUP )
	{
    // グループはメニューなしなので何もせず抜ける
    return;
  }

  for( i=0; i<TASKMENU_WIN_INITIAL_MAX; i++ )
  {
    if( wk->menu_item[i].str != NULL )
    {
      GFL_STR_DeleteBuffer( wk->menu_item[i].str );
      wk->menu_item[i].str = NULL;
    }

		wk->menu_item[i].str			= GFL_MSG_CreateString( wk->msgman, select01_01 + i );
		wk->menu_item[i].msgColor	= APP_TASKMENU_ITEM_MSGCOLOR;
		wk->menu_item[i].type			= APP_TASKMENU_WIN_TYPE_NORMAL + (i==TASKMENU_WIN_INITIAL_CANCEL);

    if( wk->menu_win[i] != NULL )
    {
      APP_TASKMENU_WIN_Delete( wk->menu_win[i] );
      wk->menu_win[i] = NULL;
    }

    wk->menu_win[i] = APP_TASKMENU_WIN_Create( wk->menu_res, &wk->menu_item[i], 
        TASKMENU_WIN_INITIAL_X + TASKMENU_WIN_INITIAL_W * i,
        TASKMENU_WIN_INITIAL_Y, 
        TASKMENU_WIN_INITIAL_W,
        HEAPID_PMS_INPUT_VIEW );
  }

  GFL_BG_LoadScreenReq( FRM_MAIN_TASKMENU );

  HOSAKA_Printf("PMSIV_MENU_SetupCategory\n");
}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  タッチバーの設定
 *
 *	@param	GFL_CLUNIT* unit
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK* touchbar_init( GFL_CLUNIT* clunit, HEAPID heap_id )
{
  TOUCHBAR_WORK* wk;

  //アイコンの設定
	//数分作る
	TOUCHBAR_SETUP	touchbar_setup = {0};

	TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
	{	
		{	
			TOUCHBAR_ICON_RETURN,
			{	TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
		},
	};

	//設定構造体
	//さきほどの窓情報＋リソース情報をいれる
	touchbar_setup.p_item		= touchbar_icon_tbl;				//上の窓情報
	touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
	touchbar_setup.p_unit		= clunit;										//OBJ読み込みのためのCLUNIT
	touchbar_setup.bar_frm	= FRM_MAIN_BAR;					//BG読み込みのためのBG面
	touchbar_setup.bg_plt		= PALNUM_MAIN_TOUCHBAR;			//BGﾊﾟﾚｯﾄ
	touchbar_setup.obj_plt	= PALNUM_OBJ_M_TOUCHBAR;		//OBJﾊﾟﾚｯﾄ
	touchbar_setup.mapping	= APP_COMMON_MAPPING_64K;	//マッピングモード
  touchbar_setup.is_notload_bg = TRUE;

  wk = TOUCHBAR_Init( &touchbar_setup, heap_id );

  // 一端全部消しておく
  TOUCHBAR_SetVisibleAll( wk, FALSE );

  return wk;
}


