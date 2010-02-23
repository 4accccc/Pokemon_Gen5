//============================================================================================
/**
 * @file  mb_seq.c
 * @brief メールボックス画面 メインシーケンス
 * @author  Hiroyuki Nakamura
 * @date  09.01.31
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/wipe.h"
#include "gamesystem/game_data.h"
#include "sound/pm_sndsys.h"
#include "poke_tool/pokeparty.h"
#include "item/item.h"
#include "savedata/myitem_savedata.h"

#include "app/pokelist.h"
#include "app/mailtool.h"

#include "mb_main.h"
#include "mb_seq.h"
#include "mb_bmp.h"
#include "mb_obj.h"
#include "mb_ui.h"
#include "mb_snd_def.h"


//============================================================================================
//  定数定義
//============================================================================================

// サブプロセスタイプ
enum {
  SUBPROC_TYPE_PL_PUT = 0,    // もたせる
  SUBPROC_TYPE_PL_DEL,      // けす
  SUBPROC_TYPE_MAILWRITE,     // メール作成
  SUBPROC_TYPE_PL_MW_END,     // メール作成後
};

// サブプロセスデータ
typedef struct {
  pMailBoxFunc  call; // 呼び出し関数
  pMailBoxFunc  exit; // 終了関数
  u32 rcvSeq;       // 復帰シーケンス
}SUBPROC_DATA;

// はい・いいえID
enum {
  YESNO_ID_DELETE = 0,  // メールを消す
  YESNO_ID_POKEADD,   // ポケモンに持たせる
};

// はい・いいえ処理
typedef struct {
  pMailBoxFunc  yes;
  pMailBoxFunc  no;
}YESNO_DATA;

// トリガーウェイト
enum {
  MSG_TRGWAIT_OFF = 0,
  MSG_TRGWAIT_ON,
};


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( MAILBOX_SYS_WORK * syswk );
static int MainSeq_Release( MAILBOX_SYS_WORK * syswk );
static int MainSeq_Wipe( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MsgWait( MAILBOX_SYS_WORK * syswk );
static int MainSeq_YesNo( MAILBOX_SYS_WORK * syswk );
static int MainSeq_ButtonAnm( MAILBOX_SYS_WORK * syswk );
static int MainSeq_SubProcCall( MAILBOX_SYS_WORK * syswk );
static int MainSeq_SubProcMain( MAILBOX_SYS_WORK * syswk );
static int MainSeq_Start( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailSelectMain( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailBoxEndSet( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailSelectPageChg( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailMenuMsgSet( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailMenuSet( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailMenuMain( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailReadMain( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailReadEnd( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailDelYesNoSet( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailPokeSetMsg( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailPokeSetYesNoSet( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailDelEnd( MAILBOX_SYS_WORK * syswk );
static int MainSeq_MailWriteExit( MAILBOX_SYS_WORK * syswk );

static int MainSeq_EraseMailPokeListExit( MAILBOX_SYS_WORK * syswk );
static int MainSeq_EraseMailPokeSetCancel( MAILBOX_SYS_WORK * syswk );
static int MainSeq_EraseMailPokeSetCancelEnd( MAILBOX_SYS_WORK * syswk );

static int FadeInSet( MAILBOX_SYS_WORK * syswk, int next );
static int FadeOutSet( MAILBOX_SYS_WORK * syswk, int next );
static int MsgWaitSet( MAILBOX_SYS_WORK * syswk, u32 wait, int next );
static int YesNoSet( MAILBOX_SYS_WORK * syswk, u32 id );
static int BgWinFrmButtonAnmSet( MAILBOX_SYS_WORK * syswk, u32 frmID, int next );
static int ObjButtonAnmSet( MAILBOX_SYS_WORK * syswk, u32 objID, int next );
static void AlphaSet( MAILBOX_APP_WORK * appwk, BOOL flg );
static void SubDispMailWrite( MAILBOX_SYS_WORK * syswk );
static void SubDispMailClear( MAILBOX_SYS_WORK * syswk );
static int MailPokeListCall( MAILBOX_SYS_WORK * syswk );
static int MailPutPokeListExit( MAILBOX_SYS_WORK * syswk );
static int EraseMailPokeListExit( MAILBOX_SYS_WORK * syswk );

static void PokeListWorkCreate( MAILBOX_SYS_WORK * syswk, u32 mode, u16 item, u16 pos );

static int MailDelYes( MAILBOX_SYS_WORK * syswk );
static int MailPokeAddYes( MAILBOX_SYS_WORK * syswk );
static int MailPokeAddNo( MAILBOX_SYS_WORK * syswk );

static void MailBagSet( MAILBOX_SYS_WORK * syswk );
static void SelMailDelete( MAILBOX_SYS_WORK * syswk );

static int MailWriteCall( MAILBOX_SYS_WORK * syswk );
static int MailWriteExit( MAILBOX_SYS_WORK * syswk );


static int MailWritePokeListEnd( MAILBOX_SYS_WORK * syswk );


//============================================================================================
//  グローバル変数
//============================================================================================

// メインシーケンステーブル
static const pMailBoxFunc MainSeq[] = {
  MainSeq_Init,                       // MBSEQ_MAINSEQ_INIT = 0,       // 初期化
  MainSeq_Release,                    // MBSEQ_MAINSEQ_RELEASE,        // 解放
  MainSeq_Wipe,                       // MBSEQ_MAINSEQ_WIPE,         // ワイプ
  MainSeq_MsgWait,                    // MBSEQ_MAINSEQ_MSG_WAIT,       // メッセージ
  MainSeq_YesNo,                      // MBSEQ_MAINSEQ_YESNO,        // はい・いいえ
  MainSeq_ButtonAnm,                  // MBSEQ_MAINSEQ_BUTTON_ANM,     // ボタンアニ
  MainSeq_SubProcCall,                // MBSEQ_MAINSEQ_SUBPROC_CALL,     // サブプロ
  MainSeq_SubProcMain,                // MBSEQ_MAINSEQ_SUBPROC_MAIN,     // サブプロ
  MainSeq_Start,                      // MBSEQ_MAINSEQ_START,        // メールボック
  MainSeq_MailSelectMain,             // MBSEQ_MAINSEQ_MAIL_SELECT_MAIN,   // メール
  MainSeq_MailBoxEndSet,              // MBSEQ_MAINSEQ_MAILBOX_END_SET,    // メール
  MainSeq_MailSelectPageChg,          // MBSEQ_MAINSEQ_MAIL_SELECT_PAGE_CHG, // メー
  MainSeq_MailMenuMsgSet,             // MBSEQ_MAINSEQ_MAIL_MENU_MSG_SET,  // メニュ
  MainSeq_MailMenuSet,                // MBSEQ_MAINSEQ_MAIL_MENU_SET,    // メニュー
  MainSeq_MailMenuMain,               // MBSEQ_MAINSEQ_MAIL_MENU_MAIN,   // メニュー
  MainSeq_MailReadMain,               // MBSEQ_MAINSEQ_MAIL_READ_MAIN,   //「メール
  MainSeq_MailReadEnd,                // MBSEQ_MAINSEQ_MAIL_READ_END,    //「メール
  MainSeq_MailDelYesNoSet,            // MBSEQ_MAINSEQ_MAILDEL_YESNO_SET,    //「メ
  MainSeq_MailPokeSetMsg,             // MBSEQ_MAINSEQ_MAILPOKESET_MSG,      // メー
  MainSeq_MailPokeSetYesNoSet,        // MBSEQ_MAINSEQ_MAILPOKESET_YESNO_SET,  //メ
  MainSeq_EraseMailPokeListExit,      // MBSEQ_MAINSEQ_ERASEMAIL_POKELIST_EXIT,    
  MainSeq_EraseMailPokeSetCancel,     // MBSEQ_MAINSEQ_ERASEMAIL_POKESET_CANCEL,   
  MainSeq_EraseMailPokeSetCancelEnd,  // MBSEQ_MAINSEQ_ERASEMAIL_POKESET_CANCEL_END
  MainSeq_MailWriteExit,              // MBSEQ_MAINSEQ_MAILWRITE_END,        // メー
  MainSeq_MailDelEnd,                 // MBSEQ_MAINSEQ_MAILDEL_END,        //「メー
};                                    // MBSEQ_MAINSEQ_END         // 終了
                                      
// サブプロセスデータ
static const SUBPROC_DATA SubProcFunc[] =
{
  { MailPokeListCall, MailPutPokeListExit, MBSEQ_MAINSEQ_INIT },              // もたせる
  { MailPokeListCall, EraseMailPokeListExit, MBSEQ_MAINSEQ_ERASEMAIL_POKELIST_EXIT },   // 消す
  { MailWriteCall, MailWriteExit, MBSEQ_MAINSEQ_MAILWRITE_END },              // メール作成
  { MailPokeListCall, MailWritePokeListEnd, MBSEQ_MAINSEQ_INIT },             // メール作成後
};

static const YESNO_DATA YesNoFunc[] =
{
  { MailDelYes, MainSeq_MailDelEnd },   // メールを消す
  { MailPokeAddYes, MailPokeAddNo },    // ポケモンに持たせる
};


//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス制御
 *
 * @param syswk メールボックス画面システムワーク
 * @param seq   メインシーケンス
 *
 * @retval  "TRUE = 処理中"
 * @retval  "FALSE = 終了"
 */
//--------------------------------------------------------------------------------------------
BOOL MBSEQ_Main( MAILBOX_SYS_WORK * syswk, int * seq )
{
  *seq = MainSeq[*seq]( syswk );

  if( *seq == MBSEQ_MAINSEQ_END ){
    return FALSE;
  }

  if( syswk->app != NULL ){
    int i;
    
    BGWINFRM_MoveMain( syswk->app->wfrm );
    MBOBJ_Main( syswk->app );

    if(syswk->app->printQue!=NULL){
      PRINTSYS_QUE_Main( syswk->app->printQue );
    }
    if(syswk->app->pMsgTcblSys!=NULL){
      GFL_TCBL_Main( syswk->app->pMsgTcblSys );
    }
    if(syswk->app->pmsPrintque){
      PMS_DRAW_Main( syswk->app->pms_draw_work );
      PRINTSYS_QUE_Main( syswk->app->pmsPrintque );
    }
  }

  return TRUE;
}



//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Init( MAILBOX_SYS_WORK * syswk )
{
  // 割り込み停止
//    sys_VBlankFuncChange( NULL, NULL );
  if(syswk->vintr_tcb!=NULL){
      GFL_TCB_DeleteTask( syswk->vintr_tcb );
      syswk->vintr_tcb = NULL;
  }
  // 表示初期化
  GFL_DISP_GX_SetVisibleControlDirect( 0 );
  GFL_DISP_GXS_SetVisibleControlDirect( 0 );
  // ブレンド初期化
  G2_BlendNone();
  G2S_BlendNone();

  // サブ画面をメインに
  GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

  // アプリヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MAILBOX_APP, 0x60000 );
  syswk->app = GFL_HEAP_AllocMemory( HEAPID_MAILBOX_APP, sizeof(MAILBOX_APP_WORK) );
  MI_CpuClearFast( syswk->app, sizeof(MAILBOX_APP_WORK) );

  MBMAIN_MailDataInit( syswk );

  MBMAIN_VramInit();

  MBMAIN_BgInit( syswk );
  MBMAIN_BgGraphicLoad( syswk );
  MBMAIN_WindowLoad( syswk );
  MBBMP_Init( syswk );
  MBMAIN_BgWinFrmInit( syswk );

  MBOBJ_Init( syswk );

  MBMAIN_PmsDrawInit( syswk );

//  MBMAIN_YesNoWinInit( syswk );

  MBUI_CursorMoveInit( syswk );

  MBMAIN_MsgInit( syswk->app );

  MBBMP_DefaultStrSet( syswk );
//  MBBMP_PagePut( syswk );   // ページ表示
  MBMAIN_MailButtonInit( syswk );
  MBMAIN_SelBoxInit( syswk );

//  sys_VBlankFuncChange( MBMAIN_VBlank, syswk );
  syswk->vintr_tcb = GFUser_VIntr_CreateTCB( MBMAIN_VBlank, syswk, 1 );

  return syswk->next_seq;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：解放
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Release( MAILBOX_SYS_WORK * syswk )
{
//  sys_VBlankFuncChange( NULL, NULL );
  if(syswk->vintr_tcb!=NULL){
      GFL_TCB_DeleteTask( syswk->vintr_tcb );
      syswk->vintr_tcb = NULL;
  }

  MBMAIN_SelBoxExit( syswk );

  MBMAIN_MsgExit( syswk->app );

  MBUI_CursorMoveExit( syswk );

//  MBMAIN_YesNoWinExit( syswk );

  MBMAIN_PmsDrawExit( syswk );

  MBOBJ_Exit( syswk );

  MBMAIN_BgWinFrmExit( syswk );
  MBBMP_Exit( syswk );
  MBMAIN_BgExit( syswk );

  MBMAIN_MailDataExit( syswk );


  G2_BlendNone();
  G2S_BlendNone();

  GFL_DISP_GX_SetVisibleControlDirect( 0 );
  GFL_DISP_GXS_SetVisibleControlDirect( 0 );

  GFL_HEAP_FreeMemory( syswk->app );
  syswk->app = NULL;

  GFL_HEAP_DeleteHeap( HEAPID_MAILBOX_APP );

  return syswk->next_seq;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ワイプ
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Wipe( MAILBOX_SYS_WORK * syswk )
{
  if( WIPE_SYS_EndCheck() == TRUE ){
    return syswk->app->wipe_seq;
  }
  return MBSEQ_MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メッセージ終了待ち
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MsgWait( MAILBOX_SYS_WORK * syswk )
{
  if(PRINTSYS_PrintStreamGetState( syswk->app->printStream )==PRINTSTREAM_STATE_DONE)
  {
    // ウェイトなし
    if( syswk->app->msgTrg == MSG_TRGWAIT_OFF ){
      PRINTSYS_PrintStreamDelete( syswk->app->printStream );
      return syswk->next_seq;
    }

    // タッチ待ち か　ボタン待ち
    if( GFL_UI_TP_GetTrg() == TRUE 
    || GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE))
    {
      PRINTSYS_PrintStreamDelete( syswk->app->printStream );
      PMSND_PlaySE( SND_MB_SELECT );
      return syswk->next_seq;
    }
    
  }

/*
  if( GF_MSG_PrintEndCheck( syswk->app->msgID ) == 0 ){
    // ウェイトなし
    if( syswk->app->msgTrg == MSG_TRGWAIT_OFF ){
      return syswk->next_seq;
    }
    // タッチ待ち
    if( GF_TP_GetTrg() == TRUE ){
      Snd_SePlay( SND_MB_SELECT );
      return syswk->next_seq;
    }
    // ボタン待ち
    if( sys.trg & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE) ){
      Snd_SePlay( SND_MB_SELECT );
      return syswk->next_seq;
    }
  }
*/
  return MBSEQ_MAINSEQ_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：はい・いいえ
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_YesNo( MAILBOX_SYS_WORK * syswk )
{
  APP_TASKMENU_UpdateMenu(syswk->app->menuwork);
  if( APP_TASKMENU_IsFinish( syswk->app->menuwork )==FALSE){
    return MBSEQ_MAINSEQ_YESNO;
  }
  switch( APP_TASKMENU_GetCursorPos(syswk->app->menuwork) ){
  case 0: // はい
    APP_TASKMENU_CloseMenu( syswk->app->menuwork );
    return YesNoFunc[syswk->app->ynID].yes( syswk );
    break;
  case 1: // いいえ
    APP_TASKMENU_CloseMenu( syswk->app->menuwork );
    return YesNoFunc[syswk->app->ynID].no( syswk );
    break;
  }

/*
  switch( TOUCH_SW_Main( syswk->app->tsw ) ){
  case TOUCH_SW_RET_YES:
    TOUCH_SW_Reset( syswk->app->tsw );
    return YesNoFunc[syswk->app->ynID].yes( syswk );

  case TOUCH_SW_RET_NO:
    TOUCH_SW_Reset( syswk->app->tsw );
    return YesNoFunc[syswk->app->ynID].no( syswk );
  }
*/

  return MBSEQ_MAINSEQ_YESNO;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：ボタンアニメ
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ButtonAnm( MAILBOX_SYS_WORK * syswk )
{
  if( MBMAIN_ButtonAnmMain( syswk ) == FALSE ){
    return syswk->next_seq;
  }
  return MBSEQ_MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：サブプロセス呼び出し
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SubProcCall( MAILBOX_SYS_WORK * syswk )
{
  SubProcFunc[syswk->subProcType].call( syswk );
  return MBSEQ_MAINSEQ_SUBPROC_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：サブプロセスメイン
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SubProcMain( MAILBOX_SYS_WORK * syswk )
{
  //ローカルPROCの更新処理
  GFL_PROC_MAIN_STATUS proc_status = GFL_PROC_LOCAL_Main( syswk->mbProcSys );

  // 終了待ち
  if(proc_status == GFL_PROC_MAIN_VALID){
    return MBSEQ_MAINSEQ_SUBPROC_MAIN;
  }

  syswk->next_seq = SubProcFunc[syswk->subProcType].exit( syswk );
  return SubProcFunc[syswk->subProcType].rcvSeq;

  
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メールボックス画面開始
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Start( MAILBOX_SYS_WORK * syswk )
{
  return FadeInSet( syswk, MBSEQ_MAINSEQ_MAIL_SELECT_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メール選択メイン
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailSelectMain( MAILBOX_SYS_WORK * syswk )
{
  u32 ret;
  
  // タッチのみボタンチェック
  switch( MBUI_MailboxTouchButtonCheck( syswk ) ){
  case 0:   // 左矢印
    if( syswk->sel_page != 0 ){
      syswk->sel_page--;
      PMSND_PlaySE( SND_MB_SELECT );
      return ObjButtonAnmSet( syswk, MBMAIN_OBJ_ARROW_L, MBSEQ_MAINSEQ_MAIL_SELECT_PAGE_CHG );
    }
    return MBSEQ_MAINSEQ_MAIL_SELECT_MAIN;
    break;
  case 1:   // 右矢印
    if( syswk->sel_page != syswk->app->page_max ){
      syswk->sel_page++;
      PMSND_PlaySE( SND_MB_SELECT );
      return ObjButtonAnmSet( syswk, MBMAIN_OBJ_ARROW_R, MBSEQ_MAINSEQ_MAIL_SELECT_PAGE_CHG );
    }
    return MBSEQ_MAINSEQ_MAIL_SELECT_MAIN;
    break;
  case 2:   // やめる
    PMSND_PlaySE( SND_MB_CANCEL );
    return ObjButtonAnmSet( syswk, MBMAIN_OBJ_RET_BTN, MBSEQ_MAINSEQ_MAILBOX_END_SET );
    break;
  }

  ret = CURSORMOVE_MainCont( syswk->app->cmwk );

  switch( ret ){
  case MBUI_MAILSEL_MAIL1:  // 00: メール１
  case MBUI_MAILSEL_MAIL2:  // 01: メール２
  case MBUI_MAILSEL_MAIL3:  // 02: メール３
  case MBUI_MAILSEL_MAIL4:  // 03: メール４
  case MBUI_MAILSEL_MAIL5:  // 04: メール５
  case MBUI_MAILSEL_MAIL6:  // 05: メール６
  case MBUI_MAILSEL_MAIL7:  // 06: メール７
  case MBUI_MAILSEL_MAIL8:  // 07: メール８
  case MBUI_MAILSEL_MAIL9:  // 08: メール９
  case MBUI_MAILSEL_MAIL10: // 09: メール１０
    if( syswk->app->mail_list[syswk->sel_page*MBMAIN_MAILLIST_MAX+ret] != MBMAIN_MAILLIST_NULL ){
      syswk->sel_pos = ret;
      syswk->lst_pos = syswk->app->mail_list[ syswk->sel_page * MBMAIN_MAILLIST_MAX + ret ];
      PMSND_PlaySE( SND_MB_DECIDE );
      return BgWinFrmButtonAnmSet( syswk, ret,  MBSEQ_MAINSEQ_MAIL_MENU_MSG_SET );
    }
    break;

//  case MBUI_MAILSEL_RETURN: // 10: やめる
  case CURSORMOVE_CANCEL:   // キャンセル
    PMSND_PlaySE( SND_MB_CANCEL );
    return ObjButtonAnmSet( syswk, MBMAIN_OBJ_RET_BTN, MBSEQ_MAINSEQ_MAILBOX_END_SET );

  case CURSORMOVE_NO_MOVE_RIGHT:     // カーソルは動かず右が押された
    {
      u8  pos = CURSORMOVE_PosGet( syswk->app->cmwk );
      if( syswk->sel_page != syswk->app->page_max ){
        syswk->sel_page++;
        PMSND_PlaySE( SND_MB_SELECT );
        return ObjButtonAnmSet( syswk, MBMAIN_OBJ_ARROW_R, MBSEQ_MAINSEQ_MAIL_SELECT_PAGE_CHG );
      }
    }
    break;
  case CURSORMOVE_NO_MOVE_LEFT:     // カーソルは動かず左が押された
    {
      u8  pos = CURSORMOVE_PosGet( syswk->app->cmwk );
      if( syswk->sel_page != 0 ){
        syswk->sel_page--;
        PMSND_PlaySE( SND_MB_SELECT );
        return ObjButtonAnmSet( syswk, MBMAIN_OBJ_ARROW_L, MBSEQ_MAINSEQ_MAIL_SELECT_PAGE_CHG );
      }
    }
    break;

  case CURSORMOVE_CURSOR_ON:    // カーソル表示
    break;

  case CURSORMOVE_CURSOR_MOVE:  // 移動
    PMSND_PlaySE( SND_MB_SELECT );
    break;
  }


  return MBSEQ_MAINSEQ_MAIL_SELECT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メールボックス終了設定
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailBoxEndSet( MAILBOX_SYS_WORK * syswk )
{
  syswk->next_seq = MBSEQ_MAINSEQ_END;
  return FadeOutSet( syswk, MBSEQ_MAINSEQ_RELEASE );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メール選択ページ切り替え
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailSelectPageChg( MAILBOX_SYS_WORK * syswk )
{
  if( syswk->sel_page == 0 ){
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_L, FALSE );
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_R, TRUE );
  }else{
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_L, TRUE );
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_R, FALSE );
  }

//  MBBMP_PagePut( syswk );   // ページ表示
  MBMAIN_MailButtonInit( syswk );
  return MBSEQ_MAINSEQ_MAIL_SELECT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メニューメッセージセット
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailMenuMsgSet( MAILBOX_SYS_WORK * syswk )
{
  MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_CURSOR, FALSE );
  AlphaSet( syswk->app, TRUE );
  MBMAIN_SelMailTabPut( syswk );
  MBBMP_TalkWinPut( syswk );
  MBBMP_SelectMsgPut( syswk );
  return MsgWaitSet( syswk, MSG_TRGWAIT_OFF, MBSEQ_MAINSEQ_MAIL_MENU_SET );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メニューセット
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailMenuSet( MAILBOX_SYS_WORK * syswk )
{
  MBMAIN_SelBoxPut( syswk );
  return MBSEQ_MAINSEQ_MAIL_MENU_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：メニューメイン
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailMenuMain( MAILBOX_SYS_WORK * syswk )
{
  APP_TASKMENU_UpdateMenu(syswk->app->menuwork);
  if( APP_TASKMENU_IsFinish( syswk->app->menuwork )==FALSE){
    return MBSEQ_MAINSEQ_MAIL_MENU_MAIN;
  }
//  switch( SelectBoxMain(syswk->app->sbwk) ){
  switch( APP_TASKMENU_GetCursorPos(syswk->app->menuwork) ){
  case MBMAIN_MENU_READ:    // メールを読む
    MBMAIN_SelBoxDel( syswk );
    MBBMP_TalkWinDel( syswk );
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_CURSOR, TRUE );
    MBBMP_ClosePut( syswk->app );
    MBOBJ_MailReadCurMove( syswk->app );
    SubDispMailWrite( syswk );
    return MBSEQ_MAINSEQ_MAIL_READ_MAIN;

  case MBMAIN_MENU_DELETE:  // メールを消す
    MBMAIN_SelBoxDel( syswk );
    MBBMP_DeleteMsgPut( syswk, MBBMP_DELMSG_ID_CHECK );
    return MsgWaitSet( syswk, MSG_TRGWAIT_OFF, MBSEQ_MAINSEQ_MAILDEL_YESNO_SET );

  case MBMAIN_MENU_POKESET: // 持たせる
    MBMAIN_SelBoxDel( syswk );
    {
      MAIL_DATA * md = syswk->app->mail[ syswk->lst_pos ];
      PokeListWorkCreate( syswk, PL_MODE_MAILBOX, ITEM_MailNumGet(MailData_GetDesignNo(md)), 0 );
    }
    syswk->subProcType = SUBPROC_TYPE_PL_PUT;
    syswk->next_seq = MBSEQ_MAINSEQ_SUBPROC_CALL;
    return FadeOutSet( syswk, MBSEQ_MAINSEQ_RELEASE );

  case MBMAIN_MENU_CANCEL:  // やめる
  case SBOX_SELECT_CANCEL:
    MBMAIN_SelBoxDel( syswk );
    MBBMP_TalkWinDel( syswk );
    BGWINFRM_FrameOff( syswk->app->wfrm, MBMAIN_BGWF_SELMAIL );
    AlphaSet( syswk->app, FALSE );
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_CURSOR, TRUE );
    return MBSEQ_MAINSEQ_MAIL_SELECT_MAIN;

  case SBOX_SELECT_NULL:
    break;
  }

  return MBSEQ_MAINSEQ_MAIL_MENU_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「メールをよむ」メイン
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailReadMain( MAILBOX_SYS_WORK * syswk )
{
  if( MBUI_MailReadCheck() == TRUE ){
    PMSND_PlaySE( SND_MB_DECIDE );
    return ObjButtonAnmSet( syswk, MBMAIN_OBJ_RET_BTN, MBSEQ_MAINSEQ_MAIL_READ_END );

  }
  return MBSEQ_MAINSEQ_MAIL_READ_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「メールをよむ」終了
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailReadEnd( MAILBOX_SYS_WORK * syswk )
{
  SubDispMailClear( syswk );
  MBUI_MailSelCurMove( syswk, CURSORMOVE_PosGet(syswk->app->cmwk) );
  MBBMP_ReturnPut( syswk->app );
  BGWINFRM_FrameOff( syswk->app->wfrm, MBMAIN_BGWF_SELMAIL );
  AlphaSet( syswk->app, FALSE );
  return MBSEQ_MAINSEQ_MAIL_SELECT_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「メールをけす」はい・いいえセット
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailDelYesNoSet( MAILBOX_SYS_WORK * syswk )
{
  return YesNoSet( syswk, YESNO_ID_DELETE );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンにもたせる」メッセージ表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailPokeSetMsg( MAILBOX_SYS_WORK * syswk )
{
  MBBMP_DeleteMsgPut( syswk, MBBMP_DELMSG_ID_POKE );
  return MsgWaitSet( syswk, MSG_TRGWAIT_OFF, MBSEQ_MAINSEQ_MAILPOKESET_YESNO_SET );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「ポケモンにもたせる」はい・いいえセット
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailPokeSetYesNoSet( MAILBOX_SYS_WORK * syswk )
{
  return YesNoSet( syswk, YESNO_ID_POKEADD );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：消去メールを持たせるポケモンを選択後
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EraseMailPokeListExit( MAILBOX_SYS_WORK * syswk )
{
  if( syswk->ret_mode != PL_RET_MAILSET ){
//  if( syswk->ret_poke > PL_SEL_POS_POKE6 ){
    syswk->next_seq = MBSEQ_MAINSEQ_ERASEMAIL_POKESET_CANCEL;
    return MBSEQ_MAINSEQ_INIT;
  }

  syswk->subProcType = SUBPROC_TYPE_MAILWRITE;
  return MBSEQ_MAINSEQ_SUBPROC_CALL;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：消去メールを持たせるポケモン選択をキャンセル
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EraseMailPokeSetCancel( MAILBOX_SYS_WORK * syswk )
{
  MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_CURSOR, FALSE );
  AlphaSet( syswk->app, TRUE );
  return FadeInSet( syswk, MBSEQ_MAINSEQ_ERASEMAIL_POKESET_CANCEL_END );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：消去メールを持たせるポケモン選択をキャンセル後のメッセージ表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EraseMailPokeSetCancelEnd( MAILBOX_SYS_WORK * syswk )
{
  MBBMP_TalkWinPut( syswk );
  return MailPokeAddNo( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：消去メールに文字を書いた後
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailWriteExit( MAILBOX_SYS_WORK * syswk )
{
  if( syswk->mw_ret == FALSE ){
    syswk->next_seq = MBSEQ_MAINSEQ_ERASEMAIL_POKESET_CANCEL;
    return MBSEQ_MAINSEQ_INIT;
  }

  PokeListWorkCreate( syswk, PL_MODE_MAILSET, syswk->ret_item, syswk->ret_poke );
  syswk->subProcType = SUBPROC_TYPE_PL_MW_END;
  return MBSEQ_MAINSEQ_SUBPROC_CALL;
}

//--------------------------------------------------------------------------------------------
/**
 * メインシーケンス：「メールをけす」終了
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_MailDelEnd( MAILBOX_SYS_WORK * syswk )
{
  MBBMP_TalkWinDel( syswk );
  BGWINFRM_FrameOff( syswk->app->wfrm, MBMAIN_BGWF_SELMAIL );
  MBUI_MailSelCurMove( syswk, CURSORMOVE_PosGet(syswk->app->cmwk) );
  MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_CURSOR, TRUE );
  AlphaSet( syswk->app, FALSE );
  return MBSEQ_MAINSEQ_MAIL_SELECT_MAIN;
}


//============================================================================================
//  サブルーチン
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * フェードインセット（ワイプ）
 *
 * @param syswk メールボックス画面システムワーク
 * @param next  フェード後のシーケンス
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int FadeInSet( MAILBOX_SYS_WORK * syswk, int next )
{
  WIPE_SYS_Start(
    WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
    WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_MAILBOX_APP );

  syswk->app->wipe_seq = next;

  return MBSEQ_MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * フェードインセット（ワイプ）
 *
 * @param syswk メールボックス画面システムワーク
 * @param next  フェード後のシーケンス
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int FadeOutSet( MAILBOX_SYS_WORK * syswk, int next )
{
  WIPE_SYS_Start(
    WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
    WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_MAILBOX_APP );

  syswk->app->wipe_seq = next;

  return MBSEQ_MAINSEQ_WIPE;
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ表示セット
 *
 * @param syswk メールボックス画面システムワーク
 * @param wait  トリガーウェイト
 * @param next  表示後のシーケンス
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MsgWaitSet( MAILBOX_SYS_WORK * syswk, u32 wait, int next )
{
  syswk->app->msgTrg = wait;
  syswk->next_seq = next;
  return MBSEQ_MAINSEQ_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえセット
 *
 * @param syswk メールボックス画面システムワーク
 * @param id    はい・いいえＩＤ
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int YesNoSet( MAILBOX_SYS_WORK * syswk, u32 id )
{
  syswk->app->ynID = id;
  MBMAIN_YesNoWinSet( syswk );
  return MBSEQ_MAINSEQ_YESNO;
}

//--------------------------------------------------------------------------------------------
/**
 * BGWINFRMのボタンアニメセット
 *
 * @param syswk メールボックス画面システムワーク
 * @param frmID BGWINFRMのＩＤ
 * @param next  アニメ後のシーケンス
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BgWinFrmButtonAnmSet( MAILBOX_SYS_WORK * syswk, u32 frmID, int next )
{
  BUTTON_ANM_WORK * bawk;
  u16 sx, sy;
  s8  px, py;
  
  BGWINFRM_PosGet( syswk->app->wfrm, frmID, &px, &py );
  BGWINFRM_SizeGet( syswk->app->wfrm, frmID, &sx, &sy );
  
  bawk = &syswk->app->bawk;

  bawk->btn_mode = MBMAIN_BTN_ANM_MODE_BG;
  bawk->btn_id   = MBMAIN_BGF_BTN_M;
  bawk->btn_pal1 = 2;
  bawk->btn_pal2 = 1;
  bawk->btn_seq  = 0;
  bawk->btn_cnt  = 0;
  bawk->btn_px   = px;
  bawk->btn_py   = py;
  bawk->btn_sx   = sx;
  bawk->btn_sy   = sy;

  syswk->next_seq = next;

  return MBSEQ_MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * OBJのボタンアニメセット
 *
 * @param syswk メールボックス画面システムワーク
 * @param objID OBJ ID
 * @param next  アニメ後のシーケンス
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int ObjButtonAnmSet( MAILBOX_SYS_WORK * syswk, u32 objID, int next )
{
  BUTTON_ANM_WORK * bawk = &syswk->app->bawk;

  bawk->btn_mode = MBMAIN_BTN_ANM_MODE_OBJ;
  bawk->btn_id   = objID;
  bawk->btn_pal1 = 1;
  bawk->btn_pal2 = 0;
  bawk->btn_seq  = 0;
  bawk->btn_cnt  = 0;

  syswk->next_seq = next;

  return MBSEQ_MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * 半透明設定
 *
 * @param flg   TRUE = 半透明, FALSE = 通常
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void AlphaSet( MAILBOX_APP_WORK * appwk, BOOL flg )
{
  if( flg == TRUE ){
    MBOBJ_BlendModeSet( appwk, MBMAIN_OBJ_ARROW_L, TRUE );
    MBOBJ_BlendModeSet( appwk, MBMAIN_OBJ_ARROW_R, TRUE );
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 4, 12 );
  }else{
    MBOBJ_BlendModeSet( appwk, MBMAIN_OBJ_ARROW_L, FALSE );
    MBOBJ_BlendModeSet( appwk, MBMAIN_OBJ_ARROW_R, FALSE );
    G2_BlendNone();
  }
}

//--------------------------------------------------------------------------------------------
/**
 * メール表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void SubDispMailWrite( MAILBOX_SYS_WORK * syswk )
{
  MBMAIN_MailGraphcLoad( syswk );
  MBBMP_MailMesPut( syswk );
  MBOBJ_PokeIconPut( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メール非表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void SubDispMailClear( MAILBOX_SYS_WORK * syswk )
{
//  MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_POKEICON1, FALSE );
//  MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_POKEICON2, FALSE );
//  MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_POKEICON3, FALSE );
  PMS_DRAW_Clear( syswk->app->pms_draw_work, 0, FALSE );
  PMS_DRAW_Clear( syswk->app->pms_draw_work, 1, FALSE );
  PMS_DRAW_Clear( syswk->app->pms_draw_work, 2, FALSE );
  GFL_BG_ClearScreen( MBMAIN_BGF_MAILMES_S );
  GFL_BG_ClearScreen( MBMAIN_BGF_MAIL_S );
}

//--------------------------------------------------------------------------------------------
/**
 * 選択メールをバッグへ入れる
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void MailBagSet( MAILBOX_SYS_WORK * syswk )
{
  MYITEM    *myitem;
  MAIL_DATA *mail;
  
  myitem = GAMEDATA_GetMyItem( syswk->dat->gamedata );
  mail   = syswk->app->mail[ syswk->lst_pos ];

  syswk->ret_item = ITEM_MailNumGet( MailData_GetDesignNo(mail) );

  if( MYITEM_AddItem( myitem, syswk->ret_item, 1, HEAPID_MAILBOX_APP ) == TRUE ){
    syswk->check_item = 0;
  }else{
    syswk->check_item = 1;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * 選択メールを消去
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void SelMailDelete( MAILBOX_SYS_WORK * syswk )
{
  MAIL_DelMailData( syswk->sv_mail, MAILBLOCK_PASOCOM, syswk->lst_pos );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンリストのワーク作成
 *
 * @param syswk メールボックス画面システムワーク
 * @param mode  呼び出しモード
 * @param item  持たせるアイテム
 * @param pos   初期位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PokeListWorkCreate( MAILBOX_SYS_WORK * syswk, u32 mode, u16 item, u16 pos )
{
  PLIST_DATA * pld = GFL_HEAP_AllocClearMemory( HEAPID_MAILBOX_SYS, sizeof(PLIST_DATA) );

  pld->pp        = GAMEDATA_GetMyPokemon( syswk->dat->gamedata );
  pld->myitem    = GAMEDATA_GetMyItem( syswk->dat->gamedata );
  pld->mailblock = syswk->sv_mail;
  pld->type      = PL_TYPE_SINGLE;
  pld->mode      = mode;
  pld->item      = item;
//  pld->fsys      = syswk->dat->fsys;
//  pld->kt_status = syswk->dat->kt_status;
  pld->ret_sel   = pos;

  syswk->subProcWork = pld;
}
FS_EXTERN_OVERLAY(pokelist);

//--------------------------------------------------------------------------------------------
/**
 * ポケモンリスト呼び出し
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
static int MailPokeListCall( MAILBOX_SYS_WORK * syswk )
{
  
  GFL_PROC_LOCAL_CallProc( syswk->mbProcSys, FS_OVERLAY_ID(pokelist), &PokeList_ProcData, syswk->subProcWork );
  return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * 「メールをもたせる」のポケモンリスト終了後の処理
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次の次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MailPutPokeListExit( MAILBOX_SYS_WORK * syswk )
{
  PLIST_DATA * pld;
  POKEMON_PARAM * pp;

  pld = syswk->subProcWork;

  if( pld->ret_sel <= PL_SEL_POS_POKE6 ){
    pp = PokeParty_GetMemberPointer(
        GAMEDATA_GetMyPokemon(syswk->dat->gamedata), pld->ret_sel );
    MailSys_MoveMailPaso2Poke( syswk->sv_mail, syswk->lst_pos, pp, HEAPID_MAILBOX_SYS );
  }

  GFL_HEAP_FreeMemory( syswk->subProcWork );

  return MBSEQ_MAINSEQ_START;
}

//--------------------------------------------------------------------------------------------
/**
 * 「メールをけす」のポケモンリスト終了後の処理
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
static int EraseMailPokeListExit( MAILBOX_SYS_WORK * syswk )
{
  PLIST_DATA * pld;

  pld = syswk->subProcWork;

  syswk->ret_poke = pld->ret_sel;
  syswk->ret_mode = pld->ret_mode;

  GFL_HEAP_FreeMemory( syswk->subProcWork );

  return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * 「メールをけす」でメールを書いた後のポケモンリスト終了後の処理
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次の次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MailWritePokeListEnd( MAILBOX_SYS_WORK * syswk )
{
  GFL_HEAP_FreeMemory( syswk->subProcWork );
  return MBSEQ_MAINSEQ_START;
}

//--------------------------------------------------------------------------------------------
/**
 * メール作成画面呼び出し
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
static int MailWriteCall( MAILBOX_SYS_WORK * syswk )
{
  MAIL_PARAM * prm;

  prm = MAILSYS_GetWorkCreate(
      syswk->dat->gamedata,
      MAILBLOCK_TEMOTI,
      syswk->ret_poke,
      ITEM_GetMailDesign(syswk->ret_item),
      HEAPID_MAILBOX_SYS );

//  prm->pKeytouch = syswk->dat->kt_status;

  syswk->subProcWork = prm;

  {
    // オーバーレイプロセス定義データ
    static const GFL_PROC_DATA MailSysProcData = {
      MailSysProc_Init,
      MailSysProc_Main,
      MailSysProc_End,
    };

    GFL_PROC_LOCAL_CallProc( syswk->mbProcSys, NO_OVERLAY_ID, &MailSysProcData, syswk->subProcWork );
  }

  return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * メール作成画面終了後
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
static int MailWriteExit( MAILBOX_SYS_WORK * syswk )
{
  syswk->mw_ret = MailSys_IsDataCreate( syswk->subProcWork );

  if( syswk->mw_ret == TRUE ){
    POKEMON_PARAM * pp;
    MYITEM * myitem;
    
    pp = PokeParty_GetMemberPointer(
        GAMEDATA_GetMyPokemon(syswk->dat->gamedata),
        syswk->ret_poke );
    MailSys_PushDataToSavePoke( syswk->subProcWork, pp );

    myitem = GAMEDATA_GetMyItem( syswk->dat->gamedata );
    MYITEM_SubItem( myitem, syswk->ret_item, 1, HEAPID_MAILBOX_SYS );
  }

  MailSys_ReleaseCallWork( syswk->subProcWork );

  return 0;

}

//--------------------------------------------------------------------------------------------
/**
 * 「メールをけす」で「はい」を選択したとき
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MailDelYes( MAILBOX_SYS_WORK * syswk )
{
  MailBagSet( syswk );    // バッグへ
  SelMailDelete( syswk );   // メール削除

  // 画面更新
  MBMAIN_MailDataExit( syswk );
  MBMAIN_MailDataInit( syswk );
  if( syswk->app->mail_max <= MBMAIN_MAILLIST_MAX ){
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_L, FALSE );
    MBOBJ_Vanish( syswk->app, MBMAIN_OBJ_ARROW_R, FALSE );
  }
  if( syswk->sel_page > syswk->app->page_max ){
    syswk->sel_page = syswk->app->page_max;
    syswk->sel_pos  = 0;
  }
//  MBBMP_PagePut( syswk );   // ページ表示
  MBMAIN_MailButtonInit( syswk );
  BGWINFRM_FrameOff( syswk->app->wfrm, MBMAIN_BGWF_SELMAIL );

  MBBMP_DeleteMsgPut( syswk, MBBMP_DELMSG_ID_ERASE );
  return MsgWaitSet( syswk, MSG_TRGWAIT_ON, MBSEQ_MAINSEQ_MAILPOKESET_MSG );
}

//--------------------------------------------------------------------------------------------
/**
 * 「メールをもたせますか」で「はい」を選択したとき
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MailPokeAddYes( MAILBOX_SYS_WORK * syswk )
{
  PokeListWorkCreate( syswk, PL_MODE_ITEMSET, syswk->ret_item, 0 );

  syswk->subProcType = SUBPROC_TYPE_PL_DEL;
  syswk->next_seq = MBSEQ_MAINSEQ_SUBPROC_CALL;
  return FadeOutSet( syswk, MBSEQ_MAINSEQ_RELEASE );
}

//--------------------------------------------------------------------------------------------
/**
 * 「メールをもたせますか」で「いいえ」を選択したとき
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MailPokeAddNo( MAILBOX_SYS_WORK * syswk )
{
  if( syswk->check_item == 0 ){
    MBBMP_DeleteMsgPut( syswk, MBBMP_DELMSG_ID_BAG );
  }else{
    MBBMP_DeleteMsgPut( syswk, MBBMP_DELMSG_ID_DELETE );
  }
  return MsgWaitSet( syswk, MSG_TRGWAIT_ON, MBSEQ_MAINSEQ_MAILDEL_END );
}
