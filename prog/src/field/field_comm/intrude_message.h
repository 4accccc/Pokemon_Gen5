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
  u8 print_flag;        ///<システム内部で使用  TRUE:出力したデータがある
  u8 padding[3];
  
  FLDMENUFUNC *fldmenu_func;
}INTRUDE_EVENT_MSGWORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void IntrudeEventPrint_SetupFieldMsg(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys);
extern void IntrudeEventPrint_ExitFieldMsg(INTRUDE_EVENT_MSGWORK *iem);
extern void IntrudeEventPrint_StartStream(INTRUDE_EVENT_MSGWORK *iem, u16 msg_id);
extern BOOL IntrudeEventPrint_WaitStream(INTRUDE_EVENT_MSGWORK *iem);
extern BOOL IntrudeEventPrint_LastKeyWait(void);
extern void IntrudeEventPrint_ExitMenu(INTRUDE_EVENT_MSGWORK *iem);
extern u32 IntrudeEventPrint_SelectMenu(INTRUDE_EVENT_MSGWORK *iem);
extern void IntrudeEventPrint_SetupMainMenu(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys);
extern void IntrudeEventPrint_SetupYesNo(INTRUDE_EVENT_MSGWORK *iem, GAMESYS_WORK *gsys);
extern void IntrudeEventPrint_ExitYesNo(INTRUDE_EVENT_MSGWORK *iem);
extern FLDMENUFUNC_YESNO IntrudeEventPrint_SelectYesNo(INTRUDE_EVENT_MSGWORK *iem);

