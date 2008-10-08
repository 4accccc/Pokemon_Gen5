/*=============================================================================
/*!

	@file	npTexture.h

	@brief	テクスチャ定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.8 $
	$Date: 2006/03/28 03:56:49 $

*//*=========================================================================*/
#ifndef	NPTEXTURE_H
#define	NPTEXTURE_H

/*! @name	C リンケージ外対応 */
//!@{

#include "npObject.h"
#include <nitro/gx/g3imm.h>
#include <nitro/gx/g3b.h>
#include <nitro/gx/g3c.h>

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

	@enum	ピクセルフォーマット

*/
enum npPIXELFORMAT
{
	//NP_PIXEL_INDEX12	= NVD_TEXPIXEL_TF_I4,
	//NP_PIXEL_INDEX16	= NVD_TEXPIXEL_TF_IA4,
	//NP_PIXEL_INDEX24	= NVD_TEXPIXEL_TF_I8,
	//NP_PIXEL_INDEX32	= NVD_TEXPIXEL_TF_IA8,

	//NP_PIXEL_R5G6B5	= NVD_TEXPIXEL_TF_RGB565,
	//NP_PIXEL_R5G5B5A3	= NVD_TEXPIXEL_TF_RGB5A3,
	NP_PIXEL_R8G8B8A8	= 0//GL_RGBA,	//	= D3DFMT_A8R8G8B8,

	//NP_PIXEL_TLUT4	= NVD_TEXPIXEL_TF_C4,
	//NP_PIXEL_TLUT8	= NVD_TEXPIXEL_TF_C8

	// CLUT : Color Look Up Table
};

#if 0
/*!

	@enum	テクスチャステージステート

*/
enum npTEXTURESTATE
{

};

/*!

	@enum	テクスチャフィルタ

*/
enum npTEXTUREFILTER
{

};
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
struct __npCONTEXT;

/*!

	@struct	テクスチャオブジェクト

*/
typedef	struct __npTEXTURE
{
	npOBJECT	Object;					/* 抽象オブジェクト */

	npCHAR		m_szName[ NP_MAXPATH ];	/* テクスチャ名 */
	npU32		m_nWidth;				///< Width
	npU32		m_nHeight;				///< Height
	npU32		m_nSize;				///< Size
	npU32		m_TexAddr;				///< TexAddr
	npU32		m_PltAddr;				///< PltAddr
	GXTexFmt	m_TexFmt;				///< TextureFormat
	GXTexSizeS	m_TexSizeS;				///< GX_TEXSIZE_S***
	GXTexSizeT	m_TexSizeT;				///< GX_TEXSIZE_T***

	struct __npTEXTURE*		m_pNext;
}
npTEXTURE;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	テクスチャオブジェクトを生成する

	@param	pContext	レンダリングコンテキスト
	@param	nWidth		横幅
	@param	nHeight		縦幅
	@param	nLevels		ミップマップレベル
	@param	nUsage		
	@param	nFormat		ピクセルフォーマットの種類
	@param	nPool		
	@param	ppTexture	生成したテクスチャオブジェクトを受け取る変数

	@return	エラーコード

	@note	外部インスタンスが有効なとき他のパラメータは無効

*/
NP_API npERROR NP_APIENTRY npCreateTexture(
	struct __npCONTEXT*	pContext,
	npU32 				nWidth,
	npU32				nHeight,
	npU32				nLevels,
	npSTATE				nUsage,
	npSTATE				nFormat,
	npSTATE				nPool,
	npTEXTURE**			ppTexture
);

/*!

	リソースからテクスチャオブジェクトを生成する

	@param	pContext	レンダリングコンテキスト
	@param	pBuf		GVR ファイルバッファ
	@param	nSize		GVR ファイルサイズ
	@param	ppTexture	生成したテクスチャオブジェクト

	@return	エラーコード

	@todo	内部で 0x20 オフセット移動するのでコピー基の<br>
			バッファにはデータの先頭を渡す

*/
NP_API npERROR NP_APIENTRY npCreateTextureFromBin( 
	struct	__npCONTEXT*	pContext, 
	npVOID*					pBuf,
	npSIZE					nSize, 
	npTEXTURE**				ppTexture 
);

/*!

	ファイルからテクスチャオブジェクトを生成する

	@param	pContex		レンダリングコンテキスト
	@param	szFileName	ファイル名
	@param	ppTexture	生成したテクスチャオブジェクト

	@return	エラーコード

	@note	内部で szFileName で npRegistTextureName() がコールされる

*/
NP_API npERROR NP_APIENTRY npCreateTextureFromFile(
	struct	__npCONTEXT*	pContext,
	npCHAR*					szFileName,
	npTEXTURE**				ppTexture
);

/*!

	システム依存のテクスチャインスタンスからオブジェクトを生成する

	@param	pContext	レンダリングコンテキスト
	@param	pInstance	システム依存のテクスチャインスタンス
	@param	ppTexture	生成したテクスチャオブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npCreateTextureFromRef(
	struct	__npCONTEXT*	pContext,
	npVOID*					pInstance,
	npTEXTURE**				ppTexture
);

/*!

	テクスチャオブジェクトのクローンを生成する

	@param	pTexture	複製するテクスチャオブジェクト

	@return	テクスチャオブジェクト

	@note	リファレンスカウンタを増加します
			VRAM 節約のため同じ形状がある場合は
			こちらの関数で複製を作ってください。

*/
#define	npCloneTexture( pTexture )	(	\
	npIncrementObjectReferences( &( pTexture )->Object ), ( pTexture ) )

/*!

	テクスチャオブジェクトを開放する

	@param	pContext	レンダリングコンテキスト
	@param	pTexture	テクスチャオブジェクト

	@note	リファレンスカウンタが 0 なら VRAM から開放.<br>
			0 になるまで開放されない

*/
NP_API npVOID NP_APIENTRY npReleaseTexture( 
	struct __npCONTEXT*	pContext,
	npTEXTURE*			pTexture
);

/*!

	リファレンスカウンタを無視してテクスチャを開放する

	@param	pContext	レンダリングコンテキスト
	@param	pTexture	テクスチャオブジェクト

*/
NP_API npVOID NP_APIENTRY npDestroyTexture(
	struct __npCONTEXT*	pContext,
	npTEXTURE*			pTexture
);

/*!

	テクスチャ名をセットする

	@param	pTexture	テクスチャオブジェクト
	@param	szName		登録するテクスチャ名

	@return	エラーコード

*/
#define	npSetTextureName( pTexture, szName )	(		\
	( strlen( ( char* )( szName ) ) < NP_MAXPATH ) ? 	\
	( strcpy( ( char* )( ( pTexture )->m_szName ), ( char* )( szName ) ), NP_SUCCESS ) : NP_ERR_OVERFLOW )
 
/*!

	テクスチャ名を取得する

	@param	pTexture	テクスチャオブジェクト
	@param	pName		取得するテクスチャ名用のポインタ

	@return	エラーコード
	
*/
#define	npGetTextureName( pTexture, pName )	(	\
	strcpy( ( pName ), ( pTexture )->m_szName ), NP_SUCCESS )

/*!

	カレントにするテクスチャをレンダリングコンテキストへ登録する

	@param	pContext	レンダリングコンテキスト
	@param	nStage		テクスチャステージ
	@param	pTexture	対象のテクスチャオブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npRegistTexture(
	struct	__npCONTEXT*	pContext,
	npU32					nStage,
	npTEXTURE*				pTexture
);

/*!

	レンダリングコンテキストに登録されているテクスチャを無効にする

	@param	pContext	レンダリングコンテキスト
	@param	nStage		テクスチャステージ

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npUnregistTexture(
	struct	__npCONTEXT*	pContext,
	npU32					nStage
);

/*!

*/
NP_API npERROR NP_APIENTRY npSetTextureParam(
	npTEXTURE	*pTexture,
	npU32		nWidth,
	npU32		nHeight,
	GXTexFmt	fmt,
	npU32		uTexAddr,
	npU32		uPltAddr
);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPTEXTURE_H */
