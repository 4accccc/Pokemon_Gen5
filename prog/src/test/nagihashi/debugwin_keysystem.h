//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debugwin_keysystem.h
 *	@brief
 *	@author		Toru=Nagihashi
 *	@date		2011.06.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef PM_DEBUG
#include "debug/debugwin_sys.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define KEYSYSTEM_DEBUGWIN_GROUP  (1)
#define KEYSYSTEM_SELF_DEBUGWIN_GROUP  (2)
#define KEYSYSTEM_RECV_DEBUGWIN_GROUP  (3)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *          デバッグウィンドウ関数
 */
//=============================================================================
static void DebugWin_U_Key_Have_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY );
    KEY_SYSTEM_SAVE_DEBUG_SetHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Have_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY );
 DEBUGWIN_ITEM_SetNameV( p_item , "かんたんキー=%d", is_exits );
}
static void DebugWin_U_Key_Have_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD );
    KEY_SYSTEM_SAVE_DEBUG_SetHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Have_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD );
  DEBUGWIN_ITEM_SetNameV( p_item , "むずいキー=%d", is_exits );
}
static void DebugWin_U_Key_Have_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC );
    KEY_SYSTEM_SAVE_DEBUG_SetHave( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Have_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsHave( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC );
  DEBUGWIN_ITEM_SetNameV( p_item , "まちキー=%d", is_exits );
}
static void DebugWin_U_Key_Use_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY );
    KEY_SYSTEM_SAVE_DEBUG_SetUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Use_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_EASY );
  DEBUGWIN_ITEM_SetNameV( p_item , "かんたんキー=%d", is_exits );
}
static void DebugWin_U_Key_Use_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD );
    KEY_SYSTEM_SAVE_DEBUG_SetUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Use_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_DIFFICULT_HARD );
  DEBUGWIN_ITEM_SetNameV( p_item , "むずいキー=%d", is_exits );
}
static void DebugWin_U_Key_Use_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC);
    KEY_SYSTEM_SAVE_DEBUG_SetUse( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC, !is_exits );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Use_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  BOOL is_exits = KEY_SYSTEM_SAVE_IsUse( p_key_system, KEY_SYSTEM_TYPE_CITY_WFBC );
  DEBUGWIN_ITEM_SetNameV( p_item , "まちキー=%d", is_exits );
}
static void DebugWin_U_Key_Difficult( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    KEY_SYSTEM_DIFFICULT dif = KEY_SYSTEM_SAVE_GetDifficult( p_key_system );
    dif++;
    dif %= KEY_SYSTEM_DIFFICULT_MAX;
    KEY_SYSTEM_SAVE_SetDifficult( p_key_system, dif );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Difficult( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char *sc_difficult[] =
  {
    "かんたん",
    "つうじょう",
    "むずかしい",
  };

  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;

  KEY_SYSTEM_DIFFICULT dif = KEY_SYSTEM_SAVE_GetDifficult( p_key_system );

  DEBUGWIN_ITEM_SetNameV( p_item , "なんいど=%s", sc_difficult[dif] );
}
static void DebugWin_U_Key_Init( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    KEY_SYSTEM_SAVE_Init( p_key_system );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Init( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DEBUGWIN_ITEM_SetName( p_item , "しょきか" );
}
static void DebugWin_U_Key_Wfbc( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    KEY_SYSTEM_WFBC wfbc = KEY_SYSTEM_SAVE_GetWfbc( p_key_system );
    wfbc++;
    wfbc %= KEY_SYSTEM_WFBC_MAX;
    KEY_SYSTEM_SAVE_SetWfbc( p_key_system, wfbc );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_Key_Wfbc( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  static const char *sc_wfbc[] =
  {
    "つうじょうのまち",
    "はんたいのまち",
  };

  KEY_SYSTEM_SAVE *p_key_system = p_wk_adrs;

  KEY_SYSTEM_WFBC wfbc = KEY_SYSTEM_SAVE_GetWfbc( p_key_system );

  DEBUGWIN_ITEM_SetNameV( p_item , "まち=%s", sc_wfbc[wfbc] );
}

static void DEBUGWIN_KEY_SYSTEM_Init( KEY_SYSTEM_SAVE *p_key_system, HEAPID heapID )
{
  DEBUGWIN_AddGroupToTop( KEYSYSTEM_DEBUGWIN_GROUP, "キーシステム", heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Difficult, 
                              DebugWin_D_Key_Difficult, 
                              p_key_system, 
                              KEYSYSTEM_DEBUGWIN_GROUP, 
                              heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Wfbc, 
                              DebugWin_D_Key_Wfbc, 
                              p_key_system, 
                              KEYSYSTEM_DEBUGWIN_GROUP, 
                              heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Init, 
                              DebugWin_D_Key_Init, 
                              p_key_system, 
                              KEYSYSTEM_DEBUGWIN_GROUP, 
                              heapID );

  DEBUGWIN_AddGroupToGroup( KEYSYSTEM_SELF_DEBUGWIN_GROUP, "しょじキー", KEYSYSTEM_DEBUGWIN_GROUP , heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Have_00, 
                              DebugWin_D_Key_Have_00, 
                              p_key_system, 
                              KEYSYSTEM_SELF_DEBUGWIN_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Have_01, 
                              DebugWin_D_Key_Have_01, 
                              p_key_system, 
                              KEYSYSTEM_SELF_DEBUGWIN_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Have_02, 
                              DebugWin_D_Key_Have_02, 
                              p_key_system, 
                              KEYSYSTEM_SELF_DEBUGWIN_GROUP, 
                              heapID );

  DEBUGWIN_AddGroupToGroup( KEYSYSTEM_RECV_DEBUGWIN_GROUP, "こうもくかいほう", KEYSYSTEM_DEBUGWIN_GROUP , heapID );


  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Use_00, 
                              DebugWin_D_Key_Use_00, 
                              p_key_system, 
                              KEYSYSTEM_RECV_DEBUGWIN_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Use_01, 
                              DebugWin_D_Key_Use_01, 
                              p_key_system, 
                              KEYSYSTEM_RECV_DEBUGWIN_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_Key_Use_02, 
                              DebugWin_D_Key_Use_02, 
                              p_key_system, 
                              KEYSYSTEM_RECV_DEBUGWIN_GROUP, 
                              heapID );
}

static void DEBUGWIN_KEY_SYSTEM_Exit( void )
{
  DEBUGWIN_RemoveGroup( KEYSYSTEM_DEBUGWIN_GROUP );
}
#endif //PM_DEBUG
