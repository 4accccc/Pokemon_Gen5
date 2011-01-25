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
#include "field/zonedata.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "comm_tvt.naix"
#include "msg/msg_pokelist.h"

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
#define LTVT_FRAME_MAIN_CGX (1)
#define LTVT_FRAME_SUB_CGX  (32*10)
#define LTVT_MSG_PRINT_MAX  (0x20)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
struct _LOCAL_TVT_MESSAGE
{
  GFL_TCBLSYS               *p_tcblsys;
  APP_KEYCURSOR_WORK        *p_cursor;
  APP_PRINTSYS_COMMON_WORK  print_wk;
  HEAPID          heapID;
  STRBUF          *p_str;
  WORDSET         *p_wordset;
  PRINT_QUE       *p_que;
  GFL_BMPWIN      *p_msgwin;
  GFL_FONT        *p_font;
  GFL_FONT        *p_font_num;
  PRINT_STREAM    *p_stream;
  GFL_MSGDATA     *p_msgdata_sys;
  GFL_MSGDATA     *p_msgdata_call;
  GFL_MSGDATA     *p_msgdata_pokelist;

  PRINT_UTIL      print_util[LTVT_MSG_PRINT_MAX];

  PRINTSYS_LSB    print_color;
  s16             print_x;
  s16             print_y;
  LOCAL_TVT_MESSAGE_POS print_pos;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void Local_Tvt_Message_CalcPos( const LOCAL_TVT_MESSAGE *cp_wk, GFL_BMPWIN *p_bmpwin, const STRBUF *cp_strbuf, s16 *p_x, s16 *p_y );

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ管理システム作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_MESSAGE * LOCAL_TVT_MESSAGE_Init( HEAPID heapID )
{
  LOCAL_TVT_MESSAGE *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_MESSAGE));
  p_wk->heapID =  heapID;

  //メッセージデータ読込み
  p_wk->p_msgdata_sys = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE , NARC_message_tvt_event_system_dat, heapID );
  p_wk->p_msgdata_call  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE , NARC_message_tvt_call_dat, heapID );
  p_wk->p_msgdata_pokelist  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pokelist_dat, heapID );

  //フォント読み込み
  p_wk->p_font = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , p_wk->heapID );
  p_wk->p_font_num = GFL_FONT_Create( ARCID_FONT , NARC_font_num_gftr, GFL_FONT_LOADTYPE_FILE , FALSE , p_wk->heapID );
  
  //フレーム読込み
  BmpWinFrame_GraphicSet( LTVT_FRAME_MESSAGE , LTVT_FRAME_MAIN_CGX, LTVT_PLT_MAIN_WINFRAME , 0 , p_wk->heapID );
  BmpWinFrame_GraphicSet( LTVT_FRAME_SUB_MSG , LTVT_FRAME_SUB_CGX, LTVT_PLT_BG_SUB_WINFRAME ,
      0 , p_wk->heapID );

  {
    //黒いほうを読む
    u32 arc = BmpWinFrame_WinPalArcGet(MENU_TYPE_SYSTEM);
    GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FLDMAP_WINFRAME,
        arc, PALTYPE_MAIN_BG, 0x20*1, LTVT_PLT_MAIN_WINFRAME*0x20, 0x20, p_wk->heapID );
  }

  //フォント色読込み
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_systemwin_nclr , PALTYPE_MAIN_BG , LTVT_PLT_MAIN_FONT*0x20, 16*2, p_wk->heapID );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG  , LTVT_PLT_BG_SUB_FONT *0x20, 16*2, p_wk->heapID );
  GFL_FONTSYS_SetDefaultColor();
  
  p_wk->p_tcblsys = GFL_TCBL_Init( p_wk->heapID , p_wk->heapID , 3 , 0x100 );
  p_wk->p_stream = NULL;
  p_wk->p_msgwin = NULL;
  p_wk->p_str = GFL_STR_CreateBuffer( 1024, p_wk->heapID );
  p_wk->p_wordset = WORDSET_Create( p_wk->heapID );
  p_wk->p_que = PRINTSYS_QUE_Create( p_wk->heapID );

  p_wk->p_cursor = APP_KEYCURSOR_Create( 0xf , TRUE , TRUE , p_wk->heapID );
  GFL_FONTSYS_SetColor( 1,2,0xf );

  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0 , 
                    GX_BLEND_PLANEMASK_OBJ|
                    GX_BLEND_PLANEMASK_BG3|
                    GX_BLEND_PLANEMASK_BG2|
                    GX_BLEND_PLANEMASK_BG1 , 
                    16 , 4 );

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
  G2_BlendNone();

  if( p_wk->p_stream != NULL )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
  }
  if( p_wk->p_msgwin != NULL )
  {
    GFL_BMPWIN_Delete( p_wk->p_msgwin );
  }
  APP_KEYCURSOR_Delete( p_wk->p_cursor );
  PRINTSYS_QUE_Delete( p_wk->p_que );
  WORDSET_Delete( p_wk->p_wordset );
  GFL_STR_DeleteBuffer( p_wk->p_str );
  GFL_TCBL_Exit( p_wk->p_tcblsys );

  GFL_FONT_Delete( p_wk->p_font_num );
  GFL_FONT_Delete( p_wk->p_font );

  GFL_MSG_Delete( p_wk->p_msgdata_pokelist );
  GFL_MSG_Delete( p_wk->p_msgdata_call );
  GFL_MSG_Delete( p_wk->p_msgdata_sys );

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
  if( p_wk->p_stream != NULL && p_wk->p_msgwin != NULL  )
  {
    APP_KEYCURSOR_Main( p_wk->p_cursor , p_wk->p_stream , p_wk->p_msgwin );
    if( APP_PRINTSYS_COMMON_PrintStreamFunc( &p_wk->print_wk, p_wk->p_stream ) == TRUE )
    {
      PRINTSYS_PrintStreamDelete( p_wk->p_stream );
      p_wk->p_stream = NULL;
    }
  }

  PRINTSYS_QUE_Main( p_wk->p_que );

  {
    int i;
    for( i = 0; i < LTVT_MSG_PRINT_MAX; i++ )
    {
      PRINT_UTIL_Trans( &p_wk->print_util[i], p_wk->p_que );
    }
  }
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
 *	@brief  システム用メッセージデータを取得
 *
 *	@param	const LOCAL_TVT_MESSAGE *cp_wk  ワーク
 *
 *	@return メッセージデータ
 */
//-----------------------------------------------------------------------------
const GFL_MSGDATA *LOCAL_TVT_MESSAGE_GetSystemMsgData( const LOCAL_TVT_MESSAGE *cp_wk )
{
  return cp_wk->p_msgdata_sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出しモード用メッセージデータを取得
 *
 *	@param	const LOCAL_TVT_MESSAGE *cp_wk  ワーク
 *
 *	@return メッセージデータ
 */
//-----------------------------------------------------------------------------
const GFL_MSGDATA *LOCAL_TVT_MESSAGE_GetCallMsgData( const LOCAL_TVT_MESSAGE *cp_wk )
{
  return cp_wk->p_msgdata_call;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンリスト用メッセージデータを取得
 *
 *	@param	const LOCAL_TVT_MESSAGE *cp_wk  ワーク
 *
 *	@return メッセージデータ
 */
//-----------------------------------------------------------------------------
const GFL_MSGDATA *LOCAL_TVT_MESSAGE_GetListMsgData( const LOCAL_TVT_MESSAGE *cp_wk )
{
  return cp_wk->p_msgdata_pokelist;
}
//----------------------------------------------------------------------------
/**
 *	@brief  プリント色を設定  表示後クリアされる
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	color   色
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_SetPrintColor( LOCAL_TVT_MESSAGE *p_wk, PRINTSYS_LSB color )
{
  p_wk->print_color = color;
}
//----------------------------------------------------------------------------
/**
 *	@brief  プリント位置を設定  表示後クリアされる
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	x X位置
 *	@param	y Y位置
 *	@param  pos 基準位置
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_SetPrintPos( LOCAL_TVT_MESSAGE *p_wk, s16 x, s16 y, LOCAL_TVT_MESSAGE_POS pos )
{
  p_wk->print_x = x;
  p_wk->print_y = y;
  p_wk->print_pos = pos;
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字描画  GFL_MSGDATA版
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	*p_bmpwin   書き込むBMPWIN
 *	@param	*p_msg      メッセージデータ
 *	@param	u16 msgId   メッセージ番号
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_PrintMsg( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin, const GFL_MSGDATA *cp_msg, const u16 msgId )
{
  STRBUF  *p_str  = GFL_MSG_CreateString( cp_msg, msgId );
  LOCAL_TVT_MESSAGE_PrintStr( p_wk, p_bmpwin, p_str );
  GFL_STR_DeleteBuffer( p_str );
}
//----------------------------------------------------------------------------
/**
 *	@brief  文字描画  STRBUF版
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	*p_bmpwin   書き込むBMPWIN
 *	@param  strbuf      文字列
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_PrintStr( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin, const STRBUF *cp_strbuf )
{
  PRINT_UTIL  *p_util = NULL;

  //PRINT_UTILを探す
  {
    int i;
    for( i = 0; i < LTVT_MSG_PRINT_MAX; i++ )
    {
      if( p_wk->print_util[i].transReq == FALSE )
      {
        p_util  = &p_wk->print_util[i];
      }
    }

    if( p_util == NULL )
    {
      GF_ASSERT(0);
    }
  }

  //PRINT_UTIL設定
  PRINT_UTIL_Setup( p_util, p_bmpwin );

  //単語設定
  WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_str, cp_strbuf );

  //背面塗る
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_bmpwin), PRINTSYS_LSB_GetB(p_wk->print_color) );

  {
    s16 x;
    s16 y;

    //表示座標計算
    Local_Tvt_Message_CalcPos( p_wk, p_bmpwin, p_wk->p_str, &x, &y );

    //書き込み
    PRINT_UTIL_PrintColor( p_util, p_wk->p_que, x, y, p_wk->p_str, p_wk->p_font, p_wk->print_color );
  }

  //装飾情報クリア
  LOCAL_TVT_MESSAGE_SetPrintPos( p_wk, 0, 0, LOCAL_TVT_MESSAGE_POS_ABSOLUTE );
  LOCAL_TVT_MESSAGE_SetPrintColor( p_wk, LOCAL_TVT_MSG_COLOR_WHITE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモンステータスを書き込み
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	*p_bmpwin       書き込むBMPWIN
 *	@param	const POKEMON_PARAM *cp_pp  ポケモンデータ
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_PrintPokeStatus( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin, const POKEMON_PARAM *cp_pp )
{
  PRINT_UTIL  *p_util = NULL;

  //PRINT_UTILを探す
  {
    int i;
    for( i = 0; i < LTVT_MSG_PRINT_MAX; i++ )
    {
      if( p_wk->print_util[i].transReq == FALSE )
      {
        p_util  = &p_wk->print_util[i];
      }
    }

    if( p_util == NULL )
    {
      GF_ASSERT(0);
    }
  }

  //PRINT_UTIL設定
  PRINT_UTIL_Setup( p_util, p_bmpwin );
  p_util->transReq  = TRUE; //１枚のBMPWINに複数回書きこむのでここでON

  //背面塗る
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_bmpwin), PRINTSYS_LSB_GetB(p_wk->print_color) );

  //ニックネーム
  {
    STRBUF  *p_str_name  = GFL_MSG_CreateString( p_wk->p_msgdata_pokelist, mes_pokelist_01_09 );
    WORDSET_RegisterPokeNickName( p_wk->p_wordset, 0, cp_pp );
    WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_str, p_str_name );
    PRINTSYS_PrintQueColor( p_wk->p_que, GFL_BMPWIN_GetBmp(p_bmpwin), 32, 8, p_wk->p_str, p_wk->p_font, LOCAL_TVT_MSG_COLOR_WHITE );
    GFL_STR_DeleteBuffer( p_str_name );
  }

  //LEVEL
  {
    STRBUF  *p_str_lv  = GFL_MSG_CreateString( p_wk->p_msgdata_pokelist, mes_pokelist_01_03 );
    u32 level = PP_Get(cp_pp,ID_PARA_level,NULL);
    WORDSET_RegisterNumber( p_wk->p_wordset, 0, level, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );

    WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_str, p_str_lv );
    PRINTSYS_PrintQueColor( p_wk->p_que, GFL_BMPWIN_GetBmp(p_bmpwin), 0, 24, p_wk->p_str, p_wk->p_font_num, LOCAL_TVT_MSG_COLOR_WHITE );
    GFL_STR_DeleteBuffer( p_str_lv );
  }

  //性別
  {
    //sex
    switch( PP_Get(cp_pp,ID_PARA_sex,NULL) )
    {
    case PTL_SEX_MALE:
      GFL_MSG_GetString( p_wk->p_msgdata_pokelist, mes_pokelist_01_28, p_wk->p_str );
      PRINTSYS_PrintQueColor( p_wk->p_que, GFL_BMPWIN_GetBmp(p_bmpwin), 12*8, 8, p_wk->p_str, p_wk->p_font, LOCAL_TVT_MSG_COLOR_BLUE );
      break;
    case PTL_SEX_FEMALE:
      GFL_MSG_GetString( p_wk->p_msgdata_pokelist, mes_pokelist_01_29, p_wk->p_str );
      PRINTSYS_PrintQueColor( p_wk->p_que, GFL_BMPWIN_GetBmp(p_bmpwin), 12*8, 8, p_wk->p_str, p_wk->p_font, LOCAL_TVT_MSG_COLOR_RED );
      break;
    }
  }
}
//---------------------------------------------------------------------------
/**
 *	@brief  フレームを作成
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	*p_bmpwin               BMPWIN
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_WriteFrame( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin )
{
  if( GFL_BMPWIN_GetFrame(p_bmpwin) < GFL_BG_FRAME0_S )
  {
    //main
    BmpWinFrame_Write( p_bmpwin, WINDOW_TRANS_OFF , 
                      LTVT_FRAME_MAIN_CGX, LTVT_PLT_MAIN_WINFRAME );
  }
  else
  {
    //sub
    BmpWinFrame_Write( p_bmpwin, WINDOW_TRANS_OFF , 
                      LTVT_FRAME_SUB_CGX, LTVT_PLT_BG_SUB_WINFRAME );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フレームを破棄
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	*p_bmpwin               BMPWIN
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_ClearFrame( LOCAL_TVT_MESSAGE *p_wk, GFL_BMPWIN *p_bmpwin )
{
  BmpWinFrame_Clear( p_bmpwin, WINDOW_TRANS_OFF );
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
 *	@brief  ポケモンニックネームを登録
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	POKEMON_PARAM *cp_pp    ポケモンぱらむ
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_RegisterPokeNickName( LOCAL_TVT_MESSAGE *p_wk, const POKEMON_PARAM *cp_pp, u16 buffID )
{
  WORDSET_RegisterPokeNickName( p_wk->p_wordset , buffID, cp_pp );
}

//----------------------------------------------------------------------------
/**
 *	@brief  文字列を登録
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	GFL_MSGDATA *cp_msg メッセージ
 *	@param	msgID ID
 *	@param	buffID  バッファ
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_RegisterWord( LOCAL_TVT_MESSAGE *p_wk, const GFL_MSGDATA *cp_msg, u16 msgID, u16 buffID )
{
  STRBUF  *p_str  = GFL_MSG_CreateString( cp_msg, msgID );
  WORDSET_RegisterWord( p_wk->p_wordset, buffID, p_str, PTL_SEX_UNKNOWN, TRUE, LANG_JAPAN );
  GFL_STR_DeleteBuffer( p_str );
}
//----------------------------------------------------------------------------
/**
 *	@brief  地名を登録
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param	*p_gamedata ゲームデータ
 *	@param	buffID  バッファID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_RegisterPlaceName( LOCAL_TVT_MESSAGE *p_wk, GAMEDATA *p_gamedata, u16 buffID )
{

  PLAYER_WORK *p_player = GAMEDATA_GetMyPlayerWork( p_gamedata );
  u16 zoneID  = PLAYERWORK_getZoneID(p_player);
  u16 placeID = ZONEDATA_GetPlaceNameID(zoneID);
  WORDSET_RegisterPlaceName( p_wk->p_wordset, buffID, placeID );
}
//----------------------------------------------------------------------------
/**
 *	@brief  数値登録
 *
 *	@param	LOCAL_TVT_MESSAGE *p_wk ワーク
 *	@param  num     数値
 *	@param  keta    桁
 *	@param	buffID  バッファID
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_MESSAGE_RegisterNumber( LOCAL_TVT_MESSAGE *p_wk, u32 num, u32 keta, u16 buffID )
{
  WORDSET_RegisterNumber( p_wk->p_wordset, buffID, num, keta, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
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
  if( p_wk->p_msgwin == NULL )
  {

    if( pos == LTMP_UP )
    {
      p_wk->p_msgwin = GFL_BMPWIN_Create( LTVT_FRAME_MESSAGE , 
          1 , 1 , 30 , 4 , 
          LTVT_PLT_MAIN_FONT ,
          GFL_BMP_CHRAREA_GET_B );
    }
    else if( pos == LTMP_DOWN )
    {
      p_wk->p_msgwin = GFL_BMPWIN_Create( LTVT_FRAME_MESSAGE , 
          1 , 19 , 30 , 4 , 
          LTVT_PLT_MAIN_FONT ,
          GFL_BMP_CHRAREA_GET_B );
    }
  
    BmpWinFrame_Write( p_wk->p_msgwin , WINDOW_TRANS_ON_V , LTVT_FRAME_MAIN_CGX, LTVT_PLT_MAIN_WINFRAME );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_msgwin ) , 0x0f );
    GFL_BMPWIN_TransVramCharacter( p_wk->p_msgwin );
    GFL_BMPWIN_MakeScreen( p_wk->p_msgwin );
    GFL_BG_LoadScreenV_Req( LTVT_FRAME_MESSAGE );
  }

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
void LOCAL_TVT_MSGWIN_DispMessage( LOCAL_TVT_MESSAGE *p_wk, const GFL_MSGDATA *cp_msg, const u16 msgId )
{
  if( p_wk->p_stream != NULL )
  {
    PRINTSYS_PrintStreamDelete( p_wk->p_stream );
    p_wk->p_stream = NULL;
  }

  {
    
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_wk->p_msgwin ) , 0xf );
    GFL_MSG_GetString( cp_msg, msgId, p_wk->p_str );
    
    if( p_wk->p_wordset != NULL )
    {
      STRBUF *msgWorkStr = GFL_MSG_CreateString( cp_msg, msgId );
      WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_str, msgWorkStr );
      GFL_STR_DeleteBuffer( msgWorkStr );
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

//----------------------------------------------------------------------------
/**
 *	@brief  文字表示位置計算
 *	@param	const LOCAL_TVT_MESSAGE *cp_wk  ワーク
 *	@param	*p_bmpwin BMPWIN
 *	@param	STRBUF *cp_strbuf 文字列
 *	@param	*p_x  計算後の座標X
 *	@param	*p_y  計算後の座標Y
 */
//-----------------------------------------------------------------------------
static void Local_Tvt_Message_CalcPos( const LOCAL_TVT_MESSAGE *cp_wk, GFL_BMPWIN *p_bmpwin, const STRBUF *cp_strbuf, s16 *p_x, s16 *p_y )
{
  switch( cp_wk->print_pos )
  {
  case LOCAL_TVT_MESSAGE_POS_ABSOLUTE: //絶対座標
    *p_x  = cp_wk->print_x;
    *p_y  = cp_wk->print_y;
    break;
  case LOCAL_TVT_MESSAGE_POS_CENTER:   //中心座標
    { 
      u32 x, y;
      x = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(p_bmpwin) ) / 2;
      y = GFL_BMP_GetSizeY( GFL_BMPWIN_GetBmp(p_bmpwin) ) / 2;
      x -= PRINTSYS_GetStrWidth( cp_strbuf, cp_wk->p_font, 0 ) / 2;
      y -= PRINTSYS_GetStrHeight( cp_strbuf, cp_wk->p_font ) / 2;
      *p_x  = x + cp_wk->print_x;
      *p_y  = y + cp_wk->print_y;
    }
    break;
  default:
    GF_ASSERT(0);
  }
}
