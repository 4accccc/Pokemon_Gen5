//============================================================================================
/**
 * @file  field_rail.c
 * @brief ノーグリッドでの自機・カメラ制御構造
 * @author  tamada, tomoya
 * @date  2009.05.18
 *
 */
//============================================================================================

#include <gflib.h>

#include "field/fieldmap_proc.h"

#include "field_rail.h"
#include "field_rail_access.h"

#include "field_camera.h"

//============================================================================================
//============================================================================================
//============================================================================================
//
//
//
//    レール表現を実現するためのツール
//
//
//
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
static const RAIL_LINEPOS_SET RAIL_LINEPOS_SET_Default =
{
  RAIL_TBL_NULL,
  RAIL_TBL_NULL,
  0,
  0,
  0,
  0,
};


//============================================================================================
//
//
//      カメラ指定のためのツール
//
//
//============================================================================================
//-----------------------------------------------------------------
//------------------------------------------------------------------
static const RAIL_CAMERA_SET RAIL_CAMERA_SET_Default =
{
  RAIL_TBL_NULL,
  0,
  0,
  0,
  0,
};
//------------------------------------------------------------------
//------------------------------------------------------------------

//------------------------------------------------------------------
/**
 * @brief	レール情報
 */
//------------------------------------------------------------------
typedef struct 
{
  const RAIL_POINT * point_table;
  const RAIL_LINE * line_table;
	const RAIL_CAMERA_SET*	camera_table;
	const RAIL_LINEPOS_SET* linepos_table;
	RAIL_CAMERA_FUNC*const*	camera_func;
	RAIL_POS_FUNC*const*		line_pos_func;
	RAIL_LINE_DIST_FUNC*const*		line_dist_func;
  u16 point_count;
  u16 line_count;
  u16 camera_count;
  u16 linepos_count;
  u16 camera_func_count;
  u16 linepos_func_count;
  u16 line_dist_func_count;
	u16 dummy00;
}RAIL_DAT;

//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
struct _FIELD_RAIL{  
  FIELD_RAIL_TYPE type;
  union { 
    void * dummy;
    const RAIL_POINT * point;
    const RAIL_LINE * line;
  };
	u32 dat_index;	// POINT or LINEのインデックス
  /// LINEにいる間の、LINE上でのオフセット位置
  s32 line_ofs;
  s32 line_ofs_max;
  /// LINEにいる間の幅オフセット位置(XZ平面上でLINEに垂直)
  s32 width_ofs;
  s32 width_ofs_max;  // 水平方向分割数
  fx32 ofs_unit;       // 1分割の距離

  RAIL_KEY key;

	/// レールデータ
	const RAIL_DAT* rail_dat;
};

//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
struct _FIELD_RAIL_MAN{
  HEAPID heapID;

  FIELD_CAMERA * field_camera;

  BOOL active_flag;

  /// 移動が起きた最新のキーバリュー
  int last_active_key;

  FIELD_RAIL now_rail;

  /// ライン、ポイントテーブル
	RAIL_DAT rail_dat;
};

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY keyContToRailKey(int cont);
static RAIL_KEY getReverseKey(RAIL_KEY key);
static RAIL_KEY getClockwiseKey(RAIL_KEY key);
static RAIL_KEY getAntiClockwiseKey(RAIL_KEY key);

//------------------------------------------------------------------
//------------------------------------------------------------------
static void initRail(FIELD_RAIL * rail, const RAIL_DAT* rail_dat );
static BOOL isValidRail(const FIELD_RAIL * rail);
static const RAIL_CAMERA_SET * getCameraSet(const FIELD_RAIL * rail);
static const char * getRailName(const FIELD_RAIL * rail);
static void getRailPosition(const FIELD_RAIL * rail, VecFx32 * pos);
static s32 getLineOfsMax( const RAIL_LINE * line, fx32 unit, const RAIL_DAT* rail_dat );
static s32 getLineWidthOfsMax( const RAIL_LINE * line, u32 line_ofs, u32 line_ofs_max, const RAIL_DAT* rail_dat );
static RAIL_KEY updateLine( FIELD_RAIL * rail, const RAIL_LINE * line, u32 line_ofs_max, u32 key );
static RAIL_KEY setLine(FIELD_RAIL * rail, const RAIL_LINE * line, RAIL_KEY key, int l_ofs, int w_ofs, int l_ofs_max);
static void setLineData(FIELD_RAIL * rail, const RAIL_LINE * line, RAIL_KEY key, int l_ofs, int w_ofs, int l_ofs_max);
static BOOL isLinePoint_S( const RAIL_DAT * rail_dat, const RAIL_LINE * line, const RAIL_POINT * point );
static BOOL isLinePoint_E( const RAIL_DAT * rail_dat, const RAIL_LINE * line, const RAIL_POINT * point );

//------------------------------------------------------------------
//------------------------------------------------------------------
static void initRailDat( RAIL_DAT * raildat, const RAIL_SETTING * setting );
static const RAIL_POINT* getRailDatPoint( const RAIL_DAT * raildat, u32 index );
static const RAIL_LINE* getRailDatLine( const RAIL_DAT * raildat, u32 index );
static const RAIL_CAMERA_SET* getRailDatCamera( const RAIL_DAT * raildat, u32 index );
static const RAIL_LINEPOS_SET* getRailDatLinePos( const RAIL_DAT * raildat, u32 index );
static RAIL_CAMERA_FUNC*const getRailDatCameraFunc( const RAIL_DAT * raildat, u32 index );
static RAIL_POS_FUNC*const getRailDatLinePosFunc( const RAIL_DAT * raildat, u32 index );
static RAIL_LINE_DIST_FUNC*const getRailDatLineDistFunc( const RAIL_DAT * raildat, u32 index );

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL debugCheckLineData(const RAIL_LINE * line, const RAIL_DAT* rail_dat);
static BOOL debugCheckPointData(const RAIL_POINT * point, const RAIL_DAT* rail_dat);
static void debugPrintPoint(const char * before, const RAIL_POINT * point, const char * after);
static const char * debugGetRailKeyName(RAIL_KEY key);
static void debugPrintRailInfo(const FIELD_RAIL * rail);


static RAIL_KEY updateLineMove_new(FIELD_RAIL * rail, RAIL_KEY key, u32 count_up);
//static RAIL_KEY updateLineMove(FIELD_RAIL * rail, RAIL_KEY key);
static RAIL_KEY updatePointMove(FIELD_RAIL * rail, RAIL_KEY key, u32 count_up);


static u32 getPointIndex( const FIELD_RAIL_MAN * man, const RAIL_POINT* point );
static u32 getLineIndex( const FIELD_RAIL_MAN * man, const RAIL_LINE* line );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void RAIL_LOCATION_Dump(const RAIL_LOCATION * railLoc)
{
  TAMADA_Printf("RAIL_LOC:type = %d, rail_index = %d\n",railLoc->type, railLoc->rail_index);
  TAMADA_Printf("RAIL_LOC:line_ofs = %d, width_ofs = %d\n", railLoc->line_ofs, railLoc->width_ofs);
  TAMADA_Printf("RAIL_LOC:key = %s\n", debugGetRailKeyName(railLoc->key));
}

//------------------------------------------------------------------
/**
 * @brief RAIL_LOCATION構造体の初期化処理
 */
//------------------------------------------------------------------
void RAIL_LOCATION_Init(RAIL_LOCATION * railLoc)
{
  railLoc->type = FIELD_RAIL_TYPE_POINT;
  railLoc->rail_index = 0;
  railLoc->line_ofs = 0;
  railLoc->width_ofs = 0;
  railLoc->key = RAIL_KEY_NULL;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief レール制御システムの生成
 */
//------------------------------------------------------------------
FIELD_RAIL_MAN * FIELD_RAIL_MAN_Create(HEAPID heapID, FIELD_CAMERA * camera)
{ 
  FIELD_RAIL_MAN * man = GFL_HEAP_AllocMemory(heapID, sizeof(FIELD_RAIL_MAN));

  man->heapID = heapID;
  man->active_flag = FALSE;
  man->field_camera = camera;
  //man->line_ofs = 0;
  man->last_active_key = 0;
  initRail(&man->now_rail, &man->rail_dat);

  return man;
}

//------------------------------------------------------------------
/**
 * @brief レール制御システムの削除
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_Delete(FIELD_RAIL_MAN * man)
{ 
  GFL_HEAP_FreeMemory(man);
}

//------------------------------------------------------------------
/**
 * @brief レールデータの読み込み
 *
 * とりあえず、最初はPOINTから始まると仮定する
 *
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_Load(FIELD_RAIL_MAN * man, const RAIL_SETTING * setting)
{ 
  FIELD_RAIL * rail = &man->now_rail;
  rail->type    = FIELD_RAIL_TYPE_POINT;
  rail->point   = &setting->point_table[0];
  rail->line_ofs_max  = 0;
  rail->width_ofs_max = setting->point_table[0].width_ofs_max[0];
  rail->ofs_unit      = setting->ofs_unit;
  man->active_flag = TRUE;

  // ポイント、ラインテーブル設定
	initRailDat( &man->rail_dat, setting );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール位置情報の取得
 *
 *	@param	man       マネージャ
 *	@param	location  位置情報格納先
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_GetLocation(const FIELD_RAIL_MAN * man, RAIL_LOCATION * location)
{
  location->type        = man->now_rail.type;

  // インデックス取得
  if( man->now_rail.type==FIELD_RAIL_TYPE_POINT ){
    location->rail_index  = getPointIndex( man, man->now_rail.point );
  }else{
    location->rail_index  = getLineIndex( man, man->now_rail.line );
  }

  // 各種オフセット
  location->line_ofs  = man->now_rail.line_ofs;
  location->width_ofs = man->now_rail.width_ofs;
  location->key       = man->now_rail.key;

  RAIL_LOCATION_Dump(location);
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール位置情報の設定
 *
 *	@param	man       マネージャ
 *	@param	location  指定位置情報
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_SetLocation(FIELD_RAIL_MAN * man, const RAIL_LOCATION * location)
{
  FIELD_RAIL * rail = &man->now_rail;

  RAIL_LOCATION_Dump(location);
  // 初期化
  initRail( rail, &man->rail_dat );

  if( location->type==FIELD_RAIL_TYPE_POINT )
  {
    // 点初期化
    rail->type          = FIELD_RAIL_TYPE_POINT;
    rail->point         = &man->rail_dat.point_table[ location->rail_index ];
  }
  else
  {
    u32 line_ofs_max;
    const RAIL_LINE* line;
    
    // ライン初期化
    line                = &man->rail_dat.line_table[ location->rail_index ];
    line_ofs_max        = getLineOfsMax( line, rail->ofs_unit, &man->rail_dat );

    setLineData( rail, line, location->key, location->line_ofs, location->width_ofs, line_ofs_max );
  }
}


//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_SetActiveFlag(FIELD_RAIL_MAN * man, BOOL flag)
{
  man->active_flag = flag;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_GetActiveFlag(const FIELD_RAIL_MAN *man)
{
  return man->active_flag;
}

//------------------------------------------------------------------
/**
 * @brief 現在位置の取得
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_GetPos(const FIELD_RAIL_MAN * man, VecFx32 * pos)
{
  getRailPosition(&man->now_rail, pos);
}

//------------------------------------------------------------------
/**
 * @brief カメラ状態のアップデート
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_UpdateCamera(const FIELD_RAIL_MAN * man)
{
  const FIELD_RAIL * rail = &man->now_rail;
	const RAIL_CAMERA_SET* camera_set;
  RAIL_CAMERA_FUNC * func = NULL;

  if (!man->active_flag)
  {
    return;
  }

  if (isValidRail(&man->now_rail) == FALSE)
  {
    return;
  }

  switch (rail->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    if (rail->point->camera_set != RAIL_TBL_NULL)
    {
			camera_set = getRailDatCamera( rail->rail_dat, rail->point->camera_set );
      func = getRailDatCameraFunc( rail->rail_dat, camera_set->func_index );
    }
    break;
  case FIELD_RAIL_TYPE_LINE:
    if (rail->line->camera_set != RAIL_TBL_NULL)
    {
			camera_set = getRailDatCamera( rail->rail_dat, rail->line->camera_set );
      func = getRailDatCameraFunc( rail->rail_dat, camera_set->func_index );
    }
    break;
  default:
    GF_ASSERT(0); //NO TYPE;
  }
  if (func)
  {
    func(man);
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	レールシステム　最新の動作を起こした時のキー情報
 *
 *	@param	man		
 *
 *	@return	キー情報
 */
//-----------------------------------------------------------------------------
int FIELD_RAIL_MAN_GetActionKey( const FIELD_RAIL_MAN * man )
{
	GF_ASSERT( man );
	return man->last_active_key;
}

//----------------------------------------------------------------------------
/**
 *	@brief	one_ofsごとのアップデート
 *
 *	@param	man				マネージャ
 *	@param	key_cont	キー管理
 *	@param	one_ofs		移動オフセット
 */
//-----------------------------------------------------------------------------
static BOOL DEBUG_req = FALSE;	// リクエスト有無
static s32 DEBUG_move_ofs = 0;
static int DEBUG_key_save = 0;
void FIELD_RAIL_MAN_UpdateOfs(FIELD_RAIL_MAN * man, int key_cont, int one_ofs)
{
	
	

  if (!man->active_flag)
  {
    return;
  }

  if (isValidRail(&man->now_rail) == FALSE)
  {
    return;
  }

	man->last_active_key = 0;

	if( DEBUG_req == FALSE )
	{
		RAIL_KEY now_key = keyContToRailKey(key_cont);
	
		if( now_key != RAIL_KEY_NULL )
		{
			DEBUG_req = TRUE;
			DEBUG_move_ofs = 0;
			DEBUG_key_save = key_cont;
		}
	}
	
	if( DEBUG_req )
	{

		FIELD_RAIL_TYPE type = man->now_rail.type;
		RAIL_KEY set_key = RAIL_KEY_NULL;
		RAIL_KEY key = keyContToRailKey(DEBUG_key_save);
		u32 count_up;


		if( DEBUG_key_save & PAD_BUTTON_B )
		{
			count_up = 2;
		}
		else
		{
			count_up = 1;
		}

		DEBUG_move_ofs += count_up;
		

		if(man->now_rail.type == FIELD_RAIL_TYPE_POINT)
		{ //POINT上のときの移動処理
			TOMOYA_Printf( "count_up point %d\n", count_up );
			set_key = updatePointMove(&man->now_rail, key, count_up);
		}
		else if(man->now_rail.type == FIELD_RAIL_TYPE_LINE)
		{ //LINE上のときの移動処理
			set_key = updateLineMove_new(&man->now_rail, key, count_up);
		}

		if (set_key != RAIL_KEY_NULL)
		{
			OS_TPrintf("RAIL:%s :line_ofs=%d line_ofs_max=%d width_ofs=%d\n",
					debugGetRailKeyName(set_key), man->now_rail.line_ofs, man->now_rail.line_ofs_max, man->now_rail.width_ofs);
			man->last_active_key = DEBUG_key_save;
		}

		if (type != man->now_rail.type)
		{
			OS_TPrintf("RAIL:change to ");
			debugPrintRailInfo(&man->now_rail);
		}

		if( DEBUG_move_ofs >= one_ofs )
		{
			DEBUG_req = FALSE;
		}
	}

}

//------------------------------------------------------------------
/**
 * @brief 現在位置のアップデート
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_Update(FIELD_RAIL_MAN * man, int key_cont)
{ 
  FIELD_RAIL_TYPE type = man->now_rail.type;
  RAIL_KEY set_key = RAIL_KEY_NULL;
  RAIL_KEY key = keyContToRailKey(key_cont);
	u32 count_up;

  if (!man->active_flag)
  {
    return;
  }

  if (isValidRail(&man->now_rail) == FALSE)
  {
    return;
  }

	man->last_active_key = 0;

  if (key == RAIL_KEY_NULL)
  {
    return;
  }

	if( key_cont & PAD_BUTTON_B )
	{
		count_up = 2;
	}
	else
	{
		count_up = 1;
	}
	

  if(man->now_rail.type == FIELD_RAIL_TYPE_POINT)
  { //POINT上のときの移動処理
		TOMOYA_Printf( "count_up point %d\n", count_up );
    set_key = updatePointMove(&man->now_rail, key, count_up);
  }
  else if(man->now_rail.type == FIELD_RAIL_TYPE_LINE)
  { //LINE上のときの移動処理
    set_key = updateLineMove_new(&man->now_rail, key, count_up);
  }

  if (set_key != RAIL_KEY_NULL)
  {
    OS_TPrintf("RAIL:%s :line_ofs=%d line_ofs_max=%d width_ofs=%d\n",
        debugGetRailKeyName(set_key), man->now_rail.line_ofs, man->now_rail.line_ofs_max, man->now_rail.width_ofs);
    man->last_active_key = key_cont;
  }

  if (type != man->now_rail.type)
  {
    OS_TPrintf("RAIL:change to ");
    debugPrintRailInfo(&man->now_rail);
  }
}



//------------------------------------------------------------------
//  FIELD_RAIL_MANアクセス関数
//------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	カメラポインタ取得
 */
//-----------------------------------------------------------------------------
FIELD_CAMERA* FIELD_RAIL_MAN_GetCamera( const FIELD_RAIL_MAN * man )
{
	GF_ASSERT( man );
	return  man->field_camera;
}

//----------------------------------------------------------------------------
/**
 *	@brief	今のレールワーク
 */
//-----------------------------------------------------------------------------
const FIELD_RAIL* FIELD_RAIL_MAN_GetNowRail( const FIELD_RAIL_MAN * man )
{
	GF_ASSERT( man );
	return &man->now_rail;
}


//------------------------------------------------------------------
//  FIELD_RAILアクセス関数
//------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	レールタイプの取得
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_TYPE FIELD_RAIL_GetType( const FIELD_RAIL* rail )
{
	GF_ASSERT( rail );
	return rail->type;
}

//----------------------------------------------------------------------------
/**
 *	@brief	レールポイントの取得
 */
//-----------------------------------------------------------------------------
const RAIL_POINT* FIELD_RAIL_GetPoint( const FIELD_RAIL* rail )
{
	GF_ASSERT( rail );
	GF_ASSERT( rail->type == FIELD_RAIL_TYPE_POINT );
	return rail->point;
}

//----------------------------------------------------------------------------
/**
 *	@brief	レールラインの取得
 */
//-----------------------------------------------------------------------------
const RAIL_LINE* FIELD_RAIL_GetLine( const FIELD_RAIL* rail )
{
	GF_ASSERT( rail );
	GF_ASSERT( rail->type == FIELD_RAIL_TYPE_LINE );
	return rail->line;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ラインオフセットの取得
 */
//-----------------------------------------------------------------------------
s32 FIELD_RAIL_GetLineOfs( const FIELD_RAIL* rail )
{
	GF_ASSERT( rail );
	return rail->line_ofs;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ラインオフセット最大値の取得
 */
//-----------------------------------------------------------------------------
s32 FIELD_RAIL_GetLineOfsMax( const FIELD_RAIL* rail )
{
	GF_ASSERT( rail );
	return rail->line_ofs_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	幅オフセットの取得
 */
//-----------------------------------------------------------------------------
s32 FIELD_RAIL_GetWidthOfs( const FIELD_RAIL* rail )
{
	GF_ASSERT( rail );
	return rail->width_ofs;
}

//----------------------------------------------------------------------------
/**
 *	@brief	幅オフセット最大値の取得
 */
//-----------------------------------------------------------------------------
s32 FIELD_RAIL_GetWidthOfsMax( const FIELD_RAIL* rail )
{
	GF_ASSERT( rail );
	return rail->width_ofs_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	移動単位の取得
 */
//-----------------------------------------------------------------------------
fx32 FIELD_RAIL_GetOfsUnit( const FIELD_RAIL* rail )
{
	GF_ASSERT( rail );
	return rail->ofs_unit;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラセットの取得
 */
//-----------------------------------------------------------------------------
const RAIL_CAMERA_SET* FIELD_RAIL_GetCameraSet( const FIELD_RAIL* rail )
{
	GF_ASSERT( rail );
	return getCameraSet( rail );
}



//------------------------------------------------------------------
//  RAIL_LINEアクセス関数
//------------------------------------------------------------------
const RAIL_POINT* FIELD_RAIL_GetPointStart( const FIELD_RAIL* rail )
{
	GF_ASSERT(rail);
	GF_ASSERT( rail->type == FIELD_RAIL_TYPE_LINE );
	return getRailDatPoint( rail->rail_dat, rail->line->point_s );
}
const RAIL_POINT* FIELD_RAIL_GetPointEnd( const FIELD_RAIL* rail )
{
	GF_ASSERT(rail);
	GF_ASSERT( rail->type == FIELD_RAIL_TYPE_LINE );
	return getRailDatPoint( rail->rail_dat, rail->line->point_e );
}
const RAIL_LINEPOS_SET* FIELD_RAIL_GetLinePosSet( const FIELD_RAIL* rail )
{
	GF_ASSERT(rail);
	GF_ASSERT( rail->type == FIELD_RAIL_TYPE_LINE );
	return getRailDatLinePos( rail->rail_dat, rail->line->line_pos_set );
}

//------------------------------------------------------------------
//  RAIL_POINT
//------------------------------------------------------------------
const RAIL_CAMERA_SET* FIELD_RAIL_POINT_GetCameraSet( const FIELD_RAIL* rail, const RAIL_POINT* point )
{
	GF_ASSERT( rail );
	GF_ASSERT( point );
	return getRailDatCamera( rail->rail_dat, point->camera_set);
}



//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static const RAIL_LINE * RAILPOINT_getLineByKey(const RAIL_POINT * point, RAIL_KEY key, const RAIL_DAT* rail_dat)
{
  int i;
  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
    if (point->keys[i] == key)
    {
      return getRailDatLine( rail_dat, point->lines[i] );
    }
  }
  return NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY setLine(FIELD_RAIL * rail, const RAIL_LINE * line, RAIL_KEY key, int l_ofs, int w_ofs, int l_ofs_max)
{
  GF_ASSERT(rail->line != line);
  GF_ASSERT(rail->line);
  GF_ASSERT(line->name);
  if (rail->type == FIELD_RAIL_TYPE_LINE)
  {
    TAMADA_Printf("RAIL: LINE \"%s\" to %s \"%s\"\n",
        rail->line->name, debugGetRailKeyName(key), line->name);
  }
  else
  {
    TAMADA_Printf("RAIL: POINT \"%s\" to %s \"%s\"\n",
        rail->point->name, debugGetRailKeyName(key), line->name);
  }

  setLineData( rail, line, key, l_ofs, w_ofs, l_ofs_max );

  debugCheckLineData(line, rail->rail_dat);
  return key;
}

// レールシステムにラインの情報を設定する
static void setLineData(FIELD_RAIL * rail, const RAIL_LINE * line, RAIL_KEY key, int l_ofs, int w_ofs, int l_ofs_max)
{
  GF_ASSERT( rail );
  GF_ASSERT( line );
  rail->type = FIELD_RAIL_TYPE_LINE;
  rail->line = line;
  rail->key = key;
  rail->line_ofs = l_ofs;
  rail->width_ofs = w_ofs;
  rail->line_ofs_max = l_ofs_max;
}

// ラインの開始ポイントと、引数のpointが一致するかチェック
static BOOL isLinePoint_S( const RAIL_DAT * rail_dat, const RAIL_LINE * line, const RAIL_POINT * point )
{
	const RAIL_POINT * point_s;

	point_s = getRailDatPoint( rail_dat, line->point_s );

	if( point_s == point )
	{
		return TRUE;
	}
	return FALSE;
}

// ラインの終了ポイントと、引数のpointが一致するかチェック
static BOOL isLinePoint_E( const RAIL_DAT * rail_dat, const RAIL_LINE * line, const RAIL_POINT * point )
{
	const RAIL_POINT * point_e;

	point_e = getRailDatPoint( rail_dat, line->point_e );

	if( point_e == point )
	{
		return TRUE;
	}
	return FALSE;
}



//------------------------------------------------------------------
//------------------------------------------------------------------
// レールデータ
static void initRailDat( RAIL_DAT * raildat, const RAIL_SETTING * setting )
{
  raildat->point_count		= setting->point_count;
  raildat->line_count			= setting->line_count;
  raildat->camera_count		= setting->camera_count;
  raildat->linepos_count	= setting->linepos_count;
  raildat->camera_func_count		= setting->camera_func_count;
  raildat->linepos_func_count		= setting->linepos_func_count;
  raildat->line_dist_func_count	= setting->line_dist_func_count;
  raildat->point_table		= setting->point_table;
  raildat->line_table			= setting->line_table;
  raildat->camera_table		= setting->camera_table;
  raildat->linepos_table  = setting->linepos_table;
  raildat->camera_func		= setting->camera_func;
  raildat->line_pos_func  = setting->line_pos_func;
  raildat->line_dist_func = setting->line_dist_func;
}
// ポイント情報取得
static const RAIL_POINT* getRailDatPoint( const RAIL_DAT * raildat, u32 index )
{
	if(raildat->point_count > index)
	{
		return &raildat->point_table[ index ];	
	}
	GF_ASSERT( RAIL_TBL_NULL==index );
	return NULL;
}
// ライン情報取得
static const RAIL_LINE* getRailDatLine( const RAIL_DAT * raildat, u32 index )
{
	if(raildat->line_count > index)
	{
		return &raildat->line_table[ index ];
	}
	GF_ASSERT( RAIL_TBL_NULL==index );
	return NULL;
}
// カメラ情報取得
static const RAIL_CAMERA_SET* getRailDatCamera( const RAIL_DAT * raildat, u32 index )
{
	if(raildat->camera_count > index)
	{
		return &raildat->camera_table[ index ];
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return &RAIL_CAMERA_SET_Default;
}
// ライン位置計算情報取得
static const RAIL_LINEPOS_SET* getRailDatLinePos( const RAIL_DAT * raildat, u32 index )
{
	if(raildat->linepos_count > index)
	{
		return &raildat->linepos_table[ index ];	
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return &RAIL_LINEPOS_SET_Default;
}
// カメラ位置計算関数取得
static RAIL_CAMERA_FUNC*const getRailDatCameraFunc( const RAIL_DAT * raildat, u32 index )
{
	if( raildat->camera_func_count > index )
	{
		return raildat->camera_func[ index ];
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return NULL;
}
// 座標計算関数取得
static RAIL_POS_FUNC*const getRailDatLinePosFunc( const RAIL_DAT * raildat, u32 index )
{
	if( raildat->linepos_func_count > index )
	{	
		return raildat->line_pos_func[ index ];
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return NULL;
}
// ライン距離取得関数
static RAIL_LINE_DIST_FUNC*const getRailDatLineDistFunc( const RAIL_DAT * raildat, u32 index )
{
	if( raildat->line_dist_func_count > index )
	{	
		return raildat->line_dist_func[ index ];
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return NULL;
}



//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY updateLineMove_new(FIELD_RAIL * rail, RAIL_KEY key, u32 count_up)
{ //LINE上のときの移動処理
  const RAIL_LINE * nLine = rail->line;
  s32 nLine_ofs_max = getLineOfsMax( nLine, rail->ofs_unit, rail->rail_dat ); // 今のLINEのオフセット最大値
  s32 ofs_max;  // 各ラインのオフセット最大値

  if (key == nLine->key)
  {//正方向キーの場合
    const RAIL_POINT* nPoint = getRailDatPoint( rail->rail_dat, nLine->point_e );
    const RAIL_LINE * front = RAILPOINT_getLineByKey(nPoint, key, rail->rail_dat);
    const RAIL_LINE * left = RAILPOINT_getLineByKey(nPoint, getAntiClockwiseKey(key), rail->rail_dat);
    const RAIL_LINE * right = RAILPOINT_getLineByKey(nPoint, getClockwiseKey(key), rail->rail_dat);
    //const RAIL_LINE * back = RAILPOINT_getLineByKey(nPoint, getReverseKey(key), rail->rail_dat);
    TAMADA_Printf("↑");
    rail->line_ofs += count_up;
    if (rail->line_ofs <= nLine_ofs_max) {
			return updateLine( rail, nLine, nLine_ofs_max, key );
		}
    if (front)
    { //正面移行処理
      debugCheckPointData(nPoint, rail->rail_dat);
      ofs_max = getLineOfsMax( front, rail->ofs_unit, rail->rail_dat );
      return setLine(rail, front, key,
          /*l_ofs*/rail->line_ofs - nLine_ofs_max,
          /*w_ofs*/rail->width_ofs,
          /*ofs_max*/ofs_max); //そのまま
    }
    if (left && rail->width_ofs < 0)
    { //左側移行処理
      if ( isLinePoint_S( rail->rail_dat, left, nPoint ) )
      { //始端の場合
        debugCheckPointData(nPoint, rail->rail_dat);
        ofs_max = getLineOfsMax( left, rail->ofs_unit, rail->rail_dat );
        return setLine(rail, left, key,
            /*l_ofs*/rail->width_ofs,
            /*w_ofs*/0,
            /*ofs_max*/ofs_max); 
      }
      else if (isLinePoint_E( rail->rail_dat, left, nPoint ))
      { //終端の場合
        debugCheckPointData(nPoint, rail->rail_dat);
        ofs_max = getLineOfsMax( left, rail->ofs_unit, rail->rail_dat );
        return setLine(rail, left, key,
            /*l_ofs*/ofs_max + rail->width_ofs, //width_ofs < 0なので実質減算
            /*w_ofs*/0,
            /*ofs_max*/ofs_max); 
      }
      else GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
    }
    if (right && rail->width_ofs > 0)
    { //右側移行処理
      if ( isLinePoint_S( rail->rail_dat, right, nPoint ) )
      { //始端の場合
        debugCheckPointData(nPoint, rail->rail_dat);
        ofs_max = getLineOfsMax( right, rail->ofs_unit, rail->rail_dat );
        return setLine(rail, right, key,
            /*l_ofs*/rail->width_ofs,
            /*w_ofs*/0,
            /*ofs_max*/ofs_max); 
      }
			else if ( isLinePoint_E( rail->rail_dat, right, nPoint ) )
      { //終端の場合
        debugCheckPointData(nPoint, rail->rail_dat);
        ofs_max = getLineOfsMax( right, rail->ofs_unit, rail->rail_dat );
        return setLine(rail, right, key,
            /*l_ofs*/ofs_max - rail->width_ofs,
            /*w_ofs*/0,
            /*ofs_max*/ofs_max);
      }
      else GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
    }

    //行くあてがない…LINEそのまま、加算をとりけし
    rail->line_ofs = nLine_ofs_max;
    return key;
  }
  else if (key == getReverseKey(nLine->key))
  {//逆方向キーの場合
    const RAIL_POINT* nPoint = getRailDatPoint( rail->rail_dat, nLine->point_s );
    //const RAIL_LINE * front = RAILPOINT_getLineByKey(nPoint, key, rail->rail_dat);
    const RAIL_LINE * left = RAILPOINT_getLineByKey(nPoint, getClockwiseKey(key), rail->rail_dat);
    const RAIL_LINE * right = RAILPOINT_getLineByKey(nPoint, getAntiClockwiseKey(key), rail->rail_dat);
    const RAIL_LINE * back = RAILPOINT_getLineByKey(nPoint, key, rail->rail_dat);
    TAMADA_Printf("↓");
    rail->line_ofs -= count_up;
    if (rail->line_ofs >= 0) {
			return updateLine( rail, nLine, nLine_ofs_max, key );
		}
    if (back)
    {//背面移行処理
      debugCheckPointData(nPoint, rail->rail_dat);
      ofs_max = getLineOfsMax( back, rail->ofs_unit, rail->rail_dat );
      return setLine(rail, back, key,
          /*l_ofs*/ofs_max - MATH_ABS(rail->line_ofs),	// ここには、必ず負の値が来る
          /*w_ofs*/rail->width_ofs,
          /*ofs_max*/ofs_max);
    }
    if (left && rail->width_ofs < 0)
    { //左側移行処理
			if ( isLinePoint_S( rail->rail_dat, left, nPoint ) )
      { //始端の場合
        debugCheckPointData(nPoint, rail->rail_dat);
        ofs_max = getLineOfsMax( left, rail->ofs_unit, rail->rail_dat );
        return setLine(rail, left, key,
            /*l_ofs*/rail->width_ofs,
            /*w_ofs*/0,
          /*ofs_max*/ofs_max);
      }
			else if ( isLinePoint_E( rail->rail_dat, left, nPoint ) )
      { //終端の場合
        debugCheckPointData(nPoint, rail->rail_dat);
        ofs_max = getLineOfsMax( right, rail->ofs_unit, rail->rail_dat );
        return setLine(rail, right, key,
            /*l_ofs*/ofs_max + rail->width_ofs, //width_ofs < 0なので実質減算
            /*w_ofs*/0,
          /*ofs_max*/ofs_max);
      }
      else GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
    }
    if (right && rail->width_ofs > 0)
    { //右側移行処理
			if ( isLinePoint_S( rail->rail_dat, right, nPoint ) )
      { //始端の場合
        debugCheckPointData(nPoint, rail->rail_dat);
        ofs_max = getLineOfsMax( right, rail->ofs_unit, rail->rail_dat );
        return setLine(rail, right, key,
            /*l_ofs*/rail->width_ofs,
            /*w_ofs*/0,
          /*ofs_max*/ofs_max);
      }
			if ( isLinePoint_E( rail->rail_dat, right, nPoint ) )
      { //終端の場合
        debugCheckPointData(nPoint, rail->rail_dat);
        ofs_max = getLineOfsMax( right, rail->ofs_unit, rail->rail_dat );
        return setLine(rail, right, key,
            /*l_ofs*/ofs_max - rail->width_ofs,
            /*w_ofs*/0,
          /*ofs_max*/ofs_max);
      }
      else GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, right->name);
    }
    //行くあてがない…LINEそのまま、減算を取り消し
    rail->line_ofs = 0;
    return key;
  }
  else if (key == getClockwiseKey(nLine->key))
  {//時計回り隣方向キーの場合
    TAMADA_Printf("→");
    rail->width_ofs += count_up;
    if (rail->width_ofs < rail->width_ofs_max)
    {//範囲内の場合、終了
			return updateLine( rail, nLine, nLine_ofs_max, key );
    }
    else if (rail->line_ofs < rail->width_ofs_max)
    { //始端に近い場合
      const RAIL_POINT* nPoint = getRailDatPoint( rail->rail_dat, nLine->point_s );
      const RAIL_LINE *right = RAILPOINT_getLineByKey(nPoint, key, rail->rail_dat);
      if (right)
      {
				if ( isLinePoint_S( rail->rail_dat, right, nPoint ) )
        {//右側LINEは始端からの場合
          debugCheckPointData(nPoint, rail->rail_dat);
          ofs_max = getLineOfsMax( right, rail->ofs_unit, rail->rail_dat );
          return setLine(rail, right, key,
              rail->width_ofs,
              - rail->line_ofs,
              ofs_max);
        }
				if ( isLinePoint_E( rail->rail_dat, right, nPoint ) )
        {//右側LINEは終端からの場合
          debugCheckPointData(nPoint, rail->rail_dat);
          ofs_max = getLineOfsMax( right, rail->ofs_unit, rail->rail_dat );
          return setLine(rail, right, key,
              ofs_max - rail->width_ofs,
              rail->line_ofs,
              ofs_max);
        }
        GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, right->name);
      }
    }
    else if (rail->line_ofs >= nLine_ofs_max - rail->width_ofs_max)
    { //終端に近い場合
      const RAIL_POINT* nPoint = getRailDatPoint( rail->rail_dat, nLine->point_e );
      const RAIL_LINE *right = RAILPOINT_getLineByKey(nPoint, key, rail->rail_dat);
      if (right)
      {
				if ( isLinePoint_S( rail->rail_dat, right, nPoint ) )
        {//右側LINEは始端からの場合
          debugCheckPointData(nPoint, rail->rail_dat);
          ofs_max = getLineOfsMax( right, rail->ofs_unit, rail->rail_dat );
          return setLine(rail, right, key,
              rail->width_ofs,
              nLine_ofs_max - rail->line_ofs,
              ofs_max);
        }
				if ( isLinePoint_E( rail->rail_dat, right, nPoint ) )
        {//右側LINEは終端からの場合
          debugCheckPointData(nPoint, rail->rail_dat);
          ofs_max = getLineOfsMax( right, rail->ofs_unit, rail->rail_dat );
          return setLine(rail, right, key,
              ofs_max - rail->width_ofs,
              - (nLine_ofs_max - rail->line_ofs),
              ofs_max);
        }
        GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, right->name);
      }
    }
    //中間地点の場合、行くあてがない場合…LINEそのまま、加算を取り消し
    rail->width_ofs = rail->width_ofs_max - 1;
    return key;
  }
  else if (key == getAntiClockwiseKey(nLine->key))
  {//反時計回り隣方向キーの場合
    TAMADA_Printf("←");
    rail->width_ofs -= count_up;
    if (MATH_ABS(rail->width_ofs) < rail->width_ofs_max)
    {//範囲内の場合、終了
			return updateLine( rail, nLine, nLine_ofs_max, key );
    }
    else if (rail->line_ofs < rail->width_ofs_max)
    {//始端に近い場合
      const RAIL_POINT* nPoint = getRailDatPoint( rail->rail_dat, nLine->point_s );
      const RAIL_LINE* left = RAILPOINT_getLineByKey(nPoint, key, rail->rail_dat);
      if (left)
      {
				if ( isLinePoint_S( rail->rail_dat, left, nPoint ) )
        {//左側LINEが始端からの場合
          debugCheckPointData(nPoint, rail->rail_dat);
          ofs_max = getLineOfsMax( left, rail->ofs_unit, rail->rail_dat );
          return setLine(rail, left, key,
              MATH_ABS(rail->width_ofs), 
              rail->line_ofs,
              ofs_max);
        }
				if ( isLinePoint_E( rail->rail_dat, left, nPoint ) )
        {//左側LINEが終端からの場合
          debugCheckPointData(nPoint, rail->rail_dat);
          ofs_max = getLineOfsMax( left, rail->ofs_unit, rail->rail_dat );
          return setLine(rail, left, key,
              ofs_max - MATH_ABS(rail->width_ofs),
              - rail->line_ofs,
              ofs_max);
        }
        GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
      }
    }
    if (rail->line_ofs >= nLine_ofs_max - rail->width_ofs_max)
    {//終端に近い場合
      const RAIL_POINT* nPoint = getRailDatPoint( rail->rail_dat, nLine->point_e );
      const RAIL_LINE* left = RAILPOINT_getLineByKey(nPoint, key, rail->rail_dat);
      if (left)
      {
				if ( isLinePoint_S( rail->rail_dat, left, nPoint ) )
        {//左側LINEが始端からの場合
          debugCheckPointData(nPoint, rail->rail_dat);
          ofs_max = getLineOfsMax( left, rail->ofs_unit, rail->rail_dat );
          return setLine(rail, left, key,
              MATH_ABS(rail->width_ofs),
              - (nLine_ofs_max - rail->line_ofs),
              ofs_max);
        }
				if ( isLinePoint_E( rail->rail_dat, left, nPoint ) )
        {//左側LINEが終端からの場合
          debugCheckPointData(nPoint, rail->rail_dat);
          ofs_max = getLineOfsMax( left, rail->ofs_unit, rail->rail_dat );
          return setLine(rail, left, key,
              ofs_max - MATH_ABS(rail->width_ofs),
              nLine_ofs_max - rail->line_ofs,
              ofs_max);
        }
        GF_ASSERT_MSG(0, "%sから%sへの接続異常\n", nLine->name, left->name);
      }
    }
    //中間地点の場合、行くあてがない場合…LINEそのまま、減算を取り消し
    rail->width_ofs += count_up;
    return key;
  }
  return RAIL_KEY_NULL;
}

#if 0
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY updateLineMove(FIELD_RAIL * rail, RAIL_KEY key)
{ //LINE上のときの移動処理
  const RAIL_LINE * nLine = rail->line;
  if (key == nLine->key)
  {//正方向キーの場合
    rail->line_ofs ++;
    if (rail->line_ofs == rail->line_ofs_max)
    { // LINE --> point_e への移行処理
      rail->type = FIELD_RAIL_TYPE_POINT;
      rail->point = nLine->point_e;
      debugCheckPointData(rail->point);
    }
    return key;
  }
  else if (key == getReverseKey(nLine->key))
  {//逆方向キーの場合
    rail->line_ofs --;
    if (rail->line_ofs == 0)
    { // LINE --> POINT_S への移行処理
      rail->type = FIELD_RAIL_TYPE_POINT;
      rail->point = nLine->point_s;
      debugCheckPointData(rail->point);
    }
    return key;
  }
  else if (key == getClockwiseKey(nLine->key))
  {//時計回りとなり方向キーの場合
    if (rail->width_ofs < rail->width_ofs_max)
    {
      rail->width_ofs ++;
    }
    return key;
  }
  else if (key == getAntiClockwiseKey(nLine->key))
  {//反時計回りとなり方向キーの場合
    if (rail->width_ofs > - rail->width_ofs_max)
    {
      rail->width_ofs --;
    }
    return key;
  }
  return RAIL_KEY_NULL;
}
#endif

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY updatePointMove(FIELD_RAIL * rail, RAIL_KEY key, u32 count_up)
{//POINT上のときの移動処理
  int i;
  const RAIL_POINT * nPoint = rail->point;
  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
    if (nPoint->keys[i] == key)
    { // POINT --> LINE[i] への移行処理
      const RAIL_LINE * nLine = getRailDatLine( rail->rail_dat, nPoint->lines[i] );
      s32 ofs_max;

      // 
      ofs_max = getLineOfsMax( nLine, rail->ofs_unit, rail->rail_dat );
      
			if ( isLinePoint_S( rail->rail_dat, nLine, nPoint ) )
      { 
        setLine(rail, nLine, key, count_up, 0, ofs_max);
        //rail->line_ofs = 1;  //最初は１から！
      }
			else if ( isLinePoint_E( rail->rail_dat, nLine, nPoint ) )
      {
        setLine(rail, nLine, key, ofs_max - count_up, 0, ofs_max);
        //rail->line_ofs = nLine->ofs_max - 1; //最後ー1から！
      }
      //つながったLINEを次のRAILとしてセット
      //rail->type = FIELD_RAIL_TYPE_LINE;
      //rail->line = nLine;
      //debugCheckLineData(rail->line, rail->rail_dat);
      return key;
    }
  }
  return RAIL_KEY_NULL;
}



//============================================================================================
/**
 */
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void initRail(FIELD_RAIL * rail, const RAIL_DAT* rail_dat )
{
  rail->type = FIELD_RAIL_TYPE_MAX;
  rail->dummy = NULL;
  rail->line_ofs = 0;
  rail->width_ofs = 0;
  rail->key = RAIL_KEY_NULL;
	rail->rail_dat = rail_dat;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL isValidRail(const FIELD_RAIL * rail)
{
  if (rail->type < FIELD_RAIL_TYPE_MAX && rail->dummy != NULL)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static const RAIL_CAMERA_SET * getCameraSet(const FIELD_RAIL * rail)
{
  switch (rail->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    return getRailDatCamera( rail->rail_dat, rail->point->camera_set );
    break;
  case FIELD_RAIL_TYPE_LINE:
    return getRailDatCamera( rail->rail_dat, rail->line->camera_set);
    break;
  }
  GF_ASSERT(0);
  return &RAIL_CAMERA_SET_Default;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void getRailPosition(const FIELD_RAIL * rail, VecFx32 * pos)
{
  RAIL_POS_FUNC * func = NULL;
	const RAIL_LINEPOS_SET* line_pos_set;

  if(rail->type == FIELD_RAIL_TYPE_POINT)
  {
    //POINTにいるときはPOINTの座標
    *pos = rail->point->pos;
  }
  else if(rail->type == FIELD_RAIL_TYPE_LINE)
  {
		line_pos_set = getRailDatLinePos( rail->rail_dat, rail->line->line_pos_set );
    func = getRailDatLinePosFunc( rail->rail_dat, line_pos_set->func_index );
    if (func)
    {
			func(rail, pos);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ラインのオフセット最大数を求める
 *
 *	@param	line    ライン情報
 *	@param	unit    １オフセットの距離
 *
 *	@return オフセット最大数
 */
//-----------------------------------------------------------------------------
static s32 getLineOfsMax( const RAIL_LINE * line, fx32 unit, const RAIL_DAT* rail_dat )
{
  const RAIL_LINEPOS_SET * line_pos_set;
	const RAIL_POINT* point_s;
	const RAIL_POINT* point_e;
	RAIL_LINE_DIST_FUNC* dist_func;
  fx32 dist;
  s32 div;

	point_s = getRailDatPoint( rail_dat, line->point_s );
	point_e = getRailDatPoint( rail_dat, line->point_e );

  // ライン動作情報を取得
  line_pos_set = getRailDatLinePos( rail_dat, line->line_pos_set );

	// 距離の取得
	dist_func = getRailDatLineDistFunc( rail_dat, line_pos_set->func_dist_index );
	if( dist_func )
	{
		dist = dist_func( point_s, point_e, line_pos_set );
		div   = FX_Div( dist, unit ) >> FX32_SHIFT; // 小数点は無視
	}
	else
	{
		div = 0;
	}


//  OS_TPrintf( "div = %d dist = 0x%x unit = 0x%x\n", div, dist, unit );

  return div;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライン幅オフセット最大値を求める
 *
 *	@param	line
 *	@param	line_ofs		ライン　オフセット値（進み具合）
 *	@param	rail_dat 
 *
 *	@return	幅オフセット最大値
 */
//-----------------------------------------------------------------------------
static s32 getLineWidthOfsMax( const RAIL_LINE * line, u32 line_ofs, u32 line_ofs_max, const RAIL_DAT* rail_dat )
{
	s32 width_ofs_max;
	s32 width_ofs_div;
	const RAIL_POINT* cp_point_s;
	const RAIL_POINT* cp_point_e;
	int i, j;
	int width_s, width_e;

	// ラインの始点と終点を取得
	GF_ASSERT( line->point_s != RAIL_TBL_NULL );
	cp_point_s = &rail_dat->point_table[ line->point_s ];
	GF_ASSERT( line->point_e != RAIL_TBL_NULL );
	cp_point_e = &rail_dat->point_table[ line->point_e ];

	// 一致するラインの幅を取得
	width_e = -1;
	width_s = -1;
	for( i=0; i<RAIL_CONNECT_LINE_MAX; i++ )
	{
		if( cp_point_s->lines[i] != RAIL_TBL_NULL )
		{
			if( (u32)(&rail_dat->line_table[cp_point_s->lines[i]]) == (u32)(line) )
			{
				width_s = cp_point_s->width_ofs_max[i];
			}
		}

		if( cp_point_e->lines[i] != RAIL_TBL_NULL )
		{
			if( (u32)(&rail_dat->line_table[cp_point_e->lines[i]]) == (u32)(line) )
			{
				width_e = cp_point_e->width_ofs_max[i];
			}
		}
	}
	if( width_s < 0 )
	{
		width_s = width_e;
	}
	if( width_e < 0 )
	{
		width_e = width_s;
	}
	GF_ASSERT( width_s >= 0 );

	width_ofs_div = width_e - width_s;
	width_ofs_max = (width_ofs_div * (s32)line_ofs) / (s32)line_ofs_max;
	width_ofs_max += width_s;
	

	return width_ofs_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライン動作　通常更新（幅チェックをする）
 *
 *	@param	rail					レールシステム
 *	@param	line					進行中のライン
 *	@param	line_ofs_max	ラインオフセット最大値
 *	@param	key						押されているキー
 *
 *	@return	情報を変更したキー
 */
//-----------------------------------------------------------------------------
static RAIL_KEY updateLine( FIELD_RAIL * rail, const RAIL_LINE * line, u32 line_ofs_max, u32 key )
{
  s32 width_ofs_max = getLineWidthOfsMax( line, rail->line_ofs, line_ofs_max, rail->rail_dat ); // 今のLINEのオフセット最大値

	rail->width_ofs_max = width_ofs_max;
	if( MATH_ABS(rail->width_ofs) >= width_ofs_max )
	{
		if(rail->width_ofs > 0)
		{
			rail->width_ofs = width_ofs_max-1;
		}
		else
		{
			rail->width_ofs = -(width_ofs_max-1);
		}
	}

	return key;
}

//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
static RAIL_KEY keyContToRailKey(int cont)
{
  if (cont & PAD_KEY_UP) return RAIL_KEY_UP;
  if (cont & PAD_KEY_LEFT) return RAIL_KEY_LEFT;
  if (cont & PAD_KEY_RIGHT) return RAIL_KEY_RIGHT;
  if (cont & PAD_KEY_DOWN) return RAIL_KEY_DOWN;
  return RAIL_KEY_NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY getReverseKey(RAIL_KEY key)
{
  switch (key) {
  case RAIL_KEY_UP: 
    return RAIL_KEY_DOWN;
  case RAIL_KEY_DOWN:
    return RAIL_KEY_UP;
  case RAIL_KEY_LEFT:
    return RAIL_KEY_RIGHT;
  case RAIL_KEY_RIGHT:
    return RAIL_KEY_LEFT;
  }
  return RAIL_KEY_NULL;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY getClockwiseKey(RAIL_KEY key)
{
  switch (key) {
  case RAIL_KEY_UP: 
    return RAIL_KEY_RIGHT;
  case RAIL_KEY_DOWN:
    return RAIL_KEY_LEFT;
  case RAIL_KEY_LEFT:
    return RAIL_KEY_UP;
  case RAIL_KEY_RIGHT:
    return RAIL_KEY_DOWN;
  }
  return RAIL_KEY_NULL;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY getAntiClockwiseKey(RAIL_KEY key)
{
  switch (key) {
  case RAIL_KEY_UP: 
    return RAIL_KEY_LEFT;
  case RAIL_KEY_DOWN:
    return RAIL_KEY_RIGHT;
  case RAIL_KEY_LEFT:
    return RAIL_KEY_DOWN;
  case RAIL_KEY_RIGHT:
    return RAIL_KEY_UP;
  }
  return RAIL_KEY_NULL;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static const char * getRailName(const FIELD_RAIL * rail)
{
  switch (rail->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    return rail->point->name;
  case FIELD_RAIL_TYPE_LINE:
    return rail->line->name;
  }
  return "NO ENTRY";
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY searchLineInPoint(const RAIL_POINT *point, const RAIL_LINE * line, const RAIL_DAT* rail_dat)
{
  int i;
	const RAIL_LINE* point_line;
  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
		point_line = getRailDatLine( rail_dat, point->lines[i] );
    if (point_line == line) return point->keys[i];
  }
  return RAIL_KEY_NULL;
}

//============================================================================================
//
//
//    デバッグ用関数
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL debugCheckLineData(const RAIL_LINE * line, const RAIL_DAT* rail_dat)
{

  RAIL_KEY key;
	const RAIL_POINT* point_s;
	const RAIL_POINT* point_e;

  if(line->point_s == RAIL_TBL_NULL || line->point_e == RAIL_TBL_NULL)
  {
    OS_TPrintf("%sはPOINTが正しくありません\n", line->name);
    return FALSE;
  }
	point_s = getRailDatPoint( rail_dat, line->point_s );
	point_e = getRailDatPoint( rail_dat, line->point_e );
	
  key = searchLineInPoint(point_s, line, rail_dat);
  if (key == RAIL_KEY_NULL)
  {
    OS_TPrintf("%sは%sと正しく接続できていません\n", line->name, point_s->name);
    return FALSE;
  }
  if (key != line->key)
  {
    OS_TPrintf("%sは始端%sとキーがあっていません\n", line->name, point_s->name);
    return FALSE;
  }
  key = searchLineInPoint(point_e, line, rail_dat);
  if (key == RAIL_KEY_NULL)
  {
    OS_TPrintf("%sは%sと正しく接続できていません\n", line->name, point_e->name);
    return FALSE;
  }
  if (key != getReverseKey(line->key))
  {
    OS_TPrintf("%sは終端%sとキーがあっていません\n", line->name, point_e->name);
    return FALSE;
  }
  return TRUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL debugCheckPointData(const RAIL_POINT * point, const RAIL_DAT* rail_dat)
{
  int i, j;
	const RAIL_LINE* line;
	const RAIL_LINE* line_sarch;
	const RAIL_POINT* point_s;
	const RAIL_POINT* point_e;

  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
    if ((point->keys[i] == RAIL_KEY_NULL) != (point->lines[i] == RAIL_TBL_NULL) )
    {
      OS_TPrintf("%s:LINEとRAIL_KEYの指定に矛盾があります\n", point->name);
      return FALSE;
    }

    if (point->lines[i] == RAIL_TBL_NULL) continue;

		// ライン情報取得
		line = getRailDatLine( rail_dat, point->lines[i] );
		point_s = getRailDatPoint( rail_dat, line->point_s );
		point_e = getRailDatPoint( rail_dat, line->point_e );
		
    if (point_s != point && point_e != point)
    {
      OS_TPrintf("%sは%sと接続していません\n",point->name, line->name);
      return FALSE;
    }

    if (point->keys[i] != RAIL_KEY_NULL)
    {
      for (j = i+1; j < RAIL_CONNECT_LINE_MAX; j++)
      {
        if (point->keys[i] == getReverseKey(point->keys[j]))
        {
					line_sarch = getRailDatLine( rail_dat, point->lines[j] );
          if (line->key != line_sarch->key)
          {
            OS_Printf("%s から %s 経由で %s へ向かうキーに矛盾があります\n",
                line->name, point->name, line_sarch->name);
            return FALSE;
          }
        }
      }
    }
  }
  return TRUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintWholeVector(const char * before, const VecFx32 * vec, const char * after)
{
  if (before)
  {
    OS_TPrintf("%s",before);
  }
  OS_TPrintf("( %d, %d, %d)", FX_Whole(vec->x), FX_Whole(vec->y), FX_Whole(vec->z));
  if (after)
  {
    OS_TPrintf("%s",after);
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintHexVector(const VecFx32 * vec)
{
  OS_TPrintf("(%08x, %08x, %08x)\n", (vec->x), (vec->y), (vec->z));
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintPoint(const char * before, const RAIL_POINT * point, const char * after)
{
  if (point != NULL)
  {
    debugPrintWholeVector(before, &point->pos, after);
  }
  else
  {
    if (before) OS_TPrintf("%s", before);
    OS_TPrintf("( NULL )");
    if (after) OS_TPrintf("%s", after);
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintDegree(const VecFx32 * p0, const VecFx32 * p1)
{
    VecFx32 s,e;
    fx32 angle;
    VEC_Normalize( p0, &s );
    VEC_Normalize( p1, &e );
    angle = FX_AcosIdx( VEC_DotProduct( &s, &e ) );
    OS_TPrintf("Degree: %08x\n",angle);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static const char * debugGetRailKeyName(RAIL_KEY key)
{
    static const char * const names[] = {
      "RAIL_KEY_NULL","RAIL_KEY_UP","RAIL_KEY_RIGHT","RAIL_KEY_DOWN","RAIL_KEY_LEFT"
    };
    GF_ASSERT(key < RAIL_KEY_MAX);
    return names[key];
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintRailInfo(const FIELD_RAIL * rail)
{
  FIELD_RAIL_TYPE type = rail->type;
  OS_TPrintf("%d(%s:%08x)\n",
      type,
      getRailName(rail),
      rail->dummy);
  if (type == FIELD_RAIL_TYPE_POINT)
  {
    debugPrintPoint(NULL, rail->point ,"\n");
  } else if (type == FIELD_RAIL_TYPE_LINE)
  {
		const RAIL_POINT* point_s;
		const RAIL_POINT* point_e;

		point_s = getRailDatPoint( rail->rail_dat, rail->line->point_s );
		point_e = getRailDatPoint( rail->rail_dat, rail->line->point_e );
    debugPrintPoint("start", point_s, NULL);
    debugPrintPoint("-end", point_e, "\n");
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief  ポイントのテーブルインデックスを取得
 *
 *	@param	man       マネージャ
 *	@param	point     ポイント
 *
 *	@return インデックス
 */
//-----------------------------------------------------------------------------
static u32 getPointIndex( const FIELD_RAIL_MAN * man, const RAIL_POINT* point )
{
  int i;

  for( i=0; i<man->rail_dat.point_count; i++ ){
    if( (u32)&man->rail_dat.point_table[i] == (u32)point ){
      return i;
    }
  }

  GF_ASSERT_MSG( i<man->rail_dat.point_count, "不明なpointです。" );
  return 0; // フリーズ回避
}

//----------------------------------------------------------------------------
/**
 *	@brief  ラインのテーブルインデックスを取得
 *
 *	@param	man       マネージャ
 *	@param	line      ライン
 *
 *	@return インデックス
 */
//-----------------------------------------------------------------------------
static u32 getLineIndex( const FIELD_RAIL_MAN * man, const RAIL_LINE* line )
{
  int i;

  for( i=0; i<man->rail_dat.line_count; i++ ){
    if( (u32)&man->rail_dat.line_table[i] == (u32)line ){
      return i;
    }
  }

  GF_ASSERT_MSG( i<man->rail_dat.line_count, "不明なlineです。" );
  return 0; // フリーズ回避
}

