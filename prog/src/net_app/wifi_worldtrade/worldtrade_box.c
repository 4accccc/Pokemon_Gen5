//============================================================================================
/**
 * @file	worldtrade_box.c
 * @brief	世界交換えせボックス画面処理
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
#include "poke_tool/poke_tool.h"
//#include "poke_tool/pokeicon.h"
#include "poke_tool/pokeparty.h"
#include "item/item.h"

#include "savedata/wifilist.h"

#include "net_app/worldtrade.h"
#include "worldtrade_local.h"

#include "msg/msg_wifi_lobby.h"
#include "msg/msg_wifi_gts.h"
#include "print/printsys.h"

#include "net_app/net_bugfix.h"
#include "poke_tool/monsno_def.h"

#include "worldtrade.naix"			// グラフィックアーカイブ定義


//============================================================================================
//	型宣言
//============================================================================================
#define POKEICON_TRANS_CHARA	(4*4)
#define POKEICON_TRANS_SIZE		(POKEICON_TRANS_CHARA*0x20)

typedef struct {
  int vadrs;				// VRAMアドレス
  int palno;				// パレット番号
  GFL_CLWK* icon;			// CLACT
  u8 chbuf[POKEICON_TRANS_SIZE];	// キャラクタバッファ
} WORLDTRADE_POKEBUF;



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
static int SubSeq_Start( WORLDTRADE_WORK *wk);
static int SubSeq_Main( WORLDTRADE_WORK *wk);
static int SubSeq_End( WORLDTRADE_WORK *wk);
static int SubSeq_YesNo( WORLDTRADE_WORK *wk);
static int SubSeq_YesNoSelect( WORLDTRADE_WORK *wk);
static int SubSeq_MessageWait( WORLDTRADE_WORK *wk );
static int SubSeq_MessageClearWait( WORLDTRADE_WORK *wk );
static void SubSeq_MessagePrint( WORLDTRADE_WORK *wk, int msgno, int wait, int flag, u16 dat, int winflag );
static void TransPokeIconCharaPal( int pokeno, int form, int tamago, int no, GFL_CLWK* icon, ARCHANDLE* handle, WORLDTRADE_POKEBUF *pbuf );
static void NowBoxPageInfoGet( WORLDTRADE_WORK *wk, int now);
static int RoundWork( int num, int max, int move );
static void CursorControl( WORLDTRADE_WORK *wk );
static int SubSeq_SelectList( WORLDTRADE_WORK *wk );
static int SubSeq_SelectWait( WORLDTRADE_WORK *wk );
static int SubSeq_ExchangeSelectList( WORLDTRADE_WORK *wk );
static int SubSeq_ExchangeSelectWait( WORLDTRADE_WORK *wk );
static int SubSeq_CBallYesNoSelect( WORLDTRADE_WORK *wk );
static int SubSeq_CBallYesNo( WORLDTRADE_WORK *wk );
static int SubSeq_CBallYesNoMessage( WORLDTRADE_WORK *wk );
static int SubSeq_CBallDepositYesNoMessage( WORLDTRADE_WORK *wk );
static int SubSeq_CBallDepositYesNo( WORLDTRADE_WORK *wk );
static int SubSeq_CBallDepositYesNoSelect( WORLDTRADE_WORK *wk );


static  int WantPokeCheck(POKEMON_PASO_PARAM *ppp, Dpw_Tr_PokemonSearchData *dtsd);
static void MakeExchangePokemonData( Dpw_Tr_Data *dtd, WORLDTRADE_WORK *wk );
static void PokemonIconSet( POKEMON_PASO_PARAM *paso, GFL_CLWK* icon, GFL_CLWK* itemact, u16 *no, int pos, ARCHANDLE* handle, Dpw_Tr_PokemonDataSimple *dat, WORLDTRADE_POKEBUF *pbuf );
static void* CharDataGetbyHandle( ARCHANDLE *handle, u32 dataIdx, NNSG2dCharacterData** charData, u32 heapID );
static  int CompareSearchData( Dpw_Tr_PokemonDataSimple *poke,  Dpw_Tr_PokemonSearchData *search );
static void PokeIconPalSet( Dpw_Tr_PokemonDataSimple *box, GFL_CLWK* *icon, Dpw_Tr_PokemonSearchData *want, WORLDTRADE_POKEBUF *pbuf);
static  int CheckPocket( POKEPARTY *party, BOX_MANAGER *box,  int  tray, int pos );
static  int PokemonCheck( POKEPARTY *party, BOX_MANAGER *box,  int  tray, int pos  );
static  int ExchangeCheck( WORLDTRADE_WORK *wk );
static  int PokeRibbonCheck( POKEMON_PASO_PARAM *ppp );
static  int PokeNewFormCheck( POKEMON_PASO_PARAM *ppp );
static  int PokeNewItemCheck( POKEMON_PASO_PARAM *ppp );
static void CursorPosPrioritySet( GFL_CLWK* cursor, int pos );

static void CancelFunc( WORLDTRADE_WORK *wk, int mode );

#define WANT_POKE_NO		( 0 )
#define WANT_POKE_OK		( 1 )
#define WANT_POKE_TAMAGO	( 2 )

enum{
	SUBSEQ_START=0,
	SUBSEQ_MAIN,
	SUBSEQ_END,
	SUBSEQ_MES_WAIT,
	SUBSEQ_MES_CLEAR_WAIT,
	SUBSEQ_YESNO,
	SUBSEQ_YESNO_SELECT,
	SUBSEQ_SELECT_LIST,
	SUBSEQ_SELECT_WAIT,
	SUBSEQ_EXCHANGE_SELECT_LIST,
	SUBSEQ_EXCHANGE_SELECT_WAIT,

	SUBSEQ_CBALL_YESNO_MES,
	SUBSEQ_CBALL_YESNO,
	SUBSEQ_CBALL_YESNO_SELECT,

	SUBSEQ_CBALL_DEPOSIT_YESNO_MES,
	SUBSEQ_CBALL_DEPOSIT_YESNO,
	SUBSEQ_CBALL_DEPOSIT_YESNO_SELECT,
};

static int (*Functable[])( WORLDTRADE_WORK *wk ) = {
	SubSeq_Start,			// SUBSEQ_START=0,
	SubSeq_Main,            // SUBSEQ_MAIN,
	SubSeq_End,             // SUBSEQ_END,
	SubSeq_MessageWait,     // SUBSEQ_MES_WAIT
	SubSeq_MessageClearWait,// SUBSEQ_MES_CLEAR_WAIT
	SubSeq_YesNo,			// SUBSEQ_YESNO
	SubSeq_YesNoSelect,		// SUBSEQ_YESNO_SELECT
	SubSeq_SelectList,		// SUBSEQ_SELECT_LIST
	SubSeq_SelectWait,		// SUBSEQ_SELECT_WAIT
	SubSeq_ExchangeSelectList,	// SUBSEQ_EXCHANGE_SELECT_LIST
	SubSeq_ExchangeSelectWait,	// SUBSEQ_EXCHANGE_SELECT_WAIT

	SubSeq_CBallYesNoMessage,	// SUBSEQ_CBALL_YESNO_MES
	SubSeq_CBallYesNo,			// SUBSEQ_CBALL_YESNO
	SubSeq_CBallYesNoSelect,	// SUBSEQ_CBALL_YESNO_SELECT

	SubSeq_CBallDepositYesNoMessage,	// SUBSEQ_CBALL_YESNO_MES
	SubSeq_CBallDepositYesNo,			// SUBSEQ_CBALL_YESNO
	SubSeq_CBallDepositYesNoSelect,	// SUBSEQ_CBALL_YESNO_SELECT
};

#define BOX_CUROSOR_END_POS			( 30 )
#define BOX_CUROSOR_TRAYNAME_POS	( 31 )


//============================================================================================
//	プロセス関数
//============================================================================================

//==============================================================================
/**
 * @brief   世界交換入り口画面初期化
 *
 * @param   wk		GTS画面ワーク
 * @param   seq		シーケンス（未使用）
 *
 * @retval  int		次のシーケンス
 */
//==============================================================================
int WorldTrade_Box_Init(WORLDTRADE_WORK *wk, int seq)
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

    WorldTrade_SetPartnerExchangePos( wk );

    if(GXS_GetMasterBrightness() == 0){
    	WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
    }
    else{
    	WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
    }

	NowBoxPageInfoGet(wk, wk->BoxTrayNo);
	

	// 通信状態を確認してアイコンの表示を変える
	WorldTrade_WifiIconAdd( wk );

	// 「ＤＳの下画面みてね」アイコンを隠す
//	GFL_CLACT_WK_SetDrawEnable( wk->PromptDsActWork, 0 );
   
	wk->subprocess_seq = SUBSEQ_START;
	
	return SEQ_FADEIN;
}
//==============================================================================
/**
 * @brief   世界交換入り口画面メイン
 *
 * @param   wk		GTS画面ワーク
 * @param   seq		シーケンス（未使用）
 *
 * @retval  int		次のシーケンス
 */
//==============================================================================
int WorldTrade_Box_Main(WORLDTRADE_WORK *wk, int seq)
{
	int ret;

	
	ret = (*Functable[wk->subprocess_seq])( wk );

	return ret;
}


//==============================================================================
/**
 * @brief   世界交換入り口画面終了
 *
 * @param   wk		GTS画面ワーク
 * @param   seq		シーケンス（未使用）
 *
 * @retval  int		次のシーケンス
 */
//==============================================================================
int WorldTrade_Box_End(WORLDTRADE_WORK *wk, int seq)
{
	//WirelessIconEasyEnd();

    if(GXS_GetMasterBrightness() != 0){
        WorldTrade_SetPartnerExchangePosIsReturns( wk );
    }

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
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
		GFL_BG_SetVisible( GFL_BG_FRAME0_M, TRUE );
	}

	// メイン画面メニュー面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M, TRUE );
	}

	// メイン画面背景面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M, TRUE );
	}

	// ボックス名表示画面テキスト面
	{	
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256, GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_M );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M, TRUE );
	}


	// サブ画面初期化
	WorldTrade_SubLcdBgInit(0, 0 );


	GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 32, 0, HEAPID_WORLDTRADE );
	GFL_BG_SetClearCharacter( GFL_BG_FRAME3_M, 32, 0, HEAPID_WORLDTRADE );

}

//--------------------------------------------------------------------------------------------
/**
 * BG解放
 *
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BgExit( void )
{

	// サブ画面ＢＧ情報解放
	WorldTrade_SubLcdBgExit();

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
 * @param	wk		GTS画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BgGraphicSet( WORLDTRADE_WORK * wk )
{
	ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_WORLDTRADE_GRA, HEAPID_WORLDTRADE );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_worldtrade_mybox_nclr, PALTYPE_MAIN_BG, 0, 16*3*2,  HEAPID_WORLDTRADE);
	GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_worldtrade_traderoom_nclr, PALTYPE_SUB_BG,  0, 16*8*2,  HEAPID_WORLDTRADE);
	
	// 会話フォントパレット転送
	TalkFontPaletteLoad( PALTYPE_MAIN_BG, WORLDTRADE_TALKFONT_PAL*0x20, HEAPID_WORLDTRADE );
	// TalkFontPaletteLoad( PALTYPE_SUB_BG,  WORLDTRADE_TALKFONT_PAL*0x20, HEAPID_WORLDTRADE );

	// 会話ウインドウグラフィック転送
	BmpWinFrame_GraphicSet(	GFL_BG_FRAME0_M, WORLDTRADE_MESFRAME_CHR, 
						WORLDTRADE_MESFRAME_PAL,  CONFIG_GetWindowType(wk->param->config), HEAPID_WORLDTRADE );

	BmpWinFrame_GraphicSet(	GFL_BG_FRAME0_M, WORLDTRADE_MENUFRAME_CHR,
						WORLDTRADE_MENUFRAME_PAL, 0, HEAPID_WORLDTRADE );



	// メイン画面BG1キャラ転送
	GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_worldtrade_mybox_lz_ncgr, GFL_BG_FRAME1_M, 0, 16*5*0x20, 1, HEAPID_WORLDTRADE);

	// メイン画面BG1スクリーン転送
	GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_worldtrade_mybox_lz_nscr, GFL_BG_FRAME1_M, 0, 32*24*2, 1, HEAPID_WORLDTRADE);

	// メイン画面BG2スクリーン転送
	GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_worldtrade_title_base_lz_nscr, GFL_BG_FRAME2_M, 0, 32*24*2, 1, HEAPID_WORLDTRADE);

	WorldTrade_SubLcdBgGraphicSet( wk );	// トレードルーム転送
	WorldTrade_SubLcdWinGraphicSet( wk );   // トレードルームウインドウ転送

	GFL_ARC_CloseDataHandle( p_handle );
}
#define BOX_SX		( 25-6 )
#define BOX_SY		( 60-24 )
#define BOX_W		( 26 )
#define BOX_H		( 23 )

static const u16 PokemonIconPosTbl[][2]={
	{BOX_SX+BOX_W*0, BOX_SY+BOX_H*0},{BOX_SX+BOX_W*1, BOX_SY+BOX_H*0},{BOX_SX+BOX_W*2, BOX_SY+BOX_H*0},
	{BOX_SX+BOX_W*3, BOX_SY+BOX_H*0},{BOX_SX+BOX_W*4, BOX_SY+BOX_H*0},{BOX_SX+BOX_W*5, BOX_SY+BOX_H*0},

	{BOX_SX+BOX_W*0, BOX_SY+BOX_H*1},{BOX_SX+BOX_W*1, BOX_SY+BOX_H*1},{BOX_SX+BOX_W*2, BOX_SY+BOX_H*1},
	{BOX_SX+BOX_W*3, BOX_SY+BOX_H*1},{BOX_SX+BOX_W*4, BOX_SY+BOX_H*1},{BOX_SX+BOX_W*5, BOX_SY+BOX_H*1},

	{BOX_SX+BOX_W*0, BOX_SY+BOX_H*2},{BOX_SX+BOX_W*1, BOX_SY+BOX_H*2},{BOX_SX+BOX_W*2, BOX_SY+BOX_H*2},
	{BOX_SX+BOX_W*3, BOX_SY+BOX_H*2},{BOX_SX+BOX_W*4, BOX_SY+BOX_H*2},{BOX_SX+BOX_W*5, BOX_SY+BOX_H*2},

	{BOX_SX+BOX_W*0, BOX_SY+BOX_H*3},{BOX_SX+BOX_W*1, BOX_SY+BOX_H*3},{BOX_SX+BOX_W*2, BOX_SY+BOX_H*3},
	{BOX_SX+BOX_W*3, BOX_SY+BOX_H*3},{BOX_SX+BOX_W*4, BOX_SY+BOX_H*3},{BOX_SX+BOX_W*5, BOX_SY+BOX_H*3},

	{BOX_SX+BOX_W*0, BOX_SY+BOX_H*4},{BOX_SX+BOX_W*1, BOX_SY+BOX_H*4},{BOX_SX+BOX_W*2, BOX_SY+BOX_H*4},
	{BOX_SX+BOX_W*3, BOX_SY+BOX_H*4},{BOX_SX+BOX_W*4, BOX_SY+BOX_H*4},{BOX_SX+BOX_W*5, BOX_SY+BOX_H*4},
	{ 224,175-5*8 },{98-16, 16}
};


static const u16 BoxArrowPos[][2]={
	{162-8, 36-24 },
	{ 22-8, 36-24,}
};

#define BT_OFS_W	( 11 )
#define BT_OFS_H	( 13 )

static const GFL_UI_TP_HITTBL BoxTouchTable[]={
	{ BOX_SY+BOX_H*0-BT_OFS_H, BOX_SY+BOX_H*0+BT_OFS_H, BOX_SX+BOX_W*0-BT_OFS_W, BOX_SX+BOX_W*0+BT_OFS_W, },
	{ BOX_SY+BOX_H*0-BT_OFS_H, BOX_SY+BOX_H*0+BT_OFS_H, BOX_SX+BOX_W*1-BT_OFS_W, BOX_SX+BOX_W*1+BT_OFS_W, },
	{ BOX_SY+BOX_H*0-BT_OFS_H, BOX_SY+BOX_H*0+BT_OFS_H, BOX_SX+BOX_W*2-BT_OFS_W, BOX_SX+BOX_W*2+BT_OFS_W, },
	{ BOX_SY+BOX_H*0-BT_OFS_H, BOX_SY+BOX_H*0+BT_OFS_H, BOX_SX+BOX_W*3-BT_OFS_W, BOX_SX+BOX_W*3+BT_OFS_W, },
	{ BOX_SY+BOX_H*0-BT_OFS_H, BOX_SY+BOX_H*0+BT_OFS_H, BOX_SX+BOX_W*4-BT_OFS_W, BOX_SX+BOX_W*4+BT_OFS_W, },
	{ BOX_SY+BOX_H*0-BT_OFS_H, BOX_SY+BOX_H*0+BT_OFS_H, BOX_SX+BOX_W*5-BT_OFS_W, BOX_SX+BOX_W*5+BT_OFS_W, },

	{ BOX_SY+BOX_H*1-BT_OFS_H, BOX_SY+BOX_H*1+BT_OFS_H, BOX_SX+BOX_W*0-BT_OFS_W, BOX_SX+BOX_W*0+BT_OFS_W, },
	{ BOX_SY+BOX_H*1-BT_OFS_H, BOX_SY+BOX_H*1+BT_OFS_H, BOX_SX+BOX_W*1-BT_OFS_W, BOX_SX+BOX_W*1+BT_OFS_W, },
	{ BOX_SY+BOX_H*1-BT_OFS_H, BOX_SY+BOX_H*1+BT_OFS_H, BOX_SX+BOX_W*2-BT_OFS_W, BOX_SX+BOX_W*2+BT_OFS_W, },
	{ BOX_SY+BOX_H*1-BT_OFS_H, BOX_SY+BOX_H*1+BT_OFS_H, BOX_SX+BOX_W*3-BT_OFS_W, BOX_SX+BOX_W*3+BT_OFS_W, },
	{ BOX_SY+BOX_H*1-BT_OFS_H, BOX_SY+BOX_H*1+BT_OFS_H, BOX_SX+BOX_W*4-BT_OFS_W, BOX_SX+BOX_W*4+BT_OFS_W, },
	{ BOX_SY+BOX_H*1-BT_OFS_H, BOX_SY+BOX_H*1+BT_OFS_H, BOX_SX+BOX_W*5-BT_OFS_W, BOX_SX+BOX_W*5+BT_OFS_W, },
                     
	{ BOX_SY+BOX_H*2-BT_OFS_H, BOX_SY+BOX_H*2+BT_OFS_H, BOX_SX+BOX_W*0-BT_OFS_W, BOX_SX+BOX_W*0+BT_OFS_W, },
	{ BOX_SY+BOX_H*2-BT_OFS_H, BOX_SY+BOX_H*2+BT_OFS_H, BOX_SX+BOX_W*1-BT_OFS_W, BOX_SX+BOX_W*1+BT_OFS_W, },
	{ BOX_SY+BOX_H*2-BT_OFS_H, BOX_SY+BOX_H*2+BT_OFS_H, BOX_SX+BOX_W*2-BT_OFS_W, BOX_SX+BOX_W*2+BT_OFS_W, },
	{ BOX_SY+BOX_H*2-BT_OFS_H, BOX_SY+BOX_H*2+BT_OFS_H, BOX_SX+BOX_W*3-BT_OFS_W, BOX_SX+BOX_W*3+BT_OFS_W, },
	{ BOX_SY+BOX_H*2-BT_OFS_H, BOX_SY+BOX_H*2+BT_OFS_H, BOX_SX+BOX_W*4-BT_OFS_W, BOX_SX+BOX_W*4+BT_OFS_W, },
	{ BOX_SY+BOX_H*2-BT_OFS_H, BOX_SY+BOX_H*2+BT_OFS_H, BOX_SX+BOX_W*5-BT_OFS_W, BOX_SX+BOX_W*5+BT_OFS_W, },
                     
	{ BOX_SY+BOX_H*3-BT_OFS_H, BOX_SY+BOX_H*3+BT_OFS_H, BOX_SX+BOX_W*0-BT_OFS_W, BOX_SX+BOX_W*0+BT_OFS_W, },
	{ BOX_SY+BOX_H*3-BT_OFS_H, BOX_SY+BOX_H*3+BT_OFS_H, BOX_SX+BOX_W*1-BT_OFS_W, BOX_SX+BOX_W*1+BT_OFS_W, },
	{ BOX_SY+BOX_H*3-BT_OFS_H, BOX_SY+BOX_H*3+BT_OFS_H, BOX_SX+BOX_W*2-BT_OFS_W, BOX_SX+BOX_W*2+BT_OFS_W, },
	{ BOX_SY+BOX_H*3-BT_OFS_H, BOX_SY+BOX_H*3+BT_OFS_H, BOX_SX+BOX_W*3-BT_OFS_W, BOX_SX+BOX_W*3+BT_OFS_W, },
	{ BOX_SY+BOX_H*3-BT_OFS_H, BOX_SY+BOX_H*3+BT_OFS_H, BOX_SX+BOX_W*4-BT_OFS_W, BOX_SX+BOX_W*4+BT_OFS_W, },
	{ BOX_SY+BOX_H*3-BT_OFS_H, BOX_SY+BOX_H*3+BT_OFS_H, BOX_SX+BOX_W*5-BT_OFS_W, BOX_SX+BOX_W*5+BT_OFS_W, },
                     
	{ BOX_SY+BOX_H*4-BT_OFS_H, BOX_SY+BOX_H*4+BT_OFS_H, BOX_SX+BOX_W*0-BT_OFS_W, BOX_SX+BOX_W*0+BT_OFS_W, },
	{ BOX_SY+BOX_H*4-BT_OFS_H, BOX_SY+BOX_H*4+BT_OFS_H, BOX_SX+BOX_W*1-BT_OFS_W, BOX_SX+BOX_W*1+BT_OFS_W, },
	{ BOX_SY+BOX_H*4-BT_OFS_H, BOX_SY+BOX_H*4+BT_OFS_H, BOX_SX+BOX_W*2-BT_OFS_W, BOX_SX+BOX_W*2+BT_OFS_W, },
	{ BOX_SY+BOX_H*4-BT_OFS_H, BOX_SY+BOX_H*4+BT_OFS_H, BOX_SX+BOX_W*3-BT_OFS_W, BOX_SX+BOX_W*3+BT_OFS_W, },
	{ BOX_SY+BOX_H*4-BT_OFS_H, BOX_SY+BOX_H*4+BT_OFS_H, BOX_SX+BOX_W*4-BT_OFS_W, BOX_SX+BOX_W*4+BT_OFS_W, },
	{ BOX_SY+BOX_H*4-BT_OFS_H, BOX_SY+BOX_H*4+BT_OFS_H, BOX_SX+BOX_W*5-BT_OFS_W, BOX_SX+BOX_W*5+BT_OFS_W, },
                     
	{  123, 148, 186,  245},			// もどる
	{  16-12,  16+12, 154-8,  154+8, },	// 右矢印
	{  16-12,  16+12,  10-8,   10+8, },	// 左矢印
	
	{GFL_UI_TP_HIT_END,0,0,0},		// 終了データ
};



//------------------------------------------------------------------
/**
 * セルアクター登録
 *
 * @param   wk			GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void SetCellActor(WORLDTRADE_WORK *wk)
{
	int i;
	//登録情報格納
	GFL_CLWK_DATA	add;
	GFL_STD_MemClear( &add, sizeof(GFL_CLWK_DATA) );
	//WorldTrade_MakeCLACT( &add,  wk, &wk->clActHeader_main, NNS_G2D_VRAM_TYPE_2DMAIN );


	// ポケモンをさす指
	add.pos_x = PokemonIconPosTbl[wk->BoxCursorPos][0];
	add.pos_y = PokemonIconPosTbl[wk->BoxCursorPos][1];
	wk->CursorActWork = GFL_CLACT_WK_Create( wk->clactSet,
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
	GFL_CLACT_WK_SetAutoAnmFlag(wk->CursorActWork,1);
	GFL_CLACT_WK_SetAnmSeq( wk->CursorActWork, 4 );	
	
	/*
	 *	tomoya takahashi
		【見出し文と指カーソルの優先順位が一瞬逆になる】

		「ポケモンをあずける」に入る際、指カーソルが「てもち」や「ボックス
		名」に有り、見出しの「あずけるポケモンをえらんでください」と重なっ
		ていると、一瞬見出しの文字が指カーソルより優先順位が高く表示されて
		しまいます。
					対処
		//	GFL_CLACT_WK_SetBgPri( wk->CursorActWork, 1 );
	 */
	// カーソルプライオリティ操作
	if(wk->BoxCursorPos==31 || (wk->BoxCursorPos>=0 && wk->BoxCursorPos<=5)){
		// ボックストレイ名の上にあるときとボックス１列目にいるときは最上位に
		GFL_CLACT_WK_SetBgPri( wk->CursorActWork, 0 );
	}else{
		// それ以外はひとつ下に
		GFL_CLACT_WK_SetBgPri( wk->CursorActWork, 1 );
	}

	
	// ポケモンアイコン
	for(i=0;i<BOX_POKE_NUM;i++){
		add.pos_x = PokemonIconPosTbl[i][0];
		add.pos_y = PokemonIconPosTbl[i][1];
		add.softpri   = 20;
		wk->PokeIconActWork[i] = GFL_CLACT_WK_Create(wk->clactSet,
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
		GFL_CLACT_WK_SetAnmSeq( wk->PokeIconActWork[i], 6+i );	
		GFL_CLACT_WK_SetBgPri( wk->PokeIconActWork[i], 1 );
	}

	// アイテムアイコン
	for(i=0;i<BOX_POKE_NUM;i++){
		add.pos_x = ( PokemonIconPosTbl[i][0]+0);
		add.pos_y = ( PokemonIconPosTbl[i][1]+6);
		add.softpri   = 10;
		wk->ItemIconActWork[i] = GFL_CLACT_WK_Create(wk->clactSet,
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
		GFL_CLACT_WK_SetAnmSeq( wk->ItemIconActWork[i], CELL_ITEMICON_NO );	
		GFL_CLACT_WK_SetBgPri( wk->ItemIconActWork[i], 1 );

	}

	// ボールカプセルアイコン
	for(i=0;i<TEMOTI_POKEMAX;i++){
		add.pos_x = ( PokemonIconPosTbl[i][0]+8);
		add.pos_y = ( PokemonIconPosTbl[i][1]+6);
		add.softpri   = 10;
		wk->CBallActWork[i] = GFL_CLACT_WK_Create(wk->clactSet,
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
		GFL_CLACT_WK_SetAnmSeq( wk->CBallActWork[i], CELL_CBALLICON_NO );	
		GFL_CLACT_WK_SetBgPri( wk->CBallActWork[i], 1 );

	}

	// ボックス切り替えの矢印
	for(i=0;i<2;i++){
		add.pos_x = BoxArrowPos[i][0];
		add.pos_y = BoxArrowPos[i][1];
		wk->BoxArrowActWork[i] = GFL_CLACT_WK_Create(wk->clactSet,
			wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES],
			wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES], 
			wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES],
			&add, CLSYS_DRAW_MAIN, HEAPID_WORLDTRADE );
///		GFL_CLACT_WK_SetAutoAnmFlag( wk->BoxArrowActWork[i], 1 );初期化時はアニメしないようにしとく
		GFL_CLACT_WK_SetAnmSeq( wk->BoxArrowActWork[i], CELL_BOXARROW_NO+i );	
		GFL_CLACT_WK_SetBgPri( wk->BoxArrowActWork[i], 1 );
	}


	// 「DSの下画面をみてねアイコン」の表示
	GFL_CLACT_WK_SetDrawEnable( wk->PromptDsActWork, 1 );
	//WorldTrade_CLACT_PosChange( wk->PromptDsActWork, DS_ICON_X, DS_ICON_Y+256 );
}


//------------------------------------------------------------------
/**
 * @brief   表示セルアクターの解放
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void DelCellActor( WORLDTRADE_WORK *wk )
{
	int i;

	// ボックス横の矢印２こ
	for(i=0;i<2;i++){
		GFL_CLACT_WK_Remove(wk->BoxArrowActWork[i]);
	}

	// カーソル削除
	GFL_CLACT_WK_Remove(wk->CursorActWork);

	// ポケモンアイコン削除
	for(i=0;i<BOX_POKE_NUM;i++){
		GFL_CLACT_WK_Remove(wk->PokeIconActWork[i]);
		GFL_CLACT_WK_Remove(wk->ItemIconActWork[i]);
	}
	// カスタムボールアイコン削除
	for(i=0;i<TEMOTI_POKEMAX;i++){
		GFL_CLACT_WK_Remove(wk->CBallActWork[i]);
	}
}

#define SELECT_MENU_X	(  21 )
#define SELECT_MENU_Y 	(  13 )
#define SELECT_MENU_SX	( 5*2 )
#define SELECT_MENU_SY	(   6 )


#define BOX_TRAY_NAME_X		(  4 )
#define BOX_TRAY_NAME_Y		(  0 )
#define BOX_TRAY_NAME_SX	( 13 )
#define BOX_TRAY_NAME_SY	(  3 )

#define END_TEXT_X			( 24 )
#define END_TEXT_Y			( 16 )
#define END_TEXT_SX			(  6 )
#define END_TEXT_SY			(  2 )


#define TITLE_MESSAGE_OFFSET   ( WORLDTRADE_MENUFRAME_CHR + MENU_WIN_CGX_SIZ )
#define BOX_TRAY_NAME_OFFSET   ( TITLE_MESSAGE_OFFSET     + TITLE_TEXT_SX*TITLE_TEXT_SY )
#define LINE_MESSAGE_OFFSET    ( BOX_TRAY_NAME_OFFSET     + BOX_TRAY_NAME_SX*BOX_TRAY_NAME_SX )
#define TALK_MESSAGE_OFFSET	   ( LINE_MESSAGE_OFFSET      + LINE_TEXT_SX*LINE_TEXT_SY )
#define END_MESSAGE_OFFSET	   ( TALK_MESSAGE_OFFSET      + TALK_WIN_X*TALK_WIN_Y )
#define SELECT_MENU_OFFSET 	   ( END_MESSAGE_OFFSET       + END_TEXT_SX*END_TEXT_SY )
#define YESNO_OFFSET 		   ( SELECT_MENU_OFFSET       + SELECT_MENU_SX*SELECT_MENU_SY )

//------------------------------------------------------------------
/**
 * BMPWIN処理（文字パネルにフォント描画）
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void BmpWinInit( WORLDTRADE_WORK *wk )
{
	// ---------- メイン画面 ------------------

	// BG0面BMPWINタイトルウインドウ確保・描画
	
//	wk->TitleWin	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME0_M,
//	TITLE_TEXT_X, TITLE_TEXT_Y, TITLE_TEXT_SX, TITLE_TEXT_SY, WORLDTRADE_TALKFONT_PAL,  TITLE_MESSAGE_OFFSET );

//	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->TitleWin), 0x0000 );
	
//	//  あずけるポケモンをえらんでください
//	WorldTrade_SysPrint( wk->TitleWin, wk->TitleString, 0, 1, 0, PRINTSYS_LSB_Make(15,14,0) );


	// ボックストレイ名BMPWIN確保
	
	wk->SubWin	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME3_M,
	BOX_TRAY_NAME_X, BOX_TRAY_NAME_Y, BOX_TRAY_NAME_SX, BOX_TRAY_NAME_SY, WORLDTRADE_TALKFONT_PAL,  
	BOX_TRAY_NAME_OFFSET );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->SubWin), 0x0000 );
	GFL_BMPWIN_MakeTransWindow(wk->SubWin);
	
	// 一行ウインドウ
	wk->MsgWin	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME0_M,
		LINE_TEXT_X, LINE_TEXT_Y, LINE_TEXT_SX, LINE_TEXT_SY, 
		WORLDTRADE_TALKFONT_PAL,  LINE_MESSAGE_OFFSET );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin), 0x0000 );
	GFL_BMPWIN_MakeTransWindow(wk->MsgWin);

	// 2行ウインドウ
	wk->TalkWin	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME0_M,
		TALK_WIN_X, TALK_WIN_Y, TALK_WIN_SX, TALK_WIN_SY, 
		WORLDTRADE_TALKFONT_PAL,  TALK_MESSAGE_OFFSET );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->TalkWin), 0x0000 );
	GFL_BMPWIN_MakeTransWindow(wk->MsgWin);

	// 「もどる」
	wk->MenuWin[1]	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME1_M,
		END_TEXT_X, END_TEXT_Y, END_TEXT_SX, END_TEXT_SY, 
		0,  END_MESSAGE_OFFSET );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MenuWin[1]), 0x0606 );
	GFL_BMPWIN_MakeTransWindow(wk->MenuWin[1]);
	// 「もどる」描画
	WorldTrade_SysPrint( wk->MenuWin[1], wk->EndString, 0, 1, 1, PRINTSYS_LSB_Make(1,3,6), &wk->print );

	// 選択メニュー領域
//	wk->MenuWin[0]	= GFL_BMPWIN_CreateFixPos( GFL_BG_FRAME0_M,
//		SELECT_MENU_X, SELECT_MENU_Y, SELECT_MENU_SX, SELECT_MENU_SY, 
//		WORLDTRADE_TALKFONT_PAL,  SELECT_MENU_OFFSET );	

	
	// サブ画面のGTS説明用BMPWINを確保する
	if(wk->sub_process_mode==MODE_DEPOSIT_SELECT){
		WorldTrade_SubLcdExpainPut( wk, EXPLAIN_AZUKERU );
	}else{
		WorldTrade_SubLcdExpainPut( wk, EXPLAIN_KOUKAN );
	}

}	

//------------------------------------------------------------------
/**
 * @brief   確保したBMPWINを解放
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void BmpWinDelete( WORLDTRADE_WORK *wk )
{
  //BMPを消す前に残ったストリームを消去
  WT_PRINT_ClearBuffer( &wk->print );
	
	GFL_BMPWIN_Delete( wk->ExplainWin );
	GFL_BMPWIN_Delete( wk->TalkWin );
	GFL_BMPWIN_Delete( wk->MenuWin[1] );
//	GFL_BMPWIN_Delete( wk->MenuWin[0] );
	GFL_BMPWIN_Delete( wk->MsgWin );
	GFL_BMPWIN_Delete( wk->SubWin );
//	GFL_BMPWIN_Delete( wk->TitleWin );


}

//------------------------------------------------------------------
/**
 * 世界交換ワーク初期化
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void InitWork( WORLDTRADE_WORK *wk )
{

	// ボックストレイ名文字列バッファ
	wk->BoxTrayNameString = GFL_STR_CreateBuffer( BOX_TRAY_NAME_BUF_NUM, HEAPID_WORLDTRADE );


	// 文字列バッファ作成
	wk->TalkString  = GFL_STR_CreateBuffer( TALK_MESSAGE_BUF_NUM, HEAPID_WORLDTRADE );
#if 0
	// 「あずけるポケモンをえらんでください」「かわりのポケモンをえらんでください」
	if(wk->sub_process_mode==MODE_DEPOSIT_SELECT){
		wk->TitleString = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_01_022 );
	}else if(wk->sub_process_mode==MODE_EXCHANGE_SELECT){
		wk->TitleString = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_01_018 );
	}
#endif
	// もどる
	wk->EndString   = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_05_014 );

	if(wk->BoxCursorPos==BOX_CUROSOR_END_POS){
		wk->BoxCursorPos = 0;
	}
	
	// ボックス用ワーク
	wk->boxWork = GFL_HEAP_AllocMemory(HEAPID_WORLDTRADE, sizeof(BOX_RESEARCH));

}


//------------------------------------------------------------------
/**
 * @brief   ワーク解放
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void FreeWork( WORLDTRADE_WORK *wk )
{
	GFL_HEAP_FreeMemory(wk->boxWork);

	GFL_STR_DeleteBuffer( wk->BoxTrayNameString );
	GFL_STR_DeleteBuffer( wk->TalkString ); 
	GFL_STR_DeleteBuffer( wk->EndString ); 
///	GFL_STR_DeleteBuffer( wk->TitleString ); 

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
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_Start( WORLDTRADE_WORK *wk)
{
	// 
	if(WIPE_SYS_EndCheck()){
		int msg;
		// 「あずけるポケモンをえらんでください」「かわりのポケモンをえらんでください」
		if(wk->sub_process_mode==MODE_DEPOSIT_SELECT){
			msg = msg_gtc_01_022;
		}else if(wk->sub_process_mode==MODE_EXCHANGE_SELECT){
			msg = msg_gtc_01_018;
		}
		SubSeq_MessagePrint( wk, msg, 1, 0, 0x0f0f, 0 );
		WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );

//		SubSeq_MessagePrint( wk, msg_gtc_01_018, 1, 0, 0x0f0f );
//		WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT, SUBSEQ_MAIN );
//		wk->subprocess_seq = SUBSEQ_MAIN;
	}

	return SEQ_MAIN;
}
/* 
       31

 0  1  2  3  4  5
 6  7  8  9 10 11
12 13 14 15 16 17
18 19 20 21 22 23
24 25 26 27 28 29 30

 */

static const u8 box_pos_table[][4]={
	// 上下左右
	{31, 6, 5, 1,},{31, 7, 0, 2,},{31, 8, 1, 3,},{31, 9, 2, 4,},{31,10, 3, 5,},{31,11, 4, 0,},
	{ 0,12,11, 7,},{ 1,13, 6, 8,},{ 2,14, 7, 9,},{ 3,15, 8,10,},{ 4,16, 9,11,},{ 5,17,10, 6,},
	{ 6,18,17,13,},{ 7,19,12,14,},{ 8,20,13,15,},{ 9,21,14,16,},{10,22,15,17,},{11,23,16,12,},
	{12,24,23,19,},{13,25,18,20,},{14,26,19,21,},{15,27,20,22,},{16,28,21,23,},{17,29,22,18,},
	{18,31,30,25,},{19,31,24,26,},{20,31,25,27,},{21,31,26,28,},{22,31,27,29,},{23,31,28,30,},{30,30,29,24},
	{ 26,2,99,101, },

};

static const s8 movetbl[][2]={
	{0,0},{0,-1},{0,1},{1,0},{-1,0},
};


static void DepositDecideFunc( WORLDTRADE_WORK*wk )
{
	if(wk->BoxCursorPos == BOX_CUROSOR_END_POS){
	// タイトルメニューにもどる
		WorldTrade_SubProcessChange( wk, WORLDTRADE_TITLE, 0 );
		wk->subprocess_seq = SUBSEQ_END;
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
	}else{
		if(wk->BoxCursorPos!=BOX_CUROSOR_TRAYNAME_POS){
			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			// ●●●をあずけますか？
			switch(PokemonCheck( wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos )){
			// だせる
			case WANT_POKE_OK:
				if(CheckPocket( wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos )){
					WT_WORDSET_RegisterPokeNickNamePPP( wk->WordSet, 0, 
							WorldTrade_GetPokePtr(wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos) );
					SubSeq_MessagePrint( wk, msg_gtc_01_023, 1, 0, 0x0f0f, 0 );
					WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT,  SUBSEQ_SELECT_LIST);
					OS_Printf("deposit_ppp0 = %08x\n",WorldTrade_GetPokePtr(wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos));
				}else{
					// てもちの最後の1匹だった場合はだせない
					SubSeq_MessagePrint( wk, msg_gtc_01_027, 1, 0, 0x0f0f, 1 );
					WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN);
				}
				break;
			// タマゴはだせない
			case WANT_POKE_TAMAGO:
				SubSeq_MessagePrint( wk, msg_gtc_01_028, 1, 0, 0x0f0f, 1 );
				WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN);
				break;
			// 特殊リボンがついているのでダメ
			}
		}
	}

}

static void ExchangeDecideFunc( WORLDTRADE_WORK*wk )
{
	if(wk->BoxCursorPos == BOX_CUROSOR_END_POS){
		// 検索結果ポケモンの画面にもどる
		CancelFunc( wk, MODE_EXCHANGE_SELECT );
/**		
		// サーバーチェックの後タイトルメニューへ
		WorldTrade_SubProcessChange( wk, WORLDTRADE_PARTNER, 0 );
		wk->subprocess_seq  = SUBSEQ_END;		
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
*/		
	}else{
		if(wk->BoxCursorPos!=BOX_CUROSOR_TRAYNAME_POS){
			// ●●●を交換にだしますか？
			switch(PokemonCheck( wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos )){
			case WANT_POKE_OK:
				{
					POKEMON_PASO_PARAM *ppp = WorldTrade_GetPokePtr(wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos);

					// 名前を取得してWordSetに登録
					if(WantPokeCheck( ppp, &wk->DownloadPokemonData[wk->TouchTrainerPos].wantSimple)){
						if(CheckPocket( wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos )){
							WT_WORDSET_RegisterPokeNickNamePPP( wk->WordSet, 0, ppp);
							SubSeq_MessagePrint( wk, msg_gtc_01_019, 1, 0, 0x0f0f, 0 );
							WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT,  SUBSEQ_EXCHANGE_SELECT_LIST);
							OS_Printf("deposit_ppp0 = %08x\n",WorldTrade_GetPokePtr(wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos));
							PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
						}else{
							// てもちの最後の1匹だった場合はだせない
							SubSeq_MessagePrint( wk, msg_gtc_01_027, 1, 0, 0x0f0f, 1 );
							WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN);
						}
					}else{
						PMSND_PlaySE(SE_CANCEL);
					}
				}
				break;
			case WANT_POKE_TAMAGO: case WANT_POKE_NO:
				PMSND_PlaySE(SE_CANCEL);
				break;
			}
		}
	}
	
}

//------------------------------------------------------------------
/**
 * @brief   タッチパネル検出処理
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  u32		タッチ判定結果
 */
//------------------------------------------------------------------
static u32 TouchFunc( WORLDTRADE_WORK *wk )
{
	u32 ret = GFL_UI_TP_HitTrg( BoxTouchTable );
	if(ret!=GFL_UI_TP_HIT_NONE){
		OS_Printf("box touch = %d\n", ret);
	}
	return ret;
}

//------------------------------------------------------------------
/**
 * @brief   キャンセル処理
 *
 * @param   wk		GTS画面ワーク
 * @param   mode	BOX画面モード
 *
 * @retval  none	
 */
//------------------------------------------------------------------
static void CancelFunc( WORLDTRADE_WORK *wk, int mode )
{
	if(mode==MODE_DEPOSIT_SELECT){
		WorldTrade_SubProcessChange( wk, WORLDTRADE_TITLE, 0 );
		wk->subprocess_seq = SUBSEQ_END;
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
	}else if(mode==MODE_EXCHANGE_SELECT){
		// 検索結果ポケモンの画面にもどる
		WorldTrade_SubProcessChange( wk, WORLDTRADE_PARTNER, MODE_FROM_BOX );
		wk->subprocess_seq  = SUBSEQ_END;
		PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
	}
}


#define TOUCH_CANCEL		( 30 ) 
#define TOUCH_RIGHT_ARROW	( 31 )
#define TOUCH_LEFT_ARROW	( 32 )

//------------------------------------------------------------------
/**
 * @brief   サブプロセスシーケンスメイン
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_Main( WORLDTRADE_WORK *wk)
{
	// タッチで決定
	u32 result=TouchFunc( wk );
	if(result!=GFL_UI_TP_HIT_NONE){
		switch(result){
		case TOUCH_RIGHT_ARROW:
			//アニメ
			GFL_CLACT_WK_SetAutoAnmFlag( wk->BoxArrowActWork[0], 1 );
			GFL_CLACT_WK_SetAnmSeq( wk->BoxArrowActWork[0], CELL_BOXARROW_NO );
			
			wk->BoxTrayNo = RoundWork( wk->BoxTrayNo, 19, 1);
			NowBoxPageInfoGet( wk, wk->BoxTrayNo );
			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			break;
		case TOUCH_LEFT_ARROW:
			//アニメ
			GFL_CLACT_WK_SetAutoAnmFlag( wk->BoxArrowActWork[1], 1 );
			GFL_CLACT_WK_SetAnmSeq( wk->BoxArrowActWork[1], CELL_BOXARROW_NO+1 );

			wk->BoxTrayNo = RoundWork( wk->BoxTrayNo, 19, -1);
			NowBoxPageInfoGet( wk, wk->BoxTrayNo );
			PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			break;
		case TOUCH_CANCEL:
			// タイトルに戻ったり、検索画面に戻ったり
			CancelFunc( wk, wk->sub_process_mode );
			wk->BoxCursorPos = result;
			CursorPosPrioritySet( wk->CursorActWork, wk->BoxCursorPos );
			break;
		default:
			wk->BoxCursorPos = result;
			CursorPosPrioritySet( wk->CursorActWork, wk->BoxCursorPos );
			if(wk->sub_process_mode==MODE_DEPOSIT_SELECT){
				DepositDecideFunc(wk);
			}else if(wk->sub_process_mode==MODE_EXCHANGE_SELECT){
				ExchangeDecideFunc(wk);
			}
		}
	}else{
	// キー処理
		CursorControl(wk);
		// あずける時
		if(wk->sub_process_mode==MODE_DEPOSIT_SELECT){
			if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
				// タイトルメニューにもどる
				CancelFunc( wk, MODE_DEPOSIT_SELECT );
			}else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
				DepositDecideFunc(wk);
			}
			
		// 交換するとき
		}else if(wk->sub_process_mode==MODE_EXCHANGE_SELECT){
			if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
				// 検索結果ポケモンの画面にもどる
				CancelFunc( wk, MODE_EXCHANGE_SELECT );
			}else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
				ExchangeDecideFunc(wk);
			}
		}
	}

	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   カーソル制御
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static void CursorControl( WORLDTRADE_WORK *wk )
{
	int move=0, arrow=0, tmp=0;

	// カーソル操作
	if(GFL_UI_KEY_GetTrg() & PAD_KEY_UP){
		arrow = 1;
	}else if(GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN){
		arrow = 2;
	}else if(GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT){
		arrow = 3;
	}else if(GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT){
		arrow = 4;
	}

	// 上下左右が入った？
	if(arrow!=0){
		tmp = box_pos_table[wk->BoxCursorPos][arrow-1];
		if(tmp!=wk->BoxCursorPos){
			if(tmp==99||tmp==101){	// ボックス切り替え
				u8 arw_idx;
				//アニメ
				if (tmp == 101){
					arw_idx = 0;
				}else{
					arw_idx = 1;
				}
				GFL_CLACT_WK_SetAutoAnmFlag( wk->BoxArrowActWork[arw_idx], 1 );
				GFL_CLACT_WK_SetAnmSeq( wk->BoxArrowActWork[arw_idx], CELL_BOXARROW_NO+arw_idx );

				wk->BoxTrayNo = RoundWork( wk->BoxTrayNo, 19, tmp-100);
				NowBoxPageInfoGet( wk, wk->BoxTrayNo );
				PMSND_PlaySE(WORLDTRADE_DECIDE_SE);
			}else{
				move = 1;
				wk->BoxCursorPos = tmp;
			}
		}
	}
	
	// 移動した？
	if(move){
		PMSND_PlaySE(WORLDTRADE_MOVE_SE);
	}
	
	// カーソル位置指定＆プライオリティ指定
	CursorPosPrioritySet( wk->CursorActWork, wk->BoxCursorPos );

}



//------------------------------------------------------------------
/**
 * @brief   ボックス画面のカーソル座標指定処理
 *
 * @param   GFL_CLWK*	カーソルのセルアクター
 * @param   pos				位置番号
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void  CursorPosPrioritySet( GFL_CLWK* cursor, int pos )
{
	WorldTrade_CLACT_PosChange(cursor, PokemonIconPosTbl[pos][0], PokemonIconPosTbl[pos][1] );
	// カーソルプライオリティ操作
	if(pos==31 || (pos>=0 && pos<=5)){
		// ボックストレイ名の上にあるときとボックス１列目にいるときは最上位に
		GFL_CLACT_WK_SetBgPri( cursor, 0 );
	}else{
		// それ以外はひとつ下に
		GFL_CLACT_WK_SetBgPri( cursor, 1 );
	}
}


//------------------------------------------------------------------
/**
 * @brief   numにmoveを足し,0以下ならmax-1に、maxなら0にして返す
 *
 * @param   num		元の値
 * @param   max		最大値
 * @param   move	足す値（＋－あり）
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int RoundWork( int num, int max, int move )
{
	num += move;
	if(num < 0) {
		return max-1;
	}
	if(num==max){
		return 0;
	}
	return num;
}


//------------------------------------------------------------------
/**
 * @brief   選択リスト作成
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_SelectList( WORLDTRADE_WORK *wk )
{
/**	
	BMPMENU_HEADER temp;

	temp.font  = FONT_SYSTEM;
	temp.x_max = 1;
	temp.y_max = 3;
	temp.line_spc = 0;
	temp.c_disp_f = 0;
	temp.loop_f   = 0;
*/

	wk->BmpMenuList = BmpMenuWork_ListCreate( 3, HEAPID_WORLDTRADE );
	// ようすをみる
	BmpMenuWork_ListAddArchiveString( wk->BmpMenuList, wk->MsgManager, msg_gtc_05_005, 1, HEAPID_WORLDTRADE );
	// あずける
	BmpMenuWork_ListAddArchiveString( wk->BmpMenuList, wk->MsgManager, msg_gtc_05_006, 2, HEAPID_WORLDTRADE );
	// やめる
	BmpMenuWork_ListAddArchiveString( wk->BmpMenuList, wk->MsgManager, msg_gtc_05_007, 3, HEAPID_WORLDTRADE );

/*
	temp.menu = wk->BmpMenuList;
	temp.win  = &wk->MenuWin[0];

	// 枠描画
	// ----------------------------------------------------------------------------
	// tomoya takahashi	ローカライズの対処を反映
	// localize_spec_mark(LANG_ALL) imatake 2007/01/17
	// 描画時に一瞬ゴミが映るのを修正
	BmpWinFrame_Write( &wk->MenuWin[0], WINDOW_TRANS_OFF, WORLDTRADE_MENUFRAME_CHR, WORLDTRADE_MENUFRAME_PAL );
	// ----------------------------------------------------------------------------

	// BMPメニュー開始
	wk->BmpMenuWork = BmpMenuAddEx( &temp, 9, 0, 0, HEAPID_WORLDTRADE, PAD_BUTTON_CANCEL );
*/

	// 選択ボックス呼び出し
	wk->SelBoxWork = WorldTrade_SelBoxInit( wk, GFL_BG_FRAME0_M, 3, 10 );


	wk->subprocess_seq = SUBSEQ_SELECT_WAIT;

	return SEQ_MAIN;
	
}


//------------------------------------------------------------------
/**
 * @brief   「ようすをみる」「あずける」「やめる」選択待ち
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_SelectWait( WORLDTRADE_WORK *wk )
{
	POKEMON_PASO_PARAM *ppp;
	u32 ret = SelectBoxMain( wk->SelBoxWork );
	int error=0;
	
//	switch(BmpMenuMain( wk->BmpMenuWork )){
	switch(ret){
	// 「ようすをみる」
	case 1:
		WorldTrade_SelBoxEnd( wk );
		BmpMenuWork_ListDelete( wk->BmpMenuList );
		BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_ON );
		wk->subprocess_seq  = SUBSEQ_END;
		WorldTrade_SubProcessChange( wk, WORLDTRADE_STATUS, MODE_DEPOSIT_SELECT );
		break;

	// 「あずける」
	case 2:
		WorldTrade_SelBoxEnd( wk );
		BmpMenuWork_ListDelete( wk->BmpMenuList );

		ppp = WorldTrade_GetPokePtr( wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos);
		if( PokeRibbonCheck( ppp ) ){
			SubSeq_MessagePrint( wk, msg_gtc_01_038, 1, 0, 0x0f0f, 1 );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN);
		}
#if 0//origin
		else if((error=PokeNewFormCheck(ppp))){
#else 
			//warningになっていたので修正
		else if((PokeNewFormCheck(ppp))){
			error=PokeNewFormCheck(ppp);
#endif
			if(error==1){
				SubSeq_MessagePrint( wk, msg_gtc_new_form, 1, 0, 0x0f0f, 1 );
			}else{
				SubSeq_MessagePrint( wk, msg_gtc_new_form_002, 1, 0, 0x0f0f, 1 );
			}
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN);
		}
		else if(PokeNewItemCheck(ppp)){
			SubSeq_MessagePrint( wk, msg_gtc_new_item, 1, 0, 0x0f0f, 1 );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN);
		}
		else{
			int flag = 0;
			// てもちのときはカスタムボールチェック
			if(WorldTrade_GetPPorPPP( wk->BoxTrayNo )){
				POKEMON_PARAM *pp;
				pp = PokeParty_GetMemberPointer(wk->param->myparty, wk->BoxCursorPos);
				if(PP_Get( pp, ID_PARA_cb_id, NULL )){
					flag = 1;
					wk->subprocess_seq = SUBSEQ_CBALL_DEPOSIT_YESNO_MES;
				}
				OS_Printf("てもち指定 カスタム=%d\n", PP_Get( pp, ID_PARA_cb_id, NULL ));
			}

			// 問題なければポケモン預けへ
			if(flag==0){
				wk->deposit_ppp     = WorldTrade_GetPokePtr( wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos );
				wk->subprocess_seq  = SUBSEQ_END;
				WorldTrade_SubProcessChange( wk, WORLDTRADE_DEPOSIT, 0 );
				OS_Printf("deposit_ppp1 = %08x\n",WorldTrade_GetPokePtr(wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos));
			}
		}
		break;
		
	// 「やめる」
	case 3:case BMPMENU_CANCEL:
		WorldTrade_SelBoxEnd( wk );
		BmpMenuWork_ListDelete( wk->BmpMenuList );
		BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_ON );
		wk->subprocess_seq  = SUBSEQ_START;

		break;
	}
	
	return SEQ_MAIN;
}

//------------------------------------------------------------------
/**
 * @brief   選択リスト作成
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_ExchangeSelectList( WORLDTRADE_WORK *wk )
{
	BMPMENU_HEADER temp;

	temp.x_max = 1;
	temp.y_max = 3;
	temp.line_spc = 0;
	temp.c_disp_f = 0;
	temp.loop_f   = 0;


	wk->BmpMenuList = BmpMenuWork_ListCreate( 3, HEAPID_WORLDTRADE );
	// ようすをみる
	BmpMenuWork_ListAddArchiveString( wk->BmpMenuList, wk->MsgManager, msg_gtc_04_018, 1, HEAPID_WORLDTRADE );
	// こうかんする
	BmpMenuWork_ListAddArchiveString( wk->BmpMenuList, wk->MsgManager, msg_gtc_04_019, 2, HEAPID_WORLDTRADE );
	// やめる
	BmpMenuWork_ListAddArchiveString( wk->BmpMenuList, wk->MsgManager, msg_gtc_04_020, 3, HEAPID_WORLDTRADE );

/*
	temp.menu = wk->BmpMenuList;
	temp.win  = &wk->MenuWin[0];
	
	GFL_BMP_Clear( temp.win, 0x0f0f );

	// 枠描画
	BmpWinFrame_Write( &wk->MenuWin[0], WINDOW_TRANS_OFF, WORLDTRADE_MENUFRAME_CHR, WORLDTRADE_MENUFRAME_PAL );

	// BMPメニュー開始
	wk->BmpMenuWork = BmpMenuAddEx( &temp, 9, 0, 0, HEAPID_WORLDTRADE, PAD_BUTTON_CANCEL );
*/
	// 選択ボックス呼び出し
	wk->SelBoxWork = WorldTrade_SelBoxInit( wk, GFL_BG_FRAME0_M, 3, 10 );

	wk->subprocess_seq = SUBSEQ_EXCHANGE_SELECT_WAIT;

	return SEQ_MAIN;
	
}


//------------------------------------------------------------------
/**
 * @brief   「ようすをみる」「こうかんする」「やめる」選択待ち
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_ExchangeSelectWait( WORLDTRADE_WORK *wk )
{
	POKEMON_PASO_PARAM *ppp;
	int error=0;
//	switch(BmpMenuMain( wk->BmpMenuWork )){
	u32 ret = SelectBoxMain( wk->SelBoxWork );
	
	switch(ret){
	// 「ようすをみる」
	case 1:
		WorldTrade_SelBoxEnd( wk );
		BmpMenuWork_ListDelete( wk->BmpMenuList );
		wk->subprocess_seq  = SUBSEQ_END;
		 WorldTrade_SubProcessChange( wk, WORLDTRADE_STATUS, MODE_EXCHANGE_SELECT );
		break;

	// 「こうかんする」
	case 2:
		WorldTrade_SelBoxEnd( wk );
		BmpMenuWork_ListDelete( wk->BmpMenuList );
		BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_ON );

		ppp = WorldTrade_GetPokePtr( wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos);
		if( PokeRibbonCheck( ppp )){
			SubSeq_MessagePrint( wk, msg_gtc_01_038, 1, 0, 0x0f0f, 1 );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN);
		}
#if 0//origin
		else if((error=PokeNewFormCheck(ppp))){
#else
			//warningになっていたので修正
		else if((PokeNewFormCheck(ppp))){
			error=PokeNewFormCheck(ppp);
#endif
			if(error==1){
				SubSeq_MessagePrint( wk, msg_gtc_new_form, 1, 0, 0x0f0f, 1 );
			}else{
				SubSeq_MessagePrint( wk, msg_gtc_new_form_002, 1, 0, 0x0f0f, 1 );
			}
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN);
		}
		else if(PokeNewItemCheck(ppp)){
			SubSeq_MessagePrint( wk, msg_gtc_new_item, 1, 0, 0x0f0f, 1 );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN);
		}
		else{
			int flag = 0;
			// てもちのときはカスタムボールチェック
			if(WorldTrade_GetPPorPPP( wk->BoxTrayNo )){
				POKEMON_PARAM *pp;
				pp = PokeParty_GetMemberPointer(wk->param->myparty, wk->BoxCursorPos);
				if(PP_Get( pp, ID_PARA_cb_id, NULL )){
					flag = 1;
					wk->subprocess_seq = SUBSEQ_CBALL_YESNO_MES;
				}
				OS_Printf("てもち指定 カスタム=%d\n", PP_Get( pp, ID_PARA_cb_id, NULL ));
			}
			
			// 問題なければ交換へ
			if(flag==0){
				ExchangeCheck(wk);
			}
		}
		break;
		
	// 「やめる」
	case 3:case BMPMENU_CANCEL:
		WorldTrade_SelBoxEnd( wk );
		BmpMenuWork_ListDelete( wk->BmpMenuList );
		BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_ON );
		wk->subprocess_seq  = SUBSEQ_START;

		break;
	}
	
	return SEQ_MAIN;
}


//------------------------------------------------------------------
/**
 * @brief   サブプロセスシーケンス終了処理
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_End( WORLDTRADE_WORK *wk )
{
	// 接続画面だったら、上下画面でフェード
	if( wk->sub_nextprocess==WORLDTRADE_ENTER || wk->sub_nextprocess==WORLDTRADE_STATUS 
	 || wk->sub_nextprocess==WORLDTRADE_PARTNER ){
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
#ifdef GTS_FADE_OSP
		OS_Printf( "******************** worldtrade_box.c [1164] MS ********************\n" );
#endif
		wk->sub_out_flg = 1;
	}else{
		// 続きの画面にいく場合は上画面だけフェード
		WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, WORLDTRADE_WIPE_SPPED, 1, HEAPID_WORLDTRADE );
#ifdef GTS_FADE_OSP
		OS_Printf( "******************** worldtrade_box.c [1169] M ********************\n" );
#endif
	}
	wk->subprocess_seq = 0;
	
	return SEQ_FADEOUT;
}

//------------------------------------------------------------------
/**
 * @brief   はい・いいえ
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_YesNo( WORLDTRADE_WORK *wk)
{
	wk->tss = WorldTrade_TouchWinYesNoMake(WORLDTRADE_YESNO_PY1, YESNO_OFFSET, 8, 1 );
	wk->subprocess_seq = SUBSEQ_YESNO_SELECT;
	return SEQ_MAIN;
	
}


//------------------------------------------------------------------
/**
 * @brief   はい・いいえ選択
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_YesNoSelect( WORLDTRADE_WORK *wk)
{

  //@おそらくつかっていない

	u32 ret = WorldTrade_TouchSwMain(wk);

  GF_ASSERT( 0 );

	if(ret==TOUCH_SW_RET_YES){
		// タイトルメニューを終了
		TOUCH_SW_FreeWork( wk->tss );
		wk->subprocess_seq  = SUBSEQ_END;
		 WorldTrade_SubProcessChange( wk, WORLDTRADE_ENTER, 0 );
	}else if(ret==TOUCH_SW_RET_NO){
		// もういっかいトライ
		TOUCH_SW_FreeWork( wk->tss );
		wk->subprocess_seq = SUBSEQ_START;
	}

	return SEQ_MAIN;
	
}



//------------------------------------------------------------------
/**
 * @brief   「ボールカプセルが外れますが、よろしいですか？」
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_CBallYesNoMessage( WORLDTRADE_WORK *wk )
{
	SubSeq_MessagePrint( wk, msg_gtc_01_026, 1, 0, 0x0f0f, 1 );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT,  SUBSEQ_CBALL_YESNO );

	return SEQ_MAIN;
	
}

//------------------------------------------------------------------
/**
 * @brief   ボールカプセルが外れますが・・・はい・いいえ
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_CBallYesNo( WORLDTRADE_WORK *wk )
{
	wk->tss = WorldTrade_TouchWinYesNoMake( WORLDTRADE_YESNO_PY2, YESNO_OFFSET, 8, 1 );
//	wk->YesNoMenuWork = WorldTrade_BmpWinYesNoMake(WORLDTRADE_YESNO_PY2, YESNO_OFFSET );
	wk->subprocess_seq = SUBSEQ_CBALL_YESNO_SELECT;

	return SEQ_MAIN;
	
}


//------------------------------------------------------------------
/**
 * @brief   ボールカプセルが外れますが・・・・・はい・いいえ選択
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_CBallYesNoSelect( WORLDTRADE_WORK *wk )
{
	u32 ret = WorldTrade_TouchSwMain(wk);

	if(ret==TOUCH_SW_RET_YES){
		// 交換へ
		TOUCH_SW_FreeWork( wk->tss );
		ExchangeCheck(wk);
	}else if(ret==TOUCH_SW_RET_NO){
		// もういっかい
		TOUCH_SW_FreeWork( wk->tss );
		BmpWinFrame_Clear( wk->TalkWin, WINDOW_TRANS_ON );
		wk->subprocess_seq  = SUBSEQ_MAIN;
	}


/*
	int ret = BmpYesNoSelectMain( wk->YesNoMenuWork, HEAPID_WORLDTRADE );

	if(ret!=BMPMENU_NULL){
		if(ret==BMPMENU_CANCEL){
			// もういっかい
			BmpWinFrame_Clear( wk->TalkWin, WINDOW_TRANS_ON );
			wk->subprocess_seq  = SUBSEQ_MAIN;
		}else{
			// 交換へ
			ExchangeCheck(wk);
		}
	}
*/
	return SEQ_MAIN;
	
}


//------------------------------------------------------------------
/**
 * @brief   交換するためのチェック
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int ExchangeCheck( WORLDTRADE_WORK *wk )
{
	POKEMON_PARAM *pp;
	pp = (POKEMON_PARAM*)wk->DownloadPokemonData[wk->TouchTrainerPos].postData.data;

	// あいてのポケモンにメールがついていて、自分のてもちが６ひきだったときは交換できない。
	if(WorldTrade_PokemonMailCheck(pp) && wk->BoxTrayNo!=18){
		if(PokeParty_GetPokeCount(wk->param->myparty)==6){
			SubSeq_MessagePrint( wk, msg_gtc_01_029, 1, 0, 0x0f0f, 1 );
			WorldTrade_SetNextSeq( wk, SUBSEQ_MES_CLEAR_WAIT,  SUBSEQ_MAIN );
			return 0;
		}
	}

	wk->deposit_ppp     = WorldTrade_GetPokePtr( wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos );
	wk->subprocess_seq  = SUBSEQ_END;
	wk->sub_out_flg = 1;
	WorldTrade_SubProcessChange( wk, WORLDTRADE_UPLOAD, MODE_EXCHANGE );
	OS_Printf("deposit_ppp1 = %08x\n",WorldTrade_GetPokePtr(wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos));

	MakeExchangePokemonData( &wk->UploadPokemonData, wk );

	// 検索ヒット数をクリアしておかないと、デモ終了後にＯＢＪが表示されてしまう
	wk->SearchResult = 0;

	return 1;
}

//------------------------------------------------------------------
/**
 * @brief   「ボールカプセルが外れますが、よろしいですか？」
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_CBallDepositYesNoMessage( WORLDTRADE_WORK *wk )
{
	SubSeq_MessagePrint( wk, msg_gtc_01_026, 1, 0, 0x0f0f, 1 );
	WorldTrade_SetNextSeq( wk, SUBSEQ_MES_WAIT,  SUBSEQ_CBALL_DEPOSIT_YESNO );

	return SEQ_MAIN;
	
}

//------------------------------------------------------------------
/**
 * @brief   ボールカプセルが外れますが・・・はい・いいえ
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_CBallDepositYesNo( WORLDTRADE_WORK *wk )
{
//	wk->YesNoMenuWork = WorldTrade_BmpWinYesNoMake(WORLDTRADE_YESNO_PY2, YESNO_OFFSET );
	wk->tss = WorldTrade_TouchWinYesNoMake( WORLDTRADE_YESNO_PY2, YESNO_OFFSET, 8 ,1 );
	wk->subprocess_seq = SUBSEQ_CBALL_DEPOSIT_YESNO_SELECT;

	return SEQ_MAIN;
	
}


//------------------------------------------------------------------
/**
 * @brief   ボールカプセルが外れますが・・・・・はい・いいえ選択
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
static int SubSeq_CBallDepositYesNoSelect( WORLDTRADE_WORK *wk )
{

	u32 ret = WorldTrade_TouchSwMain(wk);

	if(ret==TOUCH_SW_RET_YES){
		// 交換へ
		TOUCH_SW_FreeWork( wk->tss );
		wk->deposit_ppp     = WorldTrade_GetPokePtr( wk->param->myparty, wk->param->mybox, 
														wk->BoxTrayNo, wk->BoxCursorPos );
		wk->subprocess_seq  = SUBSEQ_END;
		 WorldTrade_SubProcessChange( wk, WORLDTRADE_DEPOSIT, 0 );
		OS_Printf("deposit_ppp1 = %08x\n",WorldTrade_GetPokePtr(wk->param->myparty, wk->param->mybox, 
																wk->BoxTrayNo, wk->BoxCursorPos));
	}else if(ret==TOUCH_SW_RET_NO){
		// もういっかい
		TOUCH_SW_FreeWork( wk->tss );
		BmpWinFrame_Clear( wk->TalkWin, WINDOW_TRANS_ON );
		wk->subprocess_seq  = SUBSEQ_MAIN;
	}

/*
	int ret = BmpYesNoSelectMain( wk->YesNoMenuWork, HEAPID_WORLDTRADE );

	if(ret!=BMPMENU_NULL){
		if(ret==BMPMENU_CANCEL){
			// もういっかい
			BmpWinFrame_Clear( wk->TalkWin, WINDOW_TRANS_ON );
			wk->subprocess_seq  = SUBSEQ_MAIN;
		}else{
			// 交換へ
			wk->deposit_ppp     = WorldTrade_GetPokePtr( wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos );
			wk->subprocess_seq  = SUBSEQ_END;
			 WorldTrade_SubProcessChange( wk, WORLDTRADE_DEPOSIT, 0 );
			OS_Printf("deposit_ppp1 = %08x\n",WorldTrade_GetPokePtr(wk->param->myparty, wk->param->mybox, wk->BoxTrayNo, wk->BoxCursorPos));
		}
	}
*/
	return SEQ_MAIN;
	
}


//------------------------------------------------------------------
/**
 * @brief   会話終了を待って次のシーケンスへ
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
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
 * @brief   会話終了を待ってウインドウを消去し、次のシーケンスへ
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  none
 */
//------------------------------------------------------------------
static int SubSeq_MessageClearWait( WORLDTRADE_WORK *wk )
{
	if( GF_MSG_PrintEndCheck( &wk->print )==0){
		BmpWinFrame_Clear( wk->TalkWin, WINDOW_TRANS_ON );
		wk->subprocess_seq = wk->subprocess_nextseq;
	}
	return SEQ_MAIN;

}


























//------------------------------------------------------------------
/**
 * @brief   会話ウインドウ表示
 *
 * @param   wk		GTS画面ワーク
 *
 * @retval  int		サブシーケンス
 */
//------------------------------------------------------------------
//----------------------------------------------------------------------------------
/**
 * @brief	会話ウインドウ表示
 *
 * @param   wk		GTS画面ワーク
 * @param   msgno	メッセージNO
 * @param   wait	ウェイト（BmpPrintColor用）
 * @param   flag	0:出力先が一言ウインドウ  1:出力先が会話ウインドウ
 * @param   dat		未使用
 * @param   winflag	ウインドウを描画するか1:する	0:しない
 */
//----------------------------------------------------------------------------------
static void SubSeq_MessagePrint( WORLDTRADE_WORK *wk, int msgno, int wait, int flag, u16 dat, int winflag )
{
	GFL_BMPWIN *win;
	
	// 文字列取得
	STRBUF *tempbuf;
	
	// 文字列取得
	tempbuf = GFL_MSG_CreateString(  wk->MsgManager, msgno );

	// WORDSET展開
	WORDSET_ExpandStr( wk->WordSet, wk->TalkString, tempbuf );
	

	if(winflag==0){
		win = wk->MsgWin;
	}else{
		win = wk->TalkWin;
	}
	// 会話ウインドウ枠描画
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(win),  0x0f0f );
	BmpWinFrame_Write( win, WINDOW_TRANS_ON, WORLDTRADE_MESFRAME_CHR, WORLDTRADE_MESFRAME_PAL );

	// 文字列描画開始
	GF_STR_PrintSimple( win, FONT_TALK, wk->TalkString, 0, 0, &wk->print );
	GFL_BMPWIN_MakeTransWindow(win);


	GFL_STR_DeleteBuffer(tempbuf);
}


//==============================================================================
/**
 * @brief   ポケモンアイコンの読み込みを速くするためにハンドルを開いたハンドルから読み込む
 *
 * @param   handle		アーカイブハンドル
 * @param   dataIdx		データインデックス
 * @param   charData	開いたデータのキャラデータポインタが書き込まれる
 * @param   heapID		ヒープID
 *
 * @retval  void*		開いたファイルのバッファポインタ（これを後で解放する）
 */
//==============================================================================
static void* CharDataGetbyHandle( ARCHANDLE *handle, u32 dataIdx, NNSG2dCharacterData** charData, u32 heapID )
{
	void* arcData = GFL_ARC_LoadDataAllocByHandle( handle, dataIdx, heapID );

	if( arcData != NULL )
	{
		if( NNS_G2dGetUnpackedBGCharacterData( arcData, charData ) == FALSE)
		{
			// 失敗したらNULL
			GFL_HEAP_FreeMemory( arcData );
			return NULL;
		}
	}
	return arcData;
}

//------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンを転送してアクターのパレットを合わせる
 *
 * @param   chara	ポケモンNO
 * @param   pokeno	
 * @param   no		
 * @param   icon	
 *
 * @retval  none	
 */
//------------------------------------------------------------------
//----------------------------------------------------------------------------------
/**
 * @brief	ポケモンアイコンを転送してアクターのパレットを合わせる
 *
 * @param   pokeno	ポケモンNO
 * @param   form	フォルム
 * @param   tamago	タマゴフラグ（1:タマゴ
 * @param   no		転送先オフセット（ボックス分あるので0-29）
 * @param   icon	ポケモンアイコンのセルアクターのポインタ
 * @param   handle	アーカイブハンドル
 * @param   pbuf	書き込み先のVBLANK転送用バッファ
 */
//----------------------------------------------------------------------------------
static void TransPokeIconCharaPal( int pokeno, int form, int tamago, int no, GFL_CLWK* icon, ARCHANDLE* handle, WORLDTRADE_POKEBUF *pbuf )
{
	u8 *pokepal;
	u8 *buf;
	NNSG2dCharacterData *chara;

	// ポケモンアイコンのキャラデータをバッファの読み込む

	buf = CharDataGetbyHandle( handle, PokeIconCgxArcIndexGetByMonsNumber( pokeno, tamago, form ), 
								&chara, HEAPID_WORLDTRADE );
	// VBLANK中にキャラクタを転送するための準備
	MI_CpuCopyFast(chara->pRawData, pbuf->chbuf, POKEICON_TRANS_SIZE);
	pbuf->vadrs = (POKEICON_VRAM_OFFSET+no*POKEICON_TRANS_CHARA)*0x20;
	pbuf->icon = icon;
	pbuf->palno = PokeIconPalNumGet( pokeno, form, tamago )+POKEICON_PAL_OFFSET;
	GFL_HEAP_FreeMemory(buf);
}


//------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンの転送とセルアクターの表示・非表示を制御する
 *
 * @param   paso	POKEMON_PASO_PARAM
 * @param   dat		GTS送信データ
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void PokemonLevelSet(POKEMON_PASO_PARAM *paso, Dpw_Tr_PokemonDataSimple *dat )
{
	dat->level = PokePasoLevelCalc( paso );
}

//------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンの転送（VBLANK時に実行）
 *
 * @param   work	GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void PokemonIconDraw(void *work)
{
  WORLDTRADE_WORK *wk = work;
  
  int i;
  WORLDTRADE_POKEBUF *pbuf = wk->boxicon;

  for(i = 0; i < BOX_POKE_NUM; i++, pbuf++){
    if(pbuf->icon){
      // キャラクター転送
      DC_FlushRange(pbuf->chbuf, POKEICON_TRANS_SIZE);
      GX_LoadOBJ(pbuf->chbuf, pbuf->vadrs, POKEICON_TRANS_SIZE);
      // パレット設定
			GFL_CLACT_WK_SetPlttOffs( pbuf->icon, pbuf->palno, CLWK_PLTTOFFS_MODE_PLTT_TOP );
    }
  }
  GFL_HEAP_FreeMemory( wk->boxicon );
}

//----------------------------------------------------------------------------------
/**
 * @brief	ポケモンアイコンの転送とセルアクターの表示・非表示を制御する
 *
 * @param   paso		POKEMON_PASO_PARAM
 * @param   icon		ポケモンアイコンのセルアクター
 * @param   itemact		どうぐアイコンのセルアクター
 * @param   no			ポケモンNOを取得して書き込む
 * @param   pos			ボックス内の何番目か
 * @param   handle		アーカイブハンドル
 * @param   dat			指定のポケモンの基本情報を書き込む
 * @param   pbuf		VBLANK時に転送するデータを書き込むバッファ
 */
//----------------------------------------------------------------------------------
static void PokemonIconSet( POKEMON_PASO_PARAM *paso, GFL_CLWK* icon, 
							GFL_CLWK* itemact, u16 *no, int pos, ARCHANDLE* handle, 
							Dpw_Tr_PokemonDataSimple *dat, WORLDTRADE_POKEBUF *pbuf )
{
	int flag,itemno, tamago,form;
	
	PPP_FastModeOn(paso);

	flag   = PPP_Get(paso, ID_PARA_poke_exist, NULL);
	*no    = PPP_Get(paso, ID_PARA_monsno,     NULL);
	form   = PPP_Get(paso, ID_PARA_form_no,    NULL);
	tamago = PPP_Get(paso, ID_PARA_tamago_flag,NULL);
	itemno = PPP_Get(paso, ID_PARA_item,       NULL);
	dat->characterNo = *no;
	dat->gender      = PPP_Get( paso, ID_PARA_sex, NULL )+1;

	// 検索用にタマゴのレベルを０としてしまう（本当は０じゃないけど）
	if(tamago){
		dat->level = 0;
	}

	PPP_FastModeOff(paso,TRUE);
	
	if(flag){
		TransPokeIconCharaPal( *no, form, tamago, pos, icon, handle, pbuf );
		GFL_CLACT_WK_SetDrawEnable( icon, 1 );

		// アイテムを持っているか
		if(itemno!=0){	
			GFL_CLACT_WK_SetDrawEnable( itemact, 1 );

			// アイテムはメールか？
			if(ITEM_CheckMail( itemno )){	
				GFL_CLACT_WK_SetAnmSeq( itemact, CELL_MAILICON_NO );	// メール
			}else{
				GFL_CLACT_WK_SetAnmSeq( itemact, CELL_ITEMICON_NO );	// アイテム
			}
		}else{
			GFL_CLACT_WK_SetDrawEnable( itemact, 0 );
		}
	}else{
		GFL_CLACT_WK_SetDrawEnable( icon, 0 );
		GFL_CLACT_WK_SetDrawEnable( itemact, 0 );
		pbuf->icon = NULL;
	}
}

//------------------------------------------------------------------
/**
 * @brief   ボックス・てもちのポケモンデータを読み込む
 *
 * @param   wk		GTS画面ワーク
 * @param   now		0-17:ボックスNO、18:手持ち
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void NowBoxPageInfoGet( WORLDTRADE_WORK *wk, int now)
{
	u16 monsno[30],i,flag;
	POKEMON_PARAM      *pp;
	POKEMON_PASO_PARAM *paso;
	BOX_MANAGER *boxdata = wk->param->mybox;
	ARCHANDLE* handle;
	WORLDTRADE_POKEBUF *pokebuf;
	
	// ポケモンアイコンを転送する
	wk->boxicon = pokebuf =   GFL_HEAP_AllocMemoryLo(GFL_HEAPID_APP, sizeof(WORLDTRADE_POKEBUF) * BOX_POKE_NUM);

	handle = GFL_ARC_OpenDataHandle( ARCID_POKEICON, HEAPID_WORLDTRADE );

	// ボックス
	if(now>=0 && now <18){
		for(i=0;i<BOX_POKE_NUM;i++){
		  PokemonLevelSet(BOXDAT_GetPokeDataAddress( boxdata, now, i ), &wk->boxWork->info[i] );
		}
		for(i=0;i<BOX_POKE_NUM;i++){
			wk->boxWork->info[i].characterNo = 0;
			PokemonIconSet(BOXDAT_GetPokeDataAddress( boxdata, now, i ), 
							wk->PokeIconActWork[i], wk->ItemIconActWork[i],
							&monsno[i], i, handle, &wk->boxWork->info[i], &pokebuf[i] );
			// ボールカプセルのセルアクターはすべて隠す
			if(i<TEMOTI_POKEMAX){
				GFL_CLACT_WK_SetDrawEnable( wk->CBallActWork[i], 0 );
			}
		}

		// ボックスのトレイ名取得
		BOXDAT_GetBoxName( boxdata, now, wk->BoxTrayNameString );
	
	}else{
	// てもち
		int num = PokeParty_GetPokeCount( wk->param->myparty );

		for(i=0;i<num;i++){
			pp   = PokeParty_GetMemberPointer(wk->param->myparty, i);
			paso = (POKEMON_PASO_PARAM *)PP_GetPPPPointerConst(pp);
			PokemonLevelSet(paso, &wk->boxWork->info[i] );
			PokemonIconSet( paso, wk->PokeIconActWork[i], wk->ItemIconActWork[i],
							&monsno[i], i, handle, &wk->boxWork->info[i], &pokebuf[i] );

			// ボールカプセルをつけているか？
			if(PP_Get(pp, ID_PARA_cb_id, NULL)){
				GFL_CLACT_WK_SetDrawEnable( wk->CBallActWork[i], 1 );
			}else{
				GFL_CLACT_WK_SetDrawEnable( wk->CBallActWork[i], 0 );
			}
		}

		// てもちのポケモンを表示し終わったら後は全て非表示に
		for(;i<BOX_POKE_NUM;i++){
			wk->boxWork->info[i].characterNo = 0;
			GFL_CLACT_WK_SetDrawEnable( wk->PokeIconActWork[i], 0 );
			GFL_CLACT_WK_SetDrawEnable( wk->ItemIconActWork[i], 0 );
			pokebuf[i].icon = NULL;
			if(i<TEMOTI_POKEMAX){
				GFL_CLACT_WK_SetDrawEnable( wk->CBallActWork[i], 0 );
			}
		}
		GFL_MSG_GetString( wk->MsgManager, msg_gtc_04_023, wk->BoxTrayNameString );
	}
	GFL_ARC_CloseDataHandle( handle );
	
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->SubWin), 0x0000 );
	WorldTrade_SysPrint( wk->SubWin, wk->BoxTrayNameString, 0, 5, 1, PRINTSYS_LSB_Make(1,2,0), &wk->print );
	
	// 交換ポケモン選択モードの時は条件に合わないポケモンを暗くする
	if(wk->sub_process_mode==MODE_EXCHANGE_SELECT){
		PokeIconPalSet( wk->boxWork->info, wk->PokeIconActWork, &wk->DownloadPokemonData[wk->TouchTrainerPos].wantSimple, pokebuf);
	}
	
	// pokebufの開放はPokemonIconDraw内で行われる
	wk->vfunc = PokemonIconDraw;
}


//==============================================================================
/**
 * @brief   指定したポケモンデータはPOKEMON_PARAMか、POKEMON_PASO_PARAMか？
 *
 * @param   box		0-17:ボックスNO、18:手持ち
 *
 * @retval  int		0:POKEMON_PASO_PARAM 1:POKEMON_PARAM
 */
//==============================================================================
int WorldTrade_GetPPorPPP( int tray )
{
	if( tray == 18 ){
		return 1;
	}else{
		return 0;
	}
}

//==============================================================================
/**
 * @brief   番号からてもちかボックスの中のPOKEMON_PASO_PARAMへの構造体を返す
 *
 * @param   party	POKEPARTYポインタ
 * @param   box		ボックスのセーブデータ
 * @param   tray	0-17:ボックスNO、18:手持ち
 * @param   pos		ボックス位置（０－２９）手持ちの場合は0-5
 *
 * @retval  POKEMON_PASO_PARAM *		取得したポケモンデータのポインタ
 */
//==============================================================================
POKEMON_PASO_PARAM *WorldTrade_GetPokePtr( POKEPARTY *party, BOX_MANAGER *box,  int  tray, int pos )
{
	// てもち
	if(WorldTrade_GetPPorPPP( tray )){
		if(pos>(PokeParty_GetPokeCount(party)-1)){
			return NULL;
		}
		return  (POKEMON_PASO_PARAM *)PP_GetPPPPointerConst(PokeParty_GetMemberPointer( party, pos ));
	}

	// ボックス
	return BOXDAT_GetPokeDataAddress( box, tray, pos );
}

//==============================================================================
/**
 * @brief   てもちポケモンだったときに残りをチェックする
 *
 * @param   party	POKEPARTYポインタ
 * @param   box		ボックスのセーブデータ
 * @param   tray	0-17:ボックスNO、18:手持ち
 * @param   pos		ボックス位置（０－２９）手持ちの場合は0-5
 *
 * @retval  int		
 */
//==============================================================================
static int CheckPocket( POKEPARTY *party, BOX_MANAGER *box,  int  tray, int pos )
{
	// てもちだった時は残りを数える
	if(WorldTrade_GetPPorPPP( tray )){
		if( PokeParty_GetPokeCount(party) < 2 ){
			return 0;
		}
	}
	return 1;
}



//==============================================================================
// 拡張リボン用の定義
//==============================================================================

#define SPECIAL_RIBBON_NUM	( 10 )				///< 拡張リボンの総数

static const u16 SpRibbonTbl[]={
	ID_PARA_marine_ribbon,						///< マリンリボン
	ID_PARA_land_ribbon,						///< ランドリボン
	ID_PARA_sky_ribbon,							///< スカイリボン

	ID_PARA_sinou_red_ribbon,					///< シンオウレッドリボン
	ID_PARA_sinou_green_ribbon,					///< シンオウグリーンリボン
	ID_PARA_sinou_blue_ribbon,					///< シンオウブルーリボン
	ID_PARA_sinou_festival_ribbon,				///< シンオウフェスティバルリボン
	ID_PARA_sinou_carnival_ribbon,				///< シンオウカーニバルリボン
	ID_PARA_sinou_classic_ribbon,				///< シンオウクラシックリボン
	ID_PARA_sinou_premiere_ribbon,				///< シンオウプレミアリボン
};


//------------------------------------------------------------------
/**
 * @brief   特殊リボンを持っているかチェック
 *
 * @param   ppp		POKEMON_PASO_PARAM
 *
 * @retval  int		持っているリボンンの数
 */
//------------------------------------------------------------------
static int PokeRibbonCheck( POKEMON_PASO_PARAM *ppp )
{
	// 特殊リボンを持っていないか？
	int i, ret = 0, flag;

	flag = PPP_FastModeOn(ppp);
	for(i=0;i<SPECIAL_RIBBON_NUM;i++){
		ret += PPP_Get( ppp, SpRibbonTbl[i], NULL );
	}
	PPP_FastModeOff(ppp, flag);

	OS_Printf("特殊リボン取得数 %d\n", ret);

	// 特殊リボンをもっているのでダメ
	if(ret){
		return 1;
	}
	
	return 0;
}

//------------------------------------------------------------------
/**
 * @brief   DPにはないフォルムになっていないかチェック
 *
 * @param   ppp		POKEMON_PASO_PARAM
 *
 * @retval  int		0:問題無し 1:特殊フォルムになっている 2:ギザ耳ピチューなので問題
 */
//------------------------------------------------------------------
static int PokeNewFormCheck( POKEMON_PASO_PARAM *ppp )
{
	int flag;
	int monsno, form_no;
	
	flag = PPP_FastModeOn(ppp);
	{
		monsno = PPP_Get(ppp, ID_PARA_monsno, NULL);
		form_no = PPP_Get(ppp, ID_PARA_form_no, NULL);
	}
	PPP_FastModeOff(ppp, flag);

	if(form_no > 0){
		switch(monsno){
		case MONSNO_GIRATHINA:
		case MONSNO_SHEIMI:
		case MONSNO_ROTOMU:
			return 1;
			break;
		case MONSNO_PITYUU:		// ギザ耳ピチュー対応 
			return 2;
			break;
		}
	}
	return 0;
}

//------------------------------------------------------------------
/**
 * @brief   DPにはないアイテムを持っていないかチェック
 *
 * @param   ppp		POKEMON_PASO_PARAM
 *
 * @retval  int		TRUE:新規アイテムを持っている
 * @retval  int		FALSE:新規アイテムは持っていない
 */
//------------------------------------------------------------------
static int PokeNewItemCheck( POKEMON_PASO_PARAM *ppp )
{
	int flag;
	int item;
	
	flag = PPP_FastModeOn(ppp);
	{
		item = PPP_Get(ppp, ID_PARA_item, NULL);
	}
	PPP_FastModeOff(ppp, flag);

	switch(item){
	case ITEM_HAKKINDAMA:
		return TRUE;
	}
	return FALSE;
}

//==============================================================================
/**
 * @brief   指定のボックスNO・POSにはポケモンがいるか？
 *
 * @param   party	POKEPARTYポインタ
 * @param   box		BOXセーブデータ
 * @param   tray	0-17:ボックスNO、18:手持ち
 * @param   pos		ボックス位置（０－２９）手持ちの場合は0-5
 *
 * @retval  int		WANT_POKE_NO, WANT_POME_YES, WANT_POKE_TAMAGO
 */
//==============================================================================
static int PokemonCheck( POKEPARTY *party, BOX_MANAGER *box,  int  tray, int pos  )
{
	POKEMON_PASO_PARAM *ppp = WorldTrade_GetPokePtr(party, box, tray, pos );
	
#ifndef CHANGE_POKE_RULE_IGNORE
	
	// NULLチェック
	if(ppp==NULL){
		return WANT_POKE_NO;
	}

	// ポケモンはいるか？
	if(!PPP_Get(ppp, ID_PARA_poke_exist, NULL)){
		return WANT_POKE_NO;
	}

	// タマゴじゃないか？
	if(PPP_Get(ppp, ID_PARA_tamago_exist, NULL)){
		return WANT_POKE_TAMAGO;
	}

#endif

	return WANT_POKE_OK;
}


//------------------------------------------------------------------
/**
 * @brief   検索条件に合致しているか比較する
 *
 * @param   poke		検索条件
 * @param   search		検索結果
 *
 * @retval  int		0:合致していない	1:合致している
 */
//------------------------------------------------------------------
static int CompareSearchData( Dpw_Tr_PokemonDataSimple *poke,  Dpw_Tr_PokemonSearchData *search )
{
#ifndef CHANGE_POKE_RULE_IGNORE
	// ポケモンが違う
	if( poke->characterNo != search->characterNo){
		return 0;
	}
	
	// 性別指定と違う
	if( search->gender!=DPW_TR_GENDER_NONE){
		if(search->gender != poke->gender){
			return 0;
		}
	}
	
	// ポケモンがタマゴ
	if(poke->level==0){
		return 0;
	}
	
	// 指定の最低レベルよりも低い
	if( search->level_min!=0){
		if( search->level_min > poke->level){
			return 0;
		}
	}
	// 指定の最高レベルよりも高い
	if( search->level_max!=0){
		if( search->level_max < poke->level){
			return 0;
		}
	}
#endif
	
	// 合致している
	return 1;

}

//------------------------------------------------------------------
/**
 * @brief   相手が要求するポケモンにあっているか？
 *
 * @param   ppp		POKEMON_PASO_PARAM
 * @param   dtp		検索条件
 *
 * @retval  int		0:合致してない	1:してる
 */
//------------------------------------------------------------------
static int WantPokeCheck(POKEMON_PASO_PARAM *ppp, Dpw_Tr_PokemonSearchData *dtsd)
{
	Dpw_Tr_PokemonDataSimple temp;
	temp.characterNo = PPP_Get(ppp, ID_PARA_monsno, NULL);
	temp.gender      = PPP_Get( ppp, ID_PARA_sex,   NULL )+1;
	temp.level       = PokePasoLevelCalc(ppp);

	OS_Printf("want pokeno = %d, mypoke    = %d\n", dtsd->characterNo, temp.characterNo);
	OS_Printf("want sex    = %d, minesex   = %d\n", dtsd->gender     , temp.gender   );
	OS_Printf("want level_min  = %d, max  = %d, level = %d\n", dtsd->level_min,dtsd->level_max, temp.level );

	// 比較する
	return CompareSearchData( &temp, dtsd );
}

//------------------------------------------------------------------
/**
 * @brief   送信データの準備を行う
 *
 * @param   dtd		GTS送信データバッファ
 * @param   wk		GTS画面ワーク
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void MakeExchangePokemonData( Dpw_Tr_Data *dtd, WORLDTRADE_WORK *wk )
{
	Dpw_Tr_PokemonDataSimple post;
	Dpw_Tr_PokemonSearchData want;
	POKEMON_PASO_PARAM *ppp;

	// 名前・性別・レベル取得
	post.characterNo = PPP_Get( wk->deposit_ppp, ID_PARA_monsno, NULL );
	post.gender      = PPP_Get( wk->deposit_ppp, ID_PARA_sex,   NULL )+1;
	post.level       = PokePasoLevelCalc( wk->deposit_ppp );

	dtd->postSimple = post;

	WorldTrade_PostPokemonBaseDataMake( dtd, wk );

	ppp = (POKEMON_PASO_PARAM *)PP_GetPPPPointerConst( (POKEMON_PARAM*)wk->DownloadPokemonData[wk->TouchTrainerPos].postData.data );

	want.characterNo = PPP_Get( ppp, ID_PARA_monsno, NULL );
	want.gender      = PPP_Get( ppp, ID_PARA_sex,   NULL )+1;
	want.level_min   = 0;
	want.level_max   = 0;
	dtd->wantSimple  = want;

	OS_Printf( "postData  No = %d,  gender = %d, level = %d\n", dtd->postSimple.characterNo, dtd->postSimple.gender, dtd->postSimple.level);

}

//------------------------------------------------------------------
/**
 * @brief   検索条件に合わないポケモンアイコンのカラーを暗くする
 *
 * @param   box		GTSポケモン基本情報
 * @param   icon	ポケモンアイコンセルアクター
 * @param   want	
 *
 * @retval  none		
 */
//------------------------------------------------------------------
//----------------------------------------------------------------------------------
/**
 * @brief	検索条件に合わないポケモンアイコンのカラーを暗くする
 *
 * @param   box		GTSポケモン基本情報
 * @param   icon	ポケモンアイコンセルアクター	
 * @param   want	検索条件
 * @param   pbuf	VBLANK転送用画像バッファ
 */
//----------------------------------------------------------------------------------
static void PokeIconPalSet( Dpw_Tr_PokemonDataSimple *box, GFL_CLWK* *icon, Dpw_Tr_PokemonSearchData *want, WORLDTRADE_POKEBUF *pbuf)
{
	int pal,i;
	for(i=0;i<BOX_POKE_NUM;i++){
		if(box[i].characterNo!=0){
			if(CompareSearchData( &box[i], want )==0){
				pbuf[i].palno += 3;
			}
		}
	}
}


//------------------------------------------------------------------
/**
 * @brief   メールを持っているか？
 *
 * @param   pp		POKEMON_PARAM
 *
 * @retval  BOOL	TRUE:メール持ってる	FALSE:持ってない
 */
//------------------------------------------------------------------
BOOL WorldTrade_PokemonMailCheck( POKEMON_PARAM *pp )
{
	int itemno = PP_Get( pp, ID_PARA_item,       NULL);
	// アイテムはメールか？
	if(ITEM_CheckMail( itemno )){
		return TRUE;
	}
	
	return FALSE;
}
