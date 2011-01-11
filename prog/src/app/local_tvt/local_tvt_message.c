//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_message.c
 *	@brief  ローカルライブキャスターメッセージ処理
 *	@author	Toru=Nagihashi
 *	@data		2011.01.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"
#include "print/wordset.h"
#include "app/app_keycursor.h"
#include "app/app_printsys_common.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

//自分のモジュール
#include "local_tvt_local_def.h"
#include "local_tvt_graphic.h"

//外部公開
#include "local_tvt_message.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define LTVT_MSG_MSGWIN_CGX (1)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
struct _LOCAL_TVT_MESSAGE
{
  GFL_TCBLSYS *p_tcblsys;
  APP_KEYCURSOR_WORK  *p_cursor;
  APP_PRINTSYS_COMMON_WORK print_wk;
  STRBUF          *p_str;
  WORDSET         *p_wordset;
  PRINT_QUE       *p_que;
  GFL_BMPWIN      *p_msgwin;
  GFL_FONT        *p_font;
  PRINT_STREAM    *p_stream;
  HEAPID          heapID;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
LOCAL_TVT_MESSAGE * LOCAL_TVT_MESSAGE_Init( HEAPID heapID )
{
  LOCAL_TVT_MESSAGE *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_MESSAGE));
  p_wk->heapID =  heapID;
  

  //フォント読み込み
  p_wk->p_font = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , p_wk->heapID );
  
  BmpWinFrame_GraphicSet( LTVT_FRAME_MESSAGE , LTVT_MSG_MSGWIN_CGX , LTVT_PLT_MAIN_WINFRAME , 0 , p_wk->heapID );
  {
    //黒いほうを読む
    u32 arc = BmpWinFrame_WinPalArcGet(MENU_TYPE_SYSTEM);
    GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FLDMAP_WINFRAME,
        arc, PALTYPE_MAIN_BG, 0x20*1, LTVT_PLT_MAIN_WINFRAME*0x20, 0x20, p_wk->heapID );
  }

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_systemwin_nclr , PALTYPE_MAIN_BG , LTVT_PLT_MAIN_FONT*0x20, 16*2, p_wk->heapID );
  GFL_FONTSYS_SetDefaultColor();
  
  p_wk->p_tcblsys = GFL_TCBL_Init( p_wk->heapID , p_wk->heapID , 3 , 0x100 );
  p_wk->p_stream = NULL;
  p_wk->p_msgwin = NULL;
  p_wk->p_str = NULL;
  p_wk->p_wordset = WORDSET_Create( p_wk->heapID );
  p_wk->p_que = PRINTSYS_QUE_Create( p_wk->heapID );

  p_wk->p_cursor = APP_KEYCURSOR_Create( 0xf , TRUE , TRUE , p_wk->heapID );
  GFL_FONTSYS_SetColor( 1,2,0xf );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ破棄
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_Exit( LOCAL_TVT_MESSAGE *p_wk )
{
  APP_KEYCURSOR_Delete( p_wk->p_cursor );
  PRINTSYS_QUE_Delete( p_wk->p_que );
  if( p_wk->p_stream != NULL )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
  }
  if( p_wk->p_str != NULL )
  {
    GFL_STR_DeleteBuffer( p_wk->p_str );
  }
  if( p_wk->p_wordset != NULL )
  {
    WORDSET_Delete( p_wk->p_wordset );
  }
  if( p_wk->p_msgwin != NULL )
  {
    GFL_BMPWIN_Delete( p_wk->p_msgwin );
  }
  GFL_FONT_Delete( p_wk->p_font );
  GFL_TCBL_Exit( p_wk->p_tcblsys );

  GFL_FONTSYS_SetDefaultColor();

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージメイン処理
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_Main( LOCAL_TVT_MESSAGE *p_wk )
{
  GFL_TCBL_Main( p_wk->p_tcblsys );
  if( p_wk->p_stream != NULL  )
  {
    APP_KEYCURSOR_Main( p_wk->p_cursor , p_wk->p_stream , p_wk->p_msgwin );
    /*
    LOCAL_TVT_CHARA *charaWork  = LOCAL_TVT_CHARA_SYS_GetWork( p_wk->chara_sys, p_wk->talkCharaNo );

    if( PRINTSYS_PrintStreamGetState( p_wk->p_stream ) != PRINTSTREAM_STATE_RUNNING )
    {
      LOCAL_TVT_CHARA_SetLipSync( charaWork, FALSE );
    }
    else
    {
      LOCAL_TVT_CHARA_SetLipSync( charaWork, TRUE );
    }
    */
    if( APP_PRINTSYS_COMMON_PrintStreamFunc( &p_wk->print_wk, p_wk->p_stream ) == TRUE )
    {
      PRINTSYS_PrintStreamDelete( p_wk->p_stream );
      p_wk->p_stream = NULL;
      GFL_STR_DeleteBuffer( p_wk->p_str );
      p_wk->p_str = NULL;
    }
  }
  PRINTSYS_QUE_Main( p_wk->p_que );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フォントの取得
 *
 *	@param	const LOCAL_TVT_MESSAGE *cp_wk ワーク
 *
 *	@return フォント
 */
//-----------------------------------------------------------------------------
GFL_FONT *LOCAL_TVT_MESSAGE_GetFont( const LOCAL_TVT_MESSAGE *cp_wk )
{
  return cp_wk->p_font;
}
//----------------------------------------------------------------------------
/**
 *	@brief  プリントキューを取得
 *
 *	@param	const LOCAL_TVT_MESSAGE *cp_wk ワーク
 *
 *	@return プリントキュー
 */
//-----------------------------------------------------------------------------
PRINT_QUE *LOCAL_TVT_MESSAGE_GetPrintQue( const LOCAL_TVT_MESSAGE *cp_wk )
{
  return cp_wk->p_que;
}
//----------------------------------------------------------------------------
/**
 *	@brief  主人公名を登録
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk   ワーク
 *	@param	GAMEDATA *cp_gamedata     ゲームデータ
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_RegisterPlayerName( LOCAL_TVT_MESSAGE *p_wk, GAMEDATA *p_gamedata )
{
  WORDSET_RegisterPlayerName( p_wk->p_wordset , 0 , GAMEDATA_GetMyStatus(p_gamedata) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウを開く（キャラの名前を書き潰すので注意）
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk  ワーク
 *	@param	pos                     開く位置
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MSGWIN_OpenWindow( LOCAL_TVT_MESSAGE *p_wk, LOCAL_TVT_MSG_POS pos )
{
  if( p_wk->p_msgwin != NULL )
  {
    BmpWinFrame_Clear( p_wk->p_msgwin , WINDOW_TRANS_ON_V );
    GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->p_msgwin );
    GFL_BMPWIN_Delete( p_wk->p_msgwin );
    p_wk->p_msgwin = NULL;
  }
  
  if( pos == LTMP_UP )
  {
    p_wk->p_msgwin = GFL_BMPWIN_Create( LTVT_FRAME_MESSAGE , 
                                      1 , 1 , 30 , 4 , 
                                      LTVT_PLT_MAIN_FONT ,
                                      GFL_BMP_CHRAREA_GET_B );
  }
  else
  if( pos == LTMP_DOWN )
  {
    p_wk->p_msgwin = GFL_BMPWIN_Create( LTVT_FRAME_MESSAGE , 
                                      1 , 19 , 30 , 4 , 
                                      LTVT_PLT_MAIN_FONT ,
                                      GFL_BMP_CHRAREA_GET_B );
  }
  
  BmpWinFrame_Write( p_wk->p_msgwin , WINDOW_TRANS_ON_V , LTVT_MSG_MSGWIN_CGX , LTVT_PLT_MAIN_WINFRAME );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_msgwin ) , 0x0f );
  GFL_BMPWIN_TransVramCharacter( p_wk->p_msgwin );
  GFL_BMPWIN_MakeScreen( p_wk->p_msgwin );
  GFL_BG_LoadScreenV_Req( LTVT_FRAME_MESSAGE );
  APP_PRINTSYS_COMMON_PrintStreamInit( &p_wk->print_wk, APP_PRINTSYS_COMMON_TYPE_BOTH );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ウィンドウを閉じる（キャラの名前を書き潰すのを復帰すること！）
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MSGWIN_CloseWindow( LOCAL_TVT_MESSAGE *p_wk )
{
  u8 i;
  if( p_wk->p_msgwin != NULL )
  {
    BmpWinFrame_Clear( p_wk->p_msgwin , WINDOW_TRANS_ON_V );
    GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->p_msgwin );
    GFL_BMPWIN_Delete( p_wk->p_msgwin );
    p_wk->p_msgwin = NULL;
  }
  if( p_wk->p_stream != NULL )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream = NULL;
  }
  if( p_wk->p_str != NULL )
  {
    GFL_STR_DeleteBuffer( p_wk->p_str );
    p_wk->p_str = NULL;
  }
/*
   LOCAL_TVT_CHARA_SYS_RedrawName( p_wk->chara_sys )
*/
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウに文字列表示
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk  ワーク
 *	@param	*p_msg                  メッセージデータ
 *	@param	u16 msgId               メッセージ番号
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MSGWIN_DispMessage( LOCAL_TVT_MESSAGE *p_wk, GFL_MSGDATA *p_msg, const u16 msgId )
{
  if( p_wk->p_stream != NULL )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream = NULL;
  }

  {
    if( p_wk->p_str != NULL )
    {
      GFL_STR_DeleteBuffer( p_wk->p_str );
      p_wk->p_str = NULL;
    }
    
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_msgwin ) , 0xf );
    p_wk->p_str = GFL_MSG_CreateString( p_msg, msgId );
    
    if( p_wk->p_wordset != NULL )
    {
      STRBUF *msgWorkStr = GFL_STR_CreateBuffer( 1024 , p_wk->heapID );
      WORDSET_ExpandStr( p_wk->p_wordset, msgWorkStr , p_wk->p_str );
      GFL_STR_DeleteBuffer( p_wk->p_str );
      p_wk->p_str = msgWorkStr;
    }
    
    p_wk->p_stream = PRINTSYS_PrintStream( p_wk->p_msgwin , 0,0, p_wk->p_str ,p_wk->p_font,
                        MSGSPEED_GetWait() , p_wk->p_tcblsys , 2 , p_wk->heapID , 0x0f );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージウィンドウの文字表示チェック
 *
 *	@param	const LOCAL_TVT_MESSAGE *cp_wk   ワーク
 *
 *	@return TRUE文字表示完了  FALSE文字表示中
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_MSGWIN_IsFinish( const LOCAL_TVT_MESSAGE *cp_wk )
{
  if( cp_wk->p_stream == NULL )
  {
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ストリーム状態がポーズかチェック
 *
 *	@param	const LOCAL_TVT_MESSAGE *cp_wk  ワーク
 *
 *	@return TRUEでポーズ  FALSEでストリーム流れ中
 */
//-----------------------------------------------------------------------------
BOOL LOCAL_TVT_MSGWIN_IsPause( const LOCAL_TVT_MESSAGE *cp_wk )
{
  return PRINTSYS_PrintStreamGetState( cp_wk->p_stream ) != PRINTSTREAM_STATE_RUNNING;
}
