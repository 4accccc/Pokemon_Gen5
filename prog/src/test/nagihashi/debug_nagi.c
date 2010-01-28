//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		d_nagi.c
 *	@brief	名木橋用デバッグ
 *	@author	Toru=Nagihashi
 *	@data		2009.05.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//lib
#include <gflib.h>
#include <wchar.h>	//wcslen

//contant
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"

//module
#include "system/bmp_menulist.h"
#include "poke_tool/pokeparty.h"

//archive
#include "arc_def.h"
#include "font/font.naix"

//proc
#include "net_app/irc_aura.h"
#include "net_app/irc_rhythm.h"
#include "net_app/irc_result.h"
#include "net_app/irc_compatible.h"
#include "net_app/compatible_irc_sys.h"
#include "debug_irc_name.h"
#include "net_app/irc_ranking.h"
#include "app/townmap.h"
#include "net_app/worldtrade.h"
#include "app/config_panel.h"
#include "debug_template.h"
#include "debug_poke2dcheck.h"
#include "app/name_input.h"
#include "net_app/wifibattlematch.h"
#include "net_app/battle_recorder.h"
#include "title/title.h"

#include "savedata/irc_compatible_savedata.h"
#include "savedata/shortcut.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	リストの最大文字数
//=====================================
#define LISTDATA_STR_LENGTH	(32)

//-------------------------------------
///	エクスターン
//=====================================
FS_EXTERN_OVERLAY( compatible_irc_sys );
FS_EXTERN_OVERLAY( wifibattlematch );
FS_EXTERN_OVERLAY( battle_recorder );

//-------------------------------------
///	個数
//=====================================
#define CIRCLE_MAX		(10)

//-------------------------------------
///	
//=====================================
typedef enum 
{
	PROC_TYPE_NONE,
	PROC_TYPE_CALL,
	PROC_TYPE_NEXT,
} PROC_TYPE;

//-------------------------------------
///	
//=====================================
typedef enum
{
  LIST_SELECT_NONE,
  LIST_SELECT_DECIDE,
  LIST_SELECT_CANCEL,
} LIST_SELECT;



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
///	3D描画環境
//=====================================
typedef struct {
	GFL_G3D_CAMERA		*p_camera;
} GRAPHIC_3D_WORK;
//-------------------------------------
///	円
//=====================================
enum {	
	CIRCLE_VTX_MAX	=	60,
};
typedef struct 
{
	VecFx16	vtx[CIRCLE_VTX_MAX];
	u16 r;				//半径	
	u16 red:5;		//赤
	u16	green:5;	//緑
	u16 blue:5;		//青
	u16	dummy1:1;

	u16 add_r;
	u16 add_red:5;
	u16 add_green:5;
	u16 add_blue:5;
	u16 dummy2:1;
} CIRCLE_WORK;
//-------------------------------------
///	描画関係
//=====================================
typedef struct 
{
	GRAPHIC_BG_WORK		gbg;
	GRAPHIC_3D_WORK		g3d;
	CIRCLE_WORK				c[CIRCLE_MAX];
	BOOL							is_init;

	GFL_TCB *p_tcb;
} GRAPHIC_WORK;
//-------------------------------------
///	リスト
//=====================================
typedef struct 
{
	BMPMENULIST_WORK	*p_list;
	BMP_MENULIST_DATA *p_list_data;
	u32		select;
} LIST_WORK;
//-------------------------------------
///	メッセージ
//=====================================
typedef struct 
{
	GFL_FONT*				  p_font;
  PRINT_QUE*        p_print_que;
  PRINT_UTIL        print_util;
} MSG_WORK;

//-------------------------------------
///	メインワーク
//=====================================
typedef struct
{
	GRAPHIC_WORK			grp;
	MSG_WORK					msg;
	LIST_WORK					list;
	GFL_BMPWIN				*p_bmpwin;
	BOOL	is_temp_modules;

	BOOL	is_end;
	PROC_TYPE	proc_type;

	//Proc切り替え用
	FSOverlayID overlay_Id;
	const GFL_PROC_DATA *p_procdata;
	void	*p_proc_work;

	IRC_RESULT_PARAM			result_param;
	IRC_COMPATIBLE_PARAM	compatible_param;
	IRC_AURA_PARAM				aura_param;
	IRC_RHYTHM_PARAM			rhythm_param;
	TOWNMAP_PARAM					townmap_param;
	WORLDTRADE_PARAM			gts_param;
	TEMPLATE_PARAM				template_param;
	CONFIG_PANEL_PARAM		config_param;
	WIFIBATTLEMATCH_PARAM	wifibattlematch_param;
	NAMEIN_PARAM					*p_namein_param;
} DEBUG_NAGI_MAIN_WORK;

//-------------------------------------
///	リスト設定テーブル
//=====================================
typedef struct 
{	
	u16	str[LISTDATA_STR_LENGTH];
	u32	param;
} LIST_SETUP_TBL;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Init( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work );
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Exit( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work );
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Main( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work );
//汎用
static void DEBUG_NAGI_COMMAND_CallProc( DEBUG_NAGI_MAIN_WORK * p_wk, FSOverlayID ov_id, const GFL_PROC_DATA *p_procdata, void *p_work );
static void DEBUG_NAGI_COMMAND_NextProc( DEBUG_NAGI_MAIN_WORK * p_wk, FSOverlayID ov_id, const GFL_PROC_DATA *p_procdata, void *p_work );
static void DEBUG_NAGI_COMMAND_ChangeMenu( DEBUG_NAGI_MAIN_WORK * p_wk, const LIST_SETUP_TBL *cp_tbl, u32 tbl_max );
static void DEBUG_NAGI_COMMAND_End( DEBUG_NAGI_MAIN_WORK *p_wk );

static void CreateTemporaryModules( DEBUG_NAGI_MAIN_WORK *p_wk, HEAPID heapID );
static void DeleteTemporaryModules( DEBUG_NAGI_MAIN_WORK *p_wk );
static void MainTemporaryModules( DEBUG_NAGI_MAIN_WORK *p_wk );

//grp
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
static void Graphic_Tcb_Capture( GFL_TCB *p_tcb, void *p_work );

//BG
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
//LIST
static void LIST_Init( LIST_WORK *p_wk, const LIST_SETUP_TBL *cp_tbl, u32 tbl_max, MSG_WORK *p_msg, GFL_BMPWIN*	p_bmpwin, HEAPID heapID );
static void LIST_Exit( LIST_WORK *p_wk );
static void LIST_Main( LIST_WORK *p_wk );
static LIST_SELECT LIST_IsDecide( const LIST_WORK *cp_wk, u32 *p_select );

//MSG_WORK
static void MSG_Init( MSG_WORK *p_wk, HEAPID heapID );
static void MSG_Exit( MSG_WORK *p_wk );
static BOOL MSG_Main( MSG_WORK *p_wk );
static PRINT_UTIL * MSG_GetPrintUtil( MSG_WORK *p_wk, GFL_BMPWIN*	p_bmpwin );
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk );
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk );
//LISTFUNC
typedef void (*LISTDATA_FUNCTION)( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcAura( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcRhythm( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcResult( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcCompatible( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcCompatibleDebug( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcNameDebug( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcRankingDebug( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcGts( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcConfig( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcTemplate( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcPoke2DCheck( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcNamin( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_AddRankData( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_FullRankData( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcTownMap( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallProcSkyJump( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_Return( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_NextListHome( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_NextListPage1( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_FullShortCutData( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallWifiBattleMatch( DEBUG_NAGI_MAIN_WORK *p_wk );
static void LISTDATA_CallBtlRecorder( DEBUG_NAGI_MAIN_WORK *p_wk );
//3d
static void GRAPHIC_3D_Init( GRAPHIC_3D_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_3D_Exit( GRAPHIC_3D_WORK *p_wk );
static void GRAPHIC_3D_StartDraw( GRAPHIC_3D_WORK *p_wk );
static void GRAPHIC_3D_EndDraw( GRAPHIC_3D_WORK *p_wk );
static void Graphic_3d_SetUp( void );
//円プリミティブ
static void CIRCLE_Init( CIRCLE_WORK *p_wk, u16 r, GXRgb color );
static void CIRCLE_Draw( CIRCLE_WORK *p_wk );
static void CIRCLE_SetAddR( CIRCLE_WORK *p_wk, u16 add_r );
static void CIRCLE_SetAddColor( CIRCLE_WORK *p_wk, u16 add_color );
static void Circle_DrawLine( VecFx16 *p_start, VecFx16 *p_end );

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	外部参照PROCデータ
//=====================================
const GFL_PROC_DATA	DebugNagiMainProcData	= 
{
	DEBUG_PROC_NAGI_Init,
	DEBUG_PROC_NAGI_Main,
	DEBUG_PROC_NAGI_Exit,
};

//-------------------------------------
///	BGシステム
//=====================================
typedef enum 
{
	GRAPHIC_BG_FRAME_TEXT,
	GRAPHIC_BG_FRAME_MAX
} GRAPHIC_BG_FRAME;
static const u32 sc_bgcnt_frame[ GRAPHIC_BG_FRAME_MAX ] = 
{
	GFL_BG_FRAME0_S,
};
static const GFL_BG_BGCNT_HEADER sc_bgcnt_data[ GRAPHIC_BG_FRAME_MAX ] = 
{
	// GRAPHIC_BG_FRAME_TEXT
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},
};

//-------------------------------------
///	リストで実行するもの
//=====================================
enum
{	
	LISTDATA_SEQ_PROC_AURA_DEBUG,
	LISTDATA_SEQ_PROC_RHYTHM_DEBUG,
	LISTDATA_SEQ_PROC_RESULT,
	LISTDATA_SEQ_PROC_COMPATIBLE,
	LISTDATA_SEQ_PROC_COMPATIBLE_DEBUG,
	LISTDATA_SEQ_RETURN,
	LISTDATA_SEQ_PROC_NAME_DEBUG,
	LISTDATA_SEQ_PROC_RANKING_DEBUG,
	LISTDATA_SEQ_RANKDATA_ONE,
	LISTDATA_SEQ_RANKDATA_FULL,
	LISTDATA_SEQ_PROC_TOWNMAP,
	LISTDATA_SEQ_PROC_SKYJUMP,
	LISTDATA_SEQ_NEXT_HOME,
	LISTDATA_SEQ_NEXT_PAGE1,
	LISTDATA_SEQ_PROC_GTS,
	LISTDATA_SEQ_PROC_CONFIG,
	LISTDATA_SEQ_PROC_TEMPLATE,
	LISTDATA_SEQ_PROC_POKE2DCHECK,
	LISTDATA_SEQ_PROC_NAMEIN,
	LISTDATA_SEQ_SHORTCUTDATA_FULL,
	LISTDATA_SEQ_PROC_WIFIBATTLEMATCH,
	LISTDATA_SEQ_PROC_BTLRECORDER,

	LISTDATA_SEQ_MAX,
};
static const LISTDATA_FUNCTION	sc_list_funciton[]	= 
{	
	LISTDATA_CallProcAura,
	LISTDATA_CallProcRhythm,
	LISTDATA_CallProcResult,
	LISTDATA_CallProcCompatible,
	LISTDATA_CallProcCompatibleDebug,
	LISTDATA_Return,
	LISTDATA_CallProcNameDebug,
	LISTDATA_CallProcRankingDebug,
	LISTDATA_AddRankData,
	LISTDATA_FullRankData,
	LISTDATA_CallProcTownMap,
	LISTDATA_CallProcSkyJump,
	LISTDATA_NextListHome,
	LISTDATA_NextListPage1,
	LISTDATA_CallProcGts,
	LISTDATA_CallProcConfig,
	LISTDATA_CallProcTemplate,
	LISTDATA_CallProcPoke2DCheck,
	LISTDATA_CallProcNamin,
	LISTDATA_FullShortCutData,
	LISTDATA_CallWifiBattleMatch,
	LISTDATA_CallBtlRecorder,
};

//-------------------------------------
///	リストデータ
//=====================================
static const LIST_SETUP_TBL sc_list_data_home[]	=
{	
#if 0
	{	
		L"バトルレコーダー", LISTDATA_SEQ_PROC_BTLRECORDER,
	},
#endif 
	{	
		L"相性診断（ひとり）", LISTDATA_SEQ_PROC_COMPATIBLE_DEBUG
	},
	{	
		L"運命値チェック", LISTDATA_SEQ_PROC_NAME_DEBUG,
	},
	{	
		L"ランクデータフル",	LISTDATA_SEQ_RANKDATA_FULL,
	},
	{	
		L"ポケモン２Dチェック", LISTDATA_SEQ_PROC_POKE2DCHECK,
	},
	{	
		L"名前入力", LISTDATA_SEQ_PROC_NAMEIN,
	},
	{	
		L"コンフィグ", LISTDATA_SEQ_PROC_CONFIG
	},
	{	
		L"テンプレート", LISTDATA_SEQ_PROC_TEMPLATE,
	},
#if 0
	{	
		L"GTS", LISTDATA_SEQ_PROC_GTS
	},
	{	
		L"結果", LISTDATA_SEQ_PROC_RESULT,
	},

#endif
#if 0
	{	
		L"相性診断（ひとり）", LISTDATA_SEQ_PROC_COMPATIBLE_DEBUG
	},
	{	
		L"オーラ（ひとり）", LISTDATA_SEQ_PROC_AURA_DEBUG
	},
	{	
		L"リズム（ひとり）", LISTDATA_SEQ_PROC_RHYTHM_DEBUG
	},
	{	
		L"運命値チェック", LISTDATA_SEQ_PROC_NAME_DEBUG,
	},
#endif
#if 0
	{	
		L"ランキング", LISTDATA_SEQ_PROC_RANKING_DEBUG,
	},
	{	
		L"ランクデータ１",	LISTDATA_SEQ_RANKDATA_ONE,
	},
	{	
		L"ランクデータフル",	LISTDATA_SEQ_RANKDATA_FULL,
	},
#endif
#if 0
	{	
		L"タウンマップ", LISTDATA_SEQ_PROC_TOWNMAP,
	},
	{	
		L"そらをとぶ", LISTDATA_SEQ_PROC_SKYJUMP,
	},
#endif
	{	
		L"もどる", LISTDATA_SEQ_RETURN
	},
};

static const LIST_SETUP_TBL sc_list_data_page1[]	=
{	
	{	
		L"前へ", LISTDATA_SEQ_NEXT_HOME
	}
};

static const u8 sc_testdata[]=
{
0x00,0x00,0x01,0x06,0x32,0x04,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x30,0x00,0x12,0xFF,0x10,0xFF,0x11,0xFF,0x11,0xFF,0x74,0x5E,0xA2,0x95,0x71,0x67,0x27,0x59,0x1A,0x4F,0x88,0x4E,0x78,0x90,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x0B,0x01,0x01,0x0B,0x02,0x01,0x00
};

#include "savedata/regulation.h"
//----------------------------------------------------------------------------
/**
 *	@brief	メインプロック	初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_parent					親ワーク
 *	@param	*p_work						自分のワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Init( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work )
{	
	DEBUG_NAGI_MAIN_WORK	*p_wk;
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NAGI_DEBUG, 0x20000 );
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(DEBUG_NAGI_MAIN_WORK), HEAPID_NAGI_DEBUG );
	GFL_STD_MemClear( p_wk, sizeof(DEBUG_NAGI_MAIN_WORK) );

	CreateTemporaryModules( p_wk, HEAPID_NAGI_DEBUG );
	GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
	p_wk->p_namein_param	= NAMEIN_AllocParam( HEAPID_NAGI_DEBUG, NAMEIN_GREETING_WORD, 1, 0, NAMEIN_GREETING_WORD_LENGTH, NULL );
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );

  NAGI_Printf( "pokeparty size=[%d]\n", PokeParty_GetWorkSize() );


	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メインプロック	破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_parent					親ワーク
 *	@param	*p_work						自分のワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Exit( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work )
{	
	DEBUG_NAGI_MAIN_WORK	*p_wk;

	p_wk	= p_work;

	if( p_wk->proc_type == PROC_TYPE_NEXT )
	{	
		//次のPROC予約
		GFL_PROC_SysSetNextProc(
				p_wk->overlay_Id, p_wk->p_procdata, p_wk->p_proc_work );
	}
  else if( p_wk->is_end )
  { 

		//次のPROC予約
		GFL_PROC_SysSetNextProc(
				FS_OVERLAY_ID(title), &TitleProcData, NULL );
  }

	GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
	NAMEIN_FreeParam( p_wk->p_namein_param );
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );

	DeleteTemporaryModules( p_wk );

	GFL_PROC_FreeWork( p_proc );
	GFL_HEAP_DeleteHeap( HEAPID_NAGI_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メインプロック	メイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_parent					親ワーク
 *	@param	*p_work						自分のワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_PROC_NAGI_Main( GFL_PROC *p_proc, int *p_seq, void *p_parent, void *p_work )
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

		SEQ_PROC_FADEIN_START,
		SEQ_PROC_FADEIN_WAIT,
		SEQ_CALL_PROC,
		SEQ_RET_PROC,
	};

	DEBUG_NAGI_MAIN_WORK	*p_wk;

	p_wk	= p_work;


	switch( *p_seq )
	{	
	case SEQ_INIT:
//		*p_seq	= SEQ_FADEOUTぷｒ_START;
		//この前の
			*p_seq	= SEQ_MAIN;
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
		{
			u32 idx;
      LIST_SELECT select;
			LIST_Main( &p_wk->list );
      select  = LIST_IsDecide( &p_wk->list, &idx );
			if( select == LIST_SELECT_DECIDE )
			{	
				sc_list_funciton[ idx ]( p_wk );
			}
      if( select == LIST_SELECT_CANCEL )
      { 
        p_wk->is_end  = TRUE;
      }

			//終了判定
			if( p_wk->is_end )
			{	
				*p_seq	= SEQ_FADEIN_START;
			}

			//PROC判定
			switch( p_wk->proc_type )
			{	
			case PROC_TYPE_CALL:
				p_wk->proc_type	= PROC_TYPE_NONE;
				*p_seq	= SEQ_PROC_FADEIN_START;
				break;
			case PROC_TYPE_NEXT:
				*p_seq	= SEQ_FADEIN_START;
				break;
			}
		}
		break;

	case SEQ_PROC_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= SEQ_PROC_FADEIN_WAIT;
		break;

	case SEQ_PROC_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_CALL_PROC;
		}
		break;

	case SEQ_CALL_PROC:
		DeleteTemporaryModules( p_wk );
		GFL_PROC_SysCallProc( p_wk->overlay_Id, p_wk->p_procdata, p_wk->p_proc_work );
		*p_seq	= SEQ_RET_PROC;
		break;

	case SEQ_RET_PROC:
		CreateTemporaryModules( p_wk, HEAPID_NAGI_DEBUG );
		NAGI_Printf( "名前入力:cansel%d\n", p_wk->p_namein_param->cancel );
		*p_seq	= SEQ_FADEOUT_START;
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
		GF_ASSERT_MSG( 0, "DEBUG_PROC_NAGI_MainのSEQエラー %d", *p_seq );
	}

	MainTemporaryModules( p_wk );

	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *		操作用
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	プロセス切り替え
 *
 *	@param	DEBUG_NAGI_MAIN_WORK * p_wk	ワーク
 *	@param	ov_id												オーバーレイID
 *	@param	GFL_PROC_DATA *p_procdata		プロセスデータ
 *	@param	*p_work											渡す情報
 *
 */
//-----------------------------------------------------------------------------
static void DEBUG_NAGI_COMMAND_CallProc( DEBUG_NAGI_MAIN_WORK * p_wk, FSOverlayID ov_id, const GFL_PROC_DATA *p_procdata, void *p_work )
{	
	p_wk->overlay_Id	= ov_id;
	p_wk->p_procdata	= p_procdata;
	p_wk->p_proc_work	= p_work;
//	p_wk->is_end	= TRUE;

	p_wk->proc_type	= PROC_TYPE_CALL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロセス予約
 *
 *	@param	DEBUG_NAGI_MAIN_WORK * p_wk		ワーク
 *	@param	ov_id											オーバーレイID
 *	@param	GFL_PROC_DATA *p_procdata	プロセスデータ
 *	@param	*p_work										渡す情報
 */
//-----------------------------------------------------------------------------
static void DEBUG_NAGI_COMMAND_NextProc( DEBUG_NAGI_MAIN_WORK * p_wk, FSOverlayID ov_id, const GFL_PROC_DATA *p_procdata, void *p_work )
{	
	p_wk->overlay_Id	= ov_id;
	p_wk->p_procdata	= p_procdata;
	p_wk->p_proc_work	= p_work;
//	p_wk->is_end	= TRUE;

	p_wk->proc_type	= PROC_TYPE_NEXT;
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニューを切り替え
 *
 *	@param	DEBUG_NAGI_MAIN_WORK * p_wk	ワーク
 *	@param	LIST_SETUP_TBL *cp_tbl			リスト用テーブル
 *	@param	tbl_max											リスト用テーブルの要素数
 *
 */
//-----------------------------------------------------------------------------
static void DEBUG_NAGI_COMMAND_ChangeMenu( DEBUG_NAGI_MAIN_WORK * p_wk, const LIST_SETUP_TBL *cp_tbl, u32 tbl_max )
{
	LIST_Exit( &p_wk->list );
	LIST_Init( &p_wk->list, cp_tbl, tbl_max, &p_wk->msg, p_wk->p_bmpwin, HEAPID_NAGI_DEBUG );
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void DEBUG_NAGI_COMMAND_End( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	一時的なモジュールを作成
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *	@param	heapID											ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void CreateTemporaryModules( DEBUG_NAGI_MAIN_WORK *p_wk, HEAPID heapID )
{	
	GRAPHIC_Init( &p_wk->grp, heapID );

	MSG_Init( &p_wk->msg, heapID );

	p_wk->p_bmpwin	= GFL_BMPWIN_Create( sc_bgcnt_frame[GRAPHIC_BG_FRAME_TEXT],
			1, 1, 30, 10, 0, GFL_BMP_CHRAREA_GET_B );
	GFL_BMPWIN_MakeScreen( p_wk->p_bmpwin );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(p_wk->p_bmpwin) );

	LIST_Init( &p_wk->list, sc_list_data_home, NELEMS(sc_list_data_home), 
			&p_wk->msg, p_wk->p_bmpwin, heapID );

	p_wk->is_temp_modules	= TRUE;

}
//----------------------------------------------------------------------------
/**
 *	@brief	一時的なモジュールを破棄
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void DeleteTemporaryModules( DEBUG_NAGI_MAIN_WORK *p_wk )
{	

	LIST_Exit( &p_wk->list );
	GFL_BMPWIN_Delete( p_wk->p_bmpwin );
	MSG_Exit( &p_wk->msg );
	GRAPHIC_Exit( &p_wk->grp );

	p_wk->is_temp_modules	= FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void MainTemporaryModules( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	if( p_wk->is_temp_modules )
	{	
		MSG_Main( &p_wk->msg );
		GRAPHIC_Draw( &p_wk->grp );
	}
}
//=============================================================================
/**
 *	リスト関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ProcChange
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY(irc_aura);
static void LISTDATA_CallProcAura( DEBUG_NAGI_MAIN_WORK *p_wk )
{
	GFL_STD_MemClear( &p_wk->aura_param, sizeof(IRC_AURA_PARAM) );
	p_wk->aura_param.is_only_play	= TRUE;
	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(irc_aura), &IrcAura_ProcData, &p_wk->aura_param );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ProcChange
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY(irc_rhythm);
static void LISTDATA_CallProcRhythm( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	GFL_STD_MemClear( &p_wk->rhythm_param, sizeof(IRC_RHYTHM_PARAM) );
	p_wk->rhythm_param.is_only_play	= TRUE;
	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(irc_rhythm), &IrcRhythm_ProcData, &p_wk->rhythm_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ProcChange
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY(irc_result);
static void LISTDATA_CallProcResult( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	GFL_STD_MemClear( &p_wk->result_param, sizeof(IRC_RESULT_PARAM) );
	p_wk->result_param.score	= 80;
	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(irc_result), &IrcResult_ProcData, &p_wk->result_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ProcChange
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY(irc_compatible);
static void LISTDATA_CallProcCompatible( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	p_wk->compatible_param.p_gamesys	= NULL;
	p_wk->compatible_param.is_only_play	= FALSE;
	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(irc_compatible), &IrcCompatible_ProcData, &p_wk->compatible_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ProcChange
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_CallProcCompatibleDebug( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	p_wk->compatible_param.p_gamesys	= NULL;
	p_wk->compatible_param.is_only_play	= TRUE;
	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(irc_compatible), &IrcCompatible_ProcData, &p_wk->compatible_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_CallProcNameDebug( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_CallProc( p_wk, NO_OVERLAY_ID, &DebugIrcName_ProcData, NULL );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ランキングPROCへの遷移
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wkワーク
 *
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY(irc_ranking);
static void LISTDATA_CallProcRankingDebug( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(irc_ranking), &IrcRanking_ProcData, NULL );
}
//----------------------------------------------------------------------------
/**
 *	@brief	GTSPROCへの移動
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY(worldtrade);
static void LISTDATA_CallProcGts( DEBUG_NAGI_MAIN_WORK *p_wk )
{
	GAMEDATA * p_gamedata	= GAMEDATA_Create(HEAPID_NAGI_DEBUG);

	p_wk->gts_param.savedata				= GAMEDATA_GetSaveControlWork( p_gamedata );
	p_wk->gts_param.worldtrade_data	= SaveData_GetWorldTradeData(p_wk->gts_param.savedata);
	p_wk->gts_param.systemdata			= SaveData_GetSystemData(p_wk->gts_param.savedata);
	p_wk->gts_param.myparty					= SaveData_GetTemotiPokemon(p_wk->gts_param.savedata);
	p_wk->gts_param.zukanwork				= NULL;
	p_wk->gts_param.mybox	          = GAMEDATA_GetBoxManager(p_gamedata);
	p_wk->gts_param.wifilist				= GAMEDATA_GetWiFiList(p_gamedata);
	p_wk->gts_param.wifihistory			= SaveData_GetWifiHistory(p_wk->gts_param.savedata);
	p_wk->gts_param.mystatus				= GAMEDATA_GetMyStatus(p_gamedata);
	p_wk->gts_param.config					= SaveData_GetConfig(p_wk->gts_param.savedata);
	p_wk->gts_param.record					= SaveData_GetRecord(p_wk->gts_param.savedata);
	p_wk->gts_param.myitem					= NULL;
	
	p_wk->gts_param.zukanmode				= 0;
	p_wk->gts_param.profileId				= 0;
	p_wk->gts_param.contestflag			= FALSE;
	p_wk->gts_param.connect					= 0;


	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(worldtrade), &WorldTrade_ProcData, &p_wk->gts_param );

	GAMEDATA_Delete( p_gamedata );
}
//----------------------------------------------------------------------------
/**
 *	@brief	コンフィグPROCへの移動
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_CallProcConfig( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(config_panel), &ConfigPanelProcData, &p_wk->config_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	テンプレートPROCへの移動
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_CallProcTemplate( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_CallProc( p_wk, NO_OVERLAY_ID, &DebugTemplate_ProcData, &p_wk->template_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ポケモン２Dグラフィックチェック
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void LISTDATA_CallProcPoke2DCheck( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_CallProc( p_wk, NO_OVERLAY_ID, &DebugPoke2D_ProcData, NULL );
}
//----------------------------------------------------------------------------
/**
 *	@brief	名前入力
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void LISTDATA_CallProcNamin( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
  DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(namein), &NameInputProcData, p_wk->p_namein_param );
}

#include "system/rtc_tool.h"
static u32 RULE_CalcBioRhythm( const COMPATIBLE_STATUS *cp_status )
{ 
  enum
  { 
    BIORHYTHM_CYCLE = 30,  //周期
  };


  u32 days;
  u32 now_days; //今日の日付を日数に
  u32 days_diff;
  fx32 sin;
  u32 bio;

  //今日までの総日数を計算（年が取れないので、一年だけとする）
  { 
    RTCDate date;
    GFL_RTC_GetDate( &date );
    now_days  = GFL_RTC_GetDaysOffset(&date);

    date.month  = cp_status->barth_month;
    date.day    = cp_status->barth_day;
    days        = GFL_RTC_GetDaysOffset(&date);
  }

  //誕生日から今日まで何日かかっているか
  if( now_days >= days  )
  { 
    days  += 365;
  }
  days_diff = days  - now_days;

  days_diff %= BIORHYTHM_CYCLE;

  sin = FX_SinIdx( 0xFFFF * days_diff / BIORHYTHM_CYCLE );

  bio = ((sin + FX32_ONE) * 50 ) >> FX32_SHIFT;

  OS_TFPrintf( 3, "バイオリズム %d 誕生経過%d 現在%d,差%d\n", bio, days, now_days, days_diff );

  return bio;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ランキングデータ１つ挿入
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
#include "debug/debug_str_conv.h"
static void LISTDATA_AddRankData( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	IRC_COMPATIBLE_SAVEDATA *p_sv	= IRC_COMPATIBLE_SV_GetSavedata( SaveControl_GetPointer() );
	{	
		STRCODE	str[128];
		u16	strlen;
		u16 idx;
		static const struct
		{	
			u16 * p_str;
      u8  sex;
      u8  month;
      u8  day;
			u32 ID;
		} scp_debug_rank_data[]	=
		{	
			{	
				L"かつのり",
        1,
        10,
        22,
				0x573,
			},
			{	
				L"アリイズミ",
        1,
        1,
        27,
				0x785,
			},
			{	
				L"キタさん",
        1,
        4,
        8,
				0x123,
			},
			{	
				L"いわおっち",
        1,
        7,
        14,
				0x987,
			},
			{	
				L"レイコ",
        0,
        2,
        15,
				0x782
			},
			{	
				L"イケイケ",
        0,
        8,
        2,
				0x191,
			},
			{	
				L"ぺぐ",
        0,
        1,
        7,
				0x232
			},
			{	
				L"マナ",
        0,
        11,
        27,
				0x595,
			},
			{	
				L"あさみん",
        0,
        10,
        18,
				0x999,
			},
			{	
				L"プラット",
        1,
        1,
        1,
				0x1234,
			},
			{	
				L"プラット",
        0,
        3,
        3,
				0x2345,
			},
			{	
				L"プラット",
        1,
        10,
        10,
				0x3456,
			},
		};
		u16 *p_str;

		idx	= GFUser_GetPublicRand( NELEMS(scp_debug_rank_data) );

		p_str	= scp_debug_rank_data[idx].p_str;

		strlen	= wcslen(p_str);
		GFL_STD_MemCopy(p_str, str, strlen*2);
		str[strlen]	= GFL_STR_GetEOMCode();

		//セーブする
		IRC_COMPATIBLE_SV_AddRanking( p_sv, str,
        GFUser_GetPublicRand( 101 ), scp_debug_rank_data[idx].sex, scp_debug_rank_data[idx].month, scp_debug_rank_data[idx].day, scp_debug_rank_data[idx].ID );

    { 
      char  test[128];
      DEB_STR_CONV_StrcodeToSjis( str , test, 5 );
      OS_TFPrintf( 3, "%s", test );
    }
    { 
      COMPATIBLE_STATUS status;
      status.barth_month  = scp_debug_rank_data[idx].month;
      status.barth_day    = scp_debug_rank_data[idx].day;
      RULE_CalcBioRhythm( &status );
    }
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ランキングデータをFull
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_FullRankData( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	IRC_COMPATIBLE_SAVEDATA *p_sv	= IRC_COMPATIBLE_SV_GetSavedata( SaveControl_GetPointer() );
	while( IRC_COMPATIBLE_SV_GetRankNum( p_sv ) < IRC_COMPATIBLE_SV_RANKING_MAX )
	{	
		LISTDATA_AddRankData( p_wk );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	タウンマップPROCへ
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY(townmap);
static void LISTDATA_CallProcTownMap( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	GFL_STD_MemClear( &p_wk->townmap_param, sizeof(TOWNMAP_PARAM) );
	p_wk->townmap_param.mode			= TOWNMAP_MODE_MAP;
	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(townmap), &TownMap_ProcData, &p_wk->townmap_param );
}
static void LISTDATA_CallProcSkyJump( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	GFL_STD_MemClear( &p_wk->townmap_param, sizeof(TOWNMAP_PARAM) );
	p_wk->townmap_param.mode			= TOWNMAP_MODE_DEBUGSKY;
	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(townmap), &TownMap_ProcData, &p_wk->townmap_param );
}
//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_Return( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_End( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	最初のリストに
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_NextListHome( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_ChangeMenu( p_wk, sc_list_data_home, NELEMS(sc_list_data_home) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	1ページへ飛ぶ
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LISTDATA_NextListPage1( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	DEBUG_NAGI_COMMAND_ChangeMenu( p_wk, sc_list_data_page1, NELEMS(sc_list_data_page1) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	Yボタン全部登録
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void LISTDATA_FullShortCutData( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	int i;
	SHORTCUT *p_sv	= SaveData_GetShortCut( SaveControl_GetPointer() );
	for( i = 0; i < SHORTCUT_ID_MAX; i++ )
	{	
		SHORTCUT_SetRegister( p_sv, i, TRUE );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	バトルマッチ画面へ飛ぶ
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void LISTDATA_CallWifiBattleMatch( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
	p_wk->wifibattlematch_param.mode	= WIFIBATTLEMATCH_MODE_RANDOM;
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
	{	
    p_wk->wifibattlematch_param.mode	= WIFIBATTLEMATCH_MODE_WIFI;
	}
	else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
	{	
		p_wk->wifibattlematch_param.mode	= WIFIBATTLEMATCH_MODE_LIVE;
	}

	DEBUG_NAGI_COMMAND_CallProc( p_wk, FS_OVERLAY_ID(wifibattlematch_sys), &WifiBattleMatch_ProcData, &p_wk->wifibattlematch_param );
}

//----------------------------------------------------------------------------
/**
 *	@brief	バトルレコーダーを呼ぶ
 *
 *	@param	DEBUG_NAGI_MAIN_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static	BATTLERECORDER_PARAM	s_battle_recorder_param;
static void LISTDATA_CallBtlRecorder( DEBUG_NAGI_MAIN_WORK *p_wk )
{	
  GFL_STD_MemClear( &s_battle_recorder_param, sizeof(BATTLERECORDER_PARAM) );

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  { 
    s_battle_recorder_param.mode		= BR_MODE_GLOBAL_BV;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  { 
    s_battle_recorder_param.mode		= BR_MODE_GLOBAL_MUSICAL;
  }
  else
  { 
    s_battle_recorder_param.mode		= BR_MODE_BROWSE;
  }
	s_battle_recorder_param.p_gamedata	= NULL;
	DEBUG_NAGI_COMMAND_NextProc( p_wk, FS_OVERLAY_ID(battle_recorder), &BattleRecorder_ProcData, &s_battle_recorder_param.mode );
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
		GX_VRAM_OBJ_NONE,						// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_0_D,						// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_128K,		
		GX_OBJVRAMMODE_CHAR_1D_128K,		
	};

	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );

	//VRAMクリアー
	GFL_DISP_ClearVRAM( 0 );

	// VRAMバンク設定
	GFL_DISP_SetBank( &sc_vramSetTable );
	GX_SetBankForLCDC(GX_VRAM_LCDC_B);	//Capture用

	// ディスプレイON
	GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
	GFL_DISP_SetDispOn();

	//表示
	GFL_DISP_GX_InitVisibleControl();

	//	フォント初期化
	GFL_FONTSYS_Init();

	//描画モジュール
	GRAPHIC_BG_Init( &p_wk->gbg, heapID );
	GRAPHIC_3D_Init( &p_wk->g3d, heapID );

	{	
		int i;
		for( i = 0; i < CIRCLE_MAX; i++ )
		{	
			CIRCLE_Init( &p_wk->c[i], 0, GX_RGB(31,16,0) );
			CIRCLE_SetAddR( &p_wk->c[i], 0x8F+0x1F*i );
			CIRCLE_SetAddColor( &p_wk->c[i], GX_RGB(0,0,1) );
		}
	}

	p_wk->p_tcb	= GFUser_VIntr_CreateTCB(Graphic_Tcb_Capture, p_wk, 0 );
	
	p_wk->is_init	= TRUE;
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
	GF_ASSERT( p_wk->is_init );

	GFL_TCB_DeleteTask( p_wk->p_tcb );
	GX_ResetCapture();

	GRAPHIC_3D_Exit( &p_wk->g3d );
	GRAPHIC_BG_Exit( &p_wk->gbg );

	p_wk->is_init	= FALSE;
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );
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
	if( p_wk->is_init )
	{	
		GRAPHIC_3D_StartDraw( &p_wk->g3d );
		//NNS系の3D描画
		//なし
		NNS_G3dGeFlushBuffer();
		//SDK系の3D描画

		//円の描画
		{	
			int i;
			for( i = 0; i < CIRCLE_MAX; i++ )
			{	
				CIRCLE_Draw( &p_wk->c[i] );
			}
		}
		GRAPHIC_3D_EndDraw( &p_wk->g3d );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	Vブランクタスク
 *
 *	@param	GFL_TCB *p_tcb
 *	@param	*p_work 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void Graphic_Tcb_Capture( GFL_TCB *p_tcb, void *p_work )
{	
	GFL_BG_VBlankFunc();

	GX_SetCapture(GX_CAPTURE_SIZE_256x192,  // Capture size
                      GX_CAPTURE_MODE_AB,			   // Capture mode
                      GX_CAPTURE_SRCA_3D,						 // Blend src A
                      GX_CAPTURE_SRCB_VRAM_0x00000,     // Blend src B
                      GX_CAPTURE_DEST_VRAM_B_0x00000,   // Output VRAM
                      14,             // Blend parameter for src A
                      14);            // Blend parameter for src B
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
		//	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
				GX_DISPMODE_VRAM_B,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
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
		static const GFL_G3D_LIGHT sc_GFL_G3D_LIGHT[] = {
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
 *					LIST
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	リストシステム　初期化
 *
 *	@param	LIST_WORK *p_wk	ワーク
 *	@param	*p_tbl					設定テーブル
 *	@param	tbl_max					設定テーブル数
 *	@param	*p_msg					メッセージワーク
 *	@param	HEAPID heapID		ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void LIST_Init( LIST_WORK *p_wk, const LIST_SETUP_TBL *cp_tbl, u32 tbl_max, MSG_WORK *p_msg, GFL_BMPWIN*	p_bmpwin, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(LIST_WORK));


	//LISTDATA create
	{	
		int i;
		STRBUF	*p_strbuf;
		STRCODE	str[128];
		u16	strlen;

		p_wk->p_list_data	= BmpMenuWork_ListCreate( tbl_max, heapID );

		p_strbuf = GFL_STR_CreateBuffer( LISTDATA_STR_LENGTH+1, heapID);

		for( i = 0; i < tbl_max; i++ )
		{	
			//
			strlen	= wcslen(cp_tbl[i].str);
			GFL_STD_MemCopy(cp_tbl[i].str, str, strlen*2);
			str[strlen]	= GFL_STR_GetEOMCode();
			GFL_STR_SetStringCode( p_strbuf, str);

			BmpMenuWork_ListAddString( &p_wk->p_list_data[i], p_strbuf, cp_tbl[i].param, heapID );
		}

		GFL_STR_DeleteBuffer( p_strbuf );
	}

	//LIST create
	{	
		BMPMENULIST_HEADER	header;

		GFL_STD_MemClear( &header, sizeof(BMPMENULIST_HEADER));
		header.list				= p_wk->p_list_data;
		header.win				= p_bmpwin;
		header.count			= tbl_max;
		header.line				= 5;
		header.rabel_x		= 0;
		header.data_x			= 16;
		header.cursor_x		= 0;
		header.line_y			= 2;
		header.f_col			= 1;
		header.b_col			= 15;
		header.s_col			= 2;
		header.msg_spc		= 0;
		header.line_spc		= 0;
		header.page_skip	= BMPMENULIST_LRKEY_SKIP;
		header.font				= 0;
		header.c_disp_f		= 0;
		header.work				= NULL;
		header.font_size_x= 16;
		header.font_size_y= 16;
		header.msgdata		= NULL;
		header.print_util	= MSG_GetPrintUtil( p_msg, p_bmpwin );
		header.print_que	= MSG_GetPrintQue( p_msg );
		header.font_handle= MSG_GetFont( p_msg );

		p_wk->p_list	= BmpMenuList_Set( &header, 0, 0, heapID );
		BmpMenuList_SetCursorBmp( p_wk->p_list, heapID );
	}

}

//----------------------------------------------------------------------------
/**
 *	@brief	リストシステム	破棄
 *
 *	@param	LIST_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LIST_Exit( LIST_WORK *p_wk )
{
	BmpMenuList_Exit( p_wk->p_list, NULL, NULL );
	BmpMenuWork_ListDelete( p_wk->p_list_data );
	GFL_STD_MemClear( p_wk, sizeof(LIST_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	リストシステム	メイン処理
 *
 *	@param	LIST_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void LIST_Main( LIST_WORK *p_wk )
{	
	p_wk->select	= BmpMenuList_Main(	p_wk->p_list );
}

//----------------------------------------------------------------------------
/**
 *	@brief	リスト決定
 *
 *	@param	const LIST_WORK *cp_wk	ワーク
 *	@param	*p_select								リストのデータ
 *
 *	@retval	TRUEならば決定
 *	@retval	FALSEならば選択中
 */
//-----------------------------------------------------------------------------
static LIST_SELECT LIST_IsDecide( const LIST_WORK *cp_wk, u32 *p_select )
{	
	switch( cp_wk->select)
  { 
  case BMPMENULIST_NULL:
    return LIST_SELECT_NONE;

	case BMPMENULIST_CANCEL:
    return LIST_SELECT_CANCEL;

  default:
		if( p_select )
		{	
			*p_select	= cp_wk->select;
		}
		return LIST_SELECT_DECIDE;
  }

	return LIST_SELECT_NONE;
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

	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
    NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

	p_wk->p_print_que = PRINTSYS_QUE_Create( heapID );

	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0, 0, heapID );
	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 0, 0, heapID );
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
	if( PRINTSYS_QUE_Main( p_wk->p_print_que ) )
	{
		return PRINT_UTIL_Trans( &p_wk->print_util, p_wk->p_print_que );
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PRINT_UTILを設定し取得
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *	@param	BMPWIN
 *
 *	@return	PRINT_UTIL
 */
//-----------------------------------------------------------------------------
static PRINT_UTIL * MSG_GetPrintUtil( MSG_WORK *p_wk, GFL_BMPWIN*	p_bmpwin )
{	
	PRINT_UTIL_Setup( &p_wk->print_util, p_bmpwin );
	return &p_wk->print_util;
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
//=============================================================================
/**
 *				円プリミティブ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	円プリミティブを作成
 *
 *	@param	CIRCLE_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_Init( CIRCLE_WORK *p_wk, u16 r, GXRgb color )
{	
	GFL_STD_MemClear( p_wk, sizeof(CIRCLE_WORK) );
	p_wk->r			= r;
	p_wk->red		= (color&GX_RGB_R_MASK)>>GX_RGB_R_SHIFT;
	p_wk->green	= (color&GX_RGB_G_MASK)>>GX_RGB_G_SHIFT;
	p_wk->blue	= (color&GX_RGB_B_MASK)>>GX_RGB_B_SHIFT;

	//円の頂点作成
	{	
		int i;
		for( i = 0; i < CIRCLE_VTX_MAX; i++ )
		{	
			p_wk->vtx[i].x	=	FX_CosIdx( ( i * 360 / CIRCLE_VTX_MAX) * 0xFFFF / 360 );
			p_wk->vtx[i].y	=	FX_SinIdx( ( i * 360 / CIRCLE_VTX_MAX) * 0xFFFF / 360 );
			p_wk->vtx[i].z	=	0;
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	円を描画
 *
 *	@param	CIRCLE_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_Draw( CIRCLE_WORK *p_wk )
{	
	//マテリアル設定
	G3_MaterialColorDiffAmb(GX_RGB(16, 16, 16), GX_RGB(16, 16, 16), FALSE );
	G3_MaterialColorSpecEmi(GX_RGB( 16, 16, 16 ), GX_RGB( 31,31,31 ), FALSE );
	G3_PolygonAttr( GX_LIGHTMASK_0123,GX_POLYGONMODE_MODULATE,GX_CULL_BACK,0,31,0 );

	//サイズ設定
	G3_Scale( FX_SinIdx(p_wk->r), FX_SinIdx(p_wk->r), FX_SinIdx(p_wk->r));

	//描画
	G3_Begin( GX_BEGIN_TRIANGLES );
	{
		int i;
		G3_Color(GX_RGB( p_wk->red, p_wk->green, p_wk->blue ));
		for( i = 0; i < CIRCLE_VTX_MAX - 1; i++ )
		{	
			Circle_DrawLine( &p_wk->vtx[i], &p_wk->vtx[i+1] );
		}
		Circle_DrawLine( &p_wk->vtx[i], &p_wk->vtx[0] );
	}
	G3_End();

	//動作
	p_wk->r				+= p_wk->add_r;
	p_wk->red			+= p_wk->add_red;
	p_wk->green		+= p_wk->add_green;
	p_wk->blue		+= p_wk->add_blue;
	p_wk->red			%= 31;
	p_wk->green		%= 31;
	p_wk->blue		%= 31;

}

//----------------------------------------------------------------------------
/**
 *	@brief	半径加算値をセット
 *
 *	@param	CIRCLE_WORK *p_wk	ワーク
 *	@param	add_r							半径加算値	
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_SetAddR( CIRCLE_WORK *p_wk, u16 add_r )
{	
	p_wk->add_r			= add_r;
}

//----------------------------------------------------------------------------
/**
 *	@brief	色加算値をセット
 *
 *	@param	CIRCLE_WORK *p_wk	ワーク
 *	@param	add_color					色加算値
 *
 */
//-----------------------------------------------------------------------------
static void CIRCLE_SetAddColor( CIRCLE_WORK *p_wk, u16 add_color )
{	
	p_wk->add_red		= (add_color&GX_RGB_R_MASK)>>GX_RGB_R_SHIFT;
	p_wk->add_green	= (add_color&GX_RGB_G_MASK)>>GX_RGB_G_SHIFT;
	p_wk->add_blue	= (add_color&GX_RGB_B_MASK)>>GX_RGB_B_SHIFT;
}
//----------------------------------------------------------------------------
/**
 *	@brief	線を描画
 *
 *	@param	VecFx16 *p_start		開始座標
 *	@param	*p_end							終了座標
 *
 */
//-----------------------------------------------------------------------------
static void Circle_DrawLine( VecFx16 *p_start, VecFx16 *p_end )
{	
	G3_Vtx( p_start->x, p_start->y, p_start->z);
	G3_Vtx( p_end->x, p_end->y, p_end->z);
	G3_Vtx( p_start->x, p_start->y, p_start->z);
}
