//============================================================================================
/**
 * @file	bgwinfrm.c
 * @brief	ＢＧスクリーン配置処理
 * @author	Hiroyuki Nakamura
 * @date	08.08.19
 */
//============================================================================================
#include <gflib.h>

#include "system/bgwinfrm.h"


//============================================================================================
//	定数定義
//============================================================================================

// フレームデータ
typedef struct {
	u16 * scr;			// スクリーンデータ
	u16	sx:6;			// スクリーンのＸサイズ
	u16	sy:6;			// スクリーンのＹサイズ
	u16	vanish:1;		// 表示フラグ　0 = ON, 1 = OFF
	u16	auto_flg:3;		// 自動移動フラグ　0 = ON, 1 = OFF
	// 2bit
	s8	px;				// 表示Ｘ座標
	s8	py;				// 表示Ｙ座標

	s8	mvx;			// １フレームのＸ移動量
	s8	mvy;			// １フレームのＹ移動量
	u8	frm;			// ＢＧフレーム
	u8	cnt;			// 動作カウンタ

	s8	area_lx;		// 描画範囲
	s8	area_rx;
	s8	area_uy;
	s8	area_dy;

}BGWINFRM;

// フレームワーク
struct _BGWINFRM_WORK {
	BGWINFRM * win;		// フレームデータ
	u16 max;			// フレーム数
	u16	trans_mode:15;	// 転送モード
	u16	auto_master:1;	// 自動移動マスターフラグ　0 = ON, 1 = OFF
	u32	heapID;			// ヒープＩＤ
};

typedef void (*pTransFunc)(u8);		// スクリーン転送関数型


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void LoadScreenDummy( u8 frm );


//============================================================================================
//	グローバル変数
//============================================================================================

// 転送関数
static const pTransFunc LoadScreenFunc[] = {
	LoadScreenDummy,
	GFL_BG_LoadScreenReq,
	GFL_BG_LoadScreenV_Req,
};



//--------------------------------------------------------------------------------------------
/**
 * フレームワーム作成
 *
 * @param	bgl		BGL
 * @param	mode	転送モード
 * @param	max		登録最大数
 * @param	heapID	ヒープＩＤ
 *
 * @return	フレームワーム
 */
//--------------------------------------------------------------------------------------------
BGWINFRM_WORK * BGWINFRM_Create( u32 mode, u32 max, HEAPID heapID )
{
	BGWINFRM_WORK * wk = GFL_HEAP_AllocMemory( heapID, sizeof(BGWINFRM_WORK) );

	wk->max = max;
	wk->heapID = heapID;
	wk->trans_mode = mode;
	wk->auto_master = 0;

	wk->win = GFL_HEAP_AllocClearMemory( heapID, sizeof(BGWINFRM)*max );

	return wk;
}

//--------------------------------------------------------------------------------------------
/**
 * フレームワーム解放
 *
 * @param	wk		フレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_Exit( BGWINFRM_WORK * wk )
{
	u32	i;

	for( i=0; i<wk->max; i++ ){
		if( wk->win[i].scr == NULL ){ continue; }
		GFL_HEAP_FreeMemory( wk->win[i].scr );
	}
	GFL_HEAP_FreeMemory( wk->win );
	GFL_HEAP_FreeMemory( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * フレーム追加
 *
 * @param	wk		フレームワーク
 * @param	index	インデックス
 * @param	frm		ＢＧフレーム
 * @param	sx		Ｘサイズ
 * @param	sy		Ｙサイズ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_Add( BGWINFRM_WORK * wk, u32 index, u32 frm, u32 sx, u32 sy )
{
	BGWINFRM * win;

	win = &wk->win[index];

	win->scr = GFL_HEAP_AllocMemory( wk->heapID, sx * sy * 2 );
	win->sx  = sx;
	win->sy  = sy;

	win->frm = frm;

	win->px  = 0;
	win->py  = 0;

	win->auto_flg = 0;

	win->vanish = BGWINFRM_OFF;

	win->area_lx = 0;
	win->area_rx = 32;
	win->area_uy = 0;
	win->area_dy = 24;
}

//--------------------------------------------------------------------------------------------
/**
 * フレームにスクリーンをセット
 *
 * @param	wk		フレームワーク
 * @param	index	フレームインデックス
 * @param	scr		スクリーンデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_FrameSet( BGWINFRM_WORK * wk, u32 index, u16 * scr )
{
  BGWINFRM * win;

  GF_ASSERT_MSG( index<wk->max , "indexの指定が初期化時の最大数を超えています\n");

  win = &wk->win[index];

  GFL_STD_MemCopy16( scr, win->scr, win->sx*win->sy*2 );
/*
	BGWINFRM * win = &wk->win[index];

	GFL_STD_MemCopy16( scr, win->scr, win->sx*win->sy*2 );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * フレームにアーカイブスクリーンデータをセット
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	fileIdx		アーカイブファイルインデックス
 * @param	dataIdx		データインデックス
 * @param	comp		圧縮フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_FrameSetArc( BGWINFRM_WORK * wk, u32 index, u32 fileIdx, u32 dataIdx, BOOL comp )
{
	NNSG2dScreenData * scrn;
	void * buf;

	buf = GFL_ARC_UTIL_LoadScreen( fileIdx, dataIdx, comp, &scrn, wk->heapID );

	BGWINFRM_FrameSet( wk, index, (void *)scrn->rawData );

	GFL_HEAP_FreeMemory( buf );
}

//--------------------------------------------------------------------------------------------
/**
 * フレームにアーカイブスクリーンデータをセット
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	ah			アーカイブハンドル
 * @param	dataIdx		データインデックス
 * @param	comp		圧縮フラグ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_FrameSetArcHandle( BGWINFRM_WORK * wk, u32 index, ARCHANDLE * ah, u32 dataIdx, BOOL comp )
{
	NNSG2dScreenData * scrn;
	void * buf;

	buf = GFL_ARCHDL_UTIL_LoadScreen( ah, dataIdx, comp, &scrn, wk->heapID );

	BGWINFRM_FrameSet( wk, index, (void *)scrn->rawData );

	GFL_HEAP_FreeMemory( buf );
}

//--------------------------------------------------------------------------------------------
/**
 * フレームをＢＧに配置（位置指定）
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	px			Ｘ座標
 * @param	py			Ｙ座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_FramePut( BGWINFRM_WORK * wk, u32 index, s8 px, s8 py )
{
	BGWINFRM * win;
	u8	wpx, wpy, wsx, wsy, bpx, bpy;

	win = &wk->win[index];

	win->px = px;
	win->py = py;

	if( px >= win->area_rx ||
		py >= win->area_dy ||
		px + win->sx < win->area_lx ||
		py + win->sy < win->area_uy ){
		return;
	}

	wpx = px;
	wsx = win->sx;
	bpx = 0;

	if( px < win->area_lx ){
		wpx = win->area_lx;
		wsx -= ( win->area_lx - px );
		bpx = win->area_lx - px;
	}
	if( px + win->sx >= win->area_rx ){
		wsx -= ( px + win->sx - win->area_rx );
	}

	wpy = py;
	wsy = win->sy;
	bpy = 0;

	if( py < win->area_uy ){
		wpy = win->area_uy;
		wsy -= ( win->area_uy - py );
		bpy = win->area_uy - py;
	}
	if( py + win->sy >= win->area_dy ){
		wsy -= ( py + win->sy - win->area_dy );
	}

	GFL_BG_WriteScreenExpand(
		win->frm, wpx, wpy, wsx, wsy, win->scr, bpx, bpy, win->sx, win->sy );
	LoadScreenFunc[wk->trans_mode]( win->frm );

	win->vanish = BGWINFRM_ON;
}

//--------------------------------------------------------------------------------------------
/**
 * フレームをＢＧに配置（記憶位置）
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_FrameOn( BGWINFRM_WORK * wk, u32 index )
{
	BGWINFRM_FramePut( wk, index, wk->win[index].px, wk->win[index].py );
}

//--------------------------------------------------------------------------------------------
/**
 * フレームを非表示
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_FrameOff( BGWINFRM_WORK * wk, u32 index )
{
	BGWINFRM * win;
	s8	px, py, sx, sy;

	win = &wk->win[index];

	win->vanish = BGWINFRM_OFF;

	if( win->px >= win->area_rx ||
		win->py >= win->area_dy ||
		win->px + win->sx < win->area_lx ||
		win->py + win->sy < win->area_uy ){
		return;
	}

	px = win->px;
	sx = win->sx;

	if( win->px < win->area_lx ){
		px = win->area_lx;
		sx -= ( win->area_lx - win->px );
	}
	if( win->px + win->sx >= win->area_rx ){
		sx -= ( win->px + win->sx - win->area_rx );
	}

	py = win->py;
	sy = win->sy;

	if( win->py < win->area_uy ){
		py = win->area_uy;
		sy -= ( win->area_uy - win->py );
	}
	if( win->py + win->sy >= win->area_dy ){
		sy -= ( win->py + win->sy - win->area_dy );
	}

	GFL_BG_FillScreen( win->frm, 0, px, py, sx, sy, GFL_BG_SCRWRT_PALNL );
	LoadScreenFunc[wk->trans_mode]( win->frm );
}

//--------------------------------------------------------------------------------------------
/**
 * フレーム自動移動設定
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	mvx			Ｘ移動量（１フレーム）
 * @param	mvy			Ｙ移動量（１フレーム）
 * @param	cnt			移動フレーム数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_MoveInit( BGWINFRM_WORK * wk, u32 index, s8 mvx, s8 mvy, u8 cnt )
{
	BGWINFRM * win = &wk->win[index];

	win->mvx = mvx;
	win->mvy = mvy;
	win->cnt = cnt;

	win->auto_flg = 1;

	wk->auto_master = 1;
}

//--------------------------------------------------------------------------------------------
/**
 * フレーム自動移動メイン（全体）
 *
 * @param	wk			フレームワーク
 *
 * @return	none
 *
 * @li	BGWINFRM_MoveOne() と同時に使用すると 2move/1frame になるので注意
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_MoveMain( BGWINFRM_WORK * wk )
{
	u32	i;

	if( wk->auto_master == 0 ){ return; }

	wk->auto_master = 0;

	for( i=0; i<wk->max; i++ ){
		if( BGWINFRM_MoveOne( wk, i ) == 1 ){
			wk->auto_master = 1;
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * フレーム自動移動メイン（個別）
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 *
 * @retval	"0 = 停止中"
 * @retval	"1 = 移動中"
 *
 * @li	BGWINFRM_MoveMain() と同時に使用すると 2move/1frame になるので注意
 */
//--------------------------------------------------------------------------------------------
u32 BGWINFRM_MoveOne( BGWINFRM_WORK * wk, u32 index )
{
	if( wk->win[index].auto_flg == 0 ){
		return 0;
	}

//	wk->auto_master = 0;

	BGWINFRM_FrameOff( wk, index );
	wk->win[index].px += wk->win[index].mvx;
	wk->win[index].py += wk->win[index].mvy;
	BGWINFRM_FrameOn( wk, index );
	wk->win[index].cnt--;
//	if( wk->win[index].cnt == 0 || wk->win[index].vanish == BGWINFRM_OFF ){
	if( wk->win[index].cnt == 0 ){
		wk->win[index].auto_flg = 0;
		return 0;
	}

//	wk->auto_master = 1;

	return 1;
}

//--------------------------------------------------------------------------------------------
/**
 * フレーム自動移動チェック
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 *
 * @retval	"0 = 停止中"
 * @retval	"1 = 移動中"
 */
//--------------------------------------------------------------------------------------------
u32 BGWINFRM_MoveCheck( BGWINFRM_WORK * wk, u32 index )
{
	return wk->win[index].auto_flg;
}

//--------------------------------------------------------------------------------------------
/**
 * フレーム表示チェック
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 *
 * @retval	"BGWINFRM_ON = 表示"
 * @retval	"BGWINFRM_OFF = 非表示"
 */
//--------------------------------------------------------------------------------------------
u32 BGWINFRM_VanishCheck( BGWINFRM_WORK * wk, u32 index )
{
	return wk->win[index].vanish;
}

//--------------------------------------------------------------------------------------------
/**
 * フレームを表示するＢＧを変更
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	frm			変更先のＢＧフレーム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_BGFrameChange( BGWINFRM_WORK * wk, u32 index, u32 frm )
{
	wk->win[index].frm = frm;
}

//--------------------------------------------------------------------------------------------
/**
 * 指定位置のパレットを変更
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	px			Ｘ座標
 * @param	py			Ｙ座標
 * @param	sx			Ｘサイズ
 * @param	sy			Ｙサイズ
 * @param	pal			パレット
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_PaletteChange( BGWINFRM_WORK * wk, u32 index, u8 px, u8 py, u8 sx, u8 sy, u8 pal )
{
	u16 * scr;
	u32	scr_sx;
	u16	i, j;

	scr    = wk->win[index].scr;
	scr_sx = wk->win[index].sx;

	for( i=py; i<py+sy; i++ ){
		for( j=px; j<px+sx; j++ ){
			scr[ i * scr_sx + j ] = ( scr[ i * scr_sx + j ] & 0xfff ) | ( pal << 12 );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * BMPWINを書き込む
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	win			BMPWIN
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_BmpWinOn( BGWINFRM_WORK * wk, u32 index, GFL_BMPWIN * win )
{
	u16 * scr;
	u32	sx;
	u16	i, j;
	u16	chr, pal;
	u8	bw_sx, bw_sy, bw_px, bw_py;

	scr = wk->win[index].scr;
	sx  = wk->win[index].sx;

	chr = GFL_BMPWIN_GetChrNum( win );
	pal = ( (u16)GFL_BMPWIN_GetPalette( win ) << 12 );

	bw_sx = GFL_BMPWIN_GetSizeX( win );
	bw_sy = GFL_BMPWIN_GetSizeY( win );
	bw_px = GFL_BMPWIN_GetPosX( win );
	bw_py = GFL_BMPWIN_GetPosY( win );

	for( i=0; i<bw_sy; i++ ){
		if( ( bw_py + i ) >= wk->win[index].sy ){
			break;
		}
		for( j=0; j<bw_sx; j++ ){
			if( ( bw_px + j ) >= wk->win[index].sx ){
				break;
			}
			scr[ ( bw_py + i ) * sx + bw_px + j ] = chr + j + pal;
		}
		chr += bw_sx;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * フレームのスクリーンを取得
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 *
 * @return	スクリーンのポインタ
 */
//--------------------------------------------------------------------------------------------
u16 * BGWINFRM_FrameBufGet( BGWINFRM_WORK * wk, u32 index )
{
	return wk->win[index].scr;
}

//--------------------------------------------------------------------------------------------
/**
 * フレームの表示ＢＧフレームを取得
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 *
 * @return	ＢＧフレーム
 */
//--------------------------------------------------------------------------------------------
u8 BGWINFRM_BGFrameGet( BGWINFRM_WORK * wk, u32 index )
{
	return wk->win[index].frm;
}

//--------------------------------------------------------------------------------------------
/**
 * フレームの表示位置を取得
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	px			Ｘ座標格納場所
 * @param	py			Ｙ座標格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_PosGet( BGWINFRM_WORK * wk, u32 index, s8 * px, s8 * py )
{
	if( px != NULL ){ *px = wk->win[index].px; }
	if( py != NULL ){ *py = wk->win[index].py; }
}

//--------------------------------------------------------------------------------------------
/**
 * フレームのサイズを取得
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	sx			Ｘサイズ格納場所
 * @param	sy			Ｙサイズ格納場所
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_SizeGet( BGWINFRM_WORK * wk, u32 index, u16 * sx, u16 * sy )
{
	if( sx != NULL ){ *sx = wk->win[index].sx; }
	if( sy != NULL ){ *sy = wk->win[index].sy; }
}

//--------------------------------------------------------------------------------------------
/**
 * 表示エリア設定
 *
 * @param	wk			フレームワーク
 * @param	index		フレームインデックス
 * @param	lx			左Ｘ座標
 * @param	rx			右Ｘ座標
 * @param	uy			上Ｙ座標
 * @param	dy			下Ｙ座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BGWINFRM_PutAreaSet( BGWINFRM_WORK * wk, u32 index, s8 lx, s8 rx, s8 uy, s8 dy )
{
	wk->win[index].area_lx = lx;
	wk->win[index].area_rx = rx;
	wk->win[index].area_uy = uy;
	wk->win[index].area_dy = dy;
}

//--------------------------------------------------------------------------------------------
/**
 * ダミー転送関数
 *
 * @param	frm		ＢＧフレーム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void LoadScreenDummy( u8 frm )
{
}
