/*=============================================================================
/*!

	@file	npContext.h

	@brief	レンダリングコンテキストの定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.9 $
	$Date: 2006/03/28 03:57:03 $

*//*=========================================================================*/
#ifndef	NPCONTEXT_H
#define	NPCONTEXT_H

/*! @name	C リンケージ外対応 */
//!@{

#include "core/npSystem.h"

#include "core/npCamera.h"
#include "core/npLight.h"

#include "npAnimation.h"

#include "core/npRenderState.h"
#include "core/npVertexFormat.h"

#include "core/npMaterial.h"
#include "core/npTexture.h"

#include "core/npTransform.h"
#include "core/npPrimitive.h"

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
/*! 最大レンダリングパス数 */
#define	NP_MAXPASSES	( 8 )

/*! @name	クリアするサーフェイスを示すフラグ */
//!@{

//!@}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	enum
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
/*!

	デバイスデスクリプション

*/
typedef struct __npDEVICEDESC
{
	int a;
}
npDEVICEDESC;

/*!

	@struct	レンダリングコンテキスト

*/
typedef struct __npCONTEXT
{
	npOBJECT		Object;			/* 抽象オブジェクト				*/

	npSTATE			m_hResult;		/* システム依存のエラーコード	*/
	npU32			m_nStencil;		/* ステンシル値		*/
	npSTATE			m_nVertexFormat;

//	npFLOAT			m_fDepth;		/* 深度値			*/
//	npFRGBA			m_pFColor;		/* クリアカラー		*/

	npU32			m_nColor;	///< Clear Color
	npU32			m_nDepth;	///< Clear Depth

	/* トランスフォームマトリクスパレット */
	npFMATRIX		m_matPallet[ NP_INDEXMATRIX_MAX ];

	npTEXTURE*		m_pTextureHeap;	/* テクスチャアリーナ	*/
	npTEXTURE*		m_pTextureList;	/* テクスチャリスト		*/
//	npSHADER*		m_pShaderHeap;	/* シェーダアリーナ		*/
//	npSHADER*		m_pShaderList;	/* シェーダリスト		*/

	npU32			m_nWidth;
	npU32			m_nHeight;
	npU32			m_pReserved1[ 2 ];
	
	npU32			m_nPolyMax;
	npU32			m_nPolyMin;
	npU32			m_nPolygonID;
}
npCONTEXT;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------

/*!

	レンダリングコンテキストを生成する

	@param	pSystem		システムオブジェクト
	@param	pInstance	外部インスタンス
	@param	ppContext	受け取るレンダリングコンテキスト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npCreateContext( 
	npSYSTEM*	pSystem,
	npVOID*		pInstance,
	npCONTEXT** ppContext 
);

/*!

	レンダリングコンテキストを開放する

	@param	pContext	対象のレンダリングコンテキスト

*/
NP_API npVOID NP_APIENTRY npReleaseContext( npCONTEXT* pContext );

/*!

	レンダリングコンテキストの状態をリセットする

	@param	pContext	対象のレンダリングコンテキスト

	@return	エラーコード

	@note	npCreateContext(...) 時は描画前に一度、必ずコールしなければならない

*/
NP_API npERROR NP_APIENTRY npResetEnvironment( npCONTEXT* pContext );

/*!

	レンダリングコンテキストのスペックを確認する

	@param	pContext	レンダリングコンテキスト
	@param	nState		調査する状態

	@return	スペックの状態

*/
NP_API npSTATE NP_APIENTRY npConfirmContext( npCONTEXT* pContext, npSTATE nState );

/*!

	システム依存のエラーコードを取得する

	@param	pContext	レンダリングコンテキスト

	@return	システム依存のエラーコード

*/
#define	npErrorCode( pContext )	( ( pContext )->m_hResult )

/*!

	ウィンドウハンドルをセットする

	@param	pContext	レンダリングコンテキスト
	@param	hWnd		ウィンドウハンドル

	@param	エラーコード

	@note	Windows 環境のみ

*/
#ifdef NP_WGL32
#	define	npSetWindowHandle( pContext, hWnd )	(	\
	( pContext )->m_hWnd = ( hWnd ), NP_SUCCESS )
#else
#	define	npSetWindowHandle( pContext, hWnd )	( NP_SUCCESS )
#endif

/*!

	ウィンドウハンドルを取得する

	@param	pContext	レンダリングコンテキスト

	@return	ウィンドウハンドル

	@note	Windows 環境のみ

*/
#define	npGetWindowHandle( pContext ) ( ( pContext )->m_hWnd )

/*!
	
	クリアカラー値をセットする

	@param	pContext	レンダリングコンテキスト
	@param	nColor		クリアカラー値(16bit値 X1R5G5B5)

	@return	エラーコード

*/
#define npSetClearColor(pContext, nColor)	((pContext)->m_nColor = (nColor), NP_SUCCESS)

/*!

	クリアカラー値を取得する

	@param	pContext	レンダリングコンテキスト

	@return	カラー値

*/
#define npGetClearColor(pContext)	((pContext)->nColor)

/*!

	クリアする深度値をセットする

	@param	pContext	レンダリングコンテキスト
	@param	fDepth		クリア深度値

	@return	エラーコード

*/
#define npSetClearDepth(pContext, nDepth)	((pContext)->m_nDepth = (nDepth), NP_SUCCESS)

/*!

	クリアする深度値を取得する

	@param	pContext	レンダリングコンテキスト

	@return	深度値

*/
#define	npGetClearDepth(pContext)	((pContext)->m_nDepth)

/*!

	ステンシルバッファのエントリに保存する整数値をセットする

	@param	pContext	レンダリングコンテキスト
	@param	nStencil	対象の整数値

	@return	エラーコード

*/
#define	npSetClearStencil( pContext, nStencil )	(	\
	( pContext )->m_nStencil = ( nStencil ), NP_SUCCESS )

/*!
	
	ステンシルバッファのエントリに保存する整数値を取得する

	@param	pContext	レンダリングコンテキスト

	@return	対象の整数値

*/
#define	npGetClearStencil( pContext )	( ( pContext )->m_nStencil )

/*!
	
	ポリゴンIDの幅の最大を取得する

	@param	pContext	レンダリングコンテキスト

	@return	対象の整数値

*/
#define	npGetPolygonIDMax( pContext )	( ( pContext )->m_nPolyMax )

/*!
	
	ポリゴンIDの幅の最大を設定する

	@param	pContext	レンダリングコンテキスト


*/
#define	npSetPolygonIDMax( pContext, polymax )	{( pContext )->m_nPolyMax = polymax;}

/*!
	
	ポリゴンIDの幅の最小を取得する

	@param	pContext	レンダリングコンテキスト

	@return	対象の整数値

*/
#define	npGetPolygonIDMin( pContext )	( ( pContext )->m_nPolyMin )

/*!
	
	ポリゴンIDの幅の最小を設定する

	@param	pContext	レンダリングコンテキスト


*/
#define	npSetPolygonIDMin( pContext, polymin )	{( pContext )->m_nPolyMin = polymin;}

/*!
	
	ポリゴンIDを取得する

	@param	pContext	レンダリングコンテキスト

	@return	対象の整数値

*/
#define	npGetPolygonID( pContext )	( ( pContext )->m_nPolygonID )

/*!
	
	ポリゴンIDを設定する

	@param	pContext	レンダリングコンテキスト


*/
#define	npSetPolygonID( pContext, polyID )	{( pContext )->m_nPolygonID = polyID;}

/*!

	描画用バッファをクリアする

	@param	pContext	レンダリングコンテキスト
	@param	nMask		クリアマスク

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npClear( npCONTEXT* pContext, npSTATE nMask );

/*!

	バッファのスワップを行う

	@param	pContext	レンダリングコンテキスト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npSwapBuffers( npCONTEXT* pContext );

/*!

	描画を開始する

	@param	pContext	レンダリングコンテキスト

	@return	エラーコード

*/
#define	npBeginRender( pContext )	( NP_SUCCESS )

/*!

	描画を終了する

	@param	pContext	レンダリングコンテキスト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npEndRender( npCONTEXT* pContext );

/*!

	デバッグフォントの色をセットする

	@param	pContext	レンダリングコンテキスト
	@param	nColor		フォントカラー

	@return	エラーコード

*/
#define	npDebugSetFontColor( pContext, nColor )

/*!

	デバッグフォントの色を取得する

	@param	pContext	レンダリングコンテキスト

	@return	現在のデバッグフォントカラー

*/
#define	npDebugGetFontColor( pContext )

/*!

	デバッグフォントを出力する

	@param	pContext	レンダリングコンテキスト
	@param	X			出力する文字列の X 座標
	@param	Y			出力する文字列の Y 座標
	@param	szString	出力する文字列
	@param	nLength		出力する文字列の長さ

*/
#ifdef NP_DEBUG
NP_API npVOID NP_APIENTRY npDebugTextOut( 
	npCONTEXT*	pContext,
	npU32		X,
	npU32		Y,
	npCHAR*		szString,
	npSIZE		nLength
);
#else
#	define	npDebugTextOut( pContext, X, Y, szString, nLength )
#endif

/*!

	コンテキストメモリをダンプする

*/
#ifdef NP_DEBUG
NP_API npVOID NP_APIENTRY npDebugDumpContext( npVOID );
#else
#	define npDebugDumpContext( npVOID )
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPCONTEXT_H */
