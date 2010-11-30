//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\test\nagihashi\debug_snd_strm.c
 *	@brief  サウンドストリーム実験環境
 *	@author		Toru=Nagihashi
 *	@data		2010.11.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG
//	ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"
#include "gamesystem/game_data.h"
#include "print/wordset.h"
#include "sound\snd_strm.h"

//アーカイブ
#include "arc_def.h"
#include "debug_message.naix"
#include "font/font.naix"
#include "msg/debug/msg_d_nagihashi.h"

//モジュール
#include "debug_util.h"
#include "debug_graphic.h"

//外部公開
#include "debug_snd_strm.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	パレット
//=====================================
enum
{ 
  PLT_BG_FONT_M = 0,

  PLT_BG_FONT_S = 0,
};

//-------------------------------------
///	フレーム
//=====================================
enum
{
  BG_FRAME_TEXT_M = GFL_BG_FRAME0_M,

  BG_FRAME_BACK_S = GFL_BG_FRAME0_S,
};
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	  メインワーク
//=====================================
typedef struct 
{
  DEUBG_GRAPHIC_WORK        *p_graphic;
  DEBUG_SEQ_WORK            *p_seq;
  DEBUG_TEXT_WORK           *p_text;
  DEBUG_LIST_WORK           *p_list;
  GFL_FONT                  *p_font;
  PRINT_QUE                 *p_que;
  GFL_MSGDATA               *p_msg;
  WORDSET                   *p_word;
} DEBUG_SND_STRM_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT DEBUG_SND_STRM_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT DEBUG_SND_STRM_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT DEBUG_SND_STRM_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
/// シーケンス
//=====================================
static void SEQFUNC_FadeIn( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Select( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_FadeOut( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
//-------------------------------------
/// UTIL
//=====================================
static void UTIL_LIST_Create( DEBUG_SND_STRM_WORK *p_wk );
static void UTIL_LIST_Delete( DEBUG_SND_STRM_WORK *p_wk );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	サウンドストリームプロセス
//=====================================
const GFL_PROC_DATA DEBUG_SND_STRM_ProcData =
{
  DEBUG_SND_STRM_PROC_Init,
  DEBUG_SND_STRM_PROC_Main,
  DEBUG_SND_STRM_PROC_Exit
};

//=============================================================================
/**
 *					PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	サウンドストリームプロセス  初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_SND_STRM_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_SND_STRM_WORK *p_wk;

  //ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NAGI_DEBUG_SUB, 0x40000 );

  //プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(DEBUG_SND_STRM_WORK), HEAPID_NAGI_DEBUG_SUB );
	GFL_STD_MemClear( p_wk, sizeof(DEBUG_SND_STRM_WORK ));	


  //共通モジュールの初期化
  p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_NAGI_DEBUG_SUB );
	p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_NAGI_DEBUG_SUB );
  p_wk->p_msg     = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_nagihashi_dat, HEAPID_NAGI_DEBUG_SUB );
  p_wk->p_word    = WORDSET_Create( HEAPID_NAGI_DEBUG_SUB );

  //グラフィック
  p_wk->p_graphic = DEUBG_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, HEAPID_NAGI_DEBUG_SUB );

  //シーケンス
  p_wk->p_seq = DEBUG_SEQ_Init( p_wk, SEQFUNC_FadeIn, HEAPID_NAGI_DEBUG_SUB );

  //モジュール
  p_wk->p_text  = DEBUG_TEXT_Init( BG_FRAME_TEXT_M, PLT_BG_FONT_M, p_wk->p_que, p_wk->p_font, HEAPID_NAGI_DEBUG_SUB );

  //リソース読込み
  {
    ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_FONT, HEAPID_NAGI_DEBUG_SUB );

    //上下画面フォントパレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_MAIN_BG, PLT_BG_FONT_M*0x20, 0x20, HEAPID_NAGI_DEBUG_SUB );
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_default_nclr,
        PALTYPE_SUB_BG, PLT_BG_FONT_S*0x20, 0x20, HEAPID_NAGI_DEBUG_SUB );

    GFL_ARC_CloseDataHandle( p_handle );

    //他のBGは真っ黒
    GFL_BG_SetBackGroundColor( BG_FRAME_TEXT_M, 0 );
    GFL_BG_SetBackGroundColor( BG_FRAME_BACK_S, 0 );
    GFL_BG_LoadScreenReq( BG_FRAME_TEXT_M );
    GFL_BG_LoadScreenReq( BG_FRAME_BACK_S );
  }

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	サウンドストリームプロセス  破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_SND_STRM_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_SND_STRM_WORK  *p_wk     = p_wk_adrs;

  //モジュール破棄
  UTIL_LIST_Delete( p_wk ); //消し忘れた場合も消去
  DEBUG_TEXT_Exit( p_wk->p_text );

  //シーケンス
  DEBUG_SEQ_Exit( p_wk->p_seq );

  //グラフィック
  DEUBG_GRAPHIC_Exit( p_wk->p_graphic );

	//共通モジュールの破棄
  WORDSET_Delete( p_wk->p_word );
  GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

  if( SND_STRM_CheckSetUp() )
  {
    SND_STRM_Release();
  }

  //プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

  //ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_NAGI_DEBUG_SUB );

  return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	サウンドストリームプロセス  メイン
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_SND_STRM_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
  DEBUG_SND_STRM_WORK  *p_wk     = p_wk_adrs;


  //シーケンス
  DEBUG_SEQ_Main( p_wk->p_seq );
  if( DEBUG_SEQ_IsEnd( p_wk->p_seq ) )
  { 
    return GFL_PROC_RES_FINISH;
  }

  //描画
  DEUBG_GRAPHIC_2D_Draw( p_wk->p_graphic );
  PRINTSYS_QUE_Main( p_wk->p_que );
  DEBUG_TEXT_Main( p_wk->p_text );

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
 *    シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン
 *
 *	@param	DEBUG_SEQ_WORK *p_wk  シーケンス管理
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeIn( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
	enum
	{	
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEIN_WAIT;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_END;
		}
		break;

	case SEQ_END:
		DEBUG_SEQ_SetNext( p_seqwk, SEQFUNC_Select );
		break;

	default:
		GF_ASSERT(0);
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択
 *
 *	@param	DEBUG_SEQ_WORK *p_wk  シーケンス管理
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Select( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_START_MSG,
    SEQ_START_LIST,
    SEQ_WAIT_LIST,
    SEQ_END,
  };

  DEBUG_SND_STRM_WORK  *p_wk     = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_START_MSG:
    DEBUG_TEXT_Print( p_wk->p_text, p_wk->p_msg, STRM_STR_00, DEBUG_TEXT_TYPE_STREAM );
    (*p_seq)++;
    break;
  case SEQ_START_LIST:
    if( DEBUG_TEXT_IsEndPrint( p_wk->p_text ) )
    { 
      UTIL_LIST_Create( p_wk );
      (*p_seq)++;
    }
    break;
  case SEQ_WAIT_LIST:
    {
      u32 result  = DEBUG_LIST_Main( p_wk->p_list );

      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
      {
        (*p_seq)++;
        break;
      }

      if( result != DEBUG_LIST_SELECT_NULL )
      { 
        //リスト消去
        UTIL_LIST_Delete( p_wk );

        //選択したものを文字登録
        {
          STRBUF *p_strbuf = GFL_STR_CreateBuffer( 128, HEAPID_NAGI_DEBUG_SUB );
          STRBUF *p_textstr = GFL_MSG_CreateString( p_wk->p_msg, STRM_STR_01 );
          STRBUF *p_strmname  = SND_STRM_DEBUG_CreateString( ARCID_SOUND_STREAM, result, HEAPID_NAGI_DEBUG_SUB );
          SND_STRM_HZ hz  =   SND_STRM_DEBUG_GetHertz( ARCID_SOUND_STREAM, result );
          SND_STRM_TYPE type  = SND_STRM_DEBUG_GetType( ARCID_SOUND_STREAM, result );

          //単語登録
          WORDSET_RegisterWord( p_wk->p_word, 0, p_strmname, 0, FALSE, LANG_JAPAN );
          {
            static const int  sc_hz_data[]  =
            {
              8,11,22,32,
            };
            WORDSET_RegisterNumber( p_wk->p_word, 1, sc_hz_data[ hz ], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
          }
          {
            static const int sc_pcm_data[] =
            {
              8,
              16
            };
            WORDSET_RegisterNumber( p_wk->p_word, 2, sc_pcm_data[ type ], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
          }
          WORDSET_ExpandStr( p_wk->p_word, p_strbuf, p_textstr );

          //文字列追加
          DEBUG_TEXT_PrintBuf( p_wk->p_text, p_strbuf, DEBUG_TEXT_TYPE_STREAM );

          GFL_STR_DeleteBuffer( p_strmname );
          GFL_STR_DeleteBuffer( p_textstr );
          GFL_STR_DeleteBuffer( p_strbuf );
        }

        //選択したものを鳴らす
        if( SND_STRM_CheckSetUp() )
        {
          SND_STRM_Stop();
        }
        SND_STRM_SetUp( ARCID_SOUND_STREAM, 
            result,
            DEBUG_TEXT_TYPE_STREAM );
        SND_STRM_Play();

        //また選択へ
        *p_seq  = SEQ_START_LIST;
      }

    }
    break;
  case SEQ_END:
		DEBUG_SEQ_SetNext( p_seqwk, SEQFUNC_FadeOut );
    break;

  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト
 *
 *	@param	DEBUG_SEQ_WORK *p_wk  シーケンス管理
 *	@param	*p_seq                シーケンス変数
 *	@param	*p_wk_adrs            ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeOut( DEBUG_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
	enum
	{
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_EXIT,
	};	

	switch( *p_seq )
	{	
	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_EXIT;
		}
		break;

	case SEQ_EXIT:
		DEBUG_SEQ_End( p_seqwk );
		break;

	default:
		GF_ASSERT(0);
	}
}
//=============================================================================
/**
 *      UTIL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  リスト作成
 *
 *	@param	DEBUG_SND_STRM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_LIST_Create( DEBUG_SND_STRM_WORK *p_wk )
{
  if( p_wk->p_list == NULL )
  {
    int i;
    DEBUG_LIST_SETUP  setup;
    GFL_STD_MemClear( &setup, sizeof( DEBUG_LIST_SETUP ) );
    setup.p_msg   = p_wk->p_msg;
    setup.p_font  = p_wk->p_font;
    setup.p_que   = p_wk->p_que;
    setup.list_max  = SND_STRM_DEBUG_GetSndStrmMax();
    setup.frm       = BG_FRAME_TEXT_M;
    setup.font_plt  = PLT_BG_FONT_M;
    setup.frm_plt   = PLT_BG_FONT_M;
    setup.frm_chr   = 0;

    GF_ASSERT( SND_STRM_DEBUG_GetSndStrmMax() < DEBUG_LIST_WINDOW_MAX );

    for( i = 0; i < setup.list_max; i++ )
    {
      setup.p_strbuf[i] = SND_STRM_DEBUG_CreateString( ARCID_SOUND_STREAM, i, HEAPID_NAGI_DEBUG_SUB );
    }
    p_wk->p_list  = DEBUG_LIST_InitEx( &setup, 4, 2, 24, 12, 6, HEAPID_NAGI_DEBUG_SUB );
    for( i = 0; i < setup.list_max; i++ )
    {
      GFL_STR_DeleteBuffer( setup.p_strbuf[i] );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リスト破棄
 *
 *	@param	DEBUG_SND_STRM_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_LIST_Delete( DEBUG_SND_STRM_WORK *p_wk )
{
  if( p_wk->p_list )
  {
    DEBUG_LIST_Exit( p_wk->p_list );
    p_wk->p_list = NULL;
  }
}
#endif
