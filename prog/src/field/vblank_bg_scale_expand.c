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


#define CENTER_X  (128)
#define CENTER_Y  (92)
#define SCROLL_X  (0)
#define SCROLL_Y  (0)

static int   scaleCount; // フレームカウンタ
static float scale;      // 拡大率

float scaleUpSpeed = 0.0044f; // 拡大速度

//-----------------------------------------------------------------------------
/**
 * @brief VBlank割り込み処理
 */
//-----------------------------------------------------------------------------
static void VBlankFunc_BGExpand( void *pWork )
{
  fx32 fxScale;
  float frame = 0;

  // VBlank 期間外
  if( GX_IsVBlank() == FALSE ){ return; }

  // 実拡大率を算出
  scale += scaleUpSpeed;
  fxScale = FX32_CONST( 1.0f / scale );

  // アフィン変換
  {
    MtxFx22 matrix;
    MTX_Scale22( &matrix, fxScale, fxScale );
    G2_SetBG2Affine( &matrix, CENTER_X, CENTER_Y, SCROLL_X, SCROLL_Y );
  }
}


//-----------------------------------------------------------------------------
/**
 * @brief BGの拡大を開始する
 */
//-----------------------------------------------------------------------------
void StartVBlankBGExpand()
{
  // 拡大率を初期化
  scale      = 1.0f;
  scaleCount = 0;

  // VBlank関数を登録
  GFUser_SetVIntrFunc( VBlankFunc_BGExpand , NULL );
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
