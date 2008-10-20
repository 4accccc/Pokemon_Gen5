//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
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




