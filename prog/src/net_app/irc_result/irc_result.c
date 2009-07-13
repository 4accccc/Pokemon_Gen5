//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_result_main.c
 *	@brief	赤外線ミニゲーム	結果表示
 *	@author	Toru=Nagihashi
 *	@data		2009.05.19
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

//	archive
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_irc_result.h"
#include "ircresult_gra.naix"

//	result
#include "net_app/irc_result.h"

#ifdef PM_DEBUG
//debug用
#include "system/net_err.h"	//VRAM退避用アドレスを貰うため
#include <wchar.h>					//wcslen,swprintf 
#endif

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#ifdef PM_DEBUG
//#define DEBUG_RESULT_MSG	//デバッグメッセージを出す
#endif //PM_DEBUG


//-------------------------------------
///	パレット
//=====================================
enum
{	
	// メイン画面BG
	RESULT_BG_PAL_M_00 = 0,// メモ用パレット256モードなのでパレット番号変更不可
	RESULT_BG_PAL_M_01,		// 使用していない
	RESULT_BG_PAL_M_02,		// 使用してない
	RESULT_BG_PAL_M_03,		// 使用してない
	RESULT_BG_PAL_M_04,		// 使用してない
	RESULT_BG_PAL_M_05,		// 使用してない
	RESULT_BG_PAL_M_06,		// 使用してない
	RESULT_BG_PAL_M_07,		// 使用してない
	RESULT_BG_PAL_M_08,		// 使用してない
	RESULT_BG_PAL_M_09,		// 使用してない
	RESULT_BG_PAL_M_10,		// 使用してない
	RESULT_BG_PAL_M_11,		// 使用してない
	RESULT_BG_PAL_M_12,		// 使用してない
	RESULT_BG_PAL_M_13,		// 使用してない
	RESULT_BG_PAL_M_14,		// フォント
	RESULT_BG_PAL_M_15,		// INFOWIN


	// サブ画面BG
	RESULT_BG_PAL_S_00 = 0,	//font
	RESULT_BG_PAL_S_01,		// 使用してない
	RESULT_BG_PAL_S_02,		// 使用してない
	RESULT_BG_PAL_S_03,		// 使用してない
	RESULT_BG_PAL_S_04,		// 使用してない
	RESULT_BG_PAL_S_05,		// 使用してない
	RESULT_BG_PAL_S_06,		// 使用してない
	RESULT_BG_PAL_S_07,		// 使用してない
	RESULT_BG_PAL_S_08,		// 使用してない
	RESULT_BG_PAL_S_09,		// 使用してない
	RESULT_BG_PAL_S_10,		// 使用してない
	RESULT_BG_PAL_S_11,		// 使用してない
	RESULT_BG_PAL_S_12,		// 使用してない
	RESULT_BG_PAL_S_13,		// 使用してない
	RESULT_BG_PAL_S_14,		// フォント
	RESULT_BG_PAL_S_15,		// 使用してない

	// メイン画面OBJ
	RESULT_OBJ_PAL_M_00 = 0,// タッチ反応OBJ
	RESULT_OBJ_PAL_M_01,		// 使用してない
	RESULT_OBJ_PAL_M_02,		// 使用してない
	RESULT_OBJ_PAL_M_03,		// 使用してない
	RESULT_OBJ_PAL_M_04,		// 使用してない
	RESULT_OBJ_PAL_M_05,		// 使用してない
	RESULT_OBJ_PAL_M_06,		// 使用してない
	RESULT_OBJ_PAL_M_07,		// 使用してない
	RESULT_OBJ_PAL_M_08,		// 使用してない
	RESULT_OBJ_PAL_M_09,		// 使用してない
	RESULT_OBJ_PAL_M_10,		// 使用してない
	RESULT_OBJ_PAL_M_11,		// 使用してない
	RESULT_OBJ_PAL_M_12,		// 使用してない
	RESULT_OBJ_PAL_M_13,		// 使用してない
	RESULT_OBJ_PAL_M_14,		// 使用してない
	RESULT_OBJ_PAL_M_15,		// 通信アイコン


	// サブ画面OBJ
	RESULT_OBJ_PAL_S_00 = 0,	//使用してない
	RESULT_OBJ_PAL_S_01,		// 使用してない
	RESULT_OBJ_PAL_S_02,		// 使用してない
	RESULT_OBJ_PAL_S_03,		// 使用してない
	RESULT_OBJ_PAL_S_04,		// 使用してない
	RESULT_OBJ_PAL_S_05,		// 使用してない
	RESULT_OBJ_PAL_S_06,		// 使用してない
	RESULT_OBJ_PAL_S_07,		// 使用してない
	RESULT_OBJ_PAL_S_08,		// 使用してない
	RESULT_OBJ_PAL_S_09,		// 使用してない
	RESULT_OBJ_PAL_S_10,		// 使用してない
	RESULT_OBJ_PAL_S_11,		// 使用してない
	RESULT_OBJ_PAL_S_12,		// 使用してない
	RESULT_OBJ_PAL_S_13,		// 使用してない
	RESULT_OBJ_PAL_S_14,		// 使用してない
	RESULT_OBJ_PAL_S_15,		// 使用してない
};

//-------------------------------------
///	情報バー
//=====================================
#define INFOWIN_PLT_NO		(RESULT_BG_PAL_M_15)
#define INFOWIN_BG_FRAME	(GFL_BG_FRAME0_M)

//-------------------------------------
///	文字
//=====================================
#define TEXTSTR_PLT_NO				(RESULT_BG_PAL_S_14)
#define TEXTSTR_BUFFER_LENGTH	(255)

//-------------------------------------
///	位置
//=====================================
#define	MSGWND_MAIN_X	(8)
#define	MSGWND_MAIN_Y	(3)
#define	MSGWND_MAIN_W	(16)
#define	MSGWND_MAIN_H	(14)

#define	MSGWND_SUB_X	(4)
#define	MSGWND_SUB_Y	(6)
#define	MSGWND_SUB_W	(23)
#define	MSGWND_SUB_H	(11)

#define	SCALE_MIN			(0x310c0)//(0x11)

//-------------------------------------
///	数
//=====================================
#define CLWK_SMALL_HEART_MAX	(30)
#define CLWK_NUMBER_MAX				(3)

//-------------------------------------
///	カウンタ
//=====================================
#define SEQFUNC_DECIDEHEART_WAIT	(60)
#define HEART_SCALSE_WAIT	(180)

//-------------------------------------
///		MSG_FONT
//=====================================
typedef enum
{	
	MSG_FONT_TYPE_LARGE,
	MSG_FONT_TYPE_SMALL,
}MSG_FONT_TYPE;

//-------------------------------------
///	OBJ登録ID
//=====================================
enum 
{
	OBJREGID_OBJ_PLT,
	OBJREGID_BIG_HEART_CHR,
	OBJREGID_BIG_HEART_CEL,
	OBJREGID_SMALL_HEART_CHR,
	OBJREGID_SMALL_HEART_CEL,
	OBJREGID_NUMBER_CHR,
	OBJREGID_NUMBER_CEL,

	OBJREGID_MAX
};
//-------------------------------------
///	CLWKID
//=====================================
typedef enum
{	
	CLWKID_BIG_HEART,
	CLWKID_SMALL_HEART_TOP,
	CLWKID_SMALL_HEART_END	= CLWKID_SMALL_HEART_TOP+CLWK_SMALL_HEART_MAX,
	CLWKID_NUMBER_TOP,
	CLWKID_NUMBER_END	= CLWKID_NUMBER_TOP+CLWK_NUMBER_MAX,
	
	CLWKID_MAX,
}CLWKID;
//-------------------------------------
///	MSGWNDID
//=====================================
typedef enum
{
//	MSGWNDID_MAIN,
	MSGWNDID_SUB,

	MSGWNDID_MAX
} MSGWNDID;

//-------------------------------------
///	OBJの設定値
//=====================================
#define OBJ_BIG_HEART_MAX_SIZE	(	FX32_CONST(1.5) )
#define OBJ_BIG_HEART_DEF_SIZE	( FX32_CONST(0.6) )
#define OBJ_BIG_HEART_MIN_SIZE	( FX32_CONST(0.3) )
#define OBJ_BIG_HEART_SUB_SIZE	( OBJ_BIG_HEART_MAX_SIZE-OBJ_BIG_HEART_MIN_SIZE )

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
	int dummy;
} GRAPHIC_BG_WORK;

//-------------------------------------
///	OBJ関係
//=====================================
typedef struct 
{
	GFL_CLUNIT	*p_clunit;
	u32					reg_id[OBJREGID_MAX];
	GFL_CLWK		*p_clwk[CLWKID_MAX];
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
} MSGWND_WORK;

//-------------------------------------
///	３次曲線CatmullROm曲線版
//		始点、終点、制御点０，１を与えて
//		点上を通る曲線。
//=====================================
typedef struct {
	fx32 now;
	fx32 start;
	fx32 end;
	fx32 ctrl1;
	fx32 ctrl2;
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_CATMULLROM_WORK;

//-------------------------------------
///	揺れる処理
//=====================================
typedef struct {
	fx32				now;
	fx32				start;
	fx32				end;
	fx32				shake_min;
	fx32				shake_max;
	u16					shake_cnt;
	u16					shake_cnt_max;
	u16					sync_now;
	u16					sync_max;
	u32					seq;
} PROGVAL_SHAKE_WORK;

//-------------------------------------
///	竜巻処理
//=====================================
typedef struct {
	VecFx32			now_pos;
	VecFx32			start_pos;
	u16					r_now;
	u16					r_max;
	u16					add_angle;
	u16					angle;
	u16					sync_now;
	u16					sync_max;
} PROGVAL_TORNADO_WORK;

//-------------------------------------
//	等速直線運動動作
//=====================================
typedef struct {
	fx32 now_val;		//現在の値
	fx32 start_val;		//開始の値
	fx32 end_val;		//終了の値
	fx32 add_val;		//加算値(誤差をださないようにここだけfx)
	s32 sync_now;		//現在のシンク
	s32 sync_max;		//シンク最大数
} PROGVAL_VELOCITY_WORK;

//-------------------------------------
///	OBJ数字
//=====================================
typedef struct {
	GFL_CLWK *p_clwk[CLWK_NUMBER_MAX];
	u16 sync;
	u16 sync_max;
	u8 number;
	u8 dummy;
} OBJNUBER_WORK;


#ifdef DEBUG_RESULT_MSG
//-------------------------------------
///	デバッグプリント用画面
//=====================================
typedef struct
{
	GFL_BMP_DATA *p_bmp;
	GFL_FONT*			p_font;

	BOOL	is_now_save;
	u8		frm;

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
#endif //DEBUG_RESULT_MSG

//-------------------------------------
///	結果表示メインワーク
//=====================================
typedef struct _RESULT_MAIN_WORK RESULT_MAIN_WORK;
typedef void (*SEQ_FUNCTION)( RESULT_MAIN_WORK *p_wk, u16 *p_seq );
struct _RESULT_MAIN_WORK
{
	//グラフィックモジュール
	GRAPHIC_WORK		grp;
	MSG_WORK				msg;
	MSGWND_WORK			msgwnd[MSGWNDID_MAX];

	OBJNUBER_WORK		number;

	//シーケンス管理
	SEQ_FUNCTION		seq_function;
	u16		seq;
	BOOL	is_end;

	//その他汎用
	u32		cnt;

	//動き制御
	PROGVAL_CATMULLROM_WORK	heart_size;
	PROGVAL_SHAKE_WORK			shake;
	PROGVAL_TORNADO_WORK		tornado[CLWK_SMALL_HEART_MAX];
	PROGVAL_VELOCITY_WORK		memo_scale;
	PROGVAL_VELOCITY_WORK		memo_rot;
	PROGVAL_VELOCITY_WORK		scale;

	//引数
	IRC_RESULT_PARAM	*p_param;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT IRC_RESULT_PROC_Init(
		GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_RESULT_PROC_Exit(
		GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_RESULT_PROC_Main(
		GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
//grp
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
static GFL_CLWK* GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id );
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//BG
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
//obj
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, 
		const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID );
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk );
static GFL_CLWK* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, CLWKID id );
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
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, MSG_WORK *p_msg );
static void MSGWND_Print( MSGWND_WORK* p_wk,
		const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y );
static void MSGWND_PrintCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID );
static void MSGWND_PrintNumberCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, 
		u32 strID, u16 number, u16 buff_id );
static void MSGWND_PrintBothNameCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, const MYSTATUS *cp_my, const MYSTATUS *cp_you );
static void MSGWND_Clear( MSGWND_WORK* p_wk );
//SEQ
static void SEQ_Change( RESULT_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function );
static void SEQ_End( RESULT_MAIN_WORK *p_wk );
//SEQ_FUNC
static void SEQFUNC_StartGame( RESULT_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_DecideHeart( RESULT_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_HeartEffect( RESULT_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_Memo( RESULT_MAIN_WORK *p_wk, u16 *p_seq );
//OBJNUBER
static void OBJNUBER_Init( OBJNUBER_WORK *p_wk, const GRAPHIC_WORK *cp_grp );
static void OBJNUBER_Start( OBJNUBER_WORK *p_wk, int number, int sync );
static BOOL OBJNUBER_Main( OBJNUBER_WORK *p_wk );
static void ObjNumber_SetNumber( OBJNUBER_WORK *p_wk, int number );
//汎用
static BOOL TP_GetDiamondTrg( const GFL_POINT *cp_diamond, GFL_POINT *p_trg );
static void DEBUGRESULT_PRINT_UpDate( RESULT_MAIN_WORK *p_wk );
//汎用的な動作
static void PROGVAL_CATMULLROM_Init( PROGVAL_CATMULLROM_WORK* p_wk, fx32 start, fx32 ctrl1, fx32 ctrl2, fx32 end, int sync );
static BOOL PROGVAL_CATMULLROM_Main( PROGVAL_CATMULLROM_WORK* p_wk );
static void PROGVAL_SHAKE_Init( PROGVAL_SHAKE_WORK *p_wk, fx32 start, fx32 end, fx32 shake_min, fx32 shake_max, u16 shake_cnt, u16 sync );
static BOOL PROGVAL_SHAKE_Main( PROGVAL_SHAKE_WORK *p_wk );
static int Progval_Velocity( int start, int end, int sync_now, int sync_max );
static void PROGVAL_TORNADO_Init( PROGVAL_TORNADO_WORK *p_wk, const VecFx32 *cp_start_pos, u16 r_max, u16 add_angle, int sync );
static BOOL PROGVAL_TORNADO_Main( PROGVAL_TORNADO_WORK *p_wk );
static void PROGVAL_VEL_Init( PROGVAL_VELOCITY_WORK* p_wk, fx32 start, fx32 end, s32 sync );
static BOOL	PROGVAL_VEL_Main( PROGVAL_VELOCITY_WORK* p_wk );
//ぶったいの動作
#if 0
static void BigHeart_InitScale( RESULT_MAIN_WORK *p_wk, u8 score );
static BOOL BigHeart_MainScale( RESULT_MAIN_WORK *p_wk );
#else
static void BigHeart_InitScale( RESULT_MAIN_WORK *p_wk, u8 score, u8 sync );
static BOOL BigHeart_MainScale( RESULT_MAIN_WORK *p_wk );
#endif
static void BigHeart_InitShake( RESULT_MAIN_WORK *p_wk );
static BOOL BigHeart_MainShake( RESULT_MAIN_WORK *p_wk );
static void SmallHeart_InitTornado( RESULT_MAIN_WORK *p_wk );
static BOOL SmallHeart_MainTornado( RESULT_MAIN_WORK *p_wk );

#if 0
static void Memo_InitRot( RESULT_MAIN_WORK *p_wk );
static BOOL Memo_MainRot( RESULT_MAIN_WORK *p_wk );
#endif
//DEBUG_PRINT
#ifdef DEBUG_RESULT_MSG
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
#endif //DEBUG_RESULT_MSG

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	結果表示用プロックデータ
//=====================================
const GFL_PROC_DATA IrcResult_ProcData	= 
{	
	IRC_RESULT_PROC_Init,
	IRC_RESULT_PROC_Main,
	IRC_RESULT_PROC_Exit,
};
//-------------------------------------
///	BGシステム
//=====================================
typedef enum 
{
	GRAPHIC_BG_FRAME_M_INFOWIN,
	GRAPHIC_BG_FRAME_M_TEXT	= GRAPHIC_BG_FRAME_M_INFOWIN,
	GRAPHIC_BG_FRAME_M_BACK,
	GRAPHIC_BG_FRAME_M_HEART,
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
	// GRAPHIC_BG_FRAME_M_BACK
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_M_HEART
	{
		0, 0, 0x1000, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
		GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
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
static const u32 sc_bgmode[ GRAPHIC_BG_FRAME_MAX ] =
{	
	GFL_BG_MODE_TEXT,GFL_BG_MODE_TEXT,GFL_BG_MODE_AFFINE,GFL_BG_MODE_TEXT,GFL_BG_MODE_TEXT
};

//=============================================================================
/**
 *			PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	結果表示	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_RESULT_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	RESULT_MAIN_WORK	*p_wk;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCRESULT, 0x20000 );
	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(RESULT_MAIN_WORK), HEAPID_IRCRESULT );
	GFL_STD_MemClear( p_wk, sizeof(RESULT_MAIN_WORK) );
	p_wk->p_param	= p_param;

	//モジュール初期化
	GRAPHIC_Init( &p_wk->grp, HEAPID_IRCRESULT );
	MSG_Init( &p_wk->msg, MSG_FONT_TYPE_LARGE, HEAPID_IRCRESULT );
	{
		GAME_COMM_SYS_PTR comm	= NULL;
		if( p_wk->p_param->p_gamesys )
		{	
			comm	= GAMESYSTEM_GetGameCommSysPtr(p_wk->p_param->p_gamesys);
		}
		INFOWIN_Init( INFOWIN_BG_FRAME, INFOWIN_PLT_NO, comm, HEAPID_IRCRESULT );
	}

/*	MSGWND_Init( &p_wk->msgwnd[MSGWNDID_MAIN], sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT],
			MSGWND_MAIN_X, MSGWND_MAIN_Y, MSGWND_MAIN_W, MSGWND_MAIN_H, HEAPID_IRCRESULT );
*/	MSGWND_Init( &p_wk->msgwnd[MSGWNDID_SUB], sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TEXT],
			MSGWND_SUB_X, MSGWND_SUB_Y, MSGWND_SUB_W, MSGWND_SUB_H, HEAPID_IRCRESULT );

	OBJNUBER_Init( &p_wk->number, &p_wk->grp );

	//初期メッセージ
	if( p_wk->p_param->p_gamesys )
	{	
		PLAYER_WORK *p_player;
		MYSTATUS *p_status;
		p_player	= GAMESYSTEM_GetMyPlayerWork( p_wk->p_param->p_gamesys );
		p_status	= &p_player->mystatus;
		MSGWND_PrintBothNameCenter( &p_wk->msgwnd[MSGWNDID_SUB], &p_wk->msg, 
				RESULT_STR_000, p_status, p_wk->p_param->p_you_status );
	}
	else	
	{	
		MSGWND_PrintCenter( &p_wk->msgwnd[MSGWNDID_SUB], &p_wk->msg, RESULT_STR_000 );
	}

	//デバッグ
	DEBUGPRINT_Init( sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_BACK], FALSE, HEAPID_IRCRESULT );
	DEBUGPRINT_Open();

	//リズムシーンセット
	COMPATIBLE_IRC_SetScene( p_wk->p_param->p_irc, COMPATIBLE_SCENE_RESULT );

	SEQ_Change( p_wk, SEQFUNC_StartGame );
	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	結果表示	メインプロセス破棄処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_RESULT_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	RESULT_MAIN_WORK	*p_wk;

	p_wk	= p_work;

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

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );
	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_IRCRESULT );
	
	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	結果表示	メインプロセスメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_RESULT_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
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

	RESULT_MAIN_WORK	*p_wk;
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

		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
		{	
			SEQ_Change( p_wk, SEQFUNC_StartGame );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
		{
			SEQ_End( p_wk );
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
		GF_ASSERT_MSG( 0, "IRC_RESULT_PROC_MainのSEQエラー %d", *p_seq );
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


	//ここでチェックしても遅れているのは相手なので進めることはしない
#if 0
	if( !COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) )
	{	
		SEQ_End( p_wk );
	}
#endif

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
		GX_VRAM_BG_256_AD,					// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,				// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,					// メイン2DエンジンのOBJ
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
			GX_DISPMODE_GRAPHICS,GX_BGMODE_2,GX_BGMODE_0,GX_BG0_AS_2D
		};	
		GFL_BG_SetBGMode( &sc_bg_sys_header );
	}

	//ＢＧコントロール設定
	{
		int i;

		for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
		{
			GFL_BG_SetBGControl( sc_bgcnt_frame[i], &sc_bgcnt_data[i], sc_bgmode[i] );
			GFL_BG_ClearFrame( sc_bgcnt_frame[i] );
			GFL_BG_SetVisible( sc_bgcnt_frame[i], VISIBLE_ON );
		}
	}

	//読み込み設定
	{	
		ARCHANDLE *p_handle;

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_IRCRESULT_GRAPHIC, heapID );


		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_ircresult_gra_result_bg_NCLR,
				PALTYPE_MAIN_BG, RESULT_BG_PAL_M_00*0x20, 0x20, heapID );
	
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_ircresult_gra_result_bg_back_NCGR,
				sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_BACK], 0, 0, FALSE, heapID );

		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_ircresult_gra_result_bg_back_NSCR,
				sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_BACK], 0, 0, FALSE, heapID );



		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_ircresult_gra_result_bg_heart256_NCLR,
				PALTYPE_MAIN_BG, RESULT_BG_PAL_M_00*0x20, 0x20, heapID );
	
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_ircresult_gra_result_bg_heart_NCGR,
				sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_HEART], 0, 0, FALSE, heapID );

		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_ircresult_gra_result_bg_heart_NSCR,
				sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_HEART], 0, 0, FALSE, heapID );

		GFL_ARC_CloseDataHandle( p_handle );

		GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], FALSE );
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
	int i;

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

		p_handle	= GFL_ARC_OpenDataHandle( ARCID_IRCRESULT_GRAPHIC, heapID );

		p_wk->reg_id[OBJREGID_OBJ_PLT]	= GFL_CLGRP_PLTT_Register( p_handle, 
				NARC_ircresult_gra_result_obj_NCLR, CLSYS_DRAW_MAIN, RESULT_OBJ_PAL_M_00*0x20, heapID );

		p_wk->reg_id[OBJREGID_BIG_HEART_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_ircresult_gra_result_obj_big_heart_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

		p_wk->reg_id[OBJREGID_BIG_HEART_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_ircresult_gra_result_obj_big_heart_NCER, 
				NARC_ircresult_gra_result_obj_big_heart_NANR, heapID );

		p_wk->reg_id[OBJREGID_SMALL_HEART_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_ircresult_gra_result_obj_small_heart_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

		p_wk->reg_id[OBJREGID_SMALL_HEART_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_ircresult_gra_result_obj_small_heart_NCER, 
				NARC_ircresult_gra_result_obj_small_heart_NANR, heapID );

		p_wk->reg_id[OBJREGID_NUMBER_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_ircresult_gra_result_obj_number_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

		p_wk->reg_id[OBJREGID_NUMBER_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_ircresult_gra_result_obj_number_NCER, 
				NARC_ircresult_gra_result_obj_number_NANR, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK作成
	{	
		int i;
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

		p_wk->p_clwk[CLWKID_BIG_HEART]	= GFL_CLACT_WK_Create( p_wk->p_clunit, 
				p_wk->reg_id[OBJREGID_BIG_HEART_CHR],
				p_wk->reg_id[OBJREGID_OBJ_PLT],
				p_wk->reg_id[OBJREGID_BIG_HEART_CEL],
				&cldata,
				CLSYS_DEFREND_MAIN,
				heapID
				);

		for( i = CLWKID_SMALL_HEART_TOP; i < CLWKID_SMALL_HEART_END; i++  )
		{	
			p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit, 
				p_wk->reg_id[OBJREGID_SMALL_HEART_CHR],
				p_wk->reg_id[OBJREGID_OBJ_PLT],
				p_wk->reg_id[OBJREGID_SMALL_HEART_CEL],
				&cldata,
				CLSYS_DEFREND_MAIN,
				heapID
				);

		}

		for( i = CLWKID_NUMBER_TOP; i < CLWKID_NUMBER_END; i++  )
		{	
			p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit, 
				p_wk->reg_id[OBJREGID_NUMBER_CHR],
				p_wk->reg_id[OBJREGID_OBJ_PLT],
				p_wk->reg_id[OBJREGID_NUMBER_CEL],
				&cldata,
				CLSYS_DEFREND_MAIN,
				heapID
				);

		}

		{
			GFL_CLACTPOS	pos;
			GFL_CLSCALE		scale;
			pos.x	= 128;
			pos.y	= 96;
			GFL_CLACT_WK_SetPos( p_wk->p_clwk[CLWKID_BIG_HEART], &pos, 0 );

			scale.x	= OBJ_BIG_HEART_DEF_SIZE;
			scale.y	= OBJ_BIG_HEART_DEF_SIZE;
			GFL_CLACT_WK_SetScale( p_wk->p_clwk[CLWKID_BIG_HEART], &scale );

			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CLWKID_BIG_HEART], FALSE );
		}

	}

	//CLWKの設定
	{	
		int i;
		GFL_CLACTPOS	clpos	= {	
			128, 92
		};

		///大きいハートを拡大縮小フラグをつけ、中央に配置
		GFL_CLACT_WK_SetAffineParam( p_wk->p_clwk[CLWKID_BIG_HEART], CLSYS_AFFINETYPE_DOUBLE );
		GFL_CLACT_WK_SetPos( p_wk->p_clwk[CLWKID_BIG_HEART], &clpos, 0 );
		GFL_CLACT_WK_SetBgPri( p_wk->p_clwk[CLWKID_BIG_HEART], 2 );
		GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk[CLWKID_BIG_HEART], 1 );

		//小さいハートは表示OFF
		for( i = CLWKID_SMALL_HEART_TOP; i < CLWKID_SMALL_HEART_END; i++  )
		{	
			clpos.x	= 128;
			clpos.y	= 64;
			GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, 0 );
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
			GFL_CLACT_WK_SetBgPri( p_wk->p_clwk[i], 2 );
			GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk[i], 0 );
		}

		for( i = CLWKID_NUMBER_TOP; i < CLWKID_NUMBER_END; i++  )
		{	
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
			GFL_CLACT_WK_SetBgPri( p_wk->p_clwk[i], 0 );
			GFL_CLACT_WK_SetSoftPri( p_wk->p_clwk[i], 0 );
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
		GFL_CLGRP_PLTT_Release( p_wk->reg_id[OBJREGID_OBJ_PLT] );
		GFL_CLGRP_CGR_Release( p_wk->reg_id[OBJREGID_BIG_HEART_CHR] );
		GFL_CLGRP_CELLANIM_Release( p_wk->reg_id[OBJREGID_BIG_HEART_CEL] );
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
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_irc_result_dat, heapID );

	p_wk->p_wordset	= WORDSET_Create( heapID );

	{	
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,  RESULT_BG_PAL_S_14*0x20, 0x20, heapID );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, RESULT_BG_PAL_M_14*0x20, 0x20, heapID );
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], GX_RGB(31,31,31) );
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT], GX_RGB(31,31,31) );
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
static void MSGWND_PrintNumberCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id )
{
	const GFL_MSGDATA* cp_msgdata;
	WORDSET *p_wordset;
	
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

	//モジュール取得
	p_wordset		= MSG_GetWordSet( cp_msg );
	cp_msgdata	= MSG_GetMsgDataConst( cp_msg );

	//数値をワードセットに登録
	WORDSET_RegisterNumber(	p_wordset, buff_id, number, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

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
		u16 x, y;

		p_que		= MSG_GetPrintQue( cp_msg );
		p_font	= MSG_GetFont( cp_msg );

		//センター位置計算
		x	= GFL_BMPWIN_GetSizeX( p_wk->p_bmpwin )*4;
		y	= GFL_BMPWIN_GetSizeY( p_wk->p_bmpwin )*4;
		x	-= PRINTSYS_GetStrWidth( p_wk->p_strbuf, p_font, 0 )/2;
		y	-= PRINTSYS_GetStrHeight( p_wk->p_strbuf, p_font )/2;

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
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintBothNameCenter( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, const MYSTATUS *cp_my, const MYSTATUS *cp_you )
{	
	const GFL_MSGDATA* cp_msgdata;
	WORDSET *p_wordset;
	u16 x, y;
	
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

	//モジュール取得
	p_wordset		= MSG_GetWordSet( cp_msg );
	cp_msgdata	= MSG_GetMsgDataConst( cp_msg );

	//数値をワードセットに登録
	WORDSET_RegisterPlayerName( p_wordset, 0, cp_you );
	WORDSET_RegisterPlayerName( p_wordset, 1, cp_my );

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

		//センター位置計算
		x	= GFL_BMPWIN_GetSizeX( p_wk->p_bmpwin )*4;
		y	= GFL_BMPWIN_GetSizeY( p_wk->p_bmpwin )*4;
		x	-= PRINTSYS_GetStrWidth( p_wk->p_strbuf, p_font, 0 )/2;
		y	-= PRINTSYS_GetStrHeight( p_wk->p_strbuf, p_font )/2;

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
static void PROGVAL_CATMULLROM_Init( PROGVAL_CATMULLROM_WORK* p_wk, fx32 start, fx32 ctrl1, fx32 ctrl2, fx32 end, int sync )
{
	p_wk->now				=	start;
	p_wk->start			=	start;
	p_wk->ctrl1			= ctrl1;
	p_wk->ctrl2			= ctrl2;
	p_wk->end				= end;
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
BOOL PROGVAL_CATMULLROM_Main( PROGVAL_CATMULLROM_WORK* p_wk )
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

			p_wk->now	= FX_Mul(p_wk->start, mp1) + FX_Mul(p_wk->ctrl1, mp2)
				+ FX_Mul(p_wk->ctrl2, mp3);

		//	P0からP1までの曲線
		}else if( p_wk->sync_now < sync2div3 ) {
			t1	= FX_Div( FX32_CONST(p_wk->sync_now-sync1div3), FX32_CONST(sync2div3-sync1div3));
			t2	= FX_Mul( t1, t1 );	//	t*t
			t3	= FX_Mul( t2, t1 );	//	t*t*t

			mp0	=(  -t3 + 2*t2 - t1) / 2;
			mp1	=( 3*t3 - 5*t2 + 2*FX32_ONE) / 2;
			mp2	=(-3*t3 + 4*t2 + t1) / 2;
			mp3	=(   t3 -   t2 ) / 2;

			p_wk->now	= FX_Mul(p_wk->start, mp0) + FX_Mul(p_wk->ctrl1, mp1)
				+ FX_Mul(p_wk->ctrl2, mp2) + FX_Mul(p_wk->end,   mp3);

		//	P1から終点までの曲線
		}else if( sync2div3 <= p_wk->sync_now ) {
			t1	= FX_Div( FX32_CONST(p_wk->sync_now-sync2div3), FX32_CONST(p_wk->sync_max-sync2div3));
			t2	= FX_Mul( t1, t1 );	//	t*t
			t3	= FX_Mul( t2, t1 );	//	t*t*t

			mp0	=(   t2 - t1) /2;
			mp1	=(-2*t2		   + 2*FX32_ONE) / 2;
			mp2	=(   t2 + t1) / 2;
			mp3	=0;	

			p_wk->now	= FX_Mul(p_wk->ctrl1, mp0)
				+ FX_Mul(p_wk->ctrl2, mp1) + FX_Mul(p_wk->end, mp2);
		}

		p_wk->sync_now++;

		return FALSE;
	}else{
		p_wk->now	= p_wk->end;
		return TRUE;
	}

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	揺れる処理	初期化
 *
 *	@param	PROGVAL_SHAKE_WORK_VEC *p_wk	ワーク
 *	@param	VecFx32 *cp_start				開始点
 *	@param	shake_min						揺れ幅	最小
 *	@param	shake_max						揺れ幅　最大
 *	@param	space							揺れるまでの幅
 *	@param	sync							揺れるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void PROGVAL_SHAKE_Init( PROGVAL_SHAKE_WORK *p_wk, fx32 start, fx32 end, fx32 shake_min, fx32 shake_max, u16 shake_cnt, u16 sync )
{	
	GFL_STD_MemClear( p_wk, sizeof(PROGVAL_SHAKE_WORK) );
	p_wk->now	= start;
	p_wk->start	= start;
	p_wk->end		= end;
	p_wk->shake_min	= shake_min;
	p_wk->shake_max	= shake_max;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
	p_wk->shake_cnt	= 0;
	p_wk->shake_cnt_max	= shake_cnt;
	p_wk->seq		= 0;

}
//----------------------------------------------------------------------------
/**
 *	@brief	揺れる処理	メイン処理
 *
 *	@param	PROGVAL_SHAKE_WORK *p_wk	ワーク
 *
 *	@return	TRUEならば終了　FALSEは継続
 */
//-----------------------------------------------------------------------------
static BOOL PROGVAL_SHAKE_Main( PROGVAL_SHAKE_WORK *p_wk )
{	
	if( p_wk->sync_now < p_wk->sync_max )
	{	
		int sync_now;
		int sync_max;
		int sync_div	= p_wk->sync_max / (p_wk->shake_cnt_max*2+1);	//+1は最後の分

		switch( p_wk->seq ) 
		{
		//行き
		case 0:
			sync_now	= p_wk->sync_now;
			sync_max	= sync_div;
			p_wk->now	= Progval_Velocity( p_wk->start, p_wk->shake_min, sync_now, sync_max );
			if( sync_now > sync_max )
			{	
				p_wk->seq++;
			}
			break;
		//揺れ開始
		case 1:
			sync_now	= p_wk->sync_now - sync_div * (p_wk->shake_cnt*2+1);
			sync_max	= sync_div;
			p_wk->now	= Progval_Velocity( p_wk->shake_min, p_wk->shake_max, sync_now, sync_max );
			if( sync_now > sync_max )
			{
					p_wk->seq++;
			}
			break;
		//ゆれ折り返し
		case 2:
			sync_now	= p_wk->sync_now - sync_div * (p_wk->shake_cnt*2+2);
			sync_max	= sync_div;
			p_wk->now	= Progval_Velocity( p_wk->shake_max, p_wk->shake_min, sync_now, sync_max );
			if( sync_now > sync_max )
			{
				p_wk->shake_cnt++;

				if( p_wk->shake_cnt+1 > p_wk->shake_cnt_max )
				{	
					p_wk->seq	= 3;
				}
				else
				{	
					p_wk->seq	= 1;
				}
			}
			break;
		//終わり
		case 3:
			sync_now	= p_wk->sync_now - sync_div * (p_wk->shake_cnt*2+3);
			sync_max	= sync_div;
			p_wk->now	= Progval_Velocity( p_wk->shake_max, p_wk->end, sync_now, sync_max );
			if( sync_now > sync_max )
			{	
				p_wk->seq++;
			}
			break;
		}

		p_wk->sync_now++;
	}
	else
	{	
		p_wk->now	= p_wk->end;
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	直線運動方程式
 *
 *	@param	int start	開始値
 *	@param	end				終了値
 *	@param	sync_now	現在のシンク
 *	@param	sync_max	終了のシンク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static int Progval_Velocity( int start, int end, int sync_now, int sync_max )
{	
	return start + ( end - start ) * sync_now / sync_max ; 
}
//----------------------------------------------------------------------------
/**
 *	@brief	竜巻状に回転	初期化
 *
 *	@param	PROGVAL_TORNADO_WORK *p_wk	ワーク
 *	@param	*cp_start_pos								開始座標
 *	@param	r_max												回転する円の半径最大														
 *	@param	sync												回転する円の半径が最大になるまでの時間
 *
 */
//-----------------------------------------------------------------------------
static void PROGVAL_TORNADO_Init( PROGVAL_TORNADO_WORK *p_wk, const VecFx32 *cp_start_pos, u16 r_max, u16 add_angle, int sync )
{	
	GFL_STD_MemClear( p_wk, sizeof(PROGVAL_TORNADO_WORK));
	p_wk->now_pos		= *cp_start_pos;
	p_wk->start_pos	= *cp_start_pos;
	p_wk->r_now			= 0;
	p_wk->r_max			= r_max;
	p_wk->add_angle	= add_angle;
	p_wk->angle			= 0;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
}
//----------------------------------------------------------------------------
/**
 *	@brief	竜巻状に回転	メイン処理
 *
 *	@param	PROGVAL_TORNADO_WORK *p_wk 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL PROGVAL_TORNADO_Main( PROGVAL_TORNADO_WORK *p_wk )
{	
	p_wk->now_pos.x	= p_wk->start_pos.x + ((p_wk->r_now * FX_CosIdx( p_wk->angle ))>>FX32_SHIFT);
	p_wk->now_pos.y	= p_wk->start_pos.y + ((p_wk->r_now * FX_SinIdx( p_wk->angle ))>>FX32_SHIFT);
	p_wk->angle	+= p_wk->add_angle;

	if( p_wk->sync_now < p_wk->sync_max )
	{	
		p_wk->r_now	= p_wk->r_max * p_wk->sync_now / p_wk->sync_max;
		p_wk->sync_now++;
	}
	else
	{	
		return TRUE;
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
static void PROGVAL_VEL_Init( PROGVAL_VELOCITY_WORK* p_wk, fx32 start, fx32 end, s32 sync )
{
	p_wk->now_val	= start;
	p_wk->start_val	= start;
	p_wk->end_val	= end;
	p_wk->sync_max	= sync;
	if( sync ) {
		p_wk->add_val	= (p_wk->end_val - p_wk->start_val) / sync;
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
static BOOL PROGVAL_VEL_Main( PROGVAL_VELOCITY_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		p_wk->sync_now++;
		p_wk->now_val	+= p_wk->add_val;
		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_val;
		return TRUE;
	}
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
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *	@param	seq_function					シーケンス関数
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Change( RESULT_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROC終了
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( RESULT_MAIN_WORK *p_wk )
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
 *	@param	RESULT_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_StartGame( RESULT_MAIN_WORK *p_wk, u16 *p_seq )
{	
	SEQ_Change( p_wk, SEQFUNC_DecideHeart );
}
//----------------------------------------------------------------------------
/**
 *	@brief	なかよしハート決定
 *
 *	@param	RESULT_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DecideHeart( RESULT_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
		SEQ_WAIT,
		SEQ_HEART_SCALSE_INIT,
		SEQ_HEART_SCALSE_MAIN,
		SEQ_HEART_SCALSE_EXIT,
		SEQ_NEXTPROC,
	};

	switch( *p_seq )
	{	
	case SEQ_WAIT:
		if( p_wk->cnt++ >SEQFUNC_DECIDEHEART_WAIT )
		{	
			p_wk->cnt	= 0;
			*p_seq	= SEQ_HEART_SCALSE_INIT;
		}
		break;

	case SEQ_HEART_SCALSE_INIT:
		BigHeart_InitScale( p_wk, p_wk->p_param->score, 180 );
		OBJNUBER_Start( &p_wk->number, p_wk->p_param->score, 180 );
		OS_Printf( "○相性　%d点！！", p_wk->p_param->score );
		*p_seq	= SEQ_HEART_SCALSE_MAIN;
		break;

	case SEQ_HEART_SCALSE_MAIN:
		{	
			u8 ret	= 0;
			ret	+= OBJNUBER_Main( &p_wk->number );
			ret	+= BigHeart_MainScale( p_wk );

			if(ret==2)
			{	
				*p_seq	= SEQ_HEART_SCALSE_EXIT;
			}
		}
		break;

	case SEQ_HEART_SCALSE_EXIT:
		MSGWND_PrintNumberCenter( &p_wk->msgwnd[MSGWNDID_SUB], &p_wk->msg, RESULT_STR_001,
				p_wk->p_param->score, 2 );
		*p_seq	= SEQ_NEXTPROC;
		break;

	case SEQ_NEXTPROC:
		//SEQ_Change( p_wk, SEQFUNC_HeartEffect );
		if( GFL_UI_TP_GetTrg() )
		{	
			SEQ_End( p_wk );
		}
		break;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ハートの演出
 *
 *	@param	RESULT_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_HeartEffect( RESULT_MAIN_WORK *p_wk, u16 *p_seq )
{
	enum
	{	
		SEQ_INIT,
		SEQ_TOUCH,
		SEQ_HEART_SHAKE_INIT,
		SEQ_HEART_SHAKE_MAIN,
		SEQ_HEART_SHAKE_EXIT,
		SEQ_WAIT,
		SEQ_HEART_SHOWER_INIT,
		SEQ_HEART_SHOWER_MAIN,
		SEQ_HEART_SHOWER_EXIT,
		SEQ_NEXTPROC,
	};

	switch( *p_seq )
	{	
	case SEQ_INIT:
		MSGWND_PrintCenter( &p_wk->msgwnd[MSGWNDID_SUB], &p_wk->msg, RESULT_STR_001 );
		*p_seq	= SEQ_TOUCH;
		break;

	case SEQ_TOUCH:
		if( GFL_UI_TP_GetTrg() )
		{	
			*p_seq	= SEQ_HEART_SHAKE_INIT;
		}
		break;

	case SEQ_HEART_SHAKE_INIT:
		BigHeart_InitShake( p_wk );
		SmallHeart_InitTornado( p_wk );
		*p_seq	= SEQ_HEART_SHAKE_MAIN;
		break;

	case SEQ_HEART_SHAKE_MAIN:
		if( BigHeart_MainShake( p_wk ) )
		{	
			*p_seq	= SEQ_HEART_SHAKE_EXIT;
		}

		if( p_wk->cnt ++ > 30 )
		{	
			p_wk->cnt	= 0;
			*p_seq	= SEQ_HEART_SHOWER_MAIN;
		}
		break;

	case SEQ_HEART_SHAKE_EXIT:
		*p_seq	= SEQ_WAIT;
		break;

	case SEQ_WAIT:
		if( p_wk->cnt ++ > 30 )
		{	
			p_wk->cnt	= 0;
			*p_seq	= SEQ_HEART_SHOWER_INIT;
		}
		break;

	case SEQ_HEART_SHOWER_INIT:
		SmallHeart_InitTornado( p_wk );
		*p_seq	= SEQ_HEART_SHOWER_MAIN;
		break;

	case SEQ_HEART_SHOWER_MAIN:
		BigHeart_MainShake( p_wk );
		if( SmallHeart_MainTornado( p_wk ) )
		{	
			*p_seq	= SEQ_HEART_SHOWER_EXIT;
		}
		break;

	case SEQ_HEART_SHOWER_EXIT:
		*p_seq	= SEQ_NEXTPROC;
		break;

	case SEQ_NEXTPROC:
		SEQ_Change( p_wk, SEQFUNC_Memo );
		break;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	メモ
 *
 *	@param	RESULT_MAIN_WORK *p_wk	メインワーク
 *	@param	*p_seq								シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Memo( RESULT_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_HEART_ROT_INIT,
		SEQ_HEART_ROT_MAIN,
		SEQ_HEART_ROT_EXIT,
		SEQ_TOUCH,
		SEQ_HEART_QUESTION,
		SEQ_WAIT,
		SEQ_TOUCH2,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_INIT:
		*p_seq	= SEQ_FADEOUT_START;
		break;

	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
		//	Memo_InitRot( p_wk );
			*p_seq	= SEQ_FADEIN_START;
		}
		break;

	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, 0 );
		*p_seq	= SEQ_FADEIN_WAIT;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_HEART_ROT_INIT;
		}
		break;

	case SEQ_HEART_ROT_INIT:
		//Memo_InitRot( p_wk );
		*p_seq	= SEQ_HEART_ROT_MAIN;
		break;

	case SEQ_HEART_ROT_MAIN:
		//if( Memo_MainRot( p_wk ) )
		{	
			*p_seq	= SEQ_HEART_ROT_EXIT;
		}
		break;

	case SEQ_HEART_ROT_EXIT:
		MSGWND_PrintCenter( &p_wk->msgwnd[MSGWNDID_SUB], &p_wk->msg, RESULT_STR_002 );
		*p_seq	= SEQ_TOUCH;
		break;

	case SEQ_TOUCH:
		if( GFL_UI_TP_GetTrg() )
		{	
			*p_seq	= SEQ_HEART_QUESTION;
		}
		break;

	case SEQ_HEART_QUESTION:
/*		{	
			u8	rnd	= p_wk->p_param->random;
			if( p_wk->p_param->score < 40 )
			{	
				MSGWND_PrintCenter( &p_wk->msgwnd[MSGWNDID_MAIN], &p_wk->msg, RESULT_BAD_000 + rnd );
			}
			else if( p_wk->p_param->score < 70 )
			{	
				MSGWND_PrintCenter( &p_wk->msgwnd[MSGWNDID_MAIN], &p_wk->msg, RESULT_NORMAL_000 + rnd );
			}
			else if( p_wk->p_param->score <= 100 )
			{	
				MSGWND_PrintCenter( &p_wk->msgwnd[MSGWNDID_MAIN], &p_wk->msg, RESULT_GOOD_000 + rnd );
			}
		}
	*/	*p_seq	= SEQ_WAIT;
		break;

	case SEQ_WAIT:
		if( p_wk->cnt++ > 30 )
		{	
			p_wk->cnt	= 0;
			*p_seq	= SEQ_TOUCH2;
		}
		break;

	case SEQ_TOUCH2:
		if( GFL_UI_TP_GetTrg() )
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
 *			OBJ_NUMBER
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	初期化
 *
 *	@param	OBJNUBER_WORK *p_wk	ワーク
 *	@param	*p_grp							描画ワーク
 *
 */
//-----------------------------------------------------------------------------
static void OBJNUBER_Init( OBJNUBER_WORK *p_wk, const GRAPHIC_WORK *cp_grp )
{	
	GFL_STD_MemClear( p_wk, sizeof(OBJNUBER_WORK) );
	{	
		int i;
		GFL_CLACTPOS	clpos;
		for( i = 0; i < CLWK_NUMBER_MAX; i++ )
		{	
			p_wk->p_clwk[i]	= GRAPHIC_GetClwk( cp_grp, CLWKID_NUMBER_TOP + i );
			clpos.x	= 256/2 + 36 * (i-1);
			clpos.y	= 92;
			GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, 0 );
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	開始
 *
 *	@param	OBJNUBER_WORK *p_wk	ワーク
 *	@param	number							表示する番号
 *	@param	sync								シンク
 *
 */
//-----------------------------------------------------------------------------
static void OBJNUBER_Start( OBJNUBER_WORK *p_wk, int number, int sync )
{	
	p_wk->sync			= 0;
	p_wk->sync_max	= sync;
	p_wk->number		= number;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン
 *
 *	@param	OBJNUBER_WORK *p_wk	ワーク
 *	@param	number							表示する番号
 *	@param	sync								シンク
 *
 *	@return	TRUE処理終了
 *	@return	FALSE処理中
 */
//-----------------------------------------------------------------------------
static BOOL OBJNUBER_Main( OBJNUBER_WORK *p_wk )
{	

	if( p_wk->sync++ > p_wk->sync_max )
	{	
		ObjNumber_SetNumber( p_wk, p_wk->number );
		return TRUE;
	}
	else
	{	
		int i;
		u8 anm;

		for( i = 0; i < CLWK_NUMBER_MAX; i++ )
		{	
			anm	= GFUser_GetPublicRand( 10 );
			GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i], anm );
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
		}
	}
	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJに番号をセット
 *
 *	@param	OBJNUBER_WORK *p_wk	ワーク
 *	@param	number							番号
 *
 */
//-----------------------------------------------------------------------------
static void ObjNumber_SetNumber( OBJNUBER_WORK *p_wk, int number )
{	
	static const u8 sc_num_table[] = 
	{
		1,
		10,
		100,
		1000,
	};
	u8 fig;
	u8 n;
	u8 num;
	int i;

	//何桁か調べる
	n		= number;
	fig	= 0;
	if( n == 0 )
	{	
		fig	= 1;
	}
	else
	{	
		while( 1 )
		{
			if( n == 0 )
			{	
				break;
			}
			n	/= 10;
			fig++;
		}
	}
	

	//一端消去
	for( i = 0; i < CLWK_NUMBER_MAX; i++ )
	{
		//GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i], 0 );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
	}

	//OBJのアニメを桁の数値に合わせる
	for( i = 0; i < fig; i++ )
	{
		num	= number % sc_num_table[ i+1 ] / sc_num_table[ i ];
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[CLWK_NUMBER_MAX-1-i], num );
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[CLWK_NUMBER_MAX-1-i], TRUE );
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
/*
		v[0].x	= cp_diamond->x - TOUCH_DIAMOND_W/2;
		v[0].y	= cp_diamond->y;
		v[1].x	= cp_diamond->x;
		v[1].y	= cp_diamond->y - TOUCH_DIAMOND_H/2;
		v[2].x	= cp_diamond->x + TOUCH_DIAMOND_W/2;
		v[2].y	= cp_diamond->y;
		v[3].x	= cp_diamond->x;
		v[3].y	= cp_diamond->y + TOUCH_DIAMOND_H/2;
		v[4]		= v[0];
*/
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

#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	大きいハートの拡大処理初期化
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *	@param	score										点数
 *
 */
//-----------------------------------------------------------------------------
static void BigHeart_InitScale( RESULT_MAIN_WORK *p_wk, u8 score )
{	
	fx32	rate;
	int	start;
	int	end;
	int	ctrl1;
	int	ctrl2;

	rate	= FX_Div( FX32_CONST(score), FX32_CONST(100) );
	start	= OBJ_BIG_HEART_DEF_SIZE;
	end		= FX_Mul( OBJ_BIG_HEART_SUB_SIZE, rate )+OBJ_BIG_HEART_MIN_SIZE;
	ctrl1	= FX32_CONST(1.1);
	ctrl2	= FX32_CONST(0.6);

	PROGVAL_CATMULLROM_Init( &p_wk->heart_size, start, ctrl1, ctrl2, end, HEART_SCALSE_WAIT );
}

//----------------------------------------------------------------------------
/**
 *	@brief	大きいハートの拡大処理メイン処理
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば処理終了
 *	@retval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL BigHeart_MainScale( RESULT_MAIN_WORK *p_wk )
{
	BOOL ret;
	GFL_CLWK		*p_heart;
	GFL_CLSCALE	scale;
			
	//動作
	ret	=  PROGVAL_CATMULLROM_Main( &p_wk->heart_size );

	//サイズ決定
	p_heart	= GRAPHIC_GetClwk( &p_wk->grp, CLWKID_BIG_HEART );
	scale.x	= p_wk->heart_size.now;
	scale.y	= p_wk->heart_size.now;
	GFL_CLACT_WK_SetScale( p_heart, &scale );

	return ret;
}
#else
//----------------------------------------------------------------------------
/**
 *	@brief	大きいハートの拡大処理初期化
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *	@param	score										点数
 *
 */
//-----------------------------------------------------------------------------
static void BigHeart_InitScale( RESULT_MAIN_WORK *p_wk, u8 score, u8 sync )
{	
	fx32 rate;
	fx32 start;
	fx32 end;

	rate	= FX_Div( FX32_CONST(score), FX32_CONST(100) );
	rate	= FX32_ONE - rate;
	//max		= SCALE_MIN + FX_Mul((FX32_ONE-SCALE_MIN), rate );
	start	= FX32_CONST(2);//SCALE_MIN;

//	max	= FX_Mul( FX32_ONE, rate );
//
	end	= FX32_ONE + FX_Mul((start - FX32_ONE ) , rate);

	OS_Printf( "rate %d\n", FX_FX32_TO_F32(rate) );


	PROGVAL_VEL_Init( &p_wk->scale, start, end, sync );

	GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], 
			GFL_BG_CENTER_X_SET, 128 );
	GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], 
			GFL_BG_CENTER_Y_SET, 96 );
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART],
			GFL_BG_SCALE_X_SET, p_wk->scale.now_val);
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART],
			GFL_BG_SCALE_Y_SET, p_wk->scale.now_val);
	GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], FALSE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	大きいハートの拡大処理メイン処理
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば処理終了
 *	@retval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL BigHeart_MainScale( RESULT_MAIN_WORK *p_wk )
{
	BOOL ret;
	GFL_CLWK		*p_heart;
	GFL_CLSCALE	scale;
			
	//動作
	ret	=  PROGVAL_VEL_Main( &p_wk->scale );

	//サイズ決定
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART],
			GFL_BG_SCALE_X_SET, p_wk->scale.now_val);
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART],
			GFL_BG_SCALE_Y_SET, p_wk->scale.now_val);

	GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], TRUE );

	return ret;
}

#endif

//----------------------------------------------------------------------------
/**
 *	@brief	揺れる処理	初期化
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *	@param	start										開始値
 *	@param	end											終了値
 *	@param	shake1									ゆれる点１
 *	@param	shake2									ゆれる点２
 *	@param	shake_cnt								ゆれる回数
 *	@param	sync										ゆれるシンク
 *
 */
//-----------------------------------------------------------------------------
static void BigHeart_InitShake( RESULT_MAIN_WORK *p_wk )
{	
	GFL_CLWK		*p_heart;
	int start;
	int shake_min;
	int shake_max;
	u16 space;
	u16 sync;

	p_heart	= GRAPHIC_GetClwk( &p_wk->grp, CLWKID_BIG_HEART );
	start	= GFL_CLACT_WK_GetRotation( p_heart );
	shake_min	= start	- 0x300;
	shake_max	= start	+ 0x300;

	PROGVAL_SHAKE_Init( &p_wk->shake, start, start, shake_min, shake_max, 50, 500 );

}
//----------------------------------------------------------------------------
/**
 *	@brief	揺れる処理	メイン
 *
 *	@param	RESULT_MAIN_WORK *p_wk ワーク
 *
 *	@retval	TRUEならば処理終了
 *	@retval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL BigHeart_MainShake( RESULT_MAIN_WORK *p_wk )
{
	BOOL ret;
	GFL_CLWK		*p_heart;

	//動作
	ret	=  PROGVAL_SHAKE_Main( &p_wk->shake );

	//サイズ決定
	p_heart	= GRAPHIC_GetClwk( &p_wk->grp, CLWKID_BIG_HEART );
	GFL_CLACT_WK_SetRotation( p_heart, p_wk->shake.now );

	return ret;
}
//----------------------------------------------------------------------------
/**
 *	@brief	小さいハートの竜巻処理	初期化
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク 
 *
 */
//-----------------------------------------------------------------------------
static void SmallHeart_InitTornado( RESULT_MAIN_WORK *p_wk )
{
	VecFx32				pos;
	GFL_CLWK			*p_clwk;
	GFL_CLACTPOS	clpos;
	int i;
	for( i = 0; i < CLWK_SMALL_HEART_MAX; i++ )
	{	
		p_clwk	= GRAPHIC_GetClwk( &p_wk->grp, CLWKID_SMALL_HEART_TOP + i );
		GFL_CLACT_WK_GetPos( p_clwk, &clpos, 0 );
		pos.x	= clpos.x;
		pos.y	= clpos.y;
		pos.z	= 0;

		PROGVAL_TORNADO_Init( &p_wk->tornado[i], &pos, 50+GFUser_GetPublicRand(80), 0x120, 100 );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	小さいハートの竜巻き処理	メイン
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば処理終了
 *	@retval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL SmallHeart_MainTornado( RESULT_MAIN_WORK *p_wk )
{	
	int i;
	u8	ret;
	GFL_CLWK			*p_clwk;
	GFL_CLACTPOS	clpos;

	ret	= 0;
	for( i = 0; i < CLWK_SMALL_HEART_MAX; i++ )
	{	
		p_clwk	= GRAPHIC_GetClwk( &p_wk->grp, CLWKID_SMALL_HEART_TOP + i );

		//動作は見えてから
		if( GFL_CLACT_WK_GetDrawEnable( p_clwk ) )
		{	
			ret	+= PROGVAL_TORNADO_Main( &p_wk->tornado[i] );
			clpos.x	= p_wk->tornado[i].now_pos.x;
			clpos.y	= p_wk->tornado[i].now_pos.y;
			GFL_CLACT_WK_SetPos( p_clwk, &clpos, 0 );
		}
		else
		{	
			break;
		}

	}

	//前回でたやつから一定カウントとってから表示
	if( i < CLWK_SMALL_HEART_MAX )
	{	
		if( p_wk->cnt++ > 10 )
		{	
			p_clwk	= GRAPHIC_GetClwk( &p_wk->grp, CLWKID_SMALL_HEART_TOP + i );
			GFL_CLACT_WK_SetDrawEnable( p_clwk, TRUE );
			p_wk->cnt	= 0;
		}
	}
		
	if( ret == CLWK_SMALL_HEART_MAX )
	{	
		return TRUE;
	}

	return FALSE;
}

#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	メモの回転処理	初期化
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク 
 *
 */
//-----------------------------------------------------------------------------
static void Memo_InitRot( RESULT_MAIN_WORK *p_wk )
{	
	PROGVAL_VEL_Init( &p_wk->memo_scale, FX32_CONST(40), FX32_ONE, 80 );
	PROGVAL_VEL_Init( &p_wk->memo_rot, 0, 0xFFF1, 80 );

/*	GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], 
			GFL_BG_CENTER_X_SET, 128 );
	GFL_BG_SetRotateCenterReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], 
			GFL_BG_CENTER_Y_SET, 96 );
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART],
			GFL_BG_SCALE_X_SET, p_wk->memo_scale.now_val);
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART],
			GFL_BG_SCALE_Y_SET, p_wk->memo_scale.now_val);
	GFL_BG_SetRadianReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], 
			GFL_BG_RADION_SET, p_wk->memo_rot.now_val );
	GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], TRUE );
*/}
//----------------------------------------------------------------------------
/**
 *	@brief	メモの回転処理	メイン処理
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば処理終了
 *	@retval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL Memo_MainRot( RESULT_MAIN_WORK *p_wk )
{	
	BOOL ret;
	ret	= PROGVAL_VEL_Main( &p_wk->memo_scale );
	ret |= PROGVAL_VEL_Main( &p_wk->memo_rot );

/*	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART],
			GFL_BG_SCALE_X_SET, p_wk->memo_scale.now_val);
	GFL_BG_SetScaleReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART],
			GFL_BG_SCALE_Y_SET, p_wk->memo_scale.now_val);
	GFL_BG_SetRadianReq( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_HEART], 
			GFL_BG_RADION_SET, p_wk->memo_rot.now_val );
*/
	return ret;
}

#endif
//----------------------------------------------------------------------------
/**
 *	@brief	デバッグプリントを更新
 *
 *	@param	RESULT_MAIN_WORK *p_wk	ワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void DEBUGRESULT_PRINT_UpDate( RESULT_MAIN_WORK *p_wk )
{
//	DEBUGPRINT_Clear();
	//DEBUGPRINT_Print( L"タッチ開始から終了まで", 0,  0 );
}
 
#ifdef DEBUG_RESULT_MSG
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
#endif //DEBUG_RESULT_MSG 
