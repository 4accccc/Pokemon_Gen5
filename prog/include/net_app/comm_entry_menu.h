//==============================================================================
/**
 * @file    comm_entry_menu.h
 * @brief   参加募集メニュー制御のヘッダ
 * @author  matsuda
 * @date    2009.07.28(火)
 */
//==============================================================================
#pragma once

#include "savedata/mystatus.h"
#include "savedata/mystatus_local.h"
#include "field/fieldmap_proc.h"


//==============================================================================
//  定数定義
//==============================================================================
///最大参加人数
#define COMM_ENTRY_USER_MAX     (4)

///接続モード
typedef enum{
  ///親機
  COMM_ENTRY_MODE_PARENT,
  ///子機(親機選択)
  COMM_ENTRY_MODE_CHILD,
  ///子機(既に親とネゴシエーション込みで接続済み) ※コロシアム最初のメンバー
  COMM_ENTRY_MODE_CHILD_PARENT_CONNECTED,   
  ///子機(接続する親機を外部から指定) ※コロシアム乱入
  COMM_ENTRY_MODE_CHILD_PARENT_DESIGNATE,
}COMM_ENTRY_MODE;

///開催するゲームの種類
typedef enum{
  COMM_ENTRY_GAMETYPE_COLOSSEUM,    ///<コロシアム
  COMM_ENTRY_GAMETYPE_MUSICAL,      ///<ミュージカル
  COMM_ENTRY_GAMETYPE_SUBWAY,       ///<地下鉄
  
  COMM_ENTRY_GAMETYPE_MAX,
}COMM_ENTRY_GAMETYPE;


//==============================================================================
//  型定義
//==============================================================================
typedef struct _COMM_ENTRY_MENU_SYSTEM * COMM_ENTRY_MENU_PTR;
typedef struct _ENTRYMENU_MEMBER_INFO ENTRYMENU_MEMBER_INFO;
typedef void (*COMM_ENTRY_MENU_CALLBACK)(void *work, const MYSTATUS *myst, const u8 *mac_address);

///受け入れ結果
typedef enum{
  COMM_ENTRY_ANSWER_NULL,
  COMM_ENTRY_ANSWER_COMPLETION,   ///<受け入れOK
  COMM_ENTRY_ANSWER_REFUSE,       ///<受け入れNG
}COMM_ENTRY_ANSWER;

///結果
typedef enum{
  COMM_ENTRY_RESULT_NULL,         ///<処理実行中
  COMM_ENTRY_RESULT_SUCCESS,      ///<メンバーが集まった
  COMM_ENTRY_RESULT_CANCEL,       ///<キャンセルして終了
  COMM_ENTRY_RESULT_NG,           ///<断られて終了
  COMM_ENTRY_RESULT_ERROR,        ///<エラーで終了
}COMM_ENTRY_RESULT;

///子機がエントリーした時、親機からの返事
typedef enum{
  ENTRY_PARENT_ANSWER_NULL,             ///<返事未受信
  ENTRY_PARENT_ANSWER_OK,               ///<エントリーOK
  ENTRY_PARENT_ANSWER_NG,               ///<エントリーNG
}ENTRY_PARENT_ANSWER;


//==============================================================================
//  構造体定義
//==============================================================================
///エントリーメニューを使用して参加する為のビーコン情報
#define COMM_ENTRY_MAGIC_NUMBER (0x3a0b)
typedef struct{
  MYSTATUS mystatus;    ///<自身のMyStatus
  u8 mac_address[6];    ///<自身のMacAddress
  u16 magic_number;     ///<チェック用マジックナンバー
}COMM_ENTRY_BEACON;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern COMM_ENTRY_MENU_PTR CommEntryMenu_Setup(const MYSTATUS *myst, FIELDMAP_WORK *fieldWork, int min_num, int max_num, HEAPID heap_id, COMM_ENTRY_MODE entry_mode, COMM_ENTRY_GAMETYPE game_type, const u8 *parent_mac_address);
extern u32 CommEntryMenu_Exit(COMM_ENTRY_MENU_PTR em);
extern void CommEntryMenu_SetCallback_EntryLeave(COMM_ENTRY_MENU_PTR em, void *work, COMM_ENTRY_MENU_CALLBACK entry_func, COMM_ENTRY_MENU_CALLBACK leave_func);
extern void CommEntryMenu_Entry(COMM_ENTRY_MENU_PTR em, int netID, const MYSTATUS *myst, BOOL force_entry, const u8 *mac_address);
extern COMM_ENTRY_RESULT CommEntryMenu_Update(COMM_ENTRY_MENU_PTR em);
extern void CommEntryMenu_SetGameStartReady(COMM_ENTRY_MENU_PTR em, NetID net_id);
extern void CommEntryMenu_SetGameStart(COMM_ENTRY_MENU_PTR em);
extern void CommEntryMenu_SetGameCancel(COMM_ENTRY_MENU_PTR em);
extern void CommEntryMenu_SetForbitLeave(COMM_ENTRY_MENU_PTR em);
extern COMM_ENTRY_ANSWER CommEntryMenu_GetAnswer(COMM_ENTRY_MENU_PTR em, int netID, BOOL exit_mode);
extern void CommEntryMenu_SetSendFlag(COMM_ENTRY_MENU_PTR em, int netID);
extern int CommEntryMenu_GetCompletionNum(COMM_ENTRY_MENU_PTR em);
extern u32 CommEntryMenu_GetCompletionBit(COMM_ENTRY_MENU_PTR em);
extern void CommEntyrMenu_MemberInfoReserveUpdate(COMM_ENTRY_MENU_PTR em);
extern ENTRYMENU_MEMBER_INFO * CommEntryMenu_GetMemberInfo(COMM_ENTRY_MENU_PTR em);
extern void CommEntryMenu_RecvMemberInfo(COMM_ENTRY_MENU_PTR em, const ENTRYMENU_MEMBER_INFO *member_info);
extern void CommEntryMenu_SetEntryAnswer(COMM_ENTRY_MENU_PTR em, ENTRY_PARENT_ANSWER answer, const MYSTATUS *myst);
extern u32 CommEntryMenu_GetMemberInfoSize(void);
