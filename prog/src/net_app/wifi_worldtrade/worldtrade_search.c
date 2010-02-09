//============================================================================================
/**
 * @file	worldtrade_search.c
 * @brief	世界交換ポケモン検索画面処理
 * @author	Akito Mori
 * @date	06.04.16
 */
//============================================================================================
#include <gflib.h>
#include <dwc.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include <dpw_tr.h>
#include "print/wordset.h"
#include "message.naix"
#include "system/wipe.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"
#include "savedata/wifilist.h"
#include "net_app/worldtrade.h"
#include "worldtrade_local.h"

#include "msg/msg_wifi_gts.h"
#include "system/gf_date.h"



#include "worldtrade.naix"			// グラフィックアーカイブ定義

#include "msg/msg_wifi_place_msg_world.h"

//↓グラフィックデータ依存の定義
#define FRD_VIEW_BTN_PX	(18)
#define FRD_VIEW_BTN_PY	(2)
#define FRD_VIEW_BTN_SX	(14)
#define FRD_VIEW_BTN_SY	(4)
#define FRD_VIEW_OK_PAL	(0)
#define FRD_VIEW_NG_PAL	(2)

//============================================================================================
//	プロトタイプ宣言
//============================================================================================
/*** 関数プロトタイプ ***/
static void BgInit( void );
static void BgExit( void );
static void BgGraphicSet( WORLDTRADE_WORK * wk );
static void SetCellActor(WORLDTRADE_WORK *wk);
static void DelCellActor( WORLDTRADE_WORK *wk );
static void BmpWinInit( WORLDTRADE_WORK *wk );
static void BmpWinDelete( WORLDTRADE_WORK *wk );
static void InitWork( WORLDTRADE_WORK *wk );
static void FreeWork( WORLDTRADE_WORK *wk );
static  int SubSeq_Start( WORLDTRADE_WORK *wk);
static  int SubSeq_Main( WORLDTRADE_WORK *wk);
static  int SubSeq_SearchCheck( WORLDTRADE_WORK *wk );
static  int SubSeq_ServerQuery(WORLDTRADE_WORK *wk );
static  int SubSeq_ServerResult( WORLDTRADE_WORK *wk );
static  int SubSeq_SearchResultMessage( WORLDTRADE_WORK *wk );
static  int CursorPosGet( WORLDTRADE_WORK *wk );
static void CursorMove( WORLDTRADE_WORK *wk );
static  int SubSeq_End( WORLDTRADE_WORK *wk);
static  int SubSeq_InputPokenameMessage( WORLDTRADE_WORK *wk);


static  int SUBSEQ_PokenameSelectList( WORLDTRADE_WORK *wk);
static  int SUBSEQ_PokenameSelectWait( WORLDTRADE_WORK *wk);
static  int SUBSEQ_SexSelectMes( WORLDTRADE_WORK *wk);
static  int SUBSEQ_SexSelectList( WORLDTRADE_WORK *wk);
static  int SUBSEQ_SexSelectWait( WORLDTRADE_WORK *wk);
static  int SUBSEQ_LevelSelectMes( WORLDTRADE_WORK *wk);
static  int SUBSEQ_LevelSelectList( WORLDTRADE_WORK *wk);
static  int SUBSEQ_LevelSelectWait( WORLDTRADE_WORK *wk);
static  int SUBSEQ_CountrySelectMes( WORLDTRADE_WORK *wk);
static  int SUBSEQ_CountrySelectList( WORLDTRADE_WORK *wk);
static  int SUBSEQ_CountrySelectWait( WORLDTRADE_WORK *wk);
static  int SubSeq_YesNo( WORLDTRADE_WORK *wk);
static  int SubSeq_YesNoSelect( WORLDTRADE_WORK *wk);
static  int SubSeq_PageChange( WORLDTRADE_WORK *wk);
static  int SubSeq_SearchErrorMessage(WORLDTRADE_WORK *wk);
static  int SubSeq_MessageWait( WORLDTRADE_WORK *wk );
static  int SubSeq_SearchResultMessageWait( WORLDTRADE_WORK *wk );
static void SubSeq_MessagePrint( WORLDTRADE_WORK *wk, int msgno, int wait, int flag, u16 dat );
static  int SubSeq_ServerQueryFailure( WORLDTRADE_WORK *wk );
static  int SubSeq_SearchErrorDisconnectMessage1( WORLDTRADE_WORK *wk );
static  int SubSeq_SearchErrorDisconnectMessage2( WORLDTRADE_WORK *wk );
static  int SubSeq_SearchErrorDisconnectMessage3( WORLDTRADE_WORK *wk );
static  int SubSeq_ExchangeScreen1( WORLDTRADE_WORK *wk );
static  int SubSeq_ReturnScreen2( WORLDTRADE_WORK *wk );
static  int SubSeq_ExchangeMain( WORLDTRADE_WORK *wk );


static void WantLabelPrint( GFL_BMPWIN *win[], GFL_BMPWIN *country_win[], GFL_MSGDATA *MsgManager, WT_PRINT *print );
static  int LastTradeDateCheck( WORLDTRADE_WORK *wk, int trade_type );
static  int DpwSerachCompare( const Dpw_Tr_PokemonSearchData *s1, const Dpw_Tr_PokemonSearchData *s2 , int country_code1, int country_code2);
static  int SubSeq_MessageWait1Min( WORLDTRADE_WORK *wk );
static void SelectFrameBoxWrite( u16 *scr, int x, int y, int type );
static void TouchCursorMove( WORLDTRADE_WORK *wk, int touch );
static  u32 TouchPanelFunc( WORLDTRADE_WORK *wk );
static void DecideFunc( WORLDTRADE_WORK *wk, int decide );
static void FriendViewButtonPrint( GFL_BMPWIN *win, GFL_MSGDATA *MsgManager, int flag, WT_PRINT *print );

static  u32 WordHead_SelectMain( WORLDTRADE_WORK *wk );
static void SlideScreenVFunc( void *p );
static void BgBlendSet( int rate  );

enum{
	SUBSEQ_START=0,
	SUBSEQ_MAIN,
	SUBSEQ_END,

	SUBSEQ_INPUT_POKENAME_MES,
    SUBSEQ_POKENAME_SELECT_LIST,
    SUBSEQ_POKENAME_SELECT_WAIT,
	SUBSEQ_SEX_SELECT_MES,
	SUBSEQ_SEX_SELECT_LIST,
	SUBSEQ_SEX_SELECT_WAIT,
	SUBSEQ_LEVEL_SELECT_MES,
	SUBSEQ_LEVEL_SELECT_LIST,
	SUBSEQ_LEVEL_SELECT_WAIT,
	SUBSEQ_COUNTRY_SELECT_MES,
	SUBSEQ_COUNTRY_SELECT_LIST,
	SUBSEQ_COUNTRY_SELECT_WAIT,

	SUBSEQ_SEARCH_CHECK,
	SUBSEQ_SERVER_QUERY,
	SUBSEQ_SERVER_RESULT,
	SUBSEQ_SEARCH_RESULT_MESSAGE,
	SUBSEQ_SEARCH_RESULT_MESSAGE_WAIT,

	SUBSEQ_SERVER_QUERY_FAILURE,


	SUBSEQ_MES_WAIT,
	SUBSEQ_MES_WAIT_1MIN,
	SUBSEQ_YESNO,
	SUBSEQ_YESNO_SELECT,
	SUBSEQ_PAGE_CHANGE,
	SUBSEQ_SEARCH_ERROR_MES,

	SUBSEQ_SEARCH_ERROR_DICONNECT_MES1,
	SUBSEQ_SEARCH_ERROR_DICONNECT_MES2,

	 SUBSEQ_EXCHANGE_SCREEN1,	
     SUBSEQ_RETURN_SCREEN2,	
     SUBSEQ_EXCHANGE_MAIN,	

};

static int (*Functable[])( WORLDTRADE_WORK *wk ) = {
	SubSeq_Start,				// SUBSEQ_START=0,
	SubSeq_Main,             	// SUBSEQ_MAIN,
	SubSeq_End,              	// SUBSEQ_END,

	SubSeq_InputPokenameMessage,	// SUBSEQ_INPUT_POKENAME_MES,
	
    SUBSEQ_PokenameSelectList,      // SUBSEQ_POKENAME_SELECT_LIST,            
    SUBSEQ_PokenameSelectWait,      // SUBSEQ_POKENAME_SELECT_WAIT,            
	SUBSEQ_SexSelectMes,            // SUBSEQ_SEX_SELECT_MES,            
	SUBSEQ_SexSelectList,           // SUBSEQ_SEX_SELECT_LIST,            
	SUBSEQ_SexSelectWait,           // SUBSEQ_SEX_SELECT_WAIT,            
	SUBSEQ_LevelSelectMes,          // SUBSEQ_LEVEL_SELECT_MES,            
	SUBSEQ_LevelSelectList,         // SUBSEQ_LEVEL_SELECT_LIST,            
	SUBSEQ_LevelSelectWait,         // SUBSEQ_LEVEL_SELECT_WAIT,            
	SUBSEQ_CountrySelectMes,        // SUBSEQ_COUNTRY_SELECT_MES,            
	SUBSEQ_CountrySelectList,       // SUBSEQ_COUNTRY_SELECT_LIST,            
	SUBSEQ_CountrySelectWait,       // SUBSEQ_COUNTRY_SELECT_WAIT,            
                                                        
	SubSeq_SearchCheck,				// SUBSEQ_SEARCH_CHECK,     
	SubSeq_ServerQuery,             // SUBSEQ_SERVER_QUERY,                  
	SubSeq_ServerResult,            // SUBSEQ_SERVER_RESULT,
	SubSeq_SearchResultMessage,     // SUBSEQ_SEARCH_RESULT_MESSAGE,
	SubSeq_SearchResultMessageWait, // SUBSEQ_SEARCH_RESULT_MESSAGE_WAIT,

	SubSeq_ServerQueryFailure,		// SUBSEQ_SERVER_QUERY_FAILURE,


	SubSeq_MessageWait,     		// SUBSEQ_MES_WAIT
	SubSeq_MessageWait1Min,			// SUBSEQ_MES_WAIT_1MIN
	SubSeq_YesNo,					// SUBSEQ_YESNO
	SubSeq_YesNoSelect,				// SUBSEQ_YESNO_SELECT
	SubSeq_PageChange,				// SUBSEQ_PAGE_CHANGE,
	SubSeq_SearchErrorMessage,		// SUBSEQ_SEARCH_ERROR_MES,

	SubSeq_SearchErrorDisconnectMessage1,	// 	SUBSEQ_SEARCH_ERROR_DICONNECT_MES1,
	SubSeq_SearchErrorDisconnectMessage2,	// 	SUBSEQ_SEARCH_ERROR_DICONNECT_MES2,

	SubSeq_ExchangeScreen1,			// SUBSEQ_EXCHANGE_SCREEN1,	
	SubSeq_ReturnScreen2,			// SUBSEQ_RETURN_SCREEN2,	
	SubSeq_ExchangeMain,			// SUBSEQ_EXCHANGE_MAIN,	
};

#define INFO_TEXT_WORD_NUM	(10*2)


// 「さがす」画面のカーソル位置
static u16 CursorPos[][3]={
	{   0,    0, 45 },	// X, Y, セルアクターのアニメ番号
	{   0,   40, 45 },
	{   0,   80, 45 },
	{   0,  120, 45 },
	{ 192,   32, 46 },
	{ 192,   72, 46 },
	{ 192,  112, 46 },
	
};

// カーソルの位置指定定義
enum{
	CURSOR_POS_POKEMON = 0,
	CURSOR_POS_SEX,
	CURSOR_POS_LEVEL,
	CURSOR_POS_NATION,
	CURSOR_POS_VIEW,
	CURSOR_POS_SEARCH,
	CURSOR_POS_BACK,
};


//検索条件の表示を右寄せするか、センタリングするか、左寄せにするか
#define SEARCH_INFO_PRINT_FLAG		(0)	//0:左、1:センタリング、2:右寄せ

///強制タイムアウトまでの時間
#define TIMEOUT_TIME			(30*60*2)	//2分


//============================================================================================
//	プロセス関数
//============================================================================================

//==============================================================================
/**
 * @brief   世界交換入り口画面初期化
 *
 * @param   wk		
 * @param   seq		
 *
 * @retval  int		
 */
//==============================================================================
int WorldTrade_Search_Init(WORLDTRADE_WORK *wk, int seq)
{
	// ワーク初期化
	InitWork( wk );
	
	// BG設定
	BgInit();

	// BGグラフィック転送
	BgGraphicSet( wk );

	// BMPWIN確保
	BmpWinInit( wk );

	// セルアクター登録
	SetCellActor(wk);

	// 条件入力システム初期化
	{
		WORLDTRADE_INPUT_HEADER wih;
		wih.MenuBmp   = &wk->MenuWin[0];
		wih.BackBmp   = &wk->BackWin;
		wih.CursorAct = wk->SubCursorActWork;
		wih.ArrowAct[0] = wk->BoxArrowActWork[0];
		wih.ArrowAct[1] = wk->BoxArrowActWork[1];
		wih.SearchCursorAct = wk->CursorActWork;
		wih.MsgManager      = wk->MsgManager;
		wih.MonsNameManager = wk->MonsNameManager;
		wih.CountryNameManager = wk->CountryNameManager;
		wih.Zukan			= wk->param->zukanwork;
		wih.SinouTable      = wk->dw->sinouTable;
		wk->WInputWork = WorldTrade_Input_Init( &wih,  GFL_BG_FRAME2_M, SITUATION_SEARCH );
	}

	// 「ほしいポケモン」描画
	WantLabelPrint( &wk->InfoWin[0], &wk->CountryWin[0], wk->MsgManager, &wk->print );

	// 「あいてをみる」描画
	FriendViewButtonPrint(  wk->InfoWin[8], wk->MsgManager, wk->SubLcdTouchOK, &wk->print );


	// 名前
	WorldTrade_PokeNamePrint( wk->InfoWin[1], wk->MonsNameManager, 
		wk->Search.characterNo, SEARCH_INFO_PRINT_FLAG, 0, PRINTSYS_LSB_Make(1,2,0), &wk->print  );

	// 性別条件
	WorldTrade_SexPrint( wk->InfoWin[3], wk->MsgManager, 
		wk->Search.gender, 1, 0, SEARCH_INFO_PRINT_FLAG, PRINTSYS_LSB_Make(1,2,0), &wk->print  );

	// レベル指定
	WorldTrade_WantLevelPrint( wk->InfoWin[5], wk->MsgManager, 
		WorldTrade_LevelTermGet(wk->Search.level_min,wk->Search.level_max, LEVEL_PRINT_TBL_SEARCH),
		SEARCH_INFO_PRINT_FLAG, 0, PRINTSYS_LSB_Make(1,2,0), LEVEL_PRINT_TBL_SEARCH, &wk->print );

	// 国指定
	WorldTrade_CountryPrint( wk->CountryWin[1], wk->CountryNameManager, wk->MsgManager,
		wk->country_code, SEARCH_INFO_PRINT_FLAG, 0, PRINTSYS_LSB_Make(1,2,0), &wk->print );

	wk->vfunc2 = SlideScreenVFunc;
	
	
	// ２回目以降
	// ワイプフェード開始（両画面）

	if(wk->sub_process_mode==MODE_NORMAL){
		WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
						WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
	}
	wk->subprocess_seq = SUBSEQ_START;
#ifdef GTS_FADE_OSP
	OS_Printf( "******************** worldtrade_search.c [218] M ********************\n" );
#endif

	return SEQ_FADEIN;
}
//==============================================================================
/**
 * @brief   世界交換入り口画面メイン
 *
 * @param   wk		
 * @param   seq		
 *
 * @retval  int		
 */
//==============================================================================
int WorldTrade_Search_Main(WORLDTRADE_WORK *wk, int seq)
{
	int i;
	int ret;
	
	ret = (*Functable[wk->subprocess_seq])( wk );

	// サブ画面のOBJ座標を移動させる処理
	for(i=0;i<SUB_OBJ_NUM;i++){
		WorldTrade_CLACT_PosChangeSub( wk->SubActWork[i], wk->SubActY[i][0], wk->SubActY[i][1]+wk->DrawOffset );
		//WorldTrade_CLACT_PosChange( wk->PromptDsActWork, DS_ICON_X, DS_ICON_Y+256+wk->DrawOffset );
	}

	WorldTrade_CLACT_PosChange( wk->CursorActWork,  
								CursorPos[CursorPosGet( wk )][0], 
								CursorPos[CursorPosGet( wk )][1] - wk->DrawOffset );

	return ret;
}


//==============================================================================
/**
 * @brief   世界交換入り口画面終了
 *
 * @param   wk		
 * @param   seq		
 *
 * @retval  int		
 */
//==============================================================================
int WorldTrade_Search_End(WORLDTRADE_WORK *wk, int seq)
{
	wk->vfunc2 = NULL;
	
	// 条件入力システム終了
	WorldTrade_Input_Exit( wk->WInputWork );
	
	DelCellActor( wk );
	
	FreeWork( wk );
	
	BmpWinDelete( wk );
	
	BgExit();

	// 「DSの下画面をみてねアイコン」非表示
	GFL_CLACT_WK_SetDrawEnable( wk->PromptDsActWork, 0 );
	
	WorldTrade_SubProcessUpdate( wk );
	
	return SEQ_INIT;
}





//--------------------------------------------------------------------------------------------
/**
 * BG設定
 *
 * @param	ini		BGLデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BgInit( void )
{
	// BG SYSTEM
	{	
		GFL_BG_SYS_HEADER BGsys_data = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &BGsys_data );

	}


	// メイン画面テキスト面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000,GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(  GFL_BG_FRAME0_M );
		GFL_BG_SetVisible( GFL_BG_FRAME0_M, TRUE );
	}

	// メイン画面背景面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000,GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME1_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(  GFL_BG_FRAME1_M );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M, TRUE );
	}

	// メイン画面割り込みウインドウ面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000,GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME2_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(  GFL_BG_FRAME2_M );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M, TRUE );
	}

	// メイン画面第３テキスト面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1800, GX_BG_CHARBASE_0x1c000,GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME3_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(  GFL_BG_FRAME3_M );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M, TRUE );
	}

	WorldTrade_SubLcdBgInit(  0, 0 );

	GFL_BG_SetClearCharacter( GFL_BG_FRAME2_M, 32, 0, HEAPID_WORLDTRADE );
	GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 32, 0, HEAPID_WORLDTRADE );
	GFL_BG_SetClearCharacter( GFL_BG_FRAME3_M, 32, 0, HEAPID_WORLDTRADE );

}

//--------------------------------------------------------------------------------------------
/**
 * BG解放
 *
 * @param	ini		BGLデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BgExit( void )
{

	// サブ画面ＢＧ情報解放
	WorldTrade_SubLcdBgExit( );

	// メイン画面ＢＧ情報解放
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );

}


//--------------------------------------------------------------------------------------------
/**
 * グラフィックデータセット
 *
 * @param	wk		ポケモンリスト画面のワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BgGraphicSet( WORLDTRADE_WORK * wk )
{
	ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_WORLDTRADE_GRA, HEAPID_WORLDTRADE );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_worldtrade_search_nclr, PALTYPE_MAIN_BG, 0, 16*3*2,  HEAPID_WORLDTRADE);
	GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_worldtrade_traderoom_nclr, PALTYPE_SUB_BG,  0, 16*8*2,  HEAPID_WORLDTRADE);
	
	// 会話フォントパレット転送
	TalkFontPaletteLoad( PALTYPE_MAIN_BG, WORLDTRADE_TALKFONT_PAL*0x20, HEAPID_WORLDTRADE );
  // 	TalkFontPaletteLoad( PALTYPE_SUB_BG,  WORLDTRADE_TALKFONT_PAL*0x20, HEAPID_WORLDTRADE );

	// 会話ウインドウグラフィック転送
	BmpWinFrame_GraphicSet(	 GFL_BG_FRAME0_M, WORLDTRADE_MESFRAME_CHR, 
						WORLDTRADE_MESFRAME_PAL,  CONFIG_GetWindowType(wk->param->config), HEAPID_WORLDTRADE );

	BmpWinFrame_GraphicSet(	 GFL_BG_FRAME0_M, WORLDTRADE_MENUFRAME_CHR,
						WORLDTRADE_MENUFRAME_PAL, 0, HEAPID_WORLDTRADE );



	// メイン画面BG1キャラ転送
	GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_worldtrade_search_lz_ncgr,  GFL_BG_FRAME1_M, 0, 16*5*0x20, 1, HEAPID_WORLDTRADE);

	// メイン画面BG1スクリーン転送
	GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_worldtrade_search_lz_nscr,  GFL_BG_FRAME1_M, 0, 32*24*2, 1, HEAPID_WORLDTRADE);

	// メイン画面BG2キャラ転送
	GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_worldtrade_input_lz_ncgr,   GFL_BG_FRAME2_M, 0, 16*3*0x20, 1, HEAPID_WORLDTRADE);


	// サブ画面BG1キャラ転送
	//GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_worldtrade_traderoom_lz_ncgr,  GFL_BG_FRAME1_S, 0, 32*21*0x40, 1, HEAPID_WORLDTRADE);

	// サブ画面BG1スクリーン転送
	//GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_worldtrade_traderoom_lz_nscr,  GFL_BG_FRAME1_S, 0, 32*24*2, 1, HEAPID_WORLDTRADE);

	// 会話フォントパレット転送
	TalkFontPaletteLoad( PALTYPE_SUB_BG, WORLDTRADE_SUB_TALKFONT_PAL*0x20, HEAPID_WORLDTRADE );

	WorldTrade_SubLcdWinGraphicSet( wk );   // トレードルームウインドウ転送


	GFL_ARC_CloseDataHandle( p_handle );

}


//------------------------------------------------------------------
/**
 * セルアクター登録
 *
 * @param   wk			WORLDTRADE_WORK*
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void SetCellActor(WORLDTRADE_WORK *wk)
{
	//登録情報格納
	GFL_CLWK_DATA	add;
	//WorldTrade_MakeCLACT( &add,  wk, &wk->clActHeader_main, NNS_G2D_VRAM_TYPE_2DMAIN );

	GFL_STD_MemClear( &add, sizeof(GFL_CLWK_DATA) );

	// カーソル表示
	add.pos_x = CursorPos[0][0];
	add.pos_y = CursorPos[0][1];
	wk->CursorActWork = GFL_CLACT_WK_Create( wk->clactSet,
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->CursorActWork, 1 );
	GFL_CLACT_WK_SetAnmSeq( wk->CursorActWork, 45 );
	GFL_CLACT_WK_SetBgPri( wk->CursorActWork, 1 );	//国名選択ウィンドウの下に行かせる為下げる
	GFL_CLACT_WK_SetObjMode( wk->CursorActWork, GX_OAM_MODE_XLU );
	
	// サブカーソル登録
	add.pos_x = 160;
	add.pos_y = 32;
	wk->SubCursorActWork = GFL_CLACT_WK_Create( wk->clactSet,
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
	GFL_CLACT_WK_SetAnmSeq( wk->SubCursorActWork, 47 );
	GFL_CLACT_WK_SetDrawEnable( wk->SubCursorActWork, 0 );

	// （右向き）
	add.pos_x = 228;
	add.pos_y = 117;
	wk->BoxArrowActWork[0] = GFL_CLACT_WK_Create( wk->clactSet,
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
	GFL_CLACT_WK_SetAnmSeq( wk->BoxArrowActWork[0], CELL_BOXARROW_NO );
	GFL_CLACT_WK_SetDrawEnable( wk->BoxArrowActWork[0], 0 );
	
	// ページ移動カーソル登録(左向き）
	add.pos_x = 140;
	wk->BoxArrowActWork[1] = GFL_CLACT_WK_Create( wk->clactSet,
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
	GFL_CLACT_WK_SetAnmSeq( wk->BoxArrowActWork[1], CELL_BOXARROW_NO+1 );
	GFL_CLACT_WK_SetDrawEnable( wk->BoxArrowActWork[1], 0 );
	WirelessIconEasy();
}

//------------------------------------------------------------------
/**
 * @brief   「さがす」画面で登録したセルアクターの削除
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void DelCellActor( WORLDTRADE_WORK *wk )
{
	GFL_CLACT_WK_Remove(wk->CursorActWork);
	GFL_CLACT_WK_Remove(wk->SubCursorActWork);
	GFL_CLACT_WK_Remove(wk->BoxArrowActWork[0]);
	GFL_CLACT_WK_Remove(wk->BoxArrowActWork[1]);


}


// 「ポケモン」・名前・「せいべつ」・性別・「レベル」・レベル
#define INFORMATION_STR_X	(  1 )
#define INFORMATION_STR_X2	(  2 )
#define INFORMATION1_STR_Y	(  1 )
#define INFORMATION2_STR_Y	(  3 )
#define INFORMATION3_STR_Y	(  6 )
#define INFORMATION4_STR_Y	(  8 )
#define INFORMATION5_STR_Y	( 11 )
#define INFORMATION6_STR_Y	( 13 )
#define INFORMATION7_STR_Y	( 16 )
#define INFORMATION8_STR_Y	( 18 )
#define INFORMATION_STR_SX	( 11 )
#define INFORMATION_STR_SY	(  2 )


#define BUTTON_STR_X		( 22 )
#define BUTTON1_STR_Y		(  8 )
#define BUTTON2_STR_Y		( 13 )
#define BUTTON3_STR_Y		(  3 )
#define BUTTON_STR_SX		( 10 )
#define BUTTON_STR_SY		(  2 )

#define COUNTRY_STR_SX	( 28 )
#define COUNTRY_STR_SY	(  2 )

// BG0面
#define TITLE_MESSAGE_OFFSET   ( WORLDTRADE_MENUFRAME_CHR + MENU_WIN_CGX_SIZ )
#define LINE_MESSAGE_OFFSET    ( TITLE_MESSAGE_OFFSET     + TITLE_TEXT_SX*TITLE_TEXT_SY )
//#define BUTTON_STR_OFFSET	   ( LINE_MESSAGE_OFFSET      + LINE_TEXT_SX*LINE_TEXT_SY )
//#define SELECT_MENU1_OFFSET    ( BUTTON_STR_OFFSET        + BUTTON_STR_SX*BUTTON_STR_SY*2 )
#define SELECT_MENU1_OFFSET    ( TITLE_MESSAGE_OFFSET        + BUTTON_STR_SX*BUTTON_STR_SY*2 )
#define SELECT_MENU2_OFFSET    ( SELECT_MENU1_OFFSET      + SELECT_MENU1_SX*SELECT_MENU1_SY )
#define SELECT_MENU3_OFFSET    ( SELECT_MENU1_OFFSET )
#define SELECT_MENU4_OFFSET    ( SELECT_MENU1_OFFSET )
#define SELECT_MENU5_OFFSET    ( SELECT_MENU1_OFFSET )
#define YESNO_OFFSET 		   ( SELECT_MENU4_OFFSET      + SELECT_MENU4_SX*SELECT_MENU4_SY )


//BG3面
#define INFOMATION_STR_OFFSET  ( 1 )
#define COUNTRY_STR_OFFSET	   ( INFOMATION_STR_OFFSET    + INFORMATION_STR_SX*INFORMATION_STR_SY*6)
static const u16 infomation_bmpwin_table[6][2]={
	{INFORMATION_STR_X,   INFORMATION1_STR_Y,},
	{INFORMATION_STR_X2,  INFORMATION2_STR_Y,},
	{INFORMATION_STR_X,   INFORMATION3_STR_Y,},
	{INFORMATION_STR_X2,  INFORMATION4_STR_Y,},
	{INFORMATION_STR_X,   INFORMATION5_STR_Y,},
	{INFORMATION_STR_X2,  INFORMATION6_STR_Y,},

};

#define BUTTON_STR_OFFSET	   ( COUNTRY_STR_OFFSET   +  (COUNTRY_STR_SX*COUNTRY_STR_SY)*2 + 20*2 )

static const u16 button_bmpwin_table[][2]={
	{BUTTON_STR_X,	BUTTON1_STR_Y},
	{BUTTON_STR_X,	BUTTON2_STR_Y},
	{BUTTON_STR_X,	BUTTON3_STR_Y},
};

static const u16 country_bmpwin_table[][2]={
	{INFORMATION_STR_X,		INFORMATION7_STR_Y},
	{INFORMATION_STR_X2,	INFORMATION8_STR_Y},
};

//------------------------------------------------------------------
/**
 * BMPWIN処理（文字パネルにフォント描画）
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void BmpWinInit( WORLDTRADE_WORK *wk )
{
	// ---------- メイン画面 ------------------

//	GFL_BMPWIN_CreateFixPos( &wk->TitleWin, GFL_BG_FRAME0_M,
//		TITLE_TEXT_X, TITLE_TEXT_Y, TITLE_TEXT_SX, TITLE_TEXT_SY, 
//		WORLDTRADE_TALKFONT_PAL,  TITLE_MESSAGE_OFFSET );

//	GFL_BMP_Clear( &wk->TitleWin, 0x0000 );
	
	// 「ポケモンをさがす」描画
//	WorldTrade_TalkPrint( &wk->TitleWin, wk->TitleString, 0, 1, 0, PRINTSYS_LSB_Make(15,13,0) );

	// 会話ウインドウ
	wk->MsgWin	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME0_M,
		LINE_TEXT_X, LINE_TEXT_Y, LINE_TEXT_SX, LINE_TEXT_SY, 
		WORLDTRADE_TALKFONT_PAL,  LINE_MESSAGE_OFFSET );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin), 0x0000 );
	GFL_BMPWIN_MakeTransWindow( wk->MsgWin );


	// BG0面BMPWIN情報ウインドウ確保
	{
		int i, offset;

		// ほしいポケモン情報x６文字列
		for(i=0;i<6;i++){
			wk->InfoWin[i]	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME3_M,
					infomation_bmpwin_table[i][0], 
					infomation_bmpwin_table[i][1], 
					INFORMATION_STR_SX, INFORMATION_STR_SY,
					WORLDTRADE_TALKFONT_PAL,  
					INFOMATION_STR_OFFSET + (INFORMATION_STR_SX*INFORMATION_STR_SY)*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin[i]), 0x0000 );
			GFL_BMPWIN_MakeTransWindow( wk->InfoWin[i] );
		}

		// 住んでいる所・国名
		for(i=0;i<2;i++){
			wk->CountryWin[i]	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME3_M,
					country_bmpwin_table[i][0], 
					country_bmpwin_table[i][1], 
					COUNTRY_STR_SX,COUNTRY_STR_SY,
					WORLDTRADE_TALKFONT_PAL,  
					COUNTRY_STR_OFFSET + (COUNTRY_STR_SX*COUNTRY_STR_SY)*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->CountryWin[i]), 0x0000 );
			GFL_BMPWIN_MakeTransWindow( wk->CountryWin[i] );
		}

		// さがす・もどる・あいてをみる
		for(i=0;i<3;i++){
			wk->InfoWin[6+i]	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME3_M,
					button_bmpwin_table[i][0], 
					button_bmpwin_table[i][1], 
					BUTTON_STR_SX,BUTTON_STR_SY,
					WORLDTRADE_TALKFONT_PAL,  BUTTON_STR_OFFSET+(BUTTON_STR_SX*BUTTON_STR_SY*i));
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin[6+i]), 0x0000 );
			GFL_BMPWIN_MakeTransWindow( wk->InfoWin[6+i] );
		}

	}
	
	// サブ画面のGTS説明用BMPWINを確保する
	WorldTrade_SubLcdExpainPut( wk, EXPLAIN_SAGASU );

}	

//------------------------------------------------------------------
/**
 * @brief   確保したBMPWINを解放
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void BmpWinDelete( WORLDTRADE_WORK *wk )
{
  //BMPを消す前に残ったストリームを消去
  WT_PRINT_ClearBuffer( &wk->print );
	
	GFL_BMPWIN_Delete( wk->ExplainWin );
	GFL_BMPWIN_Delete( wk->MsgWin );
//	GFL_BMPWIN_Delete( wk->TitleWin );
	{
		int i;
		for(i=0;i<9;i++){
			GFL_BMPWIN_Delete( wk->InfoWin[i] );
		}
		for(i=0;i<2;i++){
			GFL_BMPWIN_Delete( wk->CountryWin[i] );
		}
	}


}

//------------------------------------------------------------------
/**
 * 世界交換ワーク初期化
 *
 * @param   wk		WORLDTRADE_WORK*
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void InitWork( WORLDTRADE_WORK *wk )
{
	int i;

	// 文字列バッファ作成
	wk->TalkString  = GFL_STR_CreateBuffer( TALK_MESSAGE_BUF_NUM, HEAPID_WORLDTRADE );

	// 「ポケモンをさがす」
	wk->TitleString = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_02_003 );


	// ポケモン名テーブル作成用のワーク確保
	wk->dw = GFL_HEAP_AllocMemory(HEAPID_WORLDTRADE, sizeof(DEPOSIT_WORK));
	MI_CpuClearFast(wk->dw, sizeof(DEPOSIT_WORK));

	// 図鑑ソートデータ(全国図鑑と、シンオウ図鑑の分岐が必要だとおもう）


	// ↓たぶんこれいらない
//	wk->dw->nameSortTable = WorldTrade_ZukanSortDataGet( HEAPID_WORLDTRADE, 0, &wk->dw->nameSortNum );

	wk->dw->sinouTable    = WorldTrade_SinouZukanDataGet( HEAPID_WORLDTRADE );

	// カーソル位置初期化
	WorldTrade_SelectListPosInit( &wk->selectListPos );
}


//------------------------------------------------------------------
/**
 * @brief   ワーク解放
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void FreeWork( WORLDTRADE_WORK *wk )
{
	GFL_HEAP_FreeMemory( wk->dw->sinouTable );

	// ↓たぶんこれいらない
//	GFL_HEAP_FreeMemory( wk->dw->nameSortTable );
	GFL_HEAP_FreeMemory( wk->dw );

	GFL_STR_DeleteBuffer( wk->TalkString ); 
	GFL_STR_DeleteBuffer( wk->TitleString ); 


}



//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------



//------------------------------------------------------------------
/**
 * @brief   サブプロセスシーケンススタート処理
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_Start( WORLDTRADE_WORK *wk)
{
	// 
	if( wk->sub_process_mode==MODE_PARTNER_RETURN ){
		//メッセージ表示 「えらぶ」で　あいてを　きめてください
		SubSeq_MessagePrint( wk, msg_gtc_01_033, MSG_ALLPUT, 0, 0x0f0f );	//一括表示
		if(GX_GetMasterBrightness() == -16){    //真っ暗ならば「えらぶ」画面からの戻り
    		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
    						WIPE_FADE_BLACK, EXCHANGE_SCREEN_SYNC, 1, HEAPID_WORLDTRADE );
    		wk->subprocess_seq = SUBSEQ_RETURN_SCREEN2;
    	}
    	else{
    		wk->subprocess_seq = SUBSEQ_MAIN;
        }
	}else{
		SubSeq_MessagePrint( wk, msg_gtc_01_009, 1, 0, 0x0f0f );
		WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );
	}

	// 「下画面をみてね」アイコンを表示する
	GFL_CLACT_WK_SetDrawEnable( wk->PromptDsActWork, 1 );

	OS_Printf("monsno = %d, gender = %d, level = %d - %d, country = %d\n", 
			wk->Search.characterNo,wk->Search.gender,wk->Search.level_min,wk->Search.level_max,
			wk->country_code);

	OS_Printf("remain ram = %d\n", GFL_HEAP_GetHeapFreeSize( HEAPID_WORLDTRADE ));
	return SEQ_MAIN;
}

// 検索画面タッチ検出テーブル
static const GFL_UI_TP_HITTBL TouchTable[]={
	{   3,  41,   2,  99,},
	{  43,  81,   2,  99,},
	{  83, 121,   2,  99,},
	{ 123, 161,   2,  99,},
	{  19,  45, 145, 255,},
	{  58,  85, 145, 255,},
	{  98, 125, 145, 255,},
	{GFL_UI_TP_HIT_END,0,0,0},		// 終了データ
	
};
//------------------------------------------------------------------
/**
 * @brief   タッチパネル検出
 *
 * @param   wk		
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
static u32 TouchPanelFunc( WORLDTRADE_WORK *wk )
{
	u32 ret;
	ret=GFL_UI_TP_HitTrg(TouchTable);

	return ret;
}


//------------------------------------------------------------------
/**
 * @brief   決定処理（タッチ＆キー共通）
 *
 * @param   wk		
 * @param   decide		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void DecideFunc( WORLDTRADE_WORK *wk, int decide )
{
	switch(decide){
	// ポケモン指定
	case CURSOR_POS_POKEMON:	
		wk->subprocess_seq = SUBSEQ_INPUT_POKENAME_MES;	
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		break;
	// 性別指定
	case CURSOR_POS_SEX:		
		// 既にポケモンを入力していて、そのポケモンの性別が決定しているときは
		// 入力できないようにする
		if(wk->Search.characterNo!=0){
			wk->dw->sex_selection = PokePersonalParaGet(wk->Search.characterNo,POKEPER_ID_sex);
			if(WorldTrade_SexSelectionCheck( &wk->Search, wk->dw->sex_selection )){
				OS_Printf("性別決定？ = %d\n", wk->dw->sex_selection);
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				return;
			}
		}
		wk->subprocess_seq = SUBSEQ_SEX_SELECT_MES;		
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		break;
	// レベル指定
	case CURSOR_POS_LEVEL:		
		wk->subprocess_seq = SUBSEQ_LEVEL_SELECT_MES;	
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		break;
	// 国指定
	case CURSOR_POS_NATION:		
		wk->subprocess_seq = SUBSEQ_COUNTRY_SELECT_MES;	
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		break;
	// 検索結果を見る
	case CURSOR_POS_VIEW:
		if(wk->SearchResult){
			wk->subprocess_seq = SUBSEQ_EXCHANGE_SCREEN1;
			WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
						WIPE_FADE_BLACK, EXCHANGE_SCREEN_SYNC, 1, HEAPID_WORLDTRADE );
    		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		}
		break;
	// 探す
	case CURSOR_POS_SEARCH:		
		wk->subprocess_seq = SUBSEQ_SEARCH_CHECK;		
		
		break;
	// もどる
	case CURSOR_POS_BACK:
		SubSeq_MessagePrint( wk, msg_gtc_01_016, 1, 0, 0x0f0f );
		WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_YESNO );
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		break;
	}

}


//------------------------------------------------------------------
/**
 * @brief   サブプロセスシーケンスメイン
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_Main( WORLDTRADE_WORK *wk)
{
	//printstreamにしたので、
	//printstream終了前に変更してはいけない
	


	// ------タッチ処理--------
	u32 ret=TouchPanelFunc( wk );
	if(ret!=GFL_UI_TP_HIT_NONE){
		TouchCursorMove( wk, ret );
		DecideFunc( wk, ret );
		
	}else{

	// ------キー処理----------
		// カーソル移動
		CursorMove(wk);

		// 決定
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
			DecideFunc( wk, CursorPosGet(wk) );
		}else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
			// キャンセル
			SubSeq_MessagePrint( wk, msg_gtc_01_016, 1, 0, 0x0f0f );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_YESNO );
			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		}else{
#if 0
			// タッチパネルチェック
			int result = WorldTrade_SubLcdObjHitCheck( wk->SearchResult );
			if(wk->SubLcdTouchOK){
				if(result>=0){
					GFL_CLACT_WK_SetAnmSeq( wk->SubActWork[result+1], 16+result*4 );
					wk->subprocess_seq     = SUBSEQ_END; 
					WorldTrade_SubProcessChange( wk, WORLDTRADE_PARTNER, 0 );
					wk->TouchTrainerPos = result;
					PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				}
			}
#endif	
		}
	}
	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   検索できるかチェック
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_SearchCheck( WORLDTRADE_WORK *wk )
{

	// ポケモンを決めていない
	if(wk->Search.characterNo==0){
		SubSeq_MessagePrint( wk, msg_gtc_01_013, 1, 0, 0x0f0f );
		WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );
		PMSND_PlaySE(SE_GTC_NG);
	}else{
	// 以前の条件と違うなら

		if(DpwSerachCompare( &wk->Search, &wk->SearchBackup, wk->country_code, wk->SearchBackup_CountryCode)){
			// 条件が変わっていないのでダメ「かわらないみたい…」
			SubSeq_MessagePrint( wk, msg_gtc_01_034, 1, 0, 0x0f0f );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );
			PMSND_PlaySE(SE_GTC_NG);
		}else{
			PMSND_PlaySE(SE_GTC_SEARCH);

			OS_TPrintf("search start\n");
			OS_Printf( "SearchData  No = %d,  gender = %d, level min= %d max = %d, country = %d\n", wk->Search.characterNo, wk->Search.gender, wk->Search.level_min, wk->Search.level_max, wk->country_code);

			// ただいま　けんさくちゅう…
			SubSeq_MessagePrint( wk, msg_gtc_01_014, 1, 0, 0x0f0f );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_SERVER_QUERY );

			// 表示されている検索結果ＯＢＪがいる場合は隠す
			WorldTrade_SubLcdMatchObjHide( wk );
			
		}
		
	}
	
	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   最終交換成立日をチェック
 *
 * @param   wk		
 * @param   trade_type		交換成立タイプ(TRADE_TYPE_SEARCH or TRADE_TYPE_DEPOSIT)
 *
 * @retval  int		成立日が昨日か今日なら1,違うなら0
 */
//------------------------------------------------------------------
static int LastTradeDateCheck( WORLDTRADE_WORK *wk, int trade_type )
{
	GFDATE gfDate;
	RTCDate sdkDate, todayDate;
	RTCTime time;
	int diff;
	
	// セーブされている日付をSDKの日付情報に変換
	if(trade_type == TRADE_TYPE_SEARCH){
		gfDate = WorldTradeData_GetLastDate_Search( wk->param->worldtrade_data );
		OS_TPrintf("検索成立が ");
	}
	else{
		gfDate = WorldTradeData_GetLastDate_Deposit( wk->param->worldtrade_data );
		OS_TPrintf("預け成立が ");
	}
	GFDATE_GFDate2RTCDate( gfDate, &sdkDate );
	
	// 現在サーバー時刻を取得
	DWC_GetDateTime( &todayDate, &time);

	diff = RTC_ConvertDateToDay( &todayDate )-RTC_ConvertDateToDay( &sdkDate );
	if(diff>=0 && diff<3){
		OS_Printf("今日-交換最終日 = %d　なので＋２\n",diff);
		return 1;
	}

	return 0;
}

//------------------------------------------------------------------
/**
 * @brief   ポケモン検索開始
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_ServerQuery(WORLDTRADE_WORK *wk )
{
	Dpw_Tr_PokemonSearchData search;
	int  search_max = 3;				// 初期検索数は３

	// 現在サーバーにポケモンを預けているならば、最大検索数が２匹増える
#if 0	//条件変更
	if(wk->DepositFlag){
		search_max += 2;
		OS_Printf( "預けているので+2\n");
	}
#endif

	if(LastTradeDateCheck(wk, TRADE_TYPE_SEARCH)){
		search_max += 2;
	}
	if(LastTradeDateCheck(wk, TRADE_TYPE_DEPOSIT)){
		search_max += 2;
	}

#ifdef PM_DEBUG
	if(GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
		search_max = 7;
	}
#endif

	// 検索開始	
	if(wk->country_code == 0){
		Dpw_Tr_DownloadMatchDataAsync (&wk->Search, search_max, wk->DownloadPokemonData );
	}
	else{
		Dpw_Tr_PokemonSearchDataEx exs;

		MI_CpuClear8(&exs, sizeof(Dpw_Tr_PokemonSearchDataEx));
		exs.characterNo = wk->Search.characterNo;
		exs.gender = wk->Search.gender;
		exs.level_min = wk->Search.level_min;
		exs.level_max = wk->Search.level_max;
		exs.padding = wk->Search.padding;
		exs.maxNum = search_max;
		exs.countryCode = wk->country_code;

		Dpw_Tr_DownloadMatchDataExAsync(&exs, wk->DownloadPokemonData);
	}
	wk->SearchBackup = wk->Search;
	wk->SearchBackup_CountryCode = wk->country_code;
	
	OS_Printf("検索最大数 = %d\n", search_max);
	
	wk->timeout_count = 0;
	wk->subprocess_seq = SUBSEQ_SERVER_RESULT;
	
	// 下画面をタッチしても反応しない
	wk->SubLcdTouchOK = 0;
	
	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   サーバー検索応答待ち
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_ServerResult( WORLDTRADE_WORK *wk )
{
	int result;
	result=Dpw_Tr_IsAsyncEnd();
	if (result){
		s32 result = Dpw_Tr_GetAsyncResult();
		wk->timeout_count = 0;
		switch (result){
		case 0:		case 1:		case 2:		case 3:		case 4:		
		case 5:		case 6:		case 7:
			OS_TPrintf(" query is success!\n");

			// 検索の結果の数を保存
			wk->SearchResult   = result;

			//Snd_SeStopBySeqNo( SE_GTC_SEARCH, 0 );
			//TODO

			// 下画面にＯＢＪを反映させる
			WorldTrade_SubLcdMatchObjAppear( wk, result, 1 );

			if(result==0){
				FriendViewButtonPrint(  wk->InfoWin[8], wk->MsgManager, FALSE, &wk->print );
			}else{
				FriendViewButtonPrint(  wk->InfoWin[8], wk->MsgManager, TRUE, &wk->print );
			}

			wk->subprocess_seq = SUBSEQ_SEARCH_RESULT_MESSAGE;
			break;
			


	// -----------------------------------------
	// 共通エラー処理
	// -----------------------------------------

		case DPW_TR_ERROR_SERVER_TIMEOUT :
		case DPW_TR_ERROR_DISCONNECTED:	
			// サーバーと通信できません→終了
			OS_TPrintf(" server full.\n");
			wk->subprocess_seq = SUBSEQ_SEARCH_ERROR_DICONNECT_MES1;
			break;
		case DPW_TR_ERROR_FATAL:			//!< 通信致命的エラー。電源の再投入が必要です
			// 即ふっとばし
			CommFatalErrorFunc_NoNumber();
			break;
	// -----------------------------------------

		// 検索結果取得失敗（「いなかった」ではない）リトライする
		case DPW_TR_ERROR_FAILURE:
		case DPW_TR_ERROR_CANCEL :
			wk->subprocess_seq = SUBSEQ_SERVER_QUERY_FAILURE;
			break;
		default:
			OS_TPrintf(" dpw result = %d.\n",result);
			break;
		}
		
	}
	else{
		wk->timeout_count++;
		if(wk->timeout_count == TIMEOUT_TIME){
			CommFatalErrorFunc_NoNumber();	//強制ふっとばし
		}
	}
	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   検索結果表示が必要な時は表示
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_SearchResultMessage( WORLDTRADE_WORK *wk )
{
	if(wk->SearchResult==0){
		// ヒット無しだった
		SubSeq_MessagePrint( wk, msg_gtc_01_015, 1, 0, 0x0f0f );
		WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );
		PMSND_PlaySE(SE_GTC_NG);
		
	}else{
		// 仕様では検索結果ありの時はメッセージ無しだったけど
		// 要望が来たので「みつかりました！」と表示
		SubSeq_MessagePrint( wk, msg_gtc_01_031, 1, 0, 0x0f0f );
		WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_SEARCH_RESULT_MESSAGE_WAIT );
		wk->wait = 0;
	}

	return SEQ_MAIN;

}

//------------------------------------------------------------------
/**
 * @brief   「みつかりました！」を1秒待つ
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_SearchResultMessageWait( WORLDTRADE_WORK *wk )
{
	wk->wait++;
	if(wk->wait>45){
		SubSeq_MessagePrint( wk, msg_gtc_01_033, 1, 0, 0x0f0f );
		WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );
		wk->SubLcdTouchOK = 1;
	}
	
	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   検索自体に失敗している（サーバーからの返事がタイムアウトした）
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_ServerQueryFailure( WORLDTRADE_WORK *wk )
{
	// 検索失敗なので、続けてもう一度同じ検索もできるようにする
	wk->SearchBackup.characterNo = 0;

	// 「ＧＴＳのかくにんにしっぱいしました」
	SubSeq_MessagePrint( wk, msg_gtc_01_039, 1, 0, 0x0f0f );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );
	PMSND_PlaySE(SE_GTC_NG);

	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   検索エラー（ＧＴＳ終了）その１
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_SearchErrorDisconnectMessage1( WORLDTRADE_WORK *wk )
{
	// 「ＧＴＳのかくにんにしっぱいしました」
	SubSeq_MessagePrint( wk, msg_gtc_error_004, 4, 0, 0x0f0f );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT_1MIN, SUBSEQ_SEARCH_ERROR_DICONNECT_MES2 );
	wk->wait =0;
	PMSND_PlaySE(SE_GTC_NG);

	return SEQ_MAIN;
	
}

//------------------------------------------------------------------
/**
 * @brief   検索エラー（ＧＴＳ終了）その２
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_SearchErrorDisconnectMessage2( WORLDTRADE_WORK *wk )
{
	// うけつけにもどります
	SubSeq_MessagePrint( wk, msg_gtc_error_006_02, 4, 0, 0x0f0f );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT_1MIN, SUBSEQ_END );
	WorldTrade_SubProcessChange( wk, WORLDTRADE_ENTER, MODE_CONNECT );
	PMSND_PlaySE(SE_GTC_NG);

	return SEQ_MAIN;
	
}


//------------------------------------------------------------------
/**
 * @brief   カーソルの位置番号を取得
 *
 * @param   wk		
 *
 * @retval  static		
 */
//------------------------------------------------------------------
static int CursorPosGet( WORLDTRADE_WORK *wk )
{
	int pos;
	// 現在のカーソルの位置を取得
	if(wk->dw->cursorSide==0){
		pos = wk->dw->leftCursorPos;
	}else{
		pos = wk->dw->rightCursorPos+4;
	}
	return pos;
}

//------------------------------------------------------------------
/**
 * @brief   カーソル移動
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void CursorMove( WORLDTRADE_WORK *wk )
{
	int pos ;
	if(GFL_UI_KEY_GetTrg() & PAD_KEY_UP){
		if(wk->dw->cursorSide==0){
			if(wk->dw->leftCursorPos>0){
				wk->dw->leftCursorPos--;
				PMSND_PlaySE(WORLDTRADE_MOVE_SE);
			}
		}else{
			if(wk->dw->rightCursorPos>0){
				PMSND_PlaySE(WORLDTRADE_MOVE_SE);
				wk->dw->rightCursorPos--;
			}
		}
		
	}else if(GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN){
		if(wk->dw->cursorSide==0){
			if(wk->dw->leftCursorPos<3){
				wk->dw->leftCursorPos++;
				PMSND_PlaySE(WORLDTRADE_MOVE_SE);
			}
		}else{
			if(wk->dw->rightCursorPos<2){
				PMSND_PlaySE(WORLDTRADE_MOVE_SE);
				wk->dw->rightCursorPos++;
			}
		}
		
	}else if(GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT){
		if(wk->dw->cursorSide != 1){
			PMSND_PlaySE(WORLDTRADE_MOVE_SE);
		}
		wk->dw->cursorSide = 1;
	}else if(GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT){
		if(wk->dw->cursorSide != 0){
			PMSND_PlaySE(WORLDTRADE_MOVE_SE);
		}
		wk->dw->cursorSide = 0;
	}
	
///	OS_Printf("cursor side = %d, left=%d, right=%d\n", wk->dw->cursorSide, wk->dw->leftCursorPos, wk->dw->rightCursorPos);


	// カーソル移動
	WorldTrade_CLACT_PosChange( wk->CursorActWork,  
								CursorPos[CursorPosGet( wk )][0], 
								CursorPos[CursorPosGet( wk )][1] );
	GFL_CLACT_WK_SetAnmSeq( wk->CursorActWork, CursorPos[CursorPosGet( wk )][2] );

}

static u8 Touch2CursorPos[][2]={
	{ 0, 0, },
	{ 0, 1, },
	{ 0, 2, },
	{ 0, 3, },
	{ 1, 0, },
	{ 1, 1, },
	{ 1, 2, },
};

//------------------------------------------------------------------
/**
 * @brief   タッチ選択の際にカーソルの位置を変更する
 *
 * @param   wk		
 * @param   touch		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void TouchCursorMove( WORLDTRADE_WORK *wk, int touch )
{
	if(Touch2CursorPos[touch][0]==0){
		wk->dw->cursorSide    = 0;
		wk->dw->leftCursorPos = Touch2CursorPos[touch][1];
	}else{
		wk->dw->cursorSide    = 1;
		wk->dw->rightCursorPos = Touch2CursorPos[touch][1];
	
	}

	// カーソル移動
	WorldTrade_CLACT_PosChange( wk->CursorActWork,  
								CursorPos[CursorPosGet( wk )][0], 
								CursorPos[CursorPosGet( wk )][1] );
	GFL_CLACT_WK_SetAnmSeq( wk->CursorActWork, CursorPos[CursorPosGet( wk )][2] );
}


//------------------------------------------------------------------
/**
 * @brief   サブプロセスシーケンス終了処理
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_End( WORLDTRADE_WORK *wk)
{
	if(wk->DrawOffset==0){
		WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
	}

#ifdef GTS_FADE_OSP
	OS_Printf( "******************** worldtrade_search.c [1076] M ********************\n" );
#endif
	wk->subprocess_seq = 0;
	
	return SEQ_FADEOUT;
}



//------------------------------------------------------------------
/**
 * @brief   ポケモンの名前を入力してください
 *
 * @param   wk		
 *
 * @retval  int 			
 */
//------------------------------------------------------------------
static int 	SubSeq_InputPokenameMessage( WORLDTRADE_WORK *wk)
{
	int i;
	SubSeq_MessagePrint( wk, msg_gtc_01_010, 1, 0, 0x0f0f );
//	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_HEADWORD_SELECT_LIST );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_POKENAME_SELECT_LIST );

	return SEQ_MAIN;
}










//------------------------------------------------------------------
/**
 * @brief   ポケモン名選択開始
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_PokenameSelectList( WORLDTRADE_WORK *wk)
{
	// ポケモン名リスト作成
#if 0
	wk->BmpListWork = WorldTrade_PokeNameListMake( wk,
							&wk->BmpMenuList, &wk->MenuWin[1], 
							wk->MsgManager, wk->MonsNameManager,wk->dw,
							wk->param->zukanwork );
	wk->listpos = 0xffff;
#endif

	// 条件入力システム開始
	WorldTrade_Input_Start( wk->WInputWork, MODE_POKEMON_NAME );
	BgBlendSet( PASSIVE_SCREEN_RATE );

	wk->subprocess_seq = SUBSEQ_POKENAME_SELECT_WAIT;

	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   名前入力
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_PokenameSelectWait( WORLDTRADE_WORK *wk)
{
	u32 result;
//	switch((result=WorldTrade_BmpMenuList_Main( wk->BmpListWork, &wk->listpos ))){
	switch((result=WorldTrade_Input_Main( wk->WInputWork ))){

	case BMPMENULIST_NULL:
		break;

	// キャンセル
	case BMPMENU_CANCEL:
//		BmpListExit( wk->BmpListWork, &wk->dw->nameListPos, &wk->dw->namePos );
//		BmpMenuWork_ListDelete( wk->BmpMenuList );
//		BmpMenuWinClear( &wk->MenuWin[1], WINDOW_TRANS_ON );
		BgBlendSet( 0 );
//		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		wk->subprocess_seq  = SUBSEQ_START;
		WorldTrade_SelectNameListBackup( &wk->selectListPos, wk->dw->headwordListPos+wk->dw->headwordPos, 
										wk->dw->nameListPos, wk->dw->namePos );

		break;
	// 選択
	default:
//		BmpListExit( wk->BmpListWork, &wk->dw->nameListPos, &wk->dw->namePos );
//		BmpMenuWork_ListDelete( wk->BmpMenuList );
//		BmpMenuWinClear( &wk->MenuWin[0], WINDOW_TRANS_ON );
//		BmpMenuWinClear( &wk->MenuWin[1], WINDOW_TRANS_ON );
//		GFL_BMPWIN_Delete( &wk->MenuWin[0] );
//		GFL_BMPWIN_Delete( &wk->MenuWin[1] );
		BgBlendSet( 0 );
		wk->Search.characterNo = result;
		wk->subprocess_seq     = SUBSEQ_START;
//		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);

		// 名前決定
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin[1]), 0x0000 );
		WorldTrade_PokeNamePrint( wk->InfoWin[1], wk->MonsNameManager, 
			result, SEARCH_INFO_PRINT_FLAG, 0, PRINTSYS_LSB_Make(1,2,0), &wk->print  );
		
		// 決定したポケモンの性別分布を取得
		wk->dw->sex_selection = PokePersonalParaGet(result,POKEPER_ID_sex);
		OS_Printf("");

		// カーソル位置保存
		WorldTrade_SelectNameListBackup( &wk->selectListPos, wk->dw->headwordListPos+wk->dw->headwordPos, 
										wk->dw->nameListPos, wk->dw->namePos );

		if(WorldTrade_SexSelectionCheck( &wk->Search, wk->dw->sex_selection )){
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin[3]), 0x0000 );
			WorldTrade_SexPrint( wk->InfoWin[3], wk->MsgManager, wk->Search.gender, 1, 0, SEARCH_INFO_PRINT_FLAG,  PRINTSYS_LSB_Make(1,2,0), &wk->print  );
			
		}

		break;

	}

	return SEQ_MAIN;
}
//------------------------------------------------------------------
/**
 * @brief   性別を選択してください
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_SexSelectMes( WORLDTRADE_WORK *wk)
{
	// ポケモンの性別を選んでください
	SubSeq_MessagePrint( wk, msg_gtc_01_011, 1, 0, 0x0f0f );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_SEX_SELECT_LIST );

	// 性別選択ウインドウ確保
//	GFL_BMPWIN_CreateFixPos( &wk->MenuWin[0], GFL_BG_FRAME0_M,
//		SELECT_MENU3_X,		SELECT_MENU3_Y,		SELECT_MENU3_SX, 		SELECT_MENU3_SY, 
//		WORLDTRADE_TALKFONT_PAL,  SELECT_MENU3_OFFSET );
//	GFL_BMP_Clear( &wk->MenuWin[0], 0x0000 );

	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   性別選択開始
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_SexSelectList( WORLDTRADE_WORK *wk)
{
//	wk->BmpListWork = WorldTrade_SexSelectListMake( &wk->BmpMenuList, &wk->MenuWin[0], wk->MsgManager );
//	wk->listpos = 0xffff;

	// 条件入力システム開始
	WorldTrade_Input_Start( wk->WInputWork, MODE_SEX );
	BgBlendSet( PASSIVE_SCREEN_RATE );

	wk->subprocess_seq = SUBSEQ_SEX_SELECT_WAIT;
	
	
	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   性別選択待ち
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_SexSelectWait( WORLDTRADE_WORK *wk)
{
	u32 result;
//	switch((result=WorldTrade_BmpMenuList_Main( wk->BmpListWork, &wk->listpos ))){
	switch((result=WorldTrade_Input_Main ( wk->WInputWork ))){
	// キャンセル
	case BMPMENU_CANCEL:
		BgBlendSet( 0 );
		BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_ON );
		GFL_BMPWIN_MakeTransWindow( wk->MsgWin );
//		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);

		wk->subprocess_seq = SUBSEQ_START;
		break;
	// 選択
	case PARA_MALE: case PARA_FEMALE: case PARA_UNK: 
		BgBlendSet( 0 );
//		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);

		wk->Search.gender	= result+1;
		wk->subprocess_seq  = SUBSEQ_START;

		// 性別決定
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin[3]), 0x0000 );
		WorldTrade_SexPrint( wk->InfoWin[3], wk->MsgManager, wk->Search.gender, 
									1, 0, SEARCH_INFO_PRINT_FLAG, PRINTSYS_LSB_Make(1,2,0), &wk->print  );
		break;
	}


	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   ほしいレベルを選択してください
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_LevelSelectMes( WORLDTRADE_WORK *wk)
{
	// ポケモンのレベルをきめてください
	SubSeq_MessagePrint( wk, msg_gtc_01_012, 1, 0, 0x0f0f );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_LEVEL_SELECT_LIST );

	// 性別選択ウインドウ確保
//	GFL_BMPWIN_CreateFixPos( &wk->MenuWin[0], GFL_BG_FRAME0_M,
//		SELECT_MENU4_X,		SELECT_MENU4_Y,		SELECT_MENU4_SX, 		SELECT_MENU4_SY, 
//		WORLDTRADE_TALKFONT_PAL,  SELECT_MENU4_OFFSET );
//	GFL_BMP_Clear( &wk->MenuWin[0], 0x0000 );

	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   レベル選択開始
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_LevelSelectList( WORLDTRADE_WORK *wk)
{
	// レベル選択リスト作成
//	wk->BmpListWork = WorldTrade_LevelListMake(&wk->BmpMenuList, &wk->MenuWin[0], wk->MsgManager,
//		LEVEL_PRINT_TBL_SEARCH );
	wk->listpos = 0xffff;

	// 条件入力システム開始
	WorldTrade_Input_Start( wk->WInputWork, MODE_LEVEL );
	BgBlendSet( PASSIVE_SCREEN_RATE );
	

	wk->subprocess_seq = SUBSEQ_LEVEL_SELECT_WAIT;

	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   レベル選択待ち
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_LevelSelectWait( WORLDTRADE_WORK *wk)
{
	u32 result;
//	switch((result=WorldTrade_BmpMenuList_Main( wk->BmpListWork, &wk->listpos ))){
	switch((result=WorldTrade_Input_Main( wk->WInputWork ))){
	case BMPMENULIST_NULL:
		break;
	// キャンセル
	case BMPMENU_CANCEL:
	case 11:
		BgBlendSet( 0 );
//		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);

		wk->subprocess_seq = SUBSEQ_START;
		break;
	// レベル指定決定
	default: 
		BgBlendSet( 0 );
//		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);

		WorldTrade_LevelMinMaxSet(&wk->Search, result, LEVEL_PRINT_TBL_SEARCH);
		wk->subprocess_seq  = SUBSEQ_START;

		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->InfoWin[5]), 0x0000 );
		// レベル指定決定
		WorldTrade_WantLevelPrint( wk->InfoWin[5], wk->MsgManager, result, SEARCH_INFO_PRINT_FLAG, 0, PRINTSYS_LSB_Make(1,2,0), LEVEL_PRINT_TBL_SEARCH, &wk->print );

		break;
	}


	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   ほしい国を選択してください
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_CountrySelectMes( WORLDTRADE_WORK *wk)
{
	// 国をきめてください
	SubSeq_MessagePrint( wk, msg_gtc_country_000, 1, 0, 0x0f0f );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_COUNTRY_SELECT_LIST );

	// 国選択ウインドウ確保
//	GFL_BMPWIN_CreateFixPos( &wk->MenuWin[0], GFL_BG_FRAME0_M,
//		SELECT_MENU5_X,		SELECT_MENU5_Y,		SELECT_MENU5_SX, 		SELECT_MENU5_SY, 
//		WORLDTRADE_TALKFONT_PAL,  SELECT_MENU5_OFFSET );
//	GFL_BMP_Clear( &wk->MenuWin[0], 0x0000 );

	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   国選択開始
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SUBSEQ_CountrySelectList( WORLDTRADE_WORK *wk)
{
	// 国選択リスト作成
//	wk->BmpListWork = WorldTrade_CountryListMake(&wk->BmpMenuList, &wk->MenuWin[0], 
//		wk->CountryNameManager, wk->MsgManager);
	wk->listpos = 0xffff;

	// 条件入力システム開始
	WorldTrade_Input_Start( wk->WInputWork, MODE_NATION );
	BgBlendSet( PASSIVE_SCREEN_RATE );

	wk->subprocess_seq = SUBSEQ_COUNTRY_SELECT_WAIT;

	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   国選択待ち
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
///国選択ウィンドウで「もどる」のリスト位置
#define COUNTRY_SELECT_WIN_CANCEL	(CountryListTblNum + 1)	//(country233 + 1)

static int SUBSEQ_CountrySelectWait( WORLDTRADE_WORK *wk)
{
	u32 result;
	
//	result=WorldTrade_BmpMenuList_Main( wk->BmpListWork, &wk->listpos );
	result=WorldTrade_Input_Main( wk->WInputWork );
	if(result == BMPMENULIST_NULL){
		;
	}
	else if(result == BMPMENU_CANCEL || result == COUNTRY_SELECT_WIN_CANCEL){
		// キャンセル
		BgBlendSet( 0 );
		BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_ON );
		GFL_BMPWIN_MakeTransWindow( wk->MsgWin );
//		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);

		wk->subprocess_seq = SUBSEQ_START;
	}
	else{
		// 国指定決定
		BgBlendSet( 0 );
//		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);

		OS_Printf("input -> country = %d\n", result);
		WorldTrade_CountryCodeSet(wk, result);
		wk->subprocess_seq  = SUBSEQ_START;

		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->CountryWin[1]), 0x0000 );
		// 国指定決定
		WorldTrade_CountryPrint( wk->CountryWin[1], wk->CountryNameManager, wk->MsgManager,
			wk->country_code, SEARCH_INFO_PRINT_FLAG, 0, PRINTSYS_LSB_Make(1,2,0), &wk->print );
	}

	return SEQ_MAIN;
}








//------------------------------------------------------------------
/**
 * @brief   はい・いいえ
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_YesNo( WORLDTRADE_WORK *wk)
{
//	wk->YesNoMenuWork = WorldTrade_BmpWinYesNoMake( WORLDTRADE_YESNO_PY1, YESNO_OFFSET );
	wk->tss = WorldTrade_TouchWinYesNoMake(  WORLDTRADE_YESNO_PY1, YESNO_OFFSET, 3, 1 );
	wk->subprocess_seq = SUBSEQ_YESNO_SELECT;

	return SEQ_MAIN;
	
}


//------------------------------------------------------------------
/**
 * @brief   はい・いいえ選択
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_YesNoSelect( WORLDTRADE_WORK *wk)
{

	u32 ret = WorldTrade_TouchSwMain(wk);

	if(ret==TOUCH_SW_RET_YES){
		// タイトルメニューへ
		TOUCH_SW_FreeWork( wk->tss );
		wk->subprocess_seq  = SUBSEQ_END;
		WorldTrade_SubProcessChange( wk, WORLDTRADE_TITLE, 0 );

		// 下画面のOBJを隠す
		WorldTrade_SubLcdMatchObjHide( wk );

		// 検索結果人数はクリアする
		wk->SearchResult = 0;
	}else if(ret==TOUCH_SW_RET_NO){
		// もういっかいトライ
		TOUCH_SW_FreeWork( wk->tss );
		wk->subprocess_seq = SUBSEQ_START;
	}

	
/*	
	int ret = BmpYesNoSelectMain( wk->YesNoMenuWork, HEAPID_WORLDTRADE );

	if(ret!=BMPMENU_NULL){
		if(ret==BMPMENU_CANCEL){
			// もういっかいトライ
			wk->subprocess_seq = SUBSEQ_START;
		}else{
			// タイトルメニューへ
			wk->subprocess_seq  = SUBSEQ_END;
			WorldTrade_SubProcessChange( wk, WORLDTRADE_TITLE, 0 );

			// 下画面のOBJを隠す
			WorldTrade_SubLcdMatchObjHide( wk );

			// 検索結果人数はクリアする
			wk->SearchResult = 0;

		}
	}
*/
	return SEQ_MAIN;
	
}




//------------------------------------------------------------------
/**
 * @brief   情報ページを切り替える
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_PageChange( WORLDTRADE_WORK *wk)
{
//	ChangePage( wk );
	wk->subprocess_seq = SUBSEQ_MAIN;

	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   検索エラーメッセージ表示
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_SearchErrorMessage(WORLDTRADE_WORK *wk)
{
	SubSeq_MessagePrint( wk, msg_gtc_error_003, 1, 0, 0x0f0f );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );
	
	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   会話終了を待って次のシーケンスへ
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_MessageWait( WORLDTRADE_WORK *wk )
{
	if( GF_MSG_PrintEndCheck( &wk->print )==0){
		wk->subprocess_seq = wk->subprocess_nextseq;
	}
	return SEQ_MAIN;

}

//------------------------------------------------------------------
/**
 * @brief   メッセージ終了後１秒待つ
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_MessageWait1Min( WORLDTRADE_WORK *wk )
{
	if( GF_MSG_PrintEndCheck( &wk->print )==0){
		wk->wait++;
		if(wk->wait>45){
			wk->wait = 0;
			wk->subprocess_seq = wk->subprocess_nextseq;
		}
	}
	return SEQ_MAIN;


}


//------------------------------------------------------------------
/**
 * @brief   相手のポケモンを見る画面に移行する
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_ExchangeScreen1( WORLDTRADE_WORK *wk )
{
	// フェード待ちと同時に画面オフセット移動
	wk->DrawOffset++;
	if(WIPE_SYS_EndCheck()){
		GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );
		wk->DrawOffset = -16;
//		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
//						WIPE_FADE_BLACK, EXCHANGE_SCREEN_SYNC, 1, HEAPID_WORLDTRADE );

		if(wk->SubLcdTouchOK){
			// 必ず中央を指定する（＝０）
			int result=0;
				GFL_CLACT_WK_SetAnmSeq( wk->SubActWork[result+1], 16+result*4 );
				wk->subprocess_seq     = SUBSEQ_END; 
				WorldTrade_SubProcessChange( wk, WORLDTRADE_PARTNER, MODE_FROM_SEARCH );
				wk->TouchTrainerPos = result;
//				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
		}
		GFL_CLACT_WK_SetDrawEnable( wk->PromptDsActWork, 0 );


//		wk->subprocess_seq = SUBSEQ_EXCHANGE_SCREEN2;


		// この先はworldtrade_partner.cで行う

	}
	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   ポケモン見る画面から戻ってくる
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_ReturnScreen2( WORLDTRADE_WORK *wk )
{
	// フェード待ちと同時に画面オフセット移動
	wk->DrawOffset--;
	if(WIPE_SYS_EndCheck()){
		wk->DrawOffset = 0;

		wk->subprocess_seq = SUBSEQ_MAIN;
	}
	return SEQ_MAIN;
}


static int SubSeq_ExchangeMain( WORLDTRADE_WORK *wk )
{
	if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
						WIPE_FADE_BLACK, EXCHANGE_SCREEN_SYNC, 1, HEAPID_WORLDTRADE );
//		wk->subprocess_seq = SUBSEQ_RETURN_SCREEN1;
	}else{
		// タッチパネルチェック
		int result = WorldTrade_SubLcdObjHitCheck( wk->SearchResult );
		if(wk->SubLcdTouchOK){
			if(result>=0){
				GFL_CLACT_WK_SetAnmSeq( wk->SubActWork[result+1], 16+result*4 );
				wk->subprocess_seq     = SUBSEQ_END; 
				WorldTrade_SubProcessChange( wk, WORLDTRADE_PARTNER, 0 );
				wk->TouchTrainerPos = result;
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			}
		}
	
	}
	return SEQ_MAIN;
}



//------------------------------------------------------------------
/**
 * @brief   会話ウインドウ表示
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void SubSeq_MessagePrint( WORLDTRADE_WORK *wk, int msgno, int wait, int flag, u16 dat )
{
	// 文字列取得
	STRBUF *tempbuf;
	
//	tempbuf = GFL_STR_CreateBuffer( TALK_MESSAGE_BUF_NUM, HEAPID_WORLDTRADE );
	GFL_MSG_GetString(  wk->MsgManager, msgno, wk->TalkString );
//	WORDSET_ExpandStr( wk->WordSet, wk->TalkString, tempbuf );
//	GFL_STR_DeleteBuffer(tempbuf);

	// 会話ウインドウ枠描画
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin),  0x0f0f );
	BmpWinFrame_Write( wk->MsgWin, WINDOW_TRANS_ON, WORLDTRADE_MESFRAME_CHR, WORLDTRADE_MESFRAME_PAL );

	// 文字列描画開始
	GF_STR_PrintSimple( wk->MsgWin, FONT_TALK, wk->TalkString, 0, 0, &wk->print );
	GFL_BMPWIN_MakeTransWindow( wk->MsgWin );


}

//------------------------------------------------------------------
/**
 * @brief   ほしいポケモン情報の「ラベル」描画
 *
 * @param   win		
 * @param   MsgManager		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void WantLabelPrint( GFL_BMPWIN *win[], GFL_BMPWIN *country_win[], GFL_MSGDATA *MsgManager, WT_PRINT *print )
{
	STRBUF *strbuf, *levelbuf, *sexbuf;

	//「ほしいポケモン」描画
	strbuf = GFL_MSG_CreateString( MsgManager, msg_gtc_03_002 );
	WorldTrade_SysPrint( win[0], strbuf,    0, 0, 0, PRINTSYS_LSB_Make(15,2,0),print );
	GFL_STR_DeleteBuffer(strbuf);

	// せいべつ
	sexbuf = GFL_MSG_CreateString( MsgManager, msg_gtc_03_004  );
	WorldTrade_SysPrint( win[2], sexbuf,    0, 0, 0, PRINTSYS_LSB_Make(15,2,0),print );
	GFL_STR_DeleteBuffer(sexbuf);

	// レベル
	levelbuf = GFL_MSG_CreateString( MsgManager, msg_gtc_03_006 );
	WorldTrade_SysPrint( win[4], levelbuf,  0, 0, 0, PRINTSYS_LSB_Make(15,2,0),print );
	GFL_STR_DeleteBuffer(levelbuf);

	// 国
	levelbuf = GFL_MSG_CreateString( MsgManager, msg_gtc_search_013 );
	WorldTrade_SysPrint( country_win[0], levelbuf,  0, 0, 0, PRINTSYS_LSB_Make(15,2,0),print );
	GFL_STR_DeleteBuffer(levelbuf);

	//「さがす」描画
	strbuf = GFL_MSG_CreateString( MsgManager, msg_gtc_03_008 );
	WorldTrade_TouchPrint( win[6], strbuf,    0, 0, 0, PRINTSYS_LSB_Make(15,2,0),print );
	GFL_STR_DeleteBuffer(strbuf);

	//「もどる」描画
	strbuf = GFL_MSG_CreateString( MsgManager, msg_gtc_03_011 );
	WorldTrade_TouchPrint( win[7], strbuf,    0, 0, 0, PRINTSYS_LSB_Make(15,2,0),print );
	GFL_STR_DeleteBuffer(strbuf);

}

//------------------------------------------------------------------
/**
 * @brief   「あいてをみる」ボタンの文字列描画
 *
 * @param  none
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void FriendViewButtonPrint( GFL_BMPWIN *win, GFL_MSGDATA *MsgManager, int flag, WT_PRINT *print )
{
	u8 pal;
	STRBUF *strbuf;
	PRINTSYS_LSB color = PRINTSYS_LSB_Make(1,2,0);
	if(flag){
		color = PRINTSYS_LSB_Make(15,2,0);
		pal = FRD_VIEW_OK_PAL;
	}else{
		pal = FRD_VIEW_NG_PAL;
	}
	//スクリーンパレット書き換え
	GFL_BG_ChangeScreenPalette( GFL_BG_FRAME1_M,
			         	 FRD_VIEW_BTN_PX, FRD_VIEW_BTN_PY, FRD_VIEW_BTN_SX, FRD_VIEW_BTN_SY, pal );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
	//「あいてをみる」描画
	strbuf = GFL_MSG_CreateString( MsgManager, msg_gtc_03_010 );
	WorldTrade_TouchPrint( win, strbuf,    0, 0, 0, color, print );
	GFL_STR_DeleteBuffer(strbuf);

}

//------------------------------------------------------------------
/**
 * @brief   検索データがまったく同じか比較
 *
 * @param   s1		
 * @param   s2		
 *
 * @retval  int		同じなら１
 */
//------------------------------------------------------------------
static int DpwSerachCompare( const Dpw_Tr_PokemonSearchData *s1, const Dpw_Tr_PokemonSearchData *s2 , int country_code1, int country_code2)
{
	if( s1->characterNo==s2->characterNo && s1->gender==s2->gender
	&&	s1->level_min==s2->level_min     && s1->level_max==s2->level_max
	&&  country_code1 == country_code2){
		return 1;
	}
	
	return 0;
}



//------------------------------------------------------------------
/**
 * @brief   上下画面全体をズラす（VBlank内で呼び出し）
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void SlideScreenVFunc( void *p )
{
	WORLDTRADE_WORK *wk = (WORLDTRADE_WORK*)p;
	int i;
	
	GFL_BG_SetScroll(  GFL_BG_FRAME0_M, GFL_BG_SCROLL_Y_SET, wk->DrawOffset );
	GFL_BG_SetScroll(  GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, wk->DrawOffset );
	GFL_BG_SetScroll(  GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, wk->DrawOffset );
	GFL_BG_SetScroll(  GFL_BG_FRAME3_M, GFL_BG_SCROLL_Y_SET, wk->DrawOffset );

	GFL_BG_SetScroll(  GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, -wk->DrawOffset );
	GFL_BG_SetScroll(  GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, -wk->DrawOffset );
	GFL_BG_SetScroll(  GFL_BG_FRAME2_S, GFL_BG_SCROLL_Y_SET, -wk->DrawOffset );
	GFL_BG_SetScroll(  GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, -wk->DrawOffset );

}


//------------------------------------------------------------------
/**
 * @brief   BG輝度設定
 *
 * @param   rate		輝度( 0 -31 ) 0で輝度処理OFF
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void BgBlendSet( int rate  )
{
	if(rate!=0){
		G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG3, rate );
	}else{
		G2_BlendNone();
	}
}
