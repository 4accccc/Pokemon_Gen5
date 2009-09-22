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

//----------------------------------------------------------------------------------------------
//	構造体宣言
//----------------------------------------------------------------------------------------------
typedef struct{
  GAMESYS_WORK * gsys;

  HEAPID heapID;
  REVIVAL_TYPE rev_type;

	GFL_MSGDATA* msgman;						//メッセージマネージャー
	WORDSET* wordset;								//単語セット
	GFL_BMPWIN *bmpwin;							//BMPウィンドウデータ

  PRINT_QUE *printQue;
  GFL_FONT * fontHandle;
}GAME_OVER_WORK;


#define GAME_OVER_MSG_BUF_SIZE		(1024)			//メッセージバッファサイズ
#define GAME_OVER_FADE_SYNC			(8)				//フェードsync数

enum { //コンパイルを通すためにとりあえず

  FLD_SYSFONT_PAL = 13,

  FBMP_COL_NULL = 0,
  FBMP_COL_BLACK = 1,
  FBMP_COL_BLK_SDW = 2,
};
//----------------------------------------------------------------------------------------------
//	BMPウィンドウ
//----------------------------------------------------------------------------------------------
enum{
	GAME_OVER_BMPWIN_FRAME	= GFL_BG_FRAME3_M,
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
static GMEVENT * createGameOverEvent( GAMESYS_WORK * gsys, REVIVAL_TYPE rev_type );
static GMEVENT_RESULT GMEVENT_GameOver( GMEVENT * event, int*seq, void * work );
static void scr_msg_print( GAME_OVER_WORK* wk, u16 msg_id, u8 x, u8 y );


//==============================================================================================
//==============================================================================================

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setup_bg_sys( GAME_OVER_WORK * wk )
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
		GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};

	GFL_DISP_SetBank( &SetBankData );
	GFL_BG_SetBGMode( &BGsys_data );
	GFL_BG_SetBGControl( GAME_OVER_BMPWIN_FRAME, &header, GFL_BG_MODE_TEXT );

	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_systemwin_nclr, PALTYPE_MAIN_BG, 
																0x20*GAME_OVER_BMPWIN_PL, 0x20, wk->heapID );

	GFL_BG_SetBackGroundColor(GAME_OVER_BMPWIN_FRAME,0x7FFF);
}

//--------------------------------------------------------------
/**
 * @brief	ゲームオーバー画面呼び出し
 *
 * @param	gsys	GAMESYS_WORKポインタ
 * @param event 親イベントのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static GMEVENT * createGameOverEvent( GAMESYS_WORK * gsys, REVIVAL_TYPE rev_type )
{
  GMEVENT * event;
	GAME_OVER_WORK* wk;

  event = GMEVENT_Create( gsys, NULL, GMEVENT_GameOver, sizeof(GAME_OVER_WORK) );
  wk = GMEVENT_GetEventWork( event );
	if( wk == NULL ){
		GF_ASSERT_MSG(0, "メモリ確保に失敗しました！" );
	}
	GFL_STD_MemClear32( wk, sizeof(GAME_OVER_WORK) );
  wk->gsys = gsys;
  wk->heapID = HEAPID_PROC;
  wk->rev_type = rev_type;

  return event;
}

//--------------------------------------------------------------
/**
 * @brief	ゲームオーバー画面メイン
 *
 * @param	tcb		TCB_PTR型
 * @param	work	ワーク
 *
 * @retval	none
 */
//--------------------------------------------------------------
static GMEVENT_RESULT GMEVENT_GameOver( GMEVENT * event, int*seq, void * work )
{
	GAME_OVER_WORK * wk = work;
  int trg = GFL_UI_KEY_GetTrg();
  GAMESYS_WORK * gsys = wk->gsys;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );

	switch( *seq ){
  case 0:

    WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_WHITE);
    WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_WHITE);
    WIPE_ResetWndMask(WIPE_DISP_MAIN);
    WIPE_ResetWndMask(WIPE_DISP_SUB);

    setup_bg_sys(wk);

		wk->fontHandle = GFL_FONT_Create(
			ARCID_FONT,
      NARC_font_large_gftr, //新フォントID
			GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

    wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
    //メッセージデータマネージャー作成
    wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_gameover_dat, wk->heapID);
    wk->wordset = WORDSET_Create( wk->heapID );

    //ビットマップ追加
    wk->bmpwin = GFL_BMPWIN_Create(
      GAME_OVER_BMPWIN_FRAME,						//ウインドウ使用フレーム
      GAME_OVER_BMPWIN_PX1,GAME_OVER_BMPWIN_PY1,	//ウインドウ領域の左上のX,Y座標（キャラ単位で指定）
      GAME_OVER_BMPWIN_SX, GAME_OVER_BMPWIN_SY,	//ウインドウ領域のX,Yサイズ（キャラ単位で指定）
      GAME_OVER_BMPWIN_PL,						//ウインドウ領域のパレットナンバー	
      GAME_OVER_BMPWIN_CH							//ウインドウキャラ領域の開始キャラクタナンバー
    );

    //主人公名セット
    WORDSET_RegisterPlayerName( wk->wordset, 0, GAMEDATA_GetMyStatus(gamedata) );
    {
      if ( wk->rev_type == REVIVAL_TYPE_HOME )
      { //自宅に戻るとき
        scr_msg_print( wk, msg_all_dead_05, 0, 0 );
      }
      else
      { //ポケセンに戻るとき
        scr_msg_print( wk, msg_all_dead_04, 0, 0 );
      }
    }

    GFL_BMPWIN_MakeTransWindow( wk->bmpwin );

    (*seq) ++;
    break;

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

	//メイン画面ブラックアウト待ち
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
		GFL_HEAP_FreeMemory( wk );

		return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE;
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
static void scr_msg_print( GAME_OVER_WORK* wk, u16 msg_id, u8 x, u8 y )
{
  PRINTSYS_LSB lsb = PRINTSYS_LSB_Make(FBMP_COL_BLACK,FBMP_COL_BLK_SDW,FBMP_COL_NULL);
	STRBUF* tmp_buf = GFL_STR_CreateBuffer( GAME_OVER_MSG_BUF_SIZE, wk->heapID );
	STRBUF* tmp_buf2= GFL_STR_CreateBuffer( GAME_OVER_MSG_BUF_SIZE, wk->heapID );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmpwin), FBMP_COL_NULL );			//塗りつぶし

	GFL_MSG_GetString( wk->msgman, msg_id, tmp_buf );

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
/**
 * @todo  とりあえずエラー回避のために作成した関数
 */
//-----------------------------------------------------------------------------
static u16 get_warp_id( GAMEDATA * gamedata )
{
  return GAMEDATA_GetWarpID( gamedata );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static REVIVAL_TYPE getRevType( GAMEDATA * gamedata )
{
  if(	WARPDATA_GetInitializeID() == get_warp_id(gamedata) ) {
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
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT GMEVENT_NormalGameOver(GMEVENT * event, int * seq, void *work)
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork( event );
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );

	switch (*seq) {
	case 0:
		//ワープIDで指定された戻り先へ
    //この中ではフィールド復帰処理は動作しない！
    MAPCHG_GameOver( gsys );
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
    //GMEVENT_CallEvent( event, createGameOverEvent( gsys, getRevType(gamedata) ) );
		(*seq) ++;
		break;

	case 4:
		//イベントコマンド：フィールドマッププロセス復帰
    GMEVENT_CallEvent( event, EVENT_FieldOpen(gsys) );
		(*seq)++;
		break;

	case 5:
		//表示オフ解除
		SetBrightness( BRIGHTNESS_NORMAL, PLANEMASK_ALL, MASK_DOUBLE_DISPLAY);

		//気をつけてねスクリプト
		
		//話しかけ対象のOBJを取得する処理が必要になる
		//OS_Printf( "field_encount zone_id = %d\n", fsys->location->zone_id );
    if ( getRevType( gamedata ) == REVIVAL_TYPE_HOME ) {
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
	GMEVENT * event;
  event = GMEVENT_Create(gsys, NULL, GMEVENT_NormalGameOver, 0);
  return event;
}


