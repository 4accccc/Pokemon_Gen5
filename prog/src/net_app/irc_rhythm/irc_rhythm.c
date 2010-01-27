//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_rhythm_main.c
 *	@brief	赤外線ミニゲーム	リズムチェック
 *	@author	Toru=Nagihashi
 *	@data		2009.05.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	lib
#include <gflib.h>

//	constant
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"

//	module
#include "infowin/infowin.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/bmp_winframe.h"

#include "net_app/irc_appbar.h"


//	archive
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_irc_rhythm.h"
#include "irccompatible_gra.naix"

//	rhythm
#include "net_app/irc_rhythm.h"
// proc
#include "net_app/irc_result.h"

//sound
#include "../irc_compatible/irc_compatible_snd.h"

#ifdef PM_DEBUG
//debug用
#include "system/net_err.h"	//VRAM退避用アドレスを貰うため
#include <wchar.h>					//wcslen
#endif

FS_EXTERN_OVERLAY(irc_result);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	デバッグ用定義
//=====================================
#ifdef PM_DEBUG
#endif //PM_DEBUG

#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
static BOOL s_is_debug_only_play	= 0;
#endif

//-------------------------------------
///	点数
//=====================================
//間隔の時間による得点
#define VAL_RHYTHMSCORE_PROC_00	(100)
#define VAL_RHYTHMSCORE_PROC_01	(90)
#define VAL_RHYTHMSCORE_PROC_02	(80)
#define VAL_RHYTHMSCORE_PROC_03	(70)
#define VAL_RHYTHMSCORE_PROC_04	(60)
#define VAL_RHYTHMSCORE_PROC_05	(50)
#define VAL_RHYTHMSCORE_PROC_06	(40)
#define VAL_RHYTHMSCORE_PROC_07	(30)
#define VAL_RHYTHMSCORE_PROC_08	(20)
#define VAL_RHYTHMSCORE_PROC_09	(10)
#define VAL_RHYTHMSCORE_PROC_10	(0)

#define RANGE_RHYTHMSCORE_PROC_00	(0)
#define RANGE_RHYTHMSCORE_PROC_01	(500)
#define RANGE_RHYTHMSCORE_PROC_02	(1000)
#define RANGE_RHYTHMSCORE_PROC_03	(3000)
#define RANGE_RHYTHMSCORE_PROC_04	(5000)
#define RANGE_RHYTHMSCORE_PROC_05	(10000)
#define RANGE_RHYTHMSCORE_PROC_06	(15000)
#define RANGE_RHYTHMSCORE_PROC_07	(20000)
#define RANGE_RHYTHMSCORE_PROC_08	(25000)
#define RANGE_RHYTHMSCORE_PROC_09	(30000)

//間隔の時間による得点
#define VAL_RHYTHMSCORE_DIFF_00	(100)
#define VAL_RHYTHMSCORE_DIFF_01	(90)
#define VAL_RHYTHMSCORE_DIFF_02	(80)
#define VAL_RHYTHMSCORE_DIFF_03	(70)
#define VAL_RHYTHMSCORE_DIFF_04	(60)
#define VAL_RHYTHMSCORE_DIFF_05	(50)
#define VAL_RHYTHMSCORE_DIFF_06	(40)
#define VAL_RHYTHMSCORE_DIFF_07	(30)
#define VAL_RHYTHMSCORE_DIFF_08	(20)
#define VAL_RHYTHMSCORE_DIFF_09	(10)
#define VAL_RHYTHMSCORE_DIFF_10	(0)

#define RANGE_RHYTHMSCORE_DIFF_00	(0)
#define RANGE_RHYTHMSCORE_DIFF_01	(50)
#define RANGE_RHYTHMSCORE_DIFF_02	(100)
#define RANGE_RHYTHMSCORE_DIFF_03	(200)
#define RANGE_RHYTHMSCORE_DIFF_04	(300)
#define RANGE_RHYTHMSCORE_DIFF_05	(400)
#define RANGE_RHYTHMSCORE_DIFF_06	(500)
#define RANGE_RHYTHMSCORE_DIFF_07	(600)
#define RANGE_RHYTHMSCORE_DIFF_08	(1000)
#define RANGE_RHYTHMSCORE_DIFF_09	(15000)


//-------------------------------------
///	パレット
//=====================================
enum{	
	// メイン画面BG
	RHYTHM_BG_PAL_M_00 = 0,//背景用B
	RHYTHM_BG_PAL_M_01,		// 
	RHYTHM_BG_PAL_M_02,		// 
	RHYTHM_BG_PAL_M_03,		// 
	RHYTHM_BG_PAL_M_04,		// 
	RHYTHM_BG_PAL_M_05,		// INFOWIN
	RHYTHM_BG_PAL_M_06,		// 背景用ここまで
	RHYTHM_BG_PAL_M_07,		// 使用してない
	RHYTHM_BG_PAL_M_08,		// 使用してない
	RHYTHM_BG_PAL_M_09,		// 使用してない
	RHYTHM_BG_PAL_M_10,		// 使用してない
	RHYTHM_BG_PAL_M_11,		// 使用してない
	RHYTHM_BG_PAL_M_12,		// 使用してない
	RHYTHM_BG_PAL_M_13,		// 使用してない
	RHYTHM_BG_PAL_M_14,		// APPBAR
	RHYTHM_BG_PAL_M_15,		// APPBAR


	// サブ画面BG
	RHYTHM_BG_PAL_S_00 = 0,	//背景用
	RHYTHM_BG_PAL_S_01,		//
	RHYTHM_BG_PAL_S_02,		//
	RHYTHM_BG_PAL_S_03,		//
	RHYTHM_BG_PAL_S_04,		//
	RHYTHM_BG_PAL_S_05,		//
	RHYTHM_BG_PAL_S_06,		// 背景用			//上画面用バー＆フォント
	RHYTHM_BG_PAL_S_07,		// 背景
	RHYTHM_BG_PAL_S_08,		// 使用してない
	RHYTHM_BG_PAL_S_09,		// 使用してない
	RHYTHM_BG_PAL_S_10,		// 使用してない
	RHYTHM_BG_PAL_S_11,		// 使用してない
	RHYTHM_BG_PAL_S_12,		// 使用してない
	RHYTHM_BG_PAL_S_13,		// 使用してない
	RHYTHM_BG_PAL_S_14,		// 使用してない
	RHYTHM_BG_PAL_S_15,		// 使用してない

	// メイン画面OBJ
	RHYTHM_OBJ_PAL_M_00 = 0,// OBJ用
	RHYTHM_OBJ_PAL_M_01,		// 
	RHYTHM_OBJ_PAL_M_02,		// 
	RHYTHM_OBJ_PAL_M_03,		// 
	RHYTHM_OBJ_PAL_M_04,		// 
	RHYTHM_OBJ_PAL_M_05,		// 
	RHYTHM_OBJ_PAL_M_06,		// ここまでタッチOBJ
	RHYTHM_OBJ_PAL_M_07,		// 数字用
	RHYTHM_OBJ_PAL_M_08,		// 
	RHYTHM_OBJ_PAL_M_09,		// 
	RHYTHM_OBJ_PAL_M_10,		// OBJ用
	RHYTHM_OBJ_PAL_M_11,		// 使用してない
	RHYTHM_OBJ_PAL_M_12,		// 使用してない
	RHYTHM_OBJ_PAL_M_13,		// APPBAR
	RHYTHM_OBJ_PAL_M_14,		// APPBAR
	RHYTHM_OBJ_PAL_M_15,		// APPBAR


	// サブ画面OBJ
	RHYTHM_OBJ_PAL_S_00 = 0,	//使用してない
	RHYTHM_OBJ_PAL_S_01,		// 使用してない
	RHYTHM_OBJ_PAL_S_02,		// 使用してない
	RHYTHM_OBJ_PAL_S_03,		// 使用してない
	RHYTHM_OBJ_PAL_S_04,		// 使用してない
	RHYTHM_OBJ_PAL_S_05,		// 使用してない
	RHYTHM_OBJ_PAL_S_06,		// 使用してない
	RHYTHM_OBJ_PAL_S_07,		// 使用してない
	RHYTHM_OBJ_PAL_S_08,		// 使用してない
	RHYTHM_OBJ_PAL_S_09,		// 使用してない
	RHYTHM_OBJ_PAL_S_10,		// 使用してない
	RHYTHM_OBJ_PAL_S_11,		// 使用してない
	RHYTHM_OBJ_PAL_S_12,		// 使用してない
	RHYTHM_OBJ_PAL_S_13,		// 使用してない
	RHYTHM_OBJ_PAL_S_14,		// 使用してない
	RHYTHM_OBJ_PAL_S_15,		// 使用してない
};

//-------------------------------------
///	情報バー
//=====================================
#define INFOWIN_PLT_NO		(RHYTHM_BG_PAL_M_05)
#define INFOWIN_BG_FRAME	(GFL_BG_FRAME0_M)

//-------------------------------------
///	文字
//=====================================
#define TEXTSTR_PLT_NO				(RHYTHM_BG_PAL_S_14)
#define TEXTSTR_BUFFER_LENGTH	(255)

//-------------------------------------
///	位置
//=====================================
#define	MSGWND_TEXT_X	(1)
#define	MSGWND_TEXT_Y	(18)
#define	MSGWND_TEXT_W	(30)
#define	MSGWND_TEXT_H	(5)

#define	MSGWND_TITLE_X	(6)
#define	MSGWND_TITLE_Y	(3)
#define	MSGWND_TITLE_W	(20)
#define	MSGWND_TITLE_H	(5)

//-------------------------------------
///	カウント
//=====================================
#define TOUCHMARKER_VISIBLE_CNT	(10)
#define RESULT_SEND_CNT	(COMPATIBLE_IRC_SENDATA_CNT)

//-------------------------------------
///	計測情報
//=====================================
#define RHYTHMSEARCH_DATA_MAX	(15)	//計測回数最大
#define TOUCH_DIAMOND_W	(48)				//タッチ幅
#define TOUCH_DIAMOND_H	(48)

//-------------------------------------
///		MSG_FONT
//=====================================
typedef enum {	
	MSG_FONT_TYPE_LARGE,
	MSG_FONT_TYPE_SMALL,
}MSG_FONT_TYPE;

//-------------------------------------
///	OBJ登録ID
//=====================================
enum {
	OBJREGID_TOUCH_PLT_M,
	OBJREGID_TOUCH_CHR_M,
	OBJREGID_TOUCH_CEL_M,
	OBJREGID_TOUCH_PLT_S,
	OBJREGID_TOUCH_CHR_S,
	OBJREGID_TOUCH_CEL_S,

  OBJREGID_NUM_PLT_M,
  OBJREGID_NUM_CHR_M,
  OBJREGID_NUM_CEL_M,

	OBJREGID_MAX
};

//-------------------------------------
///		BACKOBJ
//=====================================
//後ろOBJの数
#define BACKOBJ_CLWK_MAX	(30)
//放出する数
#define BACKOBJ_EMIT_NUM	(12)
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
	BACKOBJ_COLOR_RED			= 0,
	BACKOBJ_COLOR_ORANGE	= 1,
	BACKOBJ_COLOR_YELLOW	= 2,
	BACKOBJ_COLOR_YEGREEN	= 3,
	BACKOBJ_COLOR_GREEN		= 4,
	BACKOBJ_COLOR_WATER		= 5,
	BACKOBJ_COLOR_BLUE		= 6,
} BACKOBJ_COLOR;

//１つのOBJが動くまでのシンク
#define BACKOBJ_MOVE_SYNC	(10)

#define BACKOBJ_ONE_MOVE_SYNC_MIN	(110)
#define BACKOBJ_ONE_MOVE_SYNC_MAX	(160)
#define BACKOBJ_ONE_MOVE_SYNC_DIF	(BACKOBJ_ONE_MOVE_SYNC_MAX-BACKOBJ_ONE_MOVE_SYNC_MIN)

#define BACKOBJ_ONE_EMIT_SYNC_MIN	(40)
#define BACKOBJ_ONE_EMIT_SYNC_MAX	(45)
#define BACKOBJ_ONE_EMIT_SYNC_DIF	(BACKOBJ_ONE_EMIT_SYNC_MAX-BACKOBJ_ONE_EMIT_SYNC_MIN)

enum
{
	BACKOBJ_SYS_MAIN,
	BACKOBJ_SYS_NUM,
} ;

//-------------------------------------
///	カうんどダウンの定数
//=====================================
#define COUNTDOWN_NUM_OBJ_MAX   (3)
#define COUNTDOWN_ONE_COUNT_MAX (60)

//-------------------------------------
///	CLWK取得
//=====================================
typedef enum
{	
	
	CLWKID_BACKOBJ_TOP_M,
	CLWKID_BACKOBJ_END_M = CLWKID_BACKOBJ_TOP_M + BACKOBJ_CLWK_MAX,

  CLWKID_NUM_TOP_M,
  CLWKID_NUM_END_M  = CLWKID_NUM_TOP_M + COUNTDOWN_NUM_OBJ_MAX,

	CLWKID_MAX
}CLWKID;

//-------------------------------------
///	MSGWNDID
//=====================================
typedef enum {
	MSGWNDID_TEXT,

	MSGWNDID_MAX
} MSGWNDID;

//-------------------------------------
///	個人結果画面の定数
//=====================================
enum
{	
	ROR_MSGWNDID_TITLE,
	ROR_MSGWNDID_MSG,
	ROR_MSGWNDID_DEBUG,

	ROR_MSGWNDID_MAX,
};

#define ROR_MSGWND_TITLE_X	(2)
#define ROR_MSGWND_TITLE_Y	(3)
#define ROR_MSGWND_TITLE_W	(20)
#define ROR_MSGWND_TITLE_H	(2)
#define ROR_MSGWND_MSG_X		(6)
#define ROR_MSGWND_MSG_Y		(9)
#define ROR_MSGWND_MSG_W		(20)
#define ROR_MSGWND_MSG_H		(6)
#define ROR_MSGWND_DEBUG_X	(2)
#define ROR_MSGWND_DEBUG_Y	(16)
#define ROR_MSGWND_DEBUG_W	(28)
#define ROR_MSGWND_DEBUG_H	(4)

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
	GFL_ARCUTIL_TRANSINFO	frame_char;		//上画面用メッセージフレーム
} GRAPHIC_BG_WORK;

//-------------------------------------
///	OBJ関係
//=====================================
typedef struct {
	GFL_CLUNIT *p_clunit;
	u32				reg_id[OBJREGID_MAX];
	GFL_CLWK	 *p_clwk[CLWKID_MAX];
} GRAPHIC_OBJ_WORK;

//-------------------------------------
///	描画関係
//=====================================
typedef struct 
{
	GRAPHIC_BG_WORK		bg;
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

//-------------------------------------
///	サーチ部
//=====================================
typedef struct
{
	//どこを押したか
//	GFL_POINT		pos;
	//押した時間
	u32					prog_ms;	//経過時間
	s32					diff_ms;	//差の時間
} RHYTHMSEARCH_DATA;
typedef struct
{	
	RHYTHMSEARCH_DATA	data[RHYTHMSEARCH_DATA_MAX];
	u32			data_idx;
	OSTick	start_time;
  u32     minus;
} RHYTHMSEARCH_WORK;

//-------------------------------------
///	RHYTHM用ネット
//=====================================
//ネットメイン
typedef struct {
	COMPATIBLE_IRC_SYS	*p_irc;
	u32 seq;
	RHYTHMSEARCH_WORK	result_recv;
	RHYTHMSEARCH_WORK	result_recv_my;
	RHYTHMSEARCH_WORK	result_send;
	BOOL is_recv;
} RHYTHMNET_WORK;

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
	u32								sync_now;	//１つのワークを開始するまでのシンク
	u32								sync_max;
} BACKOBJ_WORK;
#if 0
//-------------------------------------
///	個人成績画面
//=====================================
typedef struct 
{
	u32	seq;
	const MSG_WORK *cp_msg;
	const RHYTHMSEARCH_WORK *cp_search;
	MSGWND_WORK			msgwnd[ROR_MSGWNDID_MAX];
} RHYTHM_ONLYRESULT_WORK;
#endif
//-------------------------------------
///	カウント
//=====================================
typedef struct 
{
	GFL_CLWK					*p_clwk[COUNTDOWN_NUM_OBJ_MAX];
  u32               cnt;
  u32               cnt_idx;  //何回カウントしたか
  BOOL              is_start;
  BOOL              is_end;
} COUNTDOWN_WORK;

#ifdef DEBUG_RHYTHM_MSG
//-------------------------------------
///	デバッグプリント用画面
//=====================================
typedef struct
{
	GFL_BMP_DATA *p_bmp;
	GFL_FONT*			p_font;

	BOOL	is_now_save;
	u8	frm;

  u8  *p_char_temp_area;      ///<キャラクタVRAM退避先
  u16 *p_scrn_temp_area;      ///<スクリーンVRAM退避先
  u16 *p_pltt_temp_area;      ///<パレットVRAM退避先

 	u8  font_col_bkup[3];
  u8  prioryty_bkup;
  u8  scroll_x_bkup;
  u8  scroll_y_bkup;

	HEAPID heapID;
} DEBUG_PRINT_WORK;

#define DEBUGPRINT_CHAR_TEMP_AREA (0x4000)
#define DEBUGPRINT_SCRN_TEMP_AREA (0x800)
#define DEBUGPRINT_PLTT_TEMP_AREA (0x20)
#define DEBUGPRINT_WIDTH  (32)
#define DEBUGPRINT_HEIGHT (18)

static DEBUG_PRINT_WORK *sp_dp_wk;
#endif //DEBUG_RHYTHM_MSG

//-------------------------------------
///	リズムチェックメインワーク
//=====================================
typedef struct _RHYTHM_MAIN_WORK RHYTHM_MAIN_WORK;
typedef void (*SEQ_FUNCTION)( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq );
struct _RHYTHM_MAIN_WORK
{
	//グラフィックモジュール
	GRAPHIC_WORK		grp;
	MSG_WORK				msg;
	MSGWND_WORK			msgwnd[MSGWNDID_MAX];

	//ネット
	RHYTHMNET_WORK	net;

	//結果表示
	//RHYTHM_ONLYRESULT_WORK	onlyresult;

	//背面ぴかぴか
	BACKOBJ_WORK		backobj[BACKOBJ_SYS_NUM];

	//下画面バー
	APPBAR_WORK			*p_appbar;

  //カウントダウン
  COUNTDOWN_WORK  cntdown;

	MSGWND_WORK			msgtitle;	//タイトルメッセージ

	//シーケンス管理
	SEQ_FUNCTION		seq_function;
	u16		seq;
	u16		cnt;
	BOOL is_end;
	BOOL is_next_proc;

	//計測
	RHYTHMSEARCH_WORK	search;

	//その他
	u32								marker_cnt;	//マーカー表示時間

  u32   minus;

	//引数
	IRC_RHYTHM_PARAM	*p_param;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT IRC_RHYTHM_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_RHYTHM_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_RHYTHM_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
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
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
//obj
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID );
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk );
static GFL_CLWK* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, CLWKID id );
static GFL_CLUNIT * GRAPHIC_OBJ_GetClunit( const GRAPHIC_OBJ_WORK *cp_wk );
//MSG_WORK
static void MSG_Init( MSG_WORK *p_wk, MSG_FONT_TYPE font, HEAPID heapID );
static void MSG_Exit( MSG_WORK *p_wk );
static BOOL MSG_Main( MSG_WORK *p_wk );
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk );
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk );
static const GFL_MSGDATA * MSG_GetMsgDataConst( const MSG_WORK *cp_wk );
static WORDSET * MSG_GetWordSet( const MSG_WORK *cp_wk );
//MSG_WINDOW
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, u8 x, u8 y, u8 w, u8 h, u8 plt, HEAPID heapID );
static void MSGWND_InitEx( MSGWND_WORK* p_wk, u8 bgframe,
		u8 x, u8 y, u8 w, u8 h, u8 plt, u16 clear_chr, u8 dir, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, const MSG_WORK *cp_msg );
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y );
static void MSGWND_PrintCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID );
static void MSGWND_PrintNumber( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id, u16 x, u16 y );
static void MSGWND_PrintPlayerName( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, const COMPATIBLE_STATUS *cp_status, u16 x, u16 y, HEAPID heapID );
static void MSGWND_Clear( MSGWND_WORK* p_wk );
//ONLYRESULT
#if 0
static void RHYTHM_ONLYRESULT_Init( RHYTHM_ONLYRESULT_WORK* p_wk, u8 frm, const MSG_WORK *cp_msg, const RHYTHMSEARCH_WORK *cp_search,  HEAPID heapID );
static void RHYTHM_ONLYRESULT_Exit( RHYTHM_ONLYRESULT_WORK* p_wk );
static BOOL RHYTHM_ONLYRESULT_Main( RHYTHM_ONLYRESULT_WORK* p_wk );
#endif
static void COUNTDOWN_Init( COUNTDOWN_WORK *p_wk, const GRAPHIC_WORK *cp_grp, HEAPID heapID );
static void COUNTDOWN_Exit( COUNTDOWN_WORK *p_wk );
static void COUNTDOWN_Main( COUNTDOWN_WORK *p_wk );
static void COUNTDOWN_Start( COUNTDOWN_WORK *p_wk );
static BOOL COUNTDOWN_IsEnd( const COUNTDOWN_WORK *cp_wk );
//SEQ
static void SEQ_Change( RHYTHM_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function );
static void SEQ_End( RHYTHM_MAIN_WORK *p_wk );
//SEQ_FUNC
static void SEQFUNC_StartGame( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_CountDown( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_MainGame( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_Result( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_SceneError( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq );
//RHYTHMSEARCH
static void RHYTHMSEARCH_Init( RHYTHMSEARCH_WORK *p_wk );
static void RHYTHMSEARCH_Exit( RHYTHMSEARCH_WORK *p_wk );
static void RHYTHMSEARCH_Start( RHYTHMSEARCH_WORK *p_wk );
static BOOL RHYTHMSEARCH_IsEnd( const RHYTHMSEARCH_WORK *cp_wk );
static void RHYTHMSEARCH_SetData( RHYTHMSEARCH_WORK *p_wk, const GFL_POINT *cp_pos );
static BACKOBJ_COLOR RHYTHMSEARCH_PlaySEByRhythmSpeed( const RHYTHMSEARCH_WORK *cp_wk );
//net
static void RHYTHMNET_Init( RHYTHMNET_WORK *p_wk, COMPATIBLE_IRC_SYS *p_irc );
static void RHYTHMNET_Exit( RHYTHMNET_WORK *p_wk );
static BOOL RHYTHMNET_SendResultData( RHYTHMNET_WORK *p_wk, const RHYTHMSEARCH_WORK *cp_data );
static void RHYTHMNET_GetResultData( const RHYTHMNET_WORK *cp_wk, RHYTHMSEARCH_WORK *p_data );
//net_recv
static void NETRECV_Result( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
static u8* NETRECV_GetBufferAddr(int netID, void* pWork, int size);
//汎用
static BOOL TP_GetDiamondTrg( const GFL_POINT *cp_diamond, GFL_POINT *p_trg );
static u8		CalcScore( const RHYTHM_MAIN_WORK *cp_wk );
static u8		CalcMinus( const RHYTHM_MAIN_WORK *cp_wk );
//BACKOBJ
static void BACKOBJ_Init( BACKOBJ_WORK *p_wk, const GRAPHIC_WORK *cp_grp, BACKOBJ_MOVE_TYPE type, u32 clwk_ofs, int sf_type );
static void BACKOBJ_Exit( BACKOBJ_WORK *p_wk );
static void BACKOBJ_Main( BACKOBJ_WORK *p_wk );
static void BACKOBJ_StartEmit( BACKOBJ_WORK *p_wk, const GFL_POINT *cp_pos, BACKOBJ_COLOR color );
//BACKOBJ_ONE
static void BACKOBJ_ONE_Init( BACKOBJ_ONE *p_wk, GFL_CLWK *p_clwk, int sf_type );
static void BACKOBJ_ONE_Main( BACKOBJ_ONE *p_wk );
static void BACKOBJ_ONE_Start( BACKOBJ_ONE *p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, BACKOBJ_COLOR color, u32 sync );
static BOOL BACKOBJ_ONE_IsMove( const BACKOBJ_ONE *cp_wk );

#ifdef DEBUG_RHYTHM_MSG
static void DEBUGRHYTHM_PRINT_UpDate( RHYTHM_MAIN_WORK *p_wk );
#else
#define DEBUGRHYTHM_PRINT_UpDate(...)	((void)0)
#endif

//DEBUG_PRINT
#ifdef DEBUG_RHYTHM_MSG
static void DEBUGPRINT_Init( u8 frm, BOOL is_now_save, HEAPID heapID );
static void DEBUGPRINT_Exit( void );
static void DEBUGPRINT_Open( void );
static void DEBUGPRINT_Close( void );
static void DEBUGPRINT_Print( const u16 *cp_str, u16 x, u16 y );
static void DEBUGPRINT_PrintNumber( const u16 *cp_str, int number, u16 x, u16 y );
static void DEBUGPRINT_Clear( void );
#else
#define DEBUGPRINT_Init(...)				((void)0)
#define DEBUGPRINT_Exit(...)				((void)0)
#define DEBUGPRINT_Open(...)				((void)0)
#define DEBUGPRINT_Close(...)				((void)0)
#define DEBUGPRINT_Print(...)				((void)0)
#define DEBUGPRINT_PrintNumber(...) ((void)0)
#define DEBUGPRINT_Clear(...)				((void)0)
#endif //DEBUG_RHYTHM_MSG
static STRBUF * DEBUGPRINT_CreateWideChar( const u16 *cp_str, HEAPID heapID );
static STRBUF * DEBUGPRINT_CreateWideCharNumber( const u16 *cp_str, int number, HEAPID heapID );

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	リズムチェック用プロックデータ
//=====================================
const GFL_PROC_DATA IrcRhythm_ProcData	= 
{	
	IRC_RHYTHM_PROC_Init,
	IRC_RHYTHM_PROC_Main,
	IRC_RHYTHM_PROC_Exit,
};
//-------------------------------------
///	BGシステム
//=====================================
typedef enum 
{
	GRAPHIC_BG_FRAME_M_INFOWIN,
	GRAPHIC_BG_FRAME_M_BACK,
	GRAPHIC_BG_FRAME_S_ROGO,
	GRAPHIC_BG_FRAME_S_TEXT,
	GRAPHIC_BG_FRAME_S_BACK,
	GRAPHIC_BG_FRAME_S_TITLE,
	GRAPHIC_BG_FRAME_MAX
} GRAPHIC_BG_FRAME;
static const u32 sc_bgcnt_frame[ GRAPHIC_BG_FRAME_MAX ] = 
{
	INFOWIN_BG_FRAME, GFL_BG_FRAME1_M, GFL_BG_FRAME0_S, GFL_BG_FRAME1_S, GFL_BG_FRAME2_S,GFL_BG_FRAME3_S
};
static const u32 sc_bgcnt_mode[ GRAPHIC_BG_FRAME_MAX ] = 
{
	GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_TEXT, GFL_BG_MODE_AFFINE,
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
	// GRAPHIC_BG_FRAME_M_BACK
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_ROGO
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_TEXT
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_BACK
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_TITLE
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
		GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ256_128x128,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},
};

//-------------------------------------
///	タッチ範囲
//=====================================
static const GFL_POINT	sc_diamond_pos[]	=
{	
	{	
		31, 47
	},
	{	
		79, 47
	},
	{	
		127, 47
	},
	{	
		175, 47
	},
	{	
		223, 47
	},
	{	
		55, 71
	},
	{	
		103, 71
	},
	{	
		151, 71
	},
	{	
		199, 71
	},
	{	
		31, 95
	},
	{	
		79, 95
	},
	{	
		127, 95
	},
	{	
		175, 95
	},
	{	
		223, 95
	},
	{	
		55, 119
	},
	{	
		103, 119
	},
	{	
		151, 119
	},
	{	
		199, 119
	},
	{	
		31, 143
	},
	{	
		79, 143
	},
	{	
		127, 143
	},
	{	
		175, 143
	},
	{	
		223, 143
	},

};

//-------------------------------------
///	NET
//=====================================
enum
{	
	NETRECV_RESULT	= GFL_NET_CMD_IRCRHYTHM,
};
static const NetRecvFuncTable	sc_recv_tbl[]	=
{
	{	
		NETRECV_Result, NETRECV_GetBufferAddr
	}
};

//=============================================================================
/**
 *			PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	リズムチェック	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_RHYTHM_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	RHYTHM_MAIN_WORK	*p_wk;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCRHYTHM, 0x20000 );
	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(RHYTHM_MAIN_WORK), HEAPID_IRCRHYTHM );
	GFL_STD_MemClear( p_wk, sizeof(RHYTHM_MAIN_WORK) );
	p_wk->p_param	= p_param;

#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
	s_is_debug_only_play	= p_wk->p_param->is_only_play;
#endif

	//モジュール初期化
	GRAPHIC_Init( &p_wk->grp, HEAPID_IRCRHYTHM );
	MSG_Init( &p_wk->msg, MSG_FONT_TYPE_LARGE, HEAPID_IRCRHYTHM );
	{
		GAME_COMM_SYS_PTR comm	= NULL;
		if( p_wk->p_param->p_gamesys )
		{	
			comm	= GAMESYSTEM_GetGameCommSysPtr(p_wk->p_param->p_gamesys);
		}
		//INFOWIN_Init( INFOWIN_BG_FRAME, INFOWIN_PLT_NO, comm, HEAPID_IRCRHYTHM );
	}
	MSGWND_Init( &p_wk->msgwnd[MSGWNDID_TEXT], sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TEXT],
			MSGWND_TEXT_X, MSGWND_TEXT_Y, MSGWND_TEXT_W, MSGWND_TEXT_H, RHYTHM_BG_PAL_S_08, HEAPID_IRCRHYTHM );
	BmpWinFrame_Write( p_wk->msgwnd[MSGWNDID_TEXT].p_bmpwin, WINDOW_TRANS_ON, 
					GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->grp.bg.frame_char), RHYTHM_BG_PAL_S_06 );

  if( p_wk->p_param->p_gamesys )
  {	
    MSGWND_PrintPlayerName( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, 
        RHYTHM_STR_000, p_wk->p_param->p_you_status,  0, 0, HEAPID_IRCRHYTHM );
  }
  else
  {	
      MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, RHYTHM_STR_000, 0, 0 );
  }

  COUNTDOWN_Init( &p_wk->cntdown, &p_wk->grp, HEAPID_IRCRHYTHM );

	//タイトル文字列作成
	MSGWND_InitEx( &p_wk->msgtitle, sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE],
			MSGWND_TITLE_X, MSGWND_TITLE_Y, MSGWND_TITLE_W, MSGWND_TITLE_H, RHYTHM_BG_PAL_S_08, 1, GFL_BMP_CHRAREA_GET_B, HEAPID_IRCRHYTHM );
	MSGWND_PrintCenter( &p_wk->msgtitle, &p_wk->msg, RHYTHM_TITLE_000 );
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_SCALE_X_SET, TITLE_STR_SCALE_X );
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_SCALE_Y_SET, TITLE_STR_SCALE_Y );
	GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_CENTER_X_SET, (MSGWND_TITLE_X + MSGWND_TITLE_W/2)*8 );
	GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TITLE], GFL_BG_CENTER_Y_SET, (MSGWND_TITLE_Y + MSGWND_TITLE_H/2)*8 );



	RHYTHMNET_Init( &p_wk->net, p_wk->p_param->p_irc );

	RHYTHMSEARCH_Init( &p_wk->search );

	BACKOBJ_Init( &p_wk->backobj[BACKOBJ_SYS_MAIN], &p_wk->grp, BACKOBJ_MOVE_TYPE_EMITER, CLWKID_BACKOBJ_TOP_M, CLSYS_DEFREND_MAIN );

	{	
		GFL_CLUNIT	*p_unit	= GRAPHIC_GetClunit( &p_wk->grp );
		p_wk->p_appbar	= APPBAR_Init( APPBAR_OPTION_MASK_RETURN, p_unit, sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_INFOWIN], RHYTHM_BG_PAL_M_14, RHYTHM_OBJ_PAL_M_13, APP_COMMON_MAPPING_128K, MSG_GetFont(&p_wk->msg ), MSG_GetPrintQue(&p_wk->msg ), HEAPID_IRCRHYTHM );
	}


	//リズムシーンセット
	COMPATIBLE_IRC_SetScene( p_wk->p_param->p_irc, COMPATIBLE_SCENE_RHYTHM );

  //輝度変更
  G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 |
      GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_BD, -8 );

	SEQ_Change( p_wk, SEQFUNC_StartGame );
	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	リズムチェック	メインプロセス破棄処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_RHYTHM_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	RHYTHM_MAIN_WORK	*p_wk;

	p_wk	= p_work;

#if 0
	//次のプロック予約
	if( p_wk->is_next_proc )
	{	
		if( p_wk->p_param->p_gamesys == NULL )
		{	
			GFL_PROC_SysSetNextProc( FS_OVERLAY_ID(irc_result), &IrcResult_ProcData,p_wk->p_param );
		}
		else
		{	
			GAMESYSTEM_SetNextProc( p_wk->p_param->p_gamesys, FS_OVERLAY_ID(irc_result), &IrcResult_ProcData,p_wk->p_param );
		}
	}
#endif


	{		
		int i;
		for( i = 0; i < BACKOBJ_SYS_NUM; i++ )
		{	
			BACKOBJ_Exit( &p_wk->backobj[i] );
		}
	}

  G2_BlendNone();

	//APPBAR
	APPBAR_Exit( p_wk->p_appbar );	

	RHYTHMNET_Exit( &p_wk->net );

  COUNTDOWN_Exit( &p_wk->cntdown );

	MSGWND_Exit( &p_wk->msgtitle );
	//モジュール破棄
	{
		int i;
		for( i = 0; i < MSGWNDID_MAX; i++ )
		{	
			MSGWND_Exit( &p_wk->msgwnd[i] );
		}
	}
	//INFOWIN_Exit();
	GRAPHIC_Exit( &p_wk->grp );
	MSG_Exit( &p_wk->msg );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );
	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_IRCRHYTHM );
	
	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	リズムチェック	メインプロセスメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_RHYTHM_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
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

	RHYTHM_MAIN_WORK	*p_wk;
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
			*p_seq	= SEQ_MAIN;
		}
		break;

	case SEQ_MAIN:

		p_wk->seq_function( p_wk, &p_wk->seq );
		if( p_wk->is_end )
		{	
			*p_seq	= SEQ_FADEIN_START;
		}

#if 0
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
		{	
			SEQ_Change( p_wk, SEQFUNC_StartGame );
		}
#endif

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
		GF_ASSERT_MSG( 0, "IRC_RHYTHM_PROC_MainのSEQエラー %d", *p_seq );
	}

	//INFOWIN_Update();
	if( MSG_Main( &p_wk->msg ) )
	{
		int i;
		for( i = 0; i < MSGWNDID_MAX; i++ )
		{	
			MSGWND_Main( &p_wk->msgwnd[i], &p_wk->msg );
			MSGWND_Main( &p_wk->msgtitle, &p_wk->msg );
		}
	}

	GRAPHIC_Draw( &p_wk->grp );

	{		
		int i;
		BACKOBJ_Main( &p_wk->backobj[BACKOBJ_SYS_MAIN] );
	}

	//APPBAR
	APPBAR_Main( p_wk->p_appbar );

	//シーンを継続的に送る
	COMPATIBLE_IRC_SendSceneContinue( p_wk->p_param->p_irc );

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
		GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,						// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_NONE,				// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_128K,		
		GX_OBJVRAMMODE_CHAR_1D_128K,		
	};

	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );

	//VRAMクリアー
	GFL_DISP_ClearVRAM( 0 );

	// VRAMバンク設定
	GFL_DISP_SetBank( &sc_vramSetTable );

	// ディスプレイON
	GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
	GFL_DISP_SetDispOn();

	//表示
	GFL_DISP_GX_InitVisibleControl();

	//描画モジュール
	GRAPHIC_OBJ_Init( &p_wk->obj, &sc_vramSetTable, heapID );
	GRAPHIC_BG_Init( &p_wk->bg, heapID );

	//VBlankTask登録
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

	GRAPHIC_BG_Exit( &p_wk->bg );
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
 *	@param	id												CLWKのID
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
	GRAPHIC_BG_VBlankFunction( &p_wk->bg );
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
		ARCHANDLE *p_handle;

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_IRCCOMPATIBLE, heapID );
		//パレット
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_irccompatible_gra_aura_bg_NCLR,
				PALTYPE_MAIN_BG, RHYTHM_BG_PAL_M_00*0x20, 0, heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_irccompatible_gra_aura_bg_NCLR,
				PALTYPE_SUB_BG, RHYTHM_BG_PAL_S_00*0x20, 0, heapID );

		//キャラ
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_rhythm_bg_NCGR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BACK], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_aura_bg_NCGR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_BACK], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_irccompatible_gra_title_rogo_NCGR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_ROGO], 0, 0, FALSE, heapID );
	
		//スクリーン
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_rhythm_bg_NSCR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BACK], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_aura_bg_block_b_NSCR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_BACK], 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_irccompatible_gra_title_rhythm_NSCR,
				sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_ROGO], 0, 0, FALSE, heapID );

		//ワク
		GFL_BG_FillCharacter( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], 0, 1, 0 );
		p_wk->frame_char	= GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_irccompatible_gra_ue_frame_NCGR, sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], 0, FALSE, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}
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

	//リソース破棄
	{	
		GFL_BG_FreeCharacterArea(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT],
				GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_char),
				GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->frame_char));
		GFL_BG_FillCharacterRelease(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT],1,0);
	}

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
 *	@brief	BG描画	VBlack関数
 *
 *	@param	GRAPHIC_BG_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk )
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

		p_wk->reg_id[OBJREGID_TOUCH_PLT_M]	= GFL_CLGRP_PLTT_Register( p_handle, 
				NARC_irccompatible_gra_rhythm_aura_NCLR, CLSYS_DRAW_MAIN, RHYTHM_OBJ_PAL_M_00*0x20, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_CHR_M]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_irccompatible_gra_rhythm_aura_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_CEL_M]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_irccompatible_gra_rhythm_aura_NCER, NARC_irccompatible_gra_rhythm_aura_NANR, heapID );

    p_wk->reg_id[OBJREGID_NUM_PLT_M]  = GFL_CLGRP_PLTT_RegisterEx( p_handle, 
				NARC_irccompatible_gra_aura_obj_NCLR, CLSYS_DRAW_MAIN, RHYTHM_OBJ_PAL_M_07*0x20, 0xC,1, heapID );
    p_wk->reg_id[OBJREGID_NUM_CHR_M]  = GFL_CLGRP_CGR_Register( p_handle,
				NARC_irccompatible_gra_aura_obj_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );
    p_wk->reg_id[OBJREGID_NUM_CEL_M]  = GFL_CLGRP_CELLANIM_Register( p_handle,
        NARC_irccompatible_gra_aura_obj_NCER, NARC_irccompatible_gra_aura_obj_NANR, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_PLT_S]	= GFL_CLGRP_PLTT_Register( p_handle, 
				NARC_irccompatible_gra_rhythm_aura_NCLR, CLSYS_DRAW_SUB, RHYTHM_OBJ_PAL_S_00*0x20, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_CHR_S]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_irccompatible_gra_rhythm_aura_NCGR, FALSE, CLSYS_DRAW_SUB, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_CEL_S]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_irccompatible_gra_rhythm_aura_NCER, NARC_irccompatible_gra_rhythm_aura_NANR, heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK作成
	{	
		int i;
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

		for( i = CLWKID_BACKOBJ_TOP_M; i < CLWKID_BACKOBJ_END_M; i++  )
		{	
			p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit, 
					p_wk->reg_id[OBJREGID_TOUCH_CHR_M],
					p_wk->reg_id[OBJREGID_TOUCH_PLT_M],
					p_wk->reg_id[OBJREGID_TOUCH_CEL_M],
					&cldata,
					CLSYS_DEFREND_MAIN,
					heapID
					);
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
			GFL_CLACT_WK_SetBgPri( p_wk->p_clwk[i], 3 );
			GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[i], TRUE );
			GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i], 0 );
		}

    cldata.pos_x  = 128;
    cldata.pos_y  = 96;
    for( i = CLWKID_NUM_TOP_M; i < CLWKID_NUM_END_M; i++  )
		{	
      cldata.anmseq = 1+(i-CLWKID_NUM_TOP_M);
			p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit, 
					p_wk->reg_id[OBJREGID_NUM_CHR_M],
					p_wk->reg_id[OBJREGID_NUM_PLT_M],
					p_wk->reg_id[OBJREGID_NUM_CEL_M],
					&cldata,
					CLSYS_DEFREND_MAIN,
					heapID
					);
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk[i], 0, CLWK_PLTTOFFS_MODE_PLTT_TOP );
		}
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
	//CLWK破棄
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
		GFL_CLGRP_PLTT_Release( p_wk->reg_id[OBJREGID_TOUCH_PLT_M] );
		GFL_CLGRP_CGR_Release( p_wk->reg_id[OBJREGID_TOUCH_CHR_M] );
		GFL_CLGRP_CELLANIM_Release( p_wk->reg_id[OBJREGID_TOUCH_CEL_M] );
		GFL_CLGRP_PLTT_Release( p_wk->reg_id[OBJREGID_TOUCH_PLT_S] );
		GFL_CLGRP_CGR_Release( p_wk->reg_id[OBJREGID_TOUCH_CHR_S] );
		GFL_CLGRP_CELLANIM_Release( p_wk->reg_id[OBJREGID_TOUCH_CEL_S] );

    GFL_CLGRP_PLTT_Release( p_wk->reg_id[OBJREGID_NUM_PLT_M] );
		GFL_CLGRP_CGR_Release( p_wk->reg_id[OBJREGID_NUM_CHR_M] );
		GFL_CLGRP_CELLANIM_Release( p_wk->reg_id[OBJREGID_NUM_CEL_M] );
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
 *	@param	font						フォントタイプ
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSG_Init( MSG_WORK *p_wk, MSG_FONT_TYPE font, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );

	GFL_FONTSYS_Init();

	switch( font )
	{	
	case MSG_FONT_TYPE_LARGE:
		p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
				NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
		break;
	case MSG_FONT_TYPE_SMALL:
		p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
				NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
		break;
	default:
		GF_ASSERT_MSG( 0, "MSGFONT_ERRO %d", font );
	};

	p_wk->p_print_que = PRINTSYS_QUE_Create( heapID );

	p_wk->p_msg = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_irc_rhythm_dat, heapID );

	p_wk->p_wordset	= WORDSET_Create( heapID );

	{	
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
	}
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
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, u8 x, u8 y, u8 w, u8 h, u8 plt, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
	p_wk->clear_chr	= 0xF;
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( bgframe, x, y, w, h, plt, GFL_BMP_CHRAREA_GET_B );
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( TEXTSTR_BUFFER_LENGTH, heapID );
	PRINT_UTIL_Setup( &p_wk->print_util, p_wk->p_bmpwin );
	MSGWND_Clear( p_wk );
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
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, const MSG_WORK *cp_msg )
{	
	return PRINT_UTIL_Trans( &p_wk->print_util, cp_msg->p_print_que );
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
 *	@brief	メッセージ表示面の中央に文字を表示
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

	//表示
	PRINT_UTIL_PrintColor( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font,PRINTSYS_LSB_Make(0xf,0xe,4) );
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
 *	@brief	メッセージ表示面に数値つき文字を表示
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	MSG_WORK *cp_msg	文字管理
 *	@param	strID							文字ID
 *	@param	cp_status					プレイヤーの状態
 *	@param	x									開始位置X
 *	@param	y									開始位置Y
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintPlayerName( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, const COMPATIBLE_STATUS *cp_status, u16 x, u16 y, HEAPID heapID )
{
  const GFL_MSGDATA* cp_msgdata;
  WORDSET *p_wordset;

  //一端消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin),p_wk->clear_chr );

  //モジュール取得
  p_wordset   = MSG_GetWordSet( cp_msg );
  cp_msgdata  = MSG_GetMsgDataConst( cp_msg );

  //ワードセットに登録
  { 
    WORDSET_RegisterPlayerName( p_wordset, 0, (const MYSTATUS*)cp_status->my_status );
  }

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
 *	@brief	画面クリア
 *
 *	@param	MSGWND_WORK* p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Clear( MSGWND_WORK* p_wk )
{	
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), p_wk->clear_chr );	
	GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin );
}
//=============================================================================
/**
 *				個人結果画面
 */
//=============================================================================
#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	個人結果画面	初期化
 *
 *	@param	RHYTHM_ONLYRESULT_WORK* p_wk	ワーク
 *	@param	frm									使用フレーム
 *	@param	MSG_WORK *cp_wk			メッセージ
 *	@param	heapID							ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void RHYTHM_ONLYRESULT_Init( RHYTHM_ONLYRESULT_WORK* p_wk, u8 frm, const MSG_WORK *cp_msg, const RHYTHMSEARCH_WORK *cp_search,  HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(RHYTHM_ONLYRESULT_WORK) );
	p_wk->cp_msg		= cp_msg;
	p_wk->cp_search	= cp_search;

	MSGWND_Init( &p_wk->msgwnd[ROR_MSGWNDID_TITLE], frm, ROR_MSGWND_TITLE_X, ROR_MSGWND_TITLE_Y,
				ROR_MSGWND_TITLE_W, ROR_MSGWND_TITLE_H, heapID );
	MSGWND_Init( &p_wk->msgwnd[ROR_MSGWNDID_MSG], frm, ROR_MSGWND_MSG_X, ROR_MSGWND_MSG_Y,
				ROR_MSGWND_MSG_W, ROR_MSGWND_MSG_H, heapID );

	MSGWND_Init( &p_wk->msgwnd[ROR_MSGWNDID_DEBUG], frm, ROR_MSGWND_DEBUG_X, ROR_MSGWND_DEBUG_Y,
				ROR_MSGWND_DEBUG_W, ROR_MSGWND_DEBUG_H, heapID );

	MSGWND_Print( &p_wk->msgwnd[ROR_MSGWNDID_TITLE], cp_msg, RHYTHM_RES_000, 0, 0 );


	//文字列作成
	{
		u32 msg_idx;
		u32 ms_sum;
		int i;

		ms_sum	= 0;
		for( i = 1; i <10; i++ )
		{	
			ms_sum	+= cp_search->data[i].diff_ms;
		}
		ms_sum	/=	9;

		if( 40 <= ms_sum )
		{	
			msg_idx	= 0;
		}
		else if( 20 < ms_sum && ms_sum < 40 )
		{	
			msg_idx	= 1;
		}
		else if( ms_sum <= 20 )
		{	
			msg_idx	= 2;
		}
		else
		{	
			GF_ASSERT( 0 );
		}
		OS_Printf( "差の平均値 %d\n", ms_sum );

		MSGWND_Print( &p_wk->msgwnd[ROR_MSGWNDID_MSG], cp_msg, RHYTHM_RESMSG_000 + msg_idx, 0, 0 );
	}

	//デバッグ文字列作成
	{	
		MSGWND_WORK* p_msgwnd;
		PRINT_QUE*	p_que;
		GFL_FONT*		p_font;
		int i;
		u32 ms;
		u32 ms_sum;
		STRBUF *p_strbuf;

		p_msgwnd	= &p_wk->msgwnd[ROR_MSGWNDID_DEBUG];
		p_font	= GFL_FONT_Create( ARCID_FONT,
					NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );		
		ms_sum	= 0;
		for( i = 1; i < 10; i++ )
		{	
			ms	= cp_search->data[i].diff_ms;
			ms_sum	+= ms;

			p_strbuf	= DEBUGPRINT_CreateWideCharNumber( L"%d回目",i, heapID ); 
			PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), i*20, 0, p_strbuf, p_font );
			GFL_STR_DeleteBuffer( p_strbuf );

			p_strbuf	= DEBUGPRINT_CreateWideCharNumber( L" %d", ms, heapID ); 
			PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), i*20, 10, p_strbuf, p_font );
			GFL_STR_DeleteBuffer( p_strbuf );
		}

		p_strbuf	= DEBUGPRINT_CreateWideChar( L"平均", heapID ); 
		PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), i*20, 0, p_strbuf, p_font );
		GFL_STR_DeleteBuffer( p_strbuf );


		p_strbuf	= DEBUGPRINT_CreateWideCharNumber( L" %d", ms_sum/9, heapID ); 
		PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), i*20, 10, p_strbuf, p_font );
		GFL_STR_DeleteBuffer( p_strbuf );

		GFL_BMPWIN_MakeTransWindow( p_msgwnd->p_bmpwin );
		GFL_FONT_Delete( p_font );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	個人結果画面	破棄
 *
 *	@param	RHYTHM_ONLYRESULT_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void RHYTHM_ONLYRESULT_Exit( RHYTHM_ONLYRESULT_WORK* p_wk )
{	
	int i;
	for( i = 0; i < ROR_MSGWNDID_MAX; i++ )
	{	
		MSGWND_Exit( &p_wk->msgwnd[i] );
	}
	GFL_STD_MemClear( p_wk, sizeof(RHYTHM_ONLYRESULT_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	個人結果画面	メイン処理
 *
 *	@param	RHYTHM_ONLYRESULT_WORK* p_wk ワーク
 *
 *	@retval	TRUEならば終了
 *	@retval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL RHYTHM_ONLYRESULT_Main( RHYTHM_ONLYRESULT_WORK* p_wk )
{	

	{	
		int i;
		for( i = 0; i < MSGWNDID_MAX; i++ )
		{	
			MSGWND_Main( &p_wk->msgwnd[i], p_wk->cp_msg );
		}
	}

	if( GFL_UI_TP_GetTrg() )
	{	
		return TRUE;
	}

	return FALSE;
}
#endif
//=============================================================================
/**
 *    COUNTDOWN_WORK
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  カウントダウン  初期化
 *
 *	@param	COUNTDOWN_WORK *p_wk   ワーク
 *	@param	GRAPHIC_WORK *cp_grp  グラフィック
 *	@param  HEAPID heapID         ヒープID
 */
//-----------------------------------------------------------------------------
static void COUNTDOWN_Init( COUNTDOWN_WORK *p_wk, const GRAPHIC_WORK *cp_grp, HEAPID heapID )
{ 
  GFL_STD_MemClear( p_wk, sizeof(COUNTDOWN_WORK) );

  { 
    int i;
    for( i = 0; i < COUNTDOWN_NUM_OBJ_MAX; i++ )
    { 
      p_wk->p_clwk[0]  = GRAPHIC_GetClwk( cp_grp, CLWKID_NUM_TOP_M + i );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カウントダウン  破棄
 *
 *	@param	COUNTDOWN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void COUNTDOWN_Exit( COUNTDOWN_WORK *p_wk )
{ 
  GFL_STD_MemClear( p_wk, sizeof(COUNTDOWN_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  カウントダウン  メイン処理
 *
 *	@param	COUNTDOWN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void COUNTDOWN_Main( COUNTDOWN_WORK *p_wk )
{ 
  if( p_wk->is_start )
  { 
    if( p_wk->cnt == 0 )
    { 
      PMSND_PlaySE( SEQ_SE_SYS_43 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[0], TRUE );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[0], (4-p_wk->cnt_idx) );
    }

    if( p_wk->cnt++ >= COUNTDOWN_ONE_COUNT_MAX )
    { 
      p_wk->cnt = 0;
      p_wk->cnt_idx++;
    }

    if( p_wk->cnt_idx == 3 )
    { 
      PMSND_PlaySE( SEQ_SE_SYS_44 );
      p_wk->is_start = FALSE;
      p_wk->is_end    = TRUE;
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[0], FALSE );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  カウントダウン  開始処理
 *
 *	@param	COUNTDOWN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void COUNTDOWN_Start( COUNTDOWN_WORK *p_wk )
{ 
  p_wk->cnt       = 0;
  p_wk->cnt_idx   = 0;
  p_wk->is_start  = TRUE;
}
//----------------------------------------------------------------------------
/** 
 *	@brief  カウントダウン  終了検知
 *
 *	@param	const COUNTDOWN_WORK *cp_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
static BOOL COUNTDOWN_IsEnd( const COUNTDOWN_WORK *cp_wk )
{ 
  return cp_wk->is_end;

}
//=============================================================================
/**
 *				SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	シーケンス変更
 *
 *	@param	RHYTHM_MAIN_WORK *p_wk	ワーク
 *	@param	seq_function					シーケンス関数
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Change( RHYTHM_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROC終了
 *
 *	@param	RHYTHM_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( RHYTHM_MAIN_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}

//=============================================================================
/**
 *			SEQ_FUNCTION
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ゲーム開始直後
 *
 *	@param	RHYTHM_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_StartGame( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq )
{	
  enum
  { 
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

  switch( *p_seq )
  { 
  case SEQ_INIT:
    p_wk->cnt = 0;
    *p_seq  = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    if( p_wk->cnt++ > 60 )
    { 
      p_wk->cnt = 0;
      *p_seq  = SEQ_EXIT;
    }
    break;

  case SEQ_EXIT:
    COUNTDOWN_Start( &p_wk->cntdown );
    SEQ_Change( p_wk, SEQFUNC_CountDown );	
    break;
  }
  if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
	{
		PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
		p_wk->p_param->result	= IRCRHYTHM_RESULT_RETURN;
		SEQ_End( p_wk );
	}
	
	//シーンが異なるチェック
	if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
	{	
		SEQ_Change( p_wk, SEQFUNC_SceneError );
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	開始前カウントダウン処理
 *
 *	@param	RHYTHM_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_CountDown( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq )
{ 
  COUNTDOWN_Main( &p_wk->cntdown );
  if( COUNTDOWN_IsEnd( &p_wk->cntdown ) )
  { 
    G2_BlendNone();
    RHYTHMSEARCH_Start( &p_wk->search );
    SEQ_Change( p_wk, SEQFUNC_MainGame );	
  }

  if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
	{
		PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
		p_wk->p_param->result	= IRCRHYTHM_RESULT_RETURN;
		SEQ_End( p_wk );
	}

  //シーンが異なるチェック
	if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
	{	
		SEQ_Change( p_wk, SEQFUNC_SceneError );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 *
 *	@param	RHYTHM_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_MainGame( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq )
{	
	int i;
	GFL_POINT	trg_pos;
	RHYTHMSEARCH_WORK	*p_search;
	p_search	= &p_wk->search;


	for( i = 0; i< NELEMS(sc_diamond_pos); i++ )
	{
		if( TP_GetDiamondTrg( &sc_diamond_pos[i], &trg_pos ) )
		{	
      BACKOBJ_COLOR color;
			color = RHYTHMSEARCH_PlaySEByRhythmSpeed( p_search );
			BACKOBJ_StartEmit( &p_wk->backobj[BACKOBJ_SYS_MAIN], &trg_pos, color );
			RHYTHMSEARCH_SetData( p_search, &sc_diamond_pos[i] );
		}
	}

	//計測終了チェック
	if( RHYTHMSEARCH_IsEnd( p_search ) )
	{	
		MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, RHYTHM_STR_001, 0, 0 );
		PMSND_PlaySE( IRCCOMMON_SE_IRC );
		SEQ_Change( p_wk, SEQFUNC_Result );

	}


	if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
	{
		PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
		p_wk->p_param->result	= IRCRHYTHM_RESULT_RETURN;
		SEQ_End( p_wk );
	}

	//シーンが異なるチェック
	if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
	{	
		SEQ_Change( p_wk, SEQFUNC_SceneError );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	結果
 *
 *	@param	RHYTHM_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Result( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq )
{	

	enum
	{	
		SEQ_RESULT,
		SEQ_MSG_PRINT,
		SEQ_SENDRESULT,
		SEQ_SCENE,
		SEQ_TIMING,
		SEQ_MSG_PRINT_END,
		SEQ_CALC,
		SEQ_END,
#if 0
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_ONLYRESULT_INIT,
		SEQ_ONLYRESULT_MAIN,
		SEQ_ONLYRESULT_EXIT,
		SEQ_NEXTPROC,
#endif
	};

	switch( *p_seq )
	{	
	case SEQ_RESULT:
		if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_RHYTHM_RESULT ) )
		{	
			*p_seq	= SEQ_MSG_PRINT;
		}

		if( APPBAR_GetTrg(p_wk->p_appbar) == APPBAR_ICON_RETURN )
		{
			COMPATIBLE_IRC_Cancel( p_wk->p_param->p_irc );
			PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
			p_wk->p_param->result	= IRCRHYTHM_RESULT_RETURN;
			SEQ_End( p_wk );
		}	
		break;

	case SEQ_MSG_PRINT:
		MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, RHYTHM_STR_002, 0, 0 );
		*p_seq	= SEQ_SENDRESULT;
		break;

	case SEQ_SENDRESULT:
    p_wk->search.minus  = CalcMinus( p_wk );
		if( RHYTHMNET_SendResultData( &p_wk->net, &p_wk->search ) )
		{	
			*p_seq	= SEQ_SCENE;
		}
		break;

	case SEQ_SCENE:
		//次のシーンを設定
		COMPATIBLE_IRC_SetScene(  p_wk->p_param->p_irc, COMPATIBLE_SCENE_AURA );
		if( COMPATIBLE_IRC_SendScene( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_TIMING;
		}
		break;

	case SEQ_TIMING:
		if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_RHYTHM_END ) )
		{	
			*p_seq	= SEQ_MSG_PRINT_END;
		}
		break;

	case SEQ_MSG_PRINT_END:
		if( p_wk->cnt >= RESULT_SEND_CNT )
		{	
			PMSND_PlaySE( IRCMENU_SE_IRC_ON );
			MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, RHYTHM_STR_003, 0, 0 );
			*p_seq	= SEQ_CALC;
		}
		break;

	case SEQ_CALC:
		p_wk->p_param->score	= CalcScore( p_wk );
    { 
      RHYTHMSEARCH_WORK you;
      RHYTHMNET_GetResultData( &p_wk->net, &you );
      p_wk->p_param->minus  = CalcMinus( p_wk ) + you.minus;
    }
		p_wk->p_param->result	= IRCRHYTHM_RESULT_CLEAR;
	//	*p_seq	= SEQ_FADEIN_START;
		*p_seq	= SEQ_END;
		break;

	case SEQ_END:
		SEQ_End( p_wk );
		return;	//下記のアサートを通過しないため
#if 0
	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= SEQ_FADEIN_WAIT;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_ONLYRESULT_INIT;
		}
		break;

	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_ONLYRESULT_MAIN;
		}
		break;

	case SEQ_ONLYRESULT_INIT:
		//画面上のものを消去
		GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BACK], FALSE );
		MSGWND_Clear( &p_wk->msgwnd[MSGWNDID_TEXT] );


		*p_seq	= SEQ_FADEOUT_START;
		break;

	case SEQ_ONLYRESULT_MAIN:
		if( RHYTHM_ONLYRESULT_Main( &p_wk->onlyresult ) )
		{	
			*p_seq	= SEQ_ONLYRESULT_EXIT;
		}
		break;

	case SEQ_ONLYRESULT_EXIT:
		RHYTHM_ONLYRESULT_Exit( &p_wk->onlyresult );
		*p_seq	= SEQ_NEXTPROC;
		break;

	case SEQ_NEXTPROC:
		p_wk->is_next_proc	= TRUE;
		SEQ_End( p_wk );
		break;
#endif
	}

	//メッセージ終了待ち
	if( SEQ_MSG_PRINT <= *p_seq && *p_seq <=  SEQ_MSG_PRINT_END )
	{	
		if( p_wk->cnt <= RESULT_SEND_CNT )
		{	
			p_wk->cnt++;
		}
	}

	//シーンが異なるチェック
	if( *p_seq < SEQ_SCENE && COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
	{	
		SEQ_Change( p_wk, SEQFUNC_SceneError );
	}



}

//----------------------------------------------------------------------------
/**
 *	@brief	シーンエラーシーケンス
 *
 *	@param	RHYTHM_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SceneError( RHYTHM_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
		SEQ_JUNCTION,
		SEQ_INIT,
		SEQ_MSG_PRINT,
		SEQ_TIMING,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_JUNCTION:
		NAGI_Printf( "ErrorScene!\n" );
		//自分が先ならば
		if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) > 0 )
		{	
			p_wk->p_param->score	= 0;
			p_wk->p_param->result	= IRCRHYTHM_RESULT_RETURN;
			SEQ_End( p_wk );
		}
		//相手が先ならば
		else if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) < 0 )
		{	
			*p_seq	= SEQ_INIT;
		}
		else
		{
			GF_ASSERT(0);
		}
		break;
	case SEQ_INIT:
		p_wk->p_param->score	= 0;
		p_wk->p_param->result	= IRCRHYTHM_RESULT_RESULT;
		*p_seq	= SEQ_MSG_PRINT;
		break;
	case SEQ_MSG_PRINT:
		MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, RHYTHM_STR_004, 0, 0 );
		*p_seq	= SEQ_TIMING;
		break;
	case SEQ_TIMING:
		if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_SCENE_ERROR ) )
		{	
			*p_seq	= SEQ_END;
		}
		break;
	case SEQ_END:
		SEQ_End( p_wk );
		break;
	}
}

//=============================================================================
/**
 *			RHYTHMSEARCH
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	リズムサーチ用計測	初期化
 *
 *	@param	RHYTHMSEARCH_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void RHYTHMSEARCH_Init( RHYTHMSEARCH_WORK *p_wk )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(RHYTHMSEARCH_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	リズムサーチ用計測	破棄
 *
 *	@param	RHYTHMSEARCH_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void RHYTHMSEARCH_Exit( RHYTHMSEARCH_WORK *p_wk )
{	
	//破棄
	GFL_STD_MemClear( p_wk, sizeof(RHYTHMSEARCH_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	リズムサーチ用計測	計測開始
 *
 *	@param	RHYTHMSEARCH_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void RHYTHMSEARCH_Start( RHYTHMSEARCH_WORK *p_wk )
{	
	if( OS_IsTickAvailable() )
	{	
		OS_InitTick();
	}
	//時間計測を開始
	p_wk->start_time	=		OS_TicksToMilliSeconds(OS_GetTick());
}
//----------------------------------------------------------------------------
/**
 *	@brief	リズムサーチ用計測	計測終了判定
 *
 *	@param	RHYTHMSEARCH_WORK *p_wk		ワーク
 *
 *	@retval	TRUE計測終了
 *	@retval	FALSE計測終了
 */
//-----------------------------------------------------------------------------
static BOOL RHYTHMSEARCH_IsEnd( const RHYTHMSEARCH_WORK *cp_wk )
{	
  //5秒以上経過したとき
  if( OS_TicksToMilliSeconds(OS_GetTick()) - cp_wk->start_time >= 5000 ) 
  {	
    return TRUE;
  }
	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	リズムサーチ用計測	データ設定
 *
 *	@param	RHYTHMSEARCH_WORK *p_wk	ワーク
 *	@param	GFL_POINT *cp_pos				座標
 *
 */
//-----------------------------------------------------------------------------
static void RHYTHMSEARCH_SetData( RHYTHMSEARCH_WORK *p_wk, const GFL_POINT *cp_pos )
{	
	if( p_wk->data_idx < RHYTHMSEARCH_DATA_MAX )
	{	
		//	p_wk->data[ p_wk->data_idx ].pos						= *cp_pos;
		p_wk->data[ p_wk->data_idx ].prog_ms				= OS_TicksToMilliSeconds(OS_GetTick())
			- p_wk->start_time;

		//最初は、差が0
		if( p_wk->data_idx == 0 )
		{	
			p_wk->data[ p_wk->data_idx ].diff_ms	= 0;
		}
		else	//お互いの差
		{	
			p_wk->data[ p_wk->data_idx ].diff_ms	= p_wk->data[ p_wk->data_idx ].prog_ms
				- p_wk->data[ p_wk->data_idx-1 ].prog_ms;
		}

		//セットしたので次へ進める
		p_wk->data_idx++;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	リズムサーチ	今のリズムスピードにあったSEをならす
 *
 *	@param	const RHYTHMSEARCH_WORK *cp_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static BACKOBJ_COLOR RHYTHMSEARCH_PlaySEByRhythmSpeed( const RHYTHMSEARCH_WORK *cp_wk )
{	
	u32 se;
  BACKOBJ_COLOR color = BACKOBJ_COLOR_YEGREEN;

	if( cp_wk->data_idx < 2 )
	{	
		se	= IRCRHYTHM_SE_TAP_N;
    color = BACKOBJ_COLOR_YEGREEN;
	}
	else
	{	
		const s32 now	= cp_wk->data[ cp_wk->data_idx - 1 ].diff_ms;
		const s32 pre	= cp_wk->data[ cp_wk->data_idx - 2 ].diff_ms;
		const s32 diff	= now - pre;
		if( 0 <= MATH_IAbs(diff) && MATH_IAbs(diff) < RANGE_RHYTHMSCORE_DIFF_01 )
		{	
			//大体同じ
			se = IRCRHYTHM_SE_TAP_N;
      color = BACKOBJ_COLOR_YEGREEN;
			NAGI_Printf( "初回SE %d\n", se );
		}
		else if( diff < 0 )
		{	
			//早い
			if( MATH_IAbs(diff) < RANGE_RHYTHMSCORE_DIFF_03 )
			{	
				se = IRCRHYTHM_SE_TAP_F1;
        color = BACKOBJ_COLOR_ORANGE;
			}
			else
			{	
				se	=IRCRHYTHM_SE_TAP_F2;
        color = BACKOBJ_COLOR_RED;
			}
		}
		else
		{	
			//遅い
			if( MATH_IAbs(diff) < RANGE_RHYTHMSCORE_DIFF_03 )
			{	
				se	= IRCRHYTHM_SE_TAP_S1;
        color = BACKOBJ_COLOR_WATER;
			}
			else
			{	
				se	= IRCRHYTHM_SE_TAP_S2;
        color = BACKOBJ_COLOR_BLUE;
			}
		}
		NAGI_Printf( "SE %d  diff %d\n", se, diff );
	}
	PMSND_PlaySE( se );
  return color;
}

//=============================================================================
/**
 *							NET
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	NET初期化
 *
 *	@param	RHYTHMNET_WORK *p_wk	ワーク
 *	@param	*p_irc								赤外線システム
 *
 */
//-----------------------------------------------------------------------------
static void RHYTHMNET_Init( RHYTHMNET_WORK *p_wk, COMPATIBLE_IRC_SYS *p_irc )
{	
	GFL_STD_MemClear( p_wk, sizeof(RHYTHMNET_WORK) );
	p_wk->p_irc	= p_irc;

	COMPATIBLE_IRC_AddCommandTable( p_irc, GFL_NET_CMD_IRCRHYTHM, sc_recv_tbl, NELEMS(sc_recv_tbl), p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	NET破棄
 *
 *	@param	RHYTHMNET_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void RHYTHMNET_Exit( RHYTHMNET_WORK *p_wk )
{	
	COMPATIBLE_IRC_DelCommandTable( p_wk->p_irc, GFL_NET_CMD_IRCRHYTHM );
	GFL_STD_MemClear( p_wk, sizeof(RHYTHMNET_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	結果情報を相手へ送受信
 *
 *	@param	NET_WORK *p_wk	ワーク
 *	@param	NET_RESULT_DATA *cp_data	データ
 *
 *	@retval	TRUEならば、相手から受信
 *	@retval	FALSEならば受信待ち
 */
//-----------------------------------------------------------------------------
static BOOL RHYTHMNET_SendResultData( RHYTHMNET_WORK *p_wk, const RHYTHMSEARCH_WORK *cp_data )
{	
	enum 
	{	
		SEQ_DATA_SEND_INIT,
		SEQ_DATA_SEND_MAIN,
		SEQ_DATA_SEND_WAIT,
	};

#ifdef DEBUG_IRC_COMPATIBLE_ONLYPLAY
	if( s_is_debug_only_play )
	{	
		return TRUE;
	}
#endif

	switch( p_wk->seq )
	{	
	case SEQ_DATA_SEND_INIT:
		p_wk->result_send	= *cp_data;
		p_wk->seq = SEQ_DATA_SEND_MAIN;
		NAGI_Printf( "結果データ送信開始\n" );
		break;

	case SEQ_DATA_SEND_MAIN:
		{	
			NetID	netID;
			netID	= GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
			if( netID == 0 )
			{	
				netID	= 1;
			}else{	
				netID	= 0;
			}
			if( COMPATIBLE_IRC_SendDataEx( p_wk->p_irc, NETRECV_RESULT,
					sizeof(RHYTHMSEARCH_WORK), &p_wk->result_send, netID, FALSE, FALSE, TRUE ) )
			{	
				NAGI_Printf( "結果データ送信完了、相手待ち\n" );
				p_wk->seq	= SEQ_DATA_SEND_WAIT;
			}
		}
		break;

	case SEQ_DATA_SEND_WAIT:
		if( p_wk->is_recv )
		{
			p_wk->is_recv	= FALSE;
			p_wk->seq	= 0;
			return TRUE;
		}
		break;
	}

	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	結果データ受け取り
 *
 *	@param	NET_WORK *p_wk	ワーク
 *	@param	*p_data							データ受け取り
 *
 */
//-----------------------------------------------------------------------------
static void RHYTHMNET_GetResultData( const RHYTHMNET_WORK *cp_wk, RHYTHMSEARCH_WORK *p_data )
{	
	*p_data	= cp_wk->result_recv;
}
//=============================================================================
/**
 *					NETRECV
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	結果データ受け取り受信コールバック
 *
 *	@param	const int netID	ネットID
 *	@param	int size				サイズ
 *	@param	void* p_data		データ
 *	@param	p_work					ワーク
 *	@param	p_net_handle		ハンドル
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void NETRECV_Result( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{	
	RHYTHMNET_WORK *p_wk	= p_work;

	if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
	{
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		return;	//自分のデータは無視
	}

//	GFL_STD_MemCopy( cp_data, &p_wk->result_recv, sizeof(RHYTHMNET_RESULT_DATA) );
	NAGI_Printf("結果データ受け取り完了\n" );
	p_wk->is_recv		= TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	結果データ受け取りバッファポインタ
 *
 *	@param	netID
 *	@param	pWork
 *	@param	size
 *
 *	@return	バッファポインタ
 */
//-----------------------------------------------------------------------------
static u8* NETRECV_GetBufferAddr(int netID, void* p_work, int size)
{	
	RHYTHMNET_WORK *p_wk	= p_work;

	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		return (u8*)&p_wk->result_recv_my;
	}
	else
	{	
		return (u8*)&p_wk->result_recv;
	}
}
//=============================================================================
/**
 *				汎用
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ひし形内にTrgしたかどうか	(W＝Hの場合のみ)
 *
 *	@param	const GFL_POINT *cp_point	座標
 *	@param	w													幅
 *	@param	h													高さ
 *	@param	*p_trg										座標受け取り
 *
 *	@retval	TRUEタッチした
 *	@retval	FALSEタッチしていない
 */
//-----------------------------------------------------------------------------
static BOOL TP_GetDiamondTrg( const GFL_POINT *cp_diamond, GFL_POINT *p_trg )
{	
	u32 x, y;
	BOOL ret;

	//Cont中で、菱形内のとき
	//	当たり判定の方法は、ベクトルの右側に点があるとき～を辺の数だけ行う
	//
	//
	if( GFL_UI_TP_GetPointTrg( &x, &y ) )
	{	
		//４頂点を作成
		GFL_POINT	top, left, right, bottom;
		int i;
		int dx, dy;
		u8 ret;
		GFL_POINT	v[5];	//左、上、右、下、左の順に頂点

		v[0].x	= cp_diamond->x - TOUCH_DIAMOND_W/2;
		v[0].y	= cp_diamond->y;
		v[1].x	= cp_diamond->x;
		v[1].y	= cp_diamond->y - TOUCH_DIAMOND_H/2;
		v[2].x	= cp_diamond->x + TOUCH_DIAMOND_W/2;
		v[2].y	= cp_diamond->y;
		v[3].x	= cp_diamond->x;
		v[3].y	= cp_diamond->y + TOUCH_DIAMOND_H/2;
		v[4]		= v[0];

		dx	= x;
		dy	= y;

		ret = 0;
		for( i = 0; i < 4; i++ )
		{	
			if( (v[i+1].x - v[i].x)*(dy-v[i].y) - (dx-v[i].x)*(v[i+1].y-v[i].y) > 0 )
			{	
				ret++;
			}
		}

		if( ret == 4)
		{
			//受け取りが存在したら代入して返す
			if( p_trg )
			{	
				p_trg->x	= x;
				p_trg->y	= y;
			}
			return TRUE;
		}

	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	スコア計算
 *
 *	@param	RHYTHM_MAIN_WORK *p_wk	ワーク
 *
 *	@return	スコア
 */
//-----------------------------------------------------------------------------
static u8	CalcScore( const RHYTHM_MAIN_WORK *cp_wk )
{	
	RHYTHMSEARCH_WORK	my;
	RHYTHMSEARCH_WORK	you;

	u32	prog_score;
	u32	diff_score;

	my	= cp_wk->search;
	RHYTHMNET_GetResultData( &cp_wk->net, &you );

	OS_Printf( "■結果の表示\n" );
	//経過時間の判定
	{	
		s32 player1;
		s32 player2;
		u32	prog;
		player1	= my.data[my.data_idx-1].prog_ms - my.data[0].prog_ms;
		player2	= you.data[you.data_idx-1].prog_ms - you.data[0].prog_ms;
		prog	= MATH_IAbs(player1 - player2);
		if( prog  == RANGE_RHYTHMSCORE_PROC_00 )
		{
			prog_score	= VAL_RHYTHMSCORE_PROC_00;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_01 )
		{	
			prog_score	= VAL_RHYTHMSCORE_PROC_01;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_02 )
		{
			prog_score	= VAL_RHYTHMSCORE_PROC_02;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_03 )
		{
			prog_score	= VAL_RHYTHMSCORE_PROC_03;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_04 )
		{
			prog_score	= VAL_RHYTHMSCORE_PROC_04;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_05 )
		{
			prog_score	= VAL_RHYTHMSCORE_PROC_05;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_06 )
		{
			prog_score	= VAL_RHYTHMSCORE_PROC_06;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_07 )
		{
			prog_score	= VAL_RHYTHMSCORE_PROC_07;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_08 )
		{
			prog_score	= VAL_RHYTHMSCORE_PROC_08;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_09 )
		{
			prog_score	= VAL_RHYTHMSCORE_PROC_09;
		}
		else
		{
#if VAL_RHYTHMSCORE_PROC_10
			prog_score	= VAL_RHYTHMSCORE_PROC_10;
#endif 
		}
		OS_Printf( "経過時間 %d点", prog_score );
	}

	//時間の差の判定
	{	
		int i;
		s32 player1;
		s32 player2;
		u32	prog;
		diff_score	= 0;
		for( i = 1; i < 10; i++ )
		{	
			player1	= my.data[i].diff_ms;
			player2	= you.data[i].diff_ms;
			prog	= MATH_IAbs(player1 - player2);

			if( prog  == RANGE_RHYTHMSCORE_DIFF_00 )
			{	
				diff_score	+= VAL_RHYTHMSCORE_DIFF_00;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_01 )
			{	
				diff_score	+= VAL_RHYTHMSCORE_DIFF_01;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_02 )
			{
				diff_score	+= VAL_RHYTHMSCORE_DIFF_02;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_03 )
			{
				diff_score	+= VAL_RHYTHMSCORE_DIFF_03;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_04 )
			{
				diff_score	+= VAL_RHYTHMSCORE_DIFF_04;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_05 )
			{
				diff_score	+= VAL_RHYTHMSCORE_DIFF_05;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_06 )
			{
				diff_score	+= VAL_RHYTHMSCORE_DIFF_06;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_07 )
			{
				diff_score	+= VAL_RHYTHMSCORE_DIFF_07;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_08 )
			{
				diff_score	+= VAL_RHYTHMSCORE_DIFF_08;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_09 )
			{
				diff_score	+= VAL_RHYTHMSCORE_DIFF_09;
			}
			else
			{
#if VAL_RHYTHMSCORE_DIFF_10
				diff_score	+= VAL_RHYTHMSCORE_DIFF_10;
#endif
			}
		}
		OS_Printf( "間隔 %d点\n", diff_score/9 );
	}
	return (prog_score + (diff_score/9) )/2;
}
//----------------------------------------------------------------------------
/**
 *	@brief  減点を計算
 *
 *	@param	const RHYTHM_MAIN_WORK *cp_wk ワーク
 *
 *	@return 減点（＋の値）
 */
//-----------------------------------------------------------------------------
static u8	CalcMinus( const RHYTHM_MAIN_WORK *cp_wk )
{ 
  static const sc_minus_tbl[] =
  { 
    0,
    2,
    4,
    6,
    8,
    10,
  };

  RHYTHMSEARCH_WORK	my;
	RHYTHMSEARCH_WORK	you;

  u32 dif;

	my	= cp_wk->search;
	RHYTHMNET_GetResultData( &cp_wk->net, &you );

  dif = MATH_IAbs( my.data_idx - you.data_idx );
  dif = MATH_IMin( dif, NELEMS(sc_minus_tbl) );

  return sc_minus_tbl[ dif ];
}
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
 *	@param	clwk_ofs							CLWKオフセット何番から使い始めるか
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_Init( BACKOBJ_WORK *p_wk, const GRAPHIC_WORK *cp_grp, BACKOBJ_MOVE_TYPE type, u32 clwk_ofs, int sf_type )
{	
	GFL_STD_MemClear( p_wk, sizeof(BACKOBJ_WORK) );
	p_wk->type	= type;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= BACKOBJ_MOVE_SYNC;
	{	
		int i;
		GFL_CLWK *p_clwk;
		for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
		{	
			p_clwk	= GRAPHIC_GetClwk( cp_grp, clwk_ofs + i );
			BACKOBJ_ONE_Init( &p_wk->wk[i], p_clwk, sf_type );
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
					BACKOBJ_COLOR	color;

					start.x	= GFUser_GetPublicRand(256);
					start.y	= -36;
					end.x		= start.x;
					end.y		= 192 + 36;
					sync	= BACKOBJ_ONE_MOVE_SYNC_MIN + GFUser_GetPublicRand(BACKOBJ_ONE_MOVE_SYNC_DIF);

					//色は青か黄色
					if( GFUser_GetPublicRand(2) )
					{	
						color	= BACKOBJ_COLOR_WATER;
					}
					else
					{	
						color	= BACKOBJ_COLOR_YELLOW;
					}
					
					if( up_or_down )
					{	
						BACKOBJ_ONE_Start( &p_wk->wk[i], &end, &start, color, sync );
					}
					else
					{	
						BACKOBJ_ONE_Start( &p_wk->wk[i], &start, &end, color, sync );
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
					BACKOBJ_COLOR color;
					start.x	= (FX_SinIdx(rot) * 256) >> FX32_SHIFT;
					start.y	= (FX_CosIdx(rot) * 256) >> FX32_SHIFT;
					end.x		= 128;
					end.y		= 96;
					sync	= BACKOBJ_ONE_MOVE_SYNC_MIN + GFUser_GetPublicRand(BACKOBJ_ONE_MOVE_SYNC_DIF);


					//色は青か黄色
					if( GFUser_GetPublicRand(2) )
					{	
						color	= BACKOBJ_COLOR_WATER;
					}
					else
					{	
						color	= BACKOBJ_COLOR_YELLOW;
					}

					BACKOBJ_ONE_Start( &p_wk->wk[i], &start, &end, color, sync );
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
static void BACKOBJ_StartEmit( BACKOBJ_WORK *p_wk, const GFL_POINT *cp_pos, BACKOBJ_COLOR color )
{	
	int i, j;
	u32 sync;
	u16 rot;
	GFL_POINT	end;

	VecFx32	start_v;
	VecFx32	end_v;
	VecFx32	v;


	start_v.x	= cp_pos->x << FX32_SHIFT;
	start_v.y	= cp_pos->y << FX32_SHIFT;
	start_v.z	= 0;

	for( j = 0; j < BACKOBJ_EMIT_NUM; j++ )
	{	
		for( i = 0; i < BACKOBJ_CLWK_MAX; i++ )
		{	
			if( !BACKOBJ_ONE_IsMove( &p_wk->wk[i] ) )
			{
				rot	= GFUser_GetPublicRand(0xFFFF);
				end.x	= (FX_SinIdx(rot) * 256) >> FX32_SHIFT;
				end.y	= (FX_CosIdx(rot) * 256) >> FX32_SHIFT;

				end_v.x	= end.x << FX32_SHIFT;
				end_v.y	= end.y << FX32_SHIFT;
				end_v.z	= 0;

				VEC_Subtract( &end_v, &start_v, &v );
				VEC_Normalize( &v, &v );

				end.x	= (v.x * 256) >> FX32_SHIFT;
				end.y	= (v.y * 256) >> FX32_SHIFT;

				end.x += cp_pos->x;
				end.y	+= cp_pos->y;

				sync	= BACKOBJ_ONE_EMIT_SYNC_MIN + GFUser_GetPublicRand(BACKOBJ_ONE_EMIT_SYNC_DIF);
				BACKOBJ_ONE_Start( &p_wk->wk[i], cp_pos, &end, color, sync );
				break;
			}
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
 */
//-----------------------------------------------------------------------------
static void BACKOBJ_ONE_Init( BACKOBJ_ONE *p_wk, GFL_CLWK *p_clwk,int sf_type )
{	
	GFL_STD_MemClear( p_wk, sizeof(BACKOBJ_ONE) );
	p_wk->p_clwk	= p_clwk;
	p_wk->sf_type	= sf_type;

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
static void BACKOBJ_ONE_Start( BACKOBJ_ONE *p_wk, const GFL_POINT *cp_start, const GFL_POINT *cp_end, BACKOBJ_COLOR color, u32 sync )
{	
	p_wk->is_req		= TRUE;
	VEC_Set( &p_wk->start, FX32_CONST(cp_start->x), FX32_CONST(cp_start->y), 0 );
	VEC_Set( &p_wk->end, FX32_CONST(cp_end->x), FX32_CONST(cp_end->y), 0 );
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
	GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );
	GFL_CLACT_WK_SetPlttOffs( p_wk->p_clwk, color, CLWK_PLTTOFFS_MODE_PLTT_TOP );
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


#ifdef DEBUG_RHYTHM_MSG
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグプリントを更新
 *
 *	@param	RHYTHM_MAIN_WORK *p_wk	ワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void DEBUGRHYTHM_PRINT_UpDate( RHYTHM_MAIN_WORK *p_wk )
{
	enum{	
		DEBUGMSG_TAB	= 90,
		DEBUGMSG_TAB2	= 170,
	};

	int i,j;

	RHYTHMSEARCH_WORK	*p_search;
	p_search	= &p_wk->search;

	DEBUGPRINT_Clear();

	OS_Printf( "○リズムチェック表示スタート↓↓↓↓↓\n" );
	for( j = 0; j < 2; j++ )
	{	
		if( j == 0 )
		{	
			p_search	= &p_wk->search;
		}
		else
		{	
			p_search	= &p_wk->search2;
		}

		OS_Printf( "▼%d人目の表示\n", j );
		{	
			u32 ret;

			DEBUGPRINT_Print( L"タッチ開始から終了まで", 0,  0 );
			ret	= p_search->data[p_search->data_idx-1].prog_ms - p_search->data[0].prog_ms;
			DEBUGPRINT_PrintNumber( L"ミリ秒 %d", ret, DEBUGMSG_TAB*j,  10 );
			DEBUGPRINT_PrintNumber( L"回数 %d", p_search->data_idx, 45+DEBUGMSG_TAB*j, 10 );
			OS_Printf( "継続時間 %d, タッチ回数 %d\n", ret, p_search->data_idx );
		}

		{	
			DEBUGPRINT_Print( L"タッチ間隔", 0,  30 );
			for( i = 0; i < p_search->data_idx; i++ )
			{	
				if( i == 10 )
				{	
					break;
				}
				DEBUGPRINT_PrintNumber( L"間隔[%d]", i, DEBUGMSG_TAB*j,  40+i*10 );
				DEBUGPRINT_PrintNumber( L"%d", p_search->data[i].diff_ms, 30+DEBUGMSG_TAB*j,  40+i*10 );
				OS_Printf( "間隔[%d] %d\n", i, p_search->data[i].diff_ms );
			}
		}
	}

	OS_Printf( "◇差の表示\n" );
	//差
	{	
		s32 player1;
		s32 player2;
		player1	= p_wk->search.data[p_wk->search.data_idx-1].prog_ms - p_wk->search.data[0].prog_ms;
		player2	= p_wk->search2.data[p_wk->search2.data_idx-1].prog_ms - p_wk->search2.data[0].prog_ms;
		DEBUGPRINT_PrintNumber( L"経過の差 %d", MATH_IAbs(player1 - player2), DEBUGMSG_TAB2,  10 );
		OS_Printf( "経過時間の差 %d\n", MATH_IAbs(player1 - player2) );


		for( i = 0; i < p_search->data_idx; i++ )
		{	
			if( i == 10 )
			{	
				break;
			}
			DEBUGPRINT_PrintNumber( L"差[%d]", i, DEBUGMSG_TAB2,  40+i*10 );
			player1	= p_wk->search.data[i].diff_ms;
			player2	= p_wk->search2.data[i].diff_ms;
			DEBUGPRINT_PrintNumber( L"%d", MATH_IAbs(player1-player2), 30+DEBUGMSG_TAB2,  40+i*10 );
			OS_Printf( "間隔の差[%d] %d\n", i, MATH_IAbs(player1 - player2) );
		}
	}

	OS_Printf( "■結果の表示\n" );
	//結果
	{	
		s32 player1;
		s32 player2;
		u32	prog;
		u32	score;
		player1	= p_wk->search.data[p_wk->search.data_idx-1].prog_ms - p_wk->search.data[0].prog_ms;
		player2	= p_wk->search2.data[p_wk->search2.data_idx-1].prog_ms - p_wk->search2.data[0].prog_ms;
		prog	= MATH_IAbs(player1 - player2);
		if( prog  == RANGE_RHYTHMSCORE_PROC_00 )
		{
			score	= VAL_RHYTHMSCORE_PROC_00;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_01 )
		{	
			score	= VAL_RHYTHMSCORE_PROC_01;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_02 )
		{
			score	= VAL_RHYTHMSCORE_PROC_02;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_03 )
		{
			score	= VAL_RHYTHMSCORE_PROC_03;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_04 )
		{
			score	= VAL_RHYTHMSCORE_PROC_04;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_05 )
		{
			score	= VAL_RHYTHMSCORE_PROC_05;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_06 )
		{
			score	= VAL_RHYTHMSCORE_PROC_06;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_07 )
		{
			score	= VAL_RHYTHMSCORE_PROC_07;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_08 )
		{
			score	= VAL_RHYTHMSCORE_PROC_08;
		}
		else if( prog  < RANGE_RHYTHMSCORE_PROC_09 )
		{
			score	= VAL_RHYTHMSCORE_PROC_09;
		}
		else
		{
#if VAL_RHYTHMSCORE_PROC_10
			score	= VAL_RHYTHMSCORE_PROC_10;
#endif 
		}
		OS_Printf( "経過時間 %d点", score );
		score	= 0;
		for( i = 1; i < 10; i++ )
		{	
			player1	= p_wk->search.data[i].diff_ms;
			player2	= p_wk->search2.data[i].diff_ms;
			prog	= MATH_IAbs(player1 - player2);

			if( prog  == RANGE_RHYTHMSCORE_DIFF_00 )
			{	
				score	+= VAL_RHYTHMSCORE_DIFF_00;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_01 )
			{	
				score	+= VAL_RHYTHMSCORE_DIFF_01;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_02 )
			{
				score	+= VAL_RHYTHMSCORE_DIFF_02;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_03 )
			{
				score	+= VAL_RHYTHMSCORE_DIFF_03;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_04 )
			{
				score	+= VAL_RHYTHMSCORE_DIFF_04;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_05 )
			{
				score	+= VAL_RHYTHMSCORE_DIFF_05;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_06 )
			{
				score	+= VAL_RHYTHMSCORE_DIFF_06;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_07 )
			{
				score	+= VAL_RHYTHMSCORE_DIFF_07;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_08 )
			{
				score	+= VAL_RHYTHMSCORE_DIFF_08;
			}
			else if( prog  < RANGE_RHYTHMSCORE_DIFF_09 )
			{
				score	+= VAL_RHYTHMSCORE_DIFF_09;
			}
			else
			{
#if VAL_RHYTHMSCORE_DIFF_10
				score	+= VAL_RHYTHMSCORE_DIFF_10;
#endif
			}
		}
		OS_Printf( "間隔 点%d\n", score/9 );
		
	}
	
	OS_Printf( "↑リズムチェック表示終了↑↑↑↑↑\n" );

}
#endif //DEBUG_RHYTHM_MSG
 
#ifdef DEBUG_RHYTHM_MSG
//=============================================================================
/**
 *				デバッグプリント用画面
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域	初期化
 *
 *	@param	frm											フレーム
 *	@param	heapID									ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Init( u8 frm, BOOL is_now_save, HEAPID heapID )
{	
	DEBUG_PRINT_WORK *p_wk;

	sp_dp_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(DEBUG_PRINT_WORK) );
	p_wk = sp_dp_wk;
	GFL_STD_MemClear( p_wk, sizeof(DEBUG_PRINT_WORK) );
	p_wk->heapID						= heapID;
	p_wk->is_now_save				= is_now_save;
	p_wk->frm								= frm;

	//デバッグプリント用フォント
	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
				NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );	

	//退避エリアをNetEffから取得
	NetErr_GetTempArea( &p_wk->p_char_temp_area, &p_wk->p_scrn_temp_area, &p_wk->p_pltt_temp_area );
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域	破棄
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Exit( void )
{	
	DEBUG_PRINT_WORK *p_wk	= sp_dp_wk;

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
static void DEBUGPRINT_Open( void )
{	
	DEBUG_PRINT_WORK *p_wk	= sp_dp_wk;

	GF_ASSERT( p_wk );

	if( p_wk->is_now_save )
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
  {
    u16 col[4]={ 0xFFFF , 0x0000 , 0x7fff , 0x001f };
    GFL_BG_LoadPalette( p_wk->frm, col, sizeof(u16)*4, 0 );
  }

	//書き込むためのBMP作成
	p_wk->p_bmp	= GFL_BMP_CreateInVRAM( GFL_DISPUT_GetCgxPtr(p_wk->frm), DEBUGPRINT_WIDTH, DEBUGPRINT_HEIGHT, GFL_BMP_16_COLOR, p_wk->heapID );
	GFL_STD_MemClear16( GFL_DISPUT_GetCgxPtr(p_wk->frm) , DEBUGPRINT_CHAR_TEMP_AREA );
}

//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域	終了
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Close( void )
{	
	DEBUG_PRINT_WORK *p_wk	= sp_dp_wk;

	GF_ASSERT( p_wk );

	GFL_BMP_Delete( p_wk->p_bmp );
	if( p_wk->is_now_save )
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
static void DEBUGPRINT_Print( const u16 *cp_str, u16 x, u16 y )
{	
	STRBUF	*p_strbuf;
	STRCODE	str[128];
	u16	strlen;
	DEBUG_PRINT_WORK *p_wk	= sp_dp_wk;

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
static void DEBUGPRINT_PrintNumber( const u16 *cp_str, int number, u16 x, u16 y )
{	
	u16	str[128];
	swprintf( str, 128, cp_str, number );
	DEBUGPRINT_Print( str, x, y );
}
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグ用プリント領域をクリアー
 *
 */
//-----------------------------------------------------------------------------
static void DEBUGPRINT_Clear( void )
{	
	DEBUG_PRINT_WORK *p_wk	= sp_dp_wk;
	GF_ASSERT(p_wk);
	GFL_BMP_Clear( p_wk->p_bmp, 0 );
}
#endif //DEBUG_RHYTHM_MSG

//----------------------------------------------------------------------------
/**
 *	@brief	Wide文字列をSTRBUFに格納
 *
 *	@param	const u16 *cp_char	ワイド文字列
 *	@param	heapID							ヒープID
 *
 *	@return	STRBUF
 */
//-----------------------------------------------------------------------------
static STRBUF * DEBUGPRINT_CreateWideChar( const u16 *cp_str, HEAPID heapID )
{	
	STRBUF	*p_strbuf;
	STRCODE	str[128];
	u16	strlen;

	//STRBUF用に変換
	strlen	= wcslen(cp_str);
	GFL_STD_MemCopy(cp_str, str, strlen*2);
	str[strlen]	= GFL_STR_GetEOMCode();

	//STRBUFに転送
	p_strbuf	= GFL_STR_CreateBuffer( strlen*2, heapID );
	GFL_STR_SetStringCode( p_strbuf, str);

	return p_strbuf;
}

//----------------------------------------------------------------------------
/**
 *	@brief	数値つきWide文字列をSTRBUFに格納
 *
 *	@param	const u16 *cp_char	ワイド文字列
 *	@param	number							数値
 *	@param	heapID							ヒープID
 *
 *	@return	STRBUF
 */
//-----------------------------------------------------------------------------
static STRBUF * DEBUGPRINT_CreateWideCharNumber( const u16 *cp_str, int number, HEAPID heapID )
{	
	u16	str[128];
	swprintf( str, 128, cp_str, number ); 
	return DEBUGPRINT_CreateWideChar( str, heapID );
}
//#endif //DEBUG_RHYTHM_MSG 
