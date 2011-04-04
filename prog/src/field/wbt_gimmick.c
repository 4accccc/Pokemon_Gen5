//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wbt_gimmick.c
 *	@brief  WBT会場演出ギミック
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.04.04
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
// system
#include "fieldmap.h"
#include "system/ica_anime.h"
#include "system/main.h"    //for HEAPID_FIELDMAP

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/arc_def.h"
#include "arc/fieldmap/wbt_gmk.naix"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "wbt_gimmick.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define EXPOBJ_UNIT_IDX     (0)
#define WBT_TMP_ASSIGN_ID   (1)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ギミック一時ワーク
//=====================================
typedef struct {

  FLD_EXP_OBJ_CNT_PTR exp_obj;  // 拡張3Dモデルコントローラ

  // 発動フラグ
  BOOL      trg_vision;     // 背景ビジョン
  BOOL      trg_effect;     // エフェクト

} WBT_GMK_TMP;


//=============================================================================
/**
 *          static関数宣言
 */
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( WBT_GMK_TMP *p_wk );
static void EXPOBJ_ObjDell( WBT_GMK_TMP *p_wk );
static void EXPOBJ_ObjMain( WBT_GMK_TMP *p_wk );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  初期化関数
 *
 *	@param	p_fieldmap  フィールドワークポインタ
 */
//-----------------------------------------------------------------------------
void WBT_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap )
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, WBT_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(WBT_GMK_TMP) );

  // ギミックは発動させない
  p_wk->trg_vision = FALSE;
  p_wk->trg_effect = FALSE;

  // EXPOBJ初期化
  EXPOBJ_ObjSet( p_wk );

  ICHI_Printf("gimmick init\n");
}

void WBT_GIMMICK_End( FIELDMAP_WORK *p_fieldmap )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, WBT_TMP_ASSIGN_ID);

  ICHI_Printf("gimmick end\n");
}

void WBT_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap )
{
  WBT_GMK_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, WBT_TMP_ASSIGN_ID );

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//=============================================================================
/**
 *        static関数
 */
//=============================================================================
//
//=============================================================================
/**
 *        EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( WBT_GMK_TMP *p_wk )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( WBT_GMK_TMP *p_wk )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	WBT_GMK_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( WBT_GMK_TMP *p_wk )
{
}

