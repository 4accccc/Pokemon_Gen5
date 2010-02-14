//=============================================================================
/**
 * @file  bmp_list.c  
 * @brief ビットマップＢＧ上でのリスト表示システム
 * @author  GAME FREAK inc.
 * @date  tetsu
 *
 * ・2008.09 DPからWBへ移植
 *
 *  09,10,27 WBの仕様にあわせ上下に余白があれば、はみ出して表示させるように nagihashi
 */
//=============================================================================
#include <gflib.h>

#include "include/system/gfl_use.h"
#include "arc/arc_def.h"
#include "system/bmp_winframe.h"
#include "system/bmp_cursor.h"
#include "system/bmp_menulist.h"
#include "sound/pm_sndsys.h"



/********************************************************************/
/*                                                                  */
/*        使用定義                      */
/*                                                                  */
/********************************************************************/
enum {
  LIST_UP = 0,
  LIST_DOWN
};

enum{
  BMPWIN_SHIFT_U = 0,
  BMPWIN_SHIFT_D,
  BMPWIN_SHIFT_L,
  BMPWIN_SHIFT_R,
};

typedef struct {
  u8  f_col:4;    // 表示文字色
  u8  b_col:4;    // 表示背景色
  u8  s_col:4;    // 表示文字影色
  u8  msg_spc:6;    // 文字間隔Ｘ
  u8  dummy:6;    // 未使用
  u8  font:7;     // 文字指定
  u8  sw:1;     // 一時変更スイッチ
}BMPMENULIST_TMP;

///リストワーク構造体
struct _BMPMENULIST_WORK {
  BMPMENULIST_HEADER  hed;  //表示文字データヘッダ
  BMPMENULIST_TMP   tmp;  // 一時変更データ
  BMPCURSOR * cursor;   // BMPカーソルデータ

  u16 lp;     //リスト位置
  u16 cp;     //カーソル位置
  u8  seq;    //処理シーケンス
  u8  scrwork;  //スクロールワーク
  u8  cur_actID;  //
  u8  work;   //移動方向（使ってないようなので）

  HEAPID  heapID;   // メモリ取得モード
  u8  cancel_mode; // キャンセル
  u16 padding; //long padding
};

/********************************************************************/
/*                                                                  */
/*        ローカル関数宣言                  */
/*                                                                  */
/********************************************************************/
static void LocalMsgPrint( BMPMENULIST_WORK * lw, void * msg, u8 x, u8 y );
static void ListScreenPut( BMPMENULIST_WORK * lw, u16 print_p, u16 line, u16 len, BOOL up_write );
static void ListCursorPut( BMPMENULIST_WORK * lw );
static void ListCursorCls( BMPMENULIST_WORK * lw, u16 p );
static u8 ListCursorMoveUpDown( BMPMENULIST_WORK * lw, u8 mode );
static void ListScrollMoveUpDown( BMPMENULIST_WORK * lw, u8 len, u8 mode );
static u8 ListMoveUpDownCheck( BMPMENULIST_WORK * lw, u8 print_f, u8 req_line, u8 mode );
static void CallBackSet( BMPMENULIST_WORK * lw, u8 mode );

static void BmpWinDataShift(
  GFL_BMPWIN *bmpwin, u8 direct, u8 offset, u8 data, u16 size );

//--------------------------------------------------------------------------------------------
/**
 * 設定関数
 *
 * @param bmplist   表示リストヘッダポインタ
 * @param list_p    リスト初期位置
 * @param cursor_p  カーソル初期位置
 * @param mode    メモリ取得モード
 *
 * @return  BMPリストワーク
 *
 * @li  BMPリストワークはsys_AllocMemoryで確保
 */
//--------------------------------------------------------------------------------------------
BMPMENULIST_WORK * BmpMenuList_Set(
  const BMPMENULIST_HEADER * bmplist, u16 list_p, u16 cursor_p, HEAPID heapID )
{
  BMPMENULIST_WORK * lw;
  
  lw = GFL_HEAP_AllocClearMemory( heapID, sizeof(BMPMENULIST_WORK) );

  // 基本設定
  lw->hed       = *bmplist;
  lw->cursor    = BmpCursor_Create( heapID );
  lw->lp        = list_p;
  lw->cp        = cursor_p;
  lw->seq       = 0;
  lw->scrwork   = 0;
  lw->cur_actID = 0xff;
  lw->work      = 0;
  lw->heapID    = heapID;

  // テンポラリ初期化
  lw->tmp.f_col   = lw->hed.f_col;  // 表示文字色
  lw->tmp.b_col   = lw->hed.b_col;  // 表示背景色
  lw->tmp.s_col   = lw->hed.s_col;  // 表示文字影色
  lw->tmp.msg_spc = lw->hed.msg_spc;  // 文字間隔Ｘ
  lw->tmp.font    = lw->hed.font;   // 文字指定
  lw->tmp.sw      = 0;        // 一時変更スイッチ

  // 項目数が表示指定ライン数より少ない場合は、表示指定ライン数を項目数にあわせる
  if( lw->hed.count < lw->hed.line ){
    lw->hed.line = lw->hed.count;
  }

  //カーソルの位置がラベルでなくなるまで移動
  while(lw->hed.list[lw->lp + lw->cp].param == BMPMENULIST_LABEL)
  { 
    lw->cp++;
  }


  // BMPカーソル色変更
//  BMPCURSOR_ChangeColor( lw->cursor,
//    GF_PRINTCOLOR_MAKE(lw->hed.f_col,lw->hed.s_col,lw->hed.b_col) );

  PRINT_UTIL_Setup( lw->hed.print_util, lw->hed.win );
  
  // リストキャラデータ初期化
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(lw->hed.win), lw->hed.b_col );
  ListScreenPut( lw, lw->lp, 0, lw->hed.line, TRUE );   // 全体リスト描画
  ListCursorPut( lw );                // カーソル描画

  CallBackSet( lw, 1 );

//  GF_BGL_BmpWinOn( bmplist->win );
  return lw;
}

//--------------------------------------------------------------------------------------------
/**
 * メイン関数
 *
 * @param lw    BMPリストワーク
 *
 * @retval  "param = 選択パラメータ"
 * @retval  "BMPMENULIST_NULL = 選択中"
 * @retval  "BMPMENULIST_CANCEL = キャンセル(Ｂボタン)"
 */
//--------------------------------------------------------------------------------------------
u32 BmpMenuList_Main( BMPMENULIST_WORK * lw )
{
  u16 skip_u, skip_d;
  int trg = GFL_UI_KEY_GetTrg();
  int repeat = GFL_UI_KEY_GetRepeat();

  lw->work = BMPMENULIST_MOVE_NONE;

  if( trg & PAD_BUTTON_DECIDE ){
    PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
    return lw->hed.list[lw->lp + lw->cp].param;
  }
  if( lw->cancel_mode == BMPMENULIST_CANCELMODE_USE ){
    if( trg & PAD_BUTTON_CANCEL ){
      PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
      return BMPMENULIST_CANCEL;
    }
  }
  if( repeat & PAD_KEY_UP ){
    if( ListMoveUpDownCheck( lw, 1, 1, LIST_UP ) == 0 ){
      PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
      lw->work = BMPMENULIST_MOVE_UP;
    }
    return BMPMENULIST_NULL;
  }
  if( repeat & PAD_KEY_DOWN ){
    if( ListMoveUpDownCheck( lw, 1, 1, LIST_DOWN ) == 0 ){
      PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
      lw->work = BMPMENULIST_MOVE_DOWN;
    }
    return BMPMENULIST_NULL;
  }
  //スキップフラグ判定
  switch( lw->hed.page_skip ){
  default:
  case BMPMENULIST_NO_SKIP:
    skip_u = 0;
    skip_d = 0;
    break;
  case BMPMENULIST_LRKEY_SKIP:
    skip_u = ( repeat & PAD_KEY_LEFT );
    skip_d = ( repeat & PAD_KEY_RIGHT );
    break;
  case BMPMENULIST_LRBTN_SKIP:
    skip_u = ( repeat & PAD_BUTTON_L );
    skip_d = ( repeat & PAD_BUTTON_R );
    break;
  }
  if( skip_u ){
    if( ListMoveUpDownCheck( lw, 1, (u8)lw->hed.line, LIST_UP ) == 0 ){
      PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
      lw->work = BMPMENULIST_MOVE_UP_SKIP;
    }
    return BMPMENULIST_NULL;
  }
  if( skip_d ){
    if( ListMoveUpDownCheck( lw, 1, (u8)lw->hed.line, LIST_DOWN ) == 0 ){
      PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
      lw->work = BMPMENULIST_MOVE_DOWN_SKIP;
    }
    return BMPMENULIST_NULL;
  }

  return BMPMENULIST_NULL;
}


//--------------------------------------------------------------------------------------------
/**
 * 終了関数
 *
 * @param lw      BMPリストワーク
 * @param list_bak  リスト位置バックアップワークポインタ
 * @param cursor_bak  カーソル位置バックアップワークポインタ
 *
 * @return  none
 *
 * @li  lwをsys_FreeMemoryで開放
 * @li  バックアップワークは、NULL指定で保存しない
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_Exit(
  BMPMENULIST_WORK * lw, u16 * list_bak, u16 * cursor_bak )
{
  if( list_bak != NULL ){
    *list_bak = lw->lp;
  }
  if( cursor_bak != NULL ){
    *cursor_bak = lw->cp;
  }
/*
  if( lw->cur_actID != 0xff ){
    CursorListActorDel( lw->cur_actID, lw->lh.c_disp_f - 2 );
  }
*/
  BmpCursor_Delete( lw->cursor );
  GFL_HEAP_FreeMemory( lw );
}

//--------------------------------------------------------------------------------------------
/**
 * リスト再描画
 * @param lw    BMPリストワーク
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_Rewrite( BMPMENULIST_WORK * lw )
{
  // リストキャラデータ初期化
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(lw->hed.win), lw->hed.b_col );
  ListScreenPut( lw, lw->lp, 0, lw->hed.line, TRUE ); // 全体リスト描画
  ListCursorPut( lw );              // カーソル描画
//  GF_BGL_BmpWinOn( lw->hed.win );
}

//--------------------------------------------------------------------------------------------
/**
 * 文字色変更関数
 * @param lw      BMPリストワーク
 * @param f_col   文字色
 * @param b_col   背景色
 * @param s_col   影色
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_ColorControl(
  BMPMENULIST_WORK * lw, u8 f_col, u8 b_col, u8 s_col )
{
  lw->hed.f_col = f_col;  //文字色ナンバー
  lw->hed.b_col = b_col;  //背景色ナンバー
  lw->hed.s_col = s_col;  //影色　ナンバー
}

//--------------------------------------------------------------------------------------------
/**
 * 表示位置変更関数
 * @param lw      BMPリストワーク
 * @param x     表示Ｘ座標(キャラ単位)
 * @param y     表示Ｙ座標(キャラ単位)
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_MoveControl( BMPMENULIST_WORK * lw, u8 x, u8 y )
{
//  GF_BGL_BmpWinSet_PosX( lw->hed.win, x );
//  GF_BGL_BmpWinSet_PosY( lw->hed.win, y );
}

//--------------------------------------------------------------------------------------------
/**
 * 外部コントロール関数(描画・コールバック指定)
 *
 * @param lw      BMPリストワーク
 * @param lh      表示リストヘッダポインタ
 * @param list_p    リスト初期位置
 * @param cursor_p  カーソル初期位置
 * @param print   描画フラグ
 * @param direct    指定方向(キー定義)
 * @param list_bak  リスト位置バックアップワークポインタ
 * @param cursor_bak  カーソル位置バックアップワークポインタ
 *
 * @return  BMPMENULIST_NULL  
 *
 * リスト関数が動作していないとき、外部からの指定により
 * カーソル位置の変動をチェックし、指定バックアップワークに保存
 */
//--------------------------------------------------------------------------------------------
u32 BmpMenuList_MainOutControlEx(
  BMPMENULIST_WORK * lw, BMPMENULIST_HEADER * bmplist,
  u16 list_p, u16 cursor_p, u16 print, u16 direct,
  u16 * list_bak, u16 * cursor_bak )
{
    if( bmplist ){
    lw->hed = *bmplist;
  }
  lw->lp    = list_p;
  lw->cp    = cursor_p;
  lw->seq   = 0;
  lw->scrwork = 0;

  if( direct == PAD_KEY_UP ){
    ListMoveUpDownCheck( lw, print, 1, LIST_UP );
  }else if( direct == PAD_KEY_DOWN ){
    ListMoveUpDownCheck( lw, print, 1, LIST_DOWN );
  }
  if( list_bak != NULL ){
    *list_bak = lw->lp;
  }
  if( cursor_bak != NULL ){
    *cursor_bak = lw->cp;
  }

  return BMPMENULIST_NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * 外部コントロール関数(カーソル移動判定)
 *
 * @param lw      BMPリストワーク
 * @param bmplist   表示リストヘッダポインタ
 * @param list_p    リスト初期位置
 * @param cursor_p  カーソル初期位置
 * @param direct    指定方向(キー定義)
 * @param list_bak  リスト位置バックアップワークポインタ
 * @param cursor_bak  カーソル位置バックアップワークポインタ
 *
 * @return  BMPMENULIST_NULL  
 *
 * リスト関数が動作していないとき、外部からの指定により
 * カーソル位置の変動をチェックし、指定バックアップワークに保存
 */
//--------------------------------------------------------------------------------------------
u32 BmpMenuList_MainOutControl(
  BMPMENULIST_WORK * lw, BMPMENULIST_HEADER * bmplist,
  u16 list_p, u16 cursor_p, u16 direct, u16 * list_bak, u16 * cursor_bak )
{
  return BmpMenuList_MainOutControlEx(
    lw,bmplist,list_p,cursor_p,0,direct,list_bak,cursor_bak);
/*
    if(bmplist){
        lw->hed   = *bmplist;
    }
  lw->lp    = list_p;
  lw->cp    = cursor_p;
  lw->seq   = 0;
  lw->scrwork = 0;

  if( direct == PAD_KEY_UP ){
    ListMoveUpDownCheck( lw, 0, 1, LIST_UP );
  }else if( direct == PAD_KEY_DOWN ){
    ListMoveUpDownCheck( lw, 0, 1, LIST_DOWN );
  }
  if( list_bak != NULL ){
    *list_bak = lw->lp;
  }
  if( cursor_bak != NULL ){
    *cursor_bak = lw->cp;
  }

  return BMPMENULIST_NULL;
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 文字色一時変更
 *
 * @param lw    BMPリストワーク
 * @param f_col 文字色
 * @param b_col 背景色
 * @param s_col 影色
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_TmpColorChange( BMPMENULIST_WORK * lw, u8 f_col, u8 b_col, u8 s_col )
{
  lw->tmp.f_col = f_col;    // 文字色ナンバー
  lw->tmp.b_col = b_col;    // 背景色ナンバー
  lw->tmp.s_col = s_col;    // 影色　ナンバー
  lw->tmp.sw = 1; 
}

//--------------------------------------------------------------------------------------------
/**
 * BMPリストの全体位置を取得
 *
 * @param lw      BMPリストワーク
 * @param pos_bak   全体リスト位置バックアップワークポインタ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_DirectPosGet( BMPMENULIST_WORK * lw, u16 * pos_bak )
{
  *pos_bak = (u16)(lw->lp + lw->cp);
}

//--------------------------------------------------------------------------------------------
/**
 * BMPリストのリスト位置、カーソル位置を取得
 *
 * @param lw      BMPリストワーク
 * @param list_bak  リスト位置バックアップワークポインタ
 * @param cursor_bak  カーソル位置バックアップワークポインタ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_PosGet( BMPMENULIST_WORK * lw, u16 * list_bak, u16 * cursor_bak )
{
  if( list_bak != NULL ){
    *list_bak = lw->lp;
  }
  if( cursor_bak != NULL ){
    *cursor_bak = lw->cp;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * BMPリストのリスト位置、カーソル位置を設定
 *
 * @param lw      BMPリストワーク
 * @param list_bak  リスト位置バックアップワークポインタ
 * @param cursor_bak  カーソル位置バックアップワークポインタ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_PosSet( BMPMENULIST_WORK * lw, u16 list, u16 cursor )
{
  lw->lp  = list;
  lw->cp  = cursor;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPリストのカーソルY座標を取得
 *
 * @param lw    BMPリストワーク
 *
 * @return  カーソルY座標
 */
//--------------------------------------------------------------------------------------------
u16 BmpMenuList_CursorYGet( BMPMENULIST_WORK * lw )
{
#if 0
  u8  yblk = FontHeaderGet( lw->hed.font, FONT_HEADER_SIZE_Y ) + lw->hed.line_spc;
  return  (u16)((lw->cp * yblk) + lw->hed.line_y);
#else
  u8 yblk = lw->hed.font_size_y + lw->hed.line_spc;
  return (u16)((lw->cp * yblk) + lw->hed.line_y);
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * BMPリストのカーソル移動方向取得
 *
 * @param lw    BMPリストワーク
 *
 * @return  カーソル移動方向取得
 */
//--------------------------------------------------------------------------------------------
u8 BmpMenuList_MoveSiteGet( BMPMENULIST_WORK * lw )
{
  return lw->work;
}

//--------------------------------------------------------------------------------------------
/**
 * 指定位置のパラメータ取得
 *
 * @param lw    BMPリストワーク
 * @param pos   位置
 *
 * @return  パラメータ
 */
//--------------------------------------------------------------------------------------------
u32 BmpMenuList_PosParamGet( BMPMENULIST_WORK * lw, u16 pos )
{
  return lw->hed.list[ pos ].param;
}

//--------------------------------------------------------------------------------------------
/**
 * パラメータ取得関数
 *
 * @param lw    BMPリストワーク
 * @param mode  パラメータ取得ＩＤ
 *
 * @retval  "0xffffffff = エラー"
 * @retval  "0xffffffff != パラメータ"
 */
//--------------------------------------------------------------------------------------------
u32 BmpListParamGet( BMPMENULIST_WORK * lw, u8 mode )
{
  u32 ret;

  switch( mode ){
  case BMPMENULIST_ID_CALLBACK: // カーソル移動ごとのコールバック関数
    ret = (u32)lw->hed.call_back;
    break;
  case BMPMENULIST_ID_ICONFUNC: // 一列表示ごとのコールバック関数
    ret = (u32)lw->hed.icon;
    break;
  case BMPMENULIST_ID_COUNT:    // リスト項目数
    ret = (u32)lw->hed.count;
    break;
  case BMPMENULIST_ID_LINE:   // 表示最大項目数
    ret = (u32)lw->hed.line;
    break;
  case BMPMENULIST_ID_LABEL_X:  // ラベル表示Ｘ座標
    ret = (u32)lw->hed.label_x;
    break;
  case BMPMENULIST_ID_DATA_X:   // 項目表示Ｘ座標
    ret = (u32)lw->hed.data_x;
    break;
  case BMPMENULIST_ID_CURSOR_X: // カーソル表示Ｘ座標
    ret = (u32)lw->hed.cursor_x;
    break;
  case BMPMENULIST_ID_LINE_Y:   // 表示Ｙ座標
    ret = (u32)lw->hed.line_y;
    break;
  case BMPMENULIST_ID_LINE_YBLK:  //１行分シフト量取得(フォントの大きさ＋Ｙ間隔)
#if 0
    ret = (u32)FontHeaderGet( lw->hed.font, FONT_HEADER_SIZE_Y ) + lw->hed.line_spc;
#else
    ret = lw->hed.font_size_y + lw->hed.line_spc;
#endif
    break;
  case BMPMENULIST_ID_F_COL:    // 表示文字色
    ret = (u32)lw->hed.f_col;
    break;
  case BMPMENULIST_ID_B_COL:    // 表示背景色
    ret = (u32)lw->hed.b_col;
    break;
  case BMPMENULIST_ID_S_COL:    // 表示文字影色
    ret = (u32)lw->hed.s_col;
    break;
  case BMPMENULIST_ID_MSG_SPC:  // 文字間隔Ｘ
    ret = (u32)lw->hed.msg_spc;
    break;
  case BMPMENULIST_ID_LINE_SPC: // 文字間隔Ｙ
    ret = (u32)lw->hed.line_spc;
    break;
  case BMPMENULIST_ID_PAGE_SKIP:  // ページスキップタイプ
    ret = (u32)lw->hed.page_skip;
    break;
  case BMPMENULIST_ID_FONT:   // 文字指定
    ret = (u32)lw->hed.font;
    break;
  case BMPMENULIST_ID_C_DISP_F: // ＢＧカーソル(allow)表示フラグ
    ret = (u32)lw->hed.c_disp_f;
    break;
  case BMPMENULIST_ID_WIN:    // ウィンドウデータ
    ret = (u32)lw->hed.win;
    break;
  case BMPMENULIST_ID_WORK:   // ワーク
    ret = (u32)lw->hed.work;
    break;

  default:
    ret = 0xffffffff;
  }
  return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * パラメータ変更関数
 *
 * @param lw    BMPリストワーク
 * @param mode  パラメータ取得ＩＤ
 * @param param 変更値
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_ParamSet( BMPMENULIST_WORK * lw, u8 mode, u32 param )
{
  switch( mode ){
  case BMPMENULIST_ID_CALLBACK: // カーソル移動ごとのコールバック関数
    lw->hed.call_back = (void *)param;
    break;
  case BMPMENULIST_ID_ICONFUNC: // 一列表示ごとのコールバック関数
    lw->hed.icon = (void *)param;
    break;
  case BMPMENULIST_ID_COUNT:    // リスト項目数
    lw->hed.count = (u16)param;
    break;
  case BMPMENULIST_ID_LINE:   // 表示最大項目数
    lw->hed.line = (u16)param;
    break;
  case BMPMENULIST_ID_LABEL_X:  // ラベル表示Ｘ座標
    lw->hed.label_x = (u8)param;
    break;
  case BMPMENULIST_ID_DATA_X:   // 項目表示Ｘ座標
    lw->hed.data_x = (u8)param;
    break;
  case BMPMENULIST_ID_CURSOR_X: // カーソル表示Ｘ座標
    lw->hed.cursor_x = (u8)param;
    break;
  case BMPMENULIST_ID_LINE_Y:   // 表示Ｙ座標
    lw->hed.line_y = (u8)param;
    break;
  case BMPMENULIST_ID_F_COL:    // 表示文字色
    lw->hed.f_col = (u8)param;
    break;
  case BMPMENULIST_ID_B_COL:    // 表示背景色
    lw->hed.b_col = (u8)param;
    break;
  case BMPMENULIST_ID_S_COL:    // 表示文字影色
    lw->hed.s_col = (u8)param;
    break;
  case BMPMENULIST_ID_MSG_SPC:  // 文字間隔Ｘ
    lw->hed.msg_spc = (u8)param;
    break;
  case BMPMENULIST_ID_LINE_SPC: // 文字間隔Ｙ
    lw->hed.line_spc = (u8)param;
    break;
  case BMPMENULIST_ID_PAGE_SKIP:  // ページスキップタイプ
    lw->hed.page_skip = (u8)param;
    break;
  case BMPMENULIST_ID_FONT:   // 文字指定
    lw->hed.font = (u8)param;
    break;
  case BMPMENULIST_ID_C_DISP_F: // ＢＧカーソル(allow)表示フラグ
    lw->hed.c_disp_f = (u8)param;
    break;
  case BMPMENULIST_ID_WIN:    // ウィンドウデータ
    lw->hed.win = (GFL_BMPWIN *)param;
    break;
  case BMPMENULIST_ID_WORK:   // ワーク
    lw->hed.work = (void *)param;
    break;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * BMP_MENULIST_DATAを再度設定する
 *
 * @param BMP_MENULIST_DATA     BMPMENUリストポインタ
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BmpMenuList_SetMenuListData(BMPMENULIST_WORK * lw, BMP_MENULIST_DATA* pList)
{
    lw->hed.list = pList;
}

/********************************************************************/
/*                                                                  */
/*                                                                  */
/*          内部処理関数                  */
/*                                                                  */
/*                                                                  */
/********************************************************************/
#if 0 //old dp
#include "gflib/strbuf.h"
#endif

//------------------------------------------------------------------
//    メッセージ表示
//------------------------------------------------------------------
static void LocalMsgPrint( BMPMENULIST_WORK * lw, void * msg, u8 x, u8 y )
{
    if(msg==NULL){
        return;
    }
    
  if( lw->tmp.sw )          // 一時変更スイッチ
  {
    PRINT_UTIL_PrintColor( lw->hed.print_util, lw->hed.print_que, 
      x, y, msg, lw->hed.font_handle, 
      PRINTSYS_LSB_Make(lw->tmp.f_col, lw->tmp.s_col, lw->tmp.b_col) );
  }
  else
  {
    PRINT_UTIL_PrintColor( lw->hed.print_util, lw->hed.print_que,
      x, y, msg, lw->hed.font_handle, 
      PRINTSYS_LSB_Make(lw->hed.f_col, lw->hed.s_col, lw->hed.b_col) );
  }
}

#if 0 //old dp
static void LocalMsgPrint( BMPMENULIST_WORK * lw, void * msg, u8 x, u8 y )
{
    if(msg==NULL){
        return;
    }
    
  if( lw->tmp.sw )          // 一時変更スイッチ
  {
    GF_STR_PrintExpand( lw->hed.win, lw->tmp.font, msg, x, y, MSG_NO_PUT, 
      GF_PRINTCOLOR_MAKE(lw->tmp.f_col, lw->tmp.s_col, lw->tmp.b_col),
      lw->tmp.msg_spc, 0, NULL );
  }
  else
  {
    GF_STR_PrintExpand( lw->hed.win, lw->hed.font, msg, x, y, MSG_NO_PUT, 
      GF_PRINTCOLOR_MAKE(lw->hed.f_col, lw->hed.s_col, lw->hed.b_col),
      lw->hed.msg_spc, 0, NULL );
  }
}
#endif

//------------------------------------------------------------------
//    リスト表示
//------------------------------------------------------------------
static void ListScreenPut(
  BMPMENULIST_WORK * lw, u16 print_p, u16 line, u16 len, BOOL up_write )
{
  int i;
  u8  x,y,yblk;

#if 0 //old dp
  yblk = FontHeaderGet(lw->hed.font,FONT_HEADER_SIZE_Y) + lw->hed.line_spc;
#else
  yblk = lw->hed.font_size_y + lw->hed.line_spc;
#endif

#if 1 //余白にも書き込み
  if( print_p > 0 && up_write )
  { 
    //一端かいて上にずらす
    if( lw->hed.list[print_p-1].param != BMPMENULIST_LABEL ){
      x = lw->hed.data_x;   // 描画Ｘオフセット
    }else{
      x = lw->hed.label_x;  // 描画Ｘオフセット
    }

    GFL_BMP_Fill( GFL_BMPWIN_GetBmp(lw->hed.win),
      0, 0,
      GFL_BMPWIN_GetSizeX(lw->hed.win) * 8,
      lw->hed.line_y + (line+1) * yblk*2, 
      lw->hed.b_col );


    y = (u8)(((line) * yblk) + lw->hed.line_y);//描画Ｙオフセット

    if( lw->hed.icon != NULL ){
      lw->hed.icon( lw, lw->hed.list[print_p-1].param,y );
    }

    LocalMsgPrint( lw, (void*)lw->hed.list[print_p-1].str, x, y );

    BmpWinDataShift( lw->hed.win, BMPWIN_SHIFT_U, 
      yblk, (u8)((lw->hed.b_col<<4)|lw->hed.b_col), 
      (yblk*2/8 * (GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(lw->hed.win))/8)) );   
    
  }
  //後ろを１つ多めに書く
  len ++;
#endif 

  for( i=0; i<len ;i++ ){
    if( lw->hed.list[print_p].param != BMPMENULIST_LABEL ){
      x = lw->hed.data_x;   // 描画Ｘオフセット
    }else{
      x = lw->hed.label_x;  // 描画Ｘオフセット
    }

    y = (u8)(((i + line) * yblk) + lw->hed.line_y);//描画Ｙオフセット
    if( lw->hed.icon != NULL ){
      lw->hed.icon( lw, lw->hed.list[print_p].param,y );
    }
    LocalMsgPrint( lw, (void*)lw->hed.list[print_p].str, x, y );
    print_p++;
    if( print_p >= lw->hed.count )
    { 
      break;
    }
  }
}


//------------------------------------------------------------------
//    カーソル表示
//static u8 ListCursorActorSet(BMPMENULIST_WORK* lw,int mode);
//------------------------------------------------------------------
static void ListCursorPut( BMPMENULIST_WORK * lw )
{
  u8  x,y,yblk;

#if 0
  yblk = FontHeaderGet(lw->hed.font,FONT_HEADER_SIZE_Y) + lw->hed.line_spc;
#else
  yblk = lw->hed.font_size_y + lw->hed.line_spc;
#endif
  x = lw->hed.cursor_x; //描画Ｘオフセット
  y = (u8)((lw->cp * yblk) + lw->hed.line_y);//描画Ｙオフセット

  switch(lw->hed.c_disp_f){
  case 0:
    BmpCursor_Print(
      lw->cursor, x, y,
      lw->hed.print_util, lw->hed.print_que, lw->hed.font_handle );
    break;
  case 1:
    break;
  case 2:
/*
    if(lw->cur_actID == 0xff){
      //初期化(新規設定)
      lw->cur_actID = ListCursorActorSet(lw,0);
    }
    CursorListActorPosSet(  lw->cur_actID,
                BMPWIN_StatusGet(lw->lh.index,BMPWIN_GET_POS_X) * 8 - 1,
                BMPWIN_StatusGet(lw->lh.index,BMPWIN_GET_POS_Y) * 8 + y - 1,
                0);
*/
    break;
  case 3:
/*
    if(lw->cur_actID == 0xff){
      //初期化(新規設定)
      lw->cur_actID = ListCursorActorSet(lw,1);
    }
    CursorListActorPosSet(  lw->cur_actID,
                BMPWIN_StatusGet(lw->lh.index,BMPWIN_GET_POS_X) * 8 + x,
                BMPWIN_StatusGet(lw->lh.index,BMPWIN_GET_POS_Y) * 8 + y,
                1);
*/
    break;
  }
}

/*
static u8 ListCursorActorSet(BMPMENULIST_WORK* lw,int mode)
{
  ACT_CURSORLIST_HEADER header;

  header.x    = 0;    //表示位置Ｘ
  header.y    = 160;    //表示位置Ｙ
  header.xsize  = BMPWIN_StatusGet(lw->lh.index,BMPWIN_GET_SIZ_X) * 8 + 2;  //表示サイズＸ
  header.ysize  = FontHeaderGet(lw->lh.font,FONT_HEADER_SIZE_Y) + 2;    //表示サイズＹ
  header.cellID = 0x4000;   //カーソルセルＩＤ
  header.palID  = ACT_OAMPAL; //カーソルパレットＩＤ
  header.palnum = 15;     //カーソルパレット指定の場合に使用

  return CursorListActorSet(&header,mode);

  return 0;
}
*/

//------------------------------------------------------------------
//    カーソル消去
//------------------------------------------------------------------
static void ListCursorCls( BMPMENULIST_WORK * lw, u16 p )
{
  u8  yblk;

  switch(lw->hed.c_disp_f){
  case 0:
#if 0 //old dp
    yblk = FontHeaderGet(lw->hed.font,FONT_HEADER_SIZE_Y) + lw->hed.line_spc;
#else
    yblk = lw->hed.font_size_y + lw->hed.line_spc;
#endif

#if 0 //old dp
    GF_BGL_BmpWinFill(
        lw->hed.win,            //表示ウインドウINDEX
        (u8)lw->hed.b_col,          //背景色データ
        lw->hed.cursor_x,         //Ｘ座標
        (u16)(p * yblk + lw->hed.line_y), //Ｙ座標
        8, 16 );
#else
    GFL_BMP_Fill( GFL_BMPWIN_GetBmp(lw->hed.win),
        lw->hed.cursor_x, (u16)(p * yblk + lw->hed.line_y),
        lw->hed.font_size_x, lw->hed.font_size_y, lw->hed.b_col );
#endif
    break;
  case 1:
  case 2:
  case 3:
    break;
  }
}

//------------------------------------------------------------------
//    カーソル移動
//------------------------------------------------------------------
static u8 ListCursorMoveUpDown( BMPMENULIST_WORK * lw, u8 mode )
{
  u16 list_p,cursor_p,check_p;

  cursor_p  = lw->cp;
  list_p    = lw->lp;

  if(mode == LIST_UP){

    //スクロール判定地点(リストの半分を過ぎたら)
    if(lw->hed.line == 1){
      check_p = 0;
    }else{
      check_p = (u16)(lw->hed.line - ((lw->hed.line / 2) + (lw->hed.line % 2)) - 1);
/*
      if( ( lw->hed.line & 1 ) != 0 ){
        check_p = (u16)((lw->hed.line / 2));
      }else{
        check_p = (u16)(lw->hed.line - ((lw->hed.line / 2) + (lw->hed.line % 2)) - 1);
      }
*/
    }

    if(list_p == 0){  //リスト最初の部分
      while(cursor_p > 0){
        cursor_p--;
        if(lw->hed.list[list_p + cursor_p].param != BMPMENULIST_LABEL){
          lw->cp = cursor_p;
          return 1;
        }
      }
      return 0;
    }
    while(cursor_p > check_p){
      cursor_p--;
      if(lw->hed.list[list_p + cursor_p].param != BMPMENULIST_LABEL){
        lw->cp = cursor_p;
        return 1;
      }
    }
    list_p--;
    lw->cp = check_p;
    lw->lp = list_p;

  }else{

    //スクロール判定地点(リストの半分を過ぎたら)
    if(lw->hed.line == 1){
      check_p = 0;
    }else{
      check_p = (u16)((lw->hed.line / 2) + (lw->hed.line % 2));
/*
      if( ( lw->hed.line & 1 ) != 0 ){
        check_p = (u16)((lw->hed.line / 2));
      }else{
        check_p = (u16)((lw->hed.line / 2) + (lw->hed.line % 2));
      }
*/
    }

    if(list_p == (lw->hed.count - lw->hed.line)){//リスト最後の部分
      while(cursor_p < (lw->hed.line - 1)){
        cursor_p++;
        if(lw->hed.list[list_p + cursor_p].param != BMPMENULIST_LABEL){
          lw->cp = cursor_p;
          return 1;
        }
      }
      return 0;
    }
    while(cursor_p < check_p){
      cursor_p++;
      if(lw->hed.list[list_p + cursor_p].param != BMPMENULIST_LABEL){
        lw->cp = cursor_p;
        return 1;
      }
    }
    list_p++;
    lw->cp = check_p;
    lw->lp = list_p;

  }

  return 2;
}

//------------------------------------------------------------------
//    画面スクロール移動
//------------------------------------------------------------------
static void ListScrollMoveUpDown( BMPMENULIST_WORK * lw, u8 len, u8 mode )
{
  u8  yblk;
  u16 ypos;

  if( len >= lw->hed.line ){
#if 0 //old dp
    GF_BGL_BmpWinDataFill( lw->hed.win, lw->hed.b_col );
#else
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(lw->hed.win), lw->hed.b_col );
#endif
    ListScreenPut( lw, lw->lp, 0, lw->hed.line, TRUE ); //ライン描画
    return;
  }
  //１行分シフト量取得(フォントの大きさ＋Ｙ間隔)
#if 0 //old dp
  yblk = FontHeaderGet( lw->hed.font, FONT_HEADER_SIZE_Y ) + lw->hed.line_spc;
#else
  yblk = lw->hed.font_size_y + lw->hed.line_spc;
#endif


  if( mode == LIST_UP ){
    //データシフト
#if 0 //old dp
    GF_BGL_BmpWinShift(
      lw->hed.win, GF_BGL_BMPWIN_SHIFT_D,
      (u8)(len * yblk), (u8)((lw->hed.b_col<<4)|lw->hed.b_col) );
#else
    BmpWinDataShift( lw->hed.win, BMPWIN_SHIFT_D, 
    (u8)(len * yblk), (u8)((lw->hed.b_col<<4)|lw->hed.b_col), 0 );
#endif
    ListScreenPut( lw, lw->lp, 0, len, TRUE );  //追加ライン描画

    ypos = (u16)( lw->hed.line * yblk + lw->hed.line_y );

    //下部を消す
#if 0
    GF_BGL_BmpWinFill(
      lw->hed.win,          //表示ウインドウINDEX
      (u8)lw->hed.b_col,        //背景色データ
      0, ypos,
      (u16)(GF_BGL_BmpWinGet_SizeX( lw->hed.win ) * 8),
      (u16)(GF_BGL_BmpWinGet_SizeY( lw->hed.win ) * 8 - ypos) );
#else
/*    GFL_BMP_Fill( GFL_BMPWIN_GetBmp(lw->hed.win),
      0, ypos,
      GFL_BMPWIN_GetPosX(lw->hed.win) * 8,
      GFL_BMPWIN_GetPosY(lw->hed.win) * 8 - ypos,
      lw->hed.b_col );
*/#endif
  }else{
    //データシフト
#if 0 //old dp
    GF_BGL_BmpWinShift(
        lw->hed.win, GF_BGL_BMPWIN_SHIFT_U,
        (u8)(len * yblk), (u8)((lw->hed.b_col<<4)|lw->hed.b_col) );
#else //wb
  BmpWinDataShift( lw->hed.win, BMPWIN_SHIFT_U, 
    (u8)((len) * yblk), (u8)((lw->hed.b_col<<4)|lw->hed.b_col), 0 );
#endif
    //追加ライン描画
    ListScreenPut(
      lw, (u16)(lw->lp + (lw->hed.line-len)), (u16)(lw->hed.line - len), (u16)len, FALSE );
    //上部を消す
#if 0 //old dp
    GF_BGL_BmpWinFill(
      lw->hed.win,
      (u8)lw->hed.b_col,        //背景色データ
      0, 0,
      (u16)(GF_BGL_BmpWinGet_SizeX( lw->hed.win ) * 8),
      (u16)lw->hed.line_y );
#else
/*    GFL_BMP_Fill( GFL_BMPWIN_GetBmp(lw->hed.win),
      0, 0,
      GFL_BMPWIN_GetPosX(lw->hed.win) * 8,
      lw->hed.line_y, 
      lw->hed.b_col );
*/
#endif
  }
}



//------------------------------------------------------------------
//    リスト移動判定
//------------------------------------------------------------------
static u8 ListMoveUpDownCheck( BMPMENULIST_WORK * lw, u8 print_f, u8 req_line, u8 mode )
{
  u16 cursor_bak; //消去カーソル位置
  u8  write_flg,write_req;  //転送フラグ
  u8  reqcount,linecount;   //処理カウント

  cursor_bak  = lw->cp; //カーソル位置保存
  linecount = 0;
  write_req = 0;

  for(reqcount=0;reqcount<req_line;reqcount++){
    do{
      write_flg = ListCursorMoveUpDown( lw, mode ); //移動判定
      write_req |= write_flg;
      if(write_flg != 2){
        break;
      }
      linecount++;
    //カーソル位置がラベルにかからなくなるまで移動
    }while(lw->hed.list[lw->lp + lw->cp].param == BMPMENULIST_LABEL);
  }

  if(print_f){  //表示スイッチ
    //キャラデータ作成
    switch(write_req){
    //移動不可
    default:
    case 0:
      return 1;
      break;
    //カーソルのみ移動
    case 1:
      ListCursorCls( lw, cursor_bak );
      ListCursorPut( lw );
      CallBackSet( lw, 0 );
#if 0 //old dp
      GF_BGL_BmpWinOn( lw->hed.win );
#else
      GFL_BMPWIN_TransVramCharacter(lw->hed.win);
#endif
      break;
    //表示リスト移動 or リスト＋カーソル移動
    case 2:
    case 3:
      ListCursorCls( lw, cursor_bak );
      ListScrollMoveUpDown( lw, linecount, mode );
      ListCursorPut( lw );
      CallBackSet( lw, 0 );
#if 0 //old dp
      GF_BGL_BmpWinOn( lw->hed.win );
#endif
      break;
    }
  }

  return 0;
}



//------------------------------------------------------------------
//    コールバック関数呼び出し(初期化時 mode = 1,通常動作時 mode = 0)
//------------------------------------------------------------------
static void CallBackSet( BMPMENULIST_WORK * lw, u8 mode )
{
  if( lw->hed.call_back != NULL){
    lw->hed.call_back( lw, lw->hed.list[lw->lp + lw->cp].param, mode );
  }
}

//======================================================================
//  ビットマップ
//======================================================================
//--------------------------------------------------------------
/*
 * ビットマップウィンドウデータシフト 16色
 */
//--------------------------------------------------------------
static void BmpWinDataShift16(
    GFL_BMP_DATA *bmp, u8 direct, u8 offset, u8 data, u16 size )
{
  u8 * cgxram;
  int chroffs, woffs, roffs;
  int blanch_chr, over_offs;
  int xsiz, rline;
  int i;

#if 0
  cgxram    = (u8 *)win->chrbuf;
  blanch_chr  = (data<<24) | (data<<16) | (data<<8) | (data<<0);
  over_offs = win->sizy * win->sizx * GFL_BG_1CHRDATASIZ;
  xsiz    = win->sizx;
#else
  cgxram    = GFL_BMP_GetCharacterAdrs( bmp );
  blanch_chr  = (data<<24) | (data<<16) | (data<<8) | (data<<0);
  xsiz    = GFL_BMP_GetSizeX( bmp ) / 8;
#endif

  if( size == 0 )
  { 
    over_offs = (GFL_BMP_GetSizeY(bmp)/8) * xsiz * GFL_BG_1CHRDATASIZ;
  }
  else
  { 
    over_offs = size;
  }

  switch( direct ){
  case BMPWIN_SHIFT_U:
    for( chroffs=0; chroffs<over_offs; chroffs+=GFL_BG_1CHRDATASIZ ){
      rline = offset;

      for( i=0; i<8; i++ ){
        woffs = chroffs + (i << 2);
        roffs = chroffs + (((xsiz * (rline&0xfffffff8))|(rline&0x00000007)) << 2);
        if(roffs < over_offs){
          *(u32*)(cgxram + woffs) = *(u32*)(cgxram + roffs);
        }else{
          *(u32*)(cgxram + woffs) = blanch_chr;
        }
        rline++;
      }
    }
    break;
  case BMPWIN_SHIFT_D:
    cgxram += over_offs - 4;

    for( chroffs=0; chroffs<over_offs; chroffs+=GFL_BG_1CHRDATASIZ ){
      rline = offset;

      for( i=0; i<8; i++ ){
        woffs = chroffs + (i << 2);
        roffs = chroffs + (((xsiz * (rline&0xfffffff8))|(rline&0x00000007)) << 2);
        if(roffs < over_offs){
          *(u32*)(cgxram - woffs) = *(u32*)(cgxram - roffs);
        }else{
          *(u32*)(cgxram - woffs) = blanch_chr;
        }
        rline++;
      }
    }
    break;
  case BMPWIN_SHIFT_L:
    break;
  case BMPWIN_SHIFT_R:
    break;
  }
}

//--------------------------------------------------------------
/*
 * ビットマップウィンドウデータシフト 256色
 */
//--------------------------------------------------------------
static void BmpWinDataShift256(
    GFL_BMP_DATA *bmp, u8 direct, u8 offset, u8 data, u16 size )
{
  u8 * cgxram;
  int chroffs, woffs, roffs;
  int blanch_chr, over_offs;
  int xsiz, rline;
  int i;

#if 0
  cgxram    = (u8 *)win->chrbuf;
  blanch_chr  = ( data << 24 ) | ( data << 16 ) | ( data << 8 ) | data;
  over_offs = win->sizy * win->sizx * GF_BGL_8BITCHRSIZ;
  xsiz    = win->sizx;
#else
  cgxram    = GFL_BMP_GetCharacterAdrs( bmp );
  blanch_chr  = ( data << 24 ) | ( data << 16 ) | ( data << 8 ) | data;
  xsiz    = GFL_BMP_GetSizeX( bmp ) / 8;
#endif

  if( size == 0 )
  { 
    over_offs = (GFL_BMP_GetSizeY(bmp)/8) * xsiz * GFL_BG_8BITCHRSIZ;
  }
  else
  { 
    over_offs = size;
  }

  switch(direct){
  case BMPWIN_SHIFT_U:
    for( chroffs=0; chroffs<over_offs; chroffs+=GFL_BG_8BITCHRSIZ ){
      rline = offset;

      for( i=0; i<8; i++ ){
        woffs = chroffs + (i << 3);
        roffs = chroffs + (((xsiz * (rline&0xfffffff8))|(rline&0x00000007)) << 3);
        if( roffs < over_offs ){
          *(u32*)(cgxram + woffs) = *(u32*)(cgxram + roffs);
        }else{
          *(u32*)(cgxram + woffs) = blanch_chr;
        }

        woffs += 4;
        roffs += 4;
        if( roffs < over_offs + 4 ){
          *(u32*)(cgxram + woffs) = *(u32*)(cgxram + roffs);
        }else{
          *(u32*)(cgxram + woffs) = blanch_chr;
        }

        rline++;
      }
    }
    break;
  case BMPWIN_SHIFT_D:
    cgxram += ( over_offs - 8 );

    for( chroffs=0; chroffs<over_offs; chroffs+=GFL_BG_8BITCHRSIZ ){
      rline = offset;

      for( i=0; i<8; i++ ){
        woffs = chroffs + (i << 3);
        roffs = chroffs + (((xsiz * (rline&0xfffffff8))|(rline&0x00000007)) << 3);
        if( roffs < over_offs ){
          *(u32*)(cgxram - woffs) = *(u32*)(cgxram - roffs);
        }else{
          *(u32*)(cgxram - woffs) = blanch_chr;
        }

        woffs -= 4;
        roffs -= 4;
        if( roffs < over_offs - 4 ){
          *(u32*)(cgxram - woffs) = *(u32*)(cgxram - roffs);
        }else{
          *(u32*)(cgxram - woffs) = blanch_chr;
        }

        rline++;
      }
    }
    break;
  case BMPWIN_SHIFT_L:
    break;
  case BMPWIN_SHIFT_R:
    break;
  }
}

//--------------------------------------------------------------
/**
 * ビットマップウィンドウデータシフト
 */
//--------------------------------------------------------------
static void BmpWinDataShift(
  GFL_BMPWIN *bmpwin, u8 direct, u8 offset, u8 data, u16 size )
{
  u8 frm = GFL_BMPWIN_GetFrame( bmpwin );
  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( bmpwin );
  
  if( GFL_BG_GetScreenColorMode(frm) == GX_BG_COLORMODE_16 ){
    BmpWinDataShift16( bmp, direct, offset, data, size * GFL_BG_1CHRDATASIZ );
  }else{
    BmpWinDataShift256( bmp, direct, offset, data, size * GFL_BG_8BITCHRSIZ );
  }
}

//--------------------------------------------------------------
/**
 * カーソル文字列セット
 */
//--------------------------------------------------------------
void BmpMenuList_SetCursorString( BMPMENULIST_WORK *lw, u32 strID )
{
  BmpCursor_SetCursorFontMsg( lw->cursor, lw->hed.msgdata, strID );
  ListCursorPut( lw );
}

//--------------------------------------------------------------
/**
 * カーソルビットマップセット
 */
//--------------------------------------------------------------
void BmpMenuList_SetCursorBmp( BMPMENULIST_WORK *lw, u32 heapID )
{
  BmpCursor_SetCursorBitmap( lw->cursor, heapID );
  ListCursorPut( lw );
}

//--------------------------------------------------------------
/**
 * ヘッダーにセットしたワークを取り出す
 */
//--------------------------------------------------------------
void* BmpMenuList_GetWorkPtr( BMPMENULIST_WORK *lw )
{
  return lw->hed.work;
}

//--------------------------------------------------------------
/// キャンセルモードセット
//--------------------------------------------------------------
void BmpMenuList_SetCancelMode( BMPMENULIST_WORK *lw, BMPMENULIST_CANCELMODE mode )
{
  lw->cancel_mode = mode;
}
