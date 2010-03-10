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

// 図鑑登録
#include "app/zukan_toroku.h"

// トライアルハウス結果
#include "pm_version.h"
#include "app/th_award.h"
#include "savedata/trialhouse_save.h"
#include "../../savedata/trialhouse_save_local.h"

// 地方図鑑賞状
#include "savedata/mystatus.h"
#include "app/chihou_zukan_award.h"

// 全国図鑑賞状
#include "app/zenkoku_zukan_award.h"

// 通信対戦後の録画選択画面
#include "net_app/btl_rec_sel.h"

// 二択簡易会話
// 一択簡易会話
#include "app/pms_input.h"


// オーバーレイ
FS_EXTERN_OVERLAY(zukan_toroku);
FS_EXTERN_OVERLAY(th_award);
FS_EXTERN_OVERLAY(chihou_zukan_award);
FS_EXTERN_OVERLAY(zenkoku_zukan_award);
FS_EXTERN_OVERLAY(btl_rec_sel);
FS_EXTERN_OVERLAY(pmsinput);


//============================================================================================
//	定数定義
//============================================================================================
#define	TOP_MENU_SIZ	( 7 )

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

  // 図鑑登録
  ZUKAN_TOROKU_PARAM* zukan_toroku_param;
  POKEMON_PARAM*      pp;

  // トライアルハウス結果
  THSV_WORK*          thsv;
  TH_AWARD_PARAM*     th_award_param;

  // 地方図鑑賞状
  MYSTATUS*                    mystatus;
  CHIHOU_ZUKAN_AWARD_PARAM*    chihou_zukan_award_param;
  // 全国図鑑賞状
  ZENKOKU_ZUKAN_AWARD_PARAM*   zenkoku_zukan_award_param;

  // 通信対戦後の録画選択画面
  BTL_REC_SEL_PARAM*     btl_rec_sel_param;

  // 二択簡易会話
  // 一択簡易会話
  PMSI_PARAM*    pmsi_param;

}KAWADA_MAIN_WORK;

enum {
	MAIN_SEQ_INIT = 0,
	MAIN_SEQ_MAIN,
	MAIN_SEQ_FADE_MAIN,

  // ここから
	MAIN_SEQ_ZUKAN_TOROKU_CALL,  // top_menu00
	MAIN_SEQ_TH_AWARD_CALL,
	MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL,
	MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL,
	MAIN_SEQ_BTL_REC_SEL_CALL,
	MAIN_SEQ_PMS_INPUT_DOUBLE_CALL,
	MAIN_SEQ_PMS_INPUT_SINGLE_CALL,
  // ここまで

	MAIN_SEQ_ZUKAN_TOROKU_CALL_RETURN,
	MAIN_SEQ_TH_AWARD_CALL_RETURN,
	MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL_RETURN,
	MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL_RETURN,
	MAIN_SEQ_BTL_REC_SEL_CALL_RETURN,
	MAIN_SEQ_PMS_INPUT_DOUBLE_CALL_RETURN,
	MAIN_SEQ_PMS_INPUT_SINGLE_CALL_RETURN,
	
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


// 図鑑登録
static void ZukanTorokuInit( KAWADA_MAIN_WORK* wk );
static void ZukanTorokuExit( KAWADA_MAIN_WORK* wk );

// トライアルハウス結果
static void ThAwardInit( KAWADA_MAIN_WORK* wk );
static void ThAwardExit( KAWADA_MAIN_WORK* wk );

// 地方図鑑賞状
static void ChihouZukanAwardInit( KAWADA_MAIN_WORK* wk );
static void ChihouZukanAwardExit( KAWADA_MAIN_WORK* wk );

// 全国図鑑賞状
static void ZenkokuZukanAwardInit( KAWADA_MAIN_WORK* wk );
static void ZenkokuZukanAwardExit( KAWADA_MAIN_WORK* wk );

// 通信対戦後の録画選択画面
static void BtlRecSelInit( KAWADA_MAIN_WORK* wk );
static void BtlRecSelExit( KAWADA_MAIN_WORK* wk );

// 二択簡易会話
static void PmsInputDoubleInit( KAWADA_MAIN_WORK* wk );
static void PmsInputDoubleExit( KAWADA_MAIN_WORK* wk );

// 一択簡易会話
static void PmsInputSingleInit( KAWADA_MAIN_WORK* wk );
static void PmsInputSingleExit( KAWADA_MAIN_WORK* wk );


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

#if 0
  // フェードインありでスタート
	FadeInSet( wk, MAIN_SEQ_INIT );
	wk->main_seq  = MAIN_SEQ_FADE_MAIN;
#else
  // フェードインなしでスタート
  wk->main_seq  = MAIN_SEQ_INIT;
#endif

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




  // 図鑑登録
	case MAIN_SEQ_ZUKAN_TOROKU_CALL:
    ZukanTorokuInit(wk);
		wk->main_seq = MAIN_SEQ_ZUKAN_TOROKU_CALL_RETURN;
		break;
  case MAIN_SEQ_ZUKAN_TOROKU_CALL_RETURN:
    ZukanTorokuExit(wk);
		FadeInSet( wk, MAIN_SEQ_INIT );
		wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // トライアルハウス結果
  case MAIN_SEQ_TH_AWARD_CALL:
    ThAwardInit(wk); 
		wk->main_seq = MAIN_SEQ_TH_AWARD_CALL_RETURN;
    break;
  case MAIN_SEQ_TH_AWARD_CALL_RETURN:
    ThAwardExit(wk); 
		FadeInSet( wk, MAIN_SEQ_INIT );
		wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 地方図鑑賞状
  case MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL:
    ChihouZukanAwardInit(wk);
		wk->main_seq = MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL_RETURN;
    break;
  case MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL_RETURN:
    ChihouZukanAwardExit(wk); 
		FadeInSet( wk, MAIN_SEQ_INIT );
		wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 全国図鑑賞状
  case MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL:
    ZenkokuZukanAwardInit(wk);
		wk->main_seq = MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL_RETURN;
    break;
  case MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL_RETURN:
    ZenkokuZukanAwardExit(wk); 
		FadeInSet( wk, MAIN_SEQ_INIT );
		wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 通信対戦後の録画選択画面
  case MAIN_SEQ_BTL_REC_SEL_CALL:
    BtlRecSelInit(wk);
		wk->main_seq = MAIN_SEQ_BTL_REC_SEL_CALL_RETURN;
    break;
  case MAIN_SEQ_BTL_REC_SEL_CALL_RETURN:
    BtlRecSelExit(wk); 
		FadeInSet( wk, MAIN_SEQ_INIT );
		wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 二択簡易会話
  case MAIN_SEQ_PMS_INPUT_DOUBLE_CALL:
    PmsInputDoubleInit(wk);
		wk->main_seq = MAIN_SEQ_PMS_INPUT_DOUBLE_CALL_RETURN;
    break;
  case MAIN_SEQ_PMS_INPUT_DOUBLE_CALL_RETURN:
    PmsInputDoubleExit(wk); 
		FadeInSet( wk, MAIN_SEQ_INIT );
		wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 一択簡易会話
  case MAIN_SEQ_PMS_INPUT_SINGLE_CALL:
    PmsInputSingleInit(wk);
		wk->main_seq = MAIN_SEQ_PMS_INPUT_SINGLE_CALL_RETURN;
    break;
  case MAIN_SEQ_PMS_INPUT_SINGLE_CALL_RETURN:
    PmsInputSingleExit(wk); 
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

// 図鑑登録
static void ZukanTorokuInit( KAWADA_MAIN_WORK* wk )
{
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
}
static void ZukanTorokuExit( KAWADA_MAIN_WORK* wk )
{
    GFL_HEAP_FreeMemory( wk->pp );
    ZUKAN_TOROKU_FreeParam( wk->zukan_toroku_param );
    GFL_OVERLAY_Unload(FS_OVERLAY_ID(zukan_toroku));
}

// トライアルハウス結果
static void ThAwardInit( KAWADA_MAIN_WORK* wk )
{
      u8 sex = PM_MALE;
      u8 type = 0;  // 0=ローカルシングル; 1=ローカルダブル; 2=ダウンロードシングル; 3=ダウンロードダブル;
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ) sex = PM_FEMALE;
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ) type = 1;
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ) type = 2;
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ) type = 3;

      GFL_OVERLAY_Load(FS_OVERLAY_ID(th_award));
      wk->thsv = GFL_HEAP_AllocMemory( wk->heapID, sizeof(THSV_WORK) );
      {
        u8 i;
#if 0 
        u16 name[17] = L"あいうえおかきくけこさしすせそた";
        name[16] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode
#else
        u16 name[17] = L"あいうえおかきく";
        name[8] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode
#endif

        wk->thsv->CommonData[0].Valid              = 1;
        wk->thsv->CommonData[0].IsDouble           = 0;
        wk->thsv->CommonData[0].Point              = 6000;
        wk->thsv->CommonData[0].MonsData[0].MonsNo = 1;
        wk->thsv->CommonData[0].MonsData[0].FormNo = 0;
        wk->thsv->CommonData[0].MonsData[0].Sex    = 0;
        wk->thsv->CommonData[0].MonsData[1].MonsNo = 2;
        wk->thsv->CommonData[0].MonsData[1].FormNo = 0;
        wk->thsv->CommonData[0].MonsData[1].Sex    = 0;
        wk->thsv->CommonData[0].MonsData[2].MonsNo = 3;
        wk->thsv->CommonData[0].MonsData[2].FormNo = 0;
        wk->thsv->CommonData[0].MonsData[2].Sex    = 0;
        wk->thsv->CommonData[0].MonsData[3].MonsNo = 6;
        wk->thsv->CommonData[0].MonsData[3].FormNo = 0;
        wk->thsv->CommonData[0].MonsData[3].Sex    = 0;

        wk->thsv->CommonData[1].Valid              = 1;
        wk->thsv->CommonData[1].IsDouble           = 0;
        wk->thsv->CommonData[1].Point              = 0xFFFF;
        wk->thsv->CommonData[1].MonsData[0].MonsNo = 1;
        wk->thsv->CommonData[1].MonsData[0].FormNo = 0;
        wk->thsv->CommonData[1].MonsData[0].Sex    = 0;
        wk->thsv->CommonData[1].MonsData[1].MonsNo = 2;
        wk->thsv->CommonData[1].MonsData[1].FormNo = 0;
        wk->thsv->CommonData[1].MonsData[1].Sex    = 0;
        wk->thsv->CommonData[1].MonsData[2].MonsNo = 3;
        wk->thsv->CommonData[1].MonsData[2].FormNo = 0;
        wk->thsv->CommonData[1].MonsData[2].Sex    = 0;
        wk->thsv->CommonData[1].MonsData[3].MonsNo = 6;
        wk->thsv->CommonData[1].MonsData[3].FormNo = 0;
        wk->thsv->CommonData[1].MonsData[3].Sex    = 0;

        for( i=0; i<16; i++ ) wk->thsv->DownloadBits[i] = 0;
        GFL_STD_MemCopy( name, wk->thsv->Name, 34 );

        switch(type)
        {
        case 0:
          {
          }
          break;
        case 1:
          {
            wk->thsv->CommonData[0].IsDouble           = 1;
          }
          break;
        case 2:
          {
          }
          break;
        case 3:
          {
            wk->thsv->CommonData[1].IsDouble           = 1;
          }
          break;
        }
      }
      wk->th_award_param = TH_AWARD_AllocParam( wk->heapID, sex, wk->thsv, (type==2||type==3)?TRUE:FALSE );
      GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &TH_AWARD_ProcData, wk->th_award_param );
}
static void ThAwardExit( KAWADA_MAIN_WORK* wk )
{
      TH_AWARD_FreeParam( wk->th_award_param );
      GFL_HEAP_FreeMemory( wk->thsv );
      GFL_OVERLAY_Unload(FS_OVERLAY_ID(th_award));
}

// 地方図鑑賞状
static void ChihouZukanAwardInit( KAWADA_MAIN_WORK* wk )
{
  u8 i;
  u16 name[6] = L"ゴモジノコ";
  name[5] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode

  GFL_OVERLAY_Load(FS_OVERLAY_ID(chihou_zukan_award));
  
  wk->mystatus = GFL_HEAP_AllocMemory( wk->heapID, MYSTATUS_SAVE_SIZE );
  
  for( i=0; i<6; i++ )
  {
    wk->mystatus->name[i] = name[i];
  }
  wk->mystatus->sex = PM_MALE;

  wk->chihou_zukan_award_param = CHIHOU_ZUKAN_AWARD_AllocParam( wk->heapID, wk->mystatus );
  
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &CHIHOU_ZUKAN_AWARD_ProcData, wk->chihou_zukan_award_param );
}
static void ChihouZukanAwardExit( KAWADA_MAIN_WORK* wk )
{
  CHIHOU_ZUKAN_AWARD_FreeParam( wk->chihou_zukan_award_param );
  GFL_HEAP_FreeMemory( wk->mystatus );
  GFL_OVERLAY_Unload(FS_OVERLAY_ID(chihou_zukan_award));
}

// 全国図鑑賞状
static void ZenkokuZukanAwardInit( KAWADA_MAIN_WORK* wk )
{
  u8 i;
  u16 name[6] = L"ゴモジノコ";
  name[5] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode

  GFL_OVERLAY_Load(FS_OVERLAY_ID(zenkoku_zukan_award));
  
  wk->mystatus = GFL_HEAP_AllocMemory( wk->heapID, MYSTATUS_SAVE_SIZE );
  
  for( i=0; i<6; i++ )
  {
    wk->mystatus->name[i] = name[i];
  }
  wk->mystatus->sex = PM_FEMALE;

  wk->zenkoku_zukan_award_param = ZENKOKU_ZUKAN_AWARD_AllocParam( wk->heapID, wk->mystatus );
  
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &ZENKOKU_ZUKAN_AWARD_ProcData, wk->zenkoku_zukan_award_param );
}
static void ZenkokuZukanAwardExit( KAWADA_MAIN_WORK* wk )
{
  ZENKOKU_ZUKAN_AWARD_FreeParam( wk->zenkoku_zukan_award_param );
  GFL_HEAP_FreeMemory( wk->mystatus );
  GFL_OVERLAY_Unload(FS_OVERLAY_ID(zenkoku_zukan_award));
}

// 通信対戦後の録画選択画面
static void BtlRecSelInit( KAWADA_MAIN_WORK* wk )
{
  BOOL        b_rec       = TRUE;
  BOOL        b_sync      = TRUE;
  MYSTATUS*   mystatus    = GAMEDATA_GetMyStatus( wk->gamedata );
  MYITEM_PTR  myitem_ptr  = GAMEDATA_GetMyItem( wk->gamedata );
  if( MYITEM_CheckItem( myitem_ptr, ITEM_BATORUREKOODAA, 1, wk->heapID ) )
  {
    MYITEM_SubItem( myitem_ptr, ITEM_BATORUREKOODAA, 1, wk->heapID );
  }

  GFL_OVERLAY_Load(FS_OVERLAY_ID(btl_rec_sel));

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ) b_rec = FALSE;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ) b_sync = FALSE;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    mystatus->sex = 0;
  }
  else
  {
    mystatus->sex = 1;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    MYITEM_AddItem( myitem_ptr, ITEM_BATORUREKOODAA, 1, wk->heapID );
  }

  wk->btl_rec_sel_param = BTL_REC_SEL_AllocParam( wk->heapID, wk->gamedata, b_rec, b_sync );
  
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &BTL_REC_SEL_ProcData, wk->btl_rec_sel_param );
}
static void BtlRecSelExit( KAWADA_MAIN_WORK* wk )
{
  BTL_REC_SEL_FreeParam( wk->btl_rec_sel_param );
  GFL_OVERLAY_Unload(FS_OVERLAY_ID(btl_rec_sel));
}

// 二択簡易会話
static void PmsInputDoubleInit( KAWADA_MAIN_WORK* wk )
{
  SAVE_CONTROL_WORK*  sv  = GAMEDATA_GetSaveControlWork( wk->gamedata );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(pmsinput) );
  wk->pmsi_param = PMSI_PARAM_Create( PMSI_MODE_DOUBLE, PMSI_GUIDANCE_DEFAULT, NULL, TRUE, sv, wk->heapID );
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &ProcData_PMSInput, wk->pmsi_param );
}
static void PmsInputDoubleExit( KAWADA_MAIN_WORK* wk )
{
  if( PMSI_PARAM_CheckModified( wk->pmsi_param ) )
  { 
    //PMS_DATA pms;
    //PMSI_PARAM_GetInputDataSentence( wk->pmsi_param, &pms );
    //OS_Printf( "%d, %d\n", pms.word[0], pms.word[1] );
    PMS_WORD word[2];
    PMSI_PARAM_GetInputDataDouble(  wk->pmsi_param, word );
    OS_Printf( "double: %d, %d\n", word[0], word[1] );
  }
  else
  {
    OS_Printf( "double: not modified\n" );
  }
  PMSI_PARAM_Delete( wk->pmsi_param );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(pmsinput) );
}

// 一択簡易会話
static void PmsInputSingleInit( KAWADA_MAIN_WORK* wk )
{
  SAVE_CONTROL_WORK*  sv  = GAMEDATA_GetSaveControlWork( wk->gamedata );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(pmsinput) );
  wk->pmsi_param = PMSI_PARAM_Create( PMSI_MODE_SINGLE, PMSI_GUIDANCE_DEFAULT, NULL, TRUE, sv, wk->heapID );
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &ProcData_PMSInput, wk->pmsi_param );
}
static void PmsInputSingleExit( KAWADA_MAIN_WORK* wk )
{
  if( PMSI_PARAM_CheckModified( wk->pmsi_param ) )
  { 
    PMS_WORD word = PMSI_PARAM_GetInputDataSingle( wk->pmsi_param );
    OS_Printf( "single: %d\n", word );
  }
  else
  {
    OS_Printf( "single: not modified\n" );
  }
  PMSI_PARAM_Delete( wk->pmsi_param );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(pmsinput) );
}

