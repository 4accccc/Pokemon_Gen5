//==============================================================================
/**
 * @file	gds_connect.c
 * @brief	GDSモード：Wifi接続画面
 * @author	matsuda
 * @date	2008.01.17(木)
 */
//==============================================================================
#include <gflib.h>
#include <dwc.h>
#include <ghttp/dwci_ghttp.h>
#include "net/dwc_rap.h"
#include "system/main.h"
#include "libdpw/dpw_tr.h"
#include "net/dwc_rap.h"
#include "savedata\save_control.h"
#include "gds_battle_rec.h"
#include "gds_ranking.h"
#include "gds_boxshot.h"
#include "gds_dressup.h"

#include "gamesystem/game_data.h"
#include "poke_tool/monsno_def.h"
//#include "gflib/strbuf_family.h"
#include "savedata/gds_profile.h"

#include <arc_tool.h>
#include "print/wordset.h"
#include "message.naix"
//#include "system/fontproc.h"
//#include "gflib/strbuf_family.h"

//#include "communication\comm_system.h"
//#include "communication\comm_state.h"
//#include "communication\comm_def.h"
//#include "communication/wm_icon.h"
//#include "communication\communication.h"

#include "gds_battle_rec.h"
#include "gds_ranking.h"
#include "gds_boxshot.h"
#include "gds_dressup.h"

#include "gds_rap.h"
#include "gds_rap_response.h"
#include "gds_data_conv.h"

#include "msg/msg_wifi_system.h"

#include "savedata/config.h"
#include "savedata\system_data.h"
#include "system/bmp_menu.h"
#include <procsys.h>
#include "system/wipe.h"

#include "msg/msg_wifi_lobby.h"
#include "msg/msg_wifi_gts.h"
#include "msg/msg_wifi_system.h"
//#include "../wifi_p2pmatch/wifip2pmatch.naix"			// グラフィックアーカイブ定義

#include "net_app/connect_anm.h"

#include "net_app/gds_main.h"
#include "arc_def.h"
#include "wifip2pmatch.naix"      // グラフィックアーカイブ定義
#include "font/font.naix"
#include "system/bmp_winframe.h"
#include "net/network_define.h"
#include "system/net_err.h"


//==============================================================================
//	定数定義
//==============================================================================
///GDSプロック制御が使用するヒープサイズ
#define GDSCONNECT_HEAP_SIZE		(0x50000)

///世界交換と同じにしておく(WORLDTRADE_WORDSET_BUFLEN)
#define GDSCONNECT_WORDSET_BUFLEN	( 64 )
// 会話ウインドウ文字列バッファ長
#define TALK_MESSAGE_BUF_NUM	( 90*2 )
#define DWC_ERROR_BUF_NUM		(16*8*2)

#define GDSCONNECT_MESFRAME_PAL	 ( 10 )
#define GDSCONNECT_MENUFRAME_PAL ( 11 )
#define GDSCONNECT_MESFRAME_CHR	 (  1 )
#define GDSCONNECT_MENUFRAME_CHR ( GDSCONNECT_MESFRAME_CHR + TALK_WIN_CGX_SIZ )
#define GDSCONNECT_TALKFONT_PAL	 ( 13 )

// １秒待つ用の定義
#define WAIT_ONE_SECONDE_NUM	( 30 )

//メッセージ一括描画の場合にセットするメッセージindex
#define GDSCONNECT_MSG_NO_WAIT		(0xff)


///サブシーケンスの戻り値
enum{
	SUBSEQ_CONTINUE,	///<継続
	SUBSEQ_END,			///<終了
};

//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	GDSPROC_MAIN_WORK *oya_proc_work;

	int				subprocess_seq;						// サブプログラムシーケンスNO
	int				subprocess_nextseq;					// サブプログラムNEXTシーケンスNO

	int				ConnectErrorNo;						// DWC・またはサーバーからのエラー
	int				ErrorRet;
	int				ErrorCode;
	int ErrorType;

	// 描画まわりのワーク（主にBMP用の文字列周り）
	PRINT_STREAM *print_stream;
	GFL_FONT *font_handle;
	PRINT_QUE *printQue;
	GFL_TCBLSYS *tcblsys;
	WORDSET			*WordSet;							// メッセージ展開用ワークマネージャー
	GFL_MSGDATA *MsgManager;						// 名前入力メッセージデータマネージャー
	GFL_MSGDATA *LobbyMsgManager;					// 名前入力メッセージデータマネージャー
	GFL_MSGDATA *SystemMsgManager;					// Wifiシステムメッセージデータ
	GFL_MSGDATA *GdsConnectMsgManager;					// Eメールメッセージデータマネージャー
	STRBUF			*TalkString;						// 会話メッセージ用
	STRBUF			*TitleString;						// タイトルメッセージ用
	STRBUF			*ErrorString;


	// BMPWIN描画周り
	GFL_BMPWIN *			MsgWin;					// 会話ウインドウ
	GFL_BMPWIN *			TitleWin;				// 「レコードコーナー　ぼしゅうちゅう！」など
	GFL_BMPWIN *			SubWin;					// 「レコードコーナー　ぼしゅうちゅう！」など
	GFL_BMPWIN *			list_bmpwin;			///<メニューリスト作成用BMPWIN

	BMPMENU_WORK			*YesNoMenuWork;
	void*					timeWaitWork;			// 会話ウインドウ横アイコンワーク
	PRINT_UTIL printutil_title;
	PRINT_UTIL printutil_subwin;

	int						wait;

	
	int local_seq;
	int local_wait;
	int local_work;
	
	//WIFI接続BGパレットアニメ制御構造体へのポインタ
	CONNECT_BG_PALANM cbp;
}GDS_CONNECT_SYS;

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT GdsConnectProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GdsConnectProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GdsConnectProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static void GdsConnectMenu_VramBankSet(void);
static void BgExit( void );
static void BgGraphicSet( GDS_CONNECT_SYS * wk );
static void BmpWinInit( GDS_CONNECT_SYS *wk );
static void BmpWinDelete( GDS_CONNECT_SYS *wk );
static int printCommonFunc( GFL_BMPWIN *win, STRBUF *strbuf, int x, int flag, PRINTSYS_LSB color, GFL_FONT *font_handle );
void GdsConnect_TalkPrint( PRINT_UTIL *print_util, STRBUF *strbuf, int x, int y, int flag, PRINTSYS_LSB color, GFL_FONT *font_handle, PRINT_QUE *printQue );
static void _systemMessagePrint( GDS_CONNECT_SYS *wk, int msgno );
static void errorDisp(GDS_CONNECT_SYS* wk, int type, int code);
static int Enter_MessagePrintEndCheck(GDS_CONNECT_SYS *wk);
static void GdsConnect_TimeIconAdd( GDS_CONNECT_SYS *wk );
static void GdsConnect_TimeIconDel( GDS_CONNECT_SYS *wk );
static BMPMENU_WORK *GdsConnect_BmpWinYesNoMake( int y, int yesno_bmp_cgx );
static void Enter_MessagePrint( GDS_CONNECT_SYS *wk, GFL_MSGDATA *msgman, int msgno, int wait, u16 dat );
static void GdsConnect_SetNextSeq( GDS_CONNECT_SYS *wk, int to_seq, int next_seq );

static BOOL Enter_GetErrTypeRetry( int errno, int errtype );

static int Enter_Start( GDS_CONNECT_SYS *wk);
static int Enter_InternetConnect( GDS_CONNECT_SYS *wk );
static int Enter_InternetConnectWait( GDS_CONNECT_SYS *wk );
static int Enter_WifiConnectionLogin( GDS_CONNECT_SYS *wk );
static int Enter_WifiConnectionLoginWait( GDS_CONNECT_SYS *wk );
static int Enter_DpwTrInit( GDS_CONNECT_SYS *wk );
static int Enter_DwcErrorPrint( GDS_CONNECT_SYS *wk );
static int Enter_ErrorPadWait( GDS_CONNECT_SYS *wk );
static int Enter_End( GDS_CONNECT_SYS *wk);
static int Enter_ServerServiceError( GDS_CONNECT_SYS *wk );
static int Enter_ServerServiceEnd( GDS_CONNECT_SYS *wk );
static int Enter_MessageWait( GDS_CONNECT_SYS *wk );
static int Enter_CleanupInet( GDS_CONNECT_SYS *wk );

//==============================================================================
//	
//==============================================================================
enum{
	ENTER_START,
	ENTER_INTERNET_CONNECT,
	ENTER_INTERNET_CONNECT_WAIT,
	ENTER_WIFI_CONNECTION_LOGIN,
	ENTER_WIFI_CONNECTION_LOGIN_WAIT,
	ENTER_DPWTR_INIT,
	ENTER_DWC_ERROR_PRINT,
	ENTER_ERROR_PAD_WAIT,
	ENTER_END,
	ENTER_CONNECT_END,
	ENTER_SERVER_SERVICE_ERROR,
	ENTER_SERVER_SERVICE_END,
	ENTER_MES_WAIT,
	ENTER_CLEANUP_INET,
};

static int (*Functable[])( GDS_CONNECT_SYS *wk ) = {
	Enter_Start,                        //ENTER_START,
	Enter_InternetConnect,              //ENTER_INTERNET_CONNECT,
	Enter_InternetConnectWait,          //ENTER_INTERNET_CONNECT_WAIT,
	Enter_WifiConnectionLogin,          //ENTER_WIFI_CONNECTION_LOGIN,
	Enter_WifiConnectionLoginWait,      //ENTER_WIFI_CONNECTION_LOGIN_WAIT,
	Enter_DpwTrInit,                    //ENTER_DPWTR_INIT,
	Enter_DwcErrorPrint,                //ENTER_DWC_ERROR_PRINT,
	Enter_ErrorPadWait,                 //ENTER_ERROR_PAD_WAIT,
	Enter_End,                          //ENTER_END,
	Enter_End,                          //ENTER_CONNECT_END,	通信したまま終了
	Enter_ServerServiceError,			//ENTER_SERVER_SERVICE_ERROR,
	Enter_ServerServiceEnd,				//ENTER_SERVER_SERVICE_END,
	Enter_MessageWait,					//ENTER_MES_WAIT,
	Enter_CleanupInet,					//ENTER_CLEANUP_INET,
};

//==============================================================================
//	データ
//==============================================================================
const GFL_PROC_DATA GdsConnectProcData =
{	
	GdsConnectProc_Init,
	GdsConnectProc_Main,
	GdsConnectProc_End,
};

//--------------------------------------------------------------
//	はい・いいえ　ウィンドウ
//--------------------------------------------------------------
// はい・いいえ
#define	BMP_YESNO_PX	( 23 )
#define	BMP_YESNO_PY	( 13 )
#define	BMP_YESNO_SX	( 7 )
#define	BMP_YESNO_SY	( 4 )
#define	BMP_YESNO_PAL	( 13 )

// はい・いいえ(ウインドウ用）
static const BMPWIN_YESNO_DAT YesNoBmpWin = {
  GFL_BG_FRAME0_M, BMP_YESNO_PX, BMP_YESNO_PY, BMP_YESNO_PAL, 
	0, //後で指定する
};

// はい・いいえウインドウのY座標
#define	GDSCONNECT_YESNO_PY2	( 13 )		// 会話ウインドウが２行の時
#define	GDSCONNECT_YESNO_PY1	( 15 )		// 会話ウインドウが１行の時




//--------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GdsConnectProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GDS_CONNECT_SYS *wk;
	
	//sys_HBlankIntrStop();	//HBlank割り込み停止

	GFL_DISP_GX_SetVisibleControlDirect(0);
	GFL_DISP_GXS_SetVisibleControlDirect(0);
	GX_SetVisiblePlane(0);
	GXS_SetVisiblePlane(0);
	GX_SetVisibleWnd(GX_WNDMASK_NONE);
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);
	G2_BlendNone();
	G2S_BlendNone();

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GDS_CONNECT, GDSCONNECT_HEAP_SIZE );

	wk = GFL_PROC_AllocWork(proc, sizeof(GDS_CONNECT_SYS), HEAPID_GDS_CONNECT );
	GFL_STD_MemClear(wk, sizeof(GDS_CONNECT_SYS));
	wk->oya_proc_work = pwk;

	//VRAM割り当て設定
	GdsConnectMenu_VramBankSet();

  //フォント読み込み
  wk->font_handle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_GDS_CONNECT );
	wk->printQue = PRINTSYS_QUE_Create(HEAPID_GDS_CONNECT);
	wk->tcblsys = GFL_TCBL_Init(HEAPID_GDS_CONNECT, HEAPID_GDS_CONNECT, 8, 32);

	//メッセージマネージャ作成
	wk->WordSet    		 = WORDSET_CreateEx( 11, GDSCONNECT_WORDSET_BUFLEN, HEAPID_GDS_CONNECT );
	wk->MsgManager       = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_gts_dat, HEAPID_GDS_CONNECT );
	wk->LobbyMsgManager  = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_lobby_dat, HEAPID_GDS_CONNECT );
	wk->SystemMsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_system_dat, HEAPID_GDS_CONNECT );

	// 文字列バッファ作成
	wk->TalkString  = GFL_STR_CreateBuffer( TALK_MESSAGE_BUF_NUM, HEAPID_GDS_CONNECT );
	wk->ErrorString = GFL_STR_CreateBuffer( DWC_ERROR_BUF_NUM,    HEAPID_GDS_CONNECT );
	wk->TitleString = GFL_MSG_CreateString( wk->MsgManager, msg_gtc_01_032 );

	// BGグラフィック転送
	BgGraphicSet( wk );

	// BMPWIN確保
	BmpWinInit( wk );

	// ワイプフェード開始
	WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK, 
		WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_GDS_CONNECT );

	// BG面表示ON
	GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON );	//通信アイコン
	GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG0, VISIBLE_ON );
	GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_BG1, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );

	//メイン画面設定
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);

#if 0 //※check
	MsgPrintSkipFlagSet(MSG_SKIP_ON);
	MsgPrintAutoFlagSet(MSG_AUTO_OFF);
	MsgPrintTouchPanelFlagSet(MSG_TP_OFF);
#endif

	//実行する動作を決定
	if(wk->oya_proc_work->connect_success){	//既にネットに繋がっているので切断処理へ移行
		GdsConnect_SetNextSeq(wk, ENTER_CLEANUP_INET, ENTER_END);
	}
	else{	//ネットに繋がっていないので繋げに行く
		if(wk->oya_proc_work->proc_param->connect){
			//初回接続
			wk->subprocess_seq = ENTER_INTERNET_CONNECT;	//「はい/いいえ」選択を飛ばす
		}
		else{
			wk->subprocess_seq = ENTER_START;
		}
	}
	
	wk->oya_proc_work->ret_connect = FALSE;

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：メイン
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GdsConnectProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GDS_CONNECT_SYS * wk  = mywk;
	int ret, temp_subprocess_seq;
	enum{
		SEQ_INIT,
		SEQ_MAIN,
		SEQ_END,
	};
	
	switch(*seq){
	case SEQ_INIT:
		if(WIPE_SYS_EndCheck() == TRUE){
			*seq = SEQ_MAIN;
		}
		break;
	case SEQ_MAIN:
		// シーケンス遷移で実行
		temp_subprocess_seq = wk->subprocess_seq;
		ret = (*Functable[wk->subprocess_seq])( wk );
		if(temp_subprocess_seq != wk->subprocess_seq){
			wk->local_seq = 0;
			wk->local_work = 0;
			wk->local_wait = 0;
		}
		if(ret == SUBSEQ_END){
			*seq = SEQ_END;
		}
		break;
	case SEQ_END:
		if(WIPE_SYS_EndCheck() == TRUE){
			return GFL_PROC_RES_FINISH;
		}
		break;
	}
	
	ConnectBGPalAnm_Main(&wk->cbp);

  GFL_TCBL_Main(wk->tcblsys);
	PRINTSYS_QUE_Main(wk->printQue);
  PRINT_UTIL_Trans(&wk->printutil_title, wk->printQue);
  PRINT_UTIL_Trans(&wk->printutil_subwin, wk->printQue);
	
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：終了
 *
 * @param   proc		プロセスデータ
 * @param   seq			シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT GdsConnectProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GDS_CONNECT_SYS * wk  = mywk;

	ConnectBGPalAnm_End(&wk->cbp);

	if(wk->print_stream != NULL){
		PRINTSYS_PrintStreamDelete(wk->print_stream);
	}

	// メッセージマネージャー・ワードセットマネージャー解放
	GFL_MSG_Delete( wk->SystemMsgManager );
	GFL_MSG_Delete( wk->LobbyMsgManager );
	GFL_MSG_Delete( wk->MsgManager );
	WORDSET_Delete( wk->WordSet );

  //フォント破棄
	GFL_FONT_Delete(wk->font_handle);
	//PrintQue破棄
	PRINTSYS_QUE_Delete(wk->printQue);
	//TCBL破棄
	GFL_TCBL_Exit(wk->tcblsys);

	GFL_STR_DeleteBuffer( wk->TitleString ); 
	GFL_STR_DeleteBuffer( wk->ErrorString ); 
	GFL_STR_DeleteBuffer( wk->TalkString ); 
	
	BmpWinDelete( wk );
	
	// BG_SYSTEM解放
	BgExit();

	//sys_HBlankIntrStop();	//HBlank割り込み停止

#if 0 //※check
	MsgPrintSkipFlagSet(MSG_SKIP_OFF);
	MsgPrintAutoFlagSet(MSG_AUTO_OFF);
	MsgPrintTouchPanelFlagSet(MSG_TP_OFF);
#endif

	GFL_PROC_FreeWork( proc );				// PROCワーク開放
	GFL_HEAP_DeleteHeap( HEAPID_GDS_CONNECT );

	return GFL_PROC_RES_FINISH;
}


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   Vramバンク設定を行う
 */
//--------------------------------------------------------------
static void GdsConnectMenu_VramBankSet(void)
{
	GFL_DISP_GX_SetVisibleControlDirect(0);
	GFL_DISP_GXS_SetVisibleControlDirect(0);
	
	//VRAM設定
	{
		GFL_DISP_VRAM vramSetTable = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット

			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

			GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット

			GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

			GX_VRAM_TEX_0_B,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット

    	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
    	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
		};
		GFL_DISP_SetBank( &vramSetTable );

		//VRAMクリア
		GFL_STD_MemClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
		GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
		GFL_STD_MemClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
		GFL_STD_MemClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
	}

	// BG SYSTEM
	{
		GFL_BG_SYS_HEADER BGsys_data = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &BGsys_data );
	}
  
  GFL_BG_Init(HEAPID_GDS_CONNECT);
  GFL_BMPWIN_Init(HEAPID_GDS_CONNECT);
  
	//メイン画面フレーム設定
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME0_M	テキスト面
			{
				0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000, GX_BG_EXTPLTT_01,
				0, 0, 0, FALSE
			},
			///<FRAME1_M	背景
			{
				0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x7000, GX_BG_EXTPLTT_01,
				1, 0, 0, FALSE
			},
		};
		GFL_BG_SetBGControl(GFL_BG_FRAME0_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME0_M );
		GFL_BG_SetScroll(GFL_BG_FRAME0_M, GFL_BG_SCROLL_X_SET, 0);
		GFL_BG_SetScroll(GFL_BG_FRAME0_M, GFL_BG_SCROLL_Y_SET, 0);
		
		GFL_BG_SetBGControl(GFL_BG_FRAME1_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME1_M );
		GFL_BG_SetScroll(GFL_BG_FRAME1_M, GFL_BG_SCROLL_X_SET, 0);
		GFL_BG_SetScroll(GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, 0);
	}
	//サブ画面フレーム設定
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME0_S	テキスト面
			{
				0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, 0x8000, GX_BG_EXTPLTT_01,
				0, 0, 0, FALSE
			},
			///<FRAME1_S	背景
			{
				0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x08000, 0x5800, GX_BG_EXTPLTT_01,
				2, 0, 0, FALSE
			},
		};
		GFL_BG_SetBGControl(GFL_BG_FRAME0_S, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME0_S );
		GFL_BG_SetScroll(GFL_BG_FRAME0_S, GFL_BG_SCROLL_X_SET, 0);
		GFL_BG_SetScroll(GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, 0);
		
		GFL_BG_SetBGControl(GFL_BG_FRAME1_S, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME1_S );
		GFL_BG_SetScroll(GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, 0);
		GFL_BG_SetScroll(GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, 0);
	}

	GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 32, 0, HEAPID_GDS_CONNECT );
	GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 32, 0, HEAPID_GDS_CONNECT );
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
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
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
static void BgGraphicSet( GDS_CONNECT_SYS * wk )
{
	ARCHANDLE* p_handle;

	p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFIP2PMATCH, HEAPID_GDS_CONNECT );

	// 上下画面ＢＧパレット転送
	GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_wifip2pmatch_conect_NCLR, PALTYPE_MAIN_BG, 0, 0,  HEAPID_GDS_CONNECT);
	GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_wifip2pmatch_conect_NCLR, PALTYPE_SUB_BG,  0, 0,  HEAPID_GDS_CONNECT);
	
	// 会話フォントパレット転送
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
		GDSCONNECT_TALKFONT_PAL*0x20, 0x20, HEAPID_GDS_CONNECT);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 
		GDSCONNECT_TALKFONT_PAL*0x20, 0x20, HEAPID_GDS_CONNECT);

	// 会話ウインドウグラフィック転送
	TalkWinFrame_GraphicSet(	GFL_BG_FRAME0_M, GDSCONNECT_MESFRAME_CHR, 
				GDSCONNECT_MESFRAME_PAL,  
				CONFIG_GetWindowType(SaveData_GetConfig(wk->oya_proc_work->proc_param->savedata)), 
				HEAPID_GDS_CONNECT );

	BmpWinFrame_GraphicSet(	GFL_BG_FRAME0_M, GDSCONNECT_MENUFRAME_CHR,
						GDSCONNECT_MENUFRAME_PAL, 0, HEAPID_GDS_CONNECT );




	// メイン画面BG1キャラ転送
	GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifip2pmatch_conect_NCGR, GFL_BG_FRAME1_M, 0, 0, 0, HEAPID_GDS_CONNECT);

	// メイン画面BG1スクリーン転送
	GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_wifip2pmatch_conect_01_NSCR, GFL_BG_FRAME1_M, 0, 32*24*2, 0, HEAPID_GDS_CONNECT);



	// サブ画面BG1キャラ転送
	GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifip2pmatch_conect_sub_NCGR, GFL_BG_FRAME1_S, 0, 0, 0, HEAPID_GDS_CONNECT);

	// サブ画面BG1スクリーン転送
	GFL_ARCHDL_UTIL_TransVramScreen(   p_handle, NARC_wifip2pmatch_conect_sub_NSCR, GFL_BG_FRAME1_S, 0, 32*24*2, 0, HEAPID_GDS_CONNECT);

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0 );
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0 );

	
	//Wifi接続BGパレットアニメシステム初期化
	ConnectBGPalAnm_Init(&wk->cbp, p_handle, NARC_wifip2pmatch_conect_anm_NCLR, HEAPID_GDS_CONNECT);
	
	GFL_ARC_CloseDataHandle( p_handle );
}

#define SUB_TEXT_X		(  4 )
#define SUB_TEXT_Y		(  4 )
#define SUB_TEXT_SX		( 23 )
#define SUB_TEXT_SY		( 16 )

#define CONNECT_TEXT_X	(  4 )
#define CONNECT_TEXT_Y	(  1 )
#define CONNECT_TEXT_SX	( 24 )
#define CONNECT_TEXT_SY	(  2 )

// 会話ウインドウ表示位置定義
#define TALK_WIN_X		(  2 )
#define TALK_WIN_Y		( 19 )
#define	TALK_WIN_SX		( 27 )
#define	TALK_WIN_SY		(  4 )

#define TALK_MESSAGE_OFFSET	 ( GDSCONNECT_MENUFRAME_CHR + MENU_WIN_CGX_SIZ )
#define ERROR_MESSAGE_OFFSET ( TALK_MESSAGE_OFFSET   + TALK_WIN_SX*TALK_WIN_SY )
#define TITLE_MESSAGE_OFFSET ( ERROR_MESSAGE_OFFSET  + SUB_TEXT_SX*SUB_TEXT_SY )
#define YESNO_OFFSET 		 ( TITLE_MESSAGE_OFFSET  + CONNECT_TEXT_SX*CONNECT_TEXT_SY )
#define MENULIST_MESSAGE_OFFSET	(ERROR_MESSAGE_OFFSET)	//エラーメッセージと一緒には出ないので ※check YESNO_OFFSETの値を調べて、充分な空きがあるなら、その後ろにする

//------------------------------------------------------------------
/**
 * BMPWIN処理（文字パネルにフォント描画）
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void BmpWinInit( GDS_CONNECT_SYS *wk )
{
	// ---------- メイン画面 ------------------

	// BG0面BMPWIN(エラー説明)ウインドウ確保・描画
	wk->SubWin = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, SUB_TEXT_X, SUB_TEXT_Y, 
	  SUB_TEXT_SX, SUB_TEXT_SY, GDSCONNECT_TALKFONT_PAL, GFL_BMP_CHRAREA_GET_B );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->SubWin), 0x0000 );
	PRINT_UTIL_Setup( &wk->printutil_subwin, wk->SubWin );

	// BG0面BMPWIN(タイトル)ウインドウ確保・描画
	wk->TitleWin = GFL_BMPWIN_Create(GFL_BG_FRAME0_M, CONNECT_TEXT_X, CONNECT_TEXT_Y, 
	  CONNECT_TEXT_SX, CONNECT_TEXT_SY, GDSCONNECT_TALKFONT_PAL, GFL_BMP_CHRAREA_GET_B );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->TitleWin), 0x0000 );
	PRINT_UTIL_Setup( &wk->printutil_title, wk->TitleWin );
	GdsConnect_TalkPrint( &wk->printutil_title, wk->TitleString, 0, 1, 1, PRINTSYS_LSB_Make(15,14,0), wk->font_handle, wk->printQue );

	// BG0面BMP（会話ウインドウ）確保
	wk->MsgWin = GFL_BMPWIN_Create(GFL_BG_FRAME0_M,
		TALK_WIN_X, 
		TALK_WIN_Y, 
		TALK_WIN_SX, 
		TALK_WIN_SY, GDSCONNECT_TALKFONT_PAL, GFL_BMP_CHRAREA_GET_B );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin), 0x0000 );
}

//------------------------------------------------------------------
/**
 * $brief   確保したBMPWINを解放
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void BmpWinDelete( GDS_CONNECT_SYS *wk )
{
	GFL_BMPWIN_Delete( wk->MsgWin );
	GFL_BMPWIN_Delete( wk->TitleWin );
	GFL_BMPWIN_Delete( wk->SubWin );
}

//------------------------------------------------------------------
/**
 * @brief   
 *
 * @param   win		
 * @param   strbuf		
 * @param   flag		1だとセンタリング、２だと右よせ
 * @param   color		
 * @param   font_handle		フォント指定（FONT_TALKかFONT_SYSTEM
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int printCommonFunc( GFL_BMPWIN *win, STRBUF *strbuf, int x, int flag, PRINTSYS_LSB color, GFL_FONT *font_handle )
{
	int length=0,ground;
	switch(flag){
	// センタリング
	case 1:
	  length = PRINTSYS_GetStrWidth( strbuf, font_handle, 0 );
		x          = ((GFL_BMPWIN_GetSizeX(win)*8)-length)/2;
		break;

	// 右寄せ
	case 2:
	  length = PRINTSYS_GetStrWidth( strbuf, font_handle, 0 );
		x          = (GFL_BMPWIN_GetSizeX(win)*8)-length;
		break;
	}
	return x;
}


//------------------------------------------------------------------
/**
 * $brief   BMPWIN内の表示位置を指定してFONT_TALKでプリント(描画のみ）
 *
 * @param   win		GFL_BMPWIN *
 * @param   strbuf	
 * @param   x		X座標ずらす値
 * @param   y		Y座標ずらす値
 * @param   flag	0だと左寄せ、1だとセンタリング、2だと右寄せ
 * @param   color	文字色指定（背景色でBMPを塗りつぶします）
 *
 * @retval  none
 */
//------------------------------------------------------------------
void GdsConnect_TalkPrint( PRINT_UTIL *print_util, STRBUF *strbuf, int x, int y, int flag, PRINTSYS_LSB color, GFL_FONT *font_handle, PRINT_QUE *printQue )
{
	x = printCommonFunc( print_util->win, strbuf, x, flag, color, font_handle );

  PRINT_UTIL_PrintColor( print_util, printQue, x, y, strbuf, font_handle, color );
}

//------------------------------------------------------------------
/**
 * @brief   ｗｉｆｉエラー表示
 *
 * @param   wk		
 * @param   msgno		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void _systemMessagePrint( GDS_CONNECT_SYS *wk, int msgno )
{
  STRBUF *tmpString = GFL_STR_CreateBuffer( DWC_ERROR_BUF_NUM, HEAPID_GDS_CONNECT );
  GFL_MSG_GetString(  wk->SystemMsgManager, msgno, tmpString );
  WORDSET_ExpandStr( wk->WordSet, wk->ErrorString, tmpString );

  // 会話ウインドウ枠描画
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->SubWin), 15 );
	GFL_BMPWIN_MakeFrameScreen(wk->SubWin, GDSCONNECT_MENUFRAME_CHR, GDSCONNECT_MENUFRAME_PAL);
	BmpWinFrame_Write(wk->SubWin, 
	  WINDOW_TRANS_OFF, GDSCONNECT_MENUFRAME_CHR, GDSCONNECT_MENUFRAME_PAL);

  // 文字列描画開始
  PRINT_UTIL_Print( &wk->printutil_subwin, wk->printQue, 0, 0, wk->ErrorString, wk->font_handle );

  GFL_STR_DeleteBuffer(tmpString);
}

//------------------------------------------------------------------
/**
 * @brief   Wifiコネクションエラーの表示
 *
 * @param   wk		
 * @param   type	
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void errorDisp(GDS_CONNECT_SYS* wk, int type, int code)
{
    int msgno;

    if(type != -1){
        msgno = dwc_error_0001 + type;
    }
    else{
        msgno = dwc_error_0012;
    }
//    EndMessageWindowOff(wk);
    WORDSET_RegisterNumber(wk->WordSet, 0, code,
                           5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);

    BmpWinFrame_Clear( wk->MsgWin, WINDOW_TRANS_OFF );
    _systemMessagePrint(wk, msgno);

#if 0
    switch(type){
      case 1:
      case 4:
      case 5:
        wk->seq = WIFIP2PMATCH_RETRY_INIT;  // 再接続かフィールドか
        break;
      case 6:
      case 7:
      case 8:
      case 9:
        wk->seq = WIFIP2PMATCH_RETRY_INIT;//WIFIP2PMATCH_POWEROFF_INIT;  // 電源を切るかフィールド
        break;
      case 10:
        wk->seq = WIFIP2PMATCH_RETRY_INIT;  // メニュー一覧へ
        break;
      case 0:
      case 2:
      case 3:
      case 11:
      default:
        wk->seq = WIFIP2PMATCH_MODE_CHECK_AND_END;  // フィールド
        break;
    }
#endif
}

//--------------------------------------------------------------
/**
 * @brief   メッセージの終了処理チェック
 *
 * @param   msg_index		メッセージIndex
 *
 * @retval  TRUE:処理中。　　FALSE:終了している
 */
//--------------------------------------------------------------
static int Enter_MessagePrintEndCheck(GDS_CONNECT_SYS *wk)
{
  BOOL stream_end = FALSE;
  BOOL que_end = FALSE;
  
  if(wk->print_stream != NULL){
    if(PRINTSYS_PrintStreamGetState(wk->print_stream) == PRINTSTREAM_STATE_DONE){
      PRINTSYS_PrintStreamDelete(wk->print_stream);
      wk->print_stream = NULL;
      stream_end = TRUE;
    }
  }
  else{
    stream_end = TRUE;
  }
  
  if(PRINTSYS_QUE_IsFinished( wk->printQue ) == TRUE){
    que_end = TRUE;
  }
  
  if(stream_end == TRUE && que_end == TRUE){
    return FALSE;
  }
  return TRUE;
}

//------------------------------------------------------------------
/**
 * @brief   時間アイコン追加
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void GdsConnect_TimeIconAdd( GDS_CONNECT_SYS *wk )
{
#if 0 //※check
	if(wk->timeWaitWork == NULL){
		wk->timeWaitWork = TimeWaitIconAdd( wk->MsgWin, GDSCONNECT_MESFRAME_CHR );
	}
#endif
}

//------------------------------------------------------------------
/**
 * @brief   時間アイコン消去（NULLチェックする）
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void GdsConnect_TimeIconDel( GDS_CONNECT_SYS *wk )
{
#if 0 //※check
	if(wk->timeWaitWork!=NULL){
		TimeWaitIconDel(wk->timeWaitWork);
		wk->timeWaitWork = NULL;
	}
#endif
}

//==============================================================================
/**
 * $brief   はい・いいえウインドウ登録
 *
 * @param   menuframe		
 * @param   y		
 * @param   yesno_bmp_cgx		
 *
 * @retval  BMPMENU_WORK *		
 */
//==============================================================================
static BMPMENU_WORK *GdsConnect_BmpWinYesNoMake( int y, int yesno_bmp_cgx )
{
	BMPWIN_YESNO_DAT yesnowin;

	yesnowin        = YesNoBmpWin;
	yesnowin.pos_y  = y;
	yesnowin.chrnum = yesno_bmp_cgx;

	return BmpMenu_YesNoSelectInit(	&yesnowin, GDSCONNECT_MENUFRAME_CHR, GDSCONNECT_MENUFRAME_PAL, 0, HEAPID_GDS_CONNECT );
}

//------------------------------------------------------------------
/**
 * $brief   会話ウインドウ表示
 *
 * @param   wk		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void Enter_MessagePrint( GDS_CONNECT_SYS *wk, GFL_MSGDATA *msgman, int msgno, int wait, u16 dat )
{
	// 文字列取得
	STRBUF *tempbuf;

	// 文字列取得
	tempbuf = GFL_MSG_CreateString(  msgman, msgno );

	// WORDSET展開
	WORDSET_ExpandStr( wk->WordSet, wk->TalkString, tempbuf );

	GFL_STR_DeleteBuffer(tempbuf);

	// 会話ウインドウ枠描画
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MsgWin),  0x0f0f );
  GFL_BMPWIN_MakeScreen(wk->MsgWin);
  TalkWinFrame_Write(wk->MsgWin, WINDOW_TRANS_ON, 
    GDSCONNECT_MESFRAME_CHR, GDSCONNECT_MESFRAME_PAL);

	// 文字列描画開始
	GF_ASSERT(wk->print_stream == NULL);
  if(wk->print_stream != NULL){
    PRINTSYS_PrintStreamDelete(wk->print_stream);
  }
  wk->print_stream = PRINTSYS_PrintStream(wk->MsgWin, 0, 0, wk->TalkString,
    wk->font_handle, wait, wk->tcblsys, 5, HEAPID_GDS_CONNECT, 0xf);

	wk->wait = 0;
}

//------------------------------------------------------------------
/**
 * $brief   
 *
 * @param   wk		
 * @param   to_seq		
 * @param   next_seq		
 *
 * @retval  none		
 */
//------------------------------------------------------------------
static void GdsConnect_SetNextSeq( GDS_CONNECT_SYS *wk, int to_seq, int next_seq )
{
	wk->subprocess_seq      = to_seq;
	wk->subprocess_nextseq  = next_seq;
}


//----------------------------------------------------------------------------
/**
 *	@brief	再接続OKかどうかチェック
 *	
 *	@param	errno		エラーコード
 *	@param	errtype		エラータイプ
 *
 *	@retval	TRUE	再接続OK
 *	@retval	FALSE	再接続NG
 */
//-----------------------------------------------------------------------------
static BOOL Enter_GetErrTypeRetry( int errno, int errtype )
{
	int err_type;
	BOOL ret;

	err_type = GFL_NET_DWC_ErrorType( errno, errtype );
	
    switch(err_type){
      case 1:
      case 4:
      case 5:
      case 11:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
		ret = TRUE;
		break; 

      case 0:
      case 2:
      case 3:
      default:
		ret = FALSE;
		break; 
    }

	return ret;
}

//==============================================================================
//
//	
//
//==============================================================================
//------------------------------------------------------------------
/**
 * $brief   サブプロセスシーケンススタート処理
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int Enter_Start( GDS_CONNECT_SYS *wk)
{
	if( Enter_MessagePrintEndCheck( wk )==TRUE){
		return SUBSEQ_CONTINUE;
	}

	switch(wk->local_seq){
	case 0:
		OS_TPrintf("Enter 開始\n");

		wk->local_seq++;
		break;
	case 1:
		//Wifiコネクション　に　せつぞく　しますか？
		Enter_MessagePrint( wk, wk->SystemMsgManager, dwc_message_0002, 1, 0x0f0f );
		wk->local_seq++;
		break;

	case 2:	//はい・いいえウィンドウ表示
		wk->YesNoMenuWork = GdsConnect_BmpWinYesNoMake(GDSCONNECT_YESNO_PY2, YESNO_OFFSET );
		wk->local_seq++;
		break;
	case 3:
		{
      u32 ret = BmpMenu_YesNoSelectMain( wk->YesNoMenuWork );

			if(ret!=BMPMENU_NULL){
				if(ret==BMPMENU_CANCEL){
					wk->subprocess_seq = ENTER_END;
				}
				else{
					wk->local_seq++;
				}
			}
		}
		break;
	
	case 4:	//WiFi接続開始
		wk->subprocess_seq = ENTER_INTERNET_CONNECT;
		break;
	}
	
	return SUBSEQ_CONTINUE;
}

//------------------------------------------------------------------
/**
 * $brief   インターネット接続開始
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int Enter_InternetConnect( GDS_CONNECT_SYS *wk )
{
	switch(wk->local_seq){
	case 0:
		// 通信エラー管理のために通信ルーチンをON
		//CommStateWifiDPWStart( wk->oya_proc_work->proc_param->savedata );

		// WIFIせつぞくを開始
		Enter_MessagePrint( wk, wk->LobbyMsgManager, msg_wifilobby_002, 1, 0x0f0f );
		GdsConnect_TimeIconAdd(wk);
		wk->local_seq++;
		break;
	case 1:
		if( Enter_MessagePrintEndCheck( wk )==FALSE){
			wk->local_seq++;
		}
		break;
	case 2:
		DWC_InitInetEx(&wk->oya_proc_work->stConnCtrl, 
		  GFL_DMA_NET_NO, _NETWORK_POWERMODE, _NETWORK_SSL_PRIORITY);
    //DWC_SetAuthServer(GF_DWC_CONNECTINET_AUTH_TYPE);
		DWC_ConnectInetAsync();
		
		wk->subprocess_seq = ENTER_INTERNET_CONNECT_WAIT;
		OS_TPrintf("InternetConnet Start\n");
		break;
	}
	return SUBSEQ_CONTINUE;
}

//------------------------------------------------------------------
/**
 * $brief   ネット接続待ち
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int Enter_InternetConnectWait( GDS_CONNECT_SYS *wk )
{
	DWC_ProcessInet();

	if (DWC_CheckInet())
	{
		switch (DWC_GetInetStatus())
		{
		case DWC_CONNECTINET_STATE_ERROR:
			{
				// エラー表示
				DWCError err;
				int errcode;
                DWCErrorType errtype;
				err = DWC_GetLastErrorEx(&errcode, &errtype);
				wk->ErrorRet  = err;
				wk->ErrorCode = errcode;
				wk->ErrorType = errtype;

				OS_TPrintf("   Error occurred %d %d.\n", err, errcode);
			}
			DWC_ClearError();
			DWC_CleanupInet();

		    //画面を抜けずに2度連続で「Eメール設定」をしてWifiに繋げようとすると
		    //"dpw_tr.c:150 Panic:dpw tr is already initialized."
		    //のエラーが出るのできちんとこの終了関数を呼ぶようにする 2007.10.26(金) matsuda
	//	    Dpw_Tr_End();

			GdsConnect_TimeIconDel( wk );
			wk->subprocess_seq = ENTER_DWC_ERROR_PRINT;

			break;
		case DWC_CONNECTINET_STATE_NOT_INITIALIZED:
		case DWC_CONNECTINET_STATE_IDLE:
		case DWC_CONNECTINET_STATE_OPERATING:
		case DWC_CONNECTINET_STATE_OPERATED:
		case DWC_CONNECTINET_STATE_DISCONNECTING:
		case DWC_CONNECTINET_STATE_DISCONNECTED:
		default:
			OS_TPrintf("DWC_CONNECTINET_STATE_DISCONNECTED!\n");
			//break;
		case DWC_CONNECTINET_STATE_FATAL_ERROR:
			{
				// エラー表示
				DWCError err;
				int errcode;
				err = DWC_GetLastError(&errcode);

				OS_TPrintf("   Error occurred %d %d.\n", err, errcode);
				GdsConnect_TimeIconDel(wk);
				wk->subprocess_seq = ENTER_SERVER_SERVICE_ERROR;
				wk->ConnectErrorNo = DPW_TR_ERROR_SERVER_TIMEOUT;
			}
			break;

		case DWC_CONNECTINET_STATE_CONNECTED:
	        {	// 接続先を表示する。店舗の場合は店舗情報も表示する。
				DWCApInfo apinfo;
	
				DWC_GetApInfo(&apinfo);
	
	            OS_TPrintf("   Connected to AP type %d.\n", apinfo.aptype);

	            if (apinfo.aptype == DWC_APINFO_TYPE_SHOP)
	            {
					OS_TPrintf("<Shop AP Data>\n");
	                OS_TPrintf("area code: %d.\n", apinfo.area);
	                OS_TPrintf("spotinfo : %s.\n", apinfo.spotinfo);
	            }
	        }
	        // コネクト成功？
			wk->subprocess_seq = ENTER_WIFI_CONNECTION_LOGIN;
			break;
		}

		// 時間アイコン消去

	}
	
	return SUBSEQ_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief   GameSpyサーバーログイン開始
 *
 * @param   wk		
 *
 * @retval  int
 */
//------------------------------------------------------------------
static int Enter_WifiConnectionLogin( GDS_CONNECT_SYS *wk )
{
	DWC_NASLoginAsync();
	wk->subprocess_seq = ENTER_WIFI_CONNECTION_LOGIN_WAIT;
	OS_Printf("GameSpyサーバーログイン開始\n");

	return SUBSEQ_CONTINUE;
}
//------------------------------------------------------------------
/**
 * @brief   GameSpyサーバーログイン処理待ち
 *
 * @param   wk
 *
 * @retval  int
 */
//------------------------------------------------------------------
static int Enter_WifiConnectionLoginWait( GDS_CONNECT_SYS *wk )
{
	switch(DWC_NASLoginProcess()){
	case DWC_NASLOGIN_STATE_SUCCESS:
		OS_Printf("GameSpyサーバーログイン成功\n");
		wk->subprocess_seq = ENTER_DPWTR_INIT;
		break;
	case DWC_NASLOGIN_STATE_ERROR:
	case DWC_NASLOGIN_STATE_CANCELED:
	case DWC_NASLOGIN_STATE_DIRTY:
		GdsConnect_TimeIconDel(wk);
		OS_Printf("GameSpyサーバーログイン失敗\n");
		{
			int errCode;
			DWCErrorType errType;
			DWCError dwcError;
			dwcError = DWC_GetLastErrorEx( &errCode, &errType );
			wk->ErrorRet  = dwcError;
			wk->ErrorCode = errCode;
            wk->ErrorType = errType;

			DWC_ClearError();
			DWC_CleanupInet();

		    //画面を抜けずに2度連続で「Eメール設定」をしてWifiに繋げようとすると
		    //"dpw_tr.c:150 Panic:dpw tr is already initialized."
		    //のエラーが出るのできちんとこの終了関数を呼ぶようにする 2007.10.26(金) matsuda
		//	Dpw_Tr_End();

			//ありえないはずだが、どのエラーにも引っかからない可能性を考慮し、初期値として次のシーケンスを先に設定しておく
			wk->subprocess_seq = ENTER_DWC_ERROR_PRINT;

			switch(errType){
			case DWC_ETYPE_LIGHT:
			case DWC_ETYPE_SHOW_ERROR:
				wk->subprocess_seq = ENTER_DWC_ERROR_PRINT;
				break;
			case DWC_ETYPE_SHUTDOWN_GHTTP:
				DWC_ShutdownGHTTP();
				wk->subprocess_seq = ENTER_DWC_ERROR_PRINT;
				break;
			case DWC_ETYPE_DISCONNECT:
				wk->subprocess_seq = ENTER_DWC_ERROR_PRINT;
				break;
			case DWC_ETYPE_SHUTDOWN_FM:
				OS_TPrintf("DWC_ETYPE_SHUTDOWN_FM!\n");
				DWC_ShutdownFriendsMatch();
				wk->subprocess_seq = ENTER_DWC_ERROR_PRINT;
				break;
			case DWC_ETYPE_SHUTDOWN_ND:	//このシーケンスではありえないので一応強制ふっとばしにする
				OS_TPrintf("DWC_ETYPE_SHUTDOWN_ND!\n");
				//break;
			case DWC_ETYPE_FATAL:
				// 強制ふっとばし
			#if 0 //※check
				CommFatalErrorFunc_NoNumber();
			#else
			  NetErr_ErrorSet();
			#endif
				break;
			}

			// 20000番台をキャッチしたらerrTypeが何であろうとリセットエラーへ
			if(errCode<-20000 && errCode >=-29999){
//				CommSetErrorReset(COMM_ERROR_RESET_TITLE);
				OS_Printf("dwcError = %d  errCode = %d, errType = %d\n", dwcError, errCode, errType);
				wk->subprocess_seq = ENTER_DWC_ERROR_PRINT;
			}
		}
		break;
	}
	
	
	return SUBSEQ_CONTINUE;
}

//------------------------------------------------------------------
/**
 * $brief   ネット接続後の後処理
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int Enter_DpwTrInit( GDS_CONNECT_SYS *wk )
{
	// 世界交換接続初期化
	DWCUserData		*MyUserData;		// 認証済みのDWCUSERデータしかこないはず
	s32 profileId;
	SYSTEMDATA *systemdata;
	WIFI_LIST *wifilist;
	
	wifilist = GAMEDATA_GetWiFiList(wk->oya_proc_work->proc_param->gamedata);
	systemdata = SaveData_GetSystemData(wk->oya_proc_work->proc_param->savedata);

	// DWCUser構造体取得
	MyUserData = WifiList_GetMyUserInfo(wifilist);

	// このFriendKeyはプレイヤーが始めて取得したものか？
	//↓森さんと相談した結果、使用意図がいまいちハッキリしないので世界交換などと同じように
	//	とりあえずセットしておくことになった	2008.01.17(木)
	profileId = SYSTEMDATA_GetDpwInfo( systemdata );
	if( profileId==0 ){
		OS_TPrintf("初回取得profileIdなのでDpwInfoとして登録した %08x \n", WifiList_GetMyGSID(GAMEDATA_GetWiFiList(wk->oya_proc_work->proc_param->gamedata)));

		// 初回取得FriendKeyなので、DpwIdとして保存する
		SYSTEMDATA_SetDpwInfo( systemdata, WifiList_GetMyGSID(wifilist) );
	}

	
	// 正式なデータを取得
	profileId = SYSTEMDATA_GetDpwInfo( systemdata );
	OS_Printf("Dpwサーバーログイン情報 profileId=%08x\n", profileId);

	// DPW_TR初期化
//	Dpw_Tr_Init( profileId, DWC_CreateFriendKey( MyUserData ) );

	OS_TPrintf("Dpw Trade 初期化\n");

	wk->subprocess_seq = ENTER_CONNECT_END;	//通信したまま終了
	wk->oya_proc_work->ret_connect = TRUE;
	
	return SUBSEQ_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief   Wifi接続エラーを表示
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int Enter_DwcErrorPrint( GDS_CONNECT_SYS *wk )
{
	int type;
	
    type =  GFL_NET_DWC_ErrorType(-wk->ErrorCode, wk->ErrorType);


	OS_Printf("error code = %d, type = %d\n", wk->ErrorCode, type);

    errorDisp(wk, type, -wk->ErrorCode);


	wk->subprocess_seq = ENTER_ERROR_PAD_WAIT;
	
	return SUBSEQ_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief   プリント後キー待ち
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int Enter_ErrorPadWait( GDS_CONNECT_SYS *wk )
{
	if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE || GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
  	BmpWinFrame_Clear( wk->SubWin, WINDOW_TRANS_ON );
		wk->local_seq = 0;
		if( Enter_GetErrTypeRetry( -wk->ErrorCode, wk->ErrorType ) == TRUE ){
			wk->subprocess_seq = ENTER_START;
		}else{
			wk->subprocess_seq = ENTER_END;
		}
	}
	return SUBSEQ_CONTINUE;
}


//------------------------------------------------------------------
/**
 * $brief   サブプロセスシーケンス終了処理
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int Enter_End( GDS_CONNECT_SYS *wk)
{
	if(wk->subprocess_seq != ENTER_CONNECT_END){
		//CommStateWifiDPWEnd();
	}

	// 時間アイコン消去２重解放にならないようにNULLチェックしつつ
	GdsConnect_TimeIconDel( wk );

	
	WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK, 
		WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_GDS_CONNECT );
	
	wk->subprocess_seq = 0;
//	wk->sub_out_flg = 1;
	
	return SUBSEQ_END;
}

//==============================================================================
/**
 * $brief   ネットには繋がったけどサーバーエラーだった表示
 *
 * @param   wk		
 *
 * @retval  none		
 */
//==============================================================================
static int Enter_ServerServiceError( GDS_CONNECT_SYS *wk )
{
	int msgno =0;

	switch(wk->ConnectErrorNo){
	case DPW_TR_STATUS_SERVER_STOP_SERVICE:
		msgno = msg_gtc_error_001;
		break;
	case DPW_TR_STATUS_SERVER_FULL:
	case DPW_TR_ERROR_SERVER_FULL:
		msgno = msg_gtc_error_002;
		break;
	case DPW_TR_ERROR_SERVER_TIMEOUT:
	case DPW_TR_ERROR_DISCONNECTED:
		// ＧＴＳとのせつぞくがきれました。うけつけにもどります
		msgno = msg_gtc_error_006;
		break;
	case DPW_TR_ERROR_CANCEL  :
	case DPW_TR_ERROR_FAILURE :
	case DPW_TR_ERROR_NO_DATA:
	case DPW_TR_ERROR_ILLIGAL_REQUEST :
		//　つうしんエラーが発生しました。
		msgno = msg_gtc_error_005;
		break;
	}
	// エラー表示
	Enter_MessagePrint( wk, wk->MsgManager, msgno, 1, 0x0f0f );
	GdsConnect_SetNextSeq( wk, ENTER_MES_WAIT, ENTER_SERVER_SERVICE_END );

	OS_TPrintf("Error発生\n");

	return SUBSEQ_CONTINUE;
}

//==============================================================================
/**
 * $brief   サーバーサービスの問題で終了
 *
 * @param   wk		
 *
 * @retval  none		
 */
//==============================================================================
static int Enter_ServerServiceEnd( GDS_CONNECT_SYS *wk )
{
	switch(wk->local_seq){
	case 0:
		Enter_MessagePrint( wk, wk->MsgManager, msg_gtc_cleanup_000, 1, 0x0f0f );
		wk->local_seq++;
		break;
	case 1:
		if(Enter_MessagePrintEndCheck(wk) == FALSE){
		    // 通信エラー管理のために通信ルーチンをOFF
		    //CommStateWifiDPWEnd();
		    DWC_CleanupInet();

		    //画面を抜けずに2度連続で「Eメール設定」をしてWifiに繋げようとすると
		    //"dpw_tr.c:150 Panic:dpw tr is already initialized."
		    //のエラーが出るのできちんとこの終了関数を呼ぶようにする 2007.10.26(金) matsuda
		//    Dpw_Tr_End();

			wk->local_seq++;
		}
		break;
	case 2:
		Enter_MessagePrint( wk, wk->MsgManager, msg_gtc_cleanup_001, 1, 0x0f0f );
		wk->local_seq++;
		break;
	case 3:
		if(Enter_MessagePrintEndCheck( wk )==FALSE){
			wk->local_seq++;
		}
		break;
	default:
		wk->local_wait++;
		if(wk->local_wait > WAIT_ONE_SECONDE_NUM){
			wk->subprocess_seq  = ENTER_END;
		}
		break;
	}
	
	return SUBSEQ_CONTINUE;
}

//------------------------------------------------------------------
/**
 * $brief   会話終了を待って次のシーケンスへ
 *
 * @param   wk		
 *
 * @retval  int		
 */
//------------------------------------------------------------------
static int Enter_MessageWait( GDS_CONNECT_SYS *wk )
{
	if( Enter_MessagePrintEndCheck( wk )==FALSE){
		wk->subprocess_seq = wk->subprocess_nextseq;
	}
	return SUBSEQ_CONTINUE;

}

//--------------------------------------------------------------
/**
 * @brief   Wi-Fiコネクションから切断
 *
 * @param   wk		
 *
 * @retval  
 *
 * ・切断メッセージを表示します。
 * ・切断後の飛び先はEmail_SetNextSeqで設定しておいてください
 */
//--------------------------------------------------------------
static int Enter_CleanupInet( GDS_CONNECT_SYS *wk )
{
	if( Enter_MessagePrintEndCheck( wk )==TRUE){
		return SUBSEQ_CONTINUE;
	}

	switch(wk->local_seq){
	case 0:
		Enter_MessagePrint( wk, wk->SystemMsgManager, dwc_message_0011, 1, 0x0f0f );
		wk->local_seq++;
		break;
	case 1:
		// WIFIせつぞくを終了
//		if(DWC_CheckInet()){
		    DWC_CleanupInet();
//		}
		// 通信エラー管理のために通信ルーチンをOFF
		//CommStateWifiDPWEnd();
		
		//画面を抜けずに2度連続で「Eメール設定」をしてWifiに繋げようとすると
		//"dpw_tr.c:150 Panic:dpw tr is already initialized."
		//のエラーが出るのできちんとこの終了関数を呼ぶようにする 2007.10.26(金) matsuda
	//	Dpw_Tr_End();
		
		wk->local_seq++;
		break;
	case 2:
		Enter_MessagePrint( wk, wk->SystemMsgManager, dwc_message_0012, 1, 0x0f0f );
		wk->local_seq++;
		break;
	case 3:
		wk->local_wait++;
		if(wk->local_wait > 30){	//1秒待つ
			wk->subprocess_seq	= wk->subprocess_nextseq;
		}
		break;
	}
	return SUBSEQ_CONTINUE;
}
