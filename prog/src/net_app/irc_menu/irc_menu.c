//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_meu.c
 *	@brief	赤外線ミニゲーム　メニュー画面
 *	@author	Toru=Nagihashi
 *	@data		2009.05.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	lib
#include <gflib.h>

//	system
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"

//	module
#include "infowin/infowin.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/bmp_winframe.h"
#include "net_app/compatible_irc_sys.h"

#include "net_app/irc_appbar.h"

//sound
#include "../irc_compatible/irc_compatible_snd.h"

//	archive
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_irc_compatible.h"
#include "irccompatible_gra.naix"

#include "net_app/irc_menu.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#ifdef PM_DEBUG
static int sc_debug_num = FX32_ONE;

#endif

#define DEG_TO_IDX(x) ( 0xFFFF * x / 360 )


//-------------------------------------
///	音楽
//=====================================
#define MENU_SE_DECIDE	(SEQ_SE_DECIDE1)
#define MENU_SE_CANCEL	(SEQ_SE_CANCEL1)


//-------------------------------------
///	パレット
//=====================================
enum{	
	// メイン画面BG
	IRC_MENU_BG_PAL_M_00 = 0,//背景
	IRC_MENU_BG_PAL_M_01,		//
	IRC_MENU_BG_PAL_M_02,		//
	IRC_MENU_BG_PAL_M_03,		// 
	IRC_MENU_BG_PAL_M_04,		//	バーの色下普通
	IRC_MENU_BG_PAL_M_05,		//　INFOWIN
	IRC_MENU_BG_PAL_M_06,		// 背景ここまで	
	IRC_MENU_BG_PAL_M_07,		// 背景
	IRC_MENU_BG_PAL_M_08,		// フォント
	IRC_MENU_BG_PAL_M_09,		// バーの色下明るい
	IRC_MENU_BG_PAL_M_10,		// バーの色下くらい
	IRC_MENU_BG_PAL_M_11,		// バーの色上普通
	IRC_MENU_BG_PAL_M_12,		// バーの色上明るい
	IRC_MENU_BG_PAL_M_13,		// バーの色上暗い
	IRC_MENU_BG_PAL_M_14,		//APPBAR
	IRC_MENU_BG_PAL_M_15,		// APPBAR

	// サブ画面BG
	IRC_MENU_BG_PAL_S_00 = 0,	//背景
	IRC_MENU_BG_PAL_S_01,		// 
	IRC_MENU_BG_PAL_S_02,		// 
	IRC_MENU_BG_PAL_S_03,		// 
	IRC_MENU_BG_PAL_S_04,		// 
	IRC_MENU_BG_PAL_S_05,		//
	IRC_MENU_BG_PAL_S_06,		// 背景ここまで	//上画面用バー＆フォント
	IRC_MENU_BG_PAL_S_07,		// 背景
	IRC_MENU_BG_PAL_S_08,		// 背景
	IRC_MENU_BG_PAL_S_09,		// 使用してない
	IRC_MENU_BG_PAL_S_10,		// 使用してない
	IRC_MENU_BG_PAL_S_11,		// 使用してない
	IRC_MENU_BG_PAL_S_12,		// 使用してない
	IRC_MENU_BG_PAL_S_13,		// 使用してない
	IRC_MENU_BG_PAL_S_14,		// 使用してない
	IRC_MENU_BG_PAL_S_15,		// 使用してない

	// メイン画面OBJ
	IRC_MENU_OBJ_PAL_M_00 = 0,//
	IRC_MENU_OBJ_PAL_M_01,		// 
	IRC_MENU_OBJ_PAL_M_02,		// 使用してない
	IRC_MENU_OBJ_PAL_M_03,		// 使用してない
	IRC_MENU_OBJ_PAL_M_04,		// 使用してない
	IRC_MENU_OBJ_PAL_M_05,		// 使用してない
	IRC_MENU_OBJ_PAL_M_06,		// 使用してない
	IRC_MENU_OBJ_PAL_M_07,		// 使用してない
	IRC_MENU_OBJ_PAL_M_08,		// 使用してない
	IRC_MENU_OBJ_PAL_M_09,		// 使用してない
	IRC_MENU_OBJ_PAL_M_10,		// 使用してない
	IRC_MENU_OBJ_PAL_M_11,		// 使用してない
	IRC_MENU_OBJ_PAL_M_12,		// 使用してない
	IRC_MENU_OBJ_PAL_M_13,		// 使用してない
	IRC_MENU_OBJ_PAL_M_14,		//	
	IRC_MENU_OBJ_PAL_M_15,		// 

	// サブ画面OBJ
	IRC_MENU_OBJ_PAL_S_00 = 0,	//
	IRC_MENU_OBJ_PAL_S_01,		// 使用してない
	IRC_MENU_OBJ_PAL_S_02,		// 使用してない
	IRC_MENU_OBJ_PAL_S_03,		// 使用してない
	IRC_MENU_OBJ_PAL_S_04,		// 使用してない
	IRC_MENU_OBJ_PAL_S_05,		// 使用してない
	IRC_MENU_OBJ_PAL_S_06,		// 使用してない
	IRC_MENU_OBJ_PAL_S_07,		// 使用してない
	IRC_MENU_OBJ_PAL_S_08,		// 使用してない
	IRC_MENU_OBJ_PAL_S_09,		// 使用してない
	IRC_MENU_OBJ_PAL_S_10,		// 使用してない
	IRC_MENU_OBJ_PAL_S_11,		// 使用してない
	IRC_MENU_OBJ_PAL_S_12,		// 使用してない
	IRC_MENU_OBJ_PAL_S_13,		// 使用してない
	IRC_MENU_OBJ_PAL_S_14,		// 使用してない
	IRC_MENU_OBJ_PAL_S_15,		// 使用してない
};

//-------------------------------------
///	文字
//=====================================
#define TEXTSTR_PLT_NO				(IRC_MENU_BG_PAL_S_00)
#define TEXTSTR_BUFFER_LENGTH	(255)

//-------------------------------------
///	位置
//=====================================
#define	MSGWND_MSG_X	(1)
#define	MSGWND_MSG_Y	(18)
#define	MSGWND_MSG_W	(30)
#define	MSGWND_MSG_H	(5)

#define	MSGWND_GAME_X	(3)
#define	MSGWND_GAME_Y	(4)
#define	MSGWND_GAME_W	(26)
#define	MSGWND_GAME_H	(5)

#define	MSGWND_RANKING_X	(3)
#define	MSGWND_RANKING_Y	(14)
#define	MSGWND_RANKING_W	(26)
#define	MSGWND_RANKING_H	(5)

#define	MSGWND_TITLE_X	(6)
#define	MSGWND_TITLE_Y	(3)
#define	MSGWND_TITLE_W	(20)
#define	MSGWND_TITLE_H	(5)


//-------------------------------------
///	カウント
//=====================================
#define TOCH_COUNTER_MAX	(30*5)
#define RESULT_SEND_CNT	(COMPATIBLE_IRC_SENDATA_CNT)

//-------------------------------------
///	OBJ登録ID
//=====================================
enum {
	OBJREGID_TOUCH_PLT,
	OBJREGID_TOUCH_CHR,
	OBJREGID_TOUCH_CEL,

	OBJREGID_MAX
};
//-------------------------------------
///		BACKOBJ
//=====================================
//後ろOBJの数
#define BACKOBJ_CLWK_MAX	(32)
//
typedef enum
{
	BACKOBJ_MOVE_TYPE_RAIN,		//雨のように上から降り注ぐ
	BACKOBJ_MOVE_TYPE_EMITER,	//放出
	BACKOBJ_MOVE_TYPE_GATHER,	//集中
} BACKOBJ_MOVE_TYPE;

//OBJのパレットと対応している
typedef enum
{
	BACKOBJ_COLOR_RED			= 1,
	BACKOBJ_COLOR_ORANGE	= 5,
	BACKOBJ_COLOR_YELLOW	= 6,
	BACKOBJ_COLOR_YEGREEN	= 7,
	BACKOBJ_COLOR_GREEN		= 8,
	BACKOBJ_COLOR_WATER		= 9,
	BACKOBJ_COLOR_BLUE		= 0xA,
} BACKOBJ_COLOR;

//１つのOBJが動くまでのシンク
#define BACKOBJ_MOVE_SYNC	(10)

#define BACKOBJ_ONE_MOVE_SYNC_MIN	(110)
#define BACKOBJ_ONE_MOVE_SYNC_MAX	(160)
#define BACKOBJ_ONE_MOVE_SYNC_DIF	(BACKOBJ_ONE_MOVE_SYNC_MAX-BACKOBJ_ONE_MOVE_SYNC_MIN)

//-------------------------------------
///	背景動作
//=====================================
#define BGMOVE_MOVE_SYNC        (120)
#define BGMOVE_MOVE_START_Y     (0)
#define BGMOVE_MOVE_END_Y       (11*8)
#define BGMOVE_MOVE_VELOCITY_Y  (FX32_CONST(0))

//-------------------------------------
///	CLWK取得
//=====================================
typedef enum
{	
  CLWKID_BUTTERFLY_TOP,
  CLWKID_BUTTERFLY_END  = CLWKID_BUTTERFLY_TOP + IRC_COMPATIBLE_SV_RANKING_MAX,
  CLWKID_BUTTERFLY_SP,
	CLWKID_MAX
}CLWKID;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	BG関係
//=====================================
typedef struct
{
	GFL_ARCUTIL_TRANSINFO	frame_char2;	//上用
	GFL_TCB						*p_vblank_task;
} GRAPHIC_BG_WORK;
//-------------------------------------
///	OBJ関係
//=====================================
typedef struct {
	GFL_CLUNIT *p_clunit;
	u32					reg_id[OBJREGID_MAX];
	GFL_CLWK	 *p_clwk[CLWKID_MAX];
} GRAPHIC_OBJ_WORK;
//-------------------------------------
///	描画関係
//=====================================
typedef struct 
{
	GRAPHIC_OBJ_WORK	obj;
	GFL_TCB						*p_vblank_task;
} GRAPHIC_WORK;
//-------------------------------------
///	文字管理
//=====================================
typedef struct 
{
	GFL_FONT*				  p_font;
  GFL_MSGDATA*			p_msg;
  PRINT_QUE*        p_print_que;
	WORDSET*					p_wordset;
} MSG_WORK;

//-------------------------------------
///	メッセージ表示ウィンドウ
//=====================================
typedef struct 
{
	GFL_BMPWIN*				p_bmpwin;
	PRINT_UTIL        print_util;
	STRBUF*						p_strbuf;
	u16								clear_chr;
	u16								dummy;
} MSGWND_WORK;
#if 0
//-------------------------------------
///	BACKOBJ	背面ぴかぴか
//=====================================
typedef struct
{	
	GFL_CLWK					*p_clwk;
	int								sf_type;
	BOOL							is_req;
	VecFx32						start;
	VecFx32						end;
	u32								sync_now;
	u32								sync_max;
} BACKOBJ_ONE;
typedef struct 
{
	BACKOBJ_ONE				wk[BACKOBJ_CLWK_MAX];
	BACKOBJ_MOVE_TYPE	type;
	BACKOBJ_COLOR			color;
	u32								sync_now;	//１つのワークを開始するまでのシンク
	u32								sync_max;
} BACKOBJ_WORK;
#endif
//-------------------------------------
///	ボタン
//=====================================
#define BUTTON_MAX	(4)
typedef struct
{	
	u16				strID;
	u16				x;			//開始点X
	u16				y;			//開始点Y
	u16				w;			//開始点からの幅
	u16				h;
	u16				plt;
} BUTTON_SETUP;
typedef struct 
{
	GFL_BUTTON_MAN			*p_btn;
	GFL_BMPWIN					*p_bmpwin[BUTTON_MAX];
	GFL_UI_TP_HITTBL		hit_tbl[BUTTON_MAX+1];//+1は終了コード分
	const	 BUTTON_SETUP *cp_btn_setup_tbl;
	u16			btn_num;
	u16			frm;
	u16			select_btn_id;
	u16			is_touch;
} BUTTON_WORK;
//-------------------------------------
///	加速度
//=====================================
typedef struct {
	int now_val;		//現在の値
	int start_val;		//開始の値
	int end_val;		//終了の値
	fx32 add_val;		//加算値(誤差をださないようにここだけfx)
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_VELOCITY_WORK;
//-------------------------------------
///	３次曲線
//=====================================
typedef struct {
	VecFx32 now_val;	//現在の座標
	VecFx32	start_pos;	//開始座標
	VecFx32 ctrl_pos0;	//制御点０
	VecFx32 ctrl_pos1;	//制御点１
	VecFx32 end_pos;	//終了座標
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_CATMULLROM_WORK;
//-------------------------------------
///	背面動作
//=====================================
typedef struct 
{
  PROGVAL_VELOCITY_WORK  vel;
  BOOL is_start;
} BGMOVE_WORK;

//-------------------------------------
///	蝶操作
//=====================================
typedef struct 
{
  GFL_CLWK  *p_clwk;  //描画
  VecFx32 pre_pos;    //3D座標
  VecFx32 shake_pos;  //揺れる起動

  VecFx32 pre_rot_pos;
  fx32    angle_max;  //角度最大
  fx32    move_sync;  //進む速さ
  fx32    angle_speed;//角速度
  s8      angle_dir;
  u8      dummy;

  u16     target_x;   //目的座標X
  u16     target_y;   //目的座標Y

  PROGVAL_CATMULLROM_WORK cutmullrom;
} BUTTERFLY_WORK;
typedef struct
{ 
  BUTTERFLY_WORK  wk[IRC_COMPATIBLE_SV_RANKING_MAX];
  u32 cnt;
  u16 idx;
  u16 idx_max;
} BUTTERFLY_SYS;

//-------------------------------------
///	相性診断メニューメインワーク
//=====================================
typedef struct _IRC_MENU_MAIN_WORK IRC_MENU_MAIN_WORK;
typedef void (*SEQ_FUNCTION)( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
struct _IRC_MENU_MAIN_WORK
{
	//モジュール
	GRAPHIC_WORK		grp;
	GRAPHIC_BG_WORK	bg;
	MSG_WORK				msg;
	MSGWND_WORK			msgwnd;
	BUTTON_WORK			btn;
	//BACKOBJ_WORK		backobj;
  BGMOVE_WORK     bgmove;
  BUTTERFLY_SYS   butterfly;
	
	//下画面
	APPBAR_WORK			*p_appbar;

	MSGWND_WORK			msgtitle;	//タイトルメッセージ
	

	//シーケンス管理
	SEQ_FUNCTION		seq_function;
	u16		seq;
	u16		cnt;
	BOOL	is_end;

	//パラメータ
	IRC_MENU_PARAM	*p_param;

	//その他
	u32		start_ms;
	u32		select;
	u32		now_ms;
	BOOL	is_send;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT IRC_MENU_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_MENU_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_MENU_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
//grp
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
static GFL_CLWK* GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id );
static GFL_CLUNIT * GRAPHIC_GetClunit( const GRAPHIC_WORK *cp_wk );
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//BG
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void Graphic_BG_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//obj
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID );
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk );
static GFL_CLWK* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, CLWKID id );
static GFL_CLUNIT * GRAPHIC_OBJ_GetClunit( const GRAPHIC_OBJ_WORK *cp_wk );
//MSG_WORK
static void MSG_Init( MSG_WORK *p_wk, HEAPID heapID );
static void MSG_Exit( MSG_WORK *p_wk );
static BOOL MSG_Main( MSG_WORK *p_wk );
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk );
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk );
static const GFL_MSGDATA * MSG_GetMsgDataConst( const MSG_WORK *cp_wk );
static WORDSET * MSG_GetWordSet( const MSG_WORK *cp_wk );
//MSG_WINDOW
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe,
		u8 x, u8 y, u8 w, u8 h, u8 plt, HEAPID heapID );
static void MSGWND_InitEx( MSGWND_WORK* p_wk, u8 bgframe,
		u8 x, u8 y, u8 w, u8 h, u8 plt, u16 clear_chr, u8 dir, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, MSG_WORK *p_msg );
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y );
static void MSGWND_PrintCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID );
static void MSGWND_PrintColor( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y, PRINTSYS_LSB color );
static void MSGWND_PrintNumber( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id, u16 x, u16 y );
static GFL_BMPWIN * MSGWND_GetBmpWin( const MSGWND_WORK *cp_wk );
//SEQ
static void SEQ_Change( IRC_MENU_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function );
static void SEQ_End( IRC_MENU_MAIN_WORK *p_wk );
//SEQ_FUNC
static void SEQFUNC_Connect( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_Select( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_NextProc( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_DisConnect( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_End( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_SceneError( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
//BTN
static void BUTTON_Init( BUTTON_WORK *p_wk, u8 frm, const	 BUTTON_SETUP *cp_btn_setup_tbl, u8 tbl_max, const MSG_WORK *cp_msg, HEAPID heapID );
static void BUTTON_Exit( BUTTON_WORK *p_wk );
static void BUTTON_Main( BUTTON_WORK *p_wk );
static BOOL BUTTON_IsTouch( const BUTTON_WORK *cp_wk, u32 *p_btnID );
static void Button_TouchCallBack( u32 btnID, u32 event, void *p_param );
//BACKOBJ
#if 0
static void BACKOBJ_Init( BACKOBJ_WORK *p_wk, const GRAPHIC_WORK *cp_grp, BACKOBJ_MOVE_TYPE type, BACKOBJ_COLOR color, u32 clwk_ofs, int sf_type );
static void BACKOBJ_Exit( BACKOBJ_WORK *p_wk );
static void BACKOBJ_Main( BACKOBJ_WORK *p_wk );
static void BACKOBJ_StartEmit( BACKOBJ_WORK *p_wk, const GFL_POINT *cp_pos );
//BACKOBJ_ONE
static void BACKOBJ_ONE_Init( BACKOBJ_ONE *p_wk, GFL_CLWK *p_clwk, BACKOBJ_COLOR color, int sf_type );
static void BACKOBJ_ONE_Main( BACKOBJ_ONE *p_wk );
static void BACKOBJ_ONE_Start( BACKOBJ_ONE *p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, u32 sync );
static BOOL BACKOBJ_ONE_IsMove( const BACKOBJ_ONE *cp_wk );
#endif
//帳動作
static void BUTTERFLY_Init( BUTTERFLY_WORK *p_wk, GFL_CLWK *p_clwk, u32 move_sync, fx32 angle_speed, fx32 angle_max, BOOL is_blue, HEAPID heapID );
static void BUTTERFLY_Exit( BUTTERFLY_WORK *p_wk );
static void BUTTERFLY_Main( BUTTERFLY_WORK *p_wk );
static void Butterfly_SetTarget( BUTTERFLY_WORK *p_wk );
static BOOL Butterfly_IsArriveTarget( const BUTTERFLY_WORK *cp_wk );

static void BUTTERFLY_SYS_Init( BUTTERFLY_SYS *p_wk, const GRAPHIC_WORK *cp_grp, const IRC_COMPATIBLE_SAVEDATA *cp_sv, HEAPID heapID );
static void BUTTERFLY_SYS_Exit( BUTTERFLY_SYS *p_wk );
static void BUTTERFLY_SYS_Main( BUTTERFLY_SYS *p_wk );
//BGMOVE
static void BGMOVE_Init( BGMOVE_WORK *p_wk, HEAPID heapID );
static void BGMOVE_Exit( BGMOVE_WORK *p_wk );
static void BGMOVE_Main( BGMOVE_WORK *p_wk );
static void BGMOVE_Start( BGMOVE_WORK *p_wk );
//汎用
static void MainModules( IRC_MENU_MAIN_WORK *p_wk );
static BOOL TP_GetRectTrg( const BUTTON_SETUP *cp_btn );

//加速度
static void PROGVAL_VEL_Init( PROGVAL_VELOCITY_WORK* p_wk, int start, int end, int sync );
static BOOL	PROGVAL_VEL_Main( PROGVAL_VELOCITY_WORK* p_wk );
//３次曲線
static void PROGVAL_CATMULLROM_Init( PROGVAL_CATMULLROM_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_ctrl_pos0, const VecFx32 *cp_ctrl_pos1, const VecFx32 *cp_end_pos, int sync );
static BOOL PROGVAL_CATMULLROM_Main( PROGVAL_CATMULLROM_WORK* p_wk );

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	赤外線ミニゲームメニュー用プロックデータ
//=====================================
const GFL_PROC_DATA IrcMenu_ProcData	= 
{	
	IRC_MENU_PROC_Init,
	IRC_MENU_PROC_Main,
	IRC_MENU_PROC_Exit,
};
//-------------------------------------
///	BGシステム
//=====================================
typedef enum 
{
	GRAPHIC_BG_FRAME_M_INFOWIN,
	GRAPHIC_BG_FRAME_M_TEXT,
	GRAPHIC_BG_FRAME_M_BTN,
	GRAPHIC_BG_FRAME_M_BACK,
	GRAPHIC_BG_FRAME_S_ROGO,
	GRAPHIC_BG_FRAME_S_TEXT,
	GRAPHIC_BG_FRAME_S_BACK,
	GRAPHIC_BG_FRAME_S_TITLE,
	GRAPHIC_BG_FRAME_MAX
} GRAPHIC_BG_FRAME;
static const u32 sc_bgcnt_frame[ GRAPHIC_BG_FRAME_MAX ] = 
{
	GFL_BG_FRAME0_M, GFL_BG_FRAME1_M, GFL_BG_FRAME2_M, GFL_BG_FRAME3_M, GFL_BG_FRAME0_S, GFL_BG_FRAME1_S, GFL_BG_FRAME2_S, GFL_BG_FRAME3_S,
};
static const u32 sc_bgcnt_mode[ GRAPHIC_BG_FRAME_MAX ] = 
{
	GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_AFFINE,
};
static const GFL_BG_BGCNT_HEADER sc_bgcnt_data[ GRAPHIC_BG_FRAME_MAX ] = 
{
	// GRAPHIC_BG_FRAME_M_INFOWIN	
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_M_TEXT
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_M_BTN
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_M_BACK
	{	
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 2, 0, 0, FALSE

	},
	// GRAPHIC_BG_FRAME_S_ROGO
	{
		0, 0, 0x1000, 0,
		GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_TEXT
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_BACK
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_TITLE
	{
		0, 0, 0x1000, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
		GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ256_128x128,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},

};

//-------------------------------------
///	ボタン範囲
//=====================================
enum{	
	BTNID_COMATIBLE,
	BTNID_RANKING,

	BTNID_MAX,
	BTNID_NULL	= BTNID_MAX,
};
static const BUTTON_SETUP	sc_btn_setp_tbl[BTNID_MAX]	=
{	
	//list選択
	{	
		COMPATI_LIST_000,
		MSGWND_GAME_X,
		MSGWND_GAME_Y,
		MSGWND_GAME_W,
		MSGWND_GAME_H,
		IRC_MENU_BG_PAL_M_11,
	},
	//ランキング選択
	{	
		COMPATI_LIST_001,
		MSGWND_RANKING_X,
		MSGWND_RANKING_Y,
		MSGWND_RANKING_W,
		MSGWND_RANKING_H,
		IRC_MENU_BG_PAL_M_04,
	},
};

//=============================================================================
/**
 *			PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線ミニゲームメニュー	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_MENU_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_MENU_MAIN_WORK	*p_wk;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCCOMPATIBLE,  0x40000 );
	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(IRC_MENU_MAIN_WORK), HEAPID_IRCCOMPATIBLE );
	GFL_STD_MemClear( p_wk, sizeof(IRC_MENU_MAIN_WORK) );
	p_wk->p_param	= p_param;

	//モジュール初期化
	MSG_Init( &p_wk->msg, HEAPID_IRCCOMPATIBLE );
	GRAPHIC_Init( &p_wk->grp, HEAPID_IRCCOMPATIBLE );
	GRAPHIC_BG_Init( &p_wk->bg, HEAPID_IRCCOMPATIBLE );
	{
		GAME_COMM_SYS_PTR comm	= NULL;
		if( p_wk->p_param->p_gamesys )
		{	
			comm	= GAMESYSTEM_GetGameCommSysPtr(p_wk->p_param->p_gamesys);
		}
		INFOWIN_Init( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_INFOWIN], IRC_MENU_BG_PAL_M_05, comm, HEAPID_IRCCOMPATIBLE );
	}
	
	BUTTON_Init( &p_wk->btn, sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BTN],
			sc_btn_setp_tbl, NELEMS(sc_btn_setp_tbl), &p_wk->msg, HEAPID_IRCCOMPATIBLE );

	MSGWND_Init( &p_wk->msgwnd, sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TEXT],
			MSGWND_MSG_X, MSGWND_MSG_Y, MSGWND_MSG_W, MSGWND_MSG_H, IRC_MENU_BG_PAL_S_08, HEAPID_IRCCOMPATIBLE );
	BmpWinFrame_Write( p_wk->msgwnd.p_bmpwin, WINDOW_TRANS_ON, 
					GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->bg.frame_char2), IRC_MENU_BG_PAL_S_06 );

	MSGWND_InitEx( &p_wk->msgtitle, sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE],
			MSGWND_TITLE_X, MSGWND_TITLE_Y, MSGWND_TITLE_W, MSGWND_TITLE_H, IRC_MENU_BG_PAL_S_08, 1, GFL_BMP_CHRAREA_GET_B, HEAPID_IRCCOMPATIBLE );
	MSGWND_PrintCenter( &p_wk->msgtitle, &p_wk->msg, COMPATI_TITLE_000 );
	
//	NAGI_Printf( "text chr num = %d\n", GFL_BMPWIN_GetChrNum( p_wk->msgwnd.p_bmpwin ) );
//	NAGI_Printf( "title chr num = %d\n", GFL_BMPWIN_GetChrNum( p_wk->msgtitle.p_bmpwin ) );
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_SCALE_X_SET, TITLE_STR_SCALE_X );
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_SCALE_Y_SET, TITLE_STR_SCALE_Y );
	GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_CENTER_X_SET, (MSGWND_TITLE_X + MSGWND_TITLE_W/2)*8 );
	GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_CENTER_Y_SET, (MSGWND_TITLE_Y + MSGWND_TITLE_H/2)*8 );


	//BACKOBJ_Init( &p_wk->backobj, &p_wk->grp, BACKOBJ_MOVE_TYPE_RAIN, BACKOBJ_COLOR_RED, CLWKID_BACKOBJ_TOP, CLSYS_DEFREND_SUB );
  //
  BGMOVE_Init( &p_wk->bgmove, HEAPID_IRCCOMPATIBLE );
  { 
    GAMEDATA *p_data  = NULL;
    SAVE_CONTROL_WORK *p_sv_ctrl  = NULL;
    IRC_COMPATIBLE_SAVEDATA *p_sv = NULL;
    if( p_wk->p_param->p_gamesys )
    { 
      p_data  = GAMESYSTEM_GetGameData( p_wk->p_param->p_gamesys );
      p_sv_ctrl  = GAMEDATA_GetSaveControlWork( p_data );
      p_sv = IRC_COMPATIBLE_SV_GetSavedata( p_sv_ctrl );
    }
    BUTTERFLY_SYS_Init( &p_wk->butterfly, &p_wk->grp, p_sv, HEAPID_IRCCOMPATIBLE );
  }

	{	
		GFL_CLUNIT	*p_unit	= GRAPHIC_GetClunit( &p_wk->grp );
		p_wk->p_appbar	= APPBAR_Init( APPBAR_OPTION_MASK_RETURN, p_unit, sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_INFOWIN], IRC_MENU_BG_PAL_M_14, IRC_MENU_OBJ_PAL_M_00, APP_COMMON_MAPPING_128K, MSG_GetFont(&p_wk->msg ), MSG_GetPrintQue(&p_wk->msg ), HEAPID_IRCCOMPATIBLE );
	}

	switch( p_wk->p_param->mode )
	{	
	case IRCMENU_MODE_INIT:
		SEQ_Change( p_wk, SEQFUNC_Select );
		break;

	case IRCMENU_MODE_RETURN:
		SEQ_Change( p_wk, SEQFUNC_DisConnect );
		break;

	case IRCMENU_MODE_RANKING_RETURN:
		SEQ_Change( p_wk, SEQFUNC_Select );
		break;

	default:
		GF_ASSERT( 0 );
	}
		
	//メニューシーンセット
	COMPATIBLE_IRC_SetScene( p_wk->p_param->p_irc, COMPATIBLE_SCENE_MENU );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線ミニゲームメニュー	メインプロセス破棄処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_MENU_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_MENU_MAIN_WORK	*p_wk;

	p_wk	= p_work;

	//APPBAR
	APPBAR_Exit( p_wk->p_appbar );	

	//モジュール破棄
  BUTTERFLY_SYS_Exit( &p_wk->butterfly );
  BGMOVE_Exit( &p_wk->bgmove );
	//BACKOBJ_Exit( &p_wk->backobj );
	BUTTON_Exit( &p_wk->btn );

	MSGWND_Exit( &p_wk->msgtitle );
	MSGWND_Exit( &p_wk->msgwnd );
	INFOWIN_Exit();
	GRAPHIC_BG_Exit( &p_wk->bg );
	GRAPHIC_Exit( &p_wk->grp );
	MSG_Exit( &p_wk->msg );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );
	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_IRCCOMPATIBLE );
	
	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線ミニゲームメニュー	メインプロセスメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_MENU_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_MAIN,
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};

	IRC_MENU_MAIN_WORK	*p_wk;
	p_wk	= p_work;

	switch( *p_seq )
	{	
	case SEQ_INIT:
		*p_seq	= SEQ_FADEOUT_START;
		break;

	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
      GFL_CLWK *p_clwk  = GRAPHIC_GetClwk( &p_wk->grp, CLWKID_BUTTERFLY_SP );
      GFL_CLACT_WK_SetAutoAnmFlag( p_clwk, TRUE );
      BGMOVE_Start( &p_wk->bgmove );
			*p_seq	= SEQ_MAIN;
		}
		break;

	case SEQ_MAIN:
    
    BUTTERFLY_SYS_Main( &p_wk->butterfly );
    BGMOVE_Main( &p_wk->bgmove );
		p_wk->seq_function( p_wk, &p_wk->seq );
		if( p_wk->is_end )
		{	
			*p_seq	= SEQ_FADEIN_START;
		}
		break;

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
		return GFL_PROC_RES_FINISH;

	default:
		GF_ASSERT_MSG( 0, "IRC_MENU_PROC_MainのSEQエラー %d", *p_seq );
	}

	MainModules( p_wk );
	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *				GRAPHIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	描画関係初期化
 *
 *	@param	GRAPHIC_WORK* p_wk	ワーク
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Init( GRAPHIC_WORK* p_wk, HEAPID heapID )
{
	static const GFL_DISP_VRAM sc_vramSetTable =
	{
		GX_VRAM_BG_128_A,						// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,				// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_128_D,       // サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,						// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_NONE,				// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_128K,		
		GX_OBJVRAMMODE_CHAR_1D_128K,		
	};	

	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );

	// ディスプレイON
	GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
	GFL_DISP_SetDispOn();

	// VRAMバンク設定
	GFL_DISP_SetBank( &sc_vramSetTable );

	//モジュール
	GRAPHIC_OBJ_Init( &p_wk->obj, &sc_vramSetTable, heapID );

	//VBlackTask登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB(Graphic_VBlankTask, p_wk, 0 );
}


//----------------------------------------------------------------------------
/**
 *	@brief	描画関係破棄
 *
 *	@param	GRAPHIC_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Exit( GRAPHIC_WORK* p_wk )
{	

	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

	GRAPHIC_OBJ_Exit( &p_wk->obj );
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画関係描画処理
 *
 *	@param	GRAPHIC_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Draw( GRAPHIC_WORK* p_wk )
{
	GRAPHIC_OBJ_Main( &p_wk->obj );
}
//----------------------------------------------------------------------------
/**
 *	@brief	CLWK取得
 *
 *	@param	const GRAPHIC_WORK *cp_wk	ワーク
 *	@param	id	CLWKのID
 *
 *	@return	CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id )
{	
	return GRAPHIC_OBJ_GetClwk( &cp_wk->obj, id );
}
//----------------------------------------------------------------------------
/**
 *	@brief	CLUNIT取得
 *
 *	@param	const GRAPHIC_WORK *cp_wk		ワーク
 *
 *	@return	CLUNIT
 */
//-----------------------------------------------------------------------------
static GFL_CLUNIT * GRAPHIC_GetClunit( const GRAPHIC_WORK *cp_wk )
{	
	return GRAPHIC_OBJ_GetClunit( &cp_wk->obj );
}
//----------------------------------------------------------------------------
/**
 *	@brief	VBlank用タスク
 *
 *	@param	GFL_TCB *p_tcb	タスクControlブロック
 *	@param	*p_work					ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work )
{	
	GRAPHIC_WORK* p_wk	= p_work;
	GRAPHIC_OBJ_VBlankFunction( &p_wk->obj );
}


//=============================================================================
/**
 *					GRAPHIC_BG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BG描画	初期化
 *
 *	@param	GRAPHIC_BG_WORK* p_wk	ワーク
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK* p_wk, HEAPID heapID )
{
	int i;

	//ＢＧシステム起動
	GFL_BG_Init( heapID );
	GFL_BMPWIN_Init( heapID );

	//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sc_bg_sys_header = 
		{
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_1,GX_BG0_AS_2D
		};	
		GFL_BG_SetBGMode( &sc_bg_sys_header );
	}

	//ＢＧコントロール設定
	{
		int i;

		for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
		{
			GFL_BG_SetBGControl( sc_bgcnt_frame[i], &sc_bgcnt_data[i], sc_bgcnt_mode[i] );
			GFL_BG_ClearFrame( sc_bgcnt_frame[i] );
			GFL_BG_SetVisible( sc_bgcnt_frame[i], VISIBLE_ON );
		}
	}

	//読み込み設定
	{	
		ARCHANDLE	*p_handle	= GFL_ARC_OpenDataHandle( ARCID_IRCCOMPATIBLE, heapID );

		//パレット
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_irccompatible_gra_aura_bg_NCLR,
				PALTYPE_MAIN_BG, IRC_MENU_BG_PAL_M_00*0x20, 0, heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_irccompatible_gra_aura_bg_NCLR,
				PALTYPE_SUB_BG, IRC_MENU_BG_PAL_S_00*0x20, 0, heapID );

		//キャラ
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_aura_bg_NCGR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BACK], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_aura_bg_NCGR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_BACK], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_title_rogo_NCGR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_ROGO], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_shita_frame_01_NCGR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BTN], 0, 0, FALSE, heapID );
	
		//スクリーン
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_aura_bg_block_b_NSCR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BACK], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_aura_bg_block_b_NSCR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_BACK], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_title_aishou_NSCR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_ROGO], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_shita_frame_01_NSCR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BTN], 0, 0, FALSE, heapID );

		//test
/*		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_result_bg_03_NCGR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_result_bg_03_NSCR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], 0, 0, FALSE, heapID );
*/
		//ワク
		GFL_BG_FillCharacter( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], 0, 1, 0 );
		p_wk->frame_char2	= GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_irccompatible_gra_ue_frame_NCGR, sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], 0, FALSE, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}
	
	//VBlackTask登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB(Graphic_BG_VBlankTask, p_wk, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	BG描画	破棄
 *
 *	@param	GRAPHIC_BG_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK* p_wk )
{	
	//タスク破棄
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

	//リソース破棄
	{	
		GFL_BG_FreeCharacterArea(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT],
				GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_char2),
				GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->frame_char2));
		GFL_BG_FillCharacterRelease(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT],1,0);
	}

	//BG設定破棄
	{	
		int i;
		// BGコントロール破棄
		for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
		{
			GFL_BG_FreeBGControl( sc_bgcnt_frame[i] );
		}
	}

	// BGシステム破棄
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();
}
//----------------------------------------------------------------------------
/**
 *	@brief	VBlank用タスク
 *
 *	@param	GFL_TCB *p_tcb	タスクControlブロック
 *	@param	*p_work					ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void Graphic_BG_VBlankTask( GFL_TCB *p_tcb, void *p_work )
{	
	GFL_BG_VBlankFunc();

}
//=============================================================================
/**
 *				OBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	初期化
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk			ワーク
 *	@param	GFL_DISP_VRAM* cp_vram_bank	バンクテーブル
 *	@param	heapID											ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_OBJ_WORK) );

	//システム作成
	GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, cp_vram_bank, heapID );
	p_wk->p_clunit	= GFL_CLACT_UNIT_Create( 128, 0, heapID );
	GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );

	//表示
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

	//リソース読み込み
	{	
		ARCHANDLE *p_handle;

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_IRCCOMPATIBLE, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_PLT]	= GFL_CLGRP_PLTT_Register( p_handle, 
				NARC_irccompatible_gra_aura_obj_NCLR, CLSYS_DRAW_SUB, IRC_MENU_OBJ_PAL_S_00*0x20, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_irccompatible_gra_aura_obj_NCGR, FALSE, CLSYS_DRAW_SUB, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_irccompatible_gra_aura_obj_NCER, NARC_irccompatible_gra_aura_obj_NANR, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK作成
	{	
		int i;
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		for( i = CLWKID_BUTTERFLY_TOP; i < CLWKID_BUTTERFLY_END; i++ )
		{	
			p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit, 
					p_wk->reg_id[OBJREGID_TOUCH_CHR],
					p_wk->reg_id[OBJREGID_TOUCH_PLT],
					p_wk->reg_id[OBJREGID_TOUCH_CEL],
					&cldata,
					CLSYS_DEFREND_SUB,
					heapID
					);
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
			GFL_CLACT_WK_SetBgPri( p_wk->p_clwk[i], 3 );
			GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[i], TRUE );
      GFL_CLACT_WK_SetAffineParam( p_wk->p_clwk[i], CLSYS_AFFINETYPE_DOUBLE );
		}

    cldata.pos_x  = 129;
    cldata.pos_y  = 104;
    cldata.anmseq = 15;
    p_wk->p_clwk[CLWKID_BUTTERFLY_SP]	= GFL_CLACT_WK_Create( p_wk->p_clunit, 
        p_wk->reg_id[OBJREGID_TOUCH_CHR],
        p_wk->reg_id[OBJREGID_TOUCH_PLT],
        p_wk->reg_id[OBJREGID_TOUCH_CEL],
        &cldata,
        CLSYS_DEFREND_SUB,
					heapID
        );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CLWKID_BUTTERFLY_SP], TRUE );
    GFL_CLACT_WK_SetBgPri( p_wk->p_clwk[CLWKID_BUTTERFLY_SP], 2 );
    GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[CLWKID_BUTTERFLY_SP], FALSE );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	破棄
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk )
{	

	//セル破棄
	{	
		int i;
		for( i = 0; i < CLWKID_MAX; i++ )
		{
			if( p_wk->p_clwk[i] )
			{	
				GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
			}
		}
	}

	//リソース破棄
	{	
		GFL_CLGRP_PLTT_Release( p_wk->reg_id[OBJREGID_TOUCH_PLT] );
		GFL_CLGRP_CGR_Release( p_wk->reg_id[OBJREGID_TOUCH_CHR] );
		GFL_CLGRP_CELLANIM_Release( p_wk->reg_id[OBJREGID_TOUCH_CEL] );
	}


	//システム破棄
	GFL_CLACT_UNIT_Delete( p_wk->p_clunit );
	GFL_CLACT_SYS_Delete();
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_OBJ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	メイン処理
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk )
{	
	GFL_CLACT_SYS_Main();

}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	Vブランク処理
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk )
{	
	GFL_CLACT_SYS_VBlankFunc();
}

//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	CLWK取得
 *
 *	@param	const GRAPHIC_OBJ_WORK *cp_wk	ワーク
 *	@param	id														CLWKのID
 *
 *	@return	CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, CLWKID id )
{	
	GF_ASSERT( id < CLWKID_MAX );
	return cp_wk->p_clwk[id];
}

//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	CLUNIT取得
 *
 *	@param	const GRAPHIC_OBJ_WORK *cp_wk		ワーク
 *
 *	@return	CLUNIT
 */
//-----------------------------------------------------------------------------
static GFL_CLUNIT * GRAPHIC_OBJ_GetClunit( const GRAPHIC_OBJ_WORK *cp_wk )
{	
	return cp_wk->p_clunit;
}


//=============================================================================
/**
 *					MSG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	MSG関係を設定
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSG_Init( MSG_WORK *p_wk, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );

	GFL_FONTSYS_Init();

	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
    NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

	p_wk->p_print_que = PRINTSYS_QUE_Create( heapID );

	p_wk->p_msg = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_irc_compatible_dat, heapID );

	p_wk->p_wordset	= WORDSET_Create( heapID );

	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	MSG関係を破棄
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *	
 */
//-----------------------------------------------------------------------------
static void MSG_Exit( MSG_WORK *p_wk )
{	
	WORDSET_Delete( p_wk->p_wordset );

	GFL_MSG_Delete( p_wk->p_msg );

	PRINTSYS_QUE_Delete( p_wk->p_print_que );

	GFL_FONT_Delete( p_wk->p_font );
	GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	MSG関係	メイン処理
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *
 * @retval  BOOL	処理が終了していればTRUE／それ以外はFALSE
 *
 */
//-----------------------------------------------------------------------------
static BOOL MSG_Main( MSG_WORK *p_wk )
{	
	return PRINTSYS_QUE_Main( p_wk->p_print_que );
}

//----------------------------------------------------------------------------
/**
 *	@brief	FONTを取得
 *
 *	@param	const MSG_WORK *cp_wk		ワーク
 *
 *	@return	FONT
 */
//-----------------------------------------------------------------------------
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_font;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PRINTQUEを取得
 *
 *	@param	const MSG_WORK *cp_wk		ワーク
 *
 *	@return	PRINTQUE
 */
//-----------------------------------------------------------------------------
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_print_que;
}
//----------------------------------------------------------------------------
/**
 *	@brief	MSGDATAを取得
 *
 *	@param	const MSG_WORK *cp_wk		ワーク
 *
 *	@return	MSGDATA
 */
//-----------------------------------------------------------------------------
static const GFL_MSGDATA * MSG_GetMsgDataConst( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_msg;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WORDSETを取得
 *
 *	@param	const MSG_WORK *cp_wk ワーク
 *
 *	@return	WORDSET
 */
//-----------------------------------------------------------------------------
static WORDSET * MSG_GetWordSet( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_wordset;
}

//=============================================================================
/**
 *			MSGWND
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面	初期化
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	bgframe						表示するBG面
 *	@param	x									開始X位置（キャラ単位）
 *	@param	y									開始Y位置（キャラ単位）
 *	@param	w									幅（キャラ単位）
 *	@param	h									高さ（キャラ単位）
 *	@param	plt								パレット番号
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe,
		u8 x, u8 y, u8 w, u8 h, u8 plt, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
	p_wk->clear_chr	= 0xf;
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( bgframe, x, y, w, h, plt, GFL_BMP_CHRAREA_GET_B );
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( TEXTSTR_BUFFER_LENGTH, heapID );
	PRINT_UTIL_Setup( &p_wk->print_util, p_wk->p_bmpwin );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	
	GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面	初期化
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	bgframe						表示するBG面
 *	@param	x									開始X位置（キャラ単位）
 *	@param	y									開始Y位置（キャラ単位）
 *	@param	w									幅（キャラ単位）
 *	@param	h									高さ（キャラ単位）
 *	@param	plt								パレット番号
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_InitEx( MSGWND_WORK* p_wk, u8 bgframe,
		u8 x, u8 y, u8 w, u8 h, u8 plt, u16 clear_chr, u8 dir, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
	p_wk->clear_chr	= clear_chr;
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( bgframe, x, y, w, h, plt, dir );
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( TEXTSTR_BUFFER_LENGTH, heapID );
	PRINT_UTIL_Setup( &p_wk->print_util, p_wk->p_bmpwin );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	
	GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面	破棄
 *
 *	@param	MSGWND_WORK* p_wk		ワーク
 *
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
 *	@brief	メッセージ表示面	メイン処理
 *
 *	@param	MSGWND_WORK *p_wk		ワーク
 *	@param	MSG_WORK	*p_msg		メッセージ
 *
 *	@retval  BOOL	転送が終わっている場合はTRUE／終わっていない場合はFALSE
 *
 */
//-----------------------------------------------------------------------------
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, MSG_WORK *p_msg )
{	
	return PRINT_UTIL_Trans( &p_wk->print_util, p_msg->p_print_que );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面に文字を表示
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	MSG_WORK *cp_msg	文字管理
 *	@param	strID							文字ID
 *	@param	x									開始位置X
 *	@param	y									開始位置Y
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y )
{	
	const GFL_MSGDATA* cp_msgdata;
	PRINT_QUE*	p_que;
	GFL_FONT*		p_font;

	cp_msgdata	= MSG_GetMsgDataConst( cp_msg );
	p_que		= MSG_GetPrintQue( cp_msg );
	p_font	= MSG_GetFont( cp_msg );

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
	GFL_MSG_GetString( cp_msgdata, strID, p_wk->p_strbuf );

	//表示
	PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面に文字を表示
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	MSG_WORK *cp_msg	文字管理
 *	@param	strID							文字ID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID )
{	
	const GFL_MSGDATA* cp_msgdata;
	PRINT_QUE*	p_que;
	GFL_FONT*		p_font;
	u16 x, y;

	cp_msgdata	= MSG_GetMsgDataConst( cp_msg );
	p_que		= MSG_GetPrintQue( cp_msg );
	p_font	= MSG_GetFont( cp_msg );

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
	GFL_MSG_GetString( cp_msgdata, strID, p_wk->p_strbuf );

	//センター位置計算
	x	= GFL_BMPWIN_GetSizeX( p_wk->p_bmpwin )*4;
	y	= GFL_BMPWIN_GetSizeY( p_wk->p_bmpwin )*4;
	x	-= PRINTSYS_GetStrWidth( p_wk->p_strbuf, p_font, 0 )/2;
	y	-= PRINTSYS_GetStrHeight( p_wk->p_strbuf, p_font )/2;

	//キューをクリア
	PRINTSYS_QUE_Clear( p_que );

	//表示
	PRINT_UTIL_PrintColor( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font,PRINTSYS_LSB_Make(0xf,0xe,1) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面に文字を表示
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	MSG_WORK *cp_msg	文字管理
 *	@param	strID							文字ID
 *	@param	x									開始位置X
 *	@param	y									開始位置Y
 *	@param	color							色
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintColor( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y, PRINTSYS_LSB color )
{	
	const GFL_MSGDATA* cp_msgdata;
	PRINT_QUE*	p_que;
	GFL_FONT*		p_font;

	cp_msgdata	= MSG_GetMsgDataConst( cp_msg );
	p_que		= MSG_GetPrintQue( cp_msg );
	p_font	= MSG_GetFont( cp_msg );

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//文字列作成
	GFL_MSG_GetString( cp_msgdata, strID, p_wk->p_strbuf );

	//表示
	PRINT_UTIL_PrintColor( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font, color );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面に数値つき文字を表示
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	MSG_WORK *cp_msg	文字管理
 *	@param	strID							文字ID
 *	@param	number						数値
 *	@param	buff_id						ワードセット登録バッファ
 *	@param	x									開始位置X
 *	@param	y									開始位置Y
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintNumber( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id, u16 x, u16 y )
{
	const GFL_MSGDATA* cp_msgdata;
	WORDSET *p_wordset;
	
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	

	//モジュール取得
	p_wordset		= MSG_GetWordSet( cp_msg );
	cp_msgdata	= MSG_GetMsgDataConst( cp_msg );

	//数値をワードセットに登録
	WORDSET_RegisterNumber(	p_wordset, buff_id, number, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

	//元の文字列に数値を適用
	{	
		STRBUF	*p_strbuf;
		p_strbuf	= GFL_MSG_CreateString( cp_msgdata, strID );
		WORDSET_ExpandStr( p_wordset, p_wk->p_strbuf, p_strbuf );
		GFL_STR_DeleteBuffer( p_strbuf );
	}

	//表示
	{	
		PRINT_QUE*	p_que;
		GFL_FONT*		p_font;	
		p_que		= MSG_GetPrintQue( cp_msg );
		p_font	= MSG_GetFont( cp_msg );
		PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINを取得
 *
 *	@param	const MSGWND_WORK *cp_wk	ワーク
 *
 *	@return	BMPWIN
 */
//-----------------------------------------------------------------------------
static GFL_BMPWIN * MSGWND_GetBmpWin( const MSGWND_WORK *cp_wk )
{	
	return cp_wk->p_bmpwin;
}

//=============================================================================
/**
 *				SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQチェンジ
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	SEQ_FUNCTION										SEQ関数
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Change( IRC_MENU_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROC終了
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( IRC_MENU_MAIN_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}
//=============================================================================
/**
 *				SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	接続シーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Connect( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_BOOT,
		SEQ_MSG_STARTNET,
		SEQ_CONNECT,
		SEQ_MSG_PRINT,
		SEQ_SEND_STATUS,
		SEQ_TIMING_START,
		SEQ_CHANGE_SELECT,
		SEQ_SENDMENU,
		SEQ_RECVMENU,
		SEQ_SENDRETURNMENU,
		SEQ_RECVRETURNMENU,
		SEQ_SCENE,
		SEQ_TIMING_END,
		SEQ_MSG_CONNECT,
		SEQ_NEXTPROC,
	};

	switch( *p_seq )
	{	
	case SEQ_BOOT:
		COMPATIBLE_IRC_Cancel( p_wk->p_param->p_irc );
		*p_seq	= SEQ_MSG_STARTNET;
		break;

	case SEQ_MSG_STARTNET:
		if(COMPATIBLE_IRC_InitWait( p_wk->p_param->p_irc ) )
		{	
			PMSND_PlaySE( IRCCOMMON_SE_IRC );
			*p_seq	= SEQ_CONNECT;
		}
		break;

	case SEQ_CONNECT:
		if( COMPATIBLE_IRC_ConnextWait( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_MSG_PRINT;
		}

		if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
		{
			PMSND_PlaySystemSE( MENU_SE_CANCEL );
			COMPATIBLE_IRC_Cancel( p_wk->p_param->p_irc );
			SEQ_Change( p_wk, SEQFUNC_End );
		}
		break;

	case SEQ_MSG_PRINT:
		MSGWND_Print( &p_wk->msgwnd, &p_wk->msg, COMPATI_STR_004, 0, 0  );
		*p_seq	= SEQ_SEND_STATUS;
		break;

	case SEQ_SEND_STATUS:
		if(COMPATIBLE_MENU_SendStatusData( p_wk->p_param->p_irc, p_wk->p_param->p_gamesys ) )
		{	
			COMPATIBLE_MENU_GetStatusData( p_wk->p_param->p_irc, p_wk->p_param->p_you_status  );
			*p_seq	= SEQ_TIMING_START;
		}

		if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
		{
			PMSND_PlaySystemSE( MENU_SE_CANCEL );
			COMPATIBLE_IRC_Cancel( p_wk->p_param->p_irc );
			SEQ_Change( p_wk, SEQFUNC_End );
		}
		break;

	case SEQ_TIMING_START:
		if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_MENU_START ) )
		{	
			*p_seq	= SEQ_SENDMENU;
		}
		break;
		
	case SEQ_SENDMENU:
		p_wk->is_send	= TRUE;
		if( COMPATIBLE_MENU_SendMenuData( p_wk->p_param->p_irc, p_wk->now_ms, p_wk->select ) )
		{	
			*p_seq	= SEQ_RECVMENU;
		}
		break;

	case SEQ_RECVMENU:
		if( COMPATIBLE_MENU_RecvMenuData( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_SENDRETURNMENU;
		}
		break;

	case SEQ_SENDRETURNMENU:
		if( COMPATIBLE_MENU_SendReturnMenu( p_wk->p_param->p_irc ) )
		{
			*p_seq	= SEQ_RECVRETURNMENU;
		}
		break;

	case SEQ_RECVRETURNMENU:
		if( COMPATIBLE_MENU_RecvReturnMenu( p_wk->p_param->p_irc ) )
		{
			*p_seq	= SEQ_SCENE;
		}
		break;


		
	case SEQ_SCENE:
		COMPATIBLE_IRC_SetScene( p_wk->p_param->p_irc, COMPATIBLE_SCENE_RHYTHM );
		if( COMPATIBLE_IRC_SendScene( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_TIMING_END;
		}
		break;

	case SEQ_TIMING_END:
		if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_MENU_END ) )
		{	
			*p_seq	= SEQ_MSG_CONNECT;
		}
		break;

	case SEQ_MSG_CONNECT:
		if( p_wk->cnt >= RESULT_SEND_CNT )
		{	
			PMSND_PlaySE( IRCMENU_SE_IRC_ON );
			MSGWND_Print( &p_wk->msgwnd, &p_wk->msg, COMPATI_STR_002, 0, 0  );
			*p_seq	= SEQ_NEXTPROC;
		}
		break;

	case SEQ_NEXTPROC:
		p_wk->p_param->select	= IRCMENU_SELECT_COMPATIBLE;
		SEQ_Change( p_wk, SEQFUNC_NextProc );
		return;	//↓のアサートを通過しないため
	};

	//赤外線開始待ち
	if( SEQ_MSG_PRINT <= *p_seq && *p_seq <=  SEQ_MSG_CONNECT )
	{	
		if( p_wk->cnt <= RESULT_SEND_CNT )
		{	
			p_wk->cnt++;
		}
	}

	if( *p_seq < SEQ_SCENE && COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
	{	
		SEQ_Change( p_wk, SEQFUNC_SceneError );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	選択シーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Select( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_INIT,
		SEQ_SELECT,
		SEQ_MSG,
		SEQ_NEXTPROC,	
	};
	u32 ret;
#if 0
	//初期化以外受信して、次のPROCを決める処理
	if( *p_seq != SEQ_INIT  )
	{	
		if( COMPATIBLE_MENU_RecvMenuData( p_wk->p_param->p_irc ) )
		{	
			u32 you_ms;
			u32 you_select;
			COMPATIBLE_MENU_GetMenuData( p_wk->p_param->p_irc, &you_ms, &you_select );
			OS_TPrintf( "YOU PROC%d MS%d\n", you_select, you_ms );
			OS_TPrintf( "MY PROC%d MS%d\n", p_wk->select, p_wk->now_ms );
			//自分が選んでなかったら、相手に従う
			if( p_wk->select == BTNID_NULL )
			{	
				p_wk->select	= you_select;
			}
			else
			{
				//自分も選んでいたら、秒数が早い方にしたがう同じならば親機に従う
				if( p_wk->now_ms < you_ms )
				{	
	
				}
				else if( p_wk->now_ms > you_ms )
				{	
					p_wk->select	= you_select;
				}
				else
				{	
					if( GFL_NET_IsParentMachine() )
					{	
	
					}
					else
					{	
							p_wk->select	= you_select;
					}
				}
			}
			COMPATIBLE_MENU_SendReturnMenu( p_wk->p_param->p_irc );
			if( p_wk->is_send )
			{	
				*p_seq	= SEQ_RECVMENU;
			}
			else
			{	
				*p_seq	= SEQ_NEXTPROC;
			}
		}
	}
#endif
	switch( *p_seq )
	{	
	case SEQ_INIT:
		p_wk->select	= BTNID_NULL;
		p_wk->is_send	= FALSE;
		p_wk->start_ms	= OS_TicksToMilliSeconds32( OS_GetTick() );
		*p_seq	= SEQ_SELECT;
		break;

	case SEQ_SELECT:
		BUTTON_Main( &p_wk->btn );
		if( BUTTON_IsTouch( &p_wk->btn,  &p_wk->select ) )
		{	
			switch( p_wk->select )
			{	
			case BTNID_COMATIBLE:
				PMSND_PlaySE( IRCMENU_SE_DECIDE );
				*p_seq	= SEQ_MSG;
				break;
			case BTNID_RANKING:
				PMSND_PlaySE( IRCMENU_SE_DECIDE );
				p_wk->p_param->select	= IRCMENU_SELECT_RANKING;
				SEQ_Change( p_wk, SEQFUNC_NextProc );
				break;
			};
		}
		p_wk->now_ms	= OS_TicksToMilliSeconds32( OS_GetTick() ) - p_wk->start_ms;
		break;

	case SEQ_MSG:
		MSGWND_Print( &p_wk->msgwnd, &p_wk->msg, COMPATI_STR_003, 0, 0  );
		SEQ_Change( p_wk, SEQFUNC_Connect );
		break;
#if 0
	case SEQ_SENDMENU:
		p_wk->is_send	= TRUE;
		if( COMPATIBLE_MENU_SendMenuData( p_wk->p_param->p_irc, p_wk->now_ms, p_wk->select ) )
		{	
			*p_seq	= SEQ_RECVMENU;
		}
		break;

	case SEQ_RECVMENU:
		if( COMPATIBLE_MENU_RecvReturnMenu( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_NEXTPROC;
		}
		break;

	case SEQ_NEXTPROC:
		SEQ_Change( p_wk, SEQFUNC_NextProCOMPATIBLE_IRC_CompScene(c );
		break;
#endif
	};

	if( (APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN) && *p_seq >= SEQ_SELECT )
	{
		COMPATIBLE_IRC_Cancel( p_wk->p_param->p_irc );
		PMSND_PlaySystemSE( MENU_SE_CANCEL );
		SEQ_Change( p_wk, SEQFUNC_End );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	次のPROCにとぶため、終了
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_NextProc( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_INIT:
		*p_seq	= SEQ_END;
		break;

	case SEQ_END:
		SEQ_End( p_wk );
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	切断シーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DisConnect( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_SCENE,
		SEQ_NET_DISCONNECT,
		SEQ_NET_EXIT,
		SEQ_CHANGE_MENU,
	};

	switch( *p_seq )
	{	
	case SEQ_SCENE:
		COMPATIBLE_IRC_ResetScene( p_wk->p_param->p_irc );
		*p_seq	= SEQ_NET_DISCONNECT;
		break;

	case SEQ_NET_DISCONNECT:
		if( COMPATIBLE_IRC_DisConnextWait( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_NET_EXIT;
		}
		break;

	case SEQ_NET_EXIT:
		if( COMPATIBLE_IRC_ExitWait( p_wk->p_param->p_irc) )
		{	
			*p_seq	= SEQ_CHANGE_MENU;
		}
		break;

	case SEQ_CHANGE_MENU:
		SEQ_Change( p_wk, SEQFUNC_Select );
		break;
	};
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了シーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_JUNCTION,
		SEQ_NET_DISCONNECT,
		SEQ_NET_EXIT,
		SEQ_END,
	};

	switch( *p_seq )
	{
	case SEQ_JUNCTION:
		if( COMPATIBLE_IRC_IsConnext(p_wk->p_param->p_irc) )
		{	
			*p_seq	= SEQ_NET_DISCONNECT;
		}
		else if( COMPATIBLE_IRC_IsInit(p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_NET_EXIT;
		}
		else
		{	
			*p_seq	= SEQ_END;
		}
		break;

	case SEQ_NET_DISCONNECT:
		if( COMPATIBLE_IRC_DisConnextWait( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_NET_EXIT;
		}
		break;

	case SEQ_NET_EXIT:
		if( COMPATIBLE_IRC_ExitWait( p_wk->p_param->p_irc) )
		{	
			*p_seq	= SEQ_END;
		}
		break;

	case SEQ_END:
		p_wk->p_param->select	= IRCMENU_SELECT_RETURN;
		SEQ_End( p_wk );
		break;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	エラーシーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SceneError( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	//自分が先はありえない
	if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) > 0 )
	{	
		GF_ASSERT(0);
	}

	//接続していたら
	if( COMPATIBLE_IRC_IsConnext( p_wk->p_param->p_irc ) )
	{	
		//次へ飛ぶ
		COMPATIBLE_IRC_SetScene( p_wk->p_param->p_irc, COMPATIBLE_SCENE_RHYTHM );	
		p_wk->p_param->select	= IRCMENU_SELECT_COMPATIBLE;
		SEQ_Change( p_wk, SEQFUNC_NextProc );
	}
	else
	{	
		//接続していないならば、選択に戻る
		SEQ_Change( p_wk, SEQFUNC_Select );
	}

}
//=============================================================================
/**
 *			adapter
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	PROC切り替え時に消されるモジュール	メイン処理
 *
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MainModules( IRC_MENU_MAIN_WORK *p_wk )
{	
	INFOWIN_Update();

	//メッセージ処理
	if( MSG_Main( &p_wk->msg ) )
	{	
		MSGWND_Main( &p_wk->msgwnd, &p_wk->msg );
		MSGWND_Main( &p_wk->msgtitle, &p_wk->msg );
	}
	//１たん停止
	//BACKOBJ_Main( &p_wk->backobj );

	//APPBAR
	APPBAR_Main( p_wk->p_appbar );

	GRAPHIC_Draw( &p_wk->grp );
}

//=============================================================================
/**
 *						BUTTON
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン初期化	
 *
 *	@param	BUTTON_WORK *p_wk				ワーク
 *	@param	frm											ボタンを表示するフレーム
 *	@param	GFL_RECT *cp_rect_tbl		ボタンの設定テーブル(実体をもってください)
 *	@param	tbl_max									テーブルの個数
 *	@param	cp_msg									文字管理
 *	@param	frame_char							フレームに使うキャラ
 *	@param	plt											パレット
 *	@param	HEAPID									ヒープID
 */
//-----------------------------------------------------------------------------
static void BUTTON_Init( BUTTON_WORK *p_wk, u8 frm, const	 BUTTON_SETUP *cp_btn_setup_tbl, u8 tbl_max, const MSG_WORK *cp_msg, HEAPID heapID )
{	
	//エラー
	GF_ASSERT_MSG( tbl_max < BUTTON_MAX, "ボタン数が多いですBUTTON_MAXの定義を変えてください", tbl_max );

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(BUTTON_WORK) );
	p_wk->cp_btn_setup_tbl	= cp_btn_setup_tbl;
	p_wk->btn_num			= tbl_max;
	p_wk->frm					= frm;

	//HITTBLを作成（キャラ単位をドット単位に）
	{	
		int i;
		const	 BUTTON_SETUP *cp_setup;
		for( i = 0; i < p_wk->btn_num; i++ )
		{	
			cp_setup	= &cp_btn_setup_tbl[i];
			p_wk->hit_tbl[i].rect.top			= (cp_setup->y)*8+1;
			p_wk->hit_tbl[i].rect.bottom	= (cp_setup->y + cp_setup->h)*8-1;
			p_wk->hit_tbl[i].rect.left		= (cp_setup->x)*8+1;
			p_wk->hit_tbl[i].rect.right		= (cp_setup->x + cp_setup->w)*8-1;
		}
		p_wk->hit_tbl[i].rect.top	= GFL_UI_TP_HIT_END;
	}

	//ボタンマネージャ作成
	p_wk->p_btn	= GFL_BMN_Create( p_wk->hit_tbl, Button_TouchCallBack, p_wk, heapID );

	//BMPWIN作成し内容を書き込む
	{	
		int i;
		const	 BUTTON_SETUP *cp_setup;
		STRBUF *p_strbuf;
		for( i = 0; i < p_wk->btn_num; i++ )
		{	
			//BMPWIN作成
			cp_setup	= &cp_btn_setup_tbl[i];
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( frm, cp_setup->x+4,
					cp_setup->y+1, cp_setup->w-8, cp_setup->h-2, cp_setup->plt, GFL_BMP_CHRAREA_GET_B );

			//BMPWIN転送
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0x4 );
			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			//文字作成描画
			p_strbuf	= GFL_MSG_CreateString( MSG_GetMsgDataConst(cp_msg), cp_setup->strID );
			GFL_FONTSYS_SetColor( 0xf, 0xe, 4 );

			//中央に配置
			{	
				u32	w;
				u32 h;
				w	= PRINTSYS_GetStrWidth( p_strbuf, MSG_GetFont(cp_msg), 0 );
				h	= PRINTSYS_GetStrHeight( p_strbuf, MSG_GetFont(cp_msg) );
				PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 
					(cp_setup->w-8)*4-w/2, (cp_setup->h-2)*4-h/2, p_strbuf, MSG_GetFont(cp_msg) );
			}
			GFL_STR_DeleteBuffer( p_strbuf );

			GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin[i] );

		}
		GFL_FONTSYS_SetDefaultColor();
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン破棄
 *
 *	@param	BUTTON_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void BUTTON_Exit( BUTTON_WORK *p_wk )
{	
	//BMPWIN破棄
	{	
		int i;
		for( i = 0; i < p_wk->btn_num; i++ )
		{	
			GFL_BMPWIN_Delete(p_wk->p_bmpwin[i]);
		}
	}

	//BMN破棄
	GFL_BMN_Delete( p_wk->p_btn );

	GFL_STD_MemClear( p_wk, sizeof(BUTTON_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンメイン処理
 *
 *	@param	BUTTON_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void BUTTON_Main( BUTTON_WORK *p_wk )
{
	p_wk->is_touch	= GFL_BMN_Main( p_wk->p_btn );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンがタッチされたかどうか
 *
 *	@param	const BUTTON_WORK *cp_wk	ワーク
 *	@param	*p_btnID									タッチされたボタンID受け取り
 *
 *	@retval	TRUEならば押された
 *	@retval	FALSEならば押されていない
 */
//-----------------------------------------------------------------------------
static BOOL BUTTON_IsTouch( const BUTTON_WORK *cp_wk, u32 *p_btnID )
{	
	if( cp_wk->is_touch )
	{	
		int i;
		if( p_btnID )
		{	
			*p_btnID	= cp_wk->select_btn_id;
		}

		for( i = 0; i < cp_wk->btn_num; i++ )
		{	
			u8 active_plt;
			u8 frm;
			u8 x, y, w, h;
			GFL_BMPWIN	*p_bmpwin	= cp_wk->p_bmpwin[i];
			frm	= GFL_BMPWIN_GetFrame(p_bmpwin);
			x		= cp_wk->cp_btn_setup_tbl[i].x;
			y		= cp_wk->cp_btn_setup_tbl[i].y;
			w		= cp_wk->cp_btn_setup_tbl[i].w;
			h		= cp_wk->cp_btn_setup_tbl[i].h;
			switch( i )
			{
			case BTNID_COMATIBLE:
				active_plt		= IRC_MENU_BG_PAL_M_12;
				break;
			case BTNID_RANKING:
				active_plt		= IRC_MENU_BG_PAL_M_09;
				break;
			}

			if( i == cp_wk->select_btn_id )
			{	
				//選ばれたボタン
				GFL_BG_ChangeScreenPalette( frm, x, y, w, h, active_plt );
				GFL_BG_LoadScreenReq( frm );
			}
			else
			{
				//選ばれなかったボタン
				GFL_BG_LoadScreenReq( frm );
			}
		}


		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンが押された時のコールバック
 *
 *	@param	u32 btnID	ボタンID
 *	@param	event			イベント
 *	@param	*p_param	Createで渡されたアドレス
 *
 */
//-----------------------------------------------------------------------------
static void Button_TouchCallBack( u32 btnID, u32 event, void *p_param )
{	
	BUTTON_WORK *p_wk;
	p_wk	= p_param;

	if( event == GFL_BMN_EVENT_TOUCH )
	{	
		p_wk->select_btn_id	= btnID;
	}
}
#if 0
//=============================================================================
/**
 *			BACKOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	BACKOBJ_WORK *p_wk		ワーク
 *	@param	GRAPHIC_WORK *cp_grp	CLWK受け取り用グラフィック
 *	@param	type									動作タイプ
 *	@param	color									色
 *	@param	clwk_ofs							CLWKオフセット何番から使い始めるか
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_Init( BACKOBJ_WORK *p_wk, const GRAPHIC_WORK *cp_grp, BACKOBJ_MOVE_TYPE type, BACKOBJ_COLOR color, u32 clwk_ofs, int sf_type )
{	
	GFL_STD_MemClear( p_wk, sizeof(BACKOBJ_WORK) );
	p_wk->type	= type;
	p_wk->color	= color;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= BACKOBJ_MOVE_SYNC;
	{	
		int i;
		GFL_CLWK *p_clwk;
		for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
		{	
			p_clwk	= GRAPHIC_GetClwk( cp_grp, clwk_ofs + i );
			BACKOBJ_ONE_Init( &p_wk->wk[i], p_clwk, color, sf_type );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	破棄
 *
 *	@param	BACKOBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_Exit( BACKOBJ_WORK *p_wk )
{	
	GFL_STD_MemClear( p_wk, sizeof(BACKOBJ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 *
 *	@param	BACKOBJ_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_Main( BACKOBJ_WORK *p_wk )
{	
	int i;
	u32 sync;
	u16 rot;
	GFL_POINT	start;
	GFL_POINT	end;

	//タイプ別移動
	switch( p_wk->type )
	{	
	case BACKOBJ_MOVE_TYPE_RAIN:		//雨
		if( p_wk->sync_now++ > p_wk->sync_max )
		{	
			p_wk->sync_now	= 0;
			for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
			{	
				if( !BACKOBJ_ONE_IsMove( &p_wk->wk[i] ) )
				{	
					u8	up_or_down	= GFUser_GetPublicRand(2);

					start.x	= GFUser_GetPublicRand(256);
					start.y	= -36;
					end.x		= start.x;
					end.y		= 192 + 36;
					sync	= BACKOBJ_ONE_MOVE_SYNC_MIN + GFUser_GetPublicRand(BACKOBJ_ONE_MOVE_SYNC_DIF);
					if( up_or_down )
					{	
						BACKOBJ_ONE_Start( &p_wk->wk[i], &end, &start, sync );
					}
					else
					{	
						BACKOBJ_ONE_Start( &p_wk->wk[i], &start, &end, sync );
					}
					break;
				}
			}
		}
		break;

	case BACKOBJ_MOVE_TYPE_EMITER:	//放出
		/* BACKOBJ_StartEmitで行う  */
		break;

	case BACKOBJ_MOVE_TYPE_GATHER:	//集中
		if( p_wk->sync_now++ > p_wk->sync_max )
		{	
			p_wk->sync_now	= 0;
			for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
			{	
				if( !BACKOBJ_ONE_IsMove( &p_wk->wk[i] ) )
				{	
					u16 rot	= GFUser_GetPublicRand(0xFFFF);
					start.x	= (FX_SinIdx(rot) * 256) >> FX32_SHIFT;
					start.y	= (FX_CosIdx(rot) * 256) >> FX32_SHIFT;
					end.x		= 128;
					end.y		= 96;
					sync	= BACKOBJ_ONE_MOVE_SYNC_MIN + GFUser_GetPublicRand(BACKOBJ_ONE_MOVE_SYNC_DIF);
					BACKOBJ_ONE_Start( &p_wk->wk[i], &start, &end, sync );
					break;
				}
			}
		}
		break;
	}


	//ワーク動作
	{	
		for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
		{	
			BACKOBJ_ONE_Main( &p_wk->wk[i] );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	放出開始
 *
 *	@param	BACKOBJ_WORK *p_wk	ワーク
 *	@param	GFL_POINT *cp_pos		放出開始座標
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_StartEmit( BACKOBJ_WORK *p_wk, const GFL_POINT *cp_pos )
{	
	int i;
	u32 sync;
	u16 rot;
	GFL_POINT	end;
	for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
	{	
		if( !BACKOBJ_ONE_IsMove( &p_wk->wk[i] ) )
		{
			rot	= GFUser_GetPublicRand(0xFFFF);
			end.x	= (FX_SinIdx(rot) * 256) >> FX32_SHIFT;
			end.y	= (FX_CosIdx(rot) * 256) >> FX32_SHIFT;
			sync	= BACKOBJ_ONE_MOVE_SYNC_MIN + GFUser_GetPublicRand(BACKOBJ_ONE_MOVE_SYNC_DIF);
			BACKOBJ_ONE_Start( &p_wk->wk[i], cp_pos, &end, sync );
		}
	}
}
//=============================================================================
/**
 *		背面OBJ1個
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	初期化
 *
 *	@param	BACKOBJ_ONE *p_wk	ワーク
 *	@param	*p_clwk						アクター
 *	@param	color							色
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_ONE_Init( BACKOBJ_ONE *p_wk, GFL_CLWK *p_clwk, BACKOBJ_COLOR color, int sf_type )
{	
	GFL_STD_MemClear( p_wk, sizeof(BACKOBJ_ONE) );
	p_wk->p_clwk	= p_clwk;
	p_wk->sf_type	= sf_type;

	GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk, color, CLWK_PLTTOFFS_MODE_PLTT_TOP );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 *
 *	@param	BACKOBJ_ONE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_ONE_Main( BACKOBJ_ONE *p_wk )
{	
	if( p_wk->is_req )
	{	
		GFL_CLACTPOS pos;
		VecFx32	v;
		VecFx32	v2;
		fx32 mag;
		VEC_Subtract( &p_wk->end, &p_wk->start, &v );
		mag	= VEC_Mag( &v );
		VEC_Normalize( &v, &v );
		mag = mag * p_wk->sync_now / p_wk->sync_max;
		VEC_MultAdd( mag, &v, &p_wk->start, &v2 );

		pos.x	= v2.x >> FX32_SHIFT;
		pos.y	= v2.y >> FX32_SHIFT;

		if( p_wk->sync_now++ > p_wk->sync_max )
		{	
			pos.x	= p_wk->end.x;
			pos.y	= p_wk->end.y;
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, FALSE );

			p_wk->is_req	= FALSE;
		}

		GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, p_wk->sf_type );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	移動開始
 *
 *	@param	BACKOBJ_ONE *p_wk		ワーク
 *	@param	GFL_POINT *cp_start	開始座標
 *	@param	GFL_POINT *cp_end		終了座標
 *	@param	sync								移動シンク
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_ONE_Start( BACKOBJ_ONE *p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, u32 sync )
{	
	p_wk->is_req		= TRUE;
	VEC_Set( &p_wk->start, FX32_CONST(cp_start->x), FX32_CONST(cp_start->y), 0 );
	VEC_Set( &p_wk->end, FX32_CONST(cp_end->x), FX32_CONST(cp_end->y), 0 );
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
	GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	動作中フラグ取得
 *
 *	@param	const BACKOBJ_ONE *cp_wk	ワーク
 *
 *	@return	TRUEならば動作中	FALSEならば動いていない
 */
//-----------------------------------------------------------------------------
static BOOL BACKOBJ_ONE_IsMove( const BACKOBJ_ONE *cp_wk )
{	
	return cp_wk->is_req;
}
#endif
//=============================================================================
/**
 *    BGMOVE
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  BGワーク  初期化
 *
 *	@param	BGMOVE_WORK *p_wk   ワーク
 *	@param	heapID              ヒープID
 */
//-----------------------------------------------------------------------------
static void BGMOVE_Init( BGMOVE_WORK *p_wk, HEAPID heapID )
{ 
  GFL_STD_MemClear( p_wk, sizeof(BGMOVE_WORK) );

  GFL_BG_SetScrollReq( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_ROGO ], GFL_BG_SCROLL_Y_SET, BGMOVE_MOVE_END_Y );
  GFL_BG_SetScrollReq( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TITLE ], GFL_BG_SCROLL_Y_SET,BGMOVE_MOVE_END_Y );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BGワーク  破棄
 *
 *	@param	BGMOVE_WORK *p_wk   ワーク
 */
//-----------------------------------------------------------------------------
static void BGMOVE_Exit( BGMOVE_WORK *p_wk )
{ 
  GFL_STD_MemClear( p_wk, sizeof(BGMOVE_WORK) );
}
static int test = 0;
//----------------------------------------------------------------------------
/**
 *	@brief  BGワーク  メイン
 *
 *	@param	BGMOVE_WORK *p_wk   ワーク
 */
//-----------------------------------------------------------------------------
static void BGMOVE_Main( BGMOVE_WORK *p_wk )
{ 

  if( p_wk->is_start )
  {
  
    if( PROGVAL_VEL_Main(&p_wk->vel)  )
    { 
      p_wk->is_start = FALSE;
    }

    GFL_BG_SetScrollReq( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_ROGO ], GFL_BG_SCROLL_Y_SET, BGMOVE_MOVE_END_Y - p_wk->vel.now_val );
    GFL_BG_SetScrollReq( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TITLE ], GFL_BG_SCROLL_Y_SET, BGMOVE_MOVE_END_Y - p_wk->vel.now_val );

    test++;
  }

  NAGI_Printf( "bgmove %d\n", test );
}
//----------------------------------------------------------------------------
/**
 *	@brief  BGワーク  開始
 *
 *	@param	BGMOVE_WORK *p_wk   ワーク
 */
//-----------------------------------------------------------------------------
static void BGMOVE_Start( BGMOVE_WORK *p_wk )
{ 
  test  = 0;
  p_wk->is_start  = TRUE;
  PROGVAL_VEL_Init( &p_wk->vel, BGMOVE_MOVE_START_Y, BGMOVE_MOVE_END_Y, BGMOVE_MOVE_SYNC );
}
//=============================================================================
/**
 *      蝶動作
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  蝶1匹の動き 初期化
 *
 *	@param	BUTTERFLY_WORK *p_wk  ワーク
 *	@param	*p_clwk               CLWK
 *	@param	speed                 進行速度
 *	@param	angle_speed           角速度
 *	@param	angle_max             角度最大
 *	@param	heapID                ヒープID
 */
//-----------------------------------------------------------------------------
static void BUTTERFLY_Init( BUTTERFLY_WORK *p_wk, GFL_CLWK *p_clwk, u32 move_sync, fx32 angle_speed, fx32 angle_max, BOOL is_blue, HEAPID heapID )
{ 
  GFL_STD_MemClear( p_wk, sizeof(BUTTERFLY_WORK) );
  p_wk->p_clwk      = p_clwk;
  p_wk->angle_max   = angle_max;
  p_wk->move_sync   = move_sync;
  p_wk->angle_speed = angle_speed;
  p_wk->angle_dir   = 1;

  //色の設定
  {
    u16 anmseq;
    if( is_blue )
    { 
      anmseq  = 13;
    }
    else
    { 
      anmseq  = 12;
    }
    GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk, anmseq );
  }

  //初期は画面外にいる
  { 
    GFL_CLACTPOS  pos;
#if 1
    switch( GFUser_GetPublicRand0(4) )
    { 
    case 0: //左
      pos.x = -16;
      pos.y = GFUser_GetPublicRand0( 192 );
      break;
    case 1: //右
      pos.x = 256+16;
      pos.y = GFUser_GetPublicRand0( 192 );
      break;
    case 2: //上
      pos.x = GFUser_GetPublicRand0( 256 );
      pos.y = -16;
      break;
    case 3: //下
      pos.x = GFUser_GetPublicRand0( 256 );
      pos.y = 192 + 16;
      break;
    }
#else
    pos.x = 128;
    pos.y = 96;
#endif
    GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, CLSYS_DEFREND_SUB );
  }

  //初期の目的地設定（上記初期値設定後に行うこと）
  Butterfly_SetTarget( p_wk );

  { 
    GFL_CLACTPOS  pos;
    GFL_CLACT_WK_GetPos( p_wk->p_clwk, &pos, CLSYS_DEFREND_SUB );
    NAGI_Printf( "●蝶\n" );
    NAGI_Printf( "　初期座標 X=%d Y=%d\n", pos.x, pos.y );
    NAGI_Printf( "　目標座標 X=%d Y=%d\n", p_wk->target_x, p_wk->target_y );
    NAGI_Printf( "　速度     0x%x\n", move_sync );
    NAGI_Printf( "　角速度 0x%x\n", p_wk->angle_speed );
    NAGI_Printf( "　最大角 0x%x\n", p_wk->angle_max );
  }

  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );
  GFL_CLACT_WK_SetAnmFrame( p_wk->p_clwk, GFUser_GetPublicRand0(FX32_ONE*2) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  蝶１匹の動き  破棄
 *
 *	@param	BUTTERFLY_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BUTTERFLY_Exit( BUTTERFLY_WORK *p_wk )
{ 
  GFL_STD_MemClear( p_wk, sizeof(BUTTERFLY_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  蝶１匹の動き  メイン
 *
 *	@param	BUTTERFLY_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BUTTERFLY_Main( BUTTERFLY_WORK *p_wk )
{
  VecFx32 shake;

  //揺れる処理
  { 
    p_wk->shake_pos.x = 0;
    p_wk->shake_pos.z = 0;
    p_wk->shake_pos.y += p_wk->angle_speed * p_wk->angle_dir;
    if( p_wk->shake_pos.y > p_wk->angle_max/2 )
    { 
      p_wk->shake_pos.y = p_wk->angle_max/2;
      p_wk->angle_dir = -1;
    }
    if( p_wk->shake_pos.y < -p_wk->angle_max/2 )
    { 
      p_wk->shake_pos.y = -p_wk->angle_max/2;
      p_wk->angle_dir   = 1;
    }
  }

  //移動
  p_wk->pre_pos = p_wk->cutmullrom.now_val;
  //目的地まで来たら、再度目的地を設定
  if( PROGVAL_CATMULLROM_Main( &p_wk->cutmullrom ) )
  { 
    Butterfly_SetTarget( p_wk );
  }

   //回転
  { 
    VecFx32 vec_move;

    VEC_Subtract( &p_wk->cutmullrom.now_val, &p_wk->pre_pos, &vec_move );
    vec_move.z  = 0;
    VEC_Normalize( &vec_move, &vec_move );

    //ゆれ
    { 
      VecFx32 vec0;
      MtxFx33 rot;

      VEC_Set( &vec0, FX32_ONE, 0, 0 );
      GFL_CALC3D_MTX_GetRotVecToVec( &vec0, &vec_move, &rot );
      MTX_MultVec33( &p_wk->shake_pos, &rot, &shake );
    }

    //OBJの回転のために、移動する方向から角度を取り出す
    { 

      VecFx32 vec0;
      VecFx32 vec_dir;
      u16 obj_angle;

      VEC_Set( &vec0, 0, -FX32_ONE, 0 );

      vec_dir.x = p_wk->cutmullrom.now_val.x + shake.x;
      vec_dir.y = p_wk->cutmullrom.now_val.y + shake.y;
      vec_dir.z = 0;

      //前回と同じ座標ではないときだけ回転

      VEC_Subtract( &vec_dir, &p_wk->pre_rot_pos, &vec_dir );
      VEC_Normalize( &vec_dir, &vec_dir );
      if( vec_dir.x != 0 && vec_dir.y != 0 )
      { 
        obj_angle = FX_AcosIdx( GFL_CALC3D_VEC_GetCos( &vec_dir, &vec0 ) );

        { 
          VecFx32 vec_norm;
          VEC_Set( &vec_norm, 0, 0, FX32_ONE );
          //cosでは0～180までしか求められないので左右判別する
          if( GFL_CALC3D_VEC_CalcRotVectorLR( &vec_norm, &vec0, &vec_dir ) < 0 )
          { 
            obj_angle = DEG_TO_IDX(360) - obj_angle;
          }
        }

        GFL_CLACT_WK_SetRotation( p_wk->p_clwk, obj_angle );
      }
    }
  }


  //設定
  { 
    GFL_CLACTPOS  pos;

    pos.x = (p_wk->cutmullrom.now_val.x + shake.x) >> FX32_SHIFT;
    pos.y = (p_wk->cutmullrom.now_val.y + shake.y) >> FX32_SHIFT;
    GFL_CLACT_WK_SetPos( p_wk->p_clwk, &pos, CLSYS_DEFREND_SUB );
  }

  { 
    p_wk->pre_rot_pos.x = p_wk->cutmullrom.now_val.x + shake.x;
    p_wk->pre_rot_pos.y = p_wk->cutmullrom.now_val.y + shake.y;
    p_wk->pre_rot_pos.z = 0;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  蝶１匹の動き  ターゲット設定
 *
 *	@param	BUTTERFLY_WORK *p_wk  ワーク
 *	@param	x   X座標設定
 *	@param	y   Y座標設定
 */
//-----------------------------------------------------------------------------
static void Butterfly_SetTarget( BUTTERFLY_WORK *p_wk )
{ 
  //p_wk->target_x  = x;
  //p_wk->target_y  = y;
  //NAGI_Printf( "Set Target X=%d Y=%d\n", x, y );

  { 
    GFL_CLACTPOS  pos;
    VecFx32 start_pos;
    VecFx32 end_pos;
    VecFx32 ctrl_pos0;
    VecFx32 ctrl_pos1;

    GFL_CLACT_WK_GetPos( p_wk->p_clwk, &pos, CLSYS_DEFREND_SUB );
    VEC_Set( &start_pos, pos.x << FX32_SHIFT, pos.y << FX32_SHIFT, 0 );
    VEC_Set( &end_pos, GFUser_GetPublicRand0( 256 ) << FX32_SHIFT, GFUser_GetPublicRand0( 192 ) << FX32_SHIFT, 0 );
    VEC_Set( &ctrl_pos0, GFUser_GetPublicRand0( 256 ) << FX32_SHIFT, GFUser_GetPublicRand0( 192 ) << FX32_SHIFT, 0 );
    VEC_Set( &ctrl_pos1, GFUser_GetPublicRand0( 256 ) << FX32_SHIFT, GFUser_GetPublicRand0( 192 ) << FX32_SHIFT, 0 );

    PROGVAL_CATMULLROM_Init( &p_wk->cutmullrom, &start_pos, &ctrl_pos0, &ctrl_pos1, &end_pos, p_wk->move_sync );
  }

#if 0
  //目的地までの角度を計算
  { 
    VecFx32 dir;

    GFL_CLACTPOS  pos;
    GFL_CLACT_WK_GetPos( p_wk->p_clwk, &pos, CLSYS_DEFREND_SUB );

    VEC_Set( &dir, FX32_CONST(p_wk->target_x - pos.x), FX32_CONST(p_wk->target_y - pos.y), 0 );
    VEC_Normalize( &dir, &dir );
    OS_Printf( "dir X=%f Y=%f\n", FX_FX32_TO_F32(dir.x), FX_FX32_TO_F32(dir.y) );

    { 
      VecFx32 vec0;       //0度のベクトル
      VEC_Set( &vec0, FX32_ONE, 0, 0 );
      p_wk->angle_std = FX_AcosIdx( GFL_CALC3D_VEC_GetCos( &dir, &vec0 ) );
      { 
        VecFx32 vec_norm;
        VEC_Set( &vec_norm, 0, 0, FX32_ONE );
        //cosでは0～180までしか求められないので左右判別する
        if( GFL_CALC3D_VEC_CalcRotVectorLR( &vec_norm, &vec0, &dir ) < 0 )
        { 
          p_wk->angle_std = DEG_TO_IDX(360) - p_wk->angle_std;
        }
      }
      NAGI_Printf( "目標角度 %d\n", 360 * p_wk->angle_std / 0xFFFF  );
    }
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  蝶1匹の動き 到着チェック
 *
 *	@param	const BUTTERFLY_WORK *cp_wk ワーク
 *
 *	@return TRUEで到着  FALSEでまだ
 */
//-----------------------------------------------------------------------------
static BOOL Butterfly_IsArriveTarget( const BUTTERFLY_WORK *cp_wk )
{
	s32 vx, vy, r;
  GFL_CLACTPOS  pos;

  GFL_CLACT_WK_GetPos( cp_wk->p_clwk, &pos, CLSYS_DEFREND_SUB );

	vx	= cp_wk->target_x - pos.x;
	vy	= cp_wk->target_y - pos.y;
	r	= 20;
	
	return vx * vx + vy * vy <= r * r;
}
//----------------------------------------------------------------------------
/**
 *	@brief  蝶動きシステム  初期化
 *
 *	@param	BUTTERFLY_SYS *p_wk   ワーク
 *	@param	GRAPHIC_WORK *cp_grp  グラフィックシステム
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void BUTTERFLY_SYS_Init( BUTTERFLY_SYS *p_wk, const GRAPHIC_WORK *cp_grp, const IRC_COMPATIBLE_SAVEDATA *cp_sv, HEAPID heapID )
{ 
  GFL_STD_MemClear( p_wk, sizeof(BUTTERFLY_SYS) );
  if( cp_sv == NULL )
  { 
    p_wk->idx_max = 30;
  }
  else
  { 
    p_wk->idx_max = IRC_COMPATIBLE_SV_GetRankNum( cp_sv );
  }

  //蝶の設定
  { 
    GFL_CLWK *p_clwk;
    u32 sync;
    u16 angle_speed;
    u16 angle_max;
    BOOL is_blue;

    int i;
    for( i = 0; i < p_wk->idx_max; i++ )
    { 
      p_clwk      = GRAPHIC_GetClwk( cp_grp, CLWKID_BUTTERFLY_TOP + i );
      sync        = 1400 + GFUser_GetPublicRand0( 200 );
      angle_speed = 0;//FX32_CONST(0.1f); + GFUser_GetPublicRand0( FX32_CONST(0.2f) );
      angle_max   = 0;//FX32_CONST(160.0f); + GFUser_GetPublicRand0( FX32_CONST(40.0f) );
      if( cp_sv == NULL )
      {
        is_blue = GFUser_GetPublicRand0( 2 );
      }
      else
      { 
        is_blue     = IRC_COMPATIBLE_SV_GetSex( cp_sv, i ) == IRC_COMPATIBLE_SEX_MALE;
      }
      BUTTERFLY_Init( &p_wk->wk[i], p_clwk, sync, angle_speed, angle_max, is_blue, heapID );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  蝶動きシステム  破棄
 *
 *	@param	BUTTERFLY_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BUTTERFLY_SYS_Exit( BUTTERFLY_SYS *p_wk )
{ 
  { 
    int i;
    for( i = 0; i < p_wk->idx_max; i++ )
    { 
      BUTTERFLY_Exit( &p_wk->wk[i] );
    }
  }

  GFL_STD_MemClear( p_wk, sizeof(BUTTERFLY_SYS) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  蝶動きシステム  メイン処理
 *
 *	@param	BUTTERFLY_SYS *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BUTTERFLY_SYS_Main( BUTTERFLY_SYS *p_wk )
{ 
  { 
    int i;
    for( i = 0; i < p_wk->idx; i++ )
    { 
      BUTTERFLY_Main( &p_wk->wk[i] );
    }
  }

  //徐々に蝶が増えていく処理
  if( p_wk->cnt++> 0 )
  { 
    p_wk->cnt = 0;
    if( p_wk->idx < p_wk->idx_max )
    { 
      p_wk->idx++;
    }
  }
}

//=============================================================================
/**
 *				汎用
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	矩形内にTrgしたかどうか
 *
 *	@param	const GFL_RECT *cp_rect	矩形
 *	@param	*p_trg									座標受け取り
 *
 *	@retval	TRUEタッチした
 *	@retval	FALSEタッチしていない
 */
//-----------------------------------------------------------------------------
static BOOL TP_GetRectTrg( const BUTTON_SETUP *cp_btn )
{	
	u32 x, y;

	//Cont中で、矩形内のとき
	if( GFL_UI_TP_GetPointTrg( &x, &y ) )
	{	
		if( ((u32)( x - cp_btn->x*8) < (u32)(cp_btn->w*8))
				&	((u32)( y - cp_btn->y*8) < (u32)(cp_btn->h*8))
			)
		{
			return TRUE;
		}
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動初期化
 *
 *	@param	PROGVAL_VELOCITY_WORK* p_wk	ワーク
 *	@param	start						開始座標
 *	@param	end							終了座標
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_VEL_Init( PROGVAL_VELOCITY_WORK* p_wk, int start, int end, int sync )
{
	p_wk->now_val	= start;
	p_wk->start_val	= start;
	p_wk->end_val	= end;
	p_wk->sync_max	= sync;
	if( sync ) {
		p_wk->add_val	= FX32_CONST(p_wk->end_val - p_wk->start_val) / sync;
		p_wk->sync_now	= 0;
	}else{
		//	sync == 0 の場合は即処理終了
		p_wk->sync_now	= p_wk->sync_max-2;	//(p_wk->sync_max-1)より下の値
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動メイン処理（毎フレームよぶこと）
 *
 *	@param	PROGVAL_VELOCITY_WORK* p_wk ワーク
 *
 *	@return	TRUEなら処理終了、FALSEなら処理中。
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_VEL_Main( PROGVAL_VELOCITY_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		p_wk->sync_now++;
		p_wk->now_val	= p_wk->start_val + ((p_wk->add_val * (p_wk->sync_now)) >> FX32_SHIFT);
		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_val;
		return TRUE;
	}
}

//=============================================================================
/**
 *			３次曲線　CatmullRom曲線版
 */
//=============================================================================
//----------------------------------------------------------------------------
/*
 *	@brief	３次曲線CatmullRom曲線版	初期化
 *
 *	@param	p_wk						ワーク
 *	@param	start_pos					開始座標
 *	@param	ctrl_pos0					制御点１
 *	@param	ctrl_pos1					制御点２
 *	@param	end_pos						終了座標
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void PROGVAL_CATMULLROM_Init( PROGVAL_CATMULLROM_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_ctrl_pos0, const VecFx32 *cp_ctrl_pos1, const VecFx32 *cp_end_pos, int sync )
{
/*	GF_ASSERT( cp_start_pos != NULL
			&& cp_ctrl_pos0 != NULL
			&& cp_ctrl_pos1 != NULL 
			&& cp_end_pos != NULL );
*/
	p_wk->now_val	= *cp_start_pos;
	p_wk->start_pos	= *cp_start_pos;
	p_wk->ctrl_pos0	= *cp_ctrl_pos0;
	p_wk->ctrl_pos1	= *cp_ctrl_pos1;
	p_wk->end_pos	= *cp_end_pos;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３次曲線CatmullRom曲線版	メイン処理化
 *
 *	@param	p_wk	ワーク
 *
 *	@returnTRUEなら処理終了、FALSEなら処理中
 */
//-----------------------------------------------------------------------------
static BOOL PROGVAL_CATMULLROM_Main( PROGVAL_CATMULLROM_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		int	sync1div3	= (p_wk->sync_max-1) * 1 / 3;
		int sync2div3	= (p_wk->sync_max-1) * 2 / 3;
		fx32 t1, t2, t3;
		fx32 mp0, mp1, mp2, mp3;

		//	始点からP0までの曲線
		if( p_wk->sync_now < sync1div3 ) {
			t1	= FX_Div( FX32_CONST(p_wk->sync_now), FX32_CONST(sync1div3));
			t2	= FX_Mul( t1, t1 );	//	t*t
			t3	= FX_Mul( t2, t1 );	//	t*t*t

			mp0	= 0;
			mp1	=(   t2 - 3*t1 + 2*FX32_ONE) / 2;
			mp2	=(-2*t2 + 4*t1) / 2;
			mp3	=(   t2 -   t1) / 2;

			p_wk->now_val.x	= FX_Mul(p_wk->start_pos.x, mp1) + FX_Mul(p_wk->ctrl_pos0.x, mp2)
				+ FX_Mul(p_wk->ctrl_pos1.x, mp3);
			p_wk->now_val.y	= FX_Mul(p_wk->start_pos.y, mp1) + FX_Mul(p_wk->ctrl_pos0.y, mp2)
				+ FX_Mul(p_wk->ctrl_pos1.y, mp3);

		//	P0からP1までの曲線
		}else if( p_wk->sync_now < sync2div3 ) {
			t1	= FX_Div( FX32_CONST(p_wk->sync_now-sync1div3), FX32_CONST(sync2div3-sync1div3));
			t2	= FX_Mul( t1, t1 );	//	t*t
			t3	= FX_Mul( t2, t1 );	//	t*t*t

			mp0	=(  -t3 + 2*t2 - t1) / 2;
			mp1	=( 3*t3 - 5*t2 + 2*FX32_ONE) / 2;
			mp2	=(-3*t3 + 4*t2 + t1) / 2;
			mp3	=(   t3 -   t2 ) / 2;

			p_wk->now_val.x	= FX_Mul(p_wk->start_pos.x, mp0) + FX_Mul(p_wk->ctrl_pos0.x, mp1)
				+ FX_Mul(p_wk->ctrl_pos1.x, mp2) + FX_Mul(p_wk->end_pos.x,   mp3);
			p_wk->now_val.y	= FX_Mul(p_wk->start_pos.y, mp0) + FX_Mul(p_wk->ctrl_pos0.y, mp1)
				+ FX_Mul(p_wk->ctrl_pos1.y, mp2) + FX_Mul(p_wk->end_pos.y,   mp3);

		//	P1から終点までの曲線
		}else if( sync2div3 <= p_wk->sync_now ) {
			t1	= FX_Div( FX32_CONST(p_wk->sync_now-sync2div3), FX32_CONST(p_wk->sync_max-sync2div3));
			t2	= FX_Mul( t1, t1 );	//	t*t
			t3	= FX_Mul( t2, t1 );	//	t*t*t

			mp0	=(   t2 - t1) /2;
			mp1	=(-2*t2		   + 2*FX32_ONE) / 2;
			mp2	=(   t2 + t1) / 2;
			mp3	=0;	

			p_wk->now_val.x	= FX_Mul(p_wk->ctrl_pos0.x, mp0)
				+ FX_Mul(p_wk->ctrl_pos1.x, mp1) + FX_Mul(p_wk->end_pos.x, mp2);
			p_wk->now_val.y	= FX_Mul(p_wk->ctrl_pos0.y, mp0)
				+ FX_Mul(p_wk->ctrl_pos1.y, mp1) + FX_Mul(p_wk->end_pos.y, mp2);
			p_wk->now_val.z	= FX_Mul(p_wk->ctrl_pos0.z, mp0)
				+ FX_Mul(p_wk->ctrl_pos1.z, mp1) + FX_Mul(p_wk->end_pos.z, mp2);
		}

		p_wk->sync_now++;

		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_pos;
		return TRUE;
	}

	return TRUE;
}

