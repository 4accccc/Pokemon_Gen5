//=============================================================================
/**
 *
 *	@file		fmission_app_local.h
 *	@brief  フェスミッションアプリ ローカルヘッダ
 *	@author Miyuki Iwasawa	
 *	@data	  2011.04.22	
 */
//=============================================================================

#pragma once

// 簡易会話表示システム
#include "system/pms_draw.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "fmission_app_gra.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//マップモデルのnsbtx
#include "fieldmap/fldmmdl_mdlres.naix"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//ポケアイコン
#include "pokeicon/pokeicon.h"

//MCSS
#include "system/mcss.h"
#include "system/mcss_tool.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//どうぐアイコン
#include "item/item.h"
#include "item_icon.naix"

//ポケモンBG,OBJ読みこみ
#include "system/poke2dgra.h"

//アプリ共通素材
#include "app/app_menu_common.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/fmission_app.h"

#include "fmission_app_def.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
//#define FMISSION_INFOWIN
#define FMISSION_TOUCHBAR
//#define FMISSION_MCSS
//#define FMISSION_TASKMENU
//#define FMISSION_POKE_ICON     // ポケアイコン(ポケアイコン用アイテムアイコン込み)
////#define FMISSION_POKE2D				// ポケモンBG,OBJ読みこみ
//#define FMISSION_PMSDRAW       // 簡易会話表示

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
///	BG管理ワーク
//==============================================================
typedef struct 
{
	int dummy;
} FMISSION_APP_BGWK;


#ifdef FMISSION_PMSDRAW
#define FMISSION_PMSDRAW_NUM (3) ///< 簡易会話の個数
#endif // FMISSION_PMSDRAW 

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

  int                     seq;  //サブシーケンスNo

	FMISSION_APP_BGWK				wk_bg;

	//描画設定
	FMISSION_APP_GRAWK	*graphic;

#ifdef FMISSION_TOUCHBAR
	//タッチバー
	TOUCHBAR_WORK							*touchbar;
	//以下カスタムボタン使用する場合のサンプルリソース
	u32												ncg_btn;
	u32												ncl_btn;
	u32												nce_btn;
#endif //FMISSION_TOUCHBAR

#ifdef FMISSION_MCSS
	//MCSS
	MCSS_SYS_WORK							*mcss_sys;
	MCSS_WORK									*mcss_wk;
#endif //FMISSION_MCSS

	//フォント
	GFL_FONT									*font;

	//プリントキュー
	PRINT_QUE									*print_que;
	GFL_MSGDATA								*msg;

#ifdef FMISSION_TASKMENU
	//タスクメニュー
	APP_TASKMENU_RES					*menu_res;
	APP_TASKMENU_WORK					*menu;
#endif //FMISSION_TASKMENU

#ifdef FMISSION_POKE_ICON
  // ポケアイコン
  UI_EASY_CLWK_RES      clres_poke_icon;
  GFL_CLWK                  *clwk_poke_icon;
  // ポケアイコン用アイテムアイコン
  UI_EASY_CLWK_RES      clres_poke_item;
  GFL_CLWK                  *clwk_poke_item;
#endif //FMISSION_POKE_ICON

#ifdef FMISSION_POKE2D
	u32												ncg_poke2d;
	u32												ncl_poke2d;
	u32												nce_poke2d;
	GFL_CLWK									*clwk_poke2d;
#endif //FMISSION_POKE2D

#ifdef	FMISSION_PRINT_TOOL
	//プリントユーティリティ
	PRINT_UTIL								print_util;
	u32												seq;
#endif	//FMISSION_PRINT_TOOL

#ifdef FMISSION_PMSDRAW
  GFL_BMPWIN*               pms_win[ FMISSION_PMSDRAW_NUM ];
  PMS_DRAW_WORK*            pms_draw;
#endif //FMISSION_PMSDRAW

} FMISSION_APP_WORK;


