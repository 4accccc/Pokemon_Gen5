
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		medalbox_sys.c
 *	@brief  メダルボックスメイン
 *	@author	Masayuki Onoue
 *	@data		2011.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "system/gfl_use.h"       // GFUser_VIntr_CreateTCB
#include "system/main.h"          // HEAPID
#include "arc_def.h"              // ARC_ID
#include "message.naix"           // MESSAGE_ARC_ID

#include "sound/pm_sndsys.h"      // サウンド

#include "print/printsys.h"
#include "print/gf_font.h"        // フォント
#include "font/font.naix"         // フォント

#include "medalbox_gra.naix"      // グラフィックアーカイブ

#include "app_menu_common.naix"   // 共通素材
#include "app/app_menu_common.h"  // 共通素材

#include "msg/msg_medalname.h"    // for MDL_NAME_HINT_000

#include "app/medalbox_sys.h"
#include "medalbox_data_sys.h"
#include "medalbox_sys_local.h"

FS_EXTERN_OVERLAY(ui_common);     // オーバーレイ

//============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
///	ヒープサイズ
//=====================================
#define MEDALBOXSYS_HEAP_SIZE (0x60000)

//-------------------------------------
///	リソース初期位置
//=====================================
#define UI_SCROLBER_POS_X (24*2+7*8+12)
#define UI_SCROLBER_POS_Y (180)

#define UI_SCROLBTN_POS_X (UI_SCROLBER_POS_X - 7*8)
#define UI_SCROLBTN_POS_Y (180)

#define UI_HUTA_POS_X     (256/2)
#define UI_HUTA_POS_Y     (192/2)

#define UI_EXIT_POS_X     (256-24)
#define UI_EXIT_POS_Y     (192-24)

#define UI_RETURN_POS_X   (256-24*2)
#define UI_RETURN_POS_Y   (192-24)

#define UI_CURSOR_RIGHT_POS_X (256-24*3)
#define UI_CURSOR_RIGHT_POS_Y (192-24)

#define UI_CURSOR_LEFT_POS_X  (24)
#define UI_CURSOR_LEFT_POS_Y  (192-24)

#define UI_SEARCH_POS_X   (0)
#define UI_SEARCH_POS_Y   (192-24)


//-------------------------------------
///	パレット定義
//=====================================
#define MEDALOBJ_PALETTENUM_MEDAL       (6)       // メダルOBJのパレット使用本数
#define MEDALOBJ_PALETTENUM_UI          (4)       // UIOBJのパレット使用本数
#define MEDALOBJ_PALETTENUM_COMMON_UI   (4)       // COMMONUIOBJのパレット使用本数

//-------------------------------------
///	アプリ設定
//=====================================
#define MEDALBOX_DRAW_PAGE_NUM_LIST     (5)       // ボックスモードが一覧の時に表示するページ数
#define MEDALBOX_DRAW_PAGE_NUM_NORMAL   (3)       // ボックスモードがノーマルに表示するページ数

#define HUTA_WIDTH                      (80)      // フタの幅
#define PAGE_WIDTH                      (80)      // ページの幅

#define NAME_BMPWIN_MAX                 (5)       // メダル名表示用BMPWINの数
#define MEDAL_NAME_BMPWIN_SIZE_W        (15)      // メダル名表示領域の幅（キャラクタ単位)
#define MEDAL_NAME_BMPWIN_SIZE_H        (3)       // メダル名表示領域の高さ（キャラクタ単位)
#define MEDAL_NAME_BMPWIN_PAL_NUM       (15)       // メダル名BMPWIN用パレットナンバー
#define MEDAL_NAME_BMPWIN_STR_COL_L     (2)       // 文字(Letter)
#define MEDAL_NAME_BMPWIN_STR_COL_S     (3)       // 影(Shadow)、
#define MEDAL_NAME_BMPWIN_STR_COL_B     (1)       // 背景色(Background)

#define MEDAL_INFO_BMPWIN_PAL_NUM       (15)      // メダル名BMPWIN用パレットナンバー
#define MEDAL_INFO_INFO_DRAW_X          (20)      // 説明文表示位置X
#define MEDAL_INFO_INFO_DRAW_Y          (128)     // 説明文表示位置Y
#define MEDAL_INFO_BMPWIN_STR_COL_L     (1)       // 文字(Letter)
#define MEDAL_INFO_BMPWIN_STR_COL_S     (2)       // 影(Shadow)、
#define MEDAL_INFO_BMPWIN_STR_COL_B     (0)       // 背景色(Background)

#define MEDAL_NAME_DRAW_PAGE            (1)       // メダル名を表示するページ

#define UP_MEDAL_POS_X                  (44)      // 上画面に表示するメダルの位置
#define UP_MEDAL_POS_Y                  (70)      // 上画面に表示するメダルの位置

#define DIR_KEY_SCROLL_SPEED  (0x7000)  // ページスクロールスピード
#define DIR_KEY_SCROLL_COUNT  (1)     // 方向キーでスクロールするページ数
#define LR_KEY_SCROLL_SPEED   (0x20000)  // ページスクロールスピード
#define LR_KEY_SCROLL_COUNT   (3)     // LRキーでスクロールするページ数
#define MEDAL_HIT_RADIUS      (16)    // メダルの半径（当たり判定に使用)

//-------------------------------------
///	VRAM設定
//=====================================
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

//-------------------------------------
///	BGフレーム
//=====================================
typedef enum
{
	MEDALBOXSYS_BG_FRAME_FONTBORD_M,	// 文字盤+バー
	MEDALBOXSYS_BG_FRAME_ETC_M,		    // 光沢 or メッセージウィンドウ
	MEDALBOXSYS_BG_FRAME_PAGEUNIT_M,  // ページユニット
	MEDALBOXSYS_BG_FRAME_FONT_M,		  // 文字用

	MEDALBOXSYS_BG_FRAME_MONITOR_S,		// モニター
	MEDALBOXSYS_BG_FRAME_EFFECTS_S,	  // モニター演出
	MEDALBOXSYS_BG_FRAME_BACK_S ,	    // 背景
	MEDALBOXSYS_BG_FRAME_FONT_S,	    // 説明文表示用

	MEDALBOXSYS_BG_FRAME_MAX,			//ソース内で使用
}MEDALBOXSYS_GRAPHIC_BG_FRAME;

//-------------------------------------
///	BGプライオリティー
//=====================================
typedef enum
{
  MEDALBOXSYS_BG_PRIORITY_FONT_M     = 0,
  MEDALBOXSYS_BG_PRIORITY_FONTBORD_M,
  MEDALBOXSYS_BG_PRIORITY_ETC_M,
  MEDALBOXSYS_BG_PRIORITY_PAGEUNIT_M,

  MEDALBOXSYS_BG_PRIORITY_EFFECTS_S   = 0,
  MEDALBOXSYS_BG_PRIORITY_FONT_S,
  MEDALBOXSYS_BG_PRIORITY_MONITOR_S,
  MEDALBOXSYS_BG_PRIORITY_BACK_S,
}MEDALBOXSYS_BG_PRIORITY;

//-------------------------------------
///	OBJプライオリティー(BG)
//=====================================
typedef enum
{
  MEDALBOXSYS_OBJ_PRIORITY_UI         = 0,
  MEDALBOXSYS_OBJ_PRIORITY_MEDAL      = 2,
}MEDALBOXSYS_OBJ_PRIORITY;


//-------------------------------------
///	OBJプライオリティー(ソフト)
//=====================================
typedef enum
{
  MEDALBOXSYS_OBJ_SOFT_PRIORITY_TOP,
  MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM,
  MEDALBOXSYS_OBJ_SOFT_PRIORITY_BOTTOM,
}MEDALBOXSYS_OBJ_SOFT_PRIORITY;

//-------------------------------------
///	CKWK
//=====================================
typedef enum
{
	MEDALBOXSYS_OBJ_CLWK_MEDAL1,
	MEDALBOXSYS_OBJ_CLWK_MEDAL2,
	MEDALBOXSYS_OBJ_CLWK_MEDAL3,
	MEDALBOXSYS_OBJ_CLWK_MEDAL4,
	MEDALBOXSYS_OBJ_CLWK_MEDAL5,
	MEDALBOXSYS_OBJ_CLWK_MEDAL6,
	MEDALBOXSYS_OBJ_CLWK_MEDAL7,
	MEDALBOXSYS_OBJ_CLWK_MEDAL8,
	MEDALBOXSYS_OBJ_CLWK_MEDAL9,
	MEDALBOXSYS_OBJ_CLWK_MEDAL10,
	MEDALBOXSYS_OBJ_CLWK_MEDAL11,
	MEDALBOXSYS_OBJ_CLWK_MEDAL12,
	MEDALBOXSYS_OBJ_CLWK_MEDAL13,
	MEDALBOXSYS_OBJ_CLWK_MEDAL14,
	MEDALBOXSYS_OBJ_CLWK_MEDAL15,
	MEDALBOXSYS_OBJ_CLWK_MEDAL16,
	MEDALBOXSYS_OBJ_CLWK_MEDAL17,
	MEDALBOXSYS_OBJ_CLWK_MEDAL18,
	MEDALBOXSYS_OBJ_CLWK_MEDAL19,
	MEDALBOXSYS_OBJ_CLWK_MEDAL20,
	MEDALBOXSYS_OBJ_CLWK_MEDAL21,
	MEDALBOXSYS_OBJ_CLWK_MEDAL22,
	MEDALBOXSYS_OBJ_CLWK_MEDAL23,
	MEDALBOXSYS_OBJ_CLWK_MEDAL24,
	MEDALBOXSYS_OBJ_CLWK_MEDAL25,
	MEDALBOXSYS_OBJ_CLWK_SCROLBTN,
	MEDALBOXSYS_OBJ_CLWK_SCROLBER,
	MEDALBOXSYS_OBJ_CLWK_CURSOR,
	MEDALBOXSYS_OBJ_CLWK_ICHIOSHI,
	MEDALBOXSYS_OBJ_CLWK_KIRAKIRA,
	MEDALBOXSYS_OBJ_CLWK_HUTA,
	MEDALBOXSYS_OBJ_CLWK_RETURN,
  MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT,
  MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT,
	MEDALBOXSYS_OBJ_CLWK_EXIT,
	MEDALBOXSYS_OBJ_CLWK_SEARCH,
	MEDALBOXSYS_OBJ_CLWK_UPMEDAL,         // 上画面用メダルOBJ
	MEDALBOXSYS_OBJ_CLWK_MAX,
	MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX      = MEDALBOXSYS_OBJ_CLWK_MEDAL25 + 1 ,// メダルのセルID最大数
	MEDALBOXSYS_OBJ_UI_CLWK_MAX         = MEDALBOXSYS_OBJ_CLWK_HUTA + 1,    // UIセルワークの管理ID
	MEDALBOXSYS_OBJ_COMMON_CLWK_MAX     = MEDALBOXSYS_OBJ_CLWK_EXIT + 1,    // コモンUIセルワークの管理ID
}MEDALBOXSYS_GRAPHIC_OBJ_CLWK ;

#define MEDAL_HIT_TABLE_MAX (MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX+1-PAGE_MEDAL_MAX-PAGE_MEDAL_MAX)  // 当たり判定テーブルの数(最初の5枚と最後の5枚には当たり判定がない)

//-------------------------------------
///	アーカイブタイプ
//=====================================
enum{
  ARC_TYPE_MEDAL_GRA,
  ARC_TYPE_COMMON_BUTTON_GRA,
  ARC_TYPE_MAX,
};

//-------------------------------------
///	BG面設定
//=====================================
static const struct 
{
	u32									frame;
	GFL_BG_BGCNT_HEADER	bgcnt_header;
	u32									mode;
}	sc_bgsetup[MEDALBOXSYS_BG_FRAME_MAX]	=
{	
	// MEDALBOXSYS_BG_FRAME_FONTBORD_M
	{	
		GFL_BG_FRAME0_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_FONTBORD_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_ETC_M
	{	
		GFL_BG_FRAME1_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_ETC_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_PAGEUNIT_M
	{	
		GFL_BG_FRAME2_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_PAGEUNIT_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_FONT_M
	{	
		GFL_BG_FRAME3_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_FONT_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},


	// MEDALBOXSYS_BG_FRAME_MONITOR_S
	{	
		GFL_BG_FRAME0_S,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_MONITOR_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_EFFECTS_S
	{	
		GFL_BG_FRAME1_S,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_EFFECTS_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_BACK_S
	{	
		GFL_BG_FRAME2_S,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_BACK_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_FONT_S
	{	
		GFL_BG_FRAME3_S,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_FONT_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
};
//フレーム取得用マクロ
#define GRAPHIC_BG_GetFrame( x )  (sc_bgsetup[ x ].frame)

// ========================
// メダル座標
// ========================
static const struct
{
  s32 x;
  s32 y;
}	sc_medal_pos[PAGE_MEDAL_MAX]	=
{
  { 24      , 20        },
  { 24 + 24 , 20 + 32   },
  { 24      , 20 + 64   },
  { 24 + 24 , 20 + 96   },
  { 24      , 20 + 128  },
};

// ========================
// イチオシボタン座標
// ========================
static const struct
{
  s32 x;
  s32 y;
}	sc_ichioshi_pos[PAGE_MEDAL_MAX]	=
{
  { 224      , 24       },
  { 224 + 24 , 24 + 32  },
  { 224      , 24 + 64  },
  { 224 + 24 , 24 + 96  },
  { 224      , 24 + 128 },
};

// ========================
// メダル名表示ウィンドウ座標
// ========================
static const struct
{
  s32 x; // ドット単位
  s32 y; // ドット単位
}	sc_medalname_win_pos[NAME_BMPWIN_MAX]	=
{
  { 12*8 , 1*8+4 },
  { 14*8 , 5*8+4 },
  { 12*8 , 9*8+4 },
  { 14*8 , 13*8+4 },
  { 12*8 , 17*8+4 },
};

// ========================
// UI用当たり判定テーブル(トリガーボタン)
// ========================
static const GFL_UI_TP_HITTBL sc_ui_hittbl_trg[UI_HIT_TABLE_TRG_MAX] =
{
  //  UI_HIT_TABLE_TRG_SEARCH
  { 
    UI_SEARCH_POS_Y ,
    UI_SEARCH_POS_Y + 24,
    UI_SEARCH_POS_X ,
    UI_SEARCH_POS_X + 24,
  },
  //  UI_HIT_TABLE_TRG_CURSOR_LEFT
  { 
    UI_CURSOR_LEFT_POS_Y ,
    UI_CURSOR_LEFT_POS_Y + 24 ,
    UI_CURSOR_LEFT_POS_X ,
    UI_CURSOR_LEFT_POS_X + 24,
  }, 
  //  UI_HIT_TABLE_TRG_CURSOR_RIGHT
  { 
    UI_CURSOR_RIGHT_POS_Y ,
    UI_CURSOR_RIGHT_POS_Y + 24 ,
    UI_CURSOR_RIGHT_POS_X ,
    UI_CURSOR_RIGHT_POS_X + 24,
  }, 
  //  UI_HIT_TABLE_TRG_RETURN
  { 
    UI_RETURN_POS_Y ,
    UI_RETURN_POS_Y + 24 ,
    UI_RETURN_POS_X ,
    UI_RETURN_POS_X + 24 ,
  }, 
  //  UI_HIT_TABLE_TRG_EXIT
  { UI_EXIT_POS_Y,
    UI_EXIT_POS_Y + 24 ,
    UI_EXIT_POS_X,
    UI_EXIT_POS_X + 24 ,
  },
  // 終了コード
  { GFL_UI_TP_HIT_END , 0 , 0 , 0 },
};

// ========================
// UI用当たり判定テーブル(トリガーボタン)
// ========================
static const GFL_UI_TP_HITTBL sc_ui_hittbl_cont[UI_HIT_TABLE_CONT_MAX] =
{
  // UI_HIT_TABLE_CONT_SCROLL_BUTTON
  { 
    UI_SCROLBTN_POS_Y ,
    UI_SCROLBTN_POS_Y + 24 ,
    UI_SCROLBTN_POS_X ,
    UI_SCROLBTN_POS_X +24, 
  }, 
  // 終了コード
  { GFL_UI_TP_HIT_END , 0 , 0 , 0 },
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メダルボックスメインワーク
//=====================================
typedef struct {

  // システム関連
	HEAPID heapID;
  MEDALBOXSYS_PARAM *p_param;
	GFL_TCB						*p_vblank_task;
	GFL_CLUNIT			  *p_clunit;
	GFL_CLWK				  *p_clwk[MEDALBOXSYS_OBJ_CLWK_MAX];
  u32							  obj_res[OBJRESID_MAX];
  GFL_BMPWIN        *p_name_win;
  GFL_BMPWIN        *p_info_win;

  // メッセージ関連
	GFL_FONT			*p_font;
	GFL_MSGDATA		*p_mdlname_msg;
	GFL_MSGDATA		*p_mdlinfo_msg;
	GFL_MSGDATA		*p_mdlhelp_msg;

  // アプリ関連
  u8                sub_seq;
  MEDALBOXSYS_MODE  box_mode;       // メダルボックスモード
  MEDALBOXSYS_SEQ   now_seq;        // 現在のシーケンス
  MY_MEDALDATA      medal_data;     // メダルデータ
  u16               top_page_num;   // ページユニットの一番左端に表示するページ番号
  fx32              page_offset;    // ページオフセット
  SCROLL_DIR        scroll_dir;     // スクロール方向
  s32               scroll_count;   // スクロール回数
  u32               scroll_speed;   // スクロールスピード
  KEY_MODE          key_mode;       // キーモード
  PAGEDATA          page[ MEDALBOX_DRAW_PAGE_NUM_LIST ];   // 表示しているページデータ
  u8                cursor_pos;         // カーソル位置
  CURSOR_STATE      cursor_state;       // カーソル状態
  CURSOR_STATE      cursor_state_tmp;   // カーソル状態テンポラリ
  
  fx32              scroll_button_pos;  // スクロールバーの位置(初期座標からのオフセット)
#ifdef PM_DEBUG
  // デバッグ
  u8              debgu_seq;
#endif

}MEDALBOXSYS_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static GFL_PROC_RESULT MEDALBOXSYS_ProcInit( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT MEDALBOXSYS_ProcMain( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT MEDALBOXSYS_ProcTerm( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
// システム
//=============================================================================
static void MDLSYS_MakeCommonData( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_DeleteCommonData( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_GraphicInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BGInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BGDelete( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_OBJInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_OBJDelete( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_UI_Init( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_MedalInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BMPWinInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BMPWinDelete( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs );
static void MDLSYS_SetNextSeq( MEDALBOXSYS_WORK *p_wk ,MEDALBOXSYS_SEQ next_seq );

//=============================================================================
// 初期化シーケンス
//=============================================================================
static GFL_PROC_RESULT MDLSYS_INIT_Main( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// メインシーケンスメイン
//=============================================================================
static GFL_PROC_RESULT MDLSYS_MAIN_Main( MEDALBOXSYS_WORK *p_wk );
static void MainKeyStateCheck( MEDALBOXSYS_WORK *p_wk );
static void MakeMedalHitTbl( MEDALBOXSYS_WORK *p_wk ,GFL_UI_TP_HITTBL medal_hittbl[] );
static void GetCursorPos( MEDALBOXSYS_WORK *p_wk , u8 cursor_pos , GFL_CLACTPOS *p_pos );
static void SetCursor( MEDALBOXSYS_WORK *p_wk , u8 cursor_pos );
static void SetCursorState( MEDALBOXSYS_WORK *p_wk , CURSOR_STATE state );

//=============================================================================
// ページ切替シーケンスメイン
//=============================================================================
static GFL_PROC_RESULT MDLSYS_SCROLLPAGE_Main( MEDALBOXSYS_WORK *p_wk );
static BOOL ScrollPage( MEDALBOXSYS_WORK *p_wk , fx32 move_speed );
static void PageRefresh( MEDALBOXSYS_WORK *p_wk );
static void SetScrollDirection( MEDALBOXSYS_WORK *p_wk , SCROLL_DIR dir );
static void DrawMedalName( MEDALBOXSYS_WORK *p_wk , MedalID medal_id , u8 DrawPos );
static void ClearMedalName( MEDALBOXSYS_WORK *p_wk );
static void TransMedalName( MEDALBOXSYS_WORK *p_wk );
static void DrawMedalInfo( MEDALBOXSYS_WORK *p_wk , MY_MEDAL_ST *p_medal_st );
static PRINTSYS_LSB GetMedalNameFontColor( void );
static PRINTSYS_LSB GetMedalInfoFontColor( void );
static u8 GetCursolSelectUnitPos( MEDALBOXSYS_WORK *p_wk );
static u8 GetCursolSelectMedalPos( MEDALBOXSYS_WORK *p_wk );
static void SetScrollCount( MEDALBOXSYS_WORK *p_wk , s32 count );
static void SetScrollSpeed( MEDALBOXSYS_WORK *p_wk , u32 speed );

//=============================================================================
// スクロールボタン
//=============================================================================
static GFL_PROC_RESULT MDLSYS_SCROLLBUTTON_Main( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// 終了ボタンシーケンス
//=============================================================================
static GFL_PROC_RESULT MDLSYS_EXITBUTTON_Main( MEDALBOXSYS_WORK *p_wk );

//=============================================================================
// メダルシステム
//=============================================================================
static void MDLSYS_PageMedalDraw( MEDALBOXSYS_WORK *p_wk , u8 draw_page , u8 unit_pos);
static void MDLSYS_GetDrawMedalPos( int num , GFL_CLACTPOS* p_pos );
static void MDLSYS_GetDrawPushMedalButtonPos( int num , GFL_CLACTPOS* p_pos );

//=============================================================================
// デバッグ関数
//=============================================================================
#ifdef PM_DEBUG
static void DBEG_MDLSYS_DebugMain( MEDALBOXSYS_WORK *p_wk );
static void DBEG_MDLSYS_DebugInit( MEDALBOXSYS_WORK *p_wk );
#endif



// 外部参照用プロセスデータ定義
GFL_PROC_DATA medalBox_ProcData =
{
  MEDALBOXSYS_ProcInit,
  MEDALBOXSYS_ProcMain,
  MEDALBOXSYS_ProcTerm
};


//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステムプロセス初期化処理
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MEDALBOXSYS_ProcInit( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  // ワーク定義
  MEDALBOXSYS_WORK *p_wk;

  // プロセス用ヒープID
  HEAPID heapID = HEAPID_MEDALBOXSYS;

	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, heapID , MEDALBOXSYS_HEAP_SIZE );

	//ワーク作成
	p_wk  = GFL_PROC_AllocWork( p_proc, sizeof(MEDALBOXSYS_WORK), heapID );
	GFL_STD_MemClear( p_wk, sizeof(MEDALBOXSYS_WORK) );

  // パラメータ
	p_wk->p_param	= p_param_adrs;
  p_wk->heapID  = heapID;

  // 共通データ作成
  MDLSYS_MakeCommonData( p_wk );

  // グラフィック初期化
  MDLSYS_GraphicInit( p_wk );

  // UIの初期化
  MDLSYS_UI_Init( p_wk );
  
  // ビットマップウィンドウの初期化
  MDLSYS_BMPWinInit( p_wk );

  // メダルの初期化
  MDLSYS_MedalInit( p_wk );

#ifdef PM_DEBUG
  DBEG_MDLSYS_DebugInit( p_wk );
#endif
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステムメイン
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MEDALBOXSYS_ProcMain( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  MEDALBOXSYS_WORK *p_wk = p_wk_adrs;
  GFL_PROC_RESULT  proc_ret = GFL_PROC_RES_FINISH;

#ifdef PM_DEBUG
  const int trg = GFL_UI_KEY_GetTrg();
  
  // デバッグ関数
  DBEG_MDLSYS_DebugMain( p_wk );
  
  if( trg & PAD_BUTTON_START )
  {
    return GFL_PROC_RES_FINISH;
  }

#endif

  switch( p_wk->now_seq )
  {
  // 起動演出シーケンス
  case SEQ_MDLBOX_INIT:
    proc_ret = MDLSYS_INIT_Main( p_wk );
    break;

  // メインシーケンス
  case SEQ_MDLBOX_MAIN:
    proc_ret = MDLSYS_MAIN_Main( p_wk );
    break;

  // ページスクロール
  case SEQ_MDLBOX_SCROLLPAGE:
    proc_ret = MDLSYS_SCROLLPAGE_Main( p_wk );
    break;

  // スクロールボタン
  case SEQ_MDLBOX_SCROLL_BUTTON:
    proc_ret = MDLSYS_SCROLLBUTTON_Main( p_wk );
    break;

  // 終了シーケンス
  case SEQ_MDLBOX_EXIT:
    proc_ret = MDLSYS_EXITBUTTON_Main( p_wk );
    break;


  default:
    {
      GF_ASSERT( 0 );
    }
    proc_ret = GFL_PROC_RES_FINISH;
    break;
  }
  
  // セルの更新
	GFL_CLACT_SYS_Main();
  
  return proc_ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルボックスシステム終了処理
 *
 *	@param	* p_proc
 *	@param	* seq 
 *	@param	*p_param_adrs
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MEDALBOXSYS_ProcTerm( GFL_PROC * p_proc, int * seq , void *p_param_adrs, void *p_wk_adrs )
{
  MEDALBOXSYS_WORK *p_wk = p_wk_adrs;

  // VBLANKTask消去
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

  // BMPWinの開放
  MDLSYS_BMPWinDelete( p_wk );
  
  // OBJデータ開放
  MDLSYS_OBJDelete( p_wk );

  // BGデータ開放
  MDLSYS_BGDelete( p_wk );

  // 共通データ開放
  MDLSYS_DeleteCommonData( p_wk );

  // ワーク開放
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_MEDALBOXSYS );

	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルシステム共通データ作成
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_MakeCommonData( MEDALBOXSYS_WORK *p_wk )
{
  HEAPID heapID = p_wk->heapID;

	// フォント
	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

	// メダル名メッセージデータ
	p_wk->p_mdlname_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalname_dat, heapID );

	// メダル説明文メッセージデータ
	p_wk->p_mdlinfo_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalexplanation_dat, heapID );

	// メダルヒントメッセージデータ
	p_wk->p_mdlhelp_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_medalhelp_dat, heapID );

  // メダルデータの生成
  MEDALDATASYS_InitMedalData( &p_wk->medal_data , p_wk->p_param->p_gamedata , p_wk->heapID );


}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルシステム共通データ開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_DeleteCommonData( MEDALBOXSYS_WORK *p_wk )
{
  // メダルデータ
  MEDALDATASYS_DeleteMedalData( &p_wk->medal_data );

  // メッセージ
	GFL_MSG_Delete( p_wk->p_mdlhelp_msg );
	GFL_MSG_Delete( p_wk->p_mdlinfo_msg );
	GFL_MSG_Delete( p_wk->p_mdlname_msg );

  // フォント
	GFL_FONT_Delete( p_wk->p_font );
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィック初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_GraphicInit( MEDALBOXSYS_WORK *p_wk )
{
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
  GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
  GFL_DISP_SetDispOn();

  //	表示
  GFL_DISP_GX_InitVisibleControl();

  //	フォント初期化
  GFL_FONTSYS_Init();

  // BGの初期化
  MDLSYS_BGInit( p_wk );

  // OBJの初期化
  MDLSYS_OBJInit( p_wk );

  // 描画用Vブランクタスクを登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB( MDLSYS_Graphic_VBlankTask , p_wk , 0 );
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_BGInit( MEDALBOXSYS_WORK *p_wk )
{
  HEAPID heapID = p_wk->heapID;

	//初期化
	GFL_BG_Init( heapID );

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
		for( i = 0; i < MEDALBOXSYS_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_SetBGControl( sc_bgsetup[i].frame, &sc_bgsetup[i].bgcnt_header, sc_bgsetup[i].mode );
			GFL_BG_SetVisible( sc_bgsetup[i].frame, VISIBLE_ON );
    }
  }

	//読み込み
	{	
		ARCHANDLE	*p_handle;
		p_handle	= GFL_ARC_OpenDataHandle( ARCID_MEDALGRA , heapID );

    // ====================================
		// 下画面パレット(メイン)
    // ====================================
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_medalbox_gra_medal_bgd_NCLR,
				PALTYPE_MAIN_BG, 0, 0 , heapID );

    // キャラクターデータ（下画面共通)
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_medalbox_gra_medal_bgd_NCGR,
				GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M), 0, 0, FALSE, heapID );

    // ページユニット
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgd1_NSCR,
				GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_PAGEUNIT_M), 0, 0, FALSE, heapID );

    // 文字盤
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgd2_NSCR,
				GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M), 0, 0, FALSE, heapID );
    // 使用パレットの書き換え
    //GFL_BG_ChangeScreenPalette( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M), 0 , 0 , 32 , 32 , p_wk->medal_data.boxrank + 1 ); 
    //GFL_BG_LoadScreenReq( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) );

    // 光沢
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgd3_NSCR,
				GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), 0, 0, FALSE, heapID );

    // ====================================
		// 上画面(サブ)
    // ====================================
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_medalbox_gra_medal_bgu_NCLR,
				PALTYPE_SUB_BG, 0, 0 , heapID );

    // キャラクターデータ（上画面共通)
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_medalbox_gra_medal_bgu_NCGR,
				GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_MONITOR_S), 0, 0, FALSE, heapID );

    // モニター
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgu2_NSCR,
				GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_MONITOR_S), 0, 0, FALSE, heapID );

    // モニター演出
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgu3_NSCR,
				GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_EFFECTS_S), 0, 0, FALSE, heapID );

    // 背景
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_medalbox_gra_medal_bgu1_NSCR,
				GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_BACK_S), 0, 0, FALSE, heapID );

		GFL_ARC_CloseDataHandle( p_handle );

	}

  // ブレンド設定
  {
    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1 ,
        (GX_BLEND_PLANEMASK_BG0 |GX_BLEND_PLANEMASK_BG3) ,
        15, 15 );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  BGリソースの削除
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_BGDelete( MEDALBOXSYS_WORK *p_wk )
{	
	//破棄
	{	
		//GFL_BG_FillCharacterRelease( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_ETC_M), 1,0 );
	}
	
	//BG面破棄
	{	
		int i;
		for( i = 0; i < MEDALBOXSYS_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_ClearFrame( sc_bgsetup[i].frame );
			GFL_BG_FreeBGControl( sc_bgsetup[i].frame );
		}
	}

	//終了
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();

}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_OBJInit( MEDALBOXSYS_WORK *p_wk )
{
  
  HEAPID heapID = p_wk->heapID;

	//システム作成
  {
    const GFL_CLSYS_INIT sc_clsys_init = {
      0, 0,
      0, 512,
      4, 124,
      4, 124,
      0,
      32,32,32,32,
      16, 16,
    };
    GFL_CLACT_SYS_Create( &sc_clsys_init, &sc_vramSetTable , heapID );
  }

	// ユニット
	{	
		p_wk->p_clunit	= GFL_CLACT_UNIT_Create( 128, 1, heapID );
		GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit );
	}

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );


	//リソース読み込み
	{	
    int i;
		ARCHANDLE	*	p_handle[ARC_TYPE_MAX];
    u32* obj_res = p_wk->obj_res;
   
    // =======================================
    // リソースの読み込み
    // =======================================
    p_handle[ARC_TYPE_MEDAL_GRA] = GFL_ARC_OpenDataHandle( ARCID_MEDALGRA , heapID );
    p_handle[ARC_TYPE_COMMON_BUTTON_GRA] = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON , heapID );

    // ======================================
    // メダルOBJ
    // ======================================
    obj_res[ OBJRESID_MEDAL_CELLANM ] = GFL_CLGRP_CELLANIM_Register( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj1_NCER, NARC_medalbox_gra_medal_obj1_NANR , heapID );

    obj_res[ OBJRESID_MEDAL_PLTT ]    = GFL_CLGRP_PLTT_RegisterEx( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj1_NCLR , CLSYS_DRAW_MAX ,  0 , 0 , MEDALOBJ_PALETTENUM_MEDAL , heapID );
  
    obj_res[ OBJRESID_MEDAL_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj1_NCGR , FALSE, CLSYS_DRAW_MAX, heapID );

    // ======================================
    // UIOBJ
    // ======================================
    obj_res[ OBJRESID_UI_CELLANM ] = GFL_CLGRP_CELLANIM_Register( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj2_NCER, NARC_medalbox_gra_medal_obj2_NANR , heapID );

    obj_res[ OBJRESID_UI_PLTT ]    = GFL_CLGRP_PLTT_RegisterEx( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj2_NCLR ,
        CLSYS_DRAW_MAIN ,  32*MEDALOBJ_PALETTENUM_MEDAL , 0 , MEDALOBJ_PALETTENUM_UI , heapID );	
  
    obj_res[ OBJRESID_UI_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_MEDAL_GRA],
        NARC_medalbox_gra_medal_obj2_NCGR , FALSE, CLSYS_DRAW_MAIN, heapID );

    // ======================================
    // COMMONUIOBJ
    // ======================================
    obj_res[ OBJRESID_COMMONUI_CELLANM ] = GFL_CLGRP_CELLANIM_Register( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_bar_button_128k_NCER, NARC_app_menu_common_bar_button_128k_NANR , heapID );

    obj_res[ OBJRESID_COMMONUI_PLTT ]    = GFL_CLGRP_PLTT_RegisterEx( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_bar_button_NCLR ,
        CLSYS_DRAW_MAIN ,  32 * (MEDALOBJ_PALETTENUM_UI+MEDALOBJ_PALETTENUM_MEDAL), 0 ,
        MEDALOBJ_PALETTENUM_COMMON_UI , heapID );	
  
    obj_res[ OBJRESID_COMMONUI_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_bar_button_128k_NCGR , FALSE, CLSYS_DRAW_MAIN, heapID );

    // ======================================
    // COMMONUIOBJ(検索ボタン)
    // ================================== 
    obj_res[ OBJRESID_COMMONUI_SEARCH_CGR ]     = GFL_CLGRP_CGR_Register( p_handle[ARC_TYPE_COMMON_BUTTON_GRA],
        NARC_app_menu_common_search_button_128k_NCGR , FALSE, CLSYS_DRAW_MAIN, heapID );
    
    // ======================================
    // アーカイブハンドルの開放
    // ======================================
    for( i = 0 ; i < ARC_TYPE_MAX ; i++ )
    {
      GFL_ARC_CloseDataHandle( p_handle[i] );
    }
  }

  // ======================================
  //CLWK作成
  // ======================================
  {
		int i;
    GFL_CLACTPOS pos;
    GFL_CLWK_DATA cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    // ======================================
    // 下画面
    // ======================================
    // メダルCLWK作成
    {
      for( i = 0 ; i < MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX ; i++)
      {
        // 位置データの書き込み
        MDLSYS_GetDrawMedalPos( i , &pos );
        cldata.pos_x = pos.x;
        cldata.pos_y = pos.y;

        // プライオリティー
        cldata.bgpri = MEDALBOXSYS_OBJ_PRIORITY_MEDAL;

        // ワークの作成
        p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
            p_wk->obj_res[OBJRESID_MEDAL_CGR],
            p_wk->obj_res[OBJRESID_MEDAL_PLTT],
            p_wk->obj_res[OBJRESID_MEDAL_CELLANM],
            &cldata, CLSYS_DEFREND_MAIN, heapID );
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i] , 0 );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      }
    }

    // UIの作成
    for( i = MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX; i < MEDALBOXSYS_OBJ_UI_CLWK_MAX; i++ )
    {	
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.bgpri   = MEDALBOXSYS_OBJ_PRIORITY_UI;
      cldata.softpri = MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM;

      // ワークの作成
      p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_UI_CGR],
          p_wk->obj_res[OBJRESID_UI_PLTT],
          p_wk->obj_res[OBJRESID_UI_CELLANM],
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i] , 0 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
    }

    // COMMONUIの作成
    for( i = MEDALBOXSYS_OBJ_UI_CLWK_MAX; i < MEDALBOXSYS_OBJ_COMMON_CLWK_MAX; i++ )
    {	
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.bgpri   = MEDALBOXSYS_OBJ_PRIORITY_UI;
      cldata.softpri = MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM;

      // ワークの作成
      p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_COMMONUI_CGR],
          p_wk->obj_res[OBJRESID_COMMONUI_PLTT],
          p_wk->obj_res[OBJRESID_COMMONUI_CELLANM],
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i] , 0 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
    }

    // 検索ボタンの作成
    {
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.bgpri   = MEDALBOXSYS_OBJ_PRIORITY_UI;
      cldata.softpri = MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM;

      // ワークの作成
      p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_COMMONUI_SEARCH_CGR],
          p_wk->obj_res[OBJRESID_COMMONUI_PLTT],
          p_wk->obj_res[OBJRESID_COMMONUI_CELLANM],
          &cldata, CLSYS_DEFREND_MAIN, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH] , 0 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH], TRUE );
    }
    
    // ======================================
    // 上画面
    // ======================================
    // メダルCLWK作成
    {
      // 位置データの書き込み
      cldata.pos_x = UP_MEDAL_POS_X;
      cldata.pos_y = UP_MEDAL_POS_Y;

      // ワークの作成
      p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
          p_wk->obj_res[OBJRESID_MEDAL_CGR],
          p_wk->obj_res[OBJRESID_MEDAL_PLTT],
          p_wk->obj_res[OBJRESID_MEDAL_CELLANM],
          &cldata, CLSYS_DEFREND_SUB, heapID );
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL] , 0 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL], FALSE );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  OBJデータの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_OBJDelete( MEDALBOXSYS_WORK *p_wk )
{
  int i;

	//CLWK取得
	{
		for( i = 0; i < MEDALBOXSYS_OBJ_CLWK_MAX; i++ )
		{	
			if( p_wk->p_clwk[i] )
			{	
				GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
			}
		}
	}

  //リソース破棄
  {	
    // メダルOBJ
    {
      GFL_CLGRP_CELLANIM_Release( p_wk->obj_res[OBJRESID_MEDAL_CELLANM] );
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_MEDAL_CGR] );
      GFL_CLGRP_PLTT_Release( p_wk->obj_res[OBJRESID_MEDAL_PLTT] );	
    }
    // UIOBJ
    {
      GFL_CLGRP_CELLANIM_Release( p_wk->obj_res[OBJRESID_UI_CELLANM] );
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_UI_CGR] );
      GFL_CLGRP_PLTT_Release( p_wk->obj_res[OBJRESID_UI_PLTT] );	
    }
    // COMMONUIOBJ
    {
      GFL_CLGRP_CELLANIM_Release( p_wk->obj_res[OBJRESID_COMMONUI_CELLANM] );
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_COMMONUI_CGR] );
      GFL_CLGRP_PLTT_Release( p_wk->obj_res[OBJRESID_COMMONUI_PLTT] );	
    }
    // 検索ボタン
    {
      GFL_CLGRP_CGR_Release( p_wk->obj_res[OBJRESID_COMMONUI_SEARCH_CGR] );
    }
  }


	//システム破棄
	{
    if( p_wk->p_clunit )
    {	
      GFL_CLACT_UNIT_Delete( p_wk->p_clunit );
    }
  }

	GFL_CLACT_SYS_Delete();
}

//----------------------------------------------------------------------------
/**
 *	@brief  UIの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_UI_Init( MEDALBOXSYS_WORK *p_wk )
{
  GFL_CLACTPOS pos;
  GFL_CLWK **p_clwk = p_wk->p_clwk;

  // スクロールボタン
  pos.x = UI_SCROLBTN_POS_X;
  pos.y = UI_SCROLBTN_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBTN] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBTN] , 0 );
  
  // スクロールボタン
  pos.x = UI_SCROLBER_POS_X;
  pos.y = UI_SCROLBER_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBER] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBER] , 1 );

  // カーソル
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , 2 );
  SetCursorState( p_wk , CURSOR_STATE_NOT_SELECT );

  // イチオシ
  MDLSYS_GetDrawPushMedalButtonPos( 0 , &pos);
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI] , 3 );
  
  // キラキラ
  pos.x = 0;
  pos.y = 0;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_KIRAKIRA] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_KIRAKIRA] , 4 );
   
  // フタ
  pos.x = UI_HUTA_POS_X;
  pos.y = UI_HUTA_POS_Y;
  GFL_CLACT_WK_SetSoftPri( p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA] , MEDALBOXSYS_OBJ_SOFT_PRIORITY_TOP );
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA] , 5 );

   
  // 検索
  pos.x = UI_SEARCH_POS_X;
  pos.y = UI_SEARCH_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_SEARCH] , 0 );
   
  // キャンセル
  pos.x = UI_EXIT_POS_X;
  pos.y = UI_EXIT_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_EXIT] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_EXIT] , APP_COMMON_BARICON_EXIT );
   
  // 戻る
  pos.x = UI_RETURN_POS_X;
  pos.y = UI_RETURN_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_RETURN] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_RETURN] , APP_COMMON_BARICON_RETURN );

  // 右カーソル
  pos.x = UI_CURSOR_RIGHT_POS_X;
  pos.y = UI_CURSOR_RIGHT_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT] , APP_COMMON_BARICON_CURSOR_RIGHT );

  // 左カーソル
  pos.x = UI_CURSOR_LEFT_POS_X;
  pos.y = UI_CURSOR_LEFT_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT] , &pos , CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetAnmSeq( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT] , APP_COMMON_BARICON_CURSOR_LEFT );

}

//----------------------------------------------------------------------------
/**
 *	@brief  ビットマップウィンドウの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_BMPWinInit( MEDALBOXSYS_WORK *p_wk )
{
  int i;
	
  // BMPWINの初期化
  GFL_BMPWIN_Init( p_wk->heapID );
  
  // 名前用BMPWIN作成
  p_wk->p_name_win = GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M) ,
      0   , 0   ,
      32  , 24  ,
      MEDAL_NAME_BMPWIN_PAL_NUM , GFL_BMP_CHRAREA_GET_B );

  GF_ASSERT( p_wk->p_name_win );
  GFL_BMPWIN_MakeScreen( p_wk->p_name_win );

  // 説明文表示用BMPWIN
  p_wk->p_info_win = GFL_BMPWIN_Create( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_S) ,
      0   , 0   ,
      32  , 24  ,
      MEDAL_INFO_BMPWIN_PAL_NUM , GFL_BMP_CHRAREA_GET_B );
  GFL_BMPWIN_MakeScreen( p_wk->p_info_win );

  // 転送リクエスト
  GFL_BG_LoadScreenV_Req(GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M));
  GFL_BG_LoadScreenV_Req(GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_S));
 
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビットマップウィンドウの開放
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_BMPWinDelete( MEDALBOXSYS_WORK *p_wk )
{
  int i;

  // 名前用BMPWIN
  GFL_BMPWIN_Delete( p_wk->p_name_win );

  // 情報ウィンドウ
  GFL_BMPWIN_Delete( p_wk->p_info_win );
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックVブランクタスク
 *
 *	@param	*p_tcb
 *	@param	*p_wk_adrs 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs )
{
	GFL_CLACT_SYS_VBlankFunc();
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルの初期化
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_MedalInit( MEDALBOXSYS_WORK *p_wk )
{
  // ページのリフレッシュ
  PageRefresh( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ページのメダルを描画する
 *
 *	@param	*p_wk 
 *	@param	draw_page  表示するページ番号 
 *	@param	unit_pos  ページユニットの位置（0~4）
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_PageMedalDraw( MEDALBOXSYS_WORK *p_wk , u8 draw_page , u8 unit_pos)
{
  int i;
  PAGEDATA *p_page = &p_wk->page[ unit_pos ];
  MY_MEDALDATA *p_medal_data = &p_wk->medal_data;

  GF_ASSERT( unit_pos < MEDALBOX_DRAW_PAGE_NUM_LIST );

  MEDALDATASYS_GetPageMedal( p_medal_data , p_page , draw_page );

  for( i = 0; i < PAGE_MEDAL_MAX ; i ++ )
  {
    u8 draw_pos = unit_pos * PAGE_MEDAL_MAX + i;

    if( i < p_page->medal_max )
    {
      // ヒントメダルの場合は最後のシーケンスを設定する
      switch( p_page->p_medal_st[i]->medal_state )
      {
      case MEDAL_STATE_TITLE_GET:
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[draw_pos] , p_page->p_medal_st[i]->graphic_type );
        break;
      case MEDAL_STATE_HINT_GET:
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[draw_pos] , GFL_CLACT_WK_GetAnmSeqNum(p_wk->p_clwk[draw_pos]) -1 );
        break;
      default:
        GF_ASSERT(0);
        break;
      }

      // メダル名の描画
      if( unit_pos == MEDAL_NAME_DRAW_PAGE )
      {
        if( p_page->p_medal_st[i]->medal_state == MEDAL_STATE_TITLE_GET )
        {
          DrawMedalName( p_wk , p_page->p_medal_st[i]->medal_id , draw_pos % PAGE_MEDAL_MAX ) ;
        }
        else
        {
          DrawMedalName( p_wk , MDL_NAME_HINT_000 , draw_pos % PAGE_MEDAL_MAX ) ;
        }
      }

      // メダル画像を表示する
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[draw_pos], TRUE );
    }
    else
    {
      // 表示しない
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[draw_pos], FALSE );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダルの表示先取得
 *
 *	@param	num   メダル穴番号
 *	@param	p_pos データ取得先
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_GetDrawMedalPos( int num , GFL_CLACTPOS* p_pos )
{
  p_pos->x = sc_medal_pos[num%PAGE_MEDAL_MAX].x + ( num/PAGE_MEDAL_MAX - 1 )*PAGE_WIDTH;
  p_pos->y = sc_medal_pos[num%PAGE_MEDAL_MAX].y;
}

//----------------------------------------------------------------------------
/**
 *	@brief  イチオシボタンの描画位置を取得
 *
 *	@param	num 
 *	@param	p_pos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_GetDrawPushMedalButtonPos( int num , GFL_CLACTPOS* p_pos )
{
  p_pos->x = sc_ichioshi_pos[num].x;
  p_pos->y = sc_ichioshi_pos[num].y;
}

//----------------------------------------------------------------------------
/**
 *	@brief  シーケンスの切替
 *
 *	@param	*p_wk 
 *	@param	next_seq 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_SetNextSeq( MEDALBOXSYS_WORK *p_wk ,MEDALBOXSYS_SEQ next_seq )
{
  // シーケンスを切り替える
  p_wk->now_seq = next_seq;
  p_wk->sub_seq = 0;
}


// ============================================================
// 
// シーケンスメイン関数郡
//
// ============================================================

//----------------------------------------------------------------------------
/**
 *	@brief  ボックス初期化シーケンス
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_INIT_Main( MEDALBOXSYS_WORK *p_wk )
{
  switch( p_wk->sub_seq )
  {
  case 0:
    {
      // ブラックアウトの開始
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      if( !GFL_FADE_CheckFade() )
      {
        PMSND_PlaySE( SEQ_SE_MD_04 );
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
      }
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メインシーケンス(SEQ_MDLBOX_MAIN)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_MAIN_Main( MEDALBOXSYS_WORK *p_wk )
{
  // メインキー入力処理
  MainKeyStateCheck( p_wk );

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メインシーケンスキー入力
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MainKeyStateCheck( MEDALBOXSYS_WORK *p_wk )
{
  const int trg = GFL_UI_KEY_GetTrg();
  const int rep = GFL_UI_KEY_GetRepeat();
  const int cont = GFL_UI_KEY_GetCont();
  int tp_hit;

  // ==========================
  // メダルタッチでのカーソル移動
  // ==========================
  {
    GFL_UI_TP_HITTBL medal_hittbl[MEDAL_HIT_TABLE_MAX];

    // メダル辺り判定テーブル作成
    MakeMedalHitTbl( p_wk , medal_hittbl );

    // ヒットデータの取得
    tp_hit = GFL_UI_TP_HitTrg( medal_hittbl );

    // 当たっていない時と、テーブルエンドのときは判定をしない
    if( (tp_hit != GFL_UI_TP_HIT_NONE) )
    {
      p_wk->key_mode = KEYMODE_TP;
      SetCursor( p_wk , tp_hit );
      return;
    }
  }

  // ==========================
  // タッチでのUI操作(トリガー)
  // ==========================
  {
    tp_hit = GFL_UI_TP_HitTrg( sc_ui_hittbl_trg );
    switch( tp_hit )
    {
    case UI_HIT_TABLE_TRG_SEARCH:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_SEARCH\n");
      }
      return;
    case UI_HIT_TABLE_TRG_CURSOR_LEFT:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_CURSOR_LEFT\n");
        SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
        SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
        SetScrollDirection( p_wk , SCROLL_DIR_LEFT );
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
        p_wk->key_mode = KEYMODE_TP;
      }
      return;
    case UI_HIT_TABLE_TRG_CURSOR_RIGHT:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_CURSOR_RIGHT\n");
        SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
        SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
        SetScrollDirection( p_wk , SCROLL_DIR_RIGHT );
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
        p_wk->key_mode = KEYMODE_TP;
      }
      return;
    case UI_HIT_TABLE_TRG_RETURN:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_RETURN\n");
      }
      return;
    case UI_HIT_TABLE_TRG_EXIT:
      {
        ONOUE_Printf("UI_HIT_TABLE_TRG_EXIT\n");
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_EXIT );
        p_wk->key_mode = KEYMODE_TP;
      }
      return;
    }
  }
  
  // ==========================
  // タッチでのUI操作(トリガー)
  // ==========================
  {
    tp_hit = GFL_UI_TP_HitCont( sc_ui_hittbl_cont );

    switch( tp_hit )
    {
    case UI_HIT_TABLE_CONT_SCROLL_BUTTON:
      {
        ONOUE_Printf("UI_HIT_TABLE_CONT_SCROLL_BUTTON\n");
        MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLL_BUTTON );
        p_wk->key_mode = KEYMODE_TP;
      }
      return;
    }
  }


  // ==========================
  // キーでのカーソル移動
  // ==========================
  {
    u8 cursor_tmp = p_wk->cursor_pos;
    u8 unit_pos   = GetCursolSelectUnitPos( p_wk ); // カーソルがどのユニット位置にいるか
    u8 top_pos    = (cursor_tmp/PAGE_MEDAL_MAX) * PAGE_MEDAL_MAX;      // そのユニット内で一番上のcursorposを取得
    u8 curpos_in_page = GetCursolSelectMedalPos( p_wk );

    // カーソル位置をコピー
    cursor_tmp = p_wk->cursor_pos;

    // カーソルが選択されていない状態
    if( p_wk->cursor_state == CURSOR_STATE_NOT_SELECT )
    {
      if( (trg & PAD_KEY_UP) || (trg & PAD_KEY_DOWN) )
      {
        SetCursor( p_wk , 0 );
        return;
      }
    }
    else
    {

      // 上へ移動
      if( trg & PAD_KEY_UP )
      {
        // 一番上を選択しているかを判定
        if( curpos_in_page == 0 )
        {
          cursor_tmp = top_pos + p_wk->page[unit_pos].medal_max-1;
        }
        else
        {
          cursor_tmp--;
        }
        SetCursor( p_wk , cursor_tmp );
        p_wk->key_mode = KEYMODE_KEY;
        return;
      }

      // 下へ移動
      if( trg & PAD_KEY_DOWN )
      {
        // 一番下の場合は一番上に
        if( cursor_tmp == (PAGE_MEDAL_MAX-1) )
        {
          cursor_tmp = top_pos;
        }
        // メダルが存在しない場合は一番上に
        else if( p_wk->page[unit_pos].p_medal_st[cursor_tmp+1] == NULL )
        {
          cursor_tmp = top_pos;
        }
        else
        {
          cursor_tmp++;
        }
        SetCursor( p_wk , cursor_tmp );
        p_wk->key_mode = KEYMODE_KEY;
        return;
      }
    }
  }
 
  // ページ送り
  if( rep & PAD_KEY_RIGHT )
  {
    SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
    SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
    SetScrollDirection( p_wk , SCROLL_DIR_RIGHT );
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
    p_wk->key_mode = KEYMODE_KEY;
    return;
  }
  
  // ページ送り
  if( rep & PAD_KEY_LEFT )
  {
    SetScrollSpeed( p_wk , DIR_KEY_SCROLL_SPEED );
    SetScrollCount( p_wk , DIR_KEY_SCROLL_COUNT );
    SetScrollDirection( p_wk , SCROLL_DIR_LEFT );
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
    p_wk->key_mode = KEYMODE_KEY;
    return;
  }
   
  // 3ページ送り
  if( rep & PAD_BUTTON_R )
  {
    SetScrollSpeed( p_wk , LR_KEY_SCROLL_SPEED );
    SetScrollCount( p_wk , LR_KEY_SCROLL_COUNT );
    SetScrollDirection( p_wk , SCROLL_DIR_RIGHT );
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
    p_wk->key_mode = KEYMODE_KEY;
    return;
  }

  // 3ページ送り
  if( rep & PAD_BUTTON_L )
  {
    SetScrollSpeed( p_wk , LR_KEY_SCROLL_SPEED );
    SetScrollCount( p_wk , LR_KEY_SCROLL_COUNT );
    SetScrollDirection( p_wk , SCROLL_DIR_LEFT );
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_SCROLLPAGE );
    p_wk->key_mode = KEYMODE_KEY;
    return;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ページスクロールシーケンス（SEQ_MDLBOX_SCROLLPAGE）
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_SCROLLPAGE_Main( MEDALBOXSYS_WORK *p_wk )
{
  BOOL  scroll_end = FALSE;
  u8    sub_seq = p_wk->sub_seq;

  switch( sub_seq )
  {
  case 0:
    {
      // メダル名を非表示に
      ClearMedalName( p_wk );

      p_wk->cursor_state_tmp = p_wk->cursor_state;
      SetCursorState( p_wk , CURSOR_STATE_SCROLL );

      // 左右カーソルのアニメーション
      if( p_wk->scroll_dir == SCROLL_DIR_RIGHT )
      {
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT] , APP_COMMON_BARICON_CURSOR_RIGHT_ON );
        GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_RIGHT] , TRUE);
      }
      else
      {
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT] , APP_COMMON_BARICON_CURSOR_LEFT_ON );
        GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR_LEFT] , TRUE);
      }

      // スライド音
      PMSND_PlaySE( SEQ_SE_MD_02 );

      // サブシーケンスを進める
      p_wk->sub_seq++;
    }
    // 同一シーケンスで行うのでスルー
  case 1:
    {
      // ページのスクロール
      scroll_end = ScrollPage( p_wk , p_wk->scroll_speed * p_wk->scroll_dir );

      if( scroll_end )
      {
        // カウンタデクリメント
        p_wk->scroll_count--;

        // メインシーケンスへの移動
       if( p_wk->scroll_count <= 0 )
       {
         u8            unit_pos;
         u8            medal_pos;

         // ユニット上の位置
         unit_pos = GetCursolSelectUnitPos( p_wk );
         medal_pos = GetCursolSelectMedalPos( p_wk );

         // カーソル位置にメダルが存在する
         if( p_wk->page[unit_pos].p_medal_st[medal_pos] )
         {
           // カーソルをセットしなおす
           SetCursor( p_wk , p_wk->cursor_pos );
         }
         else
         {
           // メダルが存在しない場合
           if( p_wk->page[unit_pos].medal_max == 0 )
           {
             SetCursorState( p_wk , CURSOR_STATE_NOT_SELECT );
           }
           // 存在する中でも一番下に移動
           else
           {
             SetCursor( p_wk , p_wk->page[unit_pos].medal_max-1 );
           }
         }

         // スクロールカウントを初期化
         p_wk->scroll_count = 1;

         // メダル名の転送
         TransMedalName( p_wk );

         // メインシーケンスに返す
         MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
       }
      }
      else
      {
        PMSND_PlaySE( SEQ_SE_MD_02 ); // ページ送り音
      }
    }
    break;
  }
  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロール方向の設定
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetScrollDirection( MEDALBOXSYS_WORK *p_wk , SCROLL_DIR dir )
{
  p_wk->scroll_dir = dir;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ページのスクロール
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL ScrollPage( MEDALBOXSYS_WORK *p_wk , fx32 move_speed )
{
  int i;
  BOOL scroll_end = FALSE;
  GFL_CLACTPOS pos;
  s32 offset;

  // ページのスクロール
  p_wk->page_offset += move_speed;
  offset = FX_Whole(p_wk->page_offset);

  // ページ位置のループ
  if( MATH_ABS( offset ) >= PAGE_WIDTH )
  {
    // スクロール方向にページを進める
    if( (p_wk->top_page_num == 0) && (p_wk->scroll_dir < 0))
    {
      p_wk->top_page_num = p_wk->medal_data.page_max-1;
    }
    else
    {
      p_wk->top_page_num += p_wk->scroll_dir;
      p_wk->top_page_num %= p_wk->medal_data.page_max;
    }

    // オフセットの初期化
    p_wk->page_offset = 0;
    offset = 0;

    // ページのリフレッシュ
    PageRefresh( p_wk );

    scroll_end = TRUE;
  }

  // 位置データの書き込み
  for( i = 0 ; i < MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX ; i++ )
  {
    // 移動後の位置を計算
    MDLSYS_GetDrawMedalPos( i , &pos );
    pos.x -= offset;

    // メダルユニットはページ1枚左にはみ出している
    GFL_BG_SetScrollReq( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_PAGEUNIT_M) ,GFL_BG_SCROLL_X_SET , offset + PAGE_WIDTH );
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[i] , &pos , CLSYS_DEFREND_MAIN );
  }

  return scroll_end;
}

//----------------------------------------------------------------------------
/**
 *	@brief  top_page_numに合わせてページ状態を更新する
 *
 *	@param	*p_wk 
 *	@param	refresh_num リフレッシュするページ数
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void PageRefresh( MEDALBOXSYS_WORK *p_wk )
{
  int i;
  u8 refresh_num;

  // メダルを全表示
  if( p_wk->box_mode == BOXMODE_NORMAL )
  {
    refresh_num = MEDALBOX_DRAW_PAGE_NUM_NORMAL;
  }
  else if( p_wk->box_mode == BOXMODE_LIST )
  {
    refresh_num = MEDALBOX_DRAW_PAGE_NUM_LIST;
  }
 
  // メダル名を非表示に
  ClearMedalName( p_wk );
  
  for( i =0 ; i < refresh_num ; i++ )
  {
    int draw_page = p_wk->top_page_num + i -1;
    // 最初のページにループ
    if( draw_page < 0)
    {
      MDLSYS_PageMedalDraw( p_wk , p_wk->medal_data.page_max-1 , i );
    }
    // ページの最大数を超えている場合は最初のページへ
    else if( draw_page >= p_wk->medal_data.page_max )
    {
      MDLSYS_PageMedalDraw( p_wk , 0 , i );
    }
    else
    {
      // 左端は-1ページ前を表示
      MDLSYS_PageMedalDraw( p_wk , p_wk->top_page_num + i -1 , i );
    }
  }
 
  // メダル名の転送
  TransMedalName( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル名表示
 *
 *	@param	*p_wk     
 *	@param	medal_id  描画するメダル名
 *	@param	DrawPos   描画位置（0~4)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawMedalName( MEDALBOXSYS_WORK *p_wk , MedalID medal_id , u8 DrawPos )
{
  STRBUF * str;
  PRINTSYS_LSB font_colro = GetMedalNameFontColor();

  // ノーマルモードのときのみ描画を行う
  if( p_wk->box_mode != BOXMODE_NORMAL )
  {
    return;
  }

  // メダル名の取得
  str = GFL_MSG_CreateString( p_wk->p_mdlname_msg , medal_id );

  // 文字列を直接描画
  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(p_wk->p_name_win) ,
      sc_medalname_win_pos[DrawPos].x , sc_medalname_win_pos[DrawPos].y ,
      str , p_wk->p_font ,
      font_colro );

  // バッファーの消去 
  GFL_STR_DeleteBuffer( str );

}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル名の転送
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void TransMedalName( MEDALBOXSYS_WORK *p_wk )
{
  // 転送
  GFL_BMPWIN_TransVramCharacter( p_wk->p_name_win );
  
  // 転送リクエスト
  GFL_BG_LoadScreenV_Req(GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M));
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル情報表示
 *
 *	@param	*p_wk 
 *	@param	*p_medal_st メダルデータ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void DrawMedalInfo( MEDALBOXSYS_WORK *p_wk , MY_MEDAL_ST *p_medal_st )
{
  STRBUF * str;
  PRINTSYS_LSB font_colro = GetMedalInfoFontColor();

  GF_ASSERT( p_wk );
  GF_ASSERT( p_medal_st );

  // =======================================
  // メダル画像変更
  // =======================================
  {
    GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL] , p_medal_st->graphic_type );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_UPMEDAL], TRUE );
  }

  // =======================================
  // メダルの説明文表示
  // =======================================
  {
    // メダルの説明文取得
    str = GFL_MSG_CreateString( p_wk->p_mdlinfo_msg , p_medal_st->medal_id );

    // ウィンドウの塗りつぶし
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_info_win) , PRINTSYS_LSB_GetB(font_colro) );

    // 文字列を直接描画
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp(p_wk->p_info_win) ,
        MEDAL_INFO_INFO_DRAW_X , MEDAL_INFO_INFO_DRAW_Y,
        str , p_wk->p_font ,
        font_colro );

    // バッファーの消去 
    GFL_STR_DeleteBuffer( str );

    // 転送
    GFL_BMPWIN_TransVramCharacter( p_wk->p_info_win );

    // 転送リクエスト
    GFL_BG_LoadScreenV_Req(GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_S));
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル名のクリアー
 *
 *	@param	*p_wk 
 *	@param	ClearPos 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void ClearMedalName( MEDALBOXSYS_WORK *p_wk )
{
  PRINTSYS_LSB font_colro = GetMedalNameFontColor();

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_name_win) , 0 );

  // 転送
  GFL_BMPWIN_TransVramCharacter( p_wk->p_name_win );

  // 転送リクエスト
  GFL_BG_LoadScreenV_Req(GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M));
}

//----------------------------------------------------------------------------
/**
 *	@brief  下画面メダルタイトル部分のフォントカラーを取得する
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static PRINTSYS_LSB GetMedalNameFontColor( void )
{
  return PRINTSYS_LSB_Make(MEDAL_NAME_BMPWIN_STR_COL_L,MEDAL_NAME_BMPWIN_STR_COL_S,MEDAL_NAME_BMPWIN_STR_COL_B);
}

//----------------------------------------------------------------------------
/**
 *	@brief  上画面メダル情報部分のフォントカラーを取得する
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static PRINTSYS_LSB GetMedalInfoFontColor( void )
{
  return PRINTSYS_LSB_Make(MEDAL_INFO_BMPWIN_STR_COL_L,MEDAL_INFO_BMPWIN_STR_COL_S,MEDAL_INFO_BMPWIN_STR_COL_B);
}

//----------------------------------------------------------------------------
/**
 *	@brief  メダル用タッチ当たり判定テーブル作成
 *
 *	@param	*p_wk 
 *	@param	medal_hittbl[] 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MakeMedalHitTbl( MEDALBOXSYS_WORK *p_wk ,GFL_UI_TP_HITTBL medal_hittbl[] )
{
  u8 i;
  GFL_CLACTPOS pos;
  for( i=0; i < MEDAL_HIT_TABLE_MAX-1 ; i++ )
  {
    // カーソルの位置を取得
    GetCursorPos( p_wk , i , &pos );

    medal_hittbl[i].circle.code = GFL_UI_TP_USE_CIRCLE;
    medal_hittbl[i].circle.x    = pos.x; 
    medal_hittbl[i].circle.y    = pos.y;
    medal_hittbl[i].circle.r    = MEDAL_HIT_RADIUS;
  }

  medal_hittbl[i].circle.code = GFL_UI_TP_HIT_END;
  medal_hittbl[i].circle.x    = 0;
  medal_hittbl[i].circle.y    = 0;
  medal_hittbl[i].circle.r    = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルの位置を取得する
 *
 *	@param	cursor_pos  カーソルの場所
 *	@param	*p_pos      データの取得先
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void GetCursorPos( MEDALBOXSYS_WORK *p_wk , u8 cursor_pos , GFL_CLACTPOS *p_pos )
{
  // カーソルが番はユニット１の先頭をさしていることに注意
  MDLSYS_GetDrawMedalPos( cursor_pos + PAGE_MEDAL_MAX , p_pos );
  p_pos->y -= 4;  // メダル座標に比べて-4の位置が中心になる
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルを設定する
 *
 *	@param	*p_wk 
 *	@param	cursor_pos カーソルの場所
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetCursor( MEDALBOXSYS_WORK *p_wk , u8 cursor_pos )
{
  GFL_CLACTPOS  pos;
  MY_MEDAL_ST*  p_st;
  u8            unit_pos;
  u8            medal_pos;
  u8            cursor_tmp;

  // 同じ場所は選択できない
  if( ( p_wk->cursor_pos == cursor_pos ) && ( p_wk->cursor_state == CURSOR_STATE_SELECT ))
  {
    return;
  }

  // カーソル位置を保存
  cursor_tmp = p_wk->cursor_pos;
  p_wk->cursor_pos = cursor_pos;

  // ユニット上の位置
  unit_pos = GetCursolSelectUnitPos( p_wk );

  // ページ上のメダル位置
  medal_pos = GetCursolSelectMedalPos( p_wk );

  // ノーマルモード時は1ページ目（画面一番左端のページ）しか選択できない
  if( (p_wk->box_mode == BOXMODE_NORMAL) && (unit_pos != 1) )
  {
    return;
  }

  // メダル情報の取得
  p_st = p_wk->page[unit_pos].p_medal_st[medal_pos];

  if( p_st )
  {
    // カーソルの位置を変更する
    GetCursorPos( p_wk , cursor_pos , &pos );
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , &pos , CLSYS_DEFREND_MAIN );

    // メダル情報を表示する
    DrawMedalInfo( p_wk , p_st );

    // カーソルを選択状態にする
    SetCursorState( p_wk , CURSOR_STATE_SELECT );
  }
  else
  {
    // メダルが存在しなかった場合はカーソル位置を元に戻す
    p_wk->cursor_pos = cursor_tmp;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルの状態
 *
 *	@param	*p_wk 
 *	@param	on_off 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetCursorState( MEDALBOXSYS_WORK *p_wk , CURSOR_STATE state )
{
  p_wk->cursor_state = state;

  switch( p_wk->cursor_state )
  {
  case CURSOR_STATE_NOT_SELECT:
    {
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , FALSE );
    }
    break;
  case CURSOR_STATE_SCROLL:
    {
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , FALSE );
    }
    break;
  case CURSOR_STATE_SELECT:
    {
      // カーソルの選択音を鳴らす
      PMSND_PlaySE( SEQ_SE_SELECT1 );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOR] , TRUE );
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルがどのユニット上にいるかを返す(カーソルが0番目のユニットをさすことはない)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetCursolSelectUnitPos( MEDALBOXSYS_WORK *p_wk )
{
  u8 cursor_pos = p_wk->cursor_pos;
  return cursor_pos/PAGE_MEDAL_MAX + 1;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カーソルがページ上のどのメダルをさしているかを返す
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static u8 GetCursolSelectMedalPos( MEDALBOXSYS_WORK *p_wk )
{
  u8 cursor_pos = p_wk->cursor_pos;
  return cursor_pos%PAGE_MEDAL_MAX; // ユニット内のページ位置
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールを繰り返す回数を設定する
 *
 *	@param	*p_wk 
 *	@param	count 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetScrollCount( MEDALBOXSYS_WORK *p_wk , s32 count )
{
  p_wk->scroll_count = count;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールするスピードを変更する
 *
 *	@param	*p_wk 
 *	@param	speed: 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetScrollSpeed( MEDALBOXSYS_WORK *p_wk , u32 speed )
{
  p_wk->scroll_speed = speed;
}

//----------------------------------------------------------------------------
/**
 *	@brief  終了ボタンシーケンス(SEQ_MDLBOX_EXIT)
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_EXITBUTTON_Main( MEDALBOXSYS_WORK *p_wk )
{
  GFL_CLWK *p_button_clwk = p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_EXIT];
  // 終了ボタンシーケンス
  switch( p_wk->sub_seq )
  {
  case 0:
    {
      // 終了ボタンアニメーション
      GFL_CLACT_WK_SetAnmSeq( p_button_clwk , APP_COMMON_BARICON_EXIT_ON );
      GFL_CLACT_WK_SetAutoAnmFlag( p_button_clwk , TRUE);
      PMSND_PlaySystemSE(SEQ_SE_CLOSE1);
      p_wk->sub_seq++;
    }
    break;
  case 1:
    {
      if( GFL_CLACT_WK_CheckAnmActive(p_button_clwk) == FALSE )
      {
        p_wk->sub_seq++;
      }
    }
    break;
  case 2:
    {
      // ブラックアウトの開始
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
      p_wk->sub_seq++;
    }
    break;
  case 3:
    if( !GFL_FADE_CheckFade() ){
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールボタンシーケンスメイン( SEQ_MDLBOX_SCROLL_BUTTON )
 *
 *	@param	*p_wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT MDLSYS_SCROLLBUTTON_Main( MEDALBOXSYS_WORK *p_wk )
{
  u32 tp_x,tp_y;
  if( GFL_UI_TP_GetPointCont( &tp_x ,&tp_y ) )
  {
    GFL_CLACTPOS pos;
//    if( tp_x > (UI_SCROLBTN_POS_X + p_wk->scroll_button_pos) )
//    {
//      p_wk->scroll_button_pos = tp_x - UI_SCROLBTN_POS_X;
//    }
//    else if( tp_x < (UI_SCROLBTN_POS_X + p_wk->scroll_button_pos) )
//    {
//      p_wk->scroll_button_pos--;
//    }
    p_wk->scroll_button_pos = tp_x - UI_SCROLBTN_POS_X;
    pos.x = UI_SCROLBTN_POS_X + p_wk->scroll_button_pos;
    pos.y = UI_SCROLBTN_POS_Y;
    GFL_CLACT_WK_SetPos( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBTN] , &pos , CLSYS_DEFREND_MAIN );

  }
  else
  {
    MDLSYS_SetNextSeq( p_wk , SEQ_MDLBOX_MAIN );
  }

  return GFL_PROC_RES_CONTINUE;
}

#ifdef PM_DEBUG
// ============================================================
// 
// デバッグ関数
//
// ============================================================

static void DBEG_MDLSYS_DebugInit( MEDALBOXSYS_WORK *p_wk )
{
  GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA], FALSE );
}

static void DBEG_MDLSYS_DebugMain( MEDALBOXSYS_WORK *p_wk )
{
  const int trg = GFL_UI_KEY_GetTrg();
  const int cont = GFL_UI_KEY_GetCont();

  switch( p_wk->debgu_seq )
  {
//  case 0:
//    {
//      // ノーマルモード
//      if( trg & PAD_BUTTON_L )
//      {
//        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA], FALSE );
//        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], FALSE );
//        GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) , VISIBLE_OFF );
//        GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M) , VISIBLE_OFF );
//      }
//
//      // 一覧モード
//      if( trg & PAD_BUTTON_R )
//      {
//        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA], TRUE );
//        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], TRUE );
//        GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) , VISIBLE_ON );
//        GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONT_M) , VISIBLE_ON );
//      }
//      // フタを空ける
//      if( trg & PAD_BUTTON_Y )
//      {
//        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA], FALSE );
//      }
//
//    }
//    break;
//
//  case 1:
//    {
//      if( trg & PAD_BUTTON_A )
//      {
//        p_wk->now_seq++;
//        p_wk->now_seq %= SEQ_MDLBOX_SCROLLPAGE;
//        switch( p_wk->now_seq )
//        {
//        case SEQ_MDLBOX_INIT: OS_Printf("シーケンス：起動直後\n"); break;
//        case SEQ_MDLBOX_MAIN: OS_Printf("シーケンス：メイン\n"); break;
//        case SEQ_MDLBOX_SCROLLPAGE: OS_Printf("シーケンス：ページ移動\n"); break;
//        }
//      }
//    }
//    break;

//  case 2:
//    {
//      if( cont & PAD_BUTTON_L )
//      {
//        p_wk->debug_page_scroll_speed += 0x100;
//        OS_Printf("ページスクロールスピード：%x\n",p_wk->debug_page_scroll_speed);
//      }
//      if( cont & PAD_BUTTON_R )
//      {
//        p_wk->debug_page_scroll_speed -= 0x100;
//        OS_Printf("ページスクロールスピード：%x\n",p_wk->debug_page_scroll_speed);
//      }
//    }
//    break;

  default:
    {
      p_wk->debgu_seq = 0;
    }
    break;
  }
  
  // デバッグシーケンス切替
  if( trg & PAD_BUTTON_SELECT )
  {
    p_wk->debgu_seq ++;
    
    switch( p_wk->debgu_seq )
    {
    case 0: OS_Printf("デバッグモード：グラフィック操作\n"); break;
    case 1: OS_Printf("デバッグモード：シーケンス切替\n"); break;
//    case 2: OS_Printf("デバッグモード：スピード調節操作\n"); break;
    default:OS_Printf("デバッグモード：グラフィック操作\n"); break;
    }
  }
}
#endif

