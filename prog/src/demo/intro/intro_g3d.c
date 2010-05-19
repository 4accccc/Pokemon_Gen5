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
#include "sound/pm_sndsys.h"

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

#define	UNIT_NONE		( 0xffff )	///< ユニットなし

enum {
	UNIT_ANM_SELECT_CA = 0,
	UNIT_ANM_SELECT_MA,
	UNIT_ANM_SELECT_TP,
	UNIT_ANM_START_CA,
	UNIT_ANM_START_MA,
};

//RES
static const GFL_G3D_UTIL_RES res_unit_select[] = {
  { ARCID_INTRO_GRA, NARC_intro_intro_bg_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_GRA, NARC_intro_intro_bg_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_GRA, NARC_intro_intro_bg_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_GRA, NARC_intro_intro_bg_nsbtp, GFL_G3D_UTIL_RESARC },

  { ARCID_INTRO_GRA, NARC_intro_intro_bg_b_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_INTRO_GRA, NARC_intro_intro_bg_b_nsbma, GFL_G3D_UTIL_RESARC },
};
//ANM
static const GFL_G3D_UTIL_ANM anm_unit_select[] = {
	// 男女選択
  { 1, 0 },
  { 2, 0 },
  { 3, 0 },
	// 男女選択開始演出
  { 4, 0 },
  { 5, 0 },
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


#if 0
static const VecFx32 sc_camera_pos    = { 0xfffffdeb, 0x809b, CAMERA_POS_Z };
static const VecFx32 sc_camera_up     = { 0x0, 0x4272, 0x9f0 };
static const VecFx32 sc_camera_target = { 0x0, 0x23e0, 0xffff3802 }; 

static const VecFx32 sc_camera_pos = { 0, FX32_CONST(7), FX32_CONST(32) }; 
static const VecFx32 sc_camera_up =  { 0x0, 0x4272, 0x9f0 }; 
static const VecFx32 sc_camera_target = { 0x0, FX32_CONST(5.1), FX32_CONST(0.5) };
#endif

// カメラ設定
/*
#define CAMERA_POS_Z	( FX32_CONST(32.728027f) )
static const VecFx32 sc_camera_pos = { 0x0, FX32_CONST(7.996582f), CAMERA_POS_Z }; 
static const VecFx32 sc_camera_up =  { 0x0, FX32_CONST(1), 0x0 }; 
static const VecFx32 sc_camera_target = { 0x0, FX32_CONST(4.541260f), FX32_CONST(-0.790039f) }; 
*/
#define CAMERA_POS_Z	( 0 )
/*
static const VecFx32 sc_camera_pos = { 0, 0, FX32_CONST(300) }; 
static const VecFx32 sc_camera_up =  { 0, FX32_ONE, 0 }; 
static const VecFx32 sc_camera_target = { 0, 0, 0 }; 
*/
/*
	{
    static const VecFx32 cam_pos = {FX32_CONST(-41.0f),
                                    FX32_CONST(  0.0f),
                                    FX32_CONST(101.0f)};
    static const VecFx32 cam_target = {FX32_CONST(0.0f),
                                       FX32_CONST(0.0f),
                                       FX32_CONST(-1.0f)};
    static const VecFx32 cam_up = {0,FX32_ONE,0};
*/
#define	ORTHO_HEIGHT	( 6 )
#define	ORTHO_WIDTH		( 8 )


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
  BOOL          is_load;
  u32	is_mode;
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
INTRO_G3D_WORK* INTRO_G3D_Create( INTRO_GRAPHIC_WORK* graphic, INTRO_SCENE_ID scene, HEAPID heap_id )
{
  INTRO_G3D_WORK* wk;

  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof( INTRO_G3D_WORK ) );
  
  wk->heap_id = heap_id;
  wk->graphic = graphic;

  // ユーティリティーをセットアップ
  wk->g3d_util = GFL_G3D_UTIL_Create( 10, 16, heap_id );

  // 専用カメラを用意
	{
		VecFx32 pos			= { 0, 0, FX32_ONE };
		VecFx32 up			= { 0, FX32_ONE, 0 };
		VecFx32 target	= { 0, 0, -FX32_ONE };

	  wk->camera = GFL_G3D_CAMERA_Create(
									GFL_G3D_PRJORTH,
									FX32_CONST(ORTHO_HEIGHT),
									-FX32_CONST(ORTHO_HEIGHT),
									-FX32_CONST(ORTHO_WIDTH),
									FX32_CONST(ORTHO_WIDTH),
									FX32_ONE,
									FX32_CONST(1024),
									FX32_ONE,
									&pos, &up, &target, heap_id );
	}
  
	if( scene != INTRO_SCENE_ID_05_RETAKE_YESNO ){
    int i;
    
	  // ユニット追加
    for( i=0; i<UNIT_MAX; i++ )
    {
      const GFL_G3D_UTIL_SETUP* setup;

      setup = &sc_g3d_setup[i];

      wk->unit_idx[i] = GFL_G3D_UTIL_AddUnit( wk->g3d_util, setup );
    }
  
	  // アニメーション有効化
/*
    for( i=0; i<UNIT_MAX; i++ )
    {
      int j;
      GFL_G3D_OBJ* obj;
      int anime_count;
      
      obj         = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[i] );
      anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );

      for( j=0; j<anime_count; j++ )
      {
        const int frame = FX32_CONST(20); 

        GFL_G3D_OBJECT_EnableAnime( obj, j );
        GFL_G3D_OBJECT_SetAnimeFrame( obj, j, &frame );
      }
    }
*/
		{
      GFL_G3D_OBJ * obj = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[0] );
			GFL_G3D_OBJECT_EnableAnime( obj, UNIT_ANM_START_CA );
			GFL_G3D_OBJECT_EnableAnime( obj, UNIT_ANM_START_MA );
			GFL_G3D_OBJECT_DisableAnime( obj, UNIT_ANM_SELECT_CA );
			GFL_G3D_OBJECT_DisableAnime( obj, UNIT_ANM_SELECT_MA );
			GFL_G3D_OBJECT_DisableAnime( obj, UNIT_ANM_SELECT_TP );
		}
  }else{
    int i;
    for( i=0; i<UNIT_MAX; i++ ){
      wk->unit_idx[i] = UNIT_NONE;
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
		if( wk->unit_idx[i] != UNIT_NONE ){
			GFL_G3D_UTIL_DelUnit( wk->g3d_util, wk->unit_idx[i] );
		}
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
#ifdef PM_DEBUG
#include "..\..\ui\debug_code\g3d\camera_test.c"
#include "..\..\ui\debug_code\g3d\g3d_util_anime_frame_test.c"
#endif

void INTRO_G3D_Main( INTRO_G3D_WORK* wk )
{
  int i;
  
#ifdef PM_DEBUG
  // アニメーション調整
//  debug_g3d_util_anime_frame_test( wk->g3d_util, 0 );
 
  // 座標調整
//  debug_camera_test( wk->camera );
#endif

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

	switch( wk->start_seq ){
	case 0:
    // ブレンドモード、対象面指定
    G2_SetBlendAlpha( GX_PLANEMASK_BG0, GX_PLANEMASK_BG3|GX_PLANEMASK_BG0, 0, 0 );
		PMSND_PlaySE( SEQ_SE_OPEN2 );
    wk->start_seq++;

	case 1:
		{
			GFL_G3D_OBJ * obj;
			BOOL	caFlag, maFlag;
			obj = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[0] );
			caFlag = GFL_G3D_OBJECT_IncAnimeFrame( obj, UNIT_ANM_START_CA, FX32_ONE );
			maFlag = GFL_G3D_OBJECT_IncAnimeFrame( obj, UNIT_ANM_START_MA, FX32_ONE );
			if( caFlag == FALSE && maFlag == FALSE ){
				GFL_G3D_OBJECT_DisableAnime( obj, UNIT_ANM_START_CA );
				GFL_G3D_OBJECT_DisableAnime( obj, UNIT_ANM_START_MA );
				GFL_G3D_OBJECT_EnableAnime( obj, UNIT_ANM_SELECT_CA );
				GFL_G3D_OBJECT_EnableAnime( obj, UNIT_ANM_SELECT_MA );
				GFL_G3D_OBJECT_EnableAnime( obj, UNIT_ANM_SELECT_TP );
				// デフォルト位置のフレーム
				INTRO_G3D_SelectSet( wk, INTRO_3D_SEL_SEX_DEF_FRAME );
		    wk->start_seq++;
			}
		}
		break;

	case 2:
		return TRUE;
	}

#if 0
  switch( wk->start_seq )
  {
  case 0:
		// デフォルト位置のフレーム
		INTRO_G3D_SelectSet( wk, INTRO_3D_SEL_SEX_DEF_FRAME );
    // カメラを離す
/*
    GFL_G3D_CAMERA_GetPos( wk->camera, &pos );
    pos.z = FX32_ONE;
    GFL_G3D_CAMERA_SetPos( wk->camera, &pos );
*/
    // ブレンドモード、対象面指定
    G2_SetBlendAlpha( GX_PLANEMASK_BG0, GX_PLANEMASK_BG3|GX_PLANEMASK_BG0, 0, 0 );
		PMSND_PlaySE( SEQ_SE_OPEN2 );
    wk->start_seq++;
    break;

  case 1:
/*
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
*/
		return TRUE;

  default : GF_ASSERT(0);
  }
#endif

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
void INTRO_G3D_SelectDecideStart( INTRO_G3D_WORK* wk, u32 is_mode )
{
  wk->is_mode = is_mode;
  wk->seq = 0;

	switch( wk->is_mode ){
	case INTRO_3D_SEL_SEX_MODE_MOVE_DEFAULT:
		wk->cnt = INTRO_3D_SEL_SEX_DEF_FRAME;
		wk->is_mode = INTRO_3D_SEL_SEX_MODE_MOVE_MALE;
		break;

	case INTRO_3D_SEL_SEX_MODE_MOVE_MALE:
		wk->cnt = INTRO_3D_SEL_SEX_MOVE_FEMALE_FRAME;
		break;

	case INTRO_3D_SEL_SEX_MODE_MOVE_FEMALE:
		wk->cnt = INTRO_3D_SEL_SEX_MOVE_MALE_FRAME;
		break;

	case INTRO_3D_SEL_SEX_MODE_ENTER_MALE:
		wk->cnt = INTRO_3D_SEL_SEX_MOVE_MALE_FRAME;
		break;

	case INTRO_3D_SEL_SEX_MODE_ENTER_FEMALE:
		wk->cnt = INTRO_3D_SEL_SEX_MOVE_FEMALE_FRAME;
		break;
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
	switch( wk->is_mode ){
	case INTRO_3D_SEL_SEX_MODE_MOVE_MALE:
    wk->cnt--;
		INTRO_G3D_SelectSet( wk, wk->cnt );
    if( wk->cnt == INTRO_3D_SEL_SEX_MOVE_MALE_FRAME ){ return TRUE; }
		break;

	case INTRO_3D_SEL_SEX_MODE_MOVE_FEMALE:
    wk->cnt++;
		INTRO_G3D_SelectSet( wk, wk->cnt );
    if( wk->cnt == INTRO_3D_SEL_SEX_MOVE_FEMALE_FRAME ){ return TRUE; }
		break;

	case INTRO_3D_SEL_SEX_MODE_ENTER_MALE:
    wk->cnt--;
		INTRO_G3D_SelectSet( wk, wk->cnt );
    if( wk->cnt == INTRO_3D_SEL_SEX_ENTER_MALE_FRAME ){ return TRUE; }
		break;

	case INTRO_3D_SEL_SEX_MODE_ENTER_FEMALE:
    wk->cnt++;
		INTRO_G3D_SelectSet( wk, wk->cnt );
    if( wk->cnt == INTRO_3D_SEL_SEX_ENTER_FEMALE_FRAME ){ return TRUE; }
		break;
	}
/*
  if( wk->is_mode )
  {
    wk->cnt--;
    INTRO_G3D_SelectSet( wk, wk->cnt );
    if( wk->cnt <= 0 ){ return TRUE; }
  }
  else
  {
    wk->cnt++;
    INTRO_G3D_SelectSet( wk, wk->cnt );
    if( wk->cnt >= 120 ){ return TRUE; }
  }
*/

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
  if( wk->is_mode == INTRO_3D_SEL_SEX_MODE_ENTER_MALE )
  {
    wk->cnt++;
    INTRO_G3D_SelectSet( wk, wk->cnt );
  }
  else
  {
    wk->cnt--;
    INTRO_G3D_SelectSet( wk, wk->cnt );
  }

  if( wk->cnt == INTRO_3D_SEL_SEX_DEF_FRAME ){
		return TRUE;
	}

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
/*
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
*/
	GFL_G3D_OBJ * obj;
	int frm;

	obj = GFL_G3D_UTIL_GetObjHandle( wk->g3d_util, wk->unit_idx[0] );
	frm = FX32_CONST(in_frame);
	GFL_G3D_OBJECT_SetAnimeFrame( obj, UNIT_ANM_SELECT_CA, &frm );
	GFL_G3D_OBJECT_SetAnimeFrame( obj, UNIT_ANM_SELECT_MA, &frm );
	GFL_G3D_OBJECT_SetAnimeFrame( obj, UNIT_ANM_SELECT_TP, &frm );
}


//=============================================================================
/**
 *								static関数
 */
//=============================================================================


