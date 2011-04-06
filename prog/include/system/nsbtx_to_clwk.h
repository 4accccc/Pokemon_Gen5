//============================================================================
/**
 *
 *	@file		nsbtx_to_clwk.h
 *	@brief  テクスチャをCLWKに変換
 *	@author	hosaka genya
 *	@data		2009.10.31
 *	@note   フィールドマップモデルのリソースは resource\fldmmdl にあります。
 *
 */
//============================================================================
#pragma once

#include "system/palanm.h"

enum
{ 
  // 人物のフィールドOBJは基本 4 x 4
  NSBTX_DEF_SX = 4,
  NSBTX_DEF_SY = 4,
};

//-----------------------------------------------------------------------------
/**
 *	@brief  nsbtxをニトロキャラクタ、ニトロパレットに変換してCLWKに転送
 *
 *	@param	GFL_CLWK* act アクターへのポインタ
 *	@param  arc_idx   アーカイブＩＤ
 *	@param	tex_idx   テクスチャのARC内インデックス
 *	@param  ptn_ofs   テクスチャオフセット（アニメパターンのオフセット）
 *	@param	sx        転送元データのサイズＸ（キャラ単位）
 *	@param	sy        転送元データのサイズＹ（キャラ単位）
 *	@param	VramOfs   転送先オフセット（Byte) (登録済みCGRが転送されているアドレスからのオフセット）
 *  @param  vramType  転送先VramType  
 *	@param	HeapID    ヒープＩＤ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void CLWK_TransNSBTX( GFL_CLWK* act, u32 arc_idx, u32 tex_idx, u8 ptn_ofs, u16 sx, u16 sy, u32 VramOfs, CLSYS_DRAW_TYPE vram_type, HEAPID HeapID );

//-----------------------------------------------------------------------------
/**
 *	@brief  nsbtxをニトロキャラクタ、ニトロパレットに変換してCLWKに転送(パレットアニメ対応バージョン)
 *
 *	@param	GFL_CLWK* act アクターへのポインタ
 *	@param  arc_idx   アーカイブＩＤ
 *	@param	tex_idx   テクスチャのARC内インデックス
 *	@param  ptn_ofs   テクスチャオフセット（アニメパターンのオフセット）
 *	@param	sx        転送元データのサイズＸ（キャラ単位）
 *	@param	sy        転送元データのサイズＹ（キャラ単位）
 *	@param	VramOfs   転送先オフセット（Byte) (登録済みCGRが転送されているアドレスからのオフセット）
 *  @param  vramType  転送先VramType
 *  @param  pfd       初期化済みPALETTE_FADE_PTR(NULLなら直接VRAMに転送)
 *  @param  palno     アクターが参照しているパレットNo指定(0-15) GFL_CLWKからNoを取得できる関数が無いので自分で指定
 *	@param	HeapID    ヒープＩＤ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void CLWK_TransNSBTX_Ex( GFL_CLWK* act,
    u32 arc_idx, u32 tex_idx, u8 ptn_ofs, u16 sx, u16 sy, u32 VramOfs,
    CLSYS_DRAW_TYPE vram_type, PALETTE_FADE_PTR pfd, u8 palno, HEAPID HeapID );
