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

//-------------------------------------
///	MV_RAIL_RND_WORK　動作シーケンス
//=====================================
enum
{
  MV_RND_MOVE_SEQ_ACT,      ///<振り向きアクション
  MV_RND_MOVE_SEQ_ACT_WAIT, ///<動作待ち
  MV_RND_MOVE_SEQ_TIME_WAIT,///<待ち時間
  MV_RND_MOVE_SEQ_WALK_ACT, ///<移動アクション
  MV_RND_MOVE_SEQ_WALK_ACT_WAIT, ///<移動アクション
};


//-------------------------------------
///	MV_RAIL_RT2_WORK　動作シーケンス
//=====================================
enum
{
  MV_RT2_MOVE_SEQ_CHECK_DIR,      ///<初期方向チェック
  MV_RT2_MOVE_SEQ_ACT_SET,        ///<動作設定
  MV_RT2_MOVE_SEQ_ACT_WAIT,       ///<動作待ち
};

//--------------------------------------------------------------
///	方向テーブルID
//--------------------------------------------------------------
enum
{
	DIRID_MvDirRndDirTbl = 0,
	DIRID_MvDirRndDirTbl_UL,
	DIRID_MvDirRndDirTbl_UR,
	DIRID_MvDirRndDirTbl_DL,
	DIRID_MvDirRndDirTbl_DR,
	DIRID_MvDirRndDirTbl_UDL,
	DIRID_MvDirRndDirTbl_UDR,
	DIRID_MvDirRndDirTbl_ULR,
	DIRID_MvDirRndDirTbl_DLR,
	DIRID_MvDirRndDirTbl_UD,
	DIRID_MvDirRndDirTbl_LR,
	DIRID_END,
	DIRID_MAX,

	WAIT_END = -1,
};

//--------------------------------------------------------------
//	DIR_TBL構造体
//--------------------------------------------------------------
typedef struct
{
	int id;
	const int *tbl;
}DIR_TBL;
//--------------------------------------------------------------
///	DIR_RND ウェイトテーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndWaitTbl[] =
{ 16, 32, 48, 64, WAIT_END };

//--------------------------------------------------------------
///	DIR_RND 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl[] =
{ DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UL 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UL[] =
{ DIR_UP, DIR_LEFT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UR[] =
{ DIR_UP, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_DL 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_DL[] =
{ DIR_DOWN, DIR_LEFT, DIR_NOT };

//--------------------------------------------------------------
/// RND_DR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_DR[] =
{ DIR_DOWN, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UDL 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UDL[] =
{ DIR_UP, DIR_DOWN, DIR_LEFT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UDR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UDR[] =
{ DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_ULR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_ULR[] =
{ DIR_UP, DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_DLR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_DLR[] =
{ DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
/// RND_UD 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_UD[] =
{ DIR_UP, DIR_DOWN, DIR_NOT };

//--------------------------------------------------------------
/// RND_LR 方向テーブル
//--------------------------------------------------------------
static const int DATA_MvDirRndDirTbl_LR[] =
{ DIR_LEFT, DIR_RIGHT, DIR_NOT };

//--------------------------------------------------------------
///	テーブルデータ
//--------------------------------------------------------------
static const DIR_TBL DATA_MoveDirTbl[DIRID_MAX] =
{
	{DIRID_MvDirRndDirTbl,DATA_MvDirRndDirTbl},
	{DIRID_MvDirRndDirTbl_UL,DATA_MvDirRndDirTbl_UL},
	{DIRID_MvDirRndDirTbl_UR,DATA_MvDirRndDirTbl_UR},
	{DIRID_MvDirRndDirTbl_DL,DATA_MvDirRndDirTbl_DL},
	{DIRID_MvDirRndDirTbl_DR,DATA_MvDirRndDirTbl_DR},
	{DIRID_MvDirRndDirTbl_UDL,DATA_MvDirRndDirTbl_UDL},
	{DIRID_MvDirRndDirTbl_UDR,DATA_MvDirRndDirTbl_UDR},
	{DIRID_MvDirRndDirTbl_ULR,DATA_MvDirRndDirTbl_ULR},
	{DIRID_MvDirRndDirTbl_DLR,DATA_MvDirRndDirTbl_DLR},
	{DIRID_MvDirRndDirTbl_UD,DATA_MvDirRndDirTbl_UD},
	{DIRID_MvDirRndDirTbl_LR,DATA_MvDirRndDirTbl_LR},
	{DIRID_END,NULL},
};


//-------------------------------------
///	GRID Rをもとめる式
//=====================================
#define MMDL_GET_RAILGRID_R(grid_size)    ( FX_Div( (grid_size), 4<<FX32_SHIFT ) )



//--------------------------------------------------------------
///	レールキーが前方の場合の、ＤＩＲの変換テーブル
//--------------------------------------------------------------
static const u8 sc_RAILLINE_DIR_CHANGE_TBL[ RAIL_KEY_MAX ][ DIR_MAX4 ] = 
{
  // NULLは関係なし
  {
    0
  },
  // RAIL_KEY_UP
  {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
  },
  // RAIL_KEY_RIGHT
  {
    DIR_LEFT,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_UP,
  },
  // RAIL_KEY_DOWN
  {
    DIR_DOWN,
    DIR_UP,
    DIR_RIGHT,
    DIR_LEFT,
  },
  // RAIL_KEY_LEFT
  {
    DIR_RIGHT,
    DIR_LEFT,
    DIR_UP,
    DIR_DOWN,
  },
}; 

//--------------------------------------------------------------
///	各ＤＩＲの回転情報　フィール度３Ｄ座標系での回転情報
// -z
//      上
//   左　　右   
//      下
// +z
//--------------------------------------------------------------
static const u16 sc_RAILLINE_DIR_ROT_Y[ DIR_MAX4 ] =
{
  // DIR_UP
  0,
  // DIR_DOWN
  0x8000,
  // DIR_LEFT
  0x4000,
  // DIR_RIGHT
  0xC000,
};


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	レール共通構造体
//　必ず、ワークのトップにこの構造体を入れること！！！
//
//location.type が FIELD_RAIL_TYPE_MAXのとき、設定ロケーションデータなしになります。
//=====================================
typedef struct 
{
  FIELD_RAIL_WORK* rail_wk;           ///<レール移動ワーク 4byte
  RAIL_LOCATION location;             ///<レールロケーション 8bytie   
                                      // MMdl_RailCommon_SetSaveLocationで毎フレーム更新
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

//-------------------------------------
///	MV_RAIL_DIR_RND_WORK構造体
//=====================================
typedef struct 
{
  MV_RAIL_COMMON_WORK rail_wk;
  s16 wait;
  u16 tbl_id;
} MV_RAIL_DIR_RND_WORK;
#define MV_RAIL_DIR_RND_WORK_SIZE (sizeof(MV_RAIL_DIR_RND_WORK))		///<MV_RAIL_DIR_RNDサイズ

//-------------------------------------
///	MV_RAIL_RND_WORK構造体
//=====================================
typedef struct 
{
  MV_RAIL_COMMON_WORK rail_wk;
  u8 seq_no:4;
  u8 tbl_id:4;
  s8 wait;
  s8 init_width;
  u8 init_line;
} MV_RAIL_RND_WORK;
#define MV_RAIL_RND_WORK_SIZE (sizeof(MV_RAIL_RND_WORK))		///<MV_RAIL_RNDサイズ


//-------------------------------------
///	MV_RAIL_RT2_WORK構造体
//=====================================
typedef struct 
{
  MV_RAIL_COMMON_WORK rail_wk;
  u8 seq_no;
  u8 turn_flag;
  s8 init_width;
  u8 init_line;
} MV_RAIL_RT2_WORK;
#define MV_RAIL_RT2_WORK_SIZE (sizeof(MV_RAIL_RT2_WORK))		///<MV_RAIL_RTサイズ

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 基本関数
static RAIL_KEY MMdl_ConvertDir_RailKey( u16 dir );
static RAIL_FRAME MMdl_ConvertWait_RailFrame( s16 wait );

static void* MMdl_RailDefaultInit( MMDL* fmmdl, u32 work_size );

static BOOL MMdl_CheckRailLimit( const MMDL* fmmdl, u8 init_line, s8 init_width, const RAIL_LOCATION* cp_location );

// RNDテーブル
static int TblNumGet( const int *tbl, int end );
static int TblRndGet( const int *tbl, int end );
static int TblIDRndGet( int id, int end );
static const int * MoveDirTblIDSearch( int id );

// COMMON
static void MMdl_RailCommon_Init( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl );
static void MMdl_RailCommon_Move( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl );
static void MMdl_RailCommon_Delete( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl );
static void MMdl_RailCommon_Return( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl );

static void MMdl_RailCommon_SetSaveLocation( MV_RAIL_COMMON_WORK* p_work, const RAIL_LOCATION* location );
static void MMdl_RailCommon_ReflectSaveLocation( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl );



// DIR_RND
static void DirRndWorkInit( MMDL * fmmdl, int id );
static void DirRndWorkMove( MMDL * fmmdl );
static void DirRndWorkDelete( MMDL * fmmdl );


// MV_RND
static void MvRndWorkInit( MMDL * fmmdl, int id );
static void MvRndWorkMove( MMDL * fmmdl );
static void MvRndWorkDelete( MMDL * fmmdl );


// MV_RT2
static void MvRoot2WorkInit( MMDL * fmmdl );
static void MvRoot2WorkMove( MMDL * fmmdl );
static void MvRoot2WorkDelete( MMDL * fmmdl );

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
  const FLDNOGRID_MAPPER* cp_mapper = MMDLSYS_GetNOGRIDMapper( MMDL_GetMMdlSys( fmmdl ) );
  const FIELD_RAIL_MAN* cp_railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE ) );

	cp_work = MMDL_GetMoveProcWork( (MMDL*)fmmdl );

  if( FIELD_RAIL_WORK_IsAction( cp_work->rail_wk ) )
  {
    // 移動先を返す
    FIELD_RAIL_WORK_GetLocation( cp_work->rail_wk, location );
    FIELD_RAIL_MAN_CalcRailKeyLocation( cp_railman, location, FIELD_RAIL_WORK_GetActionKey( cp_work->rail_wk ), location );
  }
  else
  {
    // 今を返す
    FIELD_RAIL_WORK_GetLocation( cp_work->rail_wk, location );
  }
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
  FIELD_RAIL_WORK_GetLocation( cp_work->rail_wk, location );
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
 *	@brief  レールのアップデート処理を行う
 *
 *	@param	fmmdl   ワーク
 */
//-----------------------------------------------------------------------------
void MMDL_UpdateRail( MMDL* fmmdl )
{
  MV_RAIL_COMMON_WORK* p_work;

  // レール動作チェック
  GF_ASSERT( MMDL_CheckStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE ) );

	p_work = MMDL_GetMoveProcWork( (MMDL*)fmmdl );
  FIELD_RAIL_WORK_Update( p_work->rail_wk );

  // ロケーションのセーブ
  MMdl_RailCommon_Move( p_work, fmmdl );
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

  // now_locationのレールがあるか？
  if( FIELD_RAIL_WORK_CheckLocation( cp_work->rail_wk, now_location ) == FALSE )
  {
    TOMOYA_Printf( "CHECK NOW LOCATION MOVE LIMIT\n" );
    ret |= MMDL_MOVEHITBIT_LIM;
  }


  // next_locationのレールがあるか？
  if( FIELD_RAIL_WORK_CheckLocation( cp_work->rail_wk, next_location ) == FALSE )
  {
    TOMOYA_Printf( "CHECK NEXT LOCATION MOVE LIMIT\n" );
    ret |= MMDL_MOVEHITBIT_LIM;
  }

  // アトリビュートチェック
	if( MMDL_CheckStatusBitAttrGetOFF(mmdl) == FALSE )
  {
    MAPATTR attr;
    MAPATTR_FLAG attr_flag;

    attr = FLDNOGRID_MAPPER_GetAttr( p_mapper, next_location );
    attr_flag = MAPATTR_GetAttrFlag( attr );

    if( attr_flag & MAPATTR_FLAGBIT_HITCH )
    {
      TOMOYA_Printf( "HIT ATTR\n" );
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
  RAIL_LOCATION mdl_location;
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
        if( !MMDL_CheckStatusBit( cmdl,MMDL_STABIT_RAIL_MOVE) == 0 )
        {
          // NOW
          MMDL_GetRailLocation( cmdl, &mdl_location );
          FIELD_RAIL_MAN_GetLocationPosition( cp_railman, &mdl_location, &mdl_pos );

          //TOMOYA_Printf( "now grid_r 0x%x my_pos x[0x%x]y[0x%x]z[0x%x] mdl_pos x[0x%x]y[0x%x]z[0x%x] \n", grid_r, my_pos.x, my_pos.y, my_pos.z, mdl_pos.x, mdl_pos.y, mdl_pos.z );
          //TOMOYA_Printf( "mdl_location line%d width%d\n", mdl_location.line_grid, mdl_location.width_grid );
          if( FIELD_RAIL_TOOL_HitCheckSphere( &my_pos, &mdl_pos, grid_r ) )
          {
            return TRUE;
          }

          // OLD
          MMDL_GetOldRailLocation( cmdl, &mdl_location );
          FIELD_RAIL_MAN_GetLocationPosition( cp_railman, &mdl_location, &mdl_pos );

          //TOMOYA_Printf( "last grid_r 0x%x my_pos x[0x%x]y[0x%x]z[0x%x] mdl_pos x[0x%x]y[0x%x]z[0x%x] \n", grid_r, my_pos.x, my_pos.y, my_pos.z, mdl_pos.x, mdl_pos.y, mdl_pos.z );
          //TOMOYA_Printf( "mdl_location line%d width%d\n", mdl_location.line_grid, mdl_location.width_grid );
          if( FIELD_RAIL_TOOL_HitCheckSphere( &my_pos, &mdl_pos, grid_r ) )
          {
            return TRUE;
          }
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
  RAIL_LOCATION mdl_location;
  fx32 grid_r;
  

  FIELD_RAIL_MAN_GetLocationPosition( cp_railman, location, &check_pos );
  grid_r = FIELD_RAIL_MAN_GetRailGridSize( cp_railman );
  grid_r = MMDL_GET_RAILGRID_R( grid_r );

	while( MMDLSYS_SearchUseMMdl(sys,&mmdl,&no) == TRUE ){

    if( !MMDL_CheckStatusBit( mmdl,MMDL_STABIT_RAIL_MOVE) == 0 )
    {

      if( old_hit ){
        MMDL_GetOldRailLocation( mmdl, &old_location );
        FIELD_RAIL_MAN_GetLocationPosition( cp_railman, &old_location, &mdl_pos );

        if( FIELD_RAIL_TOOL_HitCheckSphere( &check_pos, &mdl_pos, grid_r ) ){
          return( mmdl );
        }
      }
      
      MMDL_GetRailLocation( mmdl, &mdl_location );
      FIELD_RAIL_MAN_GetLocationPosition( cp_railman, &mdl_location, &mdl_pos );
      if( FIELD_RAIL_TOOL_HitCheckSphere( &check_pos, &mdl_pos, grid_r ) ){
        return( mmdl );
      }
    }
	}
	
	return( NULL );
}

//----------------------------------------------------------------------------
/**
 *	@brief  目の前のロケーションを取得
 *
 *	@param	mmdl
 *	@param	location 
 *
 *	@retval TRUE  目の前がある
 *	@retval FALSE 目にはいけない
 */
//-----------------------------------------------------------------------------
BOOL MMDL_GetRailFrontLocation( const MMDL *mmdl, RAIL_LOCATION* location )
{
  const MMDLSYS* cp_sys = MMDL_GetMMdlSys( mmdl );
  const FLDNOGRID_MAPPER* cp_mapper = MMDLSYS_GetNOGRIDMapper( cp_sys );
  const FIELD_RAIL_MAN* cp_railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );
  u16 dir;

  dir = MMDL_GetDirDisp( mmdl );


  MMDL_GetRailLocation( mmdl, location );
//  TOMOYA_Printf( "dir %d  line_grid %d\n", dir, location->line_grid );
  return FIELD_RAIL_MAN_CalcRailKeyLocation( cp_railman, location, FIELD_RAIL_TOOL_ConvertDirToRailKey(dir), location ); 
}

//----------------------------------------------------------------------------
/**
 *	@brief  dir方向のロケーションを取得
 *
 *	@param	mmdl        モデル
 *	@param	dir         方向
 *	@param	location    レールロケーション格納先
 *
 *	@retval TRUE  移動可能
 *	@retval FALSE 移動不可能
 */
//-----------------------------------------------------------------------------
BOOL MMDL_GetRailDirLocation( const MMDL *mmdl, u16 dir, RAIL_LOCATION* location )
{
  const MMDLSYS* cp_sys = MMDL_GetMMdlSys( mmdl );
  const FLDNOGRID_MAPPER* cp_mapper = MMDLSYS_GetNOGRIDMapper( cp_sys );
  const FIELD_RAIL_MAN* cp_railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );

  MMDL_GetRailLocation( mmdl, location );
//  TOMOYA_Printf( "dir %d  line_grid %d\n", dir, location->line_grid );
  return FIELD_RAIL_MAN_CalcRailKeyLocation( cp_railman, location, FIELD_RAIL_TOOL_ConvertDirToRailKey(dir), location ); 
}

//----------------------------------------------------------------------------
/**
 *	@brief  dir方向にレールで進んだ場合のグリッド座標を設定する
 *
 *	@param	mmdl    モデルワーク
 *	@param	dir     方向
 */
//-----------------------------------------------------------------------------
void MMDL_Rail_UpdateGridPosDir( MMDL *mmdl, u16 dir )
{
  const MMDLSYS* cp_sys = MMDL_GetMMdlSys( mmdl );
  const FLDNOGRID_MAPPER* cp_mapper = MMDLSYS_GetNOGRIDMapper( cp_sys );
  const FIELD_RAIL_MAN* cp_railman = FLDNOGRID_MAPPER_GetRailMan( cp_mapper );
  VecFx32 pos;
  RAIL_LOCATION location;

  MMDL_GetRailLocation( mmdl, &location );
  FIELD_RAIL_MAN_CalcRailKeyPos( cp_railman, &location, FIELD_RAIL_TOOL_ConvertDirToRailKey(dir), &pos ); 
  
  // 3D座標をグリッドに設定する
	MMDL_SetOldGridPosX( mmdl, MMDL_GetGridPosX(mmdl) );
	MMDL_SetOldGridPosY( mmdl, MMDL_GetGridPosY(mmdl) );
	MMDL_SetOldGridPosZ( mmdl, MMDL_GetGridPosZ(mmdl) );
	
	MMDL_SetGridPosX( mmdl, SIZE_GRID_FX32(pos.x) );
	MMDL_SetGridPosY( mmdl, SIZE_GRID_FX32(pos.y) );
	MMDL_SetGridPosZ( mmdl, SIZE_GRID_FX32(pos.z) );
}


//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤー進行方向を取得する
 *	        
 *
 *	@param  mmdl    モデルワーク
 *	@param	way     方向
 */
//-----------------------------------------------------------------------------
void MMDL_Rail_GetFrontWay( const MMDL *mmdl, VecFx16* way )
{
  FIELD_RAIL_WORK* p_railwk;
  int dir;
  
  GF_ASSERT( mmdl );
  GF_ASSERT( way );

  p_railwk = MMDL_GetRailWork( mmdl );

  dir = MMDL_GetDirMove( mmdl );
  MMDL_Rail_GetDirLineWay( mmdl, dir, way );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ラインのdir方向の取得
 *
 *	@param	mmdl    モデルワーク
 *	@param	dir     モデルの向き情報
 *	@param	way     方向
 */
//-----------------------------------------------------------------------------
void MMDL_Rail_GetDirLineWay( const MMDL *mmdl, u16 dir, VecFx16* way )
{
  FIELD_RAIL_WORK* p_railwk;
  int rail_key;
  u16 rail_dir;
  u16 rot;
  MtxFx33 mtx; 
  VecFx32 calc_vec;
  
  GF_ASSERT( mmdl );
  GF_ASSERT( way );
  GF_ASSERT( dir < DIR_MAX4 );

  p_railwk = MMDL_GetRailWork( mmdl );
  FIELD_RAIL_WORK_GetFrontWay( p_railwk, way );

  // プレイヤーの方向とライン前方方向を使い、角度を変換する
  rail_key = FIELD_RAIL_WORK_GetFrontKey( p_railwk );
  GF_ASSERT( rail_key != RAIL_KEY_NULL );

  // レール前方方向の情報から、wayの回転角度を求める
  rail_dir  = sc_RAILLINE_DIR_CHANGE_TBL[ rail_key ][ dir ];
  rot       = sc_RAILLINE_DIR_ROT_Y[ rail_dir ];

  // XZのパラメータだけ回転させる
  VEC_Set( &calc_vec, way->x, way->y, way->z );
  MTX_RotY33( &mtx, FX_SinIdx( rot ), FX_CosIdx( rot ) );
  MTX_MultVec33( &calc_vec, &mtx, &calc_vec );
  VEC_Normalize( &calc_vec, &calc_vec );
  VEC_Fx16Set( way, calc_vec.x, calc_vec.y, calc_vec.z );
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
	p_work = MMdl_RailDefaultInit( fmmdl, MV_RAIL_DMY_WORK_SIZE );
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




//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　全方向ランダム  初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_ALL_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　UP　LEFT　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_UL_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_UL );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　UP　RIGHT　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_UR_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_UR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　DOWN　LEFT　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_DL_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_DL );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　DOWN　RIGHT　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_DR_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_DR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　UP　DOWN　LEFT　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_UDL_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_UDL );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　UP　DOWN　RIGHT　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_UDR_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_UDR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　UP　LEFT　RIGHT　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_ULR_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_ULR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　DOWN　LEFT　RIGHT　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_DLR_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_DLR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　UP　DOWN　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_UD_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_UD );
}

//----------------------------------------------------------------------------
/**
 *	@brief  振り向き　LEFT　RIGHT　方向ランダム　初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_LR_Init( MMDL * fmmdl )
{
	DirRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_LR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  方向ランダム　メイン
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_Move( MMDL * fmmdl )
{
  DirRndWorkMove( fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  方向ランダム　破棄
 */
//-----------------------------------------------------------------------------
void MMDL_RailDirRnd_Delete( MMDL * fmmdl )
{
  DirRndWorkDelete( fmmdl );
}



//----------------------------------------------------------------------------
/**
 *	@brief  移動　全方向ランダム  初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailRnd_ALL_Init( MMDL * fmmdl )
{
  MvRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動　縦方向ランダム  初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailRnd_V_Init( MMDL * fmmdl )
{
  MvRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_UD );
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動　横方向ランダム  初期化
 */
//-----------------------------------------------------------------------------
void MMDL_RailRnd_H_Init( MMDL * fmmdl )
{
  MvRndWorkInit( fmmdl, DIRID_MvDirRndDirTbl_LR );
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動　ランダム  メイン
 */
//-----------------------------------------------------------------------------
void MMDL_RailRnd_Move( MMDL * fmmdl )
{
  MvRndWorkMove( fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動　ランダム　破棄
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
void MMDL_RailRnd_Delete( MMDL * fmmdl )
{
  MvRndWorkDelete( fmmdl );
}



//----------------------------------------------------------------------------
/**
 *	@brief  ２分岐ルート移動　初期化
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
void MMDL_RailRoot2_Init( MMDL * fmmdl )
{
  MvRoot2WorkInit( fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ２分岐ルート移動　動作
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
void MMDL_RailRoot2_Move( MMDL * fmmdl )
{
  MvRoot2WorkMove( fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ２分岐ルート移動　破棄
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
void MMDL_RailRoot2_Delete( MMDL * fmmdl )
{
  MvRoot2WorkDelete( fmmdl );
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
  return FIELD_RAIL_TOOL_ConvertDirToRailKey(dir);
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


//----------------------------------------------------------------------------
/**
 *	@brief  基本初期か処理
 *
 *	@param	fmmdl       モデル
 *	@param	work_size   ワークサイズ
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static void* MMdl_RailDefaultInit( MMDL* fmmdl, u32 work_size )
{
  MV_RAIL_COMMON_WORK* p_work;
  RAIL_LOCATION location;
  BOOL setup;

  setup = MMdl_GetSetUpLocation( fmmdl, &location );
  
	p_work = MMDL_InitMoveProcWork( fmmdl, work_size );
  MMdl_RailCommon_Init( p_work, fmmdl );

  if( setup )
  {
    MMDL_SetRailLocation( fmmdl, &location );
  }

  return p_work;
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールリミットチェック
 *
 *	@param	fmmdl       モデル
 *	@param	init_line   ライングリッド初期値
 *	@param	init_width  幅グリッド初期値
 *	@param  cp_location チェックロケーション
 *
 *	@retval TRUE  リミットヒット
 *	@retval FALSE リミットしない
 */
//-----------------------------------------------------------------------------
static BOOL MMdl_CheckRailLimit( const MMDL* fmmdl, u8 init_line, s8 init_width, const RAIL_LOCATION* cp_location )
{
  int limit_x, limit_z, max, min;
  
  limit_x = MMDL_GetMoveLimitX( fmmdl );  // Width
  limit_z = MMDL_GetMoveLimitZ( fmmdl );  // Line

  // 移動リミットチェック
  if( limit_x != MOVE_LIMIT_NOT )
  {
    min = init_width - limit_x;
    max = init_width + limit_x;
    if( (min > cp_location->width_grid) || (max < cp_location->width_grid) )
    {
      return TRUE;
    }
  }

  if( limit_z != MOVE_LIMIT_NOT )
  {
    min = init_line - limit_z;
    max = init_line + limit_z;
    if( (min > cp_location->line_grid) || (max < cp_location->line_grid) )
    {
      return TRUE;
    }
  }

  return FALSE;
}


//--------------------------------------------------------------
/**
 * 方向テーブル要素数取得
 * @param	tbl		データテーブル。終端にend
 * @param	end		テーブル終端とする値
 * @retval	int		tbl要素数
 */
//--------------------------------------------------------------
static int TblNumGet( const int *tbl, int end )
{
	int i=0; while(tbl[i]!=end){i++;} GF_ASSERT(i&&"TblNumGet()要素数0"); return(i);
}

//--------------------------------------------------------------
/**
 * テーブルデータからランダムで値取得
 * @param	tbl		データテーブル。終端にend
 * @param	end		テーブル終端とする値
 * @retval	int		tbl内部の値
 */
//--------------------------------------------------------------
static int TblRndGet( const int *tbl, int end )
{
#if 0
	return( tbl[gf_rand()%TblNumGet(tbl,end)] );
#else
	return( tbl[GFUser_GetPublicRand(TblNumGet(tbl,end))] );
#endif
}

//--------------------------------------------------------------
/**
 * 方向IDからテーブルデータを取得しランダムで値取得
 * @param	tbl		データテーブル。終端にend
 * @param	end		テーブル終端とする値
 * @retval	int		tbl内部の値
 */
//--------------------------------------------------------------
static int TblIDRndGet( int id, int end )
{
	const int *tbl = MoveDirTblIDSearch( id );
	return( tbl[GFUser_GetPublicRand(TblNumGet(tbl,end))] );
}

//--------------------------------------------------------------
/**
 * DATA_MoveDirTbl検索
 * @param	id		検索ID
 * @retval	int		ヒットしたテーブル*
 */
//--------------------------------------------------------------
static const int * MoveDirTblIDSearch( int id )
{
	const DIR_TBL *tbl = DATA_MoveDirTbl;
	
	while( tbl->id != DIRID_END ){
		if( tbl->id == id ){ return( tbl->tbl ); }
		tbl++;
	}
	
	GF_ASSERT( 0 && "MoveDirTblIDSearch()ID異常" );
	return( NULL );
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
  p_work->location.type = FIELD_RAIL_TYPE_MAX; // 設定データなし
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
  RAIL_LOCATION location;
  // 位置情報の保存
  FIELD_RAIL_WORK_GetLocation( p_work->rail_wk, &location );
//  TOMOYA_Printf( "location index %d  line %d  side %d\n", location.rail_index, location.line_grid, location.width_grid );
  MMdl_RailCommon_SetSaveLocation( p_work, &location );
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

  // レールのフラグを破棄
  MMDL_OffStatusBit( fmmdl, MMDL_STABIT_RAIL_MOVE );
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

  // セーブロケーションの反映
  MMdl_RailCommon_ReflectSaveLocation( p_work, fmmdl );
}



//----------------------------------------------------------------------------
/**
 *	@brief  セーブロケーションの設定
 *
 *	@param	p_work
 *	@param	location 
 */
//-----------------------------------------------------------------------------
static void MMdl_RailCommon_SetSaveLocation( MV_RAIL_COMMON_WORK* p_work, const RAIL_LOCATION* location )
{
  p_work->location = *location;
}

//----------------------------------------------------------------------------
/**
 *	@brief  セーブロケーションの反映
 *
 *	@param	p_work
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
static void MMdl_RailCommon_ReflectSaveLocation( MV_RAIL_COMMON_WORK* p_work, MMDL * fmmdl )
{
  if( p_work->location.type != FIELD_RAIL_TYPE_MAX )
  {
    FIELD_RAIL_WORK_SetLocation( p_work->rail_wk, &p_work->location );
    p_work->location.type = FIELD_RAIL_TYPE_MAX;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  方向ランダムワーク　初期か
 *
 *	@param	fmmdl   モデル
 *	@param	id      方向テーブルID
 */
//-----------------------------------------------------------------------------
static void DirRndWorkInit( MMDL * fmmdl, int id )
{
  MV_RAIL_DIR_RND_WORK* p_wk;

  p_wk = MMdl_RailDefaultInit( fmmdl, MV_RAIL_DIR_RND_WORK_SIZE );

  p_wk->wait    = TblRndGet( DATA_MvDirRndWaitTbl, WAIT_END );			//待ち時間セット
  p_wk->tbl_id  = id;

	MMDL_SetDrawStatus( fmmdl, DRAW_STA_STOP );					//描画ステータス　停止
	MMDL_OffMoveBitMove( fmmdl );								//常に停止中
}

//----------------------------------------------------------------------------
/**
 *	@brief  方向ランダムワーク　メイン
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
static void DirRndWorkMove( MMDL * fmmdl )
{
	MV_RAIL_DIR_RND_WORK* p_wk = MMDL_GetMoveProcWork( fmmdl );
	
  p_wk->wait--;

  if( p_wk->wait <= 0 )
  {									//待ち時間 0
    p_wk->wait = TblRndGet( DATA_MvDirRndWaitTbl, WAIT_END );
    MMDL_SetDirDisp( fmmdl, TblIDRndGet(p_wk->tbl_id,DIR_NOT) );
  }

  MMdl_RailCommon_Move( &p_wk->rail_wk, fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  方向ランダムワーク　破棄
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
static void DirRndWorkDelete( MMDL * fmmdl )
{
	MV_RAIL_DIR_RND_WORK* p_wk = MMDL_GetMoveProcWork( fmmdl );
  MMdl_RailCommon_Delete( &p_wk->rail_wk, fmmdl );
}



//----------------------------------------------------------------------------
/**
 *	@brief  ランダム移動　初期化
 *
 *	@param	fmmdl   
 *	@param	id            方向テーブルID
 */
//-----------------------------------------------------------------------------
static void MvRndWorkInit( MMDL * fmmdl, int id )
{
  MV_RAIL_RND_WORK* p_wk;
  RAIL_LOCATION location;

  p_wk = MMdl_RailDefaultInit( fmmdl, MV_RAIL_RND_WORK_SIZE );

  MMDL_GetRailLocation( fmmdl, &location );

  p_wk->seq_no = 0;
  p_wk->wait = 0;
  p_wk->tbl_id = id;

  // ワークサイズをオーバーしないようにチェック
  GF_ASSERT( location.line_grid < 0xff );
  GF_ASSERT( location.width_grid < 0x7f );

  p_wk->init_line   = location.line_grid;
  p_wk->init_width  = location.width_grid;

	MMDL_SetDrawStatus( fmmdl, DRAW_STA_STOP );
	MMDL_OffMoveBitMove( fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ランダム移動　メイン
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
static void MvRndWorkMove( MMDL * fmmdl )
{
	MV_RAIL_RND_WORK* p_wk = MMDL_GetMoveProcWork( fmmdl );
  u32 ret;
	
	switch( p_wk->seq_no ){
	case MV_RND_MOVE_SEQ_ACT:
		MMDL_OffMoveBitMove( fmmdl );
		MMDL_OffMoveBitMoveEnd( fmmdl );
		
		ret = MMDL_GetDirDisp( fmmdl );
		ret = MMDL_ChangeDirAcmdCode( ret, AC_DIR_U );
		MMDL_SetLocalAcmd( fmmdl, ret );
		
		p_wk->seq_no++;
		break;
	case MV_RND_MOVE_SEQ_ACT_WAIT:
		if( MMDL_ActionLocalAcmd(fmmdl) == FALSE ){
			break;
		}
		
		p_wk->wait = TblRndGet( DATA_MvDirRndWaitTbl, WAIT_END );
		p_wk->seq_no++;
	case MV_RND_MOVE_SEQ_TIME_WAIT:
		p_wk->wait--;
		
		if( p_wk->wait ){
			break;
		}
		
		p_wk->seq_no++;
	case MV_RND_MOVE_SEQ_WALK_ACT:
		ret = TblIDRndGet( p_wk->tbl_id, DIR_NOT );
		MMDL_SetDirAll( fmmdl, ret );

    // ヒットチェック
		{
			u32 hit = MMDL_HitCheckRailMoveDir( fmmdl, ret );
			
			if( hit != MMDL_MOVEHITBIT_NON ){
        p_wk->seq_no = 0;
        break;
			}
		}

    // ライン乗り換えチェック
    // 移動リミットチェック
    {
      RAIL_LOCATION now_location;
      RAIL_LOCATION next_location;

      MMDL_GetRailLocation( fmmdl, &now_location );
      MMDL_GetRailDirLocation( fmmdl, ret, &next_location );

      // ライン乗り換えチェック
      if( now_location.rail_index != next_location.rail_index )
      {
        p_wk->seq_no = 0;
        break;
      }


      // 移動リミットチェック
      if( MMdl_CheckRailLimit( fmmdl, p_wk->init_line, p_wk->init_width, &next_location ) )
      {
        p_wk->seq_no = 0;
        break;
      }
    }
		
		ret = MMDL_ChangeDirAcmdCode( ret, AC_WALK_U_8F );
		MMDL_SetLocalAcmd( fmmdl, ret );
			
		MMDL_OnMoveBitMove( fmmdl );
		p_wk->seq_no++;
	case MV_RND_MOVE_SEQ_WALK_ACT_WAIT:
		if( MMDL_ActionLocalAcmd(fmmdl) == FALSE ){
			break;
		}
		
		MMDL_OffMoveBitMove( fmmdl );
		p_wk->seq_no = 0;
    break;

  default:
    GF_ASSERT(0);
    break;
	}

  MMdl_RailCommon_Move( &p_wk->rail_wk, fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ランダム移動　破棄
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
static void MvRndWorkDelete( MMDL * fmmdl )
{ 
	MV_RAIL_RND_WORK* p_wk = MMDL_GetMoveProcWork( fmmdl );
  MMdl_RailCommon_Delete( &p_wk->rail_wk, fmmdl );
}



//----------------------------------------------------------------------------
/**
 *	@brief  ルート移動  初期化
 *    
 *	@param	fmmdl
 *	@param	id        テーブルID
 */
//-----------------------------------------------------------------------------
static void MvRoot2WorkInit( MMDL * fmmdl )
{
  MV_RAIL_RT2_WORK* p_wk;
  RAIL_LOCATION location;

  p_wk = MMdl_RailDefaultInit( fmmdl, MV_RAIL_RT2_WORK_SIZE );

  MMDL_GetRailLocation( fmmdl, &location );

  p_wk->seq_no = 0;
  p_wk->turn_flag = 0;

  // ワークサイズをオーバーしないようにチェック
  GF_ASSERT( location.line_grid < 0xff );
  GF_ASSERT( location.width_grid < 0x7f );

  p_wk->init_line   = location.line_grid;
  p_wk->init_width  = location.width_grid;

	MMDL_SetDrawStatus( fmmdl, DRAW_STA_STOP );
	MMDL_OffMoveBitMove( fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ルート移動　メイン
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
static void MvRoot2WorkMove( MMDL * fmmdl )
{
	MV_RAIL_RT2_WORK* p_wk = MMDL_GetMoveProcWork( fmmdl );

  switch( p_wk->seq_no )
  {
  ///<初期方向チェック
  case MV_RT2_MOVE_SEQ_CHECK_DIR:      
    {
      int dir;
      
      dir = MMDL_GetDirHeader( fmmdl );						//ヘッダ指定方向

      if( p_wk->turn_flag == TRUE ){
        dir = MMDL_TOOL_FlipDir( dir );
      }
      
      MMDL_SetDirAll( fmmdl, dir );

      p_wk->seq_no = MV_RT2_MOVE_SEQ_ACT_SET;
    }
    ///↓へ
    
  ///<動作設定
  case MV_RT2_MOVE_SEQ_ACT_SET:        
    {
      RAIL_LOCATION now_location;
      RAIL_LOCATION next_location;

      // 現状のロケーション取得
      MMDL_GetRailLocation( fmmdl, &now_location );
      
      // 反転中　戻り中なら、元に戻すかチェック
      if( p_wk->turn_flag ){
        
        if( (p_wk->init_line == now_location.line_grid) && 
            (p_wk->init_width == now_location.width_grid) )
        {
          int dir = MMDL_TOOL_FlipDir( MMDL_GetDirMove(fmmdl) );
          
          MMDL_SetDirAll( fmmdl, dir );

          p_wk->turn_flag = FALSE;
        }
      }

      // 移動設定
      {
        int dir, acmd;
        u32 ret;
        BOOL limit;
        BOOL line_change = FALSE;

        dir = MMDL_GetDirMove( fmmdl );

        ret = MMDL_HitCheckRailMoveDir( fmmdl, dir );

        // 移動先ロケーション取得
        MMDL_GetRailDirLocation( fmmdl, dir, &next_location );
        
        // レールリミットも計算
        limit = MMdl_CheckRailLimit( fmmdl, p_wk->init_line, p_wk->init_width, &next_location );

        // ラインが変わらないようにチェック
        if( now_location.rail_index != next_location.rail_index )
        {
          line_change = TRUE;
        }

        // リミットで反転
        if( (ret & MMDL_MOVEHITBIT_LIM) || limit || line_change )
        {
          p_wk->turn_flag = TRUE;
          dir = MMDL_TOOL_FlipDir( dir );
          ret = MMDL_HitCheckRailMoveDir( fmmdl, dir );
          MMDL_GetRailDirLocation( fmmdl, dir, &next_location );
        }

        acmd = AC_WALK_U_8F;
        if( ret != MMDL_MOVEHITBIT_NON )
        {
          acmd = AC_STAY_WALK_U_8F;
        }


        acmd = MMDL_ChangeDirAcmdCode( dir, acmd );
        MMDL_SetLocalAcmd( fmmdl, acmd );
      }

      MMDL_OnMoveBitMove( fmmdl );
      p_wk->seq_no = MV_RT2_MOVE_SEQ_ACT_WAIT;
    }
    break;
  ///<動作待ち
  case MV_RT2_MOVE_SEQ_ACT_WAIT:       
    if( MMDL_ActionLocalAcmd(fmmdl) == TRUE ){
      MMDL_OffMoveBitMove( fmmdl );
      
      p_wk->seq_no = MV_RT2_MOVE_SEQ_CHECK_DIR;
    }
    break;

  // エラー
  default:
    GF_ASSERT(0);
    break;
  }
	
  MMdl_RailCommon_Move( &p_wk->rail_wk, fmmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ルート移動　破棄
 *
 *	@param	fmmdl 
 */
//-----------------------------------------------------------------------------
static void MvRoot2WorkDelete( MMDL * fmmdl )
{
	MV_RAIL_RT2_WORK* p_wk = MMDL_GetMoveProcWork( fmmdl );
  MMdl_RailCommon_Delete( &p_wk->rail_wk, fmmdl );
}




