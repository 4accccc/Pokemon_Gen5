/*=============================================================================
/*!

	@file	npCamera.h

	@brief	カメラ定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: $
	$Date: $

*//*=========================================================================*/
#ifndef	NPCAMERA_H
#define	NPCAMERA_H

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

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	ビューマトリクスを生成する

	@param	dst		出力先のマトリクス
	@param	eye		カメラ位置
	@param	center	注視点
	@param	up		上方ベクトル

*/
NP_API npFMATRIX* NP_APIENTRY npLookAtFMATRIX( 
	npFMATRIX*	dst,
	npFVECTOR*	eye,
	npFVECTOR*	center,
	npFVECTOR*	up
);

/*!

	正射影による平行な視体積の行列
	
	@param	dst		出力先のマトリクス
	@param	left	左位置
	@param	right	右位置
	@param	bottom	下位置
	@param	top		上位置
	@param	znear	Z near
	@param	zfar	Z far

*/
NP_API npFMATRIX* NP_APIENTRY npOrthoFMATRIX( 
	npFMATRIX*	dst,
	npFLOAT		left,
	npFLOAT		right,
	npFLOAT		bottom,
	npFLOAT		top,
	npFLOAT		znear,
	npFLOAT		zfar
);

/*!

	透視法による錐台の行列を作成する

	@param	dst		出力先のマトリクス
	@param	left	左位置
	@param	right	右位置
	@param	bottom	下位置
	@param	top		上位置
	@param	znear	Z near
	@param	zfar	Z far

*/
NP_API npFMATRIX* NP_APIENTRY npFrustumFMATRIX( 
	npFMATRIX*	dst,
	npFLOAT		left,
	npFLOAT		right,
	npFLOAT		bottom,
	npFLOAT		top,
	npFLOAT		znear,
	npFLOAT		zfar
);

/*!

	パースペクティブマトリクスを生成する

	@param	dst		出力先のマトリクス
	@param	angle	Field of View ( Radiun )
	@param	aspect	アスペクト比
	@param	znear	Z Near
	@param	zfar	Z Far

*/
NP_API npFMATRIX* NP_APIENTRY npPerspectiveFMATRIX( 
	npFMATRIX*	dst,
	npFLOAT		fovy,
	npFLOAT		aspect,
	npFLOAT		znear,
	npFLOAT		zfar
);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPCAMERA_H */
