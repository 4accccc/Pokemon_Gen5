//=============================================================================
/**
 *
 *	@file		ui_easy_clwk.h
 *	@brief  CLWKリソースの、GFL_CLWKの簡易生成＆開放 ユーティリティーです。
 *	@author	hosaka genya
 *	@data		2009.10.08
 *
 *	@note   NCL・NCG・NCE が一対一対応のものにしか使えません。
 *	@note   nsbtx_to_clwk のダミーデータを読み込む際などにご利用ください。
 *
 */
//=============================================================================
#pragma once

#include <gflib.h>

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	OBJリソース管理ワーク
//==============================================================
typedef struct {
  u32               res_ncg;
  u32               res_ncl;
  u32               res_nce;
  CLSYS_DRAW_TYPE   draw_type;
} UI_EASY_CLWK_RES;

//--------------------------------------------------------------
///	圧縮フラグビット
//==============================================================
typedef enum{
  UI_EASY_CLWK_RES_COMP_NONE = 0,     ///< 圧縮なし
  UI_EASY_CLWK_RES_COMP_NCLR = 0x1,   ///< パレット圧縮
  UI_EASY_CLWK_RES_COMP_NCGR = 0x2,   ///< キャラクタ圧縮
} UI_EASY_CLWK_RES_COMP;

//--------------------------------------------------------------
///	CLWK初期化パラメータ
//==============================================================
typedef struct {
  CLSYS_DRAW_TYPE draw_type; ///< 描画先タイプ
  UI_EASY_CLWK_RES_COMP   comp_flg;  ///< 圧縮フラグ
  u32 arc_id;   ///< アーカイブNO
  u32 pltt_id;  ///< パレットリソースNO
  u32 ncg_id;   ///< キャラクタリソースNO
  u32 cell_id;  ///< セルリソースNO
  u32 anm_id;   ///< セルアニメリソースNO
  u8 pltt_line; ///< パレット転送先NO
  u8 pltt_src_ofs;  ///< パレットデータ読み出し開始位置（パレット何本目から転送するか？ 1本=16色）
  u8 pltt_src_num;  ///< パレット転送本数（何本分転送するか？ 1本=16色／0だと全て転送）
  u8 padding[1];
} UI_EASY_CLWK_RES_PARAM;

//=============================================================================
/**
 *								EXTERN
 */
//=============================================================================
extern void UI_EASY_CLWK_LoadResource( UI_EASY_CLWK_RES* res, UI_EASY_CLWK_RES_PARAM* prm, GFL_CLUNIT* unit, HEAPID heapID );
extern void UI_EASY_CLWK_UnLoadResource( UI_EASY_CLWK_RES* res );
extern GFL_CLWK* UI_EASY_CLWK_CreateCLWK( UI_EASY_CLWK_RES* res, GFL_CLUNIT* unit, u8 px, u8 py, u8 anim, HEAPID heapID );

