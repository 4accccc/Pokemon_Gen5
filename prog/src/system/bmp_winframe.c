//======================================================================
/**
 * @file	bmp_winframe.c
 * @brief	ウィンドウフレーム表示
 * @author	Hiroyuki Nakamura
 * @date	2005.10.13
 *	・2008.09 DPからWBへ移植
 */
//======================================================================
#include <gflib.h>

#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/bmp_winframe.h"

#include "winframe.naix"	//test

//======================================================================
//	定数定義
//======================================================================
#define	MAKE_CURSOR_CGX_SIZ	( 0x20*4*3 )	// 送りカーソルサイズ

// 看板ウィンドウマップ開始アーカイブID
#define	BOARD_TOWNMAP_START	( NARC_field_board_board_town_dmy_NCGR )	// 街
#define	BOARD_ROADMAP_START	( NARC_field_board_board_road000_NCGR )		// 道

//======================================================================
//	プロトタイプ宣言
//======================================================================

//======================================================================
//	メニューウィンドウ
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
void BmpWinFrame_CgxSet( u8 frmnum, u16 cgx, u8 win_num, HEAPID heap )
{
	if( win_num == MENU_TYPE_SYSTEM ){
		GFL_ARC_UTIL_TransVramBgCharacter(
			ARCID_FLDMAP_WINFRAME, NARC_winframe_system_NCGR,
			frmnum, cgx, 0, 0, heap );
	}else{
		GFL_ARC_UTIL_TransVramBgCharacter(
			ARCID_FLDMAP_WINFRAME, NARC_winframe_fmenu_NCGR,
			frmnum, cgx, 0, 0, heap );
	}
}

//--------------------------------------------------------------
/**
 * メニューウィンドウパレットのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------
u32 BmpWinFrame_WinPalArcGet(void)
{
	return ( NARC_winframe_system_NCLR );
}

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
void BmpWinFrame_GraphicSet(
	u8 frmnum, u16 cgx, u8 pal, u8 win_num, HEAPID heap )
{
	u32	arc;
	
	// キャラ
	if( win_num == MENU_TYPE_SYSTEM ){
		arc = NARC_winframe_system_NCGR;
	}else{
		arc = NARC_winframe_fmenu_NCGR;
	}

	GFL_ARC_UTIL_TransVramBgCharacter(
		ARCID_FLDMAP_WINFRAME, arc, frmnum, cgx, 0, 0, heap );
	
	// パレット
	if( win_num == MENU_TYPE_UG ){
//		arc = NARC_winframe_ugmenu_win_NCLR;
		arc = NARC_winframe_system_NCLR;
	}else{
		arc = NARC_winframe_system_NCLR;
	}

	if( frmnum < GFL_BG_FRAME0_S ){
		GFL_ARC_UTIL_TransVramPalette( ARCID_FLDMAP_WINFRAME,
			arc, PALTYPE_MAIN_BG, pal*0x20, 0x20, heap );
	}else{
		GFL_ARC_UTIL_TransVramPalette( ARCID_FLDMAP_WINFRAME,
			arc, PALTYPE_SUB_BG, pal*0x20, 0x20, heap );
	}
}


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
GFL_ARCUTIL_TRANSINFO BmpWinFrame_GraphicSetAreaMan( u8 frmnum, u8 pal, u8 win_num, HEAPID heap )
{
	u32	arc,pos;
    GFL_ARCUTIL_TRANSINFO bgchar;
	
	// キャラ
	if( win_num == MENU_TYPE_SYSTEM ){
		arc = NARC_winframe_system_NCGR;
	}else{
		arc = NARC_winframe_fmenu_NCGR;
	}

	bgchar = GFL_ARC_UTIL_TransVramBgCharacterAreaMan(
        ARCID_FLDMAP_WINFRAME, arc, frmnum, 0, 0, heap );
	
	// パレット
	if( win_num == MENU_TYPE_UG ){
//		arc = NARC_winframe_ugmenu_win_NCLR;
		arc = NARC_winframe_system_NCLR;
	}else{
		arc = NARC_winframe_system_NCLR;
	}

	if( frmnum < GFL_BG_FRAME0_S ){
		GFL_ARC_UTIL_TransVramPalette( ARCID_FLDMAP_WINFRAME,
			arc, PALTYPE_MAIN_BG, pal*0x20, 0x20, heap );
	}else{
		GFL_ARC_UTIL_TransVramPalette( ARCID_FLDMAP_WINFRAME,
			arc, PALTYPE_SUB_BG, pal*0x20, 0x20, heap );
	}
    return bgchar;
}

//--------------------------------------------------------------
/**
 *	メニューウインドウ描画メイン
 *
 * @param	frm			BGフレームナンバー(bg_sys)
 * @param	px			Ｘ座標
 * @param	py			Ｙ座標
 * @param	sx			Ｘサイズ
 * @param	sy			サイズ
 * @param	pal			使用パレット
 *
 * @return	none
 */
//--------------------------------------------------------------
static void BmpMenuWinWriteMain(
	u8 frm, u8 px, u8 py, u8 sx, u8 sy, u8 pal, u16 cgx )
{
	GFL_BG_FillScreen( frm, cgx,   px-1,  py-1, 1,  1, pal );
	GFL_BG_FillScreen( frm, cgx+1, px,    py-1, sx, 1, pal );
	GFL_BG_FillScreen( frm, cgx+2, px+sx, py-1, 1,  1, pal );
	
	GFL_BG_FillScreen( frm, cgx+3, px-1,  py, 1, sy, pal );
	GFL_BG_FillScreen( frm, cgx+5, px+sx, py, 1, sy, pal );

	GFL_BG_FillScreen( frm, cgx+6, px-1,  py+sy, 1,  1, pal );
	GFL_BG_FillScreen( frm, cgx+7, px,    py+sy, sx, 1, pal );
	GFL_BG_FillScreen( frm, cgx+8, px+sx, py+sy, 1,  1, pal );
}

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
void BmpWinFrame_Write( GFL_BMPWIN *win, u8 trans_sw, u16 win_cgx, u8 pal )
{
	u8	frm = GFL_BMPWIN_GetFrame( win );
	
	BmpMenuWinWriteMain(
		frm, 
		GFL_BMPWIN_GetPosX( win ),
		GFL_BMPWIN_GetPosY( win ),
		GFL_BMPWIN_GetSizeX( win ),
		GFL_BMPWIN_GetSizeY( win ),
		pal, win_cgx );
	
	if( trans_sw == WINDOW_TRANS_ON ){
		GFL_BG_LoadScreenReq( frm );
	}
	else if(trans_sw == WINDOW_TRANS_ON_V){
		GFL_BG_LoadScreenV_Req( frm );
	}
}

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
void BmpWinFrame_Clear( GFL_BMPWIN *win, u8 trans_sw )
{
	u8	frm = GFL_BMPWIN_GetFrame( win );
	
	GFL_BG_FillScreen(
		frm, 0,
		GFL_BMPWIN_GetPosX( win ) - 1,
		GFL_BMPWIN_GetPosY( win ) - 1,
		GFL_BMPWIN_GetSizeX( win ) + 2,
		GFL_BMPWIN_GetSizeY( win ) + 2,
		0 );

	if( trans_sw == WINDOW_TRANS_ON ){
//		GFL_BG_ClearScreen( frm );
		GFL_BG_LoadScreenReq( frm );
	}
	else if(trans_sw == WINDOW_TRANS_ON_V){
		GFL_BG_LoadScreenV_Req( frm );
	}
}

//--------------------------------------------------------------
/**
 * @brief   BMPWINスクリーン転送
 *
 * @param	win			BMPウィンドウデータ
 * @param   trans_sw		転送スイッチ
 */
//--------------------------------------------------------------
void BmpWinFrame_TransScreen(GFL_BMPWIN *win, u8 trans_sw)
{
	u8	frm = GFL_BMPWIN_GetFrame( win );

	if( trans_sw == WINDOW_TRANS_ON ){
		GFL_BG_LoadScreenReq( frm );
	}
	else if(trans_sw == WINDOW_TRANS_ON_V){
		GFL_BG_LoadScreenV_Req( frm );
	}
}

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
void TalkWinFrame_GraphicSet(u8 frmnum, u16 cgx, u8 pal, u8 win_num, HEAPID heap )
{
	BmpWinFrame_GraphicSet(frmnum, cgx, pal, 0, heap);
}

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
GFL_ARCUTIL_TRANSINFO TalkWinFrame_GraphicSetAreaMan( u8 frmnum, u8 pal, u8 win_num, HEAPID heap )
{
	return BmpWinFrame_GraphicSetAreaMan(frmnum, pal, 0, heap);
}

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
void TalkWinFrame_Write( GFL_BMPWIN *win, u8 trans_sw, u16 win_cgx, u8 pal )
{
	BmpWinFrame_Write(win, trans_sw, win_cgx, pal);
}

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
void TalkWinFrame_Clear( GFL_BMPWIN *win, u8 trans_sw )
{
	BmpWinFrame_Clear(win, trans_sw);
}

//--------------------------------------------------------------
/**
 * 看板ウィンドウのグラフィックをセット
 *
 * @param	ini			BGLデータ
 * @param	frmnum		BGフレーム
 * @param	cgx			キャラ転送位置
 * @param	pal			パレット番号
 * @param	type		看板タイプ
 * @param	map			マップ番号（タウンマップ、標識）
 * @param	heap		ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------
void BoardWinFrame_GraphicSet(u8 frmnum, u16 cgx, u8 pal, u8 type, u16 map, HEAPID heap )
{
	BmpWinFrame_GraphicSet(frmnum, cgx, pal, 0, heap);
}

//--------------------------------------------------------------
/**
 * エリアマネージャーを使って看板ウィンドウのグラフィックをセット
 *
 * @param	ini			BGLデータ
 * @param	frmnum		BGフレーム
 * @param	pal			パレット番号
 * @param	type		看板タイプ
 * @param	map			マップ番号（タウンマップ、標識）
 * @param	heap		ヒープID
 *
 * @return	GFL_ARCUTIL_TRANSINFO
 */
//--------------------------------------------------------------
GFL_ARCUTIL_TRANSINFO BoardWinFrame_GraphicSetAreaMan( u8 frmnum, u8 pal, u8 type, u16 map, HEAPID heap )
{
	return BmpWinFrame_GraphicSetAreaMan(frmnum, pal, 0, heap);
}

//--------------------------------------------------------------
/**
 * 看板ウィンドウを描画
 *
 * @param	win			BMPウィンドウデータ
 * @param	trans_sw	転送スイッチ
 * @param	win_cgx		ウィンドウキャラ位置
 * @param	pal			パレット
 * @param	type		看板タイプ
 *
 * @return	none
 */
//--------------------------------------------------------------
void BoardWinFrame_Write( GFL_BMPWIN *win, u8 trans_sw, u16 win_cgx, u8 pal, u8 type )
{
	BmpWinFrame_Write(win, trans_sw, win_cgx, pal);
}

//--------------------------------------------------------------
/**
 *	看板ウィンドウをクリア
 *
 * @param	win			BMPウィンドウデータ
 * @param	type		看板タイプ
 * @param	trans_sw	転送スイッチ
 *
 * @return	none
 */
//--------------------------------------------------------------
void BoardWinFrame_Clear( GFL_BMPWIN *win, u8 type, u8 trans_sw )
{
	BmpWinFrame_Clear(win, trans_sw);
}
