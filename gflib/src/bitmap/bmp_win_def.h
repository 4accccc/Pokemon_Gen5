//=============================================================================================
/**
 * @file	bmp_win_def.h
 * @brief	BG描画システムプログラム
 * @author	Hiroyuki Nakamura
 * @date	2006/10/18
 */
//=============================================================================================
#ifndef _BMP_WIN_DEF_H_
#define _BMP_WIN_DEF_H_

///BMPWINデータ構造体
struct gfl_bmpwin_data{
	GFL_BG_INI	*bgl;
	u32			heapID;
	u8			frmnum;
	u8			posx;
	u8			posy;
	u8			sizx;
	u8			sizy;
	u8			palnum;
	u16			chrofs:15;
	u16			bitmode:1;	
	void		*chrbuf;
};

#endif _BMP_WIN_DEF_H_
