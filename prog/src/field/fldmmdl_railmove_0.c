//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fldfmmdl_railmove_0.c
 * @brief	  フィールド動作モデル　レール基本動作系その0
 *	@author	tomoya takahashi 
 *	@date		2009.08.26
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "fldmmdl.h"

#include "fldmmdl_procmove.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------


// DIRからレールキーに変換
static const u8 sc_DIR_TO_RAILKEY[DIR_MAX4] = 
{
  RAIL_KEY_UP,
  RAIL_KEY_DOWN,
  RAIL_KEY_LEFT,
  RAIL_KEY_RIGHT,
};


//-------------------------------------
///	GRID Rをもとめる式
//=====================================
#define MMDL_GET_RAILGRID_R(grid_size)    ( FX_Div( (grid_size), 4<<FX32_SHIFT ) )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	レール共通構造体
//=====================================
typedef struct 
{
  FIELD_RAIL_WORK* rail_wk;           ///<レール移動ワーク 4byte
  RAIL_LOCATION rail_location;        ///<レール移動用位置情報 8byte
} MV_RAIL_COMMON_WORK;
#define MV_RAIL_COMMON_WORK_SIZE (sizeof(MV_RAIL_COMMON_WORK))		///<MV_RAIL_COMMONサイズ


//-------------------------------------
///	MV_RAIL_DMY_WORK構造体
//=====================================
typedef struct 
{
  MV_RAIL_COMMON_WORK rail_wk;
} MV_RAIL_DMY_WORK;
#define MV_RAIL_DMY_WORK_SIZE (sizeof(MV_RAIL_DMY_WORK))		///<MV_RAIL_DMYサイズ

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 基本関数
static RAIL_KEY MMdl_ConvertDir_RailKey( u16 dir );
static RAIL_FRAME MMdl_ConvertWait_RailFrame( s16 wait );

// COMMON
static void MMdl_RailCommon_Init( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl );
static void MMdl_RailCommon_Move( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl );
static void MMdl_RailCommon_Delete( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl );
static void MMdl_RailCommon_Return( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl );



//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションの設定
 *
 *	@param	fmmdl        モデルワーク
 *	@param	location    ロケーション
 */
//-----------------------------------------------------------------------------
void MMDL_SetRailLocation( MMDL * fmmdl, const RAIL_LOCATION* location )
{
  MV_RAIL_COMMON_WORK* p_work;
  VecFx32 pos;

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE ) );

	p_work = MMDL_GetMoveProcWork( fmmdl );

  FIELD_RAIL_WORK_SetLocation( p_work->rail_wk, location );
  p_work->rail_location = *location;

  // 座標の更新
  FIELD_RAIL_WORK_GetPos( p_work->rail_wk, &pos );
	MMDL_SetVectorPos( fmmdl, &pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションの取得
 *
 *	@param	fmmdl        モデルワーク
 *	@param	location    ロケーション格納先
 */
//-----------------------------------------------------------------------------
void MMDL_GetRailLocation( const MMDL * fmmdl, RAIL_LOCATION* location )
{
  const MV_RAIL_COMMON_WORK* cp_work;

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE ) );

	cp_work = MMDL_GetMoveProcWork( (MMDL*)fmmdl );

  *location = cp_work->rail_location;
}

//----------------------------------------------------------------------------
/**
 *	@brief  １つ前のロケーションの取得
 *
 *	@param	fmmdl        モデルワーク
 *	@param	location    ロケーション格納先
 */
//-----------------------------------------------------------------------------
void MMDL_GetOldRailLocation( const MMDL * fmmdl, RAIL_LOCATION* location )
{
  const MV_RAIL_COMMON_WORK* cp_work;

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE ) );

	cp_work = MMDL_GetMoveProcWork( (MMDL*)fmmdl );
  FIELD_RAIL_WORK_GetLastLocation( cp_work->rail_wk, location );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールワークの取得
 *
 *	@param	fmmdl  モデルワーク
 *
 *	@return レールワーク
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_WORK* MMDL_GetRailWork( const MMDL * fmmdl )
{
  const MV_RAIL_COMMON_WORK* cp_work;

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE ) );

	cp_work = MMDL_GetMoveProcWork( (MMDL*)fmmdl );

  return cp_work->rail_wk;
}


//----------------------------------------------------------------------------
/**
 *	@brief  レール動作　リクエスト
 *
 *	@param	fmmdl   ワーク
 *	@param	dir     方向  
 *	@param	wait    ウエイト  16,8,4,2
 */
//-----------------------------------------------------------------------------
BOOL MMDL_ReqRailMove( MMDL * fmmdl, u16 dir, s16 wait )
{
  MV_RAIL_COMMON_WORK* p_work;
  u32 railkey;
  u32 frame;


  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE ) );

	p_work = MMDL_GetMoveProcWork( fmmdl );


  //
  railkey = MMdl_ConvertDir_RailKey( dir );
  frame = MMdl_ConvertWait_RailFrame( wait );
  
  return FIELD_RAIL_WORK_ForwardReq( p_work->rail_wk, frame, railkey );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドレール動作モデル　移動チェック
 *
 *	@param	mmdl              モデル
 *	@param	now_location      現在のロケーション
 *	@param  next_location     次のロケーション
 *
 * @retval	u32		ヒットビット。MMDL_MOVEHITBIT_LIM等
 */
//-----------------------------------------------------------------------------
u32 MMDL_HitCheckRailMove( const MMDL *mmdl, const RAIL_LOCATION* now_location, const RAIL_LOCATION* next_location )
{
  const MV_RAIL_COMMON_WORK* cp_work;
  u32 ret = MMDL_MOVEHITBIT_NON;
  FLDNOGRID_MAPPER* p_mapper;

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE ) );

  // レール動作ワーク取得
	cp_work = MMDL_GetMoveProcWork( (MMDL*)mmdl );

  // マッパー取得
  p_mapper = MMDLSYS_GetNOGRIDMapper( MMDL_GetMMdlSys( mmdl ) );

  // next_locationのレールがあるか？
  if( FIELD_RAIL_WORK_CheckLocation( cp_work->rail_wk, next_location ) == FALSE )
  {
    TOMOYA_Printf( "CHECK LOCATION MOVE LIMIT\n" );
    ret |= MMDL_MOVEHITBIT_LIM;
  }

  // アトリビュートチェック
	if( MMDL_CheckMoveBitAttrGetOFF(mmdl) == FALSE )
  {
    MAPATTR attr;
    MAPATTR_FLAG attr_flag;

    attr = FLDNOGRID_MAPPER_GetAttr( p_mapper, next_location );
    attr_flag = MAPATTR_GetAttrFlag( attr );

    if( attr_flag & MAPATTR_FLAGBIT_HITCH )
    {
		  ret |= MMDL_MOVEHITBIT_ATTR;
    }
  }

  // モデルあたり判定
  if( MMDL_HitCheckRailMoveFellow(mmdl, next_location) )
  {
    TOMOYA_Printf( "HIT OBJ\n" );
		ret |= MMDL_MOVEHITBIT_OBJ;
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドレール動作モデル  移動チェック  今の場所から
 *
 *	@param	mmdl              モデル
 *	@param  next_location     次のロケーション
 *
 * @retval	u32		ヒットビット。MMDL_MOVEHITBIT_LIM等
 */
//-----------------------------------------------------------------------------
u32 MMDL_HitCheckRailMoveCurrent( const MMDL *mmdl, const RAIL_LOCATION* next_location )
{
  const MV_RAIL_COMMON_WORK* cp_work;
  RAIL_LOCATION location;

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE ) );

  // レール動作ワーク取得
	cp_work = MMDL_GetMoveProcWork( (MMDL*)mmdl );

  // 今のロケーション取得
  FIELD_RAIL_WORK_GetLocation( cp_work->rail_wk, &location );

  return MMDL_HitCheckRailMove(mmdl, &location, next_location);
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドレール動作モデル  移動チェック  今の場所からdirの方向に
 *
 *	@param	mmdl              モデル
 *	@param  dir               移動方向 
 *
 * @retval	u32		ヒットビット。MMDL_MOVEHITBIT_LIM等
 */
//-----------------------------------------------------------------------------
u32 MMDL_HitCheckRailMoveDir( const MMDL *mmdl, u16 dir )
{
  const MV_RAIL_COMMON_WORK* cp_work;
  RAIL_LOCATION now_location;
  RAIL_LOCATION next_location;
  BOOL check_next_location;
  const FLDNOGRID_MAPPER* cp_mapper = MMDLSYS_GetNOGRIDMapper( MMDL_GetMMdlSys( mmdl ) );
  const FIELD_RAIL_MAN* cp_railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE ) );

  // レール動作ワーク取得
	cp_work = MMDL_GetMoveProcWork( (MMDL*)mmdl );

  // DIRに対応した、ロケーションを取得
  FIELD_RAIL_WORK_GetLocation( cp_work->rail_wk, &now_location );
  check_next_location = FIELD_RAIL_MAN_CalcRailKeyLocation( cp_railman, &now_location, MMdl_ConvertDir_RailKey( dir ), &next_location );

  
  if( check_next_location )
  {
    return MMDL_HitCheckRailMove(mmdl, &now_location, &next_location);
  }

  TOMOYA_Printf( "MOVE LIMIT\n" );
  return MMDL_MOVEHITBIT_LIM;
}


//----------------------------------------------------------------------------
/**
 *	@brief  レール動作中の物体あたり判定
 *
 *	@param	mmdl      モデル
 *	@param  location  位置
 *
 *	@retval TRUE  衝突
 *	@retval FALSE なし
 */
//-----------------------------------------------------------------------------
BOOL MMDL_HitCheckRailMoveFellow( const MMDL * mmdl, const RAIL_LOCATION* next_location )
{
  const MMDLSYS* cp_sys = MMDL_GetMMdlSys( mmdl );
  const FLDNOGRID_MAPPER* cp_mapper = MMDLSYS_GetNOGRIDMapper( cp_sys );
  const FIELD_RAIL_MAN* cp_railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );
  VecFx32 my_pos;
  VecFx32 mdl_pos;
  fx32 dist;
  MMDL* cmdl;
  u32 no = 0;
  fx32 grid_r;
  
  FIELD_RAIL_MAN_GetLocationPosition( cp_railman, next_location, &my_pos );

  grid_r = FIELD_RAIL_MAN_GetRailGridSize( cp_railman );
  grid_r = MMDL_GET_RAILGRID_R( grid_r );

	while( MMDLSYS_SearchUseMMdl(cp_sys,&cmdl,&no) == TRUE )
  {
		if( cmdl != mmdl )
    {
			if( MMDL_CheckStatusBit( cmdl,MMDL_STABIT_FELLOW_HIT_NON) == 0 )
      {
        MMDL_GetVectorPos( cmdl, &mdl_pos );

//        TOMOYA_Printf( "grid_r 0x%x my_pos x[0x%x]y[0x%x]z[0x%x] mdl_pos x[0x%x]y[0x%x]z[0x%x] \n", grid_r, my_pos.x, my_pos.y, my_pos.z, mdl_pos.x, mdl_pos.y, mdl_pos.z );
        if( FIELD_RAIL_TOOL_HitCheckSphere( &my_pos, &mdl_pos, grid_r ) )
        {
          return TRUE;
        }
      }
    }
  }
    
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションのあたりにいるOBJを取得する
 *
 *	@param	sys         MMDLSYS
 *	@param	location    ロケーション
 *	@param	old_hit     前のロケーションもちぇっくするか
 *
 *	@return ヒットしたOBJ
 */
//-----------------------------------------------------------------------------
MMDL * MMDLSYS_SearchRailLocation( const MMDLSYS *sys, const RAIL_LOCATION* location, BOOL old_hit )
{
	u32 no = 0;
	MMDL *mmdl;
  const FLDNOGRID_MAPPER* cp_mapper = MMDLSYS_GetNOGRIDMapper( sys );
  const FIELD_RAIL_MAN* cp_railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );
  VecFx32 check_pos;
  VecFx32 mdl_pos;
  RAIL_LOCATION old_location;
  fx32 grid_r;
  

  FIELD_RAIL_MAN_GetLocationPosition( cp_railman, location, &check_pos );
  grid_r = FIELD_RAIL_MAN_GetRailGridSize( cp_railman );
  grid_r = MMDL_GET_RAILGRID_R( grid_r );

	while( MMDLSYS_SearchUseMMdl(sys,&mmdl,&no) == TRUE ){
		if( old_hit ){
      MMDL_GetOldRailLocation( mmdl, &old_location );
      FIELD_RAIL_MAN_GetLocationPosition( cp_railman, &old_location, &mdl_pos );

			if( FIELD_RAIL_TOOL_HitCheckSphere( &check_pos, &mdl_pos, grid_r ) ){
				return( mmdl );
			}
		}
		
    MMDL_GetVectorPos( mmdl, &mdl_pos );
    if( FIELD_RAIL_TOOL_HitCheckSphere( &check_pos, &mdl_pos, grid_r ) ){
      return( mmdl );
		}
	}
	
	return( NULL );
}


//----------------------------------------------------------------------------
/**
 *	@brief  レール動作なし　初期化
 *
 *	@param	fmmdl   モデルワーク
 */
//-----------------------------------------------------------------------------
void MMDL_RailDmy_Init( MMDL * fmmdl )
{
  MV_RAIL_DMY_WORK* p_work;
	p_work = MMDL_InitMoveProcWork( fmmdl, MV_RAIL_DMY_WORK_SIZE );
  MMdl_RailCommon_Init( &p_work->rail_wk, fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール動作なし　動作
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
void MMDL_RailDmy_Move( MMDL * fmmdl )
{
  MV_RAIL_DMY_WORK* p_work;
	p_work = MMDL_GetMoveProcWork( fmmdl );
  MMdl_RailCommon_Move( &p_work->rail_wk, fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール動作なし　破棄
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
void MMDL_RailDmy_Delete( MMDL * fmmdl )
{
  MV_RAIL_DMY_WORK* p_work;
	p_work = MMDL_GetMoveProcWork( fmmdl );
  MMdl_RailCommon_Delete( &p_work->rail_wk, fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール動作なし　復帰
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
void MMDL_RailDmy_Return( MMDL * fmmdl )
{
  MV_RAIL_DMY_WORK* p_work;
	p_work = MMDL_GetMoveProcWork( fmmdl );
  MMdl_RailCommon_Return( &p_work->rail_wk, fmmdl );
}





//-----------------------------------------------------------------------------
/**
 *          private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  方向をレールキーに置き換える
 *
 *	@param	dir   方向
 *
 *	@return キー
 */
//-----------------------------------------------------------------------------
static RAIL_KEY MMdl_ConvertDir_RailKey( u16 dir )
{
  GF_ASSERT( dir < DIR_MAX4 );
  return sc_DIR_TO_RAILKEY[dir];
}

//----------------------------------------------------------------------------
/**
 *	@brief  ウエイトをレールフレームに置き換える
 *
 *	@param	s16 wait 
 *
 *	@return レールフレーム
 */
//-----------------------------------------------------------------------------
static RAIL_FRAME MMdl_ConvertWait_RailFrame( s16 wait )
{
  u32 ret = RAIL_FRAME_8;
  
  switch( wait )
  {
  case 16:
    ret = RAIL_FRAME_16;
    break;

  case 8:
    ret = RAIL_FRAME_8;
    break;

  case 4:
    ret = RAIL_FRAME_4;
    break;

  case 2:
    ret = RAIL_FRAME_2;
    break;

  default:
    GF_ASSERT( 0 );
    break;
  }

  return ret;
}


// COMMON
//----------------------------------------------------------------------------
/**
 *	@brief  レール移動共通  初期か
 *
 *	@param	fmmdl   ワーク
 */
//-----------------------------------------------------------------------------
static void MMdl_RailCommon_Init( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl )
{
  const MMDLSYS* cp_sys = MMDL_GetMMdlSys( fmmdl );
  FLDNOGRID_MAPPER* p_mapper = MMDLSYS_GetNOGRIDMapper( cp_sys );

  p_work->rail_wk = FLDNOGRID_MAPPER_CreateRailWork( p_mapper );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール移動共通  動作
 *
 *	@param	fmmdl   ワーク
 */
//-----------------------------------------------------------------------------
static void MMdl_RailCommon_Move( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl )
{
  // ロケーションの保存
  FIELD_RAIL_WORK_GetLocation( p_work->rail_wk, &p_work->rail_location );  
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール移動共通  破棄
 *
 *	@param	fmmdl   ワーク
 */
//-----------------------------------------------------------------------------
static void MMdl_RailCommon_Delete( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl )
{
  const MMDLSYS* cp_sys = MMDL_GetMMdlSys( fmmdl );
  FLDNOGRID_MAPPER* p_mapper = MMDLSYS_GetNOGRIDMapper( cp_sys );

  FLDNOGRID_MAPPER_DeleteRailWork( p_mapper, p_work->rail_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール移動共通  復帰
 *
 *	@param	fmmdl   ワーク
 */
//-----------------------------------------------------------------------------
static void MMdl_RailCommon_Return( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl )
{
  const MMDLSYS* cp_sys = MMDL_GetMMdlSys( fmmdl );
  FLDNOGRID_MAPPER* p_mapper = MMDLSYS_GetNOGRIDMapper( cp_sys );

  p_work->rail_wk = FLDNOGRID_MAPPER_CreateRailWork( p_mapper );
}




