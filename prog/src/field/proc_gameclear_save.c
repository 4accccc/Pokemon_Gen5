//============================================================================================
/**
 * @file  proc_gameclear_save.c
 * @date  2009.12.20
 * @author  tamada GAMEFREAK inc.
 * @brief ゲームクリア時のセーブなど
 *
 * @todo
 * 適当に作成した仮画面なので、実際にはUI班がきちんとした仕様をもとに作り直す。
 */
//============================================================================================

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"

#include "proc_gameclear_save.h"

#include "system/main.h"  //HEAPID_～
#include "arc/arc_def.h" //ARCID_～

//==============================================================================================
//
//	メッセージ表示関連
//
//==============================================================================================
#include "print/wordset.h"  //WORDSET_
#include "print/printsys.h" //PRINTSYS_
#include "print/gf_font.h"  // GFL_FONT_


#include "font/font.naix" //NARC_font_large_gftr
#include "message.naix"
#include "msg/msg_gameclear.h"
//#include "arc/script_message.naix"
//#include "msg/script/msg_common_scr.h"

//#include "field/field_msgbg.h"
#include "system/bmp_winframe.h"


//==============================================================================================
//==============================================================================================
//----------------------------------------------------------------------------------------------
//	構造体宣言
//----------------------------------------------------------------------------------------------
typedef struct{
  HEAPID heapID;
  u16 arc_id;
  GAMEDATA * gamedata;
  const MYSTATUS * mystatus;

  GFL_FONT * fontHandle;
	GFL_MSGDATA* msgman;						//メッセージマネージャー
	WORDSET* wordset;								//単語セット
	GFL_BMPWIN *bmpwin;							//BMPウィンドウデータ

  BOOL saveSuccessFlag;

}GAMECLEAR_MSG_WORK;


#define GAMECLEAR_MSG_BUF_SIZE		(1024)			//メッセージバッファサイズ
#define GAMECLEAR_FADE_SYNC			(8)				//フェードsync数

enum { //コンパイルを通すためにとりあえず

  FLD_SYSFONT_PAL = 13,

  FBMP_COL_NULL = 0,
  FBMP_COL_BLACK = 0xf,
  FBMP_COL_BLK_SDW = 2,

  PALNO_FONT = 1,//FBMP_COL_BLACK,
  PALNO_FONTSHADOW = FBMP_COL_BLK_SDW,
  PALNO_BACKGROUND = 15,//FBMP_COL_NULL,
};
//----------------------------------------------------------------------------------------------
//	BMPウィンドウ
//----------------------------------------------------------------------------------------------
enum{
	GAMECLEAR_BMPWIN_FRAME	= GFL_BG_FRAME0_M,
	GAMECLEAR_BMPWIN_PX1	= 3,
	GAMECLEAR_BMPWIN_PY1	= 5,
	GAMECLEAR_BMPWIN_SX		= 28,
	GAMECLEAR_BMPWIN_SY		= 15,
	GAMECLEAR_BMPWIN_PL		= FLD_SYSFONT_PAL,
	GAMECLEAR_BMPWIN_CH		= 1,
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
//--------------------------------------------------------------
//--------------------------------------------------------------
const GFL_PROC_DATA GameClearMsgProcData = {
  GameClearMsgProc_Init,
  GameClearMsgProc_Main,
  GameClearMsgProc_End,
};

//--------------------------------------------------------------
//--------------------------------------------------------------
static GFL_PROC_RESULT GameClearMsgProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMECLEAR_MSG_WORK * gcmwk;
  GAMESYS_WORK * gsys = pwk;

 	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMEOVER_MSG, 0x40000 );
  gcmwk = GFL_PROC_AllocWork(proc, sizeof(GAMECLEAR_MSG_WORK), HEAPID_GAMEOVER_MSG);
	GFL_STD_MemClear32( gcmwk, sizeof(GAMECLEAR_MSG_WORK) );
  gcmwk->heapID = HEAPID_GAMEOVER_MSG;
  gcmwk->arc_id = NARC_message_gameclear_dat;
  gcmwk->gamedata = GAMESYSTEM_GetGameData( gsys );
  gcmwk->mystatus = GAMEDATA_GetMyStatus( gcmwk->gamedata );
  
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

    setup_bg_sys( wk->heapID );

		wk->fontHandle = GFL_FONT_Create(
			ARCID_FONT,
      NARC_font_large_gftr, //新フォントID
			GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

    //メッセージデータマネージャー作成
    wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
        ARCID_MESSAGE, wk->arc_id, wk->heapID);
    wk->wordset = WORDSET_Create( wk->heapID );

    //ビットマップ追加
    wk->bmpwin = GFL_BMPWIN_Create(
      GAMECLEAR_BMPWIN_FRAME,						//ウインドウ使用フレーム
      GAMECLEAR_BMPWIN_PX1,GAMECLEAR_BMPWIN_PY1,	//ウインドウ領域の左上のX,Y座標（キャラ単位で指定）
      GAMECLEAR_BMPWIN_SX, GAMECLEAR_BMPWIN_SY,	//ウインドウ領域のX,Yサイズ（キャラ単位で指定）
      GAMECLEAR_BMPWIN_PL,						//ウインドウ領域のパレットナンバー	
      GAMECLEAR_BMPWIN_CH							//ウインドウキャラ領域の開始キャラクタナンバー
    );

    //でんどういり　おめでとう！
    scr_msg_print( wk, msg_gameclear_01, 0, 0 );

    GFL_BMPWIN_MakeTransWindow( wk->bmpwin );

    (*seq) ++;
    break;

	//メッセージ転送待ち
	case 1:
    {
      G2_BlendNone();
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0 );
    }
		(*seq)++;
		break;

	//メイン画面フェードイン待ち
	case 2:
    if( GFL_FADE_CheckFade() == FALSE ) {
			(*seq)++;
		}
		break;

	//キー待ち
	case 3:
		if( (trg & PAD_BUTTON_A) || (trg & PAD_BUTTON_B) ) {
      //「レポートをかいています　でんげんをきらないでください」
      scr_msg_print( wk, msg_gameclear_report_01, 0, 0 );
      GAMEDATA_SaveAsyncStart( wk->gamedata );
      (*seq) ++;
    }
    break;
  case 4:
    {
      // 分割セーブ実行
      SAVE_RESULT result = GAMEDATA_SaveAsyncMain( wk->gamedata );

      // 結果を返す
      switch( result )
      {
      case SAVE_RESULT_CONTINUE:
      case SAVE_RESULT_LAST:
        break;
      case SAVE_RESULT_OK:
        wk->saveSuccessFlag = TRUE;
        (*seq) ++;
        break;
      case SAVE_RESULT_NG:
        wk->saveSuccessFlag = FALSE;
        (*seq) ++;
        break;
      }
    }
    break;

  case 5:
    if (wk->saveSuccessFlag) {
      //「レポートをかきました」
      scr_msg_print( wk, msg_gameclear_report_02, 0, 0 );
    } else {
      //「レポートがかけませんでした」
      scr_msg_print( wk, msg_gameclear_report_03, 0, 0 );
    }
    (*seq) ++;
    break;

  case 6:
		if( (trg & PAD_BUTTON_A) || (trg & PAD_BUTTON_B) ) {
      //「おしまい」
      scr_msg_print( wk, msg_gameclear_02, 0, 0 );
      (*seq) ++;
    }
    break;

  case 7:
		if( (trg & PAD_BUTTON_A) || (trg & PAD_BUTTON_B) ) {
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 0 );
			(*seq)++;
		}
		break;

	//メイン画面フェードアウト待ち
	case 8:
    if( GFL_FADE_CheckFade() == FALSE ) {

			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmpwin),  PALNO_BACKGROUND );		//塗りつぶし

			(*seq)++;
		}
		break;

	//終了開放
	case 9:
		BmpWinFrame_Clear( wk->bmpwin, WINDOW_TRANS_ON );
		GFL_BMPWIN_Delete( wk->bmpwin );

		WORDSET_Delete( wk->wordset );
		GFL_MSG_Delete( wk->msgman );
		GFL_BG_FreeBGControl( GAMECLEAR_BMPWIN_FRAME );

    GFL_FONT_Delete( wk->fontHandle );

    release_bg_sys();

    (*seq) ++;
    break;

  case 10:
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
	GFL_BG_SetBGControl( GAMECLEAR_BMPWIN_FRAME, &header, GFL_BG_MODE_TEXT );

	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_systemwin_nclr, PALTYPE_MAIN_BG, 
																0x20*GAMECLEAR_BMPWIN_PL, 0x20, heapID );

	GFL_BG_SetBackGroundColor(GAMECLEAR_BMPWIN_FRAME,0);

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
  PRINTSYS_LSB lsb = PRINTSYS_LSB_Make( PALNO_FONT, PALNO_FONTSHADOW, PALNO_BACKGROUND);
	STRBUF* tmp_buf = GFL_STR_CreateBuffer( GAMECLEAR_MSG_BUF_SIZE, wk->heapID );
	STRBUF* tmp_buf2= GFL_STR_CreateBuffer( GAMECLEAR_MSG_BUF_SIZE, wk->heapID );

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmpwin),  PALNO_BACKGROUND );			//塗りつぶし

	GFL_MSG_GetString( wk->msgman, msg_id, tmp_buf );

  //主人公名セット
  WORDSET_RegisterPlayerName( wk->wordset, 0, wk->mystatus );

	WORDSET_ExpandStr( wk->wordset, tmp_buf2, tmp_buf );
  GFL_FONTSYS_SetColor( PALNO_FONT, PALNO_FONTSHADOW,  PALNO_BACKGROUND );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( wk->bmpwin ), x, y, tmp_buf2, wk->fontHandle );

	GFL_STR_DeleteBuffer( tmp_buf );
	GFL_STR_DeleteBuffer( tmp_buf2 );

  GFL_BMPWIN_TransVramCharacter( wk->bmpwin );  //transfer characters
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( wk->bmpwin ) );  //transfer screen

	return;
}





