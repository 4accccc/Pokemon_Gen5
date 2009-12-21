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

//システム
#include "system/palanm.h"

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

#define BR_FADE_ALPHA_PLANEMASK_M_01  (GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_OBJ)
#define BR_FADE_ALPHA_PLANEMASK_M_02  (GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3)
#define BR_FADE_ALPHA_PLANEMASK_S_01  (GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ)
#define BR_FADE_ALPHA_PLANEMASK_S_02  (GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2)

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
  PALETTE_FADE_PTR      p_pfd;
  GXRgb           pfd_color;
  u16             dummy;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//実行関数
static BOOL Br_Fade_MainMasterBrightBlack( BR_FADE_WORK *p_wk, u32 *p_seq );
static BOOL Br_Fade_MainAlpha( BR_FADE_WORK *p_wk, u32 *p_seq );
static BOOL Br_Fade_MainPallete( BR_FADE_WORK *p_wk, u32 *p_seq );
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
  p_wk->p_pfd = PaletteFadeInit( heapID );

  PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_MAIN_BG, FADE_PAL_ALL_SIZE, heapID );
  PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_MAIN_OBJ, FADE_PAL_ALL_SIZE, heapID );
  PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_SUB_BG, FADE_PAL_ALL_SIZE, heapID );
  PaletteFadeWorkAllocSet( p_wk->p_pfd, FADE_SUB_OBJ, FADE_PAL_ALL_SIZE, heapID );
  PaletteTrans_AutoSet( p_wk->p_pfd, TRUE );

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
  PaletteFadeWorkAllocFree( p_wk->p_pfd, FADE_MAIN_OBJ );	///< main	oam
  PaletteFadeWorkAllocFree( p_wk->p_pfd, FADE_MAIN_BG );	///< main	bg
  PaletteFadeWorkAllocFree( p_wk->p_pfd, FADE_SUB_OBJ );	///< main	oam
  PaletteFadeWorkAllocFree( p_wk->p_pfd, FADE_SUB_BG );	///< main	bg
  
  PaletteFadeFree( p_wk->p_pfd );
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
//----------------------------------------------------------------------------
/**
 *	@brief  フェードで使用するパレットをコピー
 *
 *	@param	BR_FADE_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_FADE_PALETTE_Copy( BR_FADE_WORK *p_wk )
{
  int i;
  for( i = 0; i < 0x10; i++ )
  { 
    PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_MAIN_BG, i, FADE_PAL_ONE_SIZE );
    PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_MAIN_OBJ, i, FADE_PAL_ONE_SIZE );
    PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_SUB_BG, i, FADE_PAL_ONE_SIZE );
    PaletteWorkSet_VramCopy( p_wk->p_pfd, FADE_SUB_OBJ, i, FADE_PAL_ONE_SIZE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレットフェードで使用する色を設定
 *
 *	@param	BR_FADE_WORK *p_wk  ワーク
 *	@param	rgb                 RGB
 */
//-----------------------------------------------------------------------------
void BR_FADE_PALETTE_SetColor( BR_FADE_WORK *p_wk, GXRgb rgb )
{ 
  p_wk->pfd_color = rgb;
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレットフェードの指定色を転送
 *
 *	@param	BR_FADE_WORK *p_wk  ワーク
 *	@param  画面
 */
//-----------------------------------------------------------------------------
void BR_FADE_PALETTE_TransColor( BR_FADE_WORK *p_wk, BR_FADE_DISPLAY display )
{ 
  //画面による設定
  if( display & BR_FADE_DISPLAY_MAIN )
  { 
    ColorConceChangePfd( p_wk->p_pfd, FADE_MAIN_OBJ, 0xFFFF, 16, p_wk->pfd_color );	///< main	oam
    ColorConceChangePfd( p_wk->p_pfd, FADE_MAIN_BG,  0xFFFF, 16, p_wk->pfd_color );	///< main	bg
  }
  if( display & BR_FADE_DISPLAY_SUB )
  { 
    ColorConceChangePfd( p_wk->p_pfd, FADE_SUB_OBJ, 0xFFFF, 16, p_wk->pfd_color );	///< main	oam
    ColorConceChangePfd( p_wk->p_pfd, FADE_SUB_BG,  0xFFFF, 16, p_wk->pfd_color );	///< main	bg
  }

  PaletteTransSwitch( p_wk->p_pfd, TRUE );
  PaletteFadeTrans( p_wk->p_pfd );
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
          BR_FADE_ALPHA_PLANEMASK_M_01,
          BR_FADE_ALPHA_PLANEMASK_M_02,
          ev1,
          ev2
          );
      }
      if( p_wk->display & BR_FADE_DISPLAY_SUB )
      { 
        G2S_SetBlendAlpha(
          BR_FADE_ALPHA_PLANEMASK_S_01,
          BR_FADE_ALPHA_PLANEMASK_S_02,
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

      //フェード方向
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

      //上画面
      if( p_wk->display & BR_FADE_DISPLAY_MAIN )
      { 
        G2_ChangeBlendAlpha( ev1, ev2 );
        if( ev1 == 0 )
        { 
          GFL_DISP_GX_SetVisibleControl( BR_FADE_ALPHA_PLANEMASK_M_01 , FALSE );
        }
        else
        { 
          GFL_DISP_GX_SetVisibleControl( BR_FADE_ALPHA_PLANEMASK_M_01 , TRUE );
        }
      }
      //下画面
      if( p_wk->display & BR_FADE_DISPLAY_SUB )
      { 
        G2S_ChangeBlendAlpha( ev1, ev2 );
        if( ev1 == 0 )
        { 
          GFL_DISP_GXS_SetVisibleControl( BR_FADE_ALPHA_PLANEMASK_S_01 , FALSE );
        }
        else
        { 
          GFL_DISP_GXS_SetVisibleControl( BR_FADE_ALPHA_PLANEMASK_S_01 , TRUE );
        }
      }
    }
    if( p_wk->cnt++ >= p_wk->max )
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
 *	@brief  パレットでのフェードメイン処理
 *
 *	@param	BR_FADE_WORK *p_wk  ワーク
 *	@param  u32 seq             シーケンス
 *
 *	@return TRUEならば処理終了  FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL Br_Fade_MainPallete( BR_FADE_WORK *p_wk, u32 *p_seq )
{ 
  enum
  { 
    SEQ_PFD_INIT,
    SEQ_PFD_MAIN,
    SEQ_PFD_EXIT,
  };

  switch( *p_seq )
  { 
  case SEQ_PFD_INIT:
    { 
      int i;

      p_wk->cnt = 0;
      p_wk->max = p_wk->sync == 0? 16: p_wk->sync;

      PaletteTransSwitch( p_wk->p_pfd, TRUE );

      *p_seq  = SEQ_PFD_MAIN;
    }
    break;

  case SEQ_PFD_MAIN:
    { 
      u8 ev;
      //フェード方向
      if( p_wk->dir == BR_FADE_DIR_IN )
      { 
        ev = 16 - 16 * p_wk->cnt / p_wk->max;
      }
      else if( p_wk->dir == BR_FADE_DIR_OUT )
      { 
        ev = 0 + 16 * p_wk->cnt / p_wk->max;
      }

      //画面による設定
      if( p_wk->display & BR_FADE_DISPLAY_MAIN )
      { 
        ColorConceChangePfd( p_wk->p_pfd, FADE_MAIN_OBJ, 0xFFFF, ev, p_wk->pfd_color );	///< main	oam
        ColorConceChangePfd( p_wk->p_pfd, FADE_MAIN_BG,  0xFFFF, ev, p_wk->pfd_color );	///< main	bg
      }
      if( p_wk->display & BR_FADE_DISPLAY_SUB )
      { 
        ColorConceChangePfd( p_wk->p_pfd, FADE_SUB_OBJ, 0xFFFF, ev, p_wk->pfd_color );	///< main	oam
        ColorConceChangePfd( p_wk->p_pfd, FADE_SUB_BG,  0xFFFF, ev, p_wk->pfd_color );	///< main	bg
      }

      PaletteFadeTrans( p_wk->p_pfd );

      if( p_wk->cnt++ >= p_wk->max )
      { 
        *p_seq  = SEQ_PFD_EXIT;
      }
    }
    break;

  case SEQ_PFD_EXIT:
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

  case BR_FADE_TYPE_PALLETE:
    return Br_Fade_MainPallete;

  default:
    GF_ASSERT( 0 );
    return NULL;
  }
}
