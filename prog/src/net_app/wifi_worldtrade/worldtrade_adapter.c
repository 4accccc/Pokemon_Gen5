//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   worldtrade_adapter.c
 *  @brief  GTSを移植する際のパック関数群
 *  @author Toru=Nagihashi
 *  @data   2009.08.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>
//system
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"
#include <dpw_tr.h>

//mine
#include "worldtrade_adapter.h"
//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *          構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *          プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  PPPからニックネームを登録する関数
 *
 *  @param  WORDSET* wordset  ワードセット
 *  @param  bufID             バッファID
 *  @param  POKEMON_PASO_PARAM * ppp  PPP
 */
//-----------------------------------------------------------------------------
void WT_WORDSET_RegisterPokeNickNamePPP( WORDSET* wordset, u32 bufID, const POKEMON_PASO_PARAM * ppp )
{
  POKEMON_PARAM *pp = PP_CreateByPPP( ppp, HEAPID_WORLDTRADE );
  WORDSET_RegisterPokeNickName( wordset, bufID, pp );
  GFL_HEAP_FreeMemory( pp );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ポケモンパラメータ分のメモリ確保
 *
 *  @param  HEAPID heapID   ヒープID
 *
 *  @return PP構造体分のメモリ
 */
//-----------------------------------------------------------------------------
POKEMON_PARAM *PokemonParam_AllocWork( HEAPID heapID )
{
  return GFL_HEAP_AllocMemory( heapID, POKETOOL_GetWorkSize() );
}
//----------------------------------------------------------------------------
/**
 *  @brief  ポケモンパラメータのコピー
 *
 *  @param  const POKEMON_PARAM *pp_src
 *  @param  *pp_dest
 */
//-----------------------------------------------------------------------------
void WT_PokeCopyPPtoPP( const POKEMON_PARAM *pp_src, POKEMON_PARAM *pp_dest )
{
  GFL_STD_MemCopy( pp_src, pp_dest, POKETOOL_GetWorkSize() );
}

//----------------------------------------------------------------------------
/**
 *  @brief  クリア
 *
 *  @param  GFL_BMPWIN * winGFLBMPWIN
 *  @param  trans_sw        転送フラグ
 */
//-----------------------------------------------------------------------------
void BmpMenuWinClear( GFL_BMPWIN * win, u8 trans_sw )
{
  GFL_BMPWIN_ClearScreen( win );
  switch( trans_sw )
  {
  case WINDOW_TRANS_ON:
    GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(win) );
    break;
  case WINDOW_TRANS_ON_V:
    GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(win) );
    break;
  }
}

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
STRBUF * MSGDAT_UTIL_AllocExpandString( const WORDSET *wordset, GFL_MSGDATA *MsgManager, u32 strID, HEAPID heapID )
{
  STRBUF  *src;
  STRBUF  *dst;
  src = GFL_MSG_CreateString( MsgManager, strID );
  dst = GFL_STR_CreateBuffer( 512, heapID );

  WORDSET_ExpandStr( wordset, dst, src );

  GFL_STR_DeleteBuffer( src );

  return dst;
}

//----------------------------------------------------------------------------
/**
 *  @brief  pppからppを作成し、コピーする
 *
 *  @param  const POKEMON_PASO_PARAM *ppp
 *  @param  *pp
 */
//-----------------------------------------------------------------------------
void PokeReplace( const POKEMON_PASO_PARAM *ppp, POKEMON_PARAM *pp )
{
  POKEMON_PARAM*  src = PP_CreateByPPP( ppp, HEAPID_WORLDTRADE );
  WT_PokeCopyPPtoPP( src, pp );
  GFL_HEAP_FreeMemory( src );
}


//----------------------------------------------------------------------------
/**
 *  @brief  PP2PPP
 *
 *  @param  POKEMON_PARAM *pp
 *
 *  @return PPP
 */
//-----------------------------------------------------------------------------
POKEMON_PASO_PARAM* PPPPointerGet( POKEMON_PARAM *pp )
{
  return (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst( pp );
}

//=============================================================================
/**
 *          プリントフォント周り
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  文字列の幅を返す
 *
 *  @param  WT_PRINT *wk  プリント情報
 *  @param  *buf    文字列
 *  @param  magin   マージン
 *
 *  @return 幅
 */
//-----------------------------------------------------------------------------
int FontProc_GetPrintStrWidth( WT_PRINT *wk, u8 font_idx, STRBUF *buf, int magin )
{
  return PRINTSYS_GetStrWidth( buf, wk->font, magin );
}

//----------------------------------------------------------------------------
/**
 *  @brief  文字列表示に必要な情報を初期化
 *
 *  @param  WT_PRINT *wk  ワーク
 *  @param  CONFIG *cfg   コンフィグ
 */
//-----------------------------------------------------------------------------
void WT_PRINT_Init( WT_PRINT *wk, const CONFIG *cfg )
{
  GFL_STD_MemClear( wk, sizeof(WT_PRINT) );

  wk->tcbsys  = GFL_TCBL_Init( HEAPID_WORLDTRADE, HEAPID_WORLDTRADE, 32, 32 );
  wk->cfg     = cfg;
  wk->font    = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WORLDTRADE );
  wk->que     = PRINTSYS_QUE_Create( HEAPID_WORLDTRADE );


  {
    int i;
    WT_PRINT_QUE  *p_one;
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      p_one = &wk->one[i];
      GFL_STD_MemClear( &p_one->util, sizeof(PRINT_UTIL) );
      p_one->use    = FALSE;
    }
  }

}

//----------------------------------------------------------------------------
/**
 *  @brief  文字列情報に必要な情報を破棄
 *
 *  @param  WT_PRINT *wk  ワーク
 */
//-----------------------------------------------------------------------------
void WT_PRINT_Exit( WT_PRINT *wk )
{
  PRINTSYS_QUE_Delete( wk->que );
  GFL_FONT_Delete( wk->font );
  GFL_TCBL_Exit( wk->tcbsys );

  GFL_STD_MemClear( wk, sizeof(WT_PRINT) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  文字列表情に必要な情報  メイン処理
 *
 *  @param  WT_PRINT *wk ワーク
 */
//-----------------------------------------------------------------------------
void WT_PRINT_Main( WT_PRINT *wk )
{
  {
    int i;
    WT_PRINT_QUE  *p_one;
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      p_one = &wk->one[i];
      if( p_one->use )
      {
        if( PRINT_UTIL_Trans( &p_one->util, wk->que ) )
        {
          p_one->use  = FALSE;
        }
      }

    }

    for( i = 0; i < WT_PRINT_STREAM_MAX; i++ )
    {
      if( wk->stream[i] != NULL )
      {
        PRINTSTREAM_STATE state;
        state  = PRINTSYS_PrintStreamGetState( wk->stream[i] );

        switch( state )
        { 
        case PRINTSTREAM_STATE_RUNNING:  ///< 処理実行中（文字列が流れている）

          // メッセージスキップ
          if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ||
              GFL_UI_TP_GetTrg() )
          {
            PRINTSYS_PrintStreamShortWait( wk->stream[i], 0 );
          }
          break;

        case PRINTSTREAM_STATE_PAUSE:    ///< 一時停止中（ページ切り替え待ち等）

          //改行
          if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ||
              GFL_UI_TP_GetTrg() )
          { 
            PRINTSYS_PrintStreamReleasePause( wk->stream[i] );
          }
          break;

        case PRINTSTREAM_STATE_DONE:     ///< 文字列終端まで表示完了
          PRINTSYS_PrintStreamDelete( wk->stream[i] );
          wk->stream[i] = NULL;
          break;
        }
      }
    }
  }


  PRINTSYS_QUE_Main( wk->que );
  GFL_TCBL_Main( wk->tcbsys );
}

//----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  PRINT_STREAM *stream  ストリーム
 *
 *  @return FALSEならば文字列描画終了 TRUEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL GF_MSG_PrintEndCheck( WT_PRINT *setup )
{
  int i;
  BOOL ret = FALSE;

  for( i = 0; i < WT_PRINT_STREAM_MAX; i++ )
  {

    if( setup->stream[i] )
    {
      if( PRINTSYS_PrintStreamGetState(setup->stream[i]) != PRINTSTREAM_STATE_DONE )
      {
        ret = TRUE;
      }
    }
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  GFL_BMPWIN *bmpwin  BMPWIN
 *  @param  *str  文字列
 *  @param  x     表示位置X
 *  @param  y     表示位置Y
 *  @param  WT_PRINT *setup   文字列に必要な情報
 *
 *  @return プリントストリーム
 */
//-----------------------------------------------------------------------------
void GF_STR_PrintSimple2( GFL_BMPWIN *bmpwin, u8 font_idx, STRBUF *str, int x, int y, WT_PRINT *setup, const char *cp_file, int line )
{
  int i;
  OS_TFPrintf( 3, "!!PRINT!! %s %d\n", cp_file, line );
 /* {
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      if( setup->stream[i] == NULL )
      {
        break;
      }
    }
    GF_ASSERT( i < WT_PRINT_BUFF_MAX );
  }
*/
  if( setup->stream[0] )
  { 
    PRINTSYS_PrintStreamDelete( setup->stream[0] );
    setup->stream[0] = NULL;
  }

  setup->stream[0]  = PRINTSYS_PrintStream(
        bmpwin, x, y, str, setup->font,
        MSGSPEED_GetWait(), setup->tcbsys, 0, HEAPID_WORLDTRADE, 0x0f );

}
//----------------------------------------------------------------------------
/**
 *  @brief  色つき一括表示
 *
 *  @param  GFL_BMPWIN *bmpwin  書き込むBMPWIN
 *  @param  *str  文字列
 *  @param  x     X
 *  @param  y     Y
 *  @param  color 色
 *  @param  *setup  文字列用情報
 */
//-----------------------------------------------------------------------------
void GF_STR_PrintColor( GFL_BMPWIN *bmpwin, u8 font_idx, STRBUF *str, int x, int y, int put_type, PRINTSYS_LSB color, WT_PRINT *setup )
{
  WT_PRINT_QUE  *p_one  = NULL;

  {
    int i;
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      if( !setup->one[i].use )
      {
        p_one = &setup->one[i];
      }
    }
  }

  GF_ASSERT( p_one != NULL );

  PRINT_UTIL_Setup( &p_one->util, bmpwin );
  PRINT_UTIL_PrintColor( &p_one->util, setup->que, x, y, str, setup->font, color );

  p_one->use  = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ストリームなどを一括消去
 *
 *	@param	WT_PRINT *wk  ワーク
 */
//-----------------------------------------------------------------------------
void WT_PRINT_ClearBuffer( WT_PRINT *wk )
{ 
  {
    int i;
    WT_PRINT_QUE  *p_one;
    for( i = 0; i < WT_PRINT_BUFF_MAX; i++ )
    {
      p_one = &wk->one[i];
      if( p_one->use )
      {
        p_one->use  = FALSE;
      }

    }

    for( i = 0; i < WT_PRINT_STREAM_MAX; i++ )
    {
      if( wk->stream[i] != NULL )
      {
        PRINTSYS_PrintStreamDelete( wk->stream[i] );
        wk->stream[i] = NULL;
      }
    }
  }
}
