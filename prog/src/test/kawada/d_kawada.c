//============================================================================================
/**
 * @file	  d_kawada.c
 * @brief		川田デバッグ処理
 * @author  Koji Kawada
 * @date	  2010.03.03
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/wipe.h"
#include "system/bmp_menulist.h"
#include "gamesystem/game_data.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "item/item.h"
#include "font/font.naix"

#include "app/box2.h"
#include "app/zukan.h"
#include "app/wifi_note.h"
#include "demo/command_demo.h"
#include "../../battle/app/vs_multi_list.h"

#include "arc_def.h"  //ARCID_MESSAGE
#include "message.naix"
#include "msg/msg_d_kawada.h"

#include "app/zukan_toroku.h"


// オーバーレイ
FS_EXTERN_OVERLAY(zukan_toroku);


//============================================================================================
//	定数定義
//============================================================================================
#define	TOP_MENU_SIZ	( 1 )

typedef struct {
	u32	main_seq;
	u32	next_seq;
	HEAPID	heapID;

	GFL_FONT * font;					// 通常フォント
	GFL_MSGDATA * mman;				// メッセージデータマネージャ
	PRINT_QUE * que;					// プリントキュー
	GFL_BMPWIN * win;					// BMPWIN
	PRINT_UTIL	util;

	BMP_MENULIST_DATA * ld;
	BMPMENULIST_WORK * lw;

	GAMEDATA * gamedata;

  GFL_PROCSYS*  local_procsys;

  ZUKAN_TOROKU_PARAM* zukan_toroku_param;
  POKEMON_PARAM*      pp;

}KAWADA_MAIN_WORK;

enum {
	MAIN_SEQ_INIT = 0,
	MAIN_SEQ_MAIN,
	MAIN_SEQ_FADE_MAIN,

  // ここから
	MAIN_SEQ_ZUKAN_TOROKU_CALL,  // top_menu00
  // ここまで

	MAIN_SEQ_ZUKAN_TOROKU_CALL_RETURN,
	
  MAIN_SEQ_END,
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT MainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT MainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT MainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

static void FadeInSet( KAWADA_MAIN_WORK * wk, u32 next );
static void FadeOutSet( KAWADA_MAIN_WORK * wk, u32 next );

static void BgInit( KAWADA_MAIN_WORK * wk );
static void BgExit(void);

static void TopMenuInit( KAWADA_MAIN_WORK * wk );
static void TopMenuExit( KAWADA_MAIN_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================
const GFL_PROC_DATA DebugKawadaMainProcData = {
  MainProcInit,
  MainProcMain,
  MainProcEnd,
};

static const BMPMENULIST_HEADER TopMenuListH = {
	NULL, NULL, NULL, NULL,
	TOP_MENU_SIZ,		// リスト項目数
	6,		// 表示最大項目数
	0,		// ラベル表示Ｘ座標
	12,		// 項目表示Ｘ座標
	0,		// カーソル表示Ｘ座標
	0,		// 表示Ｙ座標
	1,		// 表示文字色
	15,		// 表示背景色
	2,		// 表示文字影色
	0,		// 文字間隔Ｘ
	16,		// 文字間隔Ｙ
	BMPMENULIST_LRKEY_SKIP,		// ページスキップタイプ
	0,		// 文字指定(本来はu8だけどそんなに作らないと思うので)
	0,		// ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF)

	NULL,
	
	12,			// 文字サイズX(ドット
	16,			// 文字サイズY(ドット
	NULL,		// 表示に使用するメッセージバッファ
	NULL,		// 表示に使用するプリントユーティリティ
	NULL,		// 表示に使用するプリントキュー
	NULL,		// 表示に使用するフォントハンドル
};


static GFL_PROC_RESULT MainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	KAWADA_MAIN_WORK * wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_KAWADA_DEBUG, 0x20000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(KAWADA_MAIN_WORK), HEAPID_KAWADA_DEBUG );

	wk->heapID    = HEAPID_KAWADA_DEBUG;
	wk->gamedata	= GAMEDATA_Create( wk->heapID );

  wk->local_procsys = GFL_PROC_LOCAL_boot( wk->heapID );

	FadeInSet( wk, MAIN_SEQ_INIT );
	wk->main_seq  = MAIN_SEQ_FADE_MAIN;

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	KAWADA_MAIN_WORK * wk = mywk;

  GFL_PROC_MAIN_STATUS  local_proc_status   =  GFL_PROC_LOCAL_Main( wk->local_procsys );
  if( local_proc_status == GFL_PROC_MAIN_VALID ) return GFL_PROC_RES_CONTINUE;

	switch( wk->main_seq ){
	case MAIN_SEQ_INIT:
		BgInit( wk );
		TopMenuInit( wk );
		wk->main_seq = MAIN_SEQ_MAIN;
		break;

	case MAIN_SEQ_MAIN:
		{
			u32 ret = BmpMenuList_Main( wk->lw );

			switch( ret ){
			case BMPMENULIST_NULL:
			case BMPMENULIST_LABEL:
				break;

			case BMPMENULIST_CANCEL:		// キャンセル
				TopMenuExit( wk );
				BgExit();
				wk->main_seq = MAIN_SEQ_END;
				break;

			default:
				TopMenuExit( wk );
				BgExit();
				FadeOutSet( wk, ret );
				wk->main_seq = MAIN_SEQ_FADE_MAIN;
			}
		}
		break;

	case MAIN_SEQ_FADE_MAIN:
		if( WIPE_SYS_EndCheck() == TRUE ){
			wk->main_seq = wk->next_seq;
		}
		break;

	case MAIN_SEQ_END:
		OS_Printf( "kawadaデバッグ処理終了しました\n" );
	  return GFL_PROC_RES_FINISH;




	case MAIN_SEQ_ZUKAN_TOROKU_CALL:
    GFL_OVERLAY_Load(FS_OVERLAY_ID(zukan_toroku));
    wk->pp = PP_Create( 1, 1, 0, wk->heapID );
    wk->zukan_toroku_param = ZUKAN_TOROKU_AllocParam(
        wk->heapID,
        ZUKAN_TOROKU_LAUNCH_TOROKU,
        wk->pp,
        NULL,
        NULL,
        0 );
    GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &ZUKAN_TOROKU_ProcData, wk->zukan_toroku_param );
		wk->main_seq = MAIN_SEQ_ZUKAN_TOROKU_CALL_RETURN;
		break;



  case MAIN_SEQ_ZUKAN_TOROKU_CALL_RETURN:
    GFL_HEAP_FreeMemory( wk->pp );
    ZUKAN_TOROKU_FreeParam( wk->zukan_toroku_param );
    GFL_OVERLAY_Unload(FS_OVERLAY_ID(zukan_toroku));
		FadeInSet( wk, MAIN_SEQ_INIT );
		wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;

	}

  return GFL_PROC_RES_CONTINUE;
}

static GFL_PROC_RESULT MainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	KAWADA_MAIN_WORK * wk = mywk;
  
  GFL_PROC_LOCAL_Exit( wk->local_procsys ); 

	GAMEDATA_Delete( wk->gamedata );

	GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_KAWADA_DEBUG );

  return GFL_PROC_RES_FINISH;
}

static void BgInit( KAWADA_MAIN_WORK * wk )
{
	GFL_BG_Init( wk->heapID );
	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}
	{	// メイン画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0, wk->heapID );
	}
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0, 0x20, wk->heapID );
}

static void BgExit(void)
{
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
	GFL_BG_Exit();
}

// フェードイン
static void FadeInSet( KAWADA_MAIN_WORK * wk, u32 next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );

	wk->next_seq = next;
}

// フェードアウト
static void FadeOutSet( KAWADA_MAIN_WORK * wk, u32 next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );

	wk->next_seq = next;
}

static void TopMenuInit( KAWADA_MAIN_WORK * wk )
{
	BMPMENULIST_HEADER	lh;
	u32	i;

	GFL_BMPWIN_Init( wk->heapID );

	wk->mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_kawada_dat, wk->heapID );
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	wk->que  = PRINTSYS_QUE_Create( wk->heapID );
	wk->win  = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 1, 1, 16, 12, 0, GFL_BMP_CHRAREA_GET_B );

	wk->ld = BmpMenuWork_ListCreate( TOP_MENU_SIZ, wk->heapID );
	for( i=0; i<TOP_MENU_SIZ; i++ ){
		STRBUF * str = GFL_MSG_CreateString( wk->mman, top_menu00+i );
		BmpMenuWork_ListAddString( &wk->ld[i], str, MAIN_SEQ_ZUKAN_TOROKU_CALL+i, wk->heapID );
		GFL_STR_DeleteBuffer( str );
	}

	lh = TopMenuListH;
	lh.list = wk->ld;
	lh.win  = wk->win;
	lh.msgdata = wk->mman;			//表示に使用するメッセージバッファ
	lh.print_util = &wk->util;	//表示に使用するプリントユーティリティ
	lh.print_que  = wk->que;		//表示に使用するプリントキュー
	lh.font_handle = wk->font;	//表示に使用するフォントハンドル

	wk->lw = BmpMenuList_Set( &lh, 0, 0, wk->heapID );
	BmpMenuList_SetCursorBmp( wk->lw, wk->heapID );

	GFL_BMPWIN_TransVramCharacter( wk->win );
	GFL_BMPWIN_MakeScreen( wk->win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->win) );
}

static void TopMenuExit( KAWADA_MAIN_WORK * wk )
{
	BmpMenuList_Exit( wk->lw, NULL, NULL );
	BmpMenuWork_ListDelete( wk->ld );

	GFL_BMPWIN_Delete( wk->win );
	PRINTSYS_QUE_Delete( wk->que );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );

	GFL_BMPWIN_Exit();
}

