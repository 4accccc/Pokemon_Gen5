#ifdef __cplusplus
extern "C" {
#endif
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wf2dmap_scrdraw.h
 *	@brief		スクロール描画設定処理
 *	@author		tomoya takahshi
 *	@data		2007.03.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __WF2DMAP_SCRDRAW_H__
#define __WF2DMAP_SCRDRAW_H__

#include <gflib.h>
#include "net_app/wifi2dmap/wf2dmap_common.h"
#include "net_app/wifi2dmap/wf2dmap_map.h"
#include "net_app/wifi2dmap/wf2dmap_scroll.h"



//-----------------------------------------------------------------------------
/**
 *			使用制限
 *
 *			ＢＧ面
 *			１：スクリーンバッファを0x1000サイズ使用します。
 *			２：VBlank内でGF_BGL_VBlankFunc(...)を必ず呼んでください
 *			ＯＡＭ
 *			１：clact_utilのデフォルトレンダラーで初期化してある
 */
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	スクロール描画ワーク
//=====================================
typedef struct _WF2DMAP_SCRDRAW WF2DMAP_SCRDRAW;

//-------------------------------------
///	スクロール描画システム初期化データ
//	スクロールさせるサーフェース設定
//	スクロール処理させるＢＧ面コントロール設定
//=====================================
typedef struct {
	u8	surface_type;	// サーフェース面（CLACT_U_EASYRENDER_SURFACE_MAIN/CLACT_U_EASYRENDER_SURFACE_SUB）
	u8	bg_frame;		// BGフレーム面
	u8	colorMode;		// カラーモード(GX_BG_COLORMODE_16など)
	u8	screenBase;		// スクリーンベース(GX_BG_SCRBASE_0xe000など)
	u8	charbase;		// キャラクタベース(GX_BG_CHARBASE_0x00000など)
    u8	bgExtPltt;		//ＢＧ拡張パレットスロット選択(GX_BG_EXTPLTT_01など)
	u8	priority;		//表示プライオリティー
	u8	mosaic;			//モザイク設定
	u8	arcid_scrn;		// 読み込むスクリーンデータのアーカイブID
	u8	dataid_scrn;	// 読み込むスクリーンデータのデータID
	u8	arcdata_comp;	// 読み込むスクリーンデータが圧縮されているか
} WF2DMAP_SCRDRAWINIT;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// システム作成
extern WF2DMAP_SCRDRAW* WF2DMAP_SCRDrawSysInit( GFL_CLSYS_REND* p_render, GFL_BG_INI* p_bgl, const WF2DMAP_SCRDRAWINIT* cp_init, u32 heapID );
extern void WF2DMAP_SCRDrawSysExit( WF2DMAP_SCRDRAW* p_sys );

// スクロール反映
extern void WF2DMAP_SCRDrawSysMain( WF2DMAP_SCRDRAW* p_sys, const WF2DMAP_SCROLL* cp_scroll );

#endif		// __WF2DMAP_SCRDRAW_H__


#ifdef __cplusplus
} /* extern "C" */
#endif
