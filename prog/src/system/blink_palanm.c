//============================================================================================
/**
 * @file		blink_palanm.c
 * @brief		点滅パレットアニメ処理
 * @author	Hiroyuki Nakamura
 * @date		10.01.18
 *
 *	モジュール名：BLINKPALANM
 *
 *	指定カラーから指定カラーへのパレットアニメです。
 *	カーソルアニメで使用しています。
 */
//============================================================================================
#include <gflib.h>

#include "system/blink_palanm.h"


//============================================================================================
//	定数定義
//============================================================================================
struct _BLINK_PALANM_WORK {
	u16 * startPal;
	u16 * endPal;
	u16 * transPal;
	u16	offs;
	u16	size;
	u16	cnt;
	u16	seq;
	NNS_GFD_DST_TYPE	type;
	HEAPID	heapID;
};

#define ANM_CNT_VALUE		( 0x400 )			// アニメカウント
#define ANM_CNT_MAX			( 0x10000 )		// アニメカウント最大値

#define	COL_R(a)	( ( a >> GX_RGB_R_SHIFT ) & 0x1f )	// 指定カラーから赤を抜き出す
#define	COL_G(a)	( ( a >> GX_RGB_G_SHIFT ) & 0x1f )	// 指定カラーから緑を抜き出す
#define	COL_B(a)	( ( a >> GX_RGB_B_SHIFT ) & 0x1f )	// 指定カラーから青を抜き出す



//--------------------------------------------------------------------------------------------
/**
 * 点滅パレットアニメワーク作成
 *
 * @param		offs		アニメをするパレット位置（カラー単位）
 * @param		size		アニメをするパレットのサイズ（カラー単位）
 * @param		bgfrm		ＢＧフレーム
 * @param		hapID		ヒープＩＤ
 *
 * @return	アニメワーク
 */
//--------------------------------------------------------------------------------------------
BLINKPALANM_WORK * BLINKPALANM_Create( u16 offs, u16 size, u16 bgfrm, HEAPID heapID )
{
	BLINKPALANM_WORK * wk = GFL_HEAP_AllocMemory( heapID, sizeof(BLINKPALANM_WORK) );
	wk->transPal = GFL_HEAP_AllocMemory( heapID, sizeof(u16) * size );
	wk->offs = offs;
	wk->size = size;
	wk->cnt = 0;
	wk->seq = 0;
	wk->heapID = heapID;

	if( bgfrm <= GFL_BG_FRAME3_M ){
		wk->type = NNS_GFD_DST_2D_BG_PLTT_MAIN;
	}else{
		wk->type = NNS_GFD_DST_2D_BG_PLTT_SUB;
	}

	return wk;
}

//--------------------------------------------------------------------------------------------
/**
 * パレット設定
 *
 * @param		wk				パレットアニメワーク
 * @param		arcID			アーカイブＩＤ
 * @param		datID			データＩＤ
 * @param		startPos	開始基準パレット位置（カラー単位）
 * @param		endPos		終了基準パレット位置（カラー単位）
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BLINKPALANM_SetPalBufferArc( BLINKPALANM_WORK * wk, ARCID arcID, ARCDATID datID, u32 startPos, u32 endPos )
{
	NNSG2dPaletteData * pal;
	void * buf;
	u16 * dat;

	wk->startPal = GFL_HEAP_AllocMemory( wk->heapID, sizeof(u16) * wk->size );
	wk->endPal   = GFL_HEAP_AllocMemory( wk->heapID, sizeof(u16) * wk->size );

	buf = GFL_ARC_UTIL_LoadPalette( arcID, datID, &pal, wk->heapID );
	dat = (u16 *)pal->pRawData;

	GFL_STD_MemCopy( &dat[startPos], wk->startPal, sizeof(u16) * wk->size );
	GFL_STD_MemCopy( &dat[endPos], wk->endPal, sizeof(u16) * wk->size );

	GFL_HEAP_FreeMemory( buf );
}

//--------------------------------------------------------------------------------------------
/**
 * パレット設定（ハンドル指定）
 *
 * @param		wk				パレットアニメワーク
 * @param		ah				アークハンドル
 * @param		datID			データＩＤ
 * @param		startPos	開始基準パレット位置（カラー単位）
 * @param		endPos		終了基準パレット位置（カラー単位）
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BLINKPALANM_SetPalBufferArcHDL( BLINKPALANM_WORK * wk, ARCHANDLE * ah, ARCDATID datID, u32 startPos, u32 endPos )
{
	NNSG2dPaletteData * pal;
	void * buf;
	u16 * dat;

	wk->startPal = GFL_HEAP_AllocMemory( wk->heapID, sizeof(u16) * wk->size );
	wk->endPal   = GFL_HEAP_AllocMemory( wk->heapID, sizeof(u16) * wk->size );

	buf = GFL_ARCHDL_UTIL_LoadPalette( ah, datID, &pal, wk->heapID );
	dat = (u16 *)pal->pRawData;

	GFL_STD_MemCopy( &dat[startPos], wk->startPal, sizeof(u16) * wk->size );
	GFL_STD_MemCopy( &dat[endPos], wk->endPal, sizeof(u16) * wk->size );

	GFL_HEAP_FreeMemory( buf );
}

//--------------------------------------------------------------------------------------------
/**
 * 点滅パレットアニメワーク削除
 *
 * @param		wk		パレットアニメワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BLINKPALANM_Exit( BLINKPALANM_WORK * wk )
{
	GFL_HEAP_FreeMemory( wk->endPal );
	GFL_HEAP_FreeMemory( wk->startPal );
	GFL_HEAP_FreeMemory( wk->transPal );
	GFL_HEAP_FreeMemory( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * 点滅パレットアニメ動作メイン
 *
 * @param		wk		パレットアニメワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BLINKPALANM_Main( BLINKPALANM_WORK * wk )
{
	fx32	cos;
	u32	i;
	u8	r, g, b;

	if( ( wk->cnt + ANM_CNT_VALUE ) >= ANM_CNT_MAX ){
		wk->cnt = wk->cnt + ANM_CNT_VALUE - ANM_CNT_MAX;
	}else{
		wk->cnt += ANM_CNT_VALUE;
	}

	cos = ( FX_CosIdx(wk->cnt) + FX16_ONE ) / 2;

	for( i=0; i<wk->size; i++ ){
		r = COL_R(wk->startPal[i]) + (((COL_R(wk->endPal[i])-COL_R(wk->startPal[i]))*cos)>>FX16_SHIFT);
		g = COL_G(wk->startPal[i]) + (((COL_G(wk->endPal[i])-COL_G(wk->startPal[i]))*cos)>>FX16_SHIFT);
		b = COL_B(wk->startPal[i]) + (((COL_B(wk->endPal[i])-COL_B(wk->startPal[i]))*cos)>>FX16_SHIFT);

		wk->transPal[i] = GX_RGB( r, g, b );
	}

	NNS_GfdRegisterNewVramTransferTask( wk->type, wk->offs * 2, wk->transPal, wk->size * 2 );
}

//--------------------------------------------------------------------------------------------
/**
 * アニメ動作カウンターを初期化
 *
 * @param		wk		パレットアニメワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BLINKPALANM_InitAnimeCount( BLINKPALANM_WORK * wk )
{
	wk->cnt = ANM_CNT_MAX;
}
