//=============================================================================================
/**
 * @file	bmp.c
 * @brief	BMPシステム
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

#include "bmp.h"
#include "bg_sys.h"

#define	GFL_BMP_NORMAL		(0)		//GFL_BMP_Createで生成
#define	GFL_BMP_WITH_DATA	(1)		//GFL_BMP_CreateWithDataで生成

struct _GFL_BMP_DATA{
	u8 * adrs;			///<キャラクタデータの先頭アドレス
	u16	size_x;			///<キャラクタデータのXサイズ
	u16	size_y;			///<キャラクタデータのYサイズ
	u16	col;			///<カラーモード（GFL_BMP_16_COLOR:16色　GFL_BMP_256_COLOR:256色）
	u16	create_flag;	///<BMPCreate方法フラグ（GFL_BMP_NORMAL:GFL_BMP_Create　GFL_BMP_WITH_DATA:GFL_BMP_CreateWithData）
};

static	void GFL_BMP_Print16( const GFL_BMP_DATA * src, GFL_BMP_DATA * dest, u16 pos_sx, u16 pos_sy, s16 pos_dx, s16 pos_dy,
			u16 size_x, u16 size_y, u16 nuki_col );

static	void GFL_BMP_Print256( const GFL_BMP_DATA * src, GFL_BMP_DATA * dest, u16 pos_sx, u16 pos_sy, s16 pos_dx, s16 pos_dy,
		u16 size_x, u16 size_y, u16 nuki_col );

static	void GFL_BMP_Fill16(
		GFL_BMP_DATA * dest,
		s16 pos_dx, s16 pos_dy, u16 size_x, u16 size_y, u8 col_code );

static	void GFL_BMP_Fill256(
		GFL_BMP_DATA * dest,
		s16 pos_dx, s16 pos_dy, u16 size_x, u16 size_y, u8 col_code );

//=============================================================================================
//=============================================================================================
//	ビットマップ関連
//=============================================================================================
//=============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * BMP領域生成
 *
 * @param	sizex	Xサイズ（キャラ単位）
 * @param	sizey	Yサイズ（キャラ単位）
 * @param	col		カラーモード（GFL_BMP_16_COLOR:16色　GFL_BMP_256_COLOR:256色）
 * @param	heapID	ヒープＩＤ
 *
 * @return	取得したメモリのアドレス
 */
//--------------------------------------------------------------------------------------------
GFL_BMP_DATA * GFL_BMP_Create( int sizex, int sizey, int col, HEAPID heapID )
{
	GFL_BMP_DATA * bmp = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_BMP_DATA) );

	// 計算プロセス省略のため、内部ではドット単位で保持する
	bmp->size_x			=	sizex * 8;
	bmp->size_y			=	sizey * 8;
	bmp->col			=	col;
	bmp->adrs			=	GFL_HEAP_AllocClearMemory( heapID, sizex * sizey * col );
	bmp->create_flag	=	GFL_BMP_NORMAL;

	return bmp;
}

//--------------------------------------------------------------------------------------------
/**
 * BMP領域開放
 *
 * @param	bmp		システムワークエリアへのポインタ
 */
//--------------------------------------------------------------------------------------------
void	GFL_BMP_Delete( GFL_BMP_DATA *bmp )
{
	if(bmp->create_flag==GFL_BMP_NORMAL){
		GFL_HEAP_FreeMemory( bmp->adrs );
	}
	GFL_HEAP_FreeMemory( bmp );
}

//--------------------------------------------------------------------------------------------
/**
 * BMP領域生成（GFL_BMP_DATAポインタを生成して、それ以外は引数のものをメンバに代入
 *
 * @param	adrs	キャラエリアへのポインタ
 * @param	sizex	Xサイズ
 * @param	sizey	Yサイズ
 * @param	col		カラーコード
 * @param	heapID	ヒープID
 *
 * @return	取得したメモリのアドレス
 */
//--------------------------------------------------------------------------------------------
GFL_BMP_DATA * GFL_BMP_CreateWithData( u8 *adrs,int sizex, int sizey, int col, HEAPID heapID )
{
	GFL_BMP_DATA * bmp = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_BMP_DATA) );

	bmp->size_x			=	sizex ;
	bmp->size_y			=	sizey ;
	bmp->adrs			=	adrs ;
	bmp->col			=	col ;
	bmp->create_flag	=	GFL_BMP_WITH_DATA;

	return bmp;
}

//--------------------------------------------------------------------------------------------
/*
 * GFL_BMP_DATA構造体のサイズを取得
 *
 * @retval	GFL_BMP_DATA構造体のサイズ
 */
//--------------------------------------------------------------------------------------------
int		GFL_BMP_GetGFL_BMP_DATASize( void )
{
	return sizeof(GFL_BMP_DATA);
}


//--------------------------------------------------------------------------------------------
/*
 * ビットマップアドレスを取得
 *
 * @param	bmp		システムワークエリアへのポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_BMP_DATA	*GFL_BMP_GetBmpAdrs( GFL_BMP_DATA *bmp )
{
	return bmp;
}

//--------------------------------------------------------------------------------------------
/*
 * ビットマップキャラエリアアドレスを取得
 *
 * @param	bmp		システムワークエリアへのポインタ
 */
//--------------------------------------------------------------------------------------------
u8	*GFL_BMP_GetCharacterAdrs( GFL_BMP_DATA *bmp )
{
	return bmp->adrs;
}

//--------------------------------------------------------------------------------------------
/*
 * ビットマップサイズXを取得
 *
 * @param	bmp		システムワークエリアへのポインタ
 */
//--------------------------------------------------------------------------------------------
u16	GFL_BMP_GetSizeX( GFL_BMP_DATA *bmp )
{
	return bmp->size_x;
}

//--------------------------------------------------------------------------------------------
/*
 * ビットマップサイズYを取得
 *
 * @param	bmp		システムワークエリアへのポインタ
 */
//--------------------------------------------------------------------------------------------
u16	GFL_BMP_GetSizeY( GFL_BMP_DATA *bmp )
{
	return bmp->size_y;
}

//--------------------------------------------------------------------------------------------
/**
 * BMPキャラロード（BMPデータの元データ作成）
 *
 * @param	arcID		キャラのアーカイブID
 * @param	datID		キャラのアーカイブ内のデータインデックス
 * @param	compflag	圧縮、非圧縮フラグ
 * @param	heapID		ヒープＩＤ
 *
 * @return	取得したメモリのアドレス
 */
//--------------------------------------------------------------------------------------------
GFL_BMP_DATA * GFL_BMP_LoadCharacter( int arcID, int datID, int compflag, HEAPID heapID )
{
	GFL_BMP_DATA		*bmp = GFL_HEAP_AllocMemory( heapID, sizeof(GFL_BMP_DATA) );
	void				*src;
	NNSG2dCharacterData	*chr;

	src=GFL_ARC_UTIL_Load( arcID, datID, compflag, heapID );
	if( NNS_G2dGetUnpackedBGCharacterData( src, &chr ) == FALSE){
		OS_Panic("GFL_BMP_CharLoad:Faild\n");
		return NULL;
	}

	bmp->size_x			=	chr->W*GFL_BG_1CHRDOTSIZ;
	bmp->size_y			=	chr->H*GFL_BG_1CHRDOTSIZ;
	bmp->adrs			=	GFL_HEAP_AllocMemory( heapID, chr->szByte );
	bmp->create_flag	=	GFL_BMP_NORMAL;
	switch(chr->pixelFmt){ 
	case GX_TEXFMT_PLTT16:
		bmp->col	=	GFL_BMP_16_COLOR;
		break;
	case GX_TEXFMT_PLTT256:
		bmp->col	=	GFL_BMP_256_COLOR;
		break;
	default:
		//G2Dにおいては、１６色、２５６色以外のPixelフォーマットはアサートにする
		GF_ASSERT(0);
		break;
	}

	GFL_STD_MemCopy32( chr->pRawData, bmp->adrs, chr->szByte );

	GFL_HEAP_FreeMemory( src );

	return bmp;
}

//--------------------------------------------------------------------------------------------
/**
 * 読み込みと書き込みのアドレスと範囲を指定してキャラを描画(透明色指定あり）
 *
 * @param	src			読み込み元キャラデータヘッダー構造体ポインタ
 * @param	dest		書き込み先キャラデータヘッダー構造体ポインタ
 * @param	pos_sx		読み込み元読み込み開始X座標
 * @param	pos_sy		読み込み元読み込み開始Y座標
 * @param	pos_dx		書き込み先書き込み開始X座標
 * @param	pos_dy		書き込み先書き込み開始Y座標
 * @param	size_x		描画範囲Xサイズ
 * @param	size_y		描画範囲Yサイズ
 * @param	nuki_col	透明色指定（0～15 0xff:透明色指定なし）
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMP_Print( const GFL_BMP_DATA * src, GFL_BMP_DATA * dest, u16 pos_sx, u16 pos_sy, s16 pos_dx, s16 pos_dy,
			u16 size_x, u16 size_y, u16 nuki_col )
{
	//srcとdestのカラーモードに相違があったら、アサートで止める
	GF_ASSERT(src->col==dest->col);

	if(src->col==GFL_BMP_16_COLOR){
		GFL_BMP_Print16( src, dest, pos_sx, pos_sy, pos_dx, pos_dy, size_x, size_y, nuki_col );
	}
	else{
		GFL_BMP_Print256( src, dest, pos_sx, pos_sy, pos_dx, pos_dy, size_x, size_y, nuki_col );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 範囲を指定して指定された色コードで塗りつぶし
 *
 * @param	dest		書き込み先キャラデータヘッダー構造体ポインタ
 * @param	pos_dx		書き込み先書き込み開始X座標
 * @param	pos_dy		書き込み先書き込み開始Y座標
 * @param	size_x		描画範囲Xサイズ
 * @param	size_y		描画範囲Yサイズ
 * @param	col_code	塗りつぶし色コード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMP_Fill(
		GFL_BMP_DATA * dest,
		s16 pos_dx, s16 pos_dy, u16 size_x, u16 size_y, u8 col_code )
{
	if(dest->col==GFL_BMP_16_COLOR){
		GFL_BMP_Fill16( dest, pos_dx, pos_dy, size_x, size_y, col_code );
	}
	else{
		GFL_BMP_Fill256( dest, pos_dx, pos_dy, size_x, size_y, col_code );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 指定された色コードで塗りつぶし
 *
 * @param	dest		書き込み先キャラデータヘッダー構造体ポインタ
 * @param	col_code	塗りつぶし色コード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void GFL_BMP_Clear( GFL_BMP_DATA * dest, u8 col_code )
{
	u8	col;

	col=((col_code&0x0f)<<4)|(col_code&0x0f);

	GFL_STD_MemFill( dest->adrs, col, ( dest->size_x / 8 ) * ( dest->size_y / 8 ) * dest->col );
}


//--------------------------------------------------------------------------------------------
/**
 * 読み込みと書き込みのアドレスと範囲を指定してキャラを描画(透明色指定あり）
 *
 * @param	src			読み込み元キャラデータヘッダー構造体ポインタ
 * @param	dest		書き込み先キャラデータヘッダー構造体ポインタ
 * @param	pos_sx		読み込み元読み込み開始X座標
 * @param	pos_sy		読み込み元読み込み開始Y座標
 * @param	pos_dx		書き込み先書き込み開始X座標
 * @param	pos_dy		書き込み先書き込み開始Y座標
 * @param	size_x		描画範囲Xサイズ
 * @param	size_y		描画範囲Yサイズ
 * @param	nuki_col	透明色指定（0～15 0xff:透明色指定なし）
 *
 * @return	none
 *
 * @li	１６色用
 */
//--------------------------------------------------------------------------------------------
#define	NULLPAL_L	(nuki_col)

#define SRC_ADRS	(src->adrs)
#define SRC_POSX	(pos_sx)
#define SRC_POSY	(pos_sy)
#define SRC_SIZX	(src->size_x)
#define SRC_SIZY	(src->size_y)
#define	SRC_XARG	(((SRC_SIZX) + (SRC_SIZX & 7))>>3)

#define DST_ADRS	(dest->adrs)
#define DST_POSX	(pos_dx)
#define DST_POSY	(pos_dy)
#define DST_SIZX	(dest->size_x)
#define DST_SIZY	(dest->size_y)
#define	DST_XARG	(((DST_SIZX) + (DST_SIZX & 7))>>3)

#define WRT_SIZX	(size_x)
#define WRT_SIZY	(size_y)

#define DPPCALC(adrs, pos_x, pos_y, size_x)		\
	(u8*)((adrs) +								\
	((pos_x >> 1) & 0x00000003) +				\
	((pos_x << 2) & 0x00003fe0) +				\
	(((pos_y << 2) & 0x00003fe0) * size_x) +	\
	((u32)((pos_y << 2)&0x0000001c))			\
	)

#define DPPCALC_256(adrs, pos_x, pos_y, size_x)	\
	(u8*)((adrs) +								\
	(pos_x & 0x00000007) +						\
	((pos_x << 3) & 0x00007fc0) +				\
	(((pos_y << 3) & 0x00007fc0) * size_x) +	\
	((u32)((pos_y << 3)&0x00000038))			\
	)

static	void GFL_BMP_Print16( const GFL_BMP_DATA * src, GFL_BMP_DATA * dest, u16 pos_sx, u16 pos_sy, s16 pos_dx, s16 pos_dy,
			u16 size_x, u16 size_y, u16 nuki_col )
{
	int	sx, dx, sy, dy, src_dat, shiftval, x_max, y_max, srcxarg, dstxarg;
	u8	*srcadrs, *dstadrs;

	if((DST_SIZX - DST_POSX) < WRT_SIZX){
		x_max = DST_SIZX - DST_POSX + SRC_POSX;
	}else{
		x_max = WRT_SIZX + SRC_POSX;
	}

	if((DST_SIZY - DST_POSY) < WRT_SIZY){
		y_max = DST_SIZY - DST_POSY + SRC_POSY;
	}else{
		y_max = WRT_SIZY + SRC_POSY;
	}

	srcxarg = SRC_XARG;
	dstxarg = DST_XARG;

	if(nuki_col==GF_BMPPRT_NOTNUKI){	//抜き色指定なし
		for(sy=SRC_POSY, dy=DST_POSY; sy < y_max; sy++, dy++){
			for(sx=SRC_POSX, dx=DST_POSX; sx < x_max; sx++, dx++){
				if( ( dx >= 0 ) && ( dy >= 0)){

					//サイズオーバーがあったらアサートで止める
					GF_ASSERT( sx < src->size_x);
					GF_ASSERT( sy < src->size_y);
					GF_ASSERT( dx < dest->size_x);
					GF_ASSERT( dy < dest->size_y);

					srcadrs	= DPPCALC(SRC_ADRS, sx, sy, srcxarg);
					dstadrs = DPPCALC(DST_ADRS, dx, dy, dstxarg);
	
					src_dat = (*srcadrs >> ((sx & 1)*4)) & 0x0f;
					shiftval = (dx & 1)*4;
					*dstadrs = (u8)((src_dat << shiftval)|(*dstadrs & (0xf0 >> shiftval)));
				}
			}
		}
	}else{				//抜き色指定あり
		for(sy=SRC_POSY, dy=DST_POSY; sy < y_max; sy++, dy++){
			for(sx=SRC_POSX, dx=DST_POSX; sx < x_max; sx++, dx++){
				if( ( dx >= 0 ) && ( dy >= 0)){

					//サイズオーバーがあったらアサートで止める
					GF_ASSERT( sx < src->size_x);
					GF_ASSERT( sy < src->size_y);
					GF_ASSERT( dx < dest->size_x);
					GF_ASSERT( dy < dest->size_y);

					srcadrs	= DPPCALC(SRC_ADRS, sx, sy, srcxarg);
					dstadrs = DPPCALC(DST_ADRS, dx, dy, dstxarg);
	
					src_dat = (*srcadrs >> ((sx & 1)*4)) & 0x0f;
	
//					if(src_dat != NULLPAL_L){
					if(src_dat){
						shiftval = (dx & 1)*4;
						*dstadrs = (u8)((src_dat << shiftval)|(*dstadrs & (0xf0 >> shiftval)));
					}
//					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 読み込みと書き込みのアドレスと範囲を指定してキャラを描画(透明色指定あり）
 *
 * @param	src			読み込み元キャラデータヘッダー構造体ポインタ
 * @param	dest		書き込み先キャラデータヘッダー構造体ポインタ
 * @param	pos_sx		読み込み元読み込み開始X座標
 * @param	pos_sy		読み込み元読み込み開始Y座標
 * @param	pos_dx		書き込み先書き込み開始X座標
 * @param	pos_dy		書き込み先書き込み開始Y座標
 * @param	size_x		描画範囲Xサイズ
 * @param	size_y		描画範囲Yサイズ
 * @param	nuki_col	透明色指定（0～15 0xff:透明色指定なし）
 *
 * @return	none
 *
 * @li	２５６色用
 */
//--------------------------------------------------------------------------------------------
static	void GFL_BMP_Print256( const GFL_BMP_DATA * src, GFL_BMP_DATA * dest, u16 pos_sx, u16 pos_sy, s16 pos_dx, s16 pos_dy,
		u16 size_x, u16 size_y, u16 nuki_col )
{
	int	sx, dx, sy, dy, x_max, y_max, srcxarg, dstxarg;
	u8 * srcadrs;
	u8 * dstadrs;

	if( (DST_SIZX - DST_POSX) < WRT_SIZX ){
		x_max = DST_SIZX - DST_POSX + SRC_POSX;
	}else{
		x_max = WRT_SIZX + SRC_POSX;
	}

	if( (DST_SIZY - DST_POSY) < WRT_SIZY ){
		y_max = DST_SIZY - DST_POSY + SRC_POSY;
	}else{
		y_max = WRT_SIZY + SRC_POSY;
	}
	srcxarg = SRC_XARG;
	dstxarg = DST_XARG;

	if( nuki_col == GF_BMPPRT_NOTNUKI ){	//抜き色指定なし
		for( sy=SRC_POSY, dy=DST_POSY; sy<y_max; sy++, dy++ ){
			for( sx=SRC_POSX, dx=DST_POSX; sx<x_max; sx++, dx++ ){
				if( ( dx >= 0 ) && ( dy >= 0 ) ){

					//サイズオーバーがあったらアサートで止める
					GF_ASSERT( sx < src->size_x);
					GF_ASSERT( sy < src->size_y);
					GF_ASSERT( dx < dest->size_x);
					GF_ASSERT( dy < dest->size_y);

					srcadrs	= DPPCALC_256( SRC_ADRS, sx, sy, srcxarg );
					dstadrs = DPPCALC_256( DST_ADRS, dx, dy, dstxarg );
					*dstadrs = *srcadrs;
				}
			}
		}
	}else{				//抜き色指定あり
		for( sy=SRC_POSY, dy=DST_POSY; sy<y_max; sy++, dy++ ){
			for( sx=SRC_POSX, dx=DST_POSX; sx<x_max; sx++, dx++ ){
				if( ( dx >= 0 ) && ( dy >= 0 ) ){

					//サイズオーバーがあったらアサートで止める
					GF_ASSERT( sx < src->size_x);
					GF_ASSERT( sy < src->size_y);
					GF_ASSERT( dx < dest->size_x);
					GF_ASSERT( dy < dest->size_y);

					srcadrs	= DPPCALC_256( SRC_ADRS, sx, sy, srcxarg );
					dstadrs = DPPCALC_256( DST_ADRS, dx, dy, dstxarg );
					if( *srcadrs != NULLPAL_L ){
						*dstadrs = *srcadrs;
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 範囲を指定して指定された色コードで塗りつぶし
 *
 * @param	dest		書き込み先キャラデータヘッダー構造体ポインタ
 * @param	pos_dx		書き込み先書き込み開始X座標
 * @param	pos_dy		書き込み先書き込み開始Y座標
 * @param	size_x		描画範囲Xサイズ
 * @param	size_y		描画範囲Yサイズ
 * @param	col_code	塗りつぶし色コード
 *
 * @return	none
 *
 * @li	１６色用
 */
//--------------------------------------------------------------------------------------------
static	void GFL_BMP_Fill16( GFL_BMP_DATA * dest, s16 pos_dx, s16 pos_dy, u16 size_x, u16 size_y, u8 col_code )
{
	int	x,y,x_max,y_max,xarg;
	u8	*destadrs;

	x_max = pos_dx + size_x;
	if(x_max > dest->size_x){
		x_max = dest->size_x;
	}

	y_max = pos_dy + size_y;
	if(y_max > dest->size_y){
		y_max = dest->size_y;
	}

	xarg = (((dest->size_x) + (dest->size_x&7))>>3);

	for(y = pos_dy; y < y_max; y++){
		for(x = pos_dx; x < x_max; x++){
			if( ( x >= 0 ) && ( y >= 0 ) ){

				//サイズオーバーがあったらアサートで止める
				GF_ASSERT( x < dest->size_x);
				GF_ASSERT( y < dest->size_y);

				destadrs=DPPCALC(dest->adrs, x, y, xarg);
				if(x&1){
					*destadrs&=0x0f;
					*destadrs|=(col_code<<4);
				}
				else{
					*destadrs&=0xf0;
					*destadrs|=col_code;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * 範囲を指定して指定された色コードで塗りつぶし
 *
 * @param	dest		書き込み先キャラデータヘッダー構造体ポインタ
 * @param	pos_dx		書き込み先書き込み開始X座標
 * @param	pos_dy		書き込み先書き込み開始Y座標
 * @param	size_x		描画範囲Xサイズ
 * @param	size_y		描画範囲Yサイズ
 * @param	col_code	塗りつぶし色コード
 *
 * @return	none
 *
 * @li	２５６色用
 */
//--------------------------------------------------------------------------------------------
static	void GFL_BMP_Fill256( GFL_BMP_DATA * dest, s16 pos_dx, s16 pos_dy, u16 size_x, u16 size_y, u8 col_code )
{
	int	x,y,x_max,y_max,xarg;
	u8	*destadrs;

	x_max = pos_dx + size_x;
	if( x_max > dest->size_x ){
		x_max = dest->size_x;
	}

	y_max = pos_dy + size_y;
	if( y_max > dest->size_y ){
		y_max = dest->size_y;
	}

	xarg = ( dest->size_x + (dest->size_x & 7) ) >> 3;

	for( y=pos_dy; y<y_max; y++ ){
		for( x=pos_dx; x<x_max; x++ ){
			if( ( x >= 0 ) && ( y >= 0 ) ){

				//サイズオーバーがあったらアサートで止める
				GF_ASSERT( x < dest->size_x);
				GF_ASSERT( y < dest->size_y);

				destadrs = DPPCALC_256( dest->adrs, x, y, xarg );
				*destadrs = col_code;
			}
		}
	}
}

