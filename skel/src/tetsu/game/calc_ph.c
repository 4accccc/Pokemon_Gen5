//============================================================================================
/**
 * @file	calc_ph.c
 * @brief	物理演算系関数
 */
//============================================================================================
#include "gflib.h"

#include "calc_ph.h"

//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
//------------------------------------------------------------------
/**
 *
 * @brief		マップ上でのベクトル移動（重力あり）
 *
 */
//------------------------------------------------------------------
struct _CALC_PH_MV {
	GFL_PH_GROUND_VECN_FUNC*	getGroundVecN_func;		//地形の法線ベクトル取得関数
	GFL_PH_GROUND_HEIGHT_FUNC*	getGroundHeight_func;	//地形の高さ取得関数

	fx32	gravityMove;
	fx32	gravityFall;
	fx32	planeMargin;
	fx32	absorbVal;
	u16		enableMoveTheta;

	VecFx32	vecDir;
	fx32	speed;
	u16		theta;

	VecFx32	vecSpeed;
	VecFx32	vecMove;
};

static void	inline VEC_Mult( const fx32 a, const VecFx32* b, VecFx32* dst ) 
{
	dst->x = FX_Mul( a, b->x );
	dst->y = FX_Mul( a, b->y );
	dst->z = FX_Mul( a, b->z );
}

static void	inline VEC_Divt( const fx32 a, const VecFx32* b, VecFx32* dst ) 
{
	GF_ASSERT( a );

	dst->x = FX_Div( b->x, a );
	dst->y = FX_Div( b->y, a );
	dst->z = FX_Div( b->z, a );
}

static void	inline VEC_Proj( const VecFx32* a, const VecFx32* b, VecFx32* dst ) 
{
	// Ｂは正規化済みであること
	// 　※ＡをＢに投影するのは、Ｂは正規化済みであることを前提で (Ａ・Ｂ)Ｂ
	// 　　正確には(Ａ・Ｂ)Ｂ／(Ｂの長さ)
	fx32 scalar = VEC_DotProduct( a, b );
	VEC_Mult( scalar, b, dst );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL	CheckOnGround( CALC_PH_MV* calcPHMV, VecFx32* pos, fx32* y )
{
	fx32 groundHeight;

	//GetGroundPlaneHeight( pos, &groundHeight );
	calcPHMV->getGroundHeight_func( pos, &groundHeight );
	
	if( pos->y <= groundHeight + (0.001f * FX32_ONE) ){	//0.1fは誤差許容幅
		if( y != NULL ){
			*y = groundHeight;
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

static void	GetGroundGravity( CALC_PH_MV* calcPHMV,
		const VecFx32* pos, VecFx32* vecG, const fx32 gravity )
{
	VecFx32 vecN, vecH, vecV;
	VecFx32 vecGravity = { 0, -FX32_ONE, 0 };

	//平面の法線ベクトルにより地面に垂直で斜面に並行なベクトルを算出
	//GetGroundPlaneVecN( pos, &vecN );				//平面の法線を取得
	calcPHMV->getGroundVecN_func( pos, &vecN );		//平面の法線を取得
	VEC_CrossProduct( &vecN, &vecGravity, &vecH );	//平面上の水平ベクトル算出
	VEC_CrossProduct( &vecN, &vecH, &vecV );		//平面上の斜面ベクトル算出
	if( vecV.y > 0 ){
		//正方向（天向き）へのベクトルが出てしまった場合負方向（地向き）へ修正
		VEC_Set( &vecV, -vecV.x, -vecV.y, -vecV.z );
	}
	VEC_Normalize( &vecV, &vecV );

	//重力ベクトルを投影し、かかるべき重力ベクトルを算出
	vecGravity.y = -gravity;
	VEC_Proj( &vecGravity, &vecV, vecG ); 
}

static void	GetGroundVec( CALC_PH_MV* calcPHMV,
		const VecFx32* pos, const VecFx32* vecDir, VecFx32* vecGround )
{
	VecFx32 vecN, vecH, vecV;

	//平面の法線ベクトルにより移動ベクトルに垂直で斜面に並行なベクトルを算出
	//GetGroundPlaneVecN( pos, &vecN );				//平面の法線を取得
	calcPHMV->getGroundVecN_func( pos, &vecN );		//平面の法線を取得
	VEC_CrossProduct( &vecN, vecDir, &vecH );		//平面上の水平ベクトル算出
	VEC_CrossProduct( &vecN, &vecH, &vecV );		//平面上の斜面ベクトル算出

	if( VEC_DotProduct( &vecV, vecDir ) < 0 ){
		//逆方向へのベクトルが出てしまった場合修正
		VEC_Set( vecGround, -vecV.x, -vecV.y, -vecV.z );
	} else {
		VEC_Set( vecGround, vecV.x, vecV.y, vecV.z );
	}
	VEC_Normalize( vecGround, vecGround );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
CALC_PH_MV*	CreateCalcPhisicsMoving( HEAPID heapID, PHMV_SETUP* setup )
{
	CALC_PH_MV*	calcPHMV = GFL_HEAP_AllocClearMemory( heapID, sizeof(CALC_PH_MV) );

	GF_ASSERT( setup->getGroundVecN_func );
	GF_ASSERT( setup->getGroundHeight_func );

	calcPHMV->getGroundVecN_func = setup->getGroundVecN_func;
	calcPHMV->getGroundHeight_func = setup->getGroundHeight_func;

	calcPHMV->gravityMove		= setup->gravityMove;
	calcPHMV->gravityFall		= setup->gravityFall;
	calcPHMV->planeMargin		= -FX_SinIdx( setup->planeMarginTheta );
	calcPHMV->absorbVal			= setup->absorbVal;
	calcPHMV->enableMoveTheta	= setup->enableMoveTheta;
	
	ResetMovePHMV( calcPHMV );
	return calcPHMV;
}

void DeleteCalcPhisicsMoving( CALC_PH_MV* calcPHMV )
{
	GFL_HEAP_FreeMemory( calcPHMV );
}

//------------------------------------------------------------------
void SetAbsorbPHMV( CALC_PH_MV* calcPHMV, fx32 absorbVal )
{
	calcPHMV->absorbVal = absorbVal;
}

//------------------------------------------------------------------
void SetMoveVecPHMV( CALC_PH_MV* calcPHMV, VecFx32* dirXZ )
{
	VecFx32 vecDir, vecMove;

	vecDir.x = FX_Mul( dirXZ->x, FX_CosIdx( calcPHMV->theta ) );
	vecDir.y = FX_SinIdx( calcPHMV->theta );
	vecDir.z = FX_Mul( dirXZ->z, FX_CosIdx( calcPHMV->theta ) );
	VEC_Normalize( &vecDir, &vecDir );
	VEC_Mult( calcPHMV->speed, &vecDir, &vecDir );

	VEC_Add( &vecDir, &calcPHMV->vecSpeed, &vecDir );
	VEC_Normalize( &vecDir, &vecDir );

	VEC_Mult( VEC_Mag( &calcPHMV->vecSpeed ), &vecDir, &calcPHMV->vecSpeed );
}

//------------------------------------------------------------------
void GetMoveDirPHMV( CALC_PH_MV* calcPHMV, VecFx32* vecDir )
{
	VEC_Normalize( &calcPHMV->vecSpeed, vecDir );
}

//------------------------------------------------------------------
void GetMoveVecPHMV( CALC_PH_MV* calcPHMV, VecFx32* vecMove )
{
	*vecMove = calcPHMV->vecMove;
}

//------------------------------------------------------------------
fx32 GetMoveSpeedPHMV( CALC_PH_MV* calcPHMV )
{
	return VEC_Mag( &calcPHMV->vecSpeed );
}

//------------------------------------------------------------------
void SetMoveSpeedPHMV( CALC_PH_MV* calcPHMV, fx32 speed )
{
	VecFx32 vecSpeed;

	VEC_Normalize( &calcPHMV->vecSpeed, &vecSpeed );
	VEC_Mult( speed, &vecSpeed, &calcPHMV->vecSpeed );
}
//------------------------------------------------------------------
void AddMoveSpeedPHMV( CALC_PH_MV* calcPHMV, fx32 speed )
{
	VecFx32 vecSpeed;
	fx32	valSpeed = VEC_Mag( &calcPHMV->vecSpeed ) + speed;

	VEC_Normalize( &calcPHMV->vecSpeed, &vecSpeed );
	VEC_Mult( valSpeed, &vecSpeed, &calcPHMV->vecSpeed );
}

//------------------------------------------------------------------
BOOL CheckOnFloorPHMV( CALC_PH_MV* calcPHMV, VecFx32* pos )
{
	return CheckOnGround( calcPHMV, pos, NULL );	//TRUE = 地上, FALSE = 空中
}

//------------------------------------------------------------------
BOOL CheckGroundGravityPHMV( CALC_PH_MV* calcPHMV, VecFx32* pos )
{
	VecFx32 vecG;

	GetGroundGravity( calcPHMV, pos, &vecG, FX32_ONE );
	if( vecG.y < 0 ){
		return TRUE;	//空中or地上斜面(重力による仕事が発生している状態)
	} else {
		return FALSE;	//地上平面
	}
}

//------------------------------------------------------------------
void StartMovePHMV( CALC_PH_MV* calcPHMV, VecFx32* dirXZ, fx32 speed, u16 theta )
{
	calcPHMV->speed = speed;
	calcPHMV->theta = theta;

	calcPHMV->vecDir.x = FX_Mul( dirXZ->x, FX_CosIdx( calcPHMV->theta ) );
	calcPHMV->vecDir.y = FX_SinIdx( calcPHMV->theta );
	calcPHMV->vecDir.z = FX_Mul( dirXZ->z, FX_CosIdx( calcPHMV->theta ) );
	VEC_Normalize( &calcPHMV->vecDir, &calcPHMV->vecDir );

	VEC_Mult( calcPHMV->speed, &calcPHMV->vecDir, &calcPHMV->vecSpeed ); 
}

//------------------------------------------------------------------
void ResetMovePHMV( CALC_PH_MV* calcPHMV )
{
	calcPHMV->speed = 0;
	calcPHMV->theta = 0;

	calcPHMV->vecDir.x = 0;
	calcPHMV->vecDir.y = 0;
	calcPHMV->vecDir.z = 0;

	calcPHMV->vecSpeed.x = 0;
	calcPHMV->vecSpeed.y = 0;
	calcPHMV->vecSpeed.z = 0;
}

//------------------------------------------------------------------
BOOL	CalcMovePHMV( CALC_PH_MV* calcPHMV, VecFx32* posNow )
{
	VecFx32 vecMove, vecG, vecSpeed, posNext;
	VecFx32	vecSpeed_next;
	u16		thetaGround;

	if( !calcPHMV->vecDir.y ){
		//水平移動

		//移動方向の地形に沿った単位ベクトル取得
		GetGroundVec( calcPHMV, posNow, &calcPHMV->vecSpeed, &vecMove );
		//移動ベクトル取得
		VEC_Mult( VEC_Mag( &calcPHMV->vecSpeed ), &vecMove, &calcPHMV->vecSpeed );
		//予想移動位置の計算
		VEC_Add( posNow, &calcPHMV->vecSpeed, &posNext );
		GetGroundGravity( calcPHMV, &posNext, &vecG, calcPHMV->gravityMove );

		//速度の平均を算出( V0+V1 / 2 )
		VEC_Add( &calcPHMV->vecSpeed, &vecG, &vecSpeed_next );
		VEC_Add( &calcPHMV->vecSpeed, &vecSpeed_next, &vecSpeed );
		VEC_Divt( 2 * FX32_ONE, &vecSpeed, &vecSpeed );

		calcPHMV->vecSpeed = vecSpeed_next;
		//OS_Printf("speed{ %d }\n ", VEC_Mag( &calcPHMV->vecSpeed ));

		//移動位置の計算
		VEC_Add( posNow, &vecSpeed, &posNext );
		//GetGroundPlaneHeight( &posNext, &posNext.y );
		calcPHMV->getGroundHeight_func( &posNext, &posNext.y );
	} else {
		//空中移動
		vecG.x = 0;
		vecG.y = -calcPHMV->gravityFall;
		vecG.z = 0;
		
		//速度の平均を算出( V0+V1 / 2 )
		VEC_Add( &calcPHMV->vecSpeed, &vecG, &vecSpeed_next );
		VEC_Add( &calcPHMV->vecSpeed, &vecSpeed_next, &vecSpeed );
		VEC_Divt( 2 * FX32_ONE, &vecSpeed, &vecSpeed );

		calcPHMV->vecSpeed = vecSpeed_next;

		//移動位置の計算
		VEC_Add( posNow, &vecSpeed, &posNext );
		CheckOnGround( calcPHMV, &posNext, &posNext.y );
	}
	VEC_Subtract( &posNext, posNow, &calcPHMV->vecMove );
	*posNow = posNext;

	return TRUE;
}




