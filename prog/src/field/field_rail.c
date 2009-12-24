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
#include "field/field_dir.h"

#include "field_rail.h"
#include "field_rail_access.h"

#include "field_camera.h"

#ifdef PM_DEBUG

//#define DEBUG_RAIL_PRINT  // レールのデバック出力　（大量）

#endif // PM_DEBUG


#ifdef DEBUG_RAIL_PRINT

#define DEBUG_RAIL_Printf( ... )  OS_TPrintf( __VA_ARGS__ )

#else // DEBUG_RAIL_PRINT

#define DEBUG_RAIL_Printf( ... )  ((void)0)

#endif // DEBUG_RAIL_PRINT


//============================================================================================
//============================================================================================

// 歩きカウント
static u8 RAIL_COUNTUP[ RAIL_FRAME_MAX ] = 
{
  1,        //RAIL_FRAME_16,
  2,        //RAIL_FRAME_8,
  4,        //RAIL_FRAME_4,
  8,       //RAIL_FRAME_2,
  16,       //RAIL_FRAME_2,
};

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
//-------------------------------------
///	ラインスイッチ
//=====================================
typedef struct 
{
  // ライン、分岐スイッチ
  u16 flag[RAIL_LINE_SWITCH_BUFFER_MAX];
} RAIL_LINE_SWITCH;


//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
struct _FIELD_RAIL_WORK{  
  FIELD_RAIL_TYPE type;
  union { 
    void * dummy;
    const RAIL_POINT * point;
    const RAIL_LINE * line;
  };
  u16 active;
	u16 rail_index;	
  /// LINEにいる間の、LINE上でのオフセット位置
  s32 line_ofs;
  s32 line_ofs_max;
  /// LINEにいる間の幅オフセット位置(XZ平面上でLINEに垂直)
  s32 width_ofs;
  s32 width_ofs_max;  // 水平方向分割数
  fx32 ofs_unit;       // 1分割の距離

  RAIL_KEY key;

	/// レールデータ
	const RAIL_SETTING* rail_dat;
	const RAIL_LINE_SWITCH* line_switch;


	///<一定フレームでの動作用情報
	u8	  req_move;
	s8		ofs_move;
	u16		save_move_key;    // RAIL_KEY
	u16		save_move_frame;  // RAIL_FRAME

  /// 移動が起きた最新のキーバリュー
  u8 last_key;           // RAIL_KEY
  u8 last_frame;         // RAIL_FRAME
  u8 last_move;         // 最新の動作で、移動したのかチェック
  u8 last_pad;          // 予備
  
  VecFx32 pos;  // 現在座標
  RAIL_LOCATION now_location;

  // 過去の情報
  RAIL_LOCATION last_location;

  const FIELD_RAIL_MAN* cp_man;
};


//------------------------------------------------------------------
/**
 * @brief
 */
//------------------------------------------------------------------
struct _FIELD_RAIL_MAN{
  HEAPID heapID;

  u16 active_flag;
  u16 user_active_flag;

  FIELD_RAIL_WORK * rail_work;
  int work_num;

  // カメラ情報
  FIELD_CAMERA * field_camera;
  const FIELD_RAIL_WORK * camera_bind_work;
  BOOL camera_update; // カメラ強制アップデート

  // ロケーションのみでの計算用
  FIELD_RAIL_WORK* calc_work;

  /// ライン、ポイントテーブル
	RAIL_SETTING rail_dat;

  /// ラインスイッチ
  RAIL_LINE_SWITCH line_switch;
};

//============================================================================================
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
// 中途半端な位置を求める
static BOOL RailMan_CalcRailKeyAndOfs( const FIELD_RAIL_MAN * man, const RAIL_LOCATION * now_location, RAIL_KEY key, u32 add_line_ofs, VecFx32* pos );

//------------------------------------------------------------------
//------------------------------------------------------------------
static int RailKeyTokeyCont(RAIL_KEY key);
static RAIL_KEY keyContToRailKey(int cont);
static RAIL_KEY getReverseKey(RAIL_KEY key);
static RAIL_KEY getClockwiseKey(RAIL_KEY key);
static RAIL_KEY getAntiClockwiseKey(RAIL_KEY key);

//------------------------------------------------------------------
//------------------------------------------------------------------
static void calcKeyLocation( const FIELD_RAIL_WORK * work, RAIL_KEY key, RAIL_LOCATION* ans );


//------------------------------------------------------------------
// ラインスイッチ
//------------------------------------------------------------------
static void initLineSwitch( RAIL_LINE_SWITCH* work );
static void onLineSwitch( RAIL_LINE_SWITCH* work, u16 line_index );
static void offLineSwitch( RAIL_LINE_SWITCH* work, u16 line_index );
static BOOL getLineSwitch( const RAIL_LINE_SWITCH* work, u16 line_index );
static BOOL getLineSwitchPointer( const RAIL_LINE_SWITCH* work, const RAIL_LINE* line, const RAIL_SETTING* rail_dat );
static const RAIL_LINE* getLineSwitchRailDatLine( const RAIL_LINE_SWITCH* work, const RAIL_SETTING * raildat, u32 index );
static void calcLineSwitchLineOfs( const RAIL_LINE_SWITCH* work, FIELD_RAIL_WORK * railwk, s32 front_ofs, s32 side_ofs );
static BOOL IsLineSwitchMoveOk( const RAIL_LINE_SWITCH* work, const RAIL_LINE * line, int l_ofs, int w_ofs, const RAIL_SETTING * raildat, fx32 ofs_unit );


//------------------------------------------------------------------
//------------------------------------------------------------------
static void initRail(FIELD_RAIL_WORK * work, const RAIL_SETTING* rail_dat, const FIELD_RAIL_MAN* cp_man );
static void clearRail( FIELD_RAIL_WORK * work, const RAIL_SETTING* rail_dat );
static BOOL isValidRail(const FIELD_RAIL_WORK * work);
static const RAIL_CAMERA_SET * getCameraSet(const FIELD_RAIL_WORK * work);
static const char * getRailName(const FIELD_RAIL_WORK * work);
static void getRailPosition(const FIELD_RAIL_WORK * work, VecFx32 * pos);
static void getRailLocation( const FIELD_RAIL_WORK * work, RAIL_LOCATION * location );
static s32 getLineOfsMax( const RAIL_LINE * line, fx32 unit, const RAIL_SETTING* rail_dat );
static s32 getLineWidthOfsMax( const RAIL_LINE * line, s32 line_ofs, u32 line_ofs_max, const RAIL_SETTING* rail_dat );
static RAIL_KEY updateLine( FIELD_RAIL_WORK * work, const RAIL_LINE * line, u32 line_ofs_max, u32 key );
static RAIL_KEY setLine(FIELD_RAIL_WORK * work, const RAIL_LINE * line, RAIL_KEY key, int l_ofs, int w_ofs, int l_ofs_max, int w_ofs_max);
static void setLineData(FIELD_RAIL_WORK * work, const RAIL_LINE * line, RAIL_KEY key, int l_ofs, int w_ofs, int l_ofs_max, int w_ofs_max);
static BOOL isLinePoint_S( const RAIL_SETTING * rail_dat, const RAIL_LINE * line, const RAIL_POINT * point );
static BOOL isLinePoint_E( const RAIL_SETTING * rail_dat, const RAIL_LINE * line, const RAIL_POINT * point );
static void getLineCrossAreaSize( const RAIL_SETTING * rail_dat, const RAIL_LINE * line, s32* p_size_line, s32* p_size_width, const RAIL_LINE_SWITCH* line_switch, fx32 unit );

//------------------------------------------------------------------
//------------------------------------------------------------------
static void clearRailDat( RAIL_SETTING * raildat );
static void initRailDat( RAIL_SETTING * raildat, const RAIL_SETTING * setting );
static const RAIL_POINT* getRailDatPoint( const RAIL_SETTING * raildat, u32 index );
static const RAIL_LINE* getRailDatLine( const RAIL_SETTING * raildat, u32 index );
static const RAIL_CAMERA_SET* getRailDatCamera( const RAIL_SETTING * raildat, u32 index );
static const RAIL_LINEPOS_SET* getRailDatLinePos( const RAIL_SETTING * raildat, u32 index );
static RAIL_CAMERA_FUNC*const getRailDatCameraFunc( const RAIL_SETTING * raildat, u32 index );
static RAIL_POS_FUNC*const getRailDatLinePosFunc( const RAIL_SETTING * raildat, u32 index );
static RAIL_LINE_DIST_FUNC*const getRailDatLineDistFunc( const RAIL_SETTING * raildat, u32 index );
static RAIL_LINE_HIT_LOCATION_FUNC*const getRailDatLineHitLocationFunc( const RAIL_SETTING * raildat, u32 index );

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL debugCheckLineData(const RAIL_LINE * line, const RAIL_SETTING* rail_dat);
static BOOL debugCheckPointData(const RAIL_POINT * point, const RAIL_SETTING* rail_dat);
static void debugPrintPoint(const char * before, const RAIL_POINT * point, const char * after);
static const char * debugGetRailKeyName(RAIL_KEY key);
static void debugPrintRailInfo(const FIELD_RAIL_WORK * work);


static RAIL_KEY updateLineMove_new(FIELD_RAIL_WORK * work, RAIL_KEY key, u32 count_up);
//static RAIL_KEY updateLineMove(FIELD_RAIL_WORK * work, RAIL_KEY key);
static RAIL_KEY updatePointMove(FIELD_RAIL_WORK * work, RAIL_KEY key, u32 count_up);


static u32 getPointIndex( const RAIL_SETTING * raildat, const RAIL_POINT* point );
static u32 getLineIndex( const RAIL_SETTING * raildat, const RAIL_LINE* line );


static BOOL calcHitPlaneVec( const VecFx32* plane_vec1, const VecFx32* plane_vec2, const VecFx32* plane_pos, const VecFx32* vec_s, const VecFx32* vec_e, VecFx32* cross );



//============================================================================================
//============================================================================================
static void RAIL_LOCATION_Dump(const RAIL_LOCATION * railLoc)
{
  TAMADA_Printf("RAIL_LOC:type = %d, rail_index = %d\n",railLoc->type, railLoc->rail_index);
  TAMADA_Printf("RAIL_LOC:line_grid = %d, width_grid = %d\n", railLoc->line_grid, railLoc->width_grid);
  TAMADA_Printf("RAIL_LOC:key = %s\n", debugGetRailKeyName(railLoc->key));
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief レール制御システムの生成
 */
//------------------------------------------------------------------
FIELD_RAIL_MAN * FIELD_RAIL_MAN_Create(HEAPID heapID, u32 work_num, FIELD_CAMERA * camera)
{ 
  int i;
  FIELD_RAIL_MAN * man = GFL_HEAP_AllocMemory(heapID, sizeof(FIELD_RAIL_MAN));

  man->heapID = heapID;
  man->active_flag = FALSE;
  man->user_active_flag = TRUE;
  man->field_camera = camera;
  man->camera_bind_work = NULL;

  // レールワークのメモリ確保
  man->rail_work = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_RAIL_WORK)*work_num );
  man->work_num = work_num;
  for( i=0; i<work_num; i++ )
  {
    initRail(&man->rail_work[i], &man->rail_dat, man);
  }

  // 計算用ワークも初期か
  man->calc_work = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_RAIL_WORK) );
  initRail(man->calc_work, &man->rail_dat, man);
  
  return man;
}

//------------------------------------------------------------------
/**
 * @brief レール制御システムの削除
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_Delete(FIELD_RAIL_MAN * man)
{ 
  GFL_HEAP_FreeMemory( man->calc_work );
  GFL_HEAP_FreeMemory( man->rail_work );
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
  man->active_flag = TRUE;

  // ラインスイッチのクリア
  initLineSwitch( &man->line_switch );

  // ポイント、ラインテーブル設定
	initRailDat( &man->rail_dat, setting );
}

//----------------------------------------------------------------------------
/**
 *	@brief  クリア
 *
 *	@param	man マネージャ
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_Clear(FIELD_RAIL_MAN * man)
{
  man->active_flag = FALSE;
  // ポイント、ラインテーブル設定
	clearRailDat( &man->rail_dat );
  
  // ラインスイッチのクリア
  initLineSwitch( &man->line_switch );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール情報の有無を確認
 *
 *	@param	man   マネージャ
 *
 *	@retval TRUE  ある
 *	@retval FALSE なし
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_IsLoad( const FIELD_RAIL_MAN * man )
{
  if( man->rail_dat.point_count == 0 )
  {
    return FALSE;
  }
  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  移動管理ワークの生成
 *
 *	@param	man   マネージャ
 *
 *	@return 移動管理ワーク
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_WORK* FIELD_RAIL_MAN_CreateWork( FIELD_RAIL_MAN * man )
{
  FIELD_RAIL_WORK* work;
  int i;

  // 空いているワークを探す
  work = NULL;
  for( i=0; i<man->work_num; i++ )
  {
    if( isValidRail( &man->rail_work[i] ) == FALSE )
    {
      work = &man->rail_work[i];
      break;
    }
  }
  // あきなし！
  GF_ASSERT( work );

  return work;
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動管理ワークの破棄
 *
 *	@param	man   マネージャ
 *	@param	work  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_DeleteWork( FIELD_RAIL_MAN * man, FIELD_RAIL_WORK* work )
{
  initRail( work, &man->rail_dat, man );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラにレールワークをバインドする
 *
 *	@param	man   マネージャ
 *	@param	work  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_BindCamera( FIELD_RAIL_MAN * man, const FIELD_RAIL_WORK* work )
{
  man->camera_bind_work = work;
  man->camera_update    = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラのレールワークのバインドをほどく
 *
 *	@param	man   マネージャ
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_UnBindCamera( FIELD_RAIL_MAN * man )
{
  man->camera_bind_work = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  バインドした、レールワークの座標取得
 *
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_GetBindWorkPos( const FIELD_RAIL_MAN * man, VecFx32* pos )
{
  GF_ASSERT( man );
  if(man->camera_bind_work)
  {
    FIELD_RAIL_WORK_GetPos( man->camera_bind_work, pos );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  バインドした、レールワークが動作しているかチェック
 *
 *	@retval TRUE  動作している
 *	@retval FALSE 動作していない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_IsBindWorkMove( const FIELD_RAIL_MAN * man )
{
  GF_ASSERT( man );
  if(man->camera_bind_work)
  {
    if( FIELD_RAIL_WORK_IsAction( man->camera_bind_work ) || FIELD_RAIL_WORK_IsLastAction( man->camera_bind_work ) )
    {
      return TRUE;
    }
  }
  return FALSE;
}

#ifdef PM_DEBUG
// デバック専用、バインドしたワークを編集可能な形で取得
FIELD_RAIL_WORK* FIELD_RAIL_MAN_DEBUG_GetBindWork( const FIELD_RAIL_MAN * man )
{
  return (FIELD_RAIL_WORK*)man->camera_bind_work;
}


// データを刷りかえる
void FIELD_RAIL_MAN_DEBUG_ChangeData(FIELD_RAIL_MAN * man, const RAIL_SETTING * setting)
{
  initRailDat( &man->rail_dat, setting );
}
#endif

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void FIELD_RAIL_MAN_SetActiveFlag(FIELD_RAIL_MAN * man, BOOL flag)
{
  man->user_active_flag = flag;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_GetActiveFlag(const FIELD_RAIL_MAN *man)
{
  return man->user_active_flag;
}

//------------------------------------------------------------------
//  全ワークのアップデート
//------------------------------------------------------------------
void FIELD_RAIL_MAN_Update(FIELD_RAIL_MAN * man)
{
  int i;


  if( !man->active_flag )
  {
    return ;
  }

  if( !man->user_active_flag )
  {
    return ;
  }

#if 0 // 各ワークごとにアップデートするように変更
  for( i=0; i<man->work_num; i++ )
  {
    FIELD_RAIL_WORK_Update( &man->rail_work[i] );
  }
#endif
}


//------------------------------------------------------------------
/**
 * @brief カメラ状態のアップデート
 *
 * @retval  TRUE  カメラ状態の更新
 * @retval  FALSE カメラ状態を変えなかった
 */
//------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_UpdateCamera(FIELD_RAIL_MAN * man)
{
  const FIELD_RAIL_WORK * work = man->camera_bind_work;
	const RAIL_CAMERA_SET* camera_set;
  RAIL_CAMERA_FUNC * func = NULL;

  if (!man->active_flag)
  {
    return FALSE;
  }

  if( !man->user_active_flag )
  {
    return FALSE;
  }
  
  if ( !work )
  {
    return FALSE;
  }

  if (isValidRail(work) == FALSE)
  {
    return FALSE;
  }

  // 動作したフレームかチェック
  if( (FIELD_RAIL_WORK_IsLastAction( work ) == FALSE) && (man->camera_update == FALSE) )
  {
    return FALSE;
  }


  switch (work->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    if (work->point->camera_set != RAIL_TBL_NULL)
    {
			camera_set = getRailDatCamera( work->rail_dat, work->point->camera_set );
      func = getRailDatCameraFunc( work->rail_dat, camera_set->func_index );
    }
    break;
  case FIELD_RAIL_TYPE_LINE:
    if (work->line->camera_set != RAIL_TBL_NULL)
    {
			camera_set = getRailDatCamera( work->rail_dat, work->line->camera_set );
      func = getRailDatCameraFunc( work->rail_dat, camera_set->func_index );
    }
    break;
  default:
    GF_ASSERT(0); //NO TYPE;
  }
  if (func)
  {
    func(man);
  }

  man->camera_update = FALSE;

  return TRUE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  ３Ｄ座標からヒットするレールロケーションを取得
 *
 *	@param	cp_man          レールマネージャ
 *	@param	cp_pos          座標
 *	@param	p_location      レールロケーション
 *	@param  p_locpos        ロケーションの座標
 *
 *	@retval TRUE    対応するレールロケーションがある
 *	@retval FALSE   対応するレールロケーションがない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_Calc3DPosRailLocation( const FIELD_RAIL_MAN * cp_man, const VecFx32* cp_pos, RAIL_LOCATION* p_location, VecFx32* p_locpos )
{
  int i;
  const RAIL_LINE* cp_line;
  const RAIL_POINT* cp_point_s;
  const RAIL_POINT* cp_point_e;
  BOOL result;
  VecFx32 pos;
  
  GF_ASSERT( cp_man );
  GF_ASSERT( cp_pos );
  GF_ASSERT( p_location );

  pos = *cp_pos;

  // Ｙ座標が誤差以内で、その点につながるライン上にいるのか（内積を使用する）。
  for( i=0; i<cp_man->rail_dat.line_count; i++ )
  {
    cp_line = getLineSwitchRailDatLine( &cp_man->line_switch, &cp_man->rail_dat, i );
    if( cp_line )
    {
      cp_point_s = getRailDatPoint( &cp_man->rail_dat, cp_line->point_s );
      cp_point_e = getRailDatPoint( &cp_man->rail_dat, cp_line->point_e );
      
      // 求める
      {
        RAIL_LINE_HIT_LOCATION_FUNC* func;
        func = getRailDatLineHitLocationFunc( &cp_man->rail_dat, cp_line->line_pos_set );
        if( func )
        {
          result = func( i, cp_man, &pos, p_location, p_locpos );
          // ヒット
          if( result )
          {
            return result;
          }
        }
      }
    }
  }

  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ３Ｄベクトルからヒットするレールロケーションを取得
 *  
 *	@param	cp_man        レールマネージャ
 *	@param	cp_startpos   開始座標
 *	@param	cp_endpos     終了座標
 *	@param	p_location    レールロケーション
 *
 *	@retval TRUE    対応するレールロケーションがある
 *	@retval FALSE   対応するレールロケーションがない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_Calc3DVecRailLocation( const FIELD_RAIL_MAN * cp_man, const VecFx32* cp_startpos, const VecFx32* cp_endpos, RAIL_LOCATION* p_location, VecFx32* p_locpos )
{
  int i;
  const RAIL_LINE* cp_line;
  const RAIL_POINT* cp_point_s;
  const RAIL_POINT* cp_point_e;
  const RAIL_LINEPOS_SET* cp_linepos;
  VecFx32 cross_pos, plane_vec1, plane_vec2;
  VecFx32 default_vec = {0, FX32_ONE, 0};
  RAIL_LOCATION min_location;
  VecFx32 min_pos;
  fx32 min_dist, dist;
  BOOL result;
  BOOL data_in;
  VecFx32 startpos, endpos;
  GF_ASSERT( cp_man );
  GF_ASSERT( cp_startpos );
  GF_ASSERT( cp_endpos );
  GF_ASSERT( p_location );

  startpos = *cp_startpos;
  endpos = *cp_endpos;

  // Ｙ座標が誤差以内で、その点につながるライン上にいるのか（内積を使用する）。
  data_in = FALSE;
  for( i=0; i<cp_man->rail_dat.line_count; i++ )
  {
    cp_line = getLineSwitchRailDatLine( &cp_man->line_switch, &cp_man->rail_dat, i );
    if( cp_line == NULL )
    {
      continue;
    }
    
    cp_point_s = getRailDatPoint( &cp_man->rail_dat, cp_line->point_s );
    cp_point_e = getRailDatPoint( &cp_man->rail_dat, cp_line->point_e );
    cp_linepos = getRailDatLinePos( &cp_man->rail_dat, cp_line->line_pos_set );

    DEBUG_RAIL_Printf( "\nlinename %s\n", cp_line->name );

    VEC_Subtract( &cp_point_s->pos, &cp_point_e->pos, &plane_vec1 );
    VEC_CrossProduct( &plane_vec1, &default_vec, &plane_vec2 );

    // 平面とベクトルの交点から、ヒット判定を行う。
    result = calcHitPlaneVec( &plane_vec1, &plane_vec2, &cp_point_e->pos, &startpos, &endpos, &cross_pos ); 

    if( result == FALSE )
    {
      continue;
    }

    // 求める
    {
	    RAIL_LINE_HIT_LOCATION_FUNC* func;
      func = getRailDatLineHitLocationFunc( &cp_man->rail_dat, cp_linepos->func_index );
      if( func )
      {
        result = func( i, cp_man, &cross_pos, p_location, p_locpos );
        // ヒット
        if( result )
        {
          // 一番近いポジションの情報を使う
          if( data_in == FALSE )
          {
            min_location = *p_location;
            min_pos = *p_locpos;
            min_dist = VEC_Distance( &startpos, p_locpos );

            data_in = TRUE;
          }
          else
          {
            // どっちがちかい？
            dist = VEC_Distance( &startpos, p_locpos );
            if( dist < min_dist )
            {
              min_location = *p_location;
              min_pos = *p_locpos;
              min_dist = dist;
            }
          }
        }
      }
    }
  }

  if( data_in )
  {
    *p_location = min_location;
    *p_locpos = min_pos;
  }

  return data_in;
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールラインのアクティブ設定
 *
 *	@param	man       マネージャ
 *	@param	index     ラインインデックス
 *	@param	flag      フラグ
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_SetLineActive( FIELD_RAIL_MAN * man, u32 index, BOOL flag )
{
  if( flag == FALSE )
  {
    // ラインを非アクティブに
    if( getLineSwitch(&man->line_switch, index) == FALSE )
    {
      onLineSwitch( &man->line_switch, index );
    }
  }
  else
  {
    // ラインをアクティブに
    offLineSwitch( &man->line_switch, index );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールラインのアクティブ状態を取得
 *
 *	@param	man     マネージャ
 *	@param	index   ラインインデックス
 *
 *	@retval TRUE  アクティブ
 *	@retval FALSE そのラインには分岐でいかない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_GetLineActive( const FIELD_RAIL_MAN * man, u32 index )
{
  if( getLineSwitch(&man->line_switch, index) == FALSE )
  {
    return TRUE;
  }
  return FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  レールグリッドのサイズを取得
 *
 *	@param	man   マネージャ
 *
 *	@return
 */
//-----------------------------------------------------------------------------
fx32 FIELD_RAIL_MAN_GetRailGridSize( const FIELD_RAIL_MAN * man )
{
  GF_ASSERT( man );

  return FX_Mul( man->rail_dat.ofs_unit, RAIL_WALK_OFS<<FX32_SHIFT );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションをキーの方向に１グリッド分進めた３D座標を取得
 *
 *	@param	man             マネージャ
 *	@param	now_location    今のロケーション
 *	@param	key             キー
 *	@param	pos             ３D座標
 *
 *	@retval TRUE  １グリッド進める
 *	@retval FALSE １グリッドすすめない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_CalcRailKeyPos(const FIELD_RAIL_MAN * man, const RAIL_LOCATION * now_location, RAIL_KEY key, VecFx32* pos)
{
  GF_ASSERT( man );
  GF_ASSERT( now_location );
  GF_ASSERT( pos );

  FIELD_RAIL_WORK_SetLocation( man->calc_work, now_location );
  FIELD_RAIL_WORK_ForwardReq( man->calc_work, RAIL_FRAME_1, key );
  FIELD_RAIL_WORK_Update( man->calc_work );
  FIELD_RAIL_WORK_GetPos( man->calc_work, pos );

  // 変化があったか？
  if( GFL_STD_MemComp( now_location, &man->calc_work->now_location, sizeof(RAIL_LOCATION) ) == 0 )
  {
    return FALSE;
  }

  return FIELD_RAIL_WORK_CheckLocation( man->calc_work, &man->calc_work->now_location );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションをキーの方向に１グリッド分進めたロケーションを取得    
 *
 *	@param	man             マネージャ
 *	@param	now_location    今のロケーション
 *	@param	key             キー
 *	@param	next_location   次のロケーション格納先
 *
 *	@retval TRUE  １グリッド進める
 *	@retval FALSE １グリッドすすめない  FALSEの場合でも、next_locationには進んだ先のロケーションが入っています。　しかし実際には、設定できないロケーションです。
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_MAN_CalcRailKeyLocation(const FIELD_RAIL_MAN * man, const RAIL_LOCATION * now_location, RAIL_KEY key, RAIL_LOCATION * next_location)
{
  BOOL move;
  RAIL_LOCATION save_location;
  
  GF_ASSERT( man );
  GF_ASSERT( now_location );
  GF_ASSERT( next_location );

  save_location = *now_location;

  FIELD_RAIL_WORK_SetLocation( man->calc_work, now_location );
  FIELD_RAIL_WORK_ForwardReq( man->calc_work, RAIL_FRAME_1, key );
  FIELD_RAIL_WORK_Update( man->calc_work );
  FIELD_RAIL_WORK_GetLocation( man->calc_work, next_location );

  // 変化があったか？
  if( GFL_STD_MemComp( &save_location, next_location, sizeof(RAIL_LOCATION) ) == 0 )
  {
    return FALSE;
  }
  return FIELD_RAIL_WORK_CheckLocation( man->calc_work, next_location );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションの３D座標を取得する
 *
 *	@param	man           マネージャ
 *	@param	cp_location   ロケーション
 *	@param	pos           位置
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_GetLocationPosition(const FIELD_RAIL_MAN * man, const RAIL_LOCATION * location, VecFx32* pos )
{
  GF_ASSERT( man );
  GF_ASSERT( location );

  FIELD_RAIL_WORK_SetLocation( man->calc_work, location );
  FIELD_RAIL_WORK_GetPos( man->calc_work, pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  locationのラインの全長グリッドサイズを取得
 *
 *	@param	man         まねーじゃ
 *	@param	location    ロケーション
 *
 *	@return グリッドサイズ
 */
//-----------------------------------------------------------------------------
u32 FIELD_RAIL_MAN_GetLocationLineOfsMaxGrid(const FIELD_RAIL_MAN * man, const RAIL_LOCATION * location )
{
  GF_ASSERT( man );
  GF_ASSERT( location );

  FIELD_RAIL_WORK_SetLocation( man->calc_work, location );
  return RAIL_OFS_TO_GRID( man->calc_work->line_ofs_max  );
}

//----------------------------------------------------------------------------
/**
 *	@brief  locationのラインの幅グリッドサイズを取得
 *
 *	@param	man       マネージャ
 *	@param	location  ロケーション
 *
 *	@return 幅サイズ
 */
//-----------------------------------------------------------------------------
u32 FIELD_RAIL_MAN_GetLocationWidthGrid(const FIELD_RAIL_MAN * man, const RAIL_LOCATION * location )
{
  GF_ASSERT( man );
  GF_ASSERT( location );

  FIELD_RAIL_WORK_SetLocation( man->calc_work, location );
  return RAIL_OFS_TO_GRID( man->calc_work->width_ofs_max );
}


//============================================================================================
//
//
//  レール移動制御のための関数群
//
//
//============================================================================================



//----------------------------------------------------------------------------
/**
 *	@brief  レール位置情報の取得
 *
 *	@param	work      ワーク
 *	@param	location  位置情報格納先
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_WORK_GetLocation(const FIELD_RAIL_WORK * work, RAIL_LOCATION * location)
{
  GF_ASSERT( work );
  GF_ASSERT( location );

  // ロケーションの取得
  *location = work->now_location;
  //RAIL_LOCATION_Dump(location);
}

//----------------------------------------------------------------------------
/**
 *	@brief  １つ前のロケーションの取得
 *
 *	@param	work        ワーク
 *	@param	location    ロケーション
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_WORK_GetLastLocation(const FIELD_RAIL_WORK * work, RAIL_LOCATION * location)
{
  GF_ASSERT( work );
  GF_ASSERT( location );

  // ロケーションの取得
  *location = work->last_location;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションの有効せいチェック
 *
 *	@param	work        ワーク
 *	@param	location    ロケーション
 *
 *	@retval TRUE    有効
 *	@retval FALSE   無効
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_WORK_CheckLocation( const FIELD_RAIL_WORK * work, const RAIL_LOCATION * location )
{
  GF_ASSERT( work );
  GF_ASSERT( location );


  if( location->type==FIELD_RAIL_TYPE_POINT )
  {
    // ポイントインデックス
    if( location->rail_index >= work->rail_dat->point_count )
    {
      return FALSE;
    }
  }
  else
  {
    const RAIL_LINE* line = NULL;
    s32 line_ofs;
    s32 width_ofs;
    s32 line_max;
    s32 width_max;
    
    // ラインインデックス
    if( location->rail_index >= work->rail_dat->line_count )
    {
      return FALSE;
    }

    line        = &work->rail_dat->line_table[ location->rail_index ];
    line_ofs    = RAIL_GRID_TO_OFS(location->line_grid);
    width_ofs   = RAIL_GRID_TO_OFS(location->width_grid);
    line_max    = getLineOfsMax( line, work->ofs_unit, work->rail_dat );
    width_max   = getLineWidthOfsMax( line, line_ofs, line_max, work->rail_dat );

    // 幅
    if( !((-width_max <= width_ofs) && (width_ofs <= width_max)) )
    {
      return FALSE;
    }

    // ライン
    if( !( (line_ofs <= line_max) && (line_ofs >= 0) ) )
    {
      return FALSE;
    }
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール位置情報の設定
 *
 *	@param	man       マネージャ
 *	@param	location  指定位置情報
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_WORK_SetLocation(FIELD_RAIL_WORK * work, const RAIL_LOCATION * location)
{
  int i;
  const RAIL_LINE* line = NULL;
  int width_ofs_max;
  u32 line_ofs_max;
  int line_ofs;
  int width_ofs;
  int key;

  // レール読み込まれているのか？
  GF_ASSERT( work->rail_dat );


  //RAIL_LOCATION_Dump(location);
  // 初期化
  clearRail( work, work->rail_dat );

  GF_ASSERT( location->type < FIELD_RAIL_TYPE_MAX );
  
  if( location->type==FIELD_RAIL_TYPE_POINT )
  {
    
    // 点初期化
    work->type          = FIELD_RAIL_TYPE_POINT;
    GF_ASSERT( work->rail_dat->point_count > location->rail_index );
    work->point         = &work->rail_dat->point_table[ location->rail_index ];

    work->width_ofs_max = work->point->width_ofs_max[0];

    for( i=0; i<RAIL_CONNECT_LINE_MAX; i++ )
    {
      if( work->point->lines[i] != RAIL_TBL_NULL )
      {
        if( line == NULL )
        {
          line = &work->rail_dat->line_table[ work->point->lines[i] ];
          if( line->point_s == location->rail_index )
          {
            line_ofs  = 0;
            width_ofs = 0;
            key       = work->point->keys[i];
          }
          else if( line->point_e == location->rail_index )
          {
            line_ofs  = getLineOfsMax( line, work->ofs_unit, work->rail_dat );
            width_ofs = 0;
            key       = work->point->keys[i];
          }
        }
        else
        {
          // 複数候補がある場合には、最初に見つかったラインを使用します。
          OS_TPrintf( "work::SetLocation Point ラインが複数あります。\n" );
        }
      }
    }
    GF_ASSERT( line );
#ifdef PM_DEBUG
    if( !line )
    {
      // ポイントに降り立つ（エディット用）
      work->type          = FIELD_RAIL_TYPE_POINT;
      work->point         = &work->rail_dat->point_table[ location->rail_index ];

      work->width_ofs_max = work->point->width_ofs_max[0];
      work->active = TRUE;
      return ;
    }
#endif
  }
  else
  {
    int line_grid;
    
    // ライン初期化
    GF_ASSERT( work->rail_dat->line_count > location->rail_index );
    line        = &work->rail_dat->line_table[ location->rail_index ];

    // line_gridはオーバーしないように調整
    // widthは、イベント配置などで、領域外のことがある
    if( location->line_grid >= line->line_grid_max )
    {
      line_grid = location->line_grid - 1;
    }
    else
    {
      line_grid = location->line_grid;
    }

    line_ofs    = RAIL_GRID_TO_OFS(line_grid);
    width_ofs   = RAIL_GRID_TO_OFS(location->width_grid);
    key         = location->key;
  }

  line_ofs_max  = getLineOfsMax( line, work->ofs_unit, work->rail_dat );
  width_ofs_max = getLineWidthOfsMax( line, line_ofs, line_ofs_max, work->rail_dat );

  
  setLineData( work, line, key, line_ofs, width_ofs, line_ofs_max, width_ofs_max );

  
#ifdef PM_DEBUG
  
#if 0
  // ラインのline_ofs_maxと幅の最大数を取得する
  {
    int i;
    int attr_max;
    int width_s, width_e;
    attr_max = 0;
    for( i=0; i<work->rail_dat->line_count; i++ )
    {
      line_ofs_max = getLineOfsMax( &work->rail_dat->line_table[i], work->ofs_unit, work->rail_dat );

      width_s = getLineWidthOfsMax( &work->rail_dat->line_table[i], 0, line_ofs_max, work->rail_dat );
      width_e = getLineWidthOfsMax( &work->rail_dat->line_table[i], line_ofs_max, line_ofs_max, work->rail_dat );

      line_ofs_max /= RAIL_WALK_OFS;
      width_s      /= RAIL_WALK_OFS;
      width_e      /= RAIL_WALK_OFS;

      OS_TPrintf( "lineNo[%d] ofs_max[%d] width_s[%d] width_e[%d]\n", i, line_ofs_max, width_s, width_e );
      
      if( width_e > width_s )
      {
        width_s = width_e;  // width_sのほうを最大にして求める
      }

      attr_max += ((width_s*2)+1) * line_ofs_max;
    }
    
    OS_TPrintf( "attr_max = [%d]\n", attr_max );
  }
#endif 

#endif
  
  // 方向設定
  work->last_frame = RAIL_FRAME_8;
  work->last_key = key;
  work->active = TRUE;


  // 座標の初期化
  getRailPosition(work, &work->pos);
  // ロケーションの初期化
  work->now_location = *location;
  work->last_location = *location;
}

//------------------------------------------------------------------
/**
 * @brief 現在位置の取得
 */
//------------------------------------------------------------------
void FIELD_RAIL_WORK_GetPos(const FIELD_RAIL_WORK * work, VecFx32 * pos)
{
  GF_ASSERT( work );
  *pos = work->pos;
}


//----------------------------------------------------------------------------
/**
 *	@brief	レールシステム　最新の動作を起こした時の動作フレーム
 *
 *	@param	man		
 *
 *	@return	キー情報
 */
//-----------------------------------------------------------------------------
RAIL_FRAME FIELD_RAIL_WORK_GetActionFrame( const FIELD_RAIL_WORK * work )
{
	GF_ASSERT( work );
	return work->last_frame;
}

//----------------------------------------------------------------------------
/**
 *	@brief	レールシステム　最後に移動した方向を取得
 *
 *	@param	man		
 *
 *	@return	方向
 */
//-----------------------------------------------------------------------------
RAIL_KEY FIELD_RAIL_WORK_GetActionKey( const FIELD_RAIL_WORK * work )
{
	GF_ASSERT( work );
	return work->last_key;
}

//----------------------------------------------------------------------------
/**
 *	@brief  今動作状態かチェック
 *
 *	@param	work  ワーク
 *
 *	@retval TRUE  動作状態
 *	@retval FALSE 停止状態
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_WORK_IsAction( const FIELD_RAIL_WORK * work )
{
  GF_ASSERT( work );

  return work->req_move;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールシステム  最新のUPDATEでいどうしたのか
 *
 *	@param	work 
 *  
 *	@retval TRUE  移動した
 *	@retval FALSE 移動しなかった
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_WORK_IsLastAction( const FIELD_RAIL_WORK * work )
{
	GF_ASSERT( work );
	return work->last_move;
}

//----------------------------------------------------------------------------
/**
 *	@brief  前方方向を取得する
 *
 *	@param	work    ワーク
 *	@param	way     方向格納先
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_WORK_GetFrontWay( const FIELD_RAIL_WORK * work, VecFx16* way )
{
  FIELD_RAIL_WORK* local_work;
  VecFx32 now_pos;
  GF_ASSERT( work );

  // 値変更後元に戻すので、constでＯＫとする
  local_work = (FIELD_RAIL_WORK*)work;

  // 今の位置の座標（width_ofsは０を求める）
  {
    s32 width_ofs_tmp = local_work->width_ofs;

    local_work->width_ofs = 0;

    // 座標を求める
    getRailPosition(local_work, &now_pos);

    local_work->width_ofs = width_ofs_tmp;
  }

  // 前の方向を求める
  {
    VecFx32 pos;
    s32 line_ofs_tmp = local_work->line_ofs;
    s32 width_ofs_tmp = local_work->width_ofs;
    if( (local_work->line_ofs + RAIL_WALK_OFS) < (local_work->line_ofs_max) ){
      local_work->line_ofs += RAIL_WALK_OFS;
      local_work->width_ofs = 0;

      // 座標を求める
      getRailPosition(local_work, &pos);

      // 元に戻す
      local_work->width_ofs = width_ofs_tmp;
      local_work->line_ofs = line_ofs_tmp;
    }else{

      // 1歩進んだ先のロケーションを取得し、その先で、あまりオフセット分進んだ場所の座標を求める
      {
        u32 add_ofs = (local_work->line_ofs + RAIL_WALK_OFS) - (local_work->line_ofs_max);
        RAIL_LOCATION now_location;

        getRailLocation( work, &now_location );
        now_location.width_grid = 0;
        
        RailMan_CalcRailKeyAndOfs( work->cp_man, &now_location, work->line->key, add_ofs, &pos );
      }
    }


    VEC_Subtract( &pos, &now_pos, &pos );
    VEC_Normalize( &pos, &pos );
    VEC_Fx16Set( way, pos.x, pos.y, pos.z );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  現状のラインの前方方向へ向かうキーを取得
 *
 *	@param	work  ワーク
 *
 *	@return キー
 */
//-----------------------------------------------------------------------------
RAIL_KEY FIELD_RAIL_WORK_GetFrontKey( const FIELD_RAIL_WORK * work )
{
  GF_ASSERT( work );
  GF_ASSERT( work->type == FIELD_RAIL_TYPE_LINE );
  GF_ASSERT( work->line );
  return work->line->key;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ワーク単位の動作設定
 *
 *	@param	work
 *	@param	flag 
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_WORK_SetActiveFlag( FIELD_RAIL_WORK * work, BOOL flag )
{
  GF_ASSERT(work);
  work->active = flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワーク動作チェック
 *
 *	@param	work  ワーク
 *
 *	@retval TRUE  動作
 *	@retval FALSE 動作してない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_WORK_IsActive( const FIELD_RAIL_WORK * work )
{
  GF_ASSERT(work);
  return work->active;
}




//----------------------------------------------------------------------------
/**
 *	@brief  1歩移動のリクエスト
 *
 *	@param	work      ワーク
 *	@param	frame     移動フレーム数
 *	@param	key       移動方向キー
 *
 *	@retval TRUE  リクエスト受理
 *	@retval FALSE 現在動作中なので、受理できない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_WORK_ForwardReq( FIELD_RAIL_WORK * work, RAIL_FRAME frame, RAIL_KEY key )
{
  // 動作していないとき
  if( !work->req_move )
  {
    work->req_move = TRUE;
    work->ofs_move = 0;
    work->save_move_key = key;
    work->save_move_frame = frame;

    return TRUE;
  }

  return FALSE;
}


//------------------------------------------------------------------
/**
 * @brief 現在位置のアップデート
 */
//------------------------------------------------------------------
void FIELD_RAIL_WORK_Update(FIELD_RAIL_WORK * work)
{ 
  if( !work->cp_man->active_flag )
  {
    return ;
  }
  
  if (!work->active)
  {
    return;
  }

  if (isValidRail(work) == FALSE)
  {
    return;
  }
  
  work->last_move   = FALSE;
	
	if( work->req_move )
  {
		FIELD_RAIL_TYPE type = work->type;
		RAIL_KEY set_key = RAIL_KEY_NULL;
		RAIL_KEY key = work->save_move_key;
		u32 count_up = RAIL_COUNTUP[ work->save_move_frame ];

		work->ofs_move += count_up;
		

		if(work->type == FIELD_RAIL_TYPE_LINE)
		{ //LINE上のときの移動処理
			set_key = updateLineMove_new(work, key, count_up);
		}
    else if(work->type == FIELD_RAIL_TYPE_POINT)
		{ //POINT上のときの移動処理
			TOMOYA_Printf( "count_up point %d\n", count_up );
      GF_ASSERT(0);
			//set_key = updatePointMove(work, key, count_up);
		}

		if (set_key != RAIL_KEY_NULL)
    {
			DEBUG_RAIL_Printf("RAIL:%s key:%s :line_ofs=%d line_ofs_max=%d width_ofs=%d width_ofs_max=%d\n",
          work->line->name,
					debugGetRailKeyName(set_key), 
          work->line_ofs, work->line_ofs_max, 
          work->width_ofs, 
          work->width_ofs_max );

      work->last_key    = set_key;
      work->last_frame  = work->save_move_frame;
      work->last_move   = TRUE;

      // 座標の取得
      getRailPosition(work, &work->pos);
		}

		if (type != work->type)
		{
			DEBUG_RAIL_Printf("RAIL:change to ");
			debugPrintRailInfo(work);
		}

    // リクエスト移動完了
		if( (work->ofs_move >= RAIL_WALK_OFS) || (set_key == RAIL_KEY_NULL) )
		{
			work->req_move = FALSE;

      // ロケーションの更新
      work->last_location = work->now_location;
      getRailLocation( work, &work->now_location );


#ifdef DEBUG_ONLY_FOR_tomoya_takahashi
      if( (work->line_ofs % RAIL_WALK_OFS) != 0 )
      {
        OS_TPrintf( "end l_ofs %d\n", work->line_ofs );
        GF_ASSERT( (work->line_ofs % RAIL_WALK_OFS) == 0 );
      }
      if( (MATH_ABS(work->width_ofs) % RAIL_WALK_OFS) != 0 )
      {
        OS_TPrintf( "end w_ofs %d\n", work->width_ofs );
        //GF_ASSERT( (MATH_ABS(work->width_ofs) % RAIL_WALK_OFS) == 0 );
      }
#endif // DEBUG_ONLY_FOR_tomoya_takahashi

      // line_ofsの調整
      if( (work->line_ofs % RAIL_WALK_OFS) != 0 )
      {
        if( (work->line_ofs % RAIL_WALK_OFS) > (RAIL_WALK_OFS/2) )
        {
          work->line_ofs += RAIL_WALK_OFS - (work->line_ofs % RAIL_WALK_OFS);
        }
        else
        {
          work->line_ofs -= (work->line_ofs % RAIL_WALK_OFS);
        }
      }
		}
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
const FIELD_RAIL_WORK* FIELD_RAIL_MAN_GetBindWork( const FIELD_RAIL_MAN * man )
{
	GF_ASSERT( man );
	return man->camera_bind_work;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ポイント情報の取得
 */
//-----------------------------------------------------------------------------
const RAIL_POINT* FIELD_RAIL_MAN_GetRailDatPoint( const FIELD_RAIL_MAN * man, u32 index )
{
  GF_ASSERT( man );
  return getRailDatPoint( &man->rail_dat, index );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ライン情報を取得
 */
//-----------------------------------------------------------------------------
const RAIL_LINE* FIELD_RAIL_MAN_GetRailDatLine( const FIELD_RAIL_MAN * man, u32 index )
{
  GF_ASSERT( man );
  return getRailDatLine( &man->rail_dat, index );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カメラ情報の取得
 */
//-----------------------------------------------------------------------------
const RAIL_CAMERA_SET* FIELD_RAIL_MAN_GetRailDatCamera( const FIELD_RAIL_MAN * man, u32 index )
{
  GF_ASSERT( man );
  return getRailDatCamera( &man->rail_dat, index );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポジション計算情報の取得
 */
//-----------------------------------------------------------------------------
const RAIL_LINEPOS_SET* FIELD_RAIL_MAN_GetRailDatLinePos( const FIELD_RAIL_MAN * man, u32 index )
{
  GF_ASSERT( man );
  return getRailDatLinePos( &man->rail_dat, index );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ユニットサイズの取得
 */
//-----------------------------------------------------------------------------
fx32 FIELD_RAIL_MAN_GetRailDatUnitSize( const FIELD_RAIL_MAN * man )
{
  GF_ASSERT( man );
  return man->rail_dat.ofs_unit;
}

//----------------------------------------------------------------------------
/**
 *	@brief  計算用　レールワークを取得する
 *
 *	@param	man   マネージャ
 *
 *	@return レールワーク
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_WORK* FIELD_RAIL_MAN_GetCalcRailWork( const FIELD_RAIL_MAN * man )
{
  GF_ASSERT( man );
  return man->calc_work;
}


//----------------------------------------------------------------------------
/**
 *	@brief  安心なロケーションの取得
 *
 *	@param	man           マネージャ
 *	@param	cp_location   変換前ロケーション
 *	@param	p_ans         安心なロケーション格納先
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_MAN_CalcSafeLocation( const FIELD_RAIL_MAN * man, const RAIL_LOCATION* cp_location, RAIL_LOCATION* p_ans )
{
  RAIL_LOCATION location;
  const RAIL_LINE* cp_line;
  s32 line_ofs;
  s32 width_ofs;
  s32 line_ofs_max;
  s32 width_ofs_max;
  
  GF_ASSERT( man );
  GF_ASSERT( cp_location );
  GF_ASSERT( p_ans );
  GF_ASSERT( cp_location->type == FIELD_RAIL_TYPE_LINE );
  GF_ASSERT( cp_location->rail_index < man->rail_dat.line_count );

  location = *cp_location;
  
  *p_ans = location;

  cp_line = &man->rail_dat.line_table[ location.rail_index ];

  line_ofs    = RAIL_GRID_TO_OFS(location.line_grid);
  width_ofs   = RAIL_GRID_TO_OFS(location.width_grid);
  line_ofs_max    = getLineOfsMax( cp_line, man->rail_dat.ofs_unit, &man->rail_dat );
  width_ofs_max   = getLineWidthOfsMax( cp_line, line_ofs, line_ofs_max, &man->rail_dat );

  if( line_ofs >= line_ofs_max )
  {
    line_ofs = line_ofs_max - 1;
    p_ans->line_grid = RAIL_OFS_TO_GRID( line_ofs );
  }

  if( MATH_ABS(width_ofs) > width_ofs_max )
  {
    if( width_ofs > 0 )
    {
      p_ans->width_grid = RAIL_OFS_TO_GRID( width_ofs_max );
    }
    else
    {
      p_ans->width_grid = RAIL_OFS_TO_GRID( -width_ofs_max );
    }
  }
}



//------------------------------------------------------------------
//  FIELD_RAIL_WORKアクセス関数
//------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	レールタイプの取得
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_TYPE FIELD_RAIL_GetType( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT( work );
	return work->type;
}

//----------------------------------------------------------------------------
/**
 *	@brief	レールポイントの取得
 */
//-----------------------------------------------------------------------------
const RAIL_POINT* FIELD_RAIL_GetPoint( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT( work );
	GF_ASSERT( work->type == FIELD_RAIL_TYPE_POINT );
	return work->point;
}

//----------------------------------------------------------------------------
/**
 *	@brief	レールラインの取得
 */
//-----------------------------------------------------------------------------
const RAIL_LINE* FIELD_RAIL_GetLine( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT( work );
	GF_ASSERT( work->type == FIELD_RAIL_TYPE_LINE );
	return work->line;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ラインオフセットの取得
 */
//-----------------------------------------------------------------------------
s32 FIELD_RAIL_GetLineOfs( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT( work );
	return work->line_ofs;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ラインオフセット最大値の取得
 */
//-----------------------------------------------------------------------------
s32 FIELD_RAIL_GetLineOfsMax( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT( work );
	return work->line_ofs_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	幅オフセットの取得
 */
//-----------------------------------------------------------------------------
s32 FIELD_RAIL_GetWidthOfs( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT( work );
	return work->width_ofs;
}

//----------------------------------------------------------------------------
/**
 *	@brief	幅オフセット最大値の取得
 */
//-----------------------------------------------------------------------------
s32 FIELD_RAIL_GetWidthOfsMax( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT( work );
	return work->width_ofs_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	移動単位の取得
 */
//-----------------------------------------------------------------------------
fx32 FIELD_RAIL_GetOfsUnit( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT( work );
	return work->ofs_unit;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラセットの取得
 */
//-----------------------------------------------------------------------------
const RAIL_CAMERA_SET* FIELD_RAIL_GetCameraSet( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT( work );
	return getCameraSet( work );
}




//------------------------------------------------------------------
//  RAIL_LINEアクセス関数
//------------------------------------------------------------------
const RAIL_POINT* FIELD_RAIL_GetPointStart( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT(work);
	GF_ASSERT( work->type == FIELD_RAIL_TYPE_LINE );
	return getRailDatPoint( work->rail_dat, work->line->point_s );
}
const RAIL_POINT* FIELD_RAIL_GetPointEnd( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT(work);
	GF_ASSERT( work->type == FIELD_RAIL_TYPE_LINE );
	return getRailDatPoint( work->rail_dat, work->line->point_e );
}
const RAIL_LINEPOS_SET* FIELD_RAIL_GetLinePosSet( const FIELD_RAIL_WORK* work )
{
	GF_ASSERT(work);
	GF_ASSERT( work->type == FIELD_RAIL_TYPE_LINE );
	return getRailDatLinePos( work->rail_dat, work->line->line_pos_set );
}

//------------------------------------------------------------------
//  RAIL_POINT
//------------------------------------------------------------------
const RAIL_CAMERA_SET* FIELD_RAIL_POINT_GetCameraSet( const FIELD_RAIL_WORK* work, const RAIL_POINT* point )
{
	GF_ASSERT( work );
	GF_ASSERT( point );
	return getRailDatCamera( work->rail_dat, point->camera_set);
}

//------------------------------------------------------------------
// 判定系ツール
//------------------------------------------------------------------
BOOL FIELD_RAIL_TOOL_HitCheckSphere( const VecFx32* person, const VecFx32* check, fx32 r )
{
  enum 
  { 
    HIT_HEIGHT  = FX32_ONE * 8,   // 縦の判定誤差
  };
  VecFx32 pos1, pos2;
  // 平面の距離
  fx32 len;
  fx32 dist_y;

  VEC_Set( &pos1, person->x, 0, person->z ); 
  VEC_Set( &pos2, check->x, 0, check->z ); 


  len = VEC_Distance( &pos1, &pos2 );
  dist_y = MATH_ABS( person->y - check->y );


  if( (dist_y < HIT_HEIGHT) && (len < FX_Mul( r, 2<<FX32_SHIFT ) ) )
  {
    return TRUE;
  }
  return FALSE;
}



#ifdef PM_DEBUG
// レールグリッドデバック出力
void FIELD_RAIL_WORK_DEBUG_PrintRailGrid( const FIELD_RAIL_WORK * work )
{
  GF_ASSERT( work );

  if( work->type == FIELD_RAIL_TYPE_LINE )
  {
    s32 width_ofs_max_s, width_ofs_max_e;
    s32 width_ofs_max;
    int index;
    int i;

    width_ofs_max_s = getLineWidthOfsMax( work->line, 0, work->line_ofs_max, work->rail_dat );
    width_ofs_max_e = getLineWidthOfsMax( work->line, work->line_ofs_max, work->line_ofs_max, work->rail_dat );
    
    if( width_ofs_max_s >= width_ofs_max_e )
    {
      width_ofs_max = width_ofs_max_s;
    }
    else
    {
      width_ofs_max = width_ofs_max_e;
    }

    // ラインインデックスを求める
    index = 0;
    for( i=0; i<work->rail_dat->line_count; i++ )
    {
      if( (u32)&work->rail_dat->line_table[i] == (u32)work->line )
      {
        index = i;
      }
    }
    
    OS_TPrintf("レールエディタ　アトリビュート設定用のデータ\n" );
    OS_TPrintf("RAIL:%s :front_grid=%d side_grid=%d \n",
        work->line->name, 
        RAIL_OFS_TO_GRID(work->line_ofs), 
        RAIL_OFS_TO_GRID(work->width_ofs) + RAIL_OFS_TO_GRID(width_ofs_max) );

    OS_TPrintf("イベント用のデータ\n" );
    OS_TPrintf("RAIL:%s Index:%d front_grid=%d side_grid=%d \n",
        work->line->name, index,
        RAIL_OFS_TO_GRID(work->line_ofs), 
        RAIL_OFS_TO_GRID(work->width_ofs) );
  }

}

void FIELD_RAIL_WORK_DEBUG_PrintRailOffset( const FIELD_RAIL_WORK * work )
{
  GF_ASSERT( work );

  if( work->type == FIELD_RAIL_TYPE_LINE )
  {
    s32 width_ofs_max_s, width_ofs_max_e;
    s32 width_ofs_max;
    int index;
    int i;

    width_ofs_max_s = getLineWidthOfsMax( work->line, 0, work->line_ofs_max, work->rail_dat );
    width_ofs_max_e = getLineWidthOfsMax( work->line, work->line_ofs_max, work->line_ofs_max, work->rail_dat );
    
    if( width_ofs_max_s >= width_ofs_max_e )
    {
      width_ofs_max = width_ofs_max_s;
    }
    else
    {
      width_ofs_max = width_ofs_max_e;
    }

    // ラインインデックスを求める
    index = 0;
    for( i=0; i<work->rail_dat->line_count; i++ )
    {
      if( (u32)&work->rail_dat->line_table[i] == (u32)work->line )
      {
        index = i;
      }
    }
    
    OS_TPrintf("RAIL:%s :front_offs=%d side_grid=%d \n",
        work->line->name, 
        work->line_ofs, 
        work->width_ofs );

  }
}

#endif



//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static const RAIL_LINE * RAILPOINT_getLineByKey(const RAIL_POINT * point, RAIL_KEY key, const RAIL_SETTING* rail_dat, const  RAIL_LINE_SWITCH* cp_lineswitch, BOOL switch_flag )
{
  int i;
  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
    if (point->keys[i] == key)
    {
      if( switch_flag )
      {
        return getLineSwitchRailDatLine( cp_lineswitch, rail_dat, point->lines[i] );
      }
      else
      {
        return getRailDatLine( rail_dat, point->lines[i] );
      }
    }
  }
  return NULL;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY setLine(FIELD_RAIL_WORK * work, const RAIL_LINE * line, RAIL_KEY key, int l_ofs, int w_ofs, int l_ofs_max, int w_ofs_max)
{
  GF_ASSERT(work->line != line);
  GF_ASSERT(work->line);
  GF_ASSERT(line->name);
  if (work->type == FIELD_RAIL_TYPE_LINE)
  {
    TAMADA_Printf("RAIL: LINE \"%s\" to %s \"%s\"\n",
        work->line->name, debugGetRailKeyName(key), line->name);
  }
  else
  {
    TAMADA_Printf("RAIL: POINT \"%s\" to %s \"%s\"\n",
        work->point->name, debugGetRailKeyName(key), line->name);
  }

  //  接続ラインスイッチチェック front_ofsが0である必要がある。
  if( IsLineSwitchMoveOk( work->line_switch, line, l_ofs, w_ofs, work->rail_dat, work->ofs_unit )==FALSE )
  {
    return RAIL_KEY_NULL;
  }

  // 横幅チェック
  if( MATH_ABS(w_ofs) > w_ofs_max )
  {
    return RAIL_KEY_NULL;
  }

  // 次のラインに乗れるか調べる
  if( (l_ofs >= l_ofs_max) || (l_ofs < 0) )
  {
    const RAIL_LINE* next_line;
    
    // 次のラインを取得
    if( (l_ofs >= l_ofs_max) )
    {
      // もしも、lofs以上でl_ofs_maxで、次のラインがあれば、そのラインに乗る
      const RAIL_POINT* nPoint = getRailDatPoint( work->rail_dat, line->point_e );
      next_line = RAILPOINT_getLineByKey(nPoint, line->key, work->rail_dat, work->line_switch, TRUE);
    }
    else
    {
      // もしも、lofsが０未満で、次のラインがあれば、そのラインに乗る
      const RAIL_POINT* nPoint = getRailDatPoint( work->rail_dat, line->point_s );
      next_line = RAILPOINT_getLineByKey(nPoint, getReverseKey(line->key), work->rail_dat, work->line_switch, TRUE);
    }

    // ないときは、移動不可能or通常設定
    if( next_line == NULL )
    {
      // 移動不可能！！！！！！
      if( l_ofs > l_ofs_max )
      {
        return RAIL_KEY_NULL;
      }
      // l_ofs == l_ofs_maxなら、そのまま設定する
    }
    else
    {

      //  次のラインに乗る！
      line = next_line;

      // 計算
      if( (l_ofs >= l_ofs_max) )
      {
        l_ofs -= l_ofs_max;

        l_ofs_max = getLineOfsMax( line, work->ofs_unit, work->rail_dat );
        w_ofs_max = getLineWidthOfsMax( line, l_ofs, l_ofs_max, work->rail_dat );
      }
      else if( l_ofs < 0 )
      {
        l_ofs_max = getLineOfsMax( line, work->ofs_unit, work->rail_dat );

        l_ofs = l_ofs_max + l_ofs;

        w_ofs_max = getLineWidthOfsMax( line, l_ofs, l_ofs_max, work->rail_dat );
      }

      
      if( l_ofs >= l_ofs_max )
      {
        return RAIL_KEY_NULL;
      }

      if( MATH_ABS(w_ofs) > w_ofs_max )
      {
        return RAIL_KEY_NULL;
      }
    }
  }


  // 通常
  setLineData( work, line, key, l_ofs, w_ofs, l_ofs_max, w_ofs_max );
  debugCheckLineData(line, work->rail_dat);
  return key;
}

// レールシステムにラインの情報を設定する
static void setLineData(FIELD_RAIL_WORK * work, const RAIL_LINE * line, RAIL_KEY key, int l_ofs, int w_ofs, int l_ofs_max, int w_ofs_max)
{
  GF_ASSERT( work );
  GF_ASSERT( line );
  work->type = FIELD_RAIL_TYPE_LINE;
  work->line = line;
  work->key = key;
  work->line_ofs = l_ofs;
  work->width_ofs = w_ofs;
  work->line_ofs_max = l_ofs_max;
  work->width_ofs_max = w_ofs_max;
}

// ラインの開始ポイントと、引数のpointが一致するかチェック
static BOOL isLinePoint_S( const RAIL_SETTING * rail_dat, const RAIL_LINE * line, const RAIL_POINT * point )
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
static BOOL isLinePoint_E( const RAIL_SETTING * rail_dat, const RAIL_LINE * line, const RAIL_POINT * point )
{
	const RAIL_POINT * point_e;

	point_e = getRailDatPoint( rail_dat, line->point_e );

	if( point_e == point )
	{
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  始点、分岐地点の範囲を求める
 *
 *	@param	rail_dat
 *	@param	line
 *	@param	start
 *	@param	p_size_line
 *	@param	p_size_width 
 */
//-----------------------------------------------------------------------------
static void getLineCrossAreaSize( const RAIL_SETTING * rail_dat, const RAIL_LINE * line, s32* p_size_line, s32* p_size_width, const RAIL_LINE_SWITCH* line_switch, fx32 unit )
{
  // 始点の情報を取得
  const RAIL_POINT* point;
  const RAIL_LINE* revers;
  const RAIL_LINE* left;
  const RAIL_LINE* right;
  s32 line_ofs_max;
  s32 width_ofs_max;

  s32 width_ofs_max_hi = 16;
  s32 line_ofs_max_hi = 16;

  point = getRailDatPoint( rail_dat, line->point_s );

  // 各分岐のライン幅の最大値の範囲の移動を許容する。
  // 逆方向、左、右を取得
  revers = RAILPOINT_getLineByKey(point, getReverseKey(line->key), rail_dat, line_switch, TRUE);
  left = RAILPOINT_getLineByKey(point, getAntiClockwiseKey(line->key), rail_dat, line_switch, TRUE);
  right = RAILPOINT_getLineByKey(point, getClockwiseKey(line->key), rail_dat, line_switch, TRUE);

  // サイズを求める
  // width
  // 自分
  line_ofs_max  = getLineOfsMax( line, unit, rail_dat );
  width_ofs_max_hi = getLineWidthOfsMax( line, 0, line_ofs_max, rail_dat );
  // revers
  if( revers )
  {
    line_ofs_max  = getLineOfsMax( revers, unit, rail_dat );
    width_ofs_max = getLineWidthOfsMax( revers, line_ofs_max, line_ofs_max, rail_dat );
    if( width_ofs_max > width_ofs_max_hi )
    {
      width_ofs_max_hi = width_ofs_max;
    }
  }

  // line
  // left
  if(left)
  {
    line_ofs_max  = getLineOfsMax( left, unit, rail_dat );
    if( isLinePoint_S(rail_dat, left, point) ){
      width_ofs_max = getLineWidthOfsMax( left, 0, line_ofs_max, rail_dat );
    }else{
      width_ofs_max = getLineWidthOfsMax( left, line_ofs_max, line_ofs_max, rail_dat );
    }
    line_ofs_max_hi = width_ofs_max;
  }
  //right
  if(right)
  {
    line_ofs_max  = getLineOfsMax( right, unit, rail_dat );
    if( isLinePoint_S(rail_dat, right, point) ){
      width_ofs_max = getLineWidthOfsMax( right, 0, line_ofs_max, rail_dat );
    }else{
      width_ofs_max = getLineWidthOfsMax( right, line_ofs_max, line_ofs_max, rail_dat );
    }
    if( width_ofs_max > line_ofs_max_hi  )
    {
      line_ofs_max_hi = width_ofs_max;
    }
  }

  
  *p_size_line = line_ofs_max_hi;
  *p_size_width = width_ofs_max_hi;
  
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static void clearRailDat( RAIL_SETTING * raildat )
{
  GFL_STD_MemClear( raildat, sizeof(RAIL_SETTING) );
}
// レールデータ
static void initRailDat( RAIL_SETTING * raildat, const RAIL_SETTING * setting )
{
  GFL_STD_MemCopy( setting, raildat, sizeof(RAIL_SETTING) );
}
// ポイント情報取得
static const RAIL_POINT* getRailDatPoint( const RAIL_SETTING * raildat, u32 index )
{
	if(raildat->point_count > index)
	{
		return &raildat->point_table[ index ];	
	}
	GF_ASSERT( RAIL_TBL_NULL==index );
	return NULL;
}
// ライン情報取得
static const RAIL_LINE* getRailDatLine( const RAIL_SETTING * raildat, u32 index )
{
	if(raildat->line_count > index)
	{
		return &raildat->line_table[ index ];
	}
	GF_ASSERT( RAIL_TBL_NULL==index );
	return NULL;
}
// カメラ情報取得
static const RAIL_CAMERA_SET* getRailDatCamera( const RAIL_SETTING * raildat, u32 index )
{
	if(raildat->camera_count > index)
	{
		return &raildat->camera_table[ index ];
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return &RAIL_CAMERA_SET_Default;
}
// ライン位置計算情報取得
static const RAIL_LINEPOS_SET* getRailDatLinePos( const RAIL_SETTING * raildat, u32 index )
{
	if(raildat->linepos_count > index)
	{
		return &raildat->linepos_table[ index ];	
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return &RAIL_LINEPOS_SET_Default;
}
// カメラ位置計算関数取得
static RAIL_CAMERA_FUNC*const getRailDatCameraFunc( const RAIL_SETTING * raildat, u32 index )
{
	if( raildat->camera_func_count > index )
	{
		return raildat->camera_func[ index ];
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return NULL;
}
// 座標計算関数取得
static RAIL_POS_FUNC*const getRailDatLinePosFunc( const RAIL_SETTING * raildat, u32 index )
{
	if( raildat->linepos_func_count > index )
	{	
		return raildat->line_pos_func[ index ];
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return NULL;
}

// ラインと座標の当たり判定
static RAIL_LINE_HIT_LOCATION_FUNC*const getRailDatLineHitLocationFunc( const RAIL_SETTING * raildat, u32 index )
{
	if( raildat->line_hit_location_count > index )
	{	
		return raildat->line_hit_location_func[ index ];
	}
	GF_ASSERT( index == RAIL_TBL_NULL );
	return NULL;
}



//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY updateLineMove_new(FIELD_RAIL_WORK * work, RAIL_KEY key, u32 count_up)
{ //LINE上のときの移動処理
  const RAIL_LINE * nLine = work->line;
  s32 nLine_ofs_max = work->line_ofs_max; // 今のLINEのオフセット最大値
//  s32 nLine_ofs_max = getLineOfsMax( nLine, work->ofs_unit, work->rail_dat ); // 今のLINEのオフセット最大値
  s32 ofs_max;  // 各ラインのオフセット最大値
	s32 next_line_width_max;
	s32 now_line_width_max;
  RAIL_KEY result;

  if (key == nLine->key)
  {//正方向キーの場合
    const RAIL_POINT* nPoint = getRailDatPoint( work->rail_dat, nLine->point_e );
    const RAIL_LINE * front = RAILPOINT_getLineByKey(nPoint, key, work->rail_dat, work->line_switch, FALSE);
    const RAIL_LINE * left = RAILPOINT_getLineByKey(nPoint, getAntiClockwiseKey(key), work->rail_dat, work->line_switch, FALSE);
    const RAIL_LINE * right = RAILPOINT_getLineByKey(nPoint, getClockwiseKey(key), work->rail_dat, work->line_switch, FALSE);
    BOOL width_over = FALSE;
    //const RAIL_LINE * back = RAILPOINT_getLineByKey(nPoint, getReverseKey(key), work->rail_dat, work->line_switch);
    TAMADA_Printf("↑");
    calcLineSwitchLineOfs( work->line_switch, work, count_up, 0 );
    if (work->line_ofs < nLine_ofs_max) {   // nLine_ofs_max == 次のラインのline_ofs 0
      // 今の道幅チェック オーバーしてなければ、通常の更新
      now_line_width_max = getLineWidthOfsMax( nLine, work->line_ofs, nLine_ofs_max, work->rail_dat );
      if( now_line_width_max >= MATH_ABS(work->width_ofs) )
      {
        return key;
      }
      // オーバーしていたら、他の道に遷移する
      width_over = TRUE;
		}
    if ((front!=NULL) && (width_over == FALSE))
    { //正面移行処理
      debugCheckPointData(nPoint, work->rail_dat);
			// 幅チェック
			ofs_max = getLineOfsMax( front, work->ofs_unit, work->rail_dat );
			next_line_width_max = getLineWidthOfsMax( front, work->line_ofs - nLine_ofs_max, ofs_max, work->rail_dat );
      //TOMOYA_Printf( "↑ front\n" );
      result = setLine(work, front, key,
						/*l_ofs*/work->line_ofs - nLine_ofs_max,
						/*w_ofs*/work->width_ofs,
						/*ofs_max*/ofs_max,
            /*w_ofs_max*/next_line_width_max); //そのまま
			if( result != RAIL_KEY_NULL )
			{
				return result;
			}
    }
    if ( (left && work->width_ofs <= 0) )
    { //左側移行処理
      if ( isLinePoint_S( work->rail_dat, left, nPoint ) )
      { //始端の場合
        debugCheckPointData(nPoint, work->rail_dat);
        ofs_max = getLineOfsMax( left, work->ofs_unit, work->rail_dat );
			  next_line_width_max = getLineWidthOfsMax( left, -work->width_ofs, ofs_max, work->rail_dat );

        //TOMOYA_Printf( "↑ left s\n" );
        result = setLine(work, left, key,
            /*l_ofs*/-work->width_ofs,
            /*w_ofs*/(work->line_ofs-nLine_ofs_max),
            /*ofs_max*/ofs_max,
          /*w_ofs_max*/next_line_width_max); 
        if( result != RAIL_KEY_NULL )
        {
          return result;
        }
      }
      else if (isLinePoint_E( work->rail_dat, left, nPoint ))
      { //終端の場合
        debugCheckPointData(nPoint, work->rail_dat);
        ofs_max = getLineOfsMax( left, work->ofs_unit, work->rail_dat );
			  next_line_width_max = getLineWidthOfsMax( left, ofs_max+work->width_ofs, ofs_max, work->rail_dat );
        //TOMOYA_Printf( "↑ left e\n" );
        result = setLine(work, left, key,
            /*l_ofs*/ofs_max + work->width_ofs, //width_ofs < 0なので実質減算
            /*w_ofs*/-(work->line_ofs-nLine_ofs_max),
            /*ofs_max*/ofs_max,
          /*w_ofs_max*/next_line_width_max); 
        if( result != RAIL_KEY_NULL )
        {
          return result;
        }
      }
      else GF_ASSERT_MSG(width_over, "%sから%sへの接続異常\n", nLine->name, left->name);
    }
    if (right && work->width_ofs >= 0)
    { //右側移行処理
      if ( isLinePoint_S( work->rail_dat, right, nPoint ) )
      { //始端の場合
        debugCheckPointData(nPoint, work->rail_dat);
        ofs_max = getLineOfsMax( right, work->ofs_unit, work->rail_dat );
			  next_line_width_max = getLineWidthOfsMax( right, work->width_ofs, ofs_max, work->rail_dat );
        //TOMOYA_Printf( "↑ right s\n" );
        result = setLine(work, right, key,
            /*l_ofs*/work->width_ofs,
            /*w_ofs*/-(work->line_ofs-nLine_ofs_max),
            /*ofs_max*/ofs_max,
          /*w_ofs_max*/next_line_width_max); 
        if( result != RAIL_KEY_NULL )
        {
          return result;
        }
      }
			else if ( isLinePoint_E( work->rail_dat, right, nPoint ) )
      { //終端の場合
        debugCheckPointData(nPoint, work->rail_dat);
        ofs_max = getLineOfsMax( right, work->ofs_unit, work->rail_dat );
			  next_line_width_max = getLineWidthOfsMax( right, ofs_max - work->width_ofs, ofs_max, work->rail_dat );
        //TOMOYA_Printf( "↑ right e\n" );
        result = setLine(work, right, key,
            /*l_ofs*/ofs_max - work->width_ofs,
            /*w_ofs*/(work->line_ofs-nLine_ofs_max),
            /*ofs_max*/ofs_max,
          /*w_ofs_max*/next_line_width_max);
        if( result != RAIL_KEY_NULL )
        {
          return result;
        }
      }
      else GF_ASSERT_MSG(width_over, "%sから%sへの接続異常\n", nLine->name, right->name);
    }

    if( width_over )
    {
			return updateLine( work, nLine, nLine_ofs_max, key );
    }

    //行くあてがない…LINEそのまま
    if(work->line_ofs <= nLine_ofs_max)  // maxまではOK
    {
      return key;
    }
    work->line_ofs -= count_up;
    return RAIL_KEY_NULL;
  }
  else if (key == getReverseKey(nLine->key))
  {//逆方向キーの場合
    const RAIL_POINT* nPoint = getRailDatPoint( work->rail_dat, nLine->point_s );
    //const RAIL_LINE * front = RAILPOINT_getLineByKey(nPoint, key, work->rail_dat, work->line_switch);
    const RAIL_LINE * left = RAILPOINT_getLineByKey(nPoint, getClockwiseKey(key), work->rail_dat, work->line_switch, FALSE);
    const RAIL_LINE * right = RAILPOINT_getLineByKey(nPoint, getAntiClockwiseKey(key), work->rail_dat, work->line_switch, FALSE);
    const RAIL_LINE * back = RAILPOINT_getLineByKey(nPoint, key, work->rail_dat, work->line_switch, FALSE);
    BOOL width_over = FALSE;
    TAMADA_Printf("↓");
    calcLineSwitchLineOfs( work->line_switch, work, -count_up, 0 );
    if (work->line_ofs >= 0) {

      // 今の道幅チェック オーバーしてなければ、通常の更新
      now_line_width_max = getLineWidthOfsMax( nLine, work->line_ofs, nLine_ofs_max, work->rail_dat );
      if( now_line_width_max >= MATH_ABS(work->width_ofs) )
      {
        return key;
      }
      // オーバーしていたら、他の道に遷移する
      width_over = TRUE;
		}
    if ((back != NULL) && (width_over == FALSE))
    {//背面移行処理
      debugCheckPointData(nPoint, work->rail_dat);
      ofs_max = getLineOfsMax( back, work->ofs_unit, work->rail_dat );
			// 幅チェック
			next_line_width_max = getLineWidthOfsMax( back, ofs_max - MATH_ABS(work->line_ofs), ofs_max, work->rail_dat );
      
      //TOMOYA_Printf( "↓ back\n" );
      result = setLine(work, back, key,
          /*l_ofs*/ofs_max - MATH_ABS(work->line_ofs),	// ここには、必ず負の値が来る ofs_maxを使用する
          /*w_ofs*/work->width_ofs,
          /*ofs_max*/ofs_max,
          /*w_ofs_max*/next_line_width_max);
			if( result != RAIL_KEY_NULL )
			{
        return result;
			}
    }
    if (left && work->width_ofs <= 0)
    { //左側移行処理
			if ( isLinePoint_S( work->rail_dat, left, nPoint ) )
      { //始端の場合
        debugCheckPointData(nPoint, work->rail_dat);
        ofs_max = getLineOfsMax( left, work->ofs_unit, work->rail_dat );
			  next_line_width_max = getLineWidthOfsMax( left, -work->width_ofs, ofs_max, work->rail_dat );
        //TOMOYA_Printf( "↓ left s\n" );
        result = setLine(work, left, key,
            /*l_ofs*/-work->width_ofs,
            /*w_ofs*/work->line_ofs,
          /*ofs_max*/ofs_max,
          /*w_ofs_max*/next_line_width_max);
        if( result != RAIL_KEY_NULL )
        {
          return result;
        }
      }
			else if ( isLinePoint_E( work->rail_dat, left, nPoint ) )
      { //終端の場合
        debugCheckPointData(nPoint, work->rail_dat);
        ofs_max = getLineOfsMax( left, work->ofs_unit, work->rail_dat );
			  next_line_width_max = getLineWidthOfsMax( left, ofs_max+work->width_ofs, ofs_max, work->rail_dat );
        //TOMOYA_Printf( "↓ left e\n" );
        result = setLine(work, left, key,
            /*l_ofs*/ofs_max + work->width_ofs, //
            /*w_ofs*/-work->line_ofs,
          /*ofs_max*/ofs_max,
          /*w_ofs_max*/next_line_width_max);
        if( result != RAIL_KEY_NULL )
        {
          return result;
        }
      }
      else GF_ASSERT_MSG(width_over, "%sから%sへの接続異常\n", nLine->name, left->name);
    }
    if (right && work->width_ofs >= 0)
    { //右側移行処理
			if ( isLinePoint_S( work->rail_dat, right, nPoint ) )
      { //始端の場合
        debugCheckPointData(nPoint, work->rail_dat);
        ofs_max = getLineOfsMax( right, work->ofs_unit, work->rail_dat );
			  next_line_width_max = getLineWidthOfsMax( right, work->width_ofs, ofs_max, work->rail_dat );
        //TOMOYA_Printf( "↓ right s\n" );
        result = setLine(work, right, key,
            /*l_ofs*/work->width_ofs,
            /*w_ofs*/-work->line_ofs,
          /*ofs_max*/ofs_max,
          /*w_ofs_max*/next_line_width_max);
        if( result != RAIL_KEY_NULL )
        {
          return result;
        } 
      }
			if ( isLinePoint_E( work->rail_dat, right, nPoint ) )
      { //終端の場合
        debugCheckPointData(nPoint, work->rail_dat);
        ofs_max = getLineOfsMax( right, work->ofs_unit, work->rail_dat );
			  next_line_width_max = getLineWidthOfsMax( right, ofs_max - work->width_ofs, ofs_max, work->rail_dat );
        //TOMOYA_Printf( "↓ right e\n" );
        result = setLine(work, right, key,
            /*l_ofs*/ofs_max - work->width_ofs,
            /*w_ofs*/work->line_ofs,
          /*ofs_max*/ofs_max,
          /*w_ofs_max*/next_line_width_max);
        if( result != RAIL_KEY_NULL )
        {
          return result;
        }
      }
      else GF_ASSERT_MSG(width_over, "%sから%sへの接続異常\n", nLine->name, right->name);
    }

    //  幅オーバー処理
    if( width_over )
    {
			return updateLine( work, nLine, nLine_ofs_max, key );
    }

    //行くあてがない…LINEそのまま、減算を取り消し
    work->line_ofs += count_up;
    return RAIL_KEY_NULL;
  }
  else if (key == getClockwiseKey(nLine->key))
  {//時計回り隣方向キーの場合
    TAMADA_Printf("→");
    calcLineSwitchLineOfs( work->line_switch, work, 0, count_up );
    if( (work->width_ofs > 0) && ((work->width_ofs % RAIL_WALK_OFS) == 0) ) // 右サイドに移っていたら
    { 
      {
        const RAIL_POINT* nPoint = getRailDatPoint( work->rail_dat, nLine->point_s );
        const RAIL_LINE *right = RAILPOINT_getLineByKey(nPoint, key, work->rail_dat, work->line_switch, FALSE);
        if(right)
        {
          if ( isLinePoint_S( work->rail_dat, right, nPoint ) )
          {
            debugCheckPointData(nPoint, work->rail_dat);
            ofs_max = getLineOfsMax( right, work->ofs_unit, work->rail_dat );
            next_line_width_max = getLineWidthOfsMax( right, work->width_ofs, ofs_max, work->rail_dat );
            //TOMOYA_Printf( "→ right s s\n" );
            result = setLine(work, right, key,
                work->width_ofs,
                - work->line_ofs,
                ofs_max,
            /*w_ofs_max*/next_line_width_max);
            if( result != RAIL_KEY_NULL )
            {
              return result;
            }
          }
          if ( isLinePoint_E( work->rail_dat, right, nPoint ) )
          {
            debugCheckPointData(nPoint, work->rail_dat);
            ofs_max = getLineOfsMax( right, work->ofs_unit, work->rail_dat );
            next_line_width_max = getLineWidthOfsMax( right, ofs_max - work->width_ofs, ofs_max, work->rail_dat );
            
            //TOMOYA_Printf( "→ right e s\n" );
            result = setLine(work, right, key,
                ofs_max - work->width_ofs,
                work->line_ofs,
                ofs_max,
            /*w_ofs_max*/next_line_width_max);
            
            if( result != RAIL_KEY_NULL )
            {
              return result;
            }
          }
        }
      }
      {
        const RAIL_POINT* nPoint = getRailDatPoint( work->rail_dat, nLine->point_e );
        const RAIL_LINE *right = RAILPOINT_getLineByKey(nPoint, key, work->rail_dat, work->line_switch, FALSE);
        if(right)
        {
          if ( isLinePoint_S( work->rail_dat, right, nPoint ) )
          {
            debugCheckPointData(nPoint, work->rail_dat);
            ofs_max = getLineOfsMax( right, work->ofs_unit, work->rail_dat );
            next_line_width_max = getLineWidthOfsMax( right, work->width_ofs, ofs_max, work->rail_dat );
            //TOMOYA_Printf( "→ right s e\n" );
            result = setLine(work, right, key,
                work->width_ofs,
                nLine_ofs_max - work->line_ofs,
                ofs_max,
            /*w_ofs_max*/next_line_width_max);
            if( result != RAIL_KEY_NULL )
            {
              return result;
            }
          }
          if ( isLinePoint_E( work->rail_dat, right, nPoint ) )
          {
            debugCheckPointData(nPoint, work->rail_dat);
            ofs_max = getLineOfsMax( right, work->ofs_unit, work->rail_dat );
            next_line_width_max = getLineWidthOfsMax( right, ofs_max - work->width_ofs, ofs_max, work->rail_dat );
            //TOMOYA_Printf( "→ right e e\n" );
            result = setLine(work, right, key,
                ofs_max - work->width_ofs,
                - (nLine_ofs_max - work->line_ofs),
                ofs_max,
            /*w_ofs_max*/next_line_width_max);
            if( result != RAIL_KEY_NULL )
            {
              return result;
            }	
          }
        }
      }
    }
    // -の間は自由に動ける
    if(work->width_ofs > 0)
    {
      // 移る場所がないなら、そのまま
      if (work->width_ofs <= work->width_ofs_max)
      {//範囲内の場合、終了
        return updateLine( work, nLine, nLine_ofs_max, key );
      }
    }

    // ライン外ですが、予測座標で計算する
    //中間地点の場合、行くあてがない場合…LINEそのまま、加算を取り消し
    //if( work->width_ofs < 0 )
    //{
    //  work->width_ofs = -work->width_ofs_max;
    //}
    //else
    //{
    //  work->width_ofs = work->width_ofs_max;
    //}
    //return RAIL_KEY_NULL;
    return key;
  }
  else if (key == getAntiClockwiseKey(nLine->key))
  {//反時計回り隣方向キーの場合
    TAMADA_Printf("←");
    calcLineSwitchLineOfs( work->line_switch, work, 0, -count_up );
    if( (work->width_ofs < 0) && ((MATH_ABS(work->width_ofs) % RAIL_WALK_OFS) == 0) )
    {
      {
        const RAIL_POINT* nPoint = getRailDatPoint( work->rail_dat, nLine->point_s );
        const RAIL_LINE* left = RAILPOINT_getLineByKey(nPoint, key, work->rail_dat, work->line_switch, FALSE);
        if(left)
        {
          if ( isLinePoint_S( work->rail_dat, left, nPoint ) )
          {
            debugCheckPointData(nPoint, work->rail_dat);
            ofs_max = getLineOfsMax( left, work->ofs_unit, work->rail_dat );
            next_line_width_max = getLineWidthOfsMax( left, MATH_ABS(work->width_ofs), ofs_max, work->rail_dat );
            //TOMOYA_Printf( "← left s s\n" );
            result = setLine(work, left, key,
                MATH_ABS(work->width_ofs), 
                work->line_ofs,
                ofs_max,
            /*w_ofs_max*/next_line_width_max);
            if( result != RAIL_KEY_NULL )
            {
              return result;
            }
          }
          if ( isLinePoint_E( work->rail_dat, left, nPoint ) )
          {
            debugCheckPointData(nPoint, work->rail_dat);
            ofs_max = getLineOfsMax( left, work->ofs_unit, work->rail_dat );
            next_line_width_max = getLineWidthOfsMax( left, ofs_max - MATH_ABS(work->width_ofs), ofs_max, work->rail_dat );
            //TOMOYA_Printf( "← left e s\n" );
            result = setLine(work, left, key,
                ofs_max - MATH_ABS(work->width_ofs),
                - work->line_ofs,
                ofs_max,
            /*w_ofs_max*/next_line_width_max);
            if( result != RAIL_KEY_NULL )
            {
              return result;
            }
          }
        }
      }
      {
        const RAIL_POINT* nPoint = getRailDatPoint( work->rail_dat, nLine->point_e );
        const RAIL_LINE* left = RAILPOINT_getLineByKey(nPoint, key, work->rail_dat, work->line_switch, FALSE);
        if(left)
        {
          if ( isLinePoint_S( work->rail_dat, left, nPoint ) )
          {
            debugCheckPointData(nPoint, work->rail_dat);
            ofs_max = getLineOfsMax( left, work->ofs_unit, work->rail_dat );
            next_line_width_max = getLineWidthOfsMax( left, MATH_ABS(work->width_ofs), ofs_max, work->rail_dat );
            //TOMOYA_Printf( "← left s e\n" );
            result = setLine(work, left, key,
                MATH_ABS(work->width_ofs),
                - (nLine_ofs_max - work->line_ofs),
                ofs_max,
            /*w_ofs_max*/next_line_width_max);
            if( result != RAIL_KEY_NULL )
            {
              return result;
            }
          }
          if ( isLinePoint_E( work->rail_dat, left, nPoint ) )
          {
            debugCheckPointData(nPoint, work->rail_dat);
            ofs_max = getLineOfsMax( left, work->ofs_unit, work->rail_dat );
            next_line_width_max = getLineWidthOfsMax( left, ofs_max - MATH_ABS(work->width_ofs), ofs_max, work->rail_dat );
            //TOMOYA_Printf( "← left e e\n" );
            result = setLine(work, left, key,
                ofs_max - MATH_ABS(work->width_ofs),
                nLine_ofs_max - work->line_ofs,
                ofs_max,
            /*w_ofs_max*/next_line_width_max);
            if( result != RAIL_KEY_NULL )
            {
              return result;
            }
          }
        }
      }
    }
    // +の間は自由に動ける
    if(work->width_ofs < 0)
    {
      // 移る場所がないならそのまま
      if (MATH_ABS(work->width_ofs) <= work->width_ofs_max)
      {//範囲内の場合、終了
        return updateLine( work, nLine, nLine_ofs_max, key );
      }
    }

    // ライン外ですが、予測座標で計算する
    //中間地点の場合、行くあてがない場合…LINEそのまま、減算を取り消し
    //if( work->width_ofs < 0 )
    //{
    //  work->width_ofs = -work->width_ofs_max;
    //}
    //else
    //{
    //  work->width_ofs = work->width_ofs_max;
    //}
    //return RAIL_KEY_NULL;
    return key;
  }
  return RAIL_KEY_NULL;
}

#if 0
//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY updateLineMove(FIELD_RAIL_WORK * work, RAIL_KEY key)
{ //LINE上のときの移動処理
  const RAIL_LINE * nLine = work->line;
  if (key == nLine->key)
  {//正方向キーの場合
    work->line_ofs ++;
    if (work->line_ofs == work->line_ofs_max)
    { // LINE --> point_e への移行処理
      work->type = FIELD_RAIL_TYPE_POINT;
      work->point = nLine->point_e;
      debugCheckPointData(work->point);
    }
    return key;
  }
  else if (key == getReverseKey(nLine->key))
  {//逆方向キーの場合
    work->line_ofs --;
    if (work->line_ofs == 0)
    { // LINE --> POINT_S への移行処理
      work->type = FIELD_RAIL_TYPE_POINT;
      work->point = nLine->point_s;
      debugCheckPointData(work->point);
    }
    return key;
  }
  else if (key == getClockwiseKey(nLine->key))
  {//時計回りとなり方向キーの場合
    if (work->width_ofs < work->width_ofs_max)
    {
      work->width_ofs ++;
    }
    return key;
  }
  else if (key == getAntiClockwiseKey(nLine->key))
  {//反時計回りとなり方向キーの場合
    if (work->width_ofs > - work->width_ofs_max)
    {
      work->width_ofs --;
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
static RAIL_KEY updatePointMove(FIELD_RAIL_WORK * work, RAIL_KEY key, u32 count_up)
{//POINT上のときの移動処理
  int i;
  const RAIL_POINT * nPoint = work->point;
  for (i = 0; i < RAIL_CONNECT_LINE_MAX; i++)
  {
    if (nPoint->keys[i] == key)
    { // POINT --> LINE[i] への移行処理
      const RAIL_LINE * nLine = getLineSwitchRailDatLine( work->line_switch, work->rail_dat, nPoint->lines[i] );
      s32 ofs_max;

      if( nLine )
      {
        // 
        ofs_max = getLineOfsMax( nLine, work->ofs_unit, work->rail_dat );
        
        if ( isLinePoint_S( work->rail_dat, nLine, nPoint ) )
        { 
          setLine(work, nLine, key, count_up, 0, ofs_max, nPoint->width_ofs_max[i]);
          //work->line_ofs = 1;  //最初は１から！
        }
        else if ( isLinePoint_E( work->rail_dat, nLine, nPoint ) )
        {
          setLine(work, nLine, key, ofs_max - count_up, 0, ofs_max, nPoint->width_ofs_max[i]);
          //work->line_ofs = nLine->ofs_max - 1; //最後ー1から！
        }
        //つながったLINEを次のRAILとしてセット
        //work->type = FIELD_RAIL_TYPE_LINE;
        //work->line = nLine;
        //debugCheckLineData(work->line, work->rail_dat);
        return key;
      }
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
static void initRail(FIELD_RAIL_WORK * work, const RAIL_SETTING* rail_dat, const FIELD_RAIL_MAN* cp_man )
{
  work->cp_man = cp_man;
  
  work->type = FIELD_RAIL_TYPE_MAX;
  work->dummy = NULL;
  work->line_ofs = 0;
  work->width_ofs = 0;
  work->key = RAIL_KEY_NULL;
	work->rail_dat      = rail_dat;
	work->line_switch   = &cp_man->line_switch;
  work->ofs_unit = rail_dat->ofs_unit;


  work->last_frame = 0;
  work->last_key = RAIL_KEY_NULL;

  work->req_move = FALSE;

  work->active = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール状態のクリア
 *
 *	@param	work
 *	@param	rail_dat 
 */
//-----------------------------------------------------------------------------
static void clearRail( FIELD_RAIL_WORK * work, const RAIL_SETTING* rail_dat )
{
  initRail( work, rail_dat, work->cp_man );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL isValidRail(const FIELD_RAIL_WORK * work)
{
  if (work->type < FIELD_RAIL_TYPE_MAX && work->dummy != NULL)
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
static const RAIL_CAMERA_SET * getCameraSet(const FIELD_RAIL_WORK * work)
{
  switch (work->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    return getRailDatCamera( work->rail_dat, work->point->camera_set );
    break;
  case FIELD_RAIL_TYPE_LINE:
    return getRailDatCamera( work->rail_dat, work->line->camera_set);
    break;
  }
  GF_ASSERT(0);
  return &RAIL_CAMERA_SET_Default;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void getRailPosition(const FIELD_RAIL_WORK * work, VecFx32 * pos)
{
  RAIL_POS_FUNC * func = NULL;
	const RAIL_LINEPOS_SET* line_pos_set;

  if(work->type == FIELD_RAIL_TYPE_POINT)
  {
    //POINTにいるときはPOINTの座標
    *pos = work->point->pos;
  }
  else if(work->type == FIELD_RAIL_TYPE_LINE)
  {
		line_pos_set = getRailDatLinePos( work->rail_dat, work->line->line_pos_set );
    func = getRailDatLinePosFunc( work->rail_dat, line_pos_set->func_index );
    if (func)
    {
			func(work, pos);
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールロケーションの取得
 *
 *	@param	work      ワーク
 *	@param	location  ロケーション格納先
 */
//-----------------------------------------------------------------------------
static void getRailLocation( const FIELD_RAIL_WORK * work, RAIL_LOCATION * location )
{
  location->type        = work->type;

  // インデックス取得
  if( work->type==FIELD_RAIL_TYPE_POINT ){
    location->rail_index  = getPointIndex( work->rail_dat, work->point );
  }else{
    location->rail_index  = getLineIndex( work->rail_dat, work->line );
  }

  // 各種オフセット
  location->line_grid  = RAIL_OFS_TO_GRID(work->line_ofs);
  location->width_grid = RAIL_OFS_TO_GRID(work->width_ofs);
  location->key       = work->key;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ラインのオフセット最大数を求める
 *
 *	@param	line    ライン情報
 *	@param	unit    １オフセットの距離
 *
 *	@return オフセット最大数(0～この値まで移動OK)
 */
//-----------------------------------------------------------------------------
static s32 getLineOfsMax( const RAIL_LINE * line, fx32 unit, const RAIL_SETTING* rail_dat )
{
  return line->line_grid_max * RAIL_WALK_OFS;
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
static s32 getLineWidthOfsMax( const RAIL_LINE * line, s32 line_ofs, u32 line_ofs_max, const RAIL_SETTING* rail_dat )
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

  // line_ofs_maxには、ならないので、-16して出来るだけ早く幅が合うようにする。
  line_ofs_max -= 16;

  // line_ofsがline_ofs_maxでくる可能性もなくはないので、対応しておく
  if( line_ofs > line_ofs_max )
  {
    line_ofs = line_ofs_max;
  }

  
  width_ofs_div = width_e - width_s;
  if(line_ofs_max > 0)
  {
    width_ofs_max = (width_ofs_div * (s32)line_ofs) / (s32)line_ofs_max;
  }
  else
  {
    width_ofs_max = width_ofs_div;
  }
  width_ofs_max += width_s;
	

	return width_ofs_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ライン動作　通常更新（幅チェックをする）
 *
 *	@param	work					レールシステム
 *	@param	line					進行中のライン
 *	@param	line_ofs_max	ラインオフセット最大値
 *	@param	key						押されているキー
 *
 *	@return	情報を変更したキー
 */
//-----------------------------------------------------------------------------
static RAIL_KEY updateLine( FIELD_RAIL_WORK * work, const RAIL_LINE * line, u32 line_ofs_max, u32 key )
{
  s32 width_ofs_max = getLineWidthOfsMax( line, work->line_ofs, line_ofs_max, work->rail_dat ); // 今のLINEのオフセット最大値

	work->width_ofs_max = width_ofs_max;
	if( MATH_ABS(work->width_ofs) >= width_ofs_max )
	{
		if(work->width_ofs > 0)
		{
			work->width_ofs = width_ofs_max;
		}
		else
		{
			work->width_ofs = -width_ofs_max;
		}
	}

	return key;
}



//----------------------------------------------------------------------------
/**
 *	@brief  レールを　キー　と　ofs分進めた先の座標を求める
 *
 *	@param	man
 *	@param	now_location
 *	@param	key
 *	@param	add_line_ofs
 *	@param	pos 
 */
//-----------------------------------------------------------------------------
static BOOL RailMan_CalcRailKeyAndOfs( const FIELD_RAIL_MAN * man, const RAIL_LOCATION * now_location, RAIL_KEY key, u32 add_line_ofs, VecFx32* pos )
{
  int i;
  
  GF_ASSERT( man );
  GF_ASSERT( now_location );
  GF_ASSERT( pos );

  GF_ASSERT( add_line_ofs <= RAIL_WALK_OFS );

  FIELD_RAIL_WORK_SetLocation( man->calc_work, now_location );
  FIELD_RAIL_WORK_ForwardReq( man->calc_work, RAIL_FRAME_1, key );
  FIELD_RAIL_WORK_Update( man->calc_work );

  FIELD_RAIL_WORK_ForwardReq( man->calc_work, RAIL_FRAME_16, key );
  for( i=0; i<RAIL_WALK_OFS; i++ )
  {
    if( add_line_ofs == i )
    {
      FIELD_RAIL_WORK_GetPos( man->calc_work, pos );
    }
    FIELD_RAIL_WORK_Update( man->calc_work );
  }

  // 変化があったか？
  if( GFL_STD_MemComp( now_location, &man->calc_work->now_location, sizeof(RAIL_LOCATION) ) == 0 )
  {
    return FALSE;
  }

  return FIELD_RAIL_WORK_CheckLocation( man->calc_work, &man->calc_work->now_location );
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static int RailKeyTokeyCont(RAIL_KEY key)
{
  static const u16 sc_KEYCONT[ RAIL_KEY_MAX ] = 
  {
    0,
    PAD_KEY_UP,
    PAD_KEY_RIGHT,
    PAD_KEY_DOWN,
    PAD_KEY_LEFT,
  };

  return sc_KEYCONT[ key ];
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


//----------------------------------------------------------------------------
/**
 *	@brief  keyに進んだ先のロケーションを取得する 移動できるかどうか判断しない
 *
 *	@param	work  ワーク
 *	@param	key   キー
 *	@param	ans   アンサー
 */
//-----------------------------------------------------------------------------
static void calcKeyLocation( const FIELD_RAIL_WORK * work, RAIL_KEY key, RAIL_LOCATION* ans )
{
  GF_ASSERT( work->type == FIELD_RAIL_TYPE_LINE );


  // ロケーション保存
  *ans = work->now_location;

  // Front
  if( work->line->key == key )
  {
    ans->line_grid ++;
  }
  // back
  else if( getReverseKey(work->line->key) == key )
  {
    if( ans->line_grid > 0 )
    {
      ans->line_grid --;
    }
    else
    {
      OS_TPrintf( "calcKeyLocation back not answer\n" );
    }
  }
  // left
  else if( getAntiClockwiseKey(work->line->key) == key )
  {
    ans->width_grid --;
  }
  // right
  else
  {
    ans->width_grid ++;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ラインスイッチバッファのクリア
 *
 *	@param	work  ワーク
 */
//-----------------------------------------------------------------------------
static void initLineSwitch( RAIL_LINE_SWITCH* work )
{
  int i;

  for( i=0; i<RAIL_LINE_SWITCH_BUFFER_MAX; i++ )
  {
    work->flag[i] = RAIL_TBL_NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ラインスイッチ設定
 *
 *	@param	work        ワーク
 *	@param	line_index  ラインインデックス
 */
//-----------------------------------------------------------------------------
static void onLineSwitch( RAIL_LINE_SWITCH* work, u16 line_index )
{
  int i;

  for( i=0; i<RAIL_LINE_SWITCH_BUFFER_MAX; i++ )
  {
    if( work->flag[i] == RAIL_TBL_NULL )
    {
      work->flag[i] = line_index;
      return ;
    }
  }
  
  // ラインスイッチの設定数オーバー
  GF_ASSERT( 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ラインスイッチ解除
 *
 *	@param	work        ワーク
 *	@param	line_index  ラインインデックス
 */
//-----------------------------------------------------------------------------
static void offLineSwitch( RAIL_LINE_SWITCH* work, u16 line_index )
{
  int i;

  for( i=0; i<RAIL_LINE_SWITCH_BUFFER_MAX; i++ )
  {
    if( work->flag[i] == line_index )
    {
      work->flag[i] = RAIL_TBL_NULL;
      return ;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ラインスイッチ　設定されているかチェック
 *
 *	@param	work        ワーク
 *	@param	line_index  ラインインデックス
 *
 *	@retval TRUE  設定されている
 *	@retval FALSE 設定されていない
 */
//-----------------------------------------------------------------------------
static BOOL getLineSwitch( const RAIL_LINE_SWITCH* work, u16 line_index )
{
  int i;

  for( i=0; i<RAIL_LINE_SWITCH_BUFFER_MAX; i++ )
  {
    if( work->flag[i] == line_index )
    {
      return TRUE;
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ラインスイッチ　設定されているかチェック
 *
 *	@param	const RAIL_LINE_SWITCH* work
 *	@param	RAIL_LINE* line
 *	@param	RAIL_SETTING* rail_dat 
 *
 *	@retval TRUE  設定されている
 *	@retval FALSE 設定されていない
 */
//-----------------------------------------------------------------------------
static BOOL getLineSwitchPointer( const RAIL_LINE_SWITCH* work, const RAIL_LINE* line, const RAIL_SETTING* rail_dat )
{
  int i;
  int index = 0;
  
  // ラインインデックスを求める
  for( i=0; i<rail_dat->line_count; i++ )
  {
    if( (u32)(&rail_dat->line_table[i]) == (u32)line )
    {
      index = i;
    }
  }

  return getLineSwitch( work, index );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ラインスイッチ  を使用した、ラインの取得関数
 *
 *	@param	work      ワーク
 *	@param	raildat   レール情報
 *	@param	index     ラインインデックス
 *
 *	@retval  ライン
 *	@retval NULL  スイッチにより、そのラインには分岐できない
 */
//-----------------------------------------------------------------------------
static const RAIL_LINE* getLineSwitchRailDatLine( const RAIL_LINE_SWITCH* work, const RAIL_SETTING * raildat, u32 index )
{
  if( getLineSwitch( work, index ) == FALSE )
  {
    return getRailDatLine( raildat, index );
  }
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  LineSwitchで管理された、ラインオフセットの制御
 *
 *	@param	work
 *	@param	railwk
 *	@param	front_ofs
 *	@param	side_ofs 
 *
 *	@retval TRUE  他のラインに乗るべき
 *	@retval FALSE 通常動作
 */
//-----------------------------------------------------------------------------
static void calcLineSwitchLineOfs( const RAIL_LINE_SWITCH* work, FIELD_RAIL_WORK * railwk, s32 front_ofs, s32 side_ofs )
{
  if( getLineSwitchPointer( work, railwk->line, railwk->rail_dat ) )
  {
    s32  size_line, size_width;
    
    // 移動可能範囲を求める
    // 移動可能範囲内でのみ移動できる。 
    // 始点は、色々なライン共通で使う分岐点なので、使用不可能でも通れる必要がある。
    getLineCrossAreaSize( railwk->rail_dat, railwk->line, &size_line, &size_width, work, railwk->ofs_unit );

    TOMOYA_Printf( "line area line %d  width %d\n",size_line, size_width );
    
    railwk->line_ofs += front_ofs;
    railwk->width_ofs += side_ofs;

    if( railwk->line_ofs > size_line )
    {
      railwk->line_ofs = size_line;
    }
    if( MATH_ABS( railwk->width_ofs ) > size_width )
    {
      if( railwk->width_ofs > 0 ){
        railwk->width_ofs = size_width;
      }else{
        railwk->width_ofs = -size_width;
      }
    }
  }
  else
  {
    railwk->line_ofs += front_ofs;
    railwk->width_ofs += side_ofs;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  移動可能なのかチェック
 *
 *	@param	work
 *	@param	line
 *	@param	l_ofs line ofs
 *	@param	w_ofs width ofs
 *
 *	@retval TRUE  移動可能
 *	@retval FALSE 移動不可能
 */
//-----------------------------------------------------------------------------
static BOOL IsLineSwitchMoveOk( const RAIL_LINE_SWITCH* work, const RAIL_LINE * line, int l_ofs, int w_ofs, const RAIL_SETTING * raildat, fx32 ofs_unit )
{
  if( getLineSwitchPointer( work, line, raildat ) )
  {
    s32  size_line, size_width;
    
    // 移動可能範囲を求める
    // 移動可能範囲内でのみ移動できる。 
    // 始点は、色々なライン共通で使う分岐点なので、使用不可能でも通れる必要がある。
    getLineCrossAreaSize( raildat, line, &size_line, &size_width, work, ofs_unit );

    if( (l_ofs < size_line) && (MATH_ABS(w_ofs) < size_width) )
    {
      return TRUE;
    }
    return FALSE;
  }
  return TRUE;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static const char * getRailName(const FIELD_RAIL_WORK * work)
{
  switch (work->type)
  {
  case FIELD_RAIL_TYPE_POINT:
    return work->point->name;
  case FIELD_RAIL_TYPE_LINE:
    return work->line->name;
  }
  return "NO ENTRY";
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static RAIL_KEY searchLineInPoint(const RAIL_POINT *point, const RAIL_LINE * line, const RAIL_SETTING* rail_dat)
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
static BOOL debugCheckLineData(const RAIL_LINE * line, const RAIL_SETTING* rail_dat)
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
static BOOL debugCheckPointData(const RAIL_POINT * point, const RAIL_SETTING* rail_dat)
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
    DEBUG_RAIL_Printf("%s",before);
  }
  DEBUG_RAIL_Printf("( %d, %d, %d)", FX_Whole(vec->x), FX_Whole(vec->y), FX_Whole(vec->z));
  if (after)
  {
    DEBUG_RAIL_Printf("%s",after);
  }
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void debugPrintHexVector(const VecFx32 * vec)
{
  DEBUG_RAIL_Printf("(%08x, %08x, %08x)\n", (vec->x), (vec->y), (vec->z));
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
    if (before) DEBUG_RAIL_Printf("%s", before);
    DEBUG_RAIL_Printf("( NULL )");
    if (after) DEBUG_RAIL_Printf("%s", after);
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
    DEBUG_RAIL_Printf("Degree: %08x\n",angle);
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
static void debugPrintRailInfo(const FIELD_RAIL_WORK * work)
{
  FIELD_RAIL_TYPE type = work->type;
  DEBUG_RAIL_Printf("%d(%s:%08x)\n",
      type,
      getRailName(work),
      work->dummy);
  if (type == FIELD_RAIL_TYPE_POINT)
  {
    debugPrintPoint(NULL, work->point ,"\n");
  } else if (type == FIELD_RAIL_TYPE_LINE)
  {
		const RAIL_POINT* point_s;
		const RAIL_POINT* point_e;

		point_s = getRailDatPoint( work->rail_dat, work->line->point_s );
		point_e = getRailDatPoint( work->rail_dat, work->line->point_e );
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
static u32 getPointIndex( const RAIL_SETTING * raildat, const RAIL_POINT* point )
{
  int i;

  for( i=0; i<raildat->point_count; i++ ){
    if( (u32)&raildat->point_table[i] == (u32)point ){
      return i;
    }
  }

  GF_ASSERT_MSG( i<raildat->point_count, "不明なpointです。" );
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
static u32 getLineIndex( const RAIL_SETTING * raildat, const RAIL_LINE* line )
{
  int i;

  for( i=0; i<raildat->line_count; i++ ){
    if( (u32)&raildat->line_table[i] == (u32)line ){
      return i;
    }
  }

  GF_ASSERT_MSG( i<raildat->line_count, "不明なlineです。" );
  return 0; // フリーズ回避
}


//----------------------------------------------------------------------------
/**
 *	@brief  平面と直線の交点を求める
 *
 *	@param	plane_vec1    平面ベクトル１
 *	@param	plane_vec2    平面ベクトル２
 *	@param  plane_pos     平面の一点
 *	@param	vec_s         直線始点
 *	@param	vec_e         直線終点
 *	@param	cross         交点格納先
 *
 *	@retval TRUE  交点がある
 *	@retval FALSE 交点はない
 */
//-----------------------------------------------------------------------------
static BOOL calcHitPlaneVec( const VecFx32* plane_vec1, const VecFx32* plane_vec2, const VecFx32* plane_pos, const VecFx32* vec_s, const VecFx32* vec_e, VecFx32* cross )
{
  VecFx32 normal;
  VecFx32 vector;
  VecFx32 plane_nvec1, plane_nvec2;
  fx32 d;
  fx32 cross_dist;
  fx32 de;
  fx32 vector_dist;


  DEBUG_RAIL_Printf( "\ncalcHitPlaneVec\n" );

  // ベクトルを求める
  VEC_Subtract( vec_e, vec_s, &vector );
  vector_dist = VEC_Mag( &vector );
  VEC_Normalize( &vector, &vector );
  
  // 平面の方程式を求める。
  // 法線を求め、Ｄの値を求める
  VEC_Normalize( plane_vec1, &plane_nvec1);  // 大きさはいらない
  VEC_Normalize( plane_vec2, &plane_nvec2);  // 大きさはいらない
  VEC_CrossProduct( &plane_nvec1, &plane_nvec2, &normal );
  VEC_Normalize( &normal, &normal);  // 大きさはいらない
  d = FX_Mul( normal.x, plane_pos->x ) + FX_Mul( normal.y, plane_pos->y ) + FX_Mul( normal.z, plane_pos->z );
  
  DEBUG_RAIL_Printf( "normal x[%d] y[%d] z[%d]\n", FX_Whole( normal.x ), FX_Whole( normal.y ), FX_Whole( normal.z ) );

  // 交点を求める。
  // P(交点) = A(始点) + (dist * Vec)
  // dist = {d - (n_a*Ax + n_b*Ay + n_c*Az)} / (n_a*Vecx + n_b*Vecy + n_c*Vecz)
  cross_dist = d - (FX_Mul( normal.x, vec_s->x ) + FX_Mul( normal.y, vec_s->y ) + FX_Mul( normal.z, vec_s->z ));
  de = (FX_Mul( normal.x, vector.x ) + FX_Mul( normal.y, vector.y ) + FX_Mul( normal.z, vector.z ));

  if( de == 0 )
  {
    DEBUG_RAIL_Printf( "de == 0\n" );
    return FALSE; // 交点なし
  }

  DEBUG_RAIL_Printf( "cross_dist[0x%x] de[0x%x]\n", cross_dist, de );

  cross_dist = FX_Div( cross_dist, de );
  

   
  // 交点はベクトルの先
  if( cross_dist > vector_dist )
  {
    DEBUG_RAIL_Printf( "cross_dist > vector_dist\n" );
    return FALSE;
  }

  // 交点はベクトルの先
  // cross_dist == FX32_ONE今いるラインの平面上！
  if( cross_dist <= FX32_ONE )
  {
    DEBUG_RAIL_Printf( "cross_dist <= FX32_ONE\n" );
    return FALSE;
  }

  DEBUG_RAIL_Printf( "cross_dist [%d]\n", FX_Whole( cross_dist ) );
  VEC_Set( cross, 
      vec_s->x + FX_Mul( vector.x, cross_dist ),
      vec_s->y + FX_Mul( vector.y, cross_dist ),
      vec_s->z + FX_Mul( vector.z, cross_dist ) );
  
  return TRUE;
}



