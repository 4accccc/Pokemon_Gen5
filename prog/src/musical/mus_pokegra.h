//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mus_pokegra.h
 *	@brief	ミュージカル専用ポケモングラフィック素材の取得(ＢＷの計算を行う）
 *	@author	Akito Mori(BWから引越し）
 *	@date		2011.12.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//ライブラリ
#include <gflib.h>

//-------------------------------------
///	リソース取得
//=====================================
//NCGR
extern ARCDATID MUS_POKEGRA_GetCgrArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NCBR
extern ARCDATID MUS_POKEGRA_GetCbrArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NCLR
extern ARCDATID MUS_POKEGRA_GetPalArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NCER	-> マルチセル用のパーツです。
extern ARCDATID MUS_POKEGRA_GetCelArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NANR
extern ARCDATID MUS_POKEGRA_GetAnmArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NMCR
extern ARCDATID MUS_POKEGRA_GetMCelArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NMAR
extern ARCDATID MUS_POKEGRA_GetMAnmArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
//NCEC MCSS用ファイル
extern ARCDATID MUS_POKEGRA_GetNcecArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg );
