/*=============================================================================
/*!

	@file	npParticleFrameEmit.h

	@brief	放出待機オブジェクト定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.8 $
	$Date: 2006/03/13 06:13:23 $

*//*=========================================================================*/
#ifndef	NPPARTICLEFRAMEEMIT_H
#define	NPPARTICLEFRAMEEMIT_H

/*! @name	C リンケージ外対応 */
//!@{

#include "npParticleAttribute.h"

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

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
/*!

	@struct	コンスタント発生待機オブジェクト

*/
typedef NP_HSTRUCT __npPARTICLECNSTEMIT
{
	npOBJECT	Object;

	npS32		m_nCycle;		/* 残りループ回数	*/
	npS32		m_nRate;		/* 現在までの生成数 */
	npS32		m_nInterval;	/* 遅延中の残り時間	*/

	npU32		m_nReserved[ 1 ];
}
NP_FSTRUCT npPARTICLECNSTEMIT;

/*!

	@struct	シーケンス発生待機オブジェクト

*/
typedef NP_HSTRUCT __npPARTICLESEQEMIT
{
	npOBJECT	Object;

	npS32		m_nCycle;		/* 残りのループ回数	*/
	npS32		m_nStep;

	npU32		m_pReserved[ 2 ];
}
NP_FSTRUCT npPARTICLESEQEMIT;

/*!

	@struct	発生待機オブジェクトスタイル

*/
typedef union __npPARTICLEFRAMEEMITSTYLE
{
	npPARTICLECNSTEMIT	Constant;
	npPARTICLESEQEMIT	Sequence;
}
npPARTICLEFRAMEEMITSTYLE;

/*!

	@struct	発生待機オブジェクト

*/
typedef NP_HSTRUCT __npPARTICLEFRAMEEMIT
{
	npPARTICLEFRAMEEMITSTYLE		Style;

	NP_CONST npPARTICLEATTRIBUTE*	m_pAttr;
	npS32							m_nFrameLength;	/* 総フレーム数		*/
	npS32							m_nLocalFrame;	/* 現在のフレーム数 */
	struct __npPARTICLEFRAMEEMIT*	m_pNext;
}
NP_FSTRUCT npPARTICLEFRAMEEMIT;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	放出待機オブジェクトで必要なメモリサイズを取得する

	@param	nNum	使用する放出待機オブジェクトの数

	@return	必要なメモリサイズ

*/
#define	npParticleCheckFrameEmitHeapSize( nNum )	(	\
	( npSIZE )( sizeof( npPARTICLEFRAMEEMIT ) * ( nNum ) ) )

/*!

	放出待機オブジェクトのアリーナを初期化する

	@param	pBuf	放出待機オブジェクトのアリーナ領域
	@param	nSize	上記のメモリサイズ

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleInitFrameEmitList( 
	npVOID*	pBuf,
	npSIZE 	nSize
);

/*!

	放出待機オブジェクトのアリーナ領域の終了処理

*/
NP_API npVOID NP_APIENTRY npParticleExitFrameEmitList( npVOID );

/*!

	放出待機オブジェクトを生成する

	@param	pCycle		放出サイクル
	@param	ppFrame		生成した放出待機オブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleCreateFrameEmit(
	NP_CONST	npPARTICLEATTRIBUTE*	pAttribute,
				npPARTICLEFRAMEEMIT**	ppFrame
);

/*!

	放出待機オブジェクトを解放する

	@param	pFrame	対象のオブジェクト

*/
NP_API npVOID NP_APIENTRY npParticleReleaseFrameEmit(
	npPARTICLEFRAMEEMIT*	pFrame
);

/*!

	放出するオブジェクトの数を取得する

	@param	pFrame	対象のオブジェクト
	@param	nStep	ステップ数

	@return	放出するオブジェクトの数

*/
NP_API npU32 NP_APIENTRY npParticleCountFrameEmitObject(
	npPARTICLEFRAMEEMIT*	pFrame,
	npU32					nStep
);

/*!

	対象のオブジェクトをリストに追加する

	@param	pFirst	リストの先頭オブジェクト
	@param	pFrame	対象のオブジェクト

	@return	リストの先頭オブジェクト

	@note	追加される場所はシステムに依存

*/
#define	npParticleInsertFrameEmit( pFirst, pFrame )	(	\
	( pFirst ) == NP_NULL ? ( pFrame ) : ( ( pFrame )->m_pNext = ( pFirst ), ( pFrame ) ) )

/*!

	リストから対象のオブジェクトを外す

	@param	pFirst	リストの先頭オブジェクト
	@param	pFrame	対象のオブジェクト

	@return	リストの先頭オブジェクト

*/
NP_API npPARTICLEFRAMEEMIT*	npParticleRemoveFrameEmit(
	npPARTICLEFRAMEEMIT*	pFirst,
	npPARTICLEFRAMEEMIT*	pFrame
);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLEFRAMEEMIT_H */
