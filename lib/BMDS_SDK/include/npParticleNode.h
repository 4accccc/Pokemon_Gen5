/*=============================================================================
/*!

	@file	npParticleNode.h

	@brief	パーティクルの階層定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.6 $
	$Date: 2006/03/13 06:13:22 $

*//*=========================================================================*/
#ifndef	NPPARTICLENODE_H
#define	NPPARTICLENODE_H

/*! @name	C リンケージ外対応 */
//!@{

#include "npObject.h"

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

	@struct	パーティクルの階層ノード

*/
typedef NP_HSTRUCT __npPARTICLENODE
{
	npOBJECT	Object;

	struct	__npPARTICLENODE*	m_pParent;
	struct	__npPARTICLENODE*	m_pSibling;
	struct	__npPARTICLENODE*	m_pChild;

	npU32	m_pReserved[ 1 ];
}
NP_FSTRUCT npPARTICLENODE;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	自身の親ノードを取得する

	@param	pNode	対象のノード

	@return	自身の親ノード
	@retval	NP_NULL		自身がルートノード

*/
#define	npParticleParentNode( pNode )	( ( pNode )->m_pParent )

/*!

	自身の兄弟ノードを取得する

	@param	pNode	対象のノード

	@return	自身の兄弟ノード
	@retval	NM_NULL		自身が終端ノード

*/
#define	npParticleSiblingNode( pNode )	( ( pNode )->m_pSibling )

/*!

	自身の子ノードを取得する

	@param	pNode	対象のノード

	@return	自身の子ノード
	@retval	NM_NULL		自身がリーフノード

*/
#define	npParticleChildNode( pNode )	( ( pNode )->m_pChild )

/*!

	自身がルートノードか判定する

	@param	pNode	対象のノード

	@retval	NP_TRUE		ルートノード
	@retval	NP_FALSE	ルートではない

*/
#define	npParticleIsRootNode( pNode )	(	\
	( ( pNode )->m_pParent == NP_NULL ) ? NP_TRUE : NP_FALSE )

/*!

	自身のノードがリーフノードか判定する

	@param	pNode	対象のノード

	@retval	NP_TRUE		リーフノード
	@retval	NP_FALSE	リーフノードではない

*/
#define	npParticleIsLeafNode( pNode )	(	\
	( ( pNode )->m_pChild == NP_NULL ) ? NP_TRUE : NP_FALSE )

/*!

	親ノードに自身のノードを追加する

	@param	pParent	親ノード
	@param	pNode	追加するノード

	@note	pNode はルート( 親がない )である必要がある

*/
NP_API npVOID NP_APIENTRY npParticlePushNode( 
	npPARTICLENODE* pParent, 
	npPARTICLENODE* pNode 
);

/*!

	親ノードから自信のノードを外す

	@param	pNode	自身のノード

*/
NP_API npVOID NP_APIENTRY npParticlePopNode( npPARTICLENODE* pNode );

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLENODE_H */
