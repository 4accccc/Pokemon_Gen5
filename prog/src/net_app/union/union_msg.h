//==============================================================================
/**
 * @file    union_msg.h
 * @brief   ユニオンルーム：ツール類(オーバーレイ領域に配置されます)
 * @author  matsuda
 * @date    2009.07.13(月)
 */
//==============================================================================
#pragma once

#include "union_types.h"

//==============================================================================
//  定数定義
//==============================================================================
///UnionMsg_Menu_SelectLoop関数の戻り値(PLAY_CATEGORY以外の指定)
enum{
  UNION_MENU_SELECT_CANCEL = UNION_PLAY_CATEGORY_UNION,///<キャンセル(ユニオンに戻るという意味)
  
  //↓以下、送信される事はない番号(ローカルメニューの判定でのみ使用)
  //  ローカルでしか使用しないので将来の番号と被っても大丈夫
  UNION_MENU_NO_SEND_BATTLE = UNION_PLAY_CATEGORY_MAX,       ///<対戦(これが送信される事はない)
  UNION_MENU_SELECT_NULL = 0xff,  ///<何も選択されていない(初期値として使用)
};


//==============================================================================
//  外部関数宣言
//==============================================================================
extern int UnionMsg_GetMemberMax(UNION_PLAY_CATEGORY menu_index);
extern void UnionMsg_AllDel(UNION_SYSTEM_PTR unisys);

extern void UnionMsg_TalkStream_WindowSetup(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork);
extern void UnionMsg_TalkStream_WindowDel(UNION_SYSTEM_PTR unisys);
extern void UnionMsg_TalkStream_Print(UNION_SYSTEM_PTR unisys, u32 str_id);
extern BOOL UnionMsg_TalkStream_Check(UNION_SYSTEM_PTR unisys);
extern void UnionMsg_TalkStream_PrintPack(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork, u32 str_id);

extern void UnionMsg_YesNo_Setup(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork);
extern void UnionMsg_YesNo_Del(UNION_SYSTEM_PTR unisys);
extern BOOL UnionMsg_YesNo_SelectLoop(UNION_SYSTEM_PTR unisys, BOOL *result);

extern void UnionMsg_Menu_MainMenuSetup(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork);
extern void UnionMsg_Menu_MainMenuDel(UNION_SYSTEM_PTR unisys);
extern u32 UnionMsg_Menu_MainMenuSelectLoop(UNION_SYSTEM_PTR unisys);

extern void UnionMsg_Menu_BattleMenuSetup(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork, int menu_index);
extern void UnionMsg_Menu_BattleMenuDel(UNION_SYSTEM_PTR unisys);
extern u32 UnionMsg_Menu_BattleMenuSelectLoop(UNION_SYSTEM_PTR unisys, BOOL *next_sub_menu);
