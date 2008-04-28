//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
//ベクトルのスカラー乗算（a * b）
void	inline VEC_Mult( const fx32 a, const VecFx32* b, VecFx32* dst ) 
{
	dst->x = FX_Mul( a, b->x );
	dst->y = FX_Mul( a, b->y );
	dst->z = FX_Mul( a, b->z );
}

//ベクトルのスカラー除算（b / a）
void	inline VEC_Divt( const fx32 a, const VecFx32* b, VecFx32* dst ) 
{
	GF_ASSERT( a );

	dst->x = FX_Div( b->x, a );
	dst->y = FX_Div( b->y, a );
	dst->z = FX_Div( b->z, a );
}

//ベクトルの投影演算（aをbへ投影）
void	inline VEC_Proj( const VecFx32* a, const VecFx32* b, VecFx32* dst ) 
{
	// Ｂは正規化済みであること
	// 　※ＡをＢに投影するのは、Ｂは正規化済みであることを前提で (Ａ・Ｂ)Ｂ
	// 　　正確には(Ａ・Ｂ)Ｂ／(Ｂの長さ)
	fx32 scalar = VEC_DotProduct( a, b );
	VEC_Mult( scalar, b, dst );
}

//ベクトルの反射演算（aをb軸に対して反射）
void	inline VEC_Refrect( const VecFx32* a, const VecFx32* b, VecFx32* dst ) 
{
	// Ｂは正規化済みであること
	VecFx32 vecP;

	VEC_Proj( a, b, &vecP ); 
	VEC_Set( &vecP, -vecP.x, -vecP.y, -vecP.z );	//負方向に変換
	VEC_MultAdd( 2 * FX32_ONE, &vecP, a, dst );
}

//------------------------------------------------------------------
/**
 * @brief		マップ上でのベクトル移動（重力あり）
 */
//------------------------------------------------------------------
typedef struct _CALC_PH_MV CALC_PH_MV; 
typedef void	(GFL_PH_GROUND_VECN_FUNC)( const VecFx32* pos, VecFx32* vecN );
typedef void	(GFL_PH_GROUND_HEIGHT_FUNC)( const VecFx32* pos, fx32* height );

#define PHMV_FULL_ABSORB	(0xffffffff)
typedef struct {
	fx32	gravityMove;		//重力:移動用(調整可能にするため空中とは分離)
	fx32	gravityFall;		//重力:空中用
	u16		planeMarginTheta;	//平面とみなす傾き
	fx32	absorbVal;			//反発力吸収値(PHMV_FULL_ABSORB	= 反発なし)
	u16		enableMoveTheta;	//移動可能な角度

	GFL_PH_GROUND_VECN_FUNC*	getGroundVecN_func;		//地形の法線ベクトル取得関数
	GFL_PH_GROUND_HEIGHT_FUNC*	getGroundHeight_func;	//地形の高さ取得関数

}PHMV_SETUP;

extern CALC_PH_MV*	CreateCalcPhisicsMoving( HEAPID heapID, PHMV_SETUP* setup );
extern void			DeleteCalcPhisicsMoving( CALC_PH_MV* calcPHMV );
extern void			SetAbsorbPHMV( CALC_PH_MV* calcPHMV, fx32 absorbVal );
extern void			SetMoveVecPHMV( CALC_PH_MV* calcPHMV, VecFx32* dirXZ );
extern void			GetMoveDirPHMV( CALC_PH_MV* calcPHMV, VecFx32* vecDir );
extern void			GetMoveVecPHMV( CALC_PH_MV* calcPHMV, VecFx32* vecMove );
extern fx32			GetMoveSpeedPHMV( CALC_PH_MV* calcPHMV );
extern void			SetMoveSpeedPHMV( CALC_PH_MV* calcPHMV, fx32 speed );
extern void			AddMoveSpeedPHMV( CALC_PH_MV* calcPHMV, fx32 speed );
extern BOOL			CheckOnFloorPHMV( CALC_PH_MV* calcPHMV, VecFx32* pos );
extern BOOL			CheckGroundGravityPHMV( CALC_PH_MV* calcPHMV, VecFx32* pos );
extern void			StartMovePHMV( CALC_PH_MV* calcPHMV, VecFx32* dirXZ, fx32 speed, u16 theta );
extern void			ResetMovePHMV( CALC_PH_MV* calcPHMV );
extern BOOL			CalcMovePHMV( CALC_PH_MV* calcPHMV, VecFx32* posNow );




