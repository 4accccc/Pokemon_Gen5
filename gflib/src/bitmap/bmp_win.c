//=============================================================================================
/**
 *
 * @file	bmp_win.c
 * @brief	ビットマップウィンドウシステム
 *
 */
//=============================================================================================
#include "gflib.h"

//=============================================================================================
//	型宣言
//=============================================================================================
#define	GFL_BMPWIN_MAGICNUM		(0x7FB2)

///BMPWINデータ構造体
typedef struct {
	u16				heapID;

}GFL_BMPWIN_SYS;

///BMPWINデータ構造体
struct _GFL_BMPWIN {
	u16				magicnum:15;
	u16				bitmode:1;	
	u8				frmnum;
	u8				posx;
	u8				posy;
	u8				sizx;
	u8				sizy;
	u8				width;
	u8				height;
	u8				palnum;
	u16				chrnum;
	GFL_BMP_DATA	bmp;
};

static GFL_BMPWIN_SYS* bmpwin_sys = NULL;
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 *
 * ビットマップ管理システムの開始、終了
 *
 * 主にプロセス起動、終了時に呼ばれる。
 * ビットマップウインドウを使用するＢＧフレーム（複数指定可）、使用ＣＧＸ領域の設定を行う。
 * これ以後、各ビットマップ作成の際は、ここで設定したステータスが用いられる。
 */
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * システム初期化
 *
 * @param	set		使用領域設定構造体
 * @param	heapID	使用ヒープ領域
 */
//--------------------------------------------------------------------------------------------
void
	GFL_BMPWIN_sysInit
		( GFL_BMPWIN_SET* set, u16 heapID )
{
	int	i;

	bmpwin_sys = (GFL_BMPWIN_SYS*)GFL_HEAP_AllocMemory( heapID, sizeof(GFL_BMPWIN_SYS) );
	bmpwin_sys->heapID = heapID;
}

//--------------------------------------------------------------------------------------------
/**
 * システム終了
 */
//--------------------------------------------------------------------------------------------
void
	GFL_BMPWIN_sysExit
		( void )
{
	if( bmpwin_sys != NULL ){
		GFL_HEAP_FreeMemory( bmpwin_sys );
		bmpwin_sys = NULL;
	}
}


//--------------------------------------------------------------------------------------------
/**
 *
 * ビットマップウインドウの作成、破棄
 *
 * ビットマップとして使用したいＢＧフレーム、描画位置、使用ＣＧＸサイズの設定などを指定し
 * ビットマップウインドウ管理ハンドルを作成する。
 * 自動でＣＧＸバッファ領域のヒープ確保も行う。
 * 作成後のビットマップウインドウ操作は、ハンドルポインタによって行われる。
 */
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static inline void CHK_BMPWIN( GFL_BMPWIN* bmpwin )
{
	if( bmpwin->magicnum != GFL_BMPWIN_MAGICNUM ){
		OS_Panic( "指定されたハンドルはGFL_BMPWINハンドルではありません(bmp_win.c)\n" );
	}
}
//--------------------------------------------------------------------------------------------
/**
 *
 * ビットマップウインドウの作成
 *
 * @param	frmnum	GFL_BGL使用フレーム
 * @param	posx	Ｘ座標（キャラクター単位）
 * @param	posy	Ｙ座標（キャラクター単位）
 * @param	sizx	Ｘサイズ（キャラクター単位）
 * @param	sizy	Ｙサイズ（キャラクター単位）
 * @param	panum	使用パレットナンバー
 * @param	dir		確保ＶＲＡＭ方向
 *
 * @return	ビットマップウインドウハンドルポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_BMPWIN* 
	GFL_BMPWIN_Create
		( u8 frmnum, u8 posx, u8 posy, u8 sizx, u8 sizy, u8 palnum, u8 dir )
{
	GFL_BMPWIN* bmpwin;
	u32 areasiz, areapos;
	u16 heapID = bmpwin_sys->heapID;

	bmpwin = (GFL_BMPWIN*)GFL_HEAP_AllocMemory( heapID, sizeof(GFL_BMPWIN) );

	bmpwin->magicnum	= GFL_BMPWIN_MAGICNUM;
	bmpwin->frmnum		= frmnum;
	bmpwin->posx		= posx;
	bmpwin->posy		= posy;
	bmpwin->sizx		= sizx;
	bmpwin->sizy		= sizy;
	bmpwin->width		= sizx;	//スクリーン描画サイズ（デフォルトはサイズと等しい）
	bmpwin->height		= sizy;	//スクリーン描画サイズ（デフォルトはサイズと等しい）
	bmpwin->palnum		= palnum;

	//キャラクタモード判別
	if( GFL_BG_ScreenColorModeGet( frmnum) == GX_BG_COLORMODE_16){
		bmpwin->bitmode = GFL_BMPWIN_BITMODE_4;	//１６色モード
		areasiz = sizx * sizy;
	} else {
		bmpwin->bitmode = GFL_BMPWIN_BITMODE_8;	//２５６色モード
		areasiz = sizx * sizy * 2;	//使用するキャラ領域は２倍(0x40)
	}

	//キャラクター領域の確保
	areapos = GFL_BG_CharAreaGet( frmnum, areasiz * 0x20 );
	if( areapos == AREAMAN_POS_NOTFOUND ){
		OS_Panic( "ビットマップ生成に必要なキャラＶＲＡＭ領域が足りない\n" );
	} else {
		bmpwin->chrnum = areapos;
		bmpwin->bmp.adrs = GFL_HEAP_AllocMemoryClear( heapID, areasiz * 0x20 );
		bmpwin->bmp.size_x = sizx * 8;
		bmpwin->bmp.size_y = sizy * 8;
	}
	return bmpwin;
}


//--------------------------------------------------------------------------------------------
/**
 *
 * ビットマップウインドウの破棄
 *
 * @param	bmpwin	ビットマップウインドウポインタ
 *
 */
//--------------------------------------------------------------------------------------------
void
	GFL_BMPWIN_Delete
		( GFL_BMPWIN* bmpwin )
{
	u32 areasiz;

	CHK_BMPWIN( bmpwin );

	if(	bmpwin->bitmode == GFL_BMPWIN_BITMODE_4 ){
		areasiz = bmpwin->sizx * bmpwin->sizy;
	} else {
		areasiz = bmpwin->sizx * bmpwin->sizy * 2;	//使用するキャラ領域は２倍(0x40)
	}
	//キャラクター領域の解放
	GFL_BG_CharAreaFree( bmpwin->frmnum, bmpwin->chrnum, areasiz*0x20 );

	GFL_HEAP_FreeMemory( bmpwin->bmp.adrs );
	GFL_HEAP_FreeMemory( bmpwin );
}


//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 *
 *
 * 描画制御
 *
 *
 */
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//---------------------------------------------------------
/**
 * ビットマップ用キャラクターを内部バッファからＶＲＡＭへ転送
 *
 * @param	bmpwin	ビットマップウインドウポインタ
 */
//---------------------------------------------------------
void
	GFL_BMPWIN_UploadChar
		( GFL_BMPWIN * bmpwin )
{
	u32	bmpsize;

	CHK_BMPWIN( bmpwin );

	bmpsize = (u32)
		((bmpwin->sizx*bmpwin->sizy * GFL_BG_BaseCharSizeGet(bmpwin->frmnum)));

	GFL_BG_LoadCharacter( bmpwin->frmnum, bmpwin->bmp.adrs, bmpsize, bmpwin->chrnum );
}

//---------------------------------------------------------
/**
 * ビットマップ用スクリーンを関連ＢＧスクリーンバッファに作成する
 *
 * @param	bmpwin	ビットマップウインドウポインタ
 */
//---------------------------------------------------------
void
	GFL_BMPWIN_MakeScrn
		( GFL_BMPWIN * bmpwin )
{
	u16*	scrnbuf;
	u16		scrnsiz,scrnchr,scrnpal;
	int		i;

	CHK_BMPWIN( bmpwin );

	scrnsiz = bmpwin->sizx * bmpwin->sizy;
	scrnbuf = (u16*)GFL_HEAP_AllocMemoryLow( bmpwin_sys->heapID, scrnsiz*2 );
	scrnchr = bmpwin->chrnum;
	scrnpal = (bmpwin->palnum << 12);
				
	for(i=0;i<scrnsiz;i++){
		scrnbuf[i] = (scrnchr|scrnpal);
		scrnchr++;
	}
	GFL_BG_ScrWrite( bmpwin->frmnum, scrnbuf, bmpwin->posx, bmpwin->posy, bmpwin->sizx, bmpwin->sizy );

	GFL_HEAP_FreeMemory( scrnbuf );
}

//---------------------------------------------------------
/**
 * 関連ＢＧスクリーンバッファのビットマップ用スクリーンをクリアする
 *
 * @param	bmpwin	ビットマップウインドウポインタ
 */
//---------------------------------------------------------
void
	GFL_BMPWIN_ClearScrn
		( GFL_BMPWIN * bmpwin )
{
	CHK_BMPWIN( bmpwin );

	CHK_BMPWIN( bmpwin );

	GFL_BG_ScrFill( bmpwin->frmnum, 0, bmpwin->posx, bmpwin->posy, bmpwin->sizx, bmpwin->sizy, 0 );
}


//--------------------------------------------------------------------------------------------
/**
 * ビットマップウインドウステータス取得
 */
//--------------------------------------------------------------------------------------------
//--------------------------------
/**
 * フレームナンバーの取得
 */
//--------------------------------
u8
	GFL_BMPWIN_GetFrame
		( GFL_BMPWIN * bmpwin )
{
	CHK_BMPWIN( bmpwin );
	return bmpwin->frmnum;
}

//--------------------------------
/**
 * キャラクターＸサイズの取得
 */
//--------------------------------
u8
	GFL_BMPWIN_GetSizeX
		( GFL_BMPWIN * bmpwin )
{
	CHK_BMPWIN( bmpwin );
	return bmpwin->sizx;
}

//--------------------------------
/**
 * キャラクターＹサイズの取得
 */
//--------------------------------
u8
	GFL_BMPWIN_GetSizeY
		( GFL_BMPWIN * bmpwin )
{
	CHK_BMPWIN( bmpwin );
	return bmpwin->sizy;
}

//--------------------------------
/**
 * 描画Ｘ座標の取得
 */
//--------------------------------
u8
	GFL_BMPWIN_GetPosX
		( GFL_BMPWIN * bmpwin )
{
	CHK_BMPWIN( bmpwin );
	return bmpwin->posx;
}

//--------------------------------
/**
 * 描画Ｙ座標の取得
 */
//--------------------------------
u8
	GFL_BMPWIN_GetPosY
		( GFL_BMPWIN * bmpwin )
{
	CHK_BMPWIN( bmpwin );
	return bmpwin->posy;
}

//--------------------------------
/**
 * キャラクターナンバーの取得
 */
//--------------------------------
u16
	GFL_BMPWIN_GetChrNum
		( GFL_BMPWIN * bmpwin )
{
	CHK_BMPWIN( bmpwin );
	return bmpwin->chrnum;
}

//--------------------------------
/**
 * ビットマップポインタの取得
 */
//--------------------------------
GFL_BMP_DATA*
	GFL_BMPWIN_GetBmp
		( GFL_BMPWIN * bmpwin )
{
	CHK_BMPWIN( bmpwin );
	return &bmpwin->bmp;
}


//--------------------------------------------------------------------------------------------
/**
 * ビットマップウインドウステータス変更
 */
//--------------------------------------------------------------------------------------------
//--------------------------------
/**
 * 描画Ｘ座標の変更
 */
//--------------------------------
void
	GFL_BMPWIN_SetPosX
		( GFL_BMPWIN * bmpwin, u8 px )
{
	CHK_BMPWIN( bmpwin );
	bmpwin->posx = px;
}
//--------------------------------
/**
 * 描画Ｙ座標の変更
 */
//--------------------------------
void
	GFL_BMPWIN_SetPosY
		( GFL_BMPWIN * bmpwin, u8 py )
{
	CHK_BMPWIN( bmpwin );
	bmpwin->posy = py;
}
//--------------------------------
/**
 * スクリーン描画サイズＸの変更
 */
//--------------------------------
void
	GFL_BMPWIN_SetScrnSizX
		( GFL_BMPWIN * bmpwin, u8 sx )
{
	CHK_BMPWIN( bmpwin );
	bmpwin->width = sx;
}
//--------------------------------
/**
 * スクリーン描画サイズＹの変更
 */
//--------------------------------
void
	GFL_BMPWIN_SetScrnSizY
		( GFL_BMPWIN * bmpwin, u8 sy )
{
	CHK_BMPWIN( bmpwin );
	bmpwin->height = sy;
}
//--------------------------------
/**
 * パレットの変更
 */
//--------------------------------
void
	GFL_BMPWIN_SetPal
		( GFL_BMPWIN * bmpwin, u8 palnum )
{
	CHK_BMPWIN( bmpwin );
	bmpwin->palnum = palnum;
}


