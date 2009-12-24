//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_rail_loader.c
 *	@brief	フィールドレールローダー
 *	@author	tomoya takahashi
 *	@date		2009.07.07
 *
 *	モジュール名：FIELD_RAIL_LOADER
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "arc_def.h"

#include "field_rail_loader_func.h"
#include "field_rail_func.h"

#include "field/field_rail_loader.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
#define RAIL_DATA_HEADER_LABEL	(0x4c52)

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	オフセット情報
//=====================================
typedef struct {
  fx32  ofs_unit;     // 幅の移動単位
} OFS_DATA;


//-------------------------------------
///	header
//=====================================
typedef struct {
	u16 label;
	u16 dummy;
	u32 ofsdata_offset;
	u32 pointdata_offset;
	u32 linedata_offset;
	u32 cameradata_offset;
	u32 lineposdata_offset;
	u32 end_pos;
} RAIL_DATA_HEADER;


//-------------------------------------
///	ワーク
//=====================================
struct _FIELD_RAIL_LOADER 
{
	RAIL_DATA_HEADER* p_work;
	u32 work_size;
	RAIL_SETTING setting;
};


//-----------------------------------------------------------------------------
/**
 *			カメラ動作、座標動作、距離取得　関数テーブル
 */
//-----------------------------------------------------------------------------
// カメラ動作関数テーブル
static RAIL_CAMERA_FUNC* sp_RAIL_CAMERA_FUNC[FIELD_RAIL_LOADER_CAMERA_FUNC_MAX] = {
	FIELD_RAIL_CAMERAFUNC_FixPosCamera,
	FIELD_RAIL_CAMERAFUNC_FixAngleCamera,
	FIELD_RAIL_CAMERAFUNC_OfsAngleCamera,
	FIELD_RAIL_CAMERAFUNC_FixAllCamera,
	FIELD_RAIL_CAMERAFUNC_FixAllLineCamera,
	FIELD_RAIL_CAMERAFUNC_CircleCamera,
	FIELD_RAIL_CAMERAFUNC_FixLenCircleCamera,
	FIELD_RAIL_CAMERAFUNC_TargetChangeCircleCamera,
  FIELD_RAIL_CAMERAFUNC_PlayerTargetCircleCamera,
  FIELD_RAIL_CAMERAFUNC_FixAngleLineWay_XZ,
  FIELD_RAIL_CAMERAFUNC_FixAngleLineWay_XYZ,
};

// 座標動作関数テーブル
static RAIL_POS_FUNC* sp_RAIL_POS_FUNC[FIELD_RAIL_LOADER_LINEPOS_FUNC_MAX] = 
{
	FIELD_RAIL_POSFUNC_StraitLine,
	FIELD_RAIL_POSFUNC_CurveLine,
	FIELD_RAIL_POSFUNC_YCurveLine,
};

// ３Ｄ座標のロケーション当たり判定
static RAIL_LINE_HIT_LOCATION_FUNC* sp_RAIL_LINE_HIT_LOCATION_FUNC[FIELD_RAIL_LOADER_LINEPOS_FUNC_MAX] = 
{
	FIELD_RAIL_LINE_HIT_LOCATION_FUNC_StraitLine,
	FIELD_RAIL_LINE_HIT_LOCATION_FUNC_CircleLine,
	FIELD_RAIL_LINE_HIT_LOCATION_FUNC_CircleLine,
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static void SettingClear( RAIL_SETTING* p_setting )
{
	GFL_STD_MemClear( p_setting, sizeof(RAIL_SETTING) );
	p_setting->camera_func_count		= FIELD_RAIL_LOADER_CAMERA_FUNC_MAX;
	p_setting->linepos_func_count		= FIELD_RAIL_LOADER_LINEPOS_FUNC_MAX;
	p_setting->line_hit_location_count	= FIELD_RAIL_LOADER_LINEPOS_FUNC_MAX;
	p_setting->camera_func					= sp_RAIL_CAMERA_FUNC;
	p_setting->line_pos_func				= sp_RAIL_POS_FUNC;
	p_setting->line_hit_location_func = sp_RAIL_LINE_HIT_LOCATION_FUNC;

}
#ifdef PM_DEBUG
static void PrintPoint( const RAIL_POINT* cp_point )
{
	int i;

	TOMOYA_Printf( "*point name = %s\n", cp_point->name );
	for( i=0; i<RAIL_CONNECT_LINE_MAX; i++ )
	{
		TOMOYA_Printf( "lines[%d] = %d\n", i, cp_point->lines[i] );
		TOMOYA_Printf( "keys[%d] = %d\n", i, cp_point->keys[i] );
	}
	TOMOYA_Printf( "pos x[0x%x]y[0x%x]z[0x%x]\n", cp_point->pos.x, cp_point->pos.y, cp_point->pos.z );
	TOMOYA_Printf( "camera_set = %d\n", cp_point->camera_set );
	TOMOYA_Printf( "\n" );
}

static void PrintLine( const RAIL_LINE* cp_line )
{
	TOMOYA_Printf( "*line name = %s\n", cp_line->name );
	TOMOYA_Printf( "point_s = %d\n", cp_line->point_s );
	TOMOYA_Printf( "point_e = %d\n", cp_line->point_e );
	TOMOYA_Printf( "key = %d\n", cp_line->key );
	TOMOYA_Printf( "line_pos_set = %d\n", cp_line->line_pos_set );
	TOMOYA_Printf( "camera_set = %d\n", cp_line->camera_set );
	TOMOYA_Printf( "\n" );
}

static void PrintCamera( const RAIL_CAMERA_SET* cp_camera )
{
	TOMOYA_Printf( "camera_set\n" );
	TOMOYA_Printf( "funcindex = %d\n", cp_camera->func_index );
	TOMOYA_Printf( "\n" );
}

static void PrintLinePos( const RAIL_LINEPOS_SET* cp_linepos )
{
	TOMOYA_Printf( "linepos_set\n" );
	TOMOYA_Printf( "funcindex = %d\n", cp_linepos->func_index );
	TOMOYA_Printf( "\n" );
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief	レールローダー	生成
 *
 *	@param	heapID	ヒープID
 *
 *	@return	レールコントローラ
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_LOADER* FIELD_RAIL_LOADER_Create( u32 heapID )
{
	FIELD_RAIL_LOADER* p_loader;

	p_loader = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_RAIL_LOADER) );

	// 関数配列の設定
	SettingClear( &p_loader->setting );

	return p_loader;
}

//----------------------------------------------------------------------------
/**
 *	@brief	レールローダー　破棄
 *
 *	@param	p_sys		システム
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_LOADER_Delete( FIELD_RAIL_LOADER* p_sys )
{
	FIELD_RAIL_LOADER_Clear( p_sys );
	GFL_HEAP_FreeMemory( p_sys );
}


//----------------------------------------------------------------------------
/**
 *	@brief	レールローダー　情報読み込み
 *
 *	@param	p_sys				システム
 *	@param	datano			データナンバー
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_LOADER_Load( FIELD_RAIL_LOADER* p_sys, u32 datano, u32 heapID )
{
	u32 all_size;
	u32 siz;
	OFS_DATA* ofs_data;
	
	GF_ASSERT( p_sys );
	GF_ASSERT( !p_sys->p_work );
	p_sys->p_work = GFL_ARC_UTIL_LoadEx( ARCID_RAIL_DATA, datano, FALSE, heapID, &all_size );
	p_sys->work_size	= all_size;

	TOMOYA_Printf( "rail_data size = %d\n", all_size );

	GF_ASSERT( p_sys->p_work->label == RAIL_DATA_HEADER_LABEL );
	
	// ポイントテーブル
	p_sys->setting.point_table = (RAIL_POINT*)(((u32)p_sys->p_work) + p_sys->p_work->pointdata_offset);
	siz = p_sys->p_work->linedata_offset - p_sys->p_work->pointdata_offset;
	siz /= sizeof(RAIL_POINT);
	p_sys->setting.point_count = siz;
	TOMOYA_Printf( "point_count = %d\n", p_sys->setting.point_count );

	// ラインテーブル
	p_sys->setting.line_table = (RAIL_LINE*)(((u32)p_sys->p_work) + p_sys->p_work->linedata_offset);
	siz = p_sys->p_work->cameradata_offset - p_sys->p_work->linedata_offset;
	siz /= sizeof(RAIL_LINE);
	p_sys->setting.line_count = siz;
	TOMOYA_Printf( "line_count = %d\n", p_sys->setting.line_count );

	// カメラセットテーブル
	p_sys->setting.camera_table = (RAIL_CAMERA_SET*)(((u32)p_sys->p_work) + p_sys->p_work->cameradata_offset);
	siz = p_sys->p_work->lineposdata_offset - p_sys->p_work->cameradata_offset;
	siz /= sizeof(RAIL_CAMERA_SET);
	p_sys->setting.camera_count = siz;
	TOMOYA_Printf( "camera_count = %d\n", p_sys->setting.camera_count );

	// 座標セットテーブル
	p_sys->setting.linepos_table = (RAIL_LINEPOS_SET*)(((u32)p_sys->p_work) + p_sys->p_work->lineposdata_offset);
	siz = p_sys->p_work->end_pos - p_sys->p_work->lineposdata_offset;
	siz /= sizeof(RAIL_LINEPOS_SET);
	p_sys->setting.linepos_count = siz;
	TOMOYA_Printf( "linepos_count = %d\n", p_sys->setting.linepos_count );
	

	// オフセットパラメータ
	ofs_data = (OFS_DATA*)(((u32)p_sys->p_work) + p_sys->p_work->ofsdata_offset);
	p_sys->setting.ofs_unit = ofs_data->ofs_unit;

	TOMOYA_Printf( "ofs_unit = %d\n", p_sys->setting.ofs_unit );



#ifdef PM_DEBUG
	{
		int i;
		
		for( i=0; i<p_sys->setting.point_count; i++ )
		{
			PrintPoint( &p_sys->setting.point_table[i] );
		}
		for( i=0; i<p_sys->setting.line_count; i++ )
		{
			PrintLine( &p_sys->setting.line_table[i] );
		}
		for( i=0; i<p_sys->setting.camera_count; i++ )
		{
			PrintCamera( &p_sys->setting.camera_table[i] );
		}
		for( i=0; i<p_sys->setting.linepos_count; i++ )
		{
			PrintLinePos( &p_sys->setting.linepos_table[i] );
		}
	}
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	ローダー	情報クリア
 *
 *	@param	p_sys			システム
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_LOADER_Clear( FIELD_RAIL_LOADER* p_sys )
{
	GF_ASSERT( p_sys );
	if( p_sys->p_work )
	{
		GFL_HEAP_FreeMemory( p_sys->p_work );
		p_sys->p_work = NULL;
	}
	
	// 関数配列の設定
	SettingClear( &p_sys->setting );
}


//----------------------------------------------------------------------------
/**
 *	@brief	ローダー	レール情報取得
 *
 *	@param	cp_sys	システム
 *
 *	@return	レール情報
 */
//-----------------------------------------------------------------------------
const RAIL_SETTING* FIELD_RAIL_LOADER_GetData( const FIELD_RAIL_LOADER* cp_sys )
{
	GF_ASSERT( cp_sys );
	return &cp_sys->setting;
}


// デバック機能
#ifdef PM_DEBUG
void FIELD_RAIL_LOADER_DEBUG_LoadBinary( FIELD_RAIL_LOADER* p_sys, void* p_dat, u32 size )
{
	u32 siz;
	OFS_DATA* ofs_data;
	
	GF_ASSERT( p_sys );
	GF_ASSERT( !p_sys->p_work );
	p_sys->p_work = p_dat;

	p_sys->work_size	= size;

	GF_ASSERT( p_sys->p_work->label == RAIL_DATA_HEADER_LABEL );
	
	// ポイントテーブル
	p_sys->setting.point_table = (RAIL_POINT*)(((u32)p_sys->p_work) + p_sys->p_work->pointdata_offset);
	siz = p_sys->p_work->linedata_offset - p_sys->p_work->pointdata_offset;
	siz /= sizeof(RAIL_POINT);
	p_sys->setting.point_count = siz;
	TOMOYA_Printf( "point_count = %d\n", p_sys->setting.point_count );

	// ラインテーブル
	p_sys->setting.line_table = (RAIL_LINE*)(((u32)p_sys->p_work) + p_sys->p_work->linedata_offset);
	siz = p_sys->p_work->cameradata_offset - p_sys->p_work->linedata_offset;
	siz /= sizeof(RAIL_LINE);
	p_sys->setting.line_count = siz;
	TOMOYA_Printf( "line_count = %d\n", p_sys->setting.line_count );

	// カメラセットテーブル
	p_sys->setting.camera_table = (RAIL_CAMERA_SET*)(((u32)p_sys->p_work) + p_sys->p_work->cameradata_offset);
	siz = p_sys->p_work->lineposdata_offset - p_sys->p_work->cameradata_offset;
	siz /= sizeof(RAIL_CAMERA_SET);
	p_sys->setting.camera_count = siz;
	TOMOYA_Printf( "camera_count = %d\n", p_sys->setting.camera_count );

	// 座標セットテーブル
	p_sys->setting.linepos_table = (RAIL_LINEPOS_SET*)(((u32)p_sys->p_work) + p_sys->p_work->lineposdata_offset);
	siz = p_sys->p_work->end_pos - p_sys->p_work->lineposdata_offset;
	siz /= sizeof(RAIL_LINEPOS_SET);
	p_sys->setting.linepos_count = siz;
	TOMOYA_Printf( "linepos_count = %d\n", p_sys->setting.linepos_count );
	

	// オフセットパラメータ
	ofs_data = (OFS_DATA*)(((u32)p_sys->p_work) + p_sys->p_work->ofsdata_offset);
	p_sys->setting.ofs_unit = ofs_data->ofs_unit;

	TOMOYA_Printf( "ofs_unit = %d\n", p_sys->setting.ofs_unit );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レール情報の取得
 *
 *	@param	cp_sys	ワーク
 *
 *	@return	レール情報
 */
//-----------------------------------------------------------------------------
void* FIELD_RAIL_LOADER_DEBUG_GetData( const FIELD_RAIL_LOADER* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->p_work;
}

//----------------------------------------------------------------------------
/**
 *	@brief	レール情報サイズの取得
 *
 *	@param	cp_sys	ワーク
 *
 *	@return	情報サイズ
 */
//-----------------------------------------------------------------------------
u32 FIELD_RAIL_LOADER_DEBUG_GetDataSize( const FIELD_RAIL_LOADER* cp_sys )
{
	GF_ASSERT( cp_sys );
	return cp_sys->work_size;
}

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
u32 FIELD_RAIL_LOADER_GetNearestPoint( const FIELD_RAIL_LOADER* cp_sys, const VecFx32 * pos)
{
  fx32 near_len;
  u32 count, near_count;
  u32 c_max = cp_sys->setting.point_count;
  const RAIL_POINT * point = cp_sys->setting.point_table;

  GF_ASSERT(c_max > 0);
  for (count = 0; count < c_max; count ++)
  {
    fx32 len = VEC_Distance(pos, &point[count].pos);
    if (count == 0 || near_len > len)
    { //初回か、短い距離の場合はセット
      near_len = len;
      near_count = count;
    }
  }
  return near_count;
}


