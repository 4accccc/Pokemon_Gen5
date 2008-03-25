//==============================================================================
/**
 * @file	quaternion.h
 * @brief	クォータニオン
 * @author	goto
 * @date	2007.10.09(火)
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================

#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include	<nitro/types.h>
#include	<nitro/fx/fx.h>
#include	<nitro/fx/fx_cp.h>
#include	<nitro/fx/fx_mtx44.h>
#include	<nitro/fx/fx_trig.h>

#if 1
//FX32計算バージョン
//
// -----------------------------------------
//
//	クォータニオン
//
// -----------------------------------------
typedef union {
	
	struct {		
		fx32	wr;		///< 実数
		fx32	xi;		///< 虚数
		fx32	yi;		///< 虚数
		fx32	zi;		///< 虚数
	};
	fx32 q[ 4 ];
	
} GFL_QUATERNION;


// -----------------------------------------
//
//	クォータニオンマトリックス ( 4x4 )
//
// -----------------------------------------
typedef union {
	
	struct {
		fx32 _00, _01, _02, _03;
        fx32 _10, _11, _12, _13;
        fx32 _20, _21, _22, _23;
        fx32 _30, _31, _32, _33;
	};
	fx32 m[ 4 ][ 4 ];
	fx32 a[ 16 ];
	
} GFL_QUATERNION_MTX44;

// クォータニオンの初期化
extern void GFL_QUAT_Identity( GFL_QUATERNION* q );

// クォータニオンのコピー
extern void GFL_QUAT_Copy( GFL_QUATERNION* q1, GFL_QUATERNION* q2 );

// クォータニオンを軸ごとの回転値から生成
extern	void GFL_QUAT_MakeQuaternionXYZ( GFL_QUATERNION* qt, u16 rot_x, u16 rot_y,u16 rot_z );

// クォータニオンの積
extern void GFL_QUAT_Mul( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 );

// クォータニオンの加算
extern void GFL_QUAT_Add( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 );

// クォータニオンの減算
extern void GFL_QUAT_Sub( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 );

// クォータニオンのノルム || a || ^ 2
extern fx32  GFL_QUAT_GetNormSqrt( const GFL_QUATERNION* q );

// クォータニオンのノルム || a ||
extern fx32  GFL_QUAT_GetNorm( const GFL_QUATERNION* q );

// クォータニオンをマトリックスに設定
extern void GFL_QUAT_SetMtx44( MtxFx44* mtx, const GFL_QUATERNION* q );

// クォータニオンをマトリックスに設定
extern void GFL_QUAT_SetQMtx44( GFL_QUATERNION_MTX44* qmtx, const GFL_QUATERNION* q );

// クォータニオンマトリックスをfx32型4x4マトリックスに変換
extern void GFL_QUAT_SetQMtx44_to_MtxFx44( const GFL_QUATERNION_MTX44* qmtx, MtxFx44* mtx );

// クォータニオンマトリックスをfx32型4x3マトリックスに変換
extern void GFL_QUAT_SetQMtx44_to_MtxFx43( const GFL_QUATERNION_MTX44* qmtx, MtxFx43* mtx );

// クォータニオンの保持している回転量
extern u16  GFL_QUAT_GetRotation( const GFL_QUATERNION* q );
#endif

//f32計算バージョン
#if 0
// -----------------------------------------
//
//	クォータニオン
//
// -----------------------------------------
typedef union {
	
	struct {		
		f32	wr;		///< 実数
		f32	xi;		///< 虚数
		f32	yi;		///< 虚数
		f32	zi;		///< 虚数
	};
	f32 q[ 4 ];
	
} GFL_QUATERNION;


// -----------------------------------------
//
//	クォータニオンマトリックス ( 4x4 )
//
// -----------------------------------------
typedef union {
	
	struct {
		f32 _00, _01, _02, _03;
        f32 _10, _11, _12, _13;
        f32 _20, _21, _22, _23;
        f32 _30, _31, _32, _33;
	};
	f32 m[ 4 ][ 4 ];
	f32 a[ 16 ];
	
} GFL_QUATERNION_MTX44;




// クォータニオンの初期化
extern void GFL_QUAT_Identity( GFL_QUATERNION* q );

// クォータニオンのコピー
extern void GFL_QUAT_Copy( GFL_QUATERNION* q1, GFL_QUATERNION* q2 );

// クォータニオンを軸ごとの回転値から生成
extern	void GFL_QUAT_MakeQuaternionXYZ( GFL_QUATERNION* qt, u16 rot_x, u16 rot_y,u16 rot_z );

// クォータニオンの積
extern void GFL_QUAT_Mul( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 );

// クォータニオンの加算
extern void GFL_QUAT_Add( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 );

// クォータニオンの減算
extern void GFL_QUAT_Sub( GFL_QUATERNION* q, const GFL_QUATERNION* q1, const GFL_QUATERNION* q2 );

// クォータニオンのノルム || a || ^ 2
extern f32  GFL_QUAT_GetNormSqrt( const GFL_QUATERNION* q );

// クォータニオンのノルム || a ||
extern f32  GFL_QUAT_GetNorm( const GFL_QUATERNION* q );

// クォータニオンをマトリックスに設定
extern void GFL_QUAT_SetMtx44( MtxFx44* mtx, const GFL_QUATERNION* q );

// クォータニオンをマトリックスに設定
extern void GFL_QUAT_SetQMtx44( GFL_QUATERNION_MTX44* qmtx, const GFL_QUATERNION* q );

// クォータニオンマトリックスをfx32型4x4マトリックスに変換
extern void GFL_QUAT_SetQMtx44_to_MtxFx44( const GFL_QUATERNION_MTX44* qmtx, MtxFx44* mtx );

// クォータニオンマトリックスをfx32型4x3マトリックスに変換
extern void GFL_QUAT_SetQMtx44_to_MtxFx43( const GFL_QUATERNION_MTX44* qmtx, MtxFx43* mtx );

// クォータニオンの保持している回転量
extern u16  GFL_QUAT_GetRotation( const GFL_QUATERNION* q );
#endif

#endif
