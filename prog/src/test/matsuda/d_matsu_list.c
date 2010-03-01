//==============================================================================
/**
 * @file	d_matsu_item.c
 * @brief	アイテムテスト
 * @author	matsuda
 * @date	2008.11.12(水)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include <backup_system.h>
//#include "savedata/contest_savedata.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"

#include "net\network_define.h"

#include "print\printsys.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "message.naix"
#include "test_graphic\d_taya.naix"
#include "msg\msg_d_matsu.h"
#include "test/performance.h"
#include "font/font.naix"
#include "pokeicon/pokeicon.h"

#include "item/item.h"
#include "poke_tool/pokeparty.h"
#include "trade/trade_main.h"

#include "net_app/balloon.h"
#include "net_app/compati_control.h"
#include "net_app/wifi_lobby.h"

#include "test\wflby_debug.h"


//==============================================================================
//	定数定義
//==============================================================================
///STRBUFポインタの数(メニューの配列より多く持っておく事)
#define D_STRBUF_NUM			(30)

FS_EXTERN_OVERLAY(matsuda_debug);
FS_EXTERN_OVERLAY(balloon);
FS_EXTERN_OVERLAY(compati_check);
FS_EXTERN_OVERLAY(palace);
//FS_EXTERN_OVERLAY(wifilobby_common);
//FS_EXTERN_OVERLAY(netold);
//FS_EXTERN_OVERLAY(minigame_common);


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	GFL_BMPWIN		*win;
	GFL_BMP_DATA	*bmp;
	PRINT_UTIL		printUtil[1];
	BOOL			message_req;		///<TRUE:メッセージリクエストがあった
}DM_MSG_DRAW_WIN;

typedef struct {

	u16		seq;
	HEAPID	heapID;
	int debug_mode;
	int timer;
	
	GFL_FONT		*fontHandle;
	PRINT_QUE		*printQue;
	PRINT_STREAM	*printStream;
	GFL_MSGDATA		*mm;
	STRBUF			*strbuf[D_STRBUF_NUM];
	STRBUF			*strbuf_name;
	STRBUF			*strbuf_name_kanji;
	STRBUF			*strbuf_info;
	STRBUF			*strbuf_info_kanji;
	GFL_TCBLSYS		*tcbl;
	DM_MSG_DRAW_WIN drawwin;
	
	//アクター
	GFL_CLUNIT *clunit;
	GFL_CLWK	*clwk_icon;		//アイテムアイコンアクター
	NNSG2dImagePaletteProxy	icon_pal_proxy;
	NNSG2dCellDataBank	*icon_cell_data;
	NNSG2dAnimBankData	*icon_anm_data;
	void	*icon_cell_res;
	void	*icon_anm_res;
	
	int cursor_y;
	
	void *parent_work;
}D_MATSU_WORK;

///メニューリスト
typedef struct{
	u32 str_id;
	const GFL_PROC_DATA *next_proc;
	void *(*parent_work_func)(D_MATSU_WORK *);
	u32 ov_id;
}D_MENULIST;

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL DebugMatsuda_ItemDebug(D_MATSU_WORK *wk);
static void * _BalloonParentWorkCreate(D_MATSU_WORK *wk);
static void * _WifiLobbyParentWorkCreate(D_MATSU_WORK *wk);


//==============================================================================
//	外部データ
//==============================================================================
extern const GFL_PROC_DATA DebugMatsudaMainProcData;
extern const GFL_PROC_DATA DebugMatsudaNetProcData;
extern const GFL_PROC_DATA DebugMatsudaIrcMatchProcData;
extern const GFL_PROC_DATA DebugMatsudaErrorProcData;
extern const GFL_PROC_DATA DebugMatsudaItemProcData;
extern const GFL_PROC_DATA DebugSaveProcData;
extern const GFL_PROC_DATA PalaceBlockProcData;
extern const GFL_PROC_DATA PalaceHandProcData;
extern const GFL_PROC_DATA DebugMatsudaBeaconProcData;
extern const GFL_PROC_DATA PalaceEditProcData;

//==============================================================================
//	データ
//==============================================================================
//メニューデータ
static const D_MENULIST DebugMenuList[] = {
#if 0
	{//WIFI広場
		DM_MSG_MENU014, 
		&WFLBY_PROC,
		_WifiLobbyParentWorkCreate,
		FS_OVERLAY_ID(wifilobby_common),
	},
#endif
	{//パレス編集
		DM_MSG_MENU013, 
		&PalaceEditProcData,			
		NULL,
		FS_OVERLAY_ID(palace)
	},
	{//相性チェック
		DM_MSG_MENU012, 
		&CompatiControlProcData,
		NULL,
		FS_OVERLAY_ID(compati_check)
	},
	{//巨大ビーコン送受信テスト
		DM_MSG_MENU010, 
		&DebugMatsudaBeaconProcData,	
		NULL,
		FS_OVERLAY_ID(matsuda_debug)
	},
	{//アイテム
		DM_MSG_MENU004, 
		&DebugMatsudaItemProcData,		
		NULL,
		FS_OVERLAY_ID(matsuda_debug)
	},
	{//赤外線複数マッチング
		DM_MSG_MENU003, 
		&DebugMatsudaIrcMatchProcData,	
		NULL,
		FS_OVERLAY_ID(matsuda_debug)
	},
	{//ワイヤレス通信テスト
		DM_MSG_MENU002, 
		&DebugMatsudaNetProcData,		
		NULL,
		FS_OVERLAY_ID(matsuda_debug)
	},
	{//セーブテスト
		DM_MSG_MENU001,
		&DebugMatsudaMainProcData,
		NULL,
		FS_OVERLAY_ID(matsuda_debug)
	},
	{//簡易ポケモン交換
		DM_MSG_MENU006, 
		&TradeMainProcData,			
		NULL,
		FS_OVERLAY_ID(trade)
	},
	{//セーブ破壊
		DM_MSG_MENU007,
		&DebugSaveProcData,			
		NULL,
		FS_OVERLAY_ID(matsuda_debug)
	},
	{//パレス
		DM_MSG_MENU008, 
		&PalaceBlockProcData,			
		NULL,
		FS_OVERLAY_ID(matsuda_debug)
	},
	{//パレス
		DM_MSG_MENU009, 
		&PalaceHandProcData,			
		NULL,
		FS_OVERLAY_ID(matsuda_debug)
	},
};



//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	static const GFL_DISP_VRAM vramBank = {
		GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_32K,	// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
	};

	D_MATSU_WORK* wk;
	
	DEBUG_PerformanceSetActive(FALSE);
	
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MATSUDA_DEBUG, 0x70000 );
	wk = GFL_PROC_AllocWork( proc, sizeof(D_MATSU_WORK), HEAPID_MATSUDA_DEBUG );
	MI_CpuClear8(wk, sizeof(D_MATSU_WORK));
	wk->heapID = HEAPID_MATSUDA_DEBUG;

	GFL_DISP_SetBank( &vramBank );

	//バックグラウンドの色を入れておく
	GFL_STD_MemFill16((void*)HW_BG_PLTT, 0x5ad6, 2);
	
	// 各種効果レジスタ初期化
	G2_BlendNone();

	// BGsystem初期化
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );
	GFL_FONTSYS_Init();

	//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}

	// 個別フレーム設定
	{
		static const GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );

//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}

	// 上下画面設定
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

	{//メッセージ描画の為の準備
		int i;

		wk->drawwin.win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 4, 0, 24, 20, 0, GFL_BMP_CHRAREA_GET_F );
		wk->drawwin.bmp = GFL_BMPWIN_GetBmp(wk->drawwin.win);
		GFL_BMP_Clear( wk->drawwin.bmp, 0xff );
		GFL_BMPWIN_MakeScreen( wk->drawwin.win );
		GFL_BMPWIN_TransVramCharacter( wk->drawwin.win );
		PRINT_UTIL_Setup( wk->drawwin.printUtil, wk->drawwin.win );

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		wk->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

//		PRINTSYS_Init( wk->heapID );
		wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

		wk->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_matsu_dat, wk->heapID );
		for(i = 0; i < D_STRBUF_NUM; i++){
			wk->strbuf[i] = GFL_STR_CreateBuffer(64, wk->heapID);
		}

		wk->tcbl = GFL_TCBL_Init( wk->heapID, wk->heapID, 4, 32 );

		GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI
	}

	//フォントパレット転送
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
		0, 0x20, HEAPID_MATSUDA_DEBUG);

	return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;
	BOOL ret = 0;
	int i;
	BOOL que_ret;
	
	GFL_TCBL_Main( wk->tcbl );
	que_ret = PRINTSYS_QUE_Main( wk->printQue );
	if( PRINT_UTIL_Trans( wk->drawwin.printUtil, wk->printQue ) == FALSE){
		//return GFL_PROC_RES_CONTINUE;
	}
	else{
		if(que_ret == TRUE && wk->drawwin.message_req == TRUE){
//			GFL_BMP_Clear( wk->bmp, 0xff );
			GFL_BMPWIN_TransVramCharacter( wk->drawwin.win );
			wk->drawwin.message_req = FALSE;
		}
	}
	
	switch(wk->debug_mode){
	case 0:		//赤外線マッチング
		ret = DebugMatsuda_ItemDebug(wk);
		if(ret == TRUE){
			wk->seq = 0;
			wk->debug_mode = 1;
		}
		break;
	case 1:		//ワイヤレス通信
	default:
		ret = TRUE;
	}
	
	if(ret == TRUE){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;
	int i;
	void *parent_work;
	
	if(DebugMenuList[wk->cursor_y].parent_work_func != NULL){
		wk->parent_work = DebugMenuList[wk->cursor_y].parent_work_func(wk);
		parent_work = wk->parent_work;
	}
	else{
		parent_work = NULL;
	}
	
	//次のPROC予約
	GFL_PROC_SysSetNextProc(
		DebugMenuList[wk->cursor_y].ov_id, DebugMenuList[wk->cursor_y].next_proc, parent_work);

	GFL_BMPWIN_Delete(wk->drawwin.win);
	for(i = 0; i < D_STRBUF_NUM; i++){
		GFL_STR_DeleteBuffer(wk->strbuf[i]);
	}
	
	PRINTSYS_QUE_Delete(wk->printQue);
	GFL_MSG_Delete(wk->mm);
	
	GFL_FONT_Delete(wk->fontHandle);
	GFL_TCBL_Exit(wk->tcbl);
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_MATSUDA_DEBUG);
	
	OS_TPrintf("リスト終了\n");
	return GFL_PROC_RES_FINISH;
}



//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   ワイヤレス通信テスト
 *
 * @param   wk		
 *
 * @retval  TRUE:終了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL DebugMatsuda_ItemDebug(D_MATSU_WORK *wk)
{
	BOOL ret, irc_ret = 0;
	int msg_id;
	int i, x;
	
	GF_ASSERT(wk);

	switch(wk->seq){
	case 0:		//リスト描画
		GFL_BMP_Clear( wk->drawwin.bmp, 0xff );
		wk->drawwin.message_req = TRUE;
		for(i = 0; i < NELEMS(DebugMenuList); i++){
			x = (wk->cursor_y == i) ? 2 : 0;
			GFL_MSG_GetString(wk->mm, DebugMenuList[i].str_id, wk->strbuf[i]);
			PRINTSYS_PrintQue(wk->printQue, wk->drawwin.bmp, x*8, i*2*8, 
				wk->strbuf[i], wk->fontHandle);
		}
		wk->drawwin.message_req = TRUE;

		wk->seq++;
		break;
	case 1:
		{
			int before_cursor;
			
			if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_A){
				wk->seq++;
				break;
			}

			before_cursor = wk->cursor_y;
			switch(GFL_UI_KEY_GetRepeat()){
			case PAD_KEY_UP:
				wk->cursor_y--;
				break;
			case PAD_KEY_DOWN:
				wk->cursor_y++;
				break;
			}
			
			if(before_cursor != wk->cursor_y){
				OS_TPrintf("wk->cursor_y = %d\n", wk->cursor_y);
				if(wk->cursor_y < 0){
					wk->cursor_y = NELEMS(DebugMenuList) - 1;
					OS_TPrintf("over down corsor_y = %d\n", wk->cursor_y);
				}
				if(wk->cursor_y >= NELEMS(DebugMenuList)){
					wk->cursor_y = 0;
					OS_TPrintf("over up corsor_y = %d\n", wk->cursor_y);
				}
				wk->seq = 0;
			}
		}
		break;
	default:
		return TRUE;	//ワイヤレス通信処理へ
	}
	
	return FALSE;
}



//==============================================================================
//	
//==============================================================================
static void * _BalloonParentWorkCreate(D_MATSU_WORK *wk)
{
#if 0
	BALLOON_PROC_WORK *balloon_pwk;
	
	balloon_pwk = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(BALLOON_PROC_WORK));
	balloon_pwk->debug_offline = TRUE;
	
	GFL_OVERLAY_Load(FS_OVERLAY_ID(netold));
	GFL_OVERLAY_Load(FS_OVERLAY_ID(minigame_common));
	
	return balloon_pwk;
#else
  return NULL;
#endif
}

//==============================================================================
//	
//==============================================================================
static void * _WifiLobbyParentWorkCreate(D_MATSU_WORK *wk)
{
  return NULL;
}


//==============================================================================
//	
//==============================================================================
//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA DebugMatsudaListProcData = {
	DebugMatsudaMainProcInit,
	DebugMatsudaMainProcMain,
	DebugMatsudaMainProcEnd,
};


