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


  PLAYER_MOVE_VALUE last_move;
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static u16 mapCtrlHybrid_getKeyDir( const FIELDMAP_CTRL_HYBRID* cp_wk, u32 key_cont );

static void mapCtrlHybrid_Create(
	FIELDMAP_WORK* p_fieldmap, VecFx32* p_pos, u16 dir );
static void mapCtrlHybrid_Delete( FIELDMAP_WORK* p_fieldmap );
static void mapCtrlHybrid_Main( FIELDMAP_WORK* p_fieldmap, VecFx32* p_pos );
static const VecFx32 * mapCtrlHybrid_GetCameraTarget( FIELDMAP_WORK* p_fieldmap );

static void mapCtrlHybrid_Main_Grid( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk );
static void mapCtrlHybrid_Main_Rail( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk );

static void mapCtrlHybrid_ChangeGridToRail( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, u16 dir, const RAIL_LOCATION* cp_location );
static void mapCtrlHybrid_ChangeRailToGrid( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, u16 dir, const VecFx32* cp_pos );

static BOOL mapCtrlHybrid_CalcChangePos( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, VecFx32* p_pos, u16 dir );
static BOOL mapCtrlHybrid_CalcChangeRailLocation( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, RAIL_LOCATION* p_location, u16 dir );


// ベースシステムの変更
static void mapCtrlHybrid_ChangeBaseSystem( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, FLDMAP_BASESYS_TYPE type, const void* cp_pos, u16 dir );


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


//----------------------------------------------------------------------------
/**
 *	@brief  今の状態でベースシステムを変更
 *
 *	@param	p_wk          ワーク
 *	@param	p_fieldmap    フィールドワーク
 */
//-----------------------------------------------------------------------------
void FIELDMAP_CTRL_HYBRID_ChangeBaseSystem( FIELDMAP_CTRL_HYBRID* p_wk, FIELDMAP_WORK* p_fieldmap )
{
  MMDL * mmdl;
  u16 dir;
  
  mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );
  dir = MMDL_GetDirDisp( mmdl );
  
  if( p_wk->base_type == FLDMAP_BASESYS_GRID )
  {
    BOOL result;
    RAIL_LOCATION location;
    result = mapCtrlHybrid_CalcChangeRailLocation( p_fieldmap, p_wk, &location, dir );
    
    if( result )
    {
      // 動作チェンジ
      FIELD_PLAYER_GRID_Move( p_wk->p_player_grid, 0, 0 );
      mapCtrlHybrid_ChangeGridToRail( p_fieldmap, p_wk, dir, &location );
    }
  }
  else
  {
    VecFx32 pos;
    BOOL result;

    result = mapCtrlHybrid_CalcChangePos( p_fieldmap, p_wk, &pos, dir );
    
    if( result )
    {
      // 動作チェンジ
      FIELD_PLAYER_NOGRID_Move( p_wk->p_player_nogrid, 0, 0 );
      mapCtrlHybrid_ChangeRailToGrid( p_fieldmap, p_wk, dir, &pos );
    }
  }

  p_wk->last_move = PLAYER_MOVE_VALUE_STOP;
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

    // モデルの向きをDIRにする
    FIELD_PLAYER_SetDir( p_wk->p_player, dir );


    // base_typeに移動システムを遷移
    if( base_type == FLDMAP_BASESYS_GRID )
    {
      mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, base_type, p_pos, dir );
    }
    else
    {
      const RAIL_LOCATION* cp_location = PLAYERWORK_getRailPosition( cp_playerwk );
      mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, base_type, cp_location, dir );
    }
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
  MAPATTR attr;
  MAPATTR front_attr;
  MAPATTR_VALUE value;
  MMDL * mmdl;
  u16 dir;
  u16 set_dir;
  
  mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );

  if(p_wk->last_move != PLAYER_MOVE_VALUE_WALK)
  {
    dir = mapCtrlHybrid_getKeyDir( p_wk, key_cont );
    set_dir = dir;
  }
  else
  {
    dir = MMDL_GetDirMove( mmdl );
    set_dir = MMDL_GetDirDisp( mmdl );
  }



  // 移動完了しているか？
  // 1つ前が動いたか、今から動こうとして、乗り換えの上にいたら乗り換え
  if( (MMDL_CheckPossibleAcmd(mmdl) == TRUE) && (dir != DIR_NOT) )
  {

    // 足元がHYBRID、1つ前が移動不可能？

    // アトリビュートの取得
    attr = FIELD_PLAYER_GetMapAttr( p_wk->p_player );
    front_attr = FIELD_PLAYER_GetDirMapAttr( p_wk->p_player, dir );
    value = MAPATTR_GetAttrValue( attr );

    if( RAIL_ATTR_VALUE_CheckHybridBaseSystemChange( value ) )
    {
      if( MAPATTR_GetHitchFlag( front_attr ) )
      {
        BOOL result;
        RAIL_LOCATION location;
        result = mapCtrlHybrid_CalcChangeRailLocation( p_fieldmap, p_wk, &location, dir );
        
        if( result )
        {
          // 動作チェンジ
          FIELD_PLAYER_GRID_Move( p_wk->p_player_grid, 0, 0 );
          
          mapCtrlHybrid_ChangeGridToRail( p_fieldmap, p_wk, set_dir, &location );
          p_wk->last_move = PLAYER_MOVE_VALUE_STOP;
          return ;
        }
      }
    }
  }

  // 1つ前の状態を取得
  p_wk->last_move = FIELD_PLAYER_GetMoveValue( p_wk->p_player );

	FIELD_PLAYER_GRID_Move( p_wk->p_player_grid, key_trg, key_cont );
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
  MAPATTR attr;
  MAPATTR front_attr;
  MAPATTR_VALUE value;
  MMDL * mmdl;
  u16 dir;
  u16 set_dir;
  
  mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );

  if(p_wk->last_move != PLAYER_MOVE_VALUE_WALK)
  {
    dir = mapCtrlHybrid_getKeyDir( p_wk, key_cont );
    set_dir = dir;
  }
  else
  {
    dir = MMDL_GetDirMove( mmdl );
    set_dir = MMDL_GetDirDisp( mmdl );
  }



  // 移動完了しているか？
  // 1つ前が動いたか、今から動こうとして、乗り換えの上にいたら乗り換え
  if( (MMDL_CheckPossibleAcmd(mmdl) == TRUE) && (dir != DIR_NOT) )
  {
    
    // 足元がHYBRID、1つ前が移動不可能？
    // アトリビュートの取得
    attr = FIELD_PLAYER_GetMapAttr( p_wk->p_player );
    front_attr = FIELD_PLAYER_GetDirMapAttr( p_wk->p_player, dir );
    value = MAPATTR_GetAttrValue( attr );

    if( RAIL_ATTR_VALUE_CheckHybridBaseSystemChange( value ) )
    {
      if( MAPATTR_GetHitchFlag( front_attr ) )
      {
        VecFx32 pos;
        BOOL result;

        result = mapCtrlHybrid_CalcChangePos( p_fieldmap, p_wk, &pos, dir );

        if( result )
        {
          // 動作チェンジ
          FIELD_PLAYER_NOGRID_Move( p_wk->p_player_nogrid, 0, 0 );
          mapCtrlHybrid_ChangeRailToGrid( p_fieldmap, p_wk, set_dir, &pos );
          p_wk->last_move = PLAYER_MOVE_VALUE_STOP;
          return ;
        }
      }
    }
  }

  // 1つ前の状態を取得
  p_wk->last_move = FIELD_PLAYER_GetMoveValue( p_wk->p_player );


  FIELD_PLAYER_NOGRID_Move( p_wk->p_player_nogrid, key_trg, key_cont );
}

//----------------------------------------------------------------------------
/**
 *	@brief  グリッドマップからレールマップに移動
 *
 *	@param	p_fieldmap
 *	@param	p_wk 
 *	@param  dir 
 */
//-----------------------------------------------------------------------------
static void mapCtrlHybrid_ChangeGridToRail( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, u16 dir, const RAIL_LOCATION* cp_location )
{
  // レールに変更
  mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, FLDMAP_BASESYS_RAIL, cp_location, dir );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールマップからグリッドマップに移動
 *
 *	@param	p_fieldmap
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void mapCtrlHybrid_ChangeRailToGrid( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, u16 dir, const VecFx32* cp_pos )
{
  // グリッドに変更
  mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, FLDMAP_BASESYS_GRID, cp_pos, dir );
}


//----------------------------------------------------------------------------
/**
 *	@brief  乗り換え先の３D座標の取得
 *
 *	@param	p_fieldmap
 *	@param	p_wk
 *	@param	p_pos 
 *
 *	@retval TRUE  乗り換えOK 
 *	@retval FALSE 乗り換えNG
 */
//-----------------------------------------------------------------------------
static BOOL mapCtrlHybrid_CalcChangePos( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, VecFx32* p_pos, u16 dir )
{
  VecFx32 front_pos;
  MAPATTR attr;
  FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( p_fieldmap );
  
  FIELD_PLAYER_GetPos( p_wk->p_player, p_pos );
  front_pos = *p_pos;

  // 1歩前のアトリビュート取得
  MMDL_TOOL_AddDirVector( dir, &front_pos, GRID_FX32 ); 
  attr = MAPATTR_GetAttribute( mapper, &front_pos );

  if( MAPATTR_GetHitchFlag( attr ) == FALSE )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  乗り換え先のロケーションの取得処理
 *
 *	@param	p_fieldmap      
 *	@param	p_wk
 *	@param	p_location 
 *
 *	@retval TRUE  乗り換えOK 
 *	@retval FALSE 乗り換えNG
 */
//-----------------------------------------------------------------------------
static BOOL mapCtrlHybrid_CalcChangeRailLocation( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, RAIL_LOCATION* p_location, u16 dir )
{
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( p_fieldmap );
  const FIELD_RAIL_MAN* cp_railman = FLDNOGRID_MAPPER_GetRailMan( p_mapper );
  RAIL_LOCATION location;
  RAIL_LOCATION front_location;
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

  // 1歩前に進めるかチェック
  if( result )
  {
    result = FIELD_RAIL_MAN_CalcRailKeyLocation( cp_railman, &location, FIELD_RAIL_TOOL_ConvertDirToRailKey( dir ), &front_location );
  }

  *p_location = location;
  
  return result;
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
static void mapCtrlHybrid_ChangeBaseSystem( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, FLDMAP_BASESYS_TYPE type, const void* cp_pos, u16 dir )
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
    VecFx32 pos = *((VecFx32*)cp_pos);
    
    // 動作変更
    FIELD_PLAYER_NOGRID_Stop( p_wk->p_player_nogrid );
    MMDL_ChangeMoveParam( p_mmdl, &data_MMdlHeader );

    //  座標を設定
    {
      MMDL_InitGridPosition( p_mmdl, SIZE_GRID_FX32( pos.x ), SIZE_GRID_FX32( pos.y ), SIZE_GRID_FX32( pos.z ), dir );
      
      MMDL_GetVectorPos( p_mmdl, &pos );
    }
    FIELD_PLAYER_SetPos( p_wk->p_player, &pos );

    // カメラ設定
    FLDNOGRID_MAPPER_UnBindCameraWork( p_mapper );
    FIELD_CAMERA_ChangeMode( p_camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );
    FIELD_CAMERA_BindDefaultTarget( p_camera );

    // カメラ状態をデフォルト値に戻す
    FIELD_CAMERA_SetDefaultParameter( p_camera );
  }
  else
  {
    FIELD_PLAYER_NOGRID_Restart( p_wk->p_player_nogrid, cp_pos );

    // カメラ設定
    FLDNOGRID_MAPPER_BindCameraWork( p_mapper, FIELD_PLAYER_NOGRID_GetRailWork( p_wk->p_player_nogrid ) );

  }

  // 方向を合わせる
  MMDL_SetDirDisp( p_mmdl, dir );
  MMDL_SetDirMove( p_mmdl, dir );

  p_wk->base_type = type;
}


//----------------------------------------------------------------------------
/**
 *	@brief  キー入力による方向を取得
 *
 *	@param	cp_wk       ワーク
 *	@param	key_cont    キー
 *
 *	@return 方向
 */
//-----------------------------------------------------------------------------
static u16 mapCtrlHybrid_getKeyDir( const FIELDMAP_CTRL_HYBRID* cp_wk, u32 key_cont )
{
  u16 dir = DIR_NOT;

	if( (key_cont&PAD_KEY_UP) ){
		dir = DIR_UP;
	}else if( (key_cont&PAD_KEY_DOWN) ){
		dir = DIR_DOWN;
	}else if( (key_cont&PAD_KEY_LEFT) ){
		dir = DIR_LEFT;
	}else if( (key_cont&PAD_KEY_RIGHT) ){
		dir = DIR_RIGHT;
	}
	
  return dir;
}

