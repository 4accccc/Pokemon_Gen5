/*=============================================================================
/*!

	@file	npShader.h

	@brief	シェーダ定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: $
	$Date: $

*//*=========================================================================*/
#ifndef	NPSHADER_H
#define	NPSHADER_H

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
#define	NP_HLSL
//#define	NP_CGFX

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	enum
//-----------------------------------------------------------------------------
/*!

	@enum	シェーダのタイプ

*/
enum npSHADERTYPE
{
	NP_SHADER_FX	= 0,
	NP_SHADER_VERTEX,
	NP_SHADER_PIXEL,

	NP_SHADER_MAX
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
struct __npCONTEXT;
struct __npTEXTURE;

/*!

	@struct	シェーダオブジェクト

*/
typedef struct __declspec( align( 16 ) ) __npSHADER
{
	npOBJECT			Object;					/* 抽象オブジェクト */
	npCHAR				m_szName[ NP_MAXPATH ];	/* シェーダ名	*/

	npU32				m_pReserved[ 1 ];
	struct __npSHADER*	m_pNext;
}
npSHADER;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	コンパイル済みのシェーダリソースからシェーダオブジェクトを生成する

	@param	pContext	レンダリングコンテキスト
	@param	pBuf		ファイルバッファ
	@param	nSize		ファイルサイズ
	@param	ppShader	生成したシェーダオブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npCreateShaderFromBin(
	struct	__npCONTEXT*	pContext,
	npVOID*					pBuf,
	npSIZE					nSize,
	npSHADER**				ppShader
);

/*!

	(.fx) ファイルからシェーダオブジェクトを生成する

	@param	pContext	レンダリングコンテキスト
	@param	szFileName	ファイル名
	@param	ppShader	生成したシェーダオブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npCreateShaderFromFile(
	struct __npCONTEXT*		pContext,
	npCHAR*					szFileName,
	npSHADER**				ppShader
);

/*!
	@todo	未実装
			D3DXAssembleShaderFromFile(...)
			IDirect3DDevice9::CreateVertexShader(...)
			SetVertexShaderConstantF(...)
			SetVertexShader(...)
*/
NP_API npERROR NP_APIENTRY npCreateVertexShaderFromFile(
	struct __npCONTEXT*		pContext,
	npCHAR*					szFileName,
	npSHADER**				ppShader
);

/*!
	@todo	未実装
*/
NP_API npERROR NP_APIENTRY npCreateVertexShaderFromBin(
	struct __npCONTEXT*		pContext,
	npVOID*					pBuf,
	npSIZE					nSize,
	npSHADER**				ppShader
);

/*!
	@todo	未実装
*/
NP_API npERROR NP_APIENTRY npCreatePixelShaderFromFile(
	struct __npCONTEXT*		pContext,
	npCHAR*					szFileName,
	npSHADER**				ppShader
);

/*!
	@todo	未実装
*/
NP_API npERROR NP_APIENTRY npCreatePixelShaderFromBin(
	struct __npCONTEXT*		pContext,
	npVOID*					pBuf,
	npSIZE					nSize,
	npSHADER**				ppShader
);

/*!

	システム依存のシェーダインスタンスからオブジェクトを生成する

	@param	pContext	レンダリングコンテキスト
	@param	pInstance	システム依存のシェーダインスタンス
	@param	ppShader	生成したシェーダオブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npCreateShaderFromRef(
	struct __npCONTEXT*		pContext,
	npVOID*					pInstance,
	npSHADER**				ppShader
);

/*!

	シェーダオブジェクトのクローンを生成する

	@param	pShader		複製するシェーダオブジェクト

	@return	シェーダオブジェクト

	@note	リファレンスカウンタを増加します
			VRAM 節約のため同じ形状がある場合は
			こちらの関数で複製を作ってください。

*/
#define	npCloneShader( pShader )	(	\
	npIncrementObjectReferences( &( pShader )->Object ), ( pShader ) )

/*!

	シェーダオブジェクトを解放する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		対象のシェーダオブジェクト

	@note	リファレンスカウンタが 0 なら VRAM から開放.<br>
			0 になるまで開放されない

*/
NP_API npVOID NP_APIENTRY npReleaseShader(
	struct __npCONTEXT*	pContext,
	npSHADER*			pShader
);

/*!

	リファレンスカウンタを無視してテクスチャを解放する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト

*/
NP_API npVOID NP_APIENTRY npDestroyShader(
	struct __npCONTEXT*	pContext,
	npSHADER*			pShader
);

/*!

	シェーダ名をセットする

	@param	pShader		シェーダオブジェクト
	@param	szName		登録するシェーダ名

	@return	エラーコード

*/
#define	npSetShaderName( pShader, szName )	(	\
	( strlen( ( szName ) ) < NP_MAXPATH ) ? 	\
	( strcpy( ( pShader )->m_szName, ( szName ) ), NP_SUCCESS ) : NP_ERR_OVERFLOW )
 
/*!

	シェーダ名を取得する

	@param	pShader		シェーダオブジェクト
	@param	pName		取得するシェーダ名用のポインタ

	@return	エラーコード
	
*/
#define	npGetShaderName( pShader, pName )	(	\
	strcpy( ( pName ), ( pShader )->m_szName ), NP_SUCCESS )

/*!

	テクニックのハンドルを取得する

	@param	pShader	シェーダオブジェクト
	@param	szName	テクニック名

	@return	テクニックのハンドル

*/
NP_API npHANDLE NP_APIENTRY npTechniqueHANDLE( 
				npSHADER*	pShader, 
	NP_CONST	npCHAR*		szName 
);

/*!

	先頭のテクニックのハンドルを取得する

	@param	pShader	シェーダオブジェクト
	
	@return	テクニックのハンドル

*/
NP_API npHANDLE NP_APIENTRY npFirstTechniqueHANDLE( 
	npSHADER*	pShader 
);

/*!

	指定したテクニックの次のハンドルを取得する

	@param	pShader	シェーダオブジェクト

	@return	テクニックのハンドル

*/
NP_API npHANDLE NP_APIENTRY npNextTechniqueHANDLE( 
				npSHADER*	pShader, 
	NP_CONST	npHANDLE	hPrev 
);

/*!

	テクニックをデバイスにセットする

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hTechnique	テクニックのハンドル

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npRegistTechnique(
	struct	__npCONTEXT*	pContext,
	npSHADER*				pShader,
	npHANDLE				hTechnique
);

/*!

	テクニックを無効にする

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト

	@return	エラーコード

	@note	内容は空

*/
#define	npUnregistTechnique( pContext, pShader )	( NP_SUCCESS )

/*!

	シェーダのテクニックを開始する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hTechnique	テクニックのハンドル
	@param	pPass		パスの数(把握してる場合はNP_NULLで可)

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npBeginTechnique(
	struct __npCONTEXT*	pContext,
	npSHADER*			pShader,
	npU32*				pPass
);

/*!

	シェーダのテクニックを終了する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト

	@return	エラーコード

*/
NP_API npVOID NP_APIENTRY npEndTechnique( 
	struct __npCONTEXT*	pContext,
	npSHADER*			pShader
);

/*!

	シェーダパスを開始する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	nPass		パスのインデックス

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npBeginPass(
	struct __npCONTEXT*	pContext, 
	npSHADER*			pShader,
	npU32				nPass
);

/*!

	シェーダパスを終了する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト

*/
NP_API npVOID NP_APIENTRY npEndPass( 
	struct __npCONTEXT*	pContext, 
	npSHADER*			pShader 
);

/*!

	シェーダ内のグローバル変数のハンドルを取得する

	@param	pShader		シェーダオブジェクト
	@param	szName		変数名

	@return	取得する変数へのハンドル

*/
NP_API npHANDLE NP_APIENTRY npConstantHANDLE(
				npSHADER*	pShader,
	NP_CONST	npCHAR*		szName
);

/*!

	シェーダ内のセマンティクスのハンドルを取得する

	@param	pShader		シェーダオブジェクト
	@param	szName		セマンティクス名

	@return	取得するセマンティクスのハンドル

*/
NP_API npHANDLE NP_APIENTRY npSemanticsHANDLE(
				npSHADER*	pShader,
	NP_CONST	npCHAR*		szName
);

/*!

	シェーダ内のアノテーションのハンドルを取得する

	@param	pShader		シェーダオブジェクト
	@param	szName		アノテーション名

	@return	取得するアノテーションのハンドル

*/
NP_API npHANDLE NP_APIENTRY npAnnotationHANDLE(
				npSHADER*	pShader,
	NP_CONST	npCHAR*		szName
);

/*!

	シェーダ用のレジスタへテクスチャをセットする

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	pTexture	対象のテクスチャオブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npSetConstantTEXTURE(
	struct __npCONTEXT*	pContext,
	npSHADER*			pShader,
	npHANDLE			hParam,
	struct __npTEXTURE*	pTexture
);

/*!

	シェーダ用のレジスタからテクスチャを取得する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	ppTexture	対象のテクスチャオブジェクト

	@note	レンダリングコンテキスト内から変数のハンドルに
			一致するテクスチャを検索する処理が入ります。

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npGetConstantTEXTURE(
	struct __npCONTEXT*		pContext,
	npSHADER*				pShader,
	npHANDLE				hParam,
	struct __npTEXTURE**	ppTexture
);

/*!

	シェーダ用のレジスタへマトリクスをセットする

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	pValue		対象の値

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npSetConstantFMATRIX( 
	struct __npCONTEXT*		pContext, 
	npSHADER*				pShader, 
	npHANDLE				hParam, 
	npFMATRIX*				pValue
);

/*!

	シェーダ用のレジスタからマトリクスを取得する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	pValue		対象の値

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npGetConstantFMATRIX( 
	struct __npCONTEXT*		pContext, 
	npSHADER*				pShader, 
	npHANDLE				hParam, 
	npFMATRIX*				pValue
);

/*!

	シェーダ用のレジスタへ FVEC3 をセットする

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	pValue		対象の値

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npSetConstantFVEC3( 
	struct __npCONTEXT*		pContext, 
	npSHADER*				pShader, 
	npHANDLE				hParam, 
	npFVECTOR*				pValue 
);

/*!

	シェーダ用のレジスタから FVEC4 を取得する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	pValue		対象の値

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npGetConstantFVEC3( 
	struct __npCONTEXT*		pContext, 
	npSHADER*				pShader, 
	npHANDLE				hParam, 
	npFVECTOR*				pValue 
);

/*!

	シェーダ用のレジスタへ FVEC4 をセットする

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	pValue		対象の値

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npSetConstantFVEC4( 
	struct __npCONTEXT*		pContext, 
	npSHADER*				pShader, 
	npHANDLE				hParam, 
	npFVECTOR*				pValue 
);

/*!

	シェーダ用のレジスタから FVEC4 を取得する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	pValue		対象の値

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npGetConstantFVEC4( 
	struct __npCONTEXT*		pContext, 
	npSHADER*				pShader, 
	npHANDLE				hParam, 
	npFVECTOR*				pValue 
);

/*!

	シェーダ用のレジスタへ FRGBA をセットする

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	pValue		対象の値

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npSetConstantFRGBA( 
	struct __npCONTEXT*		pContext, 
	npSHADER*				pShader, 
	npHANDLE				hParam, 
	npFRGBA*				pValue 
);

/*!

	シェーダ用のレジスタから FRGBA を取得する

	@param	pContext	レンダリングコンテキスト
	@param	pShader		シェーダオブジェクト
	@param	hParam		レジスタ変数へのハンドル
	@param	pValue		対象の値

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npGetConstantFRGBA( 
	struct __npCONTEXT*		pContext, 
	npSHADER*				pShader, 
	npHANDLE				hParam, 
	npFRGBA*				pValue 
);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPSHADER_H */
