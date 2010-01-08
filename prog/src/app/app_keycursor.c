//=============================================================================
/**
 *
 *	@file		app_keycursor.c
 *	@brief  メッセージ、キーカーソル（メッセージ送りアイコン）
 *	@author	hosaka genya(kagayaさんのソースを参考に共通化)
 *	@data		2010.01.05
 *
 */
//=============================================================================
#include <gflib.h>
#include "print/printsys.h" // for PRINT_STREAM

#include "app/app_keycursor.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{
  //========================================================
  // 値の変更OK
  //========================================================
  CURSOR_ANM_FRAME    = 4*2,  ///< アニメーションフレーム
  CURSOR_BMP_DIFF_PX  = -11,  ///< カーソル表示位置X( 指定BMPの座標基準 )
  CURSOR_BMP_DIFF_PY  = -9,   ///< カーソル表示位置Y( 指定BMPの座標基準 )

  //========================================================
  // 値を変更するときは他の領域にも手を入れないといけない値
  //========================================================
  CURSOR_ANM_NO_MAX = 3,   ///< アニメパターン数
  CURSOR_BMP_SX     = 10,  ///< カーソル表示サイズX
  CURSOR_BMP_SY     = 7,   ///< カーソル表示サイズY

  CURSOR_BMP_CHR_X  = 2,
  CURSOR_BMP_CHR_Y  = 2,
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
/// メインワーク
//--------------------------------------------------------------
struct _APP_KEYCURSOR_WORK 
{
  GFL_BMP_DATA  *bmp_cursor;
  u8 cursor_anm_no;
  u8 cursor_anm_frame;
  u8 push_flag;
  u8 decide_key_flag : 1;
  u8 decide_tp_flag : 1;
  u8 padding : 6;
  u16 n_col;
  u16 padding_16;
};

//=============================================================================
/**
 *							データテーブル
 */
//=============================================================================

//======================================================================
//  キー送りカーソル
//======================================================================

// 送りカーソルデータ
static const u8 ALIGN4 sc_skip_cursor_character[128] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x22,0x22,0x22,0x22,
  0x21,0x22,0x22,0x22,0x10,0x22,0x22,0x12, 0x00,0x21,0x22,0x11,0x00,0x10,0x12,0x01,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x12,0x00,0x00,0x00,
  0x11,0x00,0x00,0x00,0x01,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

  0x00,0x00,0x11,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//--------------------------------------------------------------
/**
 * キー送りカーソル 初期化
 *  @param n_col      [IN] 透明色指定 0-15,GF_BMPPRT_NOTNUKI	
 *  @param is_decide_key  [IN] TRUE:決定ボタン、キャンセルボタンでカーソルを消す。
 *  @param is_decide_tp   [IN] TRUE:タッチでカーソルを消す。
 *  @param heapID     [IN] HEAPID ヒープID
 *  @retval nothing
 */
//--------------------------------------------------------------
APP_KEYCURSOR_WORK* APP_KEYCURSOR_Create( u16 n_col, BOOL is_decide_key, BOOL is_decide_tp, HEAPID heap_id )
{
  APP_KEYCURSOR_WORK* work;
  
  GF_ASSERT( is_decide_key == TRUE || is_decide_tp == TRUE ); // どちらかがTRUEでないとカーソルが消えない。

  work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(APP_KEYCURSOR_WORK) );
 
  work->n_col = n_col;
  work->decide_key_flag = is_decide_key;
  work->decide_tp_flag  = is_decide_tp;

  // BMP生成
  work->bmp_cursor = GFL_BMP_CreateWithData(
        (u8*)sc_skip_cursor_character,
        CURSOR_BMP_CHR_X, CURSOR_BMP_CHR_Y, GFL_BMP_16_COLOR, heap_id );

  return work;
}

//--------------------------------------------------------------
/**
 * キー送りカーソル 削除
 * @param work メインワーク
 * @retval nothing
 */
//--------------------------------------------------------------
void APP_KEYCURSOR_Delete( APP_KEYCURSOR_WORK *work )
{
  GFL_BMP_Delete( work->bmp_cursor );
  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
/**
 * キー送りカーソル クリア
 * @param work  メインワーク
 * @param bmp   表示先GFL_BMP_DATA
 * @param n_col 透明色指定 0-15,GF_BMPPRT_NOTNUKI	
 * @retval nothing
 */
//--------------------------------------------------------------
void APP_KEYCURSOR_Clear( APP_KEYCURSOR_WORK *work, GFL_BMP_DATA *bmp, u16 n_col )
{
  u16 x,y,offs;
  
  x = GFL_BMP_GetSizeX( bmp ) + CURSOR_BMP_DIFF_PX;
  y = GFL_BMP_GetSizeY( bmp ) + CURSOR_BMP_DIFF_PY;
  offs = work->cursor_anm_no; // 0, 1, 2
  GFL_BMP_Fill( bmp, x, y+offs, CURSOR_BMP_SX, CURSOR_BMP_SY, n_col );
//  HOSAKA_Printf("x=%d y=%d offs=%d \n",x,y,offs);
}

//--------------------------------------------------------------
/**
 * キー送りカーソル 表示
 * @param work APP_KEYCURSOR_WORK
 * @param bmp 表示先GFL_BMP_DATA
 * @param n_col 透明色指定 0-15,GF_BMPPRT_NOTNUKI	
 * @retval nothing
 */
//--------------------------------------------------------------
void APP_KEYCURSOR_Write( APP_KEYCURSOR_WORK *work, GFL_BMP_DATA *bmp, u16 n_col )
{
  u16 x,y,offs;
  
  APP_KEYCURSOR_Clear( work, bmp, n_col );
  
  work->cursor_anm_frame++;
  
  if( work->cursor_anm_frame >= CURSOR_ANM_FRAME ){
    work->cursor_anm_frame = 0;
    work->cursor_anm_no++;
    work->cursor_anm_no %= (CURSOR_ANM_NO_MAX);
  }

  x = GFL_BMP_GetSizeX( bmp ) - CURSOR_BMP_DIFF_PX;
  y = GFL_BMP_GetSizeY( bmp ) - CURSOR_BMP_DIFF_PY;
  offs = work->cursor_anm_no; // 0, 1, 2
  
  GFL_BMP_Print( work->bmp_cursor, bmp, 0, 2, x, y+offs, CURSOR_BMP_SX, CURSOR_BMP_SY, 0x00 );
//  HOSAKA_Printf("x=%d y=%d offs=%d \n",x,y,offs);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キーカーソル 主処理
 *
 *	@param	work メインワーク
 *	@param	stream プリントストリーム
 *	@param	msgwin メッセージウィンドウのBMPWIN
 *	@note  メッセージ表示中に毎SYNC呼ぶ。streamがNULLの時は呼ばないようにする。
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void APP_KEYCURSOR_Main( APP_KEYCURSOR_WORK* work, PRINT_STREAM* stream, GFL_BMPWIN* msgwin )
{
  PRINTSTREAM_STATE state;

  GF_ASSERT( work );
  GF_ASSERT( msgwin );
  GF_ASSERT( stream );

  state = PRINTSYS_PrintStreamGetState( stream );

  switch( state )
  {
  case PRINTSTREAM_STATE_DONE : // 終了
    work->push_flag = FALSE;
    break;

  case PRINTSTREAM_STATE_PAUSE : // 一時停止中
    // キー入力待ち
    {
      BOOL is_decide = FALSE;

      if( work->decide_key_flag )
      {
        // キー入力
        is_decide = ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE || GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL );
      }

      if( is_decide == FALSE && work->decide_tp_flag )
      {
        // タッチ入力
        is_decide = GFL_UI_TP_GetTrg();
      }

      if( is_decide )
      {
        work->push_flag = TRUE;
        APP_KEYCURSOR_Clear( work, GFL_BMPWIN_GetBmp(msgwin), work->n_col );
      }
      else
      {
        if( work->push_flag == FALSE )
        {
          APP_KEYCURSOR_Write( work, GFL_BMPWIN_GetBmp(msgwin), work->n_col );
        }
      }
    }

    GFL_BMPWIN_TransVramCharacter( msgwin );
    break;

  case PRINTSTREAM_STATE_RUNNING :  // 実行中
    work->push_flag = FALSE;
    break;
  
  default : GF_ASSERT(0);
  };

}

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

