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

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static void updateCircleCamera( const FIELD_RAIL_MAN * man, u16 pitch, fx32 len, const VecFx32* cp_target );


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
void FIELD_RAIL_POSFUNC_StraitLine(const FIELD_RAIL * rail, VecFx32 * pos)
{
  //LINEにいるときはPOINTとPOINTの間の座標
  //pos = p0 + (p1 - p0) * (ofs / div )
  const RAIL_LINE * nLine = FIELD_RAIL_GetLine( rail );
	s32 line_ofs = FIELD_RAIL_GetLineOfs( rail );
	s32 line_ofs_max = FIELD_RAIL_GetLineOfsMax( rail );
  fx32 ofs = FX_Div( (line_ofs * FX32_ONE), line_ofs_max<<FX32_SHIFT);
  VecFx32 val;
	const RAIL_POINT* point_s = FIELD_RAIL_GetPointStart( rail );
	const RAIL_POINT* point_e = FIELD_RAIL_GetPointEnd( rail );
  const VecFx32 * p0 = &point_s->pos;
  const VecFx32 * p1 = &point_e->pos;
  VEC_Subtract(p1, p0, &val);
  VEC_MultAdd(ofs, &val, p0, pos);

  {
    VecFx32 xzNormal = {0,FX32_ONE, 0}; //XZ平面に垂直な単位ベクトル
    VecFx32 width;
		s32 width_ofs = FIELD_RAIL_GetWidthOfs( rail );
		fx32 ofs_unit = FIELD_RAIL_GetOfsUnit( rail );
    fx32 w_ofs = width_ofs * ofs_unit;
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
void FIELD_RAIL_POSFUNC_CurveLine(const FIELD_RAIL * rail, VecFx32 * pos)
{
  const RAIL_LINE * nLine = FIELD_RAIL_GetLine(rail);
	const RAIL_LINEPOS_SET* line_pos_set;
	const RAIL_POINT* point_s = FIELD_RAIL_GetPointStart( rail );
	const RAIL_POINT* point_e = FIELD_RAIL_GetPointEnd( rail );
  const VecFx32 * p_s = &point_s->pos;
  const VecFx32 * p_e = &point_e->pos;
	const RAIL_POSFUNC_CURVE_WORK* work;
  VecFx32 center, vec_s, vec_e, vec_i;
	s32 line_ofs			= FIELD_RAIL_GetLineOfs( rail );
	s32 line_ofs_max	= FIELD_RAIL_GetLineOfsMax( rail );
  fx32 ofs = (line_ofs * FX32_ONE) / line_ofs_max;


	// ライン動作のつながりが良くなるように、line_ofs==0 or line_ofs == line_ofs_max
	// のときはそれぞれの点の位置に主人公がいるようにする。
	if( line_ofs == 0 )
	{
		*pos = *p_s;
	}
	else if( line_ofs == line_ofs_max )
	{
		*pos = *p_e;
	}
	else
	{
		// Y座標は、線形で求める。
		// XZは、回転でもとめる
		pos->y = p_e->y - p_s->y;
		pos->y = FX_Div( FX_Mul( pos->y, line_ofs<<FX32_SHIFT ), line_ofs_max<<FX32_SHIFT );
		pos->y += p_s->y;
		

		line_pos_set = FIELD_RAIL_GetLinePosSet( rail );
		work = (const RAIL_POSFUNC_CURVE_WORK*)line_pos_set->work;
		center.x = work->x;
		center.y = pos->y;
		center.z = work->z;

		VEC_Subtract(p_s, &center, &vec_s);
		VEC_Subtract(p_e, &center, &vec_e);
		vec_s.y = 0;
		vec_e.y = 0;

		
		getIntermediateVector(&vec_i, &vec_s, &vec_e, ofs);
		VEC_Add(&center, &vec_i, pos);
		{
			VecFx32 w_vec;
			s32 width_ofs = FIELD_RAIL_GetWidthOfs( rail );
			fx32 ofs_unit = FIELD_RAIL_GetOfsUnit( rail );
			VEC_Normalize(&vec_i, &w_vec);
			VEC_MultAdd(-(width_ofs) * ofs_unit, &w_vec, pos, pos);
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


//============================================================================================
//
//
//      ２点間の距離を求める
//
//
//============================================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	直線
 */
//-----------------------------------------------------------------------------
fx32 FIELD_RAIL_LINE_DIST_FUNC_StraitLine( const RAIL_POINT * point_s, const RAIL_POINT * point_e, const RAIL_LINEPOS_SET * line_pos_set )
{
	return VEC_Distance( &point_s->pos, &point_e->pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief	円
 */
//-----------------------------------------------------------------------------
fx32 FIELD_RAIL_LINE_DIST_FUNC_CircleLine( const RAIL_POINT * point_s, const RAIL_POINT * point_e, const RAIL_LINEPOS_SET * line_pos_set )
{
	u16     rotate;
	VecFx32 center;
	fx32    r;
	fx32    circle_par;
  VecFx32 p0;
  VecFx32 p1;
  fx32 dist;
	const RAIL_POSFUNC_CURVE_WORK* work;

  p0 = point_s->pos;
  p1 = point_e->pos;

	work = (const RAIL_POSFUNC_CURVE_WORK*)line_pos_set->work;
	center.x = work->x;
	center.y = work->y;
	center.z = work->z;

	// 中心からの距離に変更
	VEC_Subtract( &p0, &center, &p0 );
	VEC_Subtract( &p1, &center, &p1 );

	// 中心からの距離は、平均を使う
	// 直径を求めたいので、２でわらない
	r = VEC_Mag( &p0 );
	r += VEC_Mag( &p1 );

	// 角度を求める
	VEC_Normalize( &p0, &p0 );
	VEC_Normalize( &p1, &p1 );
	rotate = FX_AcosIdx( VEC_DotProduct( &p0, &p1 ) );  // 180以下の角度が戻ってくる
	
	
	// 角度の比を求める
	circle_par = FX_Div( rotate<<FX32_SHIFT, 0x10000<<FX32_SHIFT );
/*    OS_TPrintf( "circle_par 0x%x ", circle_par );
	OS_TPrintf( "rotate=%d ", rotate / 182 );
	OS_TPrintf( "r=0x%x\n", r );//*/

	// 円周から、距離を求める
	dist = FX_Mul(r, FX32_CONST(3.140f));
	dist = FX_Mul( dist, circle_par );

	return dist;
}




//------------------------------------------------------------------
/**
 * @brief カメラ指定：パラメータをFx32３つとして単純に足すだけ
 */
//------------------------------------------------------------------
void FIELD_RAIL_CAMERAFUNC_FixPosCamera(const FIELD_RAIL_MAN* man)
{
  VecFx32 pos;
	const FIELD_RAIL* rail = FIELD_RAIL_MAN_GetNowRail( man );
  const RAIL_CAMERA_SET * cam_set = FIELD_RAIL_GetCameraSet(rail);
	const RAIL_CAMERAFUNC_FIXPOS_WORK* work;
	
  FIELD_RAIL_MAN_GetPos(man, &pos);

	// 座標直指定モード
	FIELD_CAMERA_SetMode( FIELD_RAIL_MAN_GetCamera(man), FIELD_CAMERA_MODE_DIRECT_POS );

	work = (const RAIL_CAMERAFUNC_FIXPOS_WORK*)cam_set->work;
  pos.x += work->x;
  pos.y += work->y;
  pos.z += work->z;
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
	const FIELD_RAIL* rail = FIELD_RAIL_MAN_GetNowRail( man );
  const RAIL_CAMERA_SET * cam_set = FIELD_RAIL_GetCameraSet(rail);
	const RAIL_CAMERAFUNC_FIXANGLE_WORK* work;

	work = (const RAIL_CAMERAFUNC_FIXANGLE_WORK*)cam_set->work;

	// 座標直指定モード
	FIELD_CAMERA_SetMode( cam, FIELD_CAMERA_MODE_DIRECT_POS );

	// デフォルトターゲットを参照
	FIELD_CAMERA_BindDefaultTarget( FIELD_RAIL_MAN_GetCamera(man) );

  FIELD_RAIL_MAN_GetPos( man, &pos );
  {
    VecFx32 camPos;
    u16 yaw = (u16)work->yaw;
    u16 pitch = (u16)work->pitch;
    fx32 len = work->len;

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
  const FIELD_RAIL * rail = FIELD_RAIL_MAN_GetNowRail( man );
	const RAIL_POINT* point_s = FIELD_RAIL_GetPointStart( rail );
	const RAIL_POINT* point_e = FIELD_RAIL_GetPointEnd( rail );
  const RAIL_CAMERA_SET * cs;
  const RAIL_CAMERA_SET * ce;
	FIELD_CAMERA* cam = FIELD_RAIL_MAN_GetCamera( man );
	const RAIL_CAMERAFUNC_FIXANGLE_WORK* cs_work;
	const RAIL_CAMERAFUNC_FIXANGLE_WORK* ce_work;
	
  GF_ASSERT(FIELD_RAIL_GetType(rail) == FIELD_RAIL_TYPE_LINE);

	// 座標直指定モード
	FIELD_CAMERA_SetMode( cam, FIELD_CAMERA_MODE_DIRECT_POS );

	// デフォルトターゲットを参照
	FIELD_CAMERA_BindDefaultTarget( FIELD_RAIL_MAN_GetCamera(man) );

  cs = FIELD_RAIL_POINT_GetCameraSet( rail, point_s );
//  GF_ASSERT(getRailDatCameraFunc( &man->rail_dat, cs->func_index ) == FIELD_RAIL_CAMERAFUNC_FixAngleCamera);
  ce = FIELD_RAIL_POINT_GetCameraSet( rail, point_e );
//  GF_ASSERT(getRailDatCameraFunc( &man->rail_dat, ce->func_index ) == FIELD_RAIL_CAMERAFUNC_FixAngleCamera);

	cs_work = (const RAIL_CAMERAFUNC_FIXANGLE_WORK*)cs->work;
	ce_work = (const RAIL_CAMERAFUNC_FIXANGLE_WORK*)ce->work;
	
  {
    u32 div = FIELD_RAIL_GetLineOfsMax(rail);
		s32 line_ofs = FIELD_RAIL_GetLineOfs(rail);
    fx32 t = FX32_ONE * line_ofs / div;

    getVectorFromAngleValue(&c_s, cs_work->yaw, cs_work->pitch, cs_work->len);
    getVectorFromAngleValue(&c_e, ce_work->yaw, ce_work->pitch, ce_work->len);

		getIntermediateVector(&c_now, &c_s, &c_e, t);

		FIELD_RAIL_MAN_GetPos( man, &target );
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
	const FIELD_RAIL* rail = FIELD_RAIL_MAN_GetNowRail( man );
  const RAIL_CAMERA_SET * cam_set = FIELD_RAIL_GetCameraSet(rail);
	FIELD_CAMERA* cam = FIELD_RAIL_MAN_GetCamera( man );
	const RAIL_CAMERAFUNC_FIXALL_WORK* work;

	work = (const RAIL_CAMERAFUNC_FIXALL_WORK*)cam_set->work;

	// 座標直指定モード
	FIELD_CAMERA_SetMode( cam, FIELD_CAMERA_MODE_DIRECT_POS );

	// 追従ターゲットNULL
	FIELD_CAMERA_FreeTarget( FIELD_RAIL_MAN_GetCamera(man) );

  pos.x = work->pos_x;
  pos.y = work->pos_y;
  pos.z = work->pos_z;
  target.x = work->target_x;
  target.y = work->target_y;
  target.z = work->target_z;
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
	const FIELD_RAIL* rail = FIELD_RAIL_MAN_GetNowRail( man );
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

	type = FIELD_RAIL_GetType( rail );


  if( type == FIELD_RAIL_TYPE_LINE )
  {
		point_s = FIELD_RAIL_GetPointStart( rail );
		point_e = FIELD_RAIL_GetPointEnd( rail );
    cs = FIELD_RAIL_POINT_GetCameraSet( rail, point_s );
    ce = FIELD_RAIL_POINT_GetCameraSet( rail, point_e );
    div = FIELD_RAIL_GetLineOfsMax(rail);
    ofs = FIELD_RAIL_GetLineOfs(rail);
  }
  else
  {
    cs = FIELD_RAIL_GetCameraSet( rail );
    ce = FIELD_RAIL_GetCameraSet( rail );
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
	const FIELD_RAIL* rail = FIELD_RAIL_MAN_GetNowRail( man );
  const RAIL_CAMERA_SET * cline;
  u16 pitch;
  fx32 len;
  VecFx32 target;
	const RAIL_CAMERAFUNC_FIXLEN_CIRCLE_WORK* work;

	cline = FIELD_RAIL_GetCameraSet( rail );

	work = (const RAIL_CAMERAFUNC_FIXLEN_CIRCLE_WORK*)cline->work;

  // pitch len の計算
  pitch = work->pitch;
  len   = work->len;

  target.x = work->target_x;
  target.y = work->target_y;
  target.z = work->target_z;

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
  
  FIELD_RAIL_MAN_GetPos( man, &pos );
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

