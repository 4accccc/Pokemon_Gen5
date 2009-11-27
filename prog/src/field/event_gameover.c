//============================================================================================
/**
 * @file	event_gameover.c
 * @brief	イベント：全滅処理
 * @since	2006.04.18
 * @date  2009.09.20
 * @author	tamada GAME FREAK inc.
 *
 * 2006.04.18 scr_tool.c,field_encount.cからゲームオーバー処理を持ってきて再構成した
 * 2009.09.20 HGSSから移植開始
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

#include "event_gameover.h"

#include "system/main.h"  //HEAPID_～

#include "system/brightness.h"

//#include "system/snd_tool.h"
//#include "fld_bgm.h"

//#include "system/window.h"
#include "system/wipe.h"

#include "warpdata.h"			//WARPDATA_～
#include "script.h"       //SCRIPT_CallScript
#include "event_fieldmap_control.h" //
#include "event_mapchange.h"

#include "../../../resource/fldmapdata/script/common_scr_def.h" //SCRID_～

//#include "situation_local.h"		//Situation_Get～
//#include "scr_tool.h"
//#include "mapdefine.h"
//#include "..\fielddata\script\common_scr_def.h"		//SCRID_GAME_OVER_RECOVER_PC
//#include "sysflag.h"
//#include "fld_flg_init.h"			//FldFlgInit_GameOver
//#include "system/savedata.h"
//#include "poketool/pokeparty.h"
//#include "poketool\status_rcv.h"

#include "field/field_msgbg.h"
#include "system/bmp_winframe.h"
//==============================================================================================
//
//	全滅関連
//
//==============================================================================================
//#include "system/fontproc.h"						
//#include "system/msgdata.h"							//MSGMAN_TYPE_DIRECT
//#include "system/wordset.h"							//WORDSET_Create
#include "print/wordset.h"

//#include "fld_bmp.h"						

//#include "msgdata/msg.naix"							//NARC_msg_??_dat
//#include "msgdata/msg_gameover.h"					//msg_all_dead_??
//#include "arc/
//#include "system/arc_util.h"
//#include "system/font_arc.h"
#include "font/font.naix" //NARC_font_large_gftr
#include "message.naix"
#include "msg/msg_gameover.h"

#include "arc/fieldmap/zone_id.h"

#include "pleasure_boat.h"

//==============================================================================================
//==============================================================================================
//--------------------------------------------------------------
/**
 * @brief   復活処理タイプ
 */
//--------------------------------------------------------------
typedef enum {
  REVIVAL_TYPE_HOME,      ///<自宅で復活
  REVIVAL_TYPE_POKECEN,   ///<ポケセンで復活
}REVIVAL_TYPE;

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  GAMESYS_WORK * gsys;
  REVIVAL_TYPE rev_type;
  const MYSTATUS * mystatus;
}GAMEOVER_WORK;


//==============================================================================================
//==============================================================================================
//----------------------------------------------------------------------------------------------
//	構造体宣言
//----------------------------------------------------------------------------------------------
typedef struct{
  HEAPID heapID;
  REVIVAL_TYPE rev_type;
  const MYSTATUS * mystatus;

	GFL_MSGDATA* msgman;						//メッセージマネージャー
	WORDSET* wordset;								//単語セット
	GFL_BMPWIN *bmpwin;							//BMPウィンドウデータ

  PRINT_QUE *printQue;
  GFL_FONT * fontHandle;
}GAMEOVER_MSG_WORK;


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
static void scr_msg_print( GAMEOVER_MSG_WORK* wk, u16 msg_id, u8 x, u8 y );


static GFL_PROC_RESULT GameOverMsgProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameOverMsgProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameOverMsgProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

//==============================================================================================
//==============================================================================================
static const GFL_PROC_DATA GameOverMsgProcData = {
  GameOverMsgProc_Init,
  GameOverMsgProc_Main,
  GameOverMsgProc_End,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static GFL_PROC_RESULT GameOverMsgProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMEOVER_MSG_WORK * gow;
  const GAMEOVER_WORK * param = pwk;

 	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMEOVER_MSG, 0x40000 );
  gow = GFL_PROC_AllocWork(proc, sizeof(GAMEOVER_MSG_WORK), HEAPID_GAMEOVER_MSG);
	GFL_STD_MemClear32( gow, sizeof(GAMEOVER_MSG_WORK) );
  gow->heapID = HEAPID_GAMEOVER_MSG;
  gow->rev_type = param->rev_type;
  gow->mystatus = param->mystatus;
  
  return GFL_PROC_RES_FINISH;
}
  
//--------------------------------------------------------------
//--------------------------------------------------------------
static GFL_PROC_RESULT GameOverMsgProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMEOVER_MSG_WORK * wk = mywk;

  int trg = GFL_UI_KEY_GetTrg();

	switch( *seq ){
  case 0:

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
        ARCID_MESSAGE, NARC_message_gameover_dat, wk->heapID);
    wk->wordset = WORDSET_Create( wk->heapID );

    //ビットマップ追加
    wk->bmpwin = GFL_BMPWIN_Create(
      GAME_OVER_BMPWIN_FRAME,						//ウインドウ使用フレーム
      GAME_OVER_BMPWIN_PX1,GAME_OVER_BMPWIN_PY1,	//ウインドウ領域の左上のX,Y座標（キャラ単位で指定）
      GAME_OVER_BMPWIN_SX, GAME_OVER_BMPWIN_SY,	//ウインドウ領域のX,Yサイズ（キャラ単位で指定）
      GAME_OVER_BMPWIN_PL,						//ウインドウ領域のパレットナンバー	
      GAME_OVER_BMPWIN_CH							//ウインドウキャラ領域の開始キャラクタナンバー
    );

    if ( wk->rev_type == REVIVAL_TYPE_HOME )
    { //自宅に戻るとき
      scr_msg_print( wk, msg_all_dead_05, 0, 0 );
    }
    else
    { //ポケセンに戻るとき
      scr_msg_print( wk, msg_all_dead_04, 0, 0 );
    }

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
static GFL_PROC_RESULT GameOverMsgProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
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
static void scr_msg_print( GAMEOVER_MSG_WORK* wk, u16 msg_id, u8 x, u8 y )
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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static REVIVAL_TYPE getRevType( GAMEDATA * gamedata )
{
  if(	WARPDATA_GetInitializeID() == GAMEDATA_GetWarpID(gamedata) ) {
    return REVIVAL_TYPE_HOME;
  } else {
    return REVIVAL_TYPE_POKECEN;
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief	通常戦闘：全滅シーケンス
 * @param	event		イベント制御ワークへのポインタ
 * @retval	TRUE		イベント終了
 * @retval	FALSE		イベント継続中
 *
 * @todo
 * ポケセンでの復活処理を以前の仕様で作るためにはジョーイさんOBJの
 * OBJIDを何らかの方法で取得し、アニメーションさせる必要がある
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT GMEVENT_NormalGameOver(GMEVENT * event, int * seq, void *work)
{
  GAMEOVER_WORK * param = work;

	switch (*seq) {
	case 0:
		//ワープIDで指定された戻り先へ
    //この中ではフィールド復帰処理は動作しない！
    MAPCHG_GameOver( param->gsys );
		(*seq) ++;
		break;

	case 1:
		//BGMフェードアウト
		//Snd_BgmFadeOut( 0, 20 );
		(*seq) ++;
		break;

	case 2:
		//BGMフェードアウト待ち
		/*if( Snd_FadeCheck() == 0 ) */{

			//サウンドゲームオーバー処理
			//Snd_GameOverSet();

			(*seq) ++;
		}
		break;

	case 3:
		//警告BG以外を表示オフ
		SetBrightness( BRIGHTNESS_BLACK, (PLANEMASK_ALL^PLANEMASK_BG3), MASK_MAIN_DISPLAY);
		SetBrightness( BRIGHTNESS_BLACK, PLANEMASK_ALL, MASK_SUB_DISPLAY);

		//ゲームオーバー警告
    GMEVENT_CallProc( event, NO_OVERLAY_ID, &GameOverMsgProcData, param );
		(*seq) ++;
		break;

	case 4:
		//イベントコマンド：フィールドマッププロセス復帰
    GMEVENT_CallEvent( event, EVENT_FieldOpen(param->gsys) );
		(*seq)++;
		break;

	case 5:
		//表示オフ解除
		SetBrightness( BRIGHTNESS_NORMAL, PLANEMASK_ALL, MASK_DOUBLE_DISPLAY);

		//気をつけてねスクリプト
		
		//話しかけ対象のOBJを取得する処理が必要になる
		//OS_Printf( "field_encount zone_id = %d\n", fsys->location->zone_id );
    if ( param->rev_type == REVIVAL_TYPE_HOME ) {
			//初期値のワープID＝＝最初の戻り先なので自分の家
			SCRIPT_CallScript( event, SCRID_GAMEOVER_RECOVER_HOME, NULL, NULL, HEAPID_FIELDMAP );
		}else{
			//それ以外＝＝ポケセンのはず
			SCRIPT_CallScript( event, SCRID_GAMEOVER_RECOVER_POKECEN, NULL, NULL, HEAPID_FIELDMAP );
		}
		(*seq) ++;
		break;

	case 6:
		//サウンドリスタート処理(06/07/10いらないので削除)
		//Snd_RestartSet( fsys );

		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}


//-----------------------------------------------------------------------------
/**
 * @brief	イベントコマンド：通常全滅処理
 * @param	event		イベント制御ワークへのポインタ
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_NormalLose( GAMESYS_WORK * gsys )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
	GMEVENT * event;
  GAMEOVER_WORK * param;
  event = GMEVENT_Create( gsys, NULL, GMEVENT_NormalGameOver, sizeof(GAMEOVER_WORK) );
  param = GMEVENT_GetEventWork( event );
  param->gsys = gsys;
  param->rev_type = getRevType( gamedata );
  param->mystatus = GAMEDATA_GetMyStatus( gamedata );

  //遊覧船ワーク解放
  {
    PL_BOAT_WORK_PTR *ptr = GAMEDATA_GetPlBoatWorkPtr(gamedata);
    PL_BOAT_End(ptr);
  }
  return event;
}


