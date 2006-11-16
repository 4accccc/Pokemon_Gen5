//=============================================================================================
/**
 * @file	bmp_win.c
 * @brief	BMPウィンドウシステム
 * @author	Hiroyuki Nakamura
 * @date	2006/10/18
 *
 *	ポケモンDPのbg_system.cから分割
 *		BGL		->	bg_sys.c
 *		BMP		->	bmp.c
 *		BMPWIN	->	bmp_win.c
 */
//=============================================================================================
#include "gflib.h"
#include "bg_sys.h"
#include "bmp.h"

#define	__BMP_H_GLOBAL__
#include "bmp_win.h"

static void BmpWinOn_Normal( GFL_BMPWIN_DATA * win );
static void BmpWinOnVReq_Normal( GFL_BMPWIN_DATA * win );
static void BmpWinOn_Affine( GFL_BMPWIN_DATA * win );
static void BmpWinOnVReq_Affine( GFL_BMPWIN_DATA * win );
static void BmpWinOff_Normal( GFL_BMPWIN_DATA * win );
static void BmpWinOffVReq_Normal( GFL_BMPWIN_DATA * win );
static void BmpWinOff_Affine( GFL_BMPWIN_DATA * win );
static void BmpWinOffVReq_Affine( GFL_BMPWIN_DATA * win );
static void BmpWinShift16( GFL_BMPWIN_DATA * win, u8 direct, u8 offset, u8 data );
static void BmpWinShift256( GFL_BMPWIN_DATA * win, u8 direct, u8 offset, u8 data );

static void MakeWinScrnText( GFL_BMPWIN_DATA * win );
static void MakeWinScrnAffine( GFL_BMPWIN_DATA * win );
static void ClearWinScrnText( GFL_BMPWIN_DATA * win );
static void ClearWinScrnAffine( GFL_BMPWIN_DATA * win );


//*------------------------------------------*/
/**   ウィンドウデータ作成＆転送関数         */
//*------------------------------------------*/
static void (* const WinOnFunc[])(GFL_BMPWIN_DATA*) = {
	BmpWinOn_Normal,		// テキスト
	BmpWinOn_Affine,		// アフィン
	BmpWinOn_Normal,		// 拡張アフィン
};
static void (* const WinOnVReqFunc[])(GFL_BMPWIN_DATA*) = {
	BmpWinOnVReq_Normal,		// テキスト
	BmpWinOnVReq_Affine,		// アフィン
	BmpWinOnVReq_Normal,		// 拡張アフィン
};
//*------------------------------------------*/
/**   ウィンドウ表示オフ＆転送関数           */
//*------------------------------------------*/
static void (* const WinOffFunc[])(GFL_BMPWIN_DATA*) = {
	BmpWinOff_Normal,
	BmpWinOff_Affine,
	BmpWinOff_Normal,
};
static void (* const WinOffVReqFunc[])(GFL_BMPWIN_DATA*) = {
	BmpWinOffVReq_Normal,
	BmpWinOffVReq_Affine,
	BmpWinOffVReq_Normal,
};

//*------------------------------------------*/
/**   ウィンドウスクリーン作成関数テーブル   */
//*------------------------------------------*/
static void (* const WinScrnMakeFunc[])(GFL_BMPWIN_DATA*) = {
	MakeWinScrnText,		// テキスト
	MakeWinScrnAffine,		// アフィン
	MakeWinScrnText,		// 拡張アフィン
};

//*------------------------------------------*/
/**   ウィンドウスクリーンクリア関数テーブル */
//*------------------------------------------*/
static void (* const WinScrnClearFunc[])(GFL_BMPWIN_DATA*) = {
	ClearWinScrnText,		// テキスト
	ClearWinScrnAffine,		// アフィン
	ClearWinScrnText,		// 拡張アフィン
};


static const u8 ScreenXElems[] =  {
	16,		// GFL_BG_SCRSIZ_128x128
	32,		// GFL_BG_SCRSIZ_256x256
	32,		// GFL_BG_SCRSIZ_256x512
	32,		// GFL_BG_SCRSIZ_512x256
	32,		// GFL_BG_SCRSIZ_512x512
	32,		// GFL_BG_SCRSIZ_1024x1024
};




//--------------------------------------------------------------------------------------------
/**
 * BMPWINデータ確保
 *
 * @param	heapID	ヒープID
 * @param	num		確保するBMPウィンドウの数
 *
 * @return	確保したデータのアドレス
 */
//--------------------------------------------------------------------------------------------
GFL_BMPWIN_DATA * GFL_BMPWIN_AllocGet( u32 heapID, u8 num )
{
	GFL_BMPWIN_DATA * wk;
	u16	i;

	wk = (GFL_BMPWIN_DATA *)sys_AllocMemory( heapID, sizeof(GFL_BMPWIN_DATA) * num );

	for( i=0; i<num; i++ ){
		GFL_BMPWIN_Init( &wk[i], heapID );
	}

	return wk;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPWINデータ初期化
 *
 * @param	wk		BMPWINデータ
 * @param	heapID	ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_Init( GFL_BMPWIN_DATA * wk, u32 heapID )
{
	wk->heapID = heapID;
	wk->frmnum = GFL_BMPWIN_FRM_NULL;
	wk->posx   = 0;
	wk->posy   = 0;
	wk->sizx   = 0;
	wk->sizy   = 0;
	wk->palnum = 0;
	wk->chrofs = 0;
	wk->chrbuf = NULL;
	wk->bitmode = GFL_BMPWIN_BITMODE_4;		// 念のため良く使う方で初期化しておく
}

//--------------------------------------------------------------------------------------------
/**
 * BMPWIN登録チェック
 *
 * @param	win		BMPWINデータ
 *
 * @retval	"TRUE = 登録済み"
 * @retval	"FALSE = 未登録"
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BMPWIN_AddCheck( GFL_BMPWIN_DATA * win )
{
	if( win->frmnum == GFL_BMPWIN_FRM_NULL || win->chrbuf == NULL ){
		return FALSE;
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPWIN追加
 *
 * @param	win		BMPWINデータ格納場所
 * @param	frmnum	BGフレーム番号
 * @param	posx	X座標（キャラ単位）
 * @param	posy	Y座標（キャラ単位）
 * @param	sizx	Xサイズ（キャラ単位）
 * @param	sizy	Xサイズ（キャラ単位）
 * @param	palnum	使用パレット番号
 * @param	chrofs	使用CGXオフセット
 * @param	heapID	ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_Add(
		GFL_BMPWIN_DATA * win, u8 frmnum,
		u8 posx, u8 posy, u8 sizx, u8 sizy, u8 palnum, u16 chrofs, u32 heapID )
{
	void * chrvbuf;
	u32	chrvsiz;

	// 使用フレーム状況の判定（スクリーンバッファ確保されているか）
	if( GFL_BG_ScreenAdrsGet( win->bgl, frmnum ) == NULL ){
#ifdef	OSP_ERR_BGL_BMPADD		// BMP登録失敗
		OS_Printf( "ＢＭＰ登録失敗\n" );
#endif	// OSP_ERR_BGL_BMPADD
		return;
	}

	// キャラクタバッファの確保
	chrvsiz = sizx * sizy * GFL_BG_BaseCharSizeGet( win->bgl, frmnum);
	chrvbuf = sys_AllocMemory( heapID, chrvsiz );

	if( chrvbuf == NULL ){
#ifdef	OSP_ERR_BGL_BMPADD		// BMP登録失敗
		OS_Printf( "ＢＭＰ領域確保失敗\n" );
#endif	// OSP_ERR_BGL_BMPADD
		return;
	}

	// ビットマップの設定
	win->heapID = heapID;
	win->frmnum = frmnum;
	win->posx   = posx;
	win->posy   = posy;
	win->sizx   = sizx;
	win->sizy   = sizy;
	win->palnum = palnum;
	win->chrofs = chrofs;
	win->chrbuf = chrvbuf;
	win->bitmode = (GFL_BG_ScreenColorModeGet( win->bgl, frmnum) == GX_BG_COLORMODE_16)? GFL_BMPWIN_BITMODE_4 : GFL_BMPWIN_BITMODE_8;

#ifdef	OSP_BGL_BMP_SIZ		// BMPサイズ
	OS_Printf( "ＢＭＰＣＧＸ領域=%x\n", chrvbuf );
#endif	// OSP_BGL_BMP_SIZ
}

//--------------------------------------------------------------------------------------------
/**
 * BMPWIN追加（OBJ用にキャラ領域だけを確保。4bit専用。）
 *
 * @param	win			BMPWINデータ格納場所
 * @param	sizx		Xサイズ（キャラ単位）
 * @param	sizy		Xサイズ（キャラ単位）
 * @param	chrofs		使用CGXオフセット
 * @param   fill_color	埋め尽くしカラー番号(0～0xf)
 * @param	heapID		ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_AddChar(
		GFL_BMPWIN_DATA * win, u8 sizx, u8 sizy, u16 chrofs, u8 fill_color, u32 heapID )
{
	void * chrvbuf;
	u32	chrvsiz;

	chrvsiz	= (u32)( sizx * sizy * GFL_BG_1CHRDATASIZ );
	chrvbuf = sys_AllocMemory( heapID, chrvsiz );

	fill_color |= fill_color << 4;
	memset( chrvbuf, fill_color, chrvsiz );

	if( chrvbuf == NULL ){
#ifdef	OSP_ERR_BGL_BMPADD		// BMP登録失敗
		OS_Printf( "BMPOBJ領域確保失敗\n" );
#endif	// OSP_ERR_BGL_BMPADD
		return;
	}

	// ビットマップの設定
	win->heapID = heapID;
	win->sizx   = sizx;
	win->sizy   = sizy;
	win->chrofs = chrofs;
	win->chrbuf = chrvbuf;
	win->bitmode = GFL_BMPWIN_BITMODE_4;

#ifdef	OSP_BGL_BMP_SIZ		// BMPサイズ
	OS_Printf( "BMPOBJCGX領域=%x\n", chrvbuf );
#endif	// OSP_BGL_BMP_SIZ
}

//--------------------------------------------------------------------------------------------
/**
 * BMPWIN追加（データ指定）
 *
 * @param	win			BMPWINデータ格納場所
 * @param	dat			BMPWIN_SET構造体型データへのポインタ
 * @param	heapID		ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_AddEx( GFL_BMPWIN_DATA * win, const BMPWIN_SET * dat, u32 heapID )
{
	GFL_BMPWIN_Add(
		win, dat->frm_num, dat->pos_x, dat->pos_y,
		dat->siz_x, dat->siz_y, dat->palnum, dat->chrnum, heapID );
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ削除
 *
 * @param	win		BMPWINデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_Del( GFL_BMPWIN_DATA * win )
{
	// キャラクタバッファ割り当て開放
	sys_FreeMemoryEz( win->chrbuf );

	win->frmnum = GFL_BMPWIN_FRM_NULL;
	win->posx   = 0;
	win->posy   = 0;
	win->sizx   = 0;
	win->sizy   = 0;
	win->palnum = 0;
	win->chrofs = 0;
	win->chrbuf = NULL;
	win->heapID = 0;
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ削除（BMPデータも開放）
 *
 * @param	win		BMPWINデータ
 * @param	num		個数
 *
 * @return	none
 *
 *	GFL_BMPWIN_AllocGet()で確保した場合に使用して下さい
 *	
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_Free( GFL_BMPWIN_DATA * win, u8 num )
{
	u16	i;

	// キャラクタバッファ割り当て開放
	for( i=0; i<num; i++ ){
		if( win[i].chrbuf == NULL ){ continue; }
		sys_FreeMemoryEz( win[i].chrbuf );
	}

	// BMPデータ領域開放
	sys_FreeMemoryEz( win );
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ表示（スクリーン、キャラ転送）
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_On( GFL_BMPWIN_DATA * win )
{
	u8	mode;
	
	GF_ASSERT(win!=NULL);
	GF_ASSERT(win->frmnum<8);

	mode = GFL_BG_BgModeGet( win->bgl, win->frmnum );

	GF_ASSERT(mode < NELEMS(WinOnFunc));
	WinOnFunc[ mode ]( win );
}
//--------------------------------------------------------------------------------------------
/**
 * ビットマップ表示（スクリーン転送リクエスト、キャラ転送）
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_OnVReq( GFL_BMPWIN_DATA * win )
{
	u8	mode;

	GF_ASSERT(win!=NULL);
	GF_ASSERT(win->frmnum<8);

	mode = GFL_BG_BgModeGet( win->bgl, win->frmnum );

	GF_ASSERT(mode < NELEMS(WinOnVReqFunc));
	WinOnVReqFunc[ mode ]( win );
}
//------------------------------------------------------------------
/**
 * Window領域スクリーンを関連BG内部バッファに作成する
 *
 * @param   win		
 *
 */
//------------------------------------------------------------------
void GFL_BMPWIN_MakeScrn( GFL_BMPWIN_DATA * win )
{
	WinScrnMakeFunc[ GFL_BG_BgModeGet( win->bgl, win->frmnum ) ]( win );
}
//------------------------------------------------------------------
/**
 * Window領域スクリーンを関連BG内部バッファに作成する
 * （サイズ指定可。BitmapWindow作成時よりも小さい領域のスクリーン作成が可能。大きいのはＮＧ。）
 *
 * @param   win			BitmapWindow へのポインタ
 * @param   width		スクリーン作成領域の幅（キャラ単位）
 * @param   height		スクリーン作成領域の高さ（キャラ単位）
 *
 */
//------------------------------------------------------------------
void GFL_BMPWIN_MakeScrnLimited( GFL_BMPWIN_DATA * win, u32 width, u32 height )
{
	u32  x_save, y_save;

	x_save = win->sizx;
	y_save = win->sizy;
	win->sizx = width;
	win->sizy = height;

	WinScrnMakeFunc[ GFL_BG_BgModeGet( win->bgl, win->frmnum ) ]( win );

	win->sizx = x_save;
	win->sizy = y_save;

}
//------------------------------------------------------------------
/**
 * 関連BG内部バッファのWindow領域スクリーンをクリアする
 *
 * @param   win		
 *
 */
//------------------------------------------------------------------
void GFL_BMPWIN_ClearScrn( GFL_BMPWIN_DATA * win )
{
	WinScrnClearFunc[ GFL_BG_BgModeGet( win->bgl, win->frmnum) ]( win );
}

//------------------------------------------------------------------
/**
 * スクリーン作成（テキスト・拡張アフィン用）
 *
 * @param   win		
 *
 */
//------------------------------------------------------------------
static void MakeWinScrnText( GFL_BMPWIN_DATA * win )
{
	u16 * scrn = (u16 *)GFL_BG_ScreenAdrsGet( win->bgl, win->frmnum );

	if( scrn != NULL ){
		u32 x, y, x_max, y_max, chr, p, xelems;

//		xelems = ScreenXElems[ GFL_BG_ScreenTypeGet(win->frmnum) ];
		xelems = 32;

		chr = win->chrofs;

		x_max = win->posx + win->sizx;
		y_max = win->posy + win->sizy;
		for(y = win->posy; y < y_max; y++){
			for(x = win->posx; x < x_max; x++){
				p = ((y&0x20)*32) + ((x&0x20)*32) + ((y&0x1f)*xelems) + (x&0x1f);
				scrn[p] = chr | (win->palnum << 12);
				chr++;
			}
		}
	}
}
//------------------------------------------------------------------
/**
 * スクリーン作成（アフィンBG用）
 *
 * @param   win		
 *
 */
//------------------------------------------------------------------
static void MakeWinScrnAffine( GFL_BMPWIN_DATA * win )
{
	u8 * scrn = (u8 *)GFL_BG_ScreenAdrsGet( win->bgl, win->frmnum );

	if( scrn != NULL ){
		int x, y, chr, xelems;

		xelems = ScreenXElems[ GFL_BG_ScreenTypeGet( win->bgl, win->frmnum) ];

		scrn = scrn + win->posy * xelems + win->posx;

		chr = win->chrofs;

		for( y = 0; y < win->sizy; y++ ){
			for( x = 0; x < win->sizx; x++ ){
				scrn[x] = chr++;
			}
			scrn += xelems;
		}
	}
}
//------------------------------------------------------------------
/**
 * スクリーンクリア（テキスト・拡張アフィン用）
 *
 * @param   win		
 *
 */
//------------------------------------------------------------------
static void ClearWinScrnText( GFL_BMPWIN_DATA * win )
{
	u16 * scrn = (u16 *)GFL_BG_ScreenAdrsGet( win->bgl, win->frmnum );

	if( scrn != NULL ){
		u32 x, y, x_max, y_max, p, xelems;

		xelems = ScreenXElems[ GFL_BG_ScreenTypeGet( win->bgl, win->frmnum ) ];

		x_max = win->posx + win->sizx;
		y_max = win->posy + win->sizy;
		for(y = win->posy; y < y_max; y++){
			for(x = win->posx; x < x_max; x++){
				p = ((y&0x20)*32) + ((x&0x20)*32) + ((y&0x1f)*xelems) + (x&0x1f);
				scrn[p] = 0x0000;
			}
		}
	}
}
//------------------------------------------------------------------
/**
 * スクリーンクリア（アフィンBG用）
 *
 * @param   win		
 *
 */
//------------------------------------------------------------------
static void ClearWinScrnAffine( GFL_BMPWIN_DATA * win )
{
	u8 * scrn = (u8 *)GFL_BG_ScreenAdrsGet( win->bgl, win->frmnum );

	if( scrn != NULL ){
		int x, y, xelems;

		xelems = ScreenXElems[ GFL_BG_ScreenTypeGet( win->bgl, win->frmnum) ];

		scrn = scrn + win->posy * xelems + win->posx;

		for( y = 0; y < win->sizy; y++ ){
			for( x = 0; x < win->sizx; x++ ){
				scrn[x] = 0;
			}
			scrn += xelems;
		}
	}

}
//--------------------------------------------------------------------------------------------
/**
 * ビットマップ表示（スクリーン、キャラ転送（テキスト、アフィン拡張BG））
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BmpWinOn_Normal( GFL_BMPWIN_DATA * win )
{
	MakeWinScrnText( win );

	GFL_BG_LoadScreen(
		win->bgl,
		win->frmnum,
		GFL_BG_ScreenAdrsGet( win->bgl, win->frmnum ),
		GFL_BG_ScreenSizGet( win->bgl, win->frmnum ),
		GFL_BG_ScreenOfsGet( win->bgl, win->frmnum ) );

	GFL_BMPWIN_CgxOn( win );
}
//--------------------------------------------------------------------------------------------
/**
 * ビットマップ表示（スクリーン転送リクエスト、キャラ転送（テキスト、アフィン拡張BG））
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BmpWinOnVReq_Normal( GFL_BMPWIN_DATA * win )
{
	MakeWinScrnText( win );

	GFL_BG_LoadScreenV_Req( win->bgl, win->frmnum );
	GFL_BMPWIN_CgxOn( win );
}
//--------------------------------------------------------------------------------------------
/**
 * ビットマップ表示（スクリーン、キャラ転送（拡縮面））
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BmpWinOn_Affine( GFL_BMPWIN_DATA * win )
{
	MakeWinScrnAffine( win );

	GFL_BG_LoadScreen(
		win->bgl,
		win->frmnum,
		GFL_BG_ScreenAdrsGet( win->bgl, win->frmnum ),
		GFL_BG_ScreenSizGet( win->bgl, win->frmnum ),
		GFL_BG_ScreenOfsGet( win->bgl, win->frmnum ) );

	GFL_BG_LoadCharacter(
		win->bgl, win->frmnum, win->chrbuf,
		(u32)( win->sizx * win->sizy * GFL_BG_8BITCHRSIZ ), (u32)win->chrofs );
}
//--------------------------------------------------------------------------------------------
/**
 * ビットマップ表示（スクリーン転送リクエスト、キャラ転送（拡縮面））
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BmpWinOnVReq_Affine( GFL_BMPWIN_DATA * win )
{
	MakeWinScrnAffine( win );

	GFL_BG_LoadScreenV_Req( win->bgl, win->frmnum );
	GFL_BG_LoadCharacter(
		win->bgl, win->frmnum, win->chrbuf,
		(u32)( win->sizx * win->sizy * GFL_BG_8BITCHRSIZ ), (u32)win->chrofs );
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ表示（キャラ転送のみ）
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_CgxOn( GFL_BMPWIN_DATA * win )
{
	u32 chrsize = win->sizx * win->sizy * GFL_BG_BaseCharSizeGet( win->bgl, win->frmnum );

	GFL_BG_LoadCharacter( win->bgl, win->frmnum, win->chrbuf, chrsize, win->chrofs );
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ非表示（スクリーンクリア＆転送）
 *
 * @param	win		BMPデータ格納場所
 *
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_Off( GFL_BMPWIN_DATA * win )
{
	WinOffFunc[ GFL_BG_BgModeGet( win->bgl, win->frmnum ) ]( win );
}
//--------------------------------------------------------------------------------------------
/**
 * ビットマップ非表示（スクリーンクリア＆転送リクエスト）
 *
 * @param	win		BMPデータ格納場所
 *
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_OffVReq( GFL_BMPWIN_DATA * win )
{
	WinOffVReqFunc[ GFL_BG_BgModeGet( win->bgl, win->frmnum ) ]( win );
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ非表示（テキスト、アフィン拡張BG）
 *
 * @param	win
 *
 */
//--------------------------------------------------------------------------------------------
static void BmpWinOff_Normal( GFL_BMPWIN_DATA * win )
{
	ClearWinScrnText( win );
	GFL_BG_LoadScreen(
		win->bgl,
		win->frmnum,
		GFL_BG_ScreenAdrsGet( win->bgl, win->frmnum ),
		GFL_BG_ScreenSizGet( win->bgl, win->frmnum ),
		GFL_BG_ScreenOfsGet( win->bgl, win->frmnum ) );
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ非表示リクエスト（テキスト、アフィン拡張BG）
 *
 * @param	win
 *
 */
//--------------------------------------------------------------------------------------------
static void BmpWinOffVReq_Normal( GFL_BMPWIN_DATA * win )
{
	ClearWinScrnText( win );
	GFL_BG_LoadScreenV_Req( win->bgl, win->frmnum );
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ非表示（拡縮面）
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BmpWinOff_Affine( GFL_BMPWIN_DATA * win )
{
	ClearWinScrnAffine( win );
	GFL_BG_LoadScreen(
		win->bgl, 
		win->frmnum, 
		GFL_BG_ScreenAdrsGet( win->bgl, win->frmnum ),
		GFL_BG_ScreenSizGet( win->bgl, win->frmnum ),
		GFL_BG_ScreenOfsGet( win->bgl, win->frmnum ) );
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ非表示リクエスト（拡縮面）
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BmpWinOffVReq_Affine( GFL_BMPWIN_DATA * win )
{
	ClearWinScrnAffine( win );
	GFL_BG_LoadScreenV_Req( win->bgl, win->frmnum );
}

//--------------------------------------------------------------------------------------------
/**
 * 指定カラーで全体を塗りつぶす
 *
 * @param	win		BMPデータ格納場所
 * @param	col		カラーナンバー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_DataFill( GFL_BMPWIN_DATA * win, u8 col )
{
	u32	datasize;
	u32 data;
	u32	base_char_siz;

	base_char_siz = GFL_BG_BaseCharSizeGet( win->bgl, win->frmnum );

	if( base_char_siz == GFL_BG_1CHRDATASIZ ){
		col = (col<<4)|col;
	}
	data = (col<<24)|(col<<16)|(col<<8)|col;

	datasize = base_char_siz * win->sizx * win->sizy;

	MI_CpuFillFast( win->chrbuf, data, datasize );
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップ描画
 *
 * @param	win		BMPデータ格納場所
 * @param	src		ビットマップ表示データポインタ
 * @param	src_x	ビットマップ表示開始位置Ｘ
 * @param	src_y	ビットマップ表示開始位置Ｙ
 * @param	src_dx	ビットマップデータサイズＸ
 * @param	src_dy	ビットマップデータサイズＸ
 * @param	win_x	ウインドウ表示開始位置Ｘ
 * @param	win_y	ウインドウ表示開始位置Ｙ
 * @param	win_dx	ウインドウデータサイズＸ
 * @param	win_dy	ウインドウデータサイズＹ
 *
 * @retval	なし
 *
 * ビットマップデータの書き込みを実行
 * ビットマップデータの切り取りを可能にしたもの
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_Print(
			GFL_BMPWIN_DATA * win, void * src,
			u16 src_x, u16 src_y, u16 src_dx, u16 src_dy,
			u16 win_x, u16 win_y, u16 win_dx, u16 win_dy )
{
	GFL_BMPWIN_PrintEx(
		win, src, src_x, src_y, src_dx, src_dy, win_x, win_y, win_dx, win_dy, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 *	ビットマップ描画（拡張版）
 *
 * @param	win		BMPデータ格納場所
 * @param	src		ビットマップ表示データポインタ
 * @param	src_x	ビットマップ表示開始位置Ｘ
 * @param	src_y	ビットマップ表示開始位置Ｙ
 * @param	src_dx	ビットマップデータサイズＸ
 * @param	src_dy	ビットマップデータサイズＸ
 * @param	win_x	ウインドウ表示開始位置Ｘ
 * @param	win_y	ウインドウ表示開始位置Ｙ
 * @param	win_dx	ウインドウデータサイズＸ
 * @param	win_dy	ウインドウデータサイズＹ
 * @param	nuki	透明色指定（0～15 0xff:透明色指定なし）
 *
 * @retval	なし
 *
 * ビットマップデータの書き込みを実行
 * ビットマップデータの切り取りを可能にしたもの
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_PrintEx(
		GFL_BMPWIN_DATA * win, void * src,
		u16 src_x, u16 src_y, u16 src_dx, u16 src_dy,
		u16 win_x, u16 win_y, u16 win_dx, u16 win_dy, u16 nuki )
{
	BMPPRT_HEADER	src_data;
	BMPPRT_HEADER	dst_data;

	src_data.adrs	= (u8 *)src;
	src_data.size_x = src_dx;
	src_data.size_y = src_dy;

	dst_data.adrs	= (u8 *)win->chrbuf;
	dst_data.size_x = (u16)(win->sizx * GFL_BG_1CHRDOTSIZ);
	dst_data.size_y = (u16)(win->sizy * GFL_BG_1CHRDOTSIZ);

	if( GFL_BG_ScreenColorModeGet( win->bgl, win->frmnum ) == GX_BG_COLORMODE_16 ){
		GFL_BMP_PrintMain(
			&src_data, &dst_data, src_x, src_y, win_x, win_y, win_dx, win_dy, nuki );
	}else{
		GFL_BMP_PrintMain256(
			&src_data, &dst_data, src_x, src_y, win_x, win_y, win_dx, win_dy, nuki );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 指定範囲を塗りつぶす
 *
 * @param	win			BMPデータ
 * @param	col			塗りつぶしカラー
 * @param	px			開始X座標
 * @param	py			開始Y座標
 * @param	sx			Xサイズ
 * @param	sy			Yサイズ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_Fill( GFL_BMPWIN_DATA * win, u8 col, u16 px, u16 py, u16 sx, u16 sy )
{
	BMPPRT_HEADER	dst_data;

	dst_data.adrs	= (u8 *)win->chrbuf;
	dst_data.size_x = (u16)(win->sizx * GFL_BG_1CHRDOTSIZ);
	dst_data.size_y = (u16)(win->sizy * GFL_BG_1CHRDOTSIZ);

	if( GFL_BG_ScreenColorModeGet( win->bgl, win->frmnum ) == GX_BG_COLORMODE_16 ){
		GFL_BMP_Fill( (const BMPPRT_HEADER *)&dst_data, px, py, sx, sy, col );
	}else{
		GFL_BMP_Fill256( (const BMPPRT_HEADER *)&dst_data, px, py, sx, sy, col );
	}
}

//---------------------------------------------------------------------------------------------
/**
 * 文字表示専用処理効率アップ版
 *
 * @param	win		BMPデータ格納場所
 * @param	src		キャラデータ
 * @param	ssx		フォントXサイズ
 * @param	ssy		フォントYサイズ
 * @param	wx		書き込みX座標
 * @param	wy		書き込みY座標
 *
 * @return	none
 */
//---------------------------------------------------------------------------------------------
#define	M_NULLPAL_L	(0)
#define	M_NULLPAL_H	(NULLPAL_L << 4)
#define	M_BMPWIN_CHR_DOT	(8)

#define M_SRC_ADRS	(src)
#define M_SRC_POSX	(0)
#define M_SRC_POSY	(0)
#define M_SRC_SIZX	(16)
#define M_SRC_SIZY	(16)
#define	M_SRC_XARG	(((M_SRC_SIZX) + (M_SRC_SIZX & 7))>>3)

#define M_DST_ADRS	(dst)
#define M_DST_POSX	(wx)
#define M_DST_POSY	(wy)
#define M_DST_SIZX	(dsx)
#define M_DST_SIZY	(dsy)
#define	M_DST_XARG	(((M_DST_SIZX) + (M_DST_SIZX & 7))>>3)

#define M_WRT_SIZX	(ssx)
#define M_WRT_SIZY	(ssy)

#define DOT_PUT(xofs,yofs,xsiz,ysiz,dadrs,dpx,dpy,dxarg)				\
	{																	\
		int	sx, dx, sy, dy, shiftval;									\
		u8	src_dat;													\
		u8	*srcadrs, *destadrs;										\
		u32 src_base;													\
																		\
		srcadrs	= M_SRC_ADRS + (yofs / 8 * 0x40) + (xofs / 8 * 0x20);	\
		for(sy=0, dy=dpy+yofs; sy < ysiz; sy++, dy++){					\
			src_base = *(u32*)srcadrs;									\
			for(sx=0, dx=dpx+xofs; sx < xsiz; sx++, dx++){				\
				destadrs = DPPCALC(dadrs, dx, dy, dxarg);				\
																		\
				src_dat = (u8)((src_base >> (sx * 4)) & 0x0f);			\
																		\
				if(src_dat != M_NULLPAL_L){								\
					shiftval = (dx & 1)*4;								\
					*destadrs = (u8)((src_dat << shiftval)|(*destadrs & (0xf0 >> shiftval)));\
				}														\
			}															\
			srcadrs	+= 4;												\
		}																\
	}

#define DOT_PUT_256(xofs,yofs,xsiz,ysiz,dadrs,dpx,dpy,dxarg)			\
	{																	\
		int	sx, dx, sy, dy;												\
		u8	src_dat;													\
		u8	*srcadrs, *destadrs;										\
		u32 src_base;													\
																		\
		srcadrs	= M_SRC_ADRS + (yofs / 8 * 0x40) + (xofs / 8 * 0x40);	\
		for( sy=0, dy=dpy+yofs; sy<ysiz; sy++, dy++ ){					\
			src_base = *(u32*)srcadrs;									\
			for( sx=0, dx=dpx+xofs; sx<xsiz; sx++, dx++ ){				\
				destadrs = DPPCALC_256(dadrs, dx, dy, dxarg);			\
																		\
				src_dat = (u8)((src_base >> (sx * 4)) & 0x0f);			\
																		\
				if( src_dat != M_NULLPAL_L ){							\
					*destadrs = src_dat;								\
				}														\
			}															\
			srcadrs	+= 4;												\
		}																\
	}

void GFL_BMPWIN_PrintMsg( GFL_BMPWIN_DATA * win, u8 * src, u16 ssx, u16 ssy, u16 wx, u16 wy )
{
	u8 * dst; 
	u16	dsx, dsy;
	int	x_max, y_max;
	u8	print_pat;

	dst	= (u8 *)win->chrbuf;
	dsx = (u16)(win->sizx * GFL_BG_1CHRDOTSIZ);
	dsy = (u16)(win->sizy * GFL_BG_1CHRDOTSIZ);

	if((M_DST_SIZX - M_DST_POSX) < M_WRT_SIZX){
		x_max = M_DST_SIZX - M_DST_POSX;
	}else{
		x_max = M_WRT_SIZX;
	}

	if((M_DST_SIZY - M_DST_POSY) < M_WRT_SIZY){
		y_max = M_DST_SIZY - M_DST_POSY;
	}else{
		y_max = M_WRT_SIZY;
	}
	print_pat = 0;

	if( x_max > 8 ){ print_pat |= 1; }
	if( y_max > 8 ){ print_pat |= 2; }

	if( win->bitmode == GFL_BMPWIN_BITMODE_4 ){
		switch( print_pat ){
		case 0:	//１キャラ(8x8)
			DOT_PUT( 0, 0, x_max,	y_max,	 M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG );
			return;
		case 1:	//２キャラ(16x8)
			DOT_PUT( 0, 0, 8,		y_max,	 M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG );
			DOT_PUT( 8, 0, x_max-8,	y_max,	 M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG );
			return;
		case 2:	//２キャラ(8x16)
			DOT_PUT( 0, 0, x_max,	8,		 M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG );
			DOT_PUT( 0, 8, x_max,	y_max-8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG );
			return;
		case 3:	//４キャラ(16x16)
			DOT_PUT( 0, 0, 8,		8,		 M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG );
			DOT_PUT( 8, 0, x_max-8,	8,		 M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG );
			DOT_PUT( 0, 8, 8,		y_max-8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG );
			DOT_PUT( 8, 8, x_max-8,	y_max-8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG );
			return;
		}
	}else{
		switch( print_pat ){
		case 0:	//１キャラ(8x8)
			DOT_PUT_256( 0, 0, x_max,	y_max,	 M_DST_ADRS,M_DST_POSX,M_DST_POSY,M_DST_XARG);
			return;
		case 1:	//２キャラ(16x8)
			DOT_PUT_256( 0, 0, 8,		y_max,	 M_DST_ADRS,M_DST_POSX,M_DST_POSY,M_DST_XARG);
			DOT_PUT_256( 8, 0, x_max-8,	y_max,	 M_DST_ADRS,M_DST_POSX,M_DST_POSY,M_DST_XARG);
			return;
		case 2:	//２キャラ(8x16)
			DOT_PUT_256( 0, 0, x_max,	8,		 M_DST_ADRS,M_DST_POSX,M_DST_POSY,M_DST_XARG);
			DOT_PUT_256( 0, 8, x_max,	y_max-8, M_DST_ADRS,M_DST_POSX,M_DST_POSY,M_DST_XARG);
			return;
		case 3:	//４キャラ(16x16)
			DOT_PUT_256( 0, 0, 8,		8,		 M_DST_ADRS,M_DST_POSX,M_DST_POSY,M_DST_XARG);
			DOT_PUT_256( 8, 0, x_max-8,	8,		 M_DST_ADRS,M_DST_POSX,M_DST_POSY,M_DST_XARG);
			DOT_PUT_256( 0, 8, 8,		y_max-8, M_DST_ADRS,M_DST_POSX,M_DST_POSY,M_DST_XARG);
			DOT_PUT_256( 8, 8, x_max-8,	y_max-8, M_DST_ADRS,M_DST_POSX,M_DST_POSY,M_DST_XARG);
			return;
		}
	}
}


//=============================================================================================
//=============================================================================================
#define	M_PUT_TBL	( tbl )

#define DOT_PUT_W(xofs,yofs,xsiz,ysiz,dadrs,dpx,dpy,dxarg,ptbl)			\
	{																	\
		int	sx, dx, sy, dy, shiftval;									\
		u8	src_dat;													\
		u8	tbl_flg;													\
		u8	tbl_bit;													\
		u8	*destadrs;													\
		const u8* srcadrs;												\
		u32 src_base;													\
																		\
		srcadrs	= M_SRC_ADRS + (yofs / 8 * 0x40) + (xofs / 8 * 0x20);	\
																		\
		if( yofs == 0 ){												\
			dy = dpy + yofs;											\
			tbl_bit = (u8)( ptbl & 0xff );								\
		}else{															\
			dy = dpy + yofs;											\
			for( sy=0; sy<8; sy++ ){									\
				if( ( (ptbl>>sy) & 1 ) != 0 ){ dy++; }					\
			}															\
			tbl_bit = (u8)( ptbl >> 8 );								\
		}																\
																		\
		for( sy=0; sy<ysiz; sy++ ){										\
			src_base = *(u32*)srcadrs;									\
			tbl_flg = (u8)(( tbl_bit >> sy ) & 1);						\
			for(sx=0, dx=dpx+xofs; sx < xsiz; sx++, dx++){				\
				destadrs = DPPCALC(dadrs, dx, dy, dxarg);				\
																		\
				src_dat = (u8)((src_base >> (sx * 4)) & 0x0f);			\
																		\
				if(src_dat != M_NULLPAL_L){								\
					shiftval = (dx & 1)*4;								\
					src_dat = (u8)((src_dat << shiftval)|(*destadrs & (0xf0 >> shiftval)));\
					*destadrs = src_dat;								\
																		\
					if( tbl_flg != 0 ){									\
						destadrs = DPPCALC(dadrs, dx, (dy+1), dxarg);	\
						*destadrs = src_dat;							\
					}													\
				}														\
			}															\
			if( tbl_flg != 0 ){											\
				dy += 2;												\
			}else{														\
				dy++;													\
			}															\
			srcadrs	+= 4;												\
		}																\
	}
/*
#define DOT_PUT_W_256(xofs,yofs,xsiz,ysiz,dadrs,dpx,dpy,dxarg,ptbl)		\
	{																	\
		int	sx, dx, sy, dy, shiftval;									\
		u8	src_dat;													\
		u8	tbl_flg;													\
		u8	tbl_bit;													\
		u8	*destadrs;													\
		const u8* srcadrs;												\
		u32 src_base;													\
																		\
		srcadrs	= M_SRC_ADRS + (yofs / 8 * 0x40) + (xofs / 8 * 0x20);	\
																		\
		if( yofs == 0 ){												\
			dy = dpy + yofs;											\
			tbl_bit = (u8)( ptbl & 0xff );								\
		}else{															\
			dy = dpy + yofs;											\
			for( sy=0; sy<8; sy++ ){									\
				if( ( (ptbl>>sy) & 1 ) != 0 ){ dy++; }					\
			}															\
			tbl_bit = (u8)( ptbl >> 8 );								\
		}																\
																		\
		for( sy=0; sy<ysiz; sy++ ){										\
			src_base = *(u32*)srcadrs;									\
			tbl_flg = (u8)(( tbl_bit >> sy ) & 1);						\
			for(sx=0, dx=dpx+xofs; sx < xsiz; sx++, dx++){				\
				destadrs = DPPCALC_256(dadrs, dx, dy, dxarg);			\
																		\
				src_dat = (u8)((src_base >> (sx * 4)) & 0x0f);			\
																		\
				if(src_dat != M_NULLPAL_L){								\
					shiftval = (dx & 1)*4;								\
					*destadrs = src_dat;								\
																		\
					if( tbl_flg != 0 ){									\
						destadrs = DPPCALC_256(dadrs,dx,(dy+1),dxarg);	\
						*destadrs = src_dat;							\
					}													\
				}														\
			}															\
			if( tbl_flg != 0 ){											\
				dy += 2;												\
			}else{														\
				dy++;													\
			}															\
			srcadrs	+= 4;												\
		}																\
	}
*/

#define CM_SRC_ADRS	(chg_src)

#define DOT_PUT_W_256(xofs,yofs,xsiz,ysiz,dadrs,dpx,dpy,dxarg,ptbl)		\
	{																	\
		int	sx, dx, sy, dy;												\
		u8	src_dat;													\
		u8	tbl_flg;													\
		u8	tbl_bit;													\
		u8	*destadrs;													\
		const u8* srcadrs;												\
		u8 * src_base;													\
																		\
		srcadrs	= CM_SRC_ADRS + (yofs / 8 * 0x80) + (xofs / 8 * 0x40);	\
																		\
		if( yofs == 0 ){												\
			dy = dpy + yofs;											\
			tbl_bit = (u8)( ptbl & 0xff );								\
		}else{															\
			dy = dpy + yofs;											\
			for( sy=0; sy<8; sy++ ){									\
				if( ( (ptbl>>sy) & 1 ) != 0 ){ dy++; }					\
			}															\
			tbl_bit = (u8)( ptbl >> 8 );								\
		}																\
																		\
		for( sy=0; sy<ysiz; sy++ ){										\
			src_base = (u8 *)srcadrs;									\
			tbl_flg = (u8)(( tbl_bit >> sy ) & 1);						\
			for(sx=0, dx=dpx+xofs; sx < xsiz; sx++, dx++){				\
				destadrs = DPPCALC_256(dadrs, dx, dy, dxarg);			\
																		\
				src_dat = src_base[sx];									\
																		\
				if(src_dat != M_NULLPAL_L){								\
					*destadrs = src_dat;								\
																		\
					if( tbl_flg != 0 ){									\
						destadrs = DPPCALC_256(dadrs,dx,(dy+1),dxarg);	\
						*destadrs = src_dat;							\
					}													\
				}														\
			}															\
			if( tbl_flg != 0 ){											\
				dy += 2;												\
			}else{														\
				dy++;													\
			}															\
			srcadrs	+= 8;												\
		}																\
	}

void GFL_BMPWIN_PrintMsgWide(
		GFL_BMPWIN_DATA * win, const u8 * src, u16 ssx, u16 ssy, u16 wx, u16 wy, u16 tbl )
{
	u8 * dst;
	u16	dsx,dsy;
	int	x_max, y_max;
	u8	print_pat;

	dst	= (u8 *)win->chrbuf;
	dsx = (u16)(win->sizx * GFL_BG_1CHRDOTSIZ);
	dsy = (u16)(win->sizy * GFL_BG_1CHRDOTSIZ);

	if((M_DST_SIZX - M_DST_POSX) < M_WRT_SIZX){
		x_max = M_DST_SIZX - M_DST_POSX;
	}else{
		x_max = M_WRT_SIZX;
	}

	if((M_DST_SIZY - M_DST_POSY) < M_WRT_SIZY){
		y_max = M_DST_SIZY - M_DST_POSY;
	}else{
		y_max = M_WRT_SIZY;
	}

	print_pat = 0;

	if( x_max > 8 ){ print_pat |= 1; }
	if( y_max > 8 ){ print_pat |= 2; }

	if( win->bitmode == GFL_BMPWIN_BITMODE_4 ){
		switch( print_pat ){
		case 0:	//１キャラ(8x8)
			DOT_PUT_W( 0, 0, x_max,	y_max, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			return;
		case 1:	//２キャラ(16x8)
			DOT_PUT_W( 0, 0, 8, y_max, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W( 8, 0, x_max-8, y_max, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			return;
		case 2:	//２キャラ(8x16)
			DOT_PUT_W( 0, 0, x_max,	8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W( 0, 8, x_max,	y_max-8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			return;
		case 3:	//４キャラ(16x16)
			DOT_PUT_W( 0, 0, 8, 8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W( 8, 0, x_max-8, 8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W( 0, 8, 8, y_max-8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W( 8, 8, x_max-8, y_max-8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			return;
		}
	}else{
		u8 * chg_src = GFL_BG_4BitCgxChange8Bit( src, ssx*4*ssy*8, win->palnum, win->heapID );

		switch( print_pat ){
		case 0:	//１キャラ(8x8)
			DOT_PUT_W_256( 0, 0, x_max,	y_max, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			break;
		case 1:	//２キャラ(16x8)
			DOT_PUT_W_256( 0, 0, 8, y_max, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W_256( 8, 0, x_max-8, y_max, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			break;
		case 2:	//２キャラ(8x16)
			DOT_PUT_W_256( 0, 0, x_max,	8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W_256( 0, 8, x_max,	y_max-8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			break;
		case 3:	//４キャラ(16x16)
			DOT_PUT_W_256( 0, 0, 8, 8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W_256( 8, 0, x_max-8, 8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W_256( 0, 8, 8, y_max-8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			DOT_PUT_W_256( 8, 8, x_max-8, y_max-8, M_DST_ADRS, M_DST_POSX, M_DST_POSY, M_DST_XARG, M_PUT_TBL );
			break;
		}

		sys_FreeMemoryEz( chg_src );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップウインドウデータシフト
 *
 * @param	win		BMPデータ格納場所
 * @param	direct	方向
 * @param	offset	シフト幅
 * @param	data	空白になった領域を埋めるデータ
 *
 * @return	none
 *
 * ビットマップデータ(全領域)のシフトを実行
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_Shift( GFL_BMPWIN_DATA * win, u8 direct, u8 offset, u8 data )
{
	if( GFL_BG_ScreenColorModeGet( win->bgl, win->frmnum) == GX_BG_COLORMODE_16 ){
		BmpWinShift16( win, direct, offset, data );
	}else{
		BmpWinShift256( win, direct, offset, data );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップウインドウデータシフト（１６色用）
 *
 * @param	win		BMPデータ格納場所
 * @param	direct	方向
 * @param	offset	シフト幅
 * @param	data	空白になった領域を埋めるデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BmpWinShift16( GFL_BMPWIN_DATA * win, u8 direct, u8 offset, u8 data )
{
	u8 * cgxram;
	int	chroffs, woffs, roffs;
	int	blanch_chr, over_offs;
	int	xsiz, rline;
	int	i;

	cgxram		= (u8 *)win->chrbuf;
	blanch_chr	= (data<<24) | (data<<16) | (data<<8) | (data<<0);
	over_offs	= win->sizy * win->sizx * GFL_BG_1CHRDATASIZ;
	xsiz		= win->sizx;

	switch( direct ){
	case GFL_BMPWIN_SHIFT_U:
		for( chroffs=0; chroffs<over_offs; chroffs+=GFL_BG_1CHRDATASIZ ){
			rline = offset;

			for( i=0; i<8; i++ ){
				woffs = chroffs + (i << 2);
				roffs = chroffs + (((xsiz * (rline&0xfffffff8))|(rline&0x00000007)) << 2);
				if(roffs < over_offs){
					*(u32*)(cgxram + woffs) = *(u32*)(cgxram + roffs);
				}else{
					*(u32*)(cgxram + woffs) = blanch_chr;
				}
				rline++;
			}
		}
		break;
	case GFL_BMPWIN_SHIFT_D:
		cgxram += over_offs - 4;

		for( chroffs=0; chroffs<over_offs; chroffs+=GFL_BG_1CHRDATASIZ ){
			rline = offset;

			for( i=0; i<8; i++ ){
				woffs = chroffs + (i << 2);
				roffs = chroffs + (((xsiz * (rline&0xfffffff8))|(rline&0x00000007)) << 2);
				if(roffs < over_offs){
					*(u32*)(cgxram - woffs) = *(u32*)(cgxram - roffs);
				}else{
					*(u32*)(cgxram - woffs) = blanch_chr;
				}
				rline++;
			}
		}
		break;
	case GFL_BMPWIN_SHIFT_L:
		break;
	case GFL_BMPWIN_SHIFT_R:
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップウインドウデータシフト（２５６色用）
 *
 * @param	win		BMPデータ格納場所
 * @param	direct	方向
 * @param	offset	シフト幅
 * @param	data	空白になった領域を埋めるデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BmpWinShift256( GFL_BMPWIN_DATA * win, u8 direct, u8 offset, u8 data )
{
	u8 * cgxram;
	int	chroffs, woffs, roffs;
	int	blanch_chr, over_offs;
	int	xsiz, rline;
	int	i;

	cgxram		= (u8 *)win->chrbuf;
	blanch_chr	= ( data << 24 ) | ( data << 16 ) | ( data << 8 ) | data;
	over_offs	= win->sizy * win->sizx * GFL_BG_8BITCHRSIZ;
	xsiz		= win->sizx;

	switch(direct){
	case GFL_BMPWIN_SHIFT_U:
		for( chroffs=0; chroffs<over_offs; chroffs+=GFL_BG_8BITCHRSIZ ){
			rline = offset;

			for( i=0; i<8; i++ ){
				woffs = chroffs + (i << 3);
				roffs = chroffs + (((xsiz * (rline&0xfffffff8))|(rline&0x00000007)) << 3);
				if( roffs < over_offs ){
					*(u32*)(cgxram + woffs) = *(u32*)(cgxram + roffs);
				}else{
					*(u32*)(cgxram + woffs) = blanch_chr;
				}

				woffs += 4;
				roffs += 4;
				if( roffs < over_offs + 4 ){
					*(u32*)(cgxram + woffs) = *(u32*)(cgxram + roffs);
				}else{
					*(u32*)(cgxram + woffs) = blanch_chr;
				}

				rline++;
			}
		}
		break;
	case GFL_BMPWIN_SHIFT_D:
		cgxram += ( over_offs - 8 );

		for( chroffs=0; chroffs<over_offs; chroffs+=GFL_BG_8BITCHRSIZ ){
			rline = offset;

			for( i=0; i<8; i++ ){
				woffs = chroffs + (i << 3);
				roffs = chroffs + (((xsiz * (rline&0xfffffff8))|(rline&0x00000007)) << 3);
				if( roffs < over_offs ){
					*(u32*)(cgxram - woffs) = *(u32*)(cgxram - roffs);
				}else{
					*(u32*)(cgxram - woffs) = blanch_chr;
				}

				woffs -= 4;
				roffs -= 4;
				if( roffs < over_offs - 4 ){
					*(u32*)(cgxram - woffs) = *(u32*)(cgxram - roffs);
				}else{
					*(u32*)(cgxram - woffs) = blanch_chr;
				}

				rline++;
			}
		}
		break;
	case GFL_BMPWIN_SHIFT_L:
		break;
	case GFL_BMPWIN_SHIFT_R:
		break;
	}
}

//=============================================================================================
//=============================================================================================
//	BMPデータ操作
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * BMPデータ取得：BGフレーム番号
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	win->frmnum
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BMPWIN_GetFrame( GFL_BMPWIN_DATA * win )
{
	return win->frmnum;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPデータ取得：Xサイズ
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	win->sizx
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BMPWIN_GetSizeX( GFL_BMPWIN_DATA * win )
{
	return win->sizx;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPデータ取得：Yサイズ
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	win->sizy
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BMPWIN_GetSizeY( GFL_BMPWIN_DATA * win )
{
	return win->sizy;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPデータ取得：X座標
 *
 * @param	win		BMPデータ格納場所
 * @param	index	ビットマップ使用設定ヘッダINDEX
 *
 * @return	win->posx
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BMPWIN_GetPosX( GFL_BMPWIN_DATA * win )
{
	return win->posx;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPデータ取得：Y座標
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	win->posy
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BMPWIN_GetPosY( GFL_BMPWIN_DATA * win )
{
	return win->posy;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPデータ取得：キャラ番号
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	win->chrofs
 */
//--------------------------------------------------------------------------------------------
u16 GFL_BMPWIN_GetChrofs( GFL_BMPWIN_DATA * win )
{
	return win->chrofs;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPデータ設定：X座標
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_SetPosX( GFL_BMPWIN_DATA * win, u8 px )
{
	win->posx = px;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPデータ設定：Y座標
 *
 * @param	win		BMPデータ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_SetPosY( GFL_BMPWIN_DATA * win, u8 py )
{
	win->posy = py;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPデータ設定：パレット番号
 *
 * @param	win		BMPデータ格納場所
 * @param   palnum	パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_SetPal( GFL_BMPWIN_DATA * win, u8 palnum )
{
	win->palnum = palnum;
}
