//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		ctvt_game_balloon.c
 *	@brief
 *	@author		Katsuhiko=Ichiraku
 *	@date		2010.12.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "ctvt_game_balloon.h"
#include "ctvt_game.h"

#include "calctool.h"   // VEC_COPY

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

#define SCALE_ADD   ( 0x4A )      // 1フレームの拡大率加算値
#define SCALE_MAX   ( 0x310 )     // 風船の拡大率MAX
#define SCALE_S_MAX   ( 0x260 )   // 風船の拡大率MAX 結果発表版

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
  //GFL_STD_MemClear( p_wk, sizeof(BALLOON_WORK) );
  //↑したらg3dUnitが消える

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
  p_wk->anm_frm = 0;
  p_wk->anm_flg = FALSE;
  p_wk->scale   = 0;

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
    VecFx32 scale_tmp;

    // 風船を徐々に拡大する
    if( p_wk->scale < SCALE_MAX )
    {
      p_wk->scale += SCALE_ADD;
      if( p_wk->scale > SCALE_MAX )
      {
        p_wk->scale = SCALE_MAX;
      }

      VEC_Set( &scale_tmp, SCALE_ADD, SCALE_ADD, SCALE_ADD );
      VEC_Add( &p_wk->status.scale, &scale_tmp, &p_wk->status.scale );
    }

    // 拡大したら、αフェードして消える
    if( p_wk->scale == SCALE_MAX )
    {
      p_wk->alpha -= 3;
      if( p_wk->alpha <= 0 )
      {
        CTVT_BALLOON_Destroy( p_wk );
        return;
      }
    }
  }
  else
  {
    // 経過フレーム数で移動するように変更
    u16   now_frame;
    s32   def;  // マイナスになる可能性がある
    u8    i;
    now_frame = CTVT_GAME_GetFrame( p_wk->p_game_wk );
    def   = now_frame - p_wk->save_frame;

    if( def > 0 )
    {
      p_wk->save_frame = now_frame;
      for( i=0; i<def; i++ )
      {
        VEC_Add( &p_wk->status.trans, &p_wk->vec, &p_wk->status.trans );
      }
    }
    //GFL_CALC3D_VEC_MulScalar( &p_wk->vec, mul, &p_wk->status.trans );
  }

  // 画面外判定
  {
    int x, y;   // スクリーン座標
    int ret;    // 戻り値 0ならビューポート内

    // ワールド座標からスクリーン座標を求める
    ret = NNS_G3dWorldPosToScrPos( &p_wk->status.trans, &x, &y );

    // 画面外 両端のみ
    if( ( x<-64 || x>320 ) && ret == 0  )
    {
      CTVT_BALLOON_Destroy( p_wk );
      return;
    }
    // 3D画面外判定　(曖昧)
    if( p_wk->status.trans.y > FX32_CONST(240) )
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
  u16             head_obj_idx;

  // 描画
  // エフェクトより風船が手前
  if( p_wk->visible == TRUE )
  {
    // エフェクトアニメーション描画&更新
    if( p_wk->type == BALLOON_HIT && p_wk->anm_flg == TRUE )
    {
      int  tmp_frm;
      GFL_G3D_OBJSTATUS tmp_status;

      head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->parent_g3dUtil, CTVT_UNIT_IDX_EFFECT );
      obj = GFL_G3D_UTIL_GetObjHandle( p_wk->parent_g3dUtil, head_obj_idx );

      // 手動でアニメフレーム数設定
      GFL_G3D_OBJECT_SetAnimeFrame( obj, 0, (int*)&p_wk->anm_frm );
      p_wk->anm_frm += FX32_ONE;
      GFL_G3D_OBJECT_GetAnimeFrameMax( obj, 0, &tmp_frm );

      if( p_wk->anm_frm >= tmp_frm )
      {
        // アニメOFF
        p_wk->anm_flg = FALSE;
      }

      // Z軸を風船より奥にする
      {
        VEC_COPY( &tmp_status.scale, &p_wk->status.scale );
        MTX_Identity33( &tmp_status.rotate );
        VEC_COPY( &tmp_status.trans, &p_wk->status.trans );
        tmp_status.trans.z -= 0x3000;
      }
      GFL_G3D_DRAW_DrawObject( obj, &tmp_status );
    }


    // 風船の描画
    {
      head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->parent_g3dUtil, p_wk->unit_idx );
      // 3つあるからどれ使うかtypeで指定
      obj = GFL_G3D_UTIL_GetObjHandle( p_wk->parent_g3dUtil, head_obj_idx + p_wk->type);
      mdl = NNS_G3dRenderObjGetResMdl
        (GFL_G3D_RENDER_GetRenderObj(GFL_G3D_OBJECT_GetG3Drnd(obj)) );

      //NNS_G3dMdlUseMdlAlpha(mdl);
      NNS_G3dMdlSetMdlAlphaAll(mdl, p_wk->alpha);

      // 拡大率設定
      GFL_G3D_DRAW_DrawObject( obj, &p_wk->status );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船作成  ゲーム２用
 *
 *	@param	gameWork  ゲームワーク
 *
 *	@return 作成したワーク
 */
//-----------------------------------------------------------------------------
BALLOON2_WORK* CTVT_BALLOON2_Create( CTVT_GAME_WORK *gameWork, u16 unit_idx, HEAPID heapID )
{
  BALLOON2_WORK *p_wk;
  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BALLOON2_WORK) );

  p_wk->visible = TRUE;
  p_wk->tcb     = NULL;
  p_wk->p_game_wk = gameWork;
  p_wk->parent_g3dUtil = CTVT_GAME_GetgG3DUtil( gameWork );
  p_wk->unit_idx= unit_idx;

  // OBJ情報初期化
  if( unit_idx == 0)
  {
    VEC_Set( &p_wk->status.scale, 0x900, 0x900, 0x900 );
    MTX_Identity33( &p_wk->status.rotate );
    VEC_Set( &p_wk->status.trans, 0, FX32_CONST(-60), 0 );
  }
  else
  {
    VecFx32 pos;
    fx32    x, y;

    // 左・真ん中・右の位置になるようにする。
    // 真ん中は少し下になる
    switch( unit_idx ){
    case 1:
      x = FX32_CONST(-75);
      y = FX32_CONST(10);
      break;
    case 2:
      x = FX32_CONST(0);
      y = FX32_CONST(-10);
      break;
    case 3:
      x = FX32_CONST(75);
      y = FX32_CONST(10);
      break;
    }

    VEC_Set( &p_wk->status.scale, 0x500, 0x500, 0x500 );
    MTX_Identity33( &p_wk->status.rotate );
    VEC_Set( &p_wk->status.trans, x, y, 0 );
  }

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船破棄  ゲーム２用
 *
 *	@param	BALLOON2_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON2_Delete( BALLOON2_WORK *p_wk )
{
  // タスク削除
  if( p_wk->tcb != NULL )
  {
    GFL_TCB_DeleteTask( p_wk->tcb );
  }

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船更新  ゲーム２用
 *
 *	@param	BALLOON2_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON2_Update( BALLOON2_WORK *p_wk )
{
  if( p_wk->visible == TRUE )
  {
    
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船描画  ゲーム２用
 *
 *	@param	BALLOON2_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON2_Draw( BALLOON2_WORK *p_wk )
{
  GFL_G3D_OBJ*    obj;
  NNSG3dResMdl*   mdl;
  u16   head_obj_idx;

  // 1位は無条件で表示
  if( p_wk->visible == TRUE  )
  {
    
    // 風船の描画
    head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->parent_g3dUtil, p_wk->unit_idx );
    obj = GFL_G3D_UTIL_GetObjHandle( p_wk->parent_g3dUtil, head_obj_idx );
    mdl = NNS_G3dRenderObjGetResMdl
      (GFL_G3D_RENDER_GetRenderObj(GFL_G3D_OBJECT_GetG3Drnd(obj)) );

    GFL_G3D_DRAW_DrawObject( obj, &p_wk->status );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船作成  結果発表用
 *
 *	@param	gameWork  ゲームワーク
 *
 *	@return 作成した風船ワーク
 */
//-----------------------------------------------------------------------------
BALLOON_S_WORK* CTVT_BALLOON_S_Create( CTVT_GAME_WORK *gameWork, u16 unit_idx, u8 rank, fx32 max_y, HEAPID heapID )
{
  BALLOON_S_WORK *p_wk;
  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BALLOON_S_WORK) );

  p_wk->visible = TRUE;
  p_wk->effect  = FALSE;
  p_wk->tcb     = NULL;
  p_wk->p_game_wk = gameWork;
  p_wk->parent_g3dUtil = CTVT_GAME_GetgG3DUtil( gameWork );
  p_wk->unit_idx= unit_idx;
  p_wk->alpha   = 31;
  p_wk->anm_frm = 0;
  p_wk->anm_flg = FALSE;
  p_wk->scale   = 0;
  p_wk->rank    = rank;
  p_wk->max_y   = max_y;

  // OBJ情報初期化
  {
    VecFx32 pos;
    fx32  x;

    VEC_Set( &p_wk->status.scale, 0x500, 0x500, 0x500 );
    MTX_Identity33( &p_wk->status.rotate );

    // 4分割して、0～4番目の位置
    x = FX32_CONST(-90) + FX32_CONST( 60*unit_idx );
    ICHI_Printf("%d x =%x \n", unit_idx, x);
    //NNS_G3dScrPosToWorldLine( 32 +(64*unit_idx), 192, &pos, NULL );
    //ICHI_Printf("pos %x:%x:%x \n", pos.x, pos.y, pos.z );
    VEC_Set( &p_wk->status.trans, x, FX32_CONST(-100), 0 );
  }

  // ベクトル
  VEC_Set( &p_wk->vec, 0,0x480,0 ); // 0.3fぐらい

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船破棄  結果発表用
 *
 *	@param	BALLOON_S_WORK *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON_S_Delete( BALLOON_S_WORK *p_wk )
{
  // タスク削除
  if( p_wk->tcb != NULL )
  {
    GFL_TCB_DeleteTask( p_wk->tcb );
  }

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船更新  結果発表用
 *
 *	@param	BALLOON_S_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON_S_Update( BALLOON_S_WORK *p_wk )
{
  if( p_wk->visible == TRUE )
  {
    // エフェクト時移動しない
    if( p_wk->effect == TRUE )
    {
      VecFx32 scale_tmp;

      // 風船を徐々に拡大する
      if( p_wk->scale < SCALE_S_MAX )
      {
        p_wk->scale += SCALE_ADD;
        if( p_wk->scale > SCALE_S_MAX )
        {
          p_wk->scale = SCALE_S_MAX;
        }

        VEC_Set( &scale_tmp, SCALE_ADD, SCALE_ADD, SCALE_ADD );
        VEC_Add( &p_wk->status.scale, &scale_tmp, &p_wk->status.scale );
      }

      // 拡大したら、αフェードして消える
      if( p_wk->scale == SCALE_S_MAX )
      {
        p_wk->alpha -= 2;
        if( p_wk->alpha <= 0 )
        {
          p_wk->visible = FALSE;
          return;
        }
      }
    }
    else
    {
      // 移動
      VEC_Add( &p_wk->status.trans, &p_wk->vec, &p_wk->status.trans );

      // 上昇限界判定
      if( p_wk->status.trans.y > p_wk->max_y )
      {
        p_wk->effect = TRUE;
        p_wk->anm_flg = TRUE;
        if( p_wk->rank == 1 )
        {
          p_wk->visible = FALSE;
          p_wk->effect = FALSE;
          p_wk->anm_flg = FALSE;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  風船描画  結果発表用
 *
 *	@param	BALLOON_S_WORK *p_wk 
 */
//-----------------------------------------------------------------------------
void CTVT_BALLOON_S_Draw( BALLOON_S_WORK *p_wk )
{
  GFL_G3D_OBJ*    obj;
  NNSG3dResMdl*   mdl;
  u16   head_obj_idx;

  // 1位は無条件で表示
  if( p_wk->visible == TRUE || p_wk->rank == 1 )
  {
    if( p_wk->effect == TRUE && p_wk->anm_flg == TRUE )
    {
      int  tmp_frm;
      GFL_G3D_OBJSTATUS tmp_status;

      head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->parent_g3dUtil, CTVT_UNIT_IDX_EFFECT );
      obj = GFL_G3D_UTIL_GetObjHandle( p_wk->parent_g3dUtil, head_obj_idx );

      // 手動でアニメフレーム数設定
      GFL_G3D_OBJECT_SetAnimeFrame( obj, 0, (int*)&p_wk->anm_frm );
      p_wk->anm_frm += FX32_ONE;
      GFL_G3D_OBJECT_GetAnimeFrameMax( obj, 0, &tmp_frm );

      if( p_wk->anm_frm >= tmp_frm )
      {
        // アニメOFF
        p_wk->anm_flg = FALSE;
      }

      // Z軸を風船より奥にする
      {
        VEC_COPY( &tmp_status.scale, &p_wk->status.scale );
        MTX_Identity33( &tmp_status.rotate );
        VEC_COPY( &tmp_status.trans, &p_wk->status.trans );
        tmp_status.trans.z -= 0x3000;
      }
      GFL_G3D_DRAW_DrawObject( obj, &tmp_status );
    }

    // 風船の描画
    head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( p_wk->parent_g3dUtil, p_wk->unit_idx );
    obj = GFL_G3D_UTIL_GetObjHandle( p_wk->parent_g3dUtil, head_obj_idx );
    mdl = NNS_G3dRenderObjGetResMdl
      (GFL_G3D_RENDER_GetRenderObj(GFL_G3D_OBJECT_GetG3Drnd(obj)) );

    NNS_G3dMdlSetMdlAlphaAll(mdl, p_wk->alpha);

    GFL_G3D_DRAW_DrawObject( obj, &p_wk->status );
  }
}

