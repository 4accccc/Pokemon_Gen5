/*=============================================================================
/*!

	@file	npParticleAttribute.h

	@brief	パーティクルの属性定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.31 $
	$Date: 2006/04/06 11:22:34 $

*//*=========================================================================*/
#ifndef	NPPARTICLEATTRIBUTE_H
#define	NPPARTICLEATTRIBUTE_H

/*! @name	C リンケージ外対応 */
//!@{

#include "core/npTexture.h"
#include "npParticleNode.h"
#include "npParticleFrameCycle.h"
#include "npParticleKinematics.h"

//!@}

#ifdef __cplusplus
extern "C" {
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	include
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	define
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	enum
//-----------------------------------------------------------------------------
/*!

	@enum	パーティクルの種類

*/
enum npPARTICLETYPE
{
	NP_PARTICLE_EMITTER = 0,
	NP_PARTICLE_QUAD,
	//NP_PARTICLE_SPRITE,
	//NP_PARTICLE_PATH,
	//NP_PARTICLE_MODEL,
	//NP_PARTICLE_ACTOR,
	
	NP_PARTICLETYPE_MAX
};

/*!

	@enum	パーティクルの性質

*/
enum npPARTICLESTATE
{
	NP_PARTICLE_BILLBOARD		= 1,									//!< ビルボード
	NP_PARTICLE_AXISBILLBOARD	= NP_PARTICLE_BILLBOARD			<< 1,	//!< 軸固定ビルボード
	NP_PARTICLE_SUBDIVISION		= NP_PARTICLE_AXISBILLBOARD		<< 1,	//!< 分割ポリゴン
	NP_PARTICLE_ASPECT_LOCK		= NP_PARTICLE_SUBDIVISION		<< 1,	//!< アスペクトロック
	NP_PARTICLE_ACCELERATION	= NP_PARTICLE_ASPECT_LOCK		<< 1,	//!< 外部加速度

	NP_PARTICLE_TEXTURE			= NP_PARTICLE_ACCELERATION 		<< 1,	//!< テクスチャ
	NP_PARTICLE_TRANSPARENCY	= NP_PARTICLE_TEXTURE			<< 1,	//!< アルファ
	NP_PARTICLE_FILL_SMOOTH		= NP_PARTICLE_TRANSPARENCY		<< 1,	//!< グーロシェーディング
	NP_PARTICLE_LIFESPAN		= NP_PARTICLE_FILL_SMOOTH		<< 1,	//!< ライフで終了

	NP_PARTICLE_PARENT_TRANS	= NP_PARTICLE_LIFESPAN			<< 1,	//!< 親の Translation
	NP_PARTICLE_PARENT_ROT		= NP_PARTICLE_PARENT_TRANS		<< 1,	//!< 親の Rotation
	NP_PARTICLE_PARENT_ROT_Z	= NP_PARTICLE_PARENT_ROT		<< 1,	//!< 親の Z Rotation
	NP_PARTICLE_PARENT_ROT_ONCE	= NP_PARTICLE_PARENT_ROT_Z		<< 1,	//!< 生成時のみ親の Rotation 
	
	NP_PARTICLE_PARENT_LIFE		= NP_PARTICLE_PARENT_ROT_ONCE	<< 1,	//!< 親のライフを継承する

	//!<
	//!<
	//!< 

	//!< 
	//!< 
	//!<
	//!<
	NP_PARTICLE_FINISHED		= NP_PARTICLE_PARENT_LIFE		<< 1,	//!< 終了フラグ
	NP_PARTICLE_EMITOUT			= NP_PARTICLE_FINISHED			<< 1,	//!< １サイクル分放出して終了
	NP_PARTICLE_REPEAT			= NP_PARTICLE_EMITOUT			<< 1,	//!< 
	NP_PARTICLE_INV_PRIORITY	= NP_PARTICLE_REPEAT			<< 1,	//!< パーティクルの描画優先度
	NP_PARTICLE_ZSORT			= NP_PARTICLE_INV_PRIORITY		<< 1,	//!< パーティクルのZ値による描画優先度

	NP_PARTICLE_NORMAL			= NP_PARTICLE_ZSORT				<< 1,	//!< Ｚ方向に法線を持つ
	NP_PARTICLE_NORMAL_EX		= NP_PARTICLE_NORMAL			<< 1	//!< 視線方向のＺ方向に法線を持つ

};

/*
@enum
*/
enum npPARTICLEUSERSTATE
{
	NP_PARTICLE_VISIBILITY = 1
};


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
struct __npPARTICLECOMPONENT;

/*!

	@struct	パーティクル属性の抽象構造体

*/
typedef NP_HSTRUCT __npPARTICLEATTRIBUTE
{
	npPARTICLENODE			Node;

	npPARTICLEFRAMECYCLE	Cycle;			/* 放出タイミング */

	npPARTICLEKINEMATICS	Translate;		/* 平行移動情報	*/
	npPARTICLEKINEMATICS	Rotate;			/* 回転情報	*/
	npPARTICLEKINEMATICS	Scale;			/* 拡縮情報	*/

	npFVECTOR				m_vecAccel;		/* 加速度情報 */

	npS32					m_nLifespan;	/* 寿命 */
	npS32					m_nLiferange;	/* 寿命のランダム範囲 */
	
	npU32					m_pReserved[ 2 ];
}
NP_FSTRUCT npPARTICLEATTRIBUTE;

/*!

	@struct	パーティクル放出タイミング

*/
typedef NP_HSTRUCT __npPARTICLEEMITCYCLE
{
	npPARTICLEATTRIBUTE		Attribute;
}
NP_FSTRUCT npPARTICLEEMITCYCLE;

/*!

	@struct	パーティクルアイテム

*/
typedef NP_HSTRUCT __npPARTICLEITEM
{
	npPARTICLEATTRIBUTE		Attribute;

	npPARTICLEKINEMATICS	m_vecAlpha;		/* アルファ			*/
	
	npU32					m_nTexID;		/* テクスチャID		*/	
	npU32					m_nPriority;	/* 描画優先順位		*/
	npSTATE					m_nSrcBlend;	/* ブレンド係数		*/
	npSTATE					m_nDstBlend;	/* ブレンド係数		*/
}
NP_FSTRUCT npPARTICLEITEM;

/*!

	@struct	四角形アイテム

*/
typedef NP_HSTRUCT __npPARTICLEQUAD
{
	npPARTICLEITEM			Item;

	npPARTICLEKINEMATICS	m_vecVertex[ 4 ];
	npPARTICLEKINEMATICS	m_vecColor[ 4 ];
	npPARTICLEKINEMATICS	m_vecUVCoord[ 2 ];
}
NP_FSTRUCT npPARTICLEQUAD;

#if 0 
/*!

	@struct	モデルアイテム

*/
typedef struct __npPARTICLESHAPE
{
	npPARTICLEITEM	Item;
	npU32			m_pReserved[ 4 ];
}
__attribute__( ( aligned( 16 ) ) ) npPARTICLESHAPE;

/*!

*/
typedef struct __npPARTICLEPATH
{
	npPARTICLEITEM	Item;
	npU32			m_pReserved[ 4 ];
}
__attribute__( ( aligned( 16 ) ) ) npPARTICLEPATH;

/*!

	@struct	アクターアイテム

*/
typedef struct __npPARTICLEACTOR
{
	npPARTICLEMODEL	Model;
	npU32			m_pReserved[ 4 ];
}
__attribute__( ( aligned( 16 ) ) ) npPARTICLEACTOR;

#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
#if 0
/*!

	放出タイミングオブジェクトを解放する

	@param	pCycle

*/
#define	npParticleReleaseStructure( pCycle )	\
	npParticleReleaseEmitCycle( ( npPARTICLEEMITCYCLE* )pCycle )
#endif

/*!

	放出タイミングオブジェクトを生成する

	@param	ppCycle		受け取る変数

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleCreateEmitCycle( 
	npPARTICLEEMITCYCLE**	ppCycle
);

/*!

	放出タイミングオブジェクトを開放する

	@param	pCycle	対象のオブジェクト

*/
NP_API npVOID NP_APIENTRY npParticleReleaseEmitCycle( 
	struct	__npPARTICLECOMPONENT* 	pCycle
);

/*!

	四角形オブジェクトを生成する

	@param	ppQuad		受け取る変数

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleCreateQuad( 
	npPARTICLEQUAD**	ppQuad 
);

/*!

	四角形オブジェクトを開放する

	@param	pQuad	対象のオブジェクト

*/
NP_API npVOID NP_APIENTRY npParticleReleaseQuad( 
	struct __npPARTICLECOMPONENT*	pQuad 
);

/*!

	アトリビュートに寿命をセットする

	@param	pAttribute	対象のアトリビュートオブジェクト
	@param	nLength		セットする寿命の長さ

	@return	エラーコード

*/
#define	npParticleSetLifespan( pAttribute, nLength )	(	\
	( pAttribute )->m_nLifespan = ( nLength ), NP_SUCCESS )
//	( pAttribute )->m_nLifespan = ( nLength ) * 200, NP_SUCCESS )

/*!

	アトリビュートの寿命の長さを取得する

	@param	pAttribute	対象のアトリビュートオブジェクト

	@return	寿命の長さ

*/
#define	npParticleGetLifespan( pAttribute )	( ( pAttribute )->m_nLifespan )
//#define	npParticleGetLifespan( pAttribute )	( ( pAttribute )->m_nLifespan / 200 )
	
/*!

	アトリビュートに寿命のランダム幅をセットする

	@param	pAttribute	対象のアトリビュートオブジェクト
	@param	nRange		セットする寿命のランダム幅

	@return	エラーコード

*/
#define	npParticleSetLiferange( pAttribute, nRange )	(	\
	( pAttribute )->m_nLiferange = ( nRange ), NP_SUCCESS )
//	( pAttribute )->m_nLiferange = ( nRange ) * 200, NP_SUCCESS )

/*!

	アトリビュートの寿命のランダム幅を取得する

	@param	pAttribute	対象のアトリビュートオブジェクト

	@return	寿命のランダム幅

*/
#define	npParticleGetLiferange( pAttribute )	( ( pAttribute )->m_nLiferange )
//#define	npParticleGetLiferange( pAttribute )	( ( pAttribute )->m_nLiferange / 200 )


/*!

	アトリビュートの外部加速度をセットする

	@param	pAttribute	対象のアトリビュートオブジェクト
	@param	pVec		セットする npFVEC3 型の値

	@return	エラーコード

*/
#define	npParticelSetAccelFVEC3( pAttribute, pVec )	(	npSetFVEC4(	\
	&( pAttribute )->m_vecAccel, NP_X( pVec ), NP_Y( pVec ), NP_Z( pVec ), FX32_ONE ), NP_SUCCESS )

/*!

	アトリビュートの外部加速度を取得する
	
	@param	pAttribute	対象のアトリビュートオブジェクト
	@param	pVec		取得した値を所持する npFVEC3 型のポインタ

*/
#define	npParticleGetAccelFVEC3( pAttribute, pVec )	(	\
	npCopyFVEC3( ( pVec ), ( npFVEC3* )&( pAttribute )->m_vecAccel ), NP_SUCCESS )

/*!

	アイテムに描画順序の値をセットする

	@param	pItem		対象のアイテム
	@param	nPriority	描画順序

	@return	エラーコード

*/
#define	npParticleSetRenderPrioriy( pItem, nPriority )	(	\
	( pItem )->m_nPriority = ( nPriority ), NP_SUCCESS )

/*!

	アイテムの描画順序の値を取得する

	@param	pItem		対象のアイテム
	
	@return	描画順序

*/
#define	npParticleGetRenderPrioriy( pItem )	( ( pItem )->m_nPriority )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLEATTRIBUTE_H */
