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
typedef enum {
	MENU_TYPE_SYSTEM = 0,	// 選択ウィンドウ（システムウィンドウ）
	MENU_TYPE_TALK,		// フィールドのプレーンウィンドウ(と同じ見た目)
	MENU_TYPE_MAX,
}MENU_TYPE;

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
 * ウィンドウキャラクタデータのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------
extern u32 BmpWinFrame_CgxArcGet( MENU_TYPE win_type );

//--------------------------------------------------------------
/**
 * ウィンドウパレットのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------
extern u32 BmpWinFrame_WinPalArcGet( MENU_TYPE win_type );

//--------------------------------------------------------------
/**
 * メニューウィンドウのキャラをセット
 *
 * @param	frmnum		BGフレーム
 * @param	cgx			  キャラ転送位置
 * @param	win_type	ウィンドウタイプ号 MENU_TYPE型
 * @param	heap		  ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BmpWinFrame_CgxSet( u8 frmnum, u16 cgx, MENU_TYPE win_type, HEAPID heap );

//--------------------------------------------------------------
/**
 * メニューウィンドウのパレットをセット
 *
 * @param	frmnum		BGフレーム
 * @param	pal			  パレット転送位置(パレット単位)
 * @param	win_type		ウィンドウタイプ号 MENU_TYPE型
 * @param	heap		ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BmpWinFrame_PalSet( u8 frmnum, u8 pal, MENU_TYPE win_type, HEAPID heap );

//--------------------------------------------------------------
/**
 * ウィンドウフレームのグラフィックをセット
 *
 * @param	ini			  BGLデータ
 * @param	frmnum		BGフレーム
 * @param	cgx			  キャラ転送位置
 * @param	pal			  パレット番号
 * @param	win_type	ウィンドウ番号 MENU_TYPE型
 * @param	heap		ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BmpWinFrame_GraphicSet(
	u8 frmnum, u16 cgx, u8 pal, MENU_TYPE win_type, HEAPID heap );

//--------------------------------------------------------------
/**
 * エリアマネージャーを使ってウィンドウのグラフィックをセット
 *
 * @param	ini			  BGLデータ
 * @param	frmnum		BGフレーム
 * @param	pal			  パレット番号
 * @param	win_type	ウィンドウ番号 MENU_TYPE型
 * @param	heap		  ヒープID
 *
 * @return	GFL_ARCUTIL_TRANSINFO
 */
//--------------------------------------------------------------
extern GFL_ARCUTIL_TRANSINFO BmpWinFrame_GraphicSetAreaMan( u8 frmnum, u8 pal, MENU_TYPE win_type, HEAPID heap );

//--------------------------------------------------------------
/**
 * エリアマネージャーを使ってセットしたウィンドウのキャラクタ領域を開放
 *
 * @param	frmnum		BGフレーム
 * @param tInfo     GFL_ARCUTIL_TRANSINFO
 */
//--------------------------------------------------------------
extern void BmpWinFrame_GraphicFreeAreaMan( u8 frmnum, GFL_ARCUTIL_TRANSINFO tInfo );

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
 * ウィンドウを描画(AreaManを使った時用のユーティリティ)
 *
 * @param	win			  BMPウィンドウデータ
 * @param	trans_sw	転送スイッチ
 * @param	tInfo		  GFL_ARCUTIL_TRANSINFO
 * @param	pal			  パレット
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BmpWinFrame_WriteAreaMan( GFL_BMPWIN *win, u8 trans_sw, GFL_ARCUTIL_TRANSINFO tInfo, u8 pal );

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

#endif	// WINDOW_H
