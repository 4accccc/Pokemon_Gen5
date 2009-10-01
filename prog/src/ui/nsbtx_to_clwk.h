//============================================================================
/**
 *
 *	@file		nsbtx_to_clwk.h
 *	@brief  テクスチャをCLWKに変換
 *	@author	hosaka genya
 *	@data		2009.10.31
 *
 */
//============================================================================
#pragma once


//-----------------------------------------------------------------------------
/**
 *	@brief  nsbtxをニトロキャラクタ、ニトロパレットに変換してCLWKに転送
 *
 *	@param	GFL_CLWK* act
 *	@param  arc_idx
 *	@param	tex_idx
 *	@param	ptn_ofs
 *	@param	sx
 *	@param	sy
 *	@param	VramOfs
 *	@param	vram_type
 *	@param	HeapID 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
extern void CLWK_TransNSBTX( GFL_CLWK* act, u32 arc_idx, u32 tex_idx, u8 ptn_ofs, u16 sx, u16 sy, u32 VramOfs, CLSYS_DRAW_TYPE vram_type, HEAPID HeapID );

