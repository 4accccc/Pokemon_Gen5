//==============================================================================
/**
 * @file    union_msg.h
 * @brief   ユニオンルーム：ツール類(オーバーレイ領域に配置されます)
 * @author  matsuda
 * @date    2009.07.13(月)
 */
//==============================================================================
#pragma once

//==============================================================================
//  定数定義
//==============================================================================
///UnionMsg_Menu_SelectLoop関数の戻り値   ※MenuMemberMaxテーブルと並びを同じにしておくこと！
enum{
  UNION_MSG_MENU_SELECT_AISATU,       ///<挨拶
  UNION_MSG_MENU_SELECT_OEKAKI,       ///<お絵かき
  UNION_MSG_MENU_SELECT_BATTLE_2VS2_SINGLE_50,        ///<対戦:2VS2:シングル:LV50
  UNION_MSG_MENU_SELECT_BATTLE_2VS2_SINGLE_FREE,      ///<対戦:2VS2:シングル:制限なし
  UNION_MSG_MENU_SELECT_BATTLE_2VS2_SINGLE_STANDARD,  ///<対戦:2VS2:シングル:スタンダード
  UNION_MSG_MENU_SELECT_KOUKAN,       ///<交換
  UNION_MSG_MENU_SELECT_GURUGURU,     ///<ぐるぐる交換
  UNION_MSG_MENU_SELECT_RECORD,       ///<レコードコーナー
  UNION_MSG_MENU_SELECT_CANCEL,       ///<やめる
  
  UNION_MSG_MENU_SELECT_MAX,    //↓以下、送信される事はない番号(ローカルメニューの判定でのみ使用)
  UNION_MSG_MENU_SELECT_NO_SEND_BATTLE,       ///<対戦(これが送信される事はない)
  UNION_MSG_MENU_SELECT_NULL = 0xff,  ///<何も選択されていない(初期値として使用)
};


//==============================================================================
//  外部関数宣言
//==============================================================================
extern int UnionMsg_GetMemberMax(int menu_index);
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
