//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_nogrid_mapper.c
 *	@brief  ノーグリッド動作　マッパー
 *	@author	tomoya takahashi
 *	@date		2009.08.25
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "arc/fieldmap/field_rail_data.naix"
#include "arc_def.h"

#include "field_nogrid_mapper.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	ノーグリッド動作マッパーワーク
//=====================================
struct _FLDNOGRID_MAPPER 
{
  // 各モジュール
  FIELD_RAIL_MAN*       p_railMan;
  FIELD_RAIL_LOADER*    p_railLoader;
  RAIL_ATTR_DATA*       p_attr;
  FLD_SCENEAREA*        p_areaMan;
  FLD_SCENEAREA_LOADER* p_areaLoader;

  BOOL rail_camera_stop;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッド動作マッパー　ワーク生成
 *
 *	@param	heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
FLDNOGRID_MAPPER* FLDNOGRID_MAPPER_Create( u32 heapID, FIELD_CAMERA* p_camera, FLD_SCENEAREA* p_sceneArea, FLD_SCENEAREA_LOADER* p_sceneAreaLoader )
{
  FLDNOGRID_MAPPER* p_mapper;

  p_mapper = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDNOGRID_MAPPER) );

  // 各モジュールワーク生成
  p_mapper->p_railMan       = FIELD_RAIL_MAN_Create( heapID, FIELD_RAIL_WORK_MAX, p_camera ); 
  p_mapper->p_railLoader    = FIELD_RAIL_LOADER_Create( heapID );
  p_mapper->p_areaMan       = p_sceneArea;
  p_mapper->p_areaLoader    = p_sceneAreaLoader;
  p_mapper->p_attr          = RAIL_ATTR_Create( heapID );

  return p_mapper;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッド動作マッパー破棄
 *
 *	@param	p_mapper  マッパーワーク
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_Delete( FLDNOGRID_MAPPER* p_mapper )
{
  GF_ASSERT( p_mapper );

  RAIL_ATTR_Delete( p_mapper->p_attr );
  FIELD_RAIL_MAN_Delete( p_mapper->p_railMan );
  FIELD_RAIL_LOADER_Delete( p_mapper->p_railLoader );

  GFL_HEAP_FreeMemory( p_mapper );
}





//----------------------------------------------------------------------------
/**
 *	@brief  アクティブ設定
 *
 *	@param	p_mapper    マッパー
 *	@param	flag        フラグ
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_SetActive( FLDNOGRID_MAPPER* p_mapper, BOOL flag )
{
  GF_ASSERT( p_mapper );
	FIELD_RAIL_MAN_SetActiveFlag(p_mapper->p_railMan, flag);
	FLD_SCENEAREA_SetActiveFlag(p_mapper->p_areaMan, flag);
}


//----------------------------------------------------------------------------
/**
 *	@brief  アクティブ状態かチェック
 *
 *	@param	cp_mapper   マッパー
 *
 *	@retval TRUE    アクティブ
 *	@retval FALSE   非アクティブ
 */
//-----------------------------------------------------------------------------
BOOL FLDNOGRID_MAPPER_IsActive( const FLDNOGRID_MAPPER* cp_mapper )
{
  BOOL result;
  
  GF_ASSERT( cp_mapper );

  result = FIELD_RAIL_MAN_GetActiveFlag( cp_mapper->p_railMan );

  return result;
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールカメラを実行するか設定
 *
 *	@param	p_mapper  マッパー
 *	@param	flag      フラグ
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_SetRailCameraActive( FLDNOGRID_MAPPER* p_mapper, BOOL flag )
{
  if( flag )
  {
    p_mapper->rail_camera_stop = FALSE;
  }
  else
  {
    p_mapper->rail_camera_stop = TRUE;
  }
	FLD_SCENEAREA_SetActiveFlag(p_mapper->p_areaMan, flag);
}



//----------------------------------------------------------------------------
/**
 *	@brief  データの反映
 *
 *	@param	p_mapper  マッパー
 *	@param	cp_data   データ
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_ResistData( FLDNOGRID_MAPPER* p_mapper, const FLDNOGRID_RESISTDATA* cp_data, u32 heapID )
{
  u32 filecnt;
  
  GF_ASSERT( p_mapper );
  
  // 破棄処理
  FLDNOGRID_MAPPER_Release( p_mapper );

  // 情報ロード＆構築
  if( cp_data->railDataID != FLDNOGRID_RESISTDATA_NONE )
  {
    // 全ファイル数の半分からATTRは始まる
    filecnt = GFL_ARC_GetDataFileCnt( ARCID_RAIL_DATA );
    filecnt /= 2;
    
    FIELD_RAIL_LOADER_Load( p_mapper->p_railLoader, cp_data->railDataID, heapID );
    FIELD_RAIL_MAN_Load( p_mapper->p_railMan, FIELD_RAIL_LOADER_GetData( p_mapper->p_railLoader ) );

    RAIL_ATTR_Load( p_mapper->p_attr, cp_data->railDataID + filecnt, heapID );
  }
  if( cp_data->areaDataID != FLDNOGRID_RESISTDATA_NONE )
  {
    FLD_SCENEAREA_LOADER_Load( p_mapper->p_areaLoader, cp_data->areaDataID, heapID );
    FLD_SCENEAREA_Load( p_mapper->p_areaMan, 
        FLD_SCENEAREA_LOADER_GetData( p_mapper->p_areaLoader ),
        FLD_SCENEAREA_LOADER_GetDataNum( p_mapper->p_areaLoader ),
        FLD_SCENEAREA_LOADER_GetFunc( p_mapper->p_areaLoader ) );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アーカイブデータからデータ設定
 *
 *	@param	p_mapper
 *	@param	dataID
 *	@param	heapID 
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_ResistDataArc( FLDNOGRID_MAPPER* p_mapper, u32 dataID, u32 heapID )
{
  FLDNOGRID_RESISTDATA* p_resist;
  u32 size;

  size = GFL_ARC_GetDataSize( ARCID_FLD_RAILSETUP, dataID );

  p_resist = GFL_HEAP_AllocClearMemoryLo( heapID, size );

  GFL_ARC_LoadData( p_resist, ARCID_FLD_RAILSETUP, dataID );

  FLDNOGRID_MAPPER_ResistData( p_mapper, p_resist, heapID );  

  GFL_HEAP_FreeMemory( p_resist );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッド動作　情報破棄
 *
 *	@param	p_mapper  マッパー
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_Release( FLDNOGRID_MAPPER* p_mapper )
{
  GF_ASSERT( p_mapper );

  FLD_SCENEAREA_Release( p_mapper->p_areaMan );
  FIELD_RAIL_MAN_Clear( p_mapper->p_railMan );
  FIELD_RAIL_LOADER_Clear( p_mapper->p_railLoader );
  FLD_SCENEAREA_LOADER_Clear( p_mapper->p_areaLoader );
  RAIL_ATTR_Release( p_mapper->p_attr );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッド動作  設定データの有無
 *
 *	@param	cp_mapper   マッパー
 *
 *	@retval TRUE  ある
 *	@retval FALSE ない
 */
//-----------------------------------------------------------------------------
BOOL FLDNOGRID_MAPPER_IsResistData( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );

  if( FIELD_RAIL_MAN_IsLoad( cp_mapper->p_railMan ) )
  {
    return TRUE;
  }
  return FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッド動作　メイン処理
 *
 *	@param	p_mapper  マッパーワーク
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_Main( FLDNOGRID_MAPPER* p_mapper )
{
  VecFx32 rail_pos;
  BOOL camera_move = FALSE;

  // レールシステムメイン
  FIELD_RAIL_MAN_Update( p_mapper->p_railMan );
  if( p_mapper->rail_camera_stop == FALSE )
  {
    FIELD_RAIL_MAN_UpdateCamera( p_mapper->p_railMan, FALSE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラアップデート
 *
 *	@param	p_mapper  マッパー
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_UpdateCamera( FLDNOGRID_MAPPER* p_mapper )
{
  FIELD_RAIL_MAN_UpdateCamera( p_mapper->p_railMan, TRUE );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールワークの生成
 *
 *	@param	p_mapper  マッパー
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_WORK* FLDNOGRID_MAPPER_CreateRailWork( FLDNOGRID_MAPPER* p_mapper )
{
  GF_ASSERT( p_mapper );
  return FIELD_RAIL_MAN_CreateWork( p_mapper->p_railMan );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールワークの破棄
 *
 *	@param	p_mapper    マッパー
 *	@param	p_railWork  破棄するレールワーク
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_DeleteRailWork( FLDNOGRID_MAPPER* p_mapper, FIELD_RAIL_WORK* p_railWork )
{
  GF_ASSERT( p_mapper );
  FIELD_RAIL_MAN_DeleteWork( p_mapper->p_railMan, p_railWork );
}



//----------------------------------------------------------------------------
/**
 *	@brief  カメラとバインドするレールワークを取得
 *
 *	@param	p_mapper      マッパー
 *	@param	p_railWork    レールワーク
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_BindCameraWork( FLDNOGRID_MAPPER* p_mapper, const FIELD_RAIL_WORK* cp_railWork )
{
  GF_ASSERT( p_mapper );
  GF_ASSERT( cp_railWork );
  FIELD_RAIL_MAN_BindCamera( p_mapper->p_railMan, cp_railWork );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラとのバインドを破棄
 *
 *	@param	p_mapper  マッパー
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_UnBindCameraWork( FLDNOGRID_MAPPER* p_mapper )
{
  GF_ASSERT( p_mapper );
  FIELD_RAIL_MAN_UnBindCamera( p_mapper->p_railMan );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールアトリビュート情報の取得
 *
 *	@param	cp_mapper     マッパー
 *	@param	cp_location   レールロケーション
 *
 *	@return マップアトリビュート
 */
//-----------------------------------------------------------------------------
MAPATTR FLDNOGRID_MAPPER_GetAttr( const FLDNOGRID_MAPPER* cp_mapper, const RAIL_LOCATION* cp_location )
{
  GF_ASSERT( cp_mapper );
  return RAIL_ATTR_GetAttribute( cp_mapper->p_attr, cp_location );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールロケーションでのカメラパラメータを　FIELD_CAMERAに設定
 *
 *	@param	p_mapper        マッパー
 *	@param	cp_location     ロケーション
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_SetUpLocationCamera( FLDNOGRID_MAPPER* p_mapper, const RAIL_LOCATION* cp_location )
{
  GF_ASSERT( p_mapper );

  FIELD_RAIL_MAN_SetUpLocationCameraParam( p_mapper->p_railMan, cp_location );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール  ライン分岐のON・OFF
 *
 *	@param	p_mapper    マッパー
 *	@param	line_index  ラインインデックス
 *	@param	flag        フラグ  TRUE：分岐ON　　FALSE：分岐OFF
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_SetLineActive( FLDNOGRID_MAPPER* p_mapper, u32 line_index, BOOL flag )
{
  GF_ASSERT( p_mapper );
  FIELD_RAIL_MAN_SetLineActive( p_mapper->p_railMan, line_index, flag );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール　ライン分岐のON・OFF取得
 *
 *	@param	cp_mapper   マッパー
 *	@param	line_index  ラインインデックス
 *  
 *	@retval TRUE    ライン分岐可能
 *	@retval FALSE   ライン分岐不可能
 */
//-----------------------------------------------------------------------------
BOOL FLDNOGRID_MAPPER_GetLineActive( const FLDNOGRID_MAPPER* cp_mapper, u32 line_index )
{
  GF_ASSERT( cp_mapper );
  return FIELD_RAIL_MAN_GetLineActive( cp_mapper->p_railMan, line_index );
}



//----------------------------------------------------------------------------
/**
 *	@brief  レールマネージャモジュールの取得
 *
 *	@param	cp_mapper   マッパー
 *
 *	@return モジュール
 */
//-----------------------------------------------------------------------------
const FIELD_RAIL_MAN* FLDNOGRID_MAPPER_GetRailMan( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );
  return cp_mapper->p_railMan;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エリアマネージャモジュールの取得
 *
 *	@param	cp_mapper   マッパー
 *
 *	@return モジュール
 */
//-----------------------------------------------------------------------------
const FLD_SCENEAREA* FLDNOGRID_MAPPER_GetSceneAreaMan( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );
  return cp_mapper->p_areaMan;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールローダーモジュールの取得
 *
 *	@param	cp_mapper   マッパー
 *
 *	@return モジュール
 */
//-----------------------------------------------------------------------------
const FIELD_RAIL_LOADER* FLDNOGRID_MAPPER_GetRailLoader( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );
  return cp_mapper->p_railLoader;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エリアローダーのモジュール取得
 *
 *	@param	cp_mapper   マッパー
 *
 *	@return モジュール
 */
//-----------------------------------------------------------------------------
const FLD_SCENEAREA_LOADER* FLDNOGRID_MAPPER_GetSceneAreaLoader( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );
  return cp_mapper->p_areaLoader;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アトリビュートデータの取得
 *
 *	@param	cp_mapper   マッパー
 *
 *	@return アトリビュートデータ
 */
//-----------------------------------------------------------------------------
const RAIL_ATTR_DATA* FLDNOGRID_MAPPER_GetRailAttrData( const FLDNOGRID_MAPPER* cp_mapper )
{
  GF_ASSERT( cp_mapper );
  return cp_mapper->p_attr;
}


// デバック用
#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  レールbinaryの読み込み
 *
 *	@param	p_mapper    マッパー
 *	@param	cp_dat       データ
 *	@param	size        データサイズ
 *	@param  heapID      ヒープID
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_DEBUG_LoadRailBynary( FLDNOGRID_MAPPER* p_mapper, const void* cp_dat, u32 size, HEAPID heapID )
{
  void * p_buff;
  
  GF_ASSERT( p_mapper );
  GF_ASSERT( cp_dat );

  FIELD_RAIL_LOADER_Clear( p_mapper->p_railLoader );

  // バッファ確保
  p_buff = GFL_HEAP_AllocClearMemory( heapID, size );
	GFL_STD_MemCopy( cp_dat, p_buff, size );
  
  FIELD_RAIL_LOADER_DEBUG_LoadBinary( p_mapper->p_railLoader, p_buff, size );

  FIELD_RAIL_MAN_Clear( p_mapper->p_railMan );
	FIELD_RAIL_MAN_Load( p_mapper->p_railMan, FIELD_RAIL_LOADER_GetData( p_mapper->p_railLoader ) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  エリアbinaryの読み込み
 *
 *	@param	p_mapper  マッパー
 *	@param	p_dat     データ
 *	@param	size      データサイズ
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_DEBUG_LoadAreaBynary( FLDNOGRID_MAPPER* p_mapper, const void* cp_dat, u32 size, HEAPID heapID )
{
  void * p_buff;

  GF_ASSERT( p_mapper );
  GF_ASSERT( cp_dat );

  FLD_SCENEAREA_LOADER_Clear( p_mapper->p_areaLoader );

  p_buff = GFL_HEAP_AllocClearMemory( heapID, size );
	GFL_STD_MemCopy( cp_dat, p_buff, size );
  
  FLD_SCENEAREA_LOADER_LoadBinary( p_mapper->p_areaLoader, p_buff, size );


	FLD_SCENEAREA_Release( p_mapper->p_areaMan );
	FLD_SCENEAREA_Load( p_mapper->p_areaMan, 
			FLD_SCENEAREA_LOADER_GetData( p_mapper->p_areaLoader ),
			FLD_SCENEAREA_LOADER_GetDataNum( p_mapper->p_areaLoader ),
			FLD_SCENEAREA_LOADER_GetFunc( p_mapper->p_areaLoader ) );
}


//----------------------------------------------------------------------------
/**
 *	@brief  アトリビュートbinaryの読み込み
 */
//-----------------------------------------------------------------------------
void FLDNOGRID_MAPPER_DEBUG_LoadAttrBynary( FLDNOGRID_MAPPER* p_mapper, const void* cp_dat, u32 size, HEAPID heapID )
{
  void * p_buff;

  RAIL_ATTR_Release( p_mapper->p_attr );
  
  p_buff = GFL_HEAP_AllocClearMemory( heapID, size );
	GFL_STD_MemCopy( cp_dat, p_buff, size );

  RAIL_ATTR_DEBUG_LoadBynary( p_mapper->p_attr, p_buff, size, heapID );
}


//----------------------------------------------------------------------------
/**
 *	@brief  自由にアクセスできるレールマネージャを取得
 *
 *	@param	cp_mapper 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_MAN* FLDNOGRID_MAPPER_DEBUG_GetRailMan( const FLDNOGRID_MAPPER* cp_mapper )
{
  return cp_mapper->p_railMan;
}


#endif



