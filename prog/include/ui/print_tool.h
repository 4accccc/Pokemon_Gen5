//============================================================================================
/**
 * @file		print_tool.h
 * @brief		文字描画ツール
 * @author	Hiroyuki Nakamura
 * @date		09.10.01
 *
 *	モジュール名：PRINTTOOL
 */
//============================================================================================
#pragma	once

#include "print/printsys.h"


//============================================================================================
//	定数定義
//============================================================================================

// 文字表示モード
enum {
	PRINTTOOL_MODE_LEFT = 0,	// 左詰
	PRINTTOOL_MODE_RIGHT,			// 右詰
	PRINTTOOL_MODE_CENTER			// 中央
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		基準座標を指定して文字列描画
 *
 * @param		wk			PRINT_UTIL
 * @param		que			PRINT_QUE
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		buf			文字列
 * @param		font		フォント
 * @param		mode		表示モード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void PRINTTOOL_Print(
				PRINT_UTIL * wk, PRINT_QUE * que, u16 x, u16 y,
				const STRBUF * buf, GFL_FONT * font, u32 mode );


//--------------------------------------------------------------------------------------------
/**
 * @brief		基準座標を指定して文字列描画（カラー指定）
 *
 * @param		wk			PRINT_UTIL
 * @param		que			PRINT_QUE
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		buf			文字列
 * @param		font		フォント
 * @param		color		文字色
 * @param		mode		表示モード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void PRINTTOOL_PrintColor(
				PRINT_UTIL * wk, PRINT_QUE * que, u16 x, u16 y,
				const STRBUF * buf, GFL_FONT * font, PRINTSYS_LSB color, u32 mode );

//--------------------------------------------------------------------------------------------
/**
 * @brief		[ ?? / ?? ] 表示
 *
 * @param		wk			PRINT_UTIL
 * @param		que			PRINT_QUE
 * @param		font		フォント
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		nowPrm	現在の値
 * @param		maxPrm	最大値
 * @param		heapID	ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void PRINTTOOL_PrintFraction(
							PRINT_UTIL * wk, PRINT_QUE * que, GFL_FONT * font,
							u16 x, u16 y, u32 nowPrm, u32 maxPrm, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		[ ?? / ?? ] 表示（カラー指定）
 *
 * @param		wk			PRINT_UTIL
 * @param		que			PRINT_QUE
 * @param		font		フォント
 * @param		x				表示基準Ｘ座標
 * @param		y				表示基準Ｙ座標
 * @param		color		文字色
 * @param		nowPrm	現在の値
 * @param		maxPrm	最大値
 * @param		heapID	ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void PRINTTOOL_PrintFractionColor(
							PRINT_UTIL * wk, PRINT_QUE * que, GFL_FONT * font,
							u16 x, u16 y, PRINTSYS_LSB color, u32 nowPrm, u32 maxPrm, HEAPID heapID );
