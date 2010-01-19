//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_rail_func.c
 *	@brief	フィールドレール　基本関数郡
 *	@author	tomoya takahashi
 *	@date		2009.07.06
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"

#include "field/fieldmap_proc.h"

#include "field_rail_access.h"
#include "field_rail_func.h"


#ifdef PM_DEBUG

//#define DEBUG_PRINT_ON  // 大量デバック表示

#endif

#ifdef DEBUG_PRINT_ON

#define DEBUG_RAIL_Printf( ... )  OS_TPrintf( __VA_ARGS__ )

#else

#define DEBUG_RAIL_Printf( ... )  ((void)0)

#endif

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	ロケーションあたり判定　Ｗｉｄｔｈ誤差値
//=====================================
#define HIT_LOCATION_WIDHT_AREA ( RAIL_WALK_OFS*1 ) // 4グリッド分



//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static void updateCircleCamera( const FIELD_RAIL_MAN * man, u16 pitch, fx32 len, const VecFx32* cp_target );

// 平面とチェックポジションの判定
static BOOL is_Hit3DPosPlane( const RAIL_POINT * cp_point_s, const RAIL_POINT * cp_point_e, const VecFx32* cp_pos, VecFx32* p_cross, fx32* dis_bcos, VecFx32* gaiseki );

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


//============================================================================================
//
//    座標演算関連
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void getVectorFromAngleValue(VecFx32 * vec, u16 yaw, u16 pitch, fx32 len)
{
  enum {  PITCH_LIMIT = 0x200 };
  VecFx32 cameraPos;

	fx16 sinYaw = FX_SinIdx(yaw);
	fx16 cosYaw = FX_CosIdx(yaw);
	fx16 sinPitch = FX_SinIdx(pitch);
	fx16 cosPitch = FX_CosIdx(pitch);

  // 裏周りするとカメラ上位置も変更する必要がある！！
  // 裏まわりしないようにマイナス値はプラス値に補正
	if(cosPitch < 0){ cosPitch = -cosPitch; }
  // 0値近辺は不正表示になるため補正
	if(cosPitch < PITCH_LIMIT){ cosPitch = PITCH_LIMIT; }

	VEC_Set( &cameraPos, FX_Mul(sinYaw, cosPitch), sinPitch, FX_Mul(cosYaw, cosPitch) );
	VEC_Normalize(&cameraPos, &cameraPos);
  {
    const VecFx32 zeroVec = {0, 0, 0};
    VEC_MultAdd(len, &cameraPos, &zeroVec, &cameraPos);
  }
  *vec = cameraPos;
}


//------------------------------------------------------------------
/**
 * @brief ベクトルとベクトルの間の球面線形補完をおこなう
 *
 * @param out     生成したベクトル
 * @param start   開始ベクトル
 * @param end     終了ベクトル
 * @param t       0～FX32_ONE
 *
 * 生成されるベクトルは正規化されているので注意
 */
//------------------------------------------------------------------
static void getInterNormalVector(VecFx32 * out, const VecFx32 * start, const VecFx32 * end, fx32 t)
{
  VecFx32 s, e;
  fx32 angle;   //-1～1
  u16 sinTheta; //radian index
  u16 Ps, Pe;   //radian index

  VEC_Normalize( start, &s );
  VEC_Normalize( end, &e );
  angle = FX_AcosIdx( VEC_DotProduct( &s, &e ) );
	sinTheta = FX_SinIdx( angle );
	
	if( (sinTheta == 0) )
	{
		// sinThetaが0　(変化する角度が少なすぎる)場合は、終了にすぐにする
		*out = e;
	}
	else
	{
		Ps = FX_SinIdx( FX_Mul(angle<<FX32_SHIFT, FX32_ONE - t)>>FX32_SHIFT );
		Pe = FX_SinIdx( FX_Mul(angle<<FX32_SHIFT, t)>>FX32_SHIFT );

		// out = ( Ps * s + Pe * e ) / sinTheta
		GFL_CALC3D_VEC_MulScalar(&s, FX_Div( Ps, sinTheta), &s);
		GFL_CALC3D_VEC_MulScalar(&e, FX_Div( Pe, sinTheta), &e);
		//GFL_CALC3D_VEC_MulScalar(&s, FX32_ONE * Ps / 0x10000, &s);
		//GFL_CALC3D_VEC_MulScalar(&e, FX32_ONE * Pe / 0x10000, &e);
		VEC_Add(&s, &e, out);
		//GFL_CALC3D_VEC_DivScalar(out, FX32_ONE * sinTheta / 0x10000, out);
		VEC_Normalize(out, out);
	}

}

//------------------------------------------------------------------
/**
 * @brief ベクトルとベクトルの間の線形補完（長さも補完）
 *
 * @param out     生成したベクトル
 * @param start   開始ベクトル
 * @param end     終了ベクトル
 * @param t       0～FX32_ONE
 */
//------------------------------------------------------------------
static void getIntermediateVector(VecFx32 * out, const VecFx32 * start, const VecFx32 * end, fx32 t)
{
  const VecFx32 zeroVec = {0, 0, 0};
  fx32 l_s, l_e, l_i;
  getInterNormalVector(out, start, end, t);

  // out = (|start| + (|end| - |start|) * t) * out
  l_s = VEC_Mag(start);
  l_e = VEC_Mag(end);
  l_i = l_s + FX_Mul((l_e - l_s) , t);
  VEC_MultAdd(l_i, out, &zeroVec, out);
}

//------------------------------------------------------------------
/**
 * @brief レール移動：点と点の間の直線補完
 */
//------------------------------------------------------------------
void FIELD_RAIL_POSFUNC_StraitLine(const FIELD_RAIL_WORK * work, VecFx32 * pos)
{
  //LINEにいるときはPOINTとPOINTの間の座標
  //pos = p0 + (p1 - p0) * (ofs / div )
  const RAIL_LINE * nLine = FIELD_RAIL_GetLine( work );
	s32 line_ofs = FIELD_RAIL_GetLineOfs( work );
	s32 line_ofs_max = FIELD_RAIL_GetLineOfsMax( work );
  fx32 ofs = FX_Div( (line_ofs * FX32_ONE), line_ofs_max<<FX32_SHIFT);
  VecFx32 val;
	const RAIL_POINT* point_s = FIELD_RAIL_GetPointStart( work );
	const RAIL_POINT* point_e = FIELD_RAIL_GetPointEnd( work );
  const VecFx32 * p0 = &point_s->pos;
  const VecFx32 * p1 = &point_e->pos;
  VEC_Subtract(p1, p0, &val);
  VEC_MultAdd(ofs, &val, p0, pos);

  {
    VecFx32 xzNormal = {0,FX32_ONE, 0}; //XZ平面に垂直な単位ベクトル
    VecFx32 width;
		s32 width_ofs = FIELD_RAIL_GetWidthOfs( work );
		fx32 ofs_unit = FIELD_RAIL_GetOfsUnit( work );
    fx32 w_ofs = FX_Mul( width_ofs<<FX32_SHIFT, ofs_unit );
    VEC_CrossProduct(&val, &xzNormal, &width);
    VEC_Normalize(&width, &width);
    VEC_MultAdd(w_ofs, &width, pos, pos);

		if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)
		{
			debugPrintWholeVector("start ", p0, "\n");
			//debugPrintHexVector(p0);
			debugPrintWholeVector("now   ", pos, "\n");
			//debugPrintHexVector(pos);
			debugPrintWholeVector("end   ", p1, "\n");
			//debugPrintHexVector(p1);
			TOMOYA_Printf( "w_ofs = 0x%x\n", w_ofs );
			debugPrintHexVector(&width);
		}
  }

}

//------------------------------------------------------------------
/**
 *
 * @brief レール移動：点と点の間を曲線補完
 *
 * （中心点-->スタート位置）、（中心点-->終了位置）の二つのベクトルを
 * 球面線形補完している。中心点の位置はパラメータで受け取る
 */
//------------------------------------------------------------------
void FIELD_RAIL_POSFUNC_CurveLine(const FIELD_RAIL_WORK * work, VecFx32 * pos)
{
  const RAIL_LINE * nLine = FIELD_RAIL_GetLine(work);
	const RAIL_LINEPOS_SET* line_pos_set;
	const RAIL_POINT* point_s = FIELD_RAIL_GetPointStart( work );
	const RAIL_POINT* point_e = FIELD_RAIL_GetPointEnd( work );
  const VecFx32 * p_s = &point_s->pos;
  const VecFx32 * p_e = &point_e->pos;
	const RAIL_POSFUNC_CURVE_WORK* cp_wk;
  VecFx32 center, vec_s, vec_e, vec_i;
	s32 line_ofs			= FIELD_RAIL_GetLineOfs( work );
	s32 line_ofs_max	= FIELD_RAIL_GetLineOfsMax( work );
  fx32 ofs = FX_Div( (line_ofs * FX32_ONE), line_ofs_max*FX32_ONE );
  VecFx32 pos_sub;


	// Y座標は、線形で求める。
	// XZは、回転でもとめる
	pos->y = p_e->y - p_s->y;
	pos->y = FX_Div( FX_Mul( pos->y, line_ofs<<FX32_SHIFT ), line_ofs_max<<FX32_SHIFT );
	pos->y += p_s->y;
	

	line_pos_set = FIELD_RAIL_GetLinePosSet( work );
	cp_wk = (const RAIL_POSFUNC_CURVE_WORK*)line_pos_set->work;
	center.x = cp_wk->x;
	center.y = pos->y;
	center.z = cp_wk->z;

	VEC_Subtract(p_s, &center, &vec_s);
	VEC_Subtract(p_e, &center, &vec_e);
	vec_s.y = 0;
	vec_e.y = 0;

  VEC_Subtract( &vec_e, &vec_s, &pos_sub );
  pos_sub.y = 0;
  VEC_Normalize( &pos_sub, &pos_sub );

	
	getIntermediateVector(&vec_i, &vec_s, &vec_e, ofs);
	VEC_Add(&center, &vec_i, pos);

	{
		VecFx32 w_vec;
    VecFx32 normal;
		s32 width_ofs = FIELD_RAIL_GetWidthOfs( work );
		fx32 ofs_unit = FIELD_RAIL_GetOfsUnit( work );
    fx32 ofs_w = FX_Mul( width_ofs<<FX32_SHIFT, ofs_unit );
    
		VEC_Normalize(&vec_i, &w_vec);
    VEC_CrossProduct( &w_vec, &pos_sub, &normal );
    if( normal.y < 0 )
    {
  		VEC_MultAdd(-(ofs_w), &w_vec, pos, pos);
    }
    else
    {
  		VEC_MultAdd(ofs_w, &w_vec, pos, pos);
    }
	}

  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)
  {
    debugPrintWholeVector("Center:", &center, "\n");
    debugPrintWholeVector("start: ", p_s, "\n");
    debugPrintWholeVector("end:   ", p_e, "\n");
    debugPrintWholeVector("v_s:   ", &vec_s, "\n");
    OS_TPrintf("v_s:len=%08x\n",VEC_Mag(&vec_s));
    debugPrintWholeVector("v_e:   ", &vec_e, "\n");
    OS_TPrintf("v_e:len=%08x\n",VEC_Mag(&vec_e));
    debugPrintWholeVector("v_i:   ", &vec_i, "\n");
    debugPrintHexVector(&vec_i);
    OS_TPrintf("v_i:len=%08x\n",VEC_Mag(&vec_i));
    debugPrintWholeVector("pos:   ", pos, "\n");
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  Y軸にもカーブするライン処理
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_POSFUNC_YCurveLine(const FIELD_RAIL_WORK * work, VecFx32 * pos)
{
  const RAIL_LINE * nLine = FIELD_RAIL_GetLine(work);
	const RAIL_LINEPOS_SET* line_pos_set;
	const RAIL_POINT* point_s = FIELD_RAIL_GetPointStart( work );
	const RAIL_POINT* point_e = FIELD_RAIL_GetPointEnd( work );
  const VecFx32 * p_s = &point_s->pos;
  const VecFx32 * p_e = &point_e->pos;
	const RAIL_POSFUNC_CURVE_WORK* cp_wk;
  VecFx32 center, vec_s, vec_e, vec_i;
	s32 line_ofs			= FIELD_RAIL_GetLineOfs( work );
	s32 line_ofs_max	= FIELD_RAIL_GetLineOfsMax( work );
  fx32 ofs = FX_Div( (line_ofs * FX32_ONE), line_ofs_max*FX32_ONE );
  VecFx32 pos_sub;


  // すべて含めて計算
	line_pos_set = FIELD_RAIL_GetLinePosSet( work );
	cp_wk = (const RAIL_POSFUNC_CURVE_WORK*)line_pos_set->work;
	center.x = cp_wk->x;
	center.y = cp_wk->y;
	center.z = cp_wk->z;

	VEC_Subtract(p_s, &center, &vec_s);
	VEC_Subtract(p_e, &center, &vec_e);

  VEC_Subtract( &vec_e, &vec_s, &pos_sub );
  VEC_Normalize( &pos_sub, &pos_sub );

	
	getIntermediateVector(&vec_i, &vec_s, &vec_e, ofs);
	VEC_Add(&center, &vec_i, pos);

  // 幅の計算処理
	{
		VecFx32 w_vec;
    VecFx32 normal;
		s32 width_ofs = FIELD_RAIL_GetWidthOfs( work );
		fx32 ofs_unit = FIELD_RAIL_GetOfsUnit( work );
    fx32 ofs_w = FX_Mul( width_ofs<<FX32_SHIFT, ofs_unit );

    // 横方向はXZ平面
    if( (vec_i.x != 0) && (vec_i.z != 0) )
    {
      vec_i.y = 0;
    }
    else
    {
      VecFx32 up = {0,FX32_ONE,0};
      // XZの変化する方向の横を求めればよい
      VEC_Subtract( p_e, p_s, &vec_i );

      VEC_CrossProduct( &vec_i, &up, &vec_i );
    }
		VEC_Normalize(&vec_i, &w_vec);
    VEC_CrossProduct( &w_vec, &pos_sub, &normal );
    if( normal.y < 0 )
    {
  		VEC_MultAdd(-(ofs_w), &w_vec, pos, pos);
    }
    else
    {
  		VEC_MultAdd(ofs_w, &w_vec, pos, pos);
    }
	}

  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)
  {
    debugPrintWholeVector("Center:", &center, "\n");
    debugPrintWholeVector("start: ", p_s, "\n");
    debugPrintWholeVector("end:   ", p_e, "\n");
    debugPrintWholeVector("v_s:   ", &vec_s, "\n");
    OS_TPrintf("v_s:len=%08x\n",VEC_Mag(&vec_s));
    debugPrintWholeVector("v_e:   ", &vec_e, "\n");
    OS_TPrintf("v_e:len=%08x\n",VEC_Mag(&vec_e));
    debugPrintWholeVector("v_i:   ", &vec_i, "\n");
    debugPrintHexVector(&vec_i);
    OS_TPrintf("v_i:len=%08x\n",VEC_Mag(&vec_i));
    debugPrintWholeVector("pos:   ", pos, "\n");
  }
}


//-----------------------------------------------------------------------------
/**
 *					ロケーションあたり判定
*/
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  直線運動  ロケーションあたり判定
 *
 *	@retval TRUE  ＨＩＴ
 *	@retval FALSE ＭＩＳＳ
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_LINE_HIT_LOCATION_FUNC_StraitLine( u32 rail_index, const FIELD_RAIL_MAN * cp_man, const VecFx32* cp_check_pos, RAIL_LOCATION* p_location, VecFx32* p_pos )
{
  const RAIL_LINE* cp_line;
  const RAIL_POINT* cp_point_s;
  const RAIL_POINT* cp_point_e;
  const RAIL_LINEPOS_SET* cp_linepos;
  VecFx32 cross_pos, gaiseki;
  VecFx32 posa, posb;
  fx32 width_dist, front_dist;
  u32 front_ofs_width, start_width, end_width;
  fx32 b_cos;
  u32 total_ofs, front_ofs;
  int i;
  BOOL result;
  fx32 unit_size;
  RAIL_LOCATION safe_location;

  cp_line = FIELD_RAIL_MAN_GetRailDatLine( cp_man, rail_index );
  cp_point_s = FIELD_RAIL_MAN_GetRailDatPoint( cp_man, cp_line->point_s );
  cp_point_e = FIELD_RAIL_MAN_GetRailDatPoint( cp_man, cp_line->point_e );
  unit_size = FIELD_RAIL_MAN_GetRailDatUnitSize( cp_man );

  DEBUG_RAIL_Printf( "\ncheck straitLine\n" );
  
  // 幅の確認をして
  // グリッド座標を求める。
  result = is_Hit3DPosPlane( cp_point_s, cp_point_e, cp_check_pos, &cross_pos, &front_dist, &gaiseki );
  if( result == FALSE )
  {
    return FALSE;
  }

  DEBUG_RAIL_Printf( "\n" );

  // 幅のチェック 
  // ＸＺ平面で考えるので、Ｙを抜きにして、交点とcehck_posの距離を求める。
  VEC_Set( &posa, cp_check_pos->x, 0, cp_check_pos->z );
  VEC_Set( &posb, cross_pos.x, 0, cross_pos.z );
  width_dist = VEC_Distance( &posa, &posb );
  width_dist = FX_Div( width_dist, unit_size ) >> FX32_SHIFT;

  DEBUG_RAIL_Printf( "width_ofs [%d]\n", width_dist );
  
  // トータルサイズの取得
  cp_linepos = FIELD_RAIL_MAN_GetRailDatLinePos( cp_man, cp_line->line_pos_set );
  total_ofs = (cp_line->line_grid_max) * RAIL_WALK_OFS; // 最後のグリッドにたつことはない

  // 交点のオフセット
  front_ofs = FX_Div( front_dist, unit_size ) >> FX32_SHIFT;
  front_ofs = total_ofs - front_ofs;

  // 交点の幅
  for( i=0; i<RAIL_CONNECT_LINE_MAX; i++ )
  {
    if( cp_point_s->lines[i] == rail_index )
    {
      start_width = cp_point_s->width_ofs_max[i];
    }
    
    if( cp_point_e->lines[i] == rail_index )
    {
      end_width = cp_point_e->width_ofs_max[i];
    }
  }
  front_ofs_width = start_width + ( ((end_width - start_width)*front_ofs) / total_ofs );


  DEBUG_RAIL_Printf( "line_ofs of width [%d]\n", front_ofs_width );
  
  // 幅チェック
  if( width_dist <= (front_ofs_width + HIT_LOCATION_WIDHT_AREA) )
  {
    // 降り立つ場所があった！
    p_location->rail_index = rail_index;
    p_location->type = FIELD_RAIL_TYPE_LINE;
    p_location->key  = RAIL_KEY_DOWN;
    // front_ofsとwidth_distがどのグリッドよりかも考えて計算する
    if( gaiseki.y>=0 )
    {
      p_location->width_grid = FIELD_RAIL_TOOL_GetOfsToGrid_Round(width_dist);
    }
    else
    {
      p_location->width_grid = -FIELD_RAIL_TOOL_GetOfsToGrid_Round(width_dist);
    }
    p_location->line_grid = FIELD_RAIL_TOOL_GetOfsToGrid_Round(front_ofs);

    TOMOYA_Printf( "line ofs to grid ofs[%d]  grid[%d]\n", front_ofs, p_location->line_grid );



    // レール範囲内のロケーションにする
    FIELD_RAIL_MAN_CalcSafeLocation( cp_man, p_location, &safe_location );

    DEBUG_RAIL_Printf( "location width_grid [%d] line_grid [%d]\n", safe_location.width_grid, safe_location.line_grid );

    FIELD_RAIL_MAN_GetLocationPosition( cp_man, &safe_location, p_pos );

    //　安全なロケーションを見つけられたかもチェック
    if( GFL_STD_MemComp( p_location, &safe_location, sizeof(RAIL_LOCATION) ) == 0 )
    {
      return TRUE;
    }
    return FALSE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーブ運動  ロケーションあたり判定
 *
 *	@retval TRUE  ＨＩＴ
 *	@retval FALSE ＭＩＳＳ
 */
//-----------------------------------------------------------------------------
BOOL FIELD_RAIL_LINE_HIT_LOCATION_FUNC_CircleLine( u32 rail_index, const FIELD_RAIL_MAN * cp_man, const VecFx32* cp_check_pos, RAIL_LOCATION* p_location, VecFx32* p_pos )
{
  const RAIL_LINE* cp_line;
  const RAIL_POINT* cp_point_s;
  const RAIL_POINT* cp_point_e;
  FIELD_RAIL_WORK* p_calcwork;
  VecFx32 pos, last_pos;
  u32 width_ofs_max;
  u32 line_ofs_max, line_grid_max;
  VecFx32 vec_a, vec_b, way_a, cross_pos, gaiseki;
  VecFx32 plane_dista, plane_distb;
  VecFx32 vecn_a, vecn_b;
  fx32 dist_a, bcos, cross_dist, div_num;
  fx32 unit_size, front_dist;
  int i;
  BOOL result;
  RAIL_LOCATION safe_location;

  //DEBUG_RAIL_Printf( "\ncheck circleLine\n" );

  cp_line = FIELD_RAIL_MAN_GetRailDatLine( cp_man, rail_index );
  cp_point_s = FIELD_RAIL_MAN_GetRailDatPoint( cp_man, cp_line->point_s );
  cp_point_e = FIELD_RAIL_MAN_GetRailDatPoint( cp_man, cp_line->point_e );
  
  // 幅の確認をして
  // グリッド座標を求める。
  result = is_Hit3DPosPlane( cp_point_s, cp_point_e, cp_check_pos, &cross_pos, &front_dist, &gaiseki );
  if( result == FALSE )
  {
    return FALSE;
  }

  //DEBUG_RAIL_Printf( "\n" );

  // 全部なめる
  // 各グリッドでの、範囲から、求める
  // 計算用レールワークを使用し、
  // １グリッドずつ、当たり判定
  p_location->rail_index = rail_index;
  p_location->type = FIELD_RAIL_TYPE_LINE;
  p_location->key = RAIL_KEY_DOWN;
  p_location->width_grid = 0;
  p_location->line_grid = 0;

  p_calcwork = FIELD_RAIL_MAN_GetCalcRailWork( cp_man );
  FIELD_RAIL_WORK_SetLocation( p_calcwork, p_location );

  line_ofs_max = FIELD_RAIL_GetLineOfsMax( p_calcwork );
  line_grid_max = RAIL_OFS_TO_GRID( line_ofs_max );

  // 初期座標を求める
  FIELD_RAIL_WORK_GetPos( p_calcwork, &pos );
  VEC_Set( &last_pos, pos.x, pos.y, pos.z );

  // ユニットサイズ取得
  unit_size = FIELD_RAIL_MAN_GetRailDatUnitSize( cp_man );
  //DEBUG_RAIL_Printf( "unit_size[%d]\n", FX_Whole( unit_size ) );

  for( i=1; i<line_grid_max; i++ )
  {
    //　グリッド分チェックする
    p_location->line_grid = i;
    FIELD_RAIL_WORK_SetLocation( p_calcwork, p_location );
    FIELD_RAIL_WORK_GetPos( p_calcwork, &pos );
    width_ofs_max = FIELD_RAIL_GetWidthOfsMax( p_calcwork );

    //DEBUG_RAIL_Printf( "line_grid [%d] width_ofs_max [%d]\n", i, width_ofs_max );
    //DEBUG_RAIL_Printf( "pos x[%d] y[%d] z[%d]\n", FX_Whole(pos.x), FX_Whole(pos.y), FX_Whole(pos.z) );
    //DEBUG_RAIL_Printf( "last_pos x[%d] y[%d] z[%d]\n", FX_Whole(last_pos.x), FX_Whole(last_pos.y), FX_Whole(last_pos.z) );
    //DEBUG_RAIL_Printf( "check_pos x[%d] y[%d] z[%d]\n", FX_Whole(cp_check_pos->x), FX_Whole(cp_check_pos->y), FX_Whole(cp_check_pos->z) );

    // check_posがベクトル範囲内にあるかチェック
    // ここでも、ベクトルＡlast_pos - pos
    // ベクトルＢ　check_pos - pos
    // から、ベクトルＡへベクトルＢを投影し
    // ベクトルＡより大きくなったり、
    // cosが９０度を超えたらＮＧ
    VEC_Subtract( &last_pos, &pos, &vec_a );
    VEC_Subtract( cp_check_pos, &pos, &vec_b );
    VEC_Normalize( &vec_a, &way_a );
    dist_a = VEC_Mag( &vec_a );

    VEC_Normalize( &vec_a, &vecn_a );
    VEC_Normalize( &vec_b, &vecn_b );
    VEC_CrossProduct( &vecn_a, &vecn_b, &gaiseki );

    // 値がでかすぎるときがあるので、
    // 1/|A|の大きさにする
    vec_a = way_a;
    {
      fx32 dist_b = VEC_Mag( &vec_b );
      
      VEC_Normalize( &vec_b, &vec_b );
      div_num= dist_a;
      dist_b = FX_Div( dist_b, div_num );
      dist_a = FX32_ONE;
      vec_b.x = FX_Mul( vec_b.x, dist_b );
      vec_b.y = FX_Mul( vec_b.y, dist_b );
      vec_b.z = FX_Mul( vec_b.z, dist_b );
    }
    
    bcos = VEC_DotProduct( &vec_a, &vec_b );
    //bcos = FX_Div( bcos, dist_a ); // dist_a==FX32_ONEなので必要なくなる

    //DEBUG_RAIL_Printf( "bcos [%d] dist_a [%d]\n", FX_Whole(bcos), FX_Whole(dist_a) );

    if( (bcos <= dist_a) && (bcos >= 0) )
    {
      // 範囲内なので、平面で見て、距離がwidth_ofs_max内かチェックする
      {
        // 1/|A|をもとに戻す
        bcos = FX_Mul( bcos, div_num );
      }
      //DEBUG_RAIL_Printf( "real bcos [%d]\n", FX_Whole(bcos) );
      
      // 交点を求める
      VEC_Set( &cross_pos, FX_Mul( way_a.x, bcos ), 
          FX_Mul( way_a.y, bcos ), FX_Mul( way_a.z, bcos ) );
      VEC_Add( &pos, &cross_pos, &cross_pos );

      //DEBUG_RAIL_Printf( "cross pos x[%d] y[%d] z[%d]\n", FX_Whole(cross_pos.x), FX_Whole(cross_pos.y), FX_Whole(cross_pos.z) );

      VEC_Set( &plane_dista, cp_check_pos->x, 0, cp_check_pos->z );
      VEC_Set( &plane_distb, cross_pos.x, 0, cross_pos.z );

      cross_dist = VEC_Distance( &plane_dista, &plane_distb ); 
      cross_dist = FX_Div( cross_dist, unit_size ) >> FX32_SHIFT;

      //DEBUG_RAIL_Printf( "cross_ofs [%d]\n", cross_dist );

      //DEBUG_RAIL_Printf( "gaiseki.y [%d]\n", gaiseki.y );

      if( (width_ofs_max + HIT_LOCATION_WIDHT_AREA) >= cross_dist )
      {
        // 合格！
        if( gaiseki.y >= 0 )
        {
          p_location->width_grid = FIELD_RAIL_TOOL_GetOfsToGrid_Round(cross_dist);
        }
        else
        {
          p_location->width_grid = -FIELD_RAIL_TOOL_GetOfsToGrid_Round(cross_dist);
        }

        // レール範囲内のロケーションにする
        FIELD_RAIL_MAN_CalcSafeLocation( cp_man, p_location, &safe_location );
        FIELD_RAIL_MAN_GetLocationPosition( cp_man, p_location, p_pos );

        //DEBUG_RAIL_Printf( "OK width_grid[%d]\n", p_location->width_grid );
        //　安全なロケーションを見つけられたかもチェック
        if( GFL_STD_MemComp( p_location, &safe_location, sizeof(RAIL_LOCATION) ) == 0 )
        {
          return TRUE;
        }
        return FALSE;
      }
    }
    
    VEC_Set( &last_pos, pos.x, pos.y, pos.z );
  }

  return FALSE;
  
}







//------------------------------------------------------------------
/**
 * @brief カメラ指定：パラメータをFx32３つとして単純に足すだけ
 */
//------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_FixPosCamera(const FIELD_RAIL_MAN* man)
{
  VecFx32 pos;
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
  const RAIL_CAMERA_SET * cam_set = FIELD_RAIL_GetCameraSet(work);
	const RAIL_CAMERAFUNC_FIXPOS_WORK* cp_wk;
	
  FIELD_RAIL_MAN_GetBindWorkPos(man, &pos);

	// 座標直指定モード
	FIELD_CAMERA_SetMode( FIELD_RAIL_MAN_GetCamera(man), FIELD_CAMERA_MODE_DIRECT_POS );

	cp_wk = (const RAIL_CAMERAFUNC_FIXPOS_WORK*)cam_set->work;
  pos.x += cp_wk->x;
  pos.y += cp_wk->y;
  pos.z += cp_wk->z;
  FIELD_CAMERA_SetCameraPos( FIELD_RAIL_MAN_GetCamera(man), &pos );

	// デフォルトターゲットを参照
	FIELD_CAMERA_BindDefaultTarget( FIELD_RAIL_MAN_GetCamera(man) );
}

//------------------------------------------------------------------
/**
 * @brief カメラ指定：パラメータをAngle指定に使用する
 */
//------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_FixAngleCamera(const FIELD_RAIL_MAN* man)
{
  FIELD_CAMERA * cam = FIELD_RAIL_MAN_GetCamera(man);
  VecFx32 pos;
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
  const RAIL_CAMERA_SET * cam_set = FIELD_RAIL_GetCameraSet(work);
	const RAIL_CAMERAFUNC_FIXANGLE_WORK* cp_wk;

	cp_wk = (const RAIL_CAMERAFUNC_FIXANGLE_WORK*)cam_set->work;

	// 座標直指定モード
	FIELD_CAMERA_SetMode( cam, FIELD_CAMERA_MODE_DIRECT_POS );

	// デフォルトターゲットを参照
	FIELD_CAMERA_BindDefaultTarget( FIELD_RAIL_MAN_GetCamera(man) );

  FIELD_RAIL_MAN_GetBindWorkPos( man, &pos );
  {
    VecFx32 camPos;
    u16 yaw = (u16)cp_wk->yaw;
    u16 pitch = (u16)cp_wk->pitch;
    fx32 len = cp_wk->len;

    getVectorFromAngleValue(&camPos, yaw, pitch, len);
    VEC_Add(&camPos, &pos, &camPos);
    FIELD_CAMERA_SetCameraPos(cam, &camPos);

    if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y)
    {
      debugPrintWholeVector("CamTgt:", &pos, "\n");
      debugPrintWholeVector("CamPos:", &camPos, "\n");
      OS_TPrintf("yaw:%04x pitch:%04x len:%08x\n",yaw, pitch, len);
    }
  }
}

//------------------------------------------------------------------
/**
 * @brief カメラ指定：保持する二つのPOINTのangleバリューを補完する
 */
//------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_OfsAngleCamera(const FIELD_RAIL_MAN* man)
{
  VecFx32 c_s, c_e, c_now, target;
  const FIELD_RAIL_WORK * work = FIELD_RAIL_MAN_GetBindWork( man );
	const RAIL_POINT* point_s = FIELD_RAIL_GetPointStart( work );
	const RAIL_POINT* point_e = FIELD_RAIL_GetPointEnd( work );
  const RAIL_CAMERA_SET * cs;
  const RAIL_CAMERA_SET * ce;
	FIELD_CAMERA* cam = FIELD_RAIL_MAN_GetCamera( man );
	const RAIL_CAMERAFUNC_FIXANGLE_WORK* cs_work;
	const RAIL_CAMERAFUNC_FIXANGLE_WORK* ce_work;
	
  GF_ASSERT(FIELD_RAIL_GetType(work) == FIELD_RAIL_TYPE_LINE);

	// 座標直指定モード
	FIELD_CAMERA_SetMode( cam, FIELD_CAMERA_MODE_DIRECT_POS );

	// デフォルトターゲットを参照
	FIELD_CAMERA_BindDefaultTarget( FIELD_RAIL_MAN_GetCamera(man) );

  cs = FIELD_RAIL_POINT_GetCameraSet( work, point_s );
//  GF_ASSERT(getRailDatCameraFunc( &man->rail_dat, cs->func_index ) == FIELD_RAIL_CAMERAFUNC_FixAngleCamera);
  ce = FIELD_RAIL_POINT_GetCameraSet( work, point_e );
//  GF_ASSERT(getRailDatCameraFunc( &man->rail_dat, ce->func_index ) == FIELD_RAIL_CAMERAFUNC_FixAngleCamera);

	cs_work = (const RAIL_CAMERAFUNC_FIXANGLE_WORK*)cs->work;
	ce_work = (const RAIL_CAMERAFUNC_FIXANGLE_WORK*)ce->work;
	
  {
    u32 div = FIELD_RAIL_GetLineOfsMax(work);
		s32 line_ofs = FIELD_RAIL_GetLineOfs(work);
    fx32 t = FX_Div( FX32_ONE * line_ofs, div * FX32_ONE );
    u16 pitch_s = cs_work->pitch;
    fx32 len_s = cs_work->len;
    u16 pitch_e = ce_work->pitch;
    fx32 len_e = ce_work->len;


    getVectorFromAngleValue(&c_s, cs_work->yaw, pitch_s, len_s);
    getVectorFromAngleValue(&c_e, ce_work->yaw, pitch_e, len_e);

		getIntermediateVector(&c_now, &c_s, &c_e, t);

		FIELD_RAIL_MAN_GetBindWorkPos( man, &target );
		VEC_Add(&c_now, &target, &c_now);
		FIELD_CAMERA_SetCameraPos(cam, &c_now);
  }

  if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y)
  {
    debugPrintWholeVector("CamTgt:", &target, "\n");
    debugPrintWholeVector("CamPos:", &c_now, "\n");
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  すべて決めうちのカメラ
 *
 *	@param	man
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_FixAllCamera(const FIELD_RAIL_MAN* man)
{
  VecFx32 pos;
  VecFx32 target;
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
  const RAIL_CAMERA_SET * cam_set = FIELD_RAIL_GetCameraSet(work);
	FIELD_CAMERA* cam = FIELD_RAIL_MAN_GetCamera( man );
	const RAIL_CAMERAFUNC_FIXALL_WORK* cp_work;

	cp_work = (const RAIL_CAMERAFUNC_FIXALL_WORK*)cam_set->work;

	// 座標直指定モード
	FIELD_CAMERA_SetMode( cam, FIELD_CAMERA_MODE_DIRECT_POS );

	// 追従ターゲットNULL
	FIELD_CAMERA_FreeTarget( FIELD_RAIL_MAN_GetCamera(man) );

  pos.x = cp_work->pos_x;
  pos.y = cp_work->pos_y;
  pos.z = cp_work->pos_z;
  target.x = cp_work->target_x;
  target.y = cp_work->target_y;
  target.z = cp_work->target_z;
  FIELD_CAMERA_SetCameraPos(cam, &pos);
  FIELD_CAMERA_SetTargetPos(cam, &target);
}

//----------------------------------------------------------------------------
/**
 *	@brief  FIXALL同士を線形補間するカメラ
 *
 *	@param	man
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_FixAllLineCamera(const FIELD_RAIL_MAN* man)
{
  VecFx32 pos;
  VecFx32 target;
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
	FIELD_CAMERA* cam = FIELD_RAIL_MAN_GetCamera( man );
  const RAIL_CAMERA_SET * cs;
  const RAIL_CAMERA_SET * ce;
	const RAIL_CAMERAFUNC_FIXALL_WORK* cp_para_s;
	const RAIL_CAMERAFUNC_FIXALL_WORK* cp_para_e;
	const RAIL_POINT* point_s = FIELD_RAIL_GetPointStart( work );
	const RAIL_POINT* point_e = FIELD_RAIL_GetPointEnd( work );


  cs = FIELD_RAIL_POINT_GetCameraSet( work, point_s );
  ce = FIELD_RAIL_POINT_GetCameraSet( work, point_e );

	cp_para_s = (const RAIL_CAMERAFUNC_FIXALL_WORK*)cs->work;
	cp_para_e = (const RAIL_CAMERAFUNC_FIXALL_WORK*)ce->work;


	// 座標直指定モード
	FIELD_CAMERA_SetMode( cam, FIELD_CAMERA_MODE_DIRECT_POS );

	// 追従ターゲットNULL
	FIELD_CAMERA_FreeTarget( FIELD_RAIL_MAN_GetCamera(man) );

  {
    u32 ofs_max = FIELD_RAIL_GetLineOfsMax(work);
    s32 ofs = FIELD_RAIL_GetLineOfs(work);
    
    pos.x = FX_Div( FX_Mul( (cp_para_e->pos_x - cp_para_s->pos_x), ofs<<FX32_SHIFT ), ofs_max<<FX32_SHIFT );
    pos.y = FX_Div( FX_Mul( (cp_para_e->pos_y - cp_para_s->pos_y), ofs<<FX32_SHIFT ), ofs_max<<FX32_SHIFT );
    pos.z = FX_Div( FX_Mul( (cp_para_e->pos_z - cp_para_s->pos_z), ofs<<FX32_SHIFT ), ofs_max<<FX32_SHIFT );
    target.x = FX_Div( FX_Mul( (cp_para_e->target_x - cp_para_s->target_x), ofs<<FX32_SHIFT ), ofs_max<<FX32_SHIFT );
    target.y = FX_Div( FX_Mul( (cp_para_e->target_y - cp_para_s->target_y), ofs<<FX32_SHIFT ), ofs_max<<FX32_SHIFT );
    target.z = FX_Div( FX_Mul( (cp_para_e->target_z - cp_para_s->target_z), ofs<<FX32_SHIFT ), ofs_max<<FX32_SHIFT );

    pos.x += cp_para_s->pos_x;
    pos.y += cp_para_s->pos_y;
    pos.z += cp_para_s->pos_z;
    target.x += cp_para_s->target_x;
    target.y += cp_para_s->target_y;
    target.z += cp_para_s->target_z;
  }

  FIELD_CAMERA_SetCameraPos(cam, &pos);
  FIELD_CAMERA_SetTargetPos(cam, &target);
}



//----------------------------------------------------------------------------
/**
 *	@brief  円カメラ動作
 *
 *	@param	man 
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_CircleCamera( const FIELD_RAIL_MAN * man )
{
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
	const FIELD_CAMERA* cam = FIELD_RAIL_MAN_GetCamera( man );
  const RAIL_CAMERA_SET * cs;
  const RAIL_CAMERA_SET * ce;
  const RAIL_POINT * point_s;
  const RAIL_POINT * point_e;
  u16 pitch;
  fx32 len;
  s32 div;
  s32 ofs;
  VecFx32 target;
	u32 type;
	const RAIL_CAMERAFUNC_CIRCLE_WORK* cs_work;
	const RAIL_CAMERAFUNC_CIRCLE_WORK* ce_work;

	type = FIELD_RAIL_GetType( work );


  if( type == FIELD_RAIL_TYPE_LINE )
  {
		point_s = FIELD_RAIL_GetPointStart( work );
		point_e = FIELD_RAIL_GetPointEnd( work );
    cs = FIELD_RAIL_POINT_GetCameraSet( work, point_s );
    ce = FIELD_RAIL_POINT_GetCameraSet( work, point_e );
    div = FIELD_RAIL_GetLineOfsMax(work);
    ofs = FIELD_RAIL_GetLineOfs(work);
  }
  else
  {
    cs = FIELD_RAIL_GetCameraSet( work );
    ce = FIELD_RAIL_GetCameraSet( work );
    div = 1;
    ofs = 0;
  }

	cs_work = (const RAIL_CAMERAFUNC_CIRCLE_WORK*)cs->work;
	ce_work = (const RAIL_CAMERAFUNC_CIRCLE_WORK*)ce->work; 

  // pitch len の計算
  pitch = cs_work->pitch + ((((int)ce_work->pitch - (int)cs_work->pitch) * ofs) / div);
  len   = cs_work->len + FX_Div( FX_Mul(ce_work->len - cs_work->len, ofs<<FX32_SHIFT), div<<FX32_SHIFT );

  target.x = cs_work->target_x;
  target.y = cs_work->target_y;
  target.z = cs_work->target_z;

  updateCircleCamera( man, pitch, len, &target );
}

//----------------------------------------------------------------------------
/**
 *	@brief  距離固定　円運動カメラ
 *
 *	@param	man   
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_FixLenCircleCamera( const FIELD_RAIL_MAN * man )
{
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
  const RAIL_CAMERA_SET * cline;
  u16 pitch;
  fx32 len;
  VecFx32 target;
	const RAIL_CAMERAFUNC_FIXLEN_CIRCLE_WORK* cp_work;



	cline = FIELD_RAIL_GetCameraSet( work );

	cp_work = (const RAIL_CAMERAFUNC_FIXLEN_CIRCLE_WORK*)cline->work;

  // pitch len の計算
  pitch = cp_work->pitch;
  len   = cp_work->len;


  target.x = cp_work->target_x;
  target.y = cp_work->target_y;
  target.z = cp_work->target_z;

  updateCircleCamera( man, pitch, len, &target );
}

//----------------------------------------------------------------------------
/**
 *	@brief  円運動カメラの更新
 *
 *	@param	man
 *	@param	pitch
 *	@param	len 
 */
//-----------------------------------------------------------------------------
static void updateCircleCamera( const FIELD_RAIL_MAN * man, u16 pitch, fx32 len, const VecFx32* cp_target )
{
  VecFx32 pos, target, n0, camera_pos;
  FIELD_CAMERA* p_camera;
  fx32 xz_dist;
  fx32 target_y;

//	TOMOYA_Printf( "target x[0x%x] y[0x%x] z[0x%x]\n", cp_target->x, cp_target->y, cp_target->z );

  p_camera = FIELD_RAIL_MAN_GetCamera( man );

	// 座標直指定モード
	FIELD_CAMERA_SetMode( p_camera, FIELD_CAMERA_MODE_DIRECT_POS );

	// 追従ターゲットNULL
	FIELD_CAMERA_FreeTarget( p_camera );
	
  target    = *cp_target;
  target_y  = target.y;
  target.y  = 0;
  
  FIELD_RAIL_MAN_GetBindWorkPos( man, &pos );
  pos.y   = 0;

  VEC_Subtract( &pos, &target, &pos );
  VEC_Normalize( &pos, &n0 );

  // 方向ベクトルから、カメラangleを求める
  camera_pos.y = FX_Mul( FX_SinIdx( pitch ), len );
  xz_dist      = FX_Mul( FX_CosIdx( pitch ), len );
  camera_pos.x = FX_Mul( n0.x, xz_dist );
  camera_pos.z = FX_Mul( n0.z, xz_dist );
  camera_pos.x += target.x;
  camera_pos.y += target_y;
  camera_pos.z += target.z;
  
	FIELD_CAMERA_SetTargetPos( p_camera, cp_target );
	FIELD_CAMERA_SetCameraPos( p_camera, &camera_pos );
}



//----------------------------------------------------------------------------
/**
 *	@brief  ライン、ターゲット変更 円動作カメラ
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_TargetChangeCircleCamera( const FIELD_RAIL_MAN * man )
{
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
  const RAIL_CAMERA_SET * cline;
  u16 pitch;
  fx32 len;
  VecFx32 target;
	const RAIL_CAMERAFUNC_TARGETCHANGE_CIRCLE_WORK* cp_work;
	u32 type;
  s32 ofs, ofs_max;

	type = FIELD_RAIL_GetType( work );
  // レールで使ってね
  GF_ASSERT( type == FIELD_RAIL_TYPE_LINE );

  ofs_max = FIELD_RAIL_GetLineOfsMax(work); 
  ofs = FIELD_RAIL_GetLineOfs(work);


	cline = FIELD_RAIL_GetCameraSet( work );

	cp_work = (const RAIL_CAMERAFUNC_TARGETCHANGE_CIRCLE_WORK*)cline->work;

  // pitch len の計算
  pitch = cp_work->pitch;
  len   = cp_work->len;

  target.x = cp_work->target_sx + FX_Div( FX_Mul( cp_work->target_ex - cp_work->target_sx, ofs<<FX32_SHIFT ), ofs_max<<FX32_SHIFT );
  target.y = cp_work->target_sy + FX_Div( FX_Mul( cp_work->target_ey - cp_work->target_sy, ofs<<FX32_SHIFT ), ofs_max<<FX32_SHIFT );
  target.z = cp_work->target_sz + FX_Div( FX_Mul( cp_work->target_ez - cp_work->target_sz, ofs<<FX32_SHIFT ), ofs_max<<FX32_SHIFT );

  updateCircleCamera( man, pitch, len, &target );
}



//----------------------------------------------------------------------------
/**
 *	@brief  Ｐｌａｙｅｒターゲット 円動作カメラ
 */
//-----------------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_PlayerTargetCircleCamera( const FIELD_RAIL_MAN * man )
{
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
  const RAIL_CAMERA_SET * cline;
  u16 pitch;
  u16 yaw;
  fx32 len;
  VecFx32 center, pos;
  FIELD_CAMERA* p_camera;
	const RAIL_CAMERAFUNC_PLAYERTARGET_CIRCLE_WORK* cp_work;

//	TOMOYA_Printf( "target x[0x%x] y[0x%x] z[0x%x]\n", cp_target->x, cp_target->y, cp_target->z );

	cline = FIELD_RAIL_GetCameraSet( work );

	cp_work = (const RAIL_CAMERAFUNC_PLAYERTARGET_CIRCLE_WORK*)cline->work;

  p_camera = FIELD_RAIL_MAN_GetCamera( man );

	// 座標直指定モード
	FIELD_CAMERA_SetMode( p_camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

	// デフォルトターゲットを参照
	FIELD_CAMERA_BindDefaultTarget( FIELD_RAIL_MAN_GetCamera(man) );
  
	
  FIELD_RAIL_MAN_GetBindWorkPos( man, &pos );
  pos.y = 0;
  center.x = cp_work->center_x;
  center.y = 0;
  center.z = cp_work->center_z;

  VEC_Subtract( &pos, &center, &pos );
  VEC_Normalize( &pos, &pos );

  // 角度（Ｙａｗ）
  yaw = FX_Atan2Idx( pos.x, pos.z );
  
  FIELD_CAMERA_SetAnglePitch( p_camera, cp_work->pitch );
  FIELD_CAMERA_SetAngleLen( p_camera, cp_work->len );
	FIELD_CAMERA_SetAngleYaw( p_camera, yaw );
}


//------------------------------------------------------------------
//  進行方向に対するアングルを固定
//------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_FixAngleLineWay_XZ( const FIELD_RAIL_MAN * man )
{
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
  const RAIL_CAMERA_SET * cline;
  FIELD_CAMERA* p_camera;
	const RAIL_CAMERAFUNC_FIXANGLE_LINEWAY* cp_work;
  VecFx16 way, xz_way;
  u16 front_rot;

//	TOMOYA_Printf( "target x[0x%x] y[0x%x] z[0x%x]\n", cp_target->x, cp_target->y, cp_target->z );

	cline = FIELD_RAIL_GetCameraSet( work );

	cp_work = (const RAIL_CAMERAFUNC_FIXANGLE_LINEWAY*)cline->work;

  p_camera = FIELD_RAIL_MAN_GetCamera( man );

	// 座標直指定モード
	FIELD_CAMERA_SetMode( p_camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

	// デフォルトターゲットを参照
	FIELD_CAMERA_BindDefaultTarget( FIELD_RAIL_MAN_GetCamera(man) );
  
	
  // ラインの進行方向を取得
  FIELD_RAIL_WORK_GetFrontWay( work, &way );
  front_rot = FX_Atan2Idx( -way.x, -way.z );

  /*
  TOMOYA_Printf( "yaw 0x%x\n", front_rot + cp_work->yaw );
  TOMOYA_Printf( "pitch 0x%x\n", cp_work->pitch );
  TOMOYA_Printf( "len 0x%x\n", cp_work->len );
  //*/
  
  FIELD_CAMERA_SetAnglePitch( p_camera, cp_work->pitch );
  FIELD_CAMERA_SetAngleLen( p_camera, cp_work->len );
	FIELD_CAMERA_SetAngleYaw( p_camera, front_rot + cp_work->yaw );
}

void FIELD_RAIL_CAMERAFUNC_FixAngleLineWay_XYZ( const FIELD_RAIL_MAN * man )
{
	const FIELD_RAIL_WORK* work = FIELD_RAIL_MAN_GetBindWork( man );
  const RAIL_CAMERA_SET * cline;
  FIELD_CAMERA* p_camera;
	const RAIL_CAMERAFUNC_FIXANGLE_LINEWAY* cp_work;
  VecFx16 way, xz_way;
  u16 front_rot;
  u16 front_rot_pitch;
  fx32 xz_dist;

//	TOMOYA_Printf( "target x[0x%x] y[0x%x] z[0x%x]\n", cp_target->x, cp_target->y, cp_target->z );

	cline = FIELD_RAIL_GetCameraSet( work );

	cp_work = (const RAIL_CAMERAFUNC_FIXANGLE_LINEWAY*)cline->work;

  p_camera = FIELD_RAIL_MAN_GetCamera( man );

	// 座標直指定モード
	FIELD_CAMERA_SetMode( p_camera, FIELD_CAMERA_MODE_CALC_CAMERA_POS );

	// デフォルトターゲットを参照
	FIELD_CAMERA_BindDefaultTarget( FIELD_RAIL_MAN_GetCamera(man) );
  
	
  // ラインの進行方向を取得
  FIELD_RAIL_WORK_GetFrontWay( work, &way );
  front_rot = FX_Atan2Idx( -way.x, -way.z );

  xz_way.x = way.x;
  xz_way.z = way.z;
  xz_way.y = 0;
  xz_dist = VEC_Fx16Mag( &xz_way );
  front_rot_pitch = FX_Atan2Idx( -way.y, xz_dist );

  /*
  TOMOYA_Printf( "yaw 0x%x\n", front_rot + cp_work->yaw );
  TOMOYA_Printf( "pitch 0x%x\n", cp_work->pitch );
  TOMOYA_Printf( "len 0x%x\n", cp_work->len );
  //*/
  
  FIELD_CAMERA_SetAnglePitch( p_camera, front_rot_pitch + cp_work->pitch );
  FIELD_CAMERA_SetAngleLen( p_camera, cp_work->len );
	FIELD_CAMERA_SetAngleYaw( p_camera, front_rot + cp_work->yaw );
}



// 
//----------------------------------------------------------------------------
/**
 *	@brief  平面とチェックポジションの判定
 *
 *	@retval TRUE    あたる
 *	@retval FALSE   あたらない
 *
 *  cp_pos - endpos
 *	startpos - endpos   endposを基準にすべてを計算しています。
 */
//-----------------------------------------------------------------------------
static BOOL is_Hit3DPosPlane( const RAIL_POINT * cp_point_s, const RAIL_POINT * cp_point_e, const VecFx32* cp_pos, VecFx32* p_cross, fx32* dis_bcos, VecFx32* gaiseki )
{
  enum 
  { 
    HIT_HEIGHT  = FX32_ONE * 8,   // 吸い付く範囲
    CULLING_SIZE  = FX32_ONE * 256, // 確認しない、交点との距離
  };
  VecFx32 vec_a, vec_b, way_a;
  fx32 dis_a, cross_pos;
  fx32 div_num;
  VecFx32 vecn_a, vecn_b;


  DEBUG_RAIL_Printf( "\nis_Hit3DPosPlane\n" );

  DEBUG_RAIL_Printf( "pos a x[%d] y[%d] z[%d]\n", FX_Whole( cp_point_s->pos.x ), FX_Whole( cp_point_s->pos.y ), FX_Whole( cp_point_s->pos.z ) );
  DEBUG_RAIL_Printf( "pos b x[%d] y[%d] z[%d]\n", FX_Whole( cp_point_e->pos.x ), FX_Whole( cp_point_e->pos.y ), FX_Whole( cp_point_e->pos.z ) );

  DEBUG_RAIL_Printf( "check pos x[%d] y[%d] z[%d]\n", FX_Whole( cp_pos->x ), FX_Whole( cp_pos->y ), FX_Whole( cp_pos->z ) );

  // ラインの中に点はいるか？
  // A=point_s - point_e
  // B=point_s -> cp_pos
  // |A||B|cos / |A| = |B|cos(BベクトルのＡへの正射影)
  // |B|cosが|A|より大きくなったとき、cosが90を超えたとき中にはいない
  VEC_Subtract( &cp_point_s->pos, &cp_point_e->pos, &vec_a );
  VEC_Subtract( cp_pos, &cp_point_e->pos, &vec_b );


  VEC_Normalize( &vec_a, &way_a );
  dis_a = VEC_Mag( &vec_a );

  // 値がでかすぎるときがあるので値を1/|A|にする
  vec_a = way_a;
  {
    fx32 dist_b = VEC_Mag( &vec_b );
    
    VEC_Normalize( &vec_b, &vec_b );
    div_num = dis_a;
    dist_b = FX_Div( dist_b, dis_a );
    dis_a = FX32_ONE;
    vec_b.x = FX_Mul( vec_b.x, dist_b );
    vec_b.y = FX_Mul( vec_b.y, dist_b );
    vec_b.z = FX_Mul( vec_b.z, dist_b );
  }

  *dis_bcos = VEC_DotProduct( &vec_a, &vec_b );

  //*dis_bcos = FX_Div( *dis_bcos, dis_a ); dis_aはFX32_ONEなので必要なし
  DEBUG_RAIL_Printf( "dis_bsoc[%d] , dis_a[%d]\n", FX_Whole(*dis_bcos), FX_Whole(dis_a) );

  if( *dis_bcos > dis_a )
  {
    DEBUG_RAIL_Printf( "dis_bsoc[%d] > dis_a[%d]\n", FX_Whole(*dis_bcos), FX_Whole(dis_a) );
    return FALSE;
  }
  if( *dis_bcos < 0 )
  {
    DEBUG_RAIL_Printf( "dis_bsoc[%d] < 0\n", FX_Whole(*dis_bcos) );
    return FALSE;
  }

  {
    // 1/|A|をもとに戻す
    *dis_bcos = FX_Mul( *dis_bcos, div_num );
  }

  // 外積も求めておく
  VEC_Normalize( &vec_a, &vecn_a );
  VEC_Normalize( &vec_b, &vecn_b );
  VEC_CrossProduct( &vecn_a, &vecn_b, gaiseki );

  // 交点を求める。
  // Y座標が、誤差以上離れていたら確認しない
  // １ブロック分の距離が離れていたら、確認しない。
  VEC_Set( p_cross, 
      FX_Mul( way_a.x, *dis_bcos ),
      FX_Mul( way_a.y, *dis_bcos ),
      FX_Mul( way_a.z, *dis_bcos ) );
  VEC_Add( &cp_point_e->pos, p_cross, p_cross );

  DEBUG_RAIL_Printf( "cross pos x[%d] y[%d] z[%d]\n", FX_Whole( p_cross->x ), FX_Whole( p_cross->y ), FX_Whole( p_cross->z ) );

  if( VEC_Distance( cp_pos, p_cross ) >= CULLING_SIZE )
  {
    DEBUG_RAIL_Printf( "CULLING dist[%d]\n", FX_Whole( VEC_Distance( cp_pos, p_cross ) ) );
    return FALSE;
  }
  if( MATH_ABS( cp_pos->y - p_cross->y ) > HIT_HEIGHT )
  {
    DEBUG_RAIL_Printf( "HIT_HEIGHT height[%d]\n", FX_Whole( MATH_ABS( cp_pos->y - p_cross->y ) ) );
    return FALSE;
  }

  return TRUE;
}
