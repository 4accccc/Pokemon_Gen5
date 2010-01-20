//=============================================================================
/**
 *
 *	@file		demo3d_engine.c
 *	@brief  3Dデモ再生エンジン
 *	@author	hosaka genya
 *	@data		2009.12.08
 *
 */
//=============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// 文字列関連
#include "font/font.naix"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h" //PRINT_QUE

// ICA
#include "system/ica_anime.h"
#include "system/ica_camera.h"

//アーカイブ
#include "arc_def.h"

#include "demo3d_graphic.h"

//データ
#include "demo3d_data.h"

// 3Dデモコマンド
#include "demo3d_cmd.h" 

#include "demo3d_engine.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

#define ICA_BUFF_SIZE (10) ///< ICAカメラの再生で使用するバッファサイズ

//--------------------------------------------------------------
///	メインワーク
//==============================================================
struct _DEMO3D_ENGINE_WORK {
  // [IN]
  DEMO3D_GRAPHIC_WORK*    graphic;
  DEMO3D_ID               demo_id;
  u32                     start_frame;
  // [PRIVATE]
  BOOL          is_double;
  fx32          anime_speed;  ///< アニメーションスピード
  ICA_ANIME*    ica_anime;
  GFL_G3D_UTIL* g3d_util;
  GFL_TCB*      dbl3DdispVintr;
  DEMO3D_CMD_WORK*  cmd;

  u16* unit_idx; // unit_idx保持（ALLOC)
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

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dグラフィック 初期化
 *
 *	@param	DEMO3D_GRAPHIC_WORK* graphic  グラフィックワーク
 *	@param	demo_id デモID
 *	@param	start_frame 初期フレーム値(1SYNC=1)
 *	@param	heapID ヒープID
 *
 *	@retval DEMO3D_ENGINE_WORK* ワーク
 */
//-----------------------------------------------------------------------------
DEMO3D_ENGINE_WORK* Demo3D_ENGINE_Init( DEMO3D_GRAPHIC_WORK* graphic, DEMO3D_ID demo_id, u32 start_frame, HEAPID heapID )
{
  DEMO3D_ENGINE_WORK* wk;
  u8 unit_max;

  GF_ASSERT( graphic );

  HOSAKA_Printf("start_frame=%d\n",start_frame);

  // メインワーク アロケート
  wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(DEMO3D_ENGINE_WORK) );

  // メンバ初期化
  wk->graphic       = graphic;
  wk->demo_id       = demo_id;
  wk->start_frame   = start_frame;

  // コンバータデータからの初期化
  {
    GFL_G3D_CAMERA* p_camera;
    fx32 fovySin;
    fx32 fovyCos;

    p_camera = DEMO3D_GRAPHIC_GetCamera( wk->graphic );

    fovySin = Demo3D_DATA_GetCameraFovySin( demo_id );
    fovyCos = Demo3D_DATA_GetCameraFovyCos( demo_id );

    GFL_G3D_CAMERA_SetfovySin( p_camera, FX_SinIdx( (fovySin>>FX32_SHIFT) / 2 * PERSPWAY_COEFFICIENT ) );
    GFL_G3D_CAMERA_SetfovyCos( p_camera, FX_CosIdx( (fovyCos>>FX32_SHIFT) / 2 * PERSPWAY_COEFFICIENT ) );
  }
   
  // アニメーションスピードを取得
  wk->anime_speed = Demo3D_DATA_GetAnimeSpeed( demo_id );
  HOSAKA_Printf("anime_speed=%d\n", wk->anime_speed );
  
  // 2画面連結フラグを取得
  wk->is_double = Demo3D_DATA_GetDoubleFlag( demo_id );

#ifdef PM_DEBUG
  //@TODO セレクトを押しながら起動すると強制的に二画面化
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT )
  {
    wk->is_double = TRUE;
  }
#endif // PM_DEBUG
  
  HOSAKA_Printf("is_double=%d\n", wk->is_double );
  
  // 2画面連結設定初期化
  if( wk->is_double )
  {
    GFL_G3D_DOUBLE3D_Init( heapID );
    wk->dbl3DdispVintr = GFUser_VIntr_CreateTCB( GFL_G3D_DOUBLE3D_VblankIntrTCB, NULL, 0 );
  }
  
  wk->cmd = Demo3D_CMD_Init( demo_id, start_frame, heapID );

  unit_max = Demo3D_DATA_GetUnitMax( wk->demo_id );

  // ユニットID 動的配列 アロケート
  wk->unit_idx = GFL_HEAP_AllocClearMemory( heapID, sizeof(u16) * unit_max );

  // icaデータをロード
  wk->ica_anime = Demo3D_DATA_CreateICACamera( wk->demo_id, heapID, ICA_BUFF_SIZE );

  // icaアニメフレーム初期化
  ICA_ANIME_SetAnimeFrame( wk->ica_anime, FX32_CONST(start_frame) );

  // 3D管理ユーティリティーの生成
  wk->g3d_util = GFL_G3D_UTIL_Create( 10, 16, heapID );

  // ユニット追加
  {
    int i;
    for( i=0; i<unit_max; i++ )
    {
      const GFL_G3D_UTIL_SETUP* setup;

      setup = Demo3D_DATA_GetG3DUtilSetup( wk->demo_id, i );

      HOSAKA_Printf("demoid=%d setup_idx=%d setup objcnt=%d resCount=%d \n",wk->demo_id, i, setup->objCount, setup->resCount);

      wk->unit_idx[i] = GFL_G3D_UTIL_AddUnit( wk->g3d_util, setup );
    }
  }
  
  // アニメーション有効化
  {
    int i;
    for( i=0; i<unit_max; i++ )
    {
      int j;
      GFL_G3D_OBJ* obj;
      int anime_count;
      
      obj         = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[i] );
      anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );

      for( j=0; j<anime_count; j++ )
      {
        const int frame = FX32_CONST(wk->start_frame);

        GFL_G3D_OBJECT_EnableAnime( obj, j );
        GFL_G3D_OBJECT_SetAnimeFrame( obj, j, &frame );
      }
    }
  }

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dグラフィック 開放
 *
 *	@param	DEMO3D_ENGINE_WORK* wk  ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
void Demo3D_ENGINE_Exit( DEMO3D_ENGINE_WORK* wk )
{ 
  if( wk->is_double )
  {
		//終了
		GFL_TCB_DeleteTask( wk->dbl3DdispVintr );
    GFL_G3D_DOUBLE3D_Exit();
  }
  
  Demo3D_CMD_Exit( wk->cmd );

  // ICA破棄
  ICA_ANIME_Delete( wk->ica_anime );

  // ユニット破棄
  {
    int i;
    for( i=0; i<Demo3D_DATA_GetUnitMax( wk->demo_id ); i++ )
    {
      GFL_G3D_UTIL_DelUnit( wk->g3d_util, wk->unit_idx[i] );
    }
  }
  
  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( wk->g3d_util );

  // ヒープ開放
  GFL_HEAP_FreeMemory( wk->unit_idx );
  GFL_HEAP_FreeMemory( wk );
}

#if 0
static void debug_camera_test( GFL_G3D_CAMERA* camera )
{ 
  VecFx32 pos;

  static int num = 1;
  static BOOL mode = 0;

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    mode = (mode+1)%3;

    if( mode == 0 )
    {
      OS_Printf("mode=%d Pos\n",mode);
    }
    else if( mode == 1 )
    {
      OS_Printf("mode=%d CamUp\n",mode);
    }
    else
    {
      OS_Printf("mode=%d Target\n",mode);
    }
  }
  
  if( mode == 0 )
  {
    GFL_G3D_CAMERA_GetPos( camera, &pos );
  }
  else if( mode == 1 )
  {
    GFL_G3D_CAMERA_GetCamUp( camera, &pos );
  }
  else
  {
    GFL_G3D_CAMERA_GetTarget( camera, &pos );
  }

  if( CHECK_KEY_CONT( PAD_BUTTON_X ) )
  {
    num++;
    OS_Printf("num=%d \n",num);
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_Y ) )
  {
    num--;
    OS_Printf("num=%d \n",num);
  }
  else if( CHECK_KEY_CONT( PAD_KEY_UP ) )
  {
    pos.y += num;
    OS_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_KEY_DOWN ) )
  {
    pos.y -= num;
    OS_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }    
  else if( CHECK_KEY_CONT( PAD_KEY_LEFT ) )
  {
    pos.x += num;
    OS_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_KEY_RIGHT ) )
  {
    pos.x -= num;
    OS_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_L ) )
  {
    pos.z += num;
    OS_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_R ) )
  {
    pos.z -= num;
    OS_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  
  // データセット
  if( mode == 0 )
  {
    GFL_G3D_CAMERA_SetPos( camera, &pos );
  }
  else if( mode == 1 )
  {
    GFL_G3D_CAMERA_SetCamUp( camera, &pos );
  }
  else
  {
    GFL_G3D_CAMERA_SetTarget( camera, &pos );
  }

  // データ吐き出し
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    GFL_G3D_CAMERA_GetPos( camera, &pos );
    OS_Printf("static const sc_camera_pos = { 0x%x, 0x%x, 0x%x }; \n", pos.x, pos.y, pos.z );
    GFL_G3D_CAMERA_GetCamUp( camera, &pos );
    OS_Printf("static const sc_camera_up =  { 0x%x, 0x%x, 0x%x }; \n", pos.x, pos.y, pos.z );
    GFL_G3D_CAMERA_GetTarget( camera, &pos );
    OS_Printf("static const sc_camera_target = { 0x%x, 0x%x, 0x%x }; \n", pos.x, pos.y, pos.z );
  }
}
#endif

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dグラフィック 主処理
 *
 *	@param	DEMO3D_ENGINE_WORK* wk ワーク
 *
 *	@retval TRUE : 終了
 */
//-----------------------------------------------------------------------------
BOOL Demo3D_ENGINE_Main( DEMO3D_ENGINE_WORK* wk )
{
  GFL_G3D_CAMERA* p_camera;
  BOOL is_end;

//  OS_Printf("frame=%f \n", FX_FX32_TO_F32(ICA_ANIME_GetNowFrame( wk->ica_anime )) );

  // コマンド実行
  Demo3D_CMD_Main( wk->cmd, ICA_ANIME_GetNowFrame( wk->ica_anime ) );

  p_camera = DEMO3D_GRAPHIC_GetCamera( wk->graphic );
  
  // ICAカメラ更新
  is_end = ICA_ANIME_IncAnimeFrame( wk->ica_anime, wk->anime_speed );

  ICA_CAMERA_SetCameraStatus( p_camera, wk->ica_anime );

  //@TODO
  // 片方の画面の表示位置をずらす
  if( wk->is_double )
  {
    VecFx32 pos;
    VecFx32 tar;
    
    static int num = 20;
    static int _pos_y = 0;
    static int _tar_y = 0;
    
    GFL_G3D_CAMERA_GetPos( p_camera, &pos );
    GFL_G3D_CAMERA_GetTarget( p_camera, &tar );

#ifdef DEBUG_CAMERA
    debug_camera_test()
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A )
    {
      wk->anime_speed = 0;
    }
    else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_B )
    {
      wk->anime_speed = FX32_ONE;
    }
#endif

    if( GFL_G3D_DOUBLE3D_GetFlip() )
    {
      // 下画面
    }
    else
    {
      // 上画面
      pos.y += _pos_y;
      tar.y += _tar_y;
    }
      
    GFL_G3D_CAMERA_SetPos( p_camera, &pos );
    GFL_G3D_CAMERA_SetTarget( p_camera, &tar );

//  HOSAKA_Printf("pos.y=%d tar.y=%d \n", pos.y, tar.y );

//  HOSAKA_Printf("camera pos= %d %d %d flip=%d \n", pos.x, pos.y, pos.z, GFL_G3D_DOUBLE3D_GetFlip() );
  }
  
  // アニメーション更新
  {
    int i;
    for( i=0; i<Demo3D_DATA_GetUnitMax( wk->demo_id ); i++ )
    {
      int j;
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[i] );
      int anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      for( j=0; j<anime_count; j++ )
      {
        GFL_G3D_OBJECT_LoopAnimeFrame( obj, j, wk->anime_speed );
      }
    }
  }

  // 描画
	DEMO3D_GRAPHIC_3D_StartDraw( wk->graphic );
  
  {
    int i;
    GFL_G3D_OBJSTATUS status;

    // ステータス初期化
    VEC_Set( &status.trans, 0, 0, 0 );
    VEC_Set( &status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
    MTX_Identity33( &status.rotate );

    for( i=0; i<Demo3D_DATA_GetUnitMax( wk->demo_id ); i++ )
    {
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[i] );
      GFL_G3D_DRAW_DrawObject( obj, &status );
    }
  }

	DEMO3D_GRAPHIC_3D_EndDraw( wk->graphic );

  if( wk->is_double )
  {
	  GFL_G3D_DOUBLE3D_SetSwapFlag();
  }

  // ループ検出で終了
  return is_end;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  現在のフレーム値を取得
 *
 *	@param	DEMO3D_ENGINE_WORK* wk 　ワーク
 *
 *	@retval フーレム値
 */
//-----------------------------------------------------------------------------
fx32 DEMO3D_ENGINE_GetNowFrame( DEMO3D_ENGINE_WORK* wk )
{
  GF_ASSERT(wk);
  GF_ASSERT(wk->ica_anime);

  return ICA_ANIME_GetNowFrame( wk->ica_anime );
}



