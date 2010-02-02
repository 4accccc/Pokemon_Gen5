//=============================================================================
/**
 *
 *	@file		beacon_detail_local.h
 *	@brief  すれ違い詳細画面ローカルヘッダ
 *	@author	Miyuki Iwasawa
 *	@data		2010.02.01
 *
 */
//=============================================================================

#pragma once

#include "beacon_detail_def.h"

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//オブジェリソース定義
typedef struct _OBJ_RES_SRC{
  u8  type, pltt_ofs, pltt_siz;
  u16 pltt_id;
  u16 cgr_id;
  u16 cell_id;
}OBJ_RES_SRC;

//オブジェリソース管理構造体
typedef struct _OBJ_RES_TBL{
  u32 pltt;
  u32 cgr;
  u32 cell;
}OBJ_RES_TBL;


// Bmpウィンドウ
typedef struct{
  GFL_BMPWIN *win;
  GFL_BMP_DATA* bmp;
  PRINT_UTIL putil;
}BMP_WIN;

typedef struct _BEACON_WIN{
  BMP_WIN prof[BEACON_PROF_MAX];
  BMP_WIN record;
  GFL_BMPWIN* pms;
  GFL_CLWK* cRank;
  GFL_CLWK* cTrainer;
}BEACON_WIN;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;
  HEAPID tmpHeapID;
  int                       seq;
	int											  sub_seq;

  GFL_TCBLSYS*  pTcbSys;

	//描画設定
	BEACON_DETAIL_GRAPHIC_WORK	*graphic;
  ARCHANDLE* handle;
  OBJ_RES_TBL objResNormal;
  OBJ_RES_TBL objResTrainer;
  OBJ_RES_TBL objResUnion;

	//タッチバー
	TOUCHBAR_WORK							*touchbar;

	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msg;
  WORDSET                   *wset;
  STRBUF *str_tmp;
  STRBUF *str_expand;
  STRBUF *str_popup;  //ポップアップ用テンポラリ

#ifdef	BEACON_DETAIL_PRINT_TOOL
	//プリントユーティリティ
	PRINT_UTIL								print_util;
#endif	//BEACON_DETAIL_PRINT_TOOL

  PMS_DRAW_WORK*  pms_draw;
  BEACON_WIN      beacon_win[BEACON_WIN_MAX];

} BEACON_DETAIL_WORK;



