//==============================================================================
/**
 * @file    comm_entry_menu.h
 * @brief   参加募集メニュー制御のヘッダ
 * @author  matsuda
 * @date    2009.07.28(火)
 */
//==============================================================================
#pragma once

//==============================================================================
//  定数定義
//==============================================================================
///最大参加人数
#define COMM_ENTRY_USER_MAX     (4)

//==============================================================================
//  型定義
//==============================================================================
typedef struct _COMM_ENTRY_MENU_SYSTEM * COMM_ENTRY_MENU_PTR;

///受け入れ結果
typedef enum{
  COMM_ENTRY_ANSWER_NULL,
  COMM_ENTRY_ANSWER_COMPLETION,   ///<受け入れOK
  COMM_ENTRY_ANSWER_REFUSE,       ///<受け入れNG
}COMM_ENTRY_ANSWER;

//==============================================================================
//  外部関数宣言
//==============================================================================
extern COMM_ENTRY_MENU_PTR CommEntryMenu_Setup(FIELDMAP_WORK *fieldWork, int min_num, int max_num, HEAPID heap_id);
extern void CommEntryMenu_Exit(COMM_ENTRY_MENU_PTR em);
extern void CommEntryMenu_Entry(COMM_ENTRY_MENU_PTR em, int netID, const STRCODE *name, u32 id, u8 sex, BOOL force_entry);
extern BOOL CommEntryMenu_Update(COMM_ENTRY_MENU_PTR em);
extern COMM_ENTRY_ANSWER CommEntryMenu_GetAnswer(COMM_ENTRY_MENU_PTR em, int netID, BOOL exit_mode);
extern void CommEntryMenu_SetSendFlag(COMM_ENTRY_MENU_PTR em, int netID);
extern int CommEntryMenu_GetCompletionNum(COMM_ENTRY_MENU_PTR em);
