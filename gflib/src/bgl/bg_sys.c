//=============================================================================================
/**
 * @file	bg_sys.c
 * @brief	BG描画システムプログラム
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

#define	__BG_SYS_H_GLOBAL__
#include "calctool.h"


#define DMA_USE (1)

//=============================================================================================
//	型宣言
//=============================================================================================

//------------------------------------------------------------------
///フレーム設定構造体
typedef	struct	{
	void*	screen_buf;	//スクリーンデータ転送バッファポインタ
	u32		screen_buf_siz;
	u32		screen_buf_ofs;
	int		scroll_x;
	int		scroll_y;
//	u8		dispsite;
	u8		mode;
	u8		screen_siz;
	u8		col_mode;

	u8		base_char_size;

	u16		rad;
	u16		dummy;				//padding
	fx32	scale_x;
	fx32	scale_y;
	int		cx;
	int		cy;

}GFL_BG_SYS;

//------------------------------------------------------------------
///システム設定構造体（システムワーク）
struct _GFL_BG_INI {
	u32			heapID;
	u16			scroll_req;							// スクロールリクエストフラグ s3/s2/s1/s0_m3/m2/m1/m0
	u16			loadscrn_req;						// スクリーン転送リクエストフラグ s3/s2/s1/s0_m3/m2/m1/m0
	GFL_BG_SYS	bgsys[GFL_BG_FRAME_MAX];
	GFL_AREAMAN	*area_m;							//メイン画面用エリアマネージャポインタ
	GFL_AREAMAN	*area_s;							//サブ画面用エリアマネージャポインタ
	u32			CharVramBaseAdrs[GFL_BG_FRAME_MAX];	//VRAMベースアドレスを格納
	u32			ScrVramBaseAdrs[GFL_BG_FRAME_MAX];	//VRAMベースアドレスを格納
};

static	GFL_BG_INI	*bgl=NULL;

//------------------------------------------------------------------

//=============================================================================================
//	定数宣言
//=============================================================================================
#define	GFL_BG_AREA_SIZE	(0x20000/0x20)

//=============================================================================================
//	プロトタイプ宣言
//=============================================================================================
static u8 BgScreenSizeConv( u8 type, u8 mode );
static void ScrollParamSet( GFL_BG_SYS * ini, u8 mode, int value );
static void AffineScrollSetMtxFix( u8 frmnum );
static void GFL_BG_LoadScreenSub( u8 frmnum, void* src, u32 ofs, u32 siz );
static void LoadCharacter( u8 frmnum, const void * src, u32 datasiz, u32 offs );
static void GFL_BG_LoadCharacterSub( u8 frmnum, void* src, u32 ofs, u32 siz );
static void GFL_BG_ScrWrite_Normal(
					GFL_BG_SYS * ini, u8 write_px, u8 write_py, u8 write_sx, u8 write_sy,
					u16 * buf, u8 buf_px, u8 buf_py, u8 buf_sx, u8 buf_sy ,u8 mode);
static void GFL_BG_ScrWrite_Affine(
					GFL_BG_SYS * ini, u8 write_px, u8 write_py, u8 write_sx, u8 write_sy,
					u8 * buf, u8 buf_px, u8 buf_py, u8 buf_sx, u8 buf_sy ,u8 mode);
static void GFL_BG_ScrFill_Normal(
					GFL_BG_SYS * ini, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode );
static void GFL_BG_ScrFill_Affine(
					GFL_BG_SYS * ini, u8 dat, u8 px, u8 py, u8 sx, u8 sy );

static void CgxFlipCheck( u8 flip, u8 * buf ,u32 headID);

static void RadianParamSet( GFL_BG_SYS * ini, u8 mode, u16 value );
static void ScaleParamSet( GFL_BG_SYS * ini, u8 mode, fx32 value );
static void CenterParamSet( GFL_BG_SYS * ini, u8 mode, int value );

static	void	*LoadFile(int heapID,const char *path);
static	void	*LoadFileEx(int heapID,const char *path,u32 *size);

//=============================================================================================
//=============================================================================================
//	設定関数
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * BGL初期化
 *
 * @param	heapID	ヒープＩＤ
 *
 * @return	取得したメモリのアドレス
 */
//--------------------------------------------------------------------------------------------
void	GFL_BG_sysInit( u32 heapID )
{
	int			i;

	bgl = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_BG_INI) );

	memset( bgl, 0, sizeof(GFL_BG_INI) );
	bgl->heapID = heapID;
	bgl->scroll_req = 0;
	bgl->loadscrn_req = 0;

	//VRAMエリアマネージャ初期化
	bgl->area_m=GFL_AREAMAN_Create( GFL_BG_AREA_SIZE, heapID );
	bgl->area_s=GFL_AREAMAN_Create( GFL_BG_AREA_SIZE, heapID );

	for(i=0;i<GFL_BG_FRAME_MAX;i++){
		bgl->CharVramBaseAdrs[i]=NULL;
		bgl->ScrVramBaseAdrs[i]=NULL;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * システムワークエリア開放
 *
 * @param	bgl		システムワークエリアへのポインタ
 */
//--------------------------------------------------------------------------------------------
void	GFL_BG_sysExit( void )
{
	//VRAMエリアマネージャ削除
	GFL_AREAMAN_Delete( bgl->area_m );
	GFL_AREAMAN_Delete( bgl->area_s );

	GFL_HEAP_FreeMemory( bgl );

	bgl=NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * エリアマネージャを使用して未使用領域を検索して領域を確保
 *
 * @param	frmnum		ＢＧフレーム番号
 * @param	size		確保するサイズ
 * 
 * @return	pos			確保した領域のポジション（確保できなかった時はAREAMAN_POS_NOTFOUND）
 */
//--------------------------------------------------------------------------------------------
u32 GFL_BG_CharAreaGet( u32 frmnum, u32 size )
{
	u32	pos;

	switch(frmnum){
	case GFL_BG_FRAME0_M:
	case GFL_BG_FRAME1_M:
	case GFL_BG_FRAME2_M:
	case GFL_BG_FRAME3_M:
		pos=GFL_AREAMAN_ReserveAssignArea
				( bgl->area_m, bgl->CharVramBaseAdrs[frmnum]/0x20, GFL_BG_AREA_SIZE, (size/0x20)+((size%0x20)==0?0:1));
		break;
	case GFL_BG_FRAME0_S:
	case GFL_BG_FRAME1_S:
	case GFL_BG_FRAME2_S:
	case GFL_BG_FRAME3_S:
		pos=GFL_AREAMAN_ReserveAssignArea
				( bgl->area_s, bgl->CharVramBaseAdrs[frmnum]/0x20, GFL_BG_AREA_SIZE, (size/0x20)+((size%0x20)==0?0:1));
		break;
	}

	pos=(pos*0x20-bgl->CharVramBaseAdrs[frmnum])/0x20;

	return pos;
}

//--------------------------------------------------------------------------------------------
/**
 * 確保した領域を開放
 *
 * @param	frmnum		ＢＧフレーム番号
 * @param	pos			確保した領域の先頭位置
 * @param	size		開放するサイズ
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_CharAreaFree( u32 frmnum, u32 pos, u32 size )
{
	pos=(pos*0x20+bgl->CharVramBaseAdrs[frmnum])/0x20;

	switch(frmnum){
	case GFL_BG_FRAME0_M:
	case GFL_BG_FRAME1_M:
	case GFL_BG_FRAME2_M:
	case GFL_BG_FRAME3_M:
		GFL_AREAMAN_Release( bgl->area_m, pos, (size/0x20)+((size%0x20)==0?0:1));
		break;
	case GFL_BG_FRAME0_S:
	case GFL_BG_FRAME1_S:
	case GFL_BG_FRAME2_S:
	case GFL_BG_FRAME3_S:
		GFL_AREAMAN_Release( bgl->area_s, pos, (size/0x20)+((size%0x20)==0?0:1));
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * BGL取得
 *
 * @return	BGL
 */
//--------------------------------------------------------------------------------------------
GFL_BG_INI * GFL_BG_BGLGet( void )
{
	return bgl;
}

//--------------------------------------------------------------------------------------------
/**
 * ヒープID取得
 *
 * @return	BGLのヒープID
 */
//--------------------------------------------------------------------------------------------
u32	GFL_BG_HeapIDGet( void )
{
	return bgl->heapID;
}

//--------------------------------------------------------------------------------------------
/**
 * BGモード設定
 *
 * @param	data	BGモード設定データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_InitBG( const GFL_BG_SYS_HEADER * data )
{
    GX_SetGraphicsMode( data->dispMode, data->bgMode, data->bg0_2Dor3D );
    GXS_SetGraphicsMode( data->bgModeSub );

	GX_SetBGScrOffset( GX_BGSCROFFSET_0x00000 ); 
	GX_SetBGCharOffset( GX_BGCHAROFFSET_0x00000 ); 

	GFL_DISP_GX_VisibleControlInit();
	GFL_DISP_GXS_VisibleControlInit();
}

//--------------------------------------------------------------------------------------------
/**
 * BGモード設定（画面ごと）
 *
 * @param	data	BGモード設定データ
 * @param	flg		設定する画面
 *
 * @return	none
 *
 * @li	flg = GFL_BG_MAIN_DISP : メイン画面
 * @li	flg = GFL_BG_SUB_DISP : サブ画面
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_InitBGDisp( const GFL_BG_SYS_HEADER * data, u8 flg )
{
	if( flg == GFL_BG_MAIN_DISP ){
		GX_SetGraphicsMode( data->dispMode, data->bgMode, data->bg0_2Dor3D );
		GFL_DISP_GX_VisibleControlInit();
	}else{
		GXS_SetGraphicsMode( data->bgModeSub );
		GFL_DISP_GXS_VisibleControlInit();
	}
}

//--------------------------------------------------------------------------------------------
/**
 * BG面設定
 *
 * @param	frmnum		ＢＧフレーム番号
 * @param	data		ＢＧコントロールデータ
 * @param	mode		ＢＧモード
 * 
 * @return	none
 *
 * @li	mode = GFL_BG_MODE_TEXT		: テキスト
 * @li	mode = GFL_BG_MODE_AFFINE	: アフィン
 * @li	mode = GFL_BG_MODE_256X16	: アフィン拡張BG
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_BGControlSet( u8 frmnum, const GFL_BG_BGCNT_HEADER * data, u8 mode )
{
	u8	screen_size = BgScreenSizeConv( data->screenSize, mode );

	switch( frmnum ){
	case GFL_BG_FRAME0_M:
		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
		G2_SetBG0Control(
				(GXBGScrSizeText)screen_size, 
				(GXBGColorMode)data->colorMode,
				(GXBGScrBase)data->screenBase,
				(GXBGCharBase)data->charBase,
				(GXBGExtPltt)data->bgExtPltt);
		G2_SetBG0Priority( (int)data->priority );
		G2_BG0Mosaic( (BOOL)data->mosaic );
		break;

	case GFL_BG_FRAME1_M:
		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
		G2_SetBG1Control(
				(GXBGScrSizeText)screen_size, 
				(GXBGColorMode)data->colorMode,
				(GXBGScrBase)data->screenBase,
				(GXBGCharBase)data->charBase,
				(GXBGExtPltt)data->bgExtPltt);
		G2_SetBG1Priority( (int)data->priority );
		G2_BG1Mosaic( (BOOL)data->mosaic );
		break;

	case GFL_BG_FRAME2_M:
		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_BG2, VISIBLE_ON );
		switch( mode ){
		default:
		case GFL_BG_MODE_TEXT:
			G2_SetBG2ControlText(
					(GXBGScrSizeText)screen_size, 
					(GXBGColorMode)data->colorMode,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase);
			break;
		case GFL_BG_MODE_AFFINE:
			G2_SetBG2ControlAffine(
					(GXBGScrSizeAffine)screen_size, 
					(GXBGAreaOver)data->areaOver,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase );
			break;
		case GFL_BG_MODE_256X16:
			G2_SetBG2Control256x16Pltt(
					(GXBGScrSize256x16Pltt)screen_size, 
					(GXBGAreaOver)data->areaOver,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase );
			break;
		}
		G2_SetBG2Priority( (int)data->priority );
		G2_BG2Mosaic( (BOOL)data->mosaic );
		break;

	case GFL_BG_FRAME3_M:
		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
		switch( mode ){
		default:
		case GFL_BG_MODE_TEXT:
			G2_SetBG3ControlText(
					(GXBGScrSizeText)screen_size, 
					(GXBGColorMode)data->colorMode,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase);
			break;
		case GFL_BG_MODE_AFFINE:
			G2_SetBG3ControlAffine(
					(GXBGScrSizeAffine)screen_size, 
					(GXBGAreaOver)data->areaOver,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase );
			break;
		case GFL_BG_MODE_256X16:
			G2_SetBG3Control256x16Pltt(
					(GXBGScrSize256x16Pltt)screen_size, 
					(GXBGAreaOver)data->areaOver,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase );
			break;
		}
		G2_SetBG3Priority( (int)data->priority );
		G2_BG3Mosaic( (BOOL)data->mosaic );
		break;

	case GFL_BG_FRAME0_S:
		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
		G2S_SetBG0Control(
				(GXBGScrSizeText)screen_size, 
				(GXBGColorMode)data->colorMode,
				(GXBGScrBase)data->screenBase,
				(GXBGCharBase)data->charBase,
				(GXBGExtPltt)data->bgExtPltt);
		G2S_SetBG0Priority( (int)data->priority );
		G2S_BG0Mosaic( (BOOL)data->mosaic );
		break;

	case GFL_BG_FRAME1_S:
		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
		G2S_SetBG1Control(
				(GXBGScrSizeText)screen_size, 
				(GXBGColorMode)data->colorMode,
				(GXBGScrBase)data->screenBase,
				(GXBGCharBase)data->charBase,
				(GXBGExtPltt)data->bgExtPltt);
		G2S_SetBG1Priority( (int)data->priority );
		G2S_BG1Mosaic( (BOOL)data->mosaic );
		break;

	case GFL_BG_FRAME2_S:
		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_BG2, VISIBLE_ON );
		switch( mode ){
		default:
		case GFL_BG_MODE_TEXT:
			G2S_SetBG2ControlText(
					(GXBGScrSizeText)screen_size, 
					(GXBGColorMode)data->colorMode,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase);
			break;
		case GFL_BG_MODE_AFFINE:
			G2S_SetBG2ControlAffine(
					(GXBGScrSizeAffine)screen_size, 
					(GXBGAreaOver)data->areaOver,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase );
			break;
		case GFL_BG_MODE_256X16:
			G2S_SetBG2Control256x16Pltt(
					(GXBGScrSize256x16Pltt)screen_size, 
					(GXBGAreaOver)data->areaOver,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase );
			break;
		}
		G2S_SetBG2Priority( (int)data->priority );
		G2S_BG2Mosaic( (BOOL)data->mosaic );
		break;

	case GFL_BG_FRAME3_S:
		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_BG3, VISIBLE_ON );
		switch( mode ){
		default:
		case GFL_BG_MODE_TEXT:
			G2S_SetBG3ControlText(
					(GXBGScrSizeText)screen_size, 
					(GXBGColorMode)data->colorMode,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase);
			break;
		case GFL_BG_MODE_AFFINE:
			G2S_SetBG3ControlAffine(
					(GXBGScrSizeAffine)screen_size, 
					(GXBGAreaOver)data->areaOver,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase );
			break;
		case GFL_BG_MODE_256X16:
			G2S_SetBG3Control256x16Pltt(
					(GXBGScrSize256x16Pltt)screen_size, 
					(GXBGAreaOver)data->areaOver,
					(GXBGScrBase)data->screenBase,
					(GXBGCharBase)data->charBase );
			break;
		}
		G2S_SetBG3Priority( (int)data->priority );
		G2S_BG3Mosaic( (BOOL)data->mosaic );
		break;
	}

	bgl->bgsys[frmnum].rad     = 0;
	bgl->bgsys[frmnum].scale_x = FX32_ONE;
	bgl->bgsys[frmnum].scale_y = FX32_ONE;
	bgl->bgsys[frmnum].cx      = 0;
	bgl->bgsys[frmnum].cy      = 0;

	if( data->scrbufferSiz ){
		bgl->bgsys[frmnum].screen_buf = GFL_HEAP_AllocMemory( bgl->heapID, data->scrbufferSiz );

#ifdef	OSP_ERR_BGL_SCRBUF_GET		// スクリーンバッファ確保失敗
		if( ini->bgsys[frmnum].screen_buf == NULL ){
			OS_Printf("領域確保失敗\n");
		}
#endif	// OSP_ERR_BGL_SCRBUF_GET

		MI_CpuClear16( bgl->bgsys[frmnum].screen_buf, data->scrbufferSiz );
		bgl->bgsys[frmnum].screen_buf_siz = data->scrbufferSiz;
		bgl->bgsys[frmnum].screen_buf_ofs = data->scrbufferOfs;
#ifdef	OSP_BGL_SCRBUF_GET_SIZ		// 確保したスクリーンバッファのサイズ
		OS_Printf("ＳＣＲ領域=%x\n",bgl->bgsys[frmnum].screen_buf);
#endif	// OSP_BGL_SCRBUF_GET_SIZ
	}else{
		bgl->bgsys[frmnum].screen_buf = NULL;
		bgl->bgsys[frmnum].screen_buf_siz = 0;
		bgl->bgsys[frmnum].screen_buf_ofs = 0;
	}
	bgl->bgsys[frmnum].screen_siz = data->screenSize;
	bgl->bgsys[frmnum].mode = mode;
	bgl->bgsys[frmnum].col_mode = data->colorMode;

	if( mode == GFL_BG_MODE_TEXT && data->colorMode == GX_BG_COLORMODE_16 ){
		bgl->bgsys[frmnum].base_char_size = GFL_BG_1CHRDATASIZ;
	}else{
		bgl->bgsys[frmnum].base_char_size = GFL_BG_8BITCHRSIZ;
	}

	GFL_BG_ScrollSet( frmnum, GFL_BG_SCROLL_X_SET, data->scrollX );
	GFL_BG_ScrollSet( frmnum, GFL_BG_SCROLL_Y_SET, data->scrollY );

	//VRAMベースアドレスを格納
	bgl->CharVramBaseAdrs[frmnum]=((GXBGCharBase)data->charBase)*0x4000;
	bgl->ScrVramBaseAdrs[frmnum]=((GXBGScrBase)data->screenBase)*0x0800;
}

//--------------------------------------------------------------------------------------------
/**
 * BG面再設定
 *
 * @param	frmnum		ＢＧフレーム番号
 * @param	flg			変更パラメータ
 * @param	prm			変更値
 * 
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_BGControlReset( u8 frm, u8 flg, u8 prm )
{
	if( flg == BGL_RESET_COLOR ){
		bgl->bgsys[frm].col_mode = prm;
	}

	switch( frm ){
	case GFL_BG_FRAME0_M:
		{
			GXBg01Control	dat = G2_GetBG0Control();
			if( flg == BGL_RESET_SCRBASE ){
				dat.screenBase = prm;
			}else if( flg == BGL_RESET_CHRBASE ){
				dat.charBase = prm;
			}
			G2_SetBG0Control(
				(GXBGScrSizeText)dat.screenSize, 
				(GXBGColorMode)bgl->bgsys[frm].col_mode,
				(GXBGScrBase)dat.screenBase,
				(GXBGCharBase)dat.charBase,
				(GXBGExtPltt)dat.bgExtPltt );
		}
		break;

	case GFL_BG_FRAME1_M:
		{
			GXBg01Control	dat = G2_GetBG1Control();
			if( flg == BGL_RESET_SCRBASE ){
				dat.screenBase = prm;
			}else if( flg == BGL_RESET_CHRBASE ){
				dat.charBase = prm;
			}
			G2_SetBG1Control(
				(GXBGScrSizeText)dat.screenSize, 
				(GXBGColorMode)bgl->bgsys[frm].col_mode,
				(GXBGScrBase)dat.screenBase,
				(GXBGCharBase)dat.charBase,
				(GXBGExtPltt)dat.bgExtPltt );
		}
		break;

	case GFL_BG_FRAME2_M:
		switch( bgl->bgsys[frm].mode ){
		default:
		case GFL_BG_MODE_TEXT:
			{
				GXBg23ControlText	dat = G2_GetBG2ControlText();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2_SetBG2ControlText(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGColorMode)bgl->bgsys[frm].col_mode,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		case GFL_BG_MODE_AFFINE:
			{
				GXBg23ControlAffine	dat = G2_GetBG2ControlAffine();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2_SetBG2ControlAffine(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGAreaOver)dat.areaOver,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		case GFL_BG_MODE_256X16:
			{
				GXBg23Control256x16Pltt	dat = G2_GetBG2Control256x16Pltt();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2_SetBG2Control256x16Pltt(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGAreaOver)dat.areaOver,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		}
		break;

	case GFL_BG_FRAME3_M:
		switch( bgl->bgsys[frm].mode ){
		default:
		case GFL_BG_MODE_TEXT:
			{
				GXBg23ControlText	dat = G2_GetBG3ControlText();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2_SetBG3ControlText(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGColorMode)bgl->bgsys[frm].col_mode,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		case GFL_BG_MODE_AFFINE:
			{
				GXBg23ControlAffine	dat = G2_GetBG3ControlAffine();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2_SetBG3ControlAffine(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGAreaOver)dat.areaOver,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		case GFL_BG_MODE_256X16:
			{
				GXBg23Control256x16Pltt	dat = G2_GetBG3Control256x16Pltt();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2_SetBG3Control256x16Pltt(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGAreaOver)dat.areaOver,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		}
		break;

	case GFL_BG_FRAME0_S:
		{
			GXBg01Control	dat = G2S_GetBG0Control();
			if( flg == BGL_RESET_SCRBASE ){
				dat.screenBase = prm;
			}else if( flg == BGL_RESET_CHRBASE ){
				dat.charBase = prm;
			}
			G2S_SetBG0Control(
				(GXBGScrSizeText)dat.screenSize, 
				(GXBGColorMode)bgl->bgsys[frm].col_mode,
				(GXBGScrBase)dat.screenBase,
				(GXBGCharBase)dat.charBase,
				(GXBGExtPltt)dat.bgExtPltt );
		}
		break;

	case GFL_BG_FRAME1_S:
		{
			GXBg01Control	dat = G2S_GetBG1Control();
			if( flg == BGL_RESET_SCRBASE ){
				dat.screenBase = prm;
			}else if( flg == BGL_RESET_CHRBASE ){
				dat.charBase = prm;
			}
			G2S_SetBG1Control(
				(GXBGScrSizeText)dat.screenSize, 
				(GXBGColorMode)bgl->bgsys[frm].col_mode,
				(GXBGScrBase)dat.screenBase,
				(GXBGCharBase)dat.charBase,
				(GXBGExtPltt)dat.bgExtPltt );
		}
		break;

	case GFL_BG_FRAME2_S:
		switch( bgl->bgsys[frm].mode ){
		default:
		case GFL_BG_MODE_TEXT:
			{
				GXBg23ControlText	dat = G2S_GetBG2ControlText();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2S_SetBG2ControlText(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGColorMode)bgl->bgsys[frm].col_mode,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		case GFL_BG_MODE_AFFINE:
			{
				GXBg23ControlAffine	dat = G2S_GetBG2ControlAffine();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2S_SetBG2ControlAffine(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGAreaOver)dat.areaOver,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		case GFL_BG_MODE_256X16:
			{
				GXBg23Control256x16Pltt	dat = G2S_GetBG2Control256x16Pltt();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2S_SetBG2Control256x16Pltt(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGAreaOver)dat.areaOver,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		}
		break;

	case GFL_BG_FRAME3_S:
		switch( bgl->bgsys[frm].mode ){
		default:
		case GFL_BG_MODE_TEXT:
			{
				GXBg23ControlText	dat = G2S_GetBG3ControlText();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2S_SetBG3ControlText(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGColorMode)bgl->bgsys[frm].col_mode,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		case GFL_BG_MODE_AFFINE:
			{
				GXBg23ControlAffine	dat = G2S_GetBG3ControlAffine();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2S_SetBG3ControlAffine(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGAreaOver)dat.areaOver,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		case GFL_BG_MODE_256X16:
			{
				GXBg23Control256x16Pltt	dat = G2S_GetBG3Control256x16Pltt();
				if( flg == BGL_RESET_SCRBASE ){
					dat.screenBase = prm;
				}else if( flg == BGL_RESET_CHRBASE ){
					dat.charBase = prm;
				}
				G2S_SetBG3Control256x16Pltt(
					(GXBGScrSizeText)dat.screenSize,
					(GXBGAreaOver)dat.areaOver,
					(GXBGScrBase)dat.screenBase,
					(GXBGCharBase)dat.charBase );
			}
			break;
		}
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * エリアマネージャに領域確保を宣言（キャラデータ用）
 *
 * @param	frmnum	領域確保するフレームナンバー（VRAMアドレスを取得するのに使用）
 * @param	ofs		確保先オフセット
 * @param	size	確保サイズ
 *
 * @retval	データを読み込んだアドレス
 */
//--------------------------------------------------------------------------------------------
void	GFL_BG_CharAreaSet( u32 frmnum, u32 ofs, u32 size )
{
	switch(frmnum){
	case GFL_BG_FRAME0_M:
	case GFL_BG_FRAME1_M:
	case GFL_BG_FRAME2_M:
	case GFL_BG_FRAME3_M:
		GFL_AREAMAN_ReserveAssignPos(bgl->area_m,(bgl->CharVramBaseAdrs[frmnum]+ofs)/0x20,(size/0x20)+((size%0x20)==0?0:1));
		break;
	case GFL_BG_FRAME0_S:
	case GFL_BG_FRAME1_S:
	case GFL_BG_FRAME2_S:
	case GFL_BG_FRAME3_S:
		GFL_AREAMAN_ReserveAssignPos(bgl->area_s,(bgl->CharVramBaseAdrs[frmnum]+ofs)/0x20,(size/0x20)+((size%0x20)==0?0:1));
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * エリアマネージャに領域確保を宣言（スクリーンデータ用）
 *
 * @param	frmnum	領域確保するフレームナンバー（VRAMアドレスを取得するのに使用）
 * @param	ofs		確保先オフセット
 * @param	size	確保サイズ
 *
 * @retval	データを読み込んだアドレス
 */
//--------------------------------------------------------------------------------------------
void	GFL_BG_ScrAreaSet( u32 frmnum, u32 ofs, u32 size )
{
	switch(frmnum){
	case GFL_BG_FRAME0_M:
	case GFL_BG_FRAME1_M:
	case GFL_BG_FRAME2_M:
	case GFL_BG_FRAME3_M:
		GFL_AREAMAN_ReserveAssignPos(bgl->area_m,(bgl->ScrVramBaseAdrs[frmnum]+ofs)/0x20,(size/0x20)+((size%0x20)==0?0:1));
		break;
	case GFL_BG_FRAME0_S:
	case GFL_BG_FRAME1_S:
	case GFL_BG_FRAME2_S:
	case GFL_BG_FRAME3_S:
		GFL_AREAMAN_ReserveAssignPos(bgl->area_s,(bgl->ScrVramBaseAdrs[frmnum]+ofs)/0x20,(size/0x20)+((size%0x20)==0?0:1));
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンサイズ変換 ( GFLIB -> NitroSDK )
 *
 * @param	type	スクリーンサイズ ( GFLIB )
 * @param	mode	ＢＧモード
 *
 * @return	スクリーンサイズ ( NitroSDK )
 */
//--------------------------------------------------------------------------------------------
static u8 BgScreenSizeConv( u8 type, u8 mode )
{
	switch( mode ){
	case GFL_BG_MODE_TEXT:		// テキスト
		if( type == GFL_BG_SCRSIZ_256x256 ){ return GX_BG_SCRSIZE_TEXT_256x256; }
		if( type == GFL_BG_SCRSIZ_256x512 ){ return GX_BG_SCRSIZE_TEXT_256x512; }
		if( type == GFL_BG_SCRSIZ_512x256 ){ return GX_BG_SCRSIZE_TEXT_512x256; }
		if( type == GFL_BG_SCRSIZ_512x512 ){ return GX_BG_SCRSIZE_TEXT_512x512; }
		break;
	case GFL_BG_MODE_AFFINE:	// アフィン
		if( type == GFL_BG_SCRSIZ_128x128 ){ return GX_BG_SCRSIZE_AFFINE_128x128; }
		if( type == GFL_BG_SCRSIZ_256x256 ){ return GX_BG_SCRSIZE_AFFINE_256x256; }
		if( type == GFL_BG_SCRSIZ_512x512 ){ return GX_BG_SCRSIZE_AFFINE_512x512; }
		if( type == GFL_BG_SCRSIZ_1024x1024 ){ return GX_BG_SCRSIZE_AFFINE_1024x1024; }
		break;
	case GFL_BG_MODE_256X16:	// アフィン拡張BG
		if( type == GFL_BG_SCRSIZ_128x128 ){ return GX_BG_SCRSIZE_256x16PLTT_128x128; }
		if( type == GFL_BG_SCRSIZ_256x256 ){ return GX_BG_SCRSIZE_256x16PLTT_256x256; }
		if( type == GFL_BG_SCRSIZ_512x512 ){ return GX_BG_SCRSIZE_256x16PLTT_512x512; }
		if( type == GFL_BG_SCRSIZ_1024x1024 ){ return GX_BG_SCRSIZE_256x16PLTT_1024x1024; }
		break;
	}
	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * キャラ単位のスクリーンサイズ取得
 *
 * @param	type	スクリーンサイズ ( GFLIB )
 * @param	x		X方向のサイズ格納場所
 * @param	y		Y方向のサイズ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BgScreenSizeGet( u8 type, u8 * x, u8 * y )
{
	switch( type ){
	case GFL_BG_SCRSIZ_128x128:
		*x = 16;
		*y = 16;
		return;
	case GFL_BG_SCRSIZ_256x256:
		*x = 32;
		*y = 32;
		return;
	case GFL_BG_SCRSIZ_256x512:
		*x = 32;
		*y = 64;
		return;
	case GFL_BG_SCRSIZ_512x256:
		*x = 64;
		*y = 32;
		return;
	case GFL_BG_SCRSIZ_512x512:
		*x = 64;
		*y = 64;
		return;
	case GFL_BG_SCRSIZ_1024x1024:
		*x = 128;
		*y = 128;
		return;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * GFL_BG_BGControlSetで取得したメモリを開放
 *
 * @param	frmnum		BGフレーム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_BGControlExit( u8 frmnum )
{
	if( bgl->bgsys[frmnum].screen_buf == NULL ){
		return;
	}
	GFL_HEAP_FreeMemory( bgl->bgsys[frmnum].screen_buf );
	bgl->bgsys[frmnum].screen_buf = NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * 表示プライオリティ設定
 *
 * @param	frmnum		BGフレーム番号
 * @param	priority	プライオリティ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_PrioritySet( u8 frmnum, u8 priority )
{
	switch( frmnum ){
	case GFL_BG_FRAME0_M:
		G2_SetBG0Priority( priority );
		break;
	case GFL_BG_FRAME1_M:
		G2_SetBG1Priority( priority );
		break;
	case GFL_BG_FRAME2_M:
		G2_SetBG2Priority( priority );
		break;
	case GFL_BG_FRAME3_M:
		G2_SetBG3Priority( priority );
		break;
	case GFL_BG_FRAME0_S:
		G2S_SetBG0Priority( priority );
		break;
	case GFL_BG_FRAME1_S:
		G2S_SetBG1Priority( priority );
		break;
	case GFL_BG_FRAME2_S:
		G2S_SetBG2Priority( priority );
		break;
	case GFL_BG_FRAME3_S:
		G2S_SetBG3Priority( priority );
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 表示ON・OFF設定
 *
 * @param	frmnum		BGフレーム番号
 * @param	visible		VISIBLE_ON or VISIBLE_OFF
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_VisibleSet( u8 frmnum, u8 visible )
{
	switch( frmnum ){
	case GFL_BG_FRAME0_M:
		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_BG0, visible );
		break;
	case GFL_BG_FRAME1_M:
		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_BG1, visible );
		break;
	case GFL_BG_FRAME2_M:
		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_BG2, visible );
		break;
	case GFL_BG_FRAME3_M:
		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_BG3, visible );
		break;
	case GFL_BG_FRAME0_S:
		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_BG0, visible );
		break;
	case GFL_BG_FRAME1_S:
		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_BG1, visible );
		break;
	case GFL_BG_FRAME2_S:
		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_BG2, visible );
		break;
	case GFL_BG_FRAME3_S:
		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_BG3, visible );
		break;
	}
}


//=============================================================================================
//=============================================================================================
//	スクロール関数
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * スクロール処理
 *
 * @param	frmnum		BGフレーム番号
 * @param	mode		スクロールモード
 * @param	value		スクロール値
 *
 * @return	none
 *
 * @li	拡縮面が拡縮・回転する場合はGFL_BG_AffineScrollSet(...)を使用すること
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrollSet( u8 frmnum, u8 mode, int value )
{
	int	scroll_x, scroll_y;

	ScrollParamSet( &bgl->bgsys[frmnum], mode, value );

	scroll_x = bgl->bgsys[frmnum].scroll_x;
	scroll_y = bgl->bgsys[frmnum].scroll_y;

	switch( frmnum ){
	case GFL_BG_FRAME0_M:
		G2_SetBG0Offset( scroll_x, scroll_y );
		return;
	case GFL_BG_FRAME1_M:
		G2_SetBG1Offset( scroll_x, scroll_y );
		return;
	case GFL_BG_FRAME2_M:
		if( bgl->bgsys[GFL_BG_FRAME2_M].mode == GFL_BG_MODE_TEXT ){
			G2_SetBG2Offset( scroll_x, scroll_y );
		}else{
			AffineScrollSetMtxFix( GFL_BG_FRAME2_M );
		}
		return;
	case GFL_BG_FRAME3_M:
		if( bgl->bgsys[GFL_BG_FRAME3_M].mode == GFL_BG_MODE_TEXT ){
			G2_SetBG3Offset( scroll_x, scroll_y );
		}else{
			AffineScrollSetMtxFix( GFL_BG_FRAME3_M );
		}
		return;
	case GFL_BG_FRAME0_S:
		G2S_SetBG0Offset( scroll_x, scroll_y );
		return;
	case GFL_BG_FRAME1_S:
		G2S_SetBG1Offset( scroll_x, scroll_y );
		return;
	case GFL_BG_FRAME2_S:
		if( bgl->bgsys[GFL_BG_FRAME2_S].mode == GFL_BG_MODE_TEXT ){
			G2S_SetBG2Offset( scroll_x, scroll_y );
		}else{
			AffineScrollSetMtxFix( GFL_BG_FRAME2_S );
		}
		return;
	case GFL_BG_FRAME3_S:
		if( bgl->bgsys[GFL_BG_FRAME3_S].mode == GFL_BG_MODE_TEXT ){
			G2S_SetBG3Offset( scroll_x, scroll_y );
		}else{
			AffineScrollSetMtxFix( GFL_BG_FRAME3_S );
		}
		return;
	}
}
//--------------------------------------------------------------------------------------------
/**
 * スクロール値Ｘ取得
 *
 * @param	frmnum		BGフレーム番号
 *
 * @return	int			スクロール値Ｘ
 */
//--------------------------------------------------------------------------------------------
int GFL_BG_ScrollGetX( u32 frmnum )
{
	return bgl->bgsys[frmnum].scroll_x;
}
//--------------------------------------------------------------------------------------------
/**
 * スクロール値Ｙ取得
 *
 * @param	frmnum		BGフレーム番号
 *
 * @return	int			スクロール値Ｙ
 */
//--------------------------------------------------------------------------------------------
int GFL_BG_ScrollGetY( u32 frmnum )
{
	return bgl->bgsys[frmnum].scroll_y;
}

//--------------------------------------------------------------------------------------------
/**
 * 拡縮面の拡縮・回転・スクロール処理
 *
 * @param	frmnum		BGフレーム番号
 * @param	mode		スクロールモード
 * @param	value		スクロール値
 * @param	mtx			変換行列
 * @param	cx			回転中心X座標
 * @param	cy			回転中心Y座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_AffineScrollSet( u8 frmnum, u8 mode, int value, const MtxFx22 * mtx, int cx, int cy )
{
	ScrollParamSet( &bgl->bgsys[frmnum], mode, value );
	GFL_BG_AffineSet( frmnum, mtx, cx, cy );
}

//--------------------------------------------------------------------------------------------
/**
 * パラメータセット
 *
 * @param	frmnum		BGフレーム番号
 * @param	mode		スクロールモード
 * @param	value		スクロール値
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ScrollParamSet( GFL_BG_SYS * ini, u8 mode, int value )
{
	switch(mode){
	case GFL_BG_SCROLL_X_SET:
		ini->scroll_x = value;
		break;
	case GFL_BG_SCROLL_X_INC:
		ini->scroll_x += value;
		break;
	case GFL_BG_SCROLL_X_DEC:
		ini->scroll_x -= value;
		break;
	case GFL_BG_SCROLL_Y_SET:
		ini->scroll_y = value;
		break;
	case GFL_BG_SCROLL_Y_INC:
		ini->scroll_y += value;
		break;
	case GFL_BG_SCROLL_Y_DEC:
		ini->scroll_y -= value;
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 拡縮・回転処理
 *
 * @param	frmnum		BGフレーム番号
 * @param	mtx			変換行列
 * @param	cx			回転中心X座標
 * @param	cy			回転中心Y座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_AffineSet( u8 frmnum, const MtxFx22 * mtx, int cx, int cy )
{
	switch( frmnum ){
	case GFL_BG_FRAME2_M:
		G2_SetBG2Affine(
			mtx, cx, cy, bgl->bgsys[frmnum].scroll_x, bgl->bgsys[frmnum].scroll_y );
		return;
	case GFL_BG_FRAME3_M:
		G2_SetBG3Affine(
			mtx, cx, cy, bgl->bgsys[frmnum].scroll_x, bgl->bgsys[frmnum].scroll_y );
		return;
	case GFL_BG_FRAME2_S:
		G2S_SetBG2Affine(
			mtx, cx, cy, bgl->bgsys[frmnum].scroll_x, bgl->bgsys[frmnum].scroll_y );
		return;
	case GFL_BG_FRAME3_S:
		G2S_SetBG3Affine(
			mtx, cx, cy, bgl->bgsys[frmnum].scroll_x, bgl->bgsys[frmnum].scroll_y );
		return;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 拡縮面のスクロール処理（拡縮・回転していない場合の処理）
 *
 * @param	frmnum		BGフレーム番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void AffineScrollSetMtxFix( u8 frmnum )
{
	MtxFx22	mtx;

#if 0
	OS_TPrintf("[AFSCMTX]  frm:%d\n", frmnum);
#endif

	AffineMtxMake_2D( &mtx, 0, FX32_ONE, FX32_ONE, AFFINE_MAX_NORMAL );
	GFL_BG_AffineSet( frmnum, &mtx, 0, 0 );
}


//=============================================================================================
//=============================================================================================
//	展開関数
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * データ展開
 *
 * @param	src			展開元
 * @param	dst			展開先
 * @param	datasiz		サイズ
 *
 * @return	none
 *
 * @li	datasiz = GFL_BG_DATA_LZH : 圧縮データ
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_DataDecord( const void * src, void * dst, u32 datasiz )
{
	if( datasiz == GFL_BG_DATA_LZH ){
		MI_UncompressLZ8( src, dst );
	}else{
		if( (!((u32)src % 4)) && (!((u32)dst % 4)) && (!((u16)datasiz % 4)) ){
			MI_CpuCopy32( src, dst, datasiz );
		}else{
			MI_CpuCopy16( src, dst, datasiz );
		}
	}
}


//=============================================================================================
//=============================================================================================
//	転送関数
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * フレームに設定されたスクリーンデータを全転送
 *
 * @param	frmnum		BGフレーム番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_LoadScreenReq( u8 frmnum )
{
	GFL_BG_LoadScreen( frmnum, bgl->bgsys[frmnum].screen_buf,
		bgl->bgsys[frmnum].screen_buf_siz, bgl->bgsys[frmnum].screen_buf_ofs );
}

//--------------------------------------------------------------------------------------------
/**
 * 指定データをスクリーンに転送
 *
 * @param	frmnum		BGフレーム番号
 * @param	src			転送するデータ
 * @param	datasiz		転送サイズ
 * @param	offs		オフセット
 *
 * @return	none
 *
 * @li	datasiz = GFL_BG_DATA_LZH : 圧縮データ
 *
 *	圧縮されたデータは解凍場所にbgl->bgsys[frmnum].screen_bufを使用するため
 *	bgl->bgsys[frmnum].screen_bufにデータがセットされるが、非圧縮の場合は、
 *	セットされないので、注意すること。
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_LoadScreen( u8 frmnum, const void * src, u32 datasiz, u32 offs )
{
	void * decode_buf;

	if( datasiz == GFL_BG_DATA_LZH ){
		if( bgl->bgsys[frmnum].screen_buf != NULL ){
			decode_buf = bgl->bgsys[frmnum].screen_buf;
			GFL_BG_DataDecord( src, decode_buf, datasiz );

			GFL_BG_LoadScreenSub(
				frmnum, decode_buf, 
				bgl->bgsys[frmnum].screen_buf_ofs * GFL_BG_1SCRDATASIZ,
				bgl->bgsys[frmnum].screen_buf_siz );
		}else{
			u32	alloc_siz;

			alloc_siz	= ((*(u32*)src) >> 8);
			decode_buf = GFL_HEAP_AllocMemoryLow( bgl->heapID, alloc_siz );

#ifdef	OSP_ERR_BGL_DECODEBUF_GET		// 展開領域確保失敗
			if( decode_buf == NULL ){
				OS_Printf("領域確保失敗\n");
			}
#endif	// OSP_ERR_BGL_DECODEBUF_GET

			GFL_BG_DataDecord( src, decode_buf, datasiz );

			GFL_BG_LoadScreenSub( frmnum, decode_buf, offs * GFL_BG_1SCRDATASIZ, alloc_siz );
			GFL_HEAP_FreeMemory( decode_buf );
		}
	}else{
		GFL_BG_LoadScreenSub( frmnum, (void*)src, offs * GFL_BG_1SCRDATASIZ, datasiz );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 指定データをスクリーンに転送（ファイル参照）
 *
 * @param	frmnum		BGフレーム番号
 * @param	path		ファイルのパス名
 * @param	offs		オフセット
 *
 * @return	none
 *
 *	圧縮未対応
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_LoadScreenFile( u8 frmnum, const char * path, u32 offs )
{
	void * mem;
	u32	size;
	u32	mode;

	mem = LoadFileEx( bgl->heapID, path, &size );
	if( mem == NULL ){
		return;	//エラー
	}

	GFL_BG_ScrAreaSet( frmnum, offs, size );

	GFL_BG_ScreenBufSet( frmnum, mem, size );
	GFL_BG_LoadScreen( frmnum, mem, size, offs );
	GFL_HEAP_FreeMemory( mem );
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーン転送
 *
 * @param	frmnum		BGフレーム番号
 * @param	src			転送するデータ
 * @param	offs		オフセット
 * @param	siz			転送サイズ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void GFL_BG_LoadScreenSub( u8 frmnum, void* src, u32 ofs, u32 siz )
{
#if DMA_USE
	DC_FlushRange( src, siz );

	switch( frmnum ){
	case GFL_BG_FRAME0_M:
		GX_LoadBG0Scr( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME1_M:
		GX_LoadBG1Scr( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME2_M:
		GX_LoadBG2Scr( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME3_M:
		GX_LoadBG3Scr( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME0_S:
		GXS_LoadBG0Scr( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME1_S:
		GXS_LoadBG1Scr( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME2_S:
		GXS_LoadBG2Scr( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME3_S:
		GXS_LoadBG3Scr( src, ofs, siz ); 
		return;
	}
#else
	switch( frmnum ){
	case GFL_BG_FRAME0_M:
		MI_CpuCopy32(src,(void*)((u32)G2_GetBG0ScrPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME1_M:
		MI_CpuCopy32(src,(void*)((u32)G2_GetBG1ScrPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME2_M:
		MI_CpuCopy32(src,(void*)((u32)G2_GetBG2ScrPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME3_M:
		MI_CpuCopy32(src,(void*)((u32)G2_GetBG3ScrPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME0_S:
		MI_CpuCopy32(src,(void*)((u32)G2S_GetBG0ScrPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME1_S:
		MI_CpuCopy32(src,(void*)((u32)G2S_GetBG1ScrPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME2_S:
		MI_CpuCopy32(src,(void*)((u32)G2S_GetBG2ScrPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME3_S:
		MI_CpuCopy32(src,(void*)((u32)G2S_GetBG3ScrPtr() + ofs),siz);
		return;
	}
#endif
}

//--------------------------------------------------------------------------------------------
/**
 *	スクリーンデータをバッファにコピー
 *
 * @param	frmnum		BGフレーム
 * @param	dat			指定データ
 * @param	datasizpx	データサイズ
 *
 * @retrn	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScreenBufSet( u8 frmnum, const void * dat, u32 datasiz )
{
	GFL_BG_DataDecord( dat, bgl->bgsys[frmnum].screen_buf, datasiz );
}


//--------------------------------------------------------------------------------------------
/**
 * キャラクター転送
 *
 * @param	frmnum		BGフレーム番号
 * @param	src			転送するデータ
 * @param	datasiz		転送サイズ
 * @param	offs		オフセット
 *
 * @return	none
 *
 * @li	datasiz = GFL_BG_DATA_LZH : 圧縮データ
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_LoadCharacter( u8 frmnum, const void * src, u32 datasiz, u32 offs )
{
	if( bgl->bgsys[frmnum].col_mode == GX_BG_COLORMODE_16 ){
		LoadCharacter( frmnum, src, datasiz, offs * GFL_BG_1CHRDATASIZ );
	}else{
		LoadCharacter( frmnum, src, datasiz, offs * GFL_BG_8BITCHRSIZ );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * キャラクター転送（ファイル参照）
 *
 * @param	frmnum		BGフレーム番号
 * @param	src			転送するデータ
 * @param	datasiz		転送サイズ
 * @param	offs		オフセット
 *
 * @return	none
 *
 *	圧縮未対応
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_LoadCharacterFile( u8 frmnum, const char * path, u32 offs )
{
	void * mem;
	u32	size;

	mem = LoadFileEx( bgl->heapID, path, &size );
	if(mem == NULL){
		return;	//エラー
	}

	GFL_BG_CharAreaSet( frmnum, offs, size );

	GFL_BG_LoadCharacter( frmnum, mem, size, offs );
	GFL_HEAP_FreeMemory( mem );
	return;
}

//--------------------------------------------------------------------------------------------
/**
 * キャラデータ展開
 *
 * @param	frmnum		BGフレーム番号
 * @param	src			転送するデータ
 * @param	datasiz		転送サイズ
 * @param	offs		オフセット
 *
 * @return	none
 *
 * @li	datasiz = GFL_BG_DATA_LZH : 圧縮データ
 */
//--------------------------------------------------------------------------------------------
static void LoadCharacter( u8 frmnum, const void * src, u32 datasiz, u32 offs )
{
	void * decode_buf;

	if( datasiz == GFL_BG_DATA_LZH ){
		u32	alloc_siz;

		alloc_siz  = ((*(u32*)src) >> 8);
		decode_buf = GFL_HEAP_AllocMemoryLow( bgl->heapID, alloc_siz );

#ifdef	OSP_ERR_BGL_DECODEBUF_GET		// 展開領域確保失敗
		if( decode_buf == NULL ){
			OS_Printf( "領域確保失敗\n" );
		}
#endif	// OSP_ERR_BGL_DECODEBUF_GET

		GFL_BG_DataDecord( src, decode_buf, datasiz );

		GFL_BG_LoadCharacterSub( frmnum, decode_buf, offs, alloc_siz );

		GFL_HEAP_FreeMemory( decode_buf );
	}else{
		GFL_BG_LoadCharacterSub( frmnum, (void*)src, offs, datasiz );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * キャラクター転送
 *
 * @param	frmnum		BGフレーム番号
 * @param	src			転送するデータ
 * @param	offs		オフセット
 * @param	siz			転送サイズ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void GFL_BG_LoadCharacterSub( u8 frmnum, void* src, u32 ofs, u32 siz )
{
#if DMA_USE
	DC_FlushRange( src, siz );

	switch( frmnum ){
	case GFL_BG_FRAME0_M:
		GX_LoadBG0Char( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME1_M:
		GX_LoadBG1Char( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME2_M:
		GX_LoadBG2Char( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME3_M:
		GX_LoadBG3Char( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME0_S:
		GXS_LoadBG0Char( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME1_S:
		GXS_LoadBG1Char( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME2_S:
		GXS_LoadBG2Char( src, ofs, siz ); 
		return;
	case GFL_BG_FRAME3_S:
		GXS_LoadBG3Char( src, ofs, siz ); 
		return;
	}
#else
	switch( frmnum ){
	case GFL_BG_FRAME0_M:
		MI_CpuCopy32(src,(void*)((u32)G2_GetBG0CharPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME1_M:
		MI_CpuCopy32(src,(void*)((u32)G2_GetBG1CharPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME2_M:
		MI_CpuCopy32(src,(void*)((u32)G2_GetBG2CharPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME3_M:
		MI_CpuCopy32(src,(void*)((u32)G2_GetBG3CharPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME0_S:
		MI_CpuCopy32(src,(void*)((u32)G2S_GetBG0CharPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME1_S:
		MI_CpuCopy32(src,(void*)((u32)G2S_GetBG1CharPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME2_S:
		MI_CpuCopy32(src,(void*)((u32)G2S_GetBG2CharPtr() + ofs),siz);
		return;
	case GFL_BG_FRAME3_S:
		MI_CpuCopy32(src,(void*)((u32)G2S_GetBG3CharPtr() + ofs),siz);
		return;
	}
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * ０クリアされたキャラをセット
 *
 * @param	frmnum		BGフレーム番号
 * @param	datasiz		転送サイズ
 * @param	offs		オフセット
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ClearCharSet( u8 frmnum, u32 datasiz, u32 offs, u32 heap )
{
	u32 * chr = (u32 *)GFL_HEAP_AllocMemoryLow( heap, datasiz );

	memset( chr, 0, datasiz );

	GFL_BG_LoadCharacterSub( frmnum, (void*)chr, offs, datasiz );
	GFL_HEAP_FreeMemory( chr );
}

//--------------------------------------------------------------------------------------------
/**
 * 指定値でクリアされたキャラをセット
 *
 * @param	frmnum		BGフレーム番号
 * @param	clear_code	クリアコード
 * @param	charcnt		クリアするキャラ数
 * @param	offs		キャラ領域先頭からのオフセット（キャラ数）
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_CharFill( u32 frmnum, u32 clear_code, u32 charcnt, u32 offs )
{
	u32 * chr;
	u32  size;

	size = charcnt * bgl->bgsys[frmnum].base_char_size;
	chr = (u32 *)GFL_HEAP_AllocMemoryLow( bgl->heapID,  size );

	if( bgl->bgsys[frmnum].base_char_size == GFL_BG_1CHRDATASIZ ){
		clear_code = (clear_code<<12) | (clear_code<<8) | (clear_code<<4) | clear_code;
		clear_code |= ( clear_code << 16 );
	}else{
		clear_code = (clear_code<<24) | (clear_code<<16) | (clear_code<<8) | clear_code;
	}

	MI_CpuFillFast( chr, clear_code, size);

	GFL_BG_LoadCharacterSub(
		frmnum, (void*)chr, offs*bgl->bgsys[frmnum].base_char_size, size );
	GFL_HEAP_FreeMemory( chr );
}

//--------------------------------------------------------------------------------------------
/**
 * パレット転送
 *
 * @param	frmnum		BGフレーム番号
 * @param	buf			パレットデータ
 * @param	siz			転送サイズ（バイト単位）
 * @param	ofs			オフセット（バイト単位）
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_PaletteSet( u8 frmnum, void * buf, u16 siz, u16 ofs )
{
#if DMA_USE
	DC_FlushRange( (void *)buf, siz );
	if( frmnum < GFL_BG_FRAME0_S ){
		GX_LoadBGPltt( (const void *)buf, ofs, siz );
	}else{
		GXS_LoadBGPltt( (const void *)buf, ofs, siz );
	}
#else
	if( frmnum < GFL_BG_FRAME0_S ){
		MI_CpuCopy16((const void *)buf,(void*)(GF_MMAP_MainBgPlttAddr()+ofs),siz);
	}else{
		MI_CpuCopy16((const void *)buf,(void*)(GF_MMAP_SubBgPlttAddr()+ofs),siz);
	}
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * バックグラウンドカラー転送（パレット０のカラー指定）
 *
 * @param	frmnum		BGフレーム番号
 * @param	col			カラー
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_BackGroundColorSet( u8 frmnum, u16 col )
{
	GFL_BG_PaletteSet( frmnum, &col, 2, 0 );
}


//=============================================================================================
//=============================================================================================
//	スクリーン関連
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * スクリーンデータ位置取得
 *
 * @param	px		Ｘ座標
 * @param	py		Ｙ座標
 * @param	size	スクリーンデータサイズ
 *
 * @return	u16:スクリーン座標index
 *
 * @li	size = GFL_BG_SCRSIZ_128x128 ～ GFL_BG_SCRSIZ_1024x1024
 */
//--------------------------------------------------------------------------------------------
static u16 GetScreenPos( u8 px, u8 py, u8 size )
{
	u16	pos;

	switch( size ){
	case GFL_BG_SCRSIZ_128x128:
		pos = py * 16 + px;
		break;
	case GFL_BG_SCRSIZ_256x256:
	case GFL_BG_SCRSIZ_256x512:
		pos = py * 32 + px;
		break;
	case GFL_BG_SCRSIZ_512x256:
		pos = ( ( px >> 5 ) * 32 + py ) * 32 + ( px & 0x1f );
		break;
	case GFL_BG_SCRSIZ_512x512:
		pos =  ( ( px >> 5 ) + ( py >> 5 ) * 2 );
		pos *= 1024;
		pos += ( py & 0x1f )  * 32 + ( px & 0x1f );
		break;
	case GFL_BG_SCRSIZ_1024x1024:	// 未対応
		pos = 0;
	}
	return pos;
}
//--------------------------------------------------------------------------------------------
/**
 * 折り返し有りスクリーンバッファデータ位置取得
 *
 * @param	px		Ｘ座標
 * @param	py		Ｙ座標
 * @param	sx		スクリーンデータサイズ(キャラ単位)
 * @param	sy		スクリーンデータサイズ(キャラ単位)
 *
 * @retrn	none
 *
 * @li	1x1～64x64charサイズまで対応
 */
//--------------------------------------------------------------------------------------------
static u16 GetScrBufferPos(u8 px,u8 py,u8 sx,u8 sy)
{
	u8	area = 0;
	u16 pos = 0;
	s16	gx = sx-32;	
	s16	gy = sy-32;	

	if(px/32){
		area += 1;
	}
	if(py/32){
		area += 2;
	}
	switch(area){
	case 0:
		if(gx >= 0){
			pos += py*32+px;
		}else{
			pos += py*sx+px;
		}
		break;
	case 1:
		if(gy >= 0){
			pos += 1024;
		}else{
			pos += 32*sy;
		}
		pos += py*gx+(px&0x1F);
		break;
	case 2:
		pos += sx*32;
		if(gx >= 0){
			pos += (py&0x1F)*32+px;
		}else{
			pos += (py&0x1F)*sx+px;
		}
		break;
	case 3:
		pos += (u16)sx*32+(u16)32*gy;
		pos += (py&0x1F)*gx+(px&0x1F);
		break;
	}
	return pos;
}
//--------------------------------------------------------------------------------------------
/**
 * スクリーンデータ書き込み
 *
 * @param	frmnum		BGフレーム
 * @param	buf			書き込むデータ
 * @param	px			書き込み開始Ｘ座標
 * @param	py			書き込み開始Ｙ座標
 * @param	sx			書き込みＸサイズ
 * @param	sy			書き込みＹサイズ
 *
 * @retrn	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrWrite( u8 frmnum, const void * buf, u8 px, u8 py, u8 sx, u8 sy )
{
	GFL_BG_ScrWriteExpand( frmnum, px, py, sx, sy, buf, 0, 0, sx, sy );
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンデータ書き込み（拡張版）
 *
 * @param	write_px	書き込み開始Ｘ座標
 * @param	write_px	書き込み開始Ｙ座標
 * @param	write_sx	書き込みＸサイズ
 * @param	write_sy	書き込みＹサイズ
 * @param	buf			読み込みデータ
 * @param	buf_px		読み込み開始Ｘ座標
 * @param	buf_py		読み込み開始Ｙ座標
 * @param	buf_sx		読み込みデータＸサイズ（write_sx,syとは違い、読み込みデータの
 * @param	buf_sy		読み込みデータＹサイズ　全体の大きさそのものを代入する）
 *
 * @retrn	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrWriteExpand( u8 frmnum, u8 write_px, u8 write_py, u8 write_sx, u8 write_sy,
				const void * buf, u8 buf_px, u8 buf_py, u8 buf_sx, u8 buf_sy )
{
	if( bgl->bgsys[ frmnum ].mode != GFL_BG_MODE_AFFINE ){
		GFL_BG_ScrWrite_Normal(
			&bgl->bgsys[frmnum], write_px, write_py, write_sx, write_sy,
			(u16 *)buf, buf_px, buf_py, buf_sx, buf_sy , GFL_BG_MODE_1DBUF);
	}else{
		GFL_BG_ScrWrite_Affine(
			&bgl->bgsys[frmnum], write_px, write_py, write_sx, write_sy,
			(u8 *)buf, buf_px, buf_py, buf_sx, buf_sy , GFL_BG_MODE_1DBUF);
	}
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンデータ書き込み(折り返しありデータ版)
 *
 * @param	frmnum		BGフレーム
 * @param	write_px	書き込み開始Ｘ座標
 * @param	write_px	書き込み開始Ｙ座標
 * @param	write_sx	書き込みＸサイズ
 * @param	write_sy	書き込みＹサイズ
 * @param	buf			読み込みデータ
 * @param	buf_px		読み込み開始Ｘ座標
 * @param	buf_py		読み込み開始Ｙ座標
 * @param	buf_sx		読み込みデータＸサイズ（write_sx,syとは違い、読み込みデータの
 * @param	buf_sy		読み込みデータＹサイズ　全体の大きさそのものを代入する）
 *
 * @retrn	none
 *
 * @li	折り返し有り1x1～64x64キャラのフリーサイズスクリーンデータの矩形書き込み
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrWriteFree( u8 frmnum, u8 write_px, u8 write_py, u8 write_sx, u8 write_sy,
				const void * buf, u8 buf_px, u8 buf_py, u8 buf_sx, u8 buf_sy )
{
	if( bgl->bgsys[ frmnum ].mode != GFL_BG_MODE_AFFINE ){
		GFL_BG_ScrWrite_Normal(
			&bgl->bgsys[frmnum], write_px, write_py, write_sx, write_sy,
			(u16 *)buf, buf_px, buf_py, buf_sx, buf_sy, GFL_BG_MODE_2DBUF);
	}else{
		GFL_BG_ScrWrite_Affine(
			&bgl->bgsys[frmnum], write_px, write_py, write_sx, write_sy,
			(u8 *)buf, buf_px, buf_py, buf_sx, buf_sy, GFL_BG_MODE_2DBUF);
	}
}


//--------------------------------------------------------------------------------------------
/**
 * スクリーンデータ書き込み（テキスト面、アフィン拡張面用）
 *
 * @param	GFL_BG_ScrWriteExpand+
 * @param	mode	u8:バッファのデータモード
 *					0:GFL_BG_MODE_1DBUF(一次元配列データ)
 *					1:GFL_BG_MODE_2DBUF(折り返し有りのデータ)
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void GFL_BG_ScrWrite_Normal(
					GFL_BG_SYS * ini, u8 write_px, u8 write_py, u8 write_sx, u8 write_sy,
					u16 * buf, u8 buf_px, u8 buf_py, u8 buf_sx, u8 buf_sy ,u8 mode)
{
	u16 * scrn;
	u8	scr_sx, scr_sy;
	u8	i, j;

	if( ini->screen_buf == NULL ){
		return;
	}
	scrn = (u16 *)ini->screen_buf;

	BgScreenSizeGet( ini->screen_siz, &scr_sx, &scr_sy );

	if(mode == GFL_BG_MODE_1DBUF){	//折り返しなしデータ処理
		for( i=0; i<write_sy; i++ ){
			if( (write_py+i) >= scr_sy || (buf_py+i) >= buf_sy ){ break; }
			for( j=0; j<write_sx; j++ ){
				if( (write_px+j) >= scr_sx || (buf_px+j) >= buf_sx ){ break; }

				scrn[ GetScreenPos(write_px+j,write_py+i,ini->screen_siz) ] =
														buf[ (buf_py+i)*buf_sx+buf_px+j ];
			}
		}
	}else{	//折り返し有りデータ処理
		for( i=0; i<write_sy; i++ ){
			if( (write_py+i) >= scr_sy || (buf_py+i) >= buf_sy ){ break; }
			for( j=0; j<write_sx; j++ ){
				if( (write_px+j) >= scr_sx || (buf_px+j) >= buf_sx ){ break; }
				scrn[ GetScreenPos(write_px+j,write_py+i,ini->screen_siz) ] =
								buf[ GetScrBufferPos(buf_px+j,buf_py+i,buf_sx,buf_sy)];
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンデータ書き込み（拡縮面用）
 *
 * @param	GFL_BG_ScrWriteExpand　＋
 * @param	mode	u8:バッファのデータモード
 *					0:GFL_BG_MODE_1DBUF(一次元配列データ)
 *					1:GFL_BG_MODE_2DBUF(折り返し有りのデータ)
 *
 * @retrn	none
 */
//--------------------------------------------------------------------------------------------
static void GFL_BG_ScrWrite_Affine(
					GFL_BG_SYS * ini, u8 write_px, u8 write_py, u8 write_sx, u8 write_sy,
					u8 * buf, u8 buf_px, u8 buf_py, u8 buf_sx, u8 buf_sy ,u8 mode)
{
	u8 * scrn;
	u8	scr_sx, scr_sy;
	u8	i, j;

	if( ini->screen_buf == NULL ){
		return;
	}
	scrn = (u8 *)ini->screen_buf;

	BgScreenSizeGet( ini->screen_siz, &scr_sx, &scr_sy );

	if(mode == GFL_BG_MODE_1DBUF){	//折り返しなしデータ処理
		for( i=0; i<write_sy; i++ ){
			if( (write_py+i) >= scr_sy || (buf_py+i) >= buf_sy ){ break; }
			for( j=0; j<write_sx; j++ ){
				if( (write_px+j) >= scr_sx || (buf_px+j) >= buf_sx ){ break; }

				scrn[ GetScreenPos( write_px+j, write_py+i, ini->screen_siz ) ] =
														buf[ (buf_py+i)*buf_sx + buf_px+j ];
			}
		}
	}else{	//折り返しありデータ処理
		for( i=0; i<write_sy; i++ ){
			if( (write_py+i) >= scr_sy || (buf_py+i) >= buf_sy ){ break; }
			for( j=0; j<write_sx; j++ ){
				if( (write_px+j) >= scr_sx || (buf_px+j) >= buf_sx ){ break; }

				scrn[ GetScreenPos( write_px+j, write_py+i, ini->screen_siz ) ] =
								buf[ GetScrBufferPos(buf_px+j,buf_py+i,buf_sx,buf_sy)];
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンデータバッファ埋め尽くし
 *
 * @param	frmnum		BGフレーム
 * @param	dat			指定データ
 * @param	px			書き込み開始Ｘ座標
 * @param	py			書き込み開始Ｙ座標
 * @param	sx			書き込みＸサイズ
 * @param	sy			書き込みＹサイズ
 * @param	mode		パレット番号など
 *
 * @retrn	none
 *
 * @li	mode = GFL_BG_SCRWRT_PALNL : 現在のスクリーンのパレットを引き継ぐ
 * @li	mode = GFL_BG_SCRWRT_PALIN : datにパレットデータを含む
 * @li	mode = 0 ～ 15 : パレット番号
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrFill( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
{
	if( bgl->bgsys[ frmnum ].mode != GFL_BG_MODE_AFFINE ){
		GFL_BG_ScrFill_Normal( &bgl->bgsys[frmnum], dat, px, py, sx, sy, mode );
	}else{
		GFL_BG_ScrFill_Affine( &bgl->bgsys[frmnum], (u8)dat, px, py, sx, sy );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンデータバッファ埋め尽くし（テキスト面、アフィン拡張面用）
 *
 * @param	GFL_BG_ScrFillと同じ
 *
 * @retrn	none
 */
//--------------------------------------------------------------------------------------------
static void GFL_BG_ScrFill_Normal(
				GFL_BG_SYS * ini, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
{
	u16 * scrn;
	u8	scr_sx, scr_sy;
	u8	i, j;

	if( ini->screen_buf == NULL ){
		return;
	}
	scrn = (u16 *)ini->screen_buf;

	BgScreenSizeGet( ini->screen_siz, &scr_sx, &scr_sy );

	for( i=py; i<py+sy; i++ ){
		if( i >= scr_sy ){ break; }
		for( j=px; j<px+sx; j++ ){
			if( j >= scr_sx ){ break; }
			{
				u16	scr_pos = GetScreenPos( j, i, ini->screen_siz );

				if( mode == GFL_BG_SCRWRT_PALIN ){
					scrn[ scr_pos ] = dat;
				}else if( mode == GFL_BG_SCRWRT_PALNL ){
					scrn[ scr_pos ] = ( scrn[scr_pos] & 0xf000 ) + dat;
				}else{
					scrn[ scr_pos ] = ( mode << 12 ) + dat;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンデータバッファ埋め尽くし（拡縮面用）
 *
 * @param	GFL_BG_ScrFillと同じ
 *
 * @retrn	none
 */
//--------------------------------------------------------------------------------------------
static void GFL_BG_ScrFill_Affine(
				GFL_BG_SYS * ini, u8 dat, u8 px, u8 py, u8 sx, u8 sy )
{
	u8 * scrn;
	u8	scr_sx, scr_sy;
	u8	i, j;

	if( ini->screen_buf == NULL ){
		return;
	}
	scrn = (u8 *)ini->screen_buf;

	BgScreenSizeGet( ini->screen_siz, &scr_sx, &scr_sy );

	for( i=py; i<py+sy; i++ ){
		if( i >= scr_sy ){ break; }
		for( j=px; j<px+sx; j++ ){
			if( j >= scr_sx ){ break; }
			scrn[ GetScreenPos( j, i, ini->screen_siz ) ] = dat;
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 指定フレームのスクリーンの指定位置のパレットを変更する
 *
 * @param	frmnum	BGフレーム番号
 * @param	px		開始X位置
 * @param	py		開始Y位置
 * @param	sx		Xサイズ
 * @param	sy		Yサイズ
 * @param	pal		パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrPalChange( u8 frmnum, u8 px, u8 py, u8 sx, u8 sy, u8 pal )
{
	u16 * scrn;
	u8	scr_sx, scr_sy;
	u8	i, j;

	if( bgl->bgsys[frmnum].screen_buf == NULL ){
		return;
	}

	scrn = (u16 *)bgl->bgsys[frmnum].screen_buf;

	BgScreenSizeGet( bgl->bgsys[frmnum].screen_siz, &scr_sx, &scr_sy );

	for( i=py; i<py+sy; i++ ){
		if( i >= scr_sy ){ break; }
		for( j=px; j<px+sx; j++ ){
			if( j >= scr_sx ){ break; }
			{
				u16	scr_pos = GetScreenPos( j, i, bgl->bgsys[frmnum].screen_siz );

				scrn[scr_pos] = ( scrn[scr_pos] & 0xfff ) | ( pal << 12 );
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンバッファをクリアして転送
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrClear( u8 frmnum )
{
	if( bgl->bgsys[frmnum].screen_buf == NULL ){
		return;
	}
	MI_CpuClear16( bgl->bgsys[frmnum].screen_buf, bgl->bgsys[frmnum].screen_buf_siz );
	GFL_BG_LoadScreenReq( frmnum );
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンバッファを指定コードでクリアして転送
 *
 * @param	frmnum	BGフレーム番号
 * @param   clear_code	クリアコード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrClearCode( u8 frmnum, u16 clear_code )
{
	if( bgl->bgsys[frmnum].screen_buf == NULL ){
		return;
	}
	MI_CpuFill16(
		bgl->bgsys[frmnum].screen_buf, clear_code, bgl->bgsys[frmnum].screen_buf_siz );
	GFL_BG_LoadScreenReq( frmnum );
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーンバッファを指定コードでクリアして転送リクエスト
 *
 * @param	frmnum	BGフレーム番号
 * @param   clear_code	クリアコード
 *
 * @return	none
 *
 * @li	転送はVBlankで
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrClearCodeVReq( u8 frmnum, u16 clear_code )
{
	if( bgl->bgsys[frmnum].screen_buf == NULL ){
		return;
	}
	MI_CpuFill16(
		bgl->bgsys[frmnum].screen_buf, clear_code, bgl->bgsys[frmnum].screen_buf_siz );
	GFL_BG_LoadScreenV_Req( frmnum );
}


//=============================================================================================
//=============================================================================================
//	キャラクター関連
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * キャラデータを取得
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	キャラデータのアドレス
 */
//--------------------------------------------------------------------------------------------
void * GFL_BG_CgxGet( u8 frmnum )
{
	switch( frmnum ){
	case GFL_BG_FRAME0_M:
		return G2_GetBG0CharPtr();
	case GFL_BG_FRAME1_M:
		return G2_GetBG1CharPtr();
	case GFL_BG_FRAME2_M:
		return G2_GetBG2CharPtr();
	case GFL_BG_FRAME3_M:
		return G2_GetBG3CharPtr();
	case GFL_BG_FRAME0_S:
		return G2S_GetBG0CharPtr();
	case GFL_BG_FRAME1_S:
		return G2S_GetBG1CharPtr();
	case GFL_BG_FRAME2_S:
		return G2S_GetBG2CharPtr();
	case GFL_BG_FRAME3_S:
		return G2S_GetBG3CharPtr();
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * 4bitのキャラデータを8bitに変換する（変換先指定）
 *
 * @param	chr			変換元データ（4bitキャラ）
 * @param	chr_size	変換元データのサイズ
 * @param	buf			変換先
 * @param	pal_ofs		元パレット番号 ( 0 ～ 16 )
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_4BitCgxChange8BitMain( const u8 * chr, u32 chr_size, u8 * buf, u8 pal_ofs )
{
	u32	i;

	pal_ofs <<= 4;
	for( i=0; i<chr_size; i++ ){
		buf[i*2] = chr[i] & 0x0f;
		if( buf[i*2] != 0 ){ buf[i*2] += pal_ofs; }

		buf[i*2+1] = ( chr[i] >> 4 ) & 0x0f;
		if( buf[i*2+1] != 0 ){ buf[i*2+1] += pal_ofs; }
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 4bitのキャラデータを8bitに変換する（変換先取得）
 *
 * @param	chr			変換元データ（4bitキャラ）
 * @param	chr_size	変換元データのサイズ
 * @param	pal_ofs		元パレット番号 ( 0 ～ 16 )
 * @param	heap		ヒープID
 *
 * @return	取得したメモリのアドレス
 */
//--------------------------------------------------------------------------------------------
void * GFL_BG_4BitCgxChange8Bit( const u8 * chr, u32 chr_size, u8 pal_ofs, u32 heap )
{
	void * buf;

	buf = GFL_HEAP_AllocMemory( heap, chr_size * 2 );
	GFL_BG_4BitCgxChange8BitMain( chr, chr_size, (u8 *)buf, pal_ofs );
	return buf;
}


//=============================================================================================
//=============================================================================================
//	BGLステータス取得
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（スクリーンバッファアドレス）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	スクリーンバッファアドレス
 */
//--------------------------------------------------------------------------------------------
void * GFL_BG_ScreenAdrsGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].screen_buf;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（スクリーンバッファサイズ）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	スクリーンバッファサイズ
 */
//--------------------------------------------------------------------------------------------
u32 GFL_BG_ScreenSizGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].screen_buf_siz;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（スクリーンバッファオフセット）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	スクリーンバッファオフセット
 */
//--------------------------------------------------------------------------------------------
u32 GFL_BG_ScreenOfsGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].screen_buf_ofs;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（スクリーンタイプ取得）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	GFL_BG_SCRSIZ_128x128 等
 */
//--------------------------------------------------------------------------------------------
u32 GFL_BG_ScreenTypeGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].screen_siz;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（スクロールカウンタX）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	スクロールカウンタX
 */
//--------------------------------------------------------------------------------------------
int GFL_BG_ScreenScrollXGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].scroll_x;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（スクロールカウンタY）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	スクロールカウンタY
 */
//--------------------------------------------------------------------------------------------
int GFL_BG_ScreenScrollYGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].scroll_y;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（BGモード）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	BGモード
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BG_BgModeGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].mode;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（カラーモード）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	カラーモード
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BG_ScreenColorModeGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].col_mode;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（キャラサイズ）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	キャラサイズ
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BG_BaseCharSizeGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].base_char_size;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（回転角度）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	回転角度
 */
//--------------------------------------------------------------------------------------------
u16 GFL_BG_RadianGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].rad;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（X方向の拡縮パラメータ）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	X方向の拡縮パラメータ
 */
//--------------------------------------------------------------------------------------------
fx32 GFL_BG_ScaleXGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].scale_x;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（Y方向の拡縮パラメータ）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	Y方向の拡縮パラメータ
 */
//--------------------------------------------------------------------------------------------
fx32 GFL_BG_ScaleYGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].scale_y;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（回転中心X座標）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	回転中心X座標
 */
//--------------------------------------------------------------------------------------------
int GFL_BG_CenterXGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].cx;
}

//--------------------------------------------------------------------------------------------
/**
 * BGLステータス取得（回転中心Y座標）
 *
 * @param	frmnum	BGフレーム番号
 *
 * @return	回転中心Y座標
 */
//--------------------------------------------------------------------------------------------
int GFL_BG_CenterYGet( u8 frmnum )
{
	return bgl->bgsys[frmnum].cy;
}

//--------------------------------------------------------------------------------------------
/**
 * BGプライオリティ取得
 *
 * @param	frm		BGフレーム番号
 *
 * @return	プライオリティ
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BG_PriorityGet( u8 frm )
{
	switch( frm ){
	case GFL_BG_FRAME0_M:
		{
			GXBg01Control	dat = G2_GetBG0Control();
			return (u8)dat.priority;
		}
	case GFL_BG_FRAME1_M:
		{
			GXBg01Control	dat = G2_GetBG1Control();
			return (u8)dat.priority;
		}
	case GFL_BG_FRAME2_M:
		switch( bgl->bgsys[frm].mode ){
		default:
		case GFL_BG_MODE_TEXT:
			{
				GXBg23ControlText	dat = G2_GetBG2ControlText();
				return (u8)dat.priority;
			}
		case GFL_BG_MODE_AFFINE:
			{
				GXBg23ControlAffine	dat = G2_GetBG2ControlAffine();
				return (u8)dat.priority;
			}
		case GFL_BG_MODE_256X16:
			{
				GXBg23Control256x16Pltt	dat = G2_GetBG2Control256x16Pltt();
				return (u8)dat.priority;
			}
		}
	case GFL_BG_FRAME3_M:
		switch( bgl->bgsys[frm].mode ){
		default:
		case GFL_BG_MODE_TEXT:
			{
				GXBg23ControlText	dat = G2_GetBG3ControlText();
				return (u8)dat.priority;
			}
		case GFL_BG_MODE_AFFINE:
			{
				GXBg23ControlAffine	dat = G2_GetBG3ControlAffine();
				return (u8)dat.priority;
			}
		case GFL_BG_MODE_256X16:
			{
				GXBg23Control256x16Pltt	dat = G2_GetBG3Control256x16Pltt();
				return (u8)dat.priority;
			}
		}
	case GFL_BG_FRAME0_S:
		{
			GXBg01Control	dat = G2S_GetBG0Control();
			return (u8)dat.priority;
		}
	case GFL_BG_FRAME1_S:
		{
			GXBg01Control	dat = G2S_GetBG1Control();
			return (u8)dat.priority;
		}
	case GFL_BG_FRAME2_S:
		switch( bgl->bgsys[frm].mode ){
		default:
		case GFL_BG_MODE_TEXT:
			{
				GXBg23ControlText	dat = G2S_GetBG2ControlText();
				return (u8)dat.priority;
			}
		case GFL_BG_MODE_AFFINE:
			{
				GXBg23ControlAffine	dat = G2S_GetBG2ControlAffine();
				return (u8)dat.priority;
			}
		case GFL_BG_MODE_256X16:
			{
				GXBg23Control256x16Pltt	dat = G2S_GetBG2Control256x16Pltt();
				return (u8)dat.priority;
			}
		}
	case GFL_BG_FRAME3_S:
		switch( bgl->bgsys[frm].mode ){
		default:
		case GFL_BG_MODE_TEXT:
			{
				GXBg23ControlText	dat = G2S_GetBG3ControlText();
				return (u8)dat.priority;
			}
		case GFL_BG_MODE_AFFINE:
			{
				GXBg23ControlAffine	dat = G2S_GetBG3ControlAffine();
				return (u8)dat.priority;
			}
		case GFL_BG_MODE_256X16:
			{
				GXBg23Control256x16Pltt	dat = G2S_GetBG3Control256x16Pltt();
				return (u8)dat.priority;
			}
		}
	}
	return 0;
}





//=============================================================================================
//=============================================================================================
//	NITRO-CHARACTERデータ展開処理
//=============================================================================================
//=============================================================================================

void GFL_BG_NTRCHR_CharLoadEx( u8 frmnum, const char * path, u32 offs, u32 size )
{
	void * buf;
	NNSG2dCharacterData * dat;

	buf = LoadFile( bgl->heapID, path );

#ifdef	OSP_ERR_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータ取得領域確保失敗
	if( buf == NULL ){
		OS_Printf( "ERROR : GF_GBL_NTRCHR_CharLoad -load\n" );
		return;
	}
#endif	// OSP_ERR_BGL_NTRCHR_LOAD

	if( NNS_G2dGetUnpackedBGCharacterData( buf, &dat ) == TRUE ){
		if( size == 0xffffffff ){
#ifdef	OSP_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータサイズ
			OS_Printf( "GF_GBL_NTRCHR_CharLoad -size %d\n", dat->szByte );
#endif	// OSP_BGL_NTRCHR_LOAD
			GFL_BG_LoadCharacter( frmnum, dat->pRawData, dat->szByte, offs );
		}else{
#ifdef	OSP_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータサイズ
			OS_Printf( "GF_GBL_NTRCHR_CharLoad -size %d\n", size );
#endif	// OSP_BGL_NTRCHR_LOAD
			GFL_BG_LoadCharacter( frmnum, dat->pRawData, size, offs );
		}
	}

	GFL_HEAP_FreeMemory( buf );
}

//--------------------------------------------------------------------------------------------
/**
 * NITRO-CHARACTERのキャラデータを読み込む
 *
 * @param	frmnum	BGフレーム番号
 * @param	path	ファイルパス
 * @param	offs	オフセット（キャラ単位）
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_NTRCHR_CharLoad( u8 frmnum, const char * path, u32 offs )
{
	void * buf;
	NNSG2dCharacterData * dat;

	buf = LoadFile( bgl->heapID, path );

#ifdef	OSP_ERR_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータ取得領域確保失敗
	if( buf == NULL ){
		OS_Printf( "ERROR : GF_GBL_NTRCHR_CharLoad -load\n" );
		return;
	}
#endif	// OSP_ERR_BGL_NTRCHR_LOAD

	if( NNS_G2dGetUnpackedBGCharacterData( buf, &dat ) == TRUE ){
#ifdef	OSP_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータサイズ
		OS_Printf( "GF_GBL_NTRCHR_CharLoad -size %d\n", dat->szByte );
#endif	// OSP_BGL_NTRCHR_LOAD
		GFL_BG_LoadCharacter( frmnum, dat->pRawData, dat->szByte, offs );
	}

	GFL_HEAP_FreeMemory( buf );
}




//--------------------------------------------------------------------------------------------
/**
 * NITRO-CHARACTERのキャラデータを取得
 *
 * @param	buf		展開用バッファ
 * @param	mode	指定ヒープ領域定義
 * @param	path	ファイルパス
 *
 * @return	キャラデータの構造体
 */
//--------------------------------------------------------------------------------------------
NNSG2dCharacterData * GFL_BG_NTRCHR_CharGet( void ** buf, int mode, const char * path )
{
	NNSG2dCharacterData * dat;

	*buf = LoadFile( mode, path );

#ifdef	OSP_ERR_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータ取得領域確保失敗
	if( buf == NULL ){
		OS_Printf( "ERROR : GF_GBL_NTRCHR_CharLoad -load\n" );
		return NULL;
	}
#endif	// OSP_ERR_BGL_NTRCHR_LOAD

	if( NNS_G2dGetUnpackedBGCharacterData( *buf, &dat ) == TRUE ){
#ifdef	OSP_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータサイズ
		OS_Printf( "GF_GBL_NTRCHR_CharLoad -size %d\n", dat->szByte );
#endif	// OSP_BGL_NTRCHR_LOAD
	}

	return dat;
}

//--------------------------------------------------------------------------------------------
/**
 * NITRO-CHARACTERのパレットデータを展開
 *
 * @param	mem		展開場所
 * @param	mode	指定ヒープ領域定義
 * @param	path	ファイルパス
 *
 * @return	パレットデータ
 *
 * @li		pal->pRawData = パレットデータ
 * @li		pal->szByte   = サイズ
 */
//--------------------------------------------------------------------------------------------
NNSG2dPaletteData * GFL_BG_NTRCHR_PalLoad( void ** mem, int mode, const char * path )
{
	NNSG2dPaletteData * pal;

	*mem = LoadFile( mode, path );

#ifdef	OSP_ERR_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータ取得領域確保失敗
	if( mem == NULL ){
		OS_Printf( "ERROR : GF_GBL_NTRCHR_PalLoad -load\n" );
		return NULL;
	}
#endif	// OSP_ERR_BGL_NTRCHR_LOAD

	if( NNS_G2dGetUnpackedPaletteData( *mem, &pal ) == TRUE ){
#ifdef	OSP_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータサイズ
		OS_Printf( "GF_GBL_NTRCHR_PalLoad -size %d\n", pal->szByte );
#endif	// OSP_BGL_NTRCHR_LOAD
	}
	return pal;
}

//--------------------------------------------------------------------------------------------
/**
 * NITRO-CHARACTERのスクリーンデータを読み込む
 *
 * @param	frmnum	BGフレーム番号
 * @param	path	ファイルパス
 * @param	offs	オフセット（キャラ単位）
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_NTRCHR_ScrnLoad( u8 frmnum, const char * path, u32 offs )
{
	void * buf;
	NNSG2dScreenData * dat;

	buf = LoadFile( bgl->heapID, path );

#ifdef	OSP_ERR_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータ取得領域確保失敗
	if( buf == NULL ){
		OS_Printf( "ERROR : GF_GBL_NTRCHR_ScrnLoad -load\n" );
		return;
	}
#endif	// OSP_ERR_BGL_NTRCHR_LOAD

	if( NNS_G2dGetUnpackedScreenData( buf, &dat ) == TRUE ){
#ifdef	OSP_BGL_NTRCHR_LOAD		// NITRO-CHARACTERのデータサイズ
		OS_Printf( "GF_GBL_NTRCHR_ScrnLoad -size %d\n", dat->szByte );
#endif	// OSP_BGL_NTRCHR_LOAD
		GFL_BG_ScreenBufSet( frmnum, dat->rawData, dat->szByte );
		GFL_BG_LoadScreen( frmnum, dat->rawData, dat->szByte, offs );
	}

	GFL_HEAP_FreeMemory( buf );
}


//=============================================================================================
//=============================================================================================
//	VBlank関連
//=============================================================================================
//=============================================================================================
#define	SCROLL_REQ_M0	( 1 )			// Bit Flag ( Main Disp BG0 )
#define	SCROLL_REQ_M1	( 2 )			// Bit Flag ( Main Disp BG1 )
#define	SCROLL_REQ_M2	( 4 )			// Bit Flag ( Main Disp BG2 )
#define	SCROLL_REQ_M3	( 8 )			// Bit Flag ( Main Disp BG3 )
#define	SCROLL_REQ_S0	( 16 )			// Bit Flag ( Sub Disp BG0 )
#define	SCROLL_REQ_S1	( 32 )			// Bit Flag ( Sub Disp BG1 )
#define	SCROLL_REQ_S2	( 64 )			// Bit Flag ( Sub Disp BG2 )
#define	SCROLL_REQ_S3	( 128 )			// Bit Flag ( Sub Disp BG3 )

static void VBlankScroll( void );
static void VBlankLoadScreen( void );

//--------------------------------------------------------------------------------------------
/**
 * VBlank転送
 *
 * @param	none
 *
 * @return	none
 *
 * @li	VBlank内で呼んで下さい
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_VBlankFunc( void )
{
	VBlankScroll();		// スクロール
	VBlankLoadScreen();	// スクリーン転送

	bgl->scroll_req = 0;
	bgl->loadscrn_req = 0;
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーン転送 ( VBlank )
 *
 * @param	none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankLoadScreen( void )
{
	// Main Disp BG0
	if( ( bgl->loadscrn_req & SCROLL_REQ_M0 ) != 0 ){
		GFL_BG_LoadScreenSub(GFL_BG_FRAME0_M,
			bgl->bgsys[GFL_BG_FRAME0_M].screen_buf,
			bgl->bgsys[GFL_BG_FRAME0_M].screen_buf_ofs * GFL_BG_1SCRDATASIZ,
			bgl->bgsys[GFL_BG_FRAME0_M].screen_buf_siz );
	}
	// Main Disp BG1
	if( ( bgl->loadscrn_req & SCROLL_REQ_M1 ) != 0 ){
		GFL_BG_LoadScreenSub(GFL_BG_FRAME1_M,
			bgl->bgsys[GFL_BG_FRAME1_M].screen_buf,
			bgl->bgsys[GFL_BG_FRAME1_M].screen_buf_ofs * GFL_BG_1SCRDATASIZ,
			bgl->bgsys[GFL_BG_FRAME1_M].screen_buf_siz );
	}
	// Main Disp BG2
	if( ( bgl->loadscrn_req & SCROLL_REQ_M2 ) != 0 ){
		GFL_BG_LoadScreenSub(GFL_BG_FRAME2_M,
			bgl->bgsys[GFL_BG_FRAME2_M].screen_buf,
			bgl->bgsys[GFL_BG_FRAME2_M].screen_buf_ofs * GFL_BG_1SCRDATASIZ,
			bgl->bgsys[GFL_BG_FRAME2_M].screen_buf_siz );
	}
	// Main Disp BG3
	if( ( bgl->loadscrn_req & SCROLL_REQ_M3 ) != 0 ){
		GFL_BG_LoadScreenSub(GFL_BG_FRAME3_M,
			bgl->bgsys[GFL_BG_FRAME3_M].screen_buf,
			bgl->bgsys[GFL_BG_FRAME3_M].screen_buf_ofs * GFL_BG_1SCRDATASIZ,
			bgl->bgsys[GFL_BG_FRAME3_M].screen_buf_siz );
	}
	// Sub Disp BG0
	if( ( bgl->loadscrn_req & SCROLL_REQ_S0 ) != 0 ){
		GFL_BG_LoadScreenSub(GFL_BG_FRAME0_S,
			bgl->bgsys[GFL_BG_FRAME0_S].screen_buf,
			bgl->bgsys[GFL_BG_FRAME0_S].screen_buf_ofs * GFL_BG_1SCRDATASIZ,
			bgl->bgsys[GFL_BG_FRAME0_S].screen_buf_siz );
	}
	// Sub Disp BG1
	if( ( bgl->loadscrn_req & SCROLL_REQ_S1 ) != 0 ){
		GFL_BG_LoadScreenSub(GFL_BG_FRAME1_S,
			bgl->bgsys[GFL_BG_FRAME1_S].screen_buf,
			bgl->bgsys[GFL_BG_FRAME1_S].screen_buf_ofs * GFL_BG_1SCRDATASIZ,
			bgl->bgsys[GFL_BG_FRAME1_S].screen_buf_siz );
	}
	// Sub Disp BG2
	if( ( bgl->loadscrn_req & SCROLL_REQ_S2 ) != 0 ){
		GFL_BG_LoadScreenSub(GFL_BG_FRAME2_S,
			bgl->bgsys[GFL_BG_FRAME2_S].screen_buf,
			bgl->bgsys[GFL_BG_FRAME2_S].screen_buf_ofs * GFL_BG_1SCRDATASIZ,
			bgl->bgsys[GFL_BG_FRAME2_S].screen_buf_siz );
	}
	// Sub Disp BG3
	if( ( bgl->loadscrn_req & SCROLL_REQ_S3 ) != 0 ){
		GFL_BG_LoadScreenSub(GFL_BG_FRAME3_S,
			bgl->bgsys[GFL_BG_FRAME3_S].screen_buf,
			bgl->bgsys[GFL_BG_FRAME3_S].screen_buf_ofs * GFL_BG_1SCRDATASIZ,
			bgl->bgsys[GFL_BG_FRAME3_S].screen_buf_siz );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * スクリーン転送リクエスト ( VBlank )
 *
 * @param	frmnum		BGフレーム番号
 *
 * @return	none
 *
 * @li	VBlank内でGFL_BG_VBlankFunc(...)を呼ぶこと
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_LoadScreenV_Req( u8 frmnum )
{
	bgl->loadscrn_req |= ( 1 << frmnum );
}

//--------------------------------------------------------------------------------------------
/**
 * VBlankスクロール
 *
 * @param	none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankScroll( void )
{
	// Main Disp BG0
	if( ( bgl->scroll_req & SCROLL_REQ_M0 ) != 0 ){
		G2_SetBG0Offset(
			bgl->bgsys[GFL_BG_FRAME0_M].scroll_x,
			bgl->bgsys[GFL_BG_FRAME0_M].scroll_y );
	}
	// Main Disp BG1
	if( ( bgl->scroll_req & SCROLL_REQ_M1 ) != 0 ){
		G2_SetBG1Offset(
			bgl->bgsys[GFL_BG_FRAME1_M].scroll_x,
			bgl->bgsys[GFL_BG_FRAME1_M].scroll_y );
	}
	// Main Disp BG2
	if( ( bgl->scroll_req & SCROLL_REQ_M2 ) != 0 ){
		if( bgl->bgsys[GFL_BG_FRAME2_M].mode == GFL_BG_MODE_TEXT ){
			G2_SetBG2Offset(
				bgl->bgsys[GFL_BG_FRAME2_M].scroll_x,
				bgl->bgsys[GFL_BG_FRAME2_M].scroll_y );
		}else{
			MtxFx22	mtx;

			AffineMtxMake_2D(
				&mtx,
				bgl->bgsys[GFL_BG_FRAME2_M].rad,
				bgl->bgsys[GFL_BG_FRAME2_M].scale_x,
				bgl->bgsys[GFL_BG_FRAME2_M].scale_y,
				AFFINE_MAX_360 );
			G2_SetBG2Affine(
				&mtx,
				bgl->bgsys[GFL_BG_FRAME2_M].cx,
				bgl->bgsys[GFL_BG_FRAME2_M].cy,
				bgl->bgsys[GFL_BG_FRAME2_M].scroll_x,
				bgl->bgsys[GFL_BG_FRAME2_M].scroll_y );
		}
	}
	// Main Disp BG3
	if( ( bgl->scroll_req & SCROLL_REQ_M3 ) != 0 ){
		if( bgl->bgsys[GFL_BG_FRAME3_M].mode == GFL_BG_MODE_TEXT ){
			G2_SetBG3Offset(
				bgl->bgsys[GFL_BG_FRAME3_M].scroll_x,
				bgl->bgsys[GFL_BG_FRAME3_M].scroll_y );
		}else{
			MtxFx22	mtx;

			AffineMtxMake_2D(
				&mtx,
				bgl->bgsys[GFL_BG_FRAME3_M].rad,
				bgl->bgsys[GFL_BG_FRAME3_M].scale_x,
				bgl->bgsys[GFL_BG_FRAME3_M].scale_y,
				AFFINE_MAX_360 );
			G2_SetBG3Affine(
				&mtx,
				bgl->bgsys[GFL_BG_FRAME3_M].cx,
				bgl->bgsys[GFL_BG_FRAME3_M].cy,
				bgl->bgsys[GFL_BG_FRAME3_M].scroll_x,
				bgl->bgsys[GFL_BG_FRAME3_M].scroll_y );
		}
	}
	// Sub Disp BG0
	if( ( bgl->scroll_req & SCROLL_REQ_S0 ) != 0 ){
		G2S_SetBG0Offset(
			bgl->bgsys[GFL_BG_FRAME0_S].scroll_x, bgl->bgsys[GFL_BG_FRAME0_S].scroll_y );
	}
	// Sub Disp BG1
	if( ( bgl->scroll_req & SCROLL_REQ_S1 ) != 0 ){
		G2S_SetBG1Offset(
			bgl->bgsys[GFL_BG_FRAME1_S].scroll_x, bgl->bgsys[GFL_BG_FRAME1_S].scroll_y );
	}
	// Sub Disp BG2
	if( ( bgl->scroll_req & SCROLL_REQ_S2 ) != 0 ){
		if( bgl->bgsys[GFL_BG_FRAME2_S].mode == GFL_BG_MODE_TEXT ){
			G2S_SetBG2Offset(
				bgl->bgsys[GFL_BG_FRAME2_S].scroll_x,
				bgl->bgsys[GFL_BG_FRAME2_S].scroll_y );
		}else{
			MtxFx22	mtx;

			AffineMtxMake_2D(
				&mtx,
				bgl->bgsys[GFL_BG_FRAME2_S].rad,
				bgl->bgsys[GFL_BG_FRAME2_S].scale_x,
				bgl->bgsys[GFL_BG_FRAME2_S].scale_y,
				AFFINE_MAX_360 );
			G2S_SetBG2Affine(
				&mtx,
				bgl->bgsys[GFL_BG_FRAME2_S].cx,
				bgl->bgsys[GFL_BG_FRAME2_S].cy,
				bgl->bgsys[GFL_BG_FRAME2_S].scroll_x,
				bgl->bgsys[GFL_BG_FRAME2_S].scroll_y );
		}
	}
	// Sub Disp BG3
	if( ( bgl->scroll_req & SCROLL_REQ_S3 ) != 0 ){
		if( bgl->bgsys[GFL_BG_FRAME3_S].mode == GFL_BG_MODE_TEXT ){
			G2S_SetBG3Offset(
				bgl->bgsys[GFL_BG_FRAME3_S].scroll_x,
				bgl->bgsys[GFL_BG_FRAME3_S].scroll_y );
		}else{
			MtxFx22	mtx;

			AffineMtxMake_2D(
				&mtx,
				bgl->bgsys[GFL_BG_FRAME3_S].rad,
				bgl->bgsys[GFL_BG_FRAME3_S].scale_x,
				bgl->bgsys[GFL_BG_FRAME3_S].scale_y,
				AFFINE_MAX_360 );
			G2S_SetBG3Affine(
				&mtx,
				bgl->bgsys[GFL_BG_FRAME3_S].cx,
				bgl->bgsys[GFL_BG_FRAME3_S].cy,
				bgl->bgsys[GFL_BG_FRAME3_S].scroll_x,
				bgl->bgsys[GFL_BG_FRAME3_S].scroll_y );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * スクロールリクエスト
 *
 * @param	frmnum		BGフレーム番号
 * @param	mode		スクロールモード
 * @param	value		スクロール値
 *
 * @return	none
 *
 * @li	VBlank内でGFL_BG_VBlankFunc(...)を呼ぶこと
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScrollReq( u8 frmnum, u8 mode, int value )
{
	ScrollParamSet( &bgl->bgsys[frmnum], mode, value );
	bgl->scroll_req |= ( 1 << frmnum );
}

//--------------------------------------------------------------------------------------------
/**
 * 回転角度変更リクエスト
 *
 * @param	frmnum		BGフレーム番号
 * @param	mode		角度変更モード
 * @param	value		回転値
 *
 * @return	none
 *
 * @li	VBlank内でGFL_BG_VBlankFunc(...)を呼ぶこと
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_RadianSetReq( u8 frmnum, u8 mode, u16 value )
{
	RadianParamSet( &bgl->bgsys[frmnum], mode, value );
	bgl->scroll_req |= ( 1 << frmnum );
}

//--------------------------------------------------------------------------------------------
/**
 * 回転角度セット
 *
 * @param	mode		角度変更モード
 * @param	value		回転値
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void RadianParamSet( GFL_BG_SYS * ini, u8 mode, u16 value )
{
	switch( mode ){
	case GFL_BG_RADION_SET:
		ini->rad = value;
		break;
	case GFL_BG_RADION_INC:
		ini->rad += value;
		break;
	case GFL_BG_RADION_DEC:
		ini->rad -= value;
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 拡縮変更リクエスト
 *
 * @param	frmnum		BGフレーム番号
 * @param	mode		拡縮変更モード
 * @param	value		変更値
 *
 * @return	none
 *
 * @li	VBlank内でGFL_BG_VBlankFunc(...)を呼ぶこと
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_ScaleSetReq( u8 frmnum, u8 mode, fx32 value )
{
	ScaleParamSet( &bgl->bgsys[frmnum], mode, value );
	bgl->scroll_req |= ( 1 << frmnum );
}

//--------------------------------------------------------------------------------------------
/**
 * 拡縮変更セット
 *
 * @param	mode		拡縮変更モード
 * @param	value		変更値
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ScaleParamSet( GFL_BG_SYS * ini, u8 mode, fx32 value )
{
	switch( mode ){
	case GFL_BG_SCALE_X_SET:
		ini->scale_x = value;
		break;
	case GFL_BG_SCALE_X_INC:
		ini->scale_x += value;
		break;
	case GFL_BG_SCALE_X_DEC:
		ini->scale_x -= value;
		break;
	case GFL_BG_SCALE_Y_SET:
		ini->scale_y = value;
		break;
	case GFL_BG_SCALE_Y_INC:
		ini->scale_y += value;
		break;
	case GFL_BG_SCALE_Y_DEC:
		ini->scale_y -= value;
		break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 回転中心座標変更リクエスト
 *
 * @param	frmnum		BGフレーム番号
 * @param	mode		変更モード
 * @param	value		変更値
 *
 * @return	none
 *
 * @li	VBlank内でGFL_BG_VBlankFunc(...)を呼ぶこと
 */
//--------------------------------------------------------------------------------------------
void GFL_BG_RotateCenterSetReq( u8 frmnum, u8 mode, int value )
{
	CenterParamSet( &bgl->bgsys[frmnum], mode, value );
	bgl->scroll_req |= ( 1 << frmnum );
}

//--------------------------------------------------------------------------------------------
/**
 * 回転中心座標変更セット
 *
 * @param	mode		変更モード
 * @param	value		変更値
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CenterParamSet( GFL_BG_SYS * ini, u8 mode, int value )
{
	switch( mode ){
	case GFL_BG_CENTER_X_SET:
		ini->cx = value;
		break;
	case GFL_BG_CENTER_X_INC:
		ini->cx += value;
		break;
	case GFL_BG_CENTER_X_DEC:
		ini->cx -= value;
		break;
	case GFL_BG_CENTER_Y_SET:
		ini->cy = value;
		break;
	case GFL_BG_CENTER_Y_INC:
		ini->cy += value;
		break;
	case GFL_BG_CENTER_Y_DEC:
		ini->cy -= value;
		break;
	}
}


//=============================================================================================
//=============================================================================================
//	その他
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 指定座標のドットをチェック
 *
 * @param	frmnum	BGフレーム番号
 * @param	px		X座標
 * @param	py		Y座標
 * @param	pat		判定データ
 *
 * @retval	"TRUE = 判定データ(pat)にあり"
 * @retval	"FALSE = 判定データ(pat)になし"
 *
 * @li	１６色の場合、patはビットでチェック
 *			例）*pat = 0xfffd;
 *				カラー0と1の場合、FALSEが返る
 *
 * @li	２５６色の場合、patの最後に0xffffを付加
 *			例）pat[] = { 88, 124, 223, 0xffff };
 *				カラー88, 124, 223以外はFALSEが返る
 */
//--------------------------------------------------------------------------------------------
u8 GFL_BG_DotCheck( u8 frmnum, u16 px, u16 py, u16 * pat )
{
	u8 * cgx;
	u16	pos;
	u8	chr_x, chr_y;
	u8	dot;
	u8	i;

	if( bgl->bgsys[frmnum].screen_buf == NULL ){
		return FALSE;
	}

	pos = GetScreenPos( (u8)(px>>3), (u8)(py>>3), bgl->bgsys[frmnum].screen_siz );
	cgx = (u8 *)GFL_BG_CgxGet( frmnum );

	chr_x = (u8)(px&7);
	chr_y = (u8)(py&7);

	if( bgl->bgsys[frmnum].col_mode == GX_BG_COLORMODE_16 ){

		u16 * scrn;
		u8 * buf;

		scrn = (u16 *)bgl->bgsys[frmnum].screen_buf;
		buf  = GFL_HEAP_AllocMemoryLow( bgl->heapID, 64 );

		cgx += ( ( scrn[pos] & 0x3ff ) << 5 );
		for( i=0; i<32; i++ ){
			buf[ (i<<1) ]   = cgx[i] & 0x0f;
			buf[ (i<<1)+1 ] = cgx[i] >> 4;
		}

		CgxFlipCheck( (u8)((scrn[pos]>>10)&3), buf ,bgl->heapID);

		dot = buf[ chr_x+(chr_y<<3) ];
		GFL_HEAP_FreeMemory( buf );

		if( ( pat[0] & (1<<dot) ) != 0 ){
			return TRUE;
		}

	}else{
		if( bgl->bgsys[ frmnum ].mode != GFL_BG_MODE_AFFINE ){

			u16 * scrn;
			u8 * buf;

			scrn = (u16 *)bgl->bgsys[frmnum].screen_buf;
			buf  = GFL_HEAP_AllocMemoryLow( bgl->heapID, 64 );

			memcpy( buf, &cgx[(scrn[pos]&0x3ff)<<6], 64 );
			CgxFlipCheck( (u8)((scrn[pos]>>10)&3), buf ,bgl->heapID);

			dot = buf[ chr_x+(chr_y<<3) ];
			GFL_HEAP_FreeMemory(  buf );

		}else{
			u8 * scrn = (u8 *)bgl->bgsys[frmnum].screen_buf;

			dot = cgx[ (scrn[pos]<<6) + chr_x + (chr_y<<3) ];
		}

		i = 0;
		while(1){
			if( pat[i] == 0xffff ){ break; }

			if( (u8)pat[i] == dot ){
				return TRUE;
			}
		}
	}

	return FALSE;
}

static void CgxFlipCheck( u8 flip, u8 * buf ,u32 heapID)
{
	u8 * tmp;
	u8	i, j;

	if( flip == 0 ){ return; }

	tmp = GFL_HEAP_AllocMemoryLow( heapID, 64 );

	if( flip & 1 ){
		for( i=0; i<8; i++ ){
			for( j=0; j<8; j++ ){
				tmp[ i*8+j ] = buf[ i*8+(7-j) ];
			}
		}
		memcpy( buf, tmp, 64 );
	}

	if( flip & 2 ){
		for( i=0; i<8; i++ ){
			memcpy( &tmp[i*8], &buf[(7-i)*8], 8 );
		}
		memcpy( buf, tmp, 64 );
	}

	GFL_HEAP_FreeMemory( tmp );
}

//--------------------------------------------------------------------------------------------
/**
 * ファイルパスを指定してデータの読み込み
 *
 * @param	heapID	メモリ確保をするヒープID
 * @param	path	ファイルパス
 *
 * @retval	データを読み込んだアドレス
 */
//--------------------------------------------------------------------------------------------
static	void	*LoadFile(int heapID,const char *path)
{
	void	*buf;

	buf=GFL_HEAP_AllocMemory(heapID,GF_GetFileSize((char *)path));

	GF_ReadFile((char *)path,buf);

	return buf;
}

//--------------------------------------------------------------------------------------------
/**
 * ファイルパスを指定してデータの読み込み（データサイズ格納先も指定）
 *
 * @param	heapID	メモリ確保をするヒープID
 * @param	path	ファイルパス
 * @param	size	データサイズ格納先
 *
 * @retval	データを読み込んだアドレス
 */
//--------------------------------------------------------------------------------------------
static	void	*LoadFileEx(int heapID,const char *path,u32 *size)
{
	void	*buf;

	*size=GF_GetFileSize((char *)path);
	buf=GFL_HEAP_AllocMemory(heapID,*size);

	GF_ReadFile((char *)path,buf);

	return buf;
}

