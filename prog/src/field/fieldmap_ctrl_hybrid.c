//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fieldmap_ctrl_hybrid.c
 *	@brief  マップコントロール　ハイブリッド
 *	@author	tomoya takahashi
 *	@date		2009.10.27
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "fieldmap_ctrl_hybrid.h"

#include "field_player.h"
#include "field_player_grid.h"
#include "field_player_nogrid.h"

#include "field/zonedata.h"


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	レールチェックの高さ範囲
//=====================================
#define FIELDMAP_CTRL_HYBRID_RAILPLANE_CHECK_Y  ( FX32_CONST(4) )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	FIELDMAP_CTRL_HYBRID_WORK
//=====================================
struct _FIELDMAP_CTRL_HYBRID
{
  FLDMAP_BASESYS_TYPE base_type;

  FIELD_PLAYER*         p_player;
	FIELD_PLAYER_GRID*    p_player_grid;
	FIELD_PLAYER_NOGRID*  p_player_nogrid;
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static void mapCtrlHybrid_Create(
	FIELDMAP_WORK* p_fieldmap, VecFx32* p_pos, u16 dir );
static void mapCtrlHybrid_Delete( FIELDMAP_WORK* p_fieldmap );
static void mapCtrlHybrid_Main( FIELDMAP_WORK* p_fieldmap, VecFx32* p_pos );
static const VecFx32 * mapCtrlHybrid_GetCameraTarget( FIELDMAP_WORK* p_fieldmap );

static void mapCtrlHybrid_Main_Grid( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk );
static void mapCtrlHybrid_Main_Rail( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk );


// ベースシステムの変更
static void mapCtrlHybrid_ChangeBaseSystem( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, FLDMAP_BASESYS_TYPE type, const void* cp_pos );


//======================================================================
//	フィールドマップ　ハイブリッド処理
//======================================================================
//--------------------------------------------------------------
///	マップコントロール　ハイブリッド移動
//--------------------------------------------------------------
const DEPEND_FUNCTIONS FieldMapCtrl_HybridFunctions =
{
	FLDMAP_CTRLTYPE_HYBRID,
	mapCtrlHybrid_Create,
	mapCtrlHybrid_Main,
	mapCtrlHybrid_Delete,
  mapCtrlHybrid_GetCameraTarget,
};



//----------------------------------------------------------------------------
/**
 *	@brief  フィールドグリッドプレイヤーの取得
 *
 *	@param	cp_wk   ワーク
 *
 *	@return フィールドグリッドプレイヤーワーク
 */
//-----------------------------------------------------------------------------
FIELD_PLAYER_GRID* FIELDMAP_CTRL_HYBRID_GetFieldPlayerGrid( const FIELDMAP_CTRL_HYBRID* cp_wk )
{
  GF_ASSERT( cp_wk );

  return cp_wk->p_player_grid;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドベースシステムタイプの取得
 *
 *	@param	cp_wk   ワーク
 *  
 *	@return　ベースシステムタイプ
 */
//----------------------------------------------------------------------------- 
FLDMAP_BASESYS_TYPE FIELDMAP_CTRL_HYBRID_GetBaseSystemType( const FIELDMAP_CTRL_HYBRID* cp_wk )
{
  GF_ASSERT( cp_wk );
  return  cp_wk->base_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドノーグリッドプレイヤーの取得
 *
 *	@param	cp_wk   ワーク
 *
 *	@return フィールドノーグリッドプレイヤーワーク
 */
//-----------------------------------------------------------------------------
FIELD_PLAYER_NOGRID* FIELDMAP_CTRL_HYBRID_GetFieldPlayerNoGrid( const FIELDMAP_CTRL_HYBRID* cp_wk )
{
  GF_ASSERT( cp_wk );

  return cp_wk->p_player_nogrid;
}






//-----------------------------------------------------------------------------
/**
 *        private関数郡
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  Hybridマップコントロール  生成
 */
//-----------------------------------------------------------------------------
static void mapCtrlHybrid_Create( FIELDMAP_WORK* p_fieldmap, VecFx32* p_pos, u16 dir )
{
  HEAPID heapID;
  FIELDMAP_CTRL_HYBRID* p_wk;
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );
  const PLAYER_WORK* cp_playerwk = GAMEDATA_GetMyPlayerWork( p_gdata );
  u32 base_type;

  heapID  = FIELDMAP_GetHeapID( p_fieldmap );
  
  // ワーク作成＆フィールドマップに設定
  p_wk    = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELDMAP_CTRL_HYBRID) ); 
	FIELDMAP_SetMapCtrlWork( p_fieldmap, p_wk );

  // ワーク初期化
  {
    p_wk->p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
    
    // レールプレイヤー生成
		p_wk->p_player_nogrid = FIELD_PLAYER_NOGRID_Create( p_wk->p_player, heapID );
    
    // グリッドプレイヤー生成
		p_wk->p_player_grid = FIELD_PLAYER_GRID_Init( p_wk->p_player, heapID );

    // 初期動作をどちらにするかPLAYER_WORKから取得
    base_type = PLAYERWORK_getPosType( cp_playerwk );
    p_wk->base_type = FLDMAP_BASESYS_MAX;


    // base_typeに移動システムを遷移
    if( base_type == FLDMAP_BASESYS_GRID )
    {
      mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, base_type, p_pos );
    }
    else
    {
      const RAIL_LOCATION* cp_location = PLAYERWORK_getRailPosition( cp_playerwk );
      mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, base_type, cp_location );
    }

    FIELD_PLAYER_SetDir( p_wk->p_player, dir );
  }

  //カメラ座標セット
  {
    MMDL* p_mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );
    FIELDMAP_SetNowPosTarget( p_fieldmap, MMDL_GetVectorPosAddress( p_mmdl ) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief　Hybridマップコントロール　破棄
 */
//-----------------------------------------------------------------------------
static void mapCtrlHybrid_Delete( FIELDMAP_WORK* p_fieldmap )
{
	FIELDMAP_CTRL_HYBRID* p_wk;

	p_wk = FIELDMAP_GetMapCtrlWork( p_fieldmap );
	FIELD_PLAYER_GRID_Delete( p_wk->p_player_grid );
	FIELD_PLAYER_NOGRID_Delete( p_wk->p_player_nogrid );

	GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief　Hybridマップコントロール　メイン
 */
//-----------------------------------------------------------------------------
static void mapCtrlHybrid_Main( FIELDMAP_WORK* p_fieldmap, VecFx32* p_pos )
{
	FIELDMAP_CTRL_HYBRID* p_wk = FIELDMAP_GetMapCtrlWork( p_fieldmap );
  static void (*pMain[FLDMAP_BASESYS_MAX])( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk ) = 
  {
    mapCtrlHybrid_Main_Grid,
    mapCtrlHybrid_Main_Rail,
  };

  GF_ASSERT( p_wk->base_type < FLDMAP_BASESYS_MAX );
  GF_ASSERT( pMain[ p_wk->base_type ] );

  pMain[ p_wk->base_type ]( p_fieldmap, p_wk );

  FIELD_PLAYER_GetPos( p_wk->p_player, p_pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  Hybridマップコントロール　カメラターゲット取得
 *
 *	@return カメラターゲット
 */
//-----------------------------------------------------------------------------
static const VecFx32 * mapCtrlHybrid_GetCameraTarget( FIELDMAP_WORK* p_fieldmap )
{
  FIELD_PLAYER* p_fldplayer = FIELDMAP_GetFieldPlayer( p_fieldmap );
  MMDL* p_mmdl = FIELD_PLAYER_GetMMdl( p_fldplayer );
  const VecFx32 * p_pos = MMDL_GetVectorPosAddress( p_mmdl ); 
  return( p_pos );
}








//-----------------------------------------------------------------------------
/**
 *    Private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  グリッドマップ
 */
//-----------------------------------------------------------------------------
static void mapCtrlHybrid_Main_Grid( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk )
{
  int key_trg = GFL_UI_KEY_GetTrg();
  int key_cont = GFL_UI_KEY_GetCont();

	FIELD_PLAYER_GRID_Move( p_wk->p_player_grid, key_trg, key_cont );

  // 動作できないとき、レール動作への遷移を試みる
  if( FIELD_PLAYER_GetMoveValue( p_wk->p_player ) == PLAYER_MOVE_VALUE_STOP )
  {
    FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( p_fieldmap );
    const FIELD_RAIL_MAN* cp_railman = FLDNOGRID_MAPPER_GetRailMan( p_mapper );
    RAIL_LOCATION location;
    VecFx32 hitpos;
    VecFx32 startpos, endpos;
    BOOL result;

    // 開始ポジション
    FIELD_PLAYER_GetPos( p_wk->p_player, &startpos );
    VEC_Set( &endpos, startpos.x, 
        startpos.y + FIELDMAP_CTRL_HYBRID_RAILPLANE_CHECK_Y,
        startpos.z );
    startpos.y -= FIELDMAP_CTRL_HYBRID_RAILPLANE_CHECK_Y;

    // レール上の位置を取得、設定し、試す
    result = FIELD_RAIL_MAN_Calc3DVecRailLocation( cp_railman, &startpos, &endpos, &location, &hitpos );

    if( result == TRUE )
    {
      // レールに変更
      mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, FLDMAP_BASESYS_RAIL, &location );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールマップ
 */
//-----------------------------------------------------------------------------
static void mapCtrlHybrid_Main_Rail( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk )
{
  int key_trg = GFL_UI_KEY_GetTrg();
  int key_cont = GFL_UI_KEY_GetCont();

  FIELD_PLAYER_NOGRID_Move( p_wk->p_player_nogrid, key_trg, key_cont );

  // 動作できないとき、グリッド動作への遷移を試みる
  if( FIELD_PLAYER_NOGRID_IsHitch( p_wk->p_player_nogrid ) )
  {
    VecFx32 pos;

    FIELD_PLAYER_GetPos( p_wk->p_player, &pos );
    
    // 移動可能なグリッドか？
    
    // グリッドに変更
    mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, FLDMAP_BASESYS_GRID, &pos );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ベースシステムタイプの変更処理
 *
 *	@param	p_fieldmap  フィールドマップ
 *	@param	p_wk        ハイブリッドワーク
 *	@param	type        ベースシステムタイプ
 *	@param  cp_pos       GRID:VecFx32*    RAIL:RAIL_LOCATION*
 */
//-----------------------------------------------------------------------------
static void mapCtrlHybrid_ChangeBaseSystem( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, FLDMAP_BASESYS_TYPE type, const void* cp_pos )
{
  MMDL* p_mmdl;
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( p_fieldmap );
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  if( p_wk->base_type == type )
  {
    return ;
  }
  
  p_mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );
  
  // アニメーションの停止
  MMDL_FreeAcmd( p_mmdl );
  
  if( type == FLDMAP_BASESYS_GRID )
  {
    static const MMDL_HEADER data_MMdlHeader =
    {
      MMDL_ID_PLAYER,	///<識別ID
      HERO,	///<表示するOBJコード
      MV_DMY,	///<動作コード
      0,	///<イベントタイプ
      0,	///<イベントフラグ
      0,	///<イベントID
      0,	///<指定方向
      0,	///<指定パラメタ 0
      0,	///<指定パラメタ 1
      0,	///<指定パラメタ 2
      MOVE_LIMIT_NOT,	///<X方向移動制限
      MOVE_LIMIT_NOT,	///<Z方向移動制限
      MMDL_HEADER_POSTYPE_GRID,
    };
    
    // 動作変更
    FIELD_PLAYER_NOGRID_Stop( p_wk->p_player_nogrid );
    MMDL_ChangeMoveParam( p_mmdl, &data_MMdlHeader );

    FIELD_PLAYER_SetPos( p_wk->p_player, cp_pos );

    // カメラ設定
    FLDNOGRID_MAPPER_UnBindCameraWork( p_mapper );
    FIELD_CAMERA_ChangeMode( p_camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    FIELD_CAMERA_BindDefaultTarget( p_camera );
  }
  else
  {
    FIELD_PLAYER_NOGRID_Restart( p_wk->p_player_nogrid, cp_pos );

    // カメラ設定
    FLDNOGRID_MAPPER_BindCameraWork( p_mapper, FIELD_PLAYER_NOGRID_GetRailWork( p_wk->p_player_nogrid ) );
  }

  p_wk->base_type = type;
}

