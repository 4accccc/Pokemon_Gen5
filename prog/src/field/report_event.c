//============================================================================================
/**
 * @file    report_event.h
 * @brief   フィールドメニュー・レポート処理
 * @author  Hiroyuki Nakamura
 * @date    10.02.20
 */
//============================================================================================
#include <gflib.h>

#include "gamesystem/msgspeed.h"
#include "system/bmp_winframe.h"
#include "savedata/playtime.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"
#include "app/app_keycursor.h"

#include "message.naix"
#include "msg/msg_report.h"

#include "script_message.naix"
#include "msg/script/msg_common_scr.h"

#include "fieldmap.h"
#include "field_subscreen.h"

#include "field_saveanime.h"

#include "report_event.h"


//============================================================================================
//  定数定義
//============================================================================================

// メインシーケンス
enum {
  REPORT_SEQ_SUBDISP_SET = 0,         // サブ画面セット
  REPORT_SEQ_INIT_WAIT,               // サブ画面初期化待ち
  REPORT_SEQ_INIT_MESSAGE,            // 初期メッセージ
  REPORT_SEQ_WRITE_YESNO_SET,         // セーブしますか？【はい・いいえ】セット
  REPORT_SEQ_WRITE_YESNO_WAIT,        // セーブしますか？【はい・いいえ】待ち
/*
  REPORT_SEQ_OVERWRITE_YESNO_SET,     // 上書きしますか？【はい・いいえ】セット
  REPORT_SEQ_OVERWRITE_YESNO_WAIT,    // 上書きしますか？【はい・いいえ】待ち
*/
  REPORT_SEQ_SAVE_SIZE_CHECK,         // セーブサイズ取得
  REPORT_SEQ_PALYER_ANM_CHG,          // 主人公アニメ変更
  REPORT_SEQ_SAVE_INIT,               // セーブ初期設定
  REPORT_SEQ_SAVE_MAIN,               // セーブ実行
  REPORT_SEQ_RESULT_OK_BAR_WAIT,      // セーブ成功バー待ち
  REPORT_SEQ_RESULT_OK_WAIT,          // セーブ成功メッセージ待ち
  REPORT_SEQ_RESULT_SE_WAIT,          // ＳＥ待ち
  REPORT_SEQ_RESULT_NG_WAIT,          // セーブ失敗メッセージ待ち
  REPORT_SEQ_RESULT_NG_TRG_WAIT,      // セーブ失敗メッセージ後のキー待ち
  REPORT_SEQ_END_TRG_WAIT,            // セーブ後のキー待ち

  REPORT_SEQ_SAVE_CANCEL,             // セーブキャンセル
  REPORT_SEQ_SAVE_CANCEL_END,         // 終了（セーブキャンセル）
  REPORT_SEQ_SAVE_COMP,               // セーブ実行
  REPORT_SEQ_SAVE_COMP_END,           // 終了（セーブ実行）

//  REPORT_SEQ_END,                     // 終了
};

#define REPORT_STR_LEN    ( 512 )   // 文字列バッファの長さ

#define BGPAL_MSGFNT  ( 12 )        // パレット：メッセージフォント
#define BGPAL_WINFRM  ( 13 )        // パレット：メッセージウィンドウ
#define BGPAL_YESNO   ( 14 )        // パレット：はい・いいえ

#define WINFRM_CGXNUM   ( 1 )       // メッセージウィンドウキャラ読み込み位置

// メッセージウィンドウデータ
#define MSG_PX    ( 1 )
#define MSG_PY    ( 1 )
#define MSG_SX    ( 30 )
#define MSG_SY    ( 4 )

//「はい・いいえ」データ
#define YESNO_PX  ( 32 )
#define YESNO_PY  ( 12 )

#define END_AUTO_TIME   ( 30*5 )    // セーブ後、自動終了までの時間


struct _REPORT_EVENT_LOCAL {
  GFL_TCBLSYS * tcbl;

  GFL_MSGDATA * msgData;
//  GFL_BMPWIN * win;
  PRINT_UTIL  util;
  PRINT_STREAM * stream;
  PRINT_QUE * que;            // プリントキュー
  STRBUF * strBuff;
  APP_KEYCURSOR_WORK * kcwk;  // メッセージ送りカーソル
  BOOL stream_clear_flg;

  APP_TASKMENU_ITEMWORK ynList[2];
  APP_TASKMENU_RES * ynRes;
  APP_TASKMENU_WORK * ynWork;

  TIMEICON_WORK * timeIcon;
  FIELD_SAVEANIME* bgAnime;

  u32 wait;
};


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void ReleaseLocalWork( FMENU_REPORT_EVENT_WORK * wk );
static void InitReportBmpWin( FMENU_REPORT_EVENT_WORK * wk );
static void ExitReportBmpWin( FMENU_REPORT_EVENT_WORK * wk );
static void SetReportMsg( FMENU_REPORT_EVENT_WORK * wk, u32 strIdx );
static void SetReportMsgBuff( FMENU_REPORT_EVENT_WORK * wk );
static BOOL MainReportMsg( FMENU_REPORT_EVENT_WORK * wk );
static void MsgPrint( FMENU_REPORT_EVENT_WORK * wk, u32 strIdx );
static BOOL CheckMsgPrint( FMENU_REPORT_EVENT_WORK * wk );
static void SetReportPlayerAnime( FMENU_REPORT_EVENT_WORK * work );
static void ResetReportPlayerAnime( FMENU_REPORT_EVENT_WORK * work );
static BOOL CheckPlayerAnime( FMENU_REPORT_EVENT_WORK * wk );
static void SetReportBgAnime( FMENU_REPORT_EVENT_WORK * work );
static void ResetReportBgAnime( FMENU_REPORT_EVENT_WORK * work );
static void InitReportYesNo( FMENU_REPORT_EVENT_WORK * wk );
static void ExitReportYesNo( FMENU_REPORT_EVENT_WORK * wk );
static void SetReportYesNo( FMENU_REPORT_EVENT_WORK * wk );
static s32 MainReportYesNo( FMENU_REPORT_EVENT_WORK * wk );


//--------------------------------------------------------------------------------------------
/**
 * メイン処理
 *
 * @param   wk    ワーク
 * @param   seq   シーケンス
 *
 * @retval  "REPORTEVENT_RET_SAVE = セーブ実行"
 * @retval  "REPORTEVENT_RET_CANCEL = キャンセル"
 * @retval  "REPORTEVENT_RET_NONE = 処理中"
 */
//--------------------------------------------------------------------------------------------
int REPORTEVENT_Main( FMENU_REPORT_EVENT_WORK * wk, int * seq )
{
  switch( (*seq) ){
  case REPORT_SEQ_SUBDISP_SET:            // サブ画面セット
    FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork), FIELD_SUBSCREEN_REPORT );
    *seq = REPORT_SEQ_INIT_WAIT;
    break;

  case REPORT_SEQ_INIT_WAIT:              // サブ画面初期化待ち
    if( FIELD_SUBSCREEN_CanChange(FIELDMAP_GetFieldSubscreenWork(wk->fieldWork)) == TRUE ){
      wk->local = GFL_HEAP_AllocMemory( wk->heapID, sizeof(REPORT_EVENT_LOCAL) );
      wk->local->msgData = GFL_MSG_Create(
                        GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE,
                        NARC_script_message_common_scr_dat, wk->heapID );
      wk->local->strBuff = GFL_STR_CreateBuffer( REPORT_STR_LEN, wk->heapID );
      wk->local->tcbl = GFL_TCBL_Init( wk->heapID, wk->heapID, 1, 4 );
      wk->local->kcwk = APP_KEYCURSOR_Create( 15, TRUE, TRUE, wk->heapID );
      wk->local->que  = PRINTSYS_QUE_Create( wk->heapID );
      wk->local->wait = 0;
      InitReportBmpWin( wk );
      InitReportYesNo( wk );
      *seq = REPORT_SEQ_INIT_MESSAGE;
    }
    break;

  case REPORT_SEQ_INIT_MESSAGE:           // 初期メッセージ
    SetReportMsg( wk, msg_common_report_01 );
    *seq = REPORT_SEQ_WRITE_YESNO_SET;
    break;

  case REPORT_SEQ_WRITE_YESNO_SET:        // セーブしますか？【はい・いいえ】セット
    if( MainReportMsg( wk ) == FALSE ){
      SetReportYesNo( wk );
      *seq = REPORT_SEQ_WRITE_YESNO_WAIT;
    }
    break;

  case REPORT_SEQ_WRITE_YESNO_WAIT:       // セーブしますか？【はい・いいえ】待ち
    switch( MainReportYesNo(wk) ){
    case 0: // はい
      // 通信中でセーブできない
      if( GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(wk->gsys)) == GAME_COMM_NO_INVASION ){
        SetReportMsg( wk, msg_common_report_13 );
        *seq = REPORT_SEQ_RESULT_NG_WAIT;
      // 違うセーブデータがある
      }else if( SaveControl_IsOverwritingOtherData(wk->sv) == TRUE ){
        SetReportMsg( wk, msg_common_report_08 );
        *seq = REPORT_SEQ_RESULT_NG_WAIT;
/*
      // セーブデータがある
      }else if( SaveData_GetExistFlag(wk->sv) == TRUE ){
        SetReportMsg( wk, msg_common_report_02 );
        *seq = REPORT_SEQ_OVERWRITE_YESNO_SET;
*/
      // セーブ
      }else{
        BmpWinFrame_Clear( wk->local->util.win, WINDOW_TRANS_ON_V );
        *seq = REPORT_SEQ_SAVE_SIZE_CHECK;
      }
      break;

    case 1: // いいえ
      *seq = REPORT_SEQ_SAVE_CANCEL;
      break;
    }
    break;

/*
  case REPORT_SEQ_OVERWRITE_YESNO_SET:    // 上書きしますか？【はい・いいえ】セット
    if( MainReportMsg( wk ) == FALSE ){
      SetReportYesNo( wk );
      *seq = REPORT_SEQ_OVERWRITE_YESNO_WAIT;
    }
    break;

  case REPORT_SEQ_OVERWRITE_YESNO_WAIT:   // 上書きしますか？【はい・いいえ】待ち
    switch( MainReportYesNo(wk) ){
    case 0: // はい
      BmpWinFrame_Clear( wk->local->win, WINDOW_TRANS_ON_V );
      *seq = REPORT_SEQ_SAVE_SIZE_CHECK;
      break;

    case 1: // いいえ
      *seq = REPORT_SEQ_SAVE_CANCEL;
      break;
    }
    break;
*/

  case REPORT_SEQ_SAVE_SIZE_CHECK:        // セーブサイズ取得
    PLAYTIME_SetSaveTime( SaveData_GetPlayTime(wk->sv) );
    FIELD_SUBSCREEN_SetReportSize( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork) );
    if( FIELD_SUBSCREEN_CheckReportType(FIELDMAP_GetFieldSubscreenWork(wk->fieldWork)) == TRUE ){
        MsgPrint( wk, msg_common_report_09 );
//      SetReportMsg( wk, msg_common_report_09 );
    }else{
        MsgPrint( wk, msg_common_report_03 );
//      SetReportMsg( wk, msg_common_report_03 );
    }
    *seq = REPORT_SEQ_PALYER_ANM_CHG;
    break;

  case REPORT_SEQ_PALYER_ANM_CHG:         // 主人公アニメ変更
//    if( MainReportMsg( wk ) == FALSE ){
    if( CheckMsgPrint( wk ) == TRUE ){
      SetReportPlayerAnime( wk );
      *seq = REPORT_SEQ_SAVE_INIT;
    }
    break;

  case REPORT_SEQ_SAVE_INIT:              // セーブ初期設定
    if( CheckPlayerAnime( wk ) == FALSE ){
      break;
    }
    wk->local->timeIcon = TIMEICON_Create(
                            GFUser_VIntr_GetTCBSYS(),
                            wk->local->util.win, 15, TIMEICON_DEFAULT_WAIT, wk->heapID );
    FIELD_SUBSCREEN_SetReportStart( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork) );
    SetReportBgAnime( wk );
    GAMEDATA_SaveAsyncStart( GAMESYSTEM_GetGameData(wk->gsys) );
    *seq = REPORT_SEQ_SAVE_MAIN;
    break;

  case REPORT_SEQ_SAVE_MAIN:              // セーブ実行
    switch( GAMEDATA_SaveAsyncMain(GAMESYSTEM_GetGameData(wk->gsys)) ){
    case SAVE_RESULT_CONTINUE:
    case SAVE_RESULT_LAST:
      break;
      
    case SAVE_RESULT_OK:
/*
      FIELD_SUBSCREEN_SetReportEnd( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork) );
      ResetReportBgAnime( wk );
      ResetReportPlayerAnime( wk );
      {
        WORDSET * wset;
        STRBUF * str;
        wset = WORDSET_Create( wk->heapID );
        str  = GFL_MSG_CreateString( wk->local->msgData, msg_common_report_04 );
        WORDSET_RegisterPlayerName( wset, 0, GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(wk->gsys)) );
        WORDSET_ExpandStr( wset, wk->local->strBuff, str );
        GFL_STR_DeleteBuffer( str );
        WORDSET_Delete( wset );
      }
      SetReportMsgBuff( wk );
      TILEICON_Exit( wk->local->timeIcon );
      *seq = REPORT_SEQ_RESULT_OK_WAIT;
*/
      *seq = REPORT_SEQ_RESULT_OK_BAR_WAIT;
      break;

    case SAVE_RESULT_NG:
      FIELD_SUBSCREEN_SetReportBreak( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork) );
      ResetReportBgAnime( wk );
      ResetReportPlayerAnime( wk );
      SetReportMsg( wk, msg_common_report_06 );
      TILEICON_Exit( wk->local->timeIcon );
      *seq = REPORT_SEQ_RESULT_NG_WAIT;
      break;
    }
    break;

  case REPORT_SEQ_RESULT_OK_BAR_WAIT:     // セーブ成功バー待ち
    if( FIELD_SUBSCREEN_SetReportEnd( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork) ) == TRUE ){
      PMSND_PlaySE( SEQ_SE_SAVE );
      ResetReportBgAnime( wk );
      ResetReportPlayerAnime( wk );
      {
        WORDSET * wset;
        STRBUF * str;
        wset = WORDSET_Create( wk->heapID );
        str  = GFL_MSG_CreateString( wk->local->msgData, msg_common_report_04 );
        WORDSET_RegisterPlayerName( wset, 0, GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(wk->gsys)) );
        WORDSET_ExpandStr( wset, wk->local->strBuff, str );
        GFL_STR_DeleteBuffer( str );
        WORDSET_Delete( wset );
      }
      SetReportMsgBuff( wk );
      TILEICON_Exit( wk->local->timeIcon );
      *seq = REPORT_SEQ_RESULT_OK_WAIT;
    }
    break;

  case REPORT_SEQ_RESULT_OK_WAIT:   // セーブ成功メッセージ待ち
    if( CheckPlayerAnime( wk ) == FALSE ){
      break;
    }
    if( MainReportMsg( wk ) == FALSE ){
//      PMSND_PlaySE( SEQ_SE_SAVE );
      *seq = REPORT_SEQ_RESULT_SE_WAIT;
    }
    break;

  case REPORT_SEQ_RESULT_SE_WAIT:   // ＳＥ待ち
    if( PMSND_CheckPlayingSEIdx(SEQ_SE_SAVE) == FALSE ){
      *seq = REPORT_SEQ_END_TRG_WAIT;
    }
    break;

  case REPORT_SEQ_RESULT_NG_WAIT:   // セーブ失敗メッセージ待ち
    if( CheckPlayerAnime( wk ) == FALSE ){
      break;
    }
    if( MainReportMsg( wk ) == FALSE ){
      *seq = REPORT_SEQ_RESULT_NG_TRG_WAIT;
    }
    break;

  case REPORT_SEQ_RESULT_NG_TRG_WAIT:   // セーブ失敗メッセージ後のキー待ち
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
      *seq = REPORT_SEQ_SAVE_COMP;
    }
    break;

  case REPORT_SEQ_END_TRG_WAIT:     // セーブ後のキー待ち
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
//      *seq = REPORT_SEQ_SUBDISP_RESET;
      *seq = REPORT_SEQ_SAVE_COMP;
    }
    // 自動終了
    if( wk->local->wait >= END_AUTO_TIME ){
      *seq = REPORT_SEQ_SAVE_COMP;
      break;
    }
    wk->local->wait++;
    break;

/*
  case REPORT_SEQ_SUBDISP_RESET:          // サブ画面リセット
    ExitReportYesNo( wk );
    ExitReportBmpWin( wk );
    APP_KEYCURSOR_Delete( wk->local->kcwk );
    GFL_TCBL_Exit( wk->local->tcbl );
    GFL_STR_DeleteBuffer( wk->local->strBuff );
    GFL_MSG_Delete( wk->local->msgData );
    GFL_HEAP_FreeMemory( wk->local );
//    FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork), FIELD_SUBSCREEN_TOPMENU );
    FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork), FIELD_SUBSCREEN_NOGEAR );
    *seq = REPORT_SEQ_END;
    break;
*/
  case REPORT_SEQ_SAVE_CANCEL:            // セーブキャンセル
    ReleaseLocalWork( wk );
    FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork), FIELD_SUBSCREEN_TOPMENU );
    *seq = REPORT_SEQ_SAVE_CANCEL_END;
    break;

  case REPORT_SEQ_SAVE_CANCEL_END:        // 終了（セーブキャンセル）
    return REPORTEVENT_RET_CANCEL;

  case REPORT_SEQ_SAVE_COMP:              // セーブ実行
    ReleaseLocalWork( wk );
    FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(wk->fieldWork), FIELD_SUBSCREEN_NORMAL );
    *seq = REPORT_SEQ_SAVE_COMP_END;
    break;

  case REPORT_SEQ_SAVE_COMP_END:        // 終了（セーブ実行）
    FIELD_SUBSCREEN_MainDispBrightnessOff( wk->heapID );
    return REPORTEVENT_RET_SAVE;

/*
  case REPORT_SEQ_END:                    // 終了
    return REPORTEVENT_RET_NONE;
*/
  }

  return REPORTEVENT_RET_NONE;
}

//--------------------------------------------------------------------------------------------
/**
 * ワーク削除
 *
 * @param   wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ReleaseLocalWork( FMENU_REPORT_EVENT_WORK * wk )
{
  ExitReportYesNo( wk );
  ExitReportBmpWin( wk );
  PRINTSYS_QUE_Delete( wk->local->que );
  APP_KEYCURSOR_Delete( wk->local->kcwk );
  GFL_TCBL_Exit( wk->local->tcbl );
  GFL_STR_DeleteBuffer( wk->local->strBuff );
  GFL_MSG_Delete( wk->local->msgData );
  GFL_HEAP_FreeMemory( wk->local );
}

//--------------------------------------------------------------------------------------------
/**
 * 下画面会話ウィンドウ作成
 *
 * @param   wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void InitReportBmpWin( FMENU_REPORT_EVENT_WORK * wk )
{
  wk->local->util.win = GFL_BMPWIN_Create(
                          GFL_BG_FRAME2_S, MSG_PX, MSG_PY,
                          MSG_SX, MSG_SY, BGPAL_MSGFNT, GFL_BMP_CHRAREA_GET_B );

  BmpWinFrame_GraphicSet(
    GFL_BG_FRAME2_S, WINFRM_CGXNUM, BGPAL_WINFRM, MENU_TYPE_SYSTEM, wk->heapID );

  GFL_ARC_UTIL_TransVramPalette(
    ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, BGPAL_MSGFNT*0x20, 0x20, wk->heapID );
}

//--------------------------------------------------------------------------------------------
/**
 * 下画面会話ウィンドウ削除
 *
 * @param   wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ExitReportBmpWin( FMENU_REPORT_EVENT_WORK * wk )
{
  GFL_BMPWIN_Delete( wk->local->util.win );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ描画
 *
 * @param   wk        ワーク
 * @param   strIdx    文字列インデックス
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void SetReportMsg( FMENU_REPORT_EVENT_WORK * wk, u32 strIdx )
{
  GFL_MSG_GetString( wk->local->msgData, strIdx, wk->local->strBuff );
  SetReportMsgBuff( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ描画（バッファ）
 *
 * @param   wk      ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void SetReportMsgBuff( FMENU_REPORT_EVENT_WORK * wk )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->local->util.win), 15 );
  BmpWinFrame_Write( wk->local->util.win, WINDOW_TRANS_OFF, WINFRM_CGXNUM, BGPAL_WINFRM );

  wk->local->stream = PRINTSYS_PrintStream(
                  wk->local->util.win,
                  0, 0, wk->local->strBuff,
                  FLDMSGBG_GetFontHandle(wk->msgBG),
                  MSGSPEED_GetWait(),
                  wk->local->tcbl,
                  10,   // tcbl pri
                  wk->heapID,
                  15 ); // clear color

  wk->local->stream_clear_flg = FALSE;

  GFL_BMPWIN_MakeTransWindow_VBlank( wk->local->util.win );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ表示メイン
 *
 * @param   wk      ワーク
 *
 * @retval  "TRUE = 表示中"
 * @retval  "FALSE = 終了"
 */
//--------------------------------------------------------------------------------------------
static BOOL MainReportMsg( FMENU_REPORT_EVENT_WORK * wk )
{
  GFL_TCBL_Main( wk->local->tcbl );
  GFL_TCBL_Main( wk->local->tcbl );

  APP_KEYCURSOR_Main( wk->local->kcwk, wk->local->stream, wk->local->util.win );
  APP_KEYCURSOR_Main( wk->local->kcwk, wk->local->stream, wk->local->util.win );

  switch( PRINTSYS_PrintStreamGetState(wk->local->stream) ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    if( GFL_UI_TP_GetTrg() == TRUE || (GFL_UI_KEY_GetCont() & (PAD_BUTTON_A|PAD_BUTTON_B)) ){
      PRINTSYS_PrintStreamShortWait( wk->local->stream, 0 );
    }
    wk->local->stream_clear_flg = FALSE;
    break;

  case PRINTSTREAM_STATE_PAUSE: //一時停止中
    if( wk->local->stream_clear_flg == FALSE ){
      if( GFL_UI_TP_GetTrg() == TRUE || (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B)) ){
        PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
        PRINTSYS_PrintStreamReleasePause( wk->local->stream );
        wk->local->stream_clear_flg = TRUE;
      }
    }
    break;

  case PRINTSTREAM_STATE_DONE: //終了
    PRINTSYS_PrintStreamDelete( wk->local->stream );
    wk->local->stream_clear_flg = FALSE;
    return FALSE;
  }

  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ一括描画
 *
 * @param   wk      ワーク
 * @param   strIdx  文字列インデックス
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void MsgPrint( FMENU_REPORT_EVENT_WORK * wk, u32 strIdx )
{
  STRBUF * str;

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->local->util.win), 15 );
  BmpWinFrame_Write( wk->local->util.win, WINDOW_TRANS_OFF, WINFRM_CGXNUM, BGPAL_WINFRM );

  str = GFL_MSG_CreateString( wk->local->msgData, strIdx );
  PRINT_UTIL_Print( &wk->local->util, wk->local->que, 0, 0, str, FLDMSGBG_GetFontHandle(wk->msgBG) );
  GFL_STR_DeleteBuffer( str );

  GFL_BMPWIN_MakeTransWindow_VBlank( wk->local->util.win );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ一括描画待ち
 *
 * @param   wk      ワーク
 *
 * @retval  "TRUE = 描画終了"
 * @retval  "FALSE = 描画中"
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckMsgPrint( FMENU_REPORT_EVENT_WORK * wk )
{
  PRINTSYS_QUE_Main( wk->local->que );
  PRINT_UTIL_Trans( &wk->local->util, wk->local->que );
  return PRINTSYS_QUE_IsFinished( wk->local->que );
}


//--------------------------------------------------------------------------------------------
/**
 * 「はい・いいえ」初期化
 *
 * @param   wk      ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void InitReportYesNo( FMENU_REPORT_EVENT_WORK * wk )
{
  GFL_MSGDATA * mman = GFL_MSG_Create(
                          GFL_MSG_LOAD_NORMAL,
                          ARCID_MESSAGE, NARC_message_report_dat, wk->heapID );

  wk->local->ynList[0].str      = GFL_MSG_CreateString( mman, REPORT_STR_14 );
  wk->local->ynList[0].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  wk->local->ynList[0].type     = APP_TASKMENU_WIN_TYPE_NORMAL;
  wk->local->ynList[1].str      = GFL_MSG_CreateString( mman, REPORT_STR_15 );
  wk->local->ynList[1].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
  wk->local->ynList[1].type     = APP_TASKMENU_WIN_TYPE_NORMAL;

  GFL_MSG_Delete( mman );

  wk->local->ynRes = APP_TASKMENU_RES_Create(
                GFL_BG_FRAME2_S, BGPAL_YESNO,
                FLDMSGBG_GetFontHandle( wk->msgBG ),
                FLDMSGBG_GetPrintQue( wk->msgBG ),
                wk->heapID );
}

//--------------------------------------------------------------------------------------------
/**
 * 「はい・いいえ」削除
 *
 * @param   wk      ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void ExitReportYesNo( FMENU_REPORT_EVENT_WORK * wk )
{
  APP_TASKMENU_RES_Delete( wk->local->ynRes );

  GFL_STR_DeleteBuffer( wk->local->ynList[1].str );
  GFL_STR_DeleteBuffer( wk->local->ynList[0].str );
}

//--------------------------------------------------------------------------------------------
/**
 * 「はい・いいえ」開始
 *
 * @param   wk      ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void SetReportYesNo( FMENU_REPORT_EVENT_WORK * wk )
{
  APP_TASKMENU_INITWORK mwk;

  mwk.heapId   = wk->heapID;
  mwk.itemNum  = 2;
  mwk.itemWork = wk->local->ynList;
  mwk.posType  = ATPT_RIGHT_DOWN;
  mwk.charPosX = YESNO_PX;
  mwk.charPosY = YESNO_PY;
  mwk.w        = APP_TASKMENU_PLATE_WIDTH_YN_WIN;
  mwk.h        = APP_TASKMENU_PLATE_HEIGHT_YN_WIN;

  wk->local->ynWork = APP_TASKMENU_OpenMenuEx( &mwk, wk->local->ynRes );
}

//--------------------------------------------------------------------------------------------
/**
 * 「はい・いいえ」メイン
 *
 * @param   wk      ワーク
 *
 * @retval  "0 = はい"
 * @retval  "1 = いいえ"
 * @retval  "-1 = それ以外"
 */
//--------------------------------------------------------------------------------------------
static s32 MainReportYesNo( FMENU_REPORT_EVENT_WORK * wk )
{
  APP_TASKMENU_UpdateMenu( wk->local->ynWork );
  if( APP_TASKMENU_IsFinish( wk->local->ynWork ) == TRUE ){
    APP_TASKMENU_CloseMenu( wk->local->ynWork );
    if( APP_TASKMENU_GetCursorPos( wk->local->ynWork ) == 0 ){
      return 0;
    }else{
      return 1;
    }
  }

  return -1;
}


// 自機レポートに
static void SetReportPlayerAnime( FMENU_REPORT_EVENT_WORK * wk )
{
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( wk->fieldWork );
  if( FIELD_PLAYER_CheckChangeEventDrawForm(fld_player) == TRUE ){
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
    FIELD_PLAYER_SetRequest( fld_player, FIELD_PLAYER_REQBIT_REPORT );
    FIELD_PLAYER_UpdateRequest( fld_player );
    //ポーズを解除しアニメするように
    MMDL_OffMoveBitMoveProcPause( mmdl );
    MMDL_OffStatusBit( mmdl, MMDL_STABIT_PAUSE_ANM );
  }
}

// 自機元に戻す
static void ResetReportPlayerAnime( FMENU_REPORT_EVENT_WORK * wk )
{
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( wk->fieldWork );
  if( FIELD_PLAYER_CheckChangeEventDrawForm(fld_player) == TRUE ){
    FIELD_PLAYER_SetRequest( fld_player, FIELD_PLAYER_REQBIT_MOVE_FORM_TO_DRAW_FORM );
    FIELD_PLAYER_UpdateRequest( fld_player );
  }
}

// 自機アニメチェック
static BOOL CheckPlayerAnime( FMENU_REPORT_EVENT_WORK * wk )
{
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( wk->fieldWork );
  if( FIELD_PLAYER_CheckChangeEventDrawForm(fld_player) == TRUE ){
    return FIELD_PLAYER_CheckAnimeEnd( fld_player );
  }
  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *  @brief  レポートBGアニメーション    ON
 */
//-----------------------------------------------------------------------------
static void SetReportBgAnime( FMENU_REPORT_EVENT_WORK * work )
{
  work->local->bgAnime = FIELD_SAVEANIME_Create( work->heapID, work->fieldWork );
  FIELD_SAVEANIME_Start( work->local->bgAnime );
}

//----------------------------------------------------------------------------
/** 
 *  @brief  レポートBGアニメーション  OFF
 */
//-----------------------------------------------------------------------------
static void ResetReportBgAnime( FMENU_REPORT_EVENT_WORK * work )
{
  FIELD_SAVEANIME_End( work->local->bgAnime );
  FIELD_SAVEANIME_Delete( work->local->bgAnime );
}

