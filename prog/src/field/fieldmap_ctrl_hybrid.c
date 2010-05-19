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

#include "field/zonedata.h"
#include "field/field_const.h"


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

  BOOL  last_automove;
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

static void mapCtrlHybrid_ChangeGridToRail( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, u16 dir, BOOL dir_key, const RAIL_LOCATION* cp_location, int trg, int cont );
static void mapCtrlHybrid_ChangeRailToGrid( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, u16 dir, BOOL dir_key, const VecFx32* cp_pos, int trg, int cont );

static BOOL mapCtrlHybrid_CalcChangePos( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, VecFx32* p_pos, u16 dir );
static BOOL mapCtrlHybrid_CalcChangeRailLocation( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, RAIL_LOCATION* p_location, u16 dir );


// ベースシステムの変更
static void mapCtrlHybrid_ChangeBaseSystem( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, FLDMAP_BASESYS_TYPE type, const void* cp_pos, u16 dir, u16 animeIndex, u16 animeFrame );


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
      mapCtrlHybrid_ChangeGridToRail( p_fieldmap, p_wk, dir, FALSE, &location, 0,0 );
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
      mapCtrlHybrid_ChangeRailToGrid( p_fieldmap, p_wk, dir, FALSE, &pos, 0,0 );
    }
  }

  p_wk->last_automove = FALSE;

}



#ifdef PM_DEBUG
void FIELDMAP_CTRL_HYBRID_DEBUG_SetBaseSystem( FIELDMAP_CTRL_HYBRID* p_wk, FLDMAP_BASESYS_TYPE type )
{
  p_wk->base_type = type;
}
#endif

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
  u16 animeFrame;
  u16 animeIndex;
  MMDL* p_mmdl;

  heapID  = FIELDMAP_GetHeapID( p_fieldmap );
  
  // ワーク作成＆フィールドマップに設定
  p_wk    = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELDMAP_CTRL_HYBRID) ); 
	FIELDMAP_SetMapCtrlWork( p_fieldmap, p_wk );

  // ワーク初期化
  {
    p_wk->p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
    
    // 初期動作をどちらにするかPLAYER_WORKから取得
    base_type = PLAYERWORK_getPosType( cp_playerwk );
    p_wk->base_type = FLDMAP_BASESYS_MAX;

  
    p_mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );

    // アニメーションフレーム取得
    MMDL_DrawBlactWork_GetAnimeFrame( p_mmdl, &animeIndex, &animeFrame );


    // base_typeに移動システムを遷移
    if( base_type == FLDMAP_BASESYS_GRID )
    {
      //TOMOYA_Printf( "grid pos type\n" );

      // FLDEFF再設定などを正しく動かすために、セットアップ順番を気にする必要がある。
      // グリッドプレイヤー生成
      FIELD_PLAYER_SetUpGrid( p_wk->p_player, heapID );

      // レールプレイヤー生成
      FIELD_PLAYER_SetUpNoGrid( p_wk->p_player, heapID );

      // モデルの向きをDIRにする
      FIELD_PLAYER_SetDir( p_wk->p_player, dir );
      
      mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, base_type, p_pos, dir, animeIndex, animeFrame );
    }
    else
    {
      const RAIL_LOCATION* cp_location = PLAYERWORK_getRailPosition( cp_playerwk );
      //TOMOYA_Printf( "rail pos type\n" );

      // FLDEFF再設定などを正しく動かすために、セットアップ順番を気にする必要がある。
      // レールプレイヤー生成
      FIELD_PLAYER_SetUpNoGrid( p_wk->p_player, heapID );
      
      // グリッドプレイヤー生成
      FIELD_PLAYER_SetUpGrid( p_wk->p_player, heapID );

      // モデルの向きをDIRにする
      FIELD_PLAYER_SetDir( p_wk->p_player, dir );

      mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, base_type, cp_location, dir, animeIndex, animeFrame );
    }
  }

  //カメラ座標セット
  {
    FIELDMAP_SetNowPosTarget( p_fieldmap, MMDL_GetVectorPosAddress( p_mmdl ) );
  }

  FIELD_PLAYER_GetPos( p_wk->p_player, p_pos );
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
  PLAYER_MOVEBIT mbit = PLAYER_MOVEBIT_NON;
  MAPATTR attr;
  MAPATTR front_attr;
  MAPATTR_VALUE value;
  MMDL * mmdl;
  u16 dir;
  u16 set_dir;
  BOOL key_dir;
  
  mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );

  dir = FIELD_PLAYER_GetKeyDir( p_wk->p_player, key_cont );
  key_dir = TRUE;

  
  // 移動完了しているか？
  // 1つ前が動いたか、今から動こうとして、乗り換えの上にいたら乗り換え
  if( ((MMDL_CheckPossibleAcmd(mmdl) == TRUE) || (FIELD_PLAYER_IsHitch( p_wk->p_player ) == TRUE)) && 
      (dir != DIR_NOT) )
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
          // 描画方向を求める
          set_dir = dir;
          
          mapCtrlHybrid_ChangeGridToRail( p_fieldmap, p_wk, set_dir, key_dir, &location, key_trg, key_cont );
          return ;
        }
      }
    }
  }

  if( FIELD_PLAYER_CheckPossibleDash(p_wk->p_player) == TRUE ){
    mbit |= PLAYER_MOVEBIT_DASH;
  }
  
	FIELD_PLAYER_MoveGrid( p_wk->p_player, key_trg, key_cont, mbit );
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
  u16 dir = DIR_NOT;
  u16 set_dir;
  BOOL key_dir;
  
  mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );

  if( p_wk->last_automove == FALSE ){
    dir = FIELD_PLAYER_GetKeyDir( p_wk->p_player, key_cont );
    key_dir = TRUE;
  }else{
    dir = MMDL_GetDirMove( mmdl );
    key_dir = FALSE;
  }

  // 移動完了しているか？
  // 1つ前が動いたか、今から動こうとして、乗り換えの上にいたら乗り換え
  if( ((MMDL_CheckPossibleAcmd(mmdl) == TRUE) || (FIELD_PLAYER_IsHitch( p_wk->p_player ) == TRUE)) && 
      (dir != DIR_NOT) )
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
          set_dir = dir;
          
          mapCtrlHybrid_ChangeRailToGrid( p_fieldmap, p_wk, set_dir, key_dir, &pos, key_trg, key_cont );
          p_wk->last_automove = FALSE;
          return ;
        }
      }
    }
  }

  p_wk->last_automove = FIELD_PLAYER_IsAutoMoveNoGrid( p_wk->p_player );

  FIELD_PLAYER_MoveNoGrid( p_wk->p_player, key_trg, key_cont );
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
static void mapCtrlHybrid_ChangeGridToRail( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, u16 dir, BOOL dir_key, const RAIL_LOCATION* cp_location, int trg, int cont )
{
  u16 animeFrame;
  u16 animeIndex;
  PLAYER_MOVEBIT dmy = PLAYER_MOVEBIT_NON;
  MMDL* p_mmdl;

  p_mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );

  // アニメーションフレーム取得
  MMDL_DrawBlactWork_GetAnimeFrame( p_mmdl, &animeIndex, &animeFrame );

  //GridWork内部初期化
  FIELD_PLAYER_MoveGrid( p_wk->p_player, 0, 0, dmy );

  if( dir_key == FALSE ){
    dir = MMDL_GetDirDisp( p_mmdl );
  }

  // レールに変更
  mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, FLDMAP_BASESYS_RAIL, cp_location, dir, animeIndex, animeFrame );
  

  //NogridWork内部初期化
  FIELD_PLAYER_MoveNoGrid( p_wk->p_player, trg, cont );

  TOMOYA_Printf( "change grid to rail\n" );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールマップからグリッドマップに移動
 *
 *	@param	p_fieldmap
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void mapCtrlHybrid_ChangeRailToGrid( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, u16 dir, BOOL dir_key, const VecFx32* cp_pos, int trg, int cont )
{
  u16 animeFrame;
  u16 animeIndex;
  MMDL* p_mmdl;
  PLAYER_MOVEBIT mbit = PLAYER_MOVEBIT_NON;

  p_mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );

  // アニメーションフレーム取得
  MMDL_DrawBlactWork_GetAnimeFrame( p_mmdl, &animeIndex, &animeFrame );

  //NogridWork内部初期化
  FIELD_PLAYER_MoveNoGrid( p_wk->p_player, 0, 0 );

  if( dir_key == FALSE ){
    dir = MMDL_GetDirDisp( p_mmdl );
  }

  // グリッドに変更
  mapCtrlHybrid_ChangeBaseSystem( p_fieldmap, p_wk, FLDMAP_BASESYS_GRID, cp_pos, dir, animeIndex, animeFrame );

  //GridWork内部初期化
  if( FIELD_PLAYER_CheckPossibleDash(p_wk->p_player) == TRUE ){
    mbit |= PLAYER_MOVEBIT_DASH;
  }
  FIELD_PLAYER_MoveGrid( p_wk->p_player, trg, cont, mbit );

  TOMOYA_Printf( "change rail to grid\n" );
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
static void mapCtrlHybrid_ChangeBaseSystem( FIELDMAP_WORK* p_fieldmap, FIELDMAP_CTRL_HYBRID* p_wk, FLDMAP_BASESYS_TYPE type, const void* cp_pos, u16 dir, u16 animeIndex, u16 animeFrame )
{
  MMDL* p_mmdl;
  FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( p_fieldmap );
  FIELD_CAMERA* p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );

  if( p_wk->base_type == type )
  {
    return ;
  }

  
  p_mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );

  if( MMDL_CheckMoveBitAcmd( p_mmdl ) ){
    // アニメーションの停止
    MMDL_FreeAcmd( p_mmdl );
  }


  
  if( type == FLDMAP_BASESYS_GRID )
  {
    static MMDL_HEADER data_MMdlHeader =
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

    data_MMdlHeader.obj_code = MMDL_GetOBJCode( p_mmdl );
    
    // 動作変更
    FIELD_PLAYER_StopNoGrid( p_wk->p_player );
    MMDL_ChangeMoveParam( p_mmdl, &data_MMdlHeader );

    //  座標を設定
    {
      MMDL_InitGridPosition( p_mmdl, FX32_TO_GRID( pos.x ), FX32_TO_GRID( pos.y ), FX32_TO_GRID( pos.z ), dir );
      
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
    FIELD_PLAYER_RestartNoGrid( p_wk->p_player, cp_pos );

    // カメラ設定
    FLDNOGRID_MAPPER_BindCameraWork( p_mapper, FIELD_PLAYER_GetNoGridRailWork( p_wk->p_player ) );

    // カメラのがくつき回避
    // カメラトレースデータ今の位置で上書き。
    FIELD_CAMERA_RestartTrace( p_camera );

  }

  // 方向を合わせる
  MMDL_SetDirDisp( p_mmdl, dir );
  MMDL_SetDirMove( p_mmdl, dir );

  // アニメーションフレーム設定
  MMDL_DrawBlactWork_SetAnimeFrame( p_mmdl, animeIndex, animeFrame );

  p_wk->base_type = type;
}


