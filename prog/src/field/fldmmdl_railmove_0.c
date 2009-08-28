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

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE ) );

	p_work = MMDL_GetMoveProcWork( fmmdl );

  FIELD_RAIL_WORK_SetLocation( p_work->rail_wk, location );
  p_work->rail_location = *location;
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
  
  return FIELD_RAIL_WORK_ForwardReq( p_work->rail_wk, railkey, frame );
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




