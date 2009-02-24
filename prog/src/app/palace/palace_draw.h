//==============================================================================
/**
 * @file	palace_draw.h
 * @brief	パレス描画のヘッダ
 * @author	matsuda
 * @date	2009.02.23(月)
 */
//==============================================================================
#ifndef __PALACE_DRAW_H__
#define __PALACE_DRAW_H__


//==============================================================================
//	外部関数宣言
//==============================================================================
extern void PalaceDraw_LoadTex(u32 destAddress, u32 destPlttAddress);
extern void PalaceDraw_Cube(const VecFx16 *add_pos, GXRgb rgb);


#endif	//__PALACE_DRAW_H__
