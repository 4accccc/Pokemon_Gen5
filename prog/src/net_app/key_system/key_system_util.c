//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_util.c
 *	@brief  共通モジュール
 *	@author	Toru=Nagihashi
 *	@date		2011.05.13
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
#include "app/app_printsys_common.h"
#include "system/time_icon.h"

//アーカイブ
#include "arc_def.h"
#include "arc/key_system_gra.naix"

//文字表示
#include "print/printsys.h"

//SE
#include "key_system_snd.h"

//外部公開
#include "key_system_util.h"

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *          テキスト描画構造体
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
/// 定数
//=====================================
#define KEYSYS_TEXT_TYPE_NONE  (KEYSYS_TEXT_TYPE_MAX)

//-------------------------------------
/// メッセージウィンドウ
//=====================================
struct _KEYSYS_TEXT_WORK
{
  GFL_MSGDATA       *p_msg;
  GFL_FONT          *p_font;
  PRINT_STREAM      *p_stream;
  GFL_TCBLSYS       *p_tcbl;
  TIMEICON_WORK     *p_time;
  GFL_BMPWIN*       p_bmpwin;
  STRBUF*           p_strbuf;
  PRINTSYS_LSB      color;
  HEAPID            heapID;
  PRINT_UTIL        util;
  PRINT_QUE         *p_que;
  u32               print_update;
  u16               is_end_print;
  u16               is_frame;
  APP_KEYCURSOR_WORK* p_keycursor;
  APP_PRINTSYS_COMMON_WORK  common;
  GFL_POINT         ofs;
  KEYSYS_TEXT_POS pos_type;
};
//-------------------------------------
/// プロトタイプ
//=====================================
static void KeySys_Text_PrintInner( KEYSYS_TEXT_WORK* p_wk, KEYSYS_TEXT_TYPE type );
static void KeySys_Text_CalcPosCore( KEYSYS_TEXT_POS type, const GFL_POINT *cp_ofs, GFL_BMP_DATA *p_bmp, STRBUF *p_strbuf, GFL_FONT *p_font, GFL_POINT *p_pos );

//-------------------------------------
/// パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  BMPWINメッセージ  初期化
 *
 *  @param  u16 frm       BG面
 *  @param  font_plt      フォントパレット
 *  @param  frm_plt       フレームパレット
 *  @param  frm_chr       フレームキャラ
 *  @param  heapID  ヒープID
 *
 *  @return ワーク
 *
 */
//-----------------------------------------------------------------------------
KEYSYS_TEXT_WORK * KEYSYS_TEXT_Init( u16 frm, u16 x, u16 y, u16 w, u16 h, u16 font_plt, GFL_FONT *p_font, HEAPID heapID )
{ 
  KEYSYS_TEXT_WORK *p_wk;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(KEYSYS_TEXT_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_TEXT_WORK) );
  p_wk->p_font    = p_font;
  p_wk->p_que     = PRINTSYS_QUE_Create( heapID );
  p_wk->print_update  = KEYSYS_TEXT_TYPE_NONE;
  p_wk->heapID    = heapID;

  //バッファ作成
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 768, heapID );

  //BMPWIN作成
  p_wk->p_bmpwin  = GFL_BMPWIN_Create( frm, x, y, w, h, font_plt, GFL_BMP_CHRAREA_GET_B );

  //プリントキュー設定
  PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

  //転送
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), PRINTSYS_LSB_GetB(p_wk->color) );
  GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin );
	GFL_BMPWIN_MakeScreen( p_wk->p_bmpwin );

  p_wk->p_tcbl      = GFL_TCBL_Init( heapID, heapID, 1, 32 );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *  @brief  BMPWINメッセージ  破棄
 *
 *  @param  KEYSYS_TEXT_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_Exit( KEYSYS_TEXT_WORK* p_wk )
{ 
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream  = NULL;
  }

  if( p_wk->p_time )
  { 
    TIMEICON_Exit( p_wk->p_time );
    p_wk->p_time  = NULL;
  }
  if( p_wk->p_keycursor )
  { 
    APP_KEYCURSOR_Delete( p_wk->p_keycursor );
  }


  GFL_TCBL_Exit( p_wk->p_tcbl );

  BmpWinFrame_Clear( p_wk->p_bmpwin, WINDOW_TRANS_OFF );
  GFL_BMPWIN_Delete( p_wk->p_bmpwin );

  GFL_STR_DeleteBuffer( p_wk->p_strbuf );

  PRINTSYS_QUE_Delete( p_wk->p_que );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ  メイン処理
 *
 *  @param  KEYSYS_TEXT_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_Draw( KEYSYS_TEXT_WORK* p_wk )
{ 
  //プリント
  PRINTSYS_QUE_Main( p_wk->p_que );

  switch( p_wk->print_update )
  { 
  default:
    /* fallthor */
  case KEYSYS_TEXT_TYPE_NONE:
    break;
    
  case KEYSYS_TEXT_TYPE_WAIT:
    PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
    break;

  case KEYSYS_TEXT_TYPE_QUE:
    p_wk->is_end_print  = PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
    break;

  case KEYSYS_TEXT_TYPE_STREAM_FIX:
    /* 内部でKEYSYS_TEXT_TYPE_STREAMに変換しているがねんのため */
  case KEYSYS_TEXT_TYPE_STREAM:
    if( p_wk->p_stream )
    { 
      BOOL is_end;

      if( p_wk->p_keycursor )
      { 
        APP_KEYCURSOR_Main( p_wk->p_keycursor, p_wk->p_stream, p_wk->p_bmpwin );
      }
      is_end  = APP_PRINTSYS_COMMON_PrintStreamFunc( &p_wk->common, p_wk->p_stream );

      if( is_end )
      { 
        p_wk->is_end_print  = TRUE;
      }
    }
    break;
  }

  GFL_TCBL_Main( p_wk->p_tcbl );
}
//----------------------------------------------------------------------------
/**
 *  @brief  BMPWINメッセージ  プリント開始
 *
 *  @param  KEYSYS_TEXT_WORK* p_wk ワーク
 *  @param  *p_msg            メッセージ
 *  @param  strID             メッセージID
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_Print( KEYSYS_TEXT_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, KEYSYS_TEXT_TYPE type )
{ 
  //文字列作成
  GFL_MSG_GetString( p_msg, strID, p_wk->p_strbuf );

  KeySys_Text_PrintInner( p_wk, type );
}
//----------------------------------------------------------------------------
/**
 *	@brief  プリント開始  STRBUF版
 *
 *	@param	KEYSYS_TEXT_WORK* p_wk  ワーク
 *	@param	STRBUF *cp_strbuf 文字
 *	@param	type プリントタイプ
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_PrintBuf( KEYSYS_TEXT_WORK* p_wk, const STRBUF *cp_strbuf, KEYSYS_TEXT_TYPE type )
{
  GFL_STR_CopyBuffer( p_wk->p_strbuf, cp_strbuf );

  KeySys_Text_PrintInner( p_wk, type );
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字色を設定
 *
 *	@param	KEYSYS_TEXT_WORK* p_wk  ワーク
 *	@param	color 文字色
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_SetColor( KEYSYS_TEXT_WORK* p_wk, PRINTSYS_LSB color )
{
  p_wk->color = color;
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字位置を決定
 *
 *	@param	KEYSYS_TEXT_WORK* p_wk  ワーク
 *	@param	x ドットX
 *	@param	y ドットY
 *	@param	type  基準位置
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_SetPos( KEYSYS_TEXT_WORK* p_wk, s16 x, s16 y, KEYSYS_TEXT_POS type )
{
  p_wk->ofs.x = x;
  p_wk->ofs.y = y;
  p_wk->pos_type = type;
}

#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *  @brief  BMPWINメッセージ  プリント開始  （デバッグ版）
 *
 *  @param  KEYSYS_TEXT_WORK* p_wk ワーク
 *  @param  str              文字列   0xFFFF終端
 *  @param  *p_font           フォント
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_PrintDebug( KEYSYS_TEXT_WORK* p_wk, const u16 *cp_str, u16 len, GFL_FONT *p_font )
{ 
  //文字列作成
  GFL_STR_SetStringCodeOrderLength( p_wk->p_strbuf, cp_str, len );

  KeySys_Text_PrintInner( p_wk, KEYSYS_TEXT_TYPE_QUE );
}
#endif

//----------------------------------------------------------------------------
/**
 *  @brief  テキスト文字描画プライベート
 *
 *  @param  KEYSYS_TEXT_WORK* p_wk ワーク
 *  @param  type              描画タイプ
 *
 */
//-----------------------------------------------------------------------------
static void KeySys_Text_PrintInner( KEYSYS_TEXT_WORK* p_wk, KEYSYS_TEXT_TYPE type )
{ 
  GFL_POINT pos;

  //キューをクリアしてから
  PRINTSYS_QUE_Clear( p_wk->p_que );

  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), PRINTSYS_LSB_GetB(p_wk->color) );

  //ストリーム破棄
  if( p_wk->p_stream )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream  = NULL;
  }
  if( p_wk->p_keycursor )
  { 
    APP_KEYCURSOR_Delete( p_wk->p_keycursor );
    p_wk->p_keycursor = NULL;
  }

  KEYSYS_TEXT_EndWait( p_wk );

  //描画位置
  KeySys_Text_CalcPosCore( p_wk->pos_type, &p_wk->ofs, GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->p_strbuf, p_wk->p_font, &pos );

  //タイプごとの文字描画
  switch( type )
  { 
  case KEYSYS_TEXT_TYPE_WAIT:
    p_wk->p_time  = TIMEICON_Create( GFUser_VIntr_GetTCBSYS(), p_wk->p_bmpwin, PRINTSYS_LSB_GetB(p_wk->color),
        TIMEICON_DEFAULT_WAIT, p_wk->heapID );

    PRINT_UTIL_PrintColor( &p_wk->util, p_wk->p_que, pos.x, pos.y, p_wk->p_strbuf, p_wk->p_font, p_wk->color );
    p_wk->print_update  = KEYSYS_TEXT_TYPE_QUE;
    break;

  case KEYSYS_TEXT_TYPE_QUE:     //プリントキューを使う

    PRINT_UTIL_PrintColor( &p_wk->util, p_wk->p_que, pos.x, pos.y, p_wk->p_strbuf, p_wk->p_font, p_wk->color );
    p_wk->print_update  = KEYSYS_TEXT_TYPE_QUE;
    break;

  case KEYSYS_TEXT_TYPE_STREAM_FIX:
    GF_ASSERT_HEAVY( p_wk->p_keycursor == NULL );

    APP_PRINTSYS_COMMON_PrintStreamInit( &p_wk->common, APP_PRINTSYS_COMMON_TYPE_KEY|APP_PRINTSYS_COMMON_TYPE_NOTSKIP);
    //文字送りカーソル作成
    p_wk->p_keycursor  = APP_KEYCURSOR_Create( PRINTSYS_LSB_GetB(p_wk->color), TRUE, FALSE, p_wk->heapID );
    p_wk->p_stream  = PRINTSYS_PrintStream( p_wk->p_bmpwin, pos.x, pos.y, p_wk->p_strbuf,
        p_wk->p_font, MSGSPEED_GetWaitByConfigParam( MSGSPEED_NORMAL ), p_wk->p_tcbl, 0, p_wk->heapID, PRINTSYS_LSB_GetB(p_wk->color) );
    p_wk->print_update  = KEYSYS_TEXT_TYPE_STREAM;
    break;
  case KEYSYS_TEXT_TYPE_STREAM:  //ストリームを使う
    GF_ASSERT_HEAVY( p_wk->p_keycursor == NULL );

    APP_PRINTSYS_COMMON_PrintStreamInit( &p_wk->common, APP_PRINTSYS_COMMON_TYPE_KEY);
    //文字送りカーソル作成
    p_wk->p_keycursor  = APP_KEYCURSOR_Create( PRINTSYS_LSB_GetB(p_wk->color), TRUE, FALSE, p_wk->heapID );
    p_wk->p_stream  = PRINTSYS_PrintStream( p_wk->p_bmpwin, pos.x, pos.y, p_wk->p_strbuf,
        p_wk->p_font, MSGSPEED_GetWait(), p_wk->p_tcbl, 0, p_wk->heapID, PRINTSYS_LSB_GetB(p_wk->color) );
    p_wk->print_update  = KEYSYS_TEXT_TYPE_STREAM;
    break;
  }

  p_wk->is_end_print  = FALSE;
}


//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ  終了検知
 *
 *  @param  const KEYSYS_TEXT_WORK* cp_wk  ワーク
 *
 *  @return TRUEでプリント終了中  FALSEでプリント中
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_TEXT_IsMsgEnd( const KEYSYS_TEXT_WORK* cp_wk )
{ 
  return cp_wk->is_end_print;
}
//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ  待機終了
 *
 *  @param  KEYSYS_TEXT_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_EndWait( KEYSYS_TEXT_WORK* p_wk )
{ 
  p_wk->is_end_print  = TRUE;
  if( p_wk->p_time )
  { 
    TIMEICON_Exit( p_wk->p_time );
    p_wk->p_time  = NULL;

    //タイムアイコンのあとはスクリーンが壊れているため、再転送
    GFL_BMPWIN_MakeScreen( p_wk->p_bmpwin );
    GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(p_wk->p_bmpwin) );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  テキスト  枠を描画
 *
 *  @param  KEYSYS_TEXT_WORK *p_wk   ワーク
 *  @param  フレームのキャラ
 *  @param  フレームのパレット
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_WriteWindowFrame( KEYSYS_TEXT_WORK *p_wk, u16 frm_chr, u8 frm_plt )
{ 
  BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_OFF, frm_chr, frm_plt );
  p_wk->is_frame  = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief キャラクリア
 *
 *	@param	KEYSYS_TEXT_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_ClearCharacter( KEYSYS_TEXT_WORK* p_wk )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), PRINTSYS_LSB_GetB(p_wk->color) );
}

//----------------------------------------------------------------------------
/**
 *	@brief スクリーンクリア
 *
 *	@param	KEYSYS_TEXT_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_ClearScreen( KEYSYS_TEXT_WORK* p_wk )
{
  BmpWinFrame_Clear( p_wk->p_bmpwin, WINDOW_TRANS_OFF );
}
//----------------------------------------------------------------------------
/**
 *	@brief  このテキストのパレットを変更する
 *
 *	@param	KEYSYS_TEXT_WORK* p_wk  ワーク
 *	@param	plt パレット番号
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXT_ChangePalette( KEYSYS_TEXT_WORK* p_wk, u8 plt )
{
  u8  x = GFL_BMPWIN_GetPosX(p_wk->p_bmpwin);
  u8  y = GFL_BMPWIN_GetPosY(p_wk->p_bmpwin);
  u8  w = GFL_BMPWIN_GetSizeX(p_wk->p_bmpwin);
  u8  h = GFL_BMPWIN_GetSizeY(p_wk->p_bmpwin);

  if( p_wk->is_frame )
  {
    x-=1;
    y-=1;
    w+=2;
    h+=2;
  }

  GFL_BG_ChangeScreenPalette( GFL_BMPWIN_GetFrame(p_wk->p_bmpwin),
      x, y, w, h, plt );
}
//----------------------------------------------------------------------------
/**
 *  @brief  描画位置計算コア部分
 *
 *  @param  KEYSYS_TEXT_POS type 位置の種類
 *  @param  cp_ofs                  位置オフセット
 *  @param  GFL_BMP_DATA *cp_bmp    BMP
 *  @param  *p_strbuf               文字列
 *  @param  *p_font                 フォント
 *  @param  *p_pos                  座標受け取り
 */
//-----------------------------------------------------------------------------
static void KeySys_Text_CalcPosCore( KEYSYS_TEXT_POS type, const GFL_POINT *cp_ofs, GFL_BMP_DATA *p_bmp, STRBUF *p_strbuf, GFL_FONT *p_font, GFL_POINT *p_pos )
{ 
  switch( type )
  { 
  case KEYSYS_TEXT_POS_ABSOLUTE:     //絶対位置
    *p_pos  = *cp_ofs;
    break;
  case KEYSYS_TEXT_POS_WH_CENTER:  //相対座標  幅、高さともに中央
    { 
      u32 x, y;
      x = GFL_BMP_GetSizeX( p_bmp ) / 2;
      y = GFL_BMP_GetSizeY( p_bmp ) / 2;
      x -= PRINTSYS_GetStrWidth( p_strbuf, p_font, 0 ) / 2;
      y -= PRINTSYS_GetStrHeight( p_strbuf, p_font ) / 2;
      p_pos->x  = x + cp_ofs->x;
      p_pos->y  = y + cp_ofs->y;
    }
    break;
  case KEYSYS_TEXT_POS_H_CENTER:    //相対座標  高さのみ中央
    { 
      u32 y;
      y = GFL_BMP_GetSizeY( p_bmp ) / 2;
      y -= PRINTSYS_GetStrHeight( p_strbuf, p_font ) / 2;
      p_pos->x  = cp_ofs->x;
      p_pos->y  = y + cp_ofs->y;
    }
    break;

  case KEYSYS_TEXT_POS_W_BACK: //Xのみ後ろから
    { 
      s32 x;

      x = GFL_BMP_GetSizeX( p_bmp );
      x -= PRINTSYS_GetStrWidth( p_strbuf, p_font, 0 );
      x = MATH_IMax( x, 0 );

      p_pos->x  = x + cp_ofs->x;
      p_pos->y  = cp_ofs->y;
    }
    break;
  }
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					メッセージセット構造体
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	メッセージセットウィンドウ
//=====================================
struct _KEYSYS_TEXTSET_WORK
{
  u32 data_num;
  KEYSYS_TEXT_WORK *p_text[0];
};

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	const KEYSYS_TEXTSET_WND_DATA *cp_data_tbl  テキストの数
 *	@param	data_num  総数
 *	@param	frm       フレーム
 *	@param	font_plt  パレット
 *	@param	*p_font フォント
 *	@param	*p_msg  メッセージ
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
KEYSYS_TEXTSET_WORK * KEYSYS_TEXTSET_Init( const KEYSYS_TEXTSET_WND_DATA *cp_data_tbl, u16 data_num, u16 frm, u16 font_plt, GFL_FONT *p_font, GFL_MSGDATA *p_msg, HEAPID heapID )
{
  KEYSYS_TEXTSET_WORK *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(KEYSYS_TEXTSET_WORK) + data_num * sizeof(KEYSYS_TEXT_WORK *) );
  p_wk->data_num = data_num;

  {
    int i;
    const KEYSYS_TEXTSET_WND_DATA *cp_data;
    for( i = 0; i < p_wk->data_num; i++ )
    {
      cp_data = &cp_data_tbl[i];
      p_wk->p_text[i] = KEYSYS_TEXT_Init( frm, cp_data->x, cp_data->y, cp_data->w, cp_data->h, font_plt, p_font, heapID );
      KEYSYS_TEXT_SetColor( p_wk->p_text[i], PRINTSYS_LSB_Make( 1, 2, 0 ) );
      KEYSYS_TEXT_Print( p_wk->p_text[i], p_msg, cp_data->strID, KEYSYS_TEXT_TYPE_QUE );
    }
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	KEYSYS_TEXTSET_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXTSET_Exit( KEYSYS_TEXTSET_WORK* p_wk )
{
  {
    int i;
    for( i = 0;i < p_wk->data_num; i++ )
    {
      KEYSYS_TEXT_Exit( p_wk->p_text[i] );
    }
  }

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画
 *
 *	@param	KEYSYS_TEXTSET_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXTSET_Draw( KEYSYS_TEXTSET_WORK* p_wk )
{
  int i;
  for( i = 0;i < p_wk->data_num; i++ )
  {
    KEYSYS_TEXT_Draw( p_wk->p_text[i] );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  表示チェック
 *
 *	@param	const KEYSYS_TEXTSET_WORK* cp_wk  ワーク
 *
 *	@return TRUE表示終了  FALSE表示中
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_TEXTSET_IsDrawEnd( const KEYSYS_TEXTSET_WORK* cp_wk )
{
  BOOL is_end = TRUE;
  int i;
  for( i = 0; i < cp_wk->data_num; i++ )
  {
    is_end |= KEYSYS_TEXT_IsMsgEnd( cp_wk->p_text[i] );
  }

  return is_end;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *          リスト
 *            ・簡単にリストを出すために最大値決めうち
 *            ・表示数＝リスト最大数
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
/// 定数
//=====================================/
#define KEYSYS_LIST_W  (10)
//-------------------------------------
/// 選択リスト
//=====================================
struct _KEYSYS_LIST_WORK
{ 
  GFL_BMPWIN        *p_bmpwin;
  PRINT_QUE         *p_que;
  PRINT_UTIL        print_util;
  BMPMENULIST_WORK  *p_list;
  BMP_MENULIST_DATA *p_list_data;
  u32               cancel_idx;
};
//-------------------------------------
/// パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  リスト初期化
 *
 *  @param  KEYSYS_LIST_SETUP *cp_setup  リスト設定ワーク
 *  @param  heapID                    heapID
 *
 *  @return ワーク
 */
//-----------------------------------------------------------------------------
KEYSYS_LIST_WORK * KEYSYS_LIST_Init( const KEYSYS_LIST_SETUP *cp_setup, HEAPID heapID )
{ 
  //右下、テキストボックスの上に位置するため
  //表示項目から位置、高さを計算
  const u8 w  = KEYSYS_LIST_W;
  const u8 h  = cp_setup->list_max * 2;
  const u8 x  = 32 - w - 1; //1はフレーム分
  const u8 y  = 24 - h - 1 - 6; //１は自分のフレーム分と6はテキスト分
  return KEYSYS_LIST_InitEx( cp_setup, x, y, w, h, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  リスト初期化
 *
 *  @param  KEYSYS_LIST_SETUP *cp_setup  リスト設定ワーク
 *  @param  x 座標X
 *  @param  y 座標Y
 *  @param  w 幅
 *  @param  h 高さ
 *  @param  heapID                    heapID
 *
 *  @return ワーク
 */
//-----------------------------------------------------------------------------
KEYSYS_LIST_WORK * KEYSYS_LIST_InitEx( const KEYSYS_LIST_SETUP *cp_setup, u8 x, u8 y, u8 w, u8 h, HEAPID heapID )
{ 
  KEYSYS_LIST_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(KEYSYS_LIST_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_LIST_WORK) );
  p_wk->p_que = cp_setup->p_que;
  p_wk->cancel_idx  = cp_setup->cancel_idx;

  //BMPWIN作成
  { 
    p_wk->p_bmpwin  = GFL_BMPWIN_Create( cp_setup->frm, x, y, w, h, cp_setup->font_plt, GFL_BMP_CHRAREA_GET_B );
    BmpWinFrame_Write( p_wk->p_bmpwin, WINDOW_TRANS_OFF, cp_setup->frm_chr, cp_setup->frm_plt );
	  GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin );
	  GFL_BMPWIN_MakeScreen( p_wk->p_bmpwin );

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
      
      BMPMENULIST_ARROW_WAIT_MAX, //矢印カーソルが表示されるまでのウェイト
    };
    BMPMENULIST_HEADER  header;
    header  = sc_def_header;

    header.list         = p_wk->p_list_data;
    header.count        = cp_setup->list_max;
    header.line         = cp_setup->list_max;
    header.msgdata      = cp_setup->p_msg;
    header.print_util   = &p_wk->print_util;
    header.print_que    = cp_setup->p_que;
    header.font_handle  = cp_setup->p_font;
    p_wk->p_list  = BmpMenuList_Set( &header, 0, cp_setup->default_idx, heapID );

    BmpMenuList_SetCursorBmp( p_wk->p_list, heapID );

    if( cp_setup->is_cancel )
    { 
      BmpMenuList_SetCancelMode( p_wk->p_list, BMPMENULIST_CANCELMODE_USE );
    }
    else
    {
      BmpMenuList_SetCancelMode( p_wk->p_list, BMPMENULIST_CANCELMODE_NOT );
    }

    
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *  @brief  リスト破棄処理
 *
 *  @param  KEYSYS_LIST_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_LIST_Exit( KEYSYS_LIST_WORK *p_wk )
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
 *  @brief  リストメイン処理
 *
 *  @param  KEYSYS_LIST_WORK *p_wk   ワーク
 *
 *  @return 選択していないならばKEYSYS_LIST_SELECT_NULL それ以外ならば選択したリストインデックス
 */
//-----------------------------------------------------------------------------
u32 KEYSYS_LIST_Main( KEYSYS_LIST_WORK *p_wk )
{ 
  u32 ret;

  KEYSYS_LIST_Draw( p_wk );

  ret = BmpMenuList_Main( p_wk->p_list );

  if( ret == BMPMENULIST_CANCEL )
  { 
    ret = p_wk->cancel_idx;
  }

  return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字列転送処理
 *
 *	@param	KEYSYS_LIST_WORK *p_wk ワーク
 *
 *	@return TRYE転送終了  FALSEまだ
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_LIST_Draw( KEYSYS_LIST_WORK *p_wk )
{
  return PRINT_UTIL_Trans( &p_wk->print_util, p_wk->p_que );
}
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *          メニュー
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
enum
{
  KEYSYS_MENU_RES_NCL,
  KEYSYS_MENU_RES_NCG,
  KEYSYS_MENU_RES_NCE,
  KEYSYS_MENU_RES_MAX
};

//-------------------------------------
///	メニュー
//=====================================
struct _KEYSYS_MENU_WORK
{
  KEYSYS_MENU_PARAM param;
  KEYSYS_TEXT_WORK *p_wndmsg[KEYSYS_MENU_WINDOW_MAX];

  u32 seq;
  BOOL is_decide;
  BOOL is_change;
  s32 cursor;
  u16 bganm_normal[0x10];
  u16 bganm_change1[0x10];
  u16 bganm_change2[0x10];
  u16 bganm_buff[0x10];
  u16 bganm_cnt;
  u16 decide_anm;
};
//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  メニュー初期化
 *
 *	@param	const KEYSYS_MENU_PARAM *cp_param 引数
 *	@param	heapID  ヒープID
 *
 *	@return メニュー
 */
//-----------------------------------------------------------------------------
KEYSYS_MENU_WORK * KEYSYS_MENU_Init( const KEYSYS_MENU_PARAM *cp_param, HEAPID heapID )
{
  KEYSYS_MENU_WORK *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(KEYSYS_MENU_WORK) );
  p_wk->param   = *cp_param;
  p_wk->cursor  = cp_param->cursor;


  //リソース読み込み
  {
    void *p_plt_buff;
    const u16 *cp_plt_adrs;
    NNSG2dPaletteData *p_plt_data;

    p_plt_buff  = GFL_ARC_UTIL_LoadPalette( ARCID_KEY_SYSTEM, NARC_key_system_gra_menu_frame_NCLR, &p_plt_data, heapID );
    cp_plt_adrs = p_plt_data->pRawData;
    GFL_STD_MemCopy( (u8*)cp_plt_adrs + (0 * 0x20), p_wk->bganm_normal, sizeof(u16) * 0x10 );
    GFL_STD_MemCopy( (u8*)cp_plt_adrs + (1 * 0x20), p_wk->bganm_change1, sizeof(u16) * 0x10 );
    GFL_STD_MemCopy( (u8*)cp_plt_adrs + (2 * 0x20), p_wk->bganm_change2, sizeof(u16) * 0x10 );

    GFL_HEAP_FreeMemory( p_plt_buff );
  }

  {
    int i;
    for( i = 0; i < cp_param->max; i++ )
    {
      p_wk->p_wndmsg[i] = KEYSYS_TEXT_Init( cp_param->frm, cp_param->wnd_data[i].x, cp_param->wnd_data[i].y, cp_param->wnd_data[i].w, cp_param->wnd_data[i].h, cp_param->frm_plt, cp_param->p_font, heapID );
      KEYSYS_TEXT_SetColor( p_wk->p_wndmsg[i], PRINTSYS_LSB_Make( 0xE, 0xF, 1 ) );
      KEYSYS_TEXT_ClearCharacter( p_wk->p_wndmsg[i] );
      KEYSYS_TEXT_WriteWindowFrame( p_wk->p_wndmsg[i], cp_param->frm_chr, cp_param->frm_plt );
      KEYSYS_TEXT_Print( p_wk->p_wndmsg[i], cp_param->p_msg, cp_param->wnd_data[i].strID, KEYSYS_TEXT_TYPE_QUE );
    }
  }

  KEYSYS_TEXT_ChangePalette( p_wk->p_wndmsg[p_wk->cursor], cp_param->frm_plt+1 );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  メニュー破棄
 *
 *	@param	KEYSYS_MENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_MENU_Exit( KEYSYS_MENU_WORK *p_wk )
{
  {
    int i;
    for( i = 0; i < p_wk->param.max; i++ )
    {
      KEYSYS_TEXT_Exit( p_wk->p_wndmsg[i] );
    }
  }

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 *
 *	@param	KEYSYS_MENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_MENU_Main( KEYSYS_MENU_WORK *p_wk )
{
  enum
  {
    SEQ_SELECT,
    SEQ_DECIDE_ANM,
    SEQ_END,
  };


  p_wk->is_change = FALSE;

  switch( p_wk->seq )
  {
  case SEQ_SELECT:
    {
      const int repeat = GFL_UI_KEY_GetRepeat();
      BOOL is_key = FALSE;
      BOOL is_trg = FALSE;

      //キー操作
      if( repeat & PAD_KEY_UP )
      {
        PMSND_PlaySE( KEYSYS_SE_SELCT );
        p_wk->cursor--;
        p_wk->cursor = p_wk->cursor < 0 ?  p_wk->param.max-1: p_wk->cursor; 
        is_key = TRUE;
      }
      else if( repeat & PAD_KEY_DOWN )
      {
        PMSND_PlaySE( KEYSYS_SE_SELCT );
        p_wk->cursor++;
        p_wk->cursor %= p_wk->param.max;
        is_key = TRUE;
      }
      else if( repeat & PAD_BUTTON_DECIDE )
      {
        is_trg = TRUE;
        if( p_wk->param.DecideCallBack )
        {
          if( !p_wk->param.DecideCallBack( p_wk->cursor, p_wk->param.p_wk_adrs ) )
          {
            is_trg = FALSE;
          }
        }
        if( is_trg )
        {
          PMSND_PlaySE( KEYSYS_SE_DECIDE );
        }
        else
        {
          PMSND_PlaySE( KEYSYS_SE_CANCEL );
        }
      }
      else if( repeat & PAD_BUTTON_CANCEL )
      {
        p_wk->cursor = p_wk->param.max-1;
        is_trg = TRUE;
        p_wk->is_change = TRUE;
        PMSND_PlaySE( KEYSYS_SE_CANCEL );
      }

      if ( p_wk->bganm_cnt + 0x400 >= 0x10000 )
      {
        p_wk->bganm_cnt = p_wk->bganm_cnt + 0x400 - 0x10000;
      }
      else
      {
        p_wk->bganm_cnt += 0x400;
      }

      //キー操作したら切り替わり
      if( is_key )
      {
        int i;
        u8 plt;
        for( i = 0; i < p_wk->param.max; i++ )
        {
          if( i == p_wk->cursor )
          {
            plt = p_wk->param.frm_plt+1;
          }
          else
          {
            plt = p_wk->param.frm_plt;
          }
          KEYSYS_TEXT_ChangePalette( p_wk->p_wndmsg[i], plt );
        }


        p_wk->is_change = TRUE;

        p_wk->bganm_cnt = 0;
        GFL_BG_LoadScreenV_Req( p_wk->param.frm );
      }
      //決めたら初期化して次へ
      if( is_trg )
      {
        int i;
        u8 plt;
        for( i = 0; i < p_wk->param.max; i++ )
        {
          KEYSYS_TEXT_ChangePalette( p_wk->p_wndmsg[i], p_wk->param.frm_plt );
        }

        GFL_BG_LoadPalette( p_wk->param.frm, p_wk->bganm_change1, sizeof(u16)*0x10, (p_wk->param.frm_plt+1) * 0x20 );
        p_wk->seq++;
        p_wk->bganm_cnt = 0;
        p_wk->decide_anm = 0;
      }
    }
    break;
  case SEQ_DECIDE_ANM:

    //２回点滅する
    {
      u8 plt;
      if( (p_wk->decide_anm/4)%2 )
      {
        plt = p_wk->param.frm_plt+1;
      }
      else
      {
        plt = p_wk->param.frm_plt;
      }
      KEYSYS_TEXT_ChangePalette( p_wk->p_wndmsg[p_wk->cursor], plt );
      GFL_BG_LoadScreenV_Req( p_wk->param.frm );
      p_wk->bganm_cnt = 0;
    }

    if( p_wk->decide_anm++ > 16 )
    {
      p_wk->seq++;
    }
    break;
  case SEQ_END:
    p_wk->is_decide = TRUE;
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画
 *
 *	@param	KEYSYS_MENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_MENU_Draw( KEYSYS_MENU_WORK *p_wk )
{
  int i;
  for( i = 0; i < p_wk->param.max; i++ )
  {
    KEYSYS_TEXT_Draw( p_wk->p_wndmsg[i] );
  }
  
  for( i = 0; i < 0x10; i++ )
  {
    KEYSYS_UTIL_MainPltAnm( NNS_GFD_DST_2D_BG_PLTT_MAIN, &p_wk->bganm_buff[i], p_wk->bganm_cnt, p_wk->param.frm_plt + 1, i, p_wk->bganm_change2[i], p_wk->bganm_change1[i] );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画終了を検知
 *
 *	@param	const KEYSYS_MENU_WORK *cp_wk ワーク 
 *
 *	@return TRUE描画完了  FALSEまだ描画中
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_MENU_IsDrawEnd( const KEYSYS_MENU_WORK *cp_wk )
{
  BOOL is_end = TRUE;
  int i;
  for( i = 0; i < cp_wk->param.max; i++ )
  {
    is_end |= KEYSYS_TEXT_IsMsgEnd( cp_wk->p_wndmsg[i] );
  }

  return is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief  決定したかを検知
 *
 *	@param	const KEYSYS_MENU_WORK *cp_wk ワーク
 *
 *	@return TRUE決定した  FALSEまだ 
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_MENU_IsDecide( const KEYSYS_MENU_WORK *cp_wk )
{
  return cp_wk->is_decide; 
}
//----------------------------------------------------------------------------
/**
 *	@brief  再起動
 *
 *	@param	KEYSYS_MENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_MENU_ReStart( KEYSYS_MENU_WORK *p_wk )
{
  p_wk->seq = 0;
  p_wk->is_decide = FALSE;
  KEYSYS_TEXT_ChangePalette( p_wk->p_wndmsg[p_wk->cursor], p_wk->param.frm_plt+1 );
  GFL_BG_LoadScreenV_Req( p_wk->param.frm );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メニュー引数
 *
 *	@param	const KEYSYS_MENU_WORK *cp_wk   ワーク
 *
 *	@return メニュー引数
 */
//-----------------------------------------------------------------------------
const KEYSYS_MENU_PARAM * KEYSYS_MENU_GetParam( const KEYSYS_MENU_WORK *cp_wk )
{
  return &cp_wk->param;
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択しているもの取得
 *
 *	@param	const KEYSYS_MENU_WORK *cp_wk ワーク
 *
 *	@return 選択されたもの
 */
//-----------------------------------------------------------------------------
s32 KEYSYS_MENU_GetSelect( const KEYSYS_MENU_WORK *cp_wk )
{
  return cp_wk->cursor;
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択を切り替えたかどうか  切り替えた後の１フレームだけTRUEを返す
 *
 *	@param	const KEYSYS_MENU_WORK *cp_wk ワーク
 *
 *	@return TRUE切り替えた  切り替えてない
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_MENU_IsChangeSelect( const KEYSYS_MENU_WORK *cp_wk )
{
  return cp_wk->is_change;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *          シーケンス管理
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	シーケンスひとつ
//=====================================
typedef struct
{
  KEYSYS_SEQ_FUNCTION  update;          //実行中のシーケンス関数
  int seq;                              //実行中のシーケンス関数の中のシーケンス
  int reserv_seq;                       //予約シーケンス
} KEYSYS_SEQ;

//-------------------------------------
/// シーケンスワーク
//=====================================
struct _KEYSYS_SEQ_WORK
{
  void *p_wk_adrs;              //実行中のシーケンス関数に渡すワーク
  s32 stack_now;
  s32 stack_max;
  KEYSYS_SEQ seq[0];
};

//-------------------------------------
/// プライベート
//=====================================
static KEYSYS_SEQ * Keysys_Seq_Get( KEYSYS_SEQ_WORK *p_wk, u32 index );

//-------------------------------------
/// パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 初期化
 *
 *  @param  KEYSYS_SEQ_WORK *p_wk  ワーク
 *  @param  *p_param        パラメータ
 *  @param  seq_function    シーケンス
 *
 */
//-----------------------------------------------------------------------------
KEYSYS_SEQ_WORK *KEYSYS_SEQ_Init( u32 stack_max, void *p_wk_adrs, KEYSYS_SEQ_FUNCTION seq_function, HEAPID heapID )
{ 
  KEYSYS_SEQ_WORK *p_wk;

  //作成
  u32 size = sizeof(KEYSYS_SEQ_WORK) + sizeof(KEYSYS_SEQ) * stack_max;
  p_wk  = GFL_HEAP_AllocMemory( heapID, size );
  GFL_STD_MemClear( p_wk, size );

  //初期化
  p_wk->p_wk_adrs = p_wk_adrs;
  p_wk->stack_max = stack_max;

  //セット
  KEYSYS_SEQ_Next( p_wk, seq_function );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 破棄
 *
 *  @param  KEYSYS_SEQ_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_Exit( KEYSYS_SEQ_WORK *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ メイン処理
 *
 *  @param  KEYSYS_SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_Main( KEYSYS_SEQ_WORK *p_wk )
{ 
  KEYSYS_SEQ *p_seq =  Keysys_Seq_Get( p_wk, p_wk->stack_now );
  if( p_seq->update )
  { 
    p_seq->update( p_wk, &p_seq->seq, p_wk->p_wk_adrs );
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 終了取得
 *
 *  @param  const KEYSYS_SEQ_WORK *cp_wk   ワーク
 *
 *  @return TRUEならば終了  FALSEならば処理中
 */ 
//-----------------------------------------------------------------------------
BOOL KEYSYS_SEQ_IsEnd( const KEYSYS_SEQ_WORK *cp_wk )
{ 
  const KEYSYS_SEQ *cp_seq =  Keysys_Seq_Get( (KEYSYS_SEQ_WORK *)cp_wk, 0 );
  return cp_seq->update == NULL;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 次のシーケンスを設定
 *
 *  @param  KEYSYS_SEQ_WORK *p_wk  ワーク
 *  @param  seq_function    シーケンス
 *
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_Next( KEYSYS_SEQ_WORK *p_wk, KEYSYS_SEQ_FUNCTION seq_function )
{ 
  KEYSYS_SEQ *p_seq =  Keysys_Seq_Get( p_wk, p_wk->stack_now );
  GFL_STD_MemClear( p_seq, sizeof(KEYSYS_SEQ) );
  p_seq->update = seq_function;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ ひとつ上にシーケンスを載せて実行
 *
 *  @param  KEYSYS_SEQ_WORK *p_wk  ワーク
 *  @param  seq_function    シーケンス
 *
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_Call( KEYSYS_SEQ_WORK *p_wk, KEYSYS_SEQ_FUNCTION seq_function )
{
  p_wk->stack_now++;
  GF_ASSERT( p_wk->stack_now <  p_wk->stack_max );
  KEYSYS_SEQ_Next( p_wk, seq_function );
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 終了
 *
 *  @param  KEYSYS_SEQ_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_End( KEYSYS_SEQ_WORK *p_wk )
{ 
  p_wk->stack_now = 0;
  KEYSYS_SEQ_Next( p_wk, NULL );
}

//----------------------------------------------------------------------------
/**
 *  @brief  SEQ をひとつ戻す (最下層ならばEnd)
 *
 *  @param  KEYSYS_SEQ_WORK *p_wk  ワーク
 *
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_Return( KEYSYS_SEQ_WORK *p_wk )
{
  KEYSYS_SEQ_Next( p_wk, NULL );
  if( p_wk->stack_now > 0 )
  {
    p_wk->stack_now--;
  }
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 次のシーケンスを予約
 *
 *  @param  KEYSYS_SEQ_WORK *p_wk  ワーク
 *  @param  seq             次のシーケンス
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_SetReservSeq( KEYSYS_SEQ_WORK *p_wk, int seq )
{ 
  KEYSYS_SEQ *p_seq =  Keysys_Seq_Get( p_wk, p_wk->stack_now );
  p_seq->reserv_seq  = seq;
}
//----------------------------------------------------------------------------
/**
 *  @brief  SEQ 予約されたシーケンスへ飛ぶ
 *
 *  @param  KEYSYS_SEQ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_NextReservSeq( KEYSYS_SEQ_WORK *p_wk )
{ 
  KEYSYS_SEQ *p_seq =  Keysys_Seq_Get( p_wk, p_wk->stack_now );
  p_seq->seq = p_seq->reserv_seq;
}
//----------------------------------------------------------------------------
/**
 *	@brief 関数と比較
 *
 *	@param	KEYSYS_SEQ_WORK *p_wk ワーク
 *	@param	seq_function  比較する関数
 *
 *	@return TRUE同じ  FALSEで違う
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_SEQ_IsCompireFunc( const KEYSYS_SEQ_WORK *cp_wk, KEYSYS_SEQ_FUNCTION seq_function )
{
  KEYSYS_SEQ *p_seq =  Keysys_Seq_Get( (KEYSYS_SEQ_WORK *)cp_wk, cp_wk->stack_now );
  return p_seq->update == seq_function;
}
//----------------------------------------------------------------------------
/**
 *	@brief  内部シーケンスを加算する
 *
 *	@param	KEYSYS_SEQ_WORK *p_wk ワーク
 *	@param	add 加算する数
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_AddLocalSeq( KEYSYS_SEQ_WORK *p_wk, int add )
{
  KEYSYS_SEQ *p_seq =  Keysys_Seq_Get( p_wk, p_wk->stack_now );
  p_seq->seq += add;
}
//----------------------------------------------------------------------------
/**
 *	@brief  シーケンスをすべてクリアする
 *
 *	@param	KEYSYS_SEQ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_SEQ_Clear( KEYSYS_SEQ_WORK *p_wk )
{
  while( p_wk->stack_now > 0 )
  {
    KEYSYS_SEQ_Return( p_wk );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  シーケンスワークを取得
 *
 *	@param	KEYSYS_SEQ_WORK *p_wkワーク
 *	@param	index   インデックス
 *
 *	@return シーケンスワーク
 */
//-----------------------------------------------------------------------------
static KEYSYS_SEQ * Keysys_Seq_Get( KEYSYS_SEQ_WORK *p_wk, u32 index )
{
  GF_ASSERT( index < p_wk->stack_max );
  return &p_wk->seq[ index ];
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  ディスプレイ作成
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
/// ディスプレイ用シーケンス
//=====================================
typedef enum
{
  DISPLAY_SEQ_NO_DISPLAY,
  DISPLAY_SEQ_CREATE_START,
  DISPLAY_SEQ_CREATE_WAIT,
  DISPLAY_SEQ_ON_DISPLAY,
  DISPLAY_SEQ_DELETE_START,
  DISPLAY_SEQ_DELETE_WAIT,
}KEYSYS_DISPLAY_SEQ;
//-------------------------------------
///	ディスプレイ
//=====================================
struct _KEYSYS_DISPLAY_GENELATER
{
  void *p_wk_adrs;
  KEYSYS_DISPLAY_SEQ seq;
  KEYSYS_DISPLAY_FUNCTION func;
  HEAPID create_heapID;
};

//-------------------------------------
///	パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
KEYSYS_DISPLAY_GENELATER * KEYSYS_DISPLAY_Init( void *p_wk_adrs, HEAPID heapID )
{
  KEYSYS_DISPLAY_GENELATER *p_wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(KEYSYS_DISPLAY_GENELATER));
  p_wk->p_wk_adrs = p_wk_adrs;

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	KEYSYS_DISPLAY_GENELATER *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_DISPLAY_Exit( KEYSYS_DISPLAY_GENELATER *p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  作成メイン
 *
 *	@param	KEYSYS_DISPLAY_GENELATER *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_DISPLAY_Main( KEYSYS_DISPLAY_GENELATER *p_wk )
{
  switch( p_wk->seq )
  {
  case DISPLAY_SEQ_CREATE_START:
    if( p_wk->func.Create )
    {
      p_wk->func.Create( p_wk->p_wk_adrs, p_wk->create_heapID );
    }
    p_wk->seq = DISPLAY_SEQ_CREATE_WAIT;
    /* fallthor */
  case DISPLAY_SEQ_CREATE_WAIT:
    if( p_wk->func.IsEndCreate
      && !p_wk->func.IsEndCreate( p_wk->p_wk_adrs ) )
    {
      break;
    }
    p_wk->seq = DISPLAY_SEQ_ON_DISPLAY;
    /* fallthor */
  case DISPLAY_SEQ_ON_DISPLAY:
    /* none */
    break;
  case DISPLAY_SEQ_DELETE_START:
    if( p_wk->func.Delete )
    {
      p_wk->func.Delete( p_wk->p_wk_adrs );
    }
    p_wk->seq = DISPLAY_SEQ_DELETE_WAIT;
    /* fallthor */
  case DISPLAY_SEQ_DELETE_WAIT:
    if( p_wk->func.IsEndDelete
      && !p_wk->func.IsEndDelete( p_wk->p_wk_adrs ) )
    {
    }
    p_wk->seq = DISPLAY_SEQ_NO_DISPLAY;
    /* fallthor */
  case DISPLAY_SEQ_NO_DISPLAY:
    /* none */
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  作成関数を呼ぶ
 *
 *	@param	KEYSYS_DISPLAY_GENELATER *p_wk  ワーク
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void KEYSYS_DISPLAY_Create( KEYSYS_DISPLAY_GENELATER *p_wk, const KEYSYS_DISPLAY_FUNCTION *cp_func, HEAPID heapID )
{
  if( p_wk->seq == DISPLAY_SEQ_NO_DISPLAY )
  {
    p_wk->func = *cp_func;
    p_wk->create_heapID = heapID;
    p_wk->seq = DISPLAY_SEQ_CREATE_START;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  消去関数を呼ぶ
 *
 *	@param	KEYSYS_DISPLAY_GENELATER *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_DISPLAY_Delete( KEYSYS_DISPLAY_GENELATER *p_wk )
{
  if( p_wk->seq == DISPLAY_SEQ_ON_DISPLAY )
  {
    p_wk->seq = DISPLAY_SEQ_DELETE_START;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  画面作成中かチェック
 *
 *	@param	const KEYSYS_DISPLAY_GENELATER *p_wk  ワーク
 *
 *	@return TRUEならば作成していない  FALSEならば作成中
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_DISPLAY_IsComplete( const KEYSYS_DISPLAY_GENELATER *cp_wk )
{
  return cp_wk->seq == DISPLAY_SEQ_NO_DISPLAY || cp_wk->seq == DISPLAY_SEQ_ON_DISPLAY;
}

//----------------------------------------------------------------------------
/**
 *	@brief  強制消去
 *
 *	@param	KEYSYS_DISPLAY_GENELATER *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_DISPLAY_DeleteForce( KEYSYS_DISPLAY_GENELATER *p_wk )
{
  if( p_wk->seq == DISPLAY_SEQ_ON_DISPLAY )
  {
    if( p_wk->func.Delete )
    {
      p_wk->func.Delete( p_wk->p_wk_adrs );
    }

    p_wk->seq = DISPLAY_SEQ_NO_DISPLAY;
  }
}

//=============================================================================
/**
 *    TEXTOBJ
 */
//=============================================================================
//-------------------------------------
/// 定数
//=====================================
//-------------------------------------
/// メッセージOAM
//=====================================
struct _KEYSYS_TEXTOBJ_WORK
{ 
  GFL_BMP_DATA        *p_bmp;     //文字用BMP
  PRINTSYS_LSB        color;      //文字色
  BOOL                trans_req;  //BMP転送リクエストフラグ
  GFL_POINT           ofs;        //文字描画位置オフセット
  KEYSYS_TEXT_POS     pos_type;   //描画位置タイプ
  STRBUF              *p_strbuf;  //文字バッファ
  BMPOAM_ACT_PTR      p_bmpoam_wk;  //BMPOAM
  PRINT_QUE           *p_que;       //描画キュー
};

//-------------------------------------
/// 文字描画位置計算
//=====================================

//-------------------------------------
/// パブリック
//=====================================
//----------------------------------------------------------------------------
/**
 *  @brief  TEXTOBJ  作成
 *
 *  @param  const GFL_CLWK_DATA *cp_data  OAM情報
 *  @param  w                             幅  キャラ単位
 *  @param  h                             高さ  キャラ単位
 *  @param  plt_idx                       GFL_CLGRPに登録したパレットインデックス
 *  @param  plt_ofs                       パレットオフセット
 *  @param  p_bmpoam_sys                  BMPOAMシステム
 *  @param  *p_que  メッセージ描画キュー
 *  @param  heapID  ヒープID
 *
 *  @return TEXTOBJワーク
 */
//-----------------------------------------------------------------------------
KEYSYS_TEXTOBJ_WORK * KEYSYS_TEXTOBJ_Init( const GFL_CLWK_DATA *cp_cldata, u8 w, u8 h, u32 plt_idx, u8 plt_ofs, CLSYS_DRAW_TYPE draw_type, BMPOAM_SYS_PTR p_bmpoam_sys, HEAPID heapID )
{ 
  KEYSYS_TEXTOBJ_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(KEYSYS_TEXTOBJ_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(KEYSYS_TEXTOBJ_WORK) );
  p_wk->p_que = PRINTSYS_QUE_Create( heapID );

  //バッファ作成
  p_wk->p_strbuf  = GFL_STR_CreateBuffer( 128, heapID );

  //BMP作成
  p_wk->p_bmp = GFL_BMP_Create( w, h, GFL_BMP_16_COLOR, heapID );

  //BMPOAM作成
  { 
    BMPOAM_ACT_DATA actdata;
    
    GFL_STD_MemClear( &actdata, sizeof(BMPOAM_ACT_DATA) );
    actdata.bmp = p_wk->p_bmp;
    actdata.x   = cp_cldata->pos_x;
    actdata.y   = cp_cldata->pos_y;
    actdata.pltt_index  = plt_idx;
    actdata.soft_pri    = cp_cldata->softpri;
    actdata.setSerface  = draw_type;
    actdata.draw_type   = draw_type;
    actdata.bg_pri      = cp_cldata->bgpri;
    actdata.pal_offset  = plt_ofs;
    p_wk->p_bmpoam_wk   = BmpOam_ActorAdd( p_bmpoam_sys, &actdata );
  }

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *  @brief  TEXTOBJ　破棄
 *
 *  @param  KEYSYS_TEXTOBJ_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXTOBJ_Exit( KEYSYS_TEXTOBJ_WORK* p_wk )
{ 
  BmpOam_ActorDel( p_wk->p_bmpoam_wk );
  GFL_BMP_Delete( p_wk->p_bmp );
  GFL_STR_DeleteBuffer( p_wk->p_strbuf );
  PRINTSYS_QUE_Delete( p_wk->p_que );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *  @brief  TEXTOBJ  背景色でクリア
 *
 *  @param  KEYSYS_TEXTOBJ_WORK* p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXTOBJ_Clear( KEYSYS_TEXTOBJ_WORK* p_wk )
{
  GFL_BMP_Clear( p_wk->p_bmp, PRINTSYS_LSB_GetB(p_wk->color) );
  BmpOam_ActorBmpTrans( p_wk->p_bmpoam_wk );
}

//----------------------------------------------------------------------------
/**
 *  @brief  TEXTOBJ  文字描画
 *
 *  @param  KEYSYS_TEXTOBJ_WORK* p_wk ワーク
 *  @param  *p_msg                    メッセージ
 *  @param  strID                     文字番号
 *  @param  *p_font                   フォント
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXTOBJ_Print( KEYSYS_TEXTOBJ_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font )
{ 
  GFL_POINT pos;

  //一端消去
  GFL_BMP_Clear( p_wk->p_bmp, PRINTSYS_LSB_GetB( p_wk->color ) ); 

  //文字列作成
  GFL_MSG_GetString( p_msg, strID, p_wk->p_strbuf );

  //位置計算
  KeySys_Text_CalcPosCore( p_wk->pos_type, &p_wk->ofs, p_wk->p_bmp, p_wk->p_strbuf, p_font, &pos );

  //文字列描画
  PRINTSYS_PrintQueColor( p_wk->p_que, p_wk->p_bmp, pos.x, pos.y, p_wk->p_strbuf, p_font, p_wk->color );
  p_wk->trans_req = TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  TEXTOBJ  文字描画STRBUF版
 *
 *  @param  KEYSYS_TEXTOBJ_WORK* p_wk     ワーク
 *  @param  STRBUF *cp_strbuf             文字列
 *  @param  *p_font                       フォント
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXTOBJ_PrintBuf( KEYSYS_TEXTOBJ_WORK* p_wk, const STRBUF *cp_strbuf, GFL_FONT *p_font )
{ 
  GFL_POINT pos;

  //一端消去
  GFL_BMP_Clear( p_wk->p_bmp, PRINTSYS_LSB_GetB( p_wk->color ) ); 

  //文字列作成
  GFL_STR_CopyBuffer( p_wk->p_strbuf, cp_strbuf );

  //位置計算
  KeySys_Text_CalcPosCore( p_wk->pos_type, &p_wk->ofs, p_wk->p_bmp, p_wk->p_strbuf, p_font, &pos );

  //文字列描画
  PRINTSYS_PrintQueColor( p_wk->p_que, p_wk->p_bmp, pos.x, pos.y, p_wk->p_strbuf, p_font, p_wk->color );
  p_wk->trans_req = TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  文字色を設定
 *
 *  @param  KEYSYS_TEXTOBJ_WORK* p_wk   ワーク
 *  @param  color                       文字色
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXTOBJ_SetStrColor( KEYSYS_TEXTOBJ_WORK* p_wk, PRINTSYS_LSB color )
{ 
  p_wk->color = color;
}

//----------------------------------------------------------------------------
/**
 *  @brief  文字描画位置設定
 *
 *  @param  KEYSYS_TEXTOBJ_WORK* p_wk ワーク
 *  @param  x 座標
 *  @param  y 座標
 *  @param  type  座標タイプ
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXTOBJ_SetStrPos( KEYSYS_TEXTOBJ_WORK* p_wk, s16 x, s16 y, KEYSYS_TEXT_POS type )
{ 
  p_wk->ofs.x = x;
  p_wk->ofs.y = y;
  p_wk->pos_type  = type;
}
//----------------------------------------------------------------------------
/**
 *  @brief  表示処理
 *
 *  @param  KEYSYS_TEXTOBJ_WORK* p_wk ワーク
 *
 *  @return TRUEで描画完了  FALSEで描画中
 */
//-----------------------------------------------------------------------------
void KEYSYS_TEXTOBJ_Draw( KEYSYS_TEXTOBJ_WORK* p_wk )
{ 
  PRINTSYS_QUE_Main( p_wk->p_que );

  if( p_wk->trans_req )
  { 
    if( !PRINTSYS_QUE_IsExistTarget( p_wk->p_que, p_wk->p_bmp ) )
    { 
      BmpOam_ActorBmpTrans( p_wk->p_bmpoam_wk );
      p_wk->trans_req = FALSE;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画終了
 *
 *	@param	const KEYSYS_TEXTOBJ_WORK* cp_wk  ワーク
 *
 *	@return TRUE描画完了  FALSE描画終了
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_TEXTOBJ_IsDrawEnd( const KEYSYS_TEXTOBJ_WORK* cp_wk )
{
  return !cp_wk->trans_req;
}

//----------------------------------------------------------------------------
/**
 *  @brief  BMPOAM取得
 *
 *  @param  KEYSYS_TEXTOBJ_WORK* p_wk   ワーク
 *
 *  @return BMPOAM
 */
//-----------------------------------------------------------------------------
BMPOAM_ACT_PTR KEYSYS_TEXTOBJ_GetBmpOamAct( KEYSYS_TEXTOBJ_WORK* p_wk )
{ 
  return p_wk->p_bmpoam_wk;
}
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  等速加速運動
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動初期化
 *
 *	@param	KEYSYS_VELOCITY_WORK* p_wk	ワーク
 *	@param	start						開始座標
 *	@param	end							終了座標
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void KEYSYS_VEL_Init( KEYSYS_VELOCITY_WORK* p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, int sync )
{
	p_wk->now_val	= *cp_start;
	p_wk->start_val	= *cp_start;
	p_wk->end_val	= *cp_end;
	p_wk->sync_max	= sync;
	if( sync ) {
		p_wk->add_val.x	= FX32_CONST(p_wk->end_val.x - p_wk->start_val.x) / sync;
		p_wk->add_val.y	= FX32_CONST(p_wk->end_val.y - p_wk->start_val.y) / sync;
		p_wk->sync_now	= 0;
	}else{
		//	sync == 0 の場合は即処理終了
		p_wk->sync_now	= p_wk->sync_max-2;	//(p_wk->sync_max-1)より下の値
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動メイン処理（毎フレームよぶこと）
 *
 *	@param	KEYSYS_VELOCITY_WORK* p_wk ワーク
 *
 *	@return	TRUEなら処理終了、FALSEなら処理中。
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_VEL_Main( KEYSYS_VELOCITY_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		p_wk->sync_now++;
		p_wk->now_val.x	= p_wk->start_val.x + ((p_wk->add_val.x * (p_wk->sync_now)) >> FX32_SHIFT);
		p_wk->now_val.y	= p_wk->start_val.y + ((p_wk->add_val.y * (p_wk->sync_now)) >> FX32_SHIFT);
		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_val;
		return TRUE;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  等速直線運動座標取得
 *
 *	@param	const KEYSYS_VELOCITY_WORK* cp_wk  ワーク
 *	@param	*p_pos  座標
 */
//-----------------------------------------------------------------------------
void KEYSYS_VEL_GetPos( const KEYSYS_VELOCITY_WORK* cp_wk, GFL_CLACTPOS *p_pos )
{
  p_pos->x = cp_wk->now_val.x;
  p_pos->y = cp_wk->now_val.y;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *          ETC
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//----------------------------------------------------------------------------
/**
 *	@brief　等加速運動初期化
 *
 *	@param	KEYSYS_ACLR_WORK* p_wk ワーク
 *	@param	GFL_POINT *cp_start 開始点
 *	@param	GFL_POINT *cp_end   終了点
 *	@param	velocity  初速
 *	@param	sync      かかるシンク
 */
//-----------------------------------------------------------------------------
void KEYSYS_ACLR_Init( KEYSYS_ACLR_WORK* p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, fx32 velocity, int sync )
{
	fx32 t_x_t;	// タイムの２乗
	fx32 vot;	// 初速度＊タイム
	fx32 dis;
	fx32 a;

  fx32 num;

  VEC_Set( &p_wk->now_val, FX32_CONST(cp_start->x),FX32_CONST(cp_start->y),0);
  VEC_Set( &p_wk->start_val, FX32_CONST(cp_start->x),FX32_CONST(cp_start->y),0);
  VEC_Set( &p_wk->end_val, FX32_CONST(cp_end->x),FX32_CONST(cp_end->y),0);

  dis = VEC_Distance( &p_wk->end_val, &p_wk->start_val );

  VEC_Subtract( &p_wk->end_val, &p_wk->start_val, &p_wk->vec );
  VEC_Normalize( &p_wk->vec, &p_wk->vec );

	// 加速値を求める
	// a = 2(x - vot)/(t*t)
	t_x_t	= (sync * sync) << FX32_SHIFT;
	vot		= velocity * sync;
	vot		= (dis - vot) * 2;
	a		  = FX_Div( vot, t_x_t );

	p_wk->velocity	= velocity;
	p_wk->aclr		= a;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
}
//----------------------------------------------------------------------------
/**
 *	@brief	等加速運動メイン処理（毎フレームよぶこと）
 *
 *	@param	KEYSYS_ACLR_WORK* p_wk ワーク
 *
 *	@return	TRUEなら処理終了、FALSEなら処理中。
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_ACLR_Main( KEYSYS_ACLR_WORK* p_wk )
{
	fx32 dis;
	fx32 t_x_t;
	fx32 calc_work;
	fx32 vot;

	if( p_wk->sync_now < (p_wk->sync_max-1) ) {
		p_wk->sync_now++;

		// 等加速度運動
		// dis = vot + 1/2( a*(t*t) )
		vot = FX_Mul( p_wk->velocity, p_wk->sync_now << FX32_SHIFT );
		t_x_t = (p_wk->sync_now * p_wk->sync_now) << FX32_SHIFT;
		calc_work = FX_Mul( p_wk->aclr, t_x_t );
		calc_work = calc_work / 2;	// 1/2(a*(t*t))
		dis = vot + calc_work;		///<移動距離

    VEC_MultAdd( dis, &p_wk->vec, &p_wk->start_val, &p_wk->now_val );

		return FALSE;
	}else {
		p_wk->now_val = p_wk->end_val;
		return TRUE;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief 等加速運動座標取得
 *
 *	@param	const KEYSYS_ACLR_WORK* cp_wk  ワーク
 *	@param	*p_pos  座標
 */
//-----------------------------------------------------------------------------
void KEYSYS_ACLR_GetPos( const KEYSYS_ACLR_WORK* cp_wk, GFL_CLACTPOS *p_pos )
{
  p_pos->x = cp_wk->now_val.x >> FX32_SHIFT;
  p_pos->y = cp_wk->now_val.y >> FX32_SHIFT;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *          ETC
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//----------------------------------------------------------------------------
/**
 *  @brief  パレットフェード
 *
 *  @param  NNS_GFD_DST_TYPE type 転送先
 *  @param  *p_buff               バッファ
 *  @param  cnt                   カウント
 *  @param  plt_num               パレット縦
 *  @param  plt_col               パレット横
 *  @param  start                 開始色
 *  @param  end                   終了色
 */
//-----------------------------------------------------------------------------
void KEYSYS_UTIL_MainPltAnm( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 cnt, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end )
{ 
  //0～１に変換
  const fx16 cos = (FX_CosIdx(cnt)+FX16_ONE)/2;
  const u8 start_r  = (start & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
  const u8 start_g  = (start & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
  const u8 start_b  = (start & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;
  const u8 end_r  = (end & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
  const u8 end_g  = (end & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
  const u8 end_b  = (end & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;

  const u8 r = start_r + (((end_r-start_r)*cos)>>FX16_SHIFT);
  const u8 g = start_g + (((end_g-start_g)*cos)>>FX16_SHIFT);
  const u8 b = start_b + (((end_b-start_b)*cos)>>FX16_SHIFT);


  *p_buff = GX_RGB(r, g, b);

  { 
    BOOL ret;
    ret = NNS_GfdRegisterNewVramTransferTask( type,
        plt_num * 32 + plt_col * 2,
        p_buff, 2 );

    GF_ASSERT(ret);
  }
}
