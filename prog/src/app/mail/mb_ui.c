//============================================================================================
/**
 * @file  mb_ui.c
 * @brief メールボックス画面 インターフェース関連
 * @author  Hiroyuki Nakamura
 * @date  09.01.31
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/point_sel.h"
#include "mb_main.h"
#include "mb_obj.h"
#include "mb_ui.h"


//============================================================================================
//  定数定義
//============================================================================================


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos );
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos );


//============================================================================================
//  グローバル変数
//============================================================================================

// メール選択
static const CURSORMOVE_DATA MailSelTbl[] =
{
  {  64,  16, 0, 0, 0,  2, 0, 1, {   0,  31,   8, 119 },},    // 00: メール１
  { 192,  16, 0, 0, 1,  3, 0, 1, {   0,  31, 136, 244 },},    // 01: メール２
  {  64,  48, 0, 0, 0,  4, 2, 3, {  32,  63,   8, 119 },},    // 02: メール３
  { 192,  48, 0, 0, 1,  5, 2, 3, {  32,  63, 136, 244 },},    // 03: メール４
  {  64,  80, 0, 0, 2,  6, 4, 5, {  64,  95,   8, 119 },},    // 04: メール５
  { 192,  80, 0, 0, 3,  7, 4, 5, {  64,  95, 136, 244 },},    // 05: メール６
  {  64, 112, 0, 0, 4,  8, 6, 7, {  96, 127,   8, 119 },},    // 06: メール７
  { 192, 112, 0, 0, 5,  9, 6, 7, {  96, 127, 136, 244 },},    // 07: メール８
  {  64, 144, 0, 0, 6,  8, 8, 9, { 128, 159,   8, 119 },},    // 08: メール９
  { 192, 144, 0, 0, 7,  9, 8, 9, { 128, 159, 136, 244 },},    // 09: メール１０
  {   0,   0, 0, 0, 0,  0, 0, 0, { GFL_UI_TP_HIT_END, 0, 0, 0 },},

//  { 224, 176, 0, 0, CURSORMOVE_RETBIT|9, 10, 10, 10, { 160, 191, 192, 255 }, },   // 10: やめる
};
static const CURSORMOVE_CALLBACK MailSelCallBack = {
  CursorMoveCallBack_On,
  CursorMoveCallBack_Off,
  CursorMoveCallBack_Move,
  CursorMoveCallBack_Touch
};

// メール選択ページ切り替え
static const GFL_UI_TP_HITTBL TouchButtonHitTbl[] =
{
  { 168, 191,   8,  8+24 },   // 0: 左矢印
  { 168, 191,  32,  32+24 },   // 1: 右矢印
  { 168, 191, 232,  255 },   // 10: やめる
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};

//「メールをよむ」
static const GFL_UI_TP_HITTBL MailReadHitTbl[] =
{
  { 160, 191, 192, 255 },   // 0: やめる
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};


//--------------------------------------------------------------------------------------------
/**
 * ページ切り替え矢印タッチチェック
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  結果
 */
//--------------------------------------------------------------------------------------------
u32 MBUI_MailboxTouchButtonCheck( MAILBOX_SYS_WORK * syswk )
{
  if( syswk->app->page_max == 0 ){
    return GFL_UI_TP_HIT_NONE;
  }

  return GFL_UI_TP_HitTrg( TouchButtonHitTbl );
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動初期化
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBUI_CursorMoveInit( MAILBOX_SYS_WORK * syswk )
{
  int pos;

  if( syswk->sel_page > syswk->app->page_max ){
    syswk->sel_page = syswk->app->page_max;
    pos = 0;
  }else{
//    pos = syswk->sel_pos - syswk->sel_page * MBMAIN_MAILLIST_MAX;
    pos = syswk->sel_pos;
  }

  syswk->app->cmwk = CURSORMOVE_Create(
              MailSelTbl,
              &MailSelCallBack,
              syswk,
              TRUE,
              pos,
              HEAPID_MAILBOX_APP );

  MBUI_MailSelCurMove( syswk, pos );
}

//--------------------------------------------------------------------------------------------
/**
 * カーソル移動削除
 *
 * @param syswk メールボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBUI_CursorMoveExit( MAILBOX_SYS_WORK * syswk )
{
  CURSORMOVE_Exit( syswk->app->cmwk );
}

//--------------------------------------------------------------------------------------------
/**
 * コールバック関数：カーソル表示
 *
 * @param work    メールボックス画面システムワーク
 * @param now_pos   現在の位置
 * @param old_pos   前回の位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_On( void * work, int now_pos, int old_pos )
{
}

//--------------------------------------------------------------------------------------------
/**
 * コールバック関数：カーソル非表示
 *
 * @param work    メールボックス画面システムワーク
 * @param now_pos   現在の位置
 * @param old_pos   前回の位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Off( void * work, int now_pos, int old_pos )
{
}

//--------------------------------------------------------------------------------------------
/**
 * コールバック関数：カーソル移動
 *
 * @param work    メールボックス画面システムワーク
 * @param now_pos   現在の位置
 * @param old_pos   前回の位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Move( void * work, int now_pos, int old_pos )
{
  MBUI_MailSelCurMove( work, now_pos );
}

//--------------------------------------------------------------------------------------------
/**
 * コールバック関数：タッチ
 *
 * @param work    メールボックス画面システムワーク
 * @param now_pos   現在の位置
 * @param old_pos   前回の位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveCallBack_Touch( void * work, int now_pos, int old_pos )
{
  MBUI_MailSelCurMove( work, now_pos );
}

//--------------------------------------------------------------------------------------------
/**
 * メール選択のカーソル位置セット
 *
 * @param work    メールボックス画面システムワーク
 * @param pos     位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void MBUI_MailSelCurMove( MAILBOX_SYS_WORK * syswk, int pos )
{
//  const POINTSEL_WORK * pwk = CURSORMOVE_PointerWorkGet( syswk->app->cmwk, pos );
  const CURSORMOVE_DATA *cd = CURSORMOVE_GetMoveData( syswk->app->cmwk, pos );

  MBOBJ_PosSet( syswk->app, MBMAIN_OBJ_CURSOR, cd->px, cd->py );

  if( pos == MBUI_MAILSEL_RETURN ){
    MBOBJ_AnmSet( syswk->app, MBMAIN_OBJ_CURSOR, 4 );
  }else{
    MBOBJ_AnmSet( syswk->app, MBMAIN_OBJ_CURSOR, 5 );
  }
}


//--------------------------------------------------------------------------------------------
/**
 * 「メールをよむ」画面キー・タッチチェック
 *
 * @param none
 *
 * @retval  "TRUE = 終了"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL MBUI_MailReadCheck(void)
{
  if( GFL_UI_TP_HitTrg( MailReadHitTbl ) != GFL_UI_TP_HIT_NONE ){
    return TRUE;
  }

  if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE) ){
    return TRUE;
  }

  return FALSE;
}

