//==============================================================================
/**
 * @file    intrude_message.h
 * @brief   侵入イベント：メッセージ共通処理
 * @author  matsuda
 * @date    2009.10.27(火)
 */
//==============================================================================
#pragma once



//==============================================================================
//  構造体定義
//==============================================================================
///プリント制御構造体
typedef struct{
  WORDSET *wordset;
  STRBUF *msgbuf;
  STRBUF *tmpbuf;
  GFL_MSGDATA *msgdata;
  FLDMSGWIN_STREAM *msgwin_stream;
}INTRUDE_EVENT_MSGWORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void IntrudeEventPrint_SetupFieldMsg(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys);
extern void IntrudeEventPrint_ExitFieldMsg(INTRUDE_EVENT_MSGWORK *iem);
extern void IntrudeEventPrint_StartStream(INTRUDE_EVENT_MSGWORK *iem, u16 msg_id);
extern BOOL IntrudeEventPrint_WaitStream(INTRUDE_EVENT_MSGWORK *iem);
extern BOOL IntrudeEventPrint_LastKeyWait(void);

