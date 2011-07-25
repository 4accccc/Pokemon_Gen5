//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wbt_download_net_util.h
 *	@brief  共通モジュール
 *	@author	Toru=Nagihashi
 *	@date		2011.05.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "system/main.h"    //HEAPID
#include "print/gf_font.h"  //GFL_FONT
#include "system/bmp_menulist.h"  //BMPMENULIST_NULL
#include "gamesystem/game_data.h"
#include "system/bmp_oam.h"

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					テキスト描画構造体
 *            ・文字描画はストリーム
 *            ・テキスト範囲は画面下部文字2行ほど
 *            ・BMPWINの周りにフレームがつく
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	描画方式
//=====================================
typedef enum
{
  WBTDLNET_TEXT_TYPE_QUE,     //プリントキューを使う
  WBTDLNET_TEXT_TYPE_STREAM,  //ストリームを使う(読み飛ばしあり、速さ設定依存)
  WBTDLNET_TEXT_TYPE_WAIT,   //待機アイコンをつかう  終了はEndWait
  WBTDLNET_TEXT_TYPE_STREAM_FIX,  //ストリームを使う（固定速さ）

  WBTDLNET_TEXT_TYPE_MAX,    //c内部にて使用
} WBTDLNET_TEXT_TYPE;

//-------------------------------------
///	座標位置
//=====================================
typedef enum 
{
  WBTDLNET_TEXT_POS_ABSOLUTE,     //絶対位置
  WBTDLNET_TEXT_POS_WH_CENTER,  //相対座標  幅、高さともに中央
  WBTDLNET_TEXT_POS_H_CENTER,    //相対座標  高さのみ中央
  WBTDLNET_TEXT_POS_W_BACK,     //後ろから
} WBTDLNET_TEXT_POS;

//-------------------------------------
///	メッセージウィンドウ
//=====================================
typedef struct _WBTDLNET_TEXT_WORK WBTDLNET_TEXT_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern WBTDLNET_TEXT_WORK * WBTDLNET_TEXT_Init( u16 frm, u16 x, u16 y, u16 w, u16 h, u16 font_plt, GFL_FONT *p_font, HEAPID heapID );
extern void WBTDLNET_TEXT_Exit( WBTDLNET_TEXT_WORK* p_wk );
extern void WBTDLNET_TEXT_Draw( WBTDLNET_TEXT_WORK* p_wk );
extern void WBTDLNET_TEXT_Print( WBTDLNET_TEXT_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, WBTDLNET_TEXT_TYPE type );
extern void WBTDLNET_TEXT_PrintBuf( WBTDLNET_TEXT_WORK* p_wk, const STRBUF *cp_strbuf, WBTDLNET_TEXT_TYPE type );
extern void WBTDLNET_TEXT_SetColor( WBTDLNET_TEXT_WORK* p_wk, PRINTSYS_LSB color );
extern void WBTDLNET_TEXT_SetPos( WBTDLNET_TEXT_WORK* p_wk, s16 x, s16 y, WBTDLNET_TEXT_POS type );
extern BOOL WBTDLNET_TEXT_IsMsgEnd( const WBTDLNET_TEXT_WORK* cp_wk );
extern void WBTDLNET_TEXT_EndWait( WBTDLNET_TEXT_WORK* p_wk );
extern void WBTDLNET_TEXT_WriteWindowFrame( WBTDLNET_TEXT_WORK *p_wk, u16 frm_chr, u8 frm_plt );
extern void WBTDLNET_TEXT_ClearCharacter( WBTDLNET_TEXT_WORK* p_wk );
extern void WBTDLNET_TEXT_ClearScreen( WBTDLNET_TEXT_WORK* p_wk );
extern void WBTDLNET_TEXT_ChangePalette( WBTDLNET_TEXT_WORK* p_wk, u8 plt );

#ifdef PM_DEBUG
extern void WBTDLNET_TEXT_PrintDebug( WBTDLNET_TEXT_WORK* p_wk, const u16 *cp_str, u16 len, GFL_FONT *p_font );
#endif

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					メッセージセット構造体
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	メッセージセットウィンドウ
//=====================================
typedef struct _WBTDLNET_TEXTSET_WORK WBTDLNET_TEXTSET_WORK;

//-------------------------------------
///	メッセージセット初期化
//=====================================
typedef struct
{
  u16 x;
  u16 y;
  u16 w;
  u16 h;
  u32 strID;
} WBTDLNET_TEXTSET_WND_DATA;

//-------------------------------------
///	パブリック
//=====================================
extern WBTDLNET_TEXTSET_WORK * WBTDLNET_TEXTSET_Init( const WBTDLNET_TEXTSET_WND_DATA *cp_data_tbl, u16 data_num, u16 frm, u16 font_plt, GFL_FONT *p_font, GFL_MSGDATA *p_msg, HEAPID heapID );
extern void WBTDLNET_TEXTSET_Exit( WBTDLNET_TEXTSET_WORK* p_wk );
extern void WBTDLNET_TEXTSET_Draw( WBTDLNET_TEXTSET_WORK* p_wk );
extern BOOL WBTDLNET_TEXTSET_IsDrawEnd( const WBTDLNET_TEXTSET_WORK* cp_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  リスト
 *				    ・簡単にリストを出すために最大値決めうち
 *            ・表示数＝リスト最大数
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
#define WBTDLNET_LIST_SELECT_NULL    (BMPMENULIST_NULL)
#define WBTDLNET_LIST_WINDOW_MAX     (4)

//-------------------------------------
///	選択リスト
//=====================================
typedef struct 
{
  GFL_MSGDATA *p_msg;
  GFL_FONT    *p_font;
  PRINT_QUE   *p_que;
  u32 strID[WBTDLNET_LIST_WINDOW_MAX];
  u32 list_max;

  u16 frm;
  u16 font_plt;
  u16 frm_plt;
  u16 frm_chr;

  BOOL  is_cancel;
  u16   cancel_idx;
  u16   default_idx;
} WBTDLNET_LIST_SETUP;

//-------------------------------------
///	選択リスト
//=====================================
typedef struct _WBTDLNET_LIST_WORK WBTDLNET_LIST_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern WBTDLNET_LIST_WORK * WBTDLNET_LIST_Init( const WBTDLNET_LIST_SETUP *cp_setup, HEAPID heapID );
extern WBTDLNET_LIST_WORK * WBTDLNET_LIST_InitEx( const WBTDLNET_LIST_SETUP *cp_setup, u8 x, u8 y, u8 w, u8 h, HEAPID heapID );
extern void WBTDLNET_LIST_Exit( WBTDLNET_LIST_WORK *p_wk );
extern u32 WBTDLNET_LIST_Main( WBTDLNET_LIST_WORK *p_wk );
extern BOOL WBTDLNET_LIST_Draw( WBTDLNET_LIST_WORK *p_wk );

#if 0
//=============================================================================
/**
 *					メニュー
*/
//=============================================================================
#define WBTDLNET_MENU_WINDOW_MAX (4)
//-------------------------------------
///	メニュー
//=====================================
typedef struct _WBTDLNET_MENU_WORK WBTDLNET_MENU_WORK;
//-------------------------------------
///	初期化構造体
//=====================================
typedef struct
{
  u32 frm;
  u32 font_plt;
  u32 frm_chr;
  u32 frm_plt;
  GFL_MSGDATA       *p_msg;
  GFL_FONT          *p_font;
  u32 max;
  s32 cursor;
  struct
  {
    u32 strID;
    u8  x;
    u8  y;
    u8  w;
    u8  h;
  } wnd_data[WBTDLNET_MENU_WINDOW_MAX];
  BOOL (*DecideCallBack)( int index, void *p_wk_adrs ); //押せるならばTRUE
  void *p_wk_adrs;
}WBTDLNET_MENU_PARAM;
//-------------------------------------
///	パブリック
//=====================================
extern WBTDLNET_MENU_WORK * WBTDLNET_MENU_Init( const WBTDLNET_MENU_PARAM *cp_param, HEAPID heapID );
extern void WBTDLNET_MENU_Exit( WBTDLNET_MENU_WORK *p_wk );
extern void WBTDLNET_MENU_Main( WBTDLNET_MENU_WORK *p_wk );
extern void WBTDLNET_MENU_Draw( WBTDLNET_MENU_WORK *p_wk );
extern BOOL WBTDLNET_MENU_IsDrawEnd( const WBTDLNET_MENU_WORK *cp_wk );
extern s32 WBTDLNET_MENU_GetSelect( const WBTDLNET_MENU_WORK *cp_wk );
extern BOOL WBTDLNET_MENU_IsDecide( const WBTDLNET_MENU_WORK *cp_wk );
extern void WBTDLNET_MENU_ReStart( WBTDLNET_MENU_WORK *p_wk );
extern const WBTDLNET_MENU_PARAM * WBTDLNET_MENU_GetParam( const WBTDLNET_MENU_WORK *cp_wk );
extern BOOL WBTDLNET_MENU_IsChangeSelect( const WBTDLNET_MENU_WORK *cp_wk );
#endif
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  シーケンス管理
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	シーケンスワーク
//=====================================
typedef struct _WBTDLNET_SEQ_WORK WBTDLNET_SEQ_WORK;

//-------------------------------------
///	シーケンス関数
//=====================================
typedef void (*WBTDLNET_SEQ_FUNCTION)( WBTDLNET_SEQ_WORK *p_wk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	パブリック
//=====================================
extern WBTDLNET_SEQ_WORK *WBTDLNET_SEQ_Init( u32 stack_max, void *p_wk_adrs, WBTDLNET_SEQ_FUNCTION seq_function, HEAPID heapID );
extern void WBTDLNET_SEQ_Exit( WBTDLNET_SEQ_WORK *p_wk );
extern void WBTDLNET_SEQ_Main( WBTDLNET_SEQ_WORK *p_wk );
extern BOOL WBTDLNET_SEQ_IsEnd( const WBTDLNET_SEQ_WORK *cp_wk );
extern void WBTDLNET_SEQ_Next( WBTDLNET_SEQ_WORK *p_wk, WBTDLNET_SEQ_FUNCTION seq_function );
extern void WBTDLNET_SEQ_Call( WBTDLNET_SEQ_WORK *p_wk, WBTDLNET_SEQ_FUNCTION seq_function );
extern void WBTDLNET_SEQ_End( WBTDLNET_SEQ_WORK *p_wk );
extern void WBTDLNET_SEQ_Return( WBTDLNET_SEQ_WORK *p_wk );
extern void WBTDLNET_SEQ_SetReservSeq( WBTDLNET_SEQ_WORK *p_wk, int seq );
extern void WBTDLNET_SEQ_NextReservSeq( WBTDLNET_SEQ_WORK *p_wk );
extern BOOL WBTDLNET_SEQ_IsCompireFunc( const WBTDLNET_SEQ_WORK *cp_wk, WBTDLNET_SEQ_FUNCTION seq_function );
extern void WBTDLNET_SEQ_AddLocalSeq( WBTDLNET_SEQ_WORK *p_wk, int add );
extern void WBTDLNET_SEQ_Clear( WBTDLNET_SEQ_WORK *p_wk );
//汎用シーケンス
extern void WBTDLNET_SEQ_FadeInBlack( WBTDLNET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
extern void WBTDLNET_SEQ_FadeOutBlack( WBTDLNET_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  ディスプレイ作成
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	ディスプレイ作成テーブル
//=====================================
typedef struct
{
  void (*Create)( void *p_wk_adrs, HEAPID heapID );
  BOOL (*IsEndCreate)( void *p_wk_adrs );
  void (*Delete)( void *p_wk_adrs );
  BOOL (*IsEndDelete)( void *p_wk_adrs );
} WBTDLNET_DISPLAY_FUNCTION;

//-------------------------------------
///	シーケンスワーク
//=====================================
typedef struct _WBTDLNET_DISPLAY_GENELATER WBTDLNET_DISPLAY_GENELATER;

//-------------------------------------
///	パブリック
//=====================================
extern WBTDLNET_DISPLAY_GENELATER * WBTDLNET_DISPLAY_Init( void *p_wk_adrs, HEAPID heapID );
extern void WBTDLNET_DISPLAY_Exit( WBTDLNET_DISPLAY_GENELATER *p_wk );
extern void WBTDLNET_DISPLAY_Main( WBTDLNET_DISPLAY_GENELATER *p_wk );
extern void WBTDLNET_DISPLAY_Create( WBTDLNET_DISPLAY_GENELATER *p_wk, const WBTDLNET_DISPLAY_FUNCTION *cp_func, HEAPID heapID );
extern void WBTDLNET_DISPLAY_Delete( WBTDLNET_DISPLAY_GENELATER *p_wk );
extern BOOL WBTDLNET_DISPLAY_IsComplete( const WBTDLNET_DISPLAY_GENELATER *cp_wk );
extern void WBTDLNET_DISPLAY_DeleteForce( WBTDLNET_DISPLAY_GENELATER *p_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  TEXTOBJ　OAMで文字を書くモジュール
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	メッセージOAM
//=====================================
typedef struct _WBTDLNET_TEXTOBJ_WORK WBTDLNET_TEXTOBJ_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern WBTDLNET_TEXTOBJ_WORK * WBTDLNET_TEXTOBJ_Init( const GFL_CLWK_DATA *cp_cldata, u8 w, u8 h, u32 plt_idx, u8 plt_ofs, CLSYS_DRAW_TYPE draw_type, BMPOAM_SYS_PTR p_bmpoam_sys, HEAPID heapID );
extern void WBTDLNET_TEXTOBJ_Exit( WBTDLNET_TEXTOBJ_WORK* p_wk );
extern void WBTDLNET_TEXTOBJ_Clear( WBTDLNET_TEXTOBJ_WORK* p_wk );
extern void WBTDLNET_TEXTOBJ_Print( WBTDLNET_TEXTOBJ_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font );
extern void WBTDLNET_TEXTOBJ_PrintBuf( WBTDLNET_TEXTOBJ_WORK* p_wk, const STRBUF *cp_strbuf, GFL_FONT *p_font );
extern void WBTDLNET_TEXTOBJ_SetStrColor( WBTDLNET_TEXTOBJ_WORK* p_wk, PRINTSYS_LSB color );
extern void WBTDLNET_TEXTOBJ_SetStrPos( WBTDLNET_TEXTOBJ_WORK* p_wk, s16 x, s16 y, WBTDLNET_TEXT_POS type );
extern void WBTDLNET_TEXTOBJ_Draw( WBTDLNET_TEXTOBJ_WORK* p_wk );
extern BOOL WBTDLNET_TEXTOBJ_IsDrawEnd( const WBTDLNET_TEXTOBJ_WORK* cp_wk );
extern BMPOAM_ACT_PTR WBTDLNET_TEXTOBJ_GetBmpOamAct( WBTDLNET_TEXTOBJ_WORK* p_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  等速加速運動
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
typedef struct
{
	GFL_POINT now_val;		//現在の値
	GFL_POINT start_val;	//開始の値
	GFL_POINT end_val;		//終了の値
	GFL_POINT add_val;		//加算値(誤差をださないようにここだけfx)
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
}WBTDLNET_VELOCITY_WORK;
extern void WBTDLNET_VEL_Init( WBTDLNET_VELOCITY_WORK* p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, int sync );
extern BOOL WBTDLNET_VEL_Main( WBTDLNET_VELOCITY_WORK* p_wk );
extern void WBTDLNET_VEL_GetPos( const WBTDLNET_VELOCITY_WORK* cp_wk, GFL_CLACTPOS *p_pos );
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  等加速度運動
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
typedef struct {
	VecFx32 now_val;		//現在の値
	VecFx32 start_val;	//開始の値
	VecFx32 end_val;		//終了の値
  VecFx32 vec;
	fx32 velocity;	//初速度
	fx32 aclr;			//加速度
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} WBTDLNET_ACLR_WORK;
extern void WBTDLNET_ACLR_Init( WBTDLNET_ACLR_WORK* p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, fx32 velocity, int sync );
extern BOOL WBTDLNET_ACLR_Main( WBTDLNET_ACLR_WORK* p_wk );
extern void WBTDLNET_ACLR_GetPos( const WBTDLNET_ACLR_WORK* cp_wk, GFL_CLACTPOS *p_pos );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  ETC
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
extern void WBTDLNET_UTIL_MainPltAnm( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 cnt, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end );
extern void WBTDLNET_UTIL_MainPltAnmLine( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 cnt, u8 plt_num, GXRgb start[], GXRgb end[] );
