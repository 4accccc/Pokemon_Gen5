//======================================================================
/**
 * @file	window.h
 * @brief	ウィンドウ表示
 * @author	Hiroyuki Nakamura
 * @date	2005.10.13
 */
//======================================================================
#ifndef WINDOW_H
#define WINDOW_H

#include <gflib.h>

//======================================================================
//	定数定義
//======================================================================
#define	MENU_WIN_CGX_SIZ	( 9 )		// メニューウィンドウのキャラ数
#define	TALK_WIN_CGX_SIZ	( 18+12 )		// 会話ウィンドウのキャラ数
#define	BOARD_WIN_CGX_SIZ	( 18+12 )		// 看板ウィンドウのキャラ数
#define	BOARD_MAP_CGX_SIZ	( 24 )		// 看板のマップのキャラ数

// 町看板、道看板の全キャラ数
#define	BOARD_DATA_CGX_SIZ	( BOARD_WIN_CGX_SIZ + BOARD_MAP_CGX_SIZ )

enum {
	WINDOW_TRANS_ON = 0,
	WINDOW_TRANS_OFF
};

// メニューウィンドウのタイプ
enum {
	MENU_TYPE_SYSTEM = 0,	// 選択ウィンドウ（システムウィンドウ）
	MENU_TYPE_FIELD,		// フィールドメニュー
	MENU_TYPE_UG			// 地下メニュー
};

//======================================================================
//	プロトタイプ宣言
//======================================================================
//--------------------------------------------------------------
/**
 * メニューウィンドウのキャラをセット
 *
 * @param	ini			BGLデータ
 * @param	frmnum		BGフレーム
 * @param	cgx			キャラ転送位置
 * @param	win_num		ウィンドウ番号
 * @param	heap		ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BmpWinFrame_CgxSet( u8 frmnum, u16 cgx, u8 win_num, u32 heap );

//--------------------------------------------------------------
/**
 * メニューウィンドウパレットのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------
extern u32 BmpWinFrame_WinPalArcGet(void);

//--------------------------------------------------------------
/**
 * メニューウィンドウのグラフィックをセット
 *
 * @param	ini			BGLデータ
 * @param	frmnum		BGフレーム
 * @param	cgx			キャラ転送位置
 * @param	pal			パレット番号
 * @param	win_num		ウィンドウ番号
 * @param	heap		ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BmpWinFrame_GraphicSet(
	u8 frmnum, u16 cgx, u8 pal, u8 win_num, u32 heap );

//--------------------------------------------------------------
/**
 * メニューウィンドウを描画
 *
 * @param	win			BMPウィンドウデータ
 * @param	trans_sw	転送スイッチ
 * @param	win_cgx		ウィンドウキャラ位置
 * @param	pal			パレット
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BmpWinFrame_Write(
		GFL_BMPWIN *win, u8 trans_sw, u16 win_cgx, u8 pal );

//--------------------------------------------------------------
/**
 *	メニューウィンドウをクリア
 *
 * @param	win			BMPウィンドウデータ
 * @param	trans_sw	転送スイッチ
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BmpWinFrame_Clear( GFL_BMPWIN *win, u8 trans_sw );

#endif	// WINDOW_H
