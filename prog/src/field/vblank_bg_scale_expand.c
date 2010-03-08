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

#define MAX_SCALE (1.0f)
#define MAX_FRAME (150)
#define CENTER_X  (128)
#define CENTER_Y  (92)
#define SCROLL_X  (0)
#define SCROLL_Y  (0)


static int   waitCount;  // 待ち時間カウンタ
static int   scaleCount; // フレームカウンタ
static float scale;      // 拡大率

int   waitFrame = 0;
int   maxFrame  = 180;
float maxScale  = 1.8f;
float minScale  = 1.0f;
float scaleUpSpeed = 0.0044f;

BOOL  scrollFlag = FALSE;
float scrollSize = 60;

//-----------------------------------------------------------------------------
/**
 * @brief VBlank割り込み処理
 */
//-----------------------------------------------------------------------------
static void VBlankFunc_BGExpand( void )
{
  MtxFx22 matrix;
  fx32 fxScale;
  float frame = 0;
  int scrollY;

  waitCount++;
  if( waitCount < waitFrame ){ return; }

//-------------------------------------------------------------------

#if 0 // 2010.03.08 拡大率の上限をなくす → 補間でなく, 単純に加算する
  // 拡大率更新
  scaleCount++; 
  frame = scaleCount;
  if( maxFrame < frame ){ frame = maxFrame; }
  OS_Printf( "scaleCount = %d\n", scaleCount );

  // VBlank期間外
  if( GX_IsVBlank() == FALSE ){ return; }

  // 実拡大率を算出
  {
    float t, offset;
    t = frame / (float)maxFrame;
    scale = minScale + t * (maxScale - minScale);
    fxScale = FX32_CONST( 1.0f / scale );
  }
#endif
  // VBlank期間外
  if( GX_IsVBlank() == FALSE ){ return; }

  // 実拡大率を算出
  scale += scaleUpSpeed;
  fxScale = FX32_CONST( 1.0f / scale );

//-------------------------------------------------------------------

  // スクロール幅を決定
  if( scrollFlag ) {
    scrollY = scrollSize * frame / (float)maxFrame;
  }
  else {
    scrollY = 0;
  }

  // アフィン変換
  MTX_Scale22( &matrix, fxScale, fxScale );
  G2_SetBG2Affine( &matrix, CENTER_X, CENTER_Y, SCROLL_X, scrollY );
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
  waitCount  = 0;

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
