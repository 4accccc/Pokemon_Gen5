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


//==============================================================================
//  定数定義
//==============================================================================
///侵入イベントで使用するヒープID
#define INTRUDE_EVENT_HEAPID        (HEAPID_PROC)
///侵入イベントで使用するメッセージバッファサイズ
#define INTRUDE_EVENT_MSGBUF_SIZE   (256)



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
  FLDMSGWIN_STREAM_Delete(iem->msgwin_stream);
  FLDMSGBG_DeleteMSGDATA(iem->msgdata);
  GFL_STR_DeleteBuffer(iem->tmpbuf);
  GFL_STR_DeleteBuffer(iem->msgbuf);
  WORDSET_Delete(iem->wordset);
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
  GFL_MSG_GetString( iem->msgdata, msg_id, iem->tmpbuf );
  WORDSET_ExpandStr( iem->wordset, iem->msgbuf, iem->tmpbuf );

  FLDMSGWIN_STREAM_PrintStrBufStart( iem->msgwin_stream, 0, 0, iem->msgbuf );
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
