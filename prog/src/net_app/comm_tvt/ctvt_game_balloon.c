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
  p_wk->yura_move_index = 0;
  p_wk->yura_tbl_index  = 0;
  p_wk->yura_tbl_cnt    = 0;
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


// 風船がゆれる移動テーブル
static const fx32 yura_tbl[][16] = 
{
  // 右に揺れる(開始)
  { 0x800, 0x800, 0x500, 0x400, 0x400, 0x350, 0x350, 0x300, 0x300, 0x200, 0x200, 0x100, 0x50, 0x50, YURA_END},
  // 右に揺れる(終了)
  { 0x50, 0x50, 0x100, 0x200, 0x200, 0x300, 0x300, 0x350, 0x350, 0x400, 0x400, 0x500, 0x800, 0x800, YURA_END},
  // 左に揺れる(開始)
  { -0x800, -0x800, -0x500, -0x400, -0x400, -0x350, -0x350, -0x300, -0x300, -0x200, -0x200, -0x100, -0x50, -0x50, YURA_END},
  // 左に揺れる(終了)
  { -0x50, -0x50, -0x100, -0x200, -0x200, -0x300, -0x300, -0x350, -0x350, -0x400, -0x400, -0x500, -0x800, -0x800, YURA_END},
};

// ゆれるテーブルのどれを使うかのテーブル
static const u8 yura_action_tbl[][2] =
{
  { 0, 1 },   // 右左と基本的な動き1
  { 2, 3 },   // 左右と基本的な動き2
};

//----------------------------------------------------------------------------
/**
 *	@brief  風船更新
 *
 *	@param	p_wk   風船構造体へのポインタ
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON_Update( BALLOON_WORK*  p_wk )
{
#if 0
  // ゆらゆら処理
  fx32  vx;   // X軸移動距離
  vx = yura_tbl[ yura_action_tbl[p_wk->yura_tbl_index][p_wk->yura_tbl_cnt] ][ p_wk->yura_move_index ];

  p_wk->yura_move_index++;
  if( vx == YURA_END )
  {
    // 次のアクションテーブルに
    p_wk->yura_move_index = 0;
    p_wk->yura_tbl_cnt++;

    // 次に使うアクションテーブルを決める
    if( p_wk->yura_tbl_cnt > 1 )
    {
      p_wk->yura_tbl_cnt = 0;
      p_wk->yura_tbl_index = 0; // 仮
    }
  }
  else
  {
    // X移動
    if(GFL_UI_KEY_GetCont()&PAD_KEY_DOWN){
      //ICHI_Printf("vx %x m_i %d t_c %d t_i %d\n", vx, p_wk->yura_move_index, p_wk->yura_tbl_cnt, p_wk->yura_tbl_index);
      //p_wk->status.trans.x += vx ;
    }
  }
#endif

  // HIT時は移動しない
  if( p_wk->type == BALLOON_HIT )
  {
    // 当たったときのエフェクト
    p_wk->alpha -= 2;
    if( p_wk->alpha <= 12 )
    {
      p_wk->alpha = 31;
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
    VEC_Add( &p_wk->status.trans, &p_wk->vec, &p_wk->status.trans );
  }

  // 画面外判定
  {
    int x, y;   // スクリーン座標
    int ret;    // 戻り値 0ならビューポート内

    // ワールド座標からスクリーン座標を求める
    ret = NNS_G3dWorldPosToScrPos( &p_wk->status.trans, &x, &y );

    //ICHI_Printf("風船SCR_POS  %d:%d  ret:%d \n", x, y, ret);

#if 0
    // 画面外
    if( ( y <= 0 || x<=0 || x>= 256 )
        && ret == 0  )
    {
      CTVT_BALLOON_Destroy( p_wk );
    }
#endif

    // 3D画面外判定　(曖昧)
    if( p_wk->status.trans.y > FX32_CONST(150) )
    {
      CTVT_BALLOON_Destroy( p_wk );
      return;
    }
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

