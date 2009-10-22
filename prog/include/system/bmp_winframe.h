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
	WINDOW_TRANS_ON_V,	//Vブランクで転送
	WINDOW_TRANS_OFF
};

// メニューウィンドウのタイプ
enum {
	MENU_TYPE_SYSTEM = 0,	// 選択ウィンドウ（システムウィンドウ）
	MENU_TYPE_FIELD,		// フィールドメニュー
	MENU_TYPE_UG			// 地下メニュー
};

// 看板
#define BOARD_TYPE_TOWN	(0)	// タウンマップ
#define	BOARD_TYPE_ROAD	(1)	// 標識
#define	BOARD_TYPE_POST	(2)	// 表札
#define	BOARD_TYPE_INFO	(3)	// 掲示板


typedef struct{
  u32 pos;
  u32 size;
} BMPWINFRAME_AREAMANAGER_POS;


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
extern void BmpWinFrame_CgxSet( u8 frmnum, u16 cgx, u8 win_num, HEAPID heap );

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
	u8 frmnum, u16 cgx, u8 pal, u8 win_num, HEAPID heap );

//--------------------------------------------------------------
/**
 * エリアマネージャーを使ってメニューウィンドウのグラフィックをセット
 *
 * @param	ini			BGLデータ
 * @param	frmnum		BGフレーム
 * @param	pal			パレット番号
 * @param	win_num		ウィンドウ番号
 * @param	heap		ヒープID
 *
 * @return	GFL_ARCUTIL_TRANSINFO
 */
//--------------------------------------------------------------
extern GFL_ARCUTIL_TRANSINFO BmpWinFrame_GraphicSetAreaMan( u8 frmnum, u8 pal, u8 win_num, HEAPID heap );

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

//--------------------------------------------------------------
/**
 * @brief   BMPWINスクリーン転送
 *
 * @param	win				BMPウィンドウデータ
 * @param   trans_sw		転送スイッチ
 */
//--------------------------------------------------------------
extern void BmpWinFrame_TransScreen(GFL_BMPWIN *win, u8 trans_sw);

//--------------------------------------------------------------
/**
 * 会話ウィンドウのグラフィックをセット
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
extern void TalkWinFrame_GraphicSet(u8 frmnum, u16 cgx, u8 pal, u8 win_num, HEAPID heap );

//--------------------------------------------------------------
/**
 * エリアマネージャーを使って会話ウィンドウのグラフィックをセット
 *
 * @param	ini			BGLデータ
 * @param	frmnum		BGフレーム
 * @param	pal			パレット番号
 * @param	win_num		ウィンドウ番号
 * @param	heap		ヒープID
 *
 * @return	GFL_ARCUTIL_TRANSINFO
 */
//--------------------------------------------------------------
extern GFL_ARCUTIL_TRANSINFO TalkWinFrame_GraphicSetAreaMan( u8 frmnum, u8 pal, u8 win_num, HEAPID heap );

//--------------------------------------------------------------
/**
 * 会話ウィンドウを描画
 *
 * @param	win			BMPウィンドウデータ
 * @param	trans_sw	転送スイッチ
 * @param	win_cgx		ウィンドウキャラ位置
 * @param	pal			パレット
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void TalkWinFrame_Write( GFL_BMPWIN *win, u8 trans_sw, u16 win_cgx, u8 pal );

//--------------------------------------------------------------
/**
 *	会話ウィンドウをクリア
 *
 * @param	win			BMPウィンドウデータ
 * @param	trans_sw	転送スイッチ
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void TalkWinFrame_Clear( GFL_BMPWIN *win, u8 trans_sw );

//--------------------------------------------------------------
/**
 * 看板ウィンドウのグラフィックをセット
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
extern void BoardWinFrame_GraphicSet(u8 frmnum, u16 cgx, u8 pal, u8 type, u16 map, HEAPID heap );

//--------------------------------------------------------------
/**
 * エリアマネージャーを使って看板ウィンドウのグラフィックをセット
 *
 * @param	ini			BGLデータ
 * @param	frmnum		BGフレーム
 * @param	pal			パレット番号
 * @param	win_num		ウィンドウ番号
 * @param	heap		ヒープID
 *
 * @return	GFL_ARCUTIL_TRANSINFO
 */
//--------------------------------------------------------------
extern GFL_ARCUTIL_TRANSINFO BoardWinFrame_GraphicSetAreaMan( u8 frmnum, u8 pal, u8 type, u16 map, HEAPID heap );

//--------------------------------------------------------------
/**
 * 看板ウィンドウを描画
 *
 * @param	win			BMPウィンドウデータ
 * @param	trans_sw	転送スイッチ
 * @param	win_cgx		ウィンドウキャラ位置
 * @param	pal			パレット
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BoardWinFrame_Write( GFL_BMPWIN *win, u8 trans_sw, u16 win_cgx, u8 pal, u8 type );

//--------------------------------------------------------------
/**
 *	看板ウィンドウをクリア
 *
 * @param	win			BMPウィンドウデータ
 * @param	trans_sw	転送スイッチ
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BoardWinFrame_Clear( GFL_BMPWIN *win, u8 type, u8 trans_sw );


#endif	// WINDOW_H
