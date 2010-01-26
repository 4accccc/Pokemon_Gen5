//==============================================================================
/**
 * @file    union_app.h
 * @brief   ユニオンルームでのミニゲーム制御系
 * @author  matsuda
 * @date    2010.01.06(水)
 */
//==============================================================================
#pragma once

#include "gamesystem/gamedata_def.h"
#include "savedata/mystatus.h"
#include "net_app/union/union_types.h"


//==============================================================================
//  定数定義
//==============================================================================
///途中退出リクエストの親機からの返事
typedef enum{
  UNION_APP_LEAVE_NULL,   ///<返事未受信
  UNION_APP_LEAVE_OK,     ///<退出OK
  UNION_APP_LEAVE_NG,     ///<退出NG
}UNION_APP_LEAVE;


//==============================================================================
//  型定義
//==============================================================================
///乱入コールバック関数の型
typedef void (*UNION_APP_CALLBACK_ENTRY_FUNC)(NetID net_id, const MYSTATUS *mystatus, void *userwork);
///退出コールバック関数の型
typedef void (*UNION_APP_CALLBACK_LEAVE_FUNC)(NetID net_id, const MYSTATUS *mystatus, void *userwork);


//--------------------------------------------------------------
//  各ミニゲーム用のParentWork
//--------------------------------------------------------------
///お絵かき用ParentWork
typedef struct{
  UNION_APP_PTR uniapp;
  GAMEDATA *gamedata;
}PICTURE_PARENT_WORK;

///ぐるぐる交換用ParentWork
typedef struct{
  UNION_APP_PTR uniapp;
  GAMEDATA *gamedata;
  POKEPARTY *party;
}GURUGURU_PARENT_WORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
//--------------------------------------------------------------
//  システムで使用
//--------------------------------------------------------------
extern UNION_APP_PTR UnionAppSystem_AllocAppWork(
  HEAPID heap_id, u8 member_max, const MYSTATUS *myst);
extern void UnionAppSystem_FreeAppWork(UNION_APP_PTR uniapp);
extern BOOL UnionAppSystem_SetEntryUser(UNION_APP_PTR uniapp, NetID net_id, const MYSTATUS *myst);
extern void UnionAppSystem_Update(UNION_APP_PTR uniapp, UNION_SYSTEM_PTR unisys);
extern void UnionAppSystem_ReqBasicStatus(UNION_APP_PTR uniapp, NetID net_id);
extern void UnionAppSystem_SetBasicStatus(
  UNION_APP_PTR uniapp, const UNION_APP_BASIC *basic_status);
extern void UnionAppSystem_ReqMystatus(UNION_APP_PTR uniapp, NetID net_id);
extern void UnionAppSystem_SetMystatus(UNION_APP_PTR uniapp, NetID net_id, const MYSTATUS *myst);
extern BOOL UnionAppSystem_CheckBasicStatus(UNION_APP_PTR uniapp);
extern BOOL UnionAppSystem_CheckMystatus(UNION_APP_PTR uniapp);
extern void UnionAppSystem_SetIntrudeReady(UNION_APP_PTR uniapp, NetID net_id);
extern void UnionAppSystem_SetLeaveChild(UNION_APP_PTR uniapp, NetID net_id);
extern u32 UnionAppSystem_GetBasicSize(void);


//--------------------------------------------------------------
//  以下、アプリケーション用
//--------------------------------------------------------------
//==================================================================
/**
 * 乱入、退出時のコールバック関数を登録
 *
 * @param   uniapp		        ユニオンアプリ制御ワークへのポインタ
 * @param   entry_callback		乱入コールバック
 * @param   eave_callback		  退出コールバック
 * @param   userwork		      コールバック関数で引数として渡されるワークのポインタ
 */
//==================================================================
extern void Union_App_SetCallback(UNION_APP_PTR uniapp, 
  UNION_APP_CALLBACK_ENTRY_FUNC entry_callback, UNION_APP_CALLBACK_LEAVE_FUNC leave_callback, 
  void *userwork);

//==================================================================
/**
 * 乱入を禁止する　※親機専用命令
 *
 * @param   uniapp		
 *
 * @retval  BOOL		  TRUE:禁止にした。　FALSE:禁止に出来ない(既に乱入希望者がいる)
 */
//==================================================================
extern BOOL Union_App_Parent_EntryBlock(UNION_APP_PTR uniapp);

//==================================================================
/**
 * 乱入禁止をリセットする　※親機専用命令
 *
 * @param   uniapp		
 */
//==================================================================
extern void Union_App_Parent_ResetEntryBlock(UNION_APP_PTR uniapp);

//==================================================================
/**
 * 退出を禁止する(Union_App_ReqLeaveの結果が常にNGになります)　※親機専用命令
 *
 * @param   uniapp		
 */
//==================================================================
extern void Union_App_Parent_LeaveBlock(UNION_APP_PTR uniapp);

//==================================================================
/**
 * 退出禁止をリセットします　※親機専用命令
 *
 * @param   uniapp		
 */
//==================================================================
extern void Union_App_Parent_ResetLeaveBlock(UNION_APP_PTR uniapp);

//==================================================================
/**
 * 親機に途中退出の許可をリクエストします
 *
 * @param   uniapp		
 *
 * この関数実行後、Union_App_GetLeaveResultで結果の取得を行ってください
 */
//==================================================================
extern void Union_App_ReqLeave(UNION_APP_PTR uniapp);

//==================================================================
/**
 * 途中退出リクエストの親機からの返事を取得します
 *
 * @param   uniapp		
 *
 * @retval  UNION_APP_LEAVE		親機からの返事
 */
//==================================================================
extern UNION_APP_LEAVE Union_App_GetLeaveResult(UNION_APP_PTR uniapp);

//==================================================================
/**
 * 通信切断します
 *
 * @param   uniapp		
 */
//==================================================================
extern void Union_App_Shutdown(UNION_APP_PTR uniapp);

//==================================================================
/**
 * 通信切断の完了待ち
 *
 * @param   uniapp		
 *
 * @retval  BOOL		TRUE:通信切断完了　FALSE:未完了
 */
//==================================================================
extern BOOL Union_App_WaitShutdown(UNION_APP_PTR uniapp);

//==================================================================
/**
 * 指定NetIDのMYSTATUSを取得します
 *
 * @param   net_id		取得したい相手のNetID
 *
 * @retval  const MYSTATUS *		MYSTATUSへのポインタ(NetIDのユーザーが退出している場合はNULL)
 */
//==================================================================
extern const MYSTATUS * Union_App_GetMystatus(UNION_APP_PTR uniapp, NetID net_id);

//==================================================================
/**
 * 接続しているプレイヤーのNetIDをbit単位で取得します
 *    ※GFL_NET_SendDataExBitでそのまま使える形です
 *
 * @param   uniapp		
 *
 * @retval  u32		接続プレイヤーNetID(bit単位)
 *
 * NetID:0番 NetID:2番 のプレイヤーだけが繋がっている場合は
 *    return (1 << 0) | (1 << 2);
 * といった値が返ります
 */
//==================================================================
extern u32 Union_App_GetMemberNetBit(UNION_APP_PTR uniapp);

//==================================================================
/**
 * 接続人数を取得します
 *
 * @param   uniapp		
 *
 * @retval  u8		接続人数
 */
//==================================================================
extern u8 Union_App_GetMemberNum(UNION_APP_PTR uniapp);
