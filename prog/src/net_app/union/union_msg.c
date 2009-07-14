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
  
  GF_ASSERT(unisys->fld_msgwin_stream == NULL);
  
  fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
  unisys->msgdata = FLDMSGBG_CreateMSGDATA( fldmsg_bg, NARC_message_union_dat );
  unisys->fld_msgwin_stream = FLDMSGWIN_STREAM_AddTalkWin(fldmsg_bg, unisys->msgdata);
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
  GF_ASSERT(unisys->fld_msgwin_stream != NULL && unisys->msgdata != NULL);
  
  FLDMSGWIN_STREAM_ClearWindow(unisys->fld_msgwin_stream);
  FLDMSGWIN_STREAM_Delete(unisys->fld_msgwin_stream);
  unisys->fld_msgwin_stream = NULL;

  GFL_MSG_Delete(unisys->msgdata);
  unisys->msgdata = NULL;
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
BOOL UnionMsg_TalkStrem_Check(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fld_msgwin_stream == NULL){
    return TRUE;
  }
  return FLDMSGWIN_STREAM_Print(unisys->fld_msgwin_stream);
}
