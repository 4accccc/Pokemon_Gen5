/*=============================================================================
/*!

	@file	npRenderState.h

	@brief	レンダーステート定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.13 $
	$Date: 2006/03/22 05:10:40 $

*//*=========================================================================*/
#ifndef	NPRENDERSTATE_H
#define	NPRENDERSTATE_H

/*! @name	C リンケージ外対応 */
//!@{

#include "core/npTypedef.h"
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

	@enum	レンダーステート

	@note	ステートの数はアラインに会わせる

	@todo	随時追加予定

*/
enum npRENDERSTATE
{
	NP_SHADE_DEPTHTEST = 0,	//	= D3DRS_ZENABLE,
	NP_SHADE_FILLMODE,		//	= D3DRS_SHADEMODE,
	NP_SHADE_DEPTHWRITE,	//	= D3DRS_ZWRITEENABLE,
	NP_SHADE_ALPHATEST,		//	= D3DRS_ALPHABLENDENABLE,
	NP_SHADE_SRCBLEND,		//	= D3DRS_SRCBLEND,
	NP_SHADE_DSTBLEND,		//	= D3DRS_DESTBLEND,
	NP_SHADE_CULLMODE,		//	= D3DRS_CULLMODE,
	NP_SHADE_DEPTHFUNC,		//	= D3DRS_ZFUNC,
	//NP_SHADE_ALPHAFUNC,
	//NP_SHADE_SPECULAR,
	NP_SHADE_LIGHTING,		//	= D3DRS_LIGHTING,
	//NP_SHADE_BLENDOP,
	//NP_SHADE_SRCBLENDALPHA,
	//NP_SHADE_DSTBLENDALPHA,
	//NP_SHADE_BLENDOPALPHA,
/*
    D3DRS_ZENABLE			= 7,
    D3DRS_FILLMODE			= 8,
    D3DRS_SHADEMODE			= 9,
    D3DRS_ZWRITEENABLE		= 14,
    D3DRS_ALPHATESTENABLE	= 15,
    D3DRS_LASTPIXEL			= 16,
    D3DRS_SRCBLEND = 19,
    D3DRS_DESTBLEND = 20,
    D3DRS_CULLMODE = 22,
    D3DRS_ZFUNC = 23,
    D3DRS_ALPHAREF = 24,
    D3DRS_ALPHAFUNC = 25,
    D3DRS_DITHERENABLE = 26,
    D3DRS_ALPHABLENDENABLE = 27,
    D3DRS_FOGENABLE = 28,
    D3DRS_SPECULARENABLE = 29,
    D3DRS_FOGCOLOR = 34,
    D3DRS_FOGTABLEMODE = 35,
    D3DRS_FOGSTART = 36,
    D3DRS_FOGEND = 37,
    D3DRS_FOGDENSITY = 38,
    D3DRS_RANGEFOGENABLE = 48,
    D3DRS_STENCILENABLE = 52,
    D3DRS_STENCILFAIL = 53,
    D3DRS_STENCILZFAIL = 54,
    D3DRS_STENCILPASS = 55,
    D3DRS_STENCILFUNC = 56,
    D3DRS_STENCILREF = 57,
    D3DRS_STENCILMASK = 58,
    D3DRS_STENCILWRITEMASK = 59,
    D3DRS_TEXTUREFACTOR = 60,
    D3DRS_WRAP0 = 128,
    D3DRS_WRAP1 = 129,
    D3DRS_WRAP2 = 130,
    D3DRS_WRAP3 = 131,
    D3DRS_WRAP4 = 132,
    D3DRS_WRAP5 = 133,
    D3DRS_WRAP6 = 134,
    D3DRS_WRAP7 = 135,
    D3DRS_CLIPPING = 136,
    D3DRS_LIGHTING = 137,
    D3DRS_AMBIENT = 139,
    D3DRS_FOGVERTEXMODE = 140,
    D3DRS_COLORVERTEX = 141,
    D3DRS_LOCALVIEWER = 142,
    D3DRS_NORMALIZENORMALS = 143,
    D3DRS_DIFFUSEMATERIALSOURCE = 145,
    D3DRS_SPECULARMATERIALSOURCE = 146,
    D3DRS_AMBIENTMATERIALSOURCE = 147,
    D3DRS_EMISSIVEMATERIALSOURCE = 148,
    D3DRS_VERTEXBLEND = 151,
    D3DRS_CLIPPLANEENABLE = 152,
    D3DRS_POINTSIZE = 154,
    D3DRS_POINTSIZE_MIN = 155,
    D3DRS_POINTSPRITEENABLE = 156,
    D3DRS_POINTSCALEENABLE = 157,
    D3DRS_POINTSCALE_A = 158,
    D3DRS_POINTSCALE_B = 159,
    D3DRS_POINTSCALE_C = 160,
    D3DRS_MULTISAMPLEANTIALIAS = 161,
    D3DRS_MULTISAMPLEMASK = 162,
    D3DRS_PATCHEDGESTYLE = 163,
    D3DRS_DEBUGMONITORTOKEN = 165,
    D3DRS_POINTSIZE_MAX = 166,
    D3DRS_INDEXEDVERTEXBLENDENABLE = 167,
    D3DRS_COLORWRITEENABLE = 168,
    D3DRS_TWEENFACTOR = 170,
    D3DRS_BLENDOP = 171,
    D3DRS_POSITIONDEGREE = 172,
    D3DRS_NORMALDEGREE = 173,
    D3DRS_SCISSORTESTENABLE = 174,
    D3DRS_SLOPESCALEDEPTHBIAS = 175,
    D3DRS_ANTIALIASEDLINEENABLE = 176,
    D3DRS_MINTESSELLATIONLEVEL = 178,
    D3DRS_MAXTESSELLATIONLEVEL = 179,
    D3DRS_ADAPTIVETESS_X = 180,
    D3DRS_ADAPTIVETESS_Y = 181,
    D3DRS_ADAPTIVETESS_Z = 182,
    D3DRS_ADAPTIVETESS_W = 183,
    D3DRS_ENABLEADAPTIVETESSELATION = 184,
    D3DRS_TWOSIDEDSTENCILMODE = 185,
    D3DRS_CCW_STENCILFAIL = 186,
    D3DRS_CCW_STENCILZFAIL = 187,
    D3DRS_CCW_STENCILPASS = 188,
    D3DRS_CCW_STENCILFUNC = 189,
    D3DRS_COLORWRITEENABLE1 = 190,
    D3DRS_COLORWRITEENABLE2 = 191,
    D3DRS_COLORWRITEENABLE3 = 192,
    D3DRS_BLENDFACTOR = 193,
    D3DRS_SRGBWRITEENABLE = 194,
    D3DRS_DEPTHBIAS = 195,
    D3DRS_WRAP8 = 198,
    D3DRS_WRAP9 = 199,
    D3DRS_WRAP10 = 200,
    D3DRS_WRAP11 = 201,
    D3DRS_WRAP12 = 202,
    D3DRS_WRAP13 = 203,
    D3DRS_WRAP14 = 204,
    D3DRS_WRAP15 = 205,
    D3DRS_SEPARATEALPHABLENDENABLE = 206,
    D3DRS_SRCBLENDALPHA = 207,
    D3DRS_DESTBLENDALPHA = 208,
    D3DRS_BLENDOPALPHA = 209,
*/
	NP_SHADE_MAX
};

/*!

	@enum	ポリゴンの塗りつぶし方法

*/
/*
	NP_FILL_FLAT	= GL_FLAT,	//	= D3DSHADE_FLAT,
    NP_FILL_SMOOTH	= GL_SMOOTH	//	= D3DSHADE_GOURAUD
*/

enum npFILLMODE
{
	NP_FILL_FLAT	= 0,	//	= D3DSHADE_FLAT ,	/*! フラットシェーディング */
    NP_FILL_SMOOTH	= 1		//	= D3DSHADE_GOURAUD	/*! グーローシェーディング */
};

/*!

	@enum	サポートする比較関数

	LEQUALしかサポートされていない
	
*/
/*
enum npCOMPARETYPE
{
	NP_NEVER		= GL_NEVER,			//	= D3DCMP_NEVER,
	NP_LESS			= GL_LESS,			//	= D3DCMP_LESS,
	NP_EQUAL		= GL_EQUAL,			//	= D3DCMP_EQUAL,
	NP_LESSEQUAL	= GL_LEQUAL,		//	= D3DCMP_LESSEQUAL,
	NP_GREATER		= GL_GREATER,		//	= D3DCMP_GREATER,
	NP_NOTEQUAL		= GL_NOTEQUAL,		//	= D3DCMP_NOTEQUAL,
	NP_GREATEREQUAL	= GL_GEQUAL,		//	= D3DCMP_GREATEREQUAL,
	NP_ALWAYS		= GL_ALWAYS,		//	= D3DCMP_ALWAYS
};
*/
enum npCOMPARETYPE
{
	NP_LESSEQUAL	= 0		//	= D3DCMP_LESSEQUAL,
};

/*!

	@enum	サポートするカリングモード

*/
enum npCULLMODE
{
	NP_CULL_NONE	= GX_CULL_NONE,		// = D3DCULL_NONE,
	NP_CULL_CW		= GX_CULL_FRONT,	// = D3DCULL_CW,
	NP_CULL_CCW		= GX_CULL_BACK		// = D3DCULL_CCW
};

/*!

	@enum	サポートするブレンド係数

*/
enum npBLENDSTATE
{
	/*! Blending ratio is (0, 0, 0, 0) */
	NP_BLEND_ZERO			= 0,					//	= D3DBLEND_ZERO,

	/*! Blending ratio is (1, 1, 1, 1) */
	NP_BLEND_ONE			= 1,					//	= D3DBLEND_ONE,

	/*! Blending ratio is (Rs, Gs, Bs, As) */
	NP_BLEND_SRCCOLOR		= 2,				//	= D3DBLEND_SRCCOLOR,

	/*! Blending ratio is (1-Rs, 1-Gs, 1-Bs, 1-As) */
	NP_BLEND_INVSRCCOLOR	= 3,	//	= D3DBLEND_INVSRCCOLOR,

	/*! Blending ratio is (As, As, As, As) */
	NP_BLEND_SRCALPHA		= 4,				//	= D3DBLEND_SRCALPHA,

	/*! Blending ratio is (1-As, 1-As, 1-As, 1-As) */
	NP_BLEND_INVSRCALPHA	= 5,	//	= D3DBLEND_INVSRCALPHA,

	/*! Blending ratio is (Ad, Ad, Ad, Ad) */
	NP_BLEND_DSTALPHA		= 6,				//	= D3DBLEND_DESTALPHA,

	/*! Blending ratio is (1-Ad, 1-Ad, 1-Ad, 1-Ad) */
	NP_BLEND_INVDSTALPHA	= 7,	//	= D3DBLEND_INVDESTALPHA,

	/*! Blending ratio is (Rd, Gd, Bd, Ad) */
	NP_BLEND_DSTCOLOR		= 8,				//	= D3DBLEND_DESTCOLOR,

	/*! Blending ratio is (1-Rd, 1-Gd, 1-Bd, 1-Ad) */
	NP_BLEND_INVDSTCOLOR	= 9	//	= D3DBLEND_INVDESTCOLOR

};

/*
enum
{
    GX_MODULATE,
    GX_DECAL,
    GX_BLEND,
    GX_REPLACE,
    GX_PASSCLR
};
*/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
struct __npCONTEXT;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	レンダリングステートパラメータを設定する。

	@param	pContext	レンダリングコンテキスト
	@param	nType		デバイスステート
	@param	nState		レンダリングステートに設定する新しい値

	@return	エラーコード

	@note	SetRenderState() は npBeginRender()/npEndRender() の外で使用する

*/
NP_API npERROR NP_APIENTRY npSetRenderState( 
	struct	__npCONTEXT*	pContext,
	npSTATE					nType,
	npSTATE					nState
);

/*!

	レンダリングステートの値を取得する

	@param	pContext	レンダリングコンテキスト
	@param	nType		デバイスステート
	@param	pState		問い合わせたレンダリングステート変数の値を<br>
						格納する変数へのポインタ

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npGetRenderState( 
	struct	__npCONTEXT*	pContext,
	npSTATE					nType,
	npSTATE*				pState
);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPRENDERSTATE_H */
