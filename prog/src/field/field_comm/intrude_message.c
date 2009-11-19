//==============================================================================
/**
 * @file    intrude_message.c
 * @brief   侵入イベント：メッセージ共通処理
 * @author  matsuda
 * @date    2009.10.27(火)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "field/fieldmap.h"
#include "field/scrcmd.h"
#include "intrude_message.h"
#include "message.naix"
#include "msg/msg_invasion.h"
#include "field/field_comm/intrude_types.h"


//==============================================================================
//  定数定義
//==============================================================================
///侵入イベントで使用するヒープID
#define INTRUDE_EVENT_HEAPID        (HEAPID_PROC)
///侵入イベントで使用するメッセージバッファサイズ
#define INTRUDE_EVENT_MSGBUF_SIZE   (256)


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void IntrudeEventPrint_ClearStream(INTRUDE_EVENT_MSGWORK *iem);
static void IntrudeEventPrint_SetupMenu(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys,
  const FLDMENUFUNC_LIST *menulist, int list_max, const FLDMENUFUNC_HEADER *head);


//==============================================================================
//
//  侵入イベント共通メッセージ処理
//
//==============================================================================
//--------------------------------------------------------------
/**
 * プリント制御セットアップ
 *
 * @param   iem		
 * @param   gsys		
 */
//--------------------------------------------------------------
void IntrudeEventPrint_SetupFieldMsg(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys)
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldWork);
  
  iem->wordset = WORDSET_Create(INTRUDE_EVENT_HEAPID);
  iem->msgbuf = GFL_STR_CreateBuffer( INTRUDE_EVENT_MSGBUF_SIZE, INTRUDE_EVENT_HEAPID );
  iem->tmpbuf = GFL_STR_CreateBuffer( INTRUDE_EVENT_MSGBUF_SIZE, INTRUDE_EVENT_HEAPID );
  iem->msgdata = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_invasion_dat );
  iem->msgwin_stream = FLDMSGWIN_STREAM_AddTalkWin( msgBG, iem->msgdata );
  iem->print_flag = FALSE;
}

//--------------------------------------------------------------
/**
 * プリント制御削除
 *
 * @param   iem		
 */
//--------------------------------------------------------------
void IntrudeEventPrint_ExitFieldMsg(INTRUDE_EVENT_MSGWORK *iem)
{
  if(iem->msgwin_stream != NULL){
    FLDMSGWIN_STREAM_Delete(iem->msgwin_stream);
    FLDMSGBG_DeleteMSGDATA(iem->msgdata);
    GFL_STR_DeleteBuffer(iem->tmpbuf);
    GFL_STR_DeleteBuffer(iem->msgbuf);
    WORDSET_Delete(iem->wordset);
    
    iem->msgwin_stream = NULL;  //解放した事の印
  }
}

//--------------------------------------------------------------
/**
 * プリントストリーム出力
 *
 * @param   iem		
 * @param   msg_id		
 */
//--------------------------------------------------------------
void IntrudeEventPrint_StartStream(INTRUDE_EVENT_MSGWORK *iem, u16 msg_id)
{
  if(iem->print_flag == TRUE){
    IntrudeEventPrint_ClearStream(iem);
  }
  
  GFL_MSG_GetString( iem->msgdata, msg_id, iem->tmpbuf );
  WORDSET_ExpandStr( iem->wordset, iem->msgbuf, iem->tmpbuf );

  FLDMSGWIN_STREAM_PrintStrBufStart( iem->msgwin_stream, 0, 0, iem->msgbuf );
  iem->print_flag = TRUE;
}

//==================================================================
/**
 * メッセージ領域をクリア
 *
 * @param   iem		
 */
//==================================================================
static void IntrudeEventPrint_ClearStream(INTRUDE_EVENT_MSGWORK *iem)
{
  GF_ASSERT(iem->msgwin_stream != NULL);
  FLDMSGWIN_STREAM_ClearMessage(iem->msgwin_stream);
  iem->print_flag = FALSE;
}

//--------------------------------------------------------------
/**
 * プリントストリーム出力待ち
 *
 * @param   iem		
 *
 * @retval  BOOL TRUE=表示終了,FALSE=表示中
 */
//--------------------------------------------------------------
BOOL IntrudeEventPrint_WaitStream(INTRUDE_EVENT_MSGWORK *iem)
{
  return FLDMSGWIN_STREAM_Print(iem->msgwin_stream);
}

//==================================================================
/**
 * ラストキー待ち
 *
 * @retval  BOOL		TRUE：ラストキーが押された
 */
//==================================================================
BOOL IntrudeEventPrint_LastKeyWait(void)
{
  if( GFL_UI_KEY_GetTrg() & EVENT_WAIT_LAST_KEY ){
    return TRUE;
  }
  return FALSE;
}




//==============================================================================
//
//  メニュー
//
//==============================================================================
///会話メインメニューリスト
static const FLDMENUFUNC_LIST TalkMainMenuList[] =
{
  {msg_talk_menu_001, (void*)INTRUDE_TALK_STATUS_BATTLE},   //対戦
  {msg_talk_menu_002, (void*)INTRUDE_TALK_STATUS_ITEM},     //アイテム渡す
  {msg_talk_menu_003, (void*)INTRUDE_TALK_STATUS_CANCEL},   //キャンセル
};

///メニューヘッダー(メインメニュー用)
static const FLDMENUFUNC_HEADER MenuHeader_MainMenu =
{
	3,		//リスト項目数
	3,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	12,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	0,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_NON,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	16,		//文字サイズY(ドット
	32-15,		//表示座標X キャラ単位
	1,		//表示座標Y キャラ単位
	14,		//表示サイズX キャラ単位
	6,		//表示サイズY キャラ単位
};

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
static void IntrudeEventPrint_SetupMenu(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys,
  const FLDMENUFUNC_LIST *menulist, int list_max, const FLDMENUFUNC_HEADER *head)
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldWork);
  FLDMENUFUNC_LISTDATA *fldmenu_listdata;
  
  GF_ASSERT(iem->fldmenu_func == NULL && iem->msgdata != NULL);
  
  fldmenu_listdata = FLDMENUFUNC_CreateMakeListData(
      menulist, list_max, iem->msgdata, INTRUDE_EVENT_HEAPID);
  iem->fldmenu_func = FLDMENUFUNC_AddMenu(msgBG, head, fldmenu_listdata);
}

//--------------------------------------------------------------
/**
 * メニューリスト：削除
 *
 * @param   unisys		
 */
//--------------------------------------------------------------
void IntrudeEventPrint_ExitMenu(INTRUDE_EVENT_MSGWORK *iem)
{
  if(iem->fldmenu_func != NULL){
    FLDMENUFUNC_DeleteMenu(iem->fldmenu_func);
    iem->fldmenu_func = NULL;
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
u32 IntrudeEventPrint_SelectMenu(INTRUDE_EVENT_MSGWORK *iem)
{
  GF_ASSERT(iem->fldmenu_func != NULL);
  return FLDMENUFUNC_ProcMenu(iem->fldmenu_func);
}

//--------------------------------------------------------------
/**
 * メインメニューリスト：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 * @param   menulist		
 * @param   list_max		
 */
//--------------------------------------------------------------
void IntrudeEventPrint_SetupMainMenu(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys)
{
  IntrudeEventPrint_SetupMenu(iem, gsys, 
    TalkMainMenuList, NELEMS(TalkMainMenuList), &MenuHeader_MainMenu);
}

//==============================================================================
//  はい/いいえ 選択メニュー
//==============================================================================
//==================================================================
/**
 * はい/いいえ 選択メニュー：セットアップ
 *
 * @param   iem		
 * @param   gsys		
 */
//==================================================================
void IntrudeEventPrint_SetupYesNo(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys)
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(fieldWork);

  GF_ASSERT(iem->fldmenu_func == NULL && iem->msgdata != NULL);
  iem->fldmenu_func = FLDMENUFUNC_AddYesNoMenu(msgBG, FLDMENUFUNC_YESNO_YES);
}

//--------------------------------------------------------------
/**
 * はい/いいえ 選択メニュー：削除
 *
 * @param   iem		
 */
//--------------------------------------------------------------
void IntrudeEventPrint_ExitYesNo(INTRUDE_EVENT_MSGWORK *iem)
{
  IntrudeEventPrint_ExitMenu(iem);
}

//--------------------------------------------------------------
/**
 * はい/いいえ 選択メニュー：選択待ち
 *
 * @param   iem		
 *
 * @retval  FLDMENUFUNC_YESNO		結果
 */
//--------------------------------------------------------------
FLDMENUFUNC_YESNO IntrudeEventPrint_SelectYesNo(INTRUDE_EVENT_MSGWORK *iem)
{
  GF_ASSERT(iem->fldmenu_func != NULL);
  return FLDMENUFUNC_ProcYesNoMenu(iem->fldmenu_func);
}
