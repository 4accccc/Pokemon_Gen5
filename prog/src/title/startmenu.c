//============================================================================================
/**
 * @file		startmenu.c
 * @brief		最初から・続きからを行うトップメニュー
 * @author	ariizumi
 * @author	Hiroyuki Nakamura
 * @data		09/01/07
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/main.h"
#include "system/wipe.h"
#include "system/gfl_use.h"
#include "system/blink_palanm.h"
#include "system/bgwinfrm.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menu.h"
#include "system/ds_system.h"
#include "savedata/mystatus.h"
#include "savedata/playtime.h"
#include "savedata/zukan_savedata.h"
#include "savedata/situation.h"
#include "savedata/misc.h"
#include "savedata/c_gear_data.h"
#include "sound/pm_sndsys.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "msg/msg_startmenu.h"
#include "msg/msg_wifi_system.h"
#include "field/zonedata.h"
#include "net/dwc_raputil.h"
#include "net_app/mystery.h"
#include "net/delivery_beacon.h"
#include "net_app/wifibattlematch.h"
#include "app/mictest.h"
#include "app/app_keycursor.h"
#include "gamesystem/msgspeed.h"
#include "multiboot/mb_parent_sys.h"  //転送マシン

#include "title/title.h"
#include "title/game_start.h"
#include "title/startmenu.h"
#include "title/startmenu_def.h"

#include "font/font.naix"
#include "startmenu.naix"
#include "wifileadingchar.naix"

#include "title_def.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"  // SYS_FLAG_SPEXIT_REQUEST
#include "field/eventwork.h"	//EVENTWORK_
#include "field/location.h"


//============================================================================================
//	定数定義
//============================================================================================

//ワーク
typedef struct {

	SAVE_CONTROL_WORK * savedata;
  MYSTATUS * mystatus;
	MISC * misc;
	CGEAR_SAVEDATA * cgear;
	EVENTWORK * evwk;

	GFL_TCB * vtask;					// TCB ( VBLANK )

	// OBJ
	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[OBJ_ID_MAX];
	u32	chrRes[CHRRES_MAX];
	u32	palRes[PALRES_MAX];
	u32	celRes[CELRES_MAX];

	GFL_FONT * font;					// 通常フォント
	GFL_MSGDATA * mman;				// メッセージデータマネージャ
	WORDSET * wset;						// 単語セット
	STRBUF * exp;							// メッセージ展開領域
	PRINT_QUE * que;					// プリントキュー
	PRINT_STREAM * stream;		// プリントストリーム
	GFL_TCBLSYS * tcbl;
	APP_KEYCURSOR_WORK * kcwk;	// メッセージ送りカーソル
	BOOL	stream_clear_flg;

	BMPMENU_WORK * mwk;

	PRINT_UTIL	util[BMPWIN_MAX];
	PRINT_UTIL	utilWin;

//	DELIVERY_BEACON_WORK * bwk;		//「不思議な贈り物」ビーコン管理ワーク
//	BOOL	hushigiFlag;						//「不思議な贈り物」を受信許可フラグ TRUE = 許可

	u16 * listFrame[LIST_ITEM_MAX];

	BGWINFRM_WORK * wfrm;

	BLINKPALANM_WORK * blink;		// カーソルアニメワーク

	u8	list[LIST_ITEM_MAX];
	u8	listPos;
	u8	listResult;

	u8	subSeq;
	u8	wait;
	u8	continueRet;

	u8	btnSeq;
	u8	btnCnt;

	s8	cursorPutPos;
	int	bgScroll;
	int	bgScrollCount;
	int	bgScrollSpeed;

	int	mainSeq;
	int	nextSeq;
	int	fadeSeq;

	u32	svBreak;		// セーブデータ破損フラグ
	u32	svBreakPos;	// セーブデータ破損メッセージ表示位置
 
}START_MENU_WORK;

// リスト項目データ
typedef struct {
	u16	scrnIdx;		// スクリーンインデックス
	u16	sy;					// Ｙサイズ
}LIST_ITEM_DATA;

enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_RELEASE,
	MAINSEQ_FADE,
	MAINSEQ_BUTTON_ANM,

	MAINSEQ_INIT_WAIT,
	MAINSEQ_MAIN,
	MAINSEQ_SCROLL,

	MAINSEQ_CONTINUE,
	MAINSEQ_NEWGAME,
//	MAINSEQ_HUSHIGI,
	MAINSEQ_WIFISET,

	MAINSEQ_SAVE_BREAK,

	MAINSEQ_END_SET,

	MAINSEQ_END,
};

typedef int (*pSTARTMENU_FUNC)(START_MENU_WORK*);

#define	SAVE_BREAK_MESSAGE_MAX		( 8 )		// セーブデータ破損メッセージ数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT START_MENU_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT START_MENU_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT START_MENU_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

static int MainSeq_Init( START_MENU_WORK * wk );
static int MainSeq_Release( START_MENU_WORK * wk );
static int MainSeq_Fade( START_MENU_WORK * wk );
static int MainSeq_ButtonAnm( START_MENU_WORK * wk );
static int MainSeq_InitWait( START_MENU_WORK * wk );
static int MainSeq_Main( START_MENU_WORK * wk );
static int MainSeq_Scroll( START_MENU_WORK * wk );
static int MainSeq_Continue( START_MENU_WORK * wk );
static int MainSeq_NewGame( START_MENU_WORK * wk );
//static int MainSeq_Hushigi( START_MENU_WORK * wk );
static int MainSeq_WiFiSet( START_MENU_WORK * wk );
static int MainSeq_SaveBreak( START_MENU_WORK * wk );
static int MainSeq_EndSet( START_MENU_WORK * wk );

static void InitVBlank( START_MENU_WORK * wk );
static void ExitVBlank( START_MENU_WORK * wk );
static void InitVram(void);
static void InitBg(void);
static void ExitBg(void);
static void LoadBgGraphic(void);
static void InitMsg( START_MENU_WORK * wk );
static void ExitMsg( START_MENU_WORK * wk );
static void InitBmp( START_MENU_WORK * wk );
static void ExitBmp( START_MENU_WORK * wk );
static void InitObj( START_MENU_WORK * wk );
static void ExitObj( START_MENU_WORK * wk );
static void InitBgWinFrame( START_MENU_WORK * wk );
static void ExitBgWinFrame( START_MENU_WORK * wk );
static void InitBlinkAnm( START_MENU_WORK * wk );
static void ExitBlinkAnm( START_MENU_WORK * wk );

static void InitList( START_MENU_WORK * wk );
static void LoadListFrame( START_MENU_WORK * wk );
static void UnloadListFrame( START_MENU_WORK * wk );
static void PutListObj( START_MENU_WORK * wk, u16 idx, s16 py );
static void InitListPut( START_MENU_WORK * wk );
static void PutListItem( START_MENU_WORK * wk, u8 item, s8 py );
static void ChangeListItemPalette( START_MENU_WORK * wk, u8 item, s8 py, u8 pal );
static s8 GetListPutY( START_MENU_WORK * wk, s8 py );

static void ScrollObj( START_MENU_WORK * wk, int val );
static void SetBlendAlpha(void);
static int SetFadeIn( START_MENU_WORK * wk, int next );
static int SetFadeOut( START_MENU_WORK * wk, int next );
static BOOL CursorMove( START_MENU_WORK * wk, s8 vec );
static void VanishListObj( START_MENU_WORK * wk, BOOL flg );
static void PutNewGameWarrning( START_MENU_WORK * wk );
static void ClearNewGameWarrning( START_MENU_WORK * wk, BOOL flg );
static void PutWiFiWarrning( START_MENU_WORK * wk );
static void StartMessage( START_MENU_WORK * wk, int strIdx );
static void ClearMessage( START_MENU_WORK * wk );
static BOOL MainMessage( START_MENU_WORK * wk );
static void SetYesNoMenu( START_MENU_WORK * wk );
static void PutContinueInfo( START_MENU_WORK * wk );
static void ClearContinueInfo( START_MENU_WORK * wk );
//static void InitHushigiCheck( START_MENU_WORK * wk );
//static void ExitHushigiCheck( START_MENU_WORK * wk );
//static BOOL CheckHushigiBeacon( START_MENU_WORK * wk );
static int SetButtonAnm( START_MENU_WORK * wk, int next );
static BOOL CheckSaveDataBreak( START_MENU_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================

FS_EXTERN_OVERLAY(pdw_acc);
extern const GFL_PROC_DATA PDW_ACC_MainProcData;

// PROC
const GFL_PROC_DATA StartMenuProcData = {
  START_MENU_ProcInit,
  START_MENU_ProcMain,
  START_MENU_ProcEnd,
};

// VRAM割り振り
static const GFL_DISP_VRAM VramTbl = {
	GX_VRAM_BG_128_A,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_128_D,					// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,		// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_NONE,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,					// テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_128K		// サブOBJマッピングモード
};

// メインシーケンス
static const pSTARTMENU_FUNC MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_Fade,
	MainSeq_ButtonAnm,

	MainSeq_InitWait,
	MainSeq_Main,
	MainSeq_Scroll,

	MainSeq_Continue,
	MainSeq_NewGame,
//	MainSeq_Hushigi,
	MainSeq_WiFiSet,

	MainSeq_SaveBreak,

	MainSeq_EndSet
};

// リスト項目データ
static const LIST_ITEM_DATA ListItemData[] =
{
	{ NARC_startmenu_list_frame1_lz_NSCR, LIST_FRAME_CONTINUE_SY },
	{ NARC_startmenu_list_frame2_lz_NSCR, LIST_FRAME_NEW_GAME_SY },
	{ NARC_startmenu_list_frame2_lz_NSCR, LIST_FRAME_HUSHIGI_SY },
	{ NARC_startmenu_list_frame2_lz_NSCR, LIST_FRAME_BATTLE_SY },
	{ NARC_startmenu_list_frame2_lz_NSCR, LIST_FRAME_GAME_SYNC_SY },
	{ NARC_startmenu_list_frame2_lz_NSCR, LIST_FRAME_WIFI_SET_SY },
	{ NARC_startmenu_list_frame2_lz_NSCR, LIST_FRAME_MIC_TEST_SY },
	{ NARC_startmenu_list_frame2_lz_NSCR, LIST_FRAME_MACHINE_SY },
};

// BMPWINデータ
static const u8 BmpWinData[][6] =
{
	{	//「続きから始める」
		BMPWIN_LIST_FRM, BMPWIN_TITLE_PX, BMPWIN_TITLE_PY,
		BMPWIN_TITLE_SX, BMPWIN_TITLE_SY, BMPWIN_LIST_PAL
	},
	{	// プレイヤー名
		BMPWIN_LIST_FRM, BMPWIN_NAME_PX, BMPWIN_NAME_PY,
		BMPWIN_NAME_SX, BMPWIN_NAME_SY, BMPWIN_LIST_PAL
	},
	{	// セーブ場所
		BMPWIN_LIST_FRM, BMPWIN_PLACE_PX, BMPWIN_PLACE_PY,
		BMPWIN_PLACE_SX, BMPWIN_PLACE_SY, BMPWIN_LIST_PAL
	},
	{	// プレイ時間
		BMPWIN_LIST_FRM, BMPWIN_TIME_PX, BMPWIN_TIME_PY,
		BMPWIN_TIME_SX, BMPWIN_TIME_SY, BMPWIN_LIST_PAL
	},
	{	// 図鑑
		BMPWIN_LIST_FRM, BMPWIN_ZUKAN_PX, BMPWIN_ZUKAN_PY,
		BMPWIN_ZUKAN_SX, BMPWIN_ZUKAN_SY, BMPWIN_LIST_PAL
	},
	{	// バッジ
		BMPWIN_LIST_FRM, BMPWIN_BADGE_PX, BMPWIN_BADGE_PY,
		BMPWIN_BADGE_SX, BMPWIN_BADGE_SY, BMPWIN_LIST_PAL
	},
	{	//「最初から始める」
		BMPWIN_LIST_FRM, BMPWIN_START_PX, BMPWIN_START_PY,
		BMPWIN_START_SX, BMPWIN_START_SY, BMPWIN_LIST_PAL
	},
	{	//「不思議な贈り物」
		BMPWIN_LIST_FRM, BMPWIN_HUSHIGI_PX, BMPWIN_HUSHIGI_PY,
		BMPWIN_HUSHIGI_SX, BMPWIN_HUSHIGI_SY, BMPWIN_LIST_PAL
	},
	{	//「バトル大会」
		BMPWIN_LIST_FRM, BMPWIN_BATTLE_PX, BMPWIN_BATTLE_PY,
		BMPWIN_BATTLE_SX, BMPWIN_BATTLE_SY, BMPWIN_LIST_PAL
	},
	{	//「ゲームシンク設定」
		BMPWIN_LIST_FRM, BMPWIN_GAMESYNC_PX, BMPWIN_GAMESYNC_PY,
		BMPWIN_GAMESYNC_SX, BMPWIN_GAMESYNC_SY, BMPWIN_LIST_PAL
	},
	{	//「Wi-Fi設定」
		BMPWIN_LIST_FRM, BMPWIN_WIFI_PX, BMPWIN_WIFI_PY,
		BMPWIN_WIFI_SX, BMPWIN_WIFI_SY, BMPWIN_LIST_PAL
	},
	{	//「マイクテスト」
		BMPWIN_LIST_FRM, BMPWIN_MIC_PX, BMPWIN_MIC_PY,
		BMPWIN_MIC_SX, BMPWIN_MIC_SY, BMPWIN_LIST_PAL
	},
	{	//「転送マシンを使う」
		BMPWIN_LIST_FRM, BMPWIN_MACHINE_PX, BMPWIN_MACHINE_PY,
		BMPWIN_MACHINE_SX, BMPWIN_MACHINE_SY, BMPWIN_LIST_PAL
	},

	{	// メッセージ
		BMPWIN_MSG_FRM, BMPWIN_MSG_PX, BMPWIN_MSG_PY,
		BMPWIN_MSG_SX, BMPWIN_MSG_SY, BMPWIN_MSG_PAL
	},
};

// OBJ
static const GFL_CLWK_DATA PlayerObjData = { PLAYER_OBJ_PX, PLAYER_OBJ_PY, 1, 0, 1 };
static const GFL_CLWK_DATA NewObjData = { NEW_OBJ_PX, 0, 0, 0, 1 };
static const GFL_CLWK_DATA WiconObjData = { WICON_OBJ_PX, 0, 1, 0, 1 };

// セーブデータ破損メッセージ表示テーブル
static const u32 SaveBreakMessage[][2] =
{	// ビット, メッセージ
	{	// レポートが消えた
		FST_NORMAL_BREAK_BIT,
		START_MENU_LOAD_WARNING_01
	},
	{	// バトルビデオが消えた（いずれか１つ以上）
		FST_EXTRA_REC_MINE_BREAK_BIT|FST_EXTRA_REC_DL_0_BREAK_BIT|FST_EXTRA_REC_DL_1_BREAK_BIT|FST_EXTRA_REC_DL_2_BREAK_BIT,
		START_MENU_LOAD_WARNING_02
	},
	{	// Cギア スキン
		FST_EXTRA_CGEAR_PICTURE_BREAK_BIT,
		START_MENU_LOAD_WARNING_03
	},
	{	// トライアルハウス
		FST_EXTRA_BATTLE_EXAMINATION_BREAK_BIT,
		START_MENU_LOAD_WARNING_04,
	},
	{	// ミュージカル
		FST_EXTRA_STREAMING_BREAK_BIT,
		START_MENU_LOAD_WARNING_05
	},
	{	// ポケモン図鑑 壁紙
		FST_EXTRA_ZUKAN_WALLPAPER_BREAK_BIT,
		START_MENU_LOAD_WARNING_06
	},
	{	// 殿堂入りポケモン
		FST_EXTRA_DENDOU_BREAK_BIT,
		START_MENU_LOAD_WARNING_07
	},
	{	// ふしぎなおくりもの（ビクティ）
		FST_OUTSIDE_MYSTERY_BREAK_BIT,
		START_MENU_LOAD_WARNING_08
	}
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：初期化
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT START_MENU_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  START_MENU_WORK * wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_STARTMENU, 0x80000 );
  wk = GFL_PROC_AllocWork( proc, sizeof(START_MENU_WORK), HEAPID_STARTMENU );
  GFL_STD_MemClear( wk, sizeof(START_MENU_WORK) );

	wk->savedata = SaveControl_GetPointer();
  wk->mystatus = SaveData_GetMyStatus( wk->savedata );
	wk->misc     = SaveData_GetMisc( wk->savedata );
	wk->cgear    = CGEAR_SV_GetCGearSaveData( wk->savedata );
	wk->evwk     = SaveData_GetEventWork( wk->savedata );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：メイン
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT START_MENU_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  START_MENU_WORK * wk = mywk;

	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
	  return GFL_PROC_RES_FINISH;
	}

  GFL_TCBL_Main( wk->tcbl );
	GFL_CLACT_SYS_Main();

  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		プロセス関数：終了
 *
 * @param		proc	プロセスデータ
 * @param		seq		シーケンス
 * @param		pwk		親ワーク
 * @param		mywk	自作ワーク
 *
 * @return	処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT START_MENU_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  START_MENU_WORK * wk;
	u8	result;
	u8	continueRet;
	
	wk = mywk;
	result = wk->listResult;
	continueRet = wk->continueRet;

	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_STARTMENU );

	switch( result ){
	case LIST_ITEM_CONTINUE:			// 続きから
    GAMEBEACON_AMPROTECT_SetTask(); //マジコン対策
//		GameStart_Continue();
		if( continueRet == 0 ){
			GameStart_ContinueNet();
		}else{
			GameStart_ContinueNetOff();
		}
		break;

	case LIST_ITEM_NEW_GAME:			// 最初から
    GAMEBEACON_AMPROTECT_SetTask(); //マジコン対策
		GameStart_Beginning();
		break;

	case LIST_ITEM_HUSHIGI:				// 不思議な贈り物
		GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(mystery), &MysteryGiftProcData, NULL);
		break;

	case LIST_ITEM_BATTLE:				// バトル大会
    { 
      WIFIBATTLEMATCH_PARAM *p_param  = GFL_HEAP_AllocMemory( GFL_HEAPID_APP, sizeof(WIFIBATTLEMATCH_PARAM) );
      GFL_STD_MemClear( p_param, sizeof(WIFIBATTLEMATCH_PARAM) );
      p_param->mode             = WIFIBATTLEMATCH_MODE_MAINMENU;
      p_param->is_auto_release  = TRUE; //下のPROCが終了時この引数を自分で削除する
      GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(wifibattlematch_sys), &WifiBattleMatch_ProcData, p_param );
    }
		break;

	case LIST_ITEM_GAME_SYNC:			// ゲームシンク設定
		GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &PDW_ACC_MainProcData, NULL);
		break;

	case LIST_ITEM_WIFI_SET:			// Wi-Fi設定
		GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(wifi_util), &WifiUtilProcData, NULL);
		break;

	case LIST_ITEM_MIC_TEST:			// マイクテスト
		GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(mictest), &TitleMicTestProcData, NULL);
		break;

	case LIST_ITEM_MACHINE:				// 転送マシンを使う
	  {
      MB_PARENT_INIT_WORK *initWork = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof(MB_PARENT_INIT_WORK) );
      initWork->mode = MPM_MOVIE_TRANS;
      //ゲームデータの確保とワークの開放はモードを見て行う。
  		GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(multiboot), &MultiBoot_ProcData, initWork);
  	}
		break;

	default:		// キャンセル
		GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
		break;
	}

  return GFL_PROC_RES_FINISH;
}


//============================================================================================
//	メインシーケンス
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：初期化
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Init( START_MENU_WORK * wk )
{
/*
	// セーブデータがない場合は「最初から」の処理に飛ばす
	if( SaveData_GetExistFlag( wk->savedata ) == FALSE ){
		wk->listResult = LIST_ITEM_NEW_GAME;
		return MAINSEQ_END;
	}
*/

#ifdef PM_DEBUG
	// 全メニュー表示許可
	if( SaveData_GetExistFlag( wk->savedata ) == TRUE ){
		if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ){
//			MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_HUSHIGI, MISC_STARTMENU_FLAG_OPEN );
			MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_BATTLE, MISC_STARTMENU_FLAG_OPEN );
			MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_GAMESYNC, MISC_STARTMENU_FLAG_OPEN );
			MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_MACHINE, MISC_STARTMENU_FLAG_OPEN );
		}
	}
#endif	// PM_DEBUG

	// ゲームシンク設定
	if( DREAMWORLD_SV_GetAccount( DREAMWORLD_SV_GetDreamWorldSaveData(wk->savedata) ) == TRUE ){
		MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_GAMESYNC, MISC_STARTMENU_FLAG_VIEW );
	}

	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// サブ画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN);

	InitVram();
	InitBg();
	LoadBgGraphic();
	InitMsg( wk );

	InitList( wk );
	LoadListFrame( wk );

	InitBmp( wk );
	InitObj( wk );

	InitBgWinFrame( wk );

	InitListPut( wk );

	InitBlinkAnm( wk );

	SetBlendAlpha();

//	InitHushigiCheck( wk );

	InitVBlank( wk );

	return MAINSEQ_INIT_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：解放
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Release( START_MENU_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:
//		ExitHushigiCheck( wk );
		wk->subSeq++;

	case 1:
		if( GFL_NET_IsResetEnable() == FALSE ){
			break;
		}

		ExitVBlank( wk );

		ExitBlinkAnm( wk );

		ExitBgWinFrame( wk );

		ExitObj( wk );
		ExitBmp( wk );

		UnloadListFrame( wk );

		ExitMsg( wk );
		ExitBg();

		// ブレンド初期化
		G2_BlendNone();
		G2S_BlendNone();
		// 表示初期化
		GFL_DISP_GX_SetVisibleControlDirect( 0 );
		GFL_DISP_GXS_SetVisibleControlDirect( 0 );

		return MAINSEQ_END;
	}

	return MAINSEQ_RELEASE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：フェード待ち
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Fade( START_MENU_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return wk->fadeSeq;
	}
	return MAINSEQ_FADE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：ボタンアニメ
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_ButtonAnm( START_MENU_WORK * wk )
{
	switch( wk->btnSeq ){
	case 0:
	case 2:
		if( wk->btnCnt == 0 ){
			ChangeListItemPalette(
				wk, wk->list[wk->listPos], GetListPutY(wk,wk->cursorPutPos), CURSOR_PALETTE2 );
			wk->btnCnt = 4;
			wk->btnSeq++;
		}else{
			wk->btnCnt--;
		}
		break;

	case 1:
	case 3:
		if( wk->btnCnt == 0 ){
			ChangeListItemPalette(
				wk, wk->list[wk->listPos], GetListPutY(wk,wk->cursorPutPos), LIST_PALETTE );
			wk->btnCnt = 4;
			wk->btnSeq++;
		}else{
			wk->btnCnt--;
		}
		break;

	case 4:
		return wk->nextSeq;
	}

	return MAINSEQ_BUTTON_ANM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：初期化待ち
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_InitWait( START_MENU_WORK * wk )
{
	u32	i;

	PRINTSYS_QUE_Main( wk->que );
	for( i=0; i<BMPWIN_MAX; i++ ){
		PRINT_UTIL_Trans( &wk->util[i], wk->que );
	}

	if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
		if( CheckSaveDataBreak( wk ) == TRUE ){
			return MAINSEQ_SAVE_BREAK;
		}
		return SetFadeIn( wk, MAINSEQ_MAIN );
	}
	return MAINSEQ_INIT_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メイン
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Main( START_MENU_WORK * wk )
{
	BLINKPALANM_Main( wk->blink );

/*
	// 不思議な贈り物受信チェック
	if( CheckHushigiBeacon( wk ) == TRUE ){
		return MAINSEQ_HUSHIGI;
	}
*/

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
		PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
		switch( wk->list[wk->listPos] ){
		case LIST_ITEM_CONTINUE:		// 続きから
			wk->listResult = wk->list[wk->listPos];
			return SetButtonAnm( wk, MAINSEQ_CONTINUE );

		case LIST_ITEM_NEW_GAME:		// 最初から
			return SetButtonAnm( wk, MAINSEQ_NEWGAME );

		case LIST_ITEM_HUSHIGI:			// 不思議な贈り物
//			MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_HUSHIGI, MISC_STARTMENU_FLAG_VIEW );
			wk->listResult = wk->list[wk->listPos];
			return SetButtonAnm( wk, MAINSEQ_END_SET );

		case LIST_ITEM_BATTLE:			// バトル大会
			MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_BATTLE, MISC_STARTMENU_FLAG_VIEW );
			wk->listResult = wk->list[wk->listPos];
			return SetButtonAnm( wk, MAINSEQ_END_SET );

		case LIST_ITEM_GAME_SYNC:		// ゲームシンク設定
			MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_GAMESYNC, MISC_STARTMENU_FLAG_VIEW );
			wk->listResult = wk->list[wk->listPos];
			return SetButtonAnm( wk, MAINSEQ_END_SET );

		case LIST_ITEM_MACHINE:			// 転送マシンを使う
			MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_MACHINE, MISC_STARTMENU_FLAG_VIEW );
			wk->listResult = wk->list[wk->listPos];
			return SetButtonAnm( wk, MAINSEQ_END_SET );

		case LIST_ITEM_WIFI_SET:		// Wi-Fi設定
//			wk->listResult = wk->list[wk->listPos];
//			return SetButtonAnm( wk, MAINSEQ_END_SET );
			return SetButtonAnm( wk, MAINSEQ_WIFISET );

		case LIST_ITEM_MIC_TEST:		// マイクテスト
			wk->listResult = wk->list[wk->listPos];
			return SetButtonAnm( wk, MAINSEQ_END_SET );
		}
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
		wk->listResult = LIST_ITEM_MAX;
		return SetFadeOut( wk, MAINSEQ_RELEASE );
	}

	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
		if( CursorMove( wk, -1 ) == TRUE ){
			PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
			return MAINSEQ_SCROLL;
		}
	}

	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		if( CursorMove( wk, 1 ) == TRUE ){
			PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
			return MAINSEQ_SCROLL;
		}
	}

	return MAINSEQ_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：リストスクロール
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Scroll( START_MENU_WORK * wk )
{
	if( wk->bgScrollCount == 3 ){
		wk->bgScrollCount = 0;
		wk->bgScrollSpeed = 0;
		BLINKPALANM_InitAnimeCount( wk->blink );
		ChangeListItemPalette( wk, wk->list[wk->listPos], GetListPutY(wk,wk->cursorPutPos), CURSOR_PALETTE );
		return MAINSEQ_MAIN;
	}

	wk->bgScroll += wk->bgScrollSpeed;
	if( wk->bgScroll < 0 ){
		wk->bgScroll += 512;
	}else if( wk->bgScroll >= 512 ){
		wk->bgScroll -= 512;
	}

	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, wk->bgScroll );
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, wk->bgScroll );

	ScrollObj( wk, -wk->bgScrollSpeed );

	wk->bgScrollCount++;

	return MAINSEQ_SCROLL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「続きから」処理
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Continue( START_MENU_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:
		// リスト非表示
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2, VISIBLE_OFF );
		VanishListObj( wk, FALSE );
		// C-GEARチェック
		if( CGEAR_SV_GetCGearONOFF(wk->cgear) == FALSE ){
			wk->continueRet = 1;
			wk->subSeq = 0;
			return SetFadeOut( wk, MAINSEQ_RELEASE );
		}
		wk->wait = CONTINUE_1ST_WAIT;
		wk->subSeq++;
		break;

	case 1:
		if( wk->wait != 0 ){
			wk->wait--;
			break;
		}
		// メッセージ表示
		StartMessage( wk, START_MENU_STR_CONTINUE_01 );
		PutContinueInfo( wk );
		wk->subSeq++;
		break;

	case 2:
		// メッセージ待ち
		if( MainMessage( wk ) == FALSE ){
			SetYesNoMenu( wk );
			wk->subSeq++;
		}
		break;

	case 3:
		// はい・いいえ待ち
		switch( BmpMenu_YesNoSelectMain( wk->mwk ) ){
		case 0:
			// 本体設定の無線設定
			if( DS_SYSTEM_IsAvailableWireless() == FALSE ){
				StartMessage( wk, START_MENU_STR_ATTENTION_01 );
				wk->subSeq = 5;
				break;
			}
/*	いらないらしい　2010/06/24
			// ペアレンタルコントロール
			if( DS_SYSTEM_IsRestrictUGC() == TRUE ){
				StartMessage( wk, START_MENU_STR_ATTENTION_02 );
				wk->subSeq = 5;
				break;
			}
*/
			wk->continueRet = 0;
			wk->subSeq = 4;
			break;

		case BMPMENU_CANCEL:
			StartMessage( wk, START_MENU_STR_CONTINUE_02 );
			wk->subSeq = 7;
//			wk->continueRet = 1;
//			wk->subSeq = 4;
			break;
		}
		break;

	case 4:		// 終了
		ClearContinueInfo( wk );
		ClearMessage( wk );
		wk->subSeq = 0;
		return SetFadeOut( wk, MAINSEQ_RELEASE );

	case 5:		// 無線設定エラー
		// メッセージ待ち
		if( MainMessage( wk ) == FALSE ){
			wk->subSeq++;
		}
		break;

	case 6:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
			wk->continueRet = 1;
			wk->subSeq = 4;
		}
		break;

	case 7:
		// メッセージ待ち
		if( MainMessage( wk ) == FALSE ){
			SetYesNoMenu( wk );
			wk->subSeq++;
		}
		break;

	case 8:
		// はい・いいえ待ち
		switch( BmpMenu_YesNoSelectMain( wk->mwk ) ){
		case 0:
			wk->continueRet = 1;
			wk->subSeq = 4;
			break;

		case BMPMENU_CANCEL:
			StartMessage( wk, START_MENU_STR_CONTINUE_01 );
			wk->subSeq = 2;
			break;
		}
		break;
	}

	PRINTSYS_QUE_Main( wk->que );
	PRINT_UTIL_Trans( &wk->utilWin, wk->que );

	return MAINSEQ_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「最初から」処理
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_NewGame( START_MENU_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:
		// 続きからが無い場合
		if( wk->list[0] == LIST_ITEM_NEW_GAME ){
			wk->listResult = LIST_ITEM_NEW_GAME;
			wk->subSeq = 0;
			return SetFadeOut( wk, MAINSEQ_RELEASE );
		}else{
			PutNewGameWarrning( wk );
			wk->subSeq++;
		}
		break;

	case 1:
		if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
			wk->subSeq++;
		}
		break;

	case 2:
		// Ａボタン
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
			PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
			ClearNewGameWarrning( wk, TRUE );
			wk->listResult = LIST_ITEM_NEW_GAME;
			wk->subSeq = 0;
			return SetFadeOut( wk, MAINSEQ_RELEASE );
		}
		// Ｂボタン
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
			PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
			ClearNewGameWarrning( wk, FALSE );
			ChangeListItemPalette(
				wk, wk->list[wk->listPos], GetListPutY(wk,wk->cursorPutPos), CURSOR_PALETTE );
			wk->subSeq = 0;
			return MAINSEQ_MAIN;
		}
		break;
	}

	PRINTSYS_QUE_Main( wk->que );
	PRINT_UTIL_Trans( &wk->utilWin, wk->que );

	return MAINSEQ_NEWGAME;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：「不思議な贈り物」受信
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
/*
static int MainSeq_Hushigi( START_MENU_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:
		// ビーコン取得終了待ち
		if( GFL_NET_IsResetEnable() == TRUE ){
			wk->subSeq++;
		}
		break;

	case 1:
		// リスト非表示
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2, VISIBLE_OFF );
		VanishListObj( wk, FALSE );
		wk->subSeq++;
		break;

	case 2:
		// メッセージ表示
		StartMessage( wk, START_MENU_STR_ATTENTION_03 );
		wk->subSeq++;
		break;

	case 3:
		// メッセージ待ち
		if( MainMessage( wk ) == FALSE ){
			wk->subSeq++;
		}
		break;

	case 4:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
			ClearMessage( wk );
			wk->subSeq++;
		}
		break;

	case 5:
		// リスト再構築
		MISC_SetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_HUSHIGI, MISC_STARTMENU_FLAG_OPEN );
		InitList( wk );
		InitListPut( wk );
		GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, wk->bgScroll );
		GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, wk->bgScroll );
		BLINKPALANM_InitAnimeCount( wk->blink );
		wk->subSeq++;
		break;

	case 6:
		// リスト表示
		VanishListObj( wk, TRUE );
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2, VISIBLE_ON );
		wk->subSeq = 0;
		return MAINSEQ_MAIN;
	}

	return MAINSEQ_HUSHIGI;
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		Wi-Fi設定
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_WiFiSet( START_MENU_WORK * wk )
{
	// DSi
	if( DS_SYSTEM_IsRunOnTwl() == TRUE ){
		switch( wk->subSeq ){
		case 0:
			PutWiFiWarrning( wk );
			wk->subSeq++;
			break;

		case 1:
			if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
				wk->subSeq++;
			}
			break;

		case 2:
			if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
				ClearNewGameWarrning( wk, FALSE );
				ChangeListItemPalette(
					wk, wk->list[wk->listPos], GetListPutY(wk,wk->cursorPutPos), CURSOR_PALETTE );
				wk->subSeq = 0;
				return MAINSEQ_MAIN;
			}
			break;
		}

		PRINTSYS_QUE_Main( wk->que );
		PRINT_UTIL_Trans( &wk->utilWin, wk->que );

		return MAINSEQ_WIFISET;
	}

	wk->listResult = wk->list[wk->listPos];
	return MAINSEQ_END_SET;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：セーブデータ破損
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_SaveBreak( START_MENU_WORK * wk )
{
	switch( wk->subSeq ){
	case 0:		// 画面作成
		GFL_DISP_GX_SetVisibleControl(
			GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3 | GX_PLANEMASK_OBJ, VISIBLE_OFF );
		GFL_DISP_GXS_SetVisibleControl(
			GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_OBJ, VISIBLE_OFF );
		GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, BLUE_BACK_GROUND_COLOR );
		GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, BLUE_BACK_GROUND_COLOR );
		wk->subSeq++;
		return SetFadeIn( wk, MAINSEQ_SAVE_BREAK );

	case 1:		// メッセージ表示
		while( 1 ){
			if( wk->svBreakPos >= SAVE_BREAK_MESSAGE_MAX ){
				ClearMessage( wk );
				wk->subSeq = 3;
				return SetFadeOut( wk, MAINSEQ_SAVE_BREAK );
			}
			if( wk->svBreak & SaveBreakMessage[wk->svBreakPos][0] ){
				break;
			}
			wk->svBreakPos++;
		}
		StartMessage( wk, SaveBreakMessage[wk->svBreakPos][1] );
		wk->svBreakPos++;
		wk->subSeq++;
		break;

	case 2:		// メッセージ待ち
		if( MainMessage( wk ) == FALSE ){
	    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
				PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
				wk->subSeq = 1;
			}
		}
		break;

	case 3:		// 画面復帰
		GFL_DISP_GX_SetVisibleControl(
			GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3 | GX_PLANEMASK_OBJ, VISIBLE_ON );
		GFL_DISP_GXS_SetVisibleControl(
			GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_OBJ, VISIBLE_ON );
		wk->subSeq = 0;
		return SetFadeIn( wk, MAINSEQ_MAIN );
	}

	return MAINSEQ_SAVE_BREAK;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：終了設定
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	次の処理
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_EndSet( START_MENU_WORK * wk )
{
	return SetFadeOut( wk, MAINSEQ_RELEASE );
}


//============================================================================================
//	初期化関連
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理
 *
 * @param		tcb		GFL_TCB
 * @param		work	タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
	GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理設定
 *
 * @param		wk	タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitVBlank( START_MENU_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理削除
 *
 * @param		wk	タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitVBlank( START_MENU_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitVram(void)
{
	GFL_DISP_ClearVRAM( NULL );
	GFL_DISP_SetBank( &VramTbl );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitBg(void)
{
	GFL_BG_Init( HEAPID_STARTMENU );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：メッセージウィンドウ
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：リスト文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：リスト背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xb000, GX_BG_CHARBASE_0x08000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_TEXT );
	}

	{	// サブ画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// サブ画面：フレーム
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// サブ画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &cnth, GFL_BG_MODE_TEXT );
	}

	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2, VISIBLE_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG削除
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

	GFL_BG_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧグラフィック読み込み
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void LoadBgGraphic(void)
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_STARTMENU, HEAPID_STARTMENU_L );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_startmenu_bgu_NCLR, PALTYPE_MAIN_BG, 0, 0x20*6, HEAPID_STARTMENU );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_startmenu_bgu_lz_NCGR, GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_STARTMENU );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_startmenu_base_bgu_lz_NSCR,
		GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_STARTMENU );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_startmenu_bgd_NCLR, PALTYPE_SUB_BG, 0, 0x20*6, HEAPID_STARTMENU );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_startmenu_bgd_lz_NCGR, GFL_BG_FRAME1_S, 0, 0, TRUE, HEAPID_STARTMENU );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_startmenu_base_bgd_lz_NSCR,
		GFL_BG_FRAME2_S, 0, 0, TRUE, HEAPID_STARTMENU );

	GFL_ARC_CloseDataHandle( ah );

	// システムウィンドウ
	BmpWinFrame_GraphicSet(
		GFL_BG_FRAME0_M, MESSAGE_WIN_CHAR_NUM,
		MESSAGE_WIN_PLTT_NUM, MENU_TYPE_SYSTEM, HEAPID_STARTMENU );

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		FONT_PALETTE_M*0x20, 0x20, HEAPID_STARTMENU );
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		FONT_PALETTE_S*0x20, 0x20, HEAPID_STARTMENU );

#if PM_VERSION == VERSION_WHITE
	// ホワイトの場合は背景のパレットを変更
	GFL_BG_ChangeScreenPalette( GFL_BG_FRAME3_M, 0, 0, 32, 24, BGPAL_VER_WHITE );
	GFL_BG_ChangeScreenPalette( GFL_BG_FRAME2_S, 0, 0, 32, 24, BGPAL_VER_WHITE );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソルアニメ初期化
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitBlinkAnm( START_MENU_WORK * wk )
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_STARTMENU, HEAPID_STARTMENU_L );

	wk->blink = BLINKPALANM_Create(
								CURSOR_PALETTE*16, 16, GFL_BG_FRAME2_M, HEAPID_STARTMENU );

	BLINKPALANM_SetPalBufferArcHDL(
		wk->blink, ah, NARC_startmenu_bgu_NCLR, CURSOR_PALETTE*16, CURSOR_PALETTE2*16 );

	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソルアニメ削除
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitBlinkAnm( START_MENU_WORK * wk )
{
	BLINKPALANM_Exit( wk->blink );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ初期化
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitMsg( START_MENU_WORK * wk )
{
	wk->mman = GFL_MSG_Create(
							GFL_MSG_LOAD_NORMAL,
							ARCID_MESSAGE, NARC_message_startmenu_dat, HEAPID_STARTMENU );
	wk->font = GFL_FONT_Create(
							ARCID_FONT, NARC_font_large_gftr,
							GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_STARTMENU );
	wk->wset = WORDSET_Create( HEAPID_STARTMENU );
	wk->que  = PRINTSYS_QUE_Create( HEAPID_STARTMENU );
	wk->exp  = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_STARTMENU );
	wk->tcbl = GFL_TCBL_Init( HEAPID_STARTMENU, HEAPID_STARTMENU, 1, 4 );
	wk->kcwk = APP_KEYCURSOR_Create( 15, TRUE, FALSE, HEAPID_STARTMENU );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ削除
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitMsg( START_MENU_WORK * wk )
{
	APP_KEYCURSOR_Delete( wk->kcwk );
  GFL_TCBL_Exit( wk->tcbl );
	GFL_STR_DeleteBuffer( wk->exp );
	PRINTSYS_QUE_Delete( wk->que );
	WORDSET_Delete( wk->wset );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ初期化
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitBmp( START_MENU_WORK * wk )
{
	const u8 * dat;
	STRBUF * str;
	u32	i;

	GFL_BMPWIN_Init( HEAPID_STARTMENU );

	for( i=0; i<BMPWIN_MAX; i++ ){
		dat = BmpWinData[i];
		wk->util[i].win = GFL_BMPWIN_Create(
												dat[0], dat[1], dat[2], dat[3], dat[4], dat[5], GFL_BMP_CHRAREA_GET_B );
	}

	// 項目文字列描画
	// 続きから始める
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_01_01 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_TITLE], wk->que, 0, 4, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// 主人公名
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_01_06 );
	WORDSET_RegisterPlayerName( wk->wset, 0, wk->mystatus );
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	if( MyStatus_GetMySex( wk->mystatus ) == PM_MALE ){
		PRINT_UTIL_PrintColor( &wk->util[BMPWIN_NAME], wk->que, 0, 0, wk->exp, wk->font, FCOL_WP05BLN );
	}else{
		PRINT_UTIL_PrintColor( &wk->util[BMPWIN_NAME], wk->que, 0, 0, wk->exp, wk->font, FCOL_WP05RN );
	}
	GFL_STR_DeleteBuffer( str );
	// 場所
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_01_10 );
	{
		u16	num;
		ZONEDATA_Open( HEAPID_STARTMENU_L );
		// 特殊接続
		if( EVENTWORK_CheckEventFlag( wk->evwk, SYS_FLAG_SPEXIT_REQUEST ) == TRUE ){
			SITUATION * situation;
			const LOCATION * spLoc;
			situation = SaveData_GetSituation( wk->savedata );
			spLoc = Situation_GetSpecialLocation( situation );
			num = ZONEDATA_GetPlaceNameID( spLoc->zone_id );
		// 通常
		}else{
			PLAYERWORK_SAVE	plwk;
			SaveData_SituationLoad_PlayerWorkSave( wk->savedata, &plwk );
			num = ZONEDATA_GetPlaceNameID( plwk.zoneID );
		}
		ZONEDATA_Close();
		WORDSET_RegisterPlaceName( wk->wset, 0, num );
	}
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_PLACE], wk->que, 0, 0, wk->exp, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// プレイ時間
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_01_03 );
	{
		PLAYTIME * ptime = SaveData_GetPlayTime( wk->savedata );
		WORDSET_RegisterNumber( wk->wset, 0, PLAYTIME_GetHour(ptime), 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
		WORDSET_RegisterNumber( wk->wset, 1, PLAYTIME_GetMinute(ptime), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	}
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_TIME], wk->que, 0, 0, wk->exp, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// 図鑑
	if( EVENTWORK_CheckEventFlag( wk->evwk, SYS_FLAG_ZUKAN_GET ) == TRUE ){
		str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_01_04 );
		{
			u16	num = ZUKANSAVE_GetZukanPokeSeeCount( ZUKAN_SAVEDATA_GetZukanSave(wk->savedata), HEAPID_STARTMENU );
			WORDSET_RegisterNumber( wk->wset, 0, num, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
		}
		WORDSET_ExpandStr( wk->wset, wk->exp, str );
		PRINT_UTIL_PrintColor( &wk->util[BMPWIN_ZUKAN], wk->que, 0, 0, wk->exp, wk->font, FCOL_WP05WN );
		GFL_STR_DeleteBuffer( str );
	}
	// バッジ
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_01_05 );
	{
		int	num = MISC_GetBadgeCount( SaveData_GetMisc(wk->savedata) );
		WORDSET_RegisterNumber( wk->wset, 0, num, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
	}
	WORDSET_ExpandStr( wk->wset, wk->exp, str );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_BADGE], wk->que, 0, 0, wk->exp, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );

	// 最初から始める
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_02 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_START], wk->que, 0, 4, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// 不思議な贈り物
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_03 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_HUSHIGI], wk->que, 0, 4, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// バトル大会
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_05 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_BATTLE], wk->que, 0, 4, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// ゲームシンク設定
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_06 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_GAMESYNC], wk->que, 0, 4, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// Wi-Fi設定
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_04 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_WIFI], wk->que, 0, 4, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// マイクテスト
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_08 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_MIC], wk->que, 0, 4, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// 転送マシンを使う
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ITEM_07 );
	PRINT_UTIL_PrintColor( &wk->util[BMPWIN_MACHINE], wk->que, 0, 4, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ削除
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitBmp( START_MENU_WORK * wk )
{
	u32	i;

	for( i=0; i<BMPWIN_MAX; i++ ){
		GFL_BMPWIN_Delete( wk->util[i].win );
	}

	GFL_BMPWIN_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪ初期化
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitObj( START_MENU_WORK * wk )
{
	{	// システム初期化
		const GFL_CLSYS_INIT init = {
			0, 0,									// メイン　サーフェースの左上座標
			0, 512,								// サブ　サーフェースの左上座標
			4,										// メイン画面OAM管理開始位置	4の倍数
			124,									// メイン画面OAM管理数				4の倍数
			4,										// サブ画面OAM管理開始位置		4の倍数
			124,									// サブ画面OAM管理数					4の倍数
			0,										// セルVram転送管理数
	
			CHRRES_MAX,						// 登録できるキャラデータ数
			PALRES_MAX,						// 登録できるパレットデータ数
			CELRES_MAX,						// 登録できるセルアニメパターン数
			0,										// 登録できるマルチセルアニメパターン数（※現状未対応）

		  16,										// メイン CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
		  16										// サブ CGR　VRAM管理領域　開始オフセット（キャラクタ単位）
		};
		GFL_CLACT_SYS_Create( &init, &VramTbl, HEAPID_STARTMENU );
	}

	{	// リソース読み込み
		ARCHANDLE * ah;
		
		// プレイヤー
		ah = GFL_ARC_OpenDataHandle( ARCID_WIFILEADING, HEAPID_STARTMENU_L );

		if( MyStatus_GetMySex( wk->mystatus ) == PM_MALE ){
			wk->chrRes[CHRRES_PLAYER] = GFL_CLGRP_CGR_Register(
																		ah, NARC_wifileadingchar_hero_NCGR,
																		FALSE, CLSYS_DRAW_MAIN, HEAPID_STARTMENU );
		  wk->palRes[PALRES_PLAYER] = GFL_CLGRP_PLTT_Register(
																		ah, NARC_wifileadingchar_hero_NCLR,
																		CLSYS_DRAW_MAIN, PALNUM_PLAYER*0x20, HEAPID_STARTMENU );
	    wk->celRes[CELRES_PLAYER] = GFL_CLGRP_CELLANIM_Register(
																		ah,
																		NARC_wifileadingchar_hero_NCER,
																		NARC_wifileadingchar_hero_NANR,
																		HEAPID_STARTMENU );
		}else{
			wk->chrRes[CHRRES_PLAYER] = GFL_CLGRP_CGR_Register(
																		ah, NARC_wifileadingchar_heroine_NCGR,
																		FALSE, CLSYS_DRAW_MAIN, HEAPID_STARTMENU );
		  wk->palRes[PALRES_PLAYER] = GFL_CLGRP_PLTT_Register(
																		ah, NARC_wifileadingchar_heroine_NCLR,
																		CLSYS_DRAW_MAIN, PALNUM_PLAYER*0x20, HEAPID_STARTMENU );
	    wk->celRes[CELRES_PLAYER] = GFL_CLGRP_CELLANIM_Register(
																		ah,
																		NARC_wifileadingchar_heroine_NCER,
																		NARC_wifileadingchar_heroine_NANR,
																		HEAPID_STARTMENU );
		}

		GFL_ARC_CloseDataHandle( ah );

		// その他
		ah = GFL_ARC_OpenDataHandle( ARCID_STARTMENU, HEAPID_STARTMENU_L );
		wk->chrRes[CHRRES_OBJ_U] = GFL_CLGRP_CGR_Register(
																ah, NARC_startmenu_obj_u_lz_NCGR,
																TRUE, CLSYS_DRAW_MAIN, HEAPID_STARTMENU );
	  wk->palRes[PALRES_OBJ_U] = GFL_CLGRP_PLTT_Register(
																ah, NARC_startmenu_obj_u_NCLR,
																CLSYS_DRAW_MAIN, PALNUM_OBJ_U*0x20, HEAPID_STARTMENU );
    wk->celRes[CELRES_OBJ_U] = GFL_CLGRP_CELLANIM_Register(
																ah,
																NARC_startmenu_obj_u_NCER,
																NARC_startmenu_obj_u_NANR,
																HEAPID_STARTMENU );
		wk->chrRes[CHRRES_WICON] = GFL_CLGRP_CGR_Register(
																ah, NARC_startmenu_wicon_lz_NCGR,
																TRUE, CLSYS_DRAW_MAIN, HEAPID_STARTMENU );
	  wk->palRes[PALRES_WICON] = GFL_CLGRP_PLTT_Register(
																ah, NARC_startmenu_wicon_NCLR,
																CLSYS_DRAW_MAIN, PALNUM_WICON*0x20, HEAPID_STARTMENU );
    wk->celRes[CELRES_WICON] = GFL_CLGRP_CELLANIM_Register(
																ah,
																NARC_startmenu_wicon_NCER,
																NARC_startmenu_wicon_NANR,
																HEAPID_STARTMENU );
		GFL_ARC_CloseDataHandle( ah );
	}

	{	// OBJ追加
		u32	i;

		wk->clunit = GFL_CLACT_UNIT_Create( OBJ_ID_MAX, 0, HEAPID_STARTMENU );

		wk->clwk[OBJ_ID_PLAYER] = GFL_CLACT_WK_Create(
																wk->clunit,
																wk->chrRes[CHRRES_PLAYER],
																wk->palRes[PALRES_PLAYER],
																wk->celRes[CELRES_PLAYER],
																&PlayerObjData, CLSYS_DRAW_MAIN, HEAPID_STARTMENU );
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_PLAYER], FALSE );

		for( i=OBJ_ID_NEW_HUSHIGI; i<=OBJ_ID_NEW_MACHINE; i++ ){
			wk->clwk[i] = GFL_CLACT_WK_Create(
											wk->clunit,
											wk->chrRes[CHRRES_OBJ_U],
											wk->palRes[PALRES_OBJ_U],
											wk->celRes[CELRES_OBJ_U],
											&NewObjData, CLSYS_DRAW_MAIN, HEAPID_STARTMENU );
			GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[i], TRUE );
			GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
		}

		wk->clwk[OBJ_ID_WICON] = GFL_CLACT_WK_Create(
															wk->clunit,
															wk->chrRes[CHRRES_WICON],
															wk->palRes[PALRES_WICON],
															wk->celRes[CELRES_WICON],
															&WiconObjData, CLSYS_DRAW_MAIN, HEAPID_STARTMENU );
	}

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// MAIN DISP OBJ ON
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// SUB DISP OBJ ON
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＯＢＪ削除
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitObj( START_MENU_WORK * wk )
{
	u32	i;

	for( i=0; i<OBJ_ID_MAX; i++ ){
		GFL_CLACT_WK_Remove( wk->clwk[i] );
	}
	GFL_CLACT_UNIT_Delete( wk->clunit );

	for( i=0; i<CHRRES_MAX; i++ ){
		GFL_CLGRP_CGR_Release( wk->chrRes[i] );
	}
	for( i=0; i<PALRES_MAX; i++ ){
    GFL_CLGRP_PLTT_Release( wk->palRes[i] );
	}
	for( i=0; i<CELRES_MAX; i++ ){
    GFL_CLGRP_CELLANIM_Release( wk->celRes[i] );
	}

	GFL_CLACT_SYS_Delete();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGWINFRAME初期化
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitBgWinFrame( START_MENU_WORK * wk )
{
	u32	i;

	wk->wfrm = BGWINFRM_Create( BGWINFRM_TRANS_NONE, LIST_ITEM_MAX, HEAPID_STARTMENU );

	for( i=0; i<LIST_ITEM_MAX; i++ ){
		BGWINFRM_Add( wk->wfrm, i, BMPWIN_LIST_FRM, LIST_FRAME_SX, ListItemData[i].sy );
	}

	// 続きから
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_CONTINUE, wk->util[BMPWIN_TITLE].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_CONTINUE, wk->util[BMPWIN_NAME].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_CONTINUE, wk->util[BMPWIN_PLACE].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_CONTINUE, wk->util[BMPWIN_TIME].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_CONTINUE, wk->util[BMPWIN_ZUKAN].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_CONTINUE, wk->util[BMPWIN_BADGE].win );

	// それ以外の項目
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_NEW_GAME, wk->util[BMPWIN_START].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_HUSHIGI, wk->util[BMPWIN_HUSHIGI].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_BATTLE, wk->util[BMPWIN_BATTLE].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_GAME_SYNC, wk->util[BMPWIN_GAMESYNC].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_WIFI_SET, wk->util[BMPWIN_WIFI].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_MIC_TEST, wk->util[BMPWIN_MIC].win );
	BGWINFRM_BmpWinOn( wk->wfrm, LIST_ITEM_MACHINE, wk->util[BMPWIN_MACHINE].win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGWINFRAME削除
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitBgWinFrame( START_MENU_WORK * wk )
{
	BGWINFRM_Exit( wk->wfrm );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		半透明設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetBlendAlpha(void)
{
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 16, 3 );
	G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, GX_BLEND_PLANEMASK_BG2, 16, 3 );
}


//============================================================================================
//	メニューリスト関連
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		リスト初期化
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitList( START_MENU_WORK * wk )
{
	u32	i;
	
	i = 0;
	// 続きから
	if( SaveData_GetExistFlag( wk->savedata ) == TRUE ){
		wk->list[i] = LIST_ITEM_CONTINUE;
		i++;
	}
	// 最初から
	wk->list[i] = LIST_ITEM_NEW_GAME;
	i++;
	// 不思議な贈り物
//	if( MISC_GetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_HUSHIGI ) != 0 ){
		wk->list[i] = LIST_ITEM_HUSHIGI;
		i++;
//	}
	// バトル大会
	if( MISC_GetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_BATTLE ) != 0 ){
		wk->list[i] = LIST_ITEM_BATTLE;
		i++;
	}
	// ゲームシンク設定
	if( MISC_GetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_GAMESYNC ) != 0 ){
		wk->list[i] = LIST_ITEM_GAME_SYNC;
		i++;
	}
	// Wi-Fi設定
	wk->list[i] = LIST_ITEM_WIFI_SET;
	i++;
	// マイクテスト
	wk->list[i] = LIST_ITEM_MIC_TEST;
	i++;
	// 転送マシンを使う
	if( MISC_GetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_MACHINE ) != 0 ){
		wk->list[i] = LIST_ITEM_MACHINE;
		i++;
	}

	// ダミーを入れておく
	for( i=i; i<LIST_ITEM_MAX; i++ ){
		wk->list[i] = LIST_ITEM_MAX;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リストフレーム読み込み
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void LoadListFrame( START_MENU_WORK * wk )
{
	ARCHANDLE * ah;
	u32	i;
	
	ah = GFL_ARC_OpenDataHandle( ARCID_STARTMENU, HEAPID_STARTMENU_L );

	for( i=0; i<LIST_ITEM_MAX; i++ ){
		const LIST_ITEM_DATA * item;
		NNSG2dScreenData * scrn;
		void * buf;
		u32	siz;

		item = &ListItemData[i];
		siz  = LIST_FRAME_SX * item->sy * 2;

		wk->listFrame[i] = GFL_HEAP_AllocMemory( HEAPID_STARTMENU, siz );
		buf = GFL_ARCHDL_UTIL_LoadScreen( ah, item->scrnIdx, TRUE, &scrn, HEAPID_STARTMENU );
		GFL_STD_MemCopy16( (void *)scrn->rawData, wk->listFrame[i], siz );
		GFL_HEAP_FreeMemory( buf );
	}

	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リストフレーム削除
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void UnloadListFrame( START_MENU_WORK * wk )
{
	u32	i;

	for( i=0; i<LIST_ITEM_MAX; i++ ){
		GFL_HEAP_FreeMemory( wk->listFrame[i] );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リスト上のOBJ配置
 *
 * @param		wk		タイトルメニューワーク
 * @param		idx		OBJ index
 * @param		py		表示Ｙ座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PutListObj( START_MENU_WORK * wk, u16 idx, s16 py )
{
	GFL_CLACTPOS	pos;

	GFL_CLACT_WK_GetPos( wk->clwk[idx], &pos, CLSYS_DRAW_MAIN );
	pos.y = py;
	GFL_CLACT_WK_SetPos( wk->clwk[idx], &pos, CLSYS_DRAW_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リスト配置（全体）
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitListPut( START_MENU_WORK * wk )
{
	u32	i;
	BOOL continue_flg;
	s8	py;

	wk->listPos = 0;
	wk->bgScroll = 0;
	wk->cursorPutPos = LIST_ITEM_PY;
	py = LIST_ITEM_PY;

	continue_flg = FALSE;

	for( i=0; i<LIST_ITEM_MAX; i++ ){
		if( wk->list[i] == LIST_ITEM_MAX ){
			break;
		}
		PutListItem( wk, wk->list[i], py );

		if( wk->list[i] == LIST_ITEM_CONTINUE ){
			GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_PLAYER], TRUE );
/*
		}else if( wk->list[i] == LIST_ITEM_HUSHIGI ){
			if( MISC_GetStartMenuFlag(wk->misc,MISC_STARTMENU_TYPE_HUSHIGI) == MISC_STARTMENU_FLAG_OPEN ){
				PutListObj( wk, OBJ_ID_NEW_HUSHIGI, py*8+ListItemData[wk->list[i]].sy*8/2 );
				GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_NEW_HUSHIGI], TRUE );
			}
*/
		}else if( wk->list[i] == LIST_ITEM_WIFI_SET ){
			PutListObj( wk, OBJ_ID_WICON, py*8+ListItemData[wk->list[i]].sy*8/2 );
			GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_WICON], TRUE );
		}else if( wk->list[i] == LIST_ITEM_BATTLE ){
			if( MISC_GetStartMenuFlag(wk->misc,MISC_STARTMENU_TYPE_BATTLE) == MISC_STARTMENU_FLAG_OPEN ){
				PutListObj( wk, OBJ_ID_NEW_BATTLE, py*8+ListItemData[wk->list[i]].sy*8/2 );
				GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_NEW_BATTLE], TRUE );
			}
		}else if( wk->list[i] == LIST_ITEM_GAME_SYNC ){
			if( MISC_GetStartMenuFlag(wk->misc,MISC_STARTMENU_TYPE_GAMESYNC) == MISC_STARTMENU_FLAG_OPEN ){
				PutListObj( wk, OBJ_ID_NEW_GAMESYNC, py*8+ListItemData[wk->list[i]].sy*8/2 );
				GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_NEW_GAMESYNC], TRUE );
			}
		}else if( wk->list[i] == LIST_ITEM_MACHINE ){
			if( MISC_GetStartMenuFlag(wk->misc,MISC_STARTMENU_TYPE_MACHINE) == MISC_STARTMENU_FLAG_OPEN ){
				PutListObj( wk, OBJ_ID_NEW_MACHINE, py*8+ListItemData[wk->list[i]].sy*8/2 );
				GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_NEW_MACHINE], TRUE );
			}
		}

		py += ListItemData[wk->list[i]].sy;
	}

	PutListObj( wk, OBJ_ID_PLAYER, PLAYER_OBJ_PY );

	ChangeListItemPalette( wk, wk->list[0], wk->cursorPutPos, CURSOR_PALETTE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リスト配置（項目ごと）
 *
 * @param		wk		タイトルメニューワーク
 * @param		item	項目
 * @param		py		表示Ｙ座標
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PutListItem( START_MENU_WORK * wk, u8 item, s8 py )
{
	GFL_BG_WriteScreenExpand(
		GFL_BG_FRAME2_M,
		LIST_ITEM_PX, py, LIST_FRAME_SX, ListItemData[item].sy,
		wk->listFrame[item],
		0, 0, LIST_FRAME_SX, ListItemData[item].sy );

	GFL_BG_WriteScreenExpand(
		GFL_BG_FRAME1_M,
		LIST_ITEM_PX, py, LIST_FRAME_SX, ListItemData[item].sy,
		BGWINFRM_FrameBufGet(wk->wfrm,item),
		0, 0, LIST_FRAME_SX, ListItemData[item].sy );

	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_M );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リスト項目パレット変更
 *
 * @param		wk		タイトルメニューワーク
 * @param		item	項目
 * @param		py		表示Ｙ座標
 * @param		pal		パレット
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ChangeListItemPalette( START_MENU_WORK * wk, u8 item, s8 py, u8 pal )
{
	GFL_BG_ChangeScreenPalette(
		GFL_BG_FRAME2_M, LIST_ITEM_PX, py, LIST_FRAME_SX, ListItemData[item].sy, pal );

	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_M );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		スクリーン上のＹ座標を取得
 *
 * @param		wk		タイトルメニューワーク
 * @param		py		画面上のＹ座標
 *
 * @return	スクリーン上のＹ座標
 */
//--------------------------------------------------------------------------------------------
static s8 GetListPutY( START_MENU_WORK * wk, s8 py )
{
	return ( py + ( wk->bgScroll / 8 ) );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リスト上のＯＢＪをスクロール
 *
 * @param		wk		タイトルメニューワーク
 * @param		val		移動値
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ScrollObj( START_MENU_WORK * wk, int val )
{
	GFL_CLACTPOS	pos;
	u32	i;

	for( i=0; i<OBJ_ID_MAX; i++ ){
		GFL_CLACT_WK_GetPos( wk->clwk[i], &pos, CLSYS_DRAW_MAIN );
		pos.y += val;
		GFL_CLACT_WK_SetPos( wk->clwk[i], &pos, CLSYS_DRAW_MAIN );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動
 *
 * @param		wk		タイトルメニューワーク
 * @param		vec		移動方向
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static BOOL CursorMove( START_MENU_WORK * wk, s8 vec )
{
	u8	now_item;
	u8	now_pos;
	u8	mv_item;
	s8	mv_pos;
	s8	cur_py;

	now_pos  = wk->listPos;
	now_item = wk->list[ now_pos ];

	wk->bgScrollCount = 0;

	if( vec < 0 ){
		if( wk->listPos == 0 ){
			return FALSE;
		}

		mv_pos  = now_pos + vec;
		mv_item = wk->list[ mv_pos ];

		cur_py = wk->cursorPutPos - ListItemData[mv_item].sy;

		ChangeListItemPalette( wk, now_item, GetListPutY(wk,wk->cursorPutPos), LIST_PALETTE );

		if( wk->list[0] == LIST_ITEM_CONTINUE ){
			if( cur_py < LIST_ITEM_PY ){
				if( mv_pos == 0 ){
					wk->bgScrollSpeed = ( ListItemData[ mv_item ].sy - (wk->cursorPutPos-LIST_ITEM_PY) ) / 3 * -8;
					wk->cursorPutPos = LIST_ITEM_PY;
				}else{
					wk->bgScrollSpeed = ListItemData[ mv_item ].sy / 3 * -8;
				}
			}else{
				wk->cursorPutPos = cur_py;
			}
		}else{
			wk->cursorPutPos = cur_py;
		}

	}else{
		if( ( now_pos + vec ) >= LIST_ITEM_MAX ){
			return FALSE;
		}
		if( wk->list[now_pos+vec] == LIST_ITEM_MAX ){
			return FALSE;
		}

		mv_pos  = now_pos + vec;
		mv_item = wk->list[ mv_pos ];

		cur_py = wk->cursorPutPos + ListItemData[now_item].sy;

		ChangeListItemPalette( wk, now_item, GetListPutY(wk,wk->cursorPutPos), LIST_PALETTE );

		if( wk->list[0] == LIST_ITEM_CONTINUE ){
			if( cur_py == 24 ){
				wk->bgScrollSpeed = ListItemData[ mv_item ].sy / 3 * 8;
			}else if( ( cur_py + ListItemData[mv_item].sy ) > 24 ){
				wk->bgScrollSpeed = ListItemData[ mv_item ].sy / 3 * 8;
			}else{
				wk->cursorPutPos = cur_py;
			}
		}else{
			wk->cursorPutPos = cur_py;
		}

	}

	wk->listPos += vec;

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リスト上のＯＢＪの表示切り替え
 *
 * @param		wk		タイトルメニューワーク
 * @param		flg		TRUE = 表示, FALSE = 非表示
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VanishListObj( START_MENU_WORK * wk, BOOL flg )
{
	u32	i;

	// 非表示
	if( flg == FALSE ){
		for( i=OBJ_ID_PLAYER; i<=OBJ_ID_WICON; i++ ){
			GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], flg );
		}
	// 表示
	}else{
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_PLAYER], TRUE );

		for( i=0; i<LIST_ITEM_MAX; i++ ){
			if( wk->list[i] == LIST_ITEM_MAX ){
				break;
			}
/*
			if( wk->list[i] == LIST_ITEM_HUSHIGI ){
				if( MISC_GetStartMenuFlag(wk->misc,MISC_STARTMENU_TYPE_HUSHIGI) == MISC_STARTMENU_FLAG_OPEN ){
					GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_NEW_HUSHIGI], TRUE );
				}
			}else if( wk->list[i] == LIST_ITEM_BATTLE ){
*/
			if( wk->list[i] == LIST_ITEM_BATTLE ){
				if( MISC_GetStartMenuFlag(wk->misc,MISC_STARTMENU_TYPE_BATTLE) == MISC_STARTMENU_FLAG_OPEN ){
					GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_NEW_BATTLE], TRUE );
				}
			}else if( wk->list[i] == LIST_ITEM_GAME_SYNC ){
				if( MISC_GetStartMenuFlag(wk->misc,MISC_STARTMENU_TYPE_GAMESYNC) == MISC_STARTMENU_FLAG_OPEN ){
					GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_NEW_GAMESYNC], TRUE );
				}
			}else if( wk->list[i] == LIST_ITEM_MACHINE ){
				if( MISC_GetStartMenuFlag(wk->misc,MISC_STARTMENU_TYPE_MACHINE) == MISC_STARTMENU_FLAG_OPEN ){
					GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_NEW_MACHINE], TRUE );
				}
			}else if( wk->list[i] == LIST_ITEM_WIFI_SET ){
				GFL_CLACT_WK_SetDrawEnable( wk->clwk[OBJ_ID_WICON], TRUE );
			}
		}
	}
}


//============================================================================================
//	各項目の処理
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		警告ウィンドウ表示
 *
 * @param		px		表示Ｘ座標
 * @param		py		表示Ｙ座標
 * @param		sx		Ｘサイズ
 * @param		sy		Ｙサイズ
 * @param		frm		ＢＧフレーム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PutWarrningWindow( u8 px, u8 py, u8 sx, u8 sy, u8 frm )
{
	// 左上
	GFL_BG_FillScreen( frm, WARRNING_WIN_CHAR_NUM, px, py, 1, 1, WARRNING_WIN_PLTT_NUM );
	// 右上
	GFL_BG_FillScreen( frm, WARRNING_WIN_CHAR_NUM+2, px+sx-1, py, 1, 1, WARRNING_WIN_PLTT_NUM );
	// 左下
	GFL_BG_FillScreen( frm, WARRNING_WIN_CHAR_NUM+6, px, py+sy-1, 1, 1, WARRNING_WIN_PLTT_NUM );
	// 右下
	GFL_BG_FillScreen( frm, WARRNING_WIN_CHAR_NUM+8, px+sx-1, py+sy-1, 1, 1, WARRNING_WIN_PLTT_NUM );
	// 上
	GFL_BG_FillScreen( frm, WARRNING_WIN_CHAR_NUM+1, px+1, py, sx-2, 1, WARRNING_WIN_PLTT_NUM );
	// 下
	GFL_BG_FillScreen( frm, WARRNING_WIN_CHAR_NUM+7, px+1, py+sy-1, sx-2, 1, WARRNING_WIN_PLTT_NUM );
	// 左
	GFL_BG_FillScreen( frm, WARRNING_WIN_CHAR_NUM+3, px, py+1, 1, sy-2, WARRNING_WIN_PLTT_NUM );
	// 右
	GFL_BG_FillScreen( frm, WARRNING_WIN_CHAR_NUM+5, px+sx-1, py+1, 1, sy-2, WARRNING_WIN_PLTT_NUM );
	// 中
	GFL_BG_FillScreen( frm, WARRNING_WIN_CHAR_NUM+4, px+1, py+1, sx-2, sy-2, WARRNING_WIN_PLTT_NUM );

	GFL_BG_LoadScreenV_Req( frm );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		警告ウィンドウクリア
 *
 * @param		px		表示Ｘ座標
 * @param		py		表示Ｙ座標
 * @param		sx		Ｘサイズ
 * @param		sy		Ｙサイズ
 * @param		frm		ＢＧフレーム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ClearWarrningWindow( u8 px, u8 py, u8 sx, u8 sy, u8 frm )
{
	GFL_BG_FillScreen( frm, 0, px, py, sx, sy, 0 );
	GFL_BG_LoadScreenV_Req( frm );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「最初から始める」の警告ウィンドウ表示
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PutNewGameWarrning( START_MENU_WORK * wk )
{
	STRBUF * str;

	wk->utilWin.win = GFL_BMPWIN_Create(
											BMPWIN_NEWGAME_WIN_FRM,
											BMPWIN_NEWGAME_WIN_PX, BMPWIN_NEWGAME_WIN_PY,
											BMPWIN_NEWGAME_WIN_SX, BMPWIN_NEWGAME_WIN_SY,
											BMPWIN_NEWGAME_WIN_PAL, GFL_BMP_CHRAREA_GET_B );

	// 注意
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ATTENTION_06 );
	PRINT_UTIL_PrintColor( &wk->utilWin, wk->que, 0, NEW_GAME_WARRNING_ATTENTION_PY, str, wk->font, FCOL_WP05RN );
	GFL_STR_DeleteBuffer( str );
	// 文章
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ATTENTION_07 );
	PRINT_UTIL_PrintColor( &wk->utilWin, wk->que, 0, NEW_GAME_WARRNING_MESSAGE_PY, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// Ａ
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ATTENTION_08 );
	PRINT_UTIL_PrintColor( &wk->utilWin, wk->que, 0, NEW_GAME_WARRNING_A_BUTTON_PY, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );
	// Ｂ
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ATTENTION_09 );
	PRINT_UTIL_PrintColor( &wk->utilWin, wk->que, 0, NEW_GAME_WARRNING_B_BUTTON_PY, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );

	GFL_BMPWIN_MakeScreen( wk->utilWin.win );
	GFL_BG_LoadScreenV_Req( BMPWIN_NEWGAME_WIN_FRM );

	// ウィンドウ描画
	PutWarrningWindow(
		NEW_GAME_WARRNING_WIN_PX, NEW_GAME_WARRNING_WIN_PY,
		NEW_GAME_WARRNING_WIN_SX, NEW_GAME_WARRNING_WIN_SY, GFL_BG_FRAME2_M );

	// リストを退避
	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_X_SET, 256 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, 0 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_X_SET, 256 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, 0 );

	VanishListObj( wk, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「最初から始める」の警告ウィンドウクリア
 *
 * @param		wk		タイトルメニューワーク
 * @param		flg		TRUE = 非表示
 *
 * @return	none
 *
 * @li	flg = FALSE の場合はBMPWINの削除のみ
 */
//--------------------------------------------------------------------------------------------
static void ClearNewGameWarrning( START_MENU_WORK * wk, BOOL flg )
{
	GFL_BMPWIN_Delete( wk->utilWin.win );

	if( flg == TRUE ){ return; }

	ClearWarrningWindow(
		NEW_GAME_WARRNING_WIN_PX, NEW_GAME_WARRNING_WIN_PY,
		NEW_GAME_WARRNING_WIN_SX, NEW_GAME_WARRNING_WIN_SY, GFL_BG_FRAME2_M );

	GFL_BG_ClearScreenCodeVReq( BMPWIN_NEWGAME_WIN_FRM, 0 );

	// リスト復帰
	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_X_SET, 0 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, wk->bgScroll );
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_X_SET, 0 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, wk->bgScroll );

	if( wk->list[0] == LIST_ITEM_CONTINUE ){
		VanishListObj( wk, TRUE );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「続きから始める」の情報ウィンドウ表示
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PutContinueInfo( START_MENU_WORK * wk )
{
	STRBUF * str;

	wk->utilWin.win = GFL_BMPWIN_Create(
											BMPWIN_CONTINUE_WIN_FRM,
											BMPWIN_CONTINUE_WIN_PX, BMPWIN_CONTINUE_WIN_PY,
											BMPWIN_CONTINUE_WIN_SX, BMPWIN_CONTINUE_WIN_SY,
											BMPWIN_CONTINUE_WIN_PAL, GFL_BMP_CHRAREA_GET_B );

	// 注意
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ATTENTION_04 );
	PRINT_UTIL_PrintColor( &wk->utilWin, wk->que, 0, CONTINUE_INFO_ATTENTION_PY, str, wk->font, FCOL_WP05RN );
	GFL_STR_DeleteBuffer( str );
	// 文章
	str = GFL_MSG_CreateString( wk->mman, START_MENU_STR_ATTENTION_05 );
	PRINT_UTIL_PrintColor( &wk->utilWin, wk->que, 0, CONTINUE_INFO_MESSAGE_PY, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );

	GFL_BMPWIN_MakeScreen( wk->utilWin.win );
	GFL_BG_LoadScreenV_Req( BMPWIN_CONTINUE_WIN_FRM );

	// ウィンドウ描画
	PutWarrningWindow(
		CONTINUE_INFO_WIN_PX, CONTINUE_INFO_WIN_PY,
		CONTINUE_INFO_WIN_SX, CONTINUE_INFO_WIN_SY, GFL_BG_FRAME1_S );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「続きから始める」の情報ウィンドウクリア
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ClearContinueInfo( START_MENU_WORK * wk )
{
	GFL_BMPWIN_Delete( wk->utilWin.win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「Wi-Fi設定」の警告ウィンドウ表示
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PutWiFiWarrning( START_MENU_WORK * wk )
{
	GFL_MSGDATA * mman;
	STRBUF * str;

	mman = GFL_MSG_Create(
							GFL_MSG_LOAD_NORMAL,
							ARCID_MESSAGE, NARC_message_wifi_system_dat, HEAPID_STARTMENU_L );

	wk->utilWin.win = GFL_BMPWIN_Create(
											BMPWIN_NEWGAME_WIN_FRM,
											BMPWIN_NEWGAME_WIN_PX, BMPWIN_NEWGAME_WIN_PY,
											BMPWIN_NEWGAME_WIN_SX, BMPWIN_NEWGAME_WIN_SY,
											BMPWIN_NEWGAME_WIN_PAL, GFL_BMP_CHRAREA_GET_B );

	str = GFL_MSG_CreateString( mman, dwc_message_0017 );
	PRINT_UTIL_PrintColor( &wk->utilWin, wk->que, 0, 0, str, wk->font, FCOL_WP05WN );
	GFL_STR_DeleteBuffer( str );

	GFL_MSG_Delete( mman );

	GFL_BMPWIN_MakeScreen( wk->utilWin.win );
	GFL_BG_LoadScreenV_Req( BMPWIN_NEWGAME_WIN_FRM );

	// ウィンドウ描画
	PutWarrningWindow(
		NEW_GAME_WARRNING_WIN_PX, NEW_GAME_WARRNING_WIN_PY,
		NEW_GAME_WARRNING_WIN_SX, NEW_GAME_WARRNING_WIN_SY, GFL_BG_FRAME2_M );

	// リストを退避
	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_X_SET, 256 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, 0 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_X_SET, 256 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, 0 );

	VanishListObj( wk, FALSE );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ表示開始
 *
 * @param		wk				タイトルメニューワーク
 * @param		strIdx		文字列インデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void StartMessage( START_MENU_WORK * wk, int strIdx )
{
  GFL_MSG_GetString( wk->mman, strIdx, wk->exp );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->util[BMPWIN_MSG].win), 15 );
	BmpWinFrame_Write(
		wk->util[BMPWIN_MSG].win, WINDOW_TRANS_OFF, MESSAGE_WIN_CHAR_NUM, MESSAGE_WIN_PLTT_NUM );

	wk->stream = PRINTSYS_PrintStream(
								wk->util[BMPWIN_MSG].win,
								0, 0, wk->exp,
								wk->font,
								MSGSPEED_GetWait(),
								wk->tcbl,
								10,		// tcbl pri
								HEAPID_STARTMENU,
								15 );	// clear color

	wk->stream_clear_flg = FALSE;

	GFL_BMPWIN_MakeTransWindow_VBlank( wk->util[BMPWIN_MSG].win );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ非表示
 *
 * @param		wk		タイトルメニューワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ClearMessage( START_MENU_WORK * wk )
{
	BmpWinFrame_Clear( wk->util[BMPWIN_MSG].win, WINDOW_TRANS_ON_V );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージメイン
 *
 * @param		wk		タイトルメニューワーク
 *
 * @retval	"TRUE = メッセージ処理中"
 * @retval	"FALSE = メッセージ処理終了"
 */
//--------------------------------------------------------------------------------------------
static BOOL MainMessage( START_MENU_WORK * wk )
{
	if( wk->stream == NULL ){ return FALSE; }

	APP_KEYCURSOR_Main( wk->kcwk, wk->stream, wk->util[BMPWIN_MSG].win );

  switch( PRINTSYS_PrintStreamGetState(wk->stream) ){
  case PRINTSTREAM_STATE_RUNNING: //実行中
    if( GFL_UI_KEY_GetCont() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
      PRINTSYS_PrintStreamShortWait( wk->stream, 0 );
    }
		wk->stream_clear_flg = FALSE;
    break;

  case PRINTSTREAM_STATE_PAUSE: //一時停止中
		if( wk->stream_clear_flg == FALSE ){
			if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ){
	      PMSND_PlaySystemSE( SEQ_SE_MESSAGE );
	      PRINTSYS_PrintStreamReleasePause( wk->stream );
				wk->stream_clear_flg = TRUE;
	    }
		}
    break;

  case PRINTSTREAM_STATE_DONE: //終了
    PRINTSYS_PrintStreamDelete( wk->stream );
		wk->stream = NULL;
		wk->stream_clear_flg = FALSE;
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ設定
 *
 * @param		wk		タイトルメニューワーク
 *
 * @retval	none
 */
//--------------------------------------------------------------------------------------------
static void SetYesNoMenu( START_MENU_WORK * wk )
{
	BMPWIN_YESNO_DAT	dat;

	dat.frmnum = BMPWIN_YESNO_FRM;
	dat.pos_x  = BMPWIN_YESNO_PX;
	dat.pos_y  = BMPWIN_YESNO_PY;
	dat.palnum = BMPWIN_YESNO_PAL;
	dat.chrnum = 0;

	wk->mwk = BmpMenu_YesNoSelectInit(
							&dat, MESSAGE_WIN_CHAR_NUM, MESSAGE_WIN_PLTT_NUM, 0, HEAPID_STARTMENU );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		「不思議な贈り物」受信を許可するか
 *
 * @param		wk		タイトルメニューワーク
 *
 * @retval	none
 */
//--------------------------------------------------------------------------------------------
/*
static void InitHushigiCheck( START_MENU_WORK * wk )
{
	if( MISC_GetStartMenuFlag( wk->misc, MISC_STARTMENU_TYPE_HUSHIGI ) == 0 ){
		DELIVERY_BEACON_INIT init;
    GFL_STD_MemClear( &init, sizeof(DELIVERY_BEACON_INIT) );
    init.NetDevID = WB_NET_MYSTERY;   // //通信種類
    init.data[0].datasize = 0;  //データ全体サイズ
    init.data[0].pData = NULL;     // 受信バッファデータ
    init.data[0].LangCode  = CasetteLanguage;     // 受け取る言語コード
    init.data[0].version   = 1<<GET_VERSION();     // 受け取るバージョンのビット
    init.dataNum = 1;  //受け取り側は１
    init.ConfusionID = 0;   //混線しないためのID
    init.heapID = HEAPID_STARTMENU;


		wk->bwk = DELIVERY_BEACON_Init( &init );
		DELIVERY_BEACON_RecvStart( wk->bwk );
		wk->hushigiFlag = TRUE;
	}else{
		wk->hushigiFlag = FALSE;
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「不思議な贈り物」ワーク削除
 *
 * @param		wk		タイトルメニューワーク
 *
 * @retval	none
 */
//--------------------------------------------------------------------------------------------
/*
static void ExitHushigiCheck( START_MENU_WORK * wk )
{
	if( wk->bwk != NULL ){
		DELIVERY_BEACON_End( wk->bwk );
		wk->bwk = NULL;
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		「不思議な贈り物」受信チェック
 *
 * @param		wk		タイトルメニューワーク
 *
 * @retval	none
 */
//--------------------------------------------------------------------------------------------
/*
static BOOL CheckHushigiBeacon( START_MENU_WORK * wk )
{
	// 受信不許可
	if( wk->hushigiFlag == FALSE ){
		return FALSE;
	}

	DELIVERY_BEACON_Main( wk->bwk );

	if( DELIVERY_BEACON_RecvSingleCheck(wk->bwk) == TRUE ){
		ExitHushigiCheck( wk );
		wk->hushigiFlag = FALSE;
		return TRUE;
	}
	return FALSE;
}
*/

//============================================================================================
//	その他
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードイン設定
 *
 * @param		wk		タイトルメニューワーク
 * @param		next	フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetFadeIn( START_MENU_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_STARTMENU );
	wk->fadeSeq = next;
	return MAINSEQ_FADE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウト設定
 *
 * @param		wk		タイトルメニューワーク
 * @param		next	フェード後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetFadeOut( START_MENU_WORK * wk, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_STARTMENU );
	wk->fadeSeq = next;
	return MAINSEQ_FADE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボタンアニメ設定
 *
 * @param		wk		タイトルメニューワーク
 * @param		next	アニメ後のシーケンス
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int SetButtonAnm( START_MENU_WORK * wk, int next )
{
	wk->btnCnt = 0;
	wk->btnSeq = 0;
	wk->nextSeq = next;
	return MAINSEQ_BUTTON_ANM;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		セーブデータ破損チェック
 *
 * @param		wk		タイトルメニューワーク
 *
 * @retval	"TRUE = 破損あり"
 * @retval	"FALSE = 破損なし"
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckSaveDataBreak( START_MENU_WORK * wk )
{
	wk->svBreak = SaveControl_GetLoadResult( wk->savedata );

	// 破損なし
	if( wk->svBreak == 0 ){
		return FALSE;
	}
	// 破損あり
	return TRUE;
}

















