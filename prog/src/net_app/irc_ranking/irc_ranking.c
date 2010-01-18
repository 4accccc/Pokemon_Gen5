//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_ranking.c
 *	@brief	赤外線ミニゲームランキング画面
 *	@author	Toru=Nagihashi
 *	@data		2009.07.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

//	constant
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"

//	archive
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_irc_ranking.h"
#include "ircranking_gra.naix"
#include "font/font.naix"

//	print
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"

//	module
#include "infowin/infowin.h"
#include "sound/pm_sndsys.h"
#include "net_app/irc_appbar.h"

//	common
#include "app/app_menu_common.h"

//sound
#include "../irc_compatible/irc_compatible_snd.h"

//	save
#include "savedata/irc_compatible_savedata.h"

//	mine
#include "net_app/irc_ranking.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	マクロ
//=====================================
#ifdef PM_DEBUG
//#define DEBUG_RANKING_TICK
#endif

//タイム
#ifdef DEBUG_RANKING_TICK

static OSTick s_DEBUG_TICK_DRAW_start;
#define DEBUG_TICK_DRAW_Print		NAGI_Printf("line[%d] time=%dmicro\n",__LINE__,OS_TicksToMicroSeconds(OS_GetTick() - s_DEBUG_TICK_DRAW_start) )
#define DEBUG_TICK_DRAW_Start		s_DEBUG_TICK_DRAW_start = OS_GetTick()

#else
		
#define DEBUG_TICK_DRAW_Print
#define DEBUG_TICK_DRAW_Start

#endif

//-------------------------------------
///	パレット
//=====================================
enum
{	
	// メイン画面BG
	RANKING_BG_PAL_M_00 = 0,// 
	RANKING_BG_PAL_M_01,		// 
	RANKING_BG_PAL_M_02,		//
	RANKING_BG_PAL_M_03,		// 
	RANKING_BG_PAL_M_04,		// 
	RANKING_BG_PAL_M_05,		// 
	RANKING_BG_PAL_M_06,		// 
	RANKING_BG_PAL_M_07,		// 
	RANKING_BG_PAL_M_08,		// 使用してない
	RANKING_BG_PAL_M_09,		// 使用してない
	RANKING_BG_PAL_M_10,		// 使用してない
	RANKING_BG_PAL_M_11,		// 使用してない
	RANKING_BG_PAL_M_12,		// 使用してない
	RANKING_BG_PAL_M_13,		// 使用してない
	RANKING_BG_PAL_M_14,		// フォント
	RANKING_BG_PAL_M_15,		//


	// サブ画面BG
	RANKING_BG_PAL_S_00 = 0,	// 
	RANKING_BG_PAL_S_01,		//  
	RANKING_BG_PAL_S_02,		//  
	RANKING_BG_PAL_S_03,		//  
	RANKING_BG_PAL_S_04,		//  
	RANKING_BG_PAL_S_05,		//  
	RANKING_BG_PAL_S_06,		//  
	RANKING_BG_PAL_S_07,		// 使用してない
	RANKING_BG_PAL_S_08,		// 使用してない
	RANKING_BG_PAL_S_09,		// 使用してない
	RANKING_BG_PAL_S_10,		// 使用してない
	RANKING_BG_PAL_S_11,		// 使用してない
	RANKING_BG_PAL_S_12,		// 使用してない
	RANKING_BG_PAL_S_13,		// アプリバー
	RANKING_BG_PAL_S_14,		// フォント
	RANKING_BG_PAL_S_15,		// INFOWIN
};

//-------------------------------------
///	SCROLL
//=====================================
//BMPWINインデックス
enum
{	
	SCROLL_BMPWIN_FONT_M,
	SCROLL_BMPWIN_FONT_S,
	SCROLL_BMPWIN_MAX
};


#define SCROLL_BAR_X					(2)
#define SCROLL_BAR_WIDTH			(20)
#define SCROLL_BAR_HEIGHT			(2)
#define SCROLL_BAR_CNT_X			(24)
#define SCROLL_BAR_CNT_WIDTH	(5)
#define SCROLL_BAR_CNT_HEIGHT	(2)

#define SCROLL_BAR_ALL_WIDTH		(32)	//(28)
#define SCROLL_BAR_ALL_HEIGHT		(2)

#define SCROLL_BAR_FONT_RANK_X		(3)
#define SCROLL_BAR_FONT_PLAYER_X	(7)
#define SCROLL_BAR_FONT_SCORE_X		(17)
#define SCROLL_BAR_FONT_COUNT_X		(24)

//余白
#define SCROLL_MARGIN_SIZE_Y_M	(-4*GFL_BG_1CHRDOTSIZ)
#define SCROLL_MARGIN_SIZE_Y_S	(0)

#define SCROLL_REWITE_DISTANCE	(SCROLL_BAR_HEIGHT*GFL_BG_1CHRDOTSIZ)	//どのくらいの距離すすんだら張り替えて、戻すか
#define	SCROLL_WRITE_BAR_START_M	(0)	//開始インデックス
#define SCROLL_WRITE_BAR_END_M		(9)
#define SCROLL_WRITE_BAR_NUM_M		(SCROLL_WRITE_BAR_END_M-SCROLL_WRITE_BAR_START_M)
#define	SCROLL_WRITE_BAR_START_S	(7)	//開始インデックス
#define SCROLL_WRITE_BAR_END_S		(19)
#define SCROLL_WRITE_BAR_NUM_S		(SCROLL_WRITE_BAR_END_S-SCROLL_WRITE_BAR_START_S)
#define	SCROLL_WRITE_BAR_START_EX_M	(-2)	//EXが開始インデックスより前ならば、前にはる
#define	SCROLL_WRITE_BAR_START_EX_S	(7)	//EXが開始インデックスより前ならば、前にはる

#define SCROLL_WRITE_POS_START_M	(4)	//どの位置から張り始めるか
#define SCROLL_WRITE_POS_START_S	(0)	//どの位置から張り始めるか

#define SCROLL_FONT_Y_OFS	(2)	//文字Y位置

#define SCROLL_NEWRANK_NULL	(0xFFFF)

//-------------------------------------
///		BMPWIN
//=====================================
enum
{	
	BMPWIN_ID_TITLE,
	BMPWIN_ID_RANK,
	BMPWIN_ID_PLAYER,
	BMPWIN_ID_SCORE,
	BMPWIN_ID_COUNT,
	BMPWIN_ID_MAX,
} ;

#define BMPWIN_TITLE_X				(3)
#define BMPWIN_TITLE_Y				(1)
#define BMPWIN_TITLE_W				(26)
#define BMPWIN_TITLE_H				(2)

#define BMPWIN_RANK_X					(1)
#define BMPWIN_RANK_Y					(5)
#define BMPWIN_RANK_W					(6)
#define BMPWIN_RANK_H					(2)

#define BMPWIN_PLAYER_X				(8)
#define BMPWIN_PLAYER_Y				(5)
#define BMPWIN_PLAYER_W				(6)
#define BMPWIN_PLAYER_H				(2)

#define BMPWIN_SCORE_X				(15)
#define BMPWIN_SCORE_Y				(5)
#define BMPWIN_SCORE_W				(6)
#define BMPWIN_SCORE_H				(2)

#define BMPWIN_COUNT_X				(24)
#define BMPWIN_COUNT_Y				(5)
#define BMPWIN_COUNT_W				(6)
#define BMPWIN_COUNT_H				(2)

#define BMPWIN_BAR_X					(0)
#define BMPWIN_BAR_Y					(21)
#define BMPWIN_BAR_W					(32)
#define BMPWIN_BAR_H					(3)

#define BMPWIN_FONT_X					(0)
#define	BMPWIN_FONT_Y					(0)
#define BMPWIN_FONT_W					(32)
#define	BMPWIN_FONT_H					(24)

//-------------------------------------
///	UI
//=====================================
#define UI_FLIK_RELEASE_SYNC	(1)		//はじくとき、離してから何シンク有効か

//-------------------------------------
///	ACLR
//=====================================
#define	ACLR_SCROLL_MOVE_MIN					(-FX32_CONST(8))
#define	ACLR_SCROLL_MOVE_MAX					(FX32_CONST(8))

#define ACLR_SCROLL_DISTANCE_MIN			(FX32_CONST(1))	//MIN以下だと反応しない
#define ACLR_SCROLL_DISTANCE_MAX			(FX32_CONST(60))	//以上は切捨て
#define ACLR_SCROLL_DISTANCE_DIF			(ACLR_SCROLL_DISTANCE_MAX-ACLR_SCROLL_DISTANCE_MIN)

#define ACLR_SCROLL_SYNC_MIN					(FX32_CONST(0))		//以下は切り捨て
#define ACLR_SCROLL_SYNC_MAX					(FX32_CONST(13))	//MAX以上だと反応しない
#define ACLR_SCROLL_SYNC_DIF					(ACLR_SCROLL_SYNC_MAX-ACLR_SCROLL_SYNC_MIN)

#define ACLR_SCROLL_ACLR_MIN					(FX32_CONST(0))
#define ACLR_SCROLL_ACLR_MAX					(FX32_CONST(40))
#define ACLR_SCROLL_ACLR_DIF					(ACLR_SCROLL_ACLR_MAX-ACLR_SCROLL_ACLR_MIN)

#define ACLR_SCROLL_DEC_RATE					(FX32_CONST(0.05))	//減衰率

#define ACLR_SCROLL_KEY_MIN						(FX32_CONST(20))	
#define ACLR_SCROLL_KEY_MAX						(FX32_CONST(60))
#define ACLR_SCROLL_KEY_DIF						(ACLR_SCROLL_KEY_MAX-ACLR_SCROLL_KEY_MIN)

//キーでの移動パワー
#define ACLR_SCROLL_KEY_MOVE_INIT_DISTANCE	(ACLR_SCROLL_DISTANCE_MIN+FX32_ONE)
#define ACLR_SCROLL_KEY_MOVE_INIT_SYNC			((ACLR_SCROLL_SYNC_MAX-FX32_ONE)>>FX32_SHIFT)
#define ACLR_SCROLL_KEY_MOVE_ACLR_SYNC			((ACLR_SCROLL_SYNC_MAX-FX32_ONE)>>FX32_SHIFT)

#define	ACLR_SCROLL_NEWRANK_OFS				(192)

//-------------------------------------
///	DIV
//=====================================
#define DRAG_MOVE_DIV_ADD_RATE				(FX32_CONST(0.04f))
#define DRAG_MOVE_DIV_MAX							(20)
#define DRAG_MOVE_INIT_DISTANCE				(16)


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




//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ランキングデータ
//=====================================
typedef struct 
{
	STRBUF	*p_name;
	u32			score			:7;
	u32			play_cnt	:10;
	u32			plt				:8;
	u32			rank			:7;
} RANKING_DATA;
//-------------------------------------
///	BGワーク
//=====================================
typedef struct 
{
	ARCHANDLE		*p_handle;
	GFL_ARCUTIL_TRANSINFO	bar_char_m;
	GFL_ARCUTIL_TRANSINFO	bar_char_s;
	GFL_BMPWIN	*p_bmpwin[BMPWIN_ID_MAX];
} GRAPHIC_BG_WORK;
//-------------------------------------
///	OBJワーク
//=====================================
typedef struct 
{
	GFL_CLUNIT *p_clunit;
	u32					reg_id[OBJREGID_MAX];
	GFL_CLWK	 *p_clwk[CLWKID_MAX];
} GRAPHIC_OBJ_WORK;
//-------------------------------------
///	SCROLLワーク
//=====================================
typedef struct 
{
	u8	bar_frm_m;			//上画面バー表示面
	u8	font_frm_m;			//上画面フォント表示面
	u8	bar_frm_s;			//下画面バー表示面
	u8	font_frm_s;			//下画面バー表示面
	const GRAPHIC_BG_WORK	*cp_bg;		//バー用キャラ受け取りのため
	const RANKING_DATA		*cp_data;	//ランキングのデータ
	u16	data_len;										//ランキングのデータ長
	s16 y;													//スクロール絶対位置
	s16	top_limit_y;								//上方向上限
	s16	bottom_limit_y;							//下方向下限
	s16	top_bar;										//張り替える際の一番上のランキングでーたインデックス
	s16 add_y;											//スクロール移動方向
	s16 pre_dir;										//前の移動方向（+or-）
	s16 is_zero_next;								//0の次は方向を変えてもPREへいかないようにする

	s16	rewrite_scr_pos_m;					//張り替える上画面位置
	s16	rewrite_scr_pos_s;					//張り替える下画面位置

	GFL_BMPWIN		*p_bmpwin[SCROLL_BMPWIN_MAX];	//フォント用BMPWIN
	GFL_BMP_DATA	**pp_bmp;											//書き込む処理を軽減するためフォントを
																							//BMPに先に書き込んでおいたデータ（ランキングインデックスと対応）

	GFL_FONT			*p_font;
  GFL_MSGDATA		*p_msg;

	STRBUF				*p_rank_buf;
	STRBUF				*p_score_buf;
	STRBUF				*p_count_buf;
	STRBUF				*p_strbuf;

	WORDSET				*p_wordset;
	GFL_TCB				*p_vblank_task;
	BOOL					is_update;					//VBLANK張替えアップデート
	BOOL					is_move_req;				//移動リクエスト（メインへの通知）
	BOOL					is_move_update;			//VBLANK移動リクエスト
} SCROLL_WORK;
//-------------------------------------
///	加速度ワーク
//=====================================
typedef struct 
{
	fx32	distance_rate;
	fx32	sync_rate;
	fx32	aclr;
	BOOL	is_update;	//設定したので計算UPDATE
	s8		dir;				//方向
	u8		dummy;
} ACLR_WORK;

//-------------------------------------
///	グラフィックワーク
//=====================================
typedef struct 
{
	GRAPHIC_BG_WORK		bg;
	GRAPHIC_OBJ_WORK	obj;
	GFL_TCB	*p_vblank_task;
} GRAPHIC_WORK;
//-------------------------------------
///	タッチ
//=====================================
typedef struct 
{
	GFL_POINT start;
	GFL_POINT end;
	u32	flik_sync;
	u32	release_sync;
	u32 key_cont_sync;
} UI_WORK;
//-------------------------------------
///	シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_param );
struct _SEQ_WORK
{
	SEQ_FUNCTION	seq_function;
	BOOL is_end;
	int seq;
	void *p_param;
}; 

//-------------------------------------
///	メインワーク
//=====================================
typedef struct _IRC_RANKING_WORK
{
	//モジュール
	GRAPHIC_WORK	grp;
	SEQ_WORK			seq;
	SCROLL_WORK		scroll;
	UI_WORK				ui;
	ACLR_WORK			aclr;
  APPBAR_WORK		*p_appbar;
  GFL_FONT      *p_font;
  PRINT_QUE     *p_que;

	//データ
	RANKING_DATA	*p_rank_data;

	//パラメータ
	GAME_COMM_SYS_PTR	p_gamecomm;

	//etc
	s16	move_new_sync;
	u16 dummy;
	s16 drag_add;
	u16	drag_add_cnt;
	BOOL drag_init;
	GFL_POINT	drag_end;
}IRC_RANKING_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT IRC_RANKING_PROC_Init(
		GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_RANKING_PROC_Exit(
		GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_RANKING_PROC_Main(
		GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
//-------------------------------------
///	SEQ
//=====================================
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param, SEQ_FUNCTION seq_function );
static void SEQ_Exit( SEQ_WORK *p_wk );
static void SEQ_Main( SEQ_WORK *p_wk );
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk );
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( SEQ_WORK *p_wk );
//-------------------------------------
///	SEQFUNC
//=====================================
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
static void SEQFUNC_MoveNew( SEQ_WORK *p_seqwk, int *p_seq, void *p_param );
//-------------------------------------
///	BG
//=====================================
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, GAME_COMM_SYS_PTR	p_gamecomm, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
static GFL_ARCUTIL_TRANSINFO GRAPHIC_BG_GetTransInfo( const GRAPHIC_BG_WORK *cp_wk, BOOL is_m );
//-------------------------------------
///	OBJ
//=====================================
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID );
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk );
static GFL_CLWK* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, CLWKID id );
static GFL_CLUNIT * GRAPHIC_OBJ_GetUnit( const GRAPHIC_OBJ_WORK *cp_wk );
//-------------------------------------
///	GRAPHIC
//=====================================
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, GAME_COMM_SYS_PTR	p_gamecomm, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
static const GRAPHIC_BG_WORK *GRAPHIC_GetBgWorkConst( const GRAPHIC_WORK *cp_wk );
static GFL_CLWK	* GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id );
static GFL_CLUNIT * GRAPHIC_GetUnit( const GRAPHIC_WORK *cp_wk );
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//-------------------------------------
///	SCROLL	
//=====================================
static void SCROLL_Init( SCROLL_WORK *p_wk, u8 bar_frm_m, u8 font_frm_m, u8 bar_frm_s, u8 font_frm_s, const GRAPHIC_BG_WORK *cp_bg, const RANKING_DATA* cp_data, u16 data_len, HEAPID heapID );
static void SCROLL_Exit( SCROLL_WORK *p_wk );
static void SCROLL_Main( SCROLL_WORK *p_wk );
static void SCROLL_AddPos( SCROLL_WORK *p_wk, s16 y );
static s16 SCROLL_GetPosY( const SCROLL_WORK *cp_wk );
static s16 SCROLL_GetNewRankPosY( const SCROLL_WORK *cp_wk );
static s16 SCROLL_GetBottomLimitPosY( const SCROLL_WORK *cp_wk );
static void Scroll_WriteRank( SCROLL_WORK *p_wk, const RANKING_DATA *cp_rank, const GFL_BMP_DATA *cp_bmp, u16 y_chr, BOOL is_top );
static void Scroll_Write( SCROLL_WORK *p_wk );
static void Scroll_WriteVBlank( SCROLL_WORK *p_wk );
static void Scroll_VBlankTask( GFL_TCB *p_tcb, void *p_work );
static GFL_BMP_DATA **Scroll_CreateWriteData( SCROLL_WORK *p_wk, HEAPID heapID );
static void Scroll_DeleteWriteData( GFL_BMP_DATA **pp_data, u16 data_len );
//-------------------------------------
///	RANKING_DATA
//=====================================
static RANKING_DATA	*RANKING_DATA_Create( u16 data_len, HEAPID heapID );
static void RANKING_DATA_Delete( RANKING_DATA	*p_data );
static u16 RANKING_DATA_GetExistLength( void );
//-------------------------------------
///	TOUCH
//=====================================
static void UI_Init( UI_WORK *p_wk, HEAPID heapID );
static void UI_Exit( UI_WORK *p_wk );
static void UI_Main( UI_WORK *p_wk );
static BOOL UI_GetDrag( const UI_WORK *cp_wk, GFL_POINT *p_start, GFL_POINT *p_end, VecFx32 *p_vec );
static BOOL UI_GetFlik( UI_WORK *p_wk, GFL_POINT *p_start, GFL_POINT *p_end, VecFx32 *p_vec, u32 *p_sync );
static int UI_IsContKey( const UI_WORK *cp_wk, u32 *p_sync );
//-------------------------------------
///	ACLR
//=====================================
static void ACLR_Init( ACLR_WORK *p_wk );
static void ACLR_Exit( ACLR_WORK *p_wk );
static void ACLR_Main( ACLR_WORK *p_wk );
static void ACLR_SetAclr( ACLR_WORK *p_wk, fx32 distance, u32 sync );
static void ACLR_Stop( ACLR_WORK *p_wk );
static s16 ACLR_GetScrollAdd( ACLR_WORK *p_wk );
static BOOL ACLR_IsExist( const ACLR_WORK *cp_wk );
//-------------------------------------
///	ETC
//=====================================
static void PRINT_PrintCenter( GFL_BMPWIN *p_bmpwin, STRBUF *p_strbuf, GFL_FONT *p_font );
static void BMP_Copy( const GFL_BMP_DATA *cp_src, GFL_BMP_DATA *p_dst, u16 src_x, u16 src_y, u16 dst_x, u16 dst_y, u16 w, u16 h );
static s32 GFL_POINT_Distance( const GFL_POINT *cp_a, const GFL_POINT *cp_b );
static void ARCHDL_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h,  u8 plt, BOOL compressedFlag, HEAPID heapID );

//=============================================================================
/**
 *						DATA
 */
//=============================================================================
//-------------------------------------
///	データ
//=====================================
enum
{	
	GRAPHIC_BG_FRAME_TOP_M,
	GRAPHIC_BG_FRAME_BAR_M,
	GRAPHIC_BG_FRAME_FONT_M,
	GRAPHIC_BG_FRAME_BACK_M,

	GRAPHIC_BG_FRAME_INFO_S,
	GRAPHIC_BG_FRAME_BAR_S,
	GRAPHIC_BG_FRAME_FONT_S,
	GRAPHIC_BG_FRAME_BACK_S,

	GRAPHIC_BG_FRAME_MAX
};
static const struct 
{
	u32									frame;
	GFL_BG_BGCNT_HEADER	bgcnt_header;
	u32									mode;
}	sc_bgsetup[GRAPHIC_BG_FRAME_MAX]	=
{	
	{	
		GFL_BG_FRAME0_M,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	{	
		GFL_BG_FRAME1_M,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	{	
		GFL_BG_FRAME2_M,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	{	
		GFL_BG_FRAME3_M,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},


	//BOTTOM
	{	
		GFL_BG_FRAME0_S,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	{	
		GFL_BG_FRAME1_S,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	{	
		GFL_BG_FRAME2_S,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	{	
		GFL_BG_FRAME3_S,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	}
};
//フレーム取得用マクロ
#define GRAPHIC_BG_GetFrame( x )	(sc_bgsetup[ x ].frame)

//=============================================================================
/**
 *						GLOBAL
 */
//=============================================================================
//-------------------------------------
///	PROCデータ外部公開
//=====================================
const GFL_PROC_DATA	IrcRanking_ProcData	=
{
	IRC_RANKING_PROC_Init,
	IRC_RANKING_PROC_Main,
	IRC_RANKING_PROC_Exit,
} ;

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
 *	@param	*p_param					引数
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_RANKING_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_RANKING_WORK	*p_wk;
	IRC_RANKING_PARAM	*p_rank_param	;
	u16	data_len;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCRANKING, 0x40000 );

	//ワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(IRC_RANKING_WORK), HEAPID_IRCRANKING );
	GFL_STD_MemClear( p_wk, sizeof(IRC_RANKING_WORK) );

	//パラメータうけとり
	p_rank_param	= p_param;
	if( p_rank_param->p_gamesys )
	{	
		p_wk->p_gamecomm	= GAMESYSTEM_GetGameCommSysPtr(p_rank_param->p_gamesys);
	}
	else
	{	
		p_wk->p_gamecomm	= NULL;
	}

  p_wk->p_que   = PRINTSYS_QUE_Create( HEAPID_IRCRANKING );
  p_wk->p_font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
				GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_IRCRANKING ); 

	//データ作成
	data_len					= RANKING_DATA_GetExistLength();
	p_wk->p_rank_data	= RANKING_DATA_Create( data_len, HEAPID_IRCRANKING );
	NAGI_Printf( "data_len %d\n", data_len );

	//モジュール初期化
	GRAPHIC_Init( &p_wk->grp, p_wk->p_gamecomm, HEAPID_IRCRANKING );
	SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_FadeOut );
	UI_Init( &p_wk->ui, HEAPID_IRCRANKING );
	ACLR_Init( &p_wk->aclr );
	SCROLL_Init( &p_wk->scroll,
			GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_M),
			GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_FONT_M),
			GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_S),
			GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_FONT_S),
			GRAPHIC_GetBgWorkConst(&p_wk->grp),
			p_wk->p_rank_data,
			data_len,
			HEAPID_IRCRANKING
			);

  p_wk->p_appbar	= APPBAR_Init( APPBAR_OPTION_MASK_RETURN, GRAPHIC_GetUnit( &p_wk->grp ), GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_INFO_S), RANKING_BG_PAL_S_13, 0, APP_COMMON_MAPPING_128K, p_wk->p_font, p_wk->p_que, HEAPID_IRCRANKING );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	PROC破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					引数
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_RANKING_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_RANKING_WORK	*p_wk	= p_work;

	//モジュール破棄
	APPBAR_Exit( p_wk->p_appbar );
	INFOWIN_Exit();
	SCROLL_Exit( &p_wk->scroll );
	ACLR_Exit( &p_wk->aclr );
	UI_Exit( &p_wk->ui );
	SEQ_Exit( &p_wk->seq );
	GRAPHIC_Exit( &p_wk->grp );

	//データ破棄
	RANKING_DATA_Delete( p_wk->p_rank_data );

  GFL_FONT_Delete( p_wk->p_font );
  PRINTSYS_QUE_Delete( p_wk->p_que );

	//ワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap(HEAPID_IRCRANKING );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	PROCメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					引数
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_RANKING_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_RANKING_WORK	*p_wk	= p_work;

	//シーケンス
	SEQ_Main( &p_wk->seq );

	//情報バー
	INFOWIN_Update();

	//描画
	GRAPHIC_Draw( &p_wk->grp );

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
 *	@param	*p_param				パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Init( SEQ_WORK *p_wk, void *p_param, SEQ_FUNCTION seq_function )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );

	//初期化
	p_wk->p_param	= p_param;

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
		p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_param );
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
 *	@param	*p_param				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeOut( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{	
	enum
	{	
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_END,
	};

	IRC_RANKING_WORK	*p_wk	= p_param;

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
		//新規があったら、移動シーケンスへ
		if( SCROLL_GetNewRankPosY( &p_wk->scroll ) != SCROLL_NEWRANK_NULL )
		{	
			SEQ_SetNext( &p_wk->seq, SEQFUNC_MoveNew );
		}
		else
		{	
			SEQ_SetNext( p_seqwk, SEQFUNC_Main );
		}
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
 *	@param	*p_param				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_FadeIn( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{
	enum
	{
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};	

	IRC_RANKING_WORK	*p_wk	= p_param;

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
 *	@param	*p_param				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Main( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{	
	IRC_RANKING_WORK	*p_wk	= p_param;
	u32 sync;

	//キー入力による移動
	if( UI_IsContKey( &p_wk->ui, &sync ) & PAD_KEY_UP )
	{	
		fx32 rate;
		if( ACLR_SCROLL_KEY_MIN < FX32_CONST(sync) )
		{	
			rate	= FX_Div( (FX32_CONST(sync) - ACLR_SCROLL_KEY_MIN), ACLR_SCROLL_KEY_DIF );
			ACLR_SetAclr( &p_wk->aclr, FX_Mul( rate	, ACLR_SCROLL_DISTANCE_MAX ), ACLR_SCROLL_KEY_MOVE_ACLR_SYNC );
		}
		else
		{	
			ACLR_SetAclr( &p_wk->aclr, ACLR_SCROLL_KEY_MOVE_INIT_DISTANCE, ACLR_SCROLL_KEY_MOVE_INIT_SYNC );
		}
	}
	else if( UI_IsContKey( &p_wk->ui, &sync ) & PAD_KEY_DOWN )
	{	
		fx32 rate;
		if( ACLR_SCROLL_KEY_MIN < FX32_CONST(sync) )
		{	
			rate	= FX_Div( (FX32_CONST(sync) - ACLR_SCROLL_KEY_MIN), ACLR_SCROLL_KEY_DIF );
			ACLR_SetAclr( &p_wk->aclr, -FX_Mul( rate, ACLR_SCROLL_DISTANCE_MAX ), ACLR_SCROLL_KEY_MOVE_ACLR_SYNC );
		}
		else
		{	
			ACLR_SetAclr( &p_wk->aclr, -ACLR_SCROLL_KEY_MOVE_INIT_DISTANCE, ACLR_SCROLL_KEY_MOVE_INIT_SYNC );
		}
	}
	else
	//タッチによる移動
	{	
		VecFx32 dist;
		fx32 distance;
		u32		sync;
		GFL_POINT	drag_end;
		GFL_POINT	drag_start;

		static const VecFx32 sc_up_norm	= 
		{
			0, FX32_ONE, 0
		};

		//ドラッグによる移動
		if( UI_GetDrag( &p_wk->ui, &drag_start, &drag_end, &dist ) && MATH_IAbs(VEC_Mag(&dist )) > FX32_CONST(DRAG_MOVE_INIT_DISTANCE) )
		{	
			fx32 distance;
			s8	dir;
			if( p_wk->drag_init == FALSE ||
					(MATH_IAbs(MATH_IAbs(GFL_POINT_Distance( &drag_end, &drag_start )) - MATH_IAbs(GFL_POINT_Distance( &p_wk->drag_end, &drag_start ))) > DRAG_MOVE_INIT_DISTANCE ) )
			{	
				distance	= VEC_DotProduct( &sc_up_norm, &dist );
				p_wk->drag_add_cnt	= 0;
				p_wk->drag_add	= FX_Mul(distance, DRAG_MOVE_DIV_ADD_RATE);
				dir	= p_wk->drag_add / MATH_IAbs(p_wk->drag_add);
				p_wk->drag_add	= MATH_CLAMP( MATH_IAbs(p_wk->drag_add), FX32_ONE, MATH_IAbs(p_wk->drag_add));
				p_wk->drag_add	*= dir;
				p_wk->drag_end	= drag_end;
				p_wk->drag_init	= TRUE;
				OS_Printf( "init cnt%d add%f\n", p_wk->drag_add_cnt, FX_FX32_TO_F32(p_wk->drag_add) );
			}
			
			if( p_wk->drag_add_cnt++ < DRAG_MOVE_DIV_MAX )
			{
				SCROLL_AddPos( &p_wk->scroll, p_wk->drag_add >> FX32_SHIFT );
				OS_Printf( "move cnt%d add%f\n", p_wk->drag_add_cnt, FX_FX32_TO_F32(p_wk->drag_add) );
			}
			ACLR_Stop( &p_wk->aclr );
		}
		//はじきにより移動		
		else if( UI_GetFlik( &p_wk->ui, NULL, NULL, &dist, &sync ) )
		{	
			distance	= VEC_DotProduct( &sc_up_norm, &dist );
			ACLR_SetAclr( &p_wk->aclr, distance, sync );
		}
		else
		{	
			p_wk->drag_init	= FALSE;
		}
	}

	//加速可能ならば移動
	if( ACLR_IsExist(&p_wk->aclr) )
	{	
		SCROLL_AddPos( &p_wk->scroll, ACLR_GetScrollAdd( &p_wk->aclr ) );
	}



	//戻る
	if( APPBAR_GetTrg( p_wk->p_appbar ) == APPBAR_ICON_RETURN
			|| GFL_UI_KEY_GetCont() & PAD_BUTTON_B )
	{	
		PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );	
		SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
	}

	//モジュールメイン
	SCROLL_Main( &p_wk->scroll );
	UI_Main( &p_wk->ui );
	ACLR_Main( &p_wk->aclr );
	APPBAR_Main( p_wk->p_appbar );
}
//----------------------------------------------------------------------------
/**
 *	@brief	新規ランクまで移動する処理
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					パラメータ
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_MoveNew( SEQ_WORK *p_seqwk, int *p_seq, void *p_param )
{	
	IRC_RANKING_WORK	*p_wk	= p_param;
	s16 new_pos;
	s16 now_pos;
	s16 limit_pos;

	new_pos		= SCROLL_GetNewRankPosY( &p_wk->scroll );
	GF_ASSERT( new_pos != SCROLL_NEWRANK_NULL );
	new_pos		-= ACLR_SCROLL_NEWRANK_OFS;
	new_pos		= MATH_CLAMP( new_pos, 0, new_pos );
	now_pos		= SCROLL_GetPosY( &p_wk->scroll );
	limit_pos	= SCROLL_GetBottomLimitPosY( &p_wk->scroll );

	//新規が一定の場所にくるか、最後までうごいたら
	if( new_pos  > now_pos && now_pos != limit_pos )
	{	
		fx32 rate;
		p_wk->move_new_sync++;
		if( ACLR_SCROLL_KEY_MIN < FX32_CONST(p_wk->move_new_sync) )
		{	
			rate	= FX_Div( (FX32_CONST(p_wk->move_new_sync) - ACLR_SCROLL_KEY_MIN), ACLR_SCROLL_KEY_DIF );
			ACLR_SetAclr( &p_wk->aclr, FX_Mul( rate, ACLR_SCROLL_DISTANCE_MAX ), ACLR_SCROLL_KEY_MOVE_ACLR_SYNC );
		}
		else
		{	
			ACLR_SetAclr( &p_wk->aclr, ACLR_SCROLL_KEY_MOVE_INIT_DISTANCE, ACLR_SCROLL_KEY_MOVE_INIT_SYNC );
		}
	}
	else
	{	
		ACLR_Stop( &p_wk->aclr );
		SEQ_SetNext( &p_wk->seq, SEQFUNC_Main );
	}

	//加速可能ならば移動
	if( ACLR_IsExist(&p_wk->aclr) )
	{	
		SCROLL_AddPos( &p_wk->scroll, ACLR_GetScrollAdd( &p_wk->aclr ) );
	}


	//戻る
	if( APPBAR_GetTrg( p_wk->p_appbar ) == APPBAR_ICON_RETURN
			|| GFL_UI_KEY_GetCont() & PAD_BUTTON_B )
	{	
		SEQ_SetNext( p_seqwk, SEQFUNC_FadeIn );
	}
	//モジュールメイン
	SCROLL_Main( &p_wk->scroll );
	ACLR_Main( &p_wk->aclr );
	APPBAR_Main( p_wk->p_appbar );
}
//=============================================================================
/**
 *						BG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BG	初期化
 *
 *	@param	GRAPHIC_BG_WORK *p_wk	ワーク
 *	@param	heapID								ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, GAME_COMM_SYS_PTR	p_gamecomm, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_BG_WORK) );

	//初期化
	GFL_BG_Init( heapID );
	GFL_BMPWIN_Init( heapID );

	//グラフィックモード設定
	{	
		static const GFL_BG_SYS_HEADER sc_bgsys_header	=
		{	
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
		};
		GFL_BG_SetBGMode( &sc_bgsys_header );
	}

	//BG面設定
	{	
		int i;
		for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_SetBGControl( sc_bgsetup[i].frame, &sc_bgsetup[i].bgcnt_header, sc_bgsetup[i].mode );
			GFL_BG_ClearFrame( sc_bgsetup[i].frame );
			GFL_BG_SetVisible( sc_bgsetup[i].frame, VISIBLE_ON );
		}
	}

	//読み込み
	{	
		p_wk->p_handle	= GFL_ARC_OpenDataHandle( ARCID_IRCRANKING_GRAPHIC, heapID );


		GFL_ARCHDL_UTIL_TransVramPalette( p_wk->p_handle, NARC_ircranking_gra_bg_NCLR,
				PALTYPE_MAIN_BG, RANKING_BG_PAL_M_00*0x20, 0, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_wk->p_handle, NARC_ircranking_gra_back_NCGR,
				GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BACK_M), 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_wk->p_handle, NARC_ircranking_gra_top_NCGR,
				GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TOP_M), 0, 0, FALSE, heapID );


		GFL_ARCHDL_UTIL_TransVramScreen( p_wk->p_handle, NARC_ircranking_gra_back_NSCR,
				GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BACK_M), 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_wk->p_handle, NARC_ircranking_gra_top_NSCR,
				GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TOP_M), 0, 0, FALSE, heapID );

		GFL_ARCHDL_UTIL_TransVramPalette( p_wk->p_handle, NARC_ircranking_gra_bg_NCLR,
				PALTYPE_SUB_BG, RANKING_BG_PAL_S_00*0x20, 0, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_wk->p_handle, NARC_ircranking_gra_top_NCGR,
				GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BACK_S), 0, 0, FALSE, heapID );


		GFL_ARCHDL_UTIL_TransVramScreen( p_wk->p_handle, NARC_ircranking_gra_top2_NSCR,
				GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BACK_S), 0, 0, FALSE, heapID );




		GFL_BG_FillCharacter( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_M), 0, 1,  0 );
		GFL_BG_FillCharacter( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_S), 0, 1,  0 );

		p_wk->bar_char_m	= GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_wk->p_handle, 
				NARC_ircranking_gra_frame_NCGR, GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_M), 0, FALSE, heapID );
		p_wk->bar_char_s	= GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_wk->p_handle, 
				NARC_ircranking_gra_frame_NCGR, GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_S), 0, FALSE, heapID );

		//フォント用パレット
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, RANKING_BG_PAL_M_14*0x20, 0x20, heapID );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, RANKING_BG_PAL_S_14*0x20, 0x20, heapID );

		GFL_ARC_CloseDataHandle( p_wk->p_handle );

	}

	//BMPWIN
	{
		GFL_MSGDATA	*p_msg;
		GFL_FONT		*p_font;
		STRBUF			*p_strbuf;
		int i;
		u16 x, y;

		//作成
		p_msg				= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
				NARC_message_irc_ranking_dat, heapID );
		p_font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
				GFL_FONT_LOADTYPE_FILE, FALSE, heapID );


		p_wk->p_bmpwin[BMPWIN_ID_TITLE]	= GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TOP_M),
				BMPWIN_TITLE_X,BMPWIN_TITLE_Y,BMPWIN_TITLE_W,BMPWIN_TITLE_H,RANKING_BG_PAL_M_14,
			GFL_BMP_CHRAREA_GET_B );
		p_wk->p_bmpwin[BMPWIN_ID_RANK]	= GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TOP_M),
				BMPWIN_RANK_X,BMPWIN_RANK_Y,BMPWIN_RANK_W,BMPWIN_RANK_H,RANKING_BG_PAL_M_14,
			GFL_BMP_CHRAREA_GET_B );
		p_wk->p_bmpwin[BMPWIN_ID_PLAYER]	= GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TOP_M),
				BMPWIN_PLAYER_X,BMPWIN_PLAYER_Y,BMPWIN_PLAYER_W,BMPWIN_PLAYER_H,RANKING_BG_PAL_M_14,
			GFL_BMP_CHRAREA_GET_B );
		p_wk->p_bmpwin[BMPWIN_ID_SCORE]	= GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TOP_M),
				BMPWIN_SCORE_X,BMPWIN_SCORE_Y,BMPWIN_SCORE_W,BMPWIN_SCORE_H,RANKING_BG_PAL_M_14,
			GFL_BMP_CHRAREA_GET_B );
		p_wk->p_bmpwin[BMPWIN_ID_COUNT]	= GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TOP_M),
				BMPWIN_COUNT_X,BMPWIN_COUNT_Y,BMPWIN_COUNT_W,BMPWIN_COUNT_H,RANKING_BG_PAL_M_14,
			GFL_BMP_CHRAREA_GET_B );

		for( i = 0; i < BMPWIN_ID_MAX; i++ )
		{	
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0xF );	
		}


		//文字描画
		p_strbuf	= GFL_MSG_CreateString( p_msg, RANKING_TITLE_000 );
		PRINT_PrintCenter( p_wk->p_bmpwin[BMPWIN_ID_TITLE], p_strbuf, p_font );
		GFL_STR_DeleteBuffer( p_strbuf );

		p_strbuf	= GFL_MSG_CreateString( p_msg, RANKING_CAPTION_000 );
		PRINT_PrintCenter( p_wk->p_bmpwin[BMPWIN_ID_RANK], p_strbuf, p_font );
		GFL_STR_DeleteBuffer( p_strbuf );

		p_strbuf	= GFL_MSG_CreateString( p_msg, RANKING_CAPTION_001 );
		PRINT_PrintCenter( p_wk->p_bmpwin[BMPWIN_ID_PLAYER], p_strbuf, p_font );
		GFL_STR_DeleteBuffer( p_strbuf );

		p_strbuf	= GFL_MSG_CreateString( p_msg, RANKING_CAPTION_002 );
		PRINT_PrintCenter( p_wk->p_bmpwin[BMPWIN_ID_SCORE], p_strbuf, p_font );
		GFL_STR_DeleteBuffer( p_strbuf );

		p_strbuf	= GFL_MSG_CreateString( p_msg, RANKING_CAPTION_003 );
		PRINT_PrintCenter( p_wk->p_bmpwin[BMPWIN_ID_COUNT], p_strbuf, p_font );
		GFL_STR_DeleteBuffer( p_strbuf );

		//破棄
		GFL_FONT_Delete( p_font );
		GFL_MSG_Delete( p_msg );

		//転送
		for( i = 0; i < BMPWIN_ID_MAX; i++ )
		{	
			GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin[i] );
			GFL_BMPWIN_MakeScreen( p_wk->p_bmpwin[i] );
		}
		GFL_BG_LoadScreenReq(GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_TOP_M) );
	}

	INFOWIN_Init( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_INFO_S),
			RANKING_BG_PAL_S_15, p_gamecomm, HEAPID_IRCRANKING );


}
//----------------------------------------------------------------------------
/**
 *	@brief	BG	破棄
 *
 *	@param	GRAPHIC_BG_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk )
{	
	
	//BMPWIN破棄
	{	
		int i;
		for( i = 0; i < BMPWIN_ID_MAX; i++ )
		{	
			GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
		}
	}

	//破棄
	{	
		GFL_BG_FreeCharacterArea(GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_M),
				GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->bar_char_m),
				GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->bar_char_m));

		GFL_BG_FreeCharacterArea(GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_S),
				GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->bar_char_s),
				GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->bar_char_s));

		GFL_BG_FillCharacterRelease( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_S), 1, 0 );
		GFL_BG_FillCharacterRelease( GRAPHIC_BG_GetFrame(GRAPHIC_BG_FRAME_BAR_M), 1, 0 );
	}

	//BG面破棄
	{	
		int i;
		for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_FreeBGControl( sc_bgsetup[i].frame );
		}
	}

	//終了
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_BG_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	BG	VBlank関数
 *
 *	@param	GRAPHIC_BG_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk )
{	
	GFL_BG_VBlankFunc();
}
//----------------------------------------------------------------------------
/**
 *	@brief	トランスインフォ取得
 *
 *	@param	const GRAPHIC_BG_WORK *cp_wk	ワーク
 *	@param	is_m													TRUEならば上画面用
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static GFL_ARCUTIL_TRANSINFO GRAPHIC_BG_GetTransInfo( const GRAPHIC_BG_WORK *cp_wk, BOOL is_m )
{	
	if( is_m )
	{	
		return cp_wk->bar_char_m;
	}
	else
	{	
		return cp_wk->bar_char_s;
	}
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

#if 0
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
#endif


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
#if 0
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
#endif 

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
 *	@brief	CLユニットの取得
 *
 *	@param	const GRAPHIC__OBJ_WORK *cp_wk	ワーク
 *
 *	@return	GFL_CLUNIT
 */
//-----------------------------------------------------------------------------
static GFL_CLUNIT * GRAPHIC_OBJ_GetUnit( const GRAPHIC_OBJ_WORK *cp_wk )
{	
	return cp_wk->p_clunit;
}
//=============================================================================
/**
 *						GRAPHIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック設定
 *
 *	@param	GRAPHIC_WORK *p_wk	ワーク
 *	@param	heapID							ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, GAME_COMM_SYS_PTR	p_gamecomm, HEAPID heapID )
{	
	static const GFL_DISP_VRAM sc_vramSetTable =
	{
		GX_VRAM_BG_128_A,					// メイン2DエンジンのBG
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

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );

	//レジスタ初期化
	G2_BlendNone();
	G2S_BlendNone();	
	GX_SetVisibleWnd( GX_WNDMASK_NONE );
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );

	//VRAMクリアー
	GFL_DISP_ClearVRAM( 0 );

	//VRAMバンク設定
	GFL_DISP_SetBank( &sc_vramSetTable );

	// ディスプレイON
	GFL_DISP_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );
	GFL_DISP_SetDispOn();

	//	表示
	GFL_DISP_GX_InitVisibleControl();

	//	フォント初期化
	GFL_FONTSYS_Init();

	//	モジュール初期化
	GRAPHIC_BG_Init( &p_wk->bg, p_gamecomm, heapID );
	GRAPHIC_OBJ_Init( &p_wk->obj, &sc_vramSetTable, heapID );

	//VBlankTask登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB(Graphic_VBlankTask, p_wk, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック破棄
 *
 *	@param	GRAPHIC_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk )
{	
	//VBLANKTask消去
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

	//モジュール破棄
	GRAPHIC_OBJ_Exit( &p_wk->obj );
	GRAPHIC_BG_Exit( &p_wk->bg );

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック描画
 *
 *	@param	GRAPHIC_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk )
{	
	GRAPHIC_OBJ_Main( &p_wk->obj );
}

//----------------------------------------------------------------------------
/**
 *	@brief	BGワーク取得
 *
 *	@param	const GRAPHIC_WORK *cp_wk		ワーク
 *
 *	@return	BGワーク
 */
//-----------------------------------------------------------------------------
static const GRAPHIC_BG_WORK *GRAPHIC_GetBgWorkConst( const GRAPHIC_WORK *cp_wk )
{	
	return &cp_wk->bg;
}
//----------------------------------------------------------------------------
/**
 *	@brief	CLWK取得
 *
 *	@param	const GRAPHIC_WORK *cp_wk	ワーク
 *	@param	id												CLWKID
 *
 *	@return	CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK	* GRAPHIC_GetClwk( const GRAPHIC_WORK *cp_wk, CLWKID id )
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
static GFL_CLUNIT * GRAPHIC_GetUnit( const GRAPHIC_WORK *cp_wk )
{	
	return GRAPHIC_OBJ_GetUnit( &cp_wk->obj );
}
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィックVBLANK関数
 *
 *	@param	GRAPHIC_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work )
{	
	GRAPHIC_WORK *p_wk	= p_work;
	GRAPHIC_BG_VBlankFunction( &p_wk->bg );
	GRAPHIC_OBJ_VBlankFunction( &p_wk->obj );
}
//=============================================================================
/**
 *	SCROLL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SCROLL	初期化
 *
 *	@param	SCROLL_WORK *p_wk	ワーク
 *	@param	bar_frm						枠のBG面
 *	@param	font_frm					文字のBG面
 *	@param	bar_frm						枠のBG面
 *	@param	font_frm					文字のBG面
 *	@param	*p_bg							BGシステム
 *	@param	cp_data						表示するデータ
 *	@param	data_len					データの長さ
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void SCROLL_Init( SCROLL_WORK *p_wk, u8 bar_frm_m, u8 font_frm_m, u8 bar_frm_s, u8 font_frm_s, const GRAPHIC_BG_WORK *cp_bg, const RANKING_DATA* cp_data, u16 data_len, HEAPID heapID )
{	
	//クリアー
	GFL_STD_MemClear( p_wk, sizeof(SCROLL_WORK) );
	p_wk->cp_bg				= cp_bg;
	p_wk->bar_frm_m		= bar_frm_m;
	p_wk->font_frm_m	= font_frm_m;
	p_wk->bar_frm_s		= bar_frm_s;
	p_wk->font_frm_s	= font_frm_s;
	p_wk->cp_data			=	cp_data;
	p_wk->data_len		= data_len;

	//ランキング用フォント作成
	p_wk->p_font			= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, TRUE, heapID );

	//ランキング用メッセージマネージャ
	p_wk->p_msg				= GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_irc_ranking_dat, heapID );

	//BMPWIN作成
	p_wk->p_bmpwin[SCROLL_BMPWIN_FONT_M]	= GFL_BMPWIN_Create( 
			font_frm_m,BMPWIN_FONT_X,BMPWIN_FONT_Y,BMPWIN_FONT_W,BMPWIN_FONT_H,
			RANKING_BG_PAL_M_14,GFL_BMP_CHRAREA_GET_F );
	p_wk->p_bmpwin[SCROLL_BMPWIN_FONT_S]	= GFL_BMPWIN_Create( 
			font_frm_s,BMPWIN_FONT_X,BMPWIN_FONT_Y,BMPWIN_FONT_W,BMPWIN_FONT_H,
			RANKING_BG_PAL_S_14,GFL_BMP_CHRAREA_GET_F );

	//バッファ作成
	p_wk->p_rank_buf	= GFL_STR_CreateBuffer( 32, heapID );
	p_wk->p_score_buf	= GFL_STR_CreateBuffer( 32, heapID );
	p_wk->p_count_buf	= GFL_STR_CreateBuffer( 32, heapID );
	p_wk->p_strbuf		= GFL_STR_CreateBuffer( 32, heapID );

	//ワードセット作成
	p_wk->p_wordset	= WORDSET_Create( heapID );
	
	//張替えのため、スクリーン位置をずらす
	p_wk->rewrite_scr_pos_m	= SCROLL_MARGIN_SIZE_Y_M;
	p_wk->rewrite_scr_pos_s	= SCROLL_MARGIN_SIZE_Y_S;	
	GFL_BG_SetScroll( p_wk->bar_frm_m, GFL_BG_SCROLL_Y_SET, p_wk->rewrite_scr_pos_m );
	GFL_BG_SetScroll( p_wk->font_frm_m, GFL_BG_SCROLL_Y_SET, p_wk->rewrite_scr_pos_m );
	GFL_BG_SetScroll( p_wk->bar_frm_s, GFL_BG_SCROLL_Y_SET, p_wk->rewrite_scr_pos_s );
	GFL_BG_SetScroll( p_wk->font_frm_s, GFL_BG_SCROLL_Y_SET, p_wk->rewrite_scr_pos_s );

	//移動制限
	p_wk->y								= 0;
	p_wk->top_limit_y			= 0;
	p_wk->bottom_limit_y	= (data_len * 2)*GFL_BG_1CHRDOTSIZ-192+SCROLL_MARGIN_SIZE_Y_M-16-SCROLL_WRITE_POS_START_M*GFL_BG_1CHRDOTSIZ;
	p_wk->bottom_limit_y	= MATH_IMax( 0, p_wk->bottom_limit_y );

	//書き込みデータ
	p_wk->pp_bmp	= Scroll_CreateWriteData( p_wk, heapID );

	//描画のために一度呼ぶ
	Scroll_Write( p_wk );
	Scroll_WriteVBlank( p_wk );

	//VBlankTask登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB(Scroll_VBlankTask, p_wk, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SCROLL	破棄
 *
 *	@param	SCROLL_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SCROLL_Exit( SCROLL_WORK *p_wk )
{	
	int i;

	//VBLANKTask消去
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

	//書き込みデータ破棄
	Scroll_DeleteWriteData( p_wk->pp_bmp, p_wk->data_len );


	WORDSET_Delete( p_wk->p_wordset );

	//バッファ
	GFL_STR_DeleteBuffer( p_wk->p_strbuf );
	GFL_STR_DeleteBuffer( p_wk->p_rank_buf );
	GFL_STR_DeleteBuffer( p_wk->p_score_buf );
	GFL_STR_DeleteBuffer( p_wk->p_count_buf );

	//BMPWIN破棄
	for( i = 0; i < SCROLL_BMPWIN_MAX; i++ )
	{	
		GFL_BMPWIN_Delete( p_wk->p_bmpwin[i] );
	}

	//メッセージ破棄
	GFL_MSG_Delete( p_wk->p_msg );

	//フォント破棄
	GFL_FONT_Delete( p_wk->p_font );

	//クリアー
	GFL_STD_MemClear( p_wk, sizeof(SCROLL_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SCROLL	メイン処理
 *
 *	@param	SCROLL_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SCROLL_Main( SCROLL_WORK *p_wk )
{	
	int i;

	//移動時のみ更新
	if( p_wk->is_move_req )
	{	
		p_wk->is_move_req	= FALSE;
		//張替えるチェック
		if( p_wk->y	% SCROLL_REWITE_DISTANCE == 0 )
		{				

			if( p_wk->top_bar != p_wk->y / SCROLL_REWITE_DISTANCE )
			{	
				PMSND_PlaySE( IRCRANK_SE_SLIDE );
			}

			//描画先頭バーを計算
			p_wk->top_bar						= p_wk->y / SCROLL_REWITE_DISTANCE;
			p_wk->rewrite_scr_pos_m	= SCROLL_MARGIN_SIZE_Y_M;
			p_wk->rewrite_scr_pos_s	= SCROLL_MARGIN_SIZE_Y_S;
						
	DEBUG_TICK_DRAW_Start;
			Scroll_Write( p_wk );
	DEBUG_TICK_DRAW_Print;

			p_wk->is_update	= TRUE;	//VBlankの描画をUPDATE
		}
		else
		{	
			p_wk->is_move_update	= TRUE;
		}

	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	SCROLL	位置
 *
 *	@param	SCROLL_WORK *p_wk	ワーク
 *	@param	y									Y
 *
 */
//-----------------------------------------------------------------------------
static void SCROLL_AddPos( SCROLL_WORK *p_wk, s16 y )
{	
	//移動制限
	if( p_wk->top_limit_y > p_wk->y + y )
	{	
		p_wk->y	= p_wk->top_limit_y;
		p_wk->add_y		=	 0;
	}
	else if( p_wk->bottom_limit_y < p_wk->y + y )
	{	
		p_wk->y	= p_wk->bottom_limit_y;
		p_wk->add_y		=	 0;
	}
	else
	{	
		//更新タイミングを越えていたら修正
		//NAGI_Printf( "left %d right %d y%d\n", (p_wk->y / SCROLL_REWITE_DISTANCE),((p_wk->y+y)	/ SCROLL_REWITE_DISTANCE), y  );
		if( (p_wk->y / SCROLL_REWITE_DISTANCE) != ((p_wk->y+y)	/ SCROLL_REWITE_DISTANCE) &&
				p_wk->y%SCROLL_REWITE_DISTANCE!=0)
		{	
			if( 0 <= y )
			{	
				y	= y - ((p_wk->y+y) % SCROLL_REWITE_DISTANCE);
				//NAGI_Printf( "- Y POS %d add%d ofs%d\n", p_wk->y, y, p_wk->y%SCROLL_REWITE_DISTANCE );
			}
			else
			{	
				//NAGI_Printf( "te %d  %d\n", y, SCROLL_REWITE_DISTANCE-((p_wk->y+y) % SCROLL_REWITE_DISTANCE) );
				y	= y + (SCROLL_REWITE_DISTANCE-((p_wk->y+y) % SCROLL_REWITE_DISTANCE));
				//NAGI_Printf( "- Y POS %d add%d ofs%d\n", p_wk->y, y, p_wk->y%SCROLL_REWITE_DISTANCE );
			}
		}
	
		//加算
		p_wk->y				+= y;
		p_wk->add_y		=	 y;

		//上に移動
	//	NAGI_Printf( "Ypos %d top%d \n", p_wk->y, p_wk->y / SCROLL_REWITE_DISTANCE );
	}
		
	p_wk->is_move_req	= TRUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief	スクロール位置を取得
 *
 *	@param	const SCROLL_WORK *cp_wk	ワーク
 *
 *	@return	スクロール位置
 */
//-----------------------------------------------------------------------------
static s16 SCROLL_GetPosY( const SCROLL_WORK *cp_wk )
{	
	return cp_wk->y;
}
//----------------------------------------------------------------------------
/**
 *	@brief	新規ランクインの座標を取得
 *
 *	@param	const SCROLL_WORK *cp_wk	ワーク 
 *
 *	@return	新規ランクインの座標
 */
//-----------------------------------------------------------------------------
static s16 SCROLL_GetNewRankPosY( const SCROLL_WORK *cp_wk )
{	
	int i;
	BOOL is_exist	= FALSE;
	for( i = 0; i < cp_wk->data_len; i++ )
	{	
		if( cp_wk->cp_data[i].plt == 5 )
		{	
			is_exist	= TRUE;
			break;
		}
	}
	if( !is_exist )
	{	
		return SCROLL_NEWRANK_NULL;
	}

	return i * SCROLL_BAR_HEIGHT * GFL_BG_1CHRDOTSIZ;
}
//----------------------------------------------------------------------------
/**
 *	@brief	移動下限値を取得
 *
 *	@param	const SCROLL_WORK *cp_wk ワーク
 *
 *	@return	移動下限値
 */
//-----------------------------------------------------------------------------
static s16 SCROLL_GetBottomLimitPosY( const SCROLL_WORK *cp_wk )
{	
	return cp_wk->bottom_limit_y;
}
//----------------------------------------------------------------------------
/**
 *	@brief	１つのバーを描画
 *
 *	@param	SCROLL_WORK *p_wk				ワーク
 *	@param	RANKING_DATA *cp_rank		ランキングデータ（１つ）
 *	@param	y_chr										Yキャラ位置
 *	@param	is_top									TRUEならば上画面	FALSなら下画面
 *
 */
//-----------------------------------------------------------------------------
static void Scroll_WriteRank( SCROLL_WORK *p_wk, const RANKING_DATA *cp_rank, const GFL_BMP_DATA *cp_bmp, u16 y_chr, BOOL is_top )
{
	GFL_BMP_DATA*	p_bmp_font;
	GFL_BMPWIN*		p_bmpwin;
	u8						bar_frm;
	u16						start_char_num;
	u16						font_chr_num;
	
	//上下どちらに書き込むか設定
	if( is_top )
	{	
		p_bmpwin		= p_wk->p_bmpwin[SCROLL_BMPWIN_FONT_M];
		p_bmp_font	= GFL_BMPWIN_GetBmp( p_bmpwin );
		bar_frm			= p_wk->bar_frm_m;
		start_char_num	= GRAPHIC_BG_GetTransInfo( p_wk->cp_bg, TRUE );
		font_chr_num	= GFL_BMPWIN_GetChrNum( p_bmpwin );
	}
	else
	{	
		p_bmpwin		= p_wk->p_bmpwin[SCROLL_BMPWIN_FONT_S];
		p_bmp_font	= GFL_BMPWIN_GetBmp( p_bmpwin );
		bar_frm			= p_wk->bar_frm_s;
		start_char_num	= GRAPHIC_BG_GetTransInfo( p_wk->cp_bg, FALSE );
		font_chr_num	= GFL_BMPWIN_GetChrNum( p_bmpwin );
	}

	//枠描画
	//ランキング
	//左上
	GFL_BG_FillScreen( bar_frm, (start_char_num+0),
					SCROLL_BAR_X, y_chr, 1, 1, cp_rank->plt );	
	//左下
	GFL_BG_FillScreen( bar_frm, (start_char_num+4),
					SCROLL_BAR_X, y_chr+SCROLL_BAR_HEIGHT-1, 1, 1, cp_rank->plt );	

	//右上
	GFL_BG_FillScreen( bar_frm, (start_char_num+3),
					SCROLL_BAR_X+SCROLL_BAR_WIDTH-1, y_chr, 1, 1, cp_rank->plt );	
	//右下
	GFL_BG_FillScreen( bar_frm, (start_char_num+7),
					SCROLL_BAR_X+SCROLL_BAR_WIDTH-1, y_chr+SCROLL_BAR_HEIGHT-1, 1, 1, cp_rank->plt );	

	//順位空欄
	//白上
	GFL_BG_FillScreen( bar_frm, (start_char_num+1),
					SCROLL_BAR_X+1, y_chr, 3, 1, cp_rank->plt );	
	//白下
	GFL_BG_FillScreen( bar_frm, (start_char_num+5),
					SCROLL_BAR_X+1, y_chr+SCROLL_BAR_HEIGHT-1, 3, 1, cp_rank->plt );	

	//しきい上
	GFL_BG_FillScreen( bar_frm, (start_char_num+2),
					SCROLL_BAR_X+4, y_chr, 1, 1, cp_rank->plt );	
	//しきい下
	GFL_BG_FillScreen( bar_frm, (start_char_num+6),
					SCROLL_BAR_X+4, y_chr+SCROLL_BAR_HEIGHT-1, 1, 1, cp_rank->plt );

	//名前空欄
	//白上
	GFL_BG_FillScreen( bar_frm, (start_char_num+1),
					SCROLL_BAR_X+5, y_chr, 8, 1, cp_rank->plt );	
	//白下
	GFL_BG_FillScreen( bar_frm, (start_char_num+5),
					SCROLL_BAR_X+5, y_chr+SCROLL_BAR_HEIGHT-1, 8, 1, cp_rank->plt );	

	//しきい上
	GFL_BG_FillScreen( bar_frm, (start_char_num+2),
					SCROLL_BAR_X+13, y_chr, 1, 1, cp_rank->plt );	
	//しきい下
	GFL_BG_FillScreen( bar_frm, (start_char_num+6),
					SCROLL_BAR_X+13, y_chr+SCROLL_BAR_HEIGHT-1, 1, 1, cp_rank->plt );

	//点数空欄
	//白上
	GFL_BG_FillScreen( bar_frm, (start_char_num+1),
					SCROLL_BAR_X+14, y_chr, 5, 1, cp_rank->plt );	
	//白下
	GFL_BG_FillScreen( bar_frm, (start_char_num+5),
					SCROLL_BAR_X+14, y_chr+SCROLL_BAR_HEIGHT-1, 5, 1, cp_rank->plt );	


	//カウント
	//左上
	GFL_BG_FillScreen( bar_frm, (start_char_num+0),
					SCROLL_BAR_CNT_X-1, y_chr, 1, 1, cp_rank->plt );	
	//左下
	GFL_BG_FillScreen( bar_frm, (start_char_num+4),
					SCROLL_BAR_CNT_X-1, y_chr+SCROLL_BAR_CNT_HEIGHT-1, 1, 1, cp_rank->plt );	
	//右上
	GFL_BG_FillScreen( bar_frm, (start_char_num+3),
					SCROLL_BAR_CNT_X+SCROLL_BAR_CNT_WIDTH, y_chr, 1, 1, cp_rank->plt );	
	//右下
	GFL_BG_FillScreen( bar_frm, (start_char_num+7),
					SCROLL_BAR_CNT_X+SCROLL_BAR_CNT_WIDTH, y_chr+SCROLL_BAR_CNT_HEIGHT-1, 1, 1, cp_rank->plt );	
	//白上
	GFL_BG_FillScreen( bar_frm, (start_char_num+1),
					SCROLL_BAR_CNT_X, y_chr, SCROLL_BAR_CNT_WIDTH, 1, cp_rank->plt );	
	//白下
	GFL_BG_FillScreen( bar_frm, (start_char_num+5),
					SCROLL_BAR_CNT_X, y_chr+SCROLL_BAR_CNT_HEIGHT-1, SCROLL_BAR_CNT_WIDTH, 1, cp_rank->plt );	
	
	//文字書き込み
#if 0
	GFL_BMP_Print( cp_bmp, p_bmp_font, 0, 0, 0, y_chr*GFL_BG_1CHRDOTSIZ,
      SCROLL_BAR_ALL_WIDTH*GFL_BG_1CHRDOTSIZ, SCROLL_BAR_ALL_HEIGHT*GFL_BG_1CHRDOTSIZ, 0 );
#else
	BMP_Copy( cp_bmp, p_bmp_font, 0, 0, 0, y_chr, SCROLL_BAR_ALL_WIDTH, SCROLL_BAR_ALL_HEIGHT );

#endif

}
//----------------------------------------------------------------------------
/**
 *	@brief	書き込みし転送する処理
 *
 *	@param	SCROLL_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void Scroll_Write( SCROLL_WORK *p_wk )
{
	int i;
	int print_y;

	//一端クリア
	GFL_BG_FillScreen( p_wk->bar_frm_m, 0, 0, 0, 32, 24, 0 );	
	GFL_BG_FillScreen( p_wk->bar_frm_s, 0, 0, 0, 32, 24, 0 );	
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[SCROLL_BMPWIN_FONT_M]), 0 );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[SCROLL_BMPWIN_FONT_S]), 0 );

	//上画面書き込み
	for( i = SCROLL_WRITE_BAR_START_EX_M+p_wk->top_bar; i < SCROLL_WRITE_BAR_END_M+p_wk->top_bar; i++ )
	{	
		if( 0 <= i && i < p_wk->data_len )
		{
			//SCROLL_WRITE_BAR_START_EX_Mが-だと、基準値の前にプリントする
			print_y	= (i-(p_wk->top_bar+SCROLL_WRITE_BAR_START_M))*
				SCROLL_BAR_HEIGHT+SCROLL_WRITE_POS_START_M;

			Scroll_WriteRank( p_wk, &p_wk->cp_data[i], p_wk->pp_bmp[i],
					print_y, TRUE );
		}
	}

	//下[画面書き込み
	for( i = SCROLL_WRITE_BAR_START_EX_S+p_wk->top_bar; i < SCROLL_WRITE_BAR_END_S+p_wk->top_bar; i++ )
	{	
		if( 0 <= i &&i < p_wk->data_len )
		{
			print_y	= (i-(p_wk->top_bar+SCROLL_WRITE_BAR_START_S))*
				SCROLL_BAR_HEIGHT+SCROLL_WRITE_POS_START_S;

			Scroll_WriteRank( p_wk, &p_wk->cp_data[i], p_wk->pp_bmp[i],
					print_y, FALSE );
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	書き込みVBlank版
 *
 *	@param	SCROLL_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void Scroll_WriteVBlank( SCROLL_WORK *p_wk )
{	
	int i;

	//転送処理
	for( i = 0; i < SCROLL_BMPWIN_MAX; i++ )
	{	
		GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin[i] );
		GFL_BMPWIN_MakeScreen( p_wk->p_bmpwin[i] );
	}

	GFL_BG_LoadScreenReq( p_wk->bar_frm_m );
	GFL_BG_LoadScreenReq( p_wk->bar_frm_s );
	GFL_BG_LoadScreenReq( p_wk->font_frm_m );
	GFL_BG_LoadScreenReq( p_wk->font_frm_s );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SCROLL	VBLANK
 *
 *	@param	GFL_TCB *p_tcb	タスク
 *	@param	*p_work					ワーク
 *
 */
//-----------------------------------------------------------------------------
static void Scroll_VBlankTask( GFL_TCB *p_tcb, void *p_work )
{	
	SCROLL_WORK *p_wk	= p_work;
		
	//転送
	if( p_wk->is_update )
	{		
	DEBUG_TICK_DRAW_Start;
		Scroll_WriteVBlank( p_wk );
	DEBUG_TICK_DRAW_Print;

		p_wk->is_update	= FALSE;

		//転送後の張替え処理のため移動する
		p_wk->is_move_update	= TRUE;
	}

	//移動
	if( p_wk->is_move_update )
	{	
		s16 move_ofs;
		s16 pos_m;
		s16 pos_s;

		//移動オフセット
		move_ofs	= p_wk->y % SCROLL_REWITE_DISTANCE;

		//もし前回と同じ方向だったらが0ならば通常移動
		//0のみ例外移動
		if( move_ofs == 0 )
		{	
				pos_m	= p_wk->rewrite_scr_pos_m + move_ofs;
				pos_s	= p_wk->rewrite_scr_pos_s + move_ofs;
				//NAGI_Printf(  "ゼロ移動量 %d\n", move_ofs );
				p_wk->pre_dir	= p_wk->add_y;
				p_wk->is_zero_next	= TRUE;	
		}
		else  if( (0 <= p_wk->add_y && 0 <= p_wk->pre_dir)
			|| (p_wk->add_y < 0 && p_wk->pre_dir < 0) || p_wk->is_zero_next )
		{	
			p_wk->is_zero_next	= FALSE;

			//移動処理
			if( 0<= p_wk->add_y )
			{	
				pos_m	= p_wk->rewrite_scr_pos_m + move_ofs;
				pos_s	= p_wk->rewrite_scr_pos_s + move_ofs;
				p_wk->pre_dir	= 1;
				//NAGI_Printf(  "移動量 %d\n", move_ofs );
			}
			else
			{	
				pos_m	= p_wk->rewrite_scr_pos_m - ( SCROLL_REWITE_DISTANCE - move_ofs - 1 );
				pos_s	= p_wk->rewrite_scr_pos_s - ( SCROLL_REWITE_DISTANCE - move_ofs - 1 );
				p_wk->pre_dir	= -1;
				//NAGI_Printf(  "移動量 %d 補正%d\n", move_ofs, ( SCROLL_REWITE_DISTANCE - move_ofs - 1 ));
			}
		}
		else
		{	
			//違う場合は、張替え位置まで、前回と同じ移動方法
			if( 0<= p_wk->pre_dir )
			{	
				pos_m	= p_wk->rewrite_scr_pos_m + move_ofs;
				pos_s	= p_wk->rewrite_scr_pos_s + move_ofs;
				//NAGI_Printf(  "pre移動量 %d\n", move_ofs );
				p_wk->pre_dir	= 1;
			}
			else
			{	
				pos_m	= p_wk->rewrite_scr_pos_m - ( SCROLL_REWITE_DISTANCE - move_ofs - 1 );
				pos_s	= p_wk->rewrite_scr_pos_s - ( SCROLL_REWITE_DISTANCE - move_ofs - 1 );
				p_wk->pre_dir	= -1;
				//NAGI_Printf(  "pre移動量 %d 補正%d\n", move_ofs, ( SCROLL_REWITE_DISTANCE - move_ofs - 1 ));
			}

		}

		GFL_BG_SetScroll( p_wk->bar_frm_m, GFL_BG_SCROLL_Y_SET, pos_m );
		GFL_BG_SetScroll( p_wk->font_frm_m, GFL_BG_SCROLL_Y_SET, pos_m );
		GFL_BG_SetScroll( p_wk->bar_frm_s, GFL_BG_SCROLL_Y_SET, pos_s );
		GFL_BG_SetScroll( p_wk->font_frm_s, GFL_BG_SCROLL_Y_SET, pos_s );

		p_wk->is_move_update	= FALSE;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	書き込みフォントデータを作成
 *
 *	@param	RANKING_DATA* cp_data		データ
 *	@param	data_len								データ幅
 *	@param	heapID									ヒープID
 *
 *	@return	GFL_BMP_DATA配列
 */
//-----------------------------------------------------------------------------
static GFL_BMP_DATA **Scroll_CreateWriteData( SCROLL_WORK *p_wk, HEAPID heapID )
{	
	GFL_BMP_DATA **pp_bmp;

	//バッファ作成
	pp_bmp	= GFL_HEAP_AllocMemory( heapID, sizeof(GFL_BMP_DATA*)*p_wk->data_len );
	GFL_STD_MemClear( pp_bmp, sizeof(GFL_BMP_DATA*)*p_wk->data_len );

	//データ作成
	{	
		int i;
		for( i = 0; i < p_wk->data_len; i++ )
		{	
			pp_bmp[i]	= GFL_BMP_Create( SCROLL_BAR_ALL_WIDTH, SCROLL_BAR_ALL_HEIGHT,
					GFL_BMP_16_COLOR, heapID );

			//文字作成
			GFL_MSG_GetString( p_wk->p_msg, RANKING_RANK_000, p_wk->p_rank_buf );
			GFL_MSG_GetString( p_wk->p_msg, RANKING_SCORE_000, p_wk->p_score_buf );
			GFL_MSG_GetString( p_wk->p_msg, RANKING_COUNT_000, p_wk->p_count_buf );
			WORDSET_RegisterNumber( p_wk->p_wordset, 0, p_wk->cp_data[i].rank, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
			WORDSET_RegisterNumber( p_wk->p_wordset, 2, p_wk->cp_data[i].score, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
			WORDSET_RegisterNumber( p_wk->p_wordset, 3, p_wk->cp_data[i].play_cnt, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );

			//文字描画
			//ランク
			WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_strbuf, p_wk->p_rank_buf );
			PRINTSYS_Print( pp_bmp[i], SCROLL_BAR_FONT_RANK_X*GFL_BG_1CHRDOTSIZ+3, SCROLL_FONT_Y_OFS, p_wk->p_strbuf, p_wk->p_font );
			//名前
			PRINTSYS_Print( pp_bmp[i], SCROLL_BAR_FONT_PLAYER_X*GFL_BG_1CHRDOTSIZ+3, SCROLL_FONT_Y_OFS, p_wk->cp_data[i].p_name, p_wk->p_font );
			//点数
			WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_strbuf, p_wk->p_score_buf );
			PRINTSYS_Print( pp_bmp[i], SCROLL_BAR_FONT_SCORE_X*GFL_BG_1CHRDOTSIZ-3, SCROLL_FONT_Y_OFS, p_wk->p_strbuf, p_wk->p_font );
			//回数
			WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_strbuf, p_wk->p_count_buf );
			PRINTSYS_Print( pp_bmp[i], SCROLL_BAR_FONT_COUNT_X*GFL_BG_1CHRDOTSIZ+4, SCROLL_FONT_Y_OFS, p_wk->p_strbuf, p_wk->p_font );
		}
	}

	return pp_bmp;
}
//----------------------------------------------------------------------------
/**
 *	@brief	書き込みデータ破棄
 *
 *	@param	GFL_BMP_DATA *p_data	データ
 *	@param	data_len							データ幅
 *
 */
//-----------------------------------------------------------------------------
static void Scroll_DeleteWriteData( GFL_BMP_DATA **pp_data, u16 data_len )
{	
	int i;
	for( i = 0; i < data_len; i++ )
	{	
		GFL_BMP_Delete( pp_data[i] );
	}

	GFL_HEAP_FreeMemory( pp_data );
}
//=============================================================================
/**
 *	RANKING_DATA
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ランキングデータ作成
 *
 *	@param	u16 data_len	データ長
 *	@param	heapID				ヒープID
 *
 *	@return	データ
 */
//-----------------------------------------------------------------------------
static RANKING_DATA	*RANKING_DATA_Create( u16 data_len, HEAPID heapID )
{	
	RANKING_DATA	*p_data;

	//確保
	p_data	= GFL_HEAP_AllocMemory( heapID, sizeof(RANKING_DATA)*data_len );
	GFL_STD_MemClear( p_data, sizeof(RANKING_DATA)*data_len );

	//データ読み込み
	{	
		IRC_COMPATIBLE_SAVEDATA *p_sv	= IRC_COMPATIBLE_SV_GetSavedata( SaveControl_GetPointer() );
		u32 rank_max;
		u8	new_rank;
		int i;
		u8	pre_score	= 128;	//100以上のあたい
		u8	pre_rank	= 0;

		rank_max	= IRC_COMPATIBLE_SV_GetRankNum( p_sv );
		new_rank	= IRC_COMPATIBLE_SV_GetNewRank( p_sv );
		GF_ASSERT( rank_max <= data_len );
		for( i = 0; i < rank_max; i++ )
		{	
			//名前
			p_data[i].p_name	= GFL_STR_CreateBuffer( IRC_COMPATIBLE_SV_DATA_NAME_LEN, heapID );
			GFL_STR_SetStringCode( p_data[i].p_name, IRC_COMPATIBLE_SV_GetPlayerName( p_sv, i ) );	
			//スコア
			p_data[i].score	= IRC_COMPATIBLE_SV_GetScore( p_sv, i );
				
			//回数
			p_data[i].play_cnt	= IRC_COMPATIBLE_SV_GetPlayCount( p_sv, i );
			//ランク
			//点数が下ならそのときのランク
			//同じならば同ランク
			if( p_data[i].score < pre_score )
			{	
				p_data[i].rank	= i+1;
			}
			else if( p_data[i].score == pre_score )
			{	
				p_data[i].rank	= pre_rank;
			}
			else
			{	
				GF_ASSERT(0);
			}
			//パレット
			if( new_rank == i )
			{	
				p_data[i].plt		= 5;
			}else if( p_data[i].rank == 1 )
			{	
				p_data[i].plt		= 2;
			}
			else if( p_data[i].rank == 2 )
			{	
				p_data[i].plt		= 3;
			}
			else if( p_data[i].rank == 3 )
			{	
				p_data[i].plt		= 4;
			}
			else
			{	
				p_data[i].plt		= 1;
			}

			//前の人のスコア保存
			pre_score	= p_data[i].score;
			pre_rank	= p_data[i].rank;
		}
	}

	return p_data;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ランキングデータ破棄
 *
 *	@param	RANKING_DATA	*p_data		データ
 *
 */
//-----------------------------------------------------------------------------
static void RANKING_DATA_Delete( RANKING_DATA	*p_data )
{
	{	
		IRC_COMPATIBLE_SAVEDATA *p_sv	= IRC_COMPATIBLE_SV_GetSavedata( SaveControl_GetPointer() );
		u32 rank_max;
		int i;

		rank_max	= IRC_COMPATIBLE_SV_GetRankNum( p_sv );
		for( i = 0; i < rank_max; i++ )
		{	
			if( p_data[i].p_name )
			{	
				GFL_STR_DeleteBuffer( p_data[i].p_name );
			}
		}
	}

	GFL_HEAP_FreeMemory( p_data );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ランキングデータが存在している数取得
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static u16 RANKING_DATA_GetExistLength( void )
{	
	IRC_COMPATIBLE_SAVEDATA *p_sv	= IRC_COMPATIBLE_SV_GetSavedata( SaveControl_GetPointer() );
	return IRC_COMPATIBLE_SV_GetRankNum( p_sv );
}
//=============================================================================
/**
 *	UI
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
	u32 x, y;
	//タッチの処理
	if( GFL_UI_TP_GetPointTrg( &x, &y ) )
	{	
		p_wk->start.x		= x;
		p_wk->start.y		= y;
		p_wk->end.x		= x;
		p_wk->end.y		= y;

		p_wk->flik_sync	= 0;
	}
	else if(GFL_UI_TP_GetPointCont( &x, &y ) )
	{	
		p_wk->end.x		= x;
		p_wk->end.y		= y;

		p_wk->flik_sync++;
		p_wk->release_sync	= 0;
	}
	else
	{	
		p_wk->release_sync++;
	}

	//キーの処理
	if( GFL_UI_KEY_GetTrg() )
	{
		p_wk->key_cont_sync	= 0;
	}
	else if( GFL_UI_KEY_GetCont() )
	{	
		p_wk->key_cont_sync++;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	UI	ドラッグ情報取得
 *
 *	@param	const UI_WORK *p_wk	ワーク
 *	@param	*p_start						開始点
 *	@param	*p_end							終了点
 *	@param	*p_vec							ベクトル
 *
 */
//-----------------------------------------------------------------------------
static BOOL UI_GetDrag( const UI_WORK *cp_wk, GFL_POINT *p_start, GFL_POINT *p_end, VecFx32 *p_vec )
{	
	if( GFL_UI_TP_GetCont() )
	{	
		if( p_start )
		{	
			*p_start	= cp_wk->start;
		}
		if( p_end )
		{	
			*p_end	= cp_wk->end;
		}
		if( p_vec )
		{	
			VecFx32	s, e;
			VEC_Set( &s, FX32_CONST( cp_wk->start.x ), FX32_CONST( cp_wk->start.y ), 0 );
			VEC_Set( &e, FX32_CONST( cp_wk->end.x ), FX32_CONST( cp_wk->end.y ), 0 );
			VEC_Subtract( &s, &e, p_vec );
		}
		return TRUE;
	}

	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	UI	はじき情報取得
 *
 *	@param	const UI_WORK *p_wk	ワーク
 *	@param	*p_start	開始点
 *	@param	*p_end		終了点
 *	@param	*p_vec		ベクトル
 *	@param	*p_sync		はじきにかかったシンク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL UI_GetFlik( UI_WORK *p_wk, GFL_POINT *p_start, GFL_POINT *p_end, VecFx32 *p_vec, u32 *p_sync )
{	
	if( !GFL_UI_KEY_GetCont() && p_wk->release_sync < UI_FLIK_RELEASE_SYNC )
	{	
		//1回しか取得できないように
		p_wk->release_sync	= UI_FLIK_RELEASE_SYNC;
		if( p_start )
		{	
			*p_start	= p_wk->start;
		}
		if( p_end )
		{	
			*p_end	= p_wk->end;
		}
		if( p_vec )
		{	
			VecFx32	s, e;
			VEC_Set( &s, FX32_CONST( p_wk->start.x ), FX32_CONST( p_wk->start.y ), 0 );
			VEC_Set( &e, FX32_CONST( p_wk->end.x ), FX32_CONST( p_wk->end.y ), 0 );
			VEC_Subtract( &s, &e, p_vec );
		}
		if( p_sync )
		{	
			*p_sync	= p_wk->flik_sync;
		}
		return TRUE;
	}
	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	キー押しつづけを取得
 *
 *	@param	const UI_WORK *cp_wk	ワーク
 *	@param	*p_sync								押し続けている時間
 *
 *	@return	TRUEなら押している
 */
//-----------------------------------------------------------------------------
static int UI_IsContKey( const UI_WORK *cp_wk, u32 *p_sync )
{	
	if( p_sync )
	{	
		*p_sync	= cp_wk->key_cont_sync;
	}
	return GFL_UI_KEY_GetCont();
}
//=============================================================================
/**
 *	ACLR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ACLR	初期化
 *
 *	@param	ACLR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void ACLR_Init( ACLR_WORK *p_wk )
{	
	GFL_STD_MemClear(p_wk, sizeof(ACLR_WORK));
}
//----------------------------------------------------------------------------
/**
 *	@brief	ACLR	破棄
 *
 *	@param	ACLR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void ACLR_Exit( ACLR_WORK *p_wk )
{	
	GFL_STD_MemClear(p_wk, sizeof(ACLR_WORK));
}
//----------------------------------------------------------------------------
/**
 *	@brief	ACLR	メイン処理
 *
 *	@param	ACLR_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void ACLR_Main( ACLR_WORK *p_wk )
{	
	if( p_wk->is_update )
	{
		p_wk->is_update	= FALSE;
		p_wk->aclr	= p_wk->distance_rate	+ p_wk->sync_rate / 2;
		p_wk->aclr	= FX_Mul( p_wk->aclr, ACLR_SCROLL_ACLR_DIF ) + ACLR_SCROLL_ACLR_MIN;
	}
	//NAGI_Printf( "now aclr %d\n", p_wk->aclr );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ACLR	加速を決める値をセット
 *
 *	@param	ACLR_WORK *p_wk	ワーク
 *	@param	distance				距離
 *	@param	sync						シンク
 *
 */
//-----------------------------------------------------------------------------
static void ACLR_SetAclr( ACLR_WORK *p_wk, fx32 distance, u32 sync )
{	
	//上限・下限以外ならばはじく
	if( ACLR_SCROLL_DISTANCE_MIN < MATH_IAbs(distance) )
	{	
		p_wk->dir	= distance/ MATH_IAbs(distance);
		distance	= MATH_IAbs( distance );
		distance	= MATH_CLAMP( distance, ACLR_SCROLL_DISTANCE_MIN, ACLR_SCROLL_DISTANCE_MAX );
	}
	else
	{
		return ;
	}
	if( FX32_CONST(sync) < ACLR_SCROLL_SYNC_MAX ) 
	{	
		sync			= MATH_CLAMP( FX32_CONST(sync), ACLR_SCROLL_SYNC_MIN, ACLR_SCROLL_SYNC_MAX );
	}
	else
	{
		return ;
	}

	//それぞれのレートを計算
	p_wk->distance_rate	=	FX_Div((distance - ACLR_SCROLL_DISTANCE_MIN), ACLR_SCROLL_DISTANCE_DIF);
	//syncはMINの方が良い値（=RateではMAXに近くしたい）ので反転する
	p_wk->sync_rate	=	FX_Div((sync - ACLR_SCROLL_SYNC_MIN), ACLR_SCROLL_SYNC_DIF);
	p_wk->sync_rate	= FX32_ONE - p_wk->sync_rate;

	//OS_Printf( "距離%f\n シンク%f\n", FX_FX32_TO_F32(distance), FX_FX32_TO_F32(sync) );
	//OS_Printf( "rate dis%f sync%f\n", FX_FX32_TO_F32(p_wk->distance_rate), FX_FX32_TO_F32(p_wk->sync_rate) );

	p_wk->is_update	= TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ACLR	停止処理
 *
 *	@param	p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void ACLR_Stop( ACLR_WORK *p_wk )
{	
	p_wk->aclr	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ACLR	スクロール加速度を取得
 *
 *	@param	const ACLR_WORK *cp_wk	ワーク
 *
 *	@return	スクロール加速度
 */
//-----------------------------------------------------------------------------
static s16 ACLR_GetScrollAdd( ACLR_WORK *p_wk )
{	
	
	fx32 scroll_add;
	fx32 aclr_dec;

	//スクロール速度
	scroll_add	= MATH_CLAMP( p_wk->aclr*p_wk->dir, ACLR_SCROLL_MOVE_MIN, ACLR_SCROLL_MOVE_MAX );

	//減衰
	aclr_dec		= FX_Mul(p_wk->aclr, ACLR_SCROLL_DEC_RATE);
	p_wk->aclr	-= aclr_dec;
	if( -FX32_ONE <p_wk->aclr && p_wk->aclr < FX32_ONE)
	{	
		p_wk->aclr	= 0;
	}
	//OS_Printf("スクロールAdd %d\n",  scroll_add>>FX32_SHIFT );

	return (scroll_add >> FX32_SHIFT);
}
//----------------------------------------------------------------------------
/**
 *	@brief	ACLR	加速が存在するか
 *
 *	@param	const ACLR_WORK *cp_wk	ワーク
 *
 *	@return	TRUEならば加速が存在
 */
//-----------------------------------------------------------------------------
static BOOL ACLR_IsExist( const ACLR_WORK *cp_wk )
{	
	return cp_wk->aclr != 0;
}
//=============================================================================
/**
 *	ETC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	センター位置へプリント
 *
 *	@param	GFL_BMPWIN *p_bmpwin	BMPWIN
 *	@param	*p_strbuf							文字
 *	@param	*p_font								フォント
 *
 */
//-----------------------------------------------------------------------------
static void PRINT_PrintCenter( GFL_BMPWIN *p_bmpwin, STRBUF *p_strbuf, GFL_FONT *p_font )
{	
	u16 x, y;

	x	= GFL_BMPWIN_GetSizeX( p_bmpwin )*4;
	y	= GFL_BMPWIN_GetSizeY( p_bmpwin )*4;
	x	-= PRINTSYS_GetStrWidth( p_strbuf, p_font, 0 )/2;
	y	-= PRINTSYS_GetStrHeight( p_strbuf, p_font )/2;

	PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_bmpwin), x, y, p_strbuf, p_font );
}

//----------------------------------------------------------------------------
/**
 *	@brief	BMPのキャラ単位コピー処理（PRINTはドット単位でコピーするのでより早い）
 *
 *	@param	const GFL_BMP_DATA *cp_src	転送元
 *	@param	*p_dst	転送先
 *	@param	src_x		転送元X座標（以下すべてキャラ単位）
 *	@param	src_y		転送元Y座標
 *	@param	dst_x		転送先X座標	
 *	@param	dst_y		転送先Y座標
 *	@param	w				転送幅
 *	@param	h				転送高さ
 *
 */
//-----------------------------------------------------------------------------
static void BMP_Copy( const GFL_BMP_DATA *cp_src, GFL_BMP_DATA *p_dst, u16 src_x, u16 src_y, u16 dst_x, u16 dst_y, u16 w, u16 h )
{	
	const u8	*cp_src_chr		= (const u8 *)GFL_BMP_GetCharacterAdrs( (GFL_BMP_DATA *)(cp_src) );
	u8	*p_dst_chr					= (u8 *)GFL_BMP_GetCharacterAdrs( p_dst );
	const u8 *cp_src_chr_ofs;
	u8 *p_dst_chr_ofs;

	cp_src_chr_ofs	= cp_src_chr + ((w * src_y) + src_x)*GFL_BG_1CHRDATASIZ;
	p_dst_chr_ofs		= p_dst_chr	+ ((w * dst_y) + dst_x)*GFL_BG_1CHRDATASIZ;

	GFL_STD_MemCopy32( cp_src_chr_ofs, p_dst_chr_ofs, w * h * GFL_BG_1CHRDATASIZ );
}

//----------------------------------------------------------------------------
/**
 *	@brief	POINT版距離を測る
 *
 *	@param	const GFL_POINT *cp_a
 *	@param	GFL_POINT *cp_b 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static s32 GFL_POINT_Distance( const GFL_POINT *cp_a, const GFL_POINT *cp_b )
{	
	VecFx32	a;
	VecFx32	b;

	VEC_Set( &a, FX32_CONST(cp_a->x), FX32_CONST(cp_a->y), 0);
	VEC_Set( &b, FX32_CONST(cp_b->x), FX32_CONST(cp_b->y), 0);

	return VEC_Distance( &a, &b ) >> FX32_SHIFT;
}
//----------------------------------------------------------------------------
/**
 *	@brief	Screenデータの　VRAM転送拡張版（読み込んだスクリーンの一部範囲をバッファに張りつける）
 *
 *	@param	ARCHANDLE *handle	ハンドル
 *	@param	datID							データID
 *	@param	frm								フレーム
 *	@param	chr_ofs						キャラオフセット
 *	@param	src_x							転送元ｘ座標
 *	@param	src_y							転送元Y座標
 *	@param	src_w							転送元幅			//データの全体の大きさ
 *	@param	src_h							転送元高さ		//データの全体の大きさ
 *	@param	dst_x							転送先X座標
 *	@param	dst_y							転送先Y座標
 *	@param	dst_w							転送先幅
 *	@param	dst_h							転送先高さ
 *	@param	plt								パレット番号
 *	@param	compressedFlag		圧縮フラグ
 *	@param	heapID						一時バッファ用ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void ARCHDL_UTIL_TransVramScreenEx( ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 chr_ofs, u8 src_x, u8 src_y, u8 src_w, u8 src_h, u8 dst_x, u8 dst_y, u8 dst_w, u8 dst_h, u8 plt, BOOL compressedFlag, HEAPID heapID )
{	
	void *p_src_arc;
	NNSG2dScreenData* p_scr_data;

	//読み込み
	p_src_arc = GFL_ARCHDL_UTIL_Load( handle, datID, compressedFlag, GetHeapLowID(heapID) );

	//アンパック
	if(!NNS_G2dGetUnpackedScreenData( p_src_arc, &p_scr_data ) )
	{	
		GF_ASSERT(0);	//スクリーンデータじゃないよ
	}

	//エラー
	GF_ASSERT( src_w * src_h * 2 <= p_scr_data->szByte );

	//キャラオフセット計算
	if( chr_ofs )
	{	
		int i;
		if( GFL_BG_GetScreenColorMode( frm ) == GX_BG_COLORMODE_16 )
		{
			u16 *p_scr16;
			
			p_scr16	=	(u16 *)&p_scr_data->rawData;
			for( i = 0; i < src_w * src_h ; i++ )
			{
				p_scr16[i]	+= chr_ofs;
			}	
		}
		else
		{	
			GF_ASSERT(0);	//256版は作ってない
		}
	}

	//書き込み
	if( GFL_BG_GetScreenBufferAdrs( frm ) != NULL )
	{	
		GFL_BG_WriteScreenExpand( frm, dst_x, dst_y, dst_w, dst_h,
				&p_scr_data->rawData, src_x, src_y, src_w, src_h );	
		GFL_BG_ChangeScreenPalette( frm, dst_x, dst_y, dst_w, dst_h, plt );
		GFL_BG_LoadScreenReq( frm );
	}
	else
	{	
		GF_ASSERT(0);	//バッファがない
	}

	GFL_HEAP_FreeMemory( p_src_arc );
}	
