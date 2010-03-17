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
#include "system/rtc_tool.h"

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

#include "demo3d_local.h"
#include "demo3d_graphic.h"
#include "demo3d_data.h"
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
//  u32                     start_frame;

  // [PRIVATE]
  BOOL          is_double;
  fx32          anime_speed;  ///< アニメーションスピード
  ICA_ANIME*    ica_anime;
  GFL_G3D_UTIL*   g3d_util;
  GFL_G3D_CAMERA* camera;
  fx32 def_top;
  fx32 def_bottom;
  DEMO3D_CMD_WORK*  cmd;

  u16* unit_idx; // unit_idx保持（ALLOC)

  //シーンパラメータ保持
  const DEMO3D_SCENE_DATA* scene;
  DEMO3D_SCENE_ENV        env;
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static void set_SceneParam( const DEMO3D_ENGINE_WORK* wk, DEMO3D_PARAM* param, const DEMO3D_SCENE_DATA* scene, HEAPID heapID );

//=============================================================================
/**
 *								static関数
 */
//=============================================================================

// DoubleDisp用VIntr割り込み関数
static void vintrFunc(void)
{
	GFL_G3D_DOUBLE3D_VblankIntr();
}

//-------------------------------------
///	カメラ位置
//=====================================
static const VecFx32 sc_CAMERA_PER_POS		= { 0,0,FX32_CONST( 0 ) };	//位置
static const VecFx32 sc_CAMERA_PER_UP			= { 0,FX32_ONE,0 };					//上方向
static const VecFx32 sc_CAMERA_PER_TARGET	= { 0,0,FX32_CONST( 0 ) };	//ターゲット


//-----------------------------------------------------------------------------
/**
 *	@brief  Frustカメラ 削除
 *
 *	@param	GFL_G3D_CAMERA* p_camera 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void FrustCamera_Delete( GFL_G3D_CAMERA* p_camera )
{
  GF_ASSERT( p_camera );
  GFL_G3D_CAMERA_Delete( p_camera );
}


//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  3Dグラフィック 初期化
 *
 *	@param	DEMO3D_GRAPHIC_WORK* graphic  グラフィックワーク
 *	@param	DEMO3D_PARAM*
 *	@param	heapID ヒープID
 *
 *	@retval DEMO3D_ENGINE_WORK* ワーク
 */
//-----------------------------------------------------------------------------
DEMO3D_ENGINE_WORK* Demo3D_ENGINE_Init( DEMO3D_GRAPHIC_WORK* graphic, DEMO3D_PARAM* param, HEAPID heapID )
{
  DEMO3D_ENGINE_WORK* wk;
  u8 unit_max;

  GF_ASSERT( graphic );

  // メインワーク アロケート
  wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(DEMO3D_ENGINE_WORK) );

  // メンバ初期化
  wk->graphic       = graphic;
  wk->demo_id       = param->demo_id;
  
  //環境パラメータセット
  wk->env.demo_id = wk->demo_id;
  wk->env.scene_id = param->scene_id;
  
  wk->env.start_frame = param->start_frame;
  wk->env.time_zone = GFL_RTC_ConvertHourToTimeZone( param->hour );
  wk->env.player_sex = param->player_sex;
  wk->env.hour = param->hour;
  wk->env.min = param->min;
  wk->env.season = param->season;

  wk->scene = Demo3D_DATA_GetSceneData( wk->demo_id );

  IWASAWA_Printf("start_frame=%d\n",wk->env.start_frame);

  // コンバータデータからカメラ生成
  {
    fx32 fovySin, fovyCos;
    
    fovySin = FX_SinIdx( (wk->scene->fovy_sin>>FX32_SHIFT) / 2 * PERSPWAY_COEFFICIENT );
    fovyCos = FX_CosIdx( (wk->scene->fovy_cos>>FX32_SHIFT) / 2 * PERSPWAY_COEFFICIENT );

    wk->camera = GFL_G3D_CAMERA_CreateFrustumByPersPrmW( 
                  &sc_CAMERA_PER_POS, &sc_CAMERA_PER_UP, &sc_CAMERA_PER_TARGET,
                  fovySin, fovyCos, defaultCameraAspect, wk->scene->near, wk->scene->far, wk->scene->scale_w, heapID );

    // デフォルトのtop/buttomを取得
    GFL_G3D_CAMERA_GetTop( wk->camera, &wk->def_top );
    GFL_G3D_CAMERA_GetBottom( wk->camera, &wk->def_bottom );

  }
  set_SceneParam( wk, param, wk->scene, heapID );
  
  // アニメーションスピードを取得
  wk->anime_speed = Demo3D_DATA_GetAnimeSpeed( wk->demo_id );
  HOSAKA_Printf("anime_speed=%d\n", wk->anime_speed );
  
  // 2画面連結フラグを取得
  wk->is_double = Demo3D_DATA_GetDoubleFlag( wk->demo_id );

#ifdef PM_DEBUG
  //@TODO セレクトを押しながら起動すると二画面フラグ反転
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT )
  {
    wk->is_double ^= 1;
  }
#endif // PM_DEBUG
  
  HOSAKA_Printf("is_double=%d\n", wk->is_double );
  
  // 2画面連結設定初期化
  if( wk->is_double )
  {
    GFL_G3D_DOUBLE3D_Init( heapID );
		GFUser_SetVIntrFunc(vintrFunc);
  }
  
  wk->cmd = Demo3D_CMD_Init( wk->demo_id, wk->env.start_frame, heapID );

  unit_max = Demo3D_DATA_GetUnitMax( wk->demo_id );

  // ユニットID 動的配列 アロケート
  wk->unit_idx = GFL_HEAP_AllocClearMemory( heapID, sizeof(u16) * unit_max );

  // icaデータをロード
  wk->ica_anime = Demo3D_DATA_CreateICACamera( wk->demo_id, heapID, ICA_BUFF_SIZE );

  // icaアニメフレーム初期化
  ICA_ANIME_SetAnimeFrame( wk->ica_anime, FX32_CONST(wk->env.start_frame) );

  // 3D管理ユーティリティーの生成
  wk->g3d_util = GFL_G3D_UTIL_Create( 10, 16, heapID );

  // ユニット追加
  {
    GFL_G3D_UTIL_SETUP* setup;
    int i;

    setup = Demo3D_DATA_InitG3DUtilSetup(heapID);
    for( i=0; i<unit_max; i++ )
    {

      Demo3D_DATA_GetG3DUtilSetup( setup, &wk->env, i );

      IWASAWA_Printf("demoid=%d setup_idx=%d setup objcnt=%d resCount=%d \n",wk->demo_id, i, setup->objCount, setup->resCount);

      wk->unit_idx[i] = GFL_G3D_UTIL_AddUnit( wk->g3d_util, setup );
    }
    Demo3D_DATA_FreeG3DUtilSetup(setup);
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
        const int frame = FX32_CONST(wk->env.start_frame);

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
  // カメラ破棄
  FrustCamera_Delete( wk->camera );

  if( wk->is_double )
  {
		//終了
		GFUser_ResetVIntrFunc();
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

//===========================================================================
// debug camera test
//===========================================================================
#ifdef DEBUG_CAMERA_CONTROL

static BOOL debug_vec_move( VecFx32* pos, int num )
{ 
  if( CHECK_KEY_CONT( PAD_KEY_UP ) )
  {
    pos->y += num;
    //OS_Printf("{ 0x%x, 0x%x, 0x%x } \n", pos->x, pos->y, pos->z );
    return TRUE;
  }
  else if( CHECK_KEY_CONT( PAD_KEY_DOWN ) )
  {
    pos->y -= num;
    //OS_Printf("{ 0x%x, 0x%x, 0x%x } \n", pos->x, pos->y, pos->z );
    return TRUE;
  }    
  else if( CHECK_KEY_CONT( PAD_KEY_LEFT ) )
  {
    pos->x += num;
    //OS_Printf("{ 0x%x, 0x%x, 0x%x } \n", pos->x, pos->y, pos->z );
    return TRUE;
  }
  else if( CHECK_KEY_CONT( PAD_KEY_RIGHT ) )
  {
    pos->x -= num;
    //OS_Printf("{ 0x%x, 0x%x, 0x%x } \n", pos->x, pos->y, pos->z );
    return TRUE;
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_L ) )
  {
    pos->z += num;
    //OS_Printf("{ 0x%x, 0x%x, 0x%x } \n", pos->x, pos->y, pos->z );
    return TRUE;
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_R ) )
  {
    pos->z -= num;
    //OS_Printf("{ 0x%x, 0x%x, 0x%x } \n", pos->x, pos->y, pos->z );
    return TRUE;
  }

  return FALSE;
}

// データ吐き出し
static void _debug_vec_print( const VecFx32* pos, const VecFx32* tar )
{
  OS_Printf("======\n");
  OS_Printf("up pos = { 0x%x, 0x%x, 0x%x }; \n",
      pos[TRUE].x,
      pos[TRUE].y,
      pos[TRUE].z );
  
  OS_Printf("up tar = { 0x%x, 0x%x, 0x%x }; \n",
      tar[TRUE].x, 
      tar[TRUE].y,
      tar[TRUE].z );
  
  OS_Printf("down pos = { 0x%x, 0x%x, 0x%x }; \n",
      pos[FALSE].x,
      pos[FALSE].y,
      pos[FALSE].z );
  
  OS_Printf("down tar = { 0x%x, 0x%x, 0x%x }; \n",
      tar[FALSE].x, 
      tar[FALSE].y,
      tar[FALSE].z );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  DEBUG:カメラのPOS/TARGETを上下画面毎に操作/設定
 *
 *	@param	GFL_G3D_CAMERA* p_camera 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void debug_camera_control( GFL_G3D_CAMERA* p_camera )
{ 
  VecFx32 pos;
  VecFx32 tar;
  
  GFL_G3D_CAMERA_GetPos( p_camera, &pos );
  GFL_G3D_CAMERA_GetTarget( p_camera, &tar );
  {
    static BOOL _is_up = TRUE;

    static VecFx32 _pos[2] = {
      { 0 },
 //     { 0x0,   0xfffff844, 0x28 },
    };
    static VecFx32 _tar[2] = {
      { 0 },
//      { 0x140, 0x8ac,      0x0 },
    };

    static int _mode = 0;
    static int _num = 20;
   
    // 操作画面の切り替え
    if( CHECK_KEY_TRG( PAD_BUTTON_Y ) )
    {
      _is_up ^= 1;
      if( _is_up == 0 ){ OS_TPrintf("** set down disp **\n"); } else
      if( _is_up == 1 ){ OS_TPrintf("** set up disp **\n"); }
    }
    // 操作項目切り替え
    else if( CHECK_KEY_TRG( PAD_BUTTON_X ) )
    {
      _mode = (_mode+1)%2;
      if( _mode == 0 ){ OS_TPrintf("** set pos mode **\n"); } else
      if( _mode == 1 ){ OS_TPrintf("** set target mode **\n"); }
    }
    // 操作座標リセット
    else if( CHECK_KEY_TRG( PAD_BUTTON_SELECT ) )
    {
      _pos[_is_up] = (VecFx32){0};
      _tar[_is_up] = (VecFx32){0};
    }
    
    // 座標操作
    if( _mode == 0 )
    {
      if( debug_vec_move( &_pos[_is_up], _num ) )
      {
        _debug_vec_print( _pos, _tar );
      }
    }
    else
    {
      if( debug_vec_move( &_tar[_is_up], _num ) )
      {
        _debug_vec_print( _pos, _tar );
      }
    }

    if( GFL_G3D_DOUBLE3D_GetFlip() == FALSE )
    {
      // 操作座標反映
      pos.x += _pos[TRUE].x;
      pos.y += _pos[TRUE].y;
      pos.z += _pos[TRUE].z;
      tar.x += _tar[TRUE].x;
      tar.y += _tar[TRUE].y;
      tar.z += _tar[TRUE].z;
    }
    else
    {
      // 操作座標反映
      pos.x += _pos[FALSE].x;
      pos.y += _pos[FALSE].y;
      pos.z += _pos[FALSE].z;
      tar.x += _tar[FALSE].x;
      tar.y += _tar[FALSE].y;
      tar.z += _tar[FALSE].z;
    }
  }
    
  GFL_G3D_CAMERA_SetPos( p_camera, &pos );
  GFL_G3D_CAMERA_SetTarget( p_camera, &tar );

}

#endif // DEBUG_CAMERA_CONTROL

static void set_camera_disp_offset( DEMO3D_ENGINE_WORK* wk, GFL_G3D_CAMERA* p_camera )
{
  s16 top,bottom;

  // 上画面
  if( GFL_G3D_DOUBLE3D_GetFlip() == FALSE )
  {
    top = bottom = wk->scene->dview_main_ofs;
  }
  // 下画面
  else
  {
    top = bottom = wk->scene->dview_sub_ofs;
  }
#ifdef PM_DEBUG
  {
    int in = FALSE,key;
    static s16 ofs_m = 0, ofs_s = 0;
    fx32 offset;

    key =GFL_UI_KEY_GetCont();

    // デバッグ距離操作
    if( key & ( PAD_KEY_UP | PAD_BUTTON_R) ){
      ofs_m += 1;
      in = TRUE;
    } 
    else if( key & ( PAD_KEY_DOWN | PAD_BUTTON_L) ){
      ofs_m -= 1;
      in = TRUE;
    }
    if( key & ( PAD_KEY_RIGHT | PAD_BUTTON_R ) ){
      ofs_s += 1;
      in = TRUE;
    } 
    else if( key & ( PAD_KEY_LEFT | PAD_BUTTON_L ) ){
      ofs_s -= 1;
      in = TRUE;
    }
    if( in ) {
      OS_Printf("offset def_top = %f, def_bottom = %f\n",
          FX_FX32_TO_F32(wk->def_top), FX_FX32_TO_F32(wk->def_bottom));
      OS_Printf("offset_main view = %f, d = %d o = %d\n",
          FX_FX32_TO_F32(wk->scene->dview_main_ofs+wk->def_top+ofs_m),(wk->scene->dview_main_ofs+ofs_m),ofs_m );
      OS_Printf("offset_sub view = %f, d = %d o= %d\n",
          FX_FX32_TO_F32(wk->scene->dview_sub_ofs+wk->def_top+ofs_s),(wk->scene->dview_sub_ofs+ofs_s),ofs_s );
    }
    if( GFL_G3D_DOUBLE3D_GetFlip() == FALSE ){
      offset = ofs_m;
    }else{
      offset = ofs_s;
    }
    // クリップ面をずらす
    GFL_G3D_CAMERA_SetTop( p_camera, top+wk->def_top+offset );
    GFL_G3D_CAMERA_SetBottom( p_camera, bottom+wk->def_bottom+offset );
  }
#else
  // クリップ面をずらす
  GFL_G3D_CAMERA_SetTop( p_camera, top+wk->def_top );
  GFL_G3D_CAMERA_SetBottom( p_camera, buttom+wk->def_bottom );
#endif
}

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

  p_camera = wk->camera;
  
  // ICAカメラ更新
  is_end = ICA_ANIME_IncAnimeFrame( wk->ica_anime, wk->anime_speed );

  // ICAカメラ座標を設定
  ICA_CAMERA_SetCameraStatus( p_camera, wk->ica_anime );
  
#ifdef DEBUG_USE_KEY
  // アニメ再生切り替え
  if( CHECK_KEY_TRG( PAD_BUTTON_START ) )
  {
    static BOOL _is_anime = TRUE;
    _is_anime ^= 1;
    if( _is_anime == TRUE ) {
      wk->anime_speed = Demo3D_DATA_GetAnimeSpeed( wk->demo_id );
    } else {
      wk->anime_speed = 0;
    }
  }
#endif

  // 片方の画面の表示位置をずらす
  if( wk->is_double )
  {
#ifdef DEBUG_CAMERA_CONTROL
    // カメラのPOS/TARGETを上下画面毎に操作/設定
    debug_camera_control( p_camera );
#else
    set_camera_disp_offset( wk, p_camera );
#endif
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

  // カメラ切り替え
  GFL_G3D_CAMERA_Switching( p_camera );

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
fx32 DEMO3D_ENGINE_GetNowFrame( const DEMO3D_ENGINE_WORK* wk )
{
  GF_ASSERT(wk);
  GF_ASSERT(wk->ica_anime);

  return ICA_ANIME_GetNowFrame( wk->ica_anime );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  シーンパラメータ設定
 *
 *	@param	DEMO3D_ENGINE_WORK* wk 　ワーク
 *
 *	@retval フーレム値
 */
//-----------------------------------------------------------------------------
static void set_SceneParam( const DEMO3D_ENGINE_WORK* wk, DEMO3D_PARAM* param, const DEMO3D_SCENE_DATA* scene, HEAPID heapID )
{
  //フィールドライト設定引継ぎ
  {
    FIELD_LIGHT_STATUS fld_light;

    FIELD_LIGHT_STATUS_Get( scene->zone_id,
      param->hour, param->min, WEATHER_NO_SUNNY, param->season, &fld_light, heapID );
  
    DEMO3D_GRAPHIC_Scene3DParamSet( wk->graphic, &fld_light, NULL );
  }

	G3X_AntiAlias( scene->anti_alias_f ); //	アンチエイリアス
	G3X_AlphaTest( scene->alpha_test_f, wk->scene->alpha_test_val );  //	アルファテスト　　オフ
	G3X_AlphaBlend( scene->alpha_blend_f );   //	アルファブレンド　オン

	G3X_EdgeMarking( scene->edge_marking_f ); //	エッジマーキング
  if( scene->edge_marking_f ){
    G3X_SetEdgeColorTable( scene->edge_col );
  }

	G3X_SetFog( scene->fog_f, scene->fog_mode, scene->fog_shift, scene->fog_offset );	//フォグ
  if( scene->fog_f ){
    u32 i, tbl[8];
    G3X_SetFogColor( scene->fog_col, scene->fog_alpha );

    for( i = 0;i < 8;i++){
      tbl[i] = scene->fog_tbl[i]; //u8[]からu32[]に変換
    }
    G3X_SetFogTable( tbl );
  }
	// クリアカラーの設定
	G3X_SetClearColor( scene->clear_col, scene->clear_alpha, scene->clear_depth,
                     scene->clear_poly_id, scene->fog_f );	//color,alpha,depth,polygonID,fog

  G3X_SetDisp1DotDepth( scene->poly_1dot_depth);

  	//レンダリングスワップバッファ
	GFL_G3D_SetSystemSwapBufferMode( scene->sbuf_sort_mode, scene->sbuf_buf_mode );
}




