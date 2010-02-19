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
#include "msg/msg_mission_msg.h"
#include "field/field_comm/intrude_types.h"
#include "intrude_mission.h"


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
static void IntrudeEventPrint_SetupPrint(INTRUDE_EVENT_MSGWORK *iem);
static void IntrudeEventPrint_ExitPrint(INTRUDE_EVENT_MSGWORK *iem);
static void _PrintMsgWin( INTRUDE_EVENT_MSGWORK *iem, GFL_MSGDATA *msgdata, 
  FLDMSGWIN *fldmsg_win, u16 msg_id, int x, int y );
static void IntrudeEventPrint_ExitStream(INTRUDE_EVENT_MSGWORK *iem);


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
  iem->msgdata = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_mission_msg_dat );
  iem->msgdata_mission = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_mission_dat );
  iem->msgdata_mission_mono = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_mission_monolith_dat );
  iem->msgBG = msgBG;
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
  IntrudeEventPrint_ExitStream(iem);
  IntrudeEventPrint_ExitExtraMsgWin(iem);
  IntrudeEventPrint_ExitPrint(iem);
  
  if(iem->wordset != NULL){
    if(iem->msgwin_stream != NULL){
      FLDMSGWIN_STREAM_Delete(iem->msgwin_stream);
      iem->msgwin_stream = NULL;
    }
    FLDMSGBG_DeleteMSGDATA(iem->msgdata);
    FLDMSGBG_DeleteMSGDATA(iem->msgdata_mission);
    FLDMSGBG_DeleteMSGDATA(iem->msgdata_mission_mono);
    GFL_STR_DeleteBuffer(iem->tmpbuf);
    GFL_STR_DeleteBuffer(iem->msgbuf);
    WORDSET_Delete(iem->wordset);
    
    iem->wordset = NULL;  //解放した事の印
  }
}

//--------------------------------------------------------------
/**
 * プリントストリーム出力：初期設定
 *
 * @param   iem		
 */
//--------------------------------------------------------------
static void IntrudeEventPrint_SetupStream(INTRUDE_EVENT_MSGWORK *iem)
{
  if(iem->msgwin_stream != NULL){
    if(iem->print_flag == TRUE){
      IntrudeEventPrint_ClearStream(iem);
    }
    return;
  }
  iem->msgwin_stream = FLDMSGWIN_STREAM_AddTalkWin( iem->msgBG, iem->msgdata );
}

//--------------------------------------------------------------
/**
 * プリントストリーム出力：削除
 *
 * @param   iem		
 */
//--------------------------------------------------------------
static void IntrudeEventPrint_ExitStream(INTRUDE_EVENT_MSGWORK *iem)
{
  if(iem->msgwin_stream == NULL){
    return;
  }
  FLDMSGWIN_STREAM_Delete(iem->msgwin_stream);
  iem->msgwin_stream = NULL;
}

//--------------------------------------------------------------
/**
 * プリントストリーム出力：共通処理
 *
 * @param   iem		
 * @param   strbuf		出力メッセージ
 */
//--------------------------------------------------------------
static void _StartStream(INTRUDE_EVENT_MSGWORK *iem, STRBUF *strbuf)
{
  IntrudeEventPrint_SetupStream(iem);
  
  FLDMSGWIN_STREAM_PrintStrBufStart( iem->msgwin_stream, 0, 0, strbuf );
  iem->print_flag = TRUE;
}

//--------------------------------------------------------------
/**
 * プリントストリーム出力：mission_msg.gmm用
 *
 * @param   iem		
 * @param   msg_id		
 */
//--------------------------------------------------------------
void IntrudeEventPrint_StartStream(INTRUDE_EVENT_MSGWORK *iem, u16 msg_id)
{
  GFL_MSG_GetString( iem->msgdata, msg_id, iem->tmpbuf );
  WORDSET_ExpandStr( iem->wordset, iem->msgbuf, iem->tmpbuf );
  
  _StartStream(iem, iem->msgbuf);
}

//--------------------------------------------------------------
/**
 * プリントストリーム出力：mission.gmm用
 *      現在実施中のミッションを表示
 *
 * @param   iem		
 * @param   
 */
//--------------------------------------------------------------
BOOL IntrudeEventPrint_StartStreamMission(INTRUDE_EVENT_MSGWORK *iem, INTRUDE_COMM_SYS_PTR intcomm)
{
  const MISSION_DATA *mdata;

  if(MISSION_RecvCheck(&intcomm->mission) == FALSE){
    return FALSE; //実施しているミッションは無い
  }
  
  mdata = MISSION_GetRecvData(&intcomm->mission);

  GFL_MSG_GetString(iem->msgdata_mission, mdata->cdata.msg_id_contents, iem->tmpbuf);
  MISSIONDATA_Wordset(intcomm, mdata, iem->wordset, INTRUDE_EVENT_HEAPID);
  WORDSET_ExpandStr( iem->wordset, iem->msgbuf, iem->tmpbuf );
  
  _StartStream(iem, iem->msgbuf);
  return TRUE;
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

//--------------------------------------------------------------
/**
 * メッセージウィンドウ：初期設定
 *
 * @param   iem		
 */
//--------------------------------------------------------------
static void IntrudeEventPrint_SetupPrint(INTRUDE_EVENT_MSGWORK *iem)
{
  if(iem->fldmsg_win == NULL){
    iem->fldmsg_win = FLDMSGWIN_Add(iem->msgBG, iem->msgdata, 1, 19, 30, 4);
  }
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ：削除
 *
 * @param   iem		
 */
//--------------------------------------------------------------
static void IntrudeEventPrint_ExitPrint(INTRUDE_EVENT_MSGWORK *iem)
{
  if(iem->fldmsg_win != NULL){
    FLDMSGWIN_Delete(iem->fldmsg_win);
    iem->fldmsg_win = NULL;
  }
}

//--------------------------------------------------------------
/**
 * メッセージウィンドウ：文字描画
 *
 * @param   iem		
 * @param   msgdata		
 * @param   msg_id    メッセージID
 * @param   x		      描画位置X
 * @param   y		      描画位置Y
 */
//--------------------------------------------------------------
static void _PrintMsgWin( INTRUDE_EVENT_MSGWORK *iem, GFL_MSGDATA *msgdata, FLDMSGWIN *fldmsg_win, u16 msg_id, int x, int y )
{
  GFL_MSG_GetString( msgdata, msg_id, iem->tmpbuf );
  WORDSET_ExpandStr( iem->wordset, iem->msgbuf, iem->tmpbuf );
  FLDMSGWIN_PrintStrBuf( fldmsg_win, x, y, iem->msgbuf );
}

//==================================================================
/**
 * 文字一括描画
 *
 * @param   iem		
 * @param   msg_id		メッセージID
 * @param   x		      描画位置X
 * @param   y		      描画位置Y
 */
//==================================================================
void IntrudeEventPrint_Print(INTRUDE_EVENT_MSGWORK *iem, u16 msg_id, int x, int y)
{
  IntrudeEventPrint_SetupPrint(iem);
  _PrintMsgWin( iem, iem->msgdata, iem->fldmsg_win, msg_id, x, y );
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
  FLDMENUFUNC_LISTDATA *fldmenu_listdata;
  
  GF_ASSERT(iem->fldmenu_func == NULL && iem->msgdata != NULL);
  
  fldmenu_listdata = FLDMENUFUNC_CreateMakeListData(
      menulist, list_max, iem->msgdata, INTRUDE_EVENT_HEAPID);
  iem->fldmenu_func = FLDMENUFUNC_AddMenu(iem->msgBG, head, fldmenu_listdata);
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

  GF_ASSERT(iem->fldmenu_func == NULL && iem->msgdata != NULL);
  iem->fldmenu_func = FLDMENUFUNC_AddYesNoMenu(iem->msgBG, FLDMENUFUNC_YESNO_YES);
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


//==============================================================================
//
//  MSGWINセットアップ
//
//==============================================================================
//==================================================================
/**
 * メッセージウィンドウ(会話以外)：セットアップ
 *
 * @param   iem		
 * @param   gsys		
 * @param   bmp_x		  BMP X位置(キャラクタ単位)
 * @param   bmp_y		  BMP Y位置(キャラクタ単位)
 * @param   size_x		サイズX(キャラクタ単位)
 * @param   size_y		サイズY(キャラクタ単位)
 */
//==================================================================
void IntrudeEventPrint_SetupExtraMsgWin(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys, int bmp_x, int bmp_y, int size_x, int size_y)
{
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  GF_ASSERT(iem->fldmsg_win_extra == NULL);
  iem->fldmsg_win_extra = FLDMSGWIN_Add(iem->msgBG, iem->msgdata, bmp_x, bmp_y, size_x, size_y);
}

//==================================================================
/**
 * メッセージウィンドウ(会話以外)：文字描画 mission_monolith.gmm
 *
 * @param   iem		
 * @param   msg_id    メッセージID
 * @param   x		      描画位置X
 * @param   y		      描画位置Y
 */
//==================================================================
void IntrudeEventPrint_PrintExtraMsgWin_MissionMono( INTRUDE_EVENT_MSGWORK *iem, u16 msg_id, int x, int y )
{
  _PrintMsgWin( iem, iem->msgdata_mission_mono, iem->fldmsg_win_extra, msg_id, x, y );
}

//==================================================================
/**
 * メッセージウィンドウ(会話以外)：削除
 *
 * @param   iem		
 */
//==================================================================
void IntrudeEventPrint_ExitExtraMsgWin(INTRUDE_EVENT_MSGWORK *iem)
{
  if(iem->fldmsg_win_extra == NULL){
    return;
  }
  FLDMSGWIN_Delete(iem->fldmsg_win_extra);
  iem->fldmsg_win_extra = NULL;
}
