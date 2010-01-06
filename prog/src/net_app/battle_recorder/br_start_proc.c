//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_start_proc.c
 *	@brief	起動プロセス
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

//バトルレコーダー内モジュール
#include "br_util.h"
#include "br_inner.h"

//アーカイブ
#include "msg/msg_battle_rec.h"

//外部参照
#include "br_start_proc.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;	//関数型作るため前方宣言
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_wk_adrs );
struct _SEQ_WORK
{
	SEQ_FUNCTION	seq_function;		//実行中のシーケンス関数
	BOOL is_end;									//シーケンスシステム終了フラグ
	int seq;											//実行中のシーケンス関数の中のシーケンス
	void *p_wk_adrs;							//実行中のシーケンス関数に渡すワーク
};

//-------------------------------------
///	起動メインワーク
//=====================================
typedef struct 
{
  //シーケンス管理
  SEQ_WORK        seq;

  //ヒープID
	HEAPID          heapID;

  //BMPWIN
  BR_MSGWIN_WORK  *p_here;

  //プリントキュー
  PRINT_QUE       *p_que;

  //汎用カウンタ
  u32             cnt;


  //引数
  BR_START_PROC_PARAM *p_param;
} BR_START_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	起動プロセス
//=====================================
static GFL_PROC_RESULT BR_START_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_START_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT BR_START_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//-------------------------------------
///	SEQ
//=====================================
static void SEQ_Init( SEQ_WORK *p_wk, void *p_wk_adrs, SEQ_FUNCTION seq_function );
static void SEQ_Exit( SEQ_WORK *p_wk );
static void SEQ_Main( SEQ_WORK *p_wk );
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk );
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( SEQ_WORK *p_wk );

//-------------------------------------
///	SEQ
//=====================================
static void SEQFUNC_Open( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_Close( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_None( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	起動プロセスプロセス
//=====================================
const GFL_PROC_DATA BR_START_ProcData =
{	
	BR_START_PROC_Init,
	BR_START_PROC_Main,
	BR_START_PROC_Exit,
};

//=============================================================================
/**
 *					データ
 */
//=============================================================================

//=============================================================================
/**
 *					起動プロセス
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	起動プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_START_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_START_WORK				*p_wk;
	BR_START_PROC_PARAM	*p_param	= p_param_adrs;

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(BR_START_WORK), BR_PROC_SYS_GetHeapID( p_param->p_procsys ) );
	GFL_STD_MemClear( p_wk, sizeof(BR_START_WORK) );	
  p_wk->p_param = p_param;
	p_wk->heapID	= BR_PROC_SYS_GetHeapID( p_param->p_procsys );

	//グラフィック初期化

  //モジュール作成
  p_wk->p_que       = PRINTSYS_QUE_Create( p_wk->heapID );
  { 
    SEQ_FUNCTION  seq_function;
    if( p_param->mode == BR_START_PROC_MODE_OPEN )
    { 
      seq_function  = SEQFUNC_Open;
    }
    else if( p_param->mode == BR_START_PROC_MODE_CLOSE )
    { 
      seq_function  = SEQFUNC_Close;
    }
    else
    { 
      seq_function  = SEQFUNC_None;
    }
    SEQ_Init( &p_wk->seq, p_wk, seq_function );
  }

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	起動プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_START_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_START_WORK				*p_wk	= p_wk_adrs;
	BR_START_PROC_PARAM	*p_param	= p_param_adrs;

	//モジュール破棄
  PRINTSYS_QUE_Delete( p_wk->p_que );
  SEQ_Exit( &p_wk->seq );

  //グラフィック破棄

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	起動プロセス	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BR_START_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	BR_START_WORK	*p_wk	= p_wk_adrs;
	BR_START_PROC_PARAM	*p_param	= p_param_adrs;

  //シーケンス
	SEQ_Main( &p_wk->seq );

  //文字描画
  PRINTSYS_QUE_Main( p_wk->p_que );

	//終了
	if( SEQ_IsEnd( &p_wk->seq ) )
	{	
		return GFL_PROC_RES_FINISH;
	}
	else
	{	
		return GFL_PROC_RES_CONTINUE;
	}
}

//=============================================================================
/**
 *						SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	初期化
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	*p_param				パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Init( SEQ_WORK *p_wk, void *p_wk_adrs, SEQ_FUNCTION seq_function )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );

	//初期化
	p_wk->p_wk_adrs	= p_wk_adrs;

	//セット
	SEQ_SetNext( p_wk, seq_function );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	破棄
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void SEQ_Exit( SEQ_WORK *p_wk )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	メイン処理
 *
 *	@param	SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Main( SEQ_WORK *p_wk )
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
 *	@param	const SEQ_WORK *cp_wk		ワーク
 *
 *	@return	TRUEならば終了	FALSEならば処理中
 */	
//-----------------------------------------------------------------------------
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk )
{	
	return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	次のシーケンスを設定
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( SEQ_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}
//=============================================================================
/**
 *					SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	開始
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Open( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_FADE_IN,
    SEQ_FADE_WAIT,
    SEQ_TOUCH,
    SEQ_FADESTART,
    SEQ_FADEWAIT,
    SEQ_END,
  };

  BR_START_WORK  *p_wk     = p_wk_adrs;
 
  switch( *p_seq )
  { 
  case SEQ_INIT:
    p_wk->p_here  = BR_MSGWIN_Init( BG_FRAME_S_FONT, 8, 16, 16, 2, PLT_BG_S_FONT, p_wk->p_que, p_wk->heapID );
    { 
      GFL_FONT *p_font;
      GFL_MSGDATA *p_msg; 

      p_font  = BR_RES_GetFont( p_wk->p_param->p_res );
      p_msg   = BR_RES_GetMsgData( p_wk->p_param->p_res );
      BR_MSGWIN_PrintColor( p_wk->p_here, p_msg, msg_10000, p_font, PRINTSYS_LSB_Make(1,2,0) );
    }
    BR_FADE_StartFadeEx( p_wk->p_param->p_fade, BR_FADE_TYPE_PALLETE, BR_FADE_DISPLAY_TOUCH_HERE, BR_FADE_DIR_OUT, 4 );
    (*p_seq)  = SEQ_FADE_IN;
    break;

  case SEQ_FADE_IN:
    if( BR_FADE_IsEnd(p_wk->p_param->p_fade) )
    { 
      BR_FADE_StartFadeEx( p_wk->p_param->p_fade, BR_FADE_TYPE_PALLETE, BR_FADE_DISPLAY_TOUCH_HERE, BR_FADE_DIR_IN, 4 );
      (*p_seq)  = SEQ_FADE_WAIT;
    }
    break;

  case SEQ_FADE_WAIT:
    if( BR_FADE_IsEnd(p_wk->p_param->p_fade) )
    { 
      (*p_seq)  = SEQ_TOUCH;
    }
    break;

  case SEQ_TOUCH:
    if( GFL_UI_TP_GetTrg() )
    { 
      BR_MSGWIN_Exit( p_wk->p_here );
      p_wk->p_here  = NULL;
      GFL_BG_LoadScreenReq( BG_FRAME_S_FONT );

      (*p_seq)  = SEQ_FADESTART;
    }
    break;

  case SEQ_FADESTART:
    BR_FADE_StartFadeEx( p_wk->p_param->p_fade, BR_FADE_TYPE_PALLETE, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_IN, 48 );
    BR_SIDEBAR_StartBoot( p_wk->p_param->p_sidebar );
    (*p_seq)  = SEQ_FADEWAIT;
    break;

  case SEQ_FADEWAIT:
    { 
      BOOL is_end = TRUE;
      is_end  &= BR_FADE_IsEnd( p_wk->p_param->p_fade );
      //is_end  &= BR_SIDEBAR_IsMoveEnd( p_wk->p_param->p_sidebar );

      if( is_end )
      { 
        (*p_seq)  = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    BR_PROC_SYS_Push( p_wk->p_param->p_procsys, BR_PROCID_MENU );
    SEQ_SetNext( p_seqwk, SEQFUNC_End );
    break;
  }
  
  if( p_wk->p_here != NULL )
  { 
    BR_MSGWIN_PrintMain( p_wk->p_here );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Close( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_SIDEBAR_START,
    SEQ_FADESTART,
    SEQ_FADEWAIT,
    SEQ_END,
  };

  BR_START_WORK  *p_wk     = p_wk_adrs;
 
  switch( *p_seq )
  { 
  case SEQ_INIT:
    (*p_seq)  = SEQ_SIDEBAR_START;
    break;

  case SEQ_SIDEBAR_START:
    BR_SIDEBAR_StartClose( p_wk->p_param->p_sidebar );
    (*p_seq)  = SEQ_FADESTART;
    break;

  case SEQ_FADESTART:
    if( p_wk->cnt++ > 0 )
    { 
      p_wk->cnt = 0;
      BR_FADE_StartFadeEx( p_wk->p_param->p_fade, BR_FADE_TYPE_PALLETE, BR_FADE_DISPLAY_BOTH, BR_FADE_DIR_OUT, 26 );
      (*p_seq)  = SEQ_FADEWAIT;
    }
    break;

  case SEQ_FADEWAIT:
    { 
      BOOL is_end = TRUE;
      is_end  &= BR_FADE_IsEnd( p_wk->p_param->p_fade );
      //is_end  &= BR_SIDEBAR_IsMoveEnd( p_wk->p_param->p_sidebar );

      if( is_end )
      { 
        (*p_seq)  = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    BR_PROC_SYS_Pop( p_wk->p_param->p_procsys );
    SEQ_SetNext( p_seqwk, SEQFUNC_End );
    break;
  }
  
}
//----------------------------------------------------------------------------
/**
 *	@brief  何もせずに終了
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_None( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  BR_START_WORK  *p_wk     = p_wk_adrs;

  BR_SIDEBAR_SetShakePos( p_wk->p_param->p_sidebar );
  BR_SIDEBAR_StartShake( p_wk->p_param->p_sidebar );

  BR_PROC_SYS_Push( p_wk->p_param->p_procsys, BR_PROCID_MENU );
  SEQ_SetNext( p_seqwk, SEQFUNC_End );
}
//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  BR_START_WORK  *p_wk     = p_wk_adrs;

  SEQ_End( p_seqwk );
}

