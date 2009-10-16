//==============================================================================
/**
 * @file	d_save.c
 * @brief	セーブ関連のデバッグ機能
 * @author	matsuda
 * @date	2009.01.09(金)
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

#include "system/bmp_menu.h"
#include "system/bmp_menuwork.h"
#include "system/bmp_menulist.h"
#include "print/wordset.h"


//==============================================================================
//	定数定義
//==============================================================================
#define D_SAVE_CRC_FOOTER_CLEAR	(3)		///CRC + FOOTER + clear

enum{
	WIN_MENU,
	WIN_INFO,
	
	WIN_MAX,
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
	GFL_FONT		*fontHandle;
	PRINT_QUE		*printQue;
	PRINT_STREAM	*printStream;
	GFL_MSGDATA		*mm;
	GFL_TCBLSYS		*tcbl;
	DM_MSG_DRAW_WIN drawwin[2];
	
	WORDSET			*wordset;
	BMPMENULIST_HEADER	bmphead;
	BMPMENULIST_WORK *bmpmenu_list;
	BMP_MENULIST_DATA *list_data;
	STRBUF 			*info_str;
}D_MATSU_WORK;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void DebugMenuHeadCreate(D_MATSU_WORK *wk, BMPMENULIST_HEADER *bmphead);
static void DebugMenuListCreate(D_MATSU_WORK *wk);
static void DebugMenuListDelete(D_MATSU_WORK *wk);
static void Local_MessagePut(D_MATSU_WORK *wk, int win_index, STRBUF *strbuf, int x, int y);


//==============================================================================
//	外部データ
//==============================================================================
extern const GFL_SVLD_PARAM SaveParam_Normal;


//==============================================================================
//	データ
//==============================================================================
///BMPメニューヘッダ
static const BMPMENULIST_HEADER DebugSaveBmpMenuHead = {
	NULL,						//const BMP_MENULIST_DATA *list;		//表示文字データポインタ
	NULL,						//void	*call_back;	//カーソル移動ごとのコールバック関数
	NULL,						//void	*icon;		//一列表示ごとのコールバック関数
	NULL,						//GFL_BMPWIN *win;
	1,							//u16		count;		//リスト項目数
	12,							//u16		line;		//表示最大項目数
	0,							//u8		rabel_x;	//ラベル表示Ｘ座標
	11, 						//u8		data_x;		//項目表示Ｘ座標
	0,							//u8		cursor_x;	//カーソル表示Ｘ座標
	0,							//u8		line_y:4;	//表示Ｙ座標
	1,							//u8		f_col:4;	//表示文字色
	15, 						//u8		b_col:4;	//表示背景色
	2,							//u8		s_col:4;	//表示文字影色
	0, 							//u16		msg_spc:3;	//文字間隔Ｘ
	0, 							//u16		line_spc:4;	//文字間隔Ｙ
	BMPMENULIST_LRKEY_SKIP, 	//u16		page_skip:2;//ページスキップタイプ
	0,							//u16		font:6;		//文字指定
	0,							//u16		c_disp_f:1;	//ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF)
	NULL,						//void * work;
	12, 						//u16 font_size_x;		//文字サイズX(ドット
	16, 						//u16 font_size_y;		//文字サイズY(ドット
	NULL,						//GFL_MSGDATA *msgdata;	//表示に使用するメッセージバッファ
	NULL,						//PRINT_UTIL *print_util; //表示に使用するプリントユーティリティ
	NULL,						//PRINT_QUE *print_que;	//表示に使用するプリントキュー
	NULL,						//GFL_FONT *font_handle;	//表示に使用するフォントハンドル
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
static GFL_PROC_RESULT DebugSaveProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
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
	
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MATSUDA_DEBUG, 0x70000 );
	wk = GFL_PROC_AllocWork( proc, sizeof(D_MATSU_WORK), HEAPID_MATSUDA_DEBUG );
	MI_CpuClear8(wk, sizeof(D_MATSU_WORK));

	GFL_DISP_SetBank( &vramBank );

	//バックグラウンドの色を入れておく
	GFL_STD_MemFill16((void*)HW_BG_PLTT, 0x5ad6, 2);
	
	// 各種効果レジスタ初期化
	G2_BlendNone();

	// BGsystem初期化
	GFL_BG_Init( HEAPID_MATSUDA_DEBUG );
	GFL_BMPWIN_Init( HEAPID_MATSUDA_DEBUG );
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

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, HEAPID_MATSUDA_DEBUG );
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

		wk->drawwin[WIN_MENU].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 3, 0, 14, 24, 0, GFL_BMP_CHRAREA_GET_F );
		wk->drawwin[WIN_INFO].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 18, 0, 10, 20, 0, GFL_BMP_CHRAREA_GET_F );
		for(i = 0; i < WIN_MAX; i++){
			wk->drawwin[i].bmp = GFL_BMPWIN_GetBmp(wk->drawwin[i].win);
			GFL_BMP_Clear( wk->drawwin[i].bmp, 0xff );
			GFL_BMPWIN_MakeScreen( wk->drawwin[i].win );
			GFL_BMPWIN_TransVramCharacter( wk->drawwin[i].win );
			PRINT_UTIL_Setup( wk->drawwin[i].printUtil, wk->drawwin[i].win );
		}

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		wk->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_MATSUDA_DEBUG );

//		PRINTSYS_Init( HEAPID_MATSUDA_DEBUG );
		wk->printQue = PRINTSYS_QUE_Create( HEAPID_MATSUDA_DEBUG );

		wk->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_matsu_dat, HEAPID_MATSUDA_DEBUG );

		wk->wordset = WORDSET_Create(HEAPID_MATSUDA_DEBUG);

		wk->tcbl = GFL_TCBL_Init( HEAPID_MATSUDA_DEBUG, HEAPID_MATSUDA_DEBUG, 4, 32 );

		GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI
	}

	//フォントパレット転送
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
		0, 0x20, HEAPID_MATSUDA_DEBUG);

	//BMPメニュー作成
	DebugMenuListCreate(wk);
	DebugMenuHeadCreate(wk, &wk->bmphead);
	wk->bmpmenu_list = BmpMenuList_Set(&wk->bmphead, 0, 0, HEAPID_MATSUDA_DEBUG);
	BmpMenuList_SetCursorBmp(wk->bmpmenu_list, HEAPID_MATSUDA_DEBUG);

	//説明文表示
	wk->info_str = GFL_MSG_CreateString(wk->mm, DM_MSG_SAVE008);
	Local_MessagePut(wk, WIN_INFO, wk->info_str, 0,0);

	return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSaveProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;
	BOOL ret = 0;
	int i;
	BOOL que_ret;
	u32 bmp_ret;
	u32 write_data = 0x94a1;
	int save_a, save_b;
	
	GFL_TCBL_Main( wk->tcbl );
#if 1
	que_ret = PRINTSYS_QUE_Main( wk->printQue );
	for(i = 0; i < WIN_MAX; i++){
		if( PRINT_UTIL_Trans( wk->drawwin[i].printUtil, wk->printQue ) == FALSE){
			//return GFL_PROC_RES_CONTINUE;
		}
		else{
			if(1){//que_ret == TRUE && wk->drawwin[i].message_req == TRUE){
	//			GFL_BMP_Clear( wk->bmp, 0xff );
				GFL_BMPWIN_TransVramCharacter( wk->drawwin[i].win );
				wk->drawwin[i].message_req = FALSE;
			}
		}
	}
#endif

	if(GFL_UI_KEY_GetCont() & PAD_BUTTON_R){
		save_a = FALSE;
		save_b = TRUE;
	}
	else{
		save_a = TRUE;
		save_b = FALSE;
	}
	
	bmp_ret = BmpMenuList_Main(wk->bmpmenu_list);
	switch(bmp_ret){
	case BMPMENU_NULL:
		break;
	case BMPMENU_CANCEL:
		return GFL_PROC_RES_FINISH;
	default:
		if(SaveParam_Normal.table_max > bmp_ret){
			DEBUG_BACKUP_DataWrite(
				DEBUG_SaveData_PtrGet(), bmp_ret, &write_data, 0, sizeof(write_data),
				save_a, save_b, FALSE, FALSE);
		}
		else if(bmp_ret == SaveParam_Normal.table_max){
			//CRC
			DEBUG_BACKUP_DataWrite(
				DEBUG_SaveData_PtrGet(), bmp_ret, &write_data, 0, sizeof(write_data),
				save_a, save_b, TRUE, FALSE);
		}
		else if(bmp_ret == SaveParam_Normal.table_max + 1){
			//FOOTER
			DEBUG_BACKUP_DataWrite(
				DEBUG_SaveData_PtrGet(), bmp_ret, &write_data, 0, sizeof(write_data),
				save_a, save_b, FALSE, TRUE);
		}
		else{
			//セーブクリア
			OS_TPrintf("セーブを消去しています\n");
			SaveControl_Erase(SaveControl_GetPointer());
			return GFL_PROC_RES_FINISH;
		}
		break;
	}
	
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSaveProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;
	int i;
	
	OS_ResetSystem(0);	//セーブ読み込み状況を更新する為、ソフトリセットする
	
	
	
	BmpMenuList_Exit(wk->bmpmenu_list, NULL, NULL);
	
	for(i = 0; i < WIN_MAX; i++){
		GFL_BMPWIN_Delete(wk->drawwin[i].win);
	}
	
	GFL_STR_DeleteBuffer(wk->info_str);
	
	PRINTSYS_QUE_Delete(wk->printQue);
	GFL_MSG_Delete(wk->mm);
	WORDSET_Delete(wk->wordset);
	
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
 * @brief   BMPメニューヘッダを作成
 *
 * @param   wk			
 * @param   bmphead		
 *
 * 先にDebugMenuListCreateでメニューリストを作成しておく必要があります
 */
//--------------------------------------------------------------
static void DebugMenuHeadCreate(D_MATSU_WORK *wk, BMPMENULIST_HEADER *bmphead)
{
	GF_ASSERT(wk->list_data != NULL);
	
	*bmphead = DebugSaveBmpMenuHead;
	bmphead->list = wk->list_data;
	bmphead->win = wk->drawwin[WIN_MENU].win;
	bmphead->count = SaveParam_Normal.table_max + D_SAVE_CRC_FOOTER_CLEAR;
	bmphead->msgdata = wk->mm;
	bmphead->print_util = wk->drawwin[WIN_MENU].printUtil;
	bmphead->print_que = wk->printQue;
	bmphead->font_handle = wk->fontHandle;
}

static u32 SaveErrorMsgIDGet(BOOL ret_a, BOOL ret_b)
{
	u32 status_id;
	
	if(ret_a == TRUE && ret_b == TRUE){
		status_id = DM_MSG_SAVE005;
	}
	else if(ret_a == TRUE){
		status_id = DM_MSG_SAVE003;
	}
	else if(ret_b == TRUE){
		status_id = DM_MSG_SAVE004;
	}
	else{
		status_id = DM_MSG_SAVE002;
	}
	
	return status_id;
}

//--------------------------------------------------------------
/**
 * @brief   メニューリストを作成する
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void DebugMenuListCreate(D_MATSU_WORK *wk)
{
	BMP_MENULIST_DATA *list_data;
	int save_tbl_max, i;
	STRBUF *tempbuf, *status, *destsrc;
	BOOL ret_a, ret_b;
	
	save_tbl_max = SaveParam_Normal.table_max + D_SAVE_CRC_FOOTER_CLEAR;
	list_data = BmpMenuWork_ListCreate(save_tbl_max, HEAPID_MATSUDA_DEBUG);
	
	tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_SAVE001);

	for(i = 0; i < SaveParam_Normal.table_max; i++){
		list_data[i].param = i;
		destsrc = GFL_STR_CreateBuffer(64, HEAPID_MATSUDA_DEBUG);
		WORDSET_RegisterNumber(wk->wordset, 0, i, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		DEBUG_GFL_BACKUP_BlockSaveFlagGet(
			DEBUG_SaveData_PtrGet(), i, FALSE, FALSE, &ret_a, &ret_b);
		status = GFL_MSG_CreateString(wk->mm, SaveErrorMsgIDGet(ret_a, ret_b));
		WORDSET_RegisterWord(wk->wordset, 1, status, 0, TRUE, PM_LANG);
		GFL_STR_DeleteBuffer(status);
		
		WORDSET_ExpandStr(wk->wordset, destsrc, tempbuf);
		list_data[i].str = destsrc;
	}
	GFL_STR_DeleteBuffer(tempbuf);

	//CRC
	destsrc = GFL_STR_CreateBuffer(64, HEAPID_MATSUDA_DEBUG);
	tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_SAVE006);
	DEBUG_GFL_BACKUP_BlockSaveFlagGet(
		DEBUG_SaveData_PtrGet(), 0, TRUE, FALSE, &ret_a, &ret_b);
	status = GFL_MSG_CreateString(wk->mm, SaveErrorMsgIDGet(ret_a, ret_b));
	WORDSET_RegisterWord(wk->wordset, 1, status, 0, TRUE, PM_LANG);
	WORDSET_ExpandStr(wk->wordset, destsrc, tempbuf);
	list_data[SaveParam_Normal.table_max + 0].str = destsrc;
	list_data[SaveParam_Normal.table_max + 0].param = SaveParam_Normal.table_max + 0;
	GFL_STR_DeleteBuffer(status);
	GFL_STR_DeleteBuffer(tempbuf);

	//FOOTER
	destsrc = GFL_STR_CreateBuffer(64, HEAPID_MATSUDA_DEBUG);
	tempbuf = GFL_MSG_CreateString(wk->mm, DM_MSG_SAVE007);
	DEBUG_GFL_BACKUP_BlockSaveFlagGet(
		DEBUG_SaveData_PtrGet(), 0, FALSE, TRUE, &ret_a, &ret_b);
	status = GFL_MSG_CreateString(wk->mm, SaveErrorMsgIDGet(ret_a, ret_b));
	WORDSET_RegisterWord(wk->wordset, 1, status, 0, TRUE, PM_LANG);
	WORDSET_ExpandStr(wk->wordset, destsrc, tempbuf);
	list_data[SaveParam_Normal.table_max + 1].str = destsrc;
	list_data[SaveParam_Normal.table_max + 1].param = SaveParam_Normal.table_max + 1;
	GFL_STR_DeleteBuffer(status);
	GFL_STR_DeleteBuffer(tempbuf);

	//セーブクリア
	list_data[SaveParam_Normal.table_max + 2].str = GFL_MSG_CreateString(wk->mm, DM_MSG_SAVE000);
	list_data[SaveParam_Normal.table_max + 2].param = SaveParam_Normal.table_max + 2;
	
	wk->list_data = list_data;
}

//--------------------------------------------------------------
/**
 * @brief   メニューリストを削除する
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void DebugMenuListDelete(D_MATSU_WORK *wk)
{
	int save_tbl_max, i;
	
	save_tbl_max = SaveParam_Normal.table_max + D_SAVE_CRC_FOOTER_CLEAR;
	for(i = 0; i < save_tbl_max; i++){
		GFL_STR_DeleteBuffer((STRBUF*)(wk->list_data[i].str));
	}
	BmpMenuWork_ListDelete(wk->list_data);
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



//==============================================================================
//	
//==============================================================================
//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA DebugSaveProcData = {
	DebugSaveProcInit,
	DebugSaveProcMain,
	DebugSaveProcEnd,
};


