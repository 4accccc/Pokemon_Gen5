//==============================================================================
/**
 * @file	d_ohno_list.c
 * @brief	デバッグメニュー
 * @author	ohno
 * @date	2009.2.12
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
//#include "savedata/contest_savedata.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"

#include "net\network_define.h"
#include "../../field/event_gtsnego.h"

#include "print\printsys.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "message.naix"
#include "test_graphic\d_taya.naix"
#include "msg\msg_d_ohno.h"
#include "test/performance.h"
#include "font/font.naix"

#include "pokeicon/pokeicon.h"
#include "item/item.h"
#include "poke_tool/pokeparty.h"
#include "net_app/wificlub/wifi_p2pmatch.h"
#include "net_app/pokemontrade.h"
#include "poke_tool/monsno_def.h"


//==============================================================================
//	定数定義
//==============================================================================
///STRBUFポインタの数(メニューの配列より多く持っておく事)
#define D_STRBUF_NUM			(30)


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	GFL_BMPWIN		*win;
	GFL_BMP_DATA	*bmp;
//	PRINT_UTIL		printUtil[1];
	BOOL			message_req;		///<TRUE:メッセージリクエストがあった
}DM_MSG_DRAW_WIN;

typedef struct {

	u16		seq;
	HEAPID	heapID;
	int debug_mode;
	int timer;
	
	GFL_FONT		*fontHandle;
//	PRINT_QUE		*printQue;
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
}D_OHNO_WORK;

///メニューリスト
typedef struct{
	u32 str_id;
	const GFL_PROC_DATA *next_proc;
	void *(*parent_work_func)(D_OHNO_WORK *);
	u32 ov_id;
}D_MENULIST;

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL DebugOhno_ItemDebug(D_OHNO_WORK *wk);
static void * _PokeTradeDemoWorkCreate(D_OHNO_WORK *wk);
static void * _PokeTradeWorkCreate(D_OHNO_WORK *wk);
static void * _PokeIrcTradeWorkCreate(D_OHNO_WORK *wk);
static void * _PokeTradeGtsNegoCreate(D_OHNO_WORK *wk);


//==============================================================================
//	外部データ
//==============================================================================
FS_EXTERN_OVERLAY(wificlub);
FS_EXTERN_OVERLAY(ohno_debugapp);
FS_EXTERN_OVERLAY(ircbattlematch);
FS_EXTERN_OVERLAY(pokemon_trade);
FS_EXTERN_OVERLAY(gamesync);
extern const GFL_PROC_DATA MidiTest_ProcData;
extern const GFL_PROC_DATA WifiClubProcData;
extern const GFL_PROC_DATA DebugOhnoMainProcData;
extern const GFL_PROC_DATA DebugLayoutMainProcData;
extern const GFL_PROC_DATA G_SYNC_ProcData;
extern const GFL_PROC_DATA VTRProcData;
extern const GFL_PROC_DATA PokemonTradeWiFiProcData;
extern const GFL_PROC_DATA PokemonTradeProcData;
extern const GFL_PROC_DATA PokemonTradeIrcProcData;
extern const GFL_PROC_DATA DebugSaveAddrProcData;



extern const GFL_PROC_DATA NetFourParentProcData;
extern const GFL_PROC_DATA NetFourChildProcData;



//==============================================================================
//	データ
//==============================================================================
//メニューデータ
static const D_MENULIST DebugMenuList[] = {
	{//
		DEBUG_OHNO_MSG0017, 
		&PokemonTradeWiFiProcData,	
		_PokeTradeWorkCreate,
		FS_OVERLAY_ID(pokemon_trade)
	},
	{//
		DEBUG_OHNO_MSG0018, 
		&GtsNego_ProcData,	
		_PokeTradeGtsNegoCreate,
		FS_OVERLAY_ID(gts_negotiate)
	},
	{//
		DEBUG_OHNO_MSG0019, 
		&PokemonTradeDemoProcData,
		_PokeTradeDemoWorkCreate,
		FS_OVERLAY_ID(pokemon_trade)
	},
  {//
		DEBUG_OHNO_MSG0015, 
		&PokemonTradeIrcProcData,	
		_PokeIrcTradeWorkCreate,
		FS_OVERLAY_ID(pokemon_trade)
	},
	{//
		DEBUG_OHNO_MSG0013, 
		&PokemonTradeProcData,	
		_PokeIrcTradeWorkCreate,
		FS_OVERLAY_ID(pokemon_trade)
	},
	{//
		DEBUG_OHNO_MSG0002, 
		&DebugLayoutMainProcData,	
		NULL,
		FS_OVERLAY_ID(ohno_debugapp)
	},
	{//
		DEBUG_OHNO_MSG0003, 
		&DebugOhnoMainProcData,	
		NULL,
		FS_OVERLAY_ID(ohno_debugapp)
	},
	{//
		DEBUG_OHNO_MSG0014, 
		&VTRProcData,
		NULL,
		FS_OVERLAY_ID(ohno_debugapp)
	},
	{//
		DEBUG_OHNO_MSG0020, 
		&NetFourParentProcData,
		NULL,
		FS_OVERLAY_ID(ohno_debugapp)
	},
	{//
		DEBUG_OHNO_MSG0021, 
		&NetFourChildProcData,
		NULL,
		FS_OVERLAY_ID(ohno_debugapp)
	},
	{//
		DEBUG_OHNO_MSG0016, 
		&DebugSaveAddrProcData,
		NULL,
		FS_OVERLAY_ID(ohno_debugapp)
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
static GFL_PROC_RESULT DebugOhnoMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
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

	D_OHNO_WORK* wk;


	DEBUG_PerformanceSetActive(FALSE);
	  //デバッグ
	SaveControl_Load(SaveControl_GetPointer());
	
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PROC, 0x70000 );
	wk = GFL_PROC_AllocWork( proc, sizeof(D_OHNO_WORK), HEAPID_PROC );
	MI_CpuClear8(wk, sizeof(D_OHNO_WORK));
	wk->heapID = HEAPID_PROC;

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

		wk->drawwin.win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 4, 0, 24, 23, 0, GFL_BMP_CHRAREA_GET_F );
		wk->drawwin.bmp = GFL_BMPWIN_GetBmp(wk->drawwin.win);
		GFL_BMP_Clear( wk->drawwin.bmp, 0xff );
		GFL_BMPWIN_MakeScreen( wk->drawwin.win );
		GFL_BMPWIN_TransVramCharacter( wk->drawwin.win );
//		PRINT_UTIL_Setup( wk->drawwin.printUtil, wk->drawwin.win );

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		wk->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

//		PRINTSYS_Init( wk->heapID );
//		wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

		wk->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_ohno_dat, wk->heapID );
		for(i = 0; i < D_STRBUF_NUM; i++){
			wk->strbuf[i] = GFL_STR_CreateBuffer(64, wk->heapID);
		}

		wk->tcbl = GFL_TCBL_Init( wk->heapID, wk->heapID, 4, 32 );

		GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI
	}

	//フォントパレット転送
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
		0, 0x20, HEAPID_PROC);

	return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugOhnoMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_OHNO_WORK* wk = mywk;
	BOOL ret = 0;
	int i;
	BOOL que_ret=TRUE;
	
	GFL_TCBL_Main( wk->tcbl );
//	que_ret = PRINTSYS_QUE_Main( wk->printQue );
//	if( PRINT_UTIL_Trans( wk->drawwin.printUtil, wk->printQue ) == FALSE){
		//return GFL_PROC_RES_CONTINUE;
//	}
//	else{
		if(que_ret == TRUE && wk->drawwin.message_req == TRUE){
//			GFL_BMP_Clear( wk->bmp, 0xff );
			GFL_BMPWIN_TransVramCharacter( wk->drawwin.win );
			wk->drawwin.message_req = FALSE;
		}
//	}
	
	switch(wk->debug_mode){
	case 0:	
		ret = DebugOhno_ItemDebug(wk);
		if(ret == TRUE){
			wk->seq = 0;
			wk->debug_mode = 1;
		}
		break;
	case 1:
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
static GFL_PROC_RESULT DebugOhnoMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_OHNO_WORK* wk = mywk;
	int i;
	void *parent_work;
	
	if(DebugMenuList[wk->cursor_y].parent_work_func != NULL){
        if(DebugMenuList[wk->cursor_y].ov_id != GFL_OVERLAY_BLANK_ID){
            GFL_OVERLAY_Load(DebugMenuList[wk->cursor_y].ov_id);
        }
		wk->parent_work = DebugMenuList[wk->cursor_y].parent_work_func(wk);
        if(DebugMenuList[wk->cursor_y].ov_id != GFL_OVERLAY_BLANK_ID){
            GFL_OVERLAY_Unload(DebugMenuList[wk->cursor_y].ov_id);
        }
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
	
//	PRINTSYS_QUE_Delete(wk->printQue);
	GFL_MSG_Delete(wk->mm);
	
	GFL_FONT_Delete(wk->fontHandle);
	GFL_TCBL_Exit(wk->tcbl);
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_PROC);
	
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
static BOOL DebugOhno_ItemDebug(D_OHNO_WORK *wk)
{
    const int yini = 12;
    BOOL ret, irc_ret = 0;
	int msg_id;
	int i, x;
	
	GF_ASSERT(wk);

	switch(wk->seq){
	case 0:		//リスト描画
		GFL_BMP_Clear( wk->drawwin.bmp, 0xff );
		wk->drawwin.message_req = TRUE;
		for(i = 0; i < NELEMS(DebugMenuList); i++){
            if(wk->cursor_y == i){
                GFL_MSG_GetString(wk->mm, DEBUG_OHNO_MSG0005, wk->strbuf[i]);
                PRINTSYS_Print(wk->drawwin.bmp, 0, yini+i*2*8, wk->strbuf[i], wk->fontHandle);
            }
            x = 2;
			GFL_MSG_GetString(wk->mm, DebugMenuList[i].str_id, wk->strbuf[i]);
			PRINTSYS_Print( wk->drawwin.bmp, 16, yini+i*2*8, wk->strbuf[i], wk->fontHandle);
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
				if(wk->cursor_y < 0){
					wk->cursor_y = NELEMS(DebugMenuList) - 1;
				}
				if(wk->cursor_y >= NELEMS(DebugMenuList)){
					wk->cursor_y = 0;
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



static void * _PokeTradeDemoWorkCreate(D_OHNO_WORK *wk)
{
	POKEMONTRADE_DEMO_PARAM *pWork;

  
	pWork = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(POKEMONTRADE_DEMO_PARAM));

  pWork->pMyPoke = PP_Create(MONSNO_ONOKKUSU, 100, 123456, GFL_HEAPID_APP);
  pWork->pNPCPoke = PP_Create(MONSNO_ONOKKUSU, 100, 123456, GFL_HEAPID_APP);

  
  return pWork;
}




static void * _PokeTradeGtsNegoCreate(D_OHNO_WORK *wk)
{
	EVENT_GTSNEGO_WORK *pWork;

  
	pWork = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(EVENT_GTSNEGO_WORK));

  pWork->aUser[0].selectType=0;
  pWork->aUser[1].selectType=0;
  pWork->aUser[0].selectLV=0;
  pWork->aUser[1].selectLV=0;
    
  pWork->gamedata = GAMEDATA_Create(GFL_HEAPID_APP);
  pWork->pStatus[0] = MyStatus_AllocWork(GFL_HEAPID_APP);
  pWork->pStatus[1] = MyStatus_AllocWork(GFL_HEAPID_APP);
  
  
  return pWork;
}


static void * _PokeTradeWorkCreate(D_OHNO_WORK *wk)
{
	POKEMONTRADE_PARAM *pWork2;
	EVENT_GTSNEGO_WORK *pWork;

	pWork2 = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(POKEMONTRADE_PARAM));
  
	pWork2->pNego = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(EVENT_GTSNEGO_WORK));
  pWork = pWork2->pNego;

  pWork->aUser[0].selectType=0;
  pWork->aUser[1].selectType=0;
  pWork->aUser[0].selectLV=0;
  pWork->aUser[1].selectLV=0;
    
  pWork->gamedata = GAMEDATA_Create(GFL_HEAPID_APP);
  pWork->pStatus[0] = MyStatus_AllocWork(GFL_HEAPID_APP);
  pWork->pStatus[1] = MyStatus_AllocWork(GFL_HEAPID_APP);
  
  
  return pWork2;
}

static void * _PokeIrcTradeWorkCreate(D_OHNO_WORK *wk)
{
	POKEMONTRADE_PARAM *pWork;
  MYSTATUS* pFriend;

  
	pWork = GFL_HEAP_AllocClearMemory(GFL_HEAPID_APP, sizeof(POKEMONTRADE_PARAM));
  pWork->gamedata = GAMEDATA_Create(GFL_HEAPID_APP);

  pFriend = GAMEDATA_GetMyStatusPlayer(pWork->gamedata, 1);
  GFL_STD_MemCopy(MyStatus_AllocWork(GFL_HEAPID_APP),pFriend,MyStatus_GetWorkSize() );

  
  return pWork;
}





//==============================================================================
//	
//==============================================================================
//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA DebugOhnoListProcData = {
	DebugOhnoMainProcInit,
	DebugOhnoMainProcMain,
	DebugOhnoMainProcEnd,
};


