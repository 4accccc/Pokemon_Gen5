//=============================================================================
/**
 *
 *	@file		intro_msg.c
 *	@brief  イントロ メッセージ処理
 *	@author	hosaka genya
 *	@data		2009.12.15
 *
 */
//=============================================================================
#include <gflib.h>
#include "system/bmp_winframe.h" // for BmpWinFrame_

// 文字列関連
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h" // for PRINT_QUE

#include "gamesystem/msgspeed.h"  // for MSGSPEED_GetWait

#include "font/font.naix"

//アーカイブ
#include "arc_def.h" // for ARCID_XXX

#include "print/printsys.h" // for PRINTSYS_

#include "intro_sys.h"
#include "intro_msg.h" // for extern

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  CGX_BMPWIN_FRAME_POS = 1,
  STRBUF_SIZE = 1600,
};

#define MSG_SKIP_BTN (PAD_BUTTON_A|PAD_BUTTON_B)

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	メッセージ処理 メインワーク
//==============================================================
struct _INTRO_MSG_WORK {
  // [PRIVATE]
  HEAPID        heap_id;
  PRINT_QUE*    print_que;
  WORDSET*      wordset;
  PRINT_STREAM* print_stream;
  GFL_TCBLSYS*  msg_tcblsys;
  GFL_MSGDATA*  msghandle;
  GFL_FONT*     font;
  GFL_BMPWIN*   win_dispwin;
  GFL_ARCUTIL_TRANSINFO  bgchar_frame;
  STRBUF*   strbuf;
  STRBUF*   exp_strbuf;
};


//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//static STRBUF * MSGDAT_UTIL_AllocExpandString( const WORDSET *wordset, GFL_MSGDATA *MsgManager, u32 strID, HEAPID heapID );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ管理モジュール 生成
 *
 *	@param	HEAPID heap_id ヒープID
 *
 *	@retval INTRO_MSG_WORK* wk メインワーク
 */
//-----------------------------------------------------------------------------
INTRO_MSG_WORK* INTRO_MSG_Create( u16 msg_dat_id, HEAPID heap_id )
{
  INTRO_MSG_WORK* wk;

  // メインワーク アロケート
  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof(INTRO_MSG_WORK) );
  
  // メンバ初期化
  wk->heap_id = heap_id;
  wk->wordset = WORDSET_Create( heap_id );
  wk->print_que = PRINTSYS_QUE_Create( heap_id );
  
  // メッセージハンドルをROMに展開しておく
  wk->msghandle = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, msg_dat_id, heap_id );

  wk->msg_tcblsys = GFL_TCBL_Init( wk->heap_id, wk->heap_id, 1, 0 );

  // メッセージ用フォント転送
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                0x20*PLTID_BG_TEXT_M, 0x20, heap_id );

  // フレームウィンドウ用のキャラを用意
  BmpWinFrame_GraphicSet( BG_FRAME_TEXT_M, CGX_BMPWIN_FRAME_POS, PLTID_BG_TEXT_WIN_M, MENU_TYPE_FIELD, heap_id );

  // フォントを展開
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_MEMORY, FALSE, wk->heap_id );

  wk->strbuf      = GFL_STR_CreateBuffer( STRBUF_SIZE, wk->heap_id );
  wk->exp_strbuf  = GFL_STR_CreateBuffer( STRBUF_SIZE, wk->heap_id );

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ管理モジュール 破棄
 *
 *	@param	INTRO_MSG_WORK* wk  メインワーク
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MSG_Exit( INTRO_MSG_WORK* wk )
{
  GFL_STR_DeleteBuffer( wk->strbuf );
  GFL_STR_DeleteBuffer( wk->exp_strbuf );
  
  GFL_FONT_Delete( wk->font );

  // まだ終了していない文字列表示が会った場合は破棄
  if( wk->print_stream )
  {
    PRINTSYS_PrintStreamDelete( wk->print_stream );
  }
  
  if( wk->win_dispwin )
  {
    GFL_BMPWIN_Delete( wk->win_dispwin );
  }

  GFL_TCBL_Exit( wk->msg_tcblsys );

  PRINTSYS_QUE_Delete( wk->print_que );
  
  GFL_MSG_Delete( wk->msghandle );

  WORDSET_Delete( wk->wordset );

  // メインワーク 破棄
  GFL_HEAP_FreeMemory( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージ管理モジュール 主処理
 *
 *	@param	INTRO_MSG_WORK* wk 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void INTRO_MSG_Main( INTRO_MSG_WORK* wk )
{
  GFL_TCBL_Main( wk->msg_tcblsys );
	PRINTSYS_QUE_Main( wk->print_que );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージプリント 開始
 *
 *	@param	INTRO_MSG_WORK* wk　メインワーク
 *	@param	str_id メッセージID
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_MSG_SetPrint( INTRO_MSG_WORK* wk, int str_id, WORDSET_CALLBACK callback_func, void* callback_arg )
{
  const u8 clear_color = 15;
  GFL_BMPWIN* win;
  int msgspeed;

  // ウィンドウ生成
  if( wk->win_dispwin == NULL )
  {
    wk->win_dispwin = GFL_BMPWIN_Create( BG_FRAME_TEXT_M, 1, 19, 30, 4, PLTID_BG_TEXT_M, GFL_BMP_CHRAREA_GET_B );
  }
  
  msgspeed  = MSGSPEED_GetWait();
  win       = wk->win_dispwin;

  HOSAKA_Printf("msgspeed = %d str_id=%d \n", msgspeed, str_id);

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(win), clear_color);
  GFL_FONTSYS_SetColor(1, 2, clear_color);

  GFL_MSG_GetString( wk->msghandle, str_id, wk->strbuf );

#ifdef PM_DEBUG
  GFL_STR_CheckBufferValid( wk->strbuf ); ///< 破損チェック
#endif

  // コールバックでWORDSET
  if( callback_func )
  {
    callback_func( wk->wordset, callback_arg );
  }

  WORDSET_ExpandStr( wk->wordset, wk->exp_strbuf, wk->strbuf );

#ifdef PM_DEBUG
  GFL_STR_CheckBufferValid( wk->exp_strbuf ); ///< 破損チェック
#endif

  wk->print_stream = PRINTSYS_PrintStream( win, 0, 0, wk->exp_strbuf, wk->font, msgspeed,
                                           wk->msg_tcblsys, 0xffff, wk->heap_id, clear_color );

  BmpWinFrame_Write( win, WINDOW_TRANS_ON_V, CGX_BMPWIN_FRAME_POS, PLTID_BG_TEXT_M );

  // 転送
  GFL_BMPWIN_TransVramCharacter( win );
  GFL_BMPWIN_MakeScreen( win );

  GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(win) );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  メッセージプリント 終了待ち
 *
 *	@param	INTRO_MSG_WORK* wk 
 *
 *	@retval TRUE:終了
 */
//-----------------------------------------------------------------------------
BOOL INTRO_MSG_PrintProc( INTRO_MSG_WORK* wk )
{
  PRINTSTREAM_STATE state = PRINTSYS_PrintStreamGetState( wk->print_stream );

//  HOSAKA_Printf("print state= %d \n", state );

  if( wk->print_stream )
  {
    switch( state )
    {
    case PRINTSTREAM_STATE_DONE : // 終了
      PRINTSYS_PrintStreamDelete( wk->print_stream );
      wk->print_stream = NULL;
      return TRUE;

    case PRINTSTREAM_STATE_PAUSE : // キー入力待ち
      if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE || GFL_UI_TP_GetTrg() )
      {
        PRINTSYS_PrintStreamReleasePause( wk->print_stream );
      }
      break;

    case PRINTSTREAM_STATE_RUNNING :  // 実行中
      // メッセージスキップ
      if( GFL_UI_KEY_GetCont() & MSG_SKIP_BTN )
      {
        PRINTSYS_PrintStreamShortWait( wk->print_stream, 0 );
      }
      break;

    default :
      break;
    }

    return FALSE;
  }

  return FALSE;
}





//=============================================================================
/**
 *								static関数
 */
//=============================================================================

#if 0
//----------------------------------------------------------------------------
/**
 *  @brief  文字列展開しつつMSGを取得
 *
 *  @param  WORDSET *wordset  ワードセット
 *  @param  *MsgManager       メッセージ
 *  @param  strID             文字ID
 *  @param  heapID            テンポラリヒープID
 *
 *  @return 文字列展開後のSTRBUF
 */
//-----------------------------------------------------------------------------
static STRBUF * MSGDAT_UTIL_AllocExpandString( const WORDSET *wordset, GFL_MSGDATA *MsgManager, u32 strID, HEAPID heapID )
{
  STRBUF  *src;
  STRBUF  *dst;
  src = GFL_MSG_CreateString( MsgManager, strID );
  dst = GFL_STR_CreateBuffer( GFL_STR_GetBufferLength( src ), heapID );

  WORDSET_ExpandStr( wordset, src, dst );

  GFL_STR_DeleteBuffer( src );

  return dst;
}
#endif

