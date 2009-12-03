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

//-------------------------------------
///	メッセージウィンドウ
//=====================================
struct _WBM_TEXT_WORK
{
	PRINT_STREAM			*p_stream;	//プリントストリーム
  GFL_TCBLSYS       *p_tcblsys; //タスクシステム
	GFL_BMPWIN*				p_bmpwin;		//BMPWIN
	STRBUF*						p_strbuf;		//文字バッファ
  HEAPID            heapID;     //ヒープID
	u16								clear_chr;	//クリアキャラ
	u16								frm;        //BG面
};
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
WBM_TEXT_WORK * WBM_TEXT_Init( u16 frm, u16 font_plt, u16 frm_plt, u16 frm_chr, HEAPID heapID )
{ 
  WBM_TEXT_WORK *p_wk;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(WBM_TEXT_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(WBM_TEXT_WORK) );
  p_wk->clear_chr = 15;
  p_wk->frm       = frm;
  p_wk->heapID    = heapID;

  //バッファ作成
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( 255, heapID );

	//BMPWIN作成
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( frm, 1, 19, 30, 4, font_plt, GFL_BMP_CHRAREA_GET_B );

  //フレーム描画
  BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_OFF, frm_chr, frm_plt );

  //タスクシステム作成
  p_wk->p_tcblsys = GFL_TCBL_Init( heapID, heapID, 1, 32 );

	//転送
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );
	GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );

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
  //BMPWIN破棄
  GFL_BMPWIN_ClearScreen( p_wk->p_bmpwin );
	GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  //タスクシステム破棄
  GFL_TCBL_Exit( p_wk->p_tcblsys );

	//バッファ破棄
	GFL_STR_DeleteBuffer( p_wk->p_strbuf );

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
  //ストリーム終了検知
  if( p_wk->p_stream )
  { 
    if( PRINTSTREAM_STATE_DONE == PRINTSYS_PrintStreamGetState( p_wk->p_stream ) )
    { 
      PRINTSYS_PrintStreamDelete( p_wk->p_stream );
      p_wk->p_stream  = NULL;
    }
  }

  //タスクシステム面処理
  GFL_TCBL_Main( p_wk->p_tcblsys );
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
void WBM_TEXT_Print( WBM_TEXT_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font )
{ 
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
	GFL_MSG_GetString( p_msg, strID, p_wk->p_strbuf );

  //ストリーム中ならばストリーム破棄
  if( p_wk->p_stream )
  { 
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
  }

  //ストリーム作成
  p_wk->p_stream  =  PRINTSYS_PrintStream( p_wk->p_bmpwin, 0, 0, p_wk->p_strbuf, p_font, MSGSPEED_GetWait(), p_wk->p_tcblsys, 0, p_wk->heapID, p_wk->clear_chr );
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
  return cp_wk->p_stream == NULL;
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

