
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

#include "print/gf_font.h"        // フォント
#include "font/font.naix"         // フォント

#include "medalbox_gra.naix"      // グラフィックアーカイブ

#include "app_menu_common.naix"   // 共通素材
#include "app/app_menu_common.h"  // 共通素材

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
#define MEDALBOXSYS_HEAP_SIZE 0x60000

//-------------------------------------
///	リソース初期位置
//=====================================
#define UI_SCROLBTN_POS_X 256/2
#define UI_SCROLBTN_POS_Y 180

#define UI_SCROLBER_POS_X 256/2
#define UI_SCROLBER_POS_Y 180

#define UI_CURSOL_POS_X sc_medal_pos[0].x     // メダルの1枚目にセット
#define UI_CURSOL_POS_Y sc_medal_pos[0].y - 4 // メダルの1枚目にセット

#define UI_ICHIOSHI_POS_X sc_ichioshi_pos[0].x
#define UI_ICHIOSHI_POS_Y sc_ichioshi_pos[0].y

#define UI_HUTA_POS_X 256/2
#define UI_HUTA_POS_Y 192/2

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
	MEDALBOXSYS_BG_FRAME_FONTBORD_M,	// 文字盤
	MEDALBOXSYS_BG_FRAME_PAGEUNIT_M,  // ページユニット
	MEDALBOXSYS_BG_FRAME_BAR_M,		    // 情報バー
	MEDALBOXSYS_BG_FRAME_ETC_M,		    // 光沢 or メッセージウィンドウ

	MEDALBOXSYS_BG_FRAME_MONITOR_S,		// モニター
	MEDALBOXSYS_BG_FRAME_EFFECTS_S,	  // モニター演出
	MEDALBOXSYS_BG_FRAME_BACK_S ,	    // 背景

	MEDALBOXSYS_BG_FRAME_MAX,			//ソース内で使用
}MEDALBOXSYS_GRAPHIC_BG_FRAME;

//-------------------------------------
///	BGプライオリティー
//=====================================
typedef enum
{
  MEDALBOXSYS_BG_PRIORITY_BAR_M       = 1,
  MEDALBOXSYS_BG_PRIORITY_FONTBORD_M  = 1,
  MEDALBOXSYS_BG_PRIORITY_PAGEUNIT_M,
  MEDALBOXSYS_BG_PRIORITY_ETC_M,

  MEDALBOXSYS_BG_PRIORITY_MONITOR_S   = 0,
  MEDALBOXSYS_BG_PRIORITY_EFFECTS_S,
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
	MEDALBOXSYS_OBJ_CLWK_SCROLBTN,
	MEDALBOXSYS_OBJ_CLWK_SCROLBER,
	MEDALBOXSYS_OBJ_CLWK_CURSOL,
	MEDALBOXSYS_OBJ_CLWK_ICHIOSHI,
	MEDALBOXSYS_OBJ_CLWK_KIRAKIRA,
	MEDALBOXSYS_OBJ_CLWK_HUTA,
	MEDALBOXSYS_OBJ_CLWK_MAX,
	MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX      = MEDALBOXSYS_OBJ_CLWK_MEDAL20 + 1, // メダルのセルID最大数
	MEDALBOXSYS_OBJ_MEDAL_CLWK_UI_START = MEDALBOXSYS_OBJ_CLWK_SCROLBTN,    // UIセルワークの管理ID
}MEDALBOXSYS_GRAPHIC_OBJ_CLWK ;

//OBJリソースタイプ
enum{
 OBJ_RES_TYPE_PLTT,
 OBJ_RES_TYPE_CGR,
 OBJ_RES_TYPE_CELLANIM,
 OBJ_RES_TYPE_MAX,
};

//OBJリソース
enum{
 OBJ_RES_MEDAL,
 OBJ_RES_UI,
 OBJ_RES_MAX,
};

//OBJリソースデータ(リソースの種類＊3種類)
static const struct
{
  u8  res_id;
  u8  res_type;
  u32 dat_id1;
  u32 dat_id2;
  u16 offset;
  CLSYS_DRAW_TYPE draw_type;
//}	sc_res_table[OBJ_RES_MAX*OBJ_RES_TYPE_MAX]	=
}	sc_res_table[6]	=
{
  
  // =============================================
  //  メダル
  // =============================================
  {
    OBJ_RES_MEDAL,
    OBJ_RES_TYPE_CELLANIM,              // セルアニメ
    NARC_medalbox_gra_medal_obj1_NCER,  // ARCID1
    NARC_medalbox_gra_medal_obj1_NANR,  // ARCID2
    0,
    CLSYS_DRAW_MAIN,
  },
  {
    OBJ_RES_MEDAL,
    OBJ_RES_TYPE_PLTT,                  // パレット
    NARC_medalbox_gra_medal_obj1_NCLR,  // ARCID1
    0,
    0,
    CLSYS_DRAW_MAIN,
  },
  {
    OBJ_RES_MEDAL,
    OBJ_RES_TYPE_CGR,                   // キャラクタ
    NARC_medalbox_gra_medal_obj1_NCGR,  // ARCID1
    0,
    CLSYS_DRAW_MAIN,
  },

  // =============================================
  //  UI
  // =============================================
  {
    OBJ_RES_UI,
    OBJ_RES_TYPE_CELLANIM,              // セルアニメ
    NARC_medalbox_gra_medal_obj2_NCER,  // ARCID1
    NARC_medalbox_gra_medal_obj2_NANR,  // ARCID2
    0,
    CLSYS_DRAW_MAIN,
  },
  {
    OBJ_RES_UI,
    OBJ_RES_TYPE_PLTT,                  // パレット
    NARC_medalbox_gra_medal_obj2_NCLR,  // ARCID1
    0,
    32*6,                               // オフセット
    CLSYS_DRAW_MAIN,
  },
  {
    OBJ_RES_UI,
    OBJ_RES_TYPE_CGR,                   // キャラクタ
    NARC_medalbox_gra_medal_obj2_NCGR,  // ARCID1
    0,
    0,
    CLSYS_DRAW_MAIN,
  },
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
		GFL_BG_FRAME1_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_FONTBORD_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_PAGEUNIT_M
	{	
		GFL_BG_FRAME2_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_PAGEUNIT_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_BAR_M
	{	
		GFL_BG_FRAME0_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_BAR_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	// MEDALBOXSYS_BG_FRAME_ETC_M
	{	
		GFL_BG_FRAME3_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x128,
			GX_BG_EXTPLTT_01, MEDALBOXSYS_BG_PRIORITY_ETC_M, 0, 0, FALSE
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
};
//フレーム取得用マクロ
#define GRAPHIC_BG_GetFrame( x )  (sc_bgsetup[ x ].frame)

// ========================
// メダル座標
// ========================
#define PAGE_MEDAL_MAX 5 // 1ページあたりのメダル数
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
	u32							  res[OBJ_RES_MAX][OBJ_RES_TYPE_MAX];

  // メッセージ関連
	GFL_FONT			*p_font;
	GFL_MSGDATA		*p_mdlname_msg;
	GFL_MSGDATA		*p_mdlinfo_msg;
	GFL_MSGDATA		*p_mdlhelp_msg;

  // アプリ関連
  MEDALBOXSYS_SEQ seq_no;
  MY_MEDALDATA    medal_data;
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
// システム共通
//=============================================================================
static void MDLSYS_MakeCommonData( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_DeleteCommonData( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_GraphicInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BGInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_BGDelete( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_OBJInit( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_OBJDelete( MEDALBOXSYS_WORK *p_wk );
static void MDLSYS_UI_Init( MEDALBOXSYS_WORK *p_wk );

static void MDLSYS_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs );


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

  MDLSYS_UI_Init( p_wk );
  
  // マスター輝度を戻す
  {
    GX_SetMasterBrightness( 0 );
    GXS_SetMasterBrightness( 0 );
  }
  
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

  // セルの更新
	GFL_CLACT_SYS_Main();
  
  switch( p_wk->seq_no )
  {

  case SEQ_MDLBOX_INIT:
    {
      const int trg = GFL_UI_KEY_GetTrg();
      if( trg & PAD_BUTTON_START )
      {
        return GFL_PROC_RES_FINISH;
      }

#if PM_DEBUG
      // ノーマルモード
      if( trg & PAD_BUTTON_L )
      {
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA], FALSE );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], FALSE );
        GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) , VISIBLE_OFF );
      }

      // 一覧モード
      if( trg & PAD_BUTTON_R )
      {
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA], TRUE );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI], TRUE );
        GFL_BG_SetVisible( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) , VISIBLE_ON );
      }
#endif

      
    }
    return GFL_PROC_RES_CONTINUE;

  default:
    {
      GF_ASSERT( 0 );
    }
    return GFL_PROC_RES_FINISH;

  }
  return GFL_PROC_RES_FINISH;
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
    GFL_BG_ChangeScreenPalette( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M), 0 , 0 , 32 , 32 , p_wk->medal_data.boxrank + 1 ); 
    GFL_BG_LoadScreenReq( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_FONTBORD_M) );

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

    // ====================================
		// 共通素材
    // ====================================
		p_handle	= GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON , heapID );

    // パレット
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_app_menu_common_menu_bar_NCLR,
        PALTYPE_MAIN_BG, 32*7 , 0 , heapID );

    // キャラクターデータ
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_app_menu_common_menu_bar_NCGR,
         GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_BAR_M), 0, 0, FALSE, heapID );

    // 情報バー
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_app_menu_common_menu_bar_NSCR,
        GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_BAR_M), 0, 0, FALSE, heapID );

    // 使用パレットの書き換え
    GFL_BG_ChangeScreenPalette( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_BAR_M), 0 , 0 , 32 , 32 , 7 ); 
    GFL_BG_LoadScreenReq( GRAPHIC_BG_GetFrame(MEDALBOXSYS_BG_FRAME_BAR_M) );
   
    
		GFL_ARC_CloseDataHandle( p_handle );
	}

  // ブレンド設定
  {
    G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0 , GX_BLEND_PLANEMASK_BG1 , 15, 7 );
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
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_MEDALGRA , heapID );

    for( i = 0 ; i < NELEMS(sc_res_table) ; i++ )
    {
      u8  res_id    = sc_res_table[ i ].res_id;
      u8  res_type  = sc_res_table[ i ].res_type;
      u32 dat_id1   = sc_res_table[ i ].dat_id1;
      u32 dat_id2   = sc_res_table[ i ].dat_id2;
      u16 offset    = sc_res_table[ i ].offset;
      CLSYS_DRAW_TYPE draw_type = sc_res_table[ i ].draw_type;

      ONOUE_Printf( "i = %d\n",i);
      ONOUE_Printf( "res_id     = %u\n",res_id);
      ONOUE_Printf( "res_type   = %u\n",res_type);
      ONOUE_Printf( "dat_id1    = %u\n",dat_id1);
      ONOUE_Printf( "dat_id2    = %u\n",dat_id2);
      ONOUE_Printf( "draw_type  = %d\n",draw_type);

      // リソースタイプ毎の読み込み
      switch( res_type )
      {
      case OBJ_RES_TYPE_CELLANIM :
        {
          // セルアニメの読み込み
          p_wk->res[res_id][res_type]	= GFL_CLGRP_CELLANIM_Register( p_handle, dat_id1, dat_id2, heapID );
        }
        continue;
      case OBJ_RES_TYPE_CGR :
        {
          // キャラクタの読み込み
          p_wk->res[res_id][res_type]	= GFL_CLGRP_CGR_Register( p_handle, dat_id1 , FALSE, draw_type, heapID );
        }
        continue;
      case OBJ_RES_TYPE_PLTT:
        {
          // パレットの読み込み
          p_wk->res[res_id][res_type]	= GFL_CLGRP_PLTT_Register( p_handle, dat_id1 , draw_type, offset , heapID );	
        }
        continue;
      }

    }

    GFL_ARC_CloseDataHandle( p_handle );
  }

	//CLWK作成
	{
		int i;
		GFL_CLWK_DATA cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		for( i = 0; i < MEDALBOXSYS_OBJ_CLWK_MAX; i++ )
		{	
      // メダルセルの作成
      if( MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX > i )
      {
        // 位置データの書き込み
        cldata.pos_x = sc_medal_pos[i%5].x + i/5*80;
        cldata.pos_y = sc_medal_pos[i%5].y;
        cldata.bgpri = MEDALBOXSYS_OBJ_PRIORITY_MEDAL;

        // ワークの作成
        p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
            p_wk->res[OBJ_RES_MEDAL][OBJ_RES_TYPE_CGR],
            p_wk->res[OBJ_RES_MEDAL][OBJ_RES_TYPE_PLTT],
            p_wk->res[OBJ_RES_MEDAL][OBJ_RES_TYPE_CELLANIM], 
            &cldata, CLSYS_DEFREND_MAIN, heapID );
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i] , i );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
      else
      {
        GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
        cldata.bgpri   = MEDALBOXSYS_OBJ_PRIORITY_UI;
        cldata.softpri = MEDALBOXSYS_OBJ_SOFT_PRIORITY_MEDIUM;

        // ワークの作成
        p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit,
            p_wk->res[OBJ_RES_UI][OBJ_RES_TYPE_CGR],
            p_wk->res[OBJ_RES_UI][OBJ_RES_TYPE_PLTT],
            p_wk->res[OBJ_RES_UI][OBJ_RES_TYPE_CELLANIM], 
            &cldata, CLSYS_DEFREND_MAIN, heapID );
        GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[i] , i - MEDALBOXSYS_OBJ_MEDAL_CLWK_MAX );
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
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
    for( i = 0 ; i < OBJ_RES_MAX ; i++ )
    {
      GFL_CLGRP_CELLANIM_Release( p_wk->res[i][OBJ_RES_TYPE_CELLANIM] );
      GFL_CLGRP_CGR_Release( p_wk->res[i][OBJ_RES_TYPE_CGR] );
      GFL_CLGRP_PLTT_Release( p_wk->res[i][OBJ_RES_TYPE_PLTT] );	
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
  
  // スクロールボタン
  pos.x = UI_SCROLBER_POS_X;
  pos.y = UI_SCROLBER_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_SCROLBER] , &pos , CLSYS_DEFREND_MAIN );

  // カーソル
  pos.x = UI_CURSOL_POS_X;
  pos.y = UI_CURSOL_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_CURSOL] , &pos , CLSYS_DEFREND_MAIN );

  // イチオシ
  pos.x = UI_ICHIOSHI_POS_X;
  pos.y = UI_ICHIOSHI_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_ICHIOSHI] , &pos , CLSYS_DEFREND_MAIN );
  
  // キラキラ
  pos.x = UI_CURSOL_POS_X;
  pos.y = UI_CURSOL_POS_Y;
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_KIRAKIRA] , &pos , CLSYS_DEFREND_MAIN );
   
  // フタ
  pos.x = UI_HUTA_POS_X;
  pos.y = UI_HUTA_POS_Y;
  GFL_CLACT_WK_SetSoftPri( p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA] , MEDALBOXSYS_OBJ_SOFT_PRIORITY_TOP );
  GFL_CLACT_WK_SetPos( p_clwk[MEDALBOXSYS_OBJ_CLWK_HUTA] , &pos , CLSYS_DEFREND_MAIN );
}

//----------------------------------------------------------------------------
/**
 *	@brief  グラフィックVブランクタスク
 *
 *	@param	
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void MDLSYS_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs )
{
	GFL_CLACT_SYS_VBlankFunc();
  GFL_BG_VBlankFunc();
}

