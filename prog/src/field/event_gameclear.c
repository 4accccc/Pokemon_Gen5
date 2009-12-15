//============================================================================================
/**
 * @file	event_gameclear.c
 * @brief	イベント：全滅処理
 * @date  2009.12.14
 * @author	tamada GAME FREAK inc.
 *
 * 2009.12.14 event_gameover.cからコピペで作成
 *
 * @todo
 * ゲームオーバーメッセージをPROCにしたのでメモリが足りないときには
 * 別オーバーレイ領域に引越しする
 */
//============================================================================================
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"

#include "event_gameclear.h"  //GAMECLEAR_MODE

#include "system/main.h"  //HEAPID_～

#include "system/brightness.h"

#include "system/wipe.h"

//#include "warpdata.h"			//WARPDATA_～
#include "script.h"       //SCRIPT_CallScript
//#include "event_fieldmap_control.h" //
#include "event_mapchange.h"

#include "demo/demo3d.h"  //Demo3DProcData etc.
#include "app/local_tvt_sys.h"  //LocalTvt_ProcData etc.

#include "field/field_msgbg.h"
#include "system/bmp_winframe.h"

#include "sound/pm_sndsys.h"  //PMSND_

#include "move_pokemon.h"

//==============================================================================================
//
//	メッセージ表示関連
//
//==============================================================================================
#include "print/wordset.h"


#include "font/font.naix" //NARC_font_large_gftr
//#include "message.naix"
//#include "msg/msg_gameover.h"
#include "arc/script_message.naix"
#include "msg/script/msg_common_scr.h"
//#include "arc/fieldmap/zone_id.h"


//==============================================================================================
//==============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  GAMECLEAR_MODE clear_mode;
  BOOL saveSuccessFlag;
  u16 arc_id;
  u16 msg_id;
  const MYSTATUS * mystatus;
  void * pWork;
}GAMECLEAR_WORK;


//==============================================================================================
//==============================================================================================
//----------------------------------------------------------------------------------------------
//	構造体宣言
//----------------------------------------------------------------------------------------------
typedef struct{
  HEAPID heapID;
  u16 arc_id;
  u16 msg_id;
  const MYSTATUS * mystatus;

	GFL_MSGDATA* msgman;						//メッセージマネージャー
	WORDSET* wordset;								//単語セット
	GFL_BMPWIN *bmpwin;							//BMPウィンドウデータ

  PRINT_QUE *printQue;
  GFL_FONT * fontHandle;
}GAMECLEAR_MSG_WORK;


#define GAME_OVER_MSG_BUF_SIZE		(1024)			//メッセージバッファサイズ
#define GAME_OVER_FADE_SYNC			(8)				//フェードsync数

enum { //コンパイルを通すためにとりあえず

  FLD_SYSFONT_PAL = 13,

  FBMP_COL_NULL = 0,
  FBMP_COL_BLACK = 0xf,
  FBMP_COL_BLK_SDW = 2,
};
//----------------------------------------------------------------------------------------------
//	BMPウィンドウ
//----------------------------------------------------------------------------------------------
enum{
	GAME_OVER_BMPWIN_FRAME	= GFL_BG_FRAME0_M,
	//GAME_OVER_BMPWIN_PX1	= 1,//2,
	//GAME_OVER_BMPWIN_PY1	= 1,//2,
	//GAME_OVER_BMPWIN_SX		= 29,//25,
	//GAME_OVER_BMPWIN_SY		= 19,//18,
	GAME_OVER_BMPWIN_PX1	= 4,
	GAME_OVER_BMPWIN_PY1	= 5,
	GAME_OVER_BMPWIN_SX		= 25,
	GAME_OVER_BMPWIN_SY		= 15,
	GAME_OVER_BMPWIN_PL		= FLD_SYSFONT_PAL,
	GAME_OVER_BMPWIN_CH		= 1,
};

//==============================================================================================
//==============================================================================================
//----------------------------------------------------------------------------------------------
//	プロトタイプ宣言
//----------------------------------------------------------------------------------------------

static void setup_bg_sys( HEAPID heapID );
static void release_bg_sys( void );
static void scr_msg_print( GAMECLEAR_MSG_WORK* wk, u16 msg_id, u8 x, u8 y );


static GFL_PROC_RESULT GameClearMsgProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameClearMsgProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameClearMsgProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================================
//==============================================================================================
static const GFL_PROC_DATA GameClearMsgProcData = {
  GameClearMsgProc_Init,
  GameClearMsgProc_Main,
  GameClearMsgProc_End,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static GFL_PROC_RESULT GameClearMsgProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMECLEAR_MSG_WORK * gcmwk;
  const GAMECLEAR_WORK * gcwk = pwk;

 	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMEOVER_MSG, 0x40000 );
  gcmwk = GFL_PROC_AllocWork(proc, sizeof(GAMECLEAR_MSG_WORK), HEAPID_GAMEOVER_MSG);
	GFL_STD_MemClear32( gcmwk, sizeof(GAMECLEAR_MSG_WORK) );
  gcmwk->heapID = HEAPID_GAMEOVER_MSG;
  gcmwk->arc_id = gcwk->arc_id;
  gcmwk->msg_id = gcwk->msg_id;
  gcmwk->mystatus = gcwk->mystatus;
  
  return GFL_PROC_RES_FINISH;
}
  
//--------------------------------------------------------------
//--------------------------------------------------------------
static GFL_PROC_RESULT GameClearMsgProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMECLEAR_MSG_WORK * wk = mywk;

  int trg = GFL_UI_KEY_GetTrg();

	switch( *seq ){
  case 0:

    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_WHITE);
    WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_WHITE);
    WIPE_ResetWndMask(WIPE_DISP_MAIN);
    WIPE_ResetWndMask(WIPE_DISP_SUB);

    setup_bg_sys( wk->heapID );

		wk->fontHandle = GFL_FONT_Create(
			ARCID_FONT,
      NARC_font_large_gftr, //新フォントID
			GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

    wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
    //メッセージデータマネージャー作成
    wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
        ARCID_SCRIPT_MESSAGE, wk->arc_id, wk->heapID);
    wk->wordset = WORDSET_Create( wk->heapID );

    //ビットマップ追加
    wk->bmpwin = GFL_BMPWIN_Create(
      GAME_OVER_BMPWIN_FRAME,						//ウインドウ使用フレーム
      GAME_OVER_BMPWIN_PX1,GAME_OVER_BMPWIN_PY1,	//ウインドウ領域の左上のX,Y座標（キャラ単位で指定）
      GAME_OVER_BMPWIN_SX, GAME_OVER_BMPWIN_SY,	//ウインドウ領域のX,Yサイズ（キャラ単位で指定）
      GAME_OVER_BMPWIN_PL,						//ウインドウ領域のパレットナンバー	
      GAME_OVER_BMPWIN_CH							//ウインドウキャラ領域の開始キャラクタナンバー
    );

    scr_msg_print( wk, wk->msg_id, 0, 0 );

    GFL_BMPWIN_MakeTransWindow( wk->bmpwin );

    (*seq) ++;
    break;

	//メッセージ転送待ち
	case 1:
    PRINTSYS_QUE_Main( wk->printQue );
    if( PRINTSYS_QUE_IsFinished( wk->printQue ) == TRUE )
    {
      WIPE_SYS_Start(WIPE_PATTERN_M, WIPE_TYPE_FADEIN, WIPE_TYPE_MAX,
          WIPE_FADE_WHITE, GAME_OVER_FADE_SYNC, WIPE_DEF_SYNC, wk->heapID);
      G2_BlendNone();
    }
		(*seq)++;
		break;

	//メイン画面フェードイン待ち
	case 2:
		if (WIPE_SYS_EndCheck()) {
			(*seq)++;
		}
		break;

	//キー待ち
	case 3:
		if( (trg & PAD_BUTTON_A) || (trg & PAD_BUTTON_B) ) {
			WIPE_SYS_Start(WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
					WIPE_FADE_BLACK, GAME_OVER_FADE_SYNC, WIPE_DEF_SYNC, wk->heapID);
			(*seq)++;
		}
		break;

	//メイン画面フェードアウト待ち
	case 4:
		if (WIPE_SYS_EndCheck()) {

			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmpwin), FBMP_COL_NULL );		//塗りつぶし

			(*seq)++;
		}
		break;

	//終了開放
	case 5:
		BmpWinFrame_Clear( wk->bmpwin, WINDOW_TRANS_ON );
		GFL_BMPWIN_Delete( wk->bmpwin );

		WORDSET_Delete( wk->wordset );
		GFL_MSG_Delete( wk->msgman );
		GFL_BG_FreeBGControl( GAME_OVER_BMPWIN_FRAME );

    PRINTSYS_QUE_Delete( wk->printQue ); 
    GFL_FONT_Delete( wk->fontHandle );

    release_bg_sys();

		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GFL_PROC_RESULT GameClearMsgProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_GAMEOVER_MSG );
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setup_bg_sys( HEAPID heapID )
{
	static const GFL_DISP_VRAM SetBankData = {
		GX_VRAM_BG_128_B,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_0_A,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_64K, // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K, // サブOBJマッピングモード
	};

	static const GFL_BG_SYS_HEADER BGsys_data = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
	};

	static const GFL_BG_BGCNT_HEADER header = {
		0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};

  GFL_BG_Init( heapID );
	GFL_DISP_SetBank( &SetBankData );
	GFL_BG_SetBGMode( &BGsys_data );
	GFL_BG_SetBGControl( GAME_OVER_BMPWIN_FRAME, &header, GFL_BG_MODE_TEXT );

	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_systemwin_nclr, PALTYPE_MAIN_BG, 
																0x20*GAME_OVER_BMPWIN_PL, 0x20, heapID );

	GFL_BG_SetBackGroundColor(GAME_OVER_BMPWIN_FRAME,0x7FFF);

  GFL_BMPWIN_Init( heapID );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void release_bg_sys( void )
{
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

//--------------------------------------------------------------
/**
 * @brief	メッセージ表示
 *
 * @param	wk			EV_WIN_WORK型のアドレス
 * @param	msg_id		メッセージID
 * @param	x			表示Ｘ座標
 * @param	y			表示Ｙ座標
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void scr_msg_print( GAMECLEAR_MSG_WORK* wk, u16 msg_id, u8 x, u8 y )
{
  PRINTSYS_LSB lsb = PRINTSYS_LSB_Make(FBMP_COL_BLACK,FBMP_COL_BLK_SDW,FBMP_COL_NULL);
	STRBUF* tmp_buf = GFL_STR_CreateBuffer( GAME_OVER_MSG_BUF_SIZE, wk->heapID );
	STRBUF* tmp_buf2= GFL_STR_CreateBuffer( GAME_OVER_MSG_BUF_SIZE, wk->heapID );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmpwin), FBMP_COL_NULL );			//塗りつぶし

	GFL_MSG_GetString( wk->msgman, msg_id, tmp_buf );

  //主人公名セット
  WORDSET_RegisterPlayerName( wk->wordset, 0, wk->mystatus );

	WORDSET_ExpandStr( wk->wordset, tmp_buf2, tmp_buf );
  PRINTSYS_PrintQueColor( wk->printQue , GFL_BMPWIN_GetBmp( wk->bmpwin ) , x , y ,
                          tmp_buf2, wk->fontHandle ,
                          lsb );

	GFL_STR_DeleteBuffer( tmp_buf );
	GFL_STR_DeleteBuffer( tmp_buf2 );
	return;
}





//============================================================================================
//============================================================================================
// メインシーケンス
enum {
	GMCLEAR_SEQ_INIT = 0,			// 初期化
	GMCLEAR_SEQ_DENDOU_DEMO,		// 殿堂入りデモ
	GMCLEAR_SEQ_SAVE_START,			// セーブ開始
	GMCLEAR_SEQ_SAVE_MESSAGE,		// セーブ中メッセージ表示
	GMCLEAR_SEQ_SAVE_MAIN,			// セーブメイン
	GMCLEAR_SEQ_SAVE_END,			// セーブ終了
	GMCLEAR_SEQ_WAIT1,				// ウェイト１
	GMCLEAR_SEQ_FADE_OUT,			// フェードアウトセット
	GMCLEAR_SEQ_BGM_FADE_OUT,		// BGMフェードアウトセット
	GMCLEAR_SEQ_WAIT2,				// ウェイト２
	GMCLEAR_SEQ_ENDING_DEMO,		// エンディングデモ
	GMCLEAR_SEQ_END,				// 終了
};



//-----------------------------------------------------------------------------
/**
 * @brief	ゲームクリアイベント
 * @param	event		イベント制御ワークへのポインタ
 * @retval	TRUE		イベント終了
 * @retval	FALSE		イベント継続中
 *
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT GMEVENT_GameClear(GMEVENT * event, int * seq, void *work)
{
  GAMECLEAR_WORK * gcwk = work;
  GMEVENT * call_event;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gcwk->gsys );

	switch (*seq) {
	case GMCLEAR_SEQ_INIT:
    PMSND_FadeOutBGM( 30 );
    { //フィールドマップを終了させる
      FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gcwk->gsys );
      GMEVENT * new_event = DEBUG_EVENT_GameEnd( gcwk->gsys, fieldmap );
      GMEVENT_CallEvent( event, new_event );
      //call_event = EVENT_FieldFadeOut( gcwk->gsys, fieldmap, FIELD_FADE_BLACK, FIELD_FADE_WAIT );
      //GMEVENT_CallEvent( event, call_event );
    }
		(*seq) ++;
		break;

	case GMCLEAR_SEQ_DENDOU_DEMO:
    if ( PMSND_CheckFadeOnBGM() == FALSE ) break;
    SCRIPT_CallGameClearScript( gcwk->gsys, HEAPID_PROC );
    { //殿堂入りの代わりにNの城デモ
      DEMO3D_PARAM * param = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(DEMO3D_PARAM) );
      param->demo_id = DEMO3D_ID_N_CASTLE;
      GMEVENT_CallProc( event, FS_OVERLAY_ID(demo3d), &Demo3DProcData, param );
      gcwk->pWork = param;
    }

		(*seq) ++;
		break;

	case GMCLEAR_SEQ_SAVE_START:
    //「上書きします」とか表示する予定
    GFL_HEAP_FreeMemory( gcwk->pWork );
		//警告BG以外を表示オフ
		SetBrightness( BRIGHTNESS_BLACK, (PLANEMASK_ALL^PLANEMASK_BG3), MASK_MAIN_DISPLAY);
		SetBrightness( BRIGHTNESS_BLACK, PLANEMASK_ALL, MASK_SUB_DISPLAY);
		//
    GAMEDATA_SaveAsyncStart( gamedata );
    (*seq) ++;
		break;

  case GMCLEAR_SEQ_SAVE_MESSAGE:
    gcwk->msg_id = msg_common_report_07;
    GMEVENT_CallProc( event, NO_OVERLAY_ID, &GameClearMsgProcData, gcwk );
    (*seq) ++;
		break;

	case GMCLEAR_SEQ_SAVE_MAIN:
    {
      // 分割セーブ実行
      SAVE_RESULT result = GAMEDATA_SaveAsyncMain( gamedata );

      // 結果を返す
      switch( result )
      {
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
        gcwk->saveSuccessFlag = TRUE;
        (*seq) ++;
        break;
      case SAVE_RESULT_NG:
        gcwk->saveSuccessFlag = FALSE;
        (*seq) ++;
        break;
      }
    }
		break;

	case GMCLEAR_SEQ_SAVE_END:
		//
    if (gcwk->saveSuccessFlag) {
      gcwk->msg_id = msg_common_report_04;
    } else {
      gcwk->msg_id = msg_common_report_06;
    }
    GMEVENT_CallProc( event, NO_OVERLAY_ID, &GameClearMsgProcData, gcwk );
		(*seq) ++;
		break;

	case GMCLEAR_SEQ_WAIT1:				// ウェイト１
    (*seq) ++;
    break;
	case GMCLEAR_SEQ_FADE_OUT:			// フェードアウトセット
		//表示オフ解除
		SetBrightness( BRIGHTNESS_NORMAL, PLANEMASK_ALL, MASK_DOUBLE_DISPLAY);
    (*seq) ++;
    break;

	case GMCLEAR_SEQ_BGM_FADE_OUT:		// BGMフェードアウトセット
    (*seq) ++;
    break;
	case GMCLEAR_SEQ_WAIT2:				// ウェイト２
    (*seq) ++;
    break;
	case GMCLEAR_SEQ_ENDING_DEMO:
    { //エンディングのかわりにローカルトランシーバー
      u16 demo_id = 0;
      LOCAL_TVT_INIT_WORK * ltwk = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(LOCAL_TVT_INIT_WORK) );
      ltwk->scriptId = demo_id;
      ltwk->gameData = gamedata;
      gcwk->pWork = ltwk;
      GMEVENT_CallProc( event, FS_OVERLAY_ID(local_tvt), &LocalTvt_ProcData, ltwk );
    }
		(*seq) ++;
		break;

	case GMCLEAR_SEQ_END:
    GFL_HEAP_FreeMemory( gcwk->pWork );
    return GMEVENT_RES_FINISH;

  case GMCLEAR_SEQ_END + 1:
    /* 無限ループ */
    return GMEVENT_RES_CONTINUE;

	}
	return GMEVENT_RES_CONTINUE;
}


//-----------------------------------------------------------------------------
/**
 * @brief	イベントコマンド：通常全滅処理
 * @param	event		イベント制御ワークへのポインタ
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_GameClear( GAMESYS_WORK * gsys, GAMECLEAR_MODE mode )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	GMEVENT * event;
  GAMECLEAR_WORK * gcwk;
  event = GMEVENT_Create( gsys, NULL, GMEVENT_GameClear, sizeof(GAMECLEAR_WORK) );
  gcwk = GMEVENT_GetEventWork( event );
  gcwk->gsys = gsys;
  gcwk->clear_mode = mode;
  gcwk->arc_id = NARC_script_message_common_scr_dat;
  gcwk->msg_id = msg_common_report_06;
  gcwk->mystatus = GAMEDATA_GetMyStatus( gamedata );

  //移動ポケモン復活チェック
  MP_RecoverMovePoke( gamedata );

  return event;
}


