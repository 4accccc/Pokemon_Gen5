//==============================================================================
/**
 * @file	quaternion.c
 * @brief	簡単な説明を書く
 * @author	goto
 * @date	2007.10.09(火)
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================
#include "gflib.h"

#if 1
//FX32計算バージョン

static	void	MakeRotationalQuaternion(GFL_QUATERNION *q,GFL_QUATERNION_MTX44 mtx);

//--------------------------------------------------------------
/**
 * @brief	単位クォータニオン作成
 *
 * @param[out]	qt	単位クォータニオンを生成する構造体ポインタ
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Identity( GFL_QUATERNION* qt )
{
	qt->wr = FX32_ONE;
	qt->xi = 0;
	qt->yi = 0;
	qt->zi = 0;
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンのコピー ( 代入してるだけ )
 *
 * @param[out]	q1	
 * @param[out]	q2	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Copy( GFL_QUATERNION* q1, GFL_QUATERNION* q2 )
{
	*q1 = *q2;
}

//--------------------------------------------------------------
/**
 * @brief	クォータニオンを軸ごとの回転値から生成
 *
 * @param[out]	qt		生成したクォータニオンを格納する構造体ポインタ
 * @param[in]	rot_x	Ｘ軸回転値
 * @param[in]	rot_y	Ｙ軸回転値
 * @param[in]	rot_z	Ｚ軸回転値
 *
 * @retval	none	
 */
//--------------------------------------------------------------
void GFL_QUAT_MakeQuaternionXYZ( GFL_QUATERNION* qt, u16 rot_x, u16 rot_y,u16 rot_z )
{
#if 1
	MtxFx44	mtx;
	fx32	sss=FX_SinIdx(rot_y);

	if((rot_x==0)&&(rot_y==0)&&(rot_z==0)){
		GFL_QUAT_Identity(qt);
		return;
	}

	qt->wr=FX_CosIdx(rot_y);
	qt->xi=0;
	qt->yi=FX_Mul(FX32_ONE,sss);
	qt->zi=0;

	GFL_QUAT_SetMtx44(&mtx,qt);
#elif 0
	fx32	a=FX_Mul(FX_SinIdx(rot_y/2),FX_SinIdx(rot_x/2));
	fx32	b=FX_Mul(FX_CosIdx(rot_y/2),FX_CosIdx(rot_x/2));
	fx32	c=FX_Mul(FX_SinIdx(rot_y/2),FX_CosIdx(rot_x/2));
	fx32	d=FX_Mul(FX_CosIdx(rot_y/2),FX_SinIdx(rot_x/2));

	OS_Printf("a:%08x b:%08x c:%08x d:%08x\n",a,b,c,d);

	qt->xi	= FX_Mul(FX_CosIdx(rot_z/2) , d) - FX_Mul(FX_SinIdx(rot_z/2) , c);
	qt->yi	= FX_Mul(FX_SinIdx(rot_z/2) , d) + FX_Mul(FX_CosIdx(rot_z/2) , c);
	qt->zi	= FX_Mul(FX_SinIdx(rot_z/2) , b) - FX_Mul(FX_CosIdx(rot_z/2) , a);
	qt->wr	= FX_Mul(FX_CosIdx(rot_z/2) , b) + FX_Mul(FX_SinIdx(rot_z/2) , a);
#elif 0
	MtxFx33	mtx,mtx_x,mtx_y,mtx_z;
	GFL_QUATERNION_MTX44	qmtx;

//	MTX_Identity33(&mtx);
	MTX_RotX33(&mtx_x,FX_SinIdx(rot_x),FX_CosIdx(rot_x));
	MTX_RotY33(&mtx_y,FX_SinIdx(rot_y),FX_CosIdx(rot_y));
	MTX_RotZ33(&mtx_z,FX_SinIdx(rot_z),FX_CosIdx(rot_z));
	MTX_Concat33(&mtx_x,&mtx_y,&mtx);
	MTX_Concat33(&mtx,&mtx_z,&mtx);

	OS_Printf("MakeQ\n");
	OS_Printf("_00:%08x _01:%08x _02:%08x\n",mtx._00,mtx._01,mtx._02);
	OS_Printf("_10:%08x _11:%08x _12:%08x\n",mtx._10,mtx._11,mtx._12);
	OS_Printf("_20:%08x _21:%08x _22:%08x\n",mtx._20,mtx._21,mtx._22);

	qmtx._00=mtx._00;
	qmtx._01=mtx._01;
	qmtx._02=mtx._02;
	qmtx._03=0;
	qmtx._10=mtx._10;
	qmtx._11=mtx._11;
	qmtx._12=mtx._12;
	qmtx._13=0;
	qmtx._20=mtx._20;
	qmtx._21=mtx._21;
	qmtx._22=mtx._22;
	qmtx._23=0;
	qmtx._30=0;
	qmtx._31=0;
	qmtx._32=0;
	qmtx._33=FX32_ONE;
	
	MakeRotationalQuaternion(qt,qmtx);
#endif
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンの積
 *
 * @pwram	q		格納先
 * @pwram	q1		クォータニオン
 * @pwram	q2		クォータニオン
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Mul( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 )
{
	q->wr = FX_Mul( q1->wr , q2->wr ) - FX_Mul( q1->xi , q2->xi ) - FX_Mul( q1->yi , q2->yi ) - FX_Mul( q1->zi , q2->zi );
	q->xi = FX_Mul( q1->wr , q2->xi ) + FX_Mul( q1->xi , q2->wr ) + FX_Mul( q1->yi , q2->zi ) - FX_Mul( q1->zi , q2->yi );
	q->yi = FX_Mul( q1->wr , q2->yi ) - FX_Mul( q1->xi , q2->zi ) + FX_Mul( q1->yi , q2->wr ) + FX_Mul( q1->zi , q2->xi );
	q->zi = FX_Mul( q1->wr , q2->zi ) + FX_Mul( q1->xi , q2->yi ) - FX_Mul( q1->yi , q2->xi ) + FX_Mul( q1->zi , q2->wr );
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンの加算
 *
 * @pwram	q	
 * @pwram	q1	
 * @pwram	q2	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Add( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 )
{
	q->wr = q1->wr + q2->wr;
	q->xi = q1->xi + q2->xi;
	q->yi = q1->yi + q2->yi;
	q->zi = q1->zi + q2->zi;
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンの減算
 *
 * @pwram	q	
 * @pwram	q1	
 * @pwram	q2	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Sub( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 )
{
	q->wr = q1->wr - q2->wr;
	q->xi = q1->xi - q2->xi;
	q->yi = q1->yi - q2->yi;
	q->zi = q1->zi - q2->zi;
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンのノルム || a || ^ 2
 *
 * @pwram	q	
 *
 * @retval	f32	
 *
 */
//--------------------------------------------------------------
fx32 GFL_QUAT_GetNormSqrt( const GFL_QUATERNION* q )
{
	return FX_Mul( q->wr , q->wr ) + FX_Mul( q->xi , q->xi ) + FX_Mul( q->yi , q->yi ) + FX_Mul( q->zi , q->zi );
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンのノルム || a ||
 *
 * @pwram	q	
 *
 * @retval	f32	
 *
 */
//--------------------------------------------------------------
fx32 GFL_QUAT_GetNorm( const GFL_QUATERNION* q )
{
	fx32 n;
	
	n = GFL_QUAT_GetNormSqrt( q );
	
	return FX_Sqrt( n ) ;
}

//--------------------------------------------------------------
/**
 * @brief	クォータニオンをマトリックスに設定
 *
 * @pwram	qmtx			マトリックス格納先
 * @pwram	quaternion		クォータニオン
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_SetMtx44( MtxFx44* mtx, const GFL_QUATERNION* q )
{
#if 0
	GFL_QUATERNION_MTX44	qmtx;
	fx32 ww;
	fx32 xx;
	fx32 yy;
	fx32 zz;
	fx32 xy;
	fx32 xz;
	fx32 yz;
	fx32 wx;
	fx32 wy;
	fx32 wz;

	ww = FX_Mul(q->wr , q->wr);
	xx = FX_Mul(q->xi , q->xi);
	yy = FX_Mul(q->yi , q->yi);
	zz = FX_Mul(q->zi , q->zi);

	xy = FX_Mul(q->xi , q->yi);
	xz = FX_Mul(q->xi , q->zi);
	yz = FX_Mul(q->yi , q->zi);
	wx = FX_Mul(q->wr , q->xi);
	wy = FX_Mul(q->wr , q->yi);
	wz = FX_Mul(q->wr , q->zi);

	FX_Mul( xy , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( xz , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( yz , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( wx , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( wy , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( wz , FX_Mul(FX32_ONE,FX32_ONE*2));

	qmtx._00 = ww + xx - yy -zz;
	qmtx._01 = xy - wz;
	qmtx._02 = xz + wy;
	qmtx._03 = 0;

	qmtx._10 = xy + wz;
	qmtx._11 = ww - xx + yy - zz;
	qmtx._12 = yz - wx;
	qmtx._13 = 0;

	qmtx._20 = xz - wy;
	qmtx._21 = yz + wx;
	qmtx._22 = ww - xx - yy + zz;
	qmtx._23 = 0.0;

	qmtx._30 = 0.0;
	qmtx._31 = 0.0;
	qmtx._32 = 0.0;
	qmtx._33 = ww + xx + yy + zz;

	GFL_QUAT_SetQMtx44_to_MtxFx44( &qmtx, mtx );

	OS_Printf("QtoMTX\n");
	OS_Printf("_00:%08x _01:%08x _02:%08x _03:%08x\n",mtx->_00,mtx->_01,mtx->_02,mtx->_03);
	OS_Printf("_10:%08x _11:%08x _12:%08x _13:%08x\n",mtx->_10,mtx->_11,mtx->_12,mtx->_13);
	OS_Printf("_20:%08x _21:%08x _22:%08x _23:%08x\n",mtx->_20,mtx->_21,mtx->_22,mtx->_23);
	OS_Printf("_30:%08x _31:%08x _32:%08x _33:%08x\n",mtx->_30,mtx->_31,mtx->_32,mtx->_33);
#endif
//これもだめだ～
#if 0
	GFL_QUATERNION_MTX44	qmtx;
	fx32 x2;
	fx32 y2;
	fx32 z2;
	fx32 w2;
	fx32 s;
	fx32 vx;
	fx32 vy;
	fx32 vz;
	fx32 wx;
	fx32 wy;
	fx32 wz;
	fx32 sx;
	fx32 sy;
	fx32 sz;
	fx32 cx;
	fx32 cy;
	fx32 cz;


	x2 = FX_Mul(q->xi , q->xi);
	y2 = FX_Mul(q->yi , q->yi);
	z2 = FX_Mul(q->zi , q->zi);
	w2 = FX_Mul(q->wr , q->wr);
	s  = FX_Sqrt(w2+x2+y2+z2);
	s  = FX_Mul(s,s);
	s  = FX_Div(FX32_ONE*2,s);
	vx = FX_Mul(q->xi , s);
	vy = FX_Mul(q->yi , s);
	vz = FX_Mul(q->zi , s);
	wx = FX_Mul(vx , q->wr);
	wy = FX_Mul(vy , q->wr);
	wz = FX_Mul(vz , q->wr);
	sx = FX_Mul(q->xi , vx);
	sy = FX_Mul(q->yi , vy);
	sz = FX_Mul(q->zi , vz);
	cx = FX_Mul(q->yi , vz);
	cy = FX_Mul(q->zi , vx);
	cz = FX_Mul(q->xi , vy);

	qmtx._00 = FX32_ONE - sy - sz;
	qmtx._01 = cz - wz;
	qmtx._02 = cy + wy;
	qmtx._03 = 0;

	qmtx._10 = cz + wz;
	qmtx._11 = FX32_ONE - sx - sz;
	qmtx._12 = cx - wx;
	qmtx._13 = 0;

	qmtx._20 = cy - wy;
	qmtx._21 = cx + wx;
	qmtx._22 = FX32_ONE - sx - sy;
	qmtx._23 = 0.0;

	qmtx._30 = 0.0;
	qmtx._31 = 0.0;
	qmtx._32 = 0.0;
	qmtx._33 = FX32_ONE;

	GFL_QUAT_SetQMtx44_to_MtxFx44( &qmtx, mtx );

	OS_Printf("QtoMTX\n");
	OS_Printf("_00:%08x _01:%08x _02:%08x _03:%08x\n",mtx->_00,mtx->_01,mtx->_02,mtx->_03);
	OS_Printf("_10:%08x _11:%08x _12:%08x _13:%08x\n",mtx->_10,mtx->_11,mtx->_12,mtx->_13);
	OS_Printf("_20:%08x _21:%08x _22:%08x _23:%08x\n",mtx->_20,mtx->_21,mtx->_22,mtx->_23);
	OS_Printf("_30:%08x _31:%08x _32:%08x _33:%08x\n",mtx->_30,mtx->_31,mtx->_32,mtx->_33);
#endif

//左手座標系の計算らしいのでDSだとおかしな計算になるっぽい
#if 1
	GFL_QUATERNION_MTX44	qmtx;

	fx32 x2 = FX_Mul(q->xi , q->xi);
	fx32 y2 = FX_Mul(q->yi , q->yi);
	fx32 z2 = FX_Mul(q->zi , q->zi);
	fx32 xy = FX_Mul(q->xi , q->yi);
	fx32 yz = FX_Mul(q->yi , q->zi);
	fx32 zx = FX_Mul(q->zi , q->xi);
	fx32 xw = FX_Mul(q->xi , q->wr);
	fx32 yw = FX_Mul(q->yi , q->wr);
	fx32 zw = FX_Mul(q->zi , q->wr);

	FX_Mul( x2 , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( y2 , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( z2 , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( xy , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( yz , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( zx , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( xw , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( yw , FX_Mul(FX32_ONE,FX32_ONE*2));
	FX_Mul( zw , FX_Mul(FX32_ONE,FX32_ONE*2));
	
	qmtx._00 = FX32_ONE - (y2 + z2);
	qmtx._01 = xy + zw;
	qmtx._02 = zx - yw;
	qmtx._03 = 0.0;
	qmtx._10 = xy - zw;
	qmtx._11 = FX32_ONE - (z2 + x2);
	qmtx._12 = yz + xw;
	qmtx._13 = 0.0;
	qmtx._20 = zx + yw;
	qmtx._21 = yz - xw;
	qmtx._22 = FX32_ONE - (x2 + y2);
	qmtx._23 = 0.0;
	qmtx._30 = 0.0;
	qmtx._31 = 0.0;
	qmtx._32 = 0.0;
	qmtx._33 = FX32_ONE;

	GFL_QUAT_SetQMtx44_to_MtxFx44( &qmtx, mtx );

	OS_Printf("QtoMTX\n");
	OS_Printf("_00:%08x _01:%08x _02:%08x _03:%08x\n",mtx->_00,mtx->_01,mtx->_02,mtx->_03);
	OS_Printf("_10:%08x _11:%08x _12:%08x _13:%08x\n",mtx->_10,mtx->_11,mtx->_12,mtx->_13);
	OS_Printf("_20:%08x _21:%08x _22:%08x _23:%08x\n",mtx->_20,mtx->_21,mtx->_22,mtx->_23);
	OS_Printf("_30:%08x _31:%08x _32:%08x _33:%08x\n",mtx->_30,mtx->_31,mtx->_32,mtx->_33);
#endif
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンをマトリックスに設定
 *
 * @pwram	qmtx			マトリックス格納先
 * @pwram	quaternion		クォータニオン
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_SetQMtx44( GFL_QUATERNION_MTX44* qmtx, const GFL_QUATERNION* q )
{
	fx32 x2 = FX_Mul(q->xi , q->xi) ;
	fx32 y2 = FX_Mul(q->yi , q->yi) ;
	fx32 z2 = FX_Mul(q->zi , q->zi) ;
	fx32 xy = FX_Mul(q->xi , q->yi) ;
	fx32 yz = FX_Mul(q->yi , q->zi) ;
	fx32 zx = FX_Mul(q->zi , q->xi) ;
	fx32 xw = FX_Mul(q->xi , q->wr) ;
	fx32 yw = FX_Mul(q->yi , q->wr) ;
	fx32 zw = FX_Mul(q->zi , q->wr) ;

	FX_Mul( x2 , (FX32_ONE<<1));
	FX_Mul( y2 , (FX32_ONE<<1));
	FX_Mul( z2 , (FX32_ONE<<1));
	FX_Mul( xy , (FX32_ONE<<1));
	FX_Mul( yz , (FX32_ONE<<1));
	FX_Mul( zx , (FX32_ONE<<1));
	FX_Mul( xw , (FX32_ONE<<1));
	FX_Mul( yw , (FX32_ONE<<1));
	FX_Mul( zw , (FX32_ONE<<1));
	
	qmtx->_00 = FX32_ONE - (y2 + z2);
	qmtx->_01 = xy + zw;
	qmtx->_02 = zx - yw;
	qmtx->_03 = 0;
	qmtx->_10 = xy - zw;
	qmtx->_11 = FX32_ONE - (z2 + x2);
	qmtx->_12 = yz + xw;
	qmtx->_13 = 0;
	qmtx->_20 = zx + yw;
	qmtx->_21 = yz - xw;
	qmtx->_22 = FX32_ONE - x2 + y2;
	qmtx->_23 = 0;
	qmtx->_30 = 0;
	qmtx->_31 = 0;
	qmtx->_32 = 0;
	qmtx->_33 = FX32_ONE;
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンで作成したマトリックスをfx32型の4x4マトリックスに変換
 *
 * @pwram	qmtx	クォータニオンマトリックス
 * @pwram	mtx		格納先
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_SetQMtx44_to_MtxFx44( const GFL_QUATERNION_MTX44* qmtx, MtxFx44* mtx )
{
	mtx->_00 = qmtx->_00;
	mtx->_01 = qmtx->_01;
	mtx->_02 = qmtx->_02;
	mtx->_03 = qmtx->_03;
	mtx->_10 = qmtx->_10;
	mtx->_11 = qmtx->_11;
	mtx->_12 = qmtx->_12;
	mtx->_13 = qmtx->_13;
	mtx->_20 = qmtx->_20;
	mtx->_21 = qmtx->_21;
	mtx->_22 = qmtx->_22;
	mtx->_23 = qmtx->_23;
	mtx->_30 = qmtx->_30;
	mtx->_31 = qmtx->_31;
	mtx->_32 = qmtx->_32;
	mtx->_33 = qmtx->_33;
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンで作成したマトリックスをfx32型の4x3マトリックスに変換
 *
 * @pwram	qmtx	
 * @pwram	mtx	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_SetQMtx44_to_MtxFx43( const GFL_QUATERNION_MTX44* qmtx, MtxFx43* mtx )
{
	MtxFx44 mtx44;
	
	GFL_QUAT_SetQMtx44_to_MtxFx44( qmtx, &mtx44 );
	
	MTX_Copy44To43( &mtx44, mtx );
}


//--------------------------------------------------------------
/**
 * @brief	回転量を取得 ラジアン
 *
 * @pwram	q	
 *
 * @retval	u16	
 *
 */
//--------------------------------------------------------------
u16 GFL_QUAT_GetRotation( const GFL_QUATERNION* q )
{
	u16 rad;
	
	rad = FX_AcosIdx( q->wr ) * 2;
	
	return rad;
}

//--------------------------------------------------------------
//	static関数群
//--------------------------------------------------------------
static	void	MakeRotationalQuaternion(GFL_QUATERNION *q,GFL_QUATERNION_MTX44 m)
{
	fx32 s, trace = m._00 + m._11 + m._22 + FX32_ONE;
    fx32 max = m._11 > m._22 ? m._11 : m._22;

	q->wr=q->xi=q->yi=q->zi=0;

	if (trace >= FX32_ONE) {
		s = FX_Div(FX32_HALF , FX_Sqrt(trace));
		q->wr = FX_Div(FX_Div(FX32_HALF,FX32_ONE*2) , s);
		q->xi = FX_Mul((m._12 - m._21) , s);
		q->yi = FX_Mul((m._20 - m._02) , s);
		q->zi = FX_Mul((m._01 - m._10) , s);
	}
	else {
		if (max < m._00) {
			s = FX_Sqrt((m._00 - (m._11 + m._22) + FX32_ONE));
			q->xi = FX_Mul( s , FX32_HALF);
			s = FX_Div(FX32_HALF , s);
			q->yi = FX_Mul((m._01 + m._10) , s);
			q->zi = FX_Mul((m._20 + m._02) , s);
			q->wr = FX_Mul((m._12 - m._21) , s);
		}
		else if (max == m._11) {
			s = FX_Sqrt((m._11 - (m._22 + m._00) + FX32_ONE));
			q->yi = FX_Mul(s , FX32_HALF);
			s = FX_Div(FX32_HALF , s);
			q->xi = FX_Mul((m._01 + m._10) , s);
			q->zi = FX_Mul((m._12 + m._21) , s);
			q->wr = FX_Mul((m._20 - m._02) , s);
		}
		else {
			s = FX_Sqrt((m._22 - (m._00 + m._11) + FX32_ONE));
			q->zi = FX_Mul(s , FX32_HALF);
			s = FX_Div(FX32_HALF , s);
			q->xi = FX_Mul((m._20 + m._02) , s);
			q->yi = FX_Mul((m._12 + m._21) , s);
			q->wr = FX_Mul((m._01 - m._10) , s);
		}
	}
}
#endif

//f32計算バージョン
#if 0
static	void	MakeRotationalQuaternion(GFL_QUATERNION *q,GFL_QUATERNION_MTX44 mtx);

//--------------------------------------------------------------
/**
 * @brief	単位クォータニオン作成
 *
 * @param[out]	qt	単位クォータニオンを生成する構造体ポインタ
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Identity( GFL_QUATERNION* qt )
{
	qt->wr = 1.0;
	qt->xi = 0;
	qt->yi = 0;
	qt->zi = 0;
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンのコピー ( 代入してるだけ )
 *
 * @param[out]	q1	
 * @param[out]	q2	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Copy( GFL_QUATERNION* q1, GFL_QUATERNION* q2 )
{
	*q1 = *q2;
}

//--------------------------------------------------------------
/**
 * @brief	クォータニオンを軸ごとの回転値から生成
 *
 * @param[out]	qt		生成したクォータニオンを格納する構造体ポインタ
 * @param[in]	rot_x	Ｘ軸回転値
 * @param[in]	rot_y	Ｙ軸回転値
 * @param[in]	rot_z	Ｚ軸回転値
 *
 * @retval	none	
 */
//--------------------------------------------------------------
void GFL_QUAT_MakeQuaternionXYZ( GFL_QUATERNION* qt, u16 rot_x, u16 rot_y,u16 rot_z )
{
	MtxFx33	mtx,mtx_x,mtx_y,mtx_z;
	GFL_QUATERNION_MTX44	qmtx;

	MTX_Identity33(&mtx);
	MTX_RotX33(&mtx_x,FX_SinIdx(rot_x),FX_CosIdx(rot_x));
	MTX_RotY33(&mtx_y,FX_SinIdx(rot_y),FX_CosIdx(rot_y));
	MTX_RotZ33(&mtx_z,FX_SinIdx(rot_z),FX_CosIdx(rot_z));
	MTX_Concat33(&mtx_y,&mtx,&mtx);
	MTX_Concat33(&mtx_x,&mtx,&mtx);
	MTX_Concat33(&mtx_z,&mtx,&mtx);

	qmtx._00=FX_FX32_TO_F32(mtx._00);
	qmtx._01=FX_FX32_TO_F32(mtx._01);
	qmtx._02=FX_FX32_TO_F32(mtx._02);
	qmtx._03=0.0f;
	qmtx._10=FX_FX32_TO_F32(mtx._10);
	qmtx._11=FX_FX32_TO_F32(mtx._11);
	qmtx._12=FX_FX32_TO_F32(mtx._12);
	qmtx._13=0.0f;
	qmtx._20=FX_FX32_TO_F32(mtx._20);
	qmtx._21=FX_FX32_TO_F32(mtx._21);
	qmtx._22=FX_FX32_TO_F32(mtx._22);
	qmtx._23=0.0f;
	qmtx._30=0.0f;
	qmtx._31=0.0f;
	qmtx._32=0.0f;
	qmtx._33=1.0f;
	
	MakeRotationalQuaternion(qt,qmtx);
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンの積
 *
 * @pwram	q		格納先
 * @pwram	q1		クォータニオン
 * @pwram	q2		クォータニオン
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Mul( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 )
{
	q->wr = ( q1->wr * q2->wr ) - ( q1->xi * q2->xi ) - ( q1->yi * q2->yi ) - ( q1->zi * q2->zi );
	q->xi = ( q1->wr * q2->xi ) + ( q1->xi * q2->wr ) + ( q1->yi * q2->zi ) - ( q1->zi * q2->yi );
	q->yi = ( q1->wr * q2->yi ) - ( q1->xi * q2->zi ) + ( q1->yi * q2->wr ) + ( q1->zi * q2->xi );
	q->zi = ( q1->wr * q2->zi ) + ( q1->xi * q2->yi ) - ( q1->yi * q2->xi ) + ( q1->zi * q2->wr );
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンの加算
 *
 * @pwram	q	
 * @pwram	q1	
 * @pwram	q2	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Add( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 )
{
	q->wr = q1->wr + q2->wr;
	q->xi = q1->xi + q2->xi;
	q->yi = q1->yi + q2->yi;
	q->zi = q1->zi + q2->zi;
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンの減算
 *
 * @pwram	q	
 * @pwram	q1	
 * @pwram	q2	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_Sub( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 )
{
	q->wr = q1->wr - q2->wr;
	q->xi = q1->xi - q2->xi;
	q->yi = q1->yi - q2->yi;
	q->zi = q1->zi - q2->zi;
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンのノルム || a || ^ 2
 *
 * @pwram	q	
 *
 * @retval	f32	
 *
 */
//--------------------------------------------------------------
f32 GFL_QUAT_GetNormSqrt( const GFL_QUATERNION* q )
{
	return ( q->wr * q->wr ) + ( q->xi + q->xi ) + ( q->yi + q->yi ) + ( q->zi + q->zi );
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンのノルム || a ||
 *
 * @pwram	q	
 *
 * @retval	f32	
 *
 */
//--------------------------------------------------------------
f32 GFL_QUAT_GetNorm( const GFL_QUATERNION* q )
{
	fx32 n;
	
	n = FX_F32_TO_FX32( GFL_QUAT_GetNormSqrt( q ) );
	
	return FX_F32_TO_FX32( FX_Sqrt( n ) );
}

//--------------------------------------------------------------
/**
 * @brief	クォータニオンをマトリックスに設定
 *
 * @pwram	qmtx			マトリックス格納先
 * @pwram	quaternion		クォータニオン
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_SetMtx44( MtxFx44* mtx, const GFL_QUATERNION* q )
{
	GFL_QUATERNION_MTX44	qmtx;

	f32 x2 = q->xi * q->xi * 2.0;
	f32 y2 = q->yi * q->yi * 2.0;
	f32 z2 = q->zi * q->zi * 2.0;
	f32 xy = q->xi * q->yi * 2.0;
	f32 yz = q->yi * q->zi * 2.0;
	f32 zx = q->zi * q->xi * 2.0;
	f32 xw = q->xi * q->wr * 2.0;
	f32 yw = q->yi * q->wr * 2.0;
	f32 zw = q->zi * q->wr * 2.0;
	
	qmtx._00 = 1.0 - y2 - z2;
	qmtx._01 = xy + zw;
	qmtx._02 = zx - yw;
	qmtx._03 = 0.0;
	qmtx._10 = xy - zw;
	qmtx._11 = 1.0 - z2 - x2;
	qmtx._12 = yz + xw;
	qmtx._13 = 0.0;
	qmtx._20 = zx + yw;
	qmtx._21 = yz - xw;
	qmtx._22 = 1.0 - x2 - y2;
	qmtx._23 = 0.0;
	qmtx._30 = 0.0;
	qmtx._31 = 0.0;
	qmtx._32 = 0.0;
	qmtx._33 = 1.0;

	GFL_QUAT_SetQMtx44_to_MtxFx44( &qmtx, mtx );
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンをマトリックスに設定
 *
 * @pwram	qmtx			マトリックス格納先
 * @pwram	quaternion		クォータニオン
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_SetQMtx44( GFL_QUATERNION_MTX44* qmtx, const GFL_QUATERNION* q )
{
	f32 x2 = q->xi * q->xi * 2.0;
	f32 y2 = q->yi * q->yi * 2.0;
	f32 z2 = q->zi * q->zi * 2.0;
	f32 xy = q->xi * q->yi * 2.0;
	f32 yz = q->yi * q->zi * 2.0;
	f32 zx = q->zi * q->xi * 2.0;
	f32 xw = q->xi * q->wr * 2.0;
	f32 yw = q->yi * q->wr * 2.0;
	f32 zw = q->zi * q->wr * 2.0;
	
	qmtx->_00 = 1.0 - y2 - z2;
	qmtx->_01 = xy + zw;
	qmtx->_02 = zx - yw;
	qmtx->_03 = 0.0;
	qmtx->_10 = xy - zw;
	qmtx->_11 = 1.0 - z2 - x2;
	qmtx->_12 = yz + xw;
	qmtx->_13 = 0.0;
	qmtx->_20 = zx + yw;
	qmtx->_21 = yz - xw;
	qmtx->_22 = 1.0 - x2 - y2;
	qmtx->_23 = 0.0;
	qmtx->_30 = 0.0;
	qmtx->_31 = 0.0;
	qmtx->_32 = 0.0;
	qmtx->_33 = 1.0;
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンで作成したマトリックスをfx32型の4x4マトリックスに変換
 *
 * @pwram	qmtx	クォータニオンマトリックス
 * @pwram	mtx		格納先
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_SetQMtx44_to_MtxFx44( const GFL_QUATERNION_MTX44* qmtx, MtxFx44* mtx )
{
	mtx->_00 = FX32_CONST( qmtx->_00 );
	mtx->_01 = FX32_CONST( qmtx->_01 );
	mtx->_02 = FX32_CONST( qmtx->_02 );
	mtx->_03 = FX32_CONST( qmtx->_03 );
	mtx->_10 = FX32_CONST( qmtx->_10 );
	mtx->_11 = FX32_CONST( qmtx->_11 );
	mtx->_12 = FX32_CONST( qmtx->_12 );
	mtx->_13 = FX32_CONST( qmtx->_13 );
	mtx->_20 = FX32_CONST( qmtx->_20 );
	mtx->_21 = FX32_CONST( qmtx->_21 );
	mtx->_22 = FX32_CONST( qmtx->_22 );
	mtx->_23 = FX32_CONST( qmtx->_23 );
	mtx->_30 = FX32_CONST( qmtx->_30 );
	mtx->_31 = FX32_CONST( qmtx->_31 );
	mtx->_32 = FX32_CONST( qmtx->_32 );
	mtx->_33 = FX32_CONST( qmtx->_33 );
}


//--------------------------------------------------------------
/**
 * @brief	クォータニオンで作成したマトリックスをfx32型の4x3マトリックスに変換
 *
 * @pwram	qmtx	
 * @pwram	mtx	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void GFL_QUAT_SetQMtx44_to_MtxFx43( const GFL_QUATERNION_MTX44* qmtx, MtxFx43* mtx )
{
	MtxFx44 mtx44;
	
	GFL_QUAT_SetQMtx44_to_MtxFx44( qmtx, &mtx44 );
	
	MTX_Copy44To43( &mtx44, mtx );
}


//--------------------------------------------------------------
/**
 * @brief	回転量を取得 ラジアン
 *
 * @pwram	q	
 *
 * @retval	u16	
 *
 */
//--------------------------------------------------------------
u16 GFL_QUAT_GetRotation( const GFL_QUATERNION* q )
{
	u16 rad;
	fx32 x = FX32_CONST( q->wr );
	
	rad = FX_AcosIdx( x ) * 2;
	
	return rad;
}

//--------------------------------------------------------------
//	static関数群
//--------------------------------------------------------------
static	void	MakeRotationalQuaternion(GFL_QUATERNION *q,GFL_QUATERNION_MTX44 m)
{
	f32 s, trace = m._00 + m._11 + m._22 + 1.0f;
    f32 max = m._11 > m._22 ? m._11 : m._22;

	q->wr=q->xi=q->yi=q->zi=0.0f;

	if (trace >= 1.0f) {
		s = 0.5f / FX_FX32_TO_F32(FX_Sqrt(FX_F32_TO_FX32(trace)));
		q->wr = 0.25f / s;
		q->xi = (m._12 - m._21) * s;
		q->yi = (m._20 - m._02) * s;
		q->zi = (m._01 - m._10) * s;
	}
	else {
		if (max < m._00) {
			s = FX_FX32_TO_F32(FX_Sqrt(FX_F32_TO_FX32(m._00 - (m._11 + m._22) + 1.0f)));
			q->xi = s * 0.5f;
			s = 0.5f / s;
			q->yi = (m._01 + m._10) * s;
			q->zi = (m._20 + m._02) * s;
			q->wr = (m._12 - m._21) * s;
		}
		else if (max == m._11) {
			s = FX_FX32_TO_F32(FX_Sqrt(FX_F32_TO_FX32(m._11 - (m._22 + m._00) + 1.0f)));
			q->yi = s * 0.5f;
			s = 0.5f / s;
			q->xi = (m._01 + m._10) * s;
			q->zi = (m._12 + m._21) * s;
			q->wr = (m._20 - m._02) * s;
		}
		else {
			s = FX_FX32_TO_F32(FX_Sqrt(FX_F32_TO_FX32(m._22 - (m._00 + m._11) + 1.0f)));
			q->zi = s * 0.5f;
			s = 0.5f / s;
			q->xi = (m._20 + m._02) * s;
			q->yi = (m._12 + m._21) * s;
			q->wr = (m._01 - m._10) * s;
		}
	}
}
#endif
