//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_seatemple.c
 *	@brief  海底神殿スクリプトコマンド
 *	@author	tomoya takahashi
 *	@date		2010.02.02
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"

#include "fieldmap_func.h"

#include "scrcmd_seatemple.h"

#include "arc/fieldmap/field_sea_temple.naix"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
enum
{
  MODEL_SHADOW_TYPE_1,
  MODEL_SHADOW_TYPE_2,
  MODEL_SHADOW_TYPE_NONE,

  MODEL_SHADOW_TYPE_MAX,
};
//-------------------------------------
///	オブジェリソース管理
//=====================================
enum {
  // リソース数
  MODEL_RES_OBON_A = 0,
  MODEL_RES_OBON_B,
  MODEL_RES_OBON_C,
  MODEL_RES_OBON_ITA_A,
  MODEL_RES_OBON_ITA_B,
  MODEL_RES_OBON_ITA_C,
  MODEL_RES_MAX,

  // モデル数
  MODEL_MODEL_OBON_A = 0,
  MODEL_MODEL_OBON_B,
  MODEL_MODEL_OBON_C,
  MODEL_MODEL_MAX,

  // アニメ数
  MODEL_ANIME_OBON_A = 0,
  MODEL_ANIME_OBON_B,
  MODEL_ANIME_OBON_C,
  MODEL_ANIME_MAX,
} ;

static const u32 sc_MODEL_RES_TBL[MODEL_RES_MAX] = 
{
  NARC_field_sea_temple_sea_obon_a_nsbmd,
  NARC_field_sea_temple_sea_obon_b_nsbmd,
  NARC_field_sea_temple_sea_obon_c_nsbmd,
  NARC_field_sea_temple_sea_obon_a_nsbta,
  NARC_field_sea_temple_sea_obon_b_nsbta,
  NARC_field_sea_temple_sea_obon_c_nsbta,
};

static const u32 sc_MODEL_MODEL_RES[MODEL_MODEL_MAX] = 
{
  MODEL_RES_OBON_A,
  MODEL_RES_OBON_B,
  MODEL_RES_OBON_C,
};


static const u32 sc_MODEL_ANM_RES[MODEL_ANIME_MAX] = 
{
  MODEL_RES_OBON_ITA_A,
  MODEL_RES_OBON_ITA_B,
  MODEL_RES_OBON_ITA_C,
};

static const u32 sc_MODEL_ANM_RND[MODEL_ANIME_MAX] = 
{
  MODEL_MODEL_OBON_A,
  MODEL_MODEL_OBON_B,
  MODEL_MODEL_OBON_C,
};


static const GFL_G3D_OBJSTATUS sc_DRAWPARAM = 
{
  {FX32_CONST(256),0,FX32_CONST(256)},
  {FX32_ONE,FX32_ONE,FX32_ONE},
  {
    FX32_ONE, 0, 0,
    0, FX32_ONE, 0,
    0, 0, FX32_ONE,
  },
};


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	海底神殿　表示物管理処理
//=====================================
typedef struct {

  // リソース郡
  GFL_G3D_RES* p_res[MODEL_RES_MAX];
  GFL_G3D_RND* p_rnd[MODEL_MODEL_MAX];
  GFL_G3D_ANM* p_anm[MODEL_ANIME_MAX];

  // 表示オブジェ
  GFL_G3D_OBJ* p_obj[MODEL_MODEL_MAX];

#ifdef PM_DEBUG
  u8 DEBUG_print_haichi;
#endif

} SEATEMPLE_WK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 
static void SEATEMPLE_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void SEATEMPLE_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void SEATEMPLE_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );
static void SEATEMPLE_3DWrite(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work );


static const FLDMAPFUNC_DATA sc_FLDMAPFUNC_SEATEMPLE = 
{
  128,
  sizeof(SEATEMPLE_WK),
  SEATEMPLE_Create,
  SEATEMPLE_Delete,
  SEATEMPLE_Update,
  SEATEMPLE_3DWrite,
};

//----------------------------------------------------------------------------
/**
 *	@brief  海底神殿　表示物管理処理　開始
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdSeaTempleStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FLDMAPFUNC_SYS* p_funcsys = FIELDMAP_GetFldmapFuncSys( p_fieldmap );

  FLDMAPFUNC_Create(p_funcsys, &sc_FLDMAPFUNC_SEATEMPLE);

  return VMCMD_RESULT_CONTINUE;
}




//----------------------------------------------------------------------------
/**
 *	@brief  海底神殿  光の照り返し　生成
 */
//-----------------------------------------------------------------------------
static void SEATEMPLE_Create(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  SEATEMPLE_WK* p_wk = p_work;
  HEAPID heapID = FIELDMAP_GetHeapID( p_fieldmap );
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_FLD_SEA_TEMPLE, GFL_HEAP_LOWID(heapID) );
  int i;
  int idx, res_idx;
  BOOL result;
  
  // 
  for( i=0; i<MODEL_RES_MAX; i++ )
  {
    p_wk->p_res[i] = GFL_G3D_CreateResourceHandle( p_handle, sc_MODEL_RES_TBL[i] );
    // テクスチャアドレスの確保
    if( GFL_G3D_CheckResourceType( p_wk->p_res[i], GFL_G3D_RES_CHKTYPE_TEX ) )
    {
      result = GFL_G3D_TransVramTexture( p_wk->p_res[i] );
      GF_ASSERT( result );
    }
  }

  // モデル生成
  for( i=0; i<MODEL_MODEL_MAX; i++ )
  {
    idx = sc_MODEL_MODEL_RES[i];

    if( GFL_G3D_CheckResourceType( p_wk->p_res[ idx ], GFL_G3D_RES_CHKTYPE_TEX ) )
    {
      p_wk->p_rnd[i] = GFL_G3D_RENDER_Create( p_wk->p_res[ idx ], 0, p_wk->p_res[ idx ] );
    }
    else
    {
      p_wk->p_rnd[i] = GFL_G3D_RENDER_Create( p_wk->p_res[ idx ], 0, NULL );
    }

  }

  // アニメ生成
  for( i=0; i<MODEL_ANIME_MAX; i++ )
  {
    idx = sc_MODEL_ANM_RND[i];
    res_idx = sc_MODEL_ANM_RES[i];
    p_wk->p_anm[i] = GFL_G3D_ANIME_Create( p_wk->p_rnd[idx], p_wk->p_res[ res_idx ], 0 );
  }

  // 描画オブジェクト生成
  for( i=0; i<MODEL_MODEL_MAX; i++ )
  {
    p_wk->p_obj[i] = GFL_G3D_OBJECT_Create( p_wk->p_rnd[i], p_wk->p_anm, MODEL_ANIME_MAX );
  }
  

  // ライトモデルのほうだけ、アニメーションON
  GFL_G3D_OBJECT_EnableAnime( p_wk->p_obj[ MODEL_MODEL_OBON_A ], MODEL_ANIME_OBON_A );
  GFL_G3D_OBJECT_EnableAnime( p_wk->p_obj[ MODEL_MODEL_OBON_B ], MODEL_ANIME_OBON_B );
  GFL_G3D_OBJECT_EnableAnime( p_wk->p_obj[ MODEL_MODEL_OBON_C ], MODEL_ANIME_OBON_C );

  // マニュアルソートに変更
  // field_camera内でも操作しているのでタイミングに注意！
  GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_W );


  GFL_ARC_CloseDataHandle( p_handle );

#ifdef PM_DEBUG
  p_wk->DEBUG_print_haichi = TRUE;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  海底神殿  光の照り返し　破棄
 */
//-----------------------------------------------------------------------------
static void SEATEMPLE_Delete(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  int i;
  SEATEMPLE_WK* p_wk = p_work;
  BOOL result;


  // 描画オブジェクト破棄
  for( i=0; i<MODEL_MODEL_MAX; i++ )
  {
    GFL_G3D_OBJECT_Delete( p_wk->p_obj[i] );
  }

  // アニメ破棄
  for( i=0; i<MODEL_ANIME_MAX; i++ )
  {
    GFL_G3D_ANIME_Delete( p_wk->p_anm[i] );
  }



  // モデル破棄
  for( i=0; i<MODEL_MODEL_MAX; i++ )
  {
    GFL_G3D_RENDER_Delete( p_wk->p_rnd[i] );
  }

  // リソース破棄
  for( i=0; i<MODEL_RES_MAX; i++ )
  {
    // テクスチャアドレスの破棄
    if( GFL_G3D_CheckResourceType( p_wk->p_res[i], GFL_G3D_RES_CHKTYPE_TEX ) )
    {
      result = GFL_G3D_FreeVramTexture( p_wk->p_res[i] );
      GF_ASSERT( result );
    }
    GFL_G3D_DeleteResource( p_wk->p_res[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  海底神殿  光の照り返し　更新
 */
//-----------------------------------------------------------------------------
static void SEATEMPLE_Update(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  SEATEMPLE_WK* p_wk = p_work;

  // 
  GFL_G3D_OBJECT_LoopAnimeFrame( p_wk->p_obj[ MODEL_MODEL_OBON_A ], MODEL_ANIME_OBON_A, FX32_ONE );
  GFL_G3D_OBJECT_LoopAnimeFrame( p_wk->p_obj[ MODEL_MODEL_OBON_B ], MODEL_ANIME_OBON_B, FX32_ONE );
  GFL_G3D_OBJECT_LoopAnimeFrame( p_wk->p_obj[ MODEL_MODEL_OBON_C ], MODEL_ANIME_OBON_C, FX32_ONE );

#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L )
  {
    if(p_wk->DEBUG_print_haichi){
      p_wk->DEBUG_print_haichi = FALSE;
    }else{
      p_wk->DEBUG_print_haichi = TRUE;
    }
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  海底神殿  光の照り返し　描画
 */
//-----------------------------------------------------------------------------
static void SEATEMPLE_3DWrite(FLDMAPFUNC_WORK* p_funcwk, FIELDMAP_WORK* p_fieldmap, void* p_work )
{
  SEATEMPLE_WK* p_wk = p_work;

#ifdef PM_DEBUG
  if( p_wk->DEBUG_print_haichi )
#endif
  {
    GFL_G3D_DRAW_DrawObject( p_wk->p_obj[MODEL_MODEL_OBON_A], &sc_DRAWPARAM );
    GFL_G3D_DRAW_DrawObject( p_wk->p_obj[MODEL_MODEL_OBON_B], &sc_DRAWPARAM );
    GFL_G3D_DRAW_DrawObject( p_wk->p_obj[MODEL_MODEL_OBON_C], &sc_DRAWPARAM );
  }
}



