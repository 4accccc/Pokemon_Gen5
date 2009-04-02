//==============================================================================
/**
 * @file	gra_tool.h
 * @brief	グラフィックツール
 * @author	goto
 * @date	2005.11.16
 *
 * ここに色々な解説等を書いてもよい
 *	GSより移植。元はsystem/gra_toolだがトレーナーの変換に必要なものだけ移植	Ari090331
 */
//==============================================================================

#ifndef	_UNION_GRA_TOOL_H_
#define _UNION_GRA_TOOL_H_

// -----------------------------------------
//
//	■定数定義
//
// -----------------------------------------
// 2D→1D変換に使用
#define CHAR_DOT_X		(8/2)								///< 1キャラのバイトサイズ
#define CHAR_DOT_Y		(8)									///< 1キャラのバイトサイズ
#define CHAR_BYTE		(CHAR_DOT_X * CHAR_DOT_Y)			///< 1キャラバイト数

void ChangesInto_1D_from_2D(int arc_id, int index_no, int heap_id, int x, int y, int w, int h, void* buff);

#endif	//_UNION_GRA_TOOL_H_

