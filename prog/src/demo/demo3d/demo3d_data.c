//============================================================================
/**
 *
 *	@file		demo3d_data.c
 *	@brief  3Dデモエンジン コンバートデータへのアクセサ
 *	@author		hosaka genya -> miyuki iwasawa
 *	@data		2009.11.27
 *
 */
//============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/main.h"
#include "system/ica_anime.h"
#include "arc/arc_def.h"

#include "sound/pm_sndsys.h" // for SEQ_SE_XXX

#include "demo/demo3d.h"

#include "arc/fieldmap/zone_id.h"
#include "arc/demo3d.naix"

#include "demo3d_data.h"

#define DEMO3D_BGM_NONE     (0xFFFF)
#define DEMO3D_DATA_RES_MAX (6) //1unitのリソースmax(imd+アニメ5種)

typedef enum{
  DEMO3D_FRAME_RATE_60FPS,
  DEMO3D_FRAME_RATE_30FPS,
}DEMO3D_FRAME_RATE;

typedef enum{
  DEMO3D_UNITCHG_NONE,
  DEMO3D_UNITCHG_PLAYER_SEX,
  DEMO3D_UNITCHG_SCENE_ID,
  DEMO3D_UNITCHG_TYPE_MAX,
}DEMO3D_UNITCHG_TYPE;

typedef struct _DEMO3D_FILE_UNIT {
  const u16* file_tbl;
  u32   num;
}DEMO3D_FILE_UNIT;

typedef struct _DEMO3D_FILE_UNIT_TBL {
  const DEMO3D_FILE_UNIT* unit_tbl;
  u32   num;
}DEMO3D_FILE_UNIT_TBL;

typedef struct _DEMO3D_SCENE_UNIT {
  const DEMO3D_FILE_UNIT_TBL* tbl;
  u16   num;
  u16   type;
}DEMO3D_SCENE_UNIT;

//--------------------------------------------------------------
///	デモ セットアップデータ
//==============================================================
typedef struct {
  const DEMO3D_SCENE_DATA* scene_data;
  const DEMO3D_CMD_DATA* cmd_data;
  const DEMO3D_CMD_DATA* end_cmd_data;
  const DEMO3D_SCENE_UNIT* scene_unit;
  u32   scene_unit_num;

#if 0
  const GFL_G3D_UTIL_SETUP* setup;
  int max;
  int camera_idx;
  fx32 anime_speed;
  fx32 fovy_sin;
  fx32 fovy_cos;
  BOOL is_double;
#endif
} DEMO3D_SETUP_DATA;

#define DEMO_DATA(id) (c_demo3d_setup_data[id])

////////////////////////////////////////////////////////////////////////
///リテラル

// コンバートデータ
#include "data/demo3d_resdata.cdat"

////////////////////////////////////
// セットアップ用固定データ

//アニメデータ(変更する必要がないのでconstデータを持ってしまえ)
static const GFL_G3D_UTIL_ANM DATA_g3d_util_anm[] = {
	{ 1, 0 },
	{ 2, 0 },
	{ 3, 0 },
	{ 4, 0 },
	{ 5, 0 },
};

///////////////////////////////////////////
//プロトタイプ
static const DEMO3D_FILE_UNIT_TBL* data_GetUnitTbl( const DEMO3D_SCENE_UNIT* unit, DEMO3D_SCENE_ENV* env );




//============================================================================
//メイン関数群
//============================================================================
/*
 *  @brief  デモID補正
 */
static DEMO3D_ID check_demo_id( DEMO3D_ID id )
{
  if( id == DEMO3D_ID_NULL || id >= DEMO3D_ID_MAX ){
    GF_ASSERT( id < DEMO3D_ID_MAX && id != DEMO3D_ID_NULL );
    return 0;
  }
  return id;
}

/*
 *  @brief  シーンデータ取得
 */
const DEMO3D_SCENE_DATA* Demo3D_DATA_GetSceneData( DEMO3D_ID id )
{
  id = check_demo_id(id);
  return DEMO_DATA(id).scene_data;
}

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
  const DEMO3D_SCENE_DATA* scene = Demo3D_DATA_GetSceneData( id );
  return ICA_ANIME_CreateStreamingAlloc( heapID, ARCID_DEMO3D_GRA, scene->camera_bin_id, buf_interval );
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
  id = check_demo_id(id);
  return DEMO_DATA(id).scene_unit_num;  //c_demo3d_setup_data[ id ].max;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  GFL_G3D_UTIL_SETUPの動的初期化
 *
 *	@param	DEMO3D_ID id  デモID
 *	@param	setup_idx     セットアップデータのインデックス
 *
 *	@retval GFL_G3D_UTIL_SETUP*
 */
//-----------------------------------------------------------------------------
GFL_G3D_UTIL_SETUP* Demo3D_DATA_InitG3DUtilSetup( HEAPID heapID )
{
  int i;

  GFL_G3D_UTIL_SETUP* sp = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_G3D_UTIL_SETUP));
  GFL_G3D_UTIL_RES*	resTbl = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_G3D_UTIL_RES)*DEMO3D_DATA_RES_MAX);
  GFL_G3D_UTIL_OBJ*	objTbl = GFL_HEAP_AllocClearMemory( heapID, sizeof(GFL_G3D_UTIL_OBJ));

  for(i = 0;i < DEMO3D_DATA_RES_MAX;i++){
    resTbl[i].arcive = ARCID_DEMO3D_GRA;
    resTbl[i].arcType = GFL_G3D_UTIL_RESARC;
  }
  objTbl->anmTbl = DATA_g3d_util_anm; //値が変わらないので固定  

  (GFL_G3D_UTIL_RES*)sp->resTbl = resTbl;
  (GFL_G3D_UTIL_OBJ*)sp->objTbl = objTbl;

  return sp;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  GFL_G3D_UTIL_SETUPの解放
 *
 *	@param GFL_G3D_UTIL_SETUP*
 */
//-----------------------------------------------------------------------------
void Demo3D_DATA_FreeG3DUtilSetup( GFL_G3D_UTIL_SETUP* sp )
{
  GFL_HEAP_FreeMemory((void*)sp->objTbl);
  GFL_HEAP_FreeMemory((void*)sp->resTbl);
  
  GFL_HEAP_FreeMemory(sp);
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
void Demo3D_DATA_GetG3DUtilSetup( GFL_G3D_UTIL_SETUP* sp, DEMO3D_SCENE_ENV* env, u8 idx )
{
  int i;
  u8  unit_id = 0;
  const DEMO3D_SCENE_UNIT* scene_unit;
  const DEMO3D_FILE_UNIT_TBL* tbl;
  const DEMO3D_FILE_UNIT* unit;

  GFL_G3D_UTIL_RES*	resTbl = (GFL_G3D_UTIL_RES*)sp->resTbl;
  GFL_G3D_UTIL_OBJ*	objTbl = (GFL_G3D_UTIL_OBJ*)sp->objTbl;

  scene_unit = &(DEMO_DATA(env->demo_id).scene_unit[idx]);
  tbl = data_GetUnitTbl( scene_unit, env );

  if( tbl->num > 1 ){
    unit_id = env->time_zone;
  }
  if( unit_id >= tbl->num ){
    GF_ASSERT(unit_id < tbl->num);
    unit_id = 0;
  }
  unit = &(tbl->unit_tbl[unit_id]);

  //リソース格納
  sp->resCount = unit->num;
  for(i = 0;i < DEMO3D_DATA_RES_MAX;i++){
    if(i < unit->num ){
      resTbl[i].datID = unit->file_tbl[i];
    }else{
      resTbl[i].datID = 0;
    }
  }
  sp->objCount = 1;
  objTbl->anmCount = unit->num-1;

#if 0
  const GFL_G3D_UTIL_SETUP* setup;

  id = check_demo_id(id);
  GF_ASSERT( setup_idx < Demo3D_DATA_GetUnitMax( id ) );

  setup = &c_demo3d_setup_data[ id ].setup[ setup_idx ];

  return setup;
#endif
}

static const DEMO3D_FILE_UNIT_TBL* data_GetUnitTbl( const DEMO3D_SCENE_UNIT* unit, DEMO3D_SCENE_ENV* env )
{
  u8 id = 0;
  switch( unit->type ){
  case DEMO3D_UNITCHG_PLAYER_SEX:
    id = env->player_sex;
    break;
  case DEMO3D_UNITCHG_SCENE_ID:
    id = env->scene_id;
  }
  if( id >= unit->num ){
    GF_ASSERT(id < unit->num);
    id = 0;
  }
  return &(unit->tbl[id]);
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
  const DEMO3D_SCENE_DATA* scene = Demo3D_DATA_GetSceneData( id );
  return (FX32_ONE >> scene->frame_rate);  //c_demo3d_setup_data[ id ].anime_speed;
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
  const DEMO3D_SCENE_DATA* scene = Demo3D_DATA_GetSceneData( id );
  return scene->double_view_f;  //c_demo3d_setup_data[ id ].is_double;
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
  const DEMO3D_SCENE_DATA* scene = Demo3D_DATA_GetSceneData( id );

  return scene->fovy_sin;  //c_demo3d_setup_data[ id ].fovy_sin;
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
  const DEMO3D_SCENE_DATA* scene = Demo3D_DATA_GetSceneData( id );

  return scene->fovy_cos;//c_demo3d_setup_data[ id ].fovy_cos;
}


// コンバートデータ読み込み
//#include "demo3d_cmd_data.cdat"

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
  id = check_demo_id(id);
  return DEMO_DATA(id).cmd_data;
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
  id = check_demo_id(id);
  return DEMO_DATA(id).end_cmd_data;
}


