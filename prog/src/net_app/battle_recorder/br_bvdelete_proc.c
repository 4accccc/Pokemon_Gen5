//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_bvdelete_proc.c
 *	@brief	バトルビデオ消去プロセス
 *	@author	Toru=Nagihashi
 *	@data		2009.11.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID

//アーカイブ
#include "msg/msg_battle_rec.h"

//内部モジュール
#include "br_util.h"

//外部参照
#include "br_bvdelete_proc.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	MSGWIN個数
//=====================================
enum
{
  BR_BVDELETE_MSGWINID_S_YES,
  BR_BVDELETE_MSGWINID_S_NO,
  BR_BVDELETE_MSGWINID_S_MAX,
};


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	バトルビデオ消去メインワーク
//=====================================
typedef struct 
{
  PRINT_QUE             *p_que;   //プリントキュー
  BR_TEXT_WORK          *p_text;  //テキスト
  WORDSET               *p_word;
  BR_SEQ_WORK           *p_seq;
  BR_MSGWIN_WORK        *p_msgwin_s[ BR_BVDELETE_MSGWINID_S_MAX ];
  BR_BALLEFF_WORK       *p_balleff;
	HEAPID                heapID;
  BR_BVDELETE_PROC_PARAM	*p_param;
} BR_BVDELETE_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	BRコアプロセス
//=====================================
static GFL_PROC_RESULT BR_BVDELETE_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_BVDELETE_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_BVDELETE_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
///	シーケンス
//=====================================
static void Br_BvDelete_Seq_FadeIn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BvDelete_Seq_FadeOut( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void Br_BvDelete_Seq_Main( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	画面作成
//=====================================
static void Br_BvDelete_CreateDisplay( BR_BVDELETE_WORK *p_wk, BR_BVDELETE_PROC_PARAM *p_param );
static void Br_BvDelete_DeleteDisplay( BR_BVDELETE_WORK *p_wk, BR_BVDELETE_PROC_PARAM *p_param );

//-------------------------------------
///	その他
//=====================================
static BOOL Br_Record_GetTrgYes( u32 x, u32 y );
static BOOL Br_Record_GetTrgNo( u32 x, u32 y );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	バトルビデオ消去プロセスプロセス
//=====================================
const GFL_PROC_DATA BR_BVDELETE_ProcData =
{	
	BR_BVDELETE_PROC_Init,
	BR_BVDELETE_PROC_Main,
	BR_BVDELETE_PROC_Exit,
};

//=============================================================================
/**
 *					データ
 */
//=============================================================================

//=============================================================================
/**
 *					バトルビデオ消去プロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオ消去プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVDELETE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_BVDELETE_WORK				*p_wk;
	BR_BVDELETE_PROC_PARAM	*p_param	= p_param_adrs;

  GF_ASSERT( p_param->mode < BR_BVDELETE_MODE_MAX );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_BVDELETE_WORK), BR_PROC_SYS_GetHeapID( p_param->p_procsys ) );
	GFL_STD_MemClear( p_wk, sizeof(BR_BVDELETE_WORK) );	
  p_wk->p_param = p_param;
	p_wk->heapID	= BR_PROC_SYS_GetHeapID( p_param->p_procsys );

  //モジュール初期化
  p_wk->p_seq = BR_SEQ_Init( p_wk, Br_BvDelete_Seq_FadeIn, p_wk->heapID );
  p_wk->p_que = PRINTSYS_QUE_Create( p_wk->heapID );
  p_wk->p_word= WORDSET_Create( p_wk->heapID );

	//グラフィック初期化
  Br_BvDelete_CreateDisplay( p_wk, p_param );
  p_wk->p_balleff = BR_BALLEFF_Init( p_param->p_unit, p_param->p_res, CLSYS_DRAW_MAIN, p_wk->heapID );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオ消去プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVDELETE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_BVDELETE_WORK				*p_wk	= p_wk_adrs;
	BR_BVDELETE_PROC_PARAM	*p_param	= p_param_adrs;

  //グラフィック破棄
  BR_BALLEFF_Exit( p_wk->p_balleff );
  Br_BvDelete_DeleteDisplay( p_wk, p_param );

	//モジュール破棄
  WORDSET_Delete( p_wk->p_word );
  PRINTSYS_QUE_Delete( p_wk->p_que );
  BR_SEQ_Exit( p_wk->p_seq );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルビデオ消去プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_BVDELETE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	

	BR_BVDELETE_WORK	*p_wk	= p_wk_adrs;

  //シーケンス管理
  BR_SEQ_Main( p_wk->p_seq );
  if( BR_SEQ_IsEnd( p_wk->p_seq ) )
  { 
    return GFL_PROC_RES_FINISH;
  }

  //ボール処理
  BR_BALLEFF_Main( p_wk->p_balleff );

  //表示
  PRINTSYS_QUE_Main( p_wk->p_que );
  if( p_wk->p_text )
  { 
    BR_TEXT_PrintMain( p_wk->p_text );
  }

  //メッセージウィンドウ
  {
    int i;
    for( i = 0; i < BR_BVDELETE_MSGWINID_S_MAX; i++ )
    {
      if( p_wk->p_msgwin_s[i] )
      { 
        BR_MSGWIN_PrintMain( p_wk->p_msgwin_s[i] );
      }
    }
  }

	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *  シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  フェードイン
 *
 *	@param	BR_SEQ_WORK *p_seqwk    シーケンスワーク
 *	@param	*p_seq                  シーケンス変数
 *	@param	*p_wk_adrs              ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvDelete_Seq_FadeIn( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_FADEIN_START,
    SEQ_FADEIN_WAIT,
    SEQ_FADEIN_END,
  };

  BR_BVDELETE_WORK	*p_wk	= p_wk_adrs;

	//プロセス処理
  switch( *p_seq )
  { 
  case SEQ_FADEIN_START:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN );
    *p_seq  = SEQ_FADEIN_WAIT;
    break;

  case SEQ_FADEIN_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_FADEIN_END;
    }
    break;

  case SEQ_FADEIN_END:
    BR_SEQ_SetNext( p_seqwk, Br_BvDelete_Seq_Main );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウト
 *
 *	@param	BR_SEQ_WORK *p_seqwk    シーケンスワーク
 *	@param	*p_seq                  シーケンス変数
 *	@param	*p_wk_adrs              ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvDelete_Seq_FadeOut( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_FADEOUT_START,
    SEQ_FADEOUT_WAIT,
    SEQ_FADEOUT_END,
  };

  BR_BVDELETE_WORK	*p_wk	= p_wk_adrs;

  //プロセス処理
  switch( *p_seq )
  { 
  case SEQ_FADEOUT_START:
    if( p_wk->p_param->is_delete )
    { 
      //消去してならばサブ画面はすでにFADEしているので、メインだけ
      BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_MAIN, BR_FADE_DIR_OUT );
    }
    else
    { 
      BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT );
    }
    *p_seq  = SEQ_FADEOUT_WAIT;
    break;

  case SEQ_FADEOUT_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      *p_seq  = SEQ_FADEOUT_END;
    }
    break;

  case SEQ_FADEOUT_END:
    BR_PROC_SYS_Pop( p_wk->p_param->p_procsys );
    BR_SEQ_End( p_seqwk );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バトルビデオ消去選択
 *
 *	@param	BR_SEQ_WORK *p_seqwk    シーケンスワーク
 *	@param	*p_seq                  シーケンス変数
 *	@param	*p_wk_adrs              ワークアドレス
 */
//-----------------------------------------------------------------------------
static void Br_BvDelete_Seq_Main( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_YESNO,
    SEQ_CONFIRM,

    SEQ_FADEOUT_SUB_INIT,
    SEQ_FADEOUT_SUB_WAIT,

    SEQ_DELETE_INIT,
    SEQ_DELETE_MAIN,
    SEQ_DELETE_MSG,
    SEQ_DELETE_MSG_WAIT,

    SEQ_RETURN,
  };

  BR_BVDELETE_WORK  *p_wk = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_YESNO:
    { 
      u32 x, y;
      if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      { 
        if( Br_Record_GetTrgYes( x, y ) )
        { 
          BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_014 );
          *p_seq  = SEQ_CONFIRM;
          break;
        }

        if( Br_Record_GetTrgNo( x, y ) )
        { 
          *p_seq  = SEQ_RETURN;
          break;
        }
      }
    }
    break;

  case SEQ_CONFIRM:
    { 
      u32 x, y;
      if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      { 
        if( Br_Record_GetTrgYes( x, y ) )
        { 
          *p_seq  = SEQ_FADEOUT_SUB_INIT;
          break;
        }

        if( Br_Record_GetTrgNo( x, y ) )
        { 
          *p_seq  = SEQ_RETURN;
          break;
        }
      }
    }
    break;


  case SEQ_FADEOUT_SUB_INIT:
    BR_FADE_StartFade( p_wk->p_param->p_fade, BR_FADE_TYPE_ALPHA_BG012OBJ, BR_FADE_DISPLAY_SUB, BR_FADE_DIR_OUT );
    (*p_seq)++;
    break;

  case SEQ_FADEOUT_SUB_WAIT:
    if( BR_FADE_IsEnd( p_wk->p_param->p_fade ) )
    { 
      BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_015 );


      {
        GFL_POINT pos;
        pos.x = 256/2;
        pos.y = 192/2;
        BR_BALLEFF_StartMove( p_wk->p_balleff, BR_BALLEFF_MOVE_BIG_CIRCLE, &pos );
      }

      (*p_seq)++;
    }
    break;

  case SEQ_DELETE_INIT:
    { 
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
      BattleRec_SaveDataEraseStart(p_sv, p_wk->heapID, p_wk->p_param->mode);
    }
    (*p_seq)++;
    break;

  case SEQ_DELETE_MAIN:
    { 
      SAVE_RESULT result;
      SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_wk->p_param->p_gamedata );
      result  = BattleRec_SaveDataEraseMain( p_sv, p_wk->p_param->mode );

      if( SAVE_RESULT_OK == result || SAVE_RESULT_NG == result )
      { 
        p_wk->p_param->is_delete  = TRUE;
        *p_seq  = SEQ_DELETE_MSG;
      }
    }
    break;

  case SEQ_DELETE_MSG:
    BR_BALLEFF_StartMove( p_wk->p_balleff, BR_BALLEFF_MOVE_NOP, NULL );
    BR_TEXT_Print( p_wk->p_text, p_wk->p_param->p_res, msg_info_016 );
    *p_seq  = SEQ_DELETE_MSG_WAIT;
    break;
  case SEQ_DELETE_MSG_WAIT:
    if( GFL_UI_TP_GetTrg() )
    { 
      *p_seq  = SEQ_RETURN;
    }
    break;

  case SEQ_RETURN:
    BR_SEQ_SetNext( p_seqwk, Br_BvDelete_Seq_FadeOut );
    break;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ワーク
 *
 *	@param	BR_BVDELETE_WORK *p_wk  ワーク
 *	@param	*p_param                引数
 */
//-----------------------------------------------------------------------------
static void Br_BvDelete_CreateDisplay( BR_BVDELETE_WORK *p_wk, BR_BVDELETE_PROC_PARAM *p_param )
{
  //上画面はテキストだけ
  if( p_wk->p_text == NULL )
  { 
    p_wk->p_text  = BR_TEXT_Init( p_param->p_res, p_wk->p_que, p_wk->heapID );
  }

  { 
    GFL_MSGDATA *p_msg  = BR_RES_GetMsgData( p_param->p_res );
    STRBUF  *p_src  = GFL_MSG_CreateString( p_msg, msg_info_013 );
    STRBUF  *p_dst  = GFL_STR_CreateCopyBuffer( p_src, p_wk->heapID );

    WORDSET_RegisterWord( p_wk->p_word, 0, 
        p_param->cp_rec_saveinfo->p_name[ p_param->mode ],
        p_param->cp_rec_saveinfo->sex[ p_param->mode ],
        TRUE, PM_LANG );

    WORDSET_ExpandStr( p_wk->p_word, p_dst, p_src );

    BR_TEXT_PrintBuff( p_wk->p_text, p_param->p_res, p_dst );

    GFL_STR_DeleteBuffer( p_dst );
    GFL_STR_DeleteBuffer( p_src );
  }

  //下画面は専用BG
  BR_RES_LoadBG( p_param->p_res, BR_RES_BG_BVDELETE_S, p_wk->heapID );

  { 
    static const struct 
    { 
      u8 x;
      u8 y;
      u8 w;
      u8 h;
      u32 msgID;
    } sc_msgwin_data[BR_BVDELETE_MSGWINID_S_MAX]  =
    { 
      {
        4,
        6,
        9,
        2,
        msg_1000,
      },
      { 
        18,
        6,
        9,
        2,
        msg_1001,
      },
    };
    int i;

    GFL_FONT    *p_font  = BR_RES_GetFont( p_param->p_res );
    GFL_MSGDATA *p_msg   = BR_RES_GetMsgData( p_param->p_res );
    for( i = 0; i < BR_BVDELETE_MSGWINID_S_MAX; i++ )
    { 
      p_wk->p_msgwin_s[i]  = BR_MSGWIN_Init( BG_FRAME_S_FONT, sc_msgwin_data[i].x, sc_msgwin_data[i].y, sc_msgwin_data[i].w, sc_msgwin_data[i].h, PLT_BG_S_FONT, p_wk->p_que, p_wk->heapID );
      BR_MSGWIN_PrintColor( p_wk->p_msgwin_s[i], p_msg, sc_msgwin_data[i].msgID, p_font, BR_PRINT_COL_NORMAL );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワーク
 *
 *	@param	BR_BVDELETE_WORK *p_wk  ワーク
 *	@param	*p_param                引数
 */
//-----------------------------------------------------------------------------
static void Br_BvDelete_DeleteDisplay( BR_BVDELETE_WORK *p_wk, BR_BVDELETE_PROC_PARAM *p_param )
{ 
  //メッセージウィンドウ
  {
    int i;
    for( i = 0; i < BR_BVDELETE_MSGWINID_S_MAX; i++ )
    {
      if( p_wk->p_msgwin_s[i] )
      { 
        BR_MSGWIN_Exit( p_wk->p_msgwin_s[i] );
        p_wk->p_msgwin_s[i] = NULL;
      }
    }
  }
  GFL_BG_LoadScreenReq( BG_FRAME_S_FONT );

  //下画面破棄
  BR_RES_UnLoadBG( p_param->p_res, BR_RES_BG_BVDELETE_S );

  //上画面破棄
  if( p_wk->p_text )
  { 
    BR_TEXT_Exit( p_wk->p_text, p_param->p_res );
    p_wk->p_text  = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  はいを選択
 *
 *	@param	x     X座標
 *	@param	y     Y座標
 *
 *	@return TRUE入力  FALSE何もしない
 */
//-----------------------------------------------------------------------------
static BOOL Br_Record_GetTrgYes( u32 x, u32 y )
{ 
	GFL_RECT rect;

	rect.left		= (4)*8;
	rect.right	= (4 + 9)*8;
	rect.top		= (6)*8;
	rect.bottom	= (6 + 2)*8;

  return ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));
}
//----------------------------------------------------------------------------
/**
 *	@brief  いいえを選択
 *
 *	@param	x     X座標
 *	@param	y     Y座標
 *
 *	@return TRUE入力  FALSE何もしない
 */
//-----------------------------------------------------------------------------
static BOOL Br_Record_GetTrgNo( u32 x, u32 y )
{ 
	GFL_RECT rect;

	rect.left		= (18)*8;
	rect.right	= (18 + 9)*8;
	rect.top		= (6)*8;
	rect.bottom	= (6 + 2)*8;

  return ( ((u32)( x - rect.left) <= (u32)(rect.right - rect.left))
            & ((u32)( y - rect.top) <= (u32)(rect.bottom - rect.top)));
}
