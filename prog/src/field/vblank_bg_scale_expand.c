///////////////////////////////////////////////////////////////////////////////
/**
 * @brief  VBlank割り込み期間中のBGアフィン変換
 * @file   vblank_bg_scale_expand.c
 * @author obata
 * @date   2010.02.05
 *
 * マップチェンジ時の画面停止期間中にBG面を拡大させるためのVBlank関数.
 * クロスフェード開始時にキャプチャしたBG面に対して, アフィン変換を適用する.
 * それ以外の場面での使用は考慮していません。
 * 操作対象BG面はメイン画面のBG2です。
 */
///////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "vblank_bg_scale_expand.h"
#include "system/gfl_use.h"

#define SCALE_UP_SPEED (FX32_CONST(0.003f))
#define MAX_SCALE      (FX32_CONST(0.8f))
#define CENTER_X       (128)
#define CENTER_Y       (92)
#define SCROLL_X       (0)
#define SCROLL_Y       (0)

static fx32 BGScale;  // BG拡大率


//-----------------------------------------------------------------------------
/**
 * @brief VBlank割り込み処理
 */
//-----------------------------------------------------------------------------
static void VBlankFunc_BGExpand( void )
{
  MtxFx22 matrix;

  // VBlank期間外
  if( GX_IsVBlank() == FALSE ){ return; }

  // 拡大
  BGScale -= SCALE_UP_SPEED;
  if( BGScale < MAX_SCALE ){ BGScale = MAX_SCALE; }

  // アフィン変換
  MTX_Scale22( &matrix, BGScale, BGScale );
  G2_SetBG2Affine( &matrix, CENTER_X, CENTER_Y, SCROLL_X, SCROLL_Y );
}


//-----------------------------------------------------------------------------
/**
 * @brief BGの拡大を開始する
 */
//-----------------------------------------------------------------------------
void StartVBlankBGExpand()
{
  // 拡大率を初期化
  BGScale = FX32_ONE;

  // VBlank関数を登録
  GFUser_SetVIntrFunc( VBlankFunc_BGExpand );
}

//-----------------------------------------------------------------------------
/**
 * @brief BGの拡大を終了する
 */
//-----------------------------------------------------------------------------
void EndVBlankBGExpand()
{
  MtxFx22 matrix;

  // アフィン変換を解除
  MTX_Identity22( &matrix );
  G2_SetBG2Affine( &matrix, CENTER_X, CENTER_Y, SCROLL_X, SCROLL_Y );

  // VBlank関数を解除
  GFUser_ResetVIntrFunc();
}
