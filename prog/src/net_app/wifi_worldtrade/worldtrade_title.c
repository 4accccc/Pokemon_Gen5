//============================================================================================
/**
 * @file	worldtrade_title.c
 * @brief	世界交換タイトル画面処理
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

#include "msg/msg_wifi_lobby.h"
#include "msg/msg_wifi_gts.h"


#include "worldtrade.naix"			// グラフィックアーカイブ定義

#define SEVERNO_MESSAGE_WAIT	( 45 )

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
static int SubSeq_Opening( WORLDTRADE_WORK *wk);
static int SubSeq_OpeningMessage( WORLDTRADE_WORK *wk );
static int SubSeq_OpeningFade( WORLDTRADE_WORK *wk );
static int SubSeq_OpeningFadeWait( WORLDTRADE_WORK *wk );
static int SubSeq_OpeningWait( WORLDTRADE_WORK *wk);
static int SubSeq_Start( WORLDTRADE_WORK *wk);
static int SubSeq_Main( WORLDTRADE_WORK *wk);
static int SubSeq_End( WORLDTRADE_WORK *wk);
static int SubSeq_YesNo( WORLDTRADE_WORK *wk);
static int SubSeq_YesNoSelect( WORLDTRADE_WORK *wk);
static int SubSeq_MessageWait( WORLDTRADE_WORK *wk );
static int SubSeq_Message1MinWait( WORLDTRADE_WORK *wk );
static int SubSeq_EndDemo( WORLDTRADE_WORK *wk );
static int SubSeq_EndDemoWait( WORLDTRADE_WORK *wk );

static void TitleMenuPrint( WORLDTRADE_WORK *wk );
static void SubSeq_MessagePrint( WORLDTRADE_WORK *wk, int msgno, int wait, int flag, u16 dat );
static void SubSeq_TalkPrint( WORLDTRADE_WORK *wk, int msgno, int wait, int flag, u16 dat );
static void DemoBgSet( WORLDTRADE_WORK *wk );
static  u32 TouchPanelFunc( WORLDTRADE_WORK *wk );
static void DecideFunc( WORLDTRADE_WORK *wk, int decide );


enum{
	SUBSEQ_OPENING=0,
	SUBSEQ_OPENING_MESSAGE,
	SUBSEQ_OPENING_FADE,
	SUBSEQ_OPENING_FADE_WAIT,
	SUBSEQ_OPENING_WAIT,
	SUBSEQ_START,
	SUBSEQ_MAIN,
	SUBSEQ_END_DEMO,
	SUBSEQ_END_DEMO_WAIT,
	SUBSEQ_END,
	SUBSEQ_MES_WAIT,
	SUBSEQ_MES_1MIN_WAIT,
	SUBSEQ_YESNO,
	SUBSEQ_YESNO_SELECT,
};

static int (*Functable[])( WORLDTRADE_WORK *wk ) = {
	SubSeq_Opening,				// SUBSEQ_OPENING=0,
	SubSeq_OpeningMessage,      // SUBSEQ_OPENING_MESSAGE,
	SubSeq_OpeningFade,         // SUBSEQ_OPENING_FADE,
	SubSeq_OpeningFadeWait,     // SUBSEQ_OPENING_FADE_WAIT,
	SubSeq_OpeningWait,         // SUBSEQ_OPENING_WAIT,
	SubSeq_Start,			    // SUBSEQ_START,
	SubSeq_Main,                // SUBSEQ_MAIN,
	SubSeq_EndDemo,             // SUBSEQ_END_DEMO,
	SubSeq_EndDemoWait,         // SUBSEQ_END_DEMO_WAIT,
	SubSeq_End,                 // SUBSEQ_END,
	SubSeq_MessageWait,         // SUBSEQ_MES_WAIT,
	SubSeq_Message1MinWait,     // SUBSEQ_MES_1MIN_WAIT,
	SubSeq_YesNo,			    // SUBSEQ_YESNO,
	SubSeq_YesNoSelect,		    // SUBSEQ_YESNO_SELECT,
	

};

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
int WorldTrade_Title_Init(WORLDTRADE_WORK *wk, int seq)
{
	// ワーク初期化
	InitWork( wk );
	
	// メイン画面が下に出力されるようにする
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
	
	// BG設定
	BgInit();

	// BGグラフィック転送
	BgGraphicSet( wk );

	// BMPWIN確保
	BmpWinInit( wk );

	SetCellActor(wk);

	// 通信状態を確認してアイコンの表示を変える
  WorldTrade_WifiIconAdd( wk );

	// メニューの項目を描画
	TitleMenuPrint( wk );

	// サウンドデータロード(フィールド)
	//Snd_DataSetByScene( SND_SCENE_FIELD, SEQ_BLD_BLD_GTC, 1 );


	Snd_DataSetByScene( SND_SCENE_WIFI_WORLD_TRADE, SEQ_GS_BLD_GTC, 1 );


	// プラチナで、マップの曲と、画面の曲が変更になったので、
	// 画面から、フィールドに戻る時のためにシーンを変更

	if(wk->OpeningFlag==0){
		// 主人公登場デモ用の上画面設定
		DemoBgSet( wk );


		// 最初の１回目
		// ワイプフェード開始（サブ画面だけ）
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
#ifdef GTS_FADE_OSP
		OS_Printf( "******************** worldtrade_title.c [159] MS ********************\n" );
#endif

		wk->subprocess_seq = SUBSEQ_OPENING;
		wk->OpeningFlag     = 1;

		// 下画面に主人公を登場させる
		WorldTrade_HeroDemo( wk, MyStatus_GetMySex(wk->param->mystatus) );
	}else{
		// ２回目以降
		// ワイプフェード開始（両画面）
		if( wk->sub_out_flg == 1 && GXS_GetMasterBrightness() != 0 ){
            WorldTrade_SetPartnerExchangePos(wk);
			WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
#ifdef GTS_FADE_OSP
			OS_Printf( "******************** worldtrade_title.c [171] MS ********************\n" );
#endif
		}else{
			WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
#ifdef GTS_FADE_OSP
			OS_Printf( "******************** worldtrade_title.c [175] M ********************\n" );
#endif
		}
		wk->subprocess_seq = SUBSEQ_START;
	}
	wk->sub_out_flg = 0;
	OS_TPrintf("opening flag = %d\n", wk->OpeningFlag);
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
int WorldTrade_Title_Main(WORLDTRADE_WORK *wk, int seq)
{
	int ret;

	ret = (*Functable[wk->subprocess_seq])( wk );

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
int WorldTrade_Title_End(WORLDTRADE_WORK *wk, int seq)
{
	//WirelessIconEasyEnd();

	DelCellActor(wk);

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
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000,  GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(  GFL_BG_FRAME0_M );
		GFL_BG_SetVisible(GFL_BG_FRAME0_M, TRUE);
	}

	// メイン画面メニュー面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,GX_BG_EXTPLTT_01,
			1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME1_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible(GFL_BG_FRAME1_M, TRUE);
	}

	// メイン画面背景面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,GX_BG_EXTPLTT_01,
			1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME2_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible(GFL_BG_FRAME2_M, TRUE);
	}

	// サブ画面初期化
	WorldTrade_SubLcdBgInit(  0, 0 );



	GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 32, 0, HEAPID_WORLDTRADE );

	GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON );	//メイン画面OBJ面ＯＮ
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );	//サブ画面OBJ面ＯＮ

}

//==============================================================================
/**
 * @brief   サブ画面初期化(できる限り共通処理としてサブ画面のBG初期化を行う）
 *
 * @param   ini		GF_BGL_INI
 *
 * @retval  none
 */
//==============================================================================
void WorldTrade_SubLcdBgInit( int sub_bg1_y_offset, BOOL sub_bg2_no_clear )
{
	// サブ画面文字版0
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000,GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME0_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(  GFL_BG_FRAME0_S );
		GFL_BG_SetVisible(GFL_BG_FRAME0_S, TRUE);

	}

	// サブ画面プレート面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x14000,GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME2_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
		if(sub_bg2_no_clear == FALSE){
    		GFL_BG_ClearScreen(  GFL_BG_FRAME2_S );
    	}
		GFL_BG_SetVisible(GFL_BG_FRAME2_S, TRUE);

	}


	// サブ画面背景BG1( この面は256色 )
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000,GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			2, 0, 0, FALSE
		};
		TextBgCntDat.scrollY = sub_bg1_y_offset;
		GFL_BG_SetBGControl(  GFL_BG_FRAME1_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible(GFL_BG_FRAME1_S, TRUE);
	}
	GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 32, 0, HEAPID_WORLDTRADE );

	// サブ画面文字版その2
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x10000,GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl(  GFL_BG_FRAME3_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(  GFL_BG_FRAME3_S );
		GFL_BG_SetVisible(GFL_BG_FRAME3_S, TRUE);

	}
}

//==============================================================================
/**
 * @brief   サブ画面BG情報解放
 *
 * @param   ini		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_SubLcdBgExit( void )
{
	// WorldTrade_SubLcdBgInit で初期化したサブBG面を解放する

	GFL_BG_FreeBGControl(  GFL_BG_FRAME2_S );
	GFL_BG_FreeBGControl(  GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl(  GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl(  GFL_BG_FRAME3_S );	
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
	WorldTrade_SubLcdBgExit();

	// メイン画面ＢＧ情報解放
	GFL_BG_FreeBGControl(  GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl(  GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl(  GFL_BG_FRAME0_M );
}


//------------------------------------------------------------------
/**
 * @brief   主人公登場デモ時は上画面はOBJ以外OFFにする
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void DemoBgSet( WORLDTRADE_WORK *wk )
{
	GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG0, VISIBLE_OFF );	//メイン画面OBJ面ＯＮ
	GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG1, VISIBLE_OFF );	//メイン画面OBJ面ＯＮ
	GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG2, VISIBLE_OFF );	//メイン画面OBJ面ＯＮ

	GFL_CLACT_WK_SetDrawEnable( wk->CursorActWork, 0 );
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
	ARCHANDLE* p_handle;

	p_handle = GFL_ARC_OpenDataHandle( ARCID_WORLDTRADE_GRA, HEAPID_WORLDTRADE );

	// 上画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_worldtrade_title_nclr, PALTYPE_MAIN_BG, 0, 16*3*2,  HEAPID_WORLDTRADE);


	
	// 会話フォントパレット転送
	TalkFontPaletteLoad( PALTYPE_MAIN_BG, WORLDTRADE_TALKFONT_PAL*0x20, HEAPID_WORLDTRADE );
   //	TalkFontPaletteLoad( PALTYPE_SUB_BG,  WORLDTRADE_TALKFONT_PAL*0x20, HEAPID_WORLDTRADE );


	// 会話ウインドウグラフィック転送
	
	BmpWinFrame_GraphicSet(	 GFL_BG_FRAME0_M, WORLDTRADE_MESFRAME_CHR, 
						WORLDTRADE_MESFRAME_PAL,  CONFIG_GetWindowType(wk->param->config), HEAPID_WORLDTRADE );


	BmpWinFrame_GraphicSet(	 GFL_BG_FRAME0_M, WORLDTRADE_MENUFRAME_CHR,
						WORLDTRADE_MENUFRAME_PAL, 0, HEAPID_WORLDTRADE );



	// メイン画面BG1キャラ転送
	GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_worldtrade_title_lz_ncgr,  GFL_BG_FRAME1_M, 0, 16*6*0x20, 1, HEAPID_WORLDTRADE);


	// メイン画面BG1スクリーン転送
	GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_worldtrade_title_menu_lz_nscr,  GFL_BG_FRAME1_M, 0, 32*24*2, 1, HEAPID_WORLDTRADE);



	// メイン画面BG2スクリーン転送
	GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_worldtrade_title_base_lz_nscr,  GFL_BG_FRAME2_M, 0, 32*24*2, 1, HEAPID_WORLDTRADE);


	// 下画面256色背景転送
	WorldTrade_SubLcdBgGraphicSet( wk );

	if(wk->OpeningFlag){
		WorldTrade_SubLcdWinGraphicSet( wk );   // トレードルームウインドウ転送
		//サブBG0を表示
		GFL_DISP_GXS_SetVisibleControl(  GX_PLANEMASK_BG0, VISIBLE_ON );	//サブ画面OBJ面ON
	}else{
		//はじめはサブBG0を非表示
		GFL_DISP_GXS_SetVisibleControl(  GX_PLANEMASK_BG0, VISIBLE_OFF );	//サブ画面OBJ面OFF
	}


	GFL_ARC_CloseDataHandle( p_handle );
}


static const u16 CursorPosTbl[][2]={
	{128,96-5*8},
	{128,96},
	{128,96+5*8},
};

#define CLACT_TITLE_CURSOR_ACTNO ( 43 ) // 1

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
	GFL_CLWK_DATA	add;

typedef struct {
	s16	pos_x;				// ｘ座標
	s16 pos_y;				// ｙ座標
	u16 anmseq;				// アニメーションシーケンス
	u8	softpri;			// ソフト優先順位	0>0xff
	u8	bgpri;				// BG優先順位
} GFL_CLWK_DATA;
#if 0
	//登録情報格納
	CLACT_ADD add;
	{
		CLACT_HEADER header;
		// セルアクターヘッダ作成
		GFL_CLACT_WK_SetCellResData or GFL_CLACT_WK_SetTrCellResData or GFL_CLACT_WK_SetMCellResData(&header, 0, 0, 0, 0, CLACT_U_HEADER_DATA_NONE, CLACT_U_HEADER_DATA_NONE,
				0, 1,
				wk->resMan[CLACT_U_CHAR_RES],
				wk->resMan[CLACT_U_PLTT_RES],
				wk->resMan[CLACT_U_CELL_RES],
				wk->resMan[CLACT_U_CELLANM_RES],
				NULL,NULL);
		WorldTrade_MakeCLACT( &add,  wk, &header, NNS_G2D_VRAM_TYPE_2DMAIN );
	}
#endif
	add.pos_x = CursorPosTbl[wk->TitleCursorPos][0];
	add.pos_y = CursorPosTbl[wk->TitleCursorPos][1];
	add.anmseq	= 0;
	add.softpri	= 0;
	add.bgpri		= 0;
	wk->CursorActWork = GFL_CLACT_WK_Create( wk->clactSet, 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
	GFL_CLACT_WK_SetAutoAnmFlag(wk->CursorActWork,1);
	GFL_CLACT_WK_SetAnmSeq( wk->CursorActWork, CLACT_TITLE_CURSOR_ACTNO );
}

//------------------------------------------------------------------
/**
 * @brief   表示セルアクターの解放
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void DelCellActor( WORLDTRADE_WORK *wk )
{
	GFL_CLACT_WK_Remove(wk->CursorActWork);
}

#define MENU_TEXT_X		(  9 )
#define MENU_TEXT_Y		(  6 )
#define MENU_TEXT_SX	( 15 )
#define MENU_TEXT_SY	(  2 )

#define WT_MENU_MAX_NUM	(  3 )

#define MENU_MESSAGE_OFFSET    ( 16*6+1 )	// これだけBG1面なので

#define TITLE_MESSAGE_OFFSET   ( WORLDTRADE_MENUFRAME_CHR + MENU_WIN_CGX_SIZ )
#define LINE_MESSAGE_OFFSET    ( TITLE_MESSAGE_OFFSET     + TITLE_TEXT_SX*TITLE_TEXT_SY )
#define TALK_MESSAGE_OFFSET    ( LINE_MESSAGE_OFFSET      + LINE_TEXT_SX*LINE_TEXT_SY )
#define YESNO_OFFSET 		   ( TALK_MESSAGE_OFFSET      + TALK_WIN_SX*TALK_WIN_SY )



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

	// BG0面BMPWINタイトルウインドウ確保・描画
	
	wk->TitleWin	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME1_M,
	TITLE_TEXT_X, TITLE_TEXT_Y, TITLE_TEXT_SX, TITLE_TEXT_SY, WORLDTRADE_TALKFONT_PAL,  TITLE_MESSAGE_OFFSET );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->TitleWin), 0x0000 );
	GFL_BMPWIN_MakeTransWindow( wk->TitleWin );
	
	// POKMON GLOBAL TRADING SYSTEM
	GF_STR_PrintColor(	wk->TitleWin, FONT_TALK, wk->TitleString, 
				0, 2, MSG_ALLPUT, 
				PRINTSYS_LSB_Make(15,6,0),&wk->print);

	// BG0面BMPWINメニューテキストウインドウ確保・描画
	{
		int i;
		for(i=0;i<WT_MENU_MAX_NUM;i++){
			wk->MenuWin[i]	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME1_M,
			MENU_TEXT_X, MENU_TEXT_Y+i*5, MENU_TEXT_SX, MENU_TEXT_SY, 1,  
			MENU_MESSAGE_OFFSET + (MENU_TEXT_SX*MENU_TEXT_SY)*i );
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuWin[i]), 0x0000 );
			GFL_BMPWIN_MakeTransWindow( wk->MenuWin[i] );
		}
	}
	
	// 1行会話ウインドウ
	wk->MsgWin	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME0_M,
		LINE_TEXT_X, LINE_TEXT_Y, LINE_TEXT_SX, LINE_TEXT_SY, 
		WORLDTRADE_TALKFONT_PAL,  LINE_MESSAGE_OFFSET );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin), 0x0f0f );


	// 通常会話ウインドウ
	wk->TalkWin	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME0_M,
		TALK_WIN_X, TALK_WIN_Y, TALK_WIN_SX, TALK_WIN_SY, 
		WORLDTRADE_TALKFONT_PAL,  TALK_MESSAGE_OFFSET );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->TalkWin), 0x0f0f );

	// サブ画面のGTS説明用BMPWINを確保する
	WorldTrade_SubLcdExpainPut( wk, EXPLAIN_GTS );


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
	int i;

  //BMPを消す前に残ったストリームを消去
  WT_PRINT_ClearBuffer( &wk->print );

	GFL_BMPWIN_Delete( wk->ExplainWin );
	OS_Printf("説明ウインドウ解放\n");

	GFL_BMPWIN_Delete( wk->TalkWin );
	GFL_BMPWIN_Delete( wk->MsgWin  );

	for(i=0;i<WT_MENU_MAX_NUM;i++){
		GFL_BMPWIN_Delete( wk->MenuWin[i] );
	}

	GFL_BMPWIN_Delete( wk->TitleWin );


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


	// 文字列バッファ作成
	wk->TalkString  = GFL_STR_CreateBuffer( TALK_MESSAGE_BUF_NUM, HEAPID_WORLDTRADE );

	// POKeMON GLOBAL TRADING SYSTEM
	wk->TitleString = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_02_001 );

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
 * @brief   最初に世界交換メニュー画面に来たとき
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_Opening( WORLDTRADE_WORK *wk)
{
	// 主人公降りてくるデモが終わるのを待つ
	if(wk->demo_end){
		GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG0, VISIBLE_OFF );	//メイン画面OBJ面ＯＮ
		GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG1, VISIBLE_OFF );	//メイン画面OBJ面ＯＮ
		GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG2, VISIBLE_OFF );	//メイン画面OBJ面ＯＮ
//		GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_OFF );	//メイン画面OBJ面ＯＮ
		// 終わったらサーバーチェックへ
		WorldTrade_SubProcessChange( wk, WORLDTRADE_UPLOAD, MODE_SERVER_CHECK );
		wk->sub_returnprocess = WORLDTRADE_TITLE;
		wk->subprocess_seq  = SUBSEQ_END;
	}

	return SEQ_MAIN;

}

//------------------------------------------------------------------
/**
 * @brief   ???
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_OpeningMessage( WORLDTRADE_WORK *wk )
{
//	SubSeq_MessagePrint( wk, msg_gtc_01_001, 1, 0, 0x0f0f );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_OPENING_FADE );
	
	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   上画面フェードイン
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_OpeningFade( WORLDTRADE_WORK *wk )
{
	WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
#ifdef GTS_FADE_OSP
	OS_Printf( "******************** worldtrade_title.c [654] M ********************\n" );
#endif
	wk->subprocess_seq = SUBSEQ_OPENING_FADE_WAIT;
	
	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   上画面フェード待ち
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_OpeningFadeWait( WORLDTRADE_WORK *wk )
{
	if(WIPE_SYS_EndCheck()){
		wk->subprocess_seq = SUBSEQ_START;
	}
	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   フェード待ち
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_OpeningWait( WORLDTRADE_WORK *wk)
{
	if(WIPE_SYS_EndCheck()){
		wk->subprocess_seq = SUBSEQ_START;
	}
	return SEQ_MAIN;
}


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
	// おさがしですか？おあずけですか？
	SubSeq_MessagePrint( wk, msg_gtc_01_005, 1, 0, 0x0f0f );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );
	GFL_CLACT_WK_SetAutoAnmFlag(wk->CursorActWork,1);
	WorldTrade_BoxPokeNumGetStart( wk );

	return SEQ_MAIN;
}

static const GFL_UI_TP_HITTBL TouchTable[]={
	{  40,  71,  24, 231,},
	{  80, 111,  24, 231,},
	{ 120, 151,  24, 231,},
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
 * @brief   決定処理(タッチ・キー共通）
 *
 * @param   wk		
 *
 * @retval  none	
 */
//------------------------------------------------------------------
static void DecideFunc( WORLDTRADE_WORK *wk, int decide )
{
		switch(decide){
		// あずけるか？ようすをみるか？
		case 0:		
			if(wk->DepositFlag==0){
				// あずける
				WorldTrade_SubProcessChange( wk, WORLDTRADE_MYBOX, MODE_DEPOSIT_SELECT );
				wk->subprocess_seq  = SUBSEQ_END;
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			}else{
				// ようすをみる
				if(wk->serverWaitTime==0){
					WorldTrade_SubProcessChange( wk, WORLDTRADE_UPLOAD, MODE_SERVER_CHECK );
					wk->sub_returnprocess = WORLDTRADE_MYPOKE;
					wk->subprocess_seq    = SUBSEQ_END;
					wk->serverWaitTime    = SEVER_RETRY_WAIT;
					PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
				}else{
					// 待ち時間が過ぎないと再度ようすを見に行くことができない
#ifdef PM_DEBUG
					if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L){
						WorldTrade_SubProcessChange( wk, WORLDTRADE_UPLOAD, MODE_SERVER_CHECK );
						wk->sub_returnprocess = WORLDTRADE_MYPOKE;
						wk->subprocess_seq    = SUBSEQ_END;
						wk->serverWaitTime    = SEVER_RETRY_WAIT;
						PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
					}else{
						GFL_CLACT_WK_SetAutoAnmFlag(wk->CursorActWork,0);
						SubSeq_MessagePrint( wk, msg_gtc_01_035, 1, 0, 0x0f0f );
						WorldTrade_SetNextSeq( wk, SUBSEQ_MES_1MIN_WAIT, SUBSEQ_START );
						PMSND_PlaySE(SE_GTC_NG);
						wk->wait = 0;
					}
#else
					GFL_CLACT_WK_SetAutoAnmFlag(wk->CursorActWork,0);
					SubSeq_MessagePrint( wk, msg_gtc_01_035, 1, 0, 0x0f0f );
					WorldTrade_SetNextSeq( wk, SUBSEQ_MES_1MIN_WAIT, SUBSEQ_START );
					PMSND_PlaySE(SE_GTC_NG);
					wk->wait = 0;
#endif
				}
			}
			break;

		// ポケモンを探す
		case 1:
			//「あいてをみる」は押せない状態で画面構築する
			wk->SubLcdTouchOK = 0;
			WorldTrade_SubProcessChange( wk, WORLDTRADE_SEARCH, MODE_NORMAL );
			wk->subprocess_seq  = SUBSEQ_END;
			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			break;

		// しゅうりょうする
		case 2:
			SubSeq_TalkPrint( wk, msg_gtc_01_008, WorldTrade_GetTalkSpeed(wk), 0, 0x0f0f );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_YESNO );
			GFL_CLACT_WK_SetAutoAnmFlag(wk->CursorActWork,0);
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
	u32 ret;
	
	// タッチ処理
	ret=TouchPanelFunc(wk);
	if(ret!=GFL_UI_TP_HIT_NONE){
		wk->TitleCursorPos = ret;
		PMSND_PlaySE(WORLDTRADE_MOVE_SE);
		WorldTrade_CLACT_PosChange(wk->CursorActWork, 
			CursorPosTbl[wk->TitleCursorPos][0],
			CursorPosTbl[wk->TitleCursorPos][1]);
		DecideFunc(wk, ret);
		return SEQ_MAIN;
	}
	// キー処理
	if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
			SubSeq_TalkPrint( wk, msg_gtc_01_008, WorldTrade_GetTalkSpeed(wk), 0, 0x0f0f );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_YESNO );
			GFL_CLACT_WK_SetAutoAnmFlag(wk->CursorActWork,0);
	}else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
		DecideFunc( wk, wk->TitleCursorPos );
	}else if(GFL_UI_KEY_GetTrg() & PAD_KEY_UP){
		if(wk->TitleCursorPos!=0){
			wk->TitleCursorPos--;
			PMSND_PlaySE(WORLDTRADE_MOVE_SE);
			WorldTrade_CLACT_PosChange(wk->CursorActWork, 
				CursorPosTbl[wk->TitleCursorPos][0],
				CursorPosTbl[wk->TitleCursorPos][1]);
		}
	}else if(GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN){
		if(wk->TitleCursorPos<2){
			wk->TitleCursorPos++;
			PMSND_PlaySE(WORLDTRADE_MOVE_SE);
			WorldTrade_CLACT_PosChange(wk->CursorActWork, 
				CursorPosTbl[wk->TitleCursorPos][0],
				CursorPosTbl[wk->TitleCursorPos][1]);
		}
	}

	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   主人公さよならデモ開始
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_EndDemo( WORLDTRADE_WORK *wk )
{
	
	WorldTrade_ReturnHeroDemo( wk, MyStatus_GetMySex(wk->param->mystatus) );
	wk->subprocess_seq = SUBSEQ_END_DEMO_WAIT;
	wk->demo_end       = 0;

	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   主人公さよならデモ待ち
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_EndDemoWait( WORLDTRADE_WORK *wk )
{
	if(wk->demo_end){
		wk->subprocess_seq = SUBSEQ_END;
	}
	return SEQ_MAIN;
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
	// 接続画面だったら、上下画面でフェード
	if(wk->sub_nextprocess==WORLDTRADE_ENTER){
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
#ifdef GTS_FADE_OSP
		OS_Printf( "******************** worldtrade_title.c [846] MS ********************\n" );
#endif
	}else{
		// 続きの画面にいく場合は上画面だけフェード
		WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
#ifdef GTS_FADE_OSP
		OS_Printf( "******************** worldtrade_title.c [850] M ********************\n" );
#endif
	}
	wk->subprocess_seq = 0;
	
	return SEQ_FADEOUT;
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
//	wk->YesNoMenuWork = WorldTrade_BmpWinYesNoMake(, WORLDTRADE_YESNO_PY2, YESNO_OFFSET );
	wk->tss			   = WorldTrade_TouchWinYesNoMake(WORLDTRADE_YESNO_PY2, YESNO_OFFSET, 3, 1 );
	wk->subprocess_seq = SUBSEQ_YESNO_SELECT;

	// カーソル隠す
	GFL_CLACT_WK_SetDrawEnable( wk->CursorActWork, 0 );

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
		// 主人公帰るデモ
		TOUCH_SW_FreeWork( wk->tss );
		BmpWinFrame_Clear( wk->TalkWin, WINDOW_TRANS_OFF );

		GFL_BMPWIN_ClearTransWindow( wk->TalkWin );
		GFL_BMPWIN_ClearTransWindow( wk->ExplainWin );
		GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_OFF );
		GFL_CLACT_WK_SetDrawEnable( wk->PromptDsActWork, 0 );
		WorldTrade_SubProcessChange( wk, WORLDTRADE_ENTER, 0 );
		wk->subprocess_seq  = SUBSEQ_END_DEMO;

	}else if(ret==TOUCH_SW_RET_NO){
		// もういっかいトライ
		TOUCH_SW_FreeWork( wk->tss );
		BmpWinFrame_Clear( wk->TalkWin, WINDOW_TRANS_OFF );
		GFL_BMPWIN_ClearTransWindow( wk->TalkWin );
		GFL_CLACT_WK_SetAutoAnmFlag(wk->CursorActWork,1);
		wk->subprocess_seq = SUBSEQ_START;

		// カーソル表示する
		GFL_CLACT_WK_SetDrawEnable( wk->CursorActWork, 1 );

	}

/*
	int ret = BmpYesNoSelectMain( wk->YesNoMenuWork, HEAPID_WORLDTRADE );

	if(ret!=BMPMENU_NULL){
		if(ret==BMPMENU_CANCEL){
			// もういっかいトライ
			BmpWinFrame_Clear( &wk->TalkWin, WINDOW_TRANS_OFF );
			GFL_BMPWIN_ClearTransWindow( &wk->TalkWin );
			GFL_CLACT_WK_SetAutoAnmFlag(wk->CursorActWork,1);
			wk->subprocess_seq = SUBSEQ_START;
		}else{
			// 主人公帰るデモ
			BmpWinFrame_Clear( &wk->TalkWin, WINDOW_TRANS_OFF );
			GFL_BMPWIN_ClearTransWindow( &wk->TalkWin );
			WorldTrade_SubProcessChange( wk, WORLDTRADE_ENTER, 0 );
			wk->subprocess_seq  = SUBSEQ_END_DEMO;
			
		}
	}
*/
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
 * @brief   １秒待って次のシーケンスへ
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int SubSeq_Message1MinWait( WORLDTRADE_WORK *wk )
{
	if( GF_MSG_PrintEndCheck( &wk->print )==0){
		wk->wait++;
		if(wk->wait > SEVERNO_MESSAGE_WAIT){
			wk->wait = 0;
			wk->subprocess_seq = wk->subprocess_nextseq;
		}
	}
	return SEQ_MAIN;
	
}






















// ＧＴＳタイトル画面のメニュー項目
static const int menu_str_table[][3]={
	{
		msg_gtc_02_002_02,		// ぽけもんをあずける
		msg_gtc_02_003,			// ポケモンをさがす
		msg_gtc_02_004,			// しゅうりょうする
	},
	{
		msg_gtc_02_002_01,		// ようすをみる
		msg_gtc_02_003,			// ポケモンをさがす
		msg_gtc_02_004,			// しゅうりょうする
	},
};

//------------------------------------------------------------------
/**
 * @brief   ３つのメニュー項目を描画する
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void TitleMenuPrint( WORLDTRADE_WORK *wk )
{
	int i;
//	int menu = WorldTradeData_GetFlag( wk->param->worldtrade_data );
	int menu = wk->DepositFlag;

	// ポケモンを預けているかどうかで、メニューの項目が変わる
	for(i=0;i<3;i++){
		GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->MenuWin[i]), 0, 0, MENU_TEXT_SX*8, 8, 15 );
		GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->MenuWin[i]), 0, 8, MENU_TEXT_SX*8, 2, 14 );
		GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->MenuWin[i]), 0, 10, MENU_TEXT_SX*8,  6, 13 );
		WorldTrade_BmpWinPrint( wk->MenuWin[i], wk->MsgManager, FONT_TOUCH, menu_str_table[menu][i], 0x0000 ,&wk->print);
	}
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
	
	GFL_MSG_GetString(  wk->MsgManager, msgno, wk->TalkString );

	// 会話ウインドウ枠描画
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin),  0x0f0f );
	BmpWinFrame_Write( wk->MsgWin, WINDOW_TRANS_ON, WORLDTRADE_MESFRAME_CHR, WORLDTRADE_MESFRAME_PAL );

	// 文字列描画開始
	GF_STR_PrintSimple( wk->MsgWin, FONT_TALK, wk->TalkString, 0, 0, &wk->print);
	GFL_BMPWIN_MakeTransWindow( wk->MsgWin );

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
static void SubSeq_TalkPrint( WORLDTRADE_WORK *wk, int msgno, int wait, int flag, u16 dat )
{
	// 文字列取得
	STRBUF *tempbuf;
	
	GFL_MSG_GetString(  wk->MsgManager, msgno, wk->TalkString );

	// 会話ウインドウ枠描画
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->TalkWin),  0x0f0f );
	BmpWinFrame_Write( wk->TalkWin, WINDOW_TRANS_ON, WORLDTRADE_MESFRAME_CHR, WORLDTRADE_MESFRAME_PAL );

	// 文字列描画開始
	GF_STR_PrintSimple( wk->TalkWin, FONT_TALK, wk->TalkString, 0, 0, &wk->print);
	GFL_BMPWIN_MakeTransWindow( wk->TalkWin );

}

//------------------------------------------------------------------
/**
 * @brief   BMPWINに文字列描画
 *
 * @param   win		
 * @param   msgman		
 * @param   msgno		
 * @param   dat		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
void WorldTrade_BmpWinPrint( GFL_BMPWIN *win, GFL_MSGDATA *msgman, int font, int msgno, u16 dat, WT_PRINT *print )
{
	// 文字列取得
	STRBUF *tempbuf;
	
	tempbuf = GFL_MSG_CreateString(  msgman, msgno );

	// 会話ウインドウ枠描画
	//GFL_BMP_Clear( GFL_BMPWIN_GetBmp(win),  dat );

	// 文字列描画開始
	GF_STR_PrintColor( win, font, tempbuf, 0, 0, MSG_ALLPUT, PRINTSYS_LSB_Make( 10, 9, 0), print );

	GFL_STR_DeleteBuffer(tempbuf);
}


//==============================================================================
/**
 * @brief   下画面用「世界交換ルーム」256色グラフィック転送
 *
 * @param   wk		
 * @param   winflag	 1:トレード画面のウインドウを転送する 0:しない
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_SubLcdBgGraphicSet( WORLDTRADE_WORK *wk  )
{
	// 下画面ＢＧパレット転送
	GFL_ARC_UTIL_TransVramPalette( ARCID_WORLDTRADE_GRA, NARC_worldtrade_traderoom_nclr, PALTYPE_SUB_BG,  0, 16*16*2,  HEAPID_WORLDTRADE);

	// サブ画面BG1キャラ転送
	GFL_ARC_UTIL_TransVramBgCharacter( ARCID_WORLDTRADE_GRA, NARC_worldtrade_traderoom_lz_ncgr,  GFL_BG_FRAME1_S, 0, 32*21*0x40, 1, HEAPID_WORLDTRADE);

	// サブ画面BG1スクリーン転送
	GFL_ARC_UTIL_TransVramScreen( ARCID_WORLDTRADE_GRA, NARC_worldtrade_traderoom_lz_nscr, 
	     GFL_BG_FRAME1_S, 0, 32*24*2, 1, HEAPID_WORLDTRADE);
	//↑が256x256全てを埋めていないスクリーンの為、下部にゴミが残るのでクリア
	GFL_BG_FillScreen(  GFL_BG_FRAME1_S, 0, 0, 24, 32, 32-24, GFL_BG_SCRWRT_PALIN );
	GFL_BG_LoadScreenReq(  GFL_BG_FRAME1_S );

	// 会話フォントパレット転送
	TalkFontPaletteLoad( PALTYPE_SUB_BG, WORLDTRADE_SUB_TALKFONT_PAL*0x20, HEAPID_WORLDTRADE );

}	

//==============================================================================
/**
 * @brief   トレードルーム状態説明スクリーン転送
 *
 * @param   wk		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_SubLcdWinGraphicSet( WORLDTRADE_WORK *wk )
{
	// -------------サブ画面---------------------
	// メイン画面BG2キャラ転送
	GFL_ARC_UTIL_TransVramBgCharacter(  ARCID_WORLDTRADE_GRA, NARC_worldtrade_traderoom_win_lz_ncgr,
						GFL_BG_FRAME2_S,    0, 0, 1, HEAPID_WORLDTRADE);

	// 状態説明スクリーン
	GFL_ARC_UTIL_TransVramScreen( ARCID_WORLDTRADE_GRA, NARC_worldtrade_traderoom_win_lz_nscr,
					  GFL_BG_FRAME2_S,    0, 0, 1, HEAPID_WORLDTRADE);

	// 「DSの下画面をみてねアイコン」表示
	GFL_CLACT_WK_SetDrawEnable( wk->PromptDsActWork, 1 );

}


//==============================================================================
/**
 * @brief   サブ画面のGTS説明用BMPWINを確保する
 *
 * @param   wk		
 *
 * @retval  none		
 */
//==============================================================================
void WorldTrade_SubLcdExpainPut( WORLDTRADE_WORK *wk, int explain )
{
	// 「GTS」
	wk->ExplainWin	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME0_S,
		EXPLAIN_WIN_X, EXPLAIN_WIN_Y, EXPLAIN_WIN_SX, EXPLAIN_WIN_SY, 
		WORLDTRADE_TALKFONT_SUB_PAL, GTS_EXPLAIN_OFFSET  );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->ExplainWin), 0x0000 );
	GFL_BMPWIN_MakeTransWindow( wk->ExplainWin );

	// 上画面メッセージ
	WorldTrade_ExplainPrint( wk->ExplainWin,  wk->MsgManager, explain, &wk->print );

	OS_Printf("説明ウインドウ確保\n");
}
