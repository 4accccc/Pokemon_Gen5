//==============================================================================
/**
 * @file	net_bugfix.h
 * @brief	PLからの移植プログラムをmakeを通す為に無理やりな事をする
 * @author	matsuda
 * @date	2008.12.18(木)
 *
 * 最終的にはこのファイルは削除する	※check
 *
 */
//==============================================================================
#ifndef __NET_BUGFIX_H__
#define __NET_BUGFIX_H__


//==============================================================================
//	マクロ定義
//==============================================================================
/// 色番号指定 PRINT のための定数・マクロ定義

typedef u16		GF_PRINTCOLOR;

#define  GF_PRINTCOLOR_MASK			(0xff)
#define  GF_PRINTCOLOR_LETTER_SHIFT	(10)
#define  GF_PRINTCOLOR_SHADOW_SHIFT	(5)
#define  GF_PRINTCOLOR_GROUND_SHIFT	(0)

// GF_MSG_PrintColor の引数 GF_PRINT_COLOR を作成するためのマクロ（ 文字色、影色、背景色の順に番号を渡す ）
#define  GF_PRINTCOLOR_MAKE(letter, shadow, ground)	(GF_PRINTCOLOR)(	(((letter)&GF_PRINTCOLOR_MASK) << GF_PRINTCOLOR_LETTER_SHIFT)|\
																		(((shadow)&GF_PRINTCOLOR_MASK) << GF_PRINTCOLOR_SHADOW_SHIFT)|\
																		(((ground)&GF_PRINTCOLOR_MASK) << GF_PRINTCOLOR_GROUND_SHIFT)	)

// GF_PRINTCOLOR_MAKE で作った値から各色番号を取り出すマクロ
#define GF_PRINTCOLOR_GET_LETTER(c)		(((c)>>GF_PRINTCOLOR_LETTER_SHIFT)&GF_PRINTCOLOR_MASK)
#define GF_PRINTCOLOR_GET_SHADOW(c)		(((c)>>GF_PRINTCOLOR_SHADOW_SHIFT)&GF_PRINTCOLOR_MASK)
#define GF_PRINTCOLOR_GET_GROUND(c)		(((c)>>GF_PRINTCOLOR_GROUND_SHIFT)&GF_PRINTCOLOR_MASK)


// 通常のフォントカラー
#define	FBMP_COL_NULL		(0)
#define	FBMP_COL_BLACK		(1)
#define	FBMP_COL_BLK_SDW	(2)
#define	FBMP_COL_RED		(3)
#define	FBMP_COL_RED_SDW	(4)
#define	FBMP_COL_GREEN		(5)
#define	FBMP_COL_GRN_SDW	(6)
#define	FBMP_COL_BLUE		(7)
#define	FBMP_COL_BLU_SDW	(8)
#define	FBMP_COL_PINK		(9)
#define	FBMP_COL_PNK_SDW	(10)
#define	FBMP_COL_WHITE		(15)


// FONT_SYSTEM, FONT_BUTTON が無くなったので、とりあえずMAKEを通す為に仮置換用
#define NET_FONT_SYSTEM			(0)
#define NET_FONT_BUTTON			(0)


//==============================================================================
//	構造体定義
//==============================================================================
///BMPWIN_DATがWBでは存在しない為、置換用の構造体
typedef struct{
	u8	frm_num;	///<ウインドウ使用フレーム
	u8	pos_x;		///<ウインドウ領域の左上のX座標（キャラ単位で指定）
	u8	pos_y;		///<ウインドウ領域の左上のY座標（キャラ単位で指定）
	u8	siz_x;		///<ウインドウ領域のXサイズ（キャラ単位で指定）
	u8	siz_y;		///<ウインドウ領域のYサイズ（キャラ単位で指定）
	u8	palnum;		///<ウインドウ領域のパレットナンバー
	u16	chrnum;		///<ウインドウキャラ領域の開始キャラクタナンバー
}NET_BMPWIN_DAT;


//==============================================================================
//	inline関数
//==============================================================================
#include "arc_def.h"
#include "font/font.naix"

//--------------------------------------------------------------
/**
 * @brief   システムフォントパレット転送
 *
 * @param   type		
 * @param   offs		
 * @param   heap		
 *
 * 現時点ではシステムフォントや会話フォントといったものが存在するかは不明なので
 * 移植する場合はここに集約しておく
 */
//--------------------------------------------------------------
static inline void SystemFontPaletteLoad(u32 type, u32 offs, u32 heap)
{
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
		offs, 0x20, heap);
}

//--------------------------------------------------------------
/**
 * @brief   会話フォントパレット転送
 *
 * @param   type		
 * @param   offs		
 * @param   heap		
 *
 * 現時点ではシステムフォントや会話フォントといったものが存在するかは不明なので
 * 移植する場合はここに集約しておく
 */
//--------------------------------------------------------------
static inline void TalkFontPaletteLoad(u32 type, u32 offs, u32 heap)
{
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
		offs, 0x20, heap);
}


#endif	//__NET_BUGFIX_H__

