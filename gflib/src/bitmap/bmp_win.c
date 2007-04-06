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
	HEAPID			heapID;
	u16				padding;		//HEAPID=u16なので
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
	GFL_BMP_DATA	*bmp;
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
 * @param[in]	set		使用領域設定構造体
 * @param[in]	heapID	使用ヒープ領域
 */
//--------------------------------------------------------------------------------------------
void	GFL_BMPWIN_Init( HEAPID heapID )
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
void	GFL_BMPWIN_Exit( void )
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
 * @param[in]	frmnum	GFL_BGL使用フレーム
 * @param[in]	posx	Ｘ座標（キャラクター単位）
 * @param[in]	posy	Ｙ座標（キャラクター単位）
 * @param[in]	sizx	Ｘサイズ（キャラクター単位）
 * @param[in]	sizy	Ｙサイズ（キャラクター単位）
 * @param[in]	panum	使用パレットナンバー
 * @param[in]	dir		確保ＶＲＡＭ方向(GFL_BMP_CHRAREA_GET_F:前方確保　GFL_BMP_CHRAREA_GET_B:後方確保）
 *
 * @return	ビットマップウインドウハンドルポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_BMPWIN* GFL_BMPWIN_Create( u8 frmnum, u8 posx, u8 posy, u8 sizx, u8 sizy, u8 palnum, u8 dir )
{
	GFL_BMPWIN* bmpwin;
	u32 areasiz, areapos;
	HEAPID heapID = bmpwin_sys->heapID;
	int	col;

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
	if( GFL_BG_GetScreenColorMode( frmnum) == GX_BG_COLORMODE_16){
		bmpwin->bitmode = GFL_BMPWIN_BITMODE_4;	//１６色モード
		col=GFL_BMP_16_COLOR;
		areasiz = sizx * sizy;
	} else {
		bmpwin->bitmode = GFL_BMPWIN_BITMODE_8;	//２５６色モード
		col=GFL_BMP_256_COLOR;
		areasiz = sizx * sizy * 2;	//使用するキャラ領域は２倍(0x40)
	}

	//キャラクター領域の確保
	areapos = GFL_BG_AllocCharacterArea( frmnum, areasiz * 0x20, dir );
	if( areapos == AREAMAN_POS_NOTFOUND ){
		OS_Panic( "ビットマップ生成に必要なキャラＶＲＡＭ領域が足りない\n" );
	} else {
		bmpwin->chrnum = areapos;
		bmpwin->bmp=GFL_BMP_Create( sizx, sizy, col, heapID );
	}
	return bmpwin;
}


//--------------------------------------------------------------------------------------------
/**
 *
 * ビットマップウインドウの破棄
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 */
//--------------------------------------------------------------------------------------------
void GFL_BMPWIN_Delete( GFL_BMPWIN* bmpwin )
{
	u32 areasiz;

	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );

	if(	bmpwin->bitmode == GFL_BMPWIN_BITMODE_4 ){
		areasiz = bmpwin->sizx * bmpwin->sizy;
	} else {
		areasiz = bmpwin->sizx * bmpwin->sizy * 2;	//使用するキャラ領域は２倍(0x40)
	}
	//キャラクター領域の解放
	GFL_BG_FreeCharacterArea( bmpwin->frmnum, bmpwin->chrnum, areasiz*0x20 );

	GFL_BMP_Delete( bmpwin->bmp );
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
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 */
//---------------------------------------------------------
void	GFL_BMPWIN_TransVramCharacter( GFL_BMPWIN * bmpwin )
{
	u32	bmpsize;

	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );

	bmpsize = (u32)
		((bmpwin->sizx*bmpwin->sizy * GFL_BG_GetBaseCharacterSize(bmpwin->frmnum)));

	GFL_BG_LoadCharacter( bmpwin->frmnum, GFL_BMP_GetCharacterAdrs(bmpwin->bmp), bmpsize, bmpwin->chrnum );
}

//---------------------------------------------------------
/**
 * ビットマップ用スクリーンを関連ＢＧスクリーンバッファに作成する
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 */
//---------------------------------------------------------
void	GFL_BMPWIN_MakeScreen( GFL_BMPWIN * bmpwin )
{
	u16*	scrnbuf;
	u16		scrnsiz,scrnchr,scrnpal;
	int		i;

	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );

	scrnsiz = bmpwin->sizx * bmpwin->sizy;
	scrnbuf = (u16*)GFL_HEAP_AllocMemoryLo( bmpwin_sys->heapID, scrnsiz*2 );
	scrnchr = bmpwin->chrnum;
	scrnpal = (bmpwin->palnum << 12);
				
	for(i=0;i<scrnsiz;i++){
		scrnbuf[i] = (scrnchr|scrnpal);
		scrnchr++;
	}
	GFL_BG_WriteScreen( bmpwin->frmnum, scrnbuf, bmpwin->posx, bmpwin->posy, bmpwin->width, bmpwin->height );
	GFL_BG_WriteScreenExpand(  bmpwin->frmnum, bmpwin->posx, bmpwin->posy, bmpwin->width, bmpwin->height,
							scrnbuf, 0, 0, bmpwin->sizx, bmpwin->sizy );

	GFL_HEAP_FreeMemory( scrnbuf );
}

//---------------------------------------------------------
/**
 * 関連ＢＧスクリーンバッファのビットマップ用スクリーンをクリアする
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 */
//---------------------------------------------------------
void	GFL_BMPWIN_ClearScreen( GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );

	GFL_BG_FillScreen( bmpwin->frmnum, 0, bmpwin->posx, bmpwin->posy, bmpwin->sizx, bmpwin->sizy, 0 );
}


//--------------------------------------------------------------------------------------------
/**
 * ビットマップウインドウステータス取得
 */
//--------------------------------------------------------------------------------------------
//--------------------------------
/**
 *
 * フレームナンバーの取得
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 * @retval	フレームナンバー
 */
//--------------------------------
u8	GFL_BMPWIN_GetFrame( const GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );

	return bmpwin->frmnum;
}

//--------------------------------
/**
 * BMP領域のXサイズの取得
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 * @retval	BMP領域のXサイズ
 */
//--------------------------------
u8	GFL_BMPWIN_GetSizeX( const GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );

	return bmpwin->sizx;
}

//--------------------------------
/**
 * BMP領域のYサイズの取得
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 * @retval	BMP領域のYサイズ
 */
//--------------------------------
u8	GFL_BMPWIN_GetSizeY( const GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	return bmpwin->sizy;
}

//--------------------------------
/**
 * スクリーンXサイズの取得
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 * @retval	スクリーンXサイズ
 */
//--------------------------------
u8	GFL_BMPWIN_GetScreenSizeX( const GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );

	return bmpwin->width;
}

//--------------------------------
/**
 * スクリーンYサイズの取得
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 * @retval	スクリーンYサイズ
 */
//--------------------------------
u8	GFL_BMPWIN_GetScreenSizeY( const GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );

	return bmpwin->height;
}

//--------------------------------
/**
 * 描画Xサイズの取得
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 * @retval	描画Xサイズ
 */
//--------------------------------
u8	GFL_BMPWIN_GetPosX( const GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	return bmpwin->posx;
}

//--------------------------------
/**
 * 描画Yサイズの取得
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 * @retval	描画Yサイズ
 */
//--------------------------------
u8	GFL_BMPWIN_GetPosY( const GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	return bmpwin->posy;
}

//--------------------------------
/**
 * キャラクターナンバーの取得
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 * @retval	キャラクターナンバー
 */
//--------------------------------
u16	GFL_BMPWIN_GetChrNum( const GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	return bmpwin->chrnum;
}

//--------------------------------
/**
 * ビットマップポインタの取得
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 *
 * @retval	ビットマップポインタ
 */
//--------------------------------
GFL_BMP_DATA*	GFL_BMPWIN_GetBmp( GFL_BMPWIN * bmpwin )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	return GFL_BMP_GetBmpAdrs(bmpwin->bmp);
}

//--------------------------------------------------------------------------------------------
/**
 * ビットマップウインドウステータス変更
 */
//--------------------------------------------------------------------------------------------
//--------------------------------
/**
 * 描画X座標の変更
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 * @param[in]	px		セットする描画X座標
 */
//--------------------------------
void	GFL_BMPWIN_SetPosX( GFL_BMPWIN * bmpwin, u8 px )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	bmpwin->posx = px;
}

//--------------------------------
/**
 * 描画Y座標の変更
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 * @param[in]	py		セットする描画Y座標
 */
//--------------------------------
void	GFL_BMPWIN_SetPosY( GFL_BMPWIN * bmpwin, u8 py )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	bmpwin->posy = py;
}

//--------------------------------
/**
 * スクリーン描画サイズXの変更
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 * @param[in]	sx		セットするスクリーン描画サイズX
 */
//--------------------------------
void	GFL_BMPWIN_SetScreenSizeX( GFL_BMPWIN * bmpwin, u8 sx )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	bmpwin->width = sx;
}
//--------------------------------
/**
 * スクリーン描画サイズＹの変更
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 * @param[in]	sy		セットするスクリーン描画サイズY
 */
//--------------------------------
void	GFL_BMPWIN_SetScreenSizeY( GFL_BMPWIN * bmpwin, u8 sy )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	bmpwin->height = sy;
}
//--------------------------------
/**
 * パレットナンバーの変更
 *
 * @param[in]	bmpwin	ビットマップウインドウポインタ
 * @param[in]	palnum	セットするパレットナンバー
 */
//--------------------------------
void	GFL_BMPWIN_SetPalette( GFL_BMPWIN * bmpwin, u8 palnum )
{
	GF_ASSERT( bmpwin->magicnum == GFL_BMPWIN_MAGICNUM );
	bmpwin->palnum = palnum;
}

