//============================================================================================
/**
 * @file  mb_bmp.c
 * @brief メールボックス画面 BMP関連
 * @author  Hiroyuki Nakamura
 * @date  09.01.31
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "font/font.naix"
#include "arc/arc_def.h"
#include "arc/message.naix"
#include "gamesystem/msgspeed.h"
#include "msg/msg_mailbox.h"

#include "mb_main.h"
#include "mb_bmp.h"


//============================================================================================
//  定数定義
//============================================================================================

// BMPWIN定義
// メールリスト
#define BMPWIN_MAIL_FRM   ( MBMAIN_BGF_BTN_M )
#define BMPWIN_MAIL_PX    ( 4 )
#define BMPWIN_MAIL_PY    ( 1 )
#define BMPWIN_MAIL_SX    ( 8 )
#define BMPWIN_MAIL_SY    ( 2 )
#define BMPWIN_MAIL_PAL   ( 1 )
#define BMPWIN_MAIL01_CGX ( 1024 - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL02_CGX ( BMPWIN_MAIL01_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL03_CGX ( BMPWIN_MAIL02_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL04_CGX ( BMPWIN_MAIL03_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL05_CGX ( BMPWIN_MAIL04_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL06_CGX ( BMPWIN_MAIL05_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL07_CGX ( BMPWIN_MAIL06_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL08_CGX ( BMPWIN_MAIL07_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL09_CGX ( BMPWIN_MAIL08_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL10_CGX ( BMPWIN_MAIL09_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL11_CGX ( BMPWIN_MAIL10_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL12_CGX ( BMPWIN_MAIL11_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL13_CGX ( BMPWIN_MAIL12_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL14_CGX ( BMPWIN_MAIL13_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL15_CGX ( BMPWIN_MAIL14_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL16_CGX ( BMPWIN_MAIL15_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL17_CGX ( BMPWIN_MAIL16_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL18_CGX ( BMPWIN_MAIL17_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL19_CGX ( BMPWIN_MAIL18_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
#define BMPWIN_MAIL20_CGX ( BMPWIN_MAIL19_CGX - BMPWIN_MAIL_SX * BMPWIN_MAIL_SY )
// 選択メール
#define BMPWIN_SELMAIL_FRM  ( MBMAIN_BGF_STR_M )
#define BMPWIN_SELMAIL_PX ( 4 )
#define BMPWIN_SELMAIL_PY ( 1 )
#define BMPWIN_SELMAIL_SX ( BMPWIN_MAIL_SX )
#define BMPWIN_SELMAIL_SY ( BMPWIN_MAIL_SY )
#define BMPWIN_SELMAIL_PAL  ( BMPWIN_MAIL_PAL )
#define BMPWIN_SELMAIL_CGX  ( 1024 - BMPWIN_SELMAIL_SX * BMPWIN_SELMAIL_SY )
//「やめる」
#define BMPWIN_RET_FRM    ( MBMAIN_BGF_STR_M )
#define BMPWIN_RET_PX   ( 25 )
#define BMPWIN_RET_PY   ( 22 )
#define BMPWIN_RET_SX   ( 6 )
#define BMPWIN_RET_SY   ( 2 )
#define BMPWIN_RET_PAL    ( 15 )
#define BMPWIN_RET_CGX    ( BMPWIN_SELMAIL_CGX - BMPWIN_RET_SX * BMPWIN_RET_SY )
//「とじる」
#define BMPWIN_CLOSE_FRM  ( MBMAIN_BGF_STR_M )
#define BMPWIN_CLOSE_PX   ( 25 )
#define BMPWIN_CLOSE_PY   ( 21 )
#define BMPWIN_CLOSE_SX   ( 6 )
#define BMPWIN_CLOSE_SY   ( 2 )
#define BMPWIN_CLOSE_PAL  ( 15 )
#define BMPWIN_CLOSE_CGX  ( BMPWIN_RET_CGX - BMPWIN_CLOSE_SX * BMPWIN_CLOSE_SY )
// ページ
#define BMPWIN_PAGE_FRM   ( MBMAIN_BGF_STR_M )
#define BMPWIN_PAGE_PX    ( 10 )
#define BMPWIN_PAGE_PY    ( 22 )
#define BMPWIN_PAGE_SX    ( 5 )
#define BMPWIN_PAGE_SY    ( 2 )
#define BMPWIN_PAGE_PAL   ( 15 )
#define BMPWIN_PAGE_CGX   ( BMPWIN_CLOSE_CGX - BMPWIN_PAGE_SX * BMPWIN_PAGE_SY )
// メッセージ
#define BMPWIN_TALK_FRM   ( MBMAIN_BGF_MSG_M )
#define BMPWIN_TALK_PX    ( 2 )
#define BMPWIN_TALK_PY    ( 1 )
#define BMPWIN_TALK_SX    ( 27 )
#define BMPWIN_TALK_SY    ( 4 )
#define BMPWIN_TALK_PAL   ( MBMAIN_MBG_PAL_TALKFNT )
#define BMPWIN_TALK_CGX   ( 1 )

//「メールボックス」
#define BMPWIN_TITLE_FRM  ( MBMAIN_BGF_STR_S )
#define BMPWIN_TITLE_PX   ( 1 )
#define BMPWIN_TITLE_PY   ( 1 )
#define BMPWIN_TITLE_SX   ( 11 )
#define BMPWIN_TITLE_SY   ( 2 )
#define BMPWIN_TITLE_PAL  ( MBMAIN_SBG_PAL_SYSFNT )
#define BMPWIN_TITLE_CGX  ( 1 )
// メールの内容
#define BMPWIN_MAILMSG_FRM  ( MBMAIN_BGF_MAILMES_S )
#define BMPWIN_MAILMSG_PX ( 3 )
#define BMPWIN_MAILMSG_SX ( 26 )
#define BMPWIN_MAILMSG_SY ( 4 )
#define BMPWIN_MAILMSG_PAL  ( MBMAIN_SBG_PAL_MAILMSE )
// メールの内容１
#define BMPWIN_MAILMSG1_PY  ( 3 )
#define BMPWIN_MAILMSG1_CGX ( 1 )
// メールの内容２
#define BMPWIN_MAILMSG2_PY  ( 8 )
#define BMPWIN_MAILMSG2_CGX ( BMPWIN_MAILMSG1_CGX + BMPWIN_MAILMSG_SX * BMPWIN_MAILMSG_SY )
// メールの内容３
#define BMPWIN_MAILMSG3_PY  ( 13 )
#define BMPWIN_MAILMSG3_CGX ( BMPWIN_MAILMSG2_CGX + BMPWIN_MAILMSG_SX * BMPWIN_MAILMSG_SY )
// 作成者名
#define BMPWIN_NAME_FRM   ( MBMAIN_BGF_MAILMES_S )
#define BMPWIN_NAME_PX    ( 21 )
#define BMPWIN_NAME_PY    ( 20 )
#define BMPWIN_NAME_SX    ( 8 )
#define BMPWIN_NAME_SY    ( 2 )
#define BMPWIN_NAME_PAL   ( MBMAIN_SBG_PAL_MAILMSE )
#define BMPWIN_NAME_CGX   ( BMPWIN_MAILMSG3_CGX + BMPWIN_MAILMSG_SX * BMPWIN_MAILMSG_SY )

#define FCOL_TCH_BLN_M01  ( PRINTSYS_LSB_Make( 3, 2, 4 ) )    // フォントカラー：タッチ青抜M01
#define FCOL_TCH_RDN_M01  ( PRINTSYS_LSB_Make( 5, 4, 4 ) )    // フォントカラー：タッチ赤抜M01
#define FCOL_TLK_BKW_M14  ( PRINTSYS_LSB_Make( 1, 2, 15 ) )   // フォントカラー：会話黒白M14
#define FCOL_TCH_BKN_M15  ( PRINTSYS_LSB_Make( 15, 1, 0 ) )   // フォントカラー：タッチ黒抜M15
#define FCOL_SYS_BKN_M15  ( PRINTSYS_LSB_Make( 1, 2, 0 ) )    // フォントカラー：システム黒抜M15
#define FCOL_SYS_BKN_S00  ( PRINTSYS_LSB_Make( 1, 2, 0 ) )    // フォントカラー：システム黒抜S00
#define FCOL_SYS_BKN_S15  ( PRINTSYS_LSB_Make( 1, 2, 0 ) )    // フォントカラー：システム黒抜S15

// メッセージ表示モード
enum {
  STRPRINT_MODE_LEFT = 0,   // 左詰
  STRPRINT_MODE_RIGHT,    // 右詰
  STRPRINT_MODE_CENTER    // 中央
};


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void MailMesPut( MAILBOX_SYS_WORK * syswk, u32 winID, u32 msgID );


//============================================================================================
//  グローバル変数
//============================================================================================

typedef struct {
  u32 frame;
  u8 x,y,w,h;
  u16 pal,cgx;
}BMPWIN_DAT;


// BMPWINデータ
static const BMPWIN_DAT BmpWinData[] =
{
  { // 上画面0キャラ目押さえ
    BMPWIN_MAIL_FRM, 0, 24,
    1, 1, BMPWIN_MAIL_PAL, BMPWIN_MAIL01_CGX
  },
  // 下画面
  { // 下画面0キャラ目押さえ
    BMPWIN_TITLE_FRM, 0, 24,
    1, 1, BMPWIN_MAIL_PAL, BMPWIN_MAIL01_CGX
  },
  { //メールリスト
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL01_CGX
  },
  { 
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL02_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL03_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL04_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL05_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL06_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL07_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL08_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL09_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL10_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL11_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL12_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL13_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL14_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL15_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL16_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL17_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL18_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL19_CGX
  },
  {
    BMPWIN_MAIL_FRM, BMPWIN_MAIL_PX, BMPWIN_MAIL_PY,
    BMPWIN_MAIL_SX, BMPWIN_MAIL_SY, BMPWIN_MAIL_PAL, BMPWIN_MAIL20_CGX
  },

  { // 選択メール
    BMPWIN_SELMAIL_FRM, BMPWIN_SELMAIL_PX, BMPWIN_SELMAIL_PY,
    BMPWIN_SELMAIL_SX, BMPWIN_SELMAIL_SY, BMPWIN_SELMAIL_PAL, BMPWIN_SELMAIL_CGX
  },
  { //「とじる」
    BMPWIN_CLOSE_FRM, BMPWIN_CLOSE_PX, BMPWIN_CLOSE_PY,
    BMPWIN_CLOSE_SX, BMPWIN_CLOSE_SY, BMPWIN_CLOSE_PAL, BMPWIN_CLOSE_CGX
  },
  { // ページ
    BMPWIN_PAGE_FRM, BMPWIN_PAGE_PX, BMPWIN_PAGE_PY,
    BMPWIN_PAGE_SX, BMPWIN_PAGE_SY, BMPWIN_PAGE_PAL, BMPWIN_PAGE_CGX
  },

  { // メッセージ
    BMPWIN_TALK_FRM, BMPWIN_TALK_PX, BMPWIN_TALK_PY,
    BMPWIN_TALK_SX, BMPWIN_TALK_SY, BMPWIN_TALK_PAL, BMPWIN_TALK_CGX
  },

  { //「メールボックス」
    BMPWIN_TITLE_FRM, BMPWIN_TITLE_PX, BMPWIN_TITLE_PY,
    BMPWIN_TITLE_SX, BMPWIN_TITLE_SY, BMPWIN_TITLE_PAL, BMPWIN_TITLE_CGX
  },
  { // メールの内容１
    BMPWIN_MAILMSG_FRM, BMPWIN_MAILMSG_PX, BMPWIN_MAILMSG1_PY,
    BMPWIN_MAILMSG_SX, BMPWIN_MAILMSG_SY, BMPWIN_MAILMSG_PAL, BMPWIN_MAILMSG1_CGX
  },
  { // メールの内容２
    BMPWIN_MAILMSG_FRM, BMPWIN_MAILMSG_PX, BMPWIN_MAILMSG2_PY,
    BMPWIN_MAILMSG_SX, BMPWIN_MAILMSG_SY, BMPWIN_MAILMSG_PAL, BMPWIN_MAILMSG2_CGX
  },
  { // メールの内容３
    BMPWIN_MAILMSG_FRM, BMPWIN_MAILMSG_PX, BMPWIN_MAILMSG3_PY,
    BMPWIN_MAILMSG_SX, BMPWIN_MAILMSG_SY, BMPWIN_MAILMSG_PAL, BMPWIN_MAILMSG3_CGX
  },
  { // 作成者名
    BMPWIN_NAME_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
    BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_NAME_PAL, BMPWIN_NAME_CGX
  },
};



//--------------------------------------------------------------------------------------------
/**
 * ＢＭＰ関連初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @MAILBLOCK_PASOCOMreturn  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_Init( MAILBOX_SYS_WORK * syswk )
{
  u32 i;
  const BMPWIN_DAT *win;

  GFL_BMPWIN_Init( HEAPID_MAILBOX_APP );
  //FontProc_LoadFont( FONT_TOUCH, HEAPID_MAILBOX_APP );
  syswk->font = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
                              GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_MAILBOX_APP );

  // メッセージストリーム処理用タスクシステム生成
  syswk->app->pMsgTcblSys = GFL_TCBL_Init( HEAPID_MAILBOX_APP, HEAPID_MAILBOX_APP, 32 , 32 );
  syswk->app->printQue    = PRINTSYS_QUE_Create( HEAPID_MAILBOX_APP );

  for( i=0; i<MBMAIN_BMPWIN_ID_MAX; i++ ){
//    GF_BGL_BmpWinAddEx( syswk->app->bgl, &syswk->app->win[i], &BmpWinData[i] );
      win = &BmpWinData[i];
      syswk->app->win[i] = GFL_BMPWIN_Create( win->frame, win->x, win->y, win->w, win->h, 
                                              win->pal, GFL_BMP_CHRAREA_GET_B );
      PRINT_UTIL_Setup( &syswk->app->printUtil[i], syswk->app->win[i] );
  }

  OS_Printf("eom code is %x\n",GFL_STR_GetEOMCode());

  GFL_BMPWIN_MakeTransWindow( syswk->app->win[0] );
  GFL_BMPWIN_MakeTransWindow( syswk->app->win[1] );

}

//--------------------------------------------------------------------------------------------
/**
 * ＢＭＰ関連削除
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_Exit( MAILBOX_SYS_WORK * syswk )
{
  u32 i;

  PRINTSYS_QUE_Delete( syswk->app->printQue );
  GFL_TCBL_Exit( syswk->app->pMsgTcblSys );

  for( i=0; i<MBMAIN_BMPWIN_ID_MAX; i++ ){
    GFL_BMPWIN_Delete( syswk->app->win[i] );
  }

  //FontProc_UnloadFont( FONT_TOUCH );
  GFL_FONT_Delete( syswk->font );

  GFL_BMPWIN_Exit();}

//--------------------------------------------------------------------------------------------
/**
 * 文字列表示（バッファ指定）
 *
 * @param win   BMPWIN
 * @param str   文字列バッファ
 * @param x   表示基準Ｘ座標
 * @param y   表示基準Ｙ座標
 * @param fnt   フォント
 * @param col   カラー
 * @param mode  表示モード
 *
 * @return  none
 *
 * @li  mode = STRPRINT_MODE_LEFT : 左詰
 * @li  mode = STRPRINT_MODE_RIGHT  : 右詰
 * @li  mode = STRPRINT_MODE_CENTER : 中央
 */
//--------------------------------------------------------------------------------------------
static void StrPrintCore(
        GFL_BMPWIN * win, STRBUF * str, PRINT_UTIL *printUtil, PRINT_QUE *que,
        u32 x, u32 y, GFL_FONT *font, PRINTSYS_LSB col, u32 mode )
{
  if( mode == STRPRINT_MODE_RIGHT ){
    x -= PRINTSYS_GetStrWidth( str, font, 0 );
  }else if( mode == STRPRINT_MODE_CENTER ){
    x -= ( PRINTSYS_GetStrWidth( str, font, 0 ) / 2 );
  }
//  GF_STR_PrintColor( win, fnt, str, x, y, MSG_NO_PUT, col, NULL );
  PRINT_UTIL_PrintColor( printUtil, que, x,y, str, font, col );
//  GFL_BMPWIN_MakeTransWindow( wk->win[WIN_NAME] );


}

static STRCODE test_string[10];

//--------------------------------------------------------------------------------------------
/**
 * 文字列表示（固定文字列）
 *
 * @param appwk メールボックス画面アプリワーク
 * @param man   メッセージマネージャ
 * @param winID BMPWIN ID
 * @param msgID 文字列ＩＤ
 * @param x   表示基準Ｘ座標
 * @param y   表示基準Ｙ座標
 * @param fnt   フォント
 * @param col   カラー
 * @param mode  表示モード
 *
 * @return  none
 *
 * @li  mode = STRPRINT_MODE_LEFT : 左詰
 * @li  mode = STRPRINT_MODE_RIGHT  : 右詰
 * @li  mode = STRPRINT_MODE_CENTER : 中央
 */
//--------------------------------------------------------------------------------------------
static void StrPrint(
        MAILBOX_APP_WORK * appwk, GFL_MSGDATA * man,
        u32 winID, u32 msgID, u32 x, u32 y, GFL_FONT *font, PRINTSYS_LSB col, u32 mode )
{
  STRBUF * str = GFL_MSG_CreateString( man, msgID );

  StrPrintCore( appwk->win[winID], str, &appwk->printUtil[winID], 
                appwk->printQue, x, y, font, col, mode );

  GFL_STR_DeleteBuffer( str );

}

//--------------------------------------------------------------------------------------------
/**
 * メール選択ボタンにＢＭＰをセット
 *
 * @param syswk メールボックス画面システムワーク
 * @param frmID BGWINFRMのID
 * @param winID BMPWINのID
 * @param mailID  メールデータのID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_MailButtonSet( MAILBOX_SYS_WORK * syswk, u32 frmID, u32 winID, u32 mailID )
{
  GFL_BMPWIN * win;
  MAIL_DATA  * md;
  PRINT_UTIL *util;
  // 貼り付けるための生NCGRをBMPに変換
  GFL_BMP_DATA *backbmp1 =  GFL_BMP_CreateWithData( &syswk->app->btn_back_cgx[0x00],
                                                   1, 1, GFL_BMP_16_COLOR, HEAPID_MAILBOX_APP );
  GFL_BMP_DATA *backbmp2 =  GFL_BMP_CreateWithData( &syswk->app->btn_back_cgx[0x20],
                                                   1, 1, GFL_BMP_16_COLOR, HEAPID_MAILBOX_APP );
  u32 i;
  
  win  = syswk->app->win[winID];
  md   = syswk->app->mail[mailID];
  util = &syswk->app->printUtil[winID];

   GFL_STR_SetStringCode( syswk->app->expbuf, MailData_GetWriterName(md) );

  for( i=0; i<BMPWIN_MAIL_SX; i++ ){
//    GF_BGL_BmpWinPrint( win, &syswk->app->btn_back_cgx[0x00], 0, 0, 8, 8, 8*i, 0, 8, 8 );
//    GF_BGL_BmpWinPrint( win, &syswk->app->btn_back_cgx[0x20], 0, 0, 8, 8, 8*i, 8, 8, 8 );
    GFL_BMP_Print( GFL_BMPWIN_GetBmp(win), backbmp1, 
                    0, 0, 8*i, 0, 8, 8, GF_BMPPRT_NOTNUKI );
    GFL_BMP_Print( GFL_BMPWIN_GetBmp(win), backbmp2, 
                    0, 0, 8*i, 8, 8, 8, GF_BMPPRT_NOTNUKI );
  }

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(win), 4 );
//  GFL_BMP_Print( GFL_BMPWIN_GetBmp(win), backbmp1, 
//                    0, 0, 8*i, 0, 8, 8, GF_BMPPRT_NOTNUKI );
  // メールの色が男女で変わる
  if( MailData_GetWriterSex( md ) == 0 ){
    StrPrintCore( win, syswk->app->expbuf, util, syswk->app->printQue, 0, 0, 
                  syswk->font, FCOL_TCH_BLN_M01, STRPRINT_MODE_LEFT );
  }else{
    StrPrintCore( win, syswk->app->expbuf, util, syswk->app->printQue, 0, 0, 
                  syswk->font, FCOL_TCH_RDN_M01, STRPRINT_MODE_LEFT );
  }

//  GF_BGL_BmpWinCgxOn( win );
  GFL_BMPWIN_TransVramCharacter( win ); // キャラクターのみ転送
  GFL_BMPWIN_MakeScreen( win );  

  // フレームにBMPを貼り付け
  BGWINFRM_BmpWinOn( syswk->app->wfrm, frmID, win );

  // 変換して使用したBMPデータを解放
  GFL_BMP_Delete( backbmp2 );
  GFL_BMP_Delete( backbmp1 );
}

//--------------------------------------------------------------------------------------------
/**
 * デフォルト文字セット
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_DefaultStrSet( MAILBOX_SYS_WORK * syswk )
{
  // タイトル
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(syswk->app->win[MBMAIN_BMPWIN_ID_TITLE]), 0 );
  StrPrint(
    syswk->app, syswk->app->mman,
    MBMAIN_BMPWIN_ID_TITLE, msg_title,
    BMPWIN_TITLE_SX*8/2, 0,
    syswk->font, FCOL_SYS_BKN_S15, STRPRINT_MODE_CENTER );
  GFL_BMPWIN_MakeTransWindow_VBlank( syswk->app->win[MBMAIN_BMPWIN_ID_TITLE] );

  // やめる
/*
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(syswk->app->win[MBMAIN_BMPWIN_ID_RET]), 0 );
  StrPrint(
    syswk->app, syswk->app->mman,
    MBMAIN_BMPWIN_ID_RET, msg_return,
    BMPWIN_RET_SX*8/2, 0,
    syswk->font, FCOL_TCH_BKN_M15, STRPRINT_MODE_CENTER );
  GFL_BMPWIN_MakeTransWindow_VBlank( syswk->app->win[MBMAIN_BMPWIN_ID_RET] );
  // とじる
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(syswk->app->win[MBMAIN_BMPWIN_ID_CLOSE]), 0 );
  StrPrint(
    syswk->app, syswk->app->mman,
    MBMAIN_BMPWIN_ID_CLOSE, msg_close,
    BMPWIN_CLOSE_SX*8/2, 0,
    syswk->font, FCOL_TCH_BKN_M15, STRPRINT_MODE_CENTER );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 「とじる」表示
 *
 * @param appwk メールボックス画面アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_ClosePut( MAILBOX_APP_WORK * appwk )
{
/*
  GFL_BMPWIN_ClearTransWindow_VBlank( appwk->win[MBMAIN_BMPWIN_ID_RET] );
  GFL_BMPWIN_MakeTransWindow_VBlank( appwk->win[MBMAIN_BMPWIN_ID_CLOSE] );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 「やめる」表示
 *
 * @param appwk メールボックス画面アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_ReturnPut( MAILBOX_APP_WORK * appwk )
{
/*  
  GFL_BMPWIN_ClearTransWindow_VBlank( appwk->win[MBMAIN_BMPWIN_ID_CLOSE] );
  GFL_BMPWIN_MakeTransWindow_VBlank( appwk->win[MBMAIN_BMPWIN_ID_RET] );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * ページ表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_PagePut( MAILBOX_SYS_WORK * syswk )
{
  STRBUF * str;
  u16 px, sx;

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(syswk->app->win[MBMAIN_BMPWIN_ID_PAGE]), 0 );

  // スラッシュ
  GFL_MSG_GetString( syswk->app->mman, msg_page02, syswk->app->expbuf );
  StrPrintCore(
    syswk->app->win[MBMAIN_BMPWIN_ID_PAGE], syswk->app->expbuf, &syswk->app->printUtil[MBMAIN_BMPWIN_ID_PAGE],
    syswk->app->printQue, BMPWIN_PAGE_SX*8/2, 0, syswk->font, FCOL_SYS_BKN_M15, STRPRINT_MODE_CENTER );

  sx = PRINTSYS_GetStrWidth(  syswk->app->expbuf, syswk->font, 0 );

  // 現在のページ
  WORDSET_RegisterNumber(
    syswk->app->wset, 0, syswk->sel_page+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );
  str = GFL_MSG_CreateString( syswk->app->mman, msg_page01 );
  WORDSET_ExpandStr( syswk->app->wset, syswk->app->expbuf, str );
  GFL_STR_DeleteBuffer( str );
  px = PRINTSYS_GetStrWidth( syswk->app->expbuf, syswk->font, 0 );
  StrPrintCore( syswk->app->win[MBMAIN_BMPWIN_ID_PAGE], syswk->app->expbuf,
                &syswk->app->printUtil[MBMAIN_BMPWIN_ID_PAGE], syswk->app->printQue,
                BMPWIN_PAGE_SX*8/2-sx/2-px, 0,syswk->font, FCOL_SYS_BKN_M15, STRPRINT_MODE_LEFT );

  // ページ数
  WORDSET_RegisterNumber(
    syswk->app->wset, 0, syswk->app->page_max+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );
  str = GFL_MSG_CreateString( syswk->app->mman, msg_page01 );
  WORDSET_ExpandStr( syswk->app->wset, syswk->app->expbuf, str );
  GFL_STR_DeleteBuffer( str );
  StrPrintCore(
    syswk->app->win[MBMAIN_BMPWIN_ID_PAGE], syswk->app->expbuf, 
    &syswk->app->printUtil[MBMAIN_BMPWIN_ID_PAGE],syswk->app->printQue,
    BMPWIN_PAGE_SX*8/2+sx/2, 0, syswk->font, FCOL_SYS_BKN_M15, STRPRINT_MODE_LEFT );

  GFL_BMPWIN_MakeTransWindow_VBlank( syswk->app->win[MBMAIN_BMPWIN_ID_PAGE] );

}

//--------------------------------------------------------------------------------------------
/**
 * 選択されたメールの差出人を単語登録
 *
 * @param syswk メールボックス画面システムワーク
 * @param bufID 登録番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void SelMailNameWordSet( MAILBOX_SYS_WORK * syswk, u32 bufID )
{
  STRBUF * str;
  MAIL_DATA * md;
  
  md  = syswk->app->mail[ syswk->lst_pos ];

  str = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, HEAPID_MAILBOX_APP );
  {
    int i;
    STRCODE *dat = MailData_GetWriterName(md);
    OS_Printf("mail name = ");
    for(i=0;i<5;i++){
      OS_Printf("%04x ",dat[i]);
    }
    OS_Printf("\n");
  }
  GFL_STR_SetStringCode( str, MailData_GetWriterName(md) );
  WORDSET_RegisterWord( syswk->app->wset, 0, str, MailData_GetWriterSex(md), TRUE, PM_LANG );
  GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------------------------
/**
 * 会話メッセージセット
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void TalkMsgWrite( MAILBOX_SYS_WORK * syswk )
{
  GFL_BMPWIN * win = syswk->app->win[MBMAIN_BMPWIN_ID_TALK];

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(win), 15 );

//  PRINT_UTIL_PrintColor( &syswk->app->printUtil[MBMAIN_BMPWIN_ID_TALK], 
//                         syswk->app->printQue, 0,0, syswk->app->expbuf, 
//                         syswk->font, FCOL_TLK_BKW_M14 );

  syswk->app->printStream = PRINTSYS_PrintStream( win, 0,0, syswk->app->expbuf, syswk->font,
                                             MSGSPEED_GetWait(), syswk->app->pMsgTcblSys, 0,
                                             HEAPID_MAILBOX_APP, 15);
  

//  syswk->app->msgID = GF_STR_PrintColor(
//              win, FONT_TALK, syswk->app->expbuf, 0, 0,
//              CONFIG_GetMsgPrintSpeed( syswk->config ),
//              FCOL_TLK_BKW_M14, NULL );
}

//--------------------------------------------------------------------------------------------
/**
 * メール選択時のメッセージ表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_SelectMsgPut( MAILBOX_SYS_WORK * syswk )
{
  STRBUF * str;

  SelMailNameWordSet( syswk, 0 );

  str = GFL_MSG_CreateString( syswk->app->mman, msg_message01 );
  WORDSET_ExpandStr( syswk->app->wset, syswk->app->expbuf, str );
  GFL_STR_DeleteBuffer( str );

  TalkMsgWrite( syswk );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージウィンドウ表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_TalkWinPut( MAILBOX_SYS_WORK * syswk )
{
  TalkWinFrame_Write( syswk->app->win[MBMAIN_BMPWIN_ID_TALK], WINDOW_TRANS_OFF, 
                      MBMAIN_TALKWIN_CGX_POS, MBMAIN_MBG_PAL_TALKWIN );
//  BmpTalkWinWrite(
//    syswk->app->win[MBMAIN_BMPWIN_ID_TALK],
//    WINDOW_TRANS_OFF, MBMAIN_TALKWIN_CGX_POS, MBMAIN_MBG_PAL_TALKWIN );

  GFL_BMPWIN_MakeTransWindow_VBlank( syswk->app->win[MBMAIN_BMPWIN_ID_TALK] );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージウィンドウ非表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_TalkWinDel( MAILBOX_SYS_WORK * syswk )
{
  TalkWinFrame_Clear( syswk->app->win[MBMAIN_BMPWIN_ID_TALK], WINDOW_TRANS_OFF );
  GFL_BG_LoadScreenV_Req( MBMAIN_BGF_MSG_M );
}

//--------------------------------------------------------------------------------------------
/**
 * 「メールをけす」メッセージ表示
 *
 * @param syswk メールボックス画面システムワーク
 * @param id    メッセージＩＤ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_DeleteMsgPut( MAILBOX_SYS_WORK * syswk, u32 id )
{
  switch( id ){
  case MBBMP_DELMSG_ID_CHECK:   //「ないようは　きえて　しまいますが　よろしい　ですか？」
    GFL_MSG_GetString( syswk->app->mman, msg_message02, syswk->app->expbuf );
    break;
  case MBBMP_DELMSG_ID_ERASE:   //「ないようを　けしました」
    GFL_MSG_GetString( syswk->app->mman, msg_message03, syswk->app->expbuf );
    break;
  case MBBMP_DELMSG_ID_POKE:    //「ポケモンに　もたせますか？」
    GFL_MSG_GetString( syswk->app->mman, msg_message04, syswk->app->expbuf );
    break;
  case MBBMP_DELMSG_ID_BAG:   //「ないようを　けした　メールを バッグに　しまいました」
    GFL_MSG_GetString( syswk->app->mman, msg_message05, syswk->app->expbuf );
    break;
  case MBBMP_DELMSG_ID_DELETE:  //「バッグが　いっぱいです… メールを　すてました」
    GFL_MSG_GetString( syswk->app->mman, msg_message06, syswk->app->expbuf );
    break;
  }
  TalkMsgWrite( syswk );
}


//--------------------------------------------------------------------------------------------
/**
 * メールの内容表示
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBBMP_MailMesPut( MAILBOX_SYS_WORK * syswk )
{
  u32 i;
  PMS_DATA * pms;

  for(i=0;i<3;i++){
    pms = MailData_GetMsgByIndex( syswk->app->mail[syswk->lst_pos], i );
    PMS_DRAW_Print( syswk->app->pms_draw_work, 
                    syswk->app->win[MBMAIN_BMPWIN_ID_MAILMSG1+i], pms, i );
    GFL_BMPWIN_MakeTransWindow_VBlank( syswk->app->win[MBMAIN_BMPWIN_ID_MAILMSG1+i] );
  }

/*
  for( i=0; i<MAILDAT_MSGMAX; i++ ){
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(syswk->app->win[MBMAIN_BMPWIN_ID_MAILMSG1+i]), 0 );
    MailMesPut( syswk, MBMAIN_BMPWIN_ID_MAILMSG1+i, i );
    GFL_BMPWIN_MakeTransWindow_VBlank( syswk->app->win[MBMAIN_BMPWIN_ID_MAILMSG1+i] );
  }
*/

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(syswk->app->win[MBMAIN_BMPWIN_ID_NAME]), 0 );
  GFL_STR_SetStringCode(
    syswk->app->expbuf,
    MailData_GetWriterName(syswk->app->mail[syswk->lst_pos]) );
  StrPrintCore(
    syswk->app->win[MBMAIN_BMPWIN_ID_NAME], syswk->app->expbuf,
    &syswk->app->printUtil[MBMAIN_BMPWIN_ID_NAME],syswk->app->printQue,
    0, 2, syswk->font, FCOL_SYS_BKN_S00, STRPRINT_MODE_LEFT );



  GFL_BMPWIN_MakeTransWindow_VBlank( syswk->app->win[MBMAIN_BMPWIN_ID_NAME] );
}

//--------------------------------------------------------------------------------------------
/**
 * メールメッセージ表示
 *
 * @param syswk メールボックス画面システムワーク
 * @param winID ウィンドウＩＤ
 * @param msgID メッセージＩＤ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void MailMesPut( MAILBOX_SYS_WORK * syswk, u32 winID, u32 msgID )
{
  PMS_DATA * pms;
  STRBUF * str;
  
  pms = MailData_GetMsgByIndex(
      syswk->app->mail[syswk->lst_pos], msgID );

  if( PMSDAT_IsEnabled( pms ) == FALSE ){
    return;
  }

  str = PMSDAT_ToString( pms, HEAPID_MAILBOX_APP );

  StrPrintCore(
    syswk->app->win[winID], str, &syswk->app->printUtil[winID],
    syswk->app->printQue, 0, 0, syswk->font, FCOL_SYS_BKN_S00, STRPRINT_MODE_LEFT );

  GFL_STR_DeleteBuffer( str );
}
