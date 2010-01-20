//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_util.c
 *	@brief  世界対戦用共通モジュール
 *	@author	Toru=Nagihashi
 *	@data		2009.11.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
//システム
#include "system/gfl_use.h"
#include "gamesystem/msgspeed.h"
#include "system/bmp_winframe.h"
#include "app/app_keycursor.h"

//文字表示
#include "print/printsys.h"

//外部公開
#include "wifibattlematch_util.h"
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					テキスト描画構造体
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
#define WBM_TEXT_TYPE_NONE  (WBM_TEXT_TYPE_MAX)

//-------------------------------------
///	メッセージウィンドウ
//=====================================
struct _WBM_TEXT_WORK
{
  GFL_MSGDATA       *p_msg;
  GFL_FONT          *p_font;
  PRINT_STREAM      *p_stream;
  GFL_TCBLSYS       *p_tcbl;
  GFL_BMPWIN*       p_bmpwin;
  STRBUF*           p_strbuf;
  u16               clear_chr;
  u16               heapID;
  PRINT_UTIL        util;
  PRINT_QUE         *p_que;
  u32               print_update;
  BOOL              is_end_print;
  APP_KEYCURSOR_WORK* p_keycursor;
};
//-------------------------------------
///	プロトタイプ
//=====================================
static void WBM_TEXT_PrintInner( WBM_TEXT_WORK* p_wk, WBM_TEXT_TYPE type );

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  初期化
 *
 *	@param	u16 frm       BG面
 *	@param	font_plt      フォントパレット
 *	@param  frm_plt       フレームパレット
 *	@param  frm_chr       フレームキャラ
 *	@param	heapID  ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
WBM_TEXT_WORK * WBM_TEXT_Init( u16 frm, u16 font_plt, u16 frm_plt, u16 frm_chr, PRINT_QUE *p_que, GFL_FONT *p_font, HEAPID heapID )
{ 
  WBM_TEXT_WORK *p_wk;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(WBM_TEXT_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(WBM_TEXT_WORK) );
  p_wk->clear_chr = 0xF;
  p_wk->p_font    = p_font;
  p_wk->p_que     = p_que;
  p_wk->print_update  = WBM_TEXT_TYPE_NONE;

  //文字送りカーソル作成
  p_wk->p_keycursor  = APP_KEYCURSOR_Create( p_wk->clear_chr, TRUE, TRUE, heapID );

  //バッファ作成
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( 512, heapID );

	//BMPWIN作成
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( frm, 1, 19, 30, 4, font_plt, GFL_BMP_CHRAREA_GET_B );

	//プリントキュー設定
	PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

	//転送
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );
	GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

  p_wk->p_tcbl    = GFL_TCBL_Init( heapID, heapID, 1, 32 );

  //フレーム
  BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_ON, frm_chr, frm_plt );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  破棄
 *
 *	@param	WBM_TEXT_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WBM_TEXT_Exit( WBM_TEXT_WORK* p_wk )
{ 
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream  = NULL;
  }

  GFL_TCBL_Exit( p_wk->p_tcbl );

  BmpWinFrame_Clear( p_wk->p_bmpwin, WINDOW_TRANS_ON );
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  GFL_STR_DeleteBuffer( p_wk->p_strbuf );

  APP_KEYCURSOR_Delete( p_wk->p_keycursor );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ  メイン処理
 *
 *	@param	WBM_TEXT_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WBM_TEXT_Main( WBM_TEXT_WORK* p_wk )
{ 
  switch( p_wk->print_update )
  { 
  default:
    /* fallthor */
  case WBM_TEXT_TYPE_NONE:
    break;

  case WBM_TEXT_TYPE_QUE:
    p_wk->is_end_print  = PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
    break;

  case WBM_TEXT_TYPE_STREAM:
    if( p_wk->p_stream )
    { 
      PRINTSTREAM_STATE state;
      state  = PRINTSYS_PrintStreamGetState( p_wk->p_stream );

      APP_KEYCURSOR_Main( p_wk->p_keycursor, p_wk->p_stream, p_wk->p_bmpwin );

      switch( state )
      { 
      case PRINTSTREAM_STATE_RUNNING:  ///< 処理実行中（文字列が流れている）

        // メッセージスキップ
        if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) )
        {
          PRINTSYS_PrintStreamShortWait( p_wk->p_stream, 0 );
        }
        break;

      case PRINTSTREAM_STATE_PAUSE:    ///< 一時停止中（ページ切り替え待ち等）

        //改行
        if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) )
        { 
          PRINTSYS_PrintStreamReleasePause( p_wk->p_stream );
        }
        break;

      case PRINTSTREAM_STATE_DONE:     ///< 文字列終端まで表示完了
        p_wk->is_end_print  = TRUE;
        break;
      }
    }
    break;
  }

  GFL_TCBL_Main( p_wk->p_tcbl );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  プリント開始
 *
 *	@param	WBM_TEXT_WORK* p_wk ワーク
 *	@param	*p_msg            メッセージ
 *	@param	strID             メッセージID
 *	@param	*p_font           フォント
 */
//-----------------------------------------------------------------------------
void WBM_TEXT_Print( WBM_TEXT_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, WBM_TEXT_TYPE type )
{ 
  //文字列作成
  GFL_MSG_GetString( p_msg, strID, p_wk->p_strbuf );

  WBM_TEXT_PrintInner( p_wk, type );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BMPWINメッセージ  プリント開始  （デバッグ版）
 *
 *	@param	WBM_TEXT_WORK* p_wk ワーク
 *	@param	str              文字列   0xFFFF終端
 *	@param	*p_font           フォント
 */
//-----------------------------------------------------------------------------
void WBM_TEXT_PrintDebug( WBM_TEXT_WORK* p_wk, const u16 *cp_str, u16 len, GFL_FONT *p_font )
{ 
	//文字列作成
	GFL_STR_SetStringCodeOrderLength( p_wk->p_strbuf, cp_str, len );

  WBM_TEXT_PrintInner( p_wk, WBM_TEXT_TYPE_QUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  テキスト文字描画プライベート
 *
 *	@param	WBM_TEXT_WORK* p_wk ワーク
 *	@param	type              描画タイプ
 *
 */
//-----------------------------------------------------------------------------
static void WBM_TEXT_PrintInner( WBM_TEXT_WORK* p_wk, WBM_TEXT_TYPE type )
{ 
  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );

  //ストリーム破棄
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream  = NULL;
  }

  //タイプごとの文字描画
  switch( type )
  { 
  case WBM_TEXT_TYPE_QUE:     //プリントキューを使う
    PRINT_UTIL_Print( &p_wk->util, p_wk->p_que, 0, 0, p_wk->p_strbuf, p_wk->p_font );
    p_wk->print_update  = WBM_TEXT_TYPE_QUE;
    break;

  case WBM_TEXT_TYPE_STREAM:  //ストリームを使う
    p_wk->p_stream  = PRINTSYS_PrintStream( p_wk->p_bmpwin, 0, 0, p_wk->p_strbuf,
        p_wk->p_font, MSGSPEED_GetWait(), p_wk->p_tcbl, 0, p_wk->heapID, p_wk->clear_chr );
    p_wk->print_update  = WBM_TEXT_TYPE_STREAM;
    break;
  }

  p_wk->is_end_print  = FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ  終了検知
 *
 *	@param	const WBM_TEXT_WORK* cp_wk  ワーク
 *
 *	@return TRUEでプリント終了中  FALSEでプリント中
 */
//-----------------------------------------------------------------------------
BOOL WBM_TEXT_IsEnd( const WBM_TEXT_WORK* cp_wk )
{ 
  return cp_wk->is_end_print;
}
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  リスト
 *				    ・簡単にリストを出すために最大値決めうち
 *            ・表示数＝リスト最大数
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================/
#define WBM_LIST_W  (12)
//-------------------------------------
///	選択リスト
//=====================================
struct _WBM_LIST_WORK
{ 
  GFL_BMPWIN        *p_bmpwin;
  PRINT_QUE         *p_que;
  PRINT_UTIL        print_util;
  BMPMENULIST_WORK  *p_list;
  BMP_MENULIST_DATA *p_list_data;
};
//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  リスト初期化
 *
 *	@param	WBM_LIST_SETUP *cp_setup  リスト設定ワーク
 *	@param	heapID                    heapID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
WBM_LIST_WORK * WBM_LIST_Init( const WBM_LIST_SETUP *cp_setup, HEAPID heapID )
{ 
  WBM_LIST_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(WBM_LIST_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(WBM_LIST_WORK) );
  p_wk->p_que = cp_setup->p_que;

  //BMPWIN作成
  { 
    //右下、テキストボックスの上に位置するため
    //表示項目から位置、高さを計算
    const u8 w  = WBM_LIST_W;
    const u8 h  = cp_setup->list_max * 2;
    const u8 x  = 32 - w - 1; //1はフレーム分
    const u8 y  = 24 - h - 1 - 6; //１は自分のフレーム分と6はテキスト分
    p_wk->p_bmpwin  = GFL_BMPWIN_Create( cp_setup->frm, x, y, w, h, cp_setup->font_plt, GFL_BMP_CHRAREA_GET_B );
    BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_OFF, cp_setup->frm_chr, cp_setup->frm_plt );
    GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

    PRINT_UTIL_Setup( &p_wk->print_util, p_wk->p_bmpwin );
  }
  //リストデータ作成
  { 
    int i;
    p_wk->p_list_data = BmpMenuWork_ListCreate( cp_setup->list_max, heapID );
    for( i = 0; i < cp_setup->list_max; i++ )
    { 
      BmpMenuWork_ListAddArchiveString( p_wk->p_list_data,
          cp_setup->p_msg, cp_setup->strID[ i ], i, heapID );
    }
  }
  //リスト作成
  { 
    static const BMPMENULIST_HEADER sc_def_header =
    { 
      NULL,
      NULL,
      NULL,
      NULL,

      0,  //count
      0,  //line
      0,  //rabel_x
      13, //data_x  
      0,  //cursor_x
      3,  //line_y
      1,  //f
      15, //b
      2,  //s
      0,  //msg_spc
      1,  //line_spc
      BMPMENULIST_NO_SKIP,  //page_skip
      0,  //font
      0,  //c_disp_f
      NULL,
      12,
      12,

      NULL,
      NULL,
      NULL,
      NULL,
      
    };
    BMPMENULIST_HEADER  header;
    header  = sc_def_header;

    header.list         = p_wk->p_list_data;
    header.count        = cp_setup->list_max;
    header.line         = cp_setup->list_max;
    header.win          = p_wk->p_bmpwin;
    header.msgdata      = cp_setup->p_msg;
    header.print_util   = &p_wk->print_util;
    header.print_que    = cp_setup->p_que;
    header.font_handle  = cp_setup->p_font;
    p_wk->p_list  = BmpMenuList_Set( &header, 0, 0, heapID );

    BmpMenuList_SetCursorBmp( p_wk->p_list, heapID );
    BmpMenuList_SetCancelMode( p_wk->p_list, BMPMENULIST_CANCELMODE_NOT );
    
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト破棄処理
 *
 *	@param	WBM_LIST_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WBM_LIST_Exit( WBM_LIST_WORK *p_wk )
{ 
  BmpMenuList_Exit( p_wk->p_list, NULL, NULL );
  BmpMenuWork_ListDelete( p_wk->p_list_data );

  BmpWinFrame_Clear( p_wk->p_bmpwin, WINDOW_TRANS_ON );
  GFL_BMPWIN_ClearScreen( p_wk->p_bmpwin );
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リストメイン処理
 *
 *	@param	WBM_LIST_WORK *p_wk   ワーク
 *
 *	@return 選択していないならばWBM_LIST_SELECT_NULL それ以外ならば選択したリストインデックス
 */
//-----------------------------------------------------------------------------
u32 WBM_LIST_Main( WBM_LIST_WORK *p_wk )
{ 
  PRINT_UTIL_Trans( &p_wk->print_util, p_wk->p_que );
  return BmpMenuList_Main( p_wk->p_list );
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  シーケンス管理
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	シーケンスワーク
//=====================================
struct _WBM_SEQ_WORK
{
	WBM_SEQ_FUNCTION	seq_function;		//実行中のシーケンス関数
	BOOL is_end;									//シーケンスシステム終了フラグ
	int seq;											//実行中のシーケンス関数の中のシーケンス
	void *p_wk_adrs;							//実行中のシーケンス関数に渡すワーク
  int reserv_seq;               //予約シーケンス
};

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	初期化
 *
 *	@param	WBM_SEQ_WORK *p_wk	ワーク
 *	@param	*p_param				パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
WBM_SEQ_WORK * WBM_SEQ_Init( void *p_wk_adrs, WBM_SEQ_FUNCTION seq_function, HEAPID heapID )
{	
  WBM_SEQ_WORK *p_wk;

	//作成
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(WBM_SEQ_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(WBM_SEQ_WORK) );

	//初期化
	p_wk->p_wk_adrs	= p_wk_adrs;

	//セット
	WBM_SEQ_SetNext( p_wk, seq_function );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	破棄
 *
 *	@param	WBM_SEQ_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WBM_SEQ_Exit( WBM_SEQ_WORK *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	メイン処理
 *
 *	@param	WBM_SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void WBM_SEQ_Main( WBM_SEQ_WORK *p_wk )
{	
	if( !p_wk->is_end )
	{	
		p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_wk_adrs );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了取得
 *
 *	@param	const WBM_SEQ_WORK *cp_wk		ワーク
 *
 *	@return	TRUEならば終了	FALSEならば処理中
 */	
//-----------------------------------------------------------------------------
BOOL WBM_SEQ_IsEnd( const WBM_SEQ_WORK *cp_wk )
{	
	return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	次のシーケンスを設定
 *
 *	@param	WBM_SEQ_WORK *p_wk	ワーク
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
void WBM_SEQ_SetNext( WBM_SEQ_WORK *p_wk, WBM_SEQ_FUNCTION seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了
 *
 *	@param	WBM_SEQ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
void WBM_SEQ_End( WBM_SEQ_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  SEQ 次のシーケンスを予約
 *
 *	@param	WBM_SEQ_WORK *p_wk  ワーク
 *	@param	seq             次のシーケンス
 */
//-----------------------------------------------------------------------------
void WBM_SEQ_SetReservSeq( WBM_SEQ_WORK *p_wk, int seq )
{ 
  p_wk->reserv_seq  = seq;
}
//----------------------------------------------------------------------------
/**
 *	@brief  SEQ 予約されたシーケンスへ飛ぶ
 *
 *	@param	WBM_SEQ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void WBM_SEQ_NextReservSeq( WBM_SEQ_WORK *p_wk )
{ 
  p_wk->seq = p_wk->reserv_seq;
}
