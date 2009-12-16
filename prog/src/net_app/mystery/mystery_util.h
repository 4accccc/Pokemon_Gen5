//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mystery_util.h
 *	@brief  ふしぎなおくりもので使用するモジュール類
 *	@author	Toru=Nagihashi
 *	@date		2009.12.09
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//システム
#include "system/main.h"    //HEAPID
#include "print/gf_font.h"  //GFL_FONT
#include "print/printsys.h"
#include "system/bmp_menulist.h"  //BMPMENULIST_NULL


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					BMPWINメッセージ描画構造体
 *					  ・文字描画はPRINTQUE
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	メッセージウィンドウ
//=====================================
typedef struct _MYSTERY_MSGWIN_WORK MYSTERY_MSGWIN_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern MYSTERY_MSGWIN_WORK * MYSTERY_MSGWIN_Init( u16 frm, u8 x, u8 y, u8 w, u8 h, u8 plt, PRINT_QUE *p_que, HEAPID heapID );
extern void MYSTERY_MSGWIN_Exit( MYSTERY_MSGWIN_WORK* p_wk );
extern void MYSTERY_MSGWIN_Print( MYSTERY_MSGWIN_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font );
extern void MYSTERY_MSGWIN_PrintBuf( MYSTERY_MSGWIN_WORK* p_wk, const STRBUF *cp_strbuf, GFL_FONT *p_font );
extern void MYSTERY_MSGWIN_PrintColor( MYSTERY_MSGWIN_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font, PRINTSYS_LSB lsb );
extern BOOL MYSTERY_MSGWIN_PrintMain( MYSTERY_MSGWIN_WORK* p_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					テキスト描画構造体
 *					  ・ふしぎなおくりものはストリームしない
 *					  ・実はBMPWIN構造体と同じ
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	メッセージウィンドウ
//=====================================
typedef struct _MYSTERY_MSGWIN_WORK MYSTERY_TEXT_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern MYSTERY_TEXT_WORK * MYSTERY_TEXT_Init( u16 frm, u8 font_plt, u8 frm_plt, u16 frm_chr, PRINT_QUE *p_que, HEAPID heapID );
extern MYSTERY_TEXT_WORK * MYSTERY_TEXT_InitOneLine( u16 frm, u8 font_plt, u8 frm_plt, u16 frm_chr, PRINT_QUE *p_que, HEAPID heapID );
extern void MYSTERY_TEXT_Exit( MYSTERY_TEXT_WORK* p_wk );
extern void MYSTERY_TEXT_Print( MYSTERY_TEXT_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font );
extern BOOL MYSTERY_TEXT_PrintMain( MYSTERY_TEXT_WORK* p_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  リスト
 *				    ・簡単にリストを出すために最大値決めうち
 *            ・表示数＝リスト最大数
 *            ・位置固定
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
#define MYSTERY_LIST_SELECT_NULL  (BMPMENULIST_NULL)
#define MYSTERY_LIST_WINDOW_MAX   (3)
//-------------------------------------
///	選択リスト
//=====================================
typedef struct 
{
  GFL_MSGDATA *p_msg;
  GFL_FONT    *p_font;
  PRINT_QUE   *p_que;
  u32 strID[MYSTERY_LIST_WINDOW_MAX];
  u32 list_max;

  u16 frm;
  u16 font_plt;
  u16 frm_plt;
  u16 frm_chr;
} MYSTERY_LIST_SETUP;

//-------------------------------------
///	選択リスト
//=====================================
typedef struct _MYSTERY_LIST_WORK MYSTERY_LIST_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern MYSTERY_LIST_WORK * MYSTERY_LIST_Init( const MYSTERY_LIST_SETUP *cp_setup, HEAPID heapID );
extern void MYSTERY_LIST_Exit( MYSTERY_LIST_WORK *p_wk );
extern u32 MYSTERY_LIST_Main( MYSTERY_LIST_WORK *p_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  BMPWINの集まり  いろんな文字をはる情報画面などで使用
 *				    ・BMPWINを一気に作成し、一気に書き込む
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	設定ワーク
//=====================================
typedef struct 
{
  u8  x;
  u8  y;
  u8  w;
  u8  h;
  u32 strID;
  STRBUF *p_strbuf;
} MYSTERY_MSGWINSET_SETUP_TBL;

//-------------------------------------
///	メッセージウィンドウ
//=====================================
typedef struct _MYSTERY_MSGWINSET_WORK MYSTERY_MSGWINSET_WORK;
//-------------------------------------
///	パブリック
//=====================================
MYSTERY_MSGWINSET_WORK * MYSTERY_MSGWINSET_Init( const MYSTERY_MSGWINSET_SETUP_TBL *cp_tbl, u32 tbl_len, u16 frm, u8 plt, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, GFL_FONT *p_font, HEAPID heapID );
extern void MYSTERY_MSGWINSET_Exit( MYSTERY_MSGWINSET_WORK* p_wk );
extern BOOL MYSTERY_MSGWINSET_PrintMain( MYSTERY_MSGWINSET_WORK* p_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  メニュー
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
#define MYSTERY_MENU_SELECT_NULL  (BMPMENULIST_NULL)
#define MYSTERY_MENU_WINDOW_MAX   (4)

//-------------------------------------
///	メニュー設定
//=====================================
typedef struct 
{
  GFL_MSGDATA *p_msg; //NULLの場合は下記p_strbufを使用する、さもなくばstrIDを使い文章を作成
  GFL_FONT    *p_font;
  PRINT_QUE   *p_que;
  GFL_CLWK    *p_cursor;
  union
  { 
    u32     strID[MYSTERY_MENU_WINDOW_MAX];
    STRBUF  *p_strbuf[MYSTERY_MENU_WINDOW_MAX];  //すぐ開放してもOK
  };
  u32 list_max;

  u16 frm;
  u16 font_plt; // 15.～15-list_maxを背景色に使います（15を14，13に上書き）
  u16 frm_plt;
  u16 frm_chr;
} MYSTERY_MENU_SETUP;

//-------------------------------------
///	メニュー
//=====================================
typedef struct _MYSTERY_MENU_WORK MYSTERY_MENU_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern MYSTERY_MENU_WORK * MYSTERY_MENU_Init( const MYSTERY_MENU_SETUP *cp_setup, HEAPID heapID );
extern void MYSTERY_MENU_Exit( MYSTERY_MENU_WORK *p_wk );
extern u32 MYSTERY_MENU_Main( MYSTERY_MENU_WORK *p_wk );

extern void MYSTERY_MENU_BlinkMain( MYSTERY_MENU_WORK *p_wk, u32 list_num );
