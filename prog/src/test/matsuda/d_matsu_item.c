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
#include "system\gfl_use.h"

#include "item/item.h"


//==============================================================================
//	定数定義
//==============================================================================
///アクター最大数
#define D_MATSU_ACT_MAX			(64)

///アイテムアイコンのパレット展開位置
#define D_MATSU_ICON_PALNO		(0)

///BMPウィンドウ
enum{
	D_WIN_NAME,				//アイテム名
	D_WIN_NAME_KANJI,		//アイテム名
	D_WIN_INFO,				//アイテム説明文
	D_WIN_INFO_KANJI,		//アイテム説明文
	
	D_WIN_MAX,
};

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
	STRBUF			*strbuf_name;
	STRBUF			*strbuf_name_kanji;
	STRBUF			*strbuf_info;
	STRBUF			*strbuf_info_kanji;
	GFL_TCBLSYS		*tcbl;
	DM_MSG_DRAW_WIN drawwin[D_WIN_MAX];
	
	//アクター
	GFL_CLUNIT *clunit;
	GFL_CLWK	*clwk_icon;		//アイテムアイコンアクター
	NNSG2dImagePaletteProxy	icon_pal_proxy;
	NNSG2dCellDataBank	*icon_cell_data;
	NNSG2dAnimBankData	*icon_anm_data;
	u32 icon_cell_id;
	u32 icon_pltt_id;
	u32 icon_cgr_id;
	
	void	*icon_cell_res;
	void	*icon_anm_res;
	
	int item_no;
	GFL_TCB *vintr_tcb;
}D_MATSU_WORK;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL DebugMatsuda_ItemDebug(D_MATSU_WORK *wk);
static void Local_MessagePut(D_MATSU_WORK *wk, int win_index, STRBUF *strbuf, int x, int y);
static void Local_ItemIconCommonDataSet(D_MATSU_WORK *wk);
static void Local_ItemIconCommonDataFree(D_MATSU_WORK *wk);
static void Local_ItemIconAdd(D_MATSU_WORK *wk, int item_no);
static void VintrTCB_VblankFunc(GFL_TCB *tcb, void *work);


//==============================================================================
//	データ
//==============================================================================
///ポケモンアイコンのCLWK初期データ
static const GFL_CLWK_DATA ItemIconClwkData = {
	16, 16, 					//座標(XY)
	0,						//アニメシーケンス
	10,						//ソフトプライオリティ
	0,						//BGプライオリティ
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

		wk->drawwin[D_WIN_NAME].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 4, 0, 20, 2, 0, GFL_BMP_CHRAREA_GET_F );
		wk->drawwin[D_WIN_NAME_KANJI].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 4, 12, 20, 2, 0, GFL_BMP_CHRAREA_GET_F );
		wk->drawwin[D_WIN_INFO].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 4, 3, 32, 6, 0, GFL_BMP_CHRAREA_GET_F );
		wk->drawwin[D_WIN_INFO_KANJI].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 4, 15, 32, 6, 0, GFL_BMP_CHRAREA_GET_F );
		for(i = 0; i < D_WIN_MAX; i++){
			wk->drawwin[i].bmp = GFL_BMPWIN_GetBmp(wk->drawwin[i].win);
			GFL_BMP_Clear( wk->drawwin[i].bmp, 0xff );
			GFL_BMPWIN_MakeScreen( wk->drawwin[i].win );
			GFL_BMPWIN_TransVramCharacter( wk->drawwin[i].win );
			PRINT_UTIL_Setup( wk->drawwin[i].printUtil, wk->drawwin[i].win );
		}

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		wk->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

//		PRINTSYS_Init( wk->heapID );
		wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

		wk->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_matsu_dat, wk->heapID );
		wk->strbuf_name = GFL_STR_CreateBuffer( 64, wk->heapID );
		wk->strbuf_name_kanji = GFL_STR_CreateBuffer( 64, wk->heapID );
		wk->strbuf_info = GFL_STR_CreateBuffer( 512, wk->heapID );
		wk->strbuf_info_kanji = GFL_STR_CreateBuffer( 512, wk->heapID );

		wk->tcbl = GFL_TCBL_Init( wk->heapID, wk->heapID, 4, 32 );

		GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI
	}

	//フォントパレット転送
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
		0, 0x20, HEAPID_MATSUDA_DEBUG);

	//アクター設定
	{
		GFL_CLSYS_INIT clsys_init = GFL_CLSYSINIT_DEF_DIVSCREEN;
		
		clsys_init.oamst_main = GFL_CLSYS_OAMMAN_INTERVAL;	//通信アイコンの分
		clsys_init.oamnum_main = 128-GFL_CLSYS_OAMMAN_INTERVAL;
		clsys_init.tr_cell = D_MATSU_ACT_MAX;
		GFL_CLACT_SYS_Create(&clsys_init, &vramBank, HEAPID_MATSUDA_DEBUG);
		
		wk->clunit = GFL_CLACT_UNIT_Create(D_MATSU_ACT_MAX, 0, HEAPID_MATSUDA_DEBUG);
		GFL_CLACT_UNIT_SetDefaultRend(wk->clunit);

		//アイテムアイコン共通データ登録
		Local_ItemIconCommonDataSet(wk);
		
		//OBJ表示ON
		GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
	}

	wk->vintr_tcb = GFUser_VIntr_CreateTCB(VintrTCB_VblankFunc, wk, 5);
	
	wk->item_no = 1;
	
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
	for(i = 0; i < D_WIN_MAX; i++){
		if( PRINT_UTIL_Trans( wk->drawwin[i].printUtil, wk->printQue ) == FALSE){
			//return GFL_PROC_RES_CONTINUE;
		}
		else{
			if(que_ret == TRUE && wk->drawwin[i].message_req == TRUE){
	//			GFL_BMP_Clear( wk->bmp, 0xff );
				GFL_BMPWIN_TransVramCharacter( wk->drawwin[i].win );
				wk->drawwin[i].message_req = FALSE;
			}
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
//		return GFL_PROC_RES_FINISH;
	}

	GFL_CLACT_SYS_Main();
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

	GFL_TCB_DeleteTask(wk->vintr_tcb);
	
	Local_ItemIconCommonDataFree(wk);

	GFL_CLACT_UNIT_Delete(wk->clunit);
	GFL_CLACT_SYS_Delete();

	GFL_FONT_Delete(wk->fontHandle);
	GFL_TCBL_Exit(wk->tcbl);
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_MATSUDA_DEBUG);
	
	return GFL_PROC_RES_FINISH;
}

static void VintrTCB_VblankFunc(GFL_TCB *tcb, void *work)
{
	GFL_CLACT_SYS_VBlankFunc();
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
	
	GF_ASSERT(wk);

	switch(wk->seq){
	case 0:		//アイテム名などの表示
		{
			GFL_MSGSYS_SetLangID( 0 );	//JPN(ひらがな)
			ITEM_GetItemName(wk->strbuf_name, wk->item_no, HEAPID_MATSUDA_DEBUG);
			ITEM_GetInfo(wk->strbuf_info, wk->item_no, HEAPID_MATSUDA_DEBUG);
			Local_MessagePut(wk, D_WIN_NAME, wk->strbuf_name, 0, 0);
			Local_MessagePut(wk, D_WIN_INFO, wk->strbuf_info, 0, 0);

			GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI
			ITEM_GetItemName(wk->strbuf_name_kanji, wk->item_no, HEAPID_MATSUDA_DEBUG);
			ITEM_GetInfo(wk->strbuf_info_kanji, wk->item_no, HEAPID_MATSUDA_DEBUG);
			Local_MessagePut(wk, D_WIN_NAME_KANJI, wk->strbuf_name_kanji, 0, 0);
			Local_MessagePut(wk, D_WIN_INFO_KANJI, wk->strbuf_info_kanji, 0, 0);

			//アイテムアイコンアクター
			Local_ItemIconAdd(wk, wk->item_no);
		}
		wk->seq++;
		break;
	case 1:
		{
			int before_item_no;
			
			before_item_no = wk->item_no;
			switch(GFL_UI_KEY_GetRepeat()){
			case PAD_KEY_LEFT:
				wk->item_no--;
				break;
			case PAD_BUTTON_L:
				wk->item_no -= 10;
				break;
			case PAD_BUTTON_R:
				wk->item_no += 10;
				break;
			case PAD_KEY_RIGHT:
				wk->item_no++;
				break;
			}
		
			if(before_item_no != wk->item_no){
				if(wk->item_no > ITEM_DATA_MAX){
					wk->item_no = 1;
				}
				if(wk->item_no < 1){
					wk->item_no = ITEM_DATA_MAX;
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

//--------------------------------------------------------------
/**
 * @brief   メッセージを表示する
 *
 * @param   wk			
 * @param   strbuf		表示するメッセージデータ
 * @param   x			X座標
 * @param   y			Y座標
 */
//--------------------------------------------------------------
static void Local_MessagePut(D_MATSU_WORK *wk, int win_index, STRBUF *strbuf, int x, int y)
{
	GFL_BMP_Clear( wk->drawwin[win_index].bmp, 0xff );
	PRINTSYS_PrintQue(wk->printQue, wk->drawwin[win_index].bmp, x, y, strbuf, wk->fontHandle);
	wk->drawwin[win_index].message_req = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   アイテムアイコンの共通データ登録
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void Local_ItemIconCommonDataSet(D_MATSU_WORK *wk)
{
	ARCHANDLE *hdl;

	hdl = GFL_ARC_OpenDataHandle(ITEM_GetIconArcID(), HEAPID_MATSUDA_DEBUG);
	
	wk->icon_cell_id = GFL_CLGRP_CELLANIM_Register(hdl, 
		ITEM_GetIconCell(), ITEM_GetIconCellAnm(), HEAPID_MATSUDA_DEBUG);

	GFL_ARC_CloseDataHandle(hdl);
}

//--------------------------------------------------------------
/**
 * @brief   アイテムアイコンの共通データ破棄
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void Local_ItemIconCommonDataFree(D_MATSU_WORK *wk)
{
	GFL_HEAP_FreeMemory(wk->icon_cell_res);
	GFL_HEAP_FreeMemory(wk->icon_anm_res);
}

//--------------------------------------------------------------
/**
 * @brief   アイテムアイコン登録
 *
 * @param   wk		
 * @param   monsno		ポケモン番号
 * @param   net_id		ネットID
 */
//--------------------------------------------------------------
static void Local_ItemIconAdd(D_MATSU_WORK *wk, int item_no)
{
	GFL_CLWK_DATA clwk_data;
//	GFL_CLWK_RES clwk_res;
	NNSG2dImageProxy imgProxy;
	u32 icon_index;
	u32 vram_offset = 0;	//byte単位
	ARCHANDLE *hdl;
	
	hdl = GFL_ARC_OpenDataHandle(ITEM_GetIconArcID(), HEAPID_MATSUDA_DEBUG);

	vram_offset = 0;
	
	OS_TPrintf("item_no = %d\n", item_no);
	
	if(wk->clwk_icon != NULL){
		GFL_CLACT_WK_Remove(wk->clwk_icon);
	}
	
	//パレット
#if 0
	wk->icon_pltt_id = GFL_CLGRP_PLTT_RegisterEx(
		hdl, ITEM_GetIndex(item_no, ITEM_GET_ICON_PAL), CLSYS_DRAW_MAIN, 
		D_MATSU_ICON_PALNO * 0x20, 0, 1, HEAPID_MATSUDA_DEBUG);
#else
	wk->icon_pltt_id = GFL_CLGRP_PLTT_RegisterComp(hdl, ITEM_GetIndex(item_no, ITEM_GET_ICON_PAL),
		CLSYS_DRAW_MAIN, D_MATSU_ICON_PALNO * 0x20, HEAPID_MATSUDA_DEBUG);
#endif

	//キャラクタ設定
	icon_index = ITEM_GetIndex(item_no, ITEM_GET_ICON_CGX);
	wk->icon_cgr_id = GFL_CLGRP_CGR_Register(
		hdl, icon_index, FALSE, CLSYS_DRAW_MAIN, HEAPID_MATSUDA_DEBUG);

	//アクター登録
	clwk_data = ItemIconClwkData;
	wk->clwk_icon = GFL_CLACT_WK_Create(wk->clunit, wk->icon_cgr_id, wk->icon_pltt_id, 
		wk->icon_cell_id, &clwk_data, CLWK_SETSF_NONE, HEAPID_MATSUDA_DEBUG);
	
	//アニメオート設定
	GFL_CLACT_WK_SetAutoAnmSpeed(wk->clwk_icon, FX32_ONE);
	GFL_CLACT_WK_SetAutoAnmFlag(wk->clwk_icon, TRUE);

	GFL_ARC_CloseDataHandle(hdl);
}







//==============================================================================
//	
//==============================================================================
//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA DebugMatsudaItemProcData = {
	DebugMatsudaMainProcInit,
	DebugMatsudaMainProcMain,
	DebugMatsudaMainProcEnd,
};


