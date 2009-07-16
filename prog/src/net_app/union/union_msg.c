//==============================================================================
/**
 * @file    union_msg.c
 * @brief   ユニオンルーム：ツール類(オーバーレイ領域に配置されます)
 * @author  matsuda
 * @date    2009.07.13(月)
 */
//==============================================================================
#include <gflib.h>
#include "union_types.h"
#include "union_local.h"
#include "net_app/union/union_main.h"
#include "field/field_player.h"
#include "field/fieldmap.h"
#include "net_app/union/union_msg.h"
#include "message.naix"
#include "msg/msg_union.h"
#include "system/main.h"


//==============================================================================
//  データ
//==============================================================================
//--------------------------------------------------------------
//  メニューヘッダー
//--------------------------------------------------------------
///メインメニューリスト
static const FLDMENUFUNC_LIST MainMenuList[] =
{
  {msg_union_select_01, (void*)UNION_MSG_MENU_SELECT_AISATU},   //挨拶
  {msg_union_select_02, (void*)UNION_MSG_MENU_SELECT_OEKAKI},   //お絵かき
  {msg_union_select_03, (void*)UNION_MSG_MENU_SELECT_TAISEN},   //対戦
  {msg_union_select_04, (void*)UNION_MSG_MENU_SELECT_KOUKAN},   //交換
  {msg_union_select_06, (void*)UNION_MSG_MENU_SELECT_GURUGURU}, //ぐるぐる交換
  {msg_union_select_07, (void*)UNION_MSG_MENU_SELECT_RECORD},   //レコードコーナー
  {msg_union_select_05, (void*)UNION_MSG_MENU_SELECT_CANCEL},   //やめる
};

///メニューヘッダー(メインメニュー用)
static const FLDMENUFUNC_HEADER MenuHeader_MainMenu =
{
	7,		//リスト項目数
	7,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_NON,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	16,		//文字サイズY(ドット
	32-14,		//表示座標X キャラ単位
	1,		//表示座標Y キャラ単位
	14,		//表示サイズX キャラ単位
	14,		//表示サイズY キャラ単位
};
//SDK_COMPILER_ASSERT(NELEMS(MainMenuList) == MenuHeader_MainMenu.line);



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 会話ウィンドウ：セットアップ
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_TalkStream_WindowSetup(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork)
{
  FLDMSGBG *fldmsg_bg;
  
  if(unisys->fld_msgwin_stream == NULL){
    GF_ASSERT(unisys->msgdata == NULL);
    fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
    unisys->msgdata = FLDMSGBG_CreateMSGDATA( fldmsg_bg, NARC_message_union_dat );
    unisys->fld_msgwin_stream = FLDMSGWIN_STREAM_AddTalkWin(fldmsg_bg, unisys->msgdata);
  }
  else{ //既にウィンドウが表示されている場合はメッセージエリアのクリアを行う
    FLDMSGWIN_STREAM_ClearMessage(unisys->fld_msgwin_stream);
  }
}

//==================================================================
/**
 * 会話ウィンドウ：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_TalkStream_WindowDel(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fld_msgwin_stream != NULL){
    FLDMSGWIN_STREAM_ClearWindow(unisys->fld_msgwin_stream);
    FLDMSGWIN_STREAM_Delete(unisys->fld_msgwin_stream);
    unisys->fld_msgwin_stream = NULL;
  }

  if(unisys->msgdata != NULL){
    FLDMSGBG_DeleteMSGDATA(unisys->msgdata);
    unisys->msgdata = NULL;
  }
}

//==================================================================
/**
 * 会話ウィンドウ：出力開始
 *
 * @param   unisys		
 * @param   str_id		メッセージID
 */
//==================================================================
void UnionMsg_TalkStream_Print(UNION_SYSTEM_PTR unisys, u32 str_id)
{
  FLDMSGPRINT *msg_print;
  
  GF_ASSERT(unisys->fld_msgwin_stream != NULL);
  FLDMSGWIN_STREAM_PrintStart(unisys->fld_msgwin_stream, 0, 0, str_id);
}

//==================================================================
/**
 * 会話ウィンドウ：終了チェック
 *
 * @param   unisys		
 *
 * @retval  BOOL		TRUE:終了。　FALSE:継続中
 */
//==================================================================
BOOL UnionMsg_TalkStream_Check(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fld_msgwin_stream == NULL){
    return TRUE;
  }
  return FLDMSGWIN_STREAM_Print(unisys->fld_msgwin_stream);
}

//==================================================================
/**
 * 会話ウィンドウ：出力開始に加え、メッセージウィンドウが無い場合は生成も行う
 *
 * @param   unisys		  
 * @param   fieldWork		
 * @param   str_id		  メッセージID
 */
//==================================================================
void UnionMsg_TalkStream_PrintPack(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork, u32 str_id)
{
  UnionMsg_TalkStream_WindowSetup(unisys, fieldWork);
  UnionMsg_TalkStream_Print(unisys, str_id);
}

//==================================================================
/**
 * 「はい」「いいえ」選択ウィンドウセットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_YesNo_Setup(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork)
{
  FLDMSGBG *fldmsg_bg;

  if(unisys->fldmenu_yesno_func == NULL){
    fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
    unisys->fldmenu_yesno_func = FLDMENUFUNC_AddYesNoMenu(fldmsg_bg, FLDMENUFUNC_YESNO_YES);
  }
}

//==================================================================
/**
 * 「はい」「いいえ」選択ウィンドウ削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_YesNo_Del(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fldmenu_yesno_func != NULL){
    FLDMENUFUNC_DeleteMenu(unisys->fldmenu_yesno_func);
    unisys->fldmenu_yesno_func = NULL;
  }
}

//==================================================================
/**
 * 「はい」「いいえ」選択待ち
 *
 * @param   unisys		
 * @param   result		結果代入先(TRUE:はい、FALSE:いいえ)
 *
 * @retval  BOOL		TRUE:選択した。　FALSE:選択中
 */
//==================================================================
BOOL UnionMsg_YesNo_SelectLoop(UNION_SYSTEM_PTR unisys, BOOL *result)
{
  FLDMENUFUNC_YESNO yes_no;
  
  GF_ASSERT(unisys->fldmenu_yesno_func != NULL);
  
  yes_no = FLDMENUFUNC_ProcYesNoMenu(unisys->fldmenu_yesno_func);
  if(yes_no == FLDMENUFUNC_YESNO_NULL){
    return FALSE;
  }
  else if(yes_no == FLDMENUFUNC_YESNO_YES){
    *result = TRUE;
  }
  else{
    *result = FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * メニューリスト：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 * @param   menulist		
 * @param   list_max		
 */
//--------------------------------------------------------------
static void UnionMsg_Menu_WindowSetup(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork, const FLDMENUFUNC_LIST *menulist, int list_max, const FLDMENUFUNC_HEADER *menuhead)
{
  FLDMSGBG *fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
  
  if(unisys->fldmenu_listdata == NULL){
    GF_ASSERT(unisys->fldmenu_listdata == NULL && unisys->fldmenu_func == NULL);
    unisys->fldmenu_listdata = FLDMENUFUNC_CreateMakeListData(
      menulist, list_max, unisys->msgdata, HEAPID_UNION);
    unisys->fldmenu_func = FLDMENUFUNC_AddMenu(fldmsg_bg, menuhead, unisys->fldmenu_listdata);
  }
}

//--------------------------------------------------------------
/**
 * メニューリスト：削除
 *
 * @param   unisys		
 */
//--------------------------------------------------------------
static void UnionMsg_Menu_WindowDel(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fldmenu_func != NULL){
    FLDMENUFUNC_DeleteMenu(unisys->fldmenu_func);
    unisys->fldmenu_func = NULL;
  }

  if(unisys->fldmenu_listdata != NULL){
    FLDMENUFUNC_DeleteListData(unisys->fldmenu_listdata);
    unisys->fldmenu_listdata = NULL;
  }
}

//--------------------------------------------------------------
/**
 * メニューリスト：選択待ち
 *
 * @param   unisys		
 *
 * @retval  u32		結果
 */
//--------------------------------------------------------------
static u32 UnionMsg_Menu_SelectLoop(UNION_SYSTEM_PTR unisys)
{
  GF_ASSERT(unisys->fldmenu_func != NULL);
  return FLDMENUFUNC_ProcMenu(unisys->fldmenu_func);
}

//==================================================================
/**
 * メインメニュー：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_Menu_MainMenuSetup(UNION_SYSTEM_PTR unisys, FIELD_MAIN_WORK *fieldWork)
{
  UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
    MainMenuList, NELEMS(MainMenuList), &MenuHeader_MainMenu);
}

//==================================================================
/**
 * メインメニュー：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_MainMenuDel(UNION_SYSTEM_PTR unisys)
{
  UnionMsg_Menu_WindowDel(unisys);
}

//==================================================================
/**
 * メインメニュー：選択待ち
 *
 * @param   unisys		
 * 
 * @retval  u32		結果
 */
//==================================================================
u32 UnionMsg_Menu_MainMenuSelectLoop(UNION_SYSTEM_PTR unisys)
{
  return UnionMsg_Menu_SelectLoop(unisys);
}

