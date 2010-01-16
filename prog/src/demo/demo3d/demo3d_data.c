//============================================================================
/**
 *
 *	@file		demo3d_data.c
 *	@brief  3Dデモエンジン コンバートデータへのアクセサ
 *	@author		hosaka genya
 *	@data		2009.11.27
 *
 */
//============================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/ica_anime.h"
#include "arc/arc_def.h"

#include "sound/pm_sndsys.h" // for SEQ_SE_XXX

#include "demo/demo3d.h"

#include "arc/demo3d.naix"

#include "demo3d_data.h"

//--------------------------------------------------------------
///	デモ セットアップデータ
//==============================================================
typedef struct {
  const GFL_G3D_UTIL_SETUP* setup;
  int max;
  int camera_idx;
  fx32 anime_speed;
  fx32 fovy_sin;
  fx32 fovy_cos;
  BOOL is_double;
} DEMO3D_SETUP_DATA;

// コンバートデータ
#include "demo3d_resdata.cdat"

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからICAカメラを生成
 *
 *	@param	DEMO3D_ID id  デモID
 *	@param	heapID  ヒープID
 *	@param	buf_interval バッファリングする感覚
 *
 *	@retval ICA_ANIME*
 */
//-----------------------------------------------------------------------------
ICA_ANIME* Demo3D_DATA_CreateICACamera( DEMO3D_ID id, HEAPID heapID, int buf_interval )
{
  GF_ASSERT( id < DEMO3D_ID_MAX && id != DEMO3D_ID_NULL );
  return ICA_ANIME_CreateStreamingAlloc( heapID, ARCID_DEMO3D_GRA, c_demo3d_setup_data[id].camera_idx, buf_interval );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからユニット数を取得
 *
 *	@param	DEMO3D_ID id  デモID
 *
 *	@retval　ユニット数
 */
//-----------------------------------------------------------------------------
u8 Demo3D_DATA_GetUnitMax( DEMO3D_ID id )
{
  GF_ASSERT( id < DEMO3D_ID_MAX && id != DEMO3D_ID_NULL );
  return c_demo3d_setup_data[ id ].max;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからセットアップデータを取得
 *
 *	@param	DEMO3D_ID id  デモID
 *	@param	setup_idx     セットアップデータのインデックス
 *
 *	@retval GFL_G3D_UTIL_SETUP*
 */
//-----------------------------------------------------------------------------
const GFL_G3D_UTIL_SETUP* Demo3D_DATA_GetG3DUtilSetup( DEMO3D_ID id, u8 setup_idx )
{
  const GFL_G3D_UTIL_SETUP* setup;

  GF_ASSERT( id < DEMO3D_ID_MAX && id != DEMO3D_ID_NULL );
  GF_ASSERT( setup_idx < Demo3D_DATA_GetUnitMax( id ) );

  setup = &c_demo3d_setup_data[ id ].setup[ setup_idx ];

  return setup;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからアニメーションスピードを取得
 *
 *	@param	DEMO3D_ID id  デモID
 *
 *	@retval fx32 アニメーションスピード
 */
//-----------------------------------------------------------------------------
const fx32 Demo3D_DATA_GetAnimeSpeed( DEMO3D_ID id )
{
  return c_demo3d_setup_data[ id ].anime_speed;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータから2画面連結表示フラグを取得
 *
 *	@param	DEMO3D_ID id  デモID
 *
 *	@retval BOOL 2画面フラグ
 */
//-----------------------------------------------------------------------------
const BOOL Demo3D_DATA_GetDoubleFlag( DEMO3D_ID id )
{
  return c_demo3d_setup_data[ id ].is_double;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからパース値SINを取得
 *
 *	@param	DEMO3D_ID id  デモID
 *
 *	@retval fx32 fovy_sin パース値SIN
 */
//-----------------------------------------------------------------------------
const fx32 Demo3D_DATA_GetCameraFovySin( DEMO3D_ID id )
{
  GF_ASSERT( id < DEMO3D_ID_MAX && id != DEMO3D_ID_NULL );

  return c_demo3d_setup_data[ id ].fovy_sin;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからパース値COSを取得
 *
 *	@param	DEMO3D_ID id  デモID
 *
 *	@retval fx32 fovy_sin パース値COS
 */
//-----------------------------------------------------------------------------
const fx32 Demo3D_DATA_GetCameraFovyCos( DEMO3D_ID id )
{
  GF_ASSERT( id < DEMO3D_ID_MAX && id != DEMO3D_ID_NULL );

  return c_demo3d_setup_data[ id ].fovy_cos;
}


// コンバートデータ読み込み
#include "demo3d_cmd_data.cdat"

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからコマンドデータを取得
 *
 *	@param	DEMO3D_ID id デモID
 *
 *	@retval DEMO3D_CMD_DATA* コマンドデータ配列へのポインタ
 */
//-----------------------------------------------------------------------------
const DEMO3D_CMD_DATA* Demo3D_DATA_GetCmdData( DEMO3D_ID id )
{
  GF_ASSERT( id < DEMO3D_ID_MAX && id != DEMO3D_ID_NULL );
  return demo3d_cmd_access_tbl[ id ];
}

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータから終了コマンドデータを取得
 *
 *	@param	DEMO3D_ID id デモID
 *
 *	@retval DEMO3D_CMD_DATA* コマンドデータ配列へのポインタ
 */
//-----------------------------------------------------------------------------
const DEMO3D_CMD_DATA* Demo3D_DATA_GetEndCmdData( DEMO3D_ID id )
{
  GF_ASSERT( id < DEMO3D_ID_MAX && id != DEMO3D_ID_NULL );
  return demo3d_endcmd_access_tbl[ id ];
}


