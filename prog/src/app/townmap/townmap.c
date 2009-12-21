//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		map_main.c
 *	@brief	タウンマップメイン
 *	@author	Toru=Nagihashi
 *	@data		2009.07.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>

//	constant
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"

//	archive
#include "arc_def.h"
#include "townmap_gra.naix"
#include "message.naix"
#include "font/font.naix"

//	common
#include "app/app_menu_common.h"

//	Module
#include "print/gf_font.h"
#include "print/printsys.h"
#include "field/zonedata.h"
#include "townmap_data_sys.h"
#include "ui/touchbar.h"	//タッチバー

//mine
#include "townmap_grh.h"
#include "app/townmap.h"

//sound
#include "townmap_snd.h"

//debug
#include "debug/debugwin_sys.h"


FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	SWITCH
//=====================================
#ifdef PM_DEBUG
#define DEBUG_MENU_USE
#define DEBUG_PRINT_USE
#define DEBUG_POS_CHECK
#define DEBUG_GAMESYS_NONE
static GFL_POINT s_debug_pos	=
{	
	80, 25
};
#endif //PM_DEBUG


//-------------------------------------
///	COMMON
//=====================================
#define RULE_PLACE_MSG_MAX	(6)

#define MAP_SCALE_MOVE_LIMIT_TOP		(-192/2)
#define MAP_SCALE_MOVE_LIMIT_LEFT		(-256/2)
#define MAP_SCALE_MOVE_LIMIT_BOTTOM	(MAP_SCALE_MOVE_LIMIT_TOP+192 - (APPBAR_MENUBAR_H*GFL_BG_1CHRDOTSIZ))
#define MAP_SCALE_MOVE_LIMIT_RIGHT	(MAP_SCALE_MOVE_LIMIT_LEFT+256)

#define MAP_SCALE_CENTER_DISTANCE_UP			(70)
#define MAP_SCALE_CENTER_DISTANCE_DOWN		(20)

//OBJ優先度
enum
{	
	OBJ_PRIORITY_CURSOR,
	OBJ_PRIORITY_FRONTEND,
	OBJ_PRIORITY_MARK,
	OBJ_PRIORITY_EFFECT,
	OBJ_PRIORITY_MAPOBJ,
};

//中心座標
static const GFL_POINT sc_center_pos =
{	
	256/2, 192/2
};

//-------------------------------------
///	PLACE
//=====================================
#define PLACE_PULL_R				(12)
#define PLACE_PULL_STRONG		(FX32_CONST(1))	//吸い込む強さ（カーソルより弱く）
#define PLACEDATA_ANIME_SEQ_VANISH	(0xFF)
enum
{	
	//並びはタウンマップ用パラメータ(TOWNMAP_DATA_PARAM)に順ずる（開始オフセットのみ）
	PLACE_DATA_PARAM_POS_X,
	PLACE_DATA_PARAM_POS_Y,
	PLACE_DATA_PARAM_CURSOR_X,
	PLACE_DATA_PARAM_CURSOR_Y,
	PLACE_DATA_PARAM_HIT_START_X,
	PLACE_DATA_PARAM_HIT_START_Y,
	PLACE_DATA_PARAM_HIT_END_X,
	PLACE_DATA_PARAM_HIT_END_Y,
	PLACE_DATA_PARAM_HIT_WIDTH,
	
	PLACE_DATA_PARAM_MAX
};

//-------------------------------------
///	APPBAR
//=====================================
//リソース
enum
{	
	APPBAR_RES_SCALE_PLT,
	APPBAR_RES_SCALE_CHR,
	APPBAR_RES_SCALE_CEL,

	APPBAR_RES_MAX
};
//カスタムボタン
#define TOUCHBAR_ICON_SCALEUP	(TOUCHBAR_ICON_CUTSOM1)
#define TOUCHBAR_ICON_SCALEDOWN	(TOUCHBAR_ICON_CUTSOM2)

//範囲
#define APPBAR_MENUBAR_H	(3)
#define APPBAR_MENUBAR_Y	(21)

//-------------------------------------
///	CURSOR
//=====================================
//CLWK
enum
{	
	CURSOR_CLWK_CURSOR,
	CURSOR_CLWK_RING,
	
	CURSOR_CLWK_MAX
};
//位置
#define CURSOR_POS_DEFAULT_X		(128)
#define CURSOR_POS_DEFAULT_Y		(96)
//速度
#define CURSOR_ADD_SPEED				(3)
//制限
#define CURSOR_MOVE_LIMIT_TOP			(0+8)
#define CURSOR_MOVE_LIMIT_BOTTOM	(192- (APPBAR_MENUBAR_H*GFL_BG_1CHRDOTSIZ))
#define CURSOR_MOVE_LIMIT_LEFT		(0)
#define CURSOR_MOVE_LIMIT_RIGHT		(256-8)

#define CURSOR_MAP_MOVE_RANGE_TOP						(16)
#define CURSOR_MAP_MOVE_RANGE_BOTTOM_APPBAR	((APPBAR_MENUBAR_Y)*GFL_BG_1CHRDOTSIZ)
#define CURSOR_MAP_MOVE_RANGE_BOTTOM				(CURSOR_MAP_MOVE_RANGE_BOTTOM_APPBAR-32)
#define CURSOR_MAP_MOVE_RANGE_LEFT					(16)
#define CURSOR_MAP_MOVE_RANGE_RIGHT					(256-16)

//-------------------------------------
///		INFO
//=====================================
//文字面範囲
#define INFO_BMPWIN_X	(0)
#define INFO_BMPWIN_Y	(0)
#define INFO_BMPWIN_W	(32)
#define INFO_BMPWIN_H	(24)

//文字位置
#define INFO_STR_PLACE_CENTERING	//OFFにするとINFO_STR_PLACE_Xの定義を使い座標指定します
#define INFO_STR_PLACE_X	(16)

#define INFO_STR_PLACE_Y	(48)
#define INFO_STR_GUIDE_X	(32)
#define INFO_STR_GUIDE_Y	(80)
#define INFO_STR_PLACE1_X	(32)
#define INFO_STR_PLACE1_Y	(128)
#define INFO_STR_PLACE2_X	(32)
#define INFO_STR_PLACE2_Y	(144)
#define INFO_STR_PLACE3_X	(32)
#define INFO_STR_PLACE3_Y	(160)
#define INFO_STR_PLACE4_X	(144)
#define INFO_STR_PLACE4_Y	(128)
#define INFO_STR_PLACE5_X	(144)
#define INFO_STR_PLACE5_Y	(144)
#define INFO_STR_PLACE6_X	(144)
#define INFO_STR_PLACE6_Y	(160)

//-------------------------------------
///	MSGWND
//=====================================
#define MSGWND_BMPWIN_SKY_X	(0)
#define MSGWND_BMPWIN_SKY_Y	(21)
#define MSGWND_BMPWIN_SKY_W	(24)
#define MSGWND_BMPWIN_SKY_H	(3)

#define MSGWND_STR_SKY_X	(16)
#define MSGWND_STR_SKY_Y	(0)

//-------------------------------------
///	地名ウィンドウ	PLACEWND
//=====================================
#define PLACEWND_BMPWIN_X	(0)
#define PLACEWND_BMPWIN_Y	(0)
#define PLACEWND_BMPWIN_W	(24)
#define PLACEWND_BMPWIN_H	(3)

#define PLACEWND_STR_X		(16)
#define PLACEWND_STR_Y		(2)

#define PLACEWND_WND_POS_X		(128)
#define PLACEWND_WND_POS_Y		(96)

//-------------------------------------
///	MAP
//=====================================
#define MAP_SCALE_SYNC	(10)

//-------------------------------------
///	UI
//=====================================
#define UI_DRAG_MOVE_RATE						(FX32_CONST(1))	
#define UI_SLIDE_MOVE_SYNC_MIN			(1)		//このシンク以上でスライドすれば有効
#define UI_SLIDE_MOVE_SYNC_MAX			(10)	//このシンク以内でスライドすれば有効

#define UI_SLIDE_MOVE_DISTANCE_MIN	(FX32_CONST(20))	//この距離以上でスライドすれば有効
#define UI_SLIDE_MOVE_DISTANCE_MAX	(FX32_CONST(100))	//この距離以内でスライドすれば有効

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	下画面バー(アプリケーションバーと勝手に命名)
//=====================================
typedef struct 
{
	u32						res[APPBAR_RES_MAX];
	TOUCHBAR_WORK *p_touchbar;
	GAMEDATA			*p_gamedata;
} APPBAR_WORK;

//-------------------------------------
///	場所情報
//=====================================
typedef struct 
{
	GFL_CLWK						*p_clwk;

	//オリジナルデータ
	const TOWNMAP_DATA	*cp_data;
	u32									data_idx;

	//拡大縮小スクロールしたときのデータ
	s32		param[PLACE_DATA_PARAM_MAX];
	u8		non_active_anm;
	u8		active_anm;
	u16		now_anm;

	fx32	scale;
	BOOL	is_arrive;	//到達フラグ
	BOOL	is_sky;			//空を飛べるフラグTOWNMAP_DATA_PARAM_SKY_FLAGの加工値
} PLACE_DATA;
//-------------------------------------
///	マーキング
//=====================================
typedef struct
{
	GFL_CLWK	*p_clwk;
	const PLACE_DATA *cp_data;
} PLACE_MARK;
//-------------------------------------
///	場所処理
//=====================================
typedef struct
{
	GFL_CLUNIT			*p_clunit;	//場所はUNITを変えるので作成する
	GFL_CLSYS_REND	*p_rend;
	PLACE_DATA			*p_place;
	u32							data_num;
	GFL_POINT				pos;	//PLACE_DATAの基準座標

	s16	limit_top;
	s16	limit_left;
	s16 limit_right;
	s16	limit_bottom;

	PLACE_MARK	player;
} PLACE_WORK;
//-------------------------------------
///	カーソル処理
//=====================================
typedef struct 
{
	GFL_CLWK	*p_clwk[CURSOR_CLWK_MAX];
	GFL_CLACTPOS	pos;
	BOOL					is_trg;
	BOOL					is_pull;
	const PLACE_DATA *cp_target;
} CURSOR_WORK;
//-------------------------------------
///	地図処理	
//=====================================
typedef struct 
{
	u8	map_frm;
	u8	road_frm;
	u16	sync;

	s16	limit_top;
	s16	limit_left;
	s16 limit_right;
	s16	limit_bottom;

	BOOL	is_scale_end;
	BOOL	is_scale_req;
	fx32	now_scale;
	fx32	next_scale;
	fx32	init_scale;

	GFL_POINT	pos;
	GFL_POINT	next_pos;
	GFL_POINT	init_pos;

} MAP_WORK;
//-------------------------------------
///	上画面情報
//=====================================
typedef struct
{
	GFL_BMPWIN	*p_bmpwin;
	STRBUF			*p_strbuf;			//汎用バッファ
	GFL_FONT		*p_font;				//受け取る
	const GFL_MSGDATA	*cp_place_msg;		//受け取り
	const GFL_MSGDATA	*cp_guide_msg;		//受け取り
} INFO_WORK;
//-------------------------------------
///	メッセージ表示ウィンドウ
//=====================================
typedef struct
{
	GFL_BMPWIN*				p_bmpwin;
	STRBUF*						p_strbuf;
	GFL_FONT*				  p_font;
  const GFL_MSGDATA*			cp_msg;
} MSGWND_WORK;
//-------------------------------------
///	地名ウィンドウ
//=====================================
typedef struct 
{
	MSGWND_WORK	msgwnd;
	GFL_CLWK		*p_clwk;	//受け取り
	const PLACE_DATA *cp_data;
	BOOL				is_update;
	BOOL				is_start;
} PLACEWND_WORK;
//-------------------------------------
///	UI
//=====================================
typedef struct 
{
	VecFx32	start;
	VecFx32	end;
	u32 slide_sync;
	BOOL is_slide;
	BOOL is_reset;
	VecFx32	v;
} UI_WORK;
//-------------------------------------
///	シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_param_adrs );
struct _SEQ_WORK
{
	SEQ_FUNCTION	seq_function;
	BOOL is_end;
	int seq;
	void *p_param_adrs;
}; 
//-------------------------------------
///	メインワーク
//=====================================
typedef struct 
{
	//グラフィックシステム
	TOWNMAP_GRAPHIC_SYS	*p_grh;

	//シーケンス
	SEQ_WORK			seq;

	//アプリケーションバー
	APPBAR_WORK		appbar;

	//データ
	TOWNMAP_DATA	*p_data;

	//カーソル
	CURSOR_WORK		cursor;

	//場所
	PLACE_WORK		place;

	//地図
	MAP_WORK			map;

	//上画面情報
	INFO_WORK			info;

	//空を飛ぶ？のメッセージ面
	MSGWND_WORK		msgwnd;

	//地名ウィンドウ
	PLACEWND_WORK	placewnd;

	//タッチ
	UI_WORK				ui;

	//共通利用システム
	GFL_FONT			*p_font;
	GFL_MSGDATA		*p_place_msg;
	GFL_MSGDATA		*p_guide_msg;

	//引数
	TOWNMAP_PARAM	*p_param;

	//選択中のもの
	const PLACE_DATA *cp_select;

	//拡大中かどうか
	BOOL	is_scale;
	
#ifdef PM_DEBUG
	BOOL	is_print_debug;
	BOOL	is_place_visible_debug;
	BOOL	is_arrive_debug;
	BOOL	is_checkpos_debug;
	u32		checkpos_num_debug;
#endif

} TOWNMAP_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT TOWNMAP_PROC_Init(
		GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT TOWNMAP_PROC_Exit(
		GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT TOWNMAP_PROC_Main(
		GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
//-------------------------------------
///	SEQ
//=====================================
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param_adrs, SEQ_FUNCTION seq_function );
static void SEQ_Exit( SEQ_WORK *p_wk );
static void SEQ_Main( SEQ_WORK *p_wk );
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk );
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( SEQ_WORK *p_wk );
//-------------------------------------
///	SEQFUNC
//=====================================
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs );
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs );
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs );
//-------------------------------------
///	APPBAR
//=====================================
static void APPBAR_Init( APPBAR_WORK *p_wk, TOWNMAP_MODE mode, GFL_CLUNIT* p_unit, u8 bar_frm, u8 bg_plt, u8 obj_plt, GAMEDATA *p_gamedata, HEAPID heapID );
static void APPBAR_Exit( APPBAR_WORK *p_wk );
static void APPBAR_Main( APPBAR_WORK *p_wk );
static void APPBAR_SetActive( APPBAR_WORK *p_wk, TOUCHBAR_ICON icon, BOOL is_active );
static TOUCHBAR_ICON APPBAR_GetTrg( const APPBAR_WORK *cp_wk );
//-------------------------------------
///	CURSOR
//=====================================
static void CURSOR_Init( CURSOR_WORK *p_wk, GFL_CLWK *p_cursor, GFL_CLWK *p_ring, HEAPID heapID );
static void CURSOR_Exit( CURSOR_WORK *p_wk );
static void CURSOR_Main( CURSOR_WORK *p_wk, const PLACE_WORK *cp_place );
static void CURSOR_GetPos( const CURSOR_WORK *cp_wk, GFL_POINT *p_pos );
static void CURSOR_SetPos( CURSOR_WORK *p_wk, const GFL_POINT *cp_pos );
static BOOL CURSOR_GetTrg( const CURSOR_WORK *cp_wk );
static BOOL CURSOR_GetPointTrg( const CURSOR_WORK *cp_wk, u32 *p_x, u32 *p_y );
static int  CURSOR_GetContButton( const CURSOR_WORK *cp_wk );
static void CURSOR_SetTarget( CURSOR_WORK *p_wk, const PLACE_DATA *cp_data, const GFL_POINT *cp_wld );
static const PLACE_DATA * CURSOR_GetTarget( const CURSOR_WORK *cp_wk );
static void CURSOR_SetPullEnable( CURSOR_WORK *p_wk, BOOL on_off );
//-------------------------------------
///	PLACE
//=====================================
static void PLACE_Init( PLACE_WORK *p_wk, u16 now_zone_ID, u16 esc_zone_ID, const TOWNMAP_DATA *cp_data, const TOWNMAP_GRAPHIC_SYS *cp_grh, EVENTWORK *p_ev, HEAPID heapID, BOOL is_debug );
static void PLACE_Exit( PLACE_WORK *p_wk );
static void PLACE_Main( PLACE_WORK *p_wk );
static void PLACE_Scale( PLACE_WORK *p_wk, fx32 scale, const GFL_POINT *cp_center_start, const GFL_POINT *cp_center_next );
static void PLACE_SetVisible( PLACE_WORK *p_wk, BOOL is_visible );
static void PLACE_SetWldPos( PLACE_WORK *p_wk, const GFL_POINT *cp_pos );
static void PLACE_GetWldPos( const PLACE_WORK *cp_wk, GFL_POINT *p_pos );
static void PLACE_AddWldPos( PLACE_WORK *p_wk, const GFL_POINT *cp_pos );
static const PLACE_DATA* PLACE_Hit( const PLACE_WORK *cp_wk, const CURSOR_WORK *cp_cursor );
static const PLACE_DATA* PLACE_PullHit( const PLACE_WORK *cp_wk, const CURSOR_WORK *cp_cursor );
static s32 PLACEDATA_GetParam( const PLACE_DATA *cp_wk, TOWNMAP_DATA_PARAM param );
static void PLACE_Active( PLACE_WORK *p_wk, const PLACE_DATA *cp_data );
static void PLACE_NonActive( PLACE_WORK *p_wk );
static const PLACE_DATA *PLACE_GetDataByZoneID( const PLACE_WORK *cp_wk, u16 zoneID, u16 escapeID );
static void PlaceData_Init( PLACE_DATA *p_wk, const TOWNMAP_DATA *cp_data, u32 data_idx, GFL_CLUNIT *p_clunit, u32 chr, u32 cel, u32 plt, EVENTWORK *p_ev, HEAPID heapID, BOOL is_debug );
static void PlaceData_Exit( PLACE_DATA *p_wk );
static void PlaceData_SetVisible( PLACE_DATA *p_wk, BOOL is_visible );
static BOOL PlaceData_IsHit( const PLACE_DATA *cp_wk, const GFL_POINT *cp_pos, const GFL_POINT *cp_wld_pos );
static BOOL PlaceData_IsPullHit( const PLACE_DATA *cp_wk, const GFL_POINT *cp_pos, const GFL_POINT *cp_wld_pos, u32 *p_distance );
static void	PlaceData_Scale( PLACE_DATA *p_wk, fx32 scale, const GFL_POINT *cp_center_next );
static void PlaceData_SetPos(PLACE_DATA *p_wk, const GFL_POINT *cp_wld );
static void PlaceData_SetAnmSeq(PLACE_DATA *p_wk, u16 seq );
static void PlaceData_Active( PLACE_DATA *p_wk );
static void PlaceData_NonActive( PLACE_DATA *p_wk );
//-------------------------------------
///	PLACEMARK
//=====================================
static void PLACEMARK_Init( PLACE_MARK *p_wk, const PLACE_DATA *cp_data, GFL_CLUNIT *p_clunit, u32 chr, u32 cel, u32 plt, HEAPID heapID );
static void PLACEMARK_Exit( PLACE_MARK *p_wk );
static void PLACEMARK_SetPos( PLACE_MARK *p_wk, const GFL_POINT *cp_wld );
static void PLACEMARK_SetVisible( PLACE_MARK *p_wk, BOOL is_visible );
//-------------------------------------
///	MAP
//=====================================
static void MAP_Init( MAP_WORK *p_wk, u8 map_frm, u8 road_frm, HEAPID heapID );
static void MAP_Exit( MAP_WORK *p_wk );
static void MAP_Main( MAP_WORK *p_wk );
static void MAP_Scale( MAP_WORK *p_wk, fx32 scale, const GFL_POINT *cp_center_start, const GFL_POINT *cp_center_end );
static BOOL MAP_IsScaleEnd( const MAP_WORK *cp_wk );
static BOOL MAP_IsScale( const MAP_WORK *cp_wk );
static void MAP_SetWldPos( MAP_WORK *p_wk, const GFL_POINT *cp_pos );
static void MAP_GetWldPos( const MAP_WORK *cp_wk, GFL_POINT *p_pos );
static void MAP_AddWldPos( MAP_WORK *p_wk, const GFL_POINT *cp_pos );
//-------------------------------------
///	INFO
//=====================================
static void INFO_Init( INFO_WORK *p_wk, u8 frm, GFL_FONT *p_font, const GFL_MSGDATA *cp_place_msg, const GFL_MSGDATA *cp_guide_msg, HEAPID heapID );
static void INFO_Exit( INFO_WORK *p_wk );
static void INFO_Main( INFO_WORK *p_wk );
static void INFO_Update( INFO_WORK *p_wk, const PLACE_DATA *cp_data );
static void INFO_Clear( INFO_WORK *p_wk );
//-------------------------------------
///	MSGWND
//=====================================
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, GFL_FONT *p_font, const GFL_MSGDATA *cp_msg, u8 x, u8 y, u8 w, u8 h, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static void MSGWND_Print( MSGWND_WORK* p_wk, u32 strID, u16 x, u16 y );
static void MSGWND_Clear( MSGWND_WORK* p_wk );
//-------------------------------------
///	PLACEWND
//=====================================
static void PLACEWND_Init( PLACEWND_WORK *p_wk, u8 bgframe, GFL_FONT *p_font, const GFL_MSGDATA *cp_msg, GFL_CLWK *p_clwk, HEAPID heapID );
static void PLACEWND_Exit( PLACEWND_WORK *p_wk );
static void PLACEWND_Main( PLACEWND_WORK *p_wk );
static void PLACEWND_Start( PLACEWND_WORK *p_wk, const PLACE_DATA *cp_data );
static void PLACEWND_SetVisible( PLACEWND_WORK *p_wk, BOOL is_visible );
//-------------------------------------
///	UI
//=====================================
static void UI_Init( UI_WORK *p_wk, HEAPID heapID );
static void UI_Exit( UI_WORK *p_wk );
static void UI_Main( UI_WORK *p_wk );
static BOOL UI_GetSlide( const UI_WORK *cp_wk, VecFx32 *p_vec, u32 *p_sync );
static void UI_ResetSlide( UI_WORK *p_wk );
//-------------------------------------
///	ETC
//=====================================
static void BMP_DrawLine( GFL_BMP_DATA *p_bmp, const GFL_POINT *cp_start, const GFL_POINT *cp_end, u8 plt_num );
static void BMP_DrawBoldLine( GFL_BMP_DATA *p_bmp, const GFL_POINT *cp_start, const GFL_POINT *cp_end, int w, u8 plt_num );
//=============================================================================
/**
 *						DEBUG_PRINT
 */
//=============================================================================
#ifdef DEBUG_PRINT_USE
//debug用
#include "system/net_err.h"	//VRAM退避用アドレスを貰うため
#include <wchar.h>					//wcslen
//-------------------------------------
///	保存方法
//=====================================
typedef enum
{
	DEBUGPRINT_SAVE_TYPE_NONE,
	DEBUGPRINT_SAVE_TYPE_NETERR,
	DEBUGPRINT_SAVE_TYPE_HEAP,

} DEBUGPRINT_SAVE_TYPE;

//-------------------------------------
///	デバッグプリント用画面
//=====================================
typedef struct
{
	GFL_BMP_DATA *p_bmp;
	GFL_FONT*			p_font;

	DEBUGPRINT_SAVE_TYPE	save;
	u8	frm;
	u8	dummy[3];

  u8  *p_char_temp_area;      ///<キャラクタVRAM退避先
  u16 *p_scrn_temp_area;      ///<スクリーンVRAM退避先
  u16 *p_pltt_temp_area;      ///<パレットVRAM退避先

 	u8  font_col_bkup[3];
	u8	dummy2;
  u8  prioryty_bkup;
  u8  scroll_x_bkup;
  u8  scroll_y_bkup;
	u8	dummy3;

	BOOL is_open;

	HEAPID heapID;
} DEBUG_PRINT_WORK;

static DEBUG_PRINT_WORK *sp_dp_wk;

//NETERRの方は0x4000までしか確保できません
//#define DEBUGPRINT_CHAR_TEMP_AREA (NETERR_PUSH_CHARVRAM_SIZE)
#define DEBUGPRINT_CHAR_TEMP_AREA (0x8000)
#define DEBUGPRINT_SCRN_TEMP_AREA (NETERR_PUSH_SCRNVRAM_SIZE)
#define DEBUGPRINT_PLTT_TEMP_AREA (NETERR_PUSH_PLTTVRAM_SIZE)
#define DEBUGPRINT_WIDTH  (32)
//#define DEBUGPRINT_HEIGHT (18)
#define DEBUGPRINT_HEIGHT (24)

static const u16 sc_debug_print_plt[0xF]	=
{	
	0xFFFF,0xFFFF,0x7fff,0x001F,0
};

static DEBUG_PRINT_WORK * DEBUGPRINT_Init( u8 frm, DEBUGPRINT_SAVE_TYPE	save, HEAPID heapID );
static void DEBUGPRINT_Exit( DEBUG_PRINT_WORK *p_wk );
static void DEBUGPRINT_Open( DEBUG_PRINT_WORK *p_wk );
static BOOL DEBUGPRINT_IsOpen( DEBUG_PRINT_WORK *p_wk );
static void DEBUGPRINT_Close( DEBUG_PRINT_WORK *p_wk );
static void DEBUGPRINT_Print( DEBUG_PRINT_WORK *p_wk, const u16 *cp_str, u16 x, u16 y );
static void DEBUGPRINT_PrintNumber( DEBUG_PRINT_WORK *p_wk, const u16 *cp_str, int number, u16 x, u16 y );
static void DEBUGPRINT_Clear( DEBUG_PRINT_WORK *p_wk );
static void DEBUGPRINT_ClearRange( DEBUG_PRINT_WORK *p_wk, s16 x, s16 y, u16 w, u16 h );
//拡張
static void DEBUGPRINT_Update( DEBUG_PRINT_WORK *p_print, TOWNMAP_WORK *p_wk );
#else
#define DEBUGPRINT_Init(...)				((void)0)
#define DEBUGPRINT_Exit(...)				((void)0)
#define DEBUGPRINT_Open(...)				((void)0)
#define DEBUGPRINT_IsOpen(...)			((void)0)
#define DEBUGPRINT_Close(...)				((void)0)
#define DEBUGPRINT_Print(...)				((void)0)
#define DEBUGPRINT_PrintNumber(...) ((void)0)
#define DEBUGPRINT_Clear(...)				((void)0)
#define DEBUGPRINT_ClearRange(...)	((void)0)
#define DEBUGPRINT_Update(...)			((void)0)
#endif //DEBUG_PRINT_USE

//=============================================================================
/**
 *						DEBUG_MENU
 */
//=============================================================================
#ifdef DEBUG_PRINT_USE
static void DEBUGPRINT_POS_Update( DEBUG_PRINT_WORK *p_print, TOWNMAP_WORK *p_wk );
#else
#define DEBUGPRINT_POS_Update(...)			((void)0)
#endif //DEBUG_PRINT_USE

//=============================================================================
/**
 *						DEBUG_MENU
 */
//=============================================================================
#ifdef DEBUG_MENU_USE
static void DEBUGMENU_UPDATE_Print( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DEBUGMENU_DRAW_Print( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DEBUGMENU_UPDATE_CheckPos( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DEBUGMENU_DRAW_CheckPos( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DEBUGMENU_UPDATE_CheckPosNum( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DEBUGMENU_DRAW_CheckPosNum( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DEBUGMENU_UPDATE_VisiblePlace( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DEBUGMENU_DRAW_VisiblePlace( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DEBUGMENU_UPDATE_ArriveFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
static void DEBUGMENU_DRAW_ArriveFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item );
#endif //DEBUG_MENU_USE

//=============================================================================
/**
 *						DATA
 */
//=============================================================================
//=============================================================================
/**
 *						GLOBAL
 */
//=============================================================================
//-------------------------------------
///	PROCデータ外部公開
//=====================================
const GFL_PROC_DATA	TownMap_ProcData	=
{
	TOWNMAP_PROC_Init,
	TOWNMAP_PROC_Main,
	TOWNMAP_PROC_Exit,
};
//=============================================================================
/**
 *						PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	PROC初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param_adrs					引数
 *	@param	*p_wk_adrs						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TOWNMAP_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
  BOOL is_debug;

	TOWNMAP_WORK	*p_wk;
	TOWNMAP_PARAM	*p_param	= p_param_adrs;
	u16	data_len;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TOWNMAP, 0x60000 );

	//ワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(TOWNMAP_WORK), HEAPID_TOWNMAP );
	GFL_STD_MemClear( p_wk, sizeof(TOWNMAP_WORK) );
	p_wk->p_param	= p_param;

  is_debug  = p_wk->p_param->mode == TOWNMAP_MODE_DEBUGSKY;

	//共通システム作成---------------------
	//フォント
	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
    NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_TOWNMAP );

	//地名メッセージデータ
	p_wk->p_place_msg = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_place_name_dat, HEAPID_TOWNMAP );
	//ガイドメッセージデータ
	p_wk->p_guide_msg = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_townmap_dat, HEAPID_TOWNMAP );

	//モジュール作成----------------------
	//グラフィック作成
	p_wk->p_grh	= TOWNMAP_GRAPHIC_Init( HEAPID_TOWNMAP );

	//マップデータ作成
	p_wk->p_data	= TOWNMAP_DATA_Alloc( HEAPID_TOWNMAP );

	//シーケンス作成
	SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_FadeOut );

	//アプリケーションバー作成
	{	
		GAMEDATA	*p_gdata;

		p_gdata	= GAMESYSTEM_GetGameData( p_wk->p_param->p_gamesys );
		if( p_wk->p_param->mode == TOWNMAP_MODE_MAP )
		{	
		}
		else
		{	

			//空を飛ぶモードならばメッセージ面追加
			MSGWND_Init( &p_wk->msgwnd,
					TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_FONT_M),
					p_wk->p_font,
					p_wk->p_guide_msg,
					MSGWND_BMPWIN_SKY_X,
					MSGWND_BMPWIN_SKY_Y,
					MSGWND_BMPWIN_SKY_W,
					MSGWND_BMPWIN_SKY_H,
					HEAPID_TOWNMAP
					);

			GFL_FONTSYS_SetColor( 0xf, 1, 0 );
			MSGWND_Print( &p_wk->msgwnd, 0, MSGWND_STR_SKY_X, MSGWND_STR_SKY_Y );
			GFL_FONTSYS_SetDefaultColor();
		}

		APPBAR_Init( &p_wk->appbar,p_wk->p_param->mode,
				TOWNMAP_GRAPHIC_GetUnit( p_wk->p_grh, TOWNMAP_OBJ_CLUNIT_DEFAULT ),
				TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_BAR_M),
				TOWNMAP_BG_PAL_M_15, TOWNMAP_OBJ_PAL_M_08, p_gdata,
				HEAPID_TOWNMAP );
	}

	//地名ウィンドウ作成
	PLACEWND_Init( &p_wk->placewnd, 
			TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_FONT_M),
			p_wk->p_font, p_wk->p_place_msg, 
			TOWNMAP_GRAPHIC_GetClwk( p_wk->p_grh, TOWNMAP_OBJ_CLWK_WINDOW ),
			HEAPID_TOWNMAP );
	//FONT面をずらす
	GFL_BG_SetScroll( TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_FONT_M),
					GFL_BG_SCROLL_Y_SET, -4 );

	//場所作成
	{	
		GAMEDATA	*p_gdata;
		EVENTWORK	*p_ev;
#ifdef DEBUG_GAMESYS_NONE
		if( p_wk->p_param->p_gamesys == NULL )
		{	
			p_ev	= NULL;
		}
		else
		{	
			p_gdata	= GAMESYSTEM_GetGameData( p_wk->p_param->p_gamesys );
			p_ev		= GAMEDATA_GetEventWork( p_gdata );
		}
#else
		p_gdata	= GAMESYSTEM_GetGameData( p_wk->p_param->p_gamesys );
		p_ev		= GAMEDATA_GetEventWork( p_gdata );
#endif //DEBUG_GAMESYS_NONE
		PLACE_Init( &p_wk->place, p_param->zoneID, p_param->escapeID, p_wk->p_data, p_wk->p_grh,
				p_ev, HEAPID_TOWNMAP, is_debug );
	}

	//カーソル作成
	CURSOR_Init( &p_wk->cursor, TOWNMAP_GRAPHIC_GetClwk( p_wk->p_grh, TOWNMAP_OBJ_CLWK_CURSOR ),
			TOWNMAP_GRAPHIC_GetClwk( p_wk->p_grh, TOWNMAP_OBJ_CLWK_RING_CUR ), HEAPID_TOWNMAP );
	{	
		const PLACE_DATA *cp_data	= NULL;
		GFL_POINT pos;
		cp_data	= PLACE_GetDataByZoneID( &p_wk->place, p_param->zoneID, p_param->escapeID );
		if( cp_data )
		{	
			pos.x	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_CURSOR_X );
			pos.y	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_CURSOR_Y );
		}
		else
		{	
			pos.x	= 128;
			pos.y	= 96;
		}
		CURSOR_SetPos( &p_wk->cursor, &pos );
	}

	//地図作成
	MAP_Init( &p_wk->map,
			TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_MAP_M ),
			TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_ROAD_M ),
			HEAPID_TOWNMAP );

	//上画面情報作成
	INFO_Init( &p_wk->info, TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_FONT_S ),
			p_wk->p_font, p_wk->p_place_msg, p_wk->p_guide_msg, HEAPID_TOWNMAP );

	//タッチ
	UI_Init( &p_wk->ui, HEAPID_TOWNMAP );

#ifdef DEBUG_MENU_USE

	//デバッグメニュー
	DEBUGWIN_InitProc( TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_DEBUG_S ),
			p_wk->p_font );
	DEBUGWIN_ChangeLetterColor( 0, 0, 31 );
	DEBUGWIN_ChangeSelectColor( 31, 0, 31 );
	DEBUGWIN_AddGroupToTop( 0 , "タウンマップ" , HEAPID_TOWNMAP );
#endif //DEBUG_MENU_USE

	//デバッグ表示
#ifdef DEBUG_PRINT_USE
	sp_dp_wk	= DEBUGPRINT_Init( TOWNMAP_GRAPHIC_GetFrame( p_wk->p_grh, TOWNMAP_BG_FRAME_FONT_M ), 
			DEBUGPRINT_SAVE_TYPE_HEAP, HEAPID_TOWNMAP );
	DEBUGWIN_AddItemToGroupEx( DEBUGMENU_UPDATE_Print, DEBUGMENU_DRAW_Print, p_wk, 0, HEAPID_TOWNMAP );
	DEBUGWIN_AddItemToGroupEx( DEBUGMENU_UPDATE_ArriveFlag, DEBUGMENU_DRAW_ArriveFlag, p_wk, 0, HEAPID_TOWNMAP );

#ifdef DEBUG_POS_CHECK
	DEBUGWIN_AddItemToGroupEx( DEBUGMENU_UPDATE_CheckPos, DEBUGMENU_DRAW_CheckPos, p_wk, 0, HEAPID_TOWNMAP );
	DEBUGWIN_AddItemToGroupEx( DEBUGMENU_UPDATE_CheckPosNum, DEBUGMENU_DRAW_CheckPosNum, p_wk, 0, HEAPID_TOWNMAP );
#endif //DEBUG_POS_CHECK

#endif //DEBUG_PRINT_USE

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	PROC破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param_adrs					引数
 *	@param	*p_wk_adrs						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TOWNMAP_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

	//モジュール破棄-------------

	//デバッグ破棄
#ifdef DEBUG_PRINT_USE
	DEBUGPRINT_Exit(sp_dp_wk);
#endif //DEBUG_PRINT_USE
#ifdef DEBUG_MENU_USE
	DEBUGWIN_RemoveGroup( 0 );
	DEBUGWIN_ExitProc();
#endif //DEBUG_MENU_USE

	//タッチ破棄
	UI_Exit( &p_wk->ui );

	//上画面情報破棄
	INFO_Exit( &p_wk->info );

	//地図破棄
	MAP_Exit( &p_wk->map );

	//場所破棄
	PLACE_Exit( &p_wk->place );

	//カーソル破棄
	CURSOR_Exit( &p_wk->cursor );

	//地名ウィンドウ破棄
	PLACEWND_Exit( &p_wk->placewnd );

	//空を飛ぶモードならばメッセージ面破棄
	if( p_wk->p_param->mode != TOWNMAP_MODE_MAP )
	{	
		MSGWND_Exit( &p_wk->msgwnd );
	}
	//アプリケーションバー破棄
	APPBAR_Exit( &p_wk->appbar );

	//シーケンス破棄
	SEQ_Exit( &p_wk->seq );	

	//マップデータ破棄
	TOWNMAP_DATA_Free( p_wk->p_data );

	//グラフィック破棄
	TOWNMAP_GRAPHIC_Exit( p_wk->p_grh );

	//共通システム破棄-------------
	//メッセージ破棄
	GFL_MSG_Delete( p_wk->p_guide_msg );
	GFL_MSG_Delete( p_wk->p_place_msg );

	//フォント破棄
	GFL_FONT_Delete( p_wk->p_font );

	//ワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap(HEAPID_TOWNMAP );

	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	PROCメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param_adrs					引数
 *	@param	*p_wk_adrs						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT TOWNMAP_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

	//シーケンス
	SEQ_Main( &p_wk->seq );

	//描画
	TOWNMAP_GRAPHIC_Draw( p_wk->p_grh );

#ifdef PM_DEBUG
#if 0
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
	{	
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
		{	
			s_debug_pos.y--;
			NAGI_Printf( "debug pos x%d y%d\n", s_debug_pos.x, s_debug_pos.y );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
		{	
			s_debug_pos.y++;
			NAGI_Printf( "debug pos x%d y%d\n", s_debug_pos.x, s_debug_pos.y );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
		{	
			s_debug_pos.x--;
			NAGI_Printf( "debug pos x%d y%d\n", s_debug_pos.x, s_debug_pos.y );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
		{	
			s_debug_pos.x++;
			NAGI_Printf( "debug pos x%d y%d\n", s_debug_pos.x, s_debug_pos.y );
		}
	}
#endif
#endif //PM_DEBUG

	//終了
	if( SEQ_IsEnd( &p_wk->seq ) )
	{	
		return GFL_PROC_RES_FINISH;
	}
	else
	{	
		return GFL_PROC_RES_CONTINUE;
	}
}
//=============================================================================
/**
 *						SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	初期化
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	*p_param_adrs		パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param_adrs, SEQ_FUNCTION seq_function )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );

	//初期化
	p_wk->p_param_adrs	= p_param_adrs;

	//セット
	SEQ_SetNext( p_wk, seq_function );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	破棄
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void SEQ_Exit( SEQ_WORK *p_wk )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	メイン処理
 *
 *	@param	SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Main( SEQ_WORK *p_wk )
{	
	if( !p_wk->is_end )
	{	
		p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_param_adrs );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了取得
 *
 *	@param	const SEQ_WORK *cp_wk		ワーク
 *
 *	@return	TRUEならば終了	FALSEならば処理中
 */	
//-----------------------------------------------------------------------------
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk )
{	
	return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	次のシーケンスを設定
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( SEQ_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}
//=============================================================================
/**
 *					SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	フェードOUT
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_param_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs )
{	
	enum
	{	
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_END,
	};

	TOWNMAP_WORK	*p_wk	= p_param_adrs;

	switch( *p_seq )
	{	
	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_END;
		}
		break;

	case SEQ_END:
		SEQ_SetNext( p_seqwk, SEQFUNC_Main );
		break;

	default:
		GF_ASSERT(0);
	}
	
}
//----------------------------------------------------------------------------
/**
 *	@brief	フェードIN
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_param_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs )
{
	enum
	{
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};	

	TOWNMAP_WORK	*p_wk	= p_param_adrs;

	switch( *p_seq )
	{	
	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= SEQ_FADEIN_WAIT;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_EXIT;
		}
		break;

	case SEQ_EXIT:
		SEQ_End( p_seqwk );
		break;

	default:
		GF_ASSERT(0);
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	ゲーム開始
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_param_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param_adrs )
{	
	TOWNMAP_WORK	*p_wk	= p_param_adrs;
	BOOL is_map_move	= FALSE;

	//カーソルの地図移動
	if( p_wk->is_scale )
	{	
		GFL_POINT	pos;
		GFL_POINT	add	= {0,0};
		int btn;
		CURSOR_GetPos( &p_wk->cursor, &pos );
		btn	= CURSOR_GetContButton( &p_wk->cursor );

		//カーソル操作
		if( pos.y < CURSOR_MAP_MOVE_RANGE_TOP )
		{	
			if( btn & PAD_KEY_UP )
			{	
				add.y -= 4;
				is_map_move	= TRUE;
			}
		}
		if( CURSOR_MAP_MOVE_RANGE_BOTTOM_APPBAR >= pos.y && pos.y > CURSOR_MAP_MOVE_RANGE_BOTTOM )
		{	
			if( btn & PAD_KEY_DOWN )
			{	
				add.y += 4;
				is_map_move	= TRUE;
			}
		}
		if( pos.x < CURSOR_MAP_MOVE_RANGE_LEFT )
		{	
			if( btn & PAD_KEY_LEFT )
			{	
				add.x -= 4;
				is_map_move	= TRUE;
			}
		}
		if( pos.x > CURSOR_MAP_MOVE_RANGE_RIGHT )
		{	
			if( btn & PAD_KEY_RIGHT )
			{	
				add.x += 4;
				is_map_move	= TRUE;
			}
		}
		//スライドでの動作
		if( !MAP_IsScale( &p_wk->map ) )
		{	
			BOOL is_slide;
			VecFx32	v;
			u32 sync;

			is_slide	= UI_GetSlide( &p_wk->ui, &v, &sync );
			if( is_slide )
			{	
				add.x	= -FX_Mul(v.x, UI_DRAG_MOVE_RATE)>> FX32_SHIFT;
				add.y	= -FX_Mul(v.y, UI_DRAG_MOVE_RATE)>> FX32_SHIFT;

				UI_ResetSlide( &p_wk->ui );

				is_map_move	= TRUE;
			}
		}

		//移動処理
		if( is_map_move )
		{	
			GFL_POINT	pos;
			PLACE_GetWldPos( &p_wk->place, &pos );

			MAP_AddWldPos( &p_wk->map, &add );
			//OBJとBGは方向が逆
			add.x	*= -1;
			add.y *= -1;
			PLACE_AddWldPos( &p_wk->place, &add );
		}

	}

	//カーソルの場所当たり判定
	//マップ移動中は当たり判定しない
	if( !is_map_move )
	{	

		//※１

		//当たり判定
		{	
			const PLACE_DATA *cp_data;
			cp_data	= PLACE_Hit( &p_wk->place, &p_wk->cursor );
			if( cp_data != p_wk->cp_select && cp_data != NULL )
			{	
				GFL_POINT wld;

				if( !PMSND_CheckPlaySE() )
				{	
					PMSND_PlaySE( TOWNMAP_SE_SELECT );
				}
				INFO_Update( &p_wk->info, cp_data );
				PLACE_Active( &p_wk->place, cp_data );
				p_wk->cp_select	= cp_data;

				PLACE_GetWldPos(&p_wk->place, &wld );
				CURSOR_SetTarget( &p_wk->cursor, cp_data, &wld );

				//拡大中はでない
				if( !p_wk->is_scale )
				{	
					PLACEWND_Start( &p_wk->placewnd, cp_data );
				}
			}
			else if( cp_data == NULL && p_wk->cp_select )
			{	
				INFO_Clear( &p_wk->info );
				PLACEWND_SetVisible( &p_wk->placewnd, FALSE );
				PLACE_NonActive( &p_wk->place );
				CURSOR_SetTarget( &p_wk->cursor, NULL, NULL );
				p_wk->cp_select	= NULL;
			}
		}


		//選択処理
		//このifを※１に写すと1発タッチでとばなくなる
		if( p_wk->p_param->mode == TOWNMAP_MODE_SKY || p_wk->p_param->mode == TOWNMAP_MODE_DEBUGSKY )
		{
			const PLACE_DATA *cp_data;
			const PLACE_DATA *cp_now;
			cp_data	= CURSOR_GetTarget( &p_wk->cursor );
			cp_now	= PLACE_Hit( &p_wk->place, &p_wk->cursor );

			if( cp_data != NULL && cp_data == cp_now && CURSOR_GetTrg( &p_wk->cursor )
					&& PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_SKY_FLAG  ) )
			{	
				p_wk->p_param->select	= TOWNMAP_SELECT_SKY;
				p_wk->p_param->zoneID	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_ZONE_ID );
				p_wk->p_param->grid.x	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_WARP_X );
				p_wk->p_param->grid.y	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_WARP_Y );

				PMSND_PlaySE( TOWNMAP_SE_DECIDE );
				SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
			}
		}
	}
	else
	{	
		//マップ移動中は選択されていないようにする
		//画面端で選択したまま地図移動することがあるので
		if( p_wk->cp_select )
		{	
			INFO_Clear( &p_wk->info );
			PLACEWND_SetVisible( &p_wk->placewnd, FALSE );
			PLACE_NonActive( &p_wk->place );
			CURSOR_SetTarget( &p_wk->cursor, NULL, NULL );
			p_wk->cp_select	= NULL;
		}
	}

	//拡大中はでない
	if( p_wk->is_scale )
	{	
		PLACEWND_SetVisible( &p_wk->placewnd, FALSE );
	}

	//マップ判定中は引き困れない
	CURSOR_SetPullEnable( &p_wk->cursor, !is_map_move );

	///モジュールメイン処理
	APPBAR_Main( &p_wk->appbar );

	if( !MAP_IsScale( &p_wk->map ) )
	{	
		CURSOR_Main( &p_wk->cursor, &p_wk->place );
	}
	PLACE_Main( &p_wk->place );
	MAP_Main( &p_wk->map );
	PLACEWND_Main( &p_wk->placewnd );
	UI_Main( &p_wk->ui );

	//アプリケーションバー選択
	switch( APPBAR_GetTrg( &p_wk->appbar ) )
	{	
	case TOUCHBAR_ICON_CLOSE:
		p_wk->p_param->select	= TOWNMAP_SELECT_CLOSE;

		SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
		break;
	case TOUCHBAR_ICON_RETURN:
		p_wk->p_param->select	= TOWNMAP_SELECT_RETURN;

		SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
		break;
	case TOUCHBAR_ICON_SCALEUP:
		if(p_wk->is_scale == FALSE && !MAP_IsScale( &p_wk->map ))
		{	
			GFL_POINT next_pos;
			CURSOR_GetPos( &p_wk->cursor, &next_pos );

			//中心座標より一定距離離れないように
			//はなれすぎると、画面外が見えるので
			{	
				VecFx32 v;
				fx32	distance;
				v.x	= (next_pos.x	- sc_center_pos.x) << FX32_SHIFT;
				v.y	= (next_pos.y - sc_center_pos.y) <<FX32_SHIFT;
				v.z	= 0;
				distance	= VEC_Mag(&v);
				VEC_Normalize( &v, &v );

				if( distance >> FX32_SHIFT > MAP_SCALE_CENTER_DISTANCE_UP )
				{	
					next_pos.x	= sc_center_pos.x + ((v.x * MAP_SCALE_CENTER_DISTANCE_UP) >> FX32_SHIFT);
					next_pos.y	= sc_center_pos.y + ((v.y * MAP_SCALE_CENTER_DISTANCE_UP) >> FX32_SHIFT);
				}
			}

			PLACE_Scale( &p_wk->place, FX32_CONST(2), &sc_center_pos, &next_pos );
			MAP_Scale( &p_wk->map, FX32_CONST(2), &sc_center_pos, &next_pos );

			PLACE_SetVisible( &p_wk->place, FALSE );

			PMSND_PlaySE( TOWNMAP_SE_SCALEUP );

			p_wk->is_scale	= TRUE;
		}
		break;
	case TOUCHBAR_ICON_SCALEDOWN:
		if(p_wk->is_scale == TRUE && !MAP_IsScale( &p_wk->map ) )
		{	
			GFL_POINT src_pos;
			MAP_GetWldPos( &p_wk->map, &src_pos );
			//NAGI_Printf( "WLD X%d Y%d\n", src_pos.x, src_pos.y );
			src_pos.x	= src_pos.x + 256/2;
			src_pos.y	= src_pos.y + 192/2;

			//中心座標より一定距離離れないように
			//はなれすぎると、画面外が見えるので
			{	
				VecFx32 v;
				fx32	distance;
				v.x	= (src_pos.x	- sc_center_pos.x) << FX32_SHIFT;
				v.y	= (src_pos.y - sc_center_pos.y) <<FX32_SHIFT;
				v.z	= 0;
				distance	= VEC_Mag(&v);
				VEC_Normalize( &v, &v );

				if( distance >> FX32_SHIFT > MAP_SCALE_CENTER_DISTANCE_DOWN )
				{	
					src_pos.x	= sc_center_pos.x + ((v.x * MAP_SCALE_CENTER_DISTANCE_DOWN) >> FX32_SHIFT);
					src_pos.y	= sc_center_pos.y + ((v.y * MAP_SCALE_CENTER_DISTANCE_DOWN) >> FX32_SHIFT);
				}
			}

			PLACE_Scale( &p_wk->place, FX32_CONST(1), &src_pos, &sc_center_pos );
			MAP_Scale( &p_wk->map, FX32_CONST(1), &src_pos, &sc_center_pos );
			
			PLACE_SetVisible( &p_wk->place, FALSE );

			PMSND_PlaySE( TOWNMAP_SE_SCALEDOWN );

			p_wk->is_scale	= FALSE;
		}
		break;
	}

	//スケール終了後の場所表示処理
	if( MAP_IsScaleEnd( &p_wk->map ) )
	{	
		PLACE_SetVisible( &p_wk->place, TRUE );
	}
}
//=============================================================================
/**
 *	APPBAR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	初期化
 *
 *	@param	APPBAR_WORK *p_wk	ワーク
 *	@param	p_unit						APPBARのOBJ生成用p_unit
 *	@param	bar_frm						使用するBG面
 *	@param	bg_plt						使用するBGパレット番号
 *	@param	obj_plt						使用するOBJパレット番号
 *	@param	cp_gamedata				ゲームデータ（Yボタン登録確認用）
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void APPBAR_Init( APPBAR_WORK *p_wk, TOWNMAP_MODE mode, GFL_CLUNIT* p_unit, u8 bar_frm, u8 bg_plt, u8 obj_plt, GAMEDATA *p_gamedata, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(APPBAR_WORK) );
	p_wk->p_gamedata	= p_gamedata;

	//カスタムボタンのリソース
	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );

		p_wk->res[APPBAR_RES_SCALE_PLT]	= GFL_CLGRP_PLTT_RegisterEx( p_handle,
				NARC_townmap_gra_tmap_bg_d_NCLR, CLSYS_DRAW_MAIN, (0)*0x20,
				0, 4, heapID );	

		p_wk->res[APPBAR_RES_SCALE_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_townmap_gra_tmap_obj_d_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

		p_wk->res[APPBAR_RES_SCALE_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_townmap_gra_tmap_obj_d_NCER,
				NARC_townmap_gra_tmap_obj_d_NANR, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}

	//タッチバー初期化
	{	
		TOUCHBAR_SETUP	touchbar_setup;
		TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
		{	
			{	
				TOUCHBAR_ICON_RETURN,
				{	TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
			},
			{	
				TOUCHBAR_ICON_CLOSE,
				{	TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_Y },
			},
			{	
				TOUCHBAR_ICON_CUTSOM1,	//カスタムボタン１を拡大アイコンに,
				{	TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_Y },
			},
			{	
				TOUCHBAR_ICON_CUTSOM2,	//カスタムボタン１を縮小アイコンに,
				{	TOUCHBAR_ICON_X_04, TOUCHBAR_ICON_Y },
			}
		};
		//拡大アイコン
		touchbar_icon_tbl[2].cg_idx		= p_wk->res[APPBAR_RES_SCALE_CHR];				//キャラリソース
		touchbar_icon_tbl[2].plt_idx	= p_wk->res[APPBAR_RES_SCALE_PLT];				//パレットリソース
		touchbar_icon_tbl[2].cell_idx	=	p_wk->res[APPBAR_RES_SCALE_CEL];				//セルリソース
		touchbar_icon_tbl[2].active_anmseq	=	7;						//アクティブのときのアニメ
		touchbar_icon_tbl[2].noactive_anmseq	=		11;				//ノンアクティブのときのアニメ
		touchbar_icon_tbl[2].push_anmseq	=		9;						//押したときのアニメ（STOPになっていること）
		touchbar_icon_tbl[2].key	=		0;		//キーで押したときに動作させたいならば、ボタン番号
		touchbar_icon_tbl[2].se		=		0;		//押したときにSEならしたいならば、SEの番号	

		//縮小アイコン
		touchbar_icon_tbl[3].cg_idx		=  p_wk->res[APPBAR_RES_SCALE_CHR];				//キャラリソース
		touchbar_icon_tbl[3].plt_idx	= p_wk->res[APPBAR_RES_SCALE_PLT];				//パレットリソース
		touchbar_icon_tbl[3].cell_idx	=	p_wk->res[APPBAR_RES_SCALE_CEL];				//セルリソース
		touchbar_icon_tbl[3].active_anmseq	=	8;						//アクティブのときのアニメ
		touchbar_icon_tbl[3].noactive_anmseq=	12;						//ノンアクティブのときのアニメ
		touchbar_icon_tbl[3].push_anmseq		=	10;						//押したときのアニメ（STOPになっていること）
		touchbar_icon_tbl[3].key	=	0;		//キーで押したときに動作させたいならば、ボタン番号
		touchbar_icon_tbl[3].se		=	0;			//押したときにSEならしたいならば、SEの番号	

		//設定構造体
		//さきほどの窓情報＋リソース情報をいれる
		GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

		touchbar_setup.p_item		= touchbar_icon_tbl;				//上の窓情報
		touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
		touchbar_setup.p_unit		= p_unit;							//OBJ読み込みのためのCLUNIT
		touchbar_setup.bar_frm	= bar_frm;						//BG読み込みのためのBG面
		touchbar_setup.bg_plt		= bg_plt;			//BGﾊﾟﾚｯﾄ
		touchbar_setup.obj_plt	= obj_plt + 4;		//OBJﾊﾟﾚｯﾄ
		touchbar_setup.mapping	= APP_COMMON_MAPPING_128K;	//マッピングモード

		p_wk->p_touchbar	= TOUCHBAR_Init( &touchbar_setup, heapID );
	}



	APPBAR_SetActive( p_wk, TOUCHBAR_ICON_SCALEUP, TRUE );
	APPBAR_SetActive( p_wk, TOUCHBAR_ICON_SCALEDOWN, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	破棄
 *
 *	@param	APPBAR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void APPBAR_Exit( APPBAR_WORK *p_wk )
{	
	//タッチバー破棄
	TOUCHBAR_Exit( p_wk->p_touchbar );

	//OBJ
	{	
		//カスタムボタンリソース破棄
		GFL_CLGRP_CELLANIM_Release( p_wk->res[APPBAR_RES_SCALE_CEL] );
		GFL_CLGRP_CGR_Release( p_wk->res[APPBAR_RES_SCALE_CHR] );
		GFL_CLGRP_PLTT_Release( p_wk->res[APPBAR_RES_SCALE_PLT] );
	}

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(APPBAR_WORK) );

}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	メイン処理
 *
 *	@param	APPBAR_WORK *p_wk			ワーク
 *	@param	CURSOR_WORK	cp_cursor	カーソルでボタンを押すのを実現させるため
 *
 */
//-----------------------------------------------------------------------------
static void APPBAR_Main( APPBAR_WORK *p_wk )
{	
	TOUCHBAR_Main( p_wk->p_touchbar );


	if( TOUCHBAR_GetTrg( p_wk->p_touchbar ) == TOUCHBAR_ICON_SCALEUP )
	{	
		APPBAR_SetActive( p_wk, TOUCHBAR_ICON_SCALEUP, FALSE );
		APPBAR_SetActive( p_wk, TOUCHBAR_ICON_SCALEDOWN, TRUE );
	}
	else if( TOUCHBAR_GetTrg( p_wk->p_touchbar ) == TOUCHBAR_ICON_SCALEDOWN )
	{	
		APPBAR_SetActive( p_wk, TOUCHBAR_ICON_SCALEUP, TRUE );
		APPBAR_SetActive( p_wk, TOUCHBAR_ICON_SCALEDOWN, FALSE );
	}
	
}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	アイコンのアクティブ設定
 *
 *	@param	APPBAR_WORK *p_wk	ワーク
 *	@param	icon							設定するアイコン
 *	@param	is_active		TRUEでアクティブ	FALSEでノンアクティブ
 */
//-----------------------------------------------------------------------------
static void APPBAR_SetActive( APPBAR_WORK *p_wk, TOUCHBAR_ICON icon, BOOL is_active )
{	
	TOUCHBAR_SetActive(p_wk->p_touchbar, icon, is_active );
}
//----------------------------------------------------------------------------
/**
 *	@brief	APPBAR	選択されたものを取得
 *
 *	@param	const APPBAR_WORK *cp_wk ワーク
 *
 *	@return	APPBAR_SELECT列挙
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_ICON APPBAR_GetTrg( const APPBAR_WORK *cp_wk )
{	
	TOUCHBAR_ICON	icon;

	icon	= TOUCHBAR_GetTouch( cp_wk->p_touchbar );
	if( icon == TOUCHBAR_ICON_SCALEUP ||
			icon == TOUCHBAR_ICON_SCALEDOWN )
	{	
		return icon;
	}
	icon	= TOUCHBAR_GetTrg( cp_wk->p_touchbar );

	if( !(icon == TOUCHBAR_ICON_SCALEUP ||
				icon == TOUCHBAR_ICON_SCALEDOWN) )
	{	
		return icon;
	}

	return TOUCHBAR_SELECT_NONE;
}
//=============================================================================
/**
 *	CURSOR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	カーソル初期化
 *
 *	@param	CURSOR_WORK *p_wk	ワーク
 *	@param	*p_cursor					カーソルCLWK
 *	@param	*p_ring						リングCLWK
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void CURSOR_Init( CURSOR_WORK *p_wk, GFL_CLWK *p_cursor, GFL_CLWK *p_ring, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(CURSOR_WORK) );
	p_wk->p_clwk[CURSOR_CLWK_CURSOR]	= p_cursor;
	p_wk->p_clwk[CURSOR_CLWK_RING]		= p_ring;
	p_wk->pos.x	= CURSOR_POS_DEFAULT_X;
	p_wk->pos.y	= CURSOR_POS_DEFAULT_Y;

	{
		GFL_CLACT_WK_SetPos( p_wk->p_clwk[CURSOR_CLWK_CURSOR], &p_wk->pos, 0 );
		GFL_CLACT_WK_SetPos( p_wk->p_clwk[CURSOR_CLWK_RING], &p_wk->pos, 0 );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_CURSOR], TRUE );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_RING], FALSE );

		GFL_CLACT_WK_SetBgPri( p_wk->p_clwk[CURSOR_CLWK_CURSOR],	TOWNMAP_BG_PRIORITY_TOP_M );
		GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk[CURSOR_CLWK_CURSOR], OBJ_PRIORITY_CURSOR );
		GFL_CLACT_WK_SetBgPri( p_wk->p_clwk[CURSOR_CLWK_RING],	TOWNMAP_BG_PRIORITY_TOP_M );
		GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk[CURSOR_CLWK_RING], OBJ_PRIORITY_EFFECT );

		GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[CURSOR_CLWK_RING], TRUE );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	カーソル破棄
 *
 *	@param	CURSOR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void CURSOR_Exit( CURSOR_WORK *p_wk )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(CURSOR_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	カーソルメイン処理
 *
 *	@param	CURSOR_WORK *p_wk		ワーク
 *	@param	p_place	吸い込む処理のため情報をもらう
 *
 */
//-----------------------------------------------------------------------------
static void CURSOR_Main( CURSOR_WORK *p_wk, const PLACE_WORK *cp_place )
{	
	VecFx32	norm	= {0,0,0};
	VecFx32	pull	= {0,0,0};
	BOOL is_move	= FALSE;

	//情報の毎フレームクリア処理
	p_wk->is_trg	= FALSE;

	//カーソル移動方向
	if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
	{	
		norm.y	-= FX32_ONE;
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_CURSOR], TRUE );
		is_move	= TRUE;
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
	{	
		norm.y	+= FX32_ONE;
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_CURSOR], TRUE );
		is_move	= TRUE;
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
	{	
		norm.x	-= FX32_ONE;
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_CURSOR], TRUE );
		is_move	= TRUE;
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
	{	
		norm.x	+= FX32_ONE;
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_CURSOR], TRUE );
		is_move	= TRUE;
	}


	//吸い込まれる処理
	//吸い込み範囲内ならば吸い込まれる
	if( p_wk->is_pull )
	{	
		const PLACE_DATA* cp_placedata;
		cp_placedata	= PLACE_PullHit( cp_place, p_wk );
		if( cp_placedata != NULL )
		{	
			
			GFL_POINT	wld_pos;
			VecFx32	place_pos;
			VecFx32	now_pos;

			PLACE_GetWldPos( cp_place, &wld_pos );
			place_pos.x	= FX32_CONST(PLACEDATA_GetParam( cp_placedata, TOWNMAP_DATA_PARAM_CURSOR_X )+wld_pos.x);
			place_pos.y	= FX32_CONST(PLACEDATA_GetParam( cp_placedata, TOWNMAP_DATA_PARAM_CURSOR_Y )+wld_pos.y);
			place_pos.z	= 0;

			now_pos.x	= FX32_CONST(p_wk->pos.x);
			now_pos.y	= FX32_CONST(p_wk->pos.y);
			now_pos.z	= 0;

			VEC_Subtract( &place_pos, &now_pos, &pull );

			//もし一定以上の距離ならばそのまま
			if( VEC_Mag(&pull) > PLACE_PULL_STRONG )
			{	
				VEC_Normalize( &pull, &pull );
				GFL_CALC3D_VEC_MulScalar( &pull, PLACE_PULL_STRONG, &pull );
			}
			is_move	= TRUE;
		}
	}

	//移動計算
	if( is_move )
	{	
		VecFx32	add;
		VEC_Normalize( &norm, &norm );

		add.x	= (norm.x * CURSOR_ADD_SPEED)	+ pull.x;
		add.y	= (norm.y * CURSOR_ADD_SPEED)	+ pull.y;
		add.z	=	0;

		if( 0 < add.x && add.x < FX32_ONE )
		{	
			add.x	= FX32_ONE;
		}
		else if( -FX32_ONE < add.x && add.x < 0 )
		{	
			add.x = -FX32_ONE;
		}
		if( 0 < add.y && add.y < FX32_ONE )
		{	
			add.y	= FX32_ONE;
		}
		else if( -FX32_ONE < add.y && add.y < 0 )
		{	
			add.y = -FX32_ONE;
		}

		p_wk->pos.x	+= add.x >> FX32_SHIFT; 
		p_wk->pos.y	+= add.y >> FX32_SHIFT;
		p_wk->pos.x	= MATH_CLAMP( p_wk->pos.x, CURSOR_MOVE_LIMIT_LEFT, CURSOR_MOVE_LIMIT_RIGHT );
		p_wk->pos.y	= MATH_CLAMP( p_wk->pos.y, CURSOR_MOVE_LIMIT_TOP, CURSOR_MOVE_LIMIT_BOTTOM );
		GFL_CLACT_WK_SetPos( p_wk->p_clwk[CURSOR_CLWK_CURSOR], &p_wk->pos, 0 );
	}

	//決定
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
	{	
		p_wk->is_trg	= TRUE;
	}

	//タッチ処理
	{	
		u32 x, y;
		if( GFL_UI_TP_GetPointTrg( &x, &y ) )
		{	
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_CURSOR], FALSE );
			//タッチ範囲の中ならば
			if( ((u32)( x - CURSOR_MOVE_LIMIT_LEFT) <= (u32)(CURSOR_MOVE_LIMIT_RIGHT - CURSOR_MOVE_LIMIT_LEFT))
					&	((u32)( y - CURSOR_MOVE_LIMIT_TOP) <= (u32)(CURSOR_MOVE_LIMIT_BOTTOM - CURSOR_MOVE_LIMIT_TOP)))
			{	
				p_wk->is_trg	= TRUE;
				p_wk->pos.x	= x;
				p_wk->pos.y	= y;
			}
		}
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	カーソル位置を取得
 *
 *	@param	const CURSOR_WORK *cp_wk	ワーク
 *	@param	*p_pos										位置受け取り
 *
 */
//-----------------------------------------------------------------------------
static void CURSOR_GetPos( const CURSOR_WORK *cp_wk, GFL_POINT *p_pos )
{	
	p_pos->x	= cp_wk->pos.x;
	p_pos->y	= cp_wk->pos.y;
}
//----------------------------------------------------------------------------
/**
 *	@brief	カーソル位置を設定
 *
 *	@param	CURSOR_WORK *p_wk		ワーク
 *	@param	GFL_POINT *cp_pos		設定位置
 */
//-----------------------------------------------------------------------------
static void CURSOR_SetPos( CURSOR_WORK *p_wk, const GFL_POINT *cp_pos )
{	
	p_wk->pos.x	= cp_pos->x;
	p_wk->pos.y	= cp_pos->y;
	GFL_CLACT_WK_SetPos( p_wk->p_clwk[CURSOR_CLWK_CURSOR], &p_wk->pos, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief	カーソル決定を取得
 *
 *	@param	const CURSOR_WORK *cp_wk	ワーク
 *
 *	@return	TRUEで押した	FALSEで押してない
 */
//-----------------------------------------------------------------------------
static BOOL CURSOR_GetTrg( const CURSOR_WORK *cp_wk )
{	
	return cp_wk->is_trg;
}
//----------------------------------------------------------------------------
/**
 *	@brief	押し状態を取得
 *
 *	@param	const CURSOR_WORK *cp_wk	ワーク
 *	@param	*p_x		X受け取り
 *	@param	*p_y		Y受け取り
 *
 *	@return	TRUEで押した	FALSEで押していない
 */
//-----------------------------------------------------------------------------
static BOOL CURSOR_GetPointTrg( const CURSOR_WORK *cp_wk, u32 *p_x, u32 *p_y )
{	
	if( CURSOR_GetTrg( cp_wk ) )
	{	
		GFL_POINT	pos;
		CURSOR_GetPos( cp_wk, &pos );
		if( p_x )
		{	
			*p_x	= pos.x;
		}
		if( p_y )
		{	
			*p_y	= pos.y;
		}
		return TRUE;
	}

	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン押し状態取得
 *
 *	@param	const CURSOR_WORK *cp_wk	ワーク
 *
 *	@return	ボタン
 */
//-----------------------------------------------------------------------------
static int  CURSOR_GetContButton( const CURSOR_WORK *cp_wk )
{	
	return GFL_UI_KEY_GetCont();
}
//----------------------------------------------------------------------------
/**
 *	@brief	演出OBJを出す
 *
 *	@param	CURSOR_WORK *p_wk	ワーク
 *	@param	cp_place					場所	NULLならばターゲットなし
 *	@param	WLD								場所ワールド座標
 */
//-----------------------------------------------------------------------------
static void CURSOR_SetTarget( CURSOR_WORK *p_wk, const PLACE_DATA *cp_data, const GFL_POINT *cp_wld )
{	
	//ターゲットがNULLならば設定
	if( cp_data == NULL )
	{	
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_RING], FALSE );
		p_wk->cp_target	= NULL;
	}
	else if( p_wk->cp_target	!= cp_data )
	{	
		//ターゲットが変更されたら設定
		GFL_CLACTPOS pos;
		pos.x	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_CURSOR_X ) + cp_wld->x;
		pos.y	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_CURSOR_Y ) + cp_wld->y;
		GFL_CLACT_WK_SetPos( p_wk->p_clwk[CURSOR_CLWK_RING], &pos, 0 );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CURSOR_CLWK_RING], TRUE );
		GFL_CLACT_WK_ResetAnm( p_wk->p_clwk[CURSOR_CLWK_RING] );

		p_wk->cp_target	= cp_data;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ターゲットを取得
 *
 *	@param	const CURSOR_WORK *cp_wk	ワーク
 *
 *	@return	NULLならばターゲットがない	NULLではないならばターゲット
 */
//-----------------------------------------------------------------------------
static const PLACE_DATA * CURSOR_GetTarget( const CURSOR_WORK *cp_wk )
{	
	return cp_wk->cp_target;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カーソル	引きこまれ動作中許可
 *
 *	@param	const CURSOR_WORK *cp_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void CURSOR_SetPullEnable( CURSOR_WORK *p_wk, BOOL on_off )
{
	p_wk->is_pull	= on_off;
}
//=============================================================================
/**
 *					PLACE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	場所	初期化
 *
 *	@param	PLACE_WORK *p_wk			ワーク
 *	@param	TOWNMAP_DATA *cp_data	データ
 *	@param	p_grh									グラフィック
 *	@param	heapID								ヒープID
 */
//-----------------------------------------------------------------------------
static void PLACE_Init( PLACE_WORK *p_wk, u16 now_zone_ID, u16 esc_zone_ID, const TOWNMAP_DATA *cp_data, const TOWNMAP_GRAPHIC_SYS *cp_grh, EVENTWORK *p_ev, HEAPID heapID, BOOL is_debug )
{	
	//クリアー
	GFL_STD_MemClear( p_wk, sizeof(PLACE_WORK) );

	//ユニット作成
	//ユーザーレンダラーユニット
	{	
		static const GFL_REND_SURFACE_INIT sc_rend_surface_init =
		{	
			0,0,256,192,
			CLSYS_DRAW_MAIN
		};
		p_wk->p_clunit	= GFL_CLACT_UNIT_Create( 128, 0, heapID );
		p_wk->p_rend		= GFL_CLACT_USERREND_Create( &sc_rend_surface_init, 1, heapID );
		GFL_CLACT_UNIT_SetUserRend( p_wk->p_clunit, p_wk->p_rend );
	}

	//作成するCLWKの数を計算
	{	
		p_wk->data_num	= TOWNMAP_DATA_MAX;
	}
	
	//設定されている数分CLWKを出すため、バッファ作成
	p_wk->p_place	= GFL_HEAP_AllocMemory( heapID, sizeof(PLACE_DATA) * p_wk->data_num );
	GFL_STD_MemClear( p_wk->p_place, sizeof(PLACE_DATA) * p_wk->data_num );

	//場所作成
	{	
		int i;
		u32 chr, cel, plt;

		//リソース取得
		TOWNMAP_GRAPHIC_GetObjResource( cp_grh, &chr, &cel, &plt );
		//初期化
		for( i = 0; i < p_wk->data_num; i++ )
		{	
			PlaceData_Init( &p_wk->p_place[i], cp_data, i, p_wk->p_clunit, chr, cel, plt, p_ev, heapID, is_debug );
		}
	}

	//Playerの作成
	{	
		u32 chr, cel, plt;
		const PLACE_DATA *cp_placedata	= NULL;
		//ゾーンIDに対応する場所を取得
		cp_placedata	= PLACE_GetDataByZoneID( p_wk, now_zone_ID, esc_zone_ID );

		NAGI_Printf( "ゾーンID now%d esc%d\n", esc_zone_ID, now_zone_ID );

		//リソース取得
		TOWNMAP_GRAPHIC_GetObjResource( cp_grh, &chr, &cel, &plt );
		//作成
		PLACEMARK_Init( &p_wk->player, cp_placedata, p_wk->p_clunit, chr, cel, plt, heapID );
	}


	//移動制限
	p_wk->limit_top			= 0;
	p_wk->limit_left		= 0;
	p_wk->limit_right		= 0;
	p_wk->limit_bottom	= 0;


	//最初はノンアクティブ
	PLACE_NonActive( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief	場所	破棄
 *
 *	@param	PLACE_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void PLACE_Exit( PLACE_WORK *p_wk )
{	

	//プレイヤー破棄
	{
		PLACEMARK_Exit( &p_wk->player );
	}

	//CLWK破棄
	{	
		int i;
		for( i = 0; i < p_wk->data_num; i++  )
		{	
			PlaceData_Exit( &p_wk->p_place[i] );
		}
	}

	//バッファクリア
	GFL_HEAP_FreeMemory( p_wk->p_place );

	//レンダラ破棄
	{	
		GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );
		GFL_CLACT_USERREND_Delete( p_wk->p_rend );
	}

	//ウニット破棄
	GFL_CLACT_UNIT_Delete( p_wk->p_clunit );

	GFL_STD_MemClear( p_wk, sizeof(PLACE_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	メイン処理
 *
 *	@param	PLACE_WORK *p_wk		ワーク
 *	@param	CURSOR_WORK *cp_wk	カーソル情報
 *
 */
//-----------------------------------------------------------------------------
static void PLACE_Main( PLACE_WORK *p_wk )
{	
#ifdef PM_DEBUG
#if 0
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
	{	
		const PLACE_DATA *cp_data	= &p_wk->p_place[7];
		
		NAGI_Printf( "Pos sx%d yx%d ex%d ey%d w%d\n",
				PLACEDATA_GetParam( cp_data,TOWNMAP_DATA_PARAM_HIT_START_X ),
				PLACEDATA_GetParam( cp_data,TOWNMAP_DATA_PARAM_HIT_START_Y ),
				PLACEDATA_GetParam( cp_data,TOWNMAP_DATA_PARAM_HIT_END_X ),
				PLACEDATA_GetParam( cp_data,TOWNMAP_DATA_PARAM_HIT_END_Y ),
				PLACEDATA_GetParam( cp_data,TOWNMAP_DATA_PARAM_HIT_WIDTH )
				);

		NAGI_Printf( "WLD X%d Y%d\n", p_wk->pos.x, p_wk->pos.y );
	}
#endif
#endif //PM_DEBUG
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	スケールアップ開始
 *
 *	@param	PLACE_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void PLACE_Scale( PLACE_WORK *p_wk, fx32 scale, const GFL_POINT *cp_center_start, const GFL_POINT *cp_center_next )
{	

	//拡大すると動作範囲も変更
	{	
		if( scale == FX32_ONE )
		{	
			p_wk->limit_top			= 0;
			p_wk->limit_left		= 0;
			p_wk->limit_right		= 0;
			p_wk->limit_bottom	= 0;
		}
		else
		{	
			p_wk->limit_top			= MAP_SCALE_MOVE_LIMIT_TOP + 24;
			p_wk->limit_left		= MAP_SCALE_MOVE_LIMIT_LEFT;
			p_wk->limit_right		= MAP_SCALE_MOVE_LIMIT_RIGHT;
			p_wk->limit_bottom	=	MAP_SCALE_MOVE_LIMIT_BOTTOM + 24;
		}
	}

	//座標決定
	//場所拡大の中で位置設定をするので、
	//こちらを先に
	{	
		GFL_POINT	pos;

		if( scale == FX32_ONE )
		{
			pos.x	= 0;
			pos.y	= 0;
		}
		else
		{	
			//OBJは逆
			pos.x	=	-(cp_center_next->x-128);
			pos.y	=	-(cp_center_next->y-96);
		}

	/*	NAGI_Printf( "PLACE scale SX%d SY%d EX%d EY%d PX%d PY%d\n",
				cp_center_start->x, cp_center_start->y, cp_center_next->x, cp_center_next->y,
				pos.x, pos.y );
*/
		PLACE_SetWldPos( p_wk, &pos );
	}

	{	
		int i;
		//各場所、拡大
		//拡大は中心座標固定
		for( i = 0; i < p_wk->data_num; i++ )
		{	
			PlaceData_Scale( &p_wk->p_place[i], scale, &sc_center_pos );
			PlaceData_SetPos( &p_wk->p_place[i], &p_wk->pos );
		}
	}

	//プレイヤー位置を再設定
	{	
		PLACEMARK_SetPos( &p_wk->player, &p_wk->pos );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	表示、非表示設定
 *
 *	@param	PLACE_WORK *p_wk	ワーク
 *	@param	is_visible				TRUEなら表示FALSEならば非表示
 *
 */
//-----------------------------------------------------------------------------
static void PLACE_SetVisible( PLACE_WORK *p_wk, BOOL is_visible )
{	
	int i;
	for( i = 0; i < p_wk->data_num; i++ )
	{	
		PlaceData_SetVisible( &p_wk->p_place[i], is_visible );
	}

	PLACEMARK_SetVisible( &p_wk->player, is_visible );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	座標を設定
 *
 *	@param	PLACE_WORK *p_wk		ワーク
 *	@param	GFL_POINT *cp_pos		座標
 *
 */
//-----------------------------------------------------------------------------
static void PLACE_SetWldPos( PLACE_WORK *p_wk, const GFL_POINT *cp_pos )
{	
//	GFL_CLACTPOS	clpos;
	int i;

	p_wk->pos.x	= MATH_CLAMP( cp_pos->x, p_wk->limit_left, p_wk->limit_right );
	p_wk->pos.y	= MATH_CLAMP( cp_pos->y, p_wk->limit_top, p_wk->limit_bottom );

	for( i = 0; i < p_wk->data_num; i++ )
	{	
		PlaceData_SetPos( &p_wk->p_place[i], &p_wk->pos );
	}


	PLACEMARK_SetPos( &p_wk->player, &p_wk->pos );

//	clpos.x	= p_wk->pos.x;
//	clpos.y	= p_wk->pos.y;
	//NAGI_Printf( "place wld pos X%d Y%d\n", clpos.x, clpos.y );
//	GFL_CLACT_USERREND_SetSurfacePos( p_wk->p_rend, 0, &clpos );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	座標を取得
 *
 *	@param	const PLACE_WORK *cp_wk	ワーク
 *	@param	*p_pos									受け取り座標
 */
//-----------------------------------------------------------------------------
static void PLACE_GetWldPos( const PLACE_WORK *cp_wk, GFL_POINT *p_pos )
{	
	*p_pos	= cp_wk->pos;
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	座標を加算
 *
 *	@param	PLACE_WORK *p_wk	ワーク
 *	@param	GFL_POINT *cp_pos	加算値 
 */
//-----------------------------------------------------------------------------
static void PLACE_AddWldPos( PLACE_WORK *p_wk, const GFL_POINT *cp_pos )
{	
	p_wk->pos.x	+= cp_pos->x;
	p_wk->pos.y	+= cp_pos->y;
	PLACE_SetWldPos( p_wk, &p_wk->pos );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	カーソルが衝突したか
 *
 *	@param	const PLACE_WORK *cp_wk	ワーク
 *	@param	CURSOR_WORK *cp_wk			カーソル
 *
 *	@return	NULLならば当たっていない	それ以外ならば当たった場所のデータ
 */
//-----------------------------------------------------------------------------
static const PLACE_DATA* PLACE_Hit( const PLACE_WORK *cp_wk, const CURSOR_WORK *cp_cursor )
{	
	int i;
	
	GFL_POINT pos;

	CURSOR_GetPos( cp_cursor, &pos );

#if 0
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
	{	
		NAGI_Printf( "CUR x%d y%d\n", pos.x, pos.y );
	}
#endif

	//当たり判定
	for( i = 0; i < cp_wk->data_num; i++ )
	{
		if( PlaceData_IsHit( &cp_wk->p_place[i], &pos, &cp_wk->pos )  )
		{	
			return &cp_wk->p_place[i];
		}
	}

	return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所	カーソルが吸い込み範囲内か
 *
 *	@param	const PLACE_WORK *cp_wk	ワーク
 *	@param	CURSOR_WORK *cp_wk			カーソル
 *
 *	@return	NULLならば当たっていない	それ以外ならば当たった場所のデータ
 */
//-----------------------------------------------------------------------------
static const PLACE_DATA* PLACE_PullHit( const PLACE_WORK *cp_wk, const CURSOR_WORK *cp_cursor )
{	
	int i;
	GFL_POINT pos;
	u32 best_distance;
	u32 now_distance;
	s32 best_idx;

	CURSOR_GetPos( cp_cursor, &pos );

	//複数衝突していたならば一番近くのものを返す
	best_idx			=	-1;
	best_distance	= 0xFFFFFFFF;
	for( i = 0; i < cp_wk->data_num; i++ )
	{
		if( PlaceData_IsPullHit( &cp_wk->p_place[i], &pos, &cp_wk->pos, &now_distance )  )
		{
			if( best_distance > now_distance )
			{	
				best_idx	= i;
				best_distance	= now_distance;
			}
		}
	}

	if( best_idx == -1 )
	{	
		return NULL;
	}
	else
	{	
		return &cp_wk->p_place[ best_idx ];
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ	データ取得
 *
 *	@param	const PLACE_DATA *cp_wk	ワーク
 *	@param	param										情報の種類
 *
 *	@return	情報
 */
//-----------------------------------------------------------------------------
static s32 PLACEDATA_GetParam( const PLACE_DATA *cp_wk, TOWNMAP_DATA_PARAM param )
{	
	if( TOWNMAP_DATA_PARAM_POS_X <= param && param <= TOWNMAP_DATA_PARAM_HIT_WIDTH )
	{	
		//拡大縮小等で変更するデータ
		return cp_wk->param[ param - TOWNMAP_DATA_PARAM_POS_X ];
	}
	else if( param == TOWNMAP_DATA_PARAM_SKY_FLAG )
	{
		return cp_wk->is_sky;
	}
	else
	{	
		//オリジナルのまま変更がないデータ
		return TOWNMAP_DATA_GetParam( cp_wk->cp_data, cp_wk->data_idx, param );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ	場所をアクティブに
 *
 *	@param	PLACE_WORK *p_wk			ワーク
 *	@param	PLACE_DATA *cp_data		情報
 *
 */
//-----------------------------------------------------------------------------
static void PLACE_Active( PLACE_WORK *p_wk, const PLACE_DATA *cp_data )
{	
	int i;
	PLACE_DATA *p_data;

	for( i = 0; i < p_wk->data_num; i++ )
	{	
		p_data	= &p_wk->p_place[ i ];
		if( p_data == cp_data )
		{	
			PlaceData_Active( p_data );
		}
		else
		{	
			PlaceData_NonActive( p_data );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ	ノンアクティブ状態にする
 *
 *	@param	PLACE_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void PLACE_NonActive( PLACE_WORK *p_wk )
{	
	int i;
	PLACE_DATA *p_data;

	for( i = 0; i < p_wk->data_num; i++ )
	{	
		p_data	= &p_wk->p_place[ i ];

		PlaceData_NonActive( p_data );
	}	
}
//----------------------------------------------------------------------------
/**
 *	@brief	ゾーンIDからPLACE_DATAを取得
 *
 *	@param	const PLACE_DATA *cp_data	ワーク
 *	@param	zoneID										ゾーンID
 *
 *	@return	PLACE_DATA	
 */
//-----------------------------------------------------------------------------
static const PLACE_DATA *PLACE_GetDataByZoneID( const PLACE_WORK *cp_wk, u16 zoneID, u16 escapeID )
{	
	const PLACE_DATA *cp_data	= NULL;
	int i;

	//ゾーンと完全対応する場所を取得
	for( i = 0; i < cp_wk->data_num; i++ )
	{	
		cp_data	= &cp_wk->p_place[i];
		if( zoneID == PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_ZONE_ID ) )
		{
			return cp_data;
		}
	}

	//現在地がフィールドではないならば、エスケープID検索
	if( !ZONEDATA_IsFieldMatrixID( zoneID ) )
	{	
		for( i = 0; i < cp_wk->data_num; i++ )
		{	
			cp_data	= &cp_wk->p_place[i];
			if( escapeID == PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_ESCAPE_ZONE_ID ) )
			{
				return cp_data;
			}
		}
	}

	return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ作成
 *
 *	@param	PLACE_DATA *p_wk	ワーク
 *	@param	cp_data						データ
 *	@param	data_idx					データ番号
 *	@param	*p_clunit					UNIT
 *	@param	chr								チャラ
 *	@param	cel								セル
 *	@param	plt								パレット
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void PlaceData_Init( PLACE_DATA *p_wk, const TOWNMAP_DATA *cp_data, u32 data_idx, GFL_CLUNIT *p_clunit, u32 chr, u32 cel, u32 plt, EVENTWORK *p_ev, HEAPID heapID, BOOL is_debug )
{	
	GF_ASSERT( data_idx < TOWNMAP_DATA_MAX );

	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(PLACE_DATA) );
	p_wk->cp_data			= cp_data;
	p_wk->data_idx		= data_idx;
	p_wk->scale				= FX32_ONE;

	//到着フラグ取得
	{	
		u16 arrive_flag	= TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_ARRIVE_FLAG );
		if( arrive_flag != TOWNMAP_DATA_ERROR )
		{	
#ifdef DEBUG_GAMESYS_NONE
			if( p_ev == NULL )
			{	
				p_wk->is_arrive	= TRUE;
			}
			else
			{	
				p_wk->is_arrive		= EVENTWORK_CheckEventFlag( p_ev, arrive_flag );
			}
#else
			p_wk->is_arrive		= EVENTWORK_CheckEventFlag( p_ev, arrive_flag );
#endif //DEBUG_GAMESYS_NONE
		}
		else
		{	
			p_wk->is_arrive		= FALSE;
		}

		if( is_debug )
		{	
			p_wk->is_arrive	= TRUE;
		}
	}	

	//変更するデータを取得
	{	
		int i;
		for( i = 0; i < PLACE_DATA_PARAM_MAX; i++ )
		{	
			p_wk->param[i]	= TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, 
					TOWNMAP_DATA_PARAM_POS_X+i );
		}

		//TOWNMAP_DATA_PARAM_SKY_FLAG
    if( is_debug )
    {
      p_wk->is_sky  = TRUE;
    }
    else
    { 
      p_wk->is_sky	= p_wk->is_arrive && TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_SKY_FLAG );
    }
	}

	//タイプ別設定
	switch( PLACEDATA_GetParam( p_wk,TOWNMAP_DATA_PARAM_PLACE_TYPE ) )
	{	
	case TOWNMAP_PLACETYPE_TOWN:
		p_wk->active_anm			= 2;
		p_wk->non_active_anm	= 1;
		break;
	case TOWNMAP_PLACETYPE_DUNGEON:
		p_wk->active_anm			= 3;
		p_wk->non_active_anm	= PLACEDATA_ANIME_SEQ_VANISH;
		break;
	case TOWNMAP_PLACETYPE_ROAD:
		p_wk->active_anm			= PLACEDATA_ANIME_SEQ_VANISH;
		p_wk->non_active_anm	= PLACEDATA_ANIME_SEQ_VANISH;
		break;
	default:
		GF_ASSERT(0);
	}

	//セル作成
	{
		GFL_CLWK_DATA cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		cldata.pos_x	= PLACEDATA_GetParam( p_wk,TOWNMAP_DATA_PARAM_POS_X );
		cldata.pos_y	= PLACEDATA_GetParam( p_wk,TOWNMAP_DATA_PARAM_POS_Y );
		p_wk->p_clwk	= GFL_CLACT_WK_Create( p_clunit, chr, plt, cel, &cldata, 0, heapID );

		GFL_CLACT_WK_SetAffineParam( p_wk->p_clwk, CLSYS_AFFINETYPE_DOUBLE );
		GFL_CLACT_WK_SetBgPri( p_wk->p_clwk, TOWNMAP_BG_PRIORITY_BAR_M+1 );
		GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk, OBJ_PRIORITY_MAPOBJ );
	GFL_CLACT_WK_SetAffineParam( p_wk->p_clwk, CLSYS_AFFINETYPE_DOUBLE );
	}

	//最初はノンアクティブ
	PlaceData_NonActive( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ破棄
 *
 *	@param	PLACE_DATA *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void PlaceData_Exit( PLACE_DATA *p_wk )
{	
	if( p_wk->p_clwk )
	{	
		GFL_CLACT_WK_Remove( p_wk->p_clwk );
	}
	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(PLACE_DATA) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データ表示設定
 *
 *	@param	PLACE_DATA *p_wk	ワーク
 *	@param	is_visible				TRUEで表示	FALSEで非ひょ時
 *
 */
//-----------------------------------------------------------------------------
static void PlaceData_SetVisible( PLACE_DATA *p_wk, BOOL is_visible )
{	
	if( p_wk->p_clwk )
	{	
		if( p_wk->now_anm != PLACEDATA_ANIME_SEQ_VANISH )
		{	
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, is_visible );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所データと当たったか
 *
 *	@param	const PLACE_DATA *cp_wk	わーく
 *	@param	GFL_POINT *cp_pos				座標
 *
 *	@return	TRUEならば衝突	FALSEならばしていない
 */
//-----------------------------------------------------------------------------
static BOOL PlaceData_IsHit( const PLACE_DATA *cp_wk, const GFL_POINT *cp_pos, const GFL_POINT *cp_wld_pos )
{	
	GFL_COLLISION3D_CIRCLE		circle;
	GFL_COLLISION3D_CYLINDER	cylinder;
	VecFx32	v1, v2;
	fx32 w;

	v1.x	= FX32_CONST( cp_pos->x );
	v1.y	= FX32_CONST( cp_pos->y );
	v1.z	= 0;
	GFL_COLLISION3D_CIRCLE_SetData( &circle, &v1, 0 );

	v1.x	= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_START_X ) + cp_wld_pos->x );
	v1.y	= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_START_Y ) + cp_wld_pos->y );
	v1.z	= 0;
	v2.x	= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_END_X ) + cp_wld_pos->x );
	v2.y	= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_END_Y ) + cp_wld_pos->y );
	v2.z	= 0;
	w			= FX32_CONST( PLACEDATA_GetParam( cp_wk,TOWNMAP_DATA_PARAM_HIT_WIDTH ) );
	GFL_COLLISION3D_CYLINDER_SetData( &cylinder, &v1, &v2, w );
	
	return GFL_COLLISION3D_CYLXCIR_Check( &cylinder, &circle, NULL );
}

//----------------------------------------------------------------------------
/**
 *	@brief	吸い込み範囲内かどうか
 *
 *	@param	const PLACE_DATA *cp_wk	ワーク
 *	@param	GFL_POINT *cp_pos				座標
 *	@param	距離を受け取り
 *
 *	@return	TRUEならば吸い込み範囲	FALSEならば範囲外
 */
//-----------------------------------------------------------------------------
static BOOL PlaceData_IsPullHit( const PLACE_DATA *cp_wk, const GFL_POINT *cp_pos, const GFL_POINT *cp_wld_pos, u32 *p_distance )
{	
	u32 cur_x, cur_y;
	u32	x, y;

	cur_x	= PLACEDATA_GetParam( cp_wk, TOWNMAP_DATA_PARAM_CURSOR_X ) + cp_wld_pos->x;
	cur_y	= PLACEDATA_GetParam( cp_wk, TOWNMAP_DATA_PARAM_CURSOR_Y ) + cp_wld_pos->y;

	x = (cur_x - cp_pos->x) * (cur_x - cp_pos->x);
	y = (cur_y - cp_pos->y) * (cur_y - cp_pos->y);

	if( x + y < ((PLACE_PULL_R * PLACE_PULL_R * MATH_IAbs(cp_wk->scale)) >> FX32_SHIFT) )
	{
		if( p_distance )
		{	
			*p_distance	= (x + y);
		}

		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	スケール処理をする
 *
 *	@param	PLACE_DATA *p_wk	ワーク
 *	@param	scale							スケール
 *	@param	GFL_POINT *cp_center	拡大縮小中心座標
 */
//-----------------------------------------------------------------------------
static void	PlaceData_Scale( PLACE_DATA *p_wk, fx32 scale, const GFL_POINT *cp_center_next )
{	
	//データを更新
#if 0
	{
		//座標は差分をスケール倍
		p_wk->param[PLACE_DATA_PARAM_POS_X]	= 
			(((TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_POS_X ) - 
			 cp_center_next->x )* scale) >> FX32_SHIFT) + cp_center_next->x;

		p_wk->param[PLACE_DATA_PARAM_POS_Y]	= 
			(((TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_POS_Y ) - 
				cp_center_next->y )* scale) >> FX32_SHIFT) + cp_center_next->y;

		p_wk->param[PLACE_DATA_PARAM_CURSOR_X]	= 
			(((TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_CURSOR_X ) - 
				cp_center_next->x )* scale) >> FX32_SHIFT) + cp_center_next->x;

		p_wk->param[PLACE_DATA_PARAM_CURSOR_Y]	= 
			(((TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_CURSOR_Y ) - 
				cp_center_next->y) * scale) >> FX32_SHIFT) + cp_center_next->y;

		p_wk->param[PLACE_DATA_PARAM_HIT_START_X]	= 
			(((TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_HIT_START_X ) - 
				cp_center_next->x) * scale) >> FX32_SHIFT) + cp_center_next->x;

		p_wk->param[PLACE_DATA_PARAM_HIT_START_Y]	= 
			(((TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_HIT_START_Y ) - 
				cp_center_next->y) * scale) >> FX32_SHIFT) + cp_center_next->y;

		p_wk->param[PLACE_DATA_PARAM_HIT_END_X]	= 
			(((TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_HIT_END_X ) - 
				cp_center_next->x) * scale) >> FX32_SHIFT) + cp_center_next->x;

		p_wk->param[PLACE_DATA_PARAM_HIT_END_Y]	= 
			(((TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_HIT_END_Y ) - 
				cp_center_next->y) * scale) >> FX32_SHIFT) + cp_center_next->y;

		//幅は単純にスケール倍
		p_wk->param[PLACE_DATA_PARAM_HIT_WIDTH]	= 
			(TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_HIT_WIDTH )
			 * scale) >> FX32_SHIFT;
	
		p_wk->scale	= scale;
	}
#else
	{	
		int i;
		int center;
		fx32 x;
		for( i = 0; i < PLACE_DATA_PARAM_HIT_WIDTH; i++ )
		{	
			if(i % 2 == 0)
			{	
				center	= cp_center_next->x;
			}
			else
			{	
				center	= cp_center_next->y;
			}
			x	= TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_POS_X+i );
			x -= center;
			x	*= scale;
			x >>= FX32_SHIFT;
			x	+= center;

			p_wk->param[i]	=	x; 
		}

		//幅は単純にスケール倍
		p_wk->param[PLACE_DATA_PARAM_HIT_WIDTH]	= 
			(TOWNMAP_DATA_GetParam( p_wk->cp_data, p_wk->data_idx, TOWNMAP_DATA_PARAM_HIT_WIDTH )
			 * scale) >> FX32_SHIFT;
	}
	p_wk->scale	= scale;
#endif

	//OBJを拡大
	if( p_wk->p_clwk )
	{	
		GFL_CLSCALE		clscale;
		clscale.x	= scale;
		clscale.y	= scale;

		GFL_CLACT_WK_SetScale( p_wk->p_clwk, &clscale );

	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	座標設定
 *
 *	@param	*p_wk	ワーク
 *	@param	GFL_POINT *cp_pos		座標
 */
//-----------------------------------------------------------------------------
static void PlaceData_SetPos(PLACE_DATA *p_wk, const GFL_POINT *cp_wld )
{	
	GFL_CLACTPOS	pos;
	pos.x		= PLACEDATA_GetParam( p_wk,TOWNMAP_DATA_PARAM_POS_X ) + cp_wld->x;
	pos.y		= PLACEDATA_GetParam( p_wk,TOWNMAP_DATA_PARAM_POS_Y ) + cp_wld->y;

	if( p_wk->p_clwk )
	{	
		GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, 0 );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメ設定	PLACEDATA__ANIME_SEQ_VANISHにも対応
 *	
 *	@param	*p_wk		ワーク
 *	@param	seq			アニメシーケンス
 */
//-----------------------------------------------------------------------------
static void PlaceData_SetAnmSeq(PLACE_DATA *p_wk, u16 seq )
{	
	p_wk->now_anm	= seq;
	if( p_wk->now_anm != PLACEDATA_ANIME_SEQ_VANISH )
	{	
		if( p_wk->p_clwk )
		{	
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );
			GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, p_wk->now_anm );
		}
	}
	else
	{	
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, FALSE );
	}
	
}
//----------------------------------------------------------------------------
/**
 *	@brief	アクティブ設定
 *
 *	@param	PLACE_DATA *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void PlaceData_Active( PLACE_DATA *p_wk )
{	
	if( p_wk->is_arrive )
	{	
		PlaceData_SetVisible( p_wk, TRUE );
		PlaceData_SetAnmSeq( p_wk,  p_wk->active_anm );
	}
	else
	{	
		PlaceData_SetVisible( p_wk, FALSE );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ノンアクティブ設定
 *
 *	@param	PLACE_DATA *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void PlaceData_NonActive( PLACE_DATA *p_wk )
{	
	if( p_wk->is_arrive )
	{	
		if( p_wk->p_clwk )
		{	
			PlaceData_SetVisible( p_wk, TRUE );
			PlaceData_SetAnmSeq( p_wk,  p_wk->non_active_anm );
		}
	}
	else
	{	
		PlaceData_SetVisible( p_wk, FALSE );
	}
}
//=============================================================================
/**
 *		PLACE_MARK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤーマーキング	初期化
 *
 *	@param	PLACE_MARK *p_wk		ワーク
 *	@param	PLACE_DATA *cp_data	データ
 *	@param	*p_clunit						CLWK作成用ユニット
 *	@param	chr									キャラ
 *	@param	cel									セル
 *	@param	plt									パレット
 *	@param	heapID							ヒープID
 */
//-----------------------------------------------------------------------------
static void PLACEMARK_Init( PLACE_MARK *p_wk, const PLACE_DATA *cp_data, GFL_CLUNIT *p_clunit, u32 chr, u32 cel, u32 plt, HEAPID heapID )
{	
	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(PLACE_MARK) );
	p_wk->cp_data	= cp_data;

	//リソース取得
	{	
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		if( cp_data )
		{	
			cldata.pos_x	= PLACEDATA_GetParam(cp_data,TOWNMAP_DATA_PARAM_POS_X );
			cldata.pos_y	= PLACEDATA_GetParam(cp_data,TOWNMAP_DATA_PARAM_POS_Y );
		}

		p_wk->p_clwk	= GFL_CLACT_WK_Create( p_clunit, chr, plt, cel, &cldata, 0, heapID );
		GFL_CLACT_WK_SetBgPri( p_wk->p_clwk, TOWNMAP_BG_PRIORITY_BAR_M+1 );
		GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk, OBJ_PRIORITY_MARK );
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, 6 );
		GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk, TRUE );

		if( cp_data == NULL )
		{	
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, FALSE );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	プレイやーマーキング	破棄
 *
 *	@param	PLACE_MARK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void PLACEMARK_Exit( PLACE_MARK *p_wk )
{	
	GFL_CLACT_WK_Remove( p_wk->p_clwk );
	
	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(PLACE_MARK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤーマーキング	場所セット
 *
 *	@param	PLACE_MARK *p_wk			ワーク
 *	@param	PLACE_DATA *cp_data		場所データ
 */
//-----------------------------------------------------------------------------
static void PLACEMARK_SetPos( PLACE_MARK *p_wk, const GFL_POINT *cp_wld )
{	
	GFL_CLACTPOS clpos;
	if( p_wk->cp_data )
	{	
		clpos.x	= PLACEDATA_GetParam(p_wk->cp_data,TOWNMAP_DATA_PARAM_POS_X ) + cp_wld->x;
		clpos.y	= PLACEDATA_GetParam(p_wk->cp_data,TOWNMAP_DATA_PARAM_POS_Y ) + cp_wld->y;
	}
	GFL_CLACT_WK_SetPos( p_wk->p_clwk, &clpos, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤーマーキング	表示設定
 *
 *	@param	PLACE_MARK *p_wk	ワーク
 *	@param	is_visible		TRUE表示　FALSで非表示
 */
//-----------------------------------------------------------------------------
static void PLACEMARK_SetVisible( PLACE_MARK *p_wk, BOOL is_visible )
{	
	if( p_wk->cp_data )
	{	
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, is_visible );
	}
}
//=============================================================================
/**
 *		MAP
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	地図	初期化
 *
 *	@param	MAP_WORK *p_wk	ワーク
 *	@param	u8 map_frm			地図用フレーム
 *	@param	u8 road_frm			
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MAP_Init( MAP_WORK *p_wk, u8 map_frm, u8 road_frm, HEAPID heapID )
{	
	//リリア
	GFL_STD_MemClear( p_wk, sizeof(MAP_WORK) );
	p_wk->map_frm		= map_frm;
	p_wk->road_frm	= road_frm;
	p_wk->now_scale	= FX32_ONE;

	p_wk->limit_top			= 0;
	p_wk->limit_left		= 0;
	p_wk->limit_right		= 0;
	p_wk->limit_bottom	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	破棄
 *
 *	@param	MAP_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MAP_Exit( MAP_WORK *p_wk )
{	
	GFL_STD_MemClear( p_wk, sizeof(MAP_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	メイン処理
 *
 *	@param	MAP_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void MAP_Main( MAP_WORK *p_wk )
{	
	p_wk->is_scale_end	= FALSE;

	//拡大処理
	if( p_wk->is_scale_req )
	{	
		fx32		add;
		VecFx32	add_pos;
		
		//拡大率計算
		add	= (p_wk->next_scale - p_wk->init_scale)/MAP_SCALE_SYNC;
		p_wk->now_scale	= p_wk->init_scale + add * p_wk->sync;

    OS_FPrintf( 3, "init %f next %f now %f \n", FX_FX32_TO_F32(p_wk->init_scale), FX_FX32_TO_F32(p_wk->next_scale), FX_FX32_TO_F32(p_wk->now_scale) );

		//中心座標計算
		//中心座標計算といいつつも、中心は常に画面中心で、
		//中心がずれて見えるように座標をずらす
		add_pos.x	= ((p_wk->next_pos.x - p_wk->init_pos.x)<<FX32_SHIFT);
		add_pos.y	= ((p_wk->next_pos.y - p_wk->init_pos.y)<<FX32_SHIFT);
		//NAGI_Printf( "add X%d Y%d\n", add_pos.x>> FX32_SHIFT, add_pos.y>> FX32_SHIFT );
		p_wk->pos.x	= p_wk->init_pos.x + ((add_pos.x * p_wk->sync/MAP_SCALE_SYNC) >> FX32_SHIFT);
		p_wk->pos.y	= p_wk->init_pos.y + ((add_pos.y * p_wk->sync/MAP_SCALE_SYNC) >> FX32_SHIFT);
		//NAGI_Printf( "now X%d Y%d\n", p_wk->pos.x, p_wk->pos.y );

#if 1
		//制限計算
		{	
			int add_top;
			int add_bottom;
			int add_left;
			int add_right;

			add_top			= MAP_SCALE_MOVE_LIMIT_TOP / MAP_SCALE_SYNC;
			add_bottom	= MAP_SCALE_MOVE_LIMIT_BOTTOM / MAP_SCALE_SYNC;
			add_left		= MAP_SCALE_MOVE_LIMIT_LEFT	/ MAP_SCALE_SYNC;
			add_right		= MAP_SCALE_MOVE_LIMIT_RIGHT / MAP_SCALE_SYNC;
			if( p_wk->next_scale == FX32_ONE )
			{	
				p_wk->limit_top		= MAP_SCALE_MOVE_LIMIT_TOP - (add_top * p_wk->sync) ;
				p_wk->limit_bottom= MAP_SCALE_MOVE_LIMIT_BOTTOM - (add_bottom * p_wk->sync);
				p_wk->limit_left	= MAP_SCALE_MOVE_LIMIT_LEFT - (add_left * p_wk->sync);
				p_wk->limit_right	= MAP_SCALE_MOVE_LIMIT_RIGHT - (add_right * p_wk->sync);
			}
			else
			{	
				p_wk->limit_top		= add_top * p_wk->sync;
				p_wk->limit_bottom= add_bottom * p_wk->sync;
				p_wk->limit_left	= add_left * p_wk->sync;
				p_wk->limit_right	= add_right * p_wk->sync;
			}
		}
#endif

		//終了条件
		if( p_wk->sync++ >= MAP_SCALE_SYNC )
		{	
			p_wk->sync	= 0;
			p_wk->is_scale_req	= FALSE;
			p_wk->now_scale		= p_wk->next_scale;
			p_wk->pos					= p_wk->next_pos;

			//道路復活
			GFL_BG_SetVisible( p_wk->road_frm, TRUE );

			//拡大すると動作範囲も変更
			{	


				if( p_wk->next_scale == FX32_ONE )
				{	
					p_wk->limit_top			= 0;
					p_wk->limit_left		= 0;
					p_wk->limit_right		= 0;
					p_wk->limit_bottom	= 0;
				}
				else
				{	
					p_wk->limit_top			= MAP_SCALE_MOVE_LIMIT_TOP;
					p_wk->limit_left		= MAP_SCALE_MOVE_LIMIT_LEFT;
					p_wk->limit_right		= MAP_SCALE_MOVE_LIMIT_RIGHT;
					p_wk->limit_bottom	=	MAP_SCALE_MOVE_LIMIT_BOTTOM;
				}
			}
			p_wk->is_scale_end	= TRUE;
		}
		
#if 0
    { 
      MtxFx22 mtx;
      MTX_Scale22( &mtx, p_wk->now_scale, p_wk->now_scale );
      GFL_BG_SetAffine( p_wk->map_frm, &mtx, sc_center_pos.x, sc_center_pos.y );
      GFL_BG_SetAffine( p_wk->road_frm, &mtx, sc_center_pos.x, sc_center_pos.y );
    }
#endif

		//反映
#if 1
		MAP_SetWldPos( p_wk, &p_wk->pos );
#else
		GFL_BG_SetScrollReq( p_wk->map_frm, GFL_BG_SCROLL_X_SET, p_wk->pos.x );
		GFL_BG_SetScrollReq( p_wk->map_frm, GFL_BG_SCROLL_Y_SET, p_wk->pos.y );
		GFL_BG_SetRotateCenterReq( p_wk->map_frm, GFL_BG_CENTER_X_SET, sc_center_pos.x );
		GFL_BG_SetRotateCenterReq( p_wk->map_frm, GFL_BG_CENTER_Y_SET, sc_center_pos.y );
		GFL_BG_SetScaleReq( p_wk->map_frm, GFL_BG_SCALE_X_SET, p_wk->now_scale );
		GFL_BG_SetScaleReq( p_wk->map_frm, GFL_BG_SCALE_Y_SET, p_wk->now_scale );

		GFL_BG_SetScrollReq( p_wk->road_frm, GFL_BG_SCROLL_X_SET, p_wk->pos.x );
		GFL_BG_SetScrollReq( p_wk->road_frm, GFL_BG_SCROLL_Y_SET, p_wk->pos.y );
		GFL_BG_SetRotateCenterReq( p_wk->road_frm, GFL_BG_CENTER_X_SET, sc_center_pos.x );
		GFL_BG_SetRotateCenterReq( p_wk->road_frm, GFL_BG_CENTER_Y_SET, sc_center_pos.y );
		GFL_BG_SetScaleReq( p_wk->road_frm, GFL_BG_SCALE_X_SET, p_wk->now_scale );
		GFL_BG_SetScaleReq( p_wk->road_frm, GFL_BG_SCALE_Y_SET, p_wk->now_scale );
#endif
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	拡大開始
 *
 *	@param	MAP_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MAP_Scale( MAP_WORK *p_wk, fx32 scale, const GFL_POINT *cp_center_start, const GFL_POINT *cp_center_end )
{	
	//拡大縮小リクエスト
	p_wk->is_scale_req	= TRUE;
	p_wk->next_scale		= scale;
	p_wk->init_scale		= p_wk->now_scale;

	//中心座標から、座標に直す
	p_wk->init_pos.x	= cp_center_start->x - 128;
	p_wk->init_pos.y	= cp_center_start->y - 96;
	p_wk->next_pos.x	= cp_center_end->x - 128;
	p_wk->next_pos.y	= cp_center_end->y - 96;
	NAGI_Printf( "Scale start sx%d sy%d ex%d ey%d\n", p_wk->init_pos.x, p_wk->init_pos.y, 
			p_wk->next_pos.x, p_wk->next_pos.y);

  OS_FPrintf( 3, "init %f next %f\n", FX_FX32_TO_F32(p_wk->init_scale), FX_FX32_TO_F32(p_wk->next_scale) );

	//道路は消す
	GFL_BG_SetVisible( p_wk->road_frm, FALSE );

}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	拡大縮小処理チェック
 *
 *	@param	MAP_WORK *p_wk	ワーク
 *
 *	@return	TRUE処理中	FALSE処理していない
 */
//-----------------------------------------------------------------------------
static BOOL MAP_IsScaleEnd( const MAP_WORK *cp_wk )
{	
	return cp_wk->is_scale_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	拡大縮小中
 *
 *	@param	const MAP_WORK *cp_wk		ワーク
 *
 *	@return	TRUE処理中	FALSE処理していない
 */
//-----------------------------------------------------------------------------
static BOOL MAP_IsScale( const MAP_WORK *cp_wk )
{	
	return cp_wk->is_scale_req;
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	座標設定
 *
 *	@param	MAP_WORK *p_wk		ワーク
 *	@param	GFL_POINT *cp_pos 座標
 *
 */
//-----------------------------------------------------------------------------
static void MAP_SetWldPos( MAP_WORK *p_wk, const GFL_POINT *cp_pos )
{	
	p_wk->pos.x	= MATH_CLAMP( cp_pos->x, p_wk->limit_left, p_wk->limit_right );
	p_wk->pos.y	= MATH_CLAMP( cp_pos->y, p_wk->limit_top, p_wk->limit_bottom );

	GFL_BG_SetScrollReq( p_wk->map_frm, GFL_BG_SCROLL_X_SET, p_wk->pos.x );
	GFL_BG_SetScrollReq( p_wk->map_frm, GFL_BG_SCROLL_Y_SET, p_wk->pos.y );
	GFL_BG_SetRotateCenterReq( p_wk->map_frm, GFL_BG_CENTER_X_SET, sc_center_pos.x );
	GFL_BG_SetRotateCenterReq( p_wk->map_frm, GFL_BG_CENTER_Y_SET, sc_center_pos.y );
	GFL_BG_SetScaleReq( p_wk->map_frm, GFL_BG_SCALE_X_SET, p_wk->now_scale );
	GFL_BG_SetScaleReq( p_wk->map_frm, GFL_BG_SCALE_Y_SET, p_wk->now_scale );

	GFL_BG_SetScrollReq( p_wk->road_frm, GFL_BG_SCROLL_X_SET, p_wk->pos.x );
	GFL_BG_SetScrollReq( p_wk->road_frm, GFL_BG_SCROLL_Y_SET, p_wk->pos.y );
	GFL_BG_SetRotateCenterReq( p_wk->road_frm, GFL_BG_CENTER_X_SET, sc_center_pos.x );
	GFL_BG_SetRotateCenterReq( p_wk->road_frm, GFL_BG_CENTER_Y_SET, sc_center_pos.y );
	GFL_BG_SetScaleReq( p_wk->road_frm, GFL_BG_SCALE_X_SET, p_wk->now_scale );
	GFL_BG_SetScaleReq( p_wk->road_frm, GFL_BG_SCALE_Y_SET, p_wk->now_scale );
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	座標取得
 *
 *	@param	const MAP_WORK *cp_wk	ワーク
 *	@param	*p_pos								座標受け取り
 *
 */
//-----------------------------------------------------------------------------
static void MAP_GetWldPos( const MAP_WORK *cp_wk, GFL_POINT *p_pos )
{	
	*p_pos	= cp_wk->pos;
}
//----------------------------------------------------------------------------
/**
 *	@brief	地図	座標加算
 *
 *	@param	MAP_WORK *p_wk			ワーク
 *	@param	GFL_POINT *cp_pos		座標加算
 */
//-----------------------------------------------------------------------------
static void MAP_AddWldPos( MAP_WORK *p_wk, const GFL_POINT *cp_pos )
{	
	p_wk->pos.x	+= cp_pos->x;
	p_wk->pos.y	+= cp_pos->y;
	MAP_SetWldPos( p_wk, &p_wk->pos );
}
//=============================================================================
/**
 *		INFO
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	上画面情報	初期化
 *
 *	@param	INFO_WORK *p_wk	ワーク
 *	@param	frm							文字作成用フレーム
 *	@param	*p_font					フォント
 *	@param	*p_place_msg		地名GMM
 *	@param	*p_guide_msg		ガイドGMM
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void INFO_Init( INFO_WORK *p_wk, u8 frm, GFL_FONT *p_font, const GFL_MSGDATA *cp_place_msg, const GFL_MSGDATA *cp_guide_msg, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(INFO_WORK) );
	p_wk->p_font			= p_font;
	p_wk->cp_place_msg	= cp_place_msg;
	p_wk->cp_guide_msg	= cp_guide_msg;

	//文字面作成
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( frm, INFO_BMPWIN_X, INFO_BMPWIN_Y, INFO_BMPWIN_W, INFO_BMPWIN_H,
			TOWNMAP_BG_PAL_S_14, GFL_BMP_CHRAREA_GET_F );

	//汎用文字バッファ
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( 255, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	上画面情報	破棄
 *
 *	@param	INFO_WORK *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void INFO_Exit( INFO_WORK *p_wk )
{
	//汎用文字バッファ破棄
	GFL_STR_DeleteBuffer( p_wk->p_strbuf );

	//文字面削除
	GFL_BMPWIN_Delete( p_wk->p_bmpwin );

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(INFO_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	上画面情報	メイン処理
 *
 *	@param	INFO_WORK *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void INFO_Main( INFO_WORK *p_wk )
{	

}
//----------------------------------------------------------------------------
/**
 *	@brief	上画面情報	情報アップデート
 *
 *	@param	INFO_WORK *p_wk			わーく
 *	@param	PLACE_DATA *cp_data	表示する場所の情報
 */
//-----------------------------------------------------------------------------
static void INFO_Update( INFO_WORK *p_wk, const PLACE_DATA *cp_data )
{	
	GFL_BMP_DATA	*p_bmp	= GFL_BMPWIN_GetBmp( p_wk->p_bmpwin );

	//クリア
	GFL_BMP_Clear( p_bmp, 0 );

	if( cp_data )
	{	
		u16 msgID;
		u16 zoneID;

		//地名
		zoneID	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_ZONE_ID );
		GF_ASSERT( zoneID != TOWNMAP_DATA_ERROR );
		GFL_MSG_GetString( p_wk->cp_place_msg, ZONEDATA_GetPlaceNameID( zoneID ), p_wk->p_strbuf );
#ifdef INFO_STR_PLACE_CENTERING
		{	
			s16 x	= GFL_BMPWIN_GetSizeX( p_wk->p_bmpwin )*4;
			x	-= PRINTSYS_GetStrWidth( p_wk->p_strbuf, p_wk->p_font, 0 )/2;
			PRINTSYS_Print( p_bmp, x, INFO_STR_PLACE_Y, p_wk->p_strbuf, p_wk->p_font );	
		}
#else
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE_X, INFO_STR_PLACE_Y, p_wk->p_strbuf, p_wk->p_font );	
#endif

		//GUIDE
		msgID	= PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_GUIDE_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_GUIDE_X, INFO_STR_GUIDE_Y, p_wk->p_strbuf, p_wk->p_font );
		}

		//PLACE1
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE1_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE1_X, INFO_STR_PLACE1_Y, p_wk->p_strbuf, p_wk->p_font );
		}

		//PLACE2
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE2_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE2_X, INFO_STR_PLACE2_Y, p_wk->p_strbuf, p_wk->p_font );
		}
		//PLACE3
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE3_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE3_X, INFO_STR_PLACE3_Y, p_wk->p_strbuf, p_wk->p_font );
		}
		//PLACE4
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE4_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE4_X, INFO_STR_PLACE4_Y, p_wk->p_strbuf, p_wk->p_font );
		}
		//PLACE5
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE5_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE5_X, INFO_STR_PLACE5_Y, p_wk->p_strbuf, p_wk->p_font );
		}
		//PLACE6
		msgID = PLACEDATA_GetParam( cp_data, TOWNMAP_DATA_PARAM_PLACE5_MSGID);
		if( msgID != TOWNMAP_DATA_ERROR )
		{	
			GFL_MSG_GetString( p_wk->cp_guide_msg, msgID, p_wk->p_strbuf );
			PRINTSYS_Print( p_bmp, INFO_STR_PLACE6_X, INFO_STR_PLACE6_Y, p_wk->p_strbuf, p_wk->p_font );
		}

	}
	GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *	@brief	上画面情報	情報クリア
 *
 *	@param	INFO_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void INFO_Clear( INFO_WORK *p_wk )
{	
	GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->p_bmpwin );
}
//=============================================================================
/**
 *		MSGWND
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ面	初期化
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	bgframe						BGフレーム
 *	@param	*p_font						フォント
 *	@param	*p_msg						メッセージ
 *	@param	x									座標X
 *	@param	y									座標Y
 *	@param	w									幅
 *	@param	h									高さ
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, GFL_FONT *p_font, const GFL_MSGDATA *cp_msg, u8 x, u8 y, u8 w, u8 h, HEAPID heapID )
{	
	//クリアー
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
	p_wk->p_font	= p_font;
	p_wk->cp_msg		= cp_msg;

	p_wk->p_bmpwin	= GFL_BMPWIN_Create( bgframe, x, y, w, h, TOWNMAP_BG_PAL_M_14, GFL_BMP_CHRAREA_GET_F );
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( 255, heapID );

}
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ面	破棄
 *
 *	@param	MSGWND_WORK* p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void MSGWND_Exit( MSGWND_WORK* p_wk )
{	
	GFL_STR_DeleteBuffer( p_wk->p_strbuf );
	GFL_BMPWIN_Delete( p_wk->p_bmpwin );
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ面	プリント
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	strID							文字列ID
 *	@param	x									座標X
 *	@param	y									座標Y
 */
//-----------------------------------------------------------------------------
static void MSGWND_Print( MSGWND_WORK* p_wk, u32 strID, u16 x, u16 y )
{	
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

	//文字列作成
	GFL_MSG_GetString( p_wk->cp_msg, strID, p_wk->p_strbuf );
	PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), x, y, p_wk->p_strbuf, p_wk->p_font );

	GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *	@brief	消去
 *
 *	@param	MSGWND_WORK* p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void MSGWND_Clear( MSGWND_WORK* p_wk )
{	
	GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->p_bmpwin );
}
//=============================================================================
/**
 *	PLACEWND
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	地名ウィンドウ	初期化
 *
 *	@param	PLACEWND_WORK *p_wk		ワーク
 *	@param	bgframe								フォント表示面
 *	@param	*p_font								フォント
 *	@param	GFL_MSGDATA *cp_msg		地名メッセージ
 *	@param	GFL_CLWK		clwk			OBJ
 *	@param	heapID								ヒープID
 */
//-----------------------------------------------------------------------------
static void PLACEWND_Init( PLACEWND_WORK *p_wk, u8 bgframe, GFL_FONT *p_font, const GFL_MSGDATA *cp_msg, GFL_CLWK *p_clwk, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(PLACEWND_WORK) );
	p_wk->p_clwk	= p_clwk;

	//フォント面設定
	MSGWND_Init( &p_wk->msgwnd, bgframe, p_font, cp_msg, PLACEWND_BMPWIN_X, PLACEWND_BMPWIN_Y,
			PLACEWND_BMPWIN_W, PLACEWND_BMPWIN_H, heapID );

	//セル設定
	{	
		GFL_CLACTPOS	pos;
		pos.x	= PLACEWND_WND_POS_X;
		pos.y	= PLACEWND_WND_POS_Y;
		GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, 0 );
		GFL_CLACT_WK_SetBgPri( p_wk->p_clwk, TOWNMAP_BG_PRIORITY_FONT_M+1 );
		GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk, OBJ_PRIORITY_FRONTEND );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, 0 );
		GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk, TRUE );
		GFL_CLACT_WK_StopAnm( p_wk->p_clwk ); 
	}
	
}
//----------------------------------------------------------------------------
/**
 *	@brief	地名ウィンドウ	破棄
 *
 *	@param	PLACEWND_WORK *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void PLACEWND_Exit( PLACEWND_WORK *p_wk )
{	
	//フォント面解放
	MSGWND_Exit( &p_wk->msgwnd );

	GFL_STD_MemClear( p_wk, sizeof(PLACEWND_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	地名ウィンドウ	メイン処理
 *
 *	@param	PLACEWND_WORK *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void PLACEWND_Main( PLACEWND_WORK *p_wk )
{	
	//アップデートチェック
	if( p_wk->is_start )
	{	
		p_wk->is_start	= FALSE;

		//OBJアニメ開始
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, 0 );
		GFL_CLACT_WK_StartAnm( p_wk->p_clwk ); 
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );

		//フォントは消す
		MSGWND_Clear( &p_wk->msgwnd );

		//メイン処理実行
		p_wk->is_update	= TRUE;
	}

	//メイン処理
	if( p_wk->is_update )
	{	
		//アニメ終了時にフォントも出す
		if( GFL_CLACT_WK_CheckAnmActive( p_wk->p_clwk ) == FALSE )
		{	
			u16 zoneID;
			//メッセージ版語取得
			zoneID	= PLACEDATA_GetParam( p_wk->cp_data, TOWNMAP_DATA_PARAM_ZONE_ID );
			GF_ASSERT( zoneID != TOWNMAP_DATA_ERROR );

			//文字描画
			MSGWND_Print( &p_wk->msgwnd, ZONEDATA_GetPlaceNameID( zoneID ), PLACEWND_STR_X, PLACEWND_STR_Y );
			//アニメストップ
			GFL_CLACT_WK_StopAnm( p_wk->p_clwk ); 

			//メイン処理終了
			p_wk->is_update	= FALSE;
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	地名ウィンドウ	出現開始
 *
 *	@param	PLACEWND_WORK *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void PLACEWND_Start( PLACEWND_WORK *p_wk, const PLACE_DATA *cp_data )
{	
	GF_ASSERT( cp_data );
	p_wk->is_start	= TRUE;
	p_wk->cp_data		= cp_data;
}
//----------------------------------------------------------------------------
/**
 *	@brief	地名ウィンドウ	表示設定
 *	
 *	@param	PLACEWND_WORK *p_wk	ワーク
 *	@param	is_visible					TRUEで表示	FALSEで非表示
 */
//-----------------------------------------------------------------------------
static void PLACEWND_SetVisible( PLACEWND_WORK *p_wk, BOOL is_visible )
{	
	GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, is_visible );
	if( is_visible == FALSE )
	{	
		//メイン処理を止める
		p_wk->is_start	= FALSE;
		p_wk->is_update	= FALSE;

		//メッセージ面クリアする
		MSGWND_Clear( &p_wk->msgwnd );
	}
}
//=============================================================================
/**
 *		UI
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	UI初期化
 *
 *	@param	UI_WORK *p_wk	ワーク
 *	@param	heapID				ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void UI_Init( UI_WORK *p_wk, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(UI_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	UI破棄
 *
 *	@param	UI_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void UI_Exit( UI_WORK *p_wk )
{	
	GFL_STD_MemClear( p_wk, sizeof(UI_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	UIメイン処理
 *
 *	@param	UI_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void UI_Main( UI_WORK *p_wk )
{	
	BOOL is_sync;
	BOOL is_distance;
	BOOL is_cont	= FALSE;
	fx32 mag;
	u32 x, y;
	p_wk->is_slide	= FALSE;

	//タッチの処理
	if( GFL_UI_TP_GetPointTrg( &x, &y ) )
	{	
		//タッチ範囲の中ならば
		if( ((u32)( x - CURSOR_MOVE_LIMIT_LEFT) <= (u32)(CURSOR_MOVE_LIMIT_RIGHT - CURSOR_MOVE_LIMIT_LEFT))
				&	((u32)( y - CURSOR_MOVE_LIMIT_TOP) <= (u32)(CURSOR_MOVE_LIMIT_BOTTOM - CURSOR_MOVE_LIMIT_TOP)))
		{	

			p_wk->start.x		= x << FX32_SHIFT;
			p_wk->start.y		= y << FX32_SHIFT;
			p_wk->start.z		= 0;
			p_wk->end.x		= x << FX32_SHIFT;
			p_wk->end.y		= y << FX32_SHIFT;
			p_wk->end.z		= 0;
		}
	}
	else if(GFL_UI_TP_GetPointCont( &x, &y ) )
	{	
		//タッチ範囲の中ならば
		if( ((u32)( x - CURSOR_MOVE_LIMIT_LEFT) <= (u32)(CURSOR_MOVE_LIMIT_RIGHT - CURSOR_MOVE_LIMIT_LEFT))
				&	((u32)( y - CURSOR_MOVE_LIMIT_TOP) <= (u32)(CURSOR_MOVE_LIMIT_BOTTOM - CURSOR_MOVE_LIMIT_TOP)))
		{
			if( p_wk->is_reset )
			{	
				p_wk->start		= p_wk->end;
				p_wk->is_reset	= FALSE;
			}

			p_wk->end.x		= x << FX32_SHIFT;
			p_wk->end.y		= y << FX32_SHIFT;
			p_wk->end.z		= 0;

			VEC_Subtract( &p_wk->end, &p_wk->start, &p_wk->v );

			p_wk->is_slide	= TRUE;
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	スライド処理
 *
 *	@param	const UI_WORK *cp_wk	ワーク
 *	@param	*p_vec								ベクトル
 *	@param	*p_sync								シンク
 *
 *	@return	TRUEスライドした　FALSEスライドしていない
 */
//-----------------------------------------------------------------------------
static BOOL UI_GetSlide( const UI_WORK *cp_wk, VecFx32 *p_vec, u32 *p_sync )
{	
	if( cp_wk->is_slide )
	{	
		if( p_vec )
		{	
			*p_vec	= cp_wk->v;
		}
		if( p_sync )
		{	
			*p_sync	= cp_wk->slide_sync;
		}
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	スライド開始位置をリセット
 *
 *	@param	UI_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void UI_ResetSlide( UI_WORK *p_wk )
{	
	p_wk->is_reset	= TRUE;
}
//=============================================================================
/**
 *		ETC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	線分を描く
 *
 *	@param	GFL_BMP_DATA *p_bmp	書き込むBMP
 *	@param	GFL_POINT *cp_start	開始
 *	@param	GFL_POINT *cp_end		終了
 *
 */
//-----------------------------------------------------------------------------
static void BMP_DrawLine( GFL_BMP_DATA *p_bmp, const GFL_POINT *cp_start, const GFL_POINT *cp_end, u8 plt_num )
{	
	VecFx32	start;
	VecFx32	end;
	VecFx32 norm;
	fx32 mag;

	start.x	= cp_start->x << FX32_SHIFT;
	start.y	= cp_start->y << FX32_SHIFT;
	start.z	= 0;

	end.x	= cp_end->x << FX32_SHIFT;
	end.y	= cp_end->y << FX32_SHIFT;
	end.z	= 0;

	VEC_Subtract( &end, &start, &norm );
	mag	= VEC_Mag( &norm );
	VEC_Normalize( &norm, &norm );

	{	
		int i;
		GFL_POINT pos;

		for( i = 0; i < (mag>>FX32_SHIFT); i++ )
		{	
			pos.x	= (start.x + (norm.x * i) ) >> FX32_SHIFT;
			pos.y	= (start.y + (norm.y * i) ) >> FX32_SHIFT;
			GFL_BMP_Fill( p_bmp, pos.x, pos.y, 1, 1, plt_num );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	太線を書く
 *
 *	@param	GFL_BMP_DATA *p_bmp	書き込むBMP
 *	@param	GFL_POINT *cp_start	開始座標
 *	@param	GFL_POINT *cp_end		終了座標
 *	@param	w										太さ
 *	@param	plt_num							書き込むパレット番号
 */
//-----------------------------------------------------------------------------
static void BMP_DrawBoldLine( GFL_BMP_DATA *p_bmp, const GFL_POINT *cp_start, const GFL_POINT *cp_end, int w, u8 plt_num )
{	
	VecFx32	start;
	VecFx32	end;
	VecFx32 norm;
	fx32 mag;

	VecFx32	v;	//normの垂直

	start.x	= cp_start->x << FX32_SHIFT;
	start.y	= cp_start->y << FX32_SHIFT;
	start.z	= 0;

	end.x	= cp_end->x << FX32_SHIFT;
	end.y	= cp_end->y << FX32_SHIFT;
	end.z	= 0;

	VEC_Subtract( &end, &start, &norm );
	mag	= VEC_Mag( &norm );
	VEC_Normalize( &norm, &norm );

	VEC_Set( &v, -norm.y, norm.x, 0 );

	{	
		int i,j;
		GFL_POINT pos;
		VecFx32	add;

		for( j = 0; j < w/2; j++ )
		{	
			add.x	= v.x * j;
			add.y	= v.y * j;
			add.z	= 0;

			for( i = 0; i < (mag>>FX32_SHIFT); i++ )
			{	
				pos.x	= (start.x + (norm.x * i) + add.x ) >> FX32_SHIFT;
				pos.y	= (start.y + (norm.y * i) + add.y ) >> FX32_SHIFT;
				GFL_BMP_Fill( p_bmp, pos.x, pos.y, 1, 1, plt_num );
			}

			for( i = 0; i < (mag>>FX32_SHIFT); i++ )
			{	
				pos.x	= (start.x + (norm.x * i) - add.x ) >> FX32_SHIFT;
				pos.y	= (start.y + (norm.y * i) - add.y ) >> FX32_SHIFT;
				GFL_BMP_Fill( p_bmp, pos.x, pos.y, 1, 1, plt_num );
			}
		}
	}
}
//=============================================================================
/**
 *			DEBUG
 */
//=============================================================================
#ifdef DEBUG_PRINT_USE
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域	初期化
 *
 *	@param	frm											フレーム
 *	@param	heapID									ヒープID
 *
 */
//-----------------------------------------------------------------------------
static DEBUG_PRINT_WORK * DEBUGPRINT_Init( u8 frm, DEBUGPRINT_SAVE_TYPE	save, HEAPID heapID )
{	
	DEBUG_PRINT_WORK *p_wk;

	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(DEBUG_PRINT_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(DEBUG_PRINT_WORK) );
	p_wk->heapID						= heapID;
	p_wk->save							= save;
	p_wk->frm								= frm;

	//デバッグプリント用フォント
	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
				NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );	

	//退避エリアをタイプによって取得
	switch( p_wk->save )
	{
	case DEBUGPRINT_SAVE_TYPE_NONE:
		//使わないので取得しない
		break;
	case DEBUGPRINT_SAVE_TYPE_NETERR:
		NetErr_GetTempArea( &p_wk->p_char_temp_area, &p_wk->p_scrn_temp_area, &p_wk->p_pltt_temp_area );
		break;
	case DEBUGPRINT_SAVE_TYPE_HEAP:
		p_wk->p_char_temp_area	= GFL_HEAP_AllocMemory( heapID, DEBUGPRINT_CHAR_TEMP_AREA );
		p_wk->p_scrn_temp_area	= GFL_HEAP_AllocMemory( heapID, DEBUGPRINT_SCRN_TEMP_AREA );
		p_wk->p_pltt_temp_area	= GFL_HEAP_AllocMemory( heapID, DEBUGPRINT_PLTT_TEMP_AREA );
		break;
	}

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域	破棄
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Exit( DEBUG_PRINT_WORK *p_wk )
{	
	//開いていたら解放
	if( DEBUGPRINT_IsOpen(p_wk) )
	{	
		DEBUGPRINT_Close(p_wk);
	}

	//退避エリアをタイプによって解放
	switch( p_wk->save )
	{
	case DEBUGPRINT_SAVE_TYPE_NONE:
		break;
	case DEBUGPRINT_SAVE_TYPE_NETERR:
		break;
	case DEBUGPRINT_SAVE_TYPE_HEAP:
		GFL_HEAP_FreeMemory( p_wk->p_pltt_temp_area );
		GFL_HEAP_FreeMemory( p_wk->p_scrn_temp_area );
		GFL_HEAP_FreeMemory( p_wk->p_char_temp_area );
		break;
	}


	GFL_FONT_Delete( p_wk->p_font );
	GFL_STD_MemClear( p_wk, sizeof(DEBUG_PRINT_WORK) );

	GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域オープン
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Open( DEBUG_PRINT_WORK *p_wk )
{	

	GF_ASSERT( p_wk );
	GF_ASSERT( p_wk->is_open == FALSE );

	if( p_wk->save != DEBUGPRINT_SAVE_TYPE_NONE )
	{	
		//VRAMのデータを退避
		GFL_STD_MemCopy16(GFL_DISPUT_GetCgxPtr(p_wk->frm), p_wk->p_char_temp_area, DEBUGPRINT_CHAR_TEMP_AREA);
		GFL_STD_MemCopy16(GFL_DISPUT_GetScrPtr(p_wk->frm), p_wk->p_scrn_temp_area, DEBUGPRINT_SCRN_TEMP_AREA);
		GFL_STD_MemCopy16(GFL_DISPUT_GetPltPtr(p_wk->frm), p_wk->p_pltt_temp_area, DEBUGPRINT_PLTT_TEMP_AREA);	
		//Fontカラーの退避
		GFL_FONTSYS_GetColor( &p_wk->font_col_bkup[0] ,
				&p_wk->font_col_bkup[1] ,
				&p_wk->font_col_bkup[2] );

		//もろもろ退避
		p_wk->prioryty_bkup = GFL_BG_GetPriority(p_wk->frm);
		p_wk->scroll_x_bkup = GFL_BG_GetScrollX(p_wk->frm);
		p_wk->scroll_y_bkup = GFL_BG_GetScrollY(p_wk->frm);
	}

	//上で退避させたものの設定
	GFL_BG_SetPriority( p_wk->frm , 0 );
	GFL_BG_SetScroll( p_wk->frm , GFL_BG_SCROLL_X_SET , 0 );
	GFL_BG_SetScroll( p_wk->frm , GFL_BG_SCROLL_Y_SET , 0 );

	//デバッグプリント用設定
	//スクリーンの作成
  {
    u8 x,y;
		u16 buf;
    for( y = 0;y<DEBUGPRINT_HEIGHT;y++ )
    {
      for( x=0;x<DEBUGPRINT_WIDTH;x++ )
      {
        buf = x+y*DEBUGPRINT_WIDTH;
        GFL_BG_WriteScreen( p_wk->frm, &buf, x,y,1,1 );
      }
      for( x=DEBUGPRINT_WIDTH;x<32;x++ )
      {
        buf = DEBUGPRINT_HEIGHT*DEBUGPRINT_WIDTH;
        GFL_BG_WriteScreen( p_wk->frm, &buf, x,y,1,1 );
      }
    }
		for( y = DEBUGPRINT_HEIGHT;y<24;y++ )
    {
	 		buf = DEBUGPRINT_HEIGHT*DEBUGPRINT_WIDTH;
 			GFL_BG_WriteScreen( p_wk->frm, &buf, x,y,1,1 );
			for( x=0;x<32;x++ )
      {
        buf = DEBUGPRINT_HEIGHT*DEBUGPRINT_WIDTH;
        GFL_BG_WriteScreen( p_wk->frm, &buf, x,y,1,1 );
      }
		}
    GFL_BG_LoadScreenReq( p_wk->frm );
  }
  
  //パレットの作成
  GFL_BG_LoadPalette( p_wk->frm, (void*)sc_debug_print_plt, sizeof(u16)*NELEMS(sc_debug_print_plt), 0 );

	//書き込むためのBMP作成
	p_wk->p_bmp	= GFL_BMP_CreateInVRAM( GFL_DISPUT_GetCgxPtr(p_wk->frm), DEBUGPRINT_WIDTH, DEBUGPRINT_HEIGHT, GFL_BMP_16_COLOR, p_wk->heapID );
	GFL_STD_MemClear16( GFL_DISPUT_GetCgxPtr(p_wk->frm) , DEBUGPRINT_CHAR_TEMP_AREA );

	p_wk->is_open	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域を開いているか
 *
 *	@return	TRUEならば開いている	FALSEならば閉じている
 */
//-----------------------------------------------------------------------------
static BOOL DEBUGPRINT_IsOpen( DEBUG_PRINT_WORK *p_wk )
{	
	if( p_wk )
	{	
		return p_wk->is_open;
	}
	else
	{	
		return FALSE;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域	終了
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Close( DEBUG_PRINT_WORK *p_wk )
{	

	GF_ASSERT( p_wk );
	GF_ASSERT( p_wk->is_open == TRUE );


	p_wk->is_open	= FALSE;

	GFL_BMP_Delete( p_wk->p_bmp );
	if( p_wk->save != DEBUGPRINT_SAVE_TYPE_NONE )
	{	
		//もろもろ復帰
		GFL_BG_SetScroll( p_wk->frm , GFL_BG_SCROLL_X_SET , p_wk->scroll_x_bkup );
		GFL_BG_SetScroll( p_wk->frm , GFL_BG_SCROLL_Y_SET , p_wk->scroll_y_bkup );
		GFL_BG_SetPriority( p_wk->frm , p_wk->prioryty_bkup );
		//Fontカラーの復帰
		GFL_FONTSYS_SetColor( p_wk->font_col_bkup[0] ,
				p_wk->font_col_bkup[1] ,
				p_wk->font_col_bkup[2] );
		GFL_STD_MemCopy16(p_wk->p_char_temp_area, GFL_DISPUT_GetCgxPtr(p_wk->frm), DEBUGPRINT_CHAR_TEMP_AREA);
		GFL_STD_MemCopy16(p_wk->p_scrn_temp_area, GFL_DISPUT_GetScrPtr(p_wk->frm), DEBUGPRINT_SCRN_TEMP_AREA);
		GFL_STD_MemCopy16(p_wk->p_pltt_temp_area, GFL_DISPUT_GetPltPtr(p_wk->frm), DEBUGPRINT_PLTT_TEMP_AREA);
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域に書き込み
 *
 *	@param	u16 *cp_str							ワイド文字列
 *	@param	x												座標X
 *	@param	y												座標Y
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Print( DEBUG_PRINT_WORK *p_wk, const u16 *cp_str, u16 x, u16 y )
{	
	STRBUF	*p_strbuf;
	STRCODE	str[128];
	u16	strlen;

	GF_ASSERT(p_wk);

	//STRBUF用に変換
	strlen	= wcslen(cp_str);
	GFL_STD_MemCopy(cp_str, str, strlen*2);
	str[strlen]	= GFL_STR_GetEOMCode();

	//STRBUFに転送
	p_strbuf	= GFL_STR_CreateBuffer( strlen*2, p_wk->heapID );
	GFL_STR_SetStringCode( p_strbuf, str);

	//書き込み
	PRINTSYS_Print( p_wk->p_bmp, x, y, p_strbuf, p_wk->p_font );

	//破棄
	GFL_STR_DeleteBuffer( p_strbuf );
}

//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域に数値つき文字書き込み
 *
 *	@param	u16 *cp_str							ワイド文字列（%dや%fを使ってください）
 *	@param	number									数字
 *	@param	x												座標X
 *	@param	y												座標Y
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_PrintNumber( DEBUG_PRINT_WORK *p_wk, const u16 *cp_str, int number, u16 x, u16 y )
{	
	u16	str[128];
	swprintf( str, 128, cp_str, number );
	DEBUGPRINT_Print( p_wk, str, x, y );
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域をクリアー
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Clear( DEBUG_PRINT_WORK *p_wk )
{	
	GF_ASSERT(p_wk);
	GFL_BMP_Clear( p_wk->p_bmp, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域、一部範囲クリアー
 *
 *	@param	x	座標X
 *	@param	y	座標Y
 *	@param	w	幅
 *	@param	h	高さ
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_ClearRange( DEBUG_PRINT_WORK *p_wk, s16 x, s16 y, u16 w, u16 h )
{	
	GF_ASSERT(p_wk);
	GFL_BMP_Fill( p_wk->p_bmp, x, y, w, h, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグプリント
 *
 *	@param	TOWNMAP_WORK *p_wk	メインワーク
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Update( DEBUG_PRINT_WORK *p_print, TOWNMAP_WORK *p_wk )
{	
	if( !DEBUGPRINT_IsOpen(p_print) )
	{	
		return;
	}

	DEBUGPRINT_Clear(p_print);

	{	
		GFL_POINT pos;
		CURSOR_GetPos( &p_wk->cursor, &pos );
		DEBUGPRINT_Print( p_print, L"カーソル座標 ", 32, 32 );
		DEBUGPRINT_PrintNumber( p_print, L"X=[%d]", pos.x, 80, 32 );
		DEBUGPRINT_PrintNumber( p_print, L"Y=[%d]", pos.y, 120, 32 );
	}
#if 0
	//強調表示のものをデバッグプリント
	{	
		const PLACE_WORK *cp_place	= &p_wk->place;
		const PLACE_DATA* cp_data		= &cp_place->p_place[i];

		GFL_POINT pos;


		pos.x	= ;
		pos.y	= ;
		DEBUGPRINT_Print( p_print, L"場所座標 ", 32, 64 );
		DEBUGPRINT_PrintNumber( p_print, L"X=[%d]", pos.x, 80, 64 );
		DEBUGPRINT_PrintNumber( p_print, L"Y=[%d]", pos.y, 120, 64 );
	}
#endif
}
#endif //DEBUG_PRINT_USE

//=============================================================================
/**
 *	DEBUG_POS_CHECK
 */
//=============================================================================
#ifdef DEBUG_POS_CHECK
//----------------------------------------------------------------------------
/**
 *	@brief	位置を表示
 *
 *	@param	DEBUG_PRINT_WORK *p_print	プリントワーク
 *	@param	*p_wk											メインワーク
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_POS_Update( DEBUG_PRINT_WORK *p_print, TOWNMAP_WORK *p_wk )
{	
	const PLACE_WORK *cp_place	= &p_wk->place;

	GF_ASSERT(p_wk);
	GF_ASSERT(cp_place);

	if( !DEBUGPRINT_IsOpen(p_print) )
	{	
		return;
	}

	DEBUGPRINT_Clear(p_print);
	{	
		int i;
		const PLACE_DATA* cp_placedata;
		GFL_POINT	start;
		GFL_POINT	end;
		int w;
		u8	plt;
		for( i = 0; i < cp_place->data_num; i++ )
		{	
			cp_placedata	= &cp_place->p_place[i];

			if( cp_placedata )
			{	
				start.x	= PLACEDATA_GetParam( cp_placedata, TOWNMAP_DATA_PARAM_HIT_START_X )+ cp_place->pos.x;
				start.y	= PLACEDATA_GetParam( cp_placedata, TOWNMAP_DATA_PARAM_HIT_START_Y )+ cp_place->pos.y;
				end.x		= PLACEDATA_GetParam( cp_placedata, TOWNMAP_DATA_PARAM_HIT_END_X ) + cp_place->pos.x;
				end.y		= PLACEDATA_GetParam( cp_placedata, TOWNMAP_DATA_PARAM_HIT_END_Y ) + cp_place->pos.y;
				w				= PLACEDATA_GetParam( cp_placedata, TOWNMAP_DATA_PARAM_HIT_WIDTH );
			
				if(p_wk->checkpos_num_debug == i)
				{	
					plt = 3;
				}else{	
					plt	= 1;
				}
				BMP_DrawBoldLine( p_print->p_bmp, &start, &end, w, plt );
			}
		}
	}
}

#endif //DEBUG_POS_CHECK

//=============================================================================
/**
 *	DEBUG_MENU_FUNC
 */
//=============================================================================
#ifdef DEBUG_MENU_USE
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグプリント用処理関数
 *
 *	@param	void* p_wk_adrs	ワーク
 *	@param	p_item					アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_UPDATE_Print( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {	
		if( p_wk->is_print_debug )
		{	
			DEBUGPRINT_Clear(sp_dp_wk);
			DEBUGPRINT_Close(sp_dp_wk);
		}
		else
		{	
			if( DEBUGPRINT_IsOpen(sp_dp_wk) )
			{	
				DEBUGPRINT_Clear(sp_dp_wk);
				DEBUGPRINT_Close(sp_dp_wk);
			}

			DEBUGPRINT_Open(sp_dp_wk);
			DEBUGPRINT_Update(sp_dp_wk, p_wk_adrs );
		}
		p_wk->is_print_debug	^= TRUE;
		DEBUGWIN_RefreshScreen();
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグプリント用描画関数
 *
 *	@param	void* p_wk_adrs	ワーク
 *	@param	p_item					アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_DRAW_Print( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

	if( p_wk->is_print_debug )
	{	
		DEBUGWIN_ITEM_SetNameV( p_item, "デバッグ表示[現在：ON]" );
	}
	else
	{	
		DEBUGWIN_ITEM_SetNameV( p_item, "デバッグ表示[現在：OFF]" );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	当たり判定や位置をチェック表示用処理関数
 *
 *	@param	void* p_wk_adrs	ワーク
 *	@param	p_item					アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_UPDATE_CheckPos( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {	
		if( p_wk->is_checkpos_debug )
		{	
			DEBUGPRINT_Clear(sp_dp_wk);
			DEBUGPRINT_Close(sp_dp_wk);
			p_wk->is_checkpos_debug	= FALSE;
		}
		else
		{
			if( DEBUGPRINT_IsOpen(sp_dp_wk) )
			{	
				DEBUGPRINT_Clear(sp_dp_wk);
				DEBUGPRINT_Close(sp_dp_wk);
			}

			DEBUGPRINT_Open(sp_dp_wk);
			DEBUGPRINT_POS_Update( sp_dp_wk, p_wk_adrs );
			p_wk->is_checkpos_debug	= TRUE;
		}
		DEBUGWIN_RefreshScreen();
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	当たり判定や位置をチェック表示用描画関数
 *
 *	@param	void* p_wk_adrs	ワーク
 *	@param	p_item					アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_DRAW_CheckPos( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

	if( p_wk->is_checkpos_debug )
	{	
		DEBUGWIN_ITEM_SetNameV( p_item, "位置表示[現在：ON]" );
	}
	else
	{	
		DEBUGWIN_ITEM_SetNameV( p_item, "位置表示[現在：OFF]" );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	位置強調表示用処理関数
 *
 *	@param	void* p_wk_adrs	ワーク
 *	@param	p_item					アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_UPDATE_CheckPosNum( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {	
		p_wk->checkpos_num_debug++;
		p_wk->checkpos_num_debug	%= TOWNMAP_DATA_MAX;
		DEBUGWIN_RefreshScreen();

		if( p_wk->is_checkpos_debug == TRUE )
		{	
			DEBUGPRINT_POS_Update( sp_dp_wk, p_wk_adrs );
		}

	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	位置強調表示用描画関数
 *
 *	@param	void* p_wk_adrs	ワーク
 *	@param	p_item					アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_DRAW_CheckPosNum( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;
	DEBUGWIN_ITEM_SetNameV( p_item, "位置強調[現在：%d]", p_wk->checkpos_num_debug );
}

//----------------------------------------------------------------------------
/**
 *	@brief	場所OBJをすべて表示用処理関数
 *
 *	@param	void* p_wk_adrs		ワーク
 *	@param	p_item						アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_UPDATE_VisiblePlace( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {	
		p_wk->is_place_visible_debug	^= TRUE;
		PLACE_SetVisible( &p_wk->place, p_wk->is_place_visible_debug );
		DEBUGWIN_RefreshScreen();
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	場所OBJをすべて表示用描画関数
 *
 *	@param	void* p_wk_adrs		ワーク
 *	@param	p_item						アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_DRAW_VisiblePlace( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

	if( p_wk->is_place_visible_debug )
	{	
		DEBUGWIN_ITEM_SetName( p_item, "場所OBJ表示[現在：ON]" );
	}
	else
	{	
		DEBUGWIN_ITEM_SetName( p_item, "場所OBJ表示[現在：OFF]" );	
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	到着フラグ用処理関数
 *
 *	@param	void* p_wk_adrs	ワーク
 *	@param	p_item					アイテム
 */
//----------------------------------------------------------------------------
enum
{	
	DEBUGMENU_ARRIVE_NORMAL,
	DEBUGMENU_ARRIVE_ON,
	DEBUGMENU_ARRIVE_OFF,
	DEBUGMENU_ARRIVE_MAX
};
static void DEBUGMENU_UPDATE_ArriveFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;
	PLACE_WORK		*p_place	= &p_wk->place;
	PLACE_DATA		*p_data;
	int i;

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {	
		p_wk->is_arrive_debug	++;
		p_wk->is_arrive_debug	%= DEBUGMENU_ARRIVE_MAX;

		switch( p_wk->is_arrive_debug )
		{	
		case DEBUGMENU_ARRIVE_NORMAL:
			for( i = 0; i < p_place->data_num; i++ )
			{	
				p_data	= &p_place->p_place[ i ];
				p_data->is_sky	= p_data->is_arrive && TOWNMAP_DATA_GetParam( p_data->cp_data, p_data->data_idx, TOWNMAP_DATA_PARAM_SKY_FLAG );
;
			}
			break;
		case DEBUGMENU_ARRIVE_ON:
			for( i = 0; i < p_place->data_num; i++ )
			{	
				p_data	= &p_place->p_place[ i ];
				p_data->is_sky	= TRUE;
			}
			break;
		case DEBUGMENU_ARRIVE_OFF:
			for( i = 0; i < p_place->data_num; i++ )
			{	
				p_data	= &p_place->p_place[ i ];
				p_data->is_sky	= FALSE;
			}
			break;
		}
		DEBUGWIN_RefreshScreen();
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	到着フラグ用処理関数
 *
 *	@param	void* p_wk_adrs	ワーク
 *	@param	p_item					アイテム
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_DRAW_ArriveFlag( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{	
	TOWNMAP_WORK	*p_wk	= p_wk_adrs;

	switch( p_wk->is_arrive_debug )
	{	
	case DEBUGMENU_ARRIVE_NORMAL:
		DEBUGWIN_ITEM_SetName( p_item, "到着フラグ操作[現在：通常]" );
		break;
	case DEBUGMENU_ARRIVE_ON:
		DEBUGWIN_ITEM_SetName( p_item, "到着フラグ操作[現在：全てON]" );	
		break;
	case DEBUGMENU_ARRIVE_OFF:
		DEBUGWIN_ITEM_SetName( p_item, "到着フラグ操作[現在：全てOFF]" );
		break;
	}
}

#endif //DEBUG_MENU_USE
