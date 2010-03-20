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
 * ウィンドウキャラクタデータのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------
u32 BmpWinFrame_CgxArcGet( MENU_TYPE win_type )
{
  static const u8 arc_tbl[] = {
    NARC_winframe_system_NCGR,
    NARC_winframe_talk_NCGR,
  };
  GF_ASSERT(win_type < MENU_TYPE_MAX);
	return arc_tbl[win_type];
}

//--------------------------------------------------------------
/**
 * ウィンドウパレットのアーカイブインデックス取得
 *
 * @param	none
 *
 * @return	アーカイブインデックス
 */
//--------------------------------------------------------------
u32 BmpWinFrame_WinPalArcGet( MENU_TYPE win_type )
{
  static const u8 arc_tbl[] = {
    NARC_winframe_system_NCLR,
    NARC_winframe_balloonwin_NCLR,  //フィールド会話系ウィンドウballoonwin_ncgと共通のパレット
  };
  GF_ASSERT(win_type < MENU_TYPE_MAX);
	return arc_tbl[win_type];
}

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
void BmpWinFrame_CgxSet( u8 frmnum, u16 cgx, MENU_TYPE win_type, HEAPID heap )
{
  u32 arc = BmpWinFrame_CgxArcGet( win_type );

	GFL_ARC_UTIL_TransVramBgCharacter(
	  ARCID_FLDMAP_WINFRAME, arc,
		frmnum, cgx, 0, 0, heap );
}

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
void BmpWinFrame_PalSet( u8 frmnum, u8 pal, MENU_TYPE win_type, HEAPID heap )
{
  u32  arc = BmpWinFrame_WinPalArcGet( win_type );

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
void BmpWinFrame_GraphicSet(
	u8 frmnum, u16 cgx, u8 pal, MENU_TYPE win_type, HEAPID heap )
{
	// キャラ
  BmpWinFrame_CgxSet( frmnum, cgx, win_type, heap );
	// パレット
  BmpWinFrame_PalSet( frmnum, pal, win_type, heap );
}


//--------------------------------------------------------------
/**
 * エリアマネージャーを使ってウィンドウのグラフィックをセット
 *
 * @param	frmnum		BGフレーム
 * @param	pal			  パレット番号
 * @param	win_type	ウィンドウ番号 MENU_TYPE型
 * @param	heap		  ヒープID
 *
 * @return	GFL_ARCUTIL_TRANSINFO
 */
//--------------------------------------------------------------
GFL_ARCUTIL_TRANSINFO BmpWinFrame_GraphicSetAreaMan( u8 frmnum, u8 pal, MENU_TYPE win_type, HEAPID heap )
{
	u32	arc,pos;
  GFL_ARCUTIL_TRANSINFO bgchar;
	
	// キャラ
  arc = BmpWinFrame_CgxArcGet( win_type );

	bgchar = GFL_ARC_UTIL_TransVramBgCharacterAreaMan(
        ARCID_FLDMAP_WINFRAME, arc, frmnum, 0, 0, heap );
	
	// パレット
  BmpWinFrame_PalSet( frmnum, pal, win_type, heap );
  
  return bgchar;
}

//--------------------------------------------------------------
/**
 * エリアマネージャーを使ってセットしたウィンドウのキャラクタ領域を開放
 *
 * @param	frmnum		BGフレーム
 * @param tInfo     GFL_ARCUTIL_TRANSINFO
 */
//--------------------------------------------------------------
void BmpWinFrame_GraphicFreeAreaMan( u8 frmnum, GFL_ARCUTIL_TRANSINFO tInfo )
{
  GFL_BG_FreeCharacterArea( frmnum,
                            GFL_ARCUTIL_TRANSINFO_GetPos(tInfo),
                            GFL_ARCUTIL_TRANSINFO_GetSize(tInfo));	
}

//--------------------------------------------------------------
/**
 *	ウインドウ描画メイン
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
 * ウィンドウを描画
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
void BmpWinFrame_WriteAreaMan( GFL_BMPWIN *win, u8 trans_sw, GFL_ARCUTIL_TRANSINFO tInfo, u8 pal )
{
  BmpWinFrame_Write( win, trans_sw, GFL_ARCUTIL_TRANSINFO_GetPos( tInfo ), pal );
}

//--------------------------------------------------------------
/**
 *	ウィンドウをクリア
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

