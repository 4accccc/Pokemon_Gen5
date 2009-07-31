//==============================================================================
/**
 * @file    comm_entry_menu.c
 * @brief   参加募集メニュー制御
 * @author  matsuda
 * @date    2009.07.28(火)
 */
//==============================================================================
#include <gflib.h>
#include "field/fieldmap.h"
#include "message.naix"
#include "msg/msg_connect.h"
#include "net_app/comm_entry_menu.h"
#include "print/wordset.h"


//==============================================================================
//  定数定義
//==============================================================================
///IDのSTRBUFサイズ
#define ID_STRBUF_SIZE          (5)   //5桁表示の為
///EXPANDで展開するメッセージバッファのサイズ
#define EXPAND_STRBUF_SIZE      (100)

///ユーザーステータス
enum{
  USER_STATUS_NULL,         ///<ユーザーがいない
  USER_STATUS_READY,        ///<準備中(リストにまだ名前が載っていない)
  USER_STATUS_EXAMINATION,  ///<審査対象
  USER_STATUS_EXAMINATION_WAIT, ///<審査完了待ち
  USER_STATUS_COMPLETION,   ///<エントリー完了
  USER_STATUS_REFUSE,       ///<断った
  USER_STATUS_FAILE,        ///<いなくなった
};

///エントリーリストBMPサイズ(キャラ単位)
enum{
  LISTBMP_POS_LEFT = 2,
  LISTBMP_POS_RIGHT = 20,
  LISTBMP_POS_TOP = 6,
  LISTBMP_POS_BOTTOM = LISTBMP_POS_TOP + 8,
};

///エントリーリストのBMP書き込み位置(ドット単位)
enum{
  LISTBMP_START_NAME_X = 0,
  LISTBMP_START_NAME_Y = 0,
  LISTBMP_START_ID_X = (LISTBMP_POS_RIGHT - 8) * 8,
  LISTBMP_START_ID_Y = LISTBMP_START_NAME_Y,
};

///最終結果
typedef enum{
  DECIDE_TYPE_NULL,
  DECIDE_TYPE_OK,
  DECIDE_TYPE_NG,
}DECIDE_TYPE;

//==============================================================================
//  構造体定義
//==============================================================================
///エントリー者のデータ
typedef struct{
  STRBUF *strbuf_name;   ///<エントリー者の名前
  STRBUF *strbuf_id;     ///<エントリー者のID(トレーナーカードのID)
  u8 sex;
  u8 status;
  u8 force_entry;        ///<TRUE:強制的に受け入れる
  u8 send_flag;          ///<TRUE:返事送信済み(外側から立てる)
}COMM_ENTRY_USER;

///「はい・いいえ」制御
typedef struct{
  FLDMENUFUNC *menufunc;
  u8 seq;
}COMM_ENTRY_YESNO;

///参加募集管理システム
typedef struct _COMM_ENTRY_MENU_SYSTEM{
  FIELD_MAIN_WORK *fieldWork;
  WORDSET *wordset;
  GFL_MSGDATA *msgdata;
  FLDMSGWIN *fld_msgwin;
  FLDMSGWIN_STREAM *fld_stream;
  STRBUF *strbuf_expand;
  
  COMM_ENTRY_USER user[COMM_ENTRY_USER_MAX];    ///<エントリー者のデータ
  COMM_ENTRY_USER *examination_user;             ///<審査中のユーザー
  COMM_ENTRY_YESNO yesno;                       ///<「はい・いいえ」制御
  
  u8 min_num;         ///<最少人数
  u8 max_num;         ///<最大人数
  u8 seq;
  u8 padding;
}COMM_ENTRY_MENU_SYSTEM;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void CommEntryMenu_ListUpdate(COMM_ENTRY_MENU_PTR em);
static BOOL CommEntryMenu_SetExamination(COMM_ENTRY_MENU_PTR em, COMM_ENTRY_USER *user);
static void CommEntryMenu_ExaminationUpdate(COMM_ENTRY_MENU_PTR em);
static DECIDE_TYPE CommEntryMenu_DecideUpdate(COMM_ENTRY_MENU_PTR em);
static BOOL CommEntryMenu_InterruptCheck(COMM_ENTRY_MENU_PTR em);
static void CommEntryMenu_NameDraw(COMM_ENTRY_MENU_PTR em, COMM_ENTRY_USER *user, int netID);
static void CommEntryMenu_NameErase(COMM_ENTRY_MENU_PTR em, int netID);
static void _StreamMsgSet(COMM_ENTRY_MENU_PTR em, u32 msg_id);



//==================================================================
/**
 * 参加募集メニュー：セットアップ
 *
 * @param   fieldWork		
 * @param   min_num		  最少人数
 * @param   max_num		  最大人数
 * @param   heap_id		  ヒープID
 *
 * @retval  COMM_ENTRY_MENU_PTR		生成した参加募集メニューワークへのポインタ
 */
//==================================================================
COMM_ENTRY_MENU_PTR CommEntryMenu_Setup(FIELD_MAIN_WORK *fieldWork, int min_num, int max_num, HEAPID heap_id)
{
  COMM_ENTRY_MENU_PTR em;
  FLDMSGBG *fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
  int i;
  
  GF_ASSERT(max_num <= COMM_ENTRY_USER_MAX);
  
  em = GFL_HEAP_AllocClearMemory(heap_id, sizeof(COMM_ENTRY_MENU_SYSTEM));
  
  em->wordset = WORDSET_Create( heap_id );
  em->msgdata = FLDMSGBG_CreateMSGDATA( fldmsg_bg, NARC_message_connect_dat );
  em->fld_msgwin = FLDMSGWIN_Add( fldmsg_bg, em->msgdata, 
    LISTBMP_POS_LEFT, LISTBMP_POS_TOP, 
    LISTBMP_POS_RIGHT - LISTBMP_POS_LEFT, LISTBMP_POS_BOTTOM - LISTBMP_POS_TOP);
  em->fld_stream = FLDMSGWIN_STREAM_AddTalkWin(fldmsg_bg, em->msgdata);
  em->strbuf_expand = GFL_STR_CreateBuffer(EXPAND_STRBUF_SIZE, heap_id);
  em->min_num = min_num;
  em->max_num = max_num;
  em->fieldWork = fieldWork;
  
  for(i = 0; i < COMM_ENTRY_USER_MAX; i++){
    em->user[i].strbuf_name = GFL_STR_CreateBuffer(PERSON_NAME_SIZE + EOM_SIZE, heap_id);
    em->user[i].strbuf_id = GFL_STR_CreateBuffer(ID_STRBUF_SIZE + EOM_SIZE, heap_id);
  }

  //最初のメッセージ描画
  _StreamMsgSet(em, msg_connect_02_01);
  
  return em;
}

//==================================================================
/**
 * 参加募集メニュー削除
 *
 * @param   em		
 */
//==================================================================
void CommEntryMenu_Exit(COMM_ENTRY_MENU_PTR em)
{
  int i;
  
  for(i = 0; i < COMM_ENTRY_USER_MAX; i++){
    GFL_STR_DeleteBuffer(em->user[i].strbuf_name);
    GFL_STR_DeleteBuffer(em->user[i].strbuf_id);
  }
  GFL_STR_DeleteBuffer(em->strbuf_expand);
  
  FLDMSGWIN_STREAM_Delete(em->fld_stream);
  FLDMSGWIN_Delete(em->fld_msgwin);
  FLDMSGBG_DeleteMSGDATA(em->msgdata);
  WORDSET_Delete( em->wordset );
  
  GFL_HEAP_FreeMemory(em);
}

//==================================================================
/**
 * プレイヤーエントリー
 *
 * @param   em		  
 * @param   netID		      ネットID
 * @param   name		      名前
 * @param   id		        ID(トレーナーカードのID)
 * @param   sex           性別
 * @param   force_entry		TRUE:強制受け入れ。　FALSE:受け入れるか選択する
 */
//==================================================================
void CommEntryMenu_Entry(COMM_ENTRY_MENU_PTR em, int netID, const STRCODE *name, u32 id, u8 sex, BOOL force_entry)
{
  COMM_ENTRY_USER *p_user = &em->user[netID];
  
  GF_ASSERT(netID <= COMM_ENTRY_USER_MAX);
  
  GFL_STR_SetStringCode(p_user->strbuf_name, name);
  STRTOOL_SetNumber(p_user->strbuf_id, id & 0xffff, ID_STRBUF_SIZE, 
    STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU);
  p_user->sex = sex;
  p_user->status = USER_STATUS_READY;
  p_user->force_entry = force_entry;
}

//==================================================================
/**
 * 参加募集メニュー：更新処理
 *
 * @param   em		
 *
 * @retval  BOOL		TRUE:全ての処理が終了。
 */
//==================================================================
BOOL CommEntryMenu_Update(COMM_ENTRY_MENU_PTR em)
{
  switch(em->seq){
  case 0:   //募集中
    //リスト更新
    CommEntryMenu_ListUpdate(em);
    //エントリー者を受け入れるか選択
    CommEntryMenu_ExaminationUpdate(em);
    if(CommEntryMenu_InterruptCheck(em) == TRUE){
      int entry_num = CommEntryMenu_GetCompletionNum(em);
      if(entry_num >= em->max_num
          || (entry_num >= em->min_num && (GFL_UI_KEY_GetTrg() & PAD_BUTTON_A))){
        em->seq++;
      }
    }
    break;
  case 1:   //このメンバーでよろしいですか？
    {
      DECIDE_TYPE type;
      type = CommEntryMenu_DecideUpdate(em);
      if(type == DECIDE_TYPE_OK){
        em->seq++;
      }
      else if(type == DECIDE_TYPE_NG){
        em->seq--;
      }
    }
    break;
  case 2:   //終了処理
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * リスト更新処理
 *
 * @param   em		
 */
//--------------------------------------------------------------
static void CommEntryMenu_ListUpdate(COMM_ENTRY_MENU_PTR em)
{
  int i;
  COMM_ENTRY_USER *user = em->user;

  //リストの名前描画
  for(i = 0; i < COMM_ENTRY_USER_MAX; i++){
    switch(user->status){
    case USER_STATUS_READY:
      CommEntryMenu_NameDraw(em, user, i);
      if(user->force_entry == TRUE){
        user->status = USER_STATUS_COMPLETION;
      }
      else{
        user->status = USER_STATUS_EXAMINATION;
      }
      break;
    case USER_STATUS_EXAMINATION:
      if(CommEntryMenu_SetExamination(em, user) == TRUE){
        user->status = USER_STATUS_EXAMINATION_WAIT;
      }
      break;
    case USER_STATUS_FAILE:
      CommEntryMenu_NameErase(em, i);
      GFL_STD_MemClear(user, sizeof(COMM_ENTRY_USER));
      break;
    }
    
    user++;
  }
}

//--------------------------------------------------------------
/**
 * 審査対象にユーザーを登録する
 *
 * @param   em		
 * @param   user		
 *
 * @retval  BOOL		TRUE:登録成功
 */
//--------------------------------------------------------------
static BOOL CommEntryMenu_SetExamination(COMM_ENTRY_MENU_PTR em, COMM_ENTRY_USER *user)
{
  if(em->examination_user == NULL){
    em->examination_user = user;
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * エントリー者を受け入れるか選択
 *
 * @param   em		
 */
//--------------------------------------------------------------
static void CommEntryMenu_ExaminationUpdate(COMM_ENTRY_MENU_PTR em)
{
  COMM_ENTRY_USER *user = em->examination_user;
  COMM_ENTRY_YESNO *yesno = &em->yesno;
  FLDMSGBG *fldmsg_bg;
  
  if(user == NULL || FLDMSGWIN_STREAM_Print(em->fld_stream) == FALSE){
    return;
  }
  
  switch(yesno->seq){
  case 0:
    WORDSET_RegisterWord(em->wordset, 0, user->strbuf_name, user->sex, TRUE, PM_LANG);
    _StreamMsgSet(em, msg_connect_02_02);
    yesno->seq++;
    break;
  case 1:
    GF_ASSERT(yesno->menufunc == NULL);
    fldmsg_bg = FIELDMAP_GetFldMsgBG(em->fieldWork);
    yesno->menufunc = FLDMENUFUNC_AddYesNoMenu(fldmsg_bg, FLDMENUFUNC_YESNO_YES);
    yesno->seq++;
    break;
  case 2:
    {
      FLDMENUFUNC_YESNO ret;
      
      ret = FLDMENUFUNC_ProcYesNoMenu(yesno->menufunc);
      if(ret == FLDMENUFUNC_YESNO_NULL){
        break;
      }
      else if(ret == FLDMENUFUNC_YESNO_YES){
        user->status = USER_STATUS_COMPLETION;
      }
      else{
      #if 0
        user->status = USER_STATUS_REFUSE;
      #else
        user->status = USER_STATUS_COMPLETION;
      #endif
      }
      FLDMENUFUNC_DeleteMenu(yesno->menufunc);
      yesno->menufunc = NULL;
      _StreamMsgSet(em, msg_connect_02_01);
      em->examination_user = NULL;
      yesno->seq = 0;
    }
    break;
  }
}

//--------------------------------------------------------------
/**
 * 最終決定処理
 *
 * @param   em		
 *
 * @retval  
 */
//--------------------------------------------------------------
static DECIDE_TYPE CommEntryMenu_DecideUpdate(COMM_ENTRY_MENU_PTR em)
{
  FLDMSGBG *fldmsg_bg;
  COMM_ENTRY_YESNO *yesno = &em->yesno;

  if(FLDMSGWIN_STREAM_Print(em->fld_stream) == FALSE){
    return DECIDE_TYPE_NULL;
  }

  switch(yesno->seq){
  case 0:
    _StreamMsgSet(em, msg_connect_02_03);
    yesno->seq++;
    break;
  case 1:
    GF_ASSERT(yesno->menufunc == NULL);
    fldmsg_bg = FIELDMAP_GetFldMsgBG(em->fieldWork);
    yesno->menufunc = FLDMENUFUNC_AddYesNoMenu(fldmsg_bg, FLDMENUFUNC_YESNO_YES);
    yesno->seq++;
    break;
  case 2:
    {
      FLDMENUFUNC_YESNO ret;
      
      ret = FLDMENUFUNC_ProcYesNoMenu(yesno->menufunc);
      if(ret == FLDMENUFUNC_YESNO_NULL){
        break;
      }
      
      FLDMENUFUNC_DeleteMenu(yesno->menufunc);
      yesno->menufunc = NULL;
      yesno->seq = 0;
      if(ret == FLDMENUFUNC_YESNO_YES){
        return DECIDE_TYPE_OK;
      }
      else{
      #if 0
        return DECIDE_TYPE_NG;
      #else
        return DECIDE_TYPE_OK;
      #endif
      }
    }
    break;
  }
  return DECIDE_TYPE_NULL;
}

//==================================================================
/**
 * エントリー結果の取得
 *
 * @param   em		
 * @param   netID		
 * @param   exit_mode   TRUE:親が参加メンバーを決定後、まだ返事を送信していないメンバーを取得するモードです(FALSEの時の違いは、TRUEの場合は審査にすら漏れた者の状態も返す事です)
 *
 * @retval  COMM_ENTRY_ANSWER		
 *
 * 送信済みフラグが立っているものはNULLを返します
 */
//==================================================================
COMM_ENTRY_ANSWER CommEntryMenu_GetAnswer(COMM_ENTRY_MENU_PTR em, int netID, BOOL exit_mode)
{
  if(em->user[netID].send_flag == FALSE){
    if(em->user[netID].status == USER_STATUS_COMPLETION){
      return COMM_ENTRY_ANSWER_COMPLETION;
    }
    else if(em->user[netID].status == USER_STATUS_REFUSE){
//      return COMM_ENTRY_ANSWER_REFUSE;
      return COMM_ENTRY_ANSWER_COMPLETION;
    }
    
    if(exit_mode == TRUE){
      if(em->user[netID].status != USER_STATUS_NULL && em->user[netID].status != USER_STATUS_FAILE){
        return COMM_ENTRY_ANSWER_REFUSE;
      }
    }
  }
  return COMM_ENTRY_ANSWER_NULL;
}

//==================================================================
/**
 * 送信済みフラグを立てる
 *
 * @param   em		
 * @param   netID		
 *
 * CommEntryMenu_GetAnswerで取得した返事を通信相手に送信したら、これでフラグを立ててください
 * CommEntryMenu_GetAnswerでNULLが返るようになるので2重送信などが起きなくなります
 * 結果が「拒否」だった場合はリストから削除します
 */
//==================================================================
void CommEntryMenu_SetSendFlag(COMM_ENTRY_MENU_PTR em, int netID)
{
  GF_ASSERT(em->user[netID].status == USER_STATUS_COMPLETION || em->user[netID].status == USER_STATUS_REFUSE);
  
  if(em->user[netID].status == USER_STATUS_COMPLETION){
    em->user[netID].send_flag = TRUE;
  }
  else if(em->user[netID].status == USER_STATUS_REFUSE){
    CommEntryMenu_NameErase(em, netID);
    GFL_STD_MemClear(&em->user[netID], sizeof(COMM_ENTRY_USER));
  }
}

//==================================================================
/**
 * エントリー完了した人数を取得する
 *
 * @param   em		
 *
 * @retval  int		エントリー完了人数
 */
//==================================================================
int CommEntryMenu_GetCompletionNum(COMM_ENTRY_MENU_PTR em)
{
  int i, count = 0;
  
  for(i = 0; i < COMM_ENTRY_USER_MAX; i++){
    if(em->user[i].status == USER_STATUS_COMPLETION){
      count++;
    }
  }
  return count;
}

//--------------------------------------------------------------
/**
 * 何の選択も行っていないフリーの状況か調べる
 *
 * @param   em		
 *
 * @retval  BOOL		TRUE：フリー
 */
//--------------------------------------------------------------
static BOOL CommEntryMenu_InterruptCheck(COMM_ENTRY_MENU_PTR em)
{
  if(em->examination_user == NULL && FLDMSGWIN_STREAM_Print(em->fld_stream) == TRUE){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 名前描画
 *
 * @param   em		
 * @param   user		
 * @param   netID		
 */
//--------------------------------------------------------------
static void CommEntryMenu_NameDraw(COMM_ENTRY_MENU_PTR em, COMM_ENTRY_USER *user, int netID)
{
  FLDMSGWIN_PrintStrBuf(em->fld_msgwin, 
    LISTBMP_START_NAME_X, LISTBMP_START_NAME_Y + netID*8*2, user->strbuf_name);
  FLDMSGWIN_PrintStrBuf(em->fld_msgwin, 
    LISTBMP_START_ID_X, LISTBMP_START_ID_Y + netID*8*2, user->strbuf_id);
}

//--------------------------------------------------------------
/**
 * 名前消去
 *
 * @param   em		
 * @param   netID		
 */
//--------------------------------------------------------------
static void CommEntryMenu_NameErase(COMM_ENTRY_MENU_PTR em, int netID)
{
  FLDMSGWIN_FillClearWindow(em->fld_msgwin, 
    LISTBMP_START_NAME_X, LISTBMP_START_NAME_Y + netID*8*2, 
    (LISTBMP_POS_RIGHT - LISTBMP_POS_LEFT) * 8, LISTBMP_START_NAME_Y + netID*8*4);
  OS_TPrintf("Name Erase = %d, %d, %d, %d\n",     LISTBMP_START_NAME_X, LISTBMP_START_NAME_Y + netID*8*2, 
    (LISTBMP_POS_RIGHT - LISTBMP_POS_LEFT) * 8, LISTBMP_START_NAME_Y + netID*8*4);

}

//--------------------------------------------------------------
/**
 * メッセージストリーム描画
 *
 * @param   em		
 * @param   msg_id		メッセージID
 *
 * WORDSETはあらかじめセットした上で呼んでください
 */
//--------------------------------------------------------------
static void _StreamMsgSet(COMM_ENTRY_MENU_PTR em, u32 msg_id)
{
  STRBUF *temp_strbuf;
  
  temp_strbuf = GFL_MSG_CreateString( em->msgdata, msg_id );
  WORDSET_ExpandStr( em->wordset, em->strbuf_expand, temp_strbuf );
  GFL_STR_DeleteBuffer(temp_strbuf);
  
  FLDMSGWIN_STREAM_ClearMessage(em->fld_stream);
  FLDMSGWIN_STREAM_PrintStrBufStart(em->fld_stream, 0, 0, em->strbuf_expand);
}
