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
//後半は決定後の戻り値にも使う
typedef enum
{
	
  PMIT_STATSU,  //強さを見る
  PMIT_HIDEN,   //秘伝技。自動で個数分追加します
  PMIT_CHANGE,  //入れ替え
  PMIT_ITEM,    //持ち物
  PMIT_CLOSE,   //閉じる
  PMIT_LEAVE,   //預ける(育てや
  PMIT_SET_JOIN,    //参加する(参加しない)

  PMIT_GIVE,    //持たせる
  PMIT_TAKE,    //預かる
  
  PMIT_YES,    //はい
  PMIT_NO,     //いいえ

  PMIT_END_LIST,    //リスト終端用
  
  //以下戻り値専用
  PMIT_WAZA_1 = PMIT_END_LIST,   //秘伝用技スロット１
  PMIT_WAZA_2,   //秘伝用技スロット２
  PMIT_WAZA_3,   //秘伝用技スロット３
  PMIT_WAZA_4,   //秘伝用技スロット４
  
  PMIT_RET_JOIN,      //参加する
  PMIT_JOIN_CANCEL,   //参加しない
  
  PMIT_MAX,
  
  PMIT_NONE,    //選択中
  
}PLIST_MENU_ITEM_TYPE;

extern PLIST_MENU_WORK* PLIST_MENU_CreateSystem( PLIST_WORK *work );
extern void PLIST_MENU_DeleteSystem( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );

extern void PLIST_MENU_OpenMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork , PLIST_MENU_ITEM_TYPE *itemArr );
extern void PLIST_MENU_OpenMenu_YesNo( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );
extern void PLIST_MENU_CloseMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );
extern void PLIST_MENU_UpdateMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );

extern const PLIST_MENU_ITEM_TYPE PLIST_MENU_IsFinish( PLIST_WORK *work , PLIST_MENU_WORK *menuWork );
//バトル参加決定時下に出るやつ
extern GFL_BMPWIN* PLIST_MENU_CreateMenuWin_BattleMenu( PLIST_WORK *work , PLIST_MENU_WORK *menuWork , u32 strId , u8 charX , u8 charY , const BOOL isReturn );
