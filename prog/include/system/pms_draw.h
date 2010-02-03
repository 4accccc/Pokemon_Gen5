//============================================================================
/**
 *
 *	@file		pms_draw.h
 *	@brief  簡易会話表示システム（簡易会話+デコメ管理）
 *	@author	hosaka genya
 *	@data		2009.10.14
 *
 *	@note   BMPWINに対して、デコメを含んだ簡易会話を表示します。
 *	@note   OBJのキャラVRAMを約16 k。パレットを5本消費します。 
 *
 */
//============================================================================
#pragma once

#include <gflib.h>
#include "system/pms_data.h"
#include "print/printsys.h"

typedef struct _PMS_DRAW_WORK PMS_DRAW_WORK;

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  // 簡易会話表示システムで使用するパレット数
  PMS_DRAW_OBJ_PLTT_NUM = 5,
  // デフォルトの空欄塗りつぶしパレット番号
  PMS_DRAW_DEF_NULL_COLOR = 0xF,
};


//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示システム 初期化
 *
 *	@param	GFL_CLUNIT* clunit  アクターユニット
 *	@param  CLSYS_DRAW_TYPE vram_type 上下画面フラグ
 *	@param	que   プリントキュー
 *	@param  font  フォント
 *	@param	obj_pltt_ofs パレットオフセット
 *	@param	id_max  管理する表示ユニット数
 *	@param	heap_id ヒープID
 *
 *	@retval ワーク
 */
//-----------------------------------------------------------------------------
extern PMS_DRAW_WORK* PMS_DRAW_Init( GFL_CLUNIT* clunit, CLSYS_DRAW_TYPE vram_type, PRINT_QUE* que, GFL_FONT* font, u8 obj_pltt_ofs, u8 id_max, HEAPID heap_id );
//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示システム 主処理
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMS_DRAW_Main( PMS_DRAW_WORK* wk );
//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示システム 開放処理
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMS_DRAW_Exit( PMS_DRAW_WORK* wk );
//-----------------------------------------------------------------------------
/**
 *	@brief  指定IDに簡易会話を表示
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *	@param	win 表示するBMPWIN
 *	@param	pms 表示する簡易会話データ
 *	@param	id 表示ユニット管理ID
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMS_DRAW_Print( PMS_DRAW_WORK* wk, GFL_BMPWIN* win, PMS_DATA* pms, u8 id );
//-----------------------------------------------------------------------------
/**
 *	@brief  指定IDに簡易会話を表示
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *	@param	win 表示するBMPWIN
 *	@param	pms 表示する簡易会話データ
 *	@param	id 表示ユニット管理ID
 *	@param  offset 表示オフセット（ドット単位）
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMS_DRAW_PrintOffset( PMS_DRAW_WORK* wk, GFL_BMPWIN* win, PMS_DATA* pms, u8 id, GFL_POINT* offset );
//-----------------------------------------------------------------------------
/**
 *	@brief  指定IDの表示終了チェック
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *
 *	@retval	TRUE:プリント終了
 */
//-----------------------------------------------------------------------------
extern BOOL PMS_DRAW_IsPrintEnd( PMS_DRAW_WORK* wk );
//-----------------------------------------------------------------------------
/**
 *	@brief  指定IDの表示クリア
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *	@param	id 表示ユニット管理ID
 *	@param	is_trans TRUE：スクリーン、キャラを転送する
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMS_DRAW_Clear( PMS_DRAW_WORK* wk, u8 id, BOOL is_trans );
//-----------------------------------------------------------------------------
/**
 *	@brief  指定表示ユニットが表示中かどうかを返す
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *	@param	id 表示ユニット管理ID
 *
 *	@retval TRUE:表示中 FALSE:クリアされている
 */
//-----------------------------------------------------------------------------
extern BOOL PMS_DRAW_IsPrinting( PMS_DRAW_WORK* wk, u8 id );
//-----------------------------------------------------------------------------
/**
 *	@brief  表示ユニットを上書き(双方プリント済みのものに限る)
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *	@param	id_src 入れ替え元
 *	@param	id_dst 入れ替え先
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMS_DRAW_Copy( PMS_DRAW_WORK* wk, u8 id_src, u8 id_dst );
//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話の空欄を埋めるパレット番号指定
 *
 *	@param	PMS_DRAW_WORK* wk　ワーク
 *	@param	pltt_pos パレット番号
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMS_DRAW_SetNullColorPallet( PMS_DRAW_WORK* wk, u8 pltt_pos );
//-----------------------------------------------------------------------------
/**
 *	@brief  文字描画色を指定
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *	@param	color 描画色
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMS_DRAW_SetPrintColor( PMS_DRAW_WORK* wk, PRINTSYS_LSB color );
//-----------------------------------------------------------------------------
/**
 *	@brief  アクターをBGスクロールに追随させるフラグON／OFF (デフォルト無効)
 *
 *	@param	PMS_DRAW_WORK* wk ワーク
 *	@param	is_clwk_auto_scroll TRUE:追随させる
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMS_DRAW_SetCLWKAutoScrollFlag( PMS_DRAW_WORK* wk, BOOL is_clwk_auto_scroll );

