//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		ctvt_game_balloon.c
 *	@brief
 *	@author		Katsuhiko=Ichiraku
 *	@data		2010.12.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "ctvt_game_balloon.h"
#include "ctvt_game.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

#define YURA_END    (0xFFFF)    // 揺れるテーブルの終わりを示す

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  風船初期化
 *
 *	@param	p_wk   風船構造体へのポインタ
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON_Init( BALLOON_WORK* p_wk )
{

  // OBJ情報初期化
  {
    VEC_Set( &p_wk->status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
    MTX_Identity33( &p_wk->status.rotate );
    VEC_Set( &p_wk->status.trans, 0, 0, 0 );
  }

  // ベクトル
  VEC_Set( &p_wk->vec, 0,0,0 );

  // 初期値は必ずFALSE
  p_wk->visible = FALSE;

  // 初期化
  p_wk->unit_idx = 0;
  p_wk->color = COLOR_R;
  p_wk->type  = BALLOON_NORMAL;
  p_wk->size  = BALLOON_SIZE_M;
  p_wk->alpha = 31;
  p_wk->alpha_cnt = 0;

  // タスクは必ずNULL
  p_wk->tcb = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船破棄
 *
 *	@param	p_wk   風船構造体へのポインタ
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON_Destroy( BALLOON_WORK*  p_wk )
{
  // タスク削除
  if( p_wk->tcb != NULL )
  {
    //ICHI_Printf("風船破棄 %d\n", p_wk->tcb);
    GFL_TCB_DeleteTask( p_wk->tcb );
  }

  // タスクは必ずNULL
  //p_wk->tcb = NULL;
  // ↓初期化関数を呼ぶことにする
  CTVT_BALLOON_Init( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船更新
 *
 *	@param	p_wk   風船構造体へのポインタ
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON_Update( BALLOON_WORK*  p_wk )
{

  // HIT時は移動しない
  if( p_wk->type == BALLOON_HIT )
  {
    // 当たったときのエフェクト
    p_wk->alpha -= 2;
    if( p_wk->alpha <= 10 )
    {
      p_wk->alpha = 30;
      p_wk->alpha_cnt--;

      if( p_wk->alpha_cnt <= 0 )
      {
        CTVT_BALLOON_Destroy( p_wk );
        return;
      }
    }
  }
  else
  {
    // 移動
    //VEC_Add( &p_wk->status.trans, &p_wk->vec, &p_wk->status.trans );

#if 1
    // 経過フレーム数で移動するように変更
    s16   def;  // マイナスになる可能性がある
    u8    i;
    def = CTVT_GAME_GetFrame( p_wk->p_game_wk ) - p_wk->save_frame;

    if( def > 0 )
    {
      p_wk->save_frame = CTVT_GAME_GetFrame( p_wk->p_game_wk );
      for( i=0; i<def; i++ )
      {
        VEC_Add( &p_wk->status.trans, &p_wk->vec, &p_wk->status.trans );
      }
    }
    //GFL_CALC3D_VEC_MulScalar( &p_wk->vec, mul, &p_wk->status.trans );
#endif
  }

  // 画面外判定
  {
    int x, y;   // スクリーン座標
    int ret;    // 戻り値 0ならビューポート内

    // ワールド座標からスクリーン座標を求める
    ret = NNS_G3dWorldPosToScrPos( &p_wk->status.trans, &x, &y );

    //ICHI_Printf("風船SCR_POS  %d:%d  ret:%d \n", x, y, ret);

#if 1
    // 画面外 両端のみ
    if( ( x<-64 || x>320 ) && ret == 0  )
    {
      CTVT_BALLOON_Destroy( p_wk );
      return;
    }
#endif

#if 1
    // 3D画面外判定　(曖昧)
    if( p_wk->status.trans.y > FX32_CONST(150) )
    {
      CTVT_BALLOON_Destroy( p_wk );
      return;
    }
#endif
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船描画
 *
 *	@param	p_wk   風船構造体へのポインタ
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON_Draw( BALLOON_WORK*  p_wk )
{
  GFL_G3D_OBJ*    obj;
  NNSG3dResMdl*   mdl;
  u16   head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->parent_g3dUtil, p_wk->unit_idx );

  // 3つあるからどれ使うかtypeで指定
  obj = GFL_G3D_UTIL_GetObjHandle( p_wk->parent_g3dUtil, head_obj_idx + p_wk->type);
  mdl = NNS_G3dRenderObjGetResMdl
    (GFL_G3D_RENDER_GetRenderObj(GFL_G3D_OBJECT_GetG3Drnd(obj)) );

  //NNS_G3dMdlUseMdlAlpha(mdl);
  NNS_G3dMdlSetMdlAlphaAll(mdl, p_wk->alpha);

  // アニメーション更新
  // GRAPHIC_Drawで行う
  {
    //GFL_G3D_OBJECT_LoopAnimeFrame( obj, 0, FX32_ONE );
  }

  if( p_wk->visible == TRUE )
  {
    GFL_G3D_DRAW_DrawObject( obj, &p_wk->status );
  }
}

