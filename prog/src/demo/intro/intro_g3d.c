//=============================================================================
/**
 *
 *	@file		intro_g3d.c
 *	@brief
 *	@author	hosaka genya
 *	@data		2009.12.20
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

//アーカイブ
#include "arc_def.h"

#include "intro_graphic.h"
#include "intro_g3d.h"

#include "intro.naix"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  UNIT_MAX = 1,   ///< ユニット最大値
};

//RES
static const GFL_G3D_UTIL_RES res_unit_select[] = {
  { ARCID_INTRO_GRA, NARC_intro_intro_bg_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_GRA, NARC_intro_intro_bg_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_GRA, NARC_intro_intro_bg_nsbma, GFL_G3D_UTIL_RESARC },
};
//ANM
static const GFL_G3D_UTIL_ANM anm_unit_select[] = {
  { 1, 0 },
  { 2, 0 },
};
//OBJ
static const GFL_G3D_UTIL_OBJ obj_unit_select[] = {
  0, 0, 0, 
  anm_unit_select, NELEMS(anm_unit_select),
};
// セットアップ
static const GFL_G3D_UTIL_SETUP sc_g3d_setup[ UNIT_MAX ] = 
{ 
  { res_unit_select, NELEMS(res_unit_select), obj_unit_select, NELEMS(obj_unit_select), },
};


// カメラ設定
#define CAMERA_POS_Z ( 0x1b406 )
static const VecFx32 sc_camera_pos    = { 0xfffffdeb, 0x809b, CAMERA_POS_Z };
static const VecFx32 sc_camera_up     = { 0x0, 0x4272, 0x9f0 };
static const VecFx32 sc_camera_target = { 0x0, 0x23e0, 0xffff3802 }; 

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
struct _INTRO_G3D_WORK {
  // [IN]
  INTRO_GRAPHIC_WORK* graphic;
  HEAPID heap_id;
  // [PRIVATE]
  GFL_G3D_UTIL*   g3d_util;
  GFL_G3D_CAMERA* camera;
  u16           unit_idx[ UNIT_MAX ];
  BOOL          is_draw;
  BOOL          is_man;
  BOOL          is_load;
  u32 seq;
  u32 start_seq;
  u32 cnt;
  fx32 anmseq;
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  3D関連モジュール 生成
 *
 *	@param	INTRO_GRAPHIC_WORK* graphic 
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
INTRO_G3D_WORK* INTRO_G3D_Create( INTRO_GRAPHIC_WORK* graphic , HEAPID heap_id )
{
  INTRO_G3D_WORK* wk;

  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof( INTRO_G3D_WORK ) );
  
  wk->heap_id = heap_id;
  wk->graphic = graphic;

  // ユーティリティーをセットアップ
  wk->g3d_util = GFL_G3D_UTIL_Create( 10, 16, heap_id );

  // 専用カメラを用意
  wk->camera = GFL_G3D_CAMERA_Create(	GFL_G3D_PRJPERS, 
									FX_SinIdx( defaultCameraFovy/2 *PERSPWAY_COEFFICIENT ),
									FX_CosIdx( defaultCameraFovy/2 *PERSPWAY_COEFFICIENT ),
									defaultCameraAspect, 0,
									defaultCameraNear, defaultCameraFar, 0,
									&sc_camera_pos, &sc_camera_up, &sc_camera_target, heap_id );
  
  // ユニット追加
  {
    int i;
    
    for( i=0; i<UNIT_MAX; i++ )
    {
      const GFL_G3D_UTIL_SETUP* setup;

      setup = &sc_g3d_setup[i];

      wk->unit_idx[i] = GFL_G3D_UTIL_AddUnit( wk->g3d_util, setup );
    }
  }
  
  // アニメーション有効化
  {
    int i;
    for( i=0; i<UNIT_MAX; i++ )
    {
      int j;
      GFL_G3D_OBJ* obj;
      int anime_count;
      
      obj         = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[i] );
      anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );

      for( j=0; j<anime_count; j++ )
      {
        const int frame = FX32_CONST(21); // @TODO 開始値

        GFL_G3D_OBJECT_EnableAnime( obj, j );
        GFL_G3D_OBJECT_SetAnimeFrame( obj, j, &frame );
      }
    }
  }

  wk->is_load = TRUE;

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  3D関連モジュール 破棄
 *
 *	@param	INTRO_G3D_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_G3D_Exit( INTRO_G3D_WORK* wk )
{
  int i;

  // 専用カメラを破棄
  GFL_G3D_CAMERA_Delete( wk->camera );

  for( i=0; i<UNIT_MAX; i++ )
  {
    GFL_G3D_UTIL_DelUnit( wk->g3d_util, wk->unit_idx[i] );
  }

  GFL_G3D_UTIL_Delete( wk->g3d_util );

  GFL_HEAP_FreeMemory( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  描画 主処理
 *
 *	@param	INTRO_G3D_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
#include "..\..\ui\debug_code\g3d\camera_test.c"
#include "..\..\ui\debug_code\g3d\g3d_util_anime_frame_test.c"

void INTRO_G3D_Main( INTRO_G3D_WORK* wk )
{
  int i;
  
  {
    // アニメーション調整
//  debug_g3d_util_anime_frame_test( wk->g3d_util, 0 );

    // 座標調整
//  debug_camera_test( wk->camera );
  }

  // 専用カメラにスイッチ
  GFL_G3D_CAMERA_Switching( wk->camera );

  if( wk->is_draw && wk->is_load )
  {
    GFL_G3D_OBJSTATUS status;

    // ステータス初期化
    VEC_Set( &status.trans, 0, 0, 0 );
    VEC_Set( &status.scale, FX32_CONST(1), FX32_CONST(1), FX32_CONST(1) );
    MTX_Identity33( &status.rotate );

    // 描画
    for( i=0; i<UNIT_MAX; i++ )
    {
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[i] );

      GFL_G3D_DRAW_DrawObject( obj, &status );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  3D選択肢スタート
 *
 *	@param	INTRO_G3D_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL INTRO_G3D_SelectStart( INTRO_G3D_WORK* wk )
{
  fx32 val;
  VecFx32 pos;

  switch( wk->start_seq )
  {
  case 0:
    // カメラを離す
    GFL_G3D_CAMERA_GetPos( wk->camera, &pos );
    pos.z = 0x200000;
    GFL_G3D_CAMERA_SetPos( wk->camera, &pos );
    wk->start_seq++;
    break;
  case 1:
    // 拡大
    GFL_G3D_CAMERA_GetPos( wk->camera, &pos );
    
    val = MATH_IAbs( pos.z - CAMERA_POS_Z ) / 4;

    // 誤差修正
    if( val < 0x100 ){ val = 0x100; }

    pos.z -= val;

    if( pos.z < CAMERA_POS_Z )
    {
      pos.z = CAMERA_POS_Z;
      GFL_G3D_CAMERA_SetPos( wk->camera, &pos );

      return TRUE;
    }
      
    GFL_G3D_CAMERA_SetPos( wk->camera, &pos );
    break;
  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  表示非表示切り替え
 *
 *	@param	INTRO_G3D_WORK* wk
 *	@param	is_visible 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_G3D_SelectVisible( INTRO_G3D_WORK* wk, BOOL is_visible )
{
  wk->is_draw = is_visible;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  決定演出開始
 *
 *	@param	INTRO_G3D_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_G3D_SelectDecideStart( INTRO_G3D_WORK* wk, BOOL is_man )
{
  wk->is_man = is_man;
  wk->seq = 0;

  if( wk->is_man )
  {
    wk->cnt = 19;
  }
  else
  {
    wk->cnt = 21;
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  決定演出 主処理
 *
 *	@param	INTRO_G3D_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL INTRO_G3D_SelectDecideWait( INTRO_G3D_WORK* wk )
{
  if( wk->is_man )
  {
    wk->cnt--;
    INTRO_G3D_SelectSet( wk, wk->cnt );
    if( wk->cnt <= 0 ){ return TRUE; }
  }
  else
  {
    wk->cnt++;
    INTRO_G3D_SelectSet( wk, wk->cnt );
    if( wk->cnt >= 50 ){ return TRUE; }
  }
  
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  3D男女選択、リターン
 *
 *	@param	INTRO_G3D_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL INTRO_G3D_SelectDecideReturn( INTRO_G3D_WORK* wk )
{
  if( wk->is_man )
  {
    wk->cnt++;
    INTRO_G3D_SelectSet( wk, wk->cnt );
  }
  else
  {
    wk->cnt--;
    INTRO_G3D_SelectSet( wk, wk->cnt );
  }

  if( wk->cnt == 21 ){ return TRUE; }
  
  return FALSE;

}

//-----------------------------------------------------------------------------
/**
 *	@brief  3D男女選択、フレーム指定
 *
 *	@param	INTRO_G3D_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void INTRO_G3D_SelectSet( INTRO_G3D_WORK* wk, u32 in_frame )
{
  int i;

  // アニメフレーム指定
  for( i=0; i<UNIT_MAX; i++ )
  {
    int j;
    GFL_G3D_OBJ* obj;
    int anime_count;
    
    obj = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[i] );
    anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );

    for( j=0; j<anime_count; j++ )
    {
      const int frame = FX32_CONST(in_frame);

      GFL_G3D_OBJECT_SetAnimeFrame( obj, j, &frame );
    }
  }
}


//=============================================================================
/**
 *								static関数
 */
//=============================================================================


