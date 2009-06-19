//======================================================================
/**
 * @file	plist_menu.c
 * @brief	ポケモンリスト メニュー
 * @author	ariizumi
 * @data	09/06/18
 *
 * モジュール名：PLIST_MENU
 */
//======================================================================
#pragma once

#include "plist_local_def.h"

//メニューを初期化する際に渡すアイテム群
//上から並べる
typedef enum
{
  PMIT_STATSU,  //強さを見る
  PMIT_HIDEN,   //秘伝技。自動で個数分追加します
  PMIT_CHANGE,  //入れ替え
  PMIT_ITEM,    //持ち物
  PMIT_CLOSE,   //閉じる
  PMIT_LEAVE,   //預ける(育てや
  PMIT_JOIN,    //参加する・参加しない

  PMIT_END_LIST,    //リスト終端用
  
}PLIST_MENU_ITEM_TYPE;

extern PLIST_MENU_WORK* PLIST_MENU_CreateSystem( PLIST_WORK *work );
extern void PLIST_MENU_DeleteSystem( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );

extern void PLIST_MENU_OpenMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork , PLIST_MENU_ITEM_TYPE *itemArr );
extern void PLIST_MENU_CloseMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );
void PLIST_MENU_UpdateMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );

