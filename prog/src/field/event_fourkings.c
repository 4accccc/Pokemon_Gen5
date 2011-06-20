//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_fourkings.c
 *	@brief  四天王部屋　演出
 *	@author	tomoya takahashi
 *	@date		2009.11.21
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "system/ica_anime.h"
#include "system/ica_camera.h"

#include "sound/pm_sndsys.h"

#include "savedata/mystatus.h"

#include "arc/arc_def.h"
#include "arc/fourkings_scene.naix"

#include "field/field_const.h"
#include "field/eventwork.h"
#include "eventwork_def.h"

#include "event_fourkings.h"

#include "fieldmap.h"
#include "fldmmdl.h"
#include "field_camera.h"
#include "fieldmap_func.h"

#include "field_sound.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

// 配置アニメーションの有無
static const BOOL EV_DEMO_HAICHI_ANIME[ FIELD_EVENT_FOURKINGS_MAX ] = 
{
  FALSE,
  FALSE,
  TRUE,
  TRUE,
};
static const VecFx32 EV_DEMO_HAICHI_ANIME_POS[ FIELD_EVENT_FOURKINGS_MAX ] = 
{
  {0,0,0},
  {0,0,0},
  {GRID_TO_FX32(24), 0, GRID_TO_FX32(16)},
  {GRID_TO_FX32(16), 0, GRID_TO_FX32(16)},
};

// 頂上カメラID
static const u32 sc_EV_DEMO_CAMERA_ID_TBL[ FIELD_EVENT_FOURKINGS_MAX ] = 
{
  28,
  29,
  30,
  31
};

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	配置アニメーションワーク
//=====================================
typedef struct {
  
  // 
  FIELD_BMODEL* p_model;
  G3DMAPOBJST * p_objst;

  FIELD_BMODEL_MAN* p_modelman;

  
} EV_BMODEL_ANIME;

//-------------------------------------
///	イベントワーク
//=====================================
typedef struct 
{
  FIELDMAP_WORK* p_fieldmap;
  GAMESYS_WORK* p_gsys;

  // 四天王ナンバー
  u32 fourkings_no;
  
  // モデルアニメーション
  EV_BMODEL_ANIME bmodel;

} EV_CIRCLEWALK_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	イベント生成関数・イベント関数
//=====================================
static GMEVENT* EVENT_SetUp_CircleWalk( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap, u32 fourkings_no );
static GMEVENT_RESULT EVENT_CircleWalk( GMEVENT* p_event, int* p_seq, void* p_wk );


//-------------------------------------
/// 配置オブジェのアニメーション
//=====================================
static void EV_BMODEL_Init( EV_BMODEL_ANIME* p_wk, FIELD_BMODEL_MAN * bmodel_man, u32 fourkings_no );
static void EV_BMODEL_Exit( EV_BMODEL_ANIME* p_wk );
static void EV_BMODEL_Start( EV_BMODEL_ANIME* p_wk );
static BOOL EV_BMODEL_IsEnd( const EV_BMODEL_ANIME* cp_wk );


//----------------------------------------------------------------------------
/**
 *	@brief  歩いてあがる演出
 *
 *	@param	p_parent          親のイベント
 *	@param	p_gsys            ゲームデータ
 *	@param	p_fieldmap        フィールドデータ
 *	@param	fourkins_no       四天王ナンバー
 */
//-----------------------------------------------------------------------------
GMEVENT* EVENT_FourKings_CircleWalk( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap, u32 fourkins_no )
{
  GF_ASSERT( FIELD_EVENT_FOURKINGS_MAX > fourkins_no );
  return EVENT_SetUp_CircleWalk(p_gsys, p_fieldmap, fourkins_no);
}



//----------------------------------------------------------------------------
/**
 *	@brief  四天王かく部屋の頂上カメラIDを取得
 *
 *	@param	fourkins_no   四天王ナンバー
 *
 *	@return カメラID
 */
//-----------------------------------------------------------------------------
u32 EVENT_FourKings_GetCameraID( u32 fourkins_no )
{
  GF_ASSERT( fourkins_no < FIELD_EVENT_FOURKINGS_MAX );
  return sc_EV_DEMO_CAMERA_ID_TBL[ fourkins_no ];
}


//-----------------------------------------------------------------------------
/**
 *        private関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  四天王部屋　円形移動デモイベント　生成
 *
 *	@param	p_gsys        ゲームシステム
 *	@param	fieldmap      フィールドマップ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT* EVENT_SetUp_CircleWalk( GAMESYS_WORK* p_gsys, FIELDMAP_WORK* p_fieldmap, u32 fourkings_no )
{
  GMEVENT*              p_event;
  EV_CIRCLEWALK_WORK*   p_work;
  HEAPID heapID = FIELDMAP_GetHeapID( p_fieldmap );

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL, EVENT_CircleWalk, sizeof( EV_CIRCLEWALK_WORK ) );

  // イベントワークを初期化
  p_work              = GMEVENT_GetEventWork( p_event );
  p_work->p_fieldmap  = p_fieldmap;
  p_work->p_gsys      = p_gsys;
  p_work->fourkings_no= fourkings_no;


  // 生成したイベントを返す
  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  円形移動デモ　イベント
 *
 *	@param	p_event     イベントワーク
 *	@param	p_seq       シーケンス
 *	@param	p_wk        ワーク
 *
 *  @retval GMEVENT_RES_CONTINUE = 0, ///<イベント継続中
 *  @retval GMEVENT_RES_FINISH,       ///<イベント終了
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_CircleWalk( GMEVENT* p_event, int* p_seq, void* p_wk )
{
  EV_CIRCLEWALK_WORK*   p_work = p_wk;
  HEAPID heapID = FIELDMAP_GetHeapID( p_work->p_fieldmap );

  enum{
    SEQ_START,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  switch( *p_seq ){
  case SEQ_START:
    // メモリ残量の表示
    OS_Printf( "init start heap free size 0x%x\n", GFI_HEAP_GetHeapFreeSize(heapID) );
    // 配置モデル
    {
      FLDMAPPER* p_mapper = FIELDMAP_GetFieldG3Dmapper( p_work->p_fieldmap );
      FIELD_BMODEL_MAN* p_bmodelman = FLDMAPPER_GetBuildModelManager( p_mapper );

      EV_BMODEL_Init( &p_work->bmodel, p_bmodelman, p_work->fourkings_no );
    }

    // 配置アニメーション開始
    EV_BMODEL_Start( &p_work->bmodel ); 
    
    (*p_seq)++;
    break;

  case SEQ_MAIN:
    {
      BOOL result;
      result = EV_BMODEL_IsEnd( &p_work->bmodel );

      if( result )
      {
        (*p_seq) ++;
      }
    }
    break;

  case SEQ_EXIT:

    // 各デモ破棄
    EV_BMODEL_Exit( &p_work->bmodel );
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
    break;
  }

  return GMEVENT_RES_CONTINUE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  配置モデルのアニメーション初期化
 *
 *	@param	p_wk              ワーク
 *	@param	bmodel_man        配置管理システム
 *	@param	fourkings_no      四天王ナンバー
 */
//-----------------------------------------------------------------------------
static void EV_BMODEL_Init( EV_BMODEL_ANIME* p_wk, FIELD_BMODEL_MAN * bmodel_man, u32 fourkings_no )
{
  if( EV_DEMO_HAICHI_ANIME[ fourkings_no ] )
  {
    p_wk->p_model = FIELD_BMODEL_Create_Search( bmodel_man, BM_SEARCH_ID_NULL, &EV_DEMO_HAICHI_ANIME_POS[fourkings_no] );
    p_wk->p_objst = FIELD_BMODEL_MAN_SearchObjStatusPos( bmodel_man, BM_SEARCH_ID_NULL, &EV_DEMO_HAICHI_ANIME_POS[fourkings_no] );
    p_wk->p_modelman = bmodel_man;
  }
  else
  {
    p_wk->p_model = NULL;
    p_wk->p_objst = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void EV_BMODEL_Exit( EV_BMODEL_ANIME* p_wk )
{
  if( p_wk->p_model )
  {
    FIELD_BMODEL_Delete( p_wk->p_model );

    // 大本も動かしておく
    G3DMAPOBJST_setAnime( p_wk->p_modelman, p_wk->p_objst, 0, BMANM_REQ_REVERSE_START );
    G3DMAPOBJST_setAnime( p_wk->p_modelman, p_wk->p_objst, 0, BMANM_REQ_STOP );
    
    p_wk->p_model = NULL;
    p_wk->p_objst = NULL;
    p_wk->p_modelman = NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーションの開始
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void EV_BMODEL_Start( EV_BMODEL_ANIME* p_wk )
{
  if( p_wk->p_model )
  {
    FIELD_BMODEL_SetAnime( p_wk->p_model, 0, BMANM_REQ_START );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメーションの完了街
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval TRUE  完了
 *	@retval FALSE 途中
 */
//-----------------------------------------------------------------------------
static BOOL EV_BMODEL_IsEnd( const EV_BMODEL_ANIME* cp_wk )
{
  if( cp_wk->p_model )
  {
    return FIELD_BMODEL_WaitAnime( cp_wk->p_model, 0 );
  }
  return TRUE;
}


//-----------------------------------------------------------------------------
/**
 *      雷音用のシステム
 */
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
#define GHOST_SPARK_SOUND_ANM_INDEX (1)

// 位置
static const VecFx32 sc_GHOST_SPARK_SARCH_POS = 
{
  FX32_CONST(256), 0, FX32_CONST(240)
};

// 音のタイミング
//static fx32 sc_GHOST_SPARK_SOUND_TIMING[] = {
static const fx32 sc_GHOST_SPARK_SOUND_TIMING[] = {
  FX32_CONST(6), 
};

#define GHOST_SPARK_SOUND_TIMING_TBL_MAX  (NELEMS(sc_GHOST_SPARK_SOUND_TIMING))


// サウンドID
#define GHOST_SND_SE_SPARK      ( SEQ_SE_FLD_33 )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	SPARK SOUND
//=====================================
typedef struct 
{
  FIELD_BMODEL_MAN* p_man;
  const G3DMAPOBJST* cp_spark_obj;

  fx32 next_frame;
  int tbl_index;
} GHOST_SPARK_SOUND;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static void GHOST_SPARK_SOUND_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void GHOST_SPARK_SOUND_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void GHOST_SPARK_SOUND_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );


const FLDMAPFUNC_DATA c_FLDMAPFUNC_GHOST_SPARK_SOUND = 
{
  128,
  sizeof(GHOST_SPARK_SOUND),
  GHOST_SPARK_SOUND_Create,
  GHOST_SPARK_SOUND_Delete,
  GHOST_SPARK_SOUND_Update,
  NULL,
};




//----------------------------------------------------------------------------
/**
 *	@brief  ゴースト　雷音　管理
 */
//-----------------------------------------------------------------------------
static void GHOST_SPARK_SOUND_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  GHOST_SPARK_SOUND* p_wk = p_work;
  FLDMAPPER* p_mapper = FIELDMAP_GetFieldG3Dmapper( p_fieldmap );
  FIELD_BMODEL_MAN* p_model_man = FLDMAPPER_GetBuildModelManager( p_mapper );
  fx32 start_frame;
  int  i;

  // 動作配置オブジェ
  p_wk->cp_spark_obj  = FIELD_BMODEL_MAN_SearchObjStatusPos( p_model_man, BM_SEARCH_ID_NULL, &sc_GHOST_SPARK_SARCH_POS );
  p_wk->p_man         = p_model_man;

  GF_ASSERT(p_wk->cp_spark_obj);
  GF_ASSERT(p_wk->p_man);
  
  start_frame = G3DMAPOBJST_getAnimeFrame( p_wk->p_man, p_wk->cp_spark_obj, GHOST_SPARK_SOUND_ANM_INDEX );
  
  // 開始時の音だしフレームを設定
  p_wk->next_frame  = sc_GHOST_SPARK_SOUND_TIMING[0];
  p_wk->tbl_index   = 0;
  for( i=0; i<GHOST_SPARK_SOUND_TIMING_TBL_MAX; i++ )
  {
    if( start_frame < sc_GHOST_SPARK_SOUND_TIMING[i] )
    {
      p_wk->next_frame  = sc_GHOST_SPARK_SOUND_TIMING[i];
      p_wk->tbl_index   = i;
      break;
    }
  }

  // ループ再生開始
  {
    GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
    GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
    FIELD_SOUND* p_fieldsound = GAMEDATA_GetFieldSound( p_gdata );

    FSND_PlayEnvSE( p_fieldsound, SEQ_SE_FLD_111 );
  }
  //PMSND_PlaySE( SEQ_SE_FLD_112 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゴースト　雷音　破棄
 */
//-----------------------------------------------------------------------------
static void GHOST_SPARK_SOUND_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  // ループ停止
  {
    GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
    GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
    FIELD_SOUND* p_fieldsound = GAMEDATA_GetFieldSound( p_gdata );

    FSND_StopEnvSE( p_fieldsound, SEQ_SE_FLD_111 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゴースト　雷音　更新
 */
//-----------------------------------------------------------------------------
static void GHOST_SPARK_SOUND_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  GHOST_SPARK_SOUND* p_wk = p_work;
  fx32 now_frame;

  now_frame = G3DMAPOBJST_getAnimeFrame( p_wk->p_man, p_wk->cp_spark_obj, GHOST_SPARK_SOUND_ANM_INDEX );

  // 決まったフレームで、音を鳴らす。
  if( p_wk->next_frame == now_frame )
  {
    GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
    GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
    FIELD_SOUND* p_fieldsound = GAMEDATA_GetFieldSound( p_gdata );

    p_wk->tbl_index = (p_wk->tbl_index + 1) % GHOST_SPARK_SOUND_TIMING_TBL_MAX;

    p_wk->next_frame = sc_GHOST_SPARK_SOUND_TIMING[ p_wk->tbl_index ];
    FSND_PlayEnvSE( p_fieldsound, GHOST_SND_SE_SPARK );
  }

  /*
#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT  )
  {
    sc_GHOST_SPARK_SOUND_TIMING[0] -= FX32_ONE;
    TOMOYA_Printf( "frame %d\n", FX_Whole(sc_GHOST_SPARK_SOUND_TIMING[0]) );
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT  )
  {
    sc_GHOST_SPARK_SOUND_TIMING[0] += FX32_ONE;
    TOMOYA_Printf( "frame %d\n", FX_Whole(sc_GHOST_SPARK_SOUND_TIMING[0]) );
  }
#endif
//*/
}




//-----------------------------------------------------------------------------
/**
 *    えすぱー部屋
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	ESPERT SOUND
//=====================================
typedef struct 
{
  FIELD_BMODEL_MAN* p_man;
  const G3DMAPOBJST* cp_obj;
  u16 se_2_on;
  s16 se_2_time;

  FIELD_SOUND* p_fieldsound;
} ESPERT_SOUND;

// エスパー部屋星の位置
static const VecFx32 sc_ESPERT_STAR_SARCH_POS = 
{
  FX32_CONST(118), 0, FX32_CONST(236)
};

#define ESPERT_SE_START_TIMING_MAX  (2)
//static sc_ESPERT_SE_START_TIMING[ ESPERT_SE_START_TIMING_MAX ] = 
static const sc_ESPERT_SE_START_TIMING[ ESPERT_SE_START_TIMING_MAX ] = 
{
  FX32_CONST(90), FX32_CONST(213),
};
#define ESPERT_BMODEL_ANIME_IDX  ( 0 )

#define ESPERT_SE_2_TIMING  ( 31 )
//static ESPERT_SE_2_TIMING = 31;


static void ESPERT_SOUND_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void ESPERT_SOUND_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void ESPERT_SOUND_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );


const FLDMAPFUNC_DATA c_FLDMAPFUNC_ESPERT_SOUND = 
{
  128,
  sizeof(ESPERT_SOUND),
  ESPERT_SOUND_Create,
  ESPERT_SOUND_Delete,
  ESPERT_SOUND_Update,
  NULL,
};



//----------------------------------------------------------------------------
/**
 *	@brief  えすぱーSEシステム  生成
 */
//-----------------------------------------------------------------------------
static void ESPERT_SOUND_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  ESPERT_SOUND* p_wk = p_work;
  FLDMAPPER* p_mapper = FIELDMAP_GetFieldG3Dmapper( p_fieldmap );
  FIELD_BMODEL_MAN* p_model_man = FLDMAPPER_GetBuildModelManager( p_mapper );
  fx32 start_frame;
  int  i;

  // 動作配置オブジェ
  p_wk->cp_obj  = FIELD_BMODEL_MAN_SearchObjStatusPos( p_model_man, BM_SEARCH_ID_NULL, &sc_ESPERT_STAR_SARCH_POS );
  p_wk->p_man         = p_model_man;

  {
    GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
    GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
    p_wk->p_fieldsound = GAMEDATA_GetFieldSound( p_gdata );
  }
  GF_ASSERT(p_wk->cp_obj);
  GF_ASSERT(p_wk->p_man);
  
}


//----------------------------------------------------------------------------
/**
 *	@brief  えすぱーSEシステム  破棄
 */
//-----------------------------------------------------------------------------
static void ESPERT_SOUND_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  ESPERT_SOUND* p_wk = p_work;
}

//----------------------------------------------------------------------------
/**
 *	@brief  えすぱーSEシステム  更新
 */
//-----------------------------------------------------------------------------
static void ESPERT_SOUND_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  ESPERT_SOUND* p_wk = p_work;
  fx32 frame;
  int i;

  frame = G3DMAPOBJST_getAnimeFrame( p_wk->p_man, p_wk->cp_obj, ESPERT_BMODEL_ANIME_IDX );

  for( i=0; i<ESPERT_SE_START_TIMING_MAX; i++ )
  {
    if(frame == sc_ESPERT_SE_START_TIMING[i])
    {
      FSND_PlayEnvSE( p_wk->p_fieldsound, SEQ_SE_FLD_108 );
      p_wk->se_2_on = TRUE;
      p_wk->se_2_time = ESPERT_SE_2_TIMING;
      break;
    }
  }

  // 水滴の音
  if(  p_wk->se_2_on  ){
    p_wk->se_2_time --;
    if( p_wk->se_2_time <= 0 ){
      FSND_PlayEnvSE( p_wk->p_fieldsound, SEQ_SE_FLD_130 );
      p_wk->se_2_on = FALSE;
      p_wk->se_2_time = 0;
    }
  }

#if 0
  if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A )
  {
    sc_ESPERT_SE_START_TIMING[0] += FX32_HALF;
    TOMOYA_Printf( "start timing 0 0x%x\n", sc_ESPERT_SE_START_TIMING[0] );
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_B )
  {
    sc_ESPERT_SE_START_TIMING[0] -= FX32_HALF;
    TOMOYA_Printf( "start timing 0 0x%x\n", sc_ESPERT_SE_START_TIMING[0] );
  }

  if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_X )
  {
    sc_ESPERT_SE_START_TIMING[1] += FX32_HALF;
    TOMOYA_Printf( "start timing 1 0x%x\n", sc_ESPERT_SE_START_TIMING[1] );
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_Y )
  {
    sc_ESPERT_SE_START_TIMING[1] -= FX32_HALF;
    TOMOYA_Printf( "start timing 1 0x%x\n", sc_ESPERT_SE_START_TIMING[1] );
  }

  if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_START )
  {
    ESPERT_SE_2_TIMING --;
    TOMOYA_Printf( "se timing %d\n", ESPERT_SE_2_TIMING );
  }else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_SELECT )
  {
    ESPERT_SE_2_TIMING ++;
    TOMOYA_Printf( "se timing %d\n", ESPERT_SE_2_TIMING );
  }
#endif
}

//-----------------------------------------------------------------------------
/**
 *    悪部屋
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	BAD SOUND
//=====================================
typedef struct 
{
  u16*  p_localwk_1;
  u16   se_stop;
  EVENTWORK* p_evwork;
  FIELD_SOUND* p_fieldsound;
} BAD_SOUND;


static void BAD_SOUND_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void BAD_SOUND_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void BAD_SOUND_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );


const FLDMAPFUNC_DATA c_FLDMAPFUNC_BAD_SOUND = 
{
  128,
  sizeof(BAD_SOUND),
  BAD_SOUND_Create,
  BAD_SOUND_Delete,
  BAD_SOUND_Update,
  NULL,
};



//----------------------------------------------------------------------------
/**
 *	@brief  悪SEシステム  生成
 */
//-----------------------------------------------------------------------------
static void BAD_SOUND_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
  BAD_SOUND* p_wk = p_work;

  p_wk->p_evwork      = GAMEDATA_GetEventWork( p_gdata );
  p_wk->p_fieldsound  = GAMEDATA_GetFieldSound( p_gdata );
  p_wk->p_localwk_1   = EVENTWORK_GetEventWorkAdrs( p_wk->p_evwork, LOCALWORK1 );
  
  // ループ再生開始
  if( *p_wk->p_localwk_1 == FALSE ){
    FSND_PlayEnvSE( p_wk->p_fieldsound, SEQ_SE_FLD_116 );
    p_wk->se_stop = FALSE;
  }else{
    p_wk->se_stop = TRUE;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  悪SEシステム  破棄
 */
//-----------------------------------------------------------------------------
static void BAD_SOUND_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  BAD_SOUND* p_wk = p_work;

  if( p_wk->se_stop == FALSE ){
    FSND_StopEnvSE( p_wk->p_fieldsound, SEQ_SE_FLD_116 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  悪SEシステム  アップデート
 */
//-----------------------------------------------------------------------------
static void BAD_SOUND_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  BAD_SOUND* p_wk = p_work;

  if( *p_wk->p_localwk_1 ){
    if( p_wk->se_stop == FALSE ){
      FSND_StopEnvSE( p_wk->p_fieldsound, SEQ_SE_FLD_116 );
      p_wk->se_stop = TRUE;
    }
  }
}


//-----------------------------------------------------------------------------
/**
 *    格闘部屋
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	FIGHT SOUND
//=====================================
typedef struct 
{
  u16*  p_localwk_1;
  u16   se_stop;
  EVENTWORK* p_evwork;
  FIELD_SOUND* p_fieldsound;
} FIGHT_SOUND;


static void FIGHT_SOUND_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void FIGHT_SOUND_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void FIGHT_SOUND_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );


const FLDMAPFUNC_DATA c_FLDMAPFUNC_FIGHT_SOUND = 
{
  128,
  sizeof(FIGHT_SOUND),
  FIGHT_SOUND_Create,
  FIGHT_SOUND_Delete,
  FIGHT_SOUND_Update,
  NULL,
};



//----------------------------------------------------------------------------
/**
 *	@brief  格闘SEシステム  生成
 */
//-----------------------------------------------------------------------------
static void FIGHT_SOUND_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  FIGHT_SOUND* p_wk = p_work;
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys ); 
  
  p_wk->p_evwork      = GAMEDATA_GetEventWork( p_gdata );
  p_wk->p_fieldsound  = GAMEDATA_GetFieldSound( p_gdata );
  p_wk->p_localwk_1   = EVENTWORK_GetEventWorkAdrs( p_wk->p_evwork, LOCALWORK1 );

  if( *p_wk->p_localwk_1 == FALSE ){
    FSND_PlayEnvSE( p_wk->p_fieldsound, SEQ_SE_FLD_174 );
    //TOMOYA_Printf( "EnvSe Start\n" );
    p_wk->se_stop = FALSE;
  }else{
    p_wk->se_stop = TRUE;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  格闘SEシステム  破棄
 */
//-----------------------------------------------------------------------------
static void FIGHT_SOUND_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  FIGHT_SOUND* p_wk = p_work;

  if( p_wk->se_stop == FALSE ){
    FSND_StopEnvSE( p_wk->p_fieldsound, SEQ_SE_FLD_174 );
    //TOMOYA_Printf( "EnvSe Stop del\n" );
  }
}




//----------------------------------------------------------------------------
/**
 *	@brief  格闘SEシステム  更新
 */
//-----------------------------------------------------------------------------
static void FIGHT_SOUND_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  FIGHT_SOUND* p_wk = p_work;

  if( *p_wk->p_localwk_1 ){
    if( p_wk->se_stop == FALSE ){
      FSND_StopEnvSE( p_wk->p_fieldsound, SEQ_SE_FLD_174 );
      //TOMOYA_Printf( "EnvSe Stop\n" );
      p_wk->se_stop = TRUE;
    }
  }
}

