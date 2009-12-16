//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_fade.c
 *	@brief  バトルレコーダー用フェードシステム
 *	@author	Toru=Nagihashi
 *	@data		2009.12.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//外部公開
#include "br_fade.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define BR_FADE_DEFAULT_SYNC  (0)
#define BR_FADE_DEFAULT_EV1   (16)
#define BR_FADE_DEFAULT_EV2   (16)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================


//-------------------------------------
///	フェード実行関数
//=====================================
typedef BOOL (*BR_FADE_MAINFUNCTION)( BR_FADE_WORK * p_wk, u32 *p_seq );

//-------------------------------------
///	フェードシステム
//=====================================
struct _BR_FADE_WORK
{ 
  BOOL            is_end;
  BR_FADE_DISPLAY display;
  BR_FADE_DIR     dir;
  u32             seq;
  u32             cnt;
  u32             max;
  u32             sync;
  BR_FADE_MAINFUNCTION  MainFunction;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//実行関数
static BOOL Br_Fade_MainMasterBrightBlack( BR_FADE_WORK *p_wk, u32 *p_seq );
static BOOL Br_Fade_MainAlpha( BR_FADE_WORK *p_wk, u32 *p_seq );
//その他
static BR_FADE_MAINFUNCTION Br_Fade_Factory( BR_FADE_TYPE type );

//=============================================================================
/**
 *    PUBLIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/** 
 *	@brief  フェードシステム初期化
 *
 *	@param	HEAPID heapID 
 */
//-----------------------------------------------------------------------------
BR_FADE_WORK * BR_FADE_Init( HEAPID heapID )
{ 
  BR_FADE_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BR_FADE_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BR_FADE_WORK) );
  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードシステム破棄
 *
 *	@param	BR_FADE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_FADE_Exit( BR_FADE_WORK *p_wk )
{ 
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードメイン処理
 *
 *	@param	BR_FADE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_FADE_Main( BR_FADE_WORK *p_wk )
{
  if( p_wk->MainFunction )
  { 
    p_wk->is_end = p_wk->MainFunction( p_wk, &p_wk->seq );
    if( p_wk->is_end )
    { 
      p_wk->MainFunction  = NULL;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェード開始
 *
 *	@param	BR_FADE_WORK *p_wk  ワーク
 *	@param	type                フェードの種類
 *	@param  disp                画面
 *	@param  dir                 フェードの方向
 */
//-----------------------------------------------------------------------------
void BR_FADE_StartFade( BR_FADE_WORK *p_wk, BR_FADE_TYPE type, BR_FADE_DISPLAY disp, BR_FADE_DIR  dir )
{ 
  BR_FADE_StartFadeEx( p_wk, type, disp, dir, BR_FADE_DEFAULT_SYNC );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェード開始  シンク指定版
 *
 *	@param	BR_FADE_WORK *p_wk  ワーク
 *	@param	type  フェードの種類
 *	@param  disp  画面
 *	@param  dir   フェードの方向
 *	@param	sync  シンク
 */
//-----------------------------------------------------------------------------
void BR_FADE_StartFadeEx( BR_FADE_WORK *p_wk, BR_FADE_TYPE type, BR_FADE_DISPLAY disp, BR_FADE_DIR dir, u32 sync )
{ 
  p_wk->display       = disp;
  p_wk->dir           = dir;
  p_wk->sync          = sync;
  p_wk->MainFunction  = Br_Fade_Factory( type );
  p_wk->is_end        = FALSE;
  p_wk->seq           = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェード終了検知
 *
 *	@param	const BR_FADE_WORK *cp_wk   ワーク
 *
 *	@return TRUEならばフェード終了  FALSEならべフェード中
 */
//-----------------------------------------------------------------------------
BOOL BR_FADE_IsEnd( const BR_FADE_WORK *cp_wk )
{ 
  return cp_wk->is_end;
}
//=============================================================================
/**
 *    PRIVATE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  輝度でのフェードメイン処理  黒くなる
 *
 *	@param	BR_FADE_WORK *p_wk  ワーク
 *	@param  u32 seq             シーケンス
 *
 *	@return TRUEならば処理終了  FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL Br_Fade_MainMasterBrightBlack( BR_FADE_WORK *p_wk, u32 *p_seq )
{ 
  enum
  { 
    SEQ_MBB_INIT,
    SEQ_MBB_MAIN,
    SEQ_MBB_EXIT,
  };

  switch( *p_seq )
  { 
  case SEQ_MBB_INIT:
    { 
      u32 mode  = 0;
      u32 start, end;
      //画面による設定
      if( p_wk->display & BR_FADE_DISPLAY_MAIN )
      { 
        mode  |= GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN;
      }
      if( p_wk->display & BR_FADE_DISPLAY_SUB )
      { 
        mode  |= GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB;
      }
      //方向による設定
      if( p_wk->dir == BR_FADE_DIR_IN )
      { 
        start = 16;
        end   = 0;
      }
      else if( p_wk->dir == BR_FADE_DIR_OUT )
      { 
        start = 0;
        end   = 16;
      }

      GFL_FADE_SetMasterBrightReq( mode, start, end, p_wk->sync );
      *p_seq  = SEQ_MBB_MAIN;
    }
    break;

  case SEQ_MBB_MAIN:
    if( !GFL_FADE_CheckFade() )
    { 
      *p_seq  = SEQ_MBB_EXIT;
    }
    break;

  case SEQ_MBB_EXIT:
    return TRUE;
  }


  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  アルファでのフェードメイン処理
 *
 *	@param	BR_FADE_WORK *p_wk  ワーク
 *	@param  u32 seq             シーケンス
 *
 *	@return TRUEならば処理終了  FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL Br_Fade_MainAlpha( BR_FADE_WORK *p_wk, u32 *p_seq )
{ 
  enum
  { 
    SEQ_AL_INIT,
    SEQ_AL_MAIN,
    SEQ_AL_EXIT,
  };

  switch( *p_seq )
  { 
  case SEQ_AL_INIT:
    {
      int ev1, ev2;

      if( p_wk->dir == BR_FADE_DIR_IN )
      { 
        ev1 = 0;
        ev2 = 16;
      }
      else if( p_wk->dir == BR_FADE_DIR_OUT )
      { 
        ev1 = 16;
        ev2 = 0;
      }

      if( p_wk->display & BR_FADE_DISPLAY_MAIN )
      { 
        G2_SetBlendAlpha(
          GX_BLEND_PLANEMASK_BG2,
          GX_BLEND_PLANEMASK_BG0 |
          GX_BLEND_PLANEMASK_BG3,
          ev1,
          ev2
          );
      }
      if( p_wk->display & BR_FADE_DISPLAY_SUB )
      { 
        G2S_SetBlendAlpha(
          GX_BLEND_PLANEMASK_BG1,
          GX_BLEND_PLANEMASK_BG0 |
          GX_BLEND_PLANEMASK_BG1 |
          GX_BLEND_PLANEMASK_BG2,
          ev1,
          ev2
          );
      }

      p_wk->cnt = 0;
      p_wk->max = p_wk->sync == 0? 16: p_wk->sync;
      *p_seq    = SEQ_AL_MAIN;
    }
    break;

  case SEQ_AL_MAIN:
    { 
      int ev1, ev2;

      if( p_wk->dir == BR_FADE_DIR_IN )
      { 
        ev1 = 0 + 16 * p_wk->cnt / p_wk->max;
        ev2 = 16 - 16 * p_wk->cnt / p_wk->max;
      }
      else if( p_wk->dir == BR_FADE_DIR_OUT )
      { 
        ev1 = 16 - 16 * p_wk->cnt / p_wk->max;
        ev2 = 0 + 16 * p_wk->cnt / p_wk->max;
      }

      if( p_wk->display & BR_FADE_DISPLAY_MAIN )
      { 
        G2_ChangeBlendAlpha( ev1, ev2 );
      }
      if( p_wk->display & BR_FADE_DISPLAY_SUB )
      { 
        G2S_ChangeBlendAlpha( ev1, ev2 );
      }
    }
    if( p_wk->cnt++ >= p_wk->max-1 )
    { 
      *p_seq  = SEQ_AL_EXIT;
    }
    break;

  case SEQ_AL_EXIT:
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フェードの種類により実行関数を返す
 *
 *	@param	BR_FADE_TYPE  フェードの種類
 *
 *	@return 実行関数
 */
//-----------------------------------------------------------------------------
static BR_FADE_MAINFUNCTION Br_Fade_Factory( BR_FADE_TYPE type )
{ 
  switch( type )
  { 
  case BR_FADE_TYPE_MASTERBRIGHT_BLACK:
    return Br_Fade_MainMasterBrightBlack;

  case BR_FADE_TYPE_ALPHA_BG012OBJ:
    return Br_Fade_MainAlpha;

  default:
    GF_ASSERT( 0 );
    return NULL;
  }
}
