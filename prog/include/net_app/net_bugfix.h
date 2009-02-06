//==============================================================================
/**
 * @file	net_bugfix.h
 * @brief	PLからの移植プログラムをmakeを通す為に無理やりな事をする
 * @author	matsuda
 * @date	2008.12.18(木)
 *
 * 最終的にはこのファイルは削除する
 *
 */
//==============================================================================
#ifndef __NET_BUGFIX_H__
#define __NET_BUGFIX_H__


//==============================================================================
//	マクロ定義
//==============================================================================
/// 色番号指定 PRINT のための定数・マクロ定義

typedef u32		GF_PRINTCOLOR;

#define  GF_PRINTCOLOR_MASK			(0xff)
#define  GF_PRINTCOLOR_LETTER_SHIFT	(16)
#define  GF_PRINTCOLOR_SHADOW_SHIFT	(8)
#define  GF_PRINTCOLOR_GROUND_SHIFT	(0)

// GF_MSG_PrintColor の引数 GF_PRINT_COLOR を作成するためのマクロ（ 文字色、影色、背景色の順に番号を渡す ）
#define  GF_PRINTCOLOR_MAKE(letter, shadow, ground)	(GF_PRINTCOLOR)(	(((letter)&GF_PRINTCOLOR_MASK) << GF_PRINTCOLOR_LETTER_SHIFT)|\
																		(((shadow)&GF_PRINTCOLOR_MASK) << GF_PRINTCOLOR_SHADOW_SHIFT)|\
																		(((ground)&GF_PRINTCOLOR_MASK) << GF_PRINTCOLOR_GROUND_SHIFT)	)

// GF_PRINTCOLOR_MAKE で作った値から各色番号を取り出すマクロ
#define GF_PRINTCOLOR_GET_LETTER(c)		(((c)>>GF_PRINTCOLOR_LETTER_SHIFT)&GF_PRINTCOLOR_MASK)
#define GF_PRINTCOLOR_GET_SHADOW(c)		(((c)>>GF_PRINTCOLOR_SHADOW_SHIFT)&GF_PRINTCOLOR_MASK)
#define GF_PRINTCOLOR_GET_GROUND(c)		(((c)>>GF_PRINTCOLOR_GROUND_SHIFT)&GF_PRINTCOLOR_MASK)




#endif	//__NET_BUGFIX_H__

