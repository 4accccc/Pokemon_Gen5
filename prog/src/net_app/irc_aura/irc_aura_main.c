//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_aura_main.c
 *	@brief	赤外線ミニゲーム	オーラチェック
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

//	module
#include "infowin/infowin.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/bmp_winframe.h"

//	archive
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_irc_aura.h"
#include "ircaura_gra.naix"

//	aura
#include "net_app/irc_aura.h"

// proc
#include "net_app/irc_result.h"

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
#ifdef PM_DEBUG
//#define DEBUG_AURA_MSG	//デバッグメッセージを出す
//#define DEBUG_ONLY_PLAY	//デバッグ用一台でプレイするモードを出す
#endif //PM_DEBUG


//-------------------------------------
///	点数
//=====================================
//左手最初の座標の距離による点数
#define VAL_POS_SCORE_00 (100)
#define VAL_POS_SCORE_01 (80)
#define VAL_POS_SCORE_02 (50)
#define VAL_POS_SCORE_03 (30)
#define VAL_POS_SCORE_04 (10)
#define VAL_POS_SCORE_05 (0)

#define RANGE_POS_SCORE_00 (8*8)
#define RANGE_POS_SCORE_01 (16*16)
#define RANGE_POS_SCORE_02 (24*24)
#define RANGE_POS_SCORE_03 (32*32)
#define RANGE_POS_SCORE_04 (40*40)

//ブレの座標による点数
#define VAL_SHAKE_SCORE_00 (100)
#define VAL_SHAKE_SCORE_01 (80)
#define VAL_SHAKE_SCORE_02 (50)
#define VAL_SHAKE_SCORE_03 (30)
#define VAL_SHAKE_SCORE_04 (0)

#define RANGE_SHAKE_SCORE_00(x) ( x == 0 )
#define RANGE_SHAKE_SCORE_01(x) ( x == 1 )
#define RANGE_SHAKE_SCORE_02(x) ( x == 2 )
#define RANGE_SHAKE_SCORE_03(x) ( 3 <= x && x <= 5 )
#define RANGE_SHAKE_SCORE_04(x) ( x <= 6)

//-------------------------------------
///	パレット
//=====================================
enum{	
	// メイン画面BG
	AURA_BG_PAL_M_00 = 0,//GUIDE用BG
	AURA_BG_PAL_M_01,		// 窓枠用
	AURA_BG_PAL_M_02,		// 使用してない
	AURA_BG_PAL_M_03,		// 使用してない
	AURA_BG_PAL_M_04,		// 使用してない
	AURA_BG_PAL_M_05,		// 使用してない
	AURA_BG_PAL_M_06,		// 使用してない
	AURA_BG_PAL_M_07,		// 使用してない
	AURA_BG_PAL_M_08,		// 使用してない
	AURA_BG_PAL_M_09,		// 使用してない
	AURA_BG_PAL_M_10,		// 使用してない
	AURA_BG_PAL_M_11,		// 使用してない
	AURA_BG_PAL_M_12,		// 使用してない
	AURA_BG_PAL_M_13,		// 使用してない
	AURA_BG_PAL_M_14,		// フォント用PLT
	AURA_BG_PAL_M_15,		// INFOWIN


	// サブ画面BG
	AURA_BG_PAL_S_00 = 0,	//フォント
	AURA_BG_PAL_S_01,		// 使用してない
	AURA_BG_PAL_S_02,		// 使用してない
	AURA_BG_PAL_S_03,		// 使用してない
	AURA_BG_PAL_S_04,		// 使用してない
	AURA_BG_PAL_S_05,		// 使用してない
	AURA_BG_PAL_S_06,		// 使用してない
	AURA_BG_PAL_S_07,		// 使用してない
	AURA_BG_PAL_S_08,		// 使用してない
	AURA_BG_PAL_S_09,		// 使用してない
	AURA_BG_PAL_S_10,		// 使用してない
	AURA_BG_PAL_S_11,		// 使用してない
	AURA_BG_PAL_S_12,		// 使用してない
	AURA_BG_PAL_S_13,		// 使用してない
	AURA_BG_PAL_S_14,		// フォント用PLT
	AURA_BG_PAL_S_15,		// 使用してない

	// メイン画面OBJ
	AURA_OBJ_PAL_M_00 = 0,//GUIDE用OBJ
	AURA_OBJ_PAL_M_01,		// 使用してない
	AURA_OBJ_PAL_M_02,		// 使用してない
	AURA_OBJ_PAL_M_03,		// 使用してない
	AURA_OBJ_PAL_M_04,		// 使用してない
	AURA_OBJ_PAL_M_05,		// 使用してない
	AURA_OBJ_PAL_M_06,		// 使用してない
	AURA_OBJ_PAL_M_07,		// 使用してない
	AURA_OBJ_PAL_M_08,		// 使用してない
	AURA_OBJ_PAL_M_09,		// 使用してない
	AURA_OBJ_PAL_M_10,		// 使用してない
	AURA_OBJ_PAL_M_11,		// 使用してない
	AURA_OBJ_PAL_M_12,		// 使用してない
	AURA_OBJ_PAL_M_13,		// 使用してない
	AURA_OBJ_PAL_M_14,		// 使用してない
	AURA_OBJ_PAL_M_15,		// 使用してない


	// サブ画面OBJ
	AURA_OBJ_PAL_S_00 = 0,	//使用してない
	AURA_OBJ_PAL_S_01,		// 使用してない
	AURA_OBJ_PAL_S_02,		// 使用してない
	AURA_OBJ_PAL_S_03,		// 使用してない
	AURA_OBJ_PAL_S_04,		// 使用してない
	AURA_OBJ_PAL_S_05,		// 使用してない
	AURA_OBJ_PAL_S_06,		// 使用してない
	AURA_OBJ_PAL_S_07,		// 使用してない
	AURA_OBJ_PAL_S_08,		// 使用してない
	AURA_OBJ_PAL_S_09,		// 使用してない
	AURA_OBJ_PAL_S_10,		// 使用してない
	AURA_OBJ_PAL_S_11,		// 使用してない
	AURA_OBJ_PAL_S_12,		// 使用してない
	AURA_OBJ_PAL_S_13,		// 使用してない
	AURA_OBJ_PAL_S_14,		// 使用してない
	AURA_OBJ_PAL_S_15,		// 使用してない
};

//-------------------------------------
///	情報バー
//=====================================
#define INFOWIN_PLT_NO		(AURA_BG_PAL_M_15)
#define INFOWIN_BG_FRAME	(GFL_BG_FRAME0_M)

//-------------------------------------
///	文字
//=====================================
#define TEXTSTR_PLT_NO				(AURA_BG_PAL_M_14)
#define TEXTSTR_BUFFER_LENGTH	(255)

//-------------------------------------
///	位置
//=====================================
#define	MSGWND_TEXT_X	(1)
#define	MSGWND_TEXT_Y	(18)
#define	MSGWND_TEXT_W	(30)
#define	MSGWND_TEXT_H	(5)

#define	MSGWND_RETURN_X	(1)
#define	MSGWND_RETURN_Y	(22)
#define	MSGWND_RETURN_W	(30)
#define	MSGWND_RETURN_H	(2)

#define DEBUGMSG_LEFT1	(0)
#define DEBUGMSG_LEFT2	(20)
//#define DEBUGMSG_LEFT3	(40)
//#define DEBUGMSG_LEFT4	(60)

#define DEBUGMSG_RIGHT1	(40)
#define DEBUGMSG_RIGHT2	(60)
//#define DEBUGMSG_RIGHT3	(120)
//#define DEBUGMSG_RIGHT4	(140)

#define DEBUGMSG2_TAB			(170)
#define DEBUGMSG3_TAB			(85)

//-------------------------------------
///	カウント
//=====================================
#define TOUCH_COUNTER_MAX	(60*5)
#define TOUCH_COUNTER_SHAKE_SYNC	(30)	//ブレを観測するための1回のシンク
#define TOUCH_COUNTER_SHAKE_MAX	(TOUCH_COUNTER_MAX/TOUCH_COUNTER_SHAKE_SYNC)	//ブレを取得する回数

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
	OBJREGID_TOUCH_PLT,
	OBJREGID_TOUCH_CHR,
	OBJREGID_TOUCH_CEL,

	OBJREGID_MAX
};
//-------------------------------------
///	CLWK取得
//=====================================
typedef enum{	
	CLWKID_TOUCH,
	
	CLWKID_MAX
}CLWKID;

//-------------------------------------
///	メッセージ表示ウィンドウ
//=====================================
enum {
	MSGWNDID_TEXT,
	MSGWNDID_RETURN,

	MSGWNDID_MAX,
};

//-------------------------------------
///	個人結果画面の定数
//=====================================
enum
{	
	AOR_MSGWNDID_TITLE,
	AOR_MSGWNDID_MSG,
	AOR_MSGWNDID_DEBUG,

	AOR_MSGWNDID_MAX,
};

#define AOR_MSGWND_TITLE_X	(2)
#define AOR_MSGWND_TITLE_Y	(3)
#define AOR_MSGWND_TITLE_W	(20)
#define AOR_MSGWND_TITLE_H	(2)
#define AOR_MSGWND_MSG_X		(6)
#define AOR_MSGWND_MSG_Y		(9)
#define AOR_MSGWND_MSG_W		(20)
#define AOR_MSGWND_MSG_H		(6)
#define AOR_MSGWND_DEBUG_X	(2)
#define AOR_MSGWND_DEBUG_Y	(16)
#define AOR_MSGWND_DEBUG_W	(28)
#define AOR_MSGWND_DEBUG_H	(4)

//-------------------------------------
///	デバッグ用人数セーブ機能
//=====================================
#ifdef DEBUG_ONLY_PLAY
#define DEBUG_GAME_NUM	(5)
#define DEBUG_PLAYER_SAVE_NUM	(DEBUG_GAME_NUM*2)
#else
#define DEBUG_PLAYER_SAVE_NUM (1)
#endif //DEBUG_ONLY_PLAY

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
	GFL_ARCUTIL_TRANSINFO	frame_char;
} GRAPHIC_BG_WORK;
//-------------------------------------
///	3D描画環境
//=====================================
typedef struct 
{
	GFL_G3D_CAMERA		*p_camera;
} GRAPHIC_3D_WORK;
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
	GRAPHIC_BG_WORK		bg;
	GRAPHIC_3D_WORK		g3d;
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
} MSGWND_WORK;

//-------------------------------------
///	ブレ計測
//=====================================
typedef struct 
{
	GFL_POINT		shake[TOUCH_COUNTER_SHAKE_MAX];
	u16					shake_idx;
	u32					cnt;				//カウンタ
	u32					shake_cnt;	//ブレ計測用カウンタ
	BOOL				is_end;			//1回取得したフラグ
}SHAKE_SEARCH_WORK;

//-------------------------------------
///	オーラ用ネット
//=====================================
//データ
typedef struct 
{
	GFL_POINT	trg_left;
	GFL_POINT	trg_right;
	SHAKE_SEARCH_WORK	shake_left;
	SHAKE_SEARCH_WORK	shake_right;
} AURANET_RESULT_DATA;
//ネットメイン
typedef struct 
{
	COMPATIBLE_IRC_SYS	*p_irc;
	u32 seq;
	AURANET_RESULT_DATA	result_recv;	//受信バッファ
	AURANET_RESULT_DATA	result_recv_my;	//受信バッファ
	AURANET_RESULT_DATA	result_send;	//送信バッファ
	BOOL is_recv;
} AURANET_WORK;

//-------------------------------------
///	個人成績画面
//=====================================
typedef struct 
{
	u32	seq;
	const MSG_WORK *cp_msg;
	const SHAKE_SEARCH_WORK *cp_search;
	MSGWND_WORK			msgwnd[AOR_MSGWNDID_MAX];
} AURA_ONLYRESULT_WORK;


#ifdef DEBUG_AURA_MSG
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
#endif //DEBUG_AURA_MSG




//-------------------------------------
///	オーラチェックメインワーク
//=====================================
typedef struct _AURA_MAIN_WORK AURA_MAIN_WORK;
typedef void (*SEQ_FUNCTION)( AURA_MAIN_WORK *p_wk, u16 *p_seq );
struct _AURA_MAIN_WORK
{
	//グラフィックモジュール
	GRAPHIC_WORK		grp;
	MSG_WORK				msg;
	MSGWND_WORK			msgwnd[MSGWNDID_MAX];

	//シーケンス管理
	SEQ_FUNCTION		seq_function;
	u16		seq;
	BOOL is_end;
	BOOL is_next_proc;

#ifdef DEBUG_ONLY_PLAY
	u16							debug_player;			//自分か相手か
	u16							debug_game_cnt;	//何ゲーム目か
#endif //DEBUG_ONLY_PLAY

	//ネット
	AURANET_WORK	net;

	//結果表示
	AURA_ONLYRESULT_WORK	onlyresult;

	//結果
	GFL_POINT		trg_left[DEBUG_PLAYER_SAVE_NUM];
	GFL_POINT		trg_right[DEBUG_PLAYER_SAVE_NUM];
	SHAKE_SEARCH_WORK	shake_left[DEBUG_PLAYER_SAVE_NUM];
	SHAKE_SEARCH_WORK	shake_right[DEBUG_PLAYER_SAVE_NUM];

	//引数
	IRC_AURA_PARAM	*p_param;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT IRC_AURA_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_AURA_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_AURA_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
//grp
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
static GFL_CLWK* GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id );
static void GRAPHIC_WriteBmpwinFrame( GRAPHIC_WORK *p_wk, GFL_BMPWIN *p_bmpwin );
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//BG
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_WriteBmpwinFrame( GRAPHIC_BG_WORK *p_wk, GFL_BMPWIN *p_bmpwin );
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
//obj
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID );
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk );
static GFL_CLWK* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, CLWKID id );
//3d
static void GRAPHIC_3D_Init( GRAPHIC_3D_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_3D_Exit( GRAPHIC_3D_WORK *p_wk );
static void GRAPHIC_3D_StartDraw( GRAPHIC_3D_WORK *p_wk );
static void GRAPHIC_3D_EndDraw( GRAPHIC_3D_WORK *p_wk );
static void Graphic_3d_SetUp( void );
//MSG_WORK
static void MSG_Init( MSG_WORK *p_wk, MSG_FONT_TYPE font, HEAPID heapID );
static void MSG_Exit( MSG_WORK *p_wk );
static BOOL MSG_Main( MSG_WORK *p_wk );
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk );
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk );
static const GFL_MSGDATA * MSG_GetMsgDataConst( const MSG_WORK *cp_wk );
static WORDSET * MSG_GetWordSet( const MSG_WORK *cp_wk );
//MSG_WINDOW
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe,
		u8 x, u8 y, u8 w, u8 h, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, const MSG_WORK *cp_msg );
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y );
static void MSGWND_PrintCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID );
static void MSGWND_PrintNumber( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id, u16 x, u16 y );
static void MSGWND_Clear( MSGWND_WORK* p_wk );
static GFL_BMPWIN *MSGWND_GetBmpWin( const MSGWND_WORK* cp_wk );
//ONLYRESULT
static void AURA_ONLYRESULT_Init( AURA_ONLYRESULT_WORK* p_wk, u8 frm, const MSG_WORK *cp_msg, const SHAKE_SEARCH_WORK *cp_search,  HEAPID heapID );
static void AURA_ONLYRESULT_Exit( AURA_ONLYRESULT_WORK* p_wk );
static BOOL AURA_ONLYRESULT_Main( AURA_ONLYRESULT_WORK* p_wk );

//SEQ
static void SEQ_Change( AURA_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function );
static void SEQ_End( AURA_MAIN_WORK *p_wk );
//SEQ_FUNC
static void SEQFUNC_StartGame( AURA_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_TouchLeft( AURA_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_TouchRight( AURA_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_Result( AURA_MAIN_WORK *p_wk, u16 *p_seq );
//net
static void AURANET_Init( AURANET_WORK *p_wk, COMPATIBLE_IRC_SYS *p_irc );
static void AURANET_Exit( AURANET_WORK *p_wk );
static BOOL AURANET_SendResultData( AURANET_WORK *p_wk, const AURANET_RESULT_DATA *cp_data );
static void AURANET_GetResultData( const AURANET_WORK *cp_wk, AURANET_RESULT_DATA *p_data );
//net_recv
static void NETRECV_Result( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
static u8* NETRECV_GetBufferAddr(int netID, void* pWork, int size);
//汎用
static BOOL TP_GetRectTrg( const GFL_RECT *cp_rect, GFL_POINT *p_trg );
static BOOL TP_GetRectCont( const GFL_RECT *cp_rect, GFL_POINT *p_cont );
static void TouchMarker_SetPos( AURA_MAIN_WORK *p_wk, const GFL_POINT *cp_pos );
static void TouchMarker_OffVisible( AURA_MAIN_WORK *p_wk );
static u8		CalcScore( AURA_MAIN_WORK *p_wk );
static BOOL TouchReturnBtn( void );

#ifdef DEBUG_AURA_MSG
static void DEBUGAURA_PRINT_UpDate( AURA_MAIN_WORK *p_wk );
#else
#define DEBUGAURA_PRINT_UpDate( ... )	((void)0)
#endif //DEBUG_AURA_MSG

//ブレ計測
static void	SHAKESEARCH_Init( SHAKE_SEARCH_WORK *p_wk );
static void	SHAKESEARCH_Exit( SHAKE_SEARCH_WORK *p_wk );
static BOOL	SHAKESEARCH_Main( SHAKE_SEARCH_WORK *p_wk, const GFL_RECT	*cp_rect );
//DEBUG_PRINT
#ifdef DEBUG_AURA_MSG
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
#endif //DEBUG_AURA_MSG
static STRBUF * DEBUGPRINT_CreateWideChar( const u16 *cp_str, HEAPID heapID );
static STRBUF * DEBUGPRINT_CreateWideCharNumber( const u16 *cp_str, int number, HEAPID heapID );

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	オーラチェック用プロックデータ
//=====================================
const GFL_PROC_DATA IrcAura_ProcData	= 
{	
	IRC_AURA_PROC_Init,
	IRC_AURA_PROC_Main,
	IRC_AURA_PROC_Exit,
};
//-------------------------------------
///	BGシステム
//=====================================
typedef enum 
{
	GRAPHIC_BG_FRAME_M_INFOWIN,
	GRAPHIC_BG_FRAME_M_TEXT	= GRAPHIC_BG_FRAME_M_INFOWIN,
	GRAPHIC_BG_FRAME_M_GUIDE_L,
	GRAPHIC_BG_FRAME_M_GUIDE_R,
	GRAPHIC_BG_FRAME_S_TEXT,
	GRAPHIC_BG_FRAME_S_BACK,
	GRAPHIC_BG_FRAME_MAX
} GRAPHIC_BG_FRAME;
static const u32 sc_bgcnt_frame[ GRAPHIC_BG_FRAME_MAX ] = 
{
	INFOWIN_BG_FRAME, GFL_BG_FRAME1_M, GFL_BG_FRAME2_M, GFL_BG_FRAME0_S, GFL_BG_FRAME1_S,
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
	// GRAPHIC_BG_FRAME_M_GUIDE_L
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_M_GUIDE_R
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_TEXT
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_BACK
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
	},

};

//-------------------------------------
///	タッチ範囲
//=====================================
static const GFL_RECT	sc_left	=
{	
	0, 2*8, 11*8, 20*8
};
#if 0
static const GFL_RECT	sc_right	=
{	
	19, 2, 22, 20
};
#else	//2点タッチなので、上の座標と左座標の中点
static const GFL_RECT	sc_right	=
{	
	11*8+1, 2*8, 22*8, 20*8
};
#endif

//-------------------------------------
///	NET
//=====================================
enum
{	
	NETRECV_RESULT	= GFL_NET_CMD_IRCAURA,
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
 *	@brief	オーラチェック	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_AURA_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	AURA_MAIN_WORK	*p_wk;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCAURA, 0x20000 );
	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(AURA_MAIN_WORK), HEAPID_IRCAURA );
	GFL_STD_MemClear( p_wk, sizeof(AURA_MAIN_WORK) );
	p_wk->p_param	= p_param;

	//モジュール初期化
	AURANET_Init( &p_wk->net, p_wk->p_param->p_irc );
	GRAPHIC_Init( &p_wk->grp, HEAPID_IRCAURA );
	MSG_Init( &p_wk->msg, MSG_FONT_TYPE_LARGE, HEAPID_IRCAURA );
	INFOWIN_Init( INFOWIN_BG_FRAME, INFOWIN_PLT_NO, HEAPID_IRCAURA );
	MSGWND_Init( &p_wk->msgwnd[MSGWNDID_TEXT], sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TEXT],
			MSGWND_TEXT_X, MSGWND_TEXT_Y, MSGWND_TEXT_W, MSGWND_TEXT_H, HEAPID_IRCAURA );
	MSGWND_Init( &p_wk->msgwnd[MSGWNDID_RETURN], sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT],
			MSGWND_RETURN_X, MSGWND_RETURN_Y, MSGWND_RETURN_W, MSGWND_RETURN_H, HEAPID_IRCAURA );
//	GRAPHIC_WriteBmpwinFrame( &p_wk->grp, MSGWND_GetBmpWin( &p_wk->msgwnd[MSGWNDID_RETURN]) );
	MSGWND_PrintCenter( &p_wk->msgwnd[MSGWNDID_RETURN], &p_wk->msg, AURA_BTN_000 );

	{	
		int i;
		for( i = 0; i < DEBUG_PLAYER_SAVE_NUM; i++ )
		{	
			SHAKESEARCH_Init( &p_wk->shake_left[ i ] );
			SHAKESEARCH_Init( &p_wk->shake_right[ i ] );
		}
	}

	//デバッグ
	DEBUGPRINT_Init( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_BACK], FALSE, HEAPID_IRCAURA );
	DEBUGPRINT_Open();


	SEQ_Change( p_wk, SEQFUNC_StartGame );
	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	オーラチェック	メインプロセス破棄処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_AURA_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	AURA_MAIN_WORK	*p_wk;

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

	//デバッグ破棄
	DEBUGPRINT_Close();
	DEBUGPRINT_Exit();

	//モジュール破棄
	{	
		int i;
		for( i = 0; i < MSGWNDID_MAX; i++ )
		{	
			MSGWND_Exit( &p_wk->msgwnd[i] );
		}
	}
	INFOWIN_Exit();
	GRAPHIC_Exit( &p_wk->grp );
	MSG_Exit( &p_wk->msg );
	AURANET_Exit( &p_wk->net );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );
	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_IRCAURA );
	
	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	オーラチェック	メインプロセスメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_AURA_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
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

	AURA_MAIN_WORK	*p_wk;
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
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
		{
			SEQ_End( p_wk );
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
		GF_ASSERT_MSG( 0, "IRC_AURA_PROC_MainのSEQエラー %d", *p_seq );
	}

	INFOWIN_Update();
	if( MSG_Main( &p_wk->msg ) )
	{	
		int i;
		for( i = 0; i < MSGWNDID_MAX; i++ )
		{	
			MSGWND_Main( &p_wk->msgwnd[i], &p_wk->msg );
		}
	}
	GRAPHIC_Draw( &p_wk->grp );

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
//	GRAPHIC_3D_Init( &p_wk->g3d, heapID );

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

//	GRAPHIC_3D_Exit( &p_wk->g3d );
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
 *	@brief	枠描画
 *
 *	@param	const GRAPHIC_WORK *cp_wk	ワーク
 *	@param	*p_bmpwin									BMPWIN
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_WriteBmpwinFrame( GRAPHIC_WORK *p_wk, GFL_BMPWIN *p_bmpwin )
{	
	GRAPHIC_BG_WriteBmpwinFrame( &p_wk->bg, p_bmpwin );
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
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
		};	
		GFL_BG_SetBGMode( &sc_bg_sys_header );
	}

	//ＢＧコントロール設定
	{
		int i;

		for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
		{
			GFL_BG_SetBGControl( sc_bgcnt_frame[i], &sc_bgcnt_data[i], GFL_BG_MODE_TEXT );
			GFL_BG_ClearFrame( sc_bgcnt_frame[i] );
			GFL_BG_SetVisible( sc_bgcnt_frame[i], VISIBLE_ON );
		}
	}

	//読み込み設定
	{	
		ARCHANDLE *p_handle;

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_IRCAURA_GRAPHIC, heapID );

		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_ircaura_gra_aura_bg_guide_NCLR,
				PALTYPE_MAIN_BG, PALTYPE_MAIN_BG*0x20, 0x20, heapID );
	
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_ircaura_gra_aura_bg_guide_NCGR,
				sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_GUIDE_L], 0, 0, FALSE, heapID );

		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_ircaura_gra_aura_bg_guide_l_NSCR,
				sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_GUIDE_L], 0, 0, FALSE, heapID );

		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_ircaura_gra_aura_bg_guide_r_NSCR,
				sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_GUIDE_R], 0, 0, FALSE, heapID );

		GFL_ARC_CloseDataHandle( p_handle );

		GFL_BG_FillCharacter( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT], 0x00, 1, 0 );
		p_wk->frame_char	= BmpWinFrame_GraphicSetAreaMan(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT], AURA_BG_PAL_M_01, MENU_TYPE_SYSTEM, heapID);
	}

	GFL_BG_SetVisible( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_GUIDE_R], VISIBLE_OFF );
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
	int i;

	//読み込み破棄
	{	
		GFL_BG_FreeCharacterArea(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT],
				GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_char),
				GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->frame_char));
		GFL_BG_FillCharacterRelease(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT], 1,0);
	}

	// BGコントロール破棄
	for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
	{
		GFL_BG_FreeBGControl( sc_bgcnt_frame[i] );
	}

	// BGシステム破棄
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();
}

//----------------------------------------------------------------------------
/**
 *	@brief	窓枠を書き込み
 *
 *	@param	GRAPHIC_BG_WORK *p_wk	ワーク
 *	@param	*p_bmpwin							書き込むBMPWIN
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_WriteBmpwinFrame( GRAPHIC_BG_WORK *p_wk, GFL_BMPWIN *p_bmpwin )
{	
	BmpWinFrame_Write( p_bmpwin, WINDOW_TRANS_ON, 
					GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_char), AURA_BG_PAL_M_01 );
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

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_IRCAURA_GRAPHIC, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_PLT]	= GFL_CLGRP_PLTT_Register( p_handle, 
				NARC_ircaura_gra_aura_obj_touch_NCLR, CLSYS_DRAW_MAIN, AURA_OBJ_PAL_M_00*0x20, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_ircaura_gra_aura_obj_touch_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

		p_wk->reg_id[OBJREGID_TOUCH_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_ircaura_gra_aura_obj_touch_NCER, NARC_ircaura_gra_aura_obj_touch_NANR, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK作成
	{	
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

		p_wk->p_clwk[CLWKID_TOUCH]	= GFL_CLACT_WK_Create( p_wk->p_clunit, 
				p_wk->reg_id[OBJREGID_TOUCH_CHR],
				p_wk->reg_id[OBJREGID_TOUCH_PLT],
				p_wk->reg_id[OBJREGID_TOUCH_CEL],
				&cldata,
				CLSYS_DEFREND_MAIN,
				heapID
				);
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
			GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
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

//=============================================================================
/**
 *					GRAPHIC_3D
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	３D環境の初期化
 *
 *	@param	p_wk			ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Init( GRAPHIC_3D_WORK *p_wk, HEAPID heapID )
{
	static const VecFx32 sc_CAMERA_PER_POS		= { 0,0,FX32_CONST(5) };
	static const VecFx32 sc_CAMERA_PER_UP			= { 0,FX32_ONE,0 };
	static const VecFx32 sc_CAMERA_PER_TARGET	= { 0,0,FX32_CONST( 0 ) };

	enum{	
		CAMERA_PER_FOVY	=	(40),
		CAMERA_PER_ASPECT =	(FX32_ONE * 4 / 3),
		CAMERA_PER_NEAR	=	(FX32_ONE * 1),
		CAMERA_PER_FER	=	(FX32_ONE * 800),
		CAMERA_PER_SCALEW	=(0),
	};

	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K,
			GFL_G3D_VMANLNK, GFL_G3D_PLT32K, 0, heapID, Graphic_3d_SetUp );
	p_wk->p_camera = GFL_G3D_CAMERA_CreatePerspective( CAMERA_PER_FOVY, CAMERA_PER_ASPECT,
				CAMERA_PER_NEAR, CAMERA_PER_FER, CAMERA_PER_SCALEW, 
				&sc_CAMERA_PER_POS, &sc_CAMERA_PER_UP, &sc_CAMERA_PER_TARGET, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	３D環境の破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Exit( GRAPHIC_3D_WORK *p_wk )
{
	GFL_G3D_CAMERA_Delete( p_wk->p_camera );
	GFL_G3D_Exit();
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画開始
 *	
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_StartDraw( GRAPHIC_3D_WORK *p_wk )
{	
	GFL_G3D_DRAW_Start();
	GFL_G3D_CAMERA_Switching( p_wk->p_camera );
	GFL_G3D_DRAW_SetLookAt();
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画終了
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_EndDraw( GRAPHIC_3D_WORK *p_wk )
{	
	GFL_G3D_DRAW_End();
}

//----------------------------------------------------------------------------
/**
 *	@brief	環境セットアップコールバック関数
 */
//-----------------------------------------------------------------------------
static void Graphic_3d_SetUp( void )
{
	// ３Ｄ使用面の設定(表示＆プライオリティー)
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
	G2_SetBG0Priority(0);

	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	G3X_SetShading( GX_SHADING_HIGHLIGHT );
	G3X_AntiAlias( FALSE );
	G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
	G3X_AlphaBlend( TRUE );		// アルファブレンド　オン
	G3X_EdgeMarking( FALSE );
	G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),31,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog
	// ビューポートの設定
	G3_ViewPort(0, 0, 255, 191);

	// ライト設定
	{
		static const GFL_G3D_LIGHT sc_GFL_G3D_LIGHT[] = 
		{
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
		};
		int i;
		
		for( i=0; i<NELEMS(sc_GFL_G3D_LIGHT); i++ ){
			GFL_G3D_SetSystemLight( i, &sc_GFL_G3D_LIGHT[i] );
		}
	}

	//レンダリングスワップバッファ
	GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
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
				NARC_font_large_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
		break;
	case MSG_FONT_TYPE_SMALL:
		p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
				NARC_font_small_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
		break;
	default:
		GF_ASSERT_MSG( 0, "MSGFONT_ERRO %d", font );
	};

	p_wk->p_print_que = PRINTSYS_QUE_Create( heapID );

	p_wk->p_msg = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_irc_aura_dat, heapID );

	p_wk->p_wordset	= WORDSET_Create( heapID );

	{	
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], GX_RGB(31,31,31) );
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
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, u8 x, u8 y, u8 w, u8 h, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( bgframe, x, y, w, h, TEXTSTR_PLT_NO, GFL_BMP_CHRAREA_GET_B );
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( TEXTSTR_BUFFER_LENGTH, heapID );
	PRINT_UTIL_Setup( &p_wk->print_util, p_wk->p_bmpwin );
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
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

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
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

	//文字列作成
	GFL_MSG_GetString( cp_msgdata, strID, p_wk->p_strbuf );

	//センター位置計算
	x	= GFL_BMPWIN_GetSizeX( p_wk->p_bmpwin )*4;
	y	= GFL_BMPWIN_GetSizeY( p_wk->p_bmpwin )*4;
	x	-= PRINTSYS_GetStrWidth( p_wk->p_strbuf, p_font, 0 )/2;
	y	-= PRINTSYS_GetStrHeight( p_wk->p_strbuf, p_font )/2;

	//表示
	PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font );
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
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

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
 *	@brief	画面クリア
 *
 *	@param	MSGWND_WORK* p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Clear( MSGWND_WORK* p_wk )
{	
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	
	GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin );
}

//----------------------------------------------------------------------------
/**
 *	@brief	BMPWIN取得
 *
 *	@param	const MSGWND_WORK* cp_wk	ワーク
 *
 *	@return	BMPWIN
 */
//-----------------------------------------------------------------------------
static GFL_BMPWIN *MSGWND_GetBmpWin( const MSGWND_WORK* cp_wk )
{	
	return cp_wk->p_bmpwin;
}
//=============================================================================
/**
 *				個人結果画面
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	個人結果画面	初期化
 *
 *	@param	AURA_ONLYRESULT_WORK* p_wk	ワーク
 *	@param	frm									使用フレーム
 *	@param	MSG_WORK *cp_wk			メッセージ
 *	@param	heapID							ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void AURA_ONLYRESULT_Init( AURA_ONLYRESULT_WORK* p_wk, u8 frm, const MSG_WORK *cp_msg, const SHAKE_SEARCH_WORK *cp_search,  HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(AURA_ONLYRESULT_WORK) );
	p_wk->cp_msg		= cp_msg;
	p_wk->cp_search	= cp_search;

	MSGWND_Init( &p_wk->msgwnd[AOR_MSGWNDID_TITLE], frm, AOR_MSGWND_TITLE_X, AOR_MSGWND_TITLE_Y,
				AOR_MSGWND_TITLE_W, AOR_MSGWND_TITLE_H, heapID );
	MSGWND_Init( &p_wk->msgwnd[AOR_MSGWNDID_MSG], frm, AOR_MSGWND_MSG_X, AOR_MSGWND_MSG_Y,
				AOR_MSGWND_MSG_W, AOR_MSGWND_MSG_H, heapID );

	MSGWND_Init( &p_wk->msgwnd[AOR_MSGWNDID_DEBUG], frm, AOR_MSGWND_DEBUG_X, AOR_MSGWND_DEBUG_Y,
				AOR_MSGWND_DEBUG_W, AOR_MSGWND_DEBUG_H, heapID );

	MSGWND_Print( &p_wk->msgwnd[AOR_MSGWNDID_TITLE], cp_msg, AURA_RES_000, 0, 0 );


	//文字列作成
	{
		u32 msg_idx;
		u32	shake_abs;
		int i;

		shake_abs	= 0;
		for( i = 1; i <TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			shake_abs	+= MATH_IAbs( cp_search->shake[i].x - cp_search->shake[0].x );
			shake_abs	+= MATH_IAbs( cp_search->shake[i].y - cp_search->shake[0].y );
		}

		if( 40 <= shake_abs )
		{	
			msg_idx	= 0;
		}
		else if( 20 < shake_abs && shake_abs < 40 )
		{	
			msg_idx	= 1;
		}
		else if( shake_abs <= 20 )
		{	
			msg_idx	= 2;
		}
		else
		{	
			GF_ASSERT( 0 );
		}
		OS_Printf( "ブレ幅の絶対値 %d\n", shake_abs );

		MSGWND_Print( &p_wk->msgwnd[AOR_MSGWNDID_MSG], cp_msg, AURA_RESMSG_000 + msg_idx, 0, 0 );
	}

	//デバッグ文字列作成
	{	
		MSGWND_WORK* p_msgwnd;
		PRINT_QUE*	p_que;
		GFL_FONT*		p_font;
		int i;
		u32 shake_abs;
		u32 shake_sum;
		STRBUF *p_strbuf;
		
		static const u16 *scp_sec[]	=
		{	
			L"0.5s",
			L"1.0s",
			L"1.5s",
			L"2.0s",
			L"2.5s",
			L"3.0s",
			L"3.5s",
			L"4.0s",
			L"4.5s",
			L"5.0s",
		};

		p_msgwnd	= &p_wk->msgwnd[AOR_MSGWNDID_DEBUG];
		p_font	= GFL_FONT_Create( ARCID_FONT,
					NARC_font_small_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );		
		shake_sum	= 0;
		for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			shake_abs	= MATH_IAbs( cp_search->shake[i].x - cp_search->shake[0].x );
			shake_abs	+= MATH_IAbs( cp_search->shake[i].y - cp_search->shake[0].y );
			shake_sum	+= shake_abs;

			p_strbuf	= DEBUGPRINT_CreateWideChar( scp_sec[i], heapID ); 
			PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), i*20, 0, p_strbuf, p_font );
			GFL_STR_DeleteBuffer( p_strbuf );

			p_strbuf	= DEBUGPRINT_CreateWideCharNumber( L" %d", shake_abs, heapID ); 
			PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), i*20, 10, p_strbuf, p_font );
			GFL_STR_DeleteBuffer( p_strbuf );
		}

		p_strbuf	= DEBUGPRINT_CreateWideChar( L"合計", heapID ); 
		PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_msgwnd->p_bmpwin ), i*20, 0, p_strbuf, p_font );
		GFL_STR_DeleteBuffer( p_strbuf );


		p_strbuf	= DEBUGPRINT_CreateWideCharNumber( L" %d", shake_sum, heapID ); 
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
 *	@param	AURA_ONLYRESULT_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void AURA_ONLYRESULT_Exit( AURA_ONLYRESULT_WORK* p_wk )
{	
	int i;
	for( i = 0; i < AOR_MSGWNDID_MAX; i++ )
	{	
		MSGWND_Exit( &p_wk->msgwnd[i] );
	}
	GFL_STD_MemClear( p_wk, sizeof(AURA_ONLYRESULT_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	個人結果画面	メイン処理
 *
 *	@param	AURA_ONLYRESULT_WORK* p_wk ワーク
 *
 *	@retval	TRUEならば終了
 *	@retval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL AURA_ONLYRESULT_Main( AURA_ONLYRESULT_WORK* p_wk )
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
//=============================================================================
/**
 *				SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	シーケンス変更
 *
 *	@param	AURA_MAIN_WORK *p_wk	ワーク
 *	@param	seq_function					シーケンス関数
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Change( AURA_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROC終了
 *
 *	@param	AURA_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( AURA_MAIN_WORK *p_wk )
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
 *	@param	AURA_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_StartGame( AURA_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_INIT,
		SEQ_MAIN,
		SEQ_END,
	};
	GFL_POINT		*p_trg_left;

#ifdef DEBUG_ONLY_PLAY
	p_trg_left	= &p_wk->trg_left[ p_wk->debug_game_cnt + (p_wk->debug_player*DEBUG_GAME_NUM) ];
#else
	p_trg_left	= &p_wk->trg_left[0];
#endif //DEBUG_ONLY_PLAY

	switch( *p_seq )
	{	
	case SEQ_INIT:

		DEBUGAURA_PRINT_UpDate( p_wk );

		MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_000, 0, 0 );
#ifdef DEBUG_ONLY_PLAY
		if( p_wk->debug_player != 0 )
		{	
			MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_DEBUG_001, 0, 0 );
		}
		{	
			int i;
			for( i = 0 ; i < DEBUG_PLAYER_SAVE_NUM; i++ )
			{
				p_wk->shake_left[i].cnt				= 0;
				p_wk->shake_left[i].shake_cnt	= 0;
				p_wk->shake_left[i].shake_idx	= 0;
			}
		}
#endif	//DEBUG_ONLY_PLAY 

		TouchMarker_OffVisible( p_wk );
		GFL_BG_SetVisible( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_GUIDE_R], VISIBLE_OFF );
		GFL_BG_SetVisible( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_GUIDE_L], VISIBLE_ON );

		*p_seq	= SEQ_MAIN;
		break;

	case SEQ_MAIN:
		if( TP_GetRectTrg( &sc_left, p_trg_left ) )
		{	
			DEBUGAURA_PRINT_UpDate( p_wk );
			MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_001, 0, 0 );
			*p_seq	= SEQ_END;
		}

#ifdef DEBUG_ONLY_PLAY
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L )
		{	
			if( p_wk->debug_game_cnt == 0 )
			{	
				p_wk->debug_game_cnt	= DEBUG_GAME_NUM-1;
			}
			else
			{	
				p_wk->debug_game_cnt--;
			}
			SEQ_Change( p_wk, SEQFUNC_StartGame);
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
		{	
			p_wk->debug_game_cnt++;
			p_wk->debug_game_cnt	%= DEBUG_GAME_NUM;
			SEQ_Change( p_wk, SEQFUNC_StartGame);
		}
#endif //DEBUG_ONLY_PLAY

		break;

	case SEQ_END:
		SEQ_Change( p_wk, SEQFUNC_TouchLeft );
		break;
	}

	if( TouchReturnBtn() )
	{
		SEQ_End( p_wk );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	左手をタッチ中
 *
 *	@param	AURA_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_TouchLeft( AURA_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_MAIN,
		SEQ_WAIT_RIGHT,
		SEQ_RET,
	};
	GFL_POINT		*p_trg_right;
	SHAKE_SEARCH_WORK	*p_shake_left;
	GFL_POINT	pos;

#ifdef DEBUG_ONLY_PLAY
	p_trg_right		= &p_wk->trg_right[ p_wk->debug_game_cnt + (p_wk->debug_player*DEBUG_GAME_NUM) ];
	p_shake_left	= &p_wk->shake_left[ p_wk->debug_game_cnt + (p_wk->debug_player*DEBUG_GAME_NUM) ];
#else
	p_trg_right		= &p_wk->trg_right[0];
	p_shake_left	= &p_wk->shake_left[0];
#endif //DEBUG_ONLY_PLAY

	TouchMarker_OffVisible( p_wk );

	switch( *p_seq )
	{	
	case SEQ_MAIN:
		if( TP_GetRectCont( &sc_left, &pos ) )
		{	
			//計測終了待ち
			if( SHAKESEARCH_Main( p_shake_left, &sc_left ) )
			{	
				DEBUGAURA_PRINT_UpDate( p_wk );
				MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_002, 0, 0 );
				GFL_BG_SetVisible( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_GUIDE_R], VISIBLE_ON );
				*p_seq	= SEQ_WAIT_RIGHT;
			}

			TouchMarker_SetPos( p_wk, &pos );
		}
		else
		{	
			SHAKESEARCH_Init( p_shake_left);
			*p_seq	= SEQ_RET;
		}
		break;

	case SEQ_WAIT_RIGHT:
		if( TP_GetRectCont( &sc_right, p_trg_right ) )
		{	
			TouchMarker_SetPos( p_wk, p_trg_right );


			DEBUGAURA_PRINT_UpDate( p_wk );
			MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_001, 0, 0 );
			SEQ_Change( p_wk, SEQFUNC_TouchRight );
		}
		else if( TP_GetRectCont( &sc_left, &pos ) )
		{	
			TouchMarker_SetPos( p_wk, &pos );
		}
		else
		{
			*p_seq	= SEQ_RET;
		}
		break;

	case SEQ_RET:
		SEQ_Change( p_wk, SEQFUNC_StartGame );
		break;
	}

	if( TouchReturnBtn() )
	{
		SEQ_End( p_wk );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	右手タッチ中
 *
 *	@param	AURA_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_TouchRight( AURA_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_MAIN,
		SEQ_RET,
	};


	GFL_POINT	pos;
	SHAKE_SEARCH_WORK	*p_shake_right;
#ifdef DEBUG_ONLY_PLAY
	p_shake_right	= &p_wk->shake_right[ p_wk->debug_game_cnt + (p_wk->debug_player*DEBUG_GAME_NUM) ];
#else
	p_shake_right	= &p_wk->shake_right[0];
#endif //DEBUG_ONLY_PLAY

	TouchMarker_OffVisible( p_wk );

	switch( *p_seq )
	{	
	case SEQ_MAIN:
		if( TP_GetRectCont( &sc_right, &pos ) )
		{	
			if( SHAKESEARCH_Main( p_shake_right, &sc_right ) )
			{	
				int i;

				DEBUGAURA_PRINT_UpDate( p_wk );
#ifdef DEBUG_ONLY_PLAY
				//0ならば自分の番なので、次は相手の番
				if( p_wk->debug_player == 0 )
				{	
					p_wk->debug_player++;
					SEQ_Change( p_wk, SEQFUNC_StartGame );
				}
				else
				{
					MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_DEBUG_002, 0, 0 );
					SEQ_Change( p_wk, SEQFUNC_Result );
				}
#else
				MSGWND_Print( &p_wk->msgwnd[MSGWNDID_TEXT], &p_wk->msg, AURA_STR_003, 0, 0 );
				SEQ_Change( p_wk, SEQFUNC_Result );
#endif //DEBUG_ONLY_PLAY
			}

			TouchMarker_SetPos( p_wk, &pos );
		}
		else
		{	
			SHAKESEARCH_Init( p_shake_right );
			*p_seq	= SEQ_RET;
		}
		break;

	case SEQ_RET:
		SEQ_Change( p_wk, SEQFUNC_StartGame );
		break;
	}


	if( TouchReturnBtn() )
	{
		SEQ_End( p_wk );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	結果
 *
 *	@param	AURA_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Result( AURA_MAIN_WORK *p_wk, u16 *p_seq )
{	
#ifdef DEBUG_ONLY_PLAY
	if(	GFL_UI_TP_GetTrg()	)
	{	
		p_wk->debug_game_cnt++;
		p_wk->debug_game_cnt	%=	DEBUG_GAME_NUM;
		p_wk->debug_player		= 0;
		SEQ_Change( p_wk, SEQFUNC_StartGame );
	}
#else
	enum{	
		SEQ_SENDRESULT,
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
	AURANET_RESULT_DATA result;

	switch( *p_seq )
	{	
	case SEQ_SENDRESULT:
		result.trg_left			=	p_wk->trg_left[0];
		result.trg_right		=	p_wk->trg_right[0];
		result.shake_left		= p_wk->shake_left[0];
		result.shake_right	=	p_wk->shake_right[0];
		if( AURANET_SendResultData( &p_wk->net, &result ) )
		{	
			*p_seq	= SEQ_CALC;
		}
		break;

	case SEQ_CALC:
		p_wk->p_param->score	= CalcScore( p_wk );
	//	*p_seq	= SEQ_FADEIN_START;
		*p_seq	= SEQ_END;
		break;

	case SEQ_END:
		SEQ_End( p_wk );
		break;

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
		GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_GUIDE_L], FALSE );
		GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_GUIDE_R], FALSE );
		TouchMarker_OffVisible( p_wk );
		MSGWND_Clear( &p_wk->msgwnd[MSGWNDID_TEXT] );
		MSGWND_Clear( &p_wk->msgwnd[MSGWNDID_RETURN] );

		//新たに結果画面作成
		AURA_ONLYRESULT_Init( &p_wk->onlyresult, sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT], &p_wk->msg, &p_wk->shake_left[0], HEAPID_IRCAURA );

		*p_seq	= SEQ_FADEOUT_START;
		break;

	case SEQ_ONLYRESULT_MAIN:
		if( AURA_ONLYRESULT_Main( &p_wk->onlyresult ) )
		{	
			*p_seq	= SEQ_ONLYRESULT_EXIT;
		}
		break;

	case SEQ_ONLYRESULT_EXIT:
		AURA_ONLYRESULT_Exit( &p_wk->onlyresult );
		*p_seq	= SEQ_NEXTPROC;
		break;

	case SEQ_NEXTPROC:
		p_wk->is_next_proc	= TRUE;
		SEQ_End( p_wk );
		break;
#endif
	}

#endif //DEBUG_ONLY_PLAY
}
//=============================================================================
/**
 *					NET
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	NET初期化
 *
 *	@param	AURANET_WORK *p_wk	ワーク
 *	@param	COMPATIBLE_IRC_SYS	赤外線
 *
 */
//-----------------------------------------------------------------------------
static void AURANET_Init( AURANET_WORK *p_wk, COMPATIBLE_IRC_SYS *p_irc )
{	
	GFL_STD_MemClear( p_wk, sizeof(AURANET_WORK) );
	p_wk->p_irc	= p_irc;

	COMPATIBLE_IRC_AddCommandTable( p_irc, GFL_NET_CMD_IRCAURA, sc_recv_tbl, NELEMS(sc_recv_tbl), p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	NET破棄
 *
 *	@param	AURANET_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void AURANET_Exit( AURANET_WORK *p_wk )
{	
	COMPATIBLE_IRC_DelCommandTable( p_wk->p_irc, GFL_NET_CMD_IRCAURA );
	GFL_STD_MemClear( p_wk, sizeof(AURANET_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	結果情報を相手へ送受信
 *
 *	@param	AURANET_WORK *p_wk	ワーク
 *	@param	AURANET_RESULT_DATA *cp_data	データ
 *
 *	@retval	TRUEならば、相手から受信
 *	@retval	FALSEならば受信待ち
 */
//-----------------------------------------------------------------------------
static BOOL AURANET_SendResultData( AURANET_WORK *p_wk, const AURANET_RESULT_DATA *cp_data )
{	
	enum
	{	
		SEQ_BUFF_SAVE,
		SEQ_SEND_DATA,
		SEQ_RECV_WAIT,
	};

	switch( p_wk->seq )
	{	
	case SEQ_BUFF_SAVE:
		p_wk->result_send	= *cp_data;
		p_wk->seq = SEQ_SEND_DATA;
		NAGI_Printf( "結果データ送信開始\n" );
		break;

	case SEQ_SEND_DATA:
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
					sizeof(AURANET_RESULT_DATA), &p_wk->result_send, netID, FALSE, FALSE, TRUE ) )
			{	
				NAGI_Printf( "結果データ送信完了、相手待ち\n" );
				p_wk->seq	= SEQ_RECV_WAIT;
			}
		}
		break;

	case SEQ_RECV_WAIT:
		if( p_wk->is_recv )
		{
			NAGI_Printf( "結果データ受信完了\n" );
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
 *	@param	AURANET_WORK *p_wk	ワーク
 *	@param	*p_data							データ受け取り
 *
 */
//-----------------------------------------------------------------------------
static void AURANET_GetResultData( const AURANET_WORK *cp_wk, AURANET_RESULT_DATA *p_data )
{	
	*p_data	= cp_wk->result_recv;
}
//=============================================================================
/**
 *				NETRECV
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
	AURANET_WORK *p_wk	= p_work;

	if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
	{
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		return;	//自分のデータは無視
	}

//	GFL_STD_MemCopy( cp_data, &p_wk->result_recv, sizeof(AURANET_RESULT_DATA) );
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
	AURANET_WORK *p_wk	= p_work;

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
 *	@brief	矩形内にTrgしたかどうか
 *
 *	@param	const GFL_RECT *cp_rect	矩形
 *	@param	*p_trg									座標受け取り
 *
 *	@retval	TRUEタッチした
 *	@retval	FALSEタッチしていない
 */
//-----------------------------------------------------------------------------
static BOOL TP_GetRectTrg( const GFL_RECT *cp_rect, GFL_POINT *p_trg )
{	
	u32 x, y;
	BOOL ret;

	//Cont中で、矩形内のとき
	if( GFL_UI_TP_GetPointTrg( &x, &y ) )
	{	
		if( ((u32)( x - cp_rect->left) < (u32)(cp_rect->right - cp_rect->left))
				&	((u32)( y - cp_rect->top) < (u32)(cp_rect->bottom - cp_rect->top))
			)
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
 *	@brief	矩形内にタッチしたかどうか
 *
 *	@param	const GFL_RECT *cp_rect	矩形
 *	@param	*p_cont									座標受け取り
 *
 *	@retval	TRUEタッチした
 *	@retval	FALSEタッチしていない
 */
//-----------------------------------------------------------------------------
static BOOL TP_GetRectCont( const GFL_RECT *cp_rect, GFL_POINT *p_cont )
{	
	u32 x, y;
	BOOL ret;

	//Cont中で、矩形内のとき
	if( GFL_UI_TP_GetPointCont( &x, &y ) )
	{	
		if( ((u32)( x - cp_rect->left) < (u32)(cp_rect->right - cp_rect->left))
				&	((u32)( y - cp_rect->top) < (u32)(cp_rect->bottom - cp_rect->top))
			)
		{
			//受け取りが存在したら代入して返す
			if( p_cont )
			{	
				p_cont->x	= x;
				p_cont->y	= y;
			}
			return TRUE;
		}
	}

	return FALSE;
}

#ifdef DEBUG_AURA_MSG
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグプリントを更新
 *
 *	@param	AURA_MAIN_WORK *p_wk	ワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void DEBUGAURA_PRINT_UpDate( AURA_MAIN_WORK *p_wk )
{	
	int i, j;
	int now_idx;

	DEBUGPRINT_Clear();

	DEBUGPRINT_PrintNumber( L"%d番目のゲーム", p_wk->debug_game_cnt, 0,  0 );
	DEBUGPRINT_PrintNumber( L"%d人目の番です", p_wk->debug_player, 100,  0 );


	//プリント
	OS_Printf( "↓○%d番目のゲーム　プリント開始↓↓↓↓↓↓↓↓\n",p_wk->debug_game_cnt );
	for( j = 0; j < 2; j++ )
	{	
		now_idx	= p_wk->debug_game_cnt + (j*DEBUG_GAME_NUM);

		DEBUGPRINT_Print( L"左指の座標", DEBUGMSG_LEFT1+(DEBUGMSG2_TAB*j), 10 );
		DEBUGPRINT_Print( L"右指の座標", DEBUGMSG_RIGHT1+(DEBUGMSG2_TAB*j),10 );
	
		DEBUGPRINT_PrintNumber( L"X %d", p_wk->trg_left[now_idx].x, DEBUGMSG_LEFT1+(DEBUGMSG2_TAB*j), 20 );
		DEBUGPRINT_PrintNumber( L"Y %d", p_wk->trg_left[now_idx].y, DEBUGMSG_LEFT2+(DEBUGMSG2_TAB*j), 20 );
	
		DEBUGPRINT_PrintNumber( L"X %d", p_wk->trg_right[now_idx].x, DEBUGMSG_RIGHT1+(DEBUGMSG2_TAB*j), 20 );
		DEBUGPRINT_PrintNumber( L"Y %d", p_wk->trg_right[now_idx].y, DEBUGMSG_RIGHT2+(DEBUGMSG2_TAB*j), 20 );
	
		DEBUGPRINT_Print( L"左指のブレ", DEBUGMSG_LEFT1+(DEBUGMSG2_TAB*j), 30 );
		DEBUGPRINT_Print( L"右指のブレ", DEBUGMSG_RIGHT1+(DEBUGMSG2_TAB*j),30 );

		for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			//DEBUGPRINT_PrintNumber( L"X %d", p_wk->shake_left.shake[i].x, DEBUGMSG_LEFT1, i*10+40 );
			DEBUGPRINT_PrintNumber( L"X %d ", p_wk->shake_left[now_idx].shake[i].x- p_wk->shake_left[now_idx].shake[0].x, DEBUGMSG_LEFT1+(DEBUGMSG2_TAB*j), i*10+40 );
			//DEBUGPRINT_PrintNumber( L"Y %d", p_wk->shake_left.shake[i].y, DEBUGMSG_LEFT3, i*10+40 );
			DEBUGPRINT_PrintNumber( L"Y %d ", p_wk->shake_left[now_idx].shake[i].y- p_wk->shake_left[now_idx].shake[0].y, DEBUGMSG_LEFT2+(DEBUGMSG2_TAB*j), i*10+40 );
	
			//DEBUGPRINT_PrintNumber( L"X %d", p_wk->shake_right.shake[i].x, DEBUGMSG_RIGHT1, i*10+40 );
			DEBUGPRINT_PrintNumber( L"X %d ", p_wk->shake_right[now_idx].shake[i].x - p_wk->shake_right[now_idx].shake[0].x, DEBUGMSG_RIGHT1+(DEBUGMSG2_TAB*j), i*10+40 );
			//DEBUGPRINT_PrintNumber( L"Y %d", p_wk->shake_right.shake[i].y, DEBUGMSG_RIGHT3, i*10+40 );
			DEBUGPRINT_PrintNumber( L"Y %d ", p_wk->shake_right[now_idx].shake[i].y - p_wk->shake_right[now_idx].shake[0].y,DEBUGMSG_RIGHT2+(DEBUGMSG2_TAB*j), i*10+40 );
		}
	

		OS_Printf( "◇%d人目のデータ\n", j );
	
		OS_Printf("左手座標\n");
		OS_Printf("X %d Y %d\n", p_wk->trg_left[now_idx].x, p_wk->trg_left[now_idx].y);
	
		OS_Printf("右手座標\n");
		OS_Printf("X %d Y %d\n", p_wk->trg_right[now_idx].x, p_wk->trg_right[now_idx].y);
	
		OS_Printf("左手ブレ幅\n");
		for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			OS_Printf( "X %d (%d) Y %d (%d)\n", 
					p_wk->shake_left[now_idx].shake[i].x, 
					p_wk->shake_left[now_idx].shake[i].x - p_wk->shake_left[now_idx].shake[0].x, 
					p_wk->shake_left[now_idx].shake[i].y, 
					p_wk->shake_left[now_idx].shake[i].y - p_wk->shake_left[now_idx].shake[0].y );
		}
	
		OS_Printf("右手ブレ幅\n");
		for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			OS_Printf( "X %d (%d) Y %d (%d)\n", 
					p_wk->shake_right[now_idx].shake[i].x, 
					p_wk->shake_right[now_idx].shake[i].x - p_wk->shake_right[now_idx].shake[0].x, 
					p_wk->shake_right[now_idx].shake[i].y, 
					p_wk->shake_right[now_idx].shake[i].y - p_wk->shake_right[now_idx].shake[0].y );
		}
	}

	{	
		u16 idx1, idx2;
		s32	ofs1[4];
		s32	ofs2[4];

		idx1	= p_wk->debug_game_cnt + (0*DEBUG_GAME_NUM);;
		idx2	= p_wk->debug_game_cnt + (1*DEBUG_GAME_NUM);;

		//差---------------------
		DEBUGPRINT_PrintNumber( L"X %d", p_wk->trg_left[idx1].x - p_wk->trg_left[idx2].x,
				DEBUGMSG3_TAB, 20 );
		DEBUGPRINT_PrintNumber( L"Y %d", p_wk->trg_left[idx1].y - p_wk->trg_left[idx2].y,
				DEBUGMSG3_TAB+20, 20 );
		DEBUGPRINT_PrintNumber( L"X %d", p_wk->trg_right[idx1].x - p_wk->trg_right[idx2].x,
				DEBUGMSG3_TAB+40, 20 );
		DEBUGPRINT_PrintNumber( L"Y %d", p_wk->trg_right[idx1].y - p_wk->trg_right[idx2].y,
				DEBUGMSG3_TAB+60, 20 );

		for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			ofs1[0]	= p_wk->shake_left[idx1].shake[i].x- p_wk->shake_left[idx1].shake[0].x;
			ofs1[1]	= p_wk->shake_left[idx1].shake[i].y- p_wk->shake_left[idx1].shake[0].y;
			ofs1[2]	= p_wk->shake_right[idx1].shake[i].x- p_wk->shake_right[idx1].shake[0].x;
			ofs1[3]	= p_wk->shake_right[idx1].shake[i].y- p_wk->shake_right[idx1].shake[0].y;
			ofs2[0]	= p_wk->shake_left[idx2].shake[i].x- p_wk->shake_left[idx2].shake[0].x;
			ofs2[1]	= p_wk->shake_left[idx2].shake[i].y- p_wk->shake_left[idx2].shake[0].y;
			ofs2[2]	= p_wk->shake_right[idx2].shake[i].x- p_wk->shake_right[idx2].shake[0].x;
			ofs2[3]	= p_wk->shake_right[idx2].shake[i].y- p_wk->shake_right[idx2].shake[0].y;

			DEBUGPRINT_PrintNumber( L"X %d ", ofs1[0]-ofs2[0], DEBUGMSG3_TAB, i*10+40 );
			DEBUGPRINT_PrintNumber( L"Y %d ", ofs1[1]-ofs2[1], DEBUGMSG3_TAB+20, i*10+40 );
			DEBUGPRINT_PrintNumber( L"X %d ", ofs1[2]-ofs2[2], DEBUGMSG3_TAB+40, i*10+40 );
			DEBUGPRINT_PrintNumber( L"Y %d ", ofs1[3]-ofs2[3], DEBUGMSG3_TAB+60, i*10+40 );
		}

		OS_Printf( "◇差\n" );
		OS_Printf("左手座標の差\n");
		OS_Printf("X %d Y %d\n", 
				p_wk->trg_left[idx1].x - p_wk->trg_left[idx2].x,
				p_wk->trg_left[idx1].y - p_wk->trg_left[idx2].y);
	
		OS_Printf("右手座標の差\n");
		OS_Printf("X %d Y %d\n", 
				p_wk->trg_right[idx1].x - p_wk->trg_right[idx2].x,
				p_wk->trg_right[idx1].y - p_wk->trg_right[idx2].y);

		OS_Printf( "左手ブレの差\n" );
		for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			ofs1[0]	= p_wk->shake_left[idx1].shake[i].x- p_wk->shake_left[idx1].shake[0].x;
			ofs1[1]	= p_wk->shake_left[idx1].shake[i].y- p_wk->shake_left[idx1].shake[0].y;
			ofs2[0]	= p_wk->shake_left[idx2].shake[i].x- p_wk->shake_left[idx2].shake[0].x;
			ofs2[1]	= p_wk->shake_left[idx2].shake[i].y- p_wk->shake_left[idx2].shake[0].y;
			OS_Printf( "X (%d) Y (%d)\n",ofs1[0]-ofs2[0], ofs1[1]-ofs2[1] );
		}

		OS_Printf( "右手ブレの差\n" );
		for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			ofs1[2]	= p_wk->shake_right[idx1].shake[i].x- p_wk->shake_right[idx1].shake[0].x;
			ofs1[3]	= p_wk->shake_right[idx1].shake[i].y- p_wk->shake_right[idx1].shake[0].y;
			ofs2[2]	= p_wk->shake_right[idx2].shake[i].x- p_wk->shake_right[idx2].shake[0].x;
			ofs2[3]	= p_wk->shake_right[idx2].shake[i].y- p_wk->shake_right[idx2].shake[0].y;
			OS_Printf( "X (%d) Y (%d)\n",ofs1[2]-ofs2[2], ofs1[3]-ofs2[3] );
		}

	}
	
	{
		GFL_POINT	temp;
		u32	checklist[9];		//左上、右上、左下、右下、左、上、右、下、中心



		for( j = 0; j < 2; j++ )
		{	
			OS_Printf( "△%d人目の左手ブレ計測\n", j );
			now_idx	= p_wk->debug_game_cnt + (j*DEBUG_GAME_NUM);
			GFL_STD_MemClear( checklist, sizeof(u32)*9 );

			for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
			{	
				temp.x	= p_wk->shake_left[now_idx].shake[i].x - p_wk->shake_left[now_idx].shake[0].x;
				temp.y	= p_wk->shake_left[now_idx].shake[i].y - p_wk->shake_left[now_idx].shake[0].y;
				if( temp.x < 0 && temp.y > 0 )				//左上
				{	
					checklist[0]++;
				}
				else if( temp.x > 0 && temp.y > 0 )		//右上
				{	
					checklist[1]++;
				}
				else if( temp.x < 0 && temp.y < 0 )		//左下
				{	
					checklist[2]++;
				}
				else if( temp.x > 0 && temp.y < 0 )		//右下
				{	
					checklist[3]++;
				}
				else if( temp.x < 0 && temp.y == 0 )		//左
				{
					checklist[4]++;
				}
				else if( temp.x == 0 && temp.y > 0 )		//上
				{
					checklist[5]++;
				}
				else if( temp.x > 0 && temp.y == 0 )		//右
				{
					checklist[6]++;
				}
				else if( temp.x == 0 && temp.y < 0 )	//下
				{
					checklist[7]++;
				}
				else if( temp.x == 0 && temp.y == 0 )	//中心
				{
					checklist[8]++;
				}

			}

			OS_Printf( "左上　%d個\n", checklist[0] );
			OS_Printf( "右上　%d個\n", checklist[1] );
			OS_Printf( "左下　%d個\n", checklist[2] );
			OS_Printf( "右下　%d個\n", checklist[3] );
			OS_Printf( "左　　%d個\n", checklist[4] );
			OS_Printf( "上　　%d個\n", checklist[5] );
			OS_Printf( "右　　%d個\n", checklist[6] );
			OS_Printf( "下　　%d個\n", checklist[7] );
			OS_Printf( "中心　%d個\n", checklist[8] );


			OS_Printf( "△%d人目の右手ブレ計測\n", j );
			GFL_STD_MemClear( checklist, sizeof(u32)*9 );
			for( i = 0; i < TOUCH_COUNTER_SHAKE_MAX; i++ )
			{	
				temp.x	= p_wk->shake_right[now_idx].shake[i].x - p_wk->shake_right[now_idx].shake[0].x;
				temp.y	= p_wk->shake_right[now_idx].shake[i].y - p_wk->shake_right[now_idx].shake[0].y;
				if( temp.x < 0 && temp.y > 0 )				//左上
				{	
					checklist[0]++;
				}
				else if( temp.x > 0 && temp.y > 0 )		//右上
				{	
					checklist[1]++;
				}
				else if( temp.x < 0 && temp.y < 0 )		//左下
				{	
					checklist[2]++;
				}
				else if( temp.x > 0 && temp.y < 0 )		//右下
				{	
					checklist[3]++;
				}
				else if( temp.x < 0 && temp.y == 0 )		//左
				{
					checklist[4]++;
				}
				else if( temp.x == 0 && temp.y > 0 )		//上
				{
					checklist[5]++;
				}
				else if( temp.x > 0 && temp.y == 0 )		//右
				{
					checklist[6]++;
				}
				else if( temp.x == 0 && temp.y < 0 )	//下
				{
					checklist[7]++;
				}
				else if( temp.x == 0 && temp.y == 0 )	//中心
				{
					checklist[8]++;
				}

			}

			OS_Printf( "左上　%d個\n", checklist[0] );
			OS_Printf( "右上　%d個\n", checklist[1] );
			OS_Printf( "左下　%d個\n", checklist[2] );
			OS_Printf( "右下　%d個\n", checklist[3] );
			OS_Printf( "左　　%d個\n", checklist[4] );
			OS_Printf( "上　　%d個\n", checklist[5] );
			OS_Printf( "右　　%d個\n", checklist[6] );
			OS_Printf( "下　　%d個\n", checklist[7] );
			OS_Printf( "中心　%d個\n", checklist[8] );

		}
	}

	{	
		GFL_POINT	pos1, pos2;
		u32 ret;
		OS_Printf( "△お互いの座標計測\n" );
		pos1	= p_wk->trg_left[p_wk->debug_game_cnt + (0*DEBUG_GAME_NUM)];
		pos2	= p_wk->trg_left[p_wk->debug_game_cnt + (1*DEBUG_GAME_NUM)];
		ret	= ((u32)(pos1.x-pos2.x)*(u32)(pos1.x-pos2.x))+((u32)(pos1.y-pos2.y)*(u32)(pos1.y-pos2.y));

		if( 0 < ret & ret < 8*8 )
		{	
			OS_Printf( "距離　100点\n" );
		}
		else if( 8*8 < ret & ret < 16 * 16 )
		{	
			OS_Printf( "距離　80点\n" );
		}
		else if( 16*16 < ret & ret < 24 * 24 )
		{	
			OS_Printf( "距離　50点\n" );
		}
		else if( 24*24 < ret & ret < 32 * 32 )
		{	
			OS_Printf( "距離　30点\n" );
		}
		else if( 32*32 < ret & ret < 40 * 40 )
		{	
			OS_Printf( "距離　10点\n" );
		}
		else{	
			OS_Printf( "距離　0点\n" );
		}
	}

	{
		GFL_POINT temp1, temp2;
		u32 shake1, shake2;
		u8 idx1, idx2;
		u16 ret;
		u16 score;
		OS_Printf( "△お互いのブレ計算結果\n" );

		idx1	= p_wk->debug_game_cnt + (0*DEBUG_GAME_NUM);
		idx2	= p_wk->debug_game_cnt + (1*DEBUG_GAME_NUM);

		score	= 0;
		for( i = 1; i <TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			temp1.x	= p_wk->shake_left[idx1].shake[i].x - p_wk->shake_left[idx1].shake[0].x;
			temp1.y	= p_wk->shake_left[idx1].shake[i].y - p_wk->shake_left[idx1].shake[0].y;
			temp2.x	= p_wk->shake_left[idx2].shake[i].x - p_wk->shake_left[idx2].shake[0].x;
			temp2.y	= p_wk->shake_left[idx2].shake[i].y - p_wk->shake_left[idx2].shake[0].y;

			shake1	= MATH_IAbs( temp1.x ) + MATH_IAbs( temp1.y );
			shake2	= MATH_IAbs( temp2.x ) + MATH_IAbs( temp2.y );

			if( shake1 > shake2 )
			{	
				ret	= shake1 - shake2;
			}
			else
			{	
				ret	= shake2 - shake1;
			}

			switch( ret )
			{	
			case 0:
				score	+= 100;
				OS_Printf( "ブレ[i]　100点加算\n", i );
				break;
			case 1:
				score	+= 80;
				OS_Printf( "ブレ[i]　80点加算\n", i );
				break;
			case 2:
				//fall through
			case 3:
				score	+= 50;
				OS_Printf( "ブレ[i]　50点加算\n", i );
				break;
			case 4:
				//fall through
			case 5:
				score	+= 30;
				OS_Printf( "ブレ[i]　30点加算\n", i );
				break;
			default:	//6～
				//score	+= 0;
				OS_Printf( "ブレ[i]　0点加算\n", i );
			}
		}

		OS_Printf( "ブレ結果 %d点\n", score/9 );
	}


	OS_Printf( "↑○%d番目のゲーム　プリント終了↑↑↑↑↑↑↑↑\n", p_wk->debug_game_cnt );
}
#endif //DEBUG_AURA_MSG

//----------------------------------------------------------------------------
/**
 *	@brief	タッチマーカーの位置を決定し表示
 *
 *	@param	AURA_MAIN_WORK *p_wk	ワーク
 *	@param	GFL_POINT *p_pos	位置
 *
 */
//-----------------------------------------------------------------------------
static void TouchMarker_SetPos( AURA_MAIN_WORK *p_wk, const GFL_POINT *cp_pos )
{	
	GFL_CLWK	*p_marker;
	p_marker	= GRAPHIC_GetClwk( &p_wk->grp, CLWKID_TOUCH );
	GFL_CLACT_WK_SetDrawEnable( p_marker, TRUE );

	{
		GFL_CLACTPOS clpos;
		clpos.x	= cp_pos->x;
		clpos.y	= cp_pos->y;
		GFL_CLACT_WK_SetPos( p_marker, &clpos, 0 );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	タッチマーカーの表示をOFF
 *
 *	@param	AURA_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void TouchMarker_OffVisible( AURA_MAIN_WORK *p_wk )
{	
	GFL_CLWK	*p_marker;
	p_marker	= GRAPHIC_GetClwk( &p_wk->grp, CLWKID_TOUCH );
	GFL_CLACT_WK_SetDrawEnable( p_marker, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief	スコア計算
 *
 *	@param	AURA_MAIN_WORK *p_wk	ワーク
 *
 *	@return	スコア
 */
//-----------------------------------------------------------------------------
static u8	 CalcScore( AURA_MAIN_WORK *p_wk )
{	
	AURANET_RESULT_DATA	my;
	AURANET_RESULT_DATA	you;

	u32	pos_score;
	u32 shake_score;

	my.trg_left		= p_wk->trg_left[0];
	my.trg_right	= p_wk->trg_right[0];
	my.shake_left	= p_wk->shake_left[0];
	my.shake_right= p_wk->shake_right[0];

	AURANET_GetResultData( &p_wk->net, &you );

	//座標スコア
	{	
		GFL_POINT	pos1, pos2;
		u32 ret;
		OS_Printf( "△お互いの座標計測\n" );
		pos1	= my.trg_left;
		pos2	= you.trg_left;
		ret	= ((u32)(pos1.x-pos2.x)*(u32)(pos1.x-pos2.x))+((u32)(pos1.y-pos2.y)*(u32)(pos1.y-pos2.y));
		if( 0 < ret & ret <= RANGE_POS_SCORE_00 )
		{	
			pos_score	= VAL_POS_SCORE_00;
		}
		else if( RANGE_POS_SCORE_00 < ret & ret <= RANGE_POS_SCORE_01 )
		{	
			pos_score	= VAL_POS_SCORE_01;
		}
		else if( RANGE_POS_SCORE_01 < ret & ret <= RANGE_POS_SCORE_02 )
		{	
			pos_score	= VAL_POS_SCORE_02;
		}
		else if( RANGE_POS_SCORE_02 < ret & ret <= RANGE_POS_SCORE_03 )
		{	
			pos_score	= VAL_POS_SCORE_03;
		}
		else if( RANGE_POS_SCORE_03 < ret & ret <= RANGE_POS_SCORE_04 )
		{	
			pos_score	= VAL_POS_SCORE_04;
		}
		else{	
			pos_score	= VAL_POS_SCORE_05;
		}
		OS_Printf("自分の左手座標\n");
		OS_Printf("X %d Y %d\n", 
				pos1.x,
				pos1.y);
	
		OS_Printf("相手の左手座標\n");
		OS_Printf("X %d Y %d\n", 
				pos2.x,
				pos2.y);
		OS_Printf( "距離　%d点\n", pos_score );
	}

	//ブレスコア
	{	
		int i;
		GFL_POINT temp1, temp2;
		u32 shake1, shake2;
		u16 ret;
		shake_score	= 0;
		for( i = 1; i <TOUCH_COUNTER_SHAKE_MAX; i++ )
		{	
			temp1.x	= my.shake_left.shake[i].x - my.shake_left.shake[0].x;
			temp1.y	= my.shake_left.shake[i].y - my.shake_left.shake[0].y;
			temp2.x	= you.shake_left.shake[i].x - you.shake_left.shake[0].x;
			temp2.y	= you.shake_left.shake[i].y - you.shake_left.shake[0].y;

			OS_Printf( "自分のブレ[%d] X %d Y %d\n", i, temp1.x, temp1.y );
			OS_Printf( "相手のブレ[%d] X %d Y %d\n", i, temp2.x, temp2.y );
			


			shake1	= MATH_IAbs( temp1.x ) + MATH_IAbs( temp1.y );
			shake2	= MATH_IAbs( temp2.x ) + MATH_IAbs( temp2.y );

			if( shake1 > shake2 )
			{	
				ret	= shake1 - shake2;
			}
			else
			{	
				ret	= shake2 - shake1;
			}

			if( RANGE_SHAKE_SCORE_00(ret) )
			{	
				shake_score	+= VAL_SHAKE_SCORE_00;
				OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_00 );
			}
			else if( RANGE_SHAKE_SCORE_01(ret) )
			{	
				shake_score	+= VAL_SHAKE_SCORE_01;
				OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_01 );
			}
			else if( RANGE_SHAKE_SCORE_02(ret) )
			{	
				shake_score	+= VAL_SHAKE_SCORE_02;
				OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_02 );
			}
			else if( RANGE_SHAKE_SCORE_03(ret) )
			{	
				shake_score	+= VAL_SHAKE_SCORE_03;
				OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_03 );
			}
			else if( RANGE_SHAKE_SCORE_04(ret) )
			{	
#if VAL_SHAKE_SCORE_04
					shake_score	+= VAL_SHAKE_SCORE_04;
#endif //VAL_SHAKE_SCORE_04
					OS_Printf( "ブレ[%d]　%d点加算\n", i, VAL_SHAKE_SCORE_04 );
			}
		}
		shake_score	/= 9;
		OS_Printf( "ブレ　%d点\n", shake_score );
	}

	return (pos_score + shake_score)/2;
}

//----------------------------------------------------------------------------
/**
 *	@brief	戻るボタンを押したかどうか
 *
 *	@param	void 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL TouchReturnBtn( void )
{	

	u32 x;
	u32 y;
	if( GFL_UI_TP_GetPointTrg( &x, &y) )
	{	
		if( ((u32)( x - MSGWND_RETURN_X*8) < (u32)(MSGWND_RETURN_W*8))
				&	((u32)( y - MSGWND_RETURN_Y*8) < (u32)(MSGWND_RETURN_H*8))
			){
			return TRUE;
		}

	}
	return FALSE;
}

//=============================================================================
/**
 *			ブレ計測
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ブレ計測	初期化
 *
 *	@param	SHAKE_SEARCH_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void	SHAKESEARCH_Init( SHAKE_SEARCH_WORK *p_wk )
{	
	GFL_STD_MemClear( p_wk, sizeof(SHAKE_SEARCH_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ブレ計測	破棄
 *
 *	@param	SHAKE_SEARCH_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void	SHAKESEARCH_Exit( SHAKE_SEARCH_WORK *p_wk )
{	
	GFL_STD_MemClear( p_wk, sizeof(SHAKE_SEARCH_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ブレ計測	計測メイン
 *
 *	@param	SHAKE_SEARCH_WORK *p_wk 
 *	@param	cp_rect　計測する範囲
 *
 *	@retval	TRUE	計測終了
 *	@retval	FALSE	計測中
 */
//-----------------------------------------------------------------------------
static BOOL	SHAKESEARCH_Main( SHAKE_SEARCH_WORK *p_wk, const GFL_RECT	*cp_rect )
{	

	//終了していたらTRUE
	if( p_wk->is_end )
	{	
		return TRUE;
	}

	//計測終了待ち
	if( p_wk->cnt++ > TOUCH_COUNTER_MAX )
	{	
		TP_GetRectCont( cp_rect, &p_wk->shake[TOUCH_COUNTER_SHAKE_MAX-1] );
		p_wk->is_end	= TRUE;
		return TRUE;
	}
	else
	{	
		//計測
		if( p_wk->shake_cnt++ > TOUCH_COUNTER_SHAKE_SYNC )
		{	
			p_wk->shake_cnt	= 0;
			TP_GetRectCont( cp_rect, &p_wk->shake[p_wk->shake_idx++] );
			GF_ASSERT_MSG( p_wk->shake_idx < TOUCH_COUNTER_SHAKE_MAX, 
					"ブレ計測インデックスエラーです %d\n" , p_wk->shake_idx );
		}
	}

	return FALSE;
}
 
#ifdef DEBUG_AURA_MSG
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
				NARC_font_small_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );	

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

	//ワイド文字をSTRBUFに変換
	p_strbuf	= DEBUGPRINT_CreateWideChar( cp_str, p_wk->heapID );

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
#endif //DEBUG_AURA_MSG

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
//#endif //DEBUG_AURA_MSG 
