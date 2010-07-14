//============================================================================================
/**
 * @file  proc_gameclear_save.c
 * @date  2009.12.20
 * @author  tamada / mori GAMEFREAK inc.
 * @brief ゲームクリア時のセーブなど
 *
 */
//============================================================================================

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/msgspeed.h"
#include "system/bmp_winframe.h"     // BmpWinFrame_GraphicSet
#include "app/app_printsys_common.h" //
#include "system/time_icon.h"
#include "sound/pm_sndsys.h"

#include "savedata/save_tbl.h"
#include "savedata/record.h"
#include "savedata/gametime.h"
#include "savedata/playtime.h"
#include "ev_time.h"

#include "proc_gameclear_save.h"

#include "system/main.h"  //HEAPID_～
#include "arc/arc_def.h" //ARCID_～

//==============================================================================================
//
//  メッセージ表示関連
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
//  構造体宣言
//----------------------------------------------------------------------------------------------
typedef struct{
  HEAPID heapID;
  u16 arc_id;
  GAMEDATA * gamedata;            ///< ゲームデータアクセス構造体
  const MYSTATUS * mystatus;      ///< 自分情報

  STRBUF*   expand_buf;           ///< 文字列表示用バッファ
  GFL_FONT* fontHandle;
  GFL_MSGDATA* msgman;            ///< メッセージマネージャー
  WORDSET* wordset;               ///< 単語セット
  GFL_BMPWIN *bmpwin;             ///< BMPウィンドウデータ
  PRINT_STREAM *printStream;      ///< 文字列表示管理構造体
  GFL_TCBLSYS  *pMsgTcbSys;       ///< メッセージ表示管理タスク
  APP_PRINTSYS_COMMON_WORK AppPrintWait;
  TIMEICON_WORK *TimeIconWork;

  BOOL saveSuccessFlag;           ///< セーブに成功したかどうか
  BOOL dendouSaveInitFlag;        ///<「殿堂入り」データの分割セーブ初期化が完了したかどうか
  BOOL dendouSaveFinishFlag;      ///<「殿堂入り」データの分割セーブが完了したかどうか
  BOOL otherSaveExist;            ///< 既存のデータが存在するかどうか

  int  next_seq;
  int  wait;

}GAMECLEAR_MSG_WORK;


#define GAMECLEAR_MSG_BUF_SIZE    (1024)  ///< メッセージバッファサイズ
#define GAMECLEAR_FADE_SYNC       (   8)  ///< フェードsync数

// 文字色指定
#define GAMECELAR_MSG_COL       ( PRINTSYS_LSB PRINTSYS_LSB_Make( u8 l, u8 s, u8 b ) )

// BG面初期化用定義
enum { 

  FLD_SYSFONT_PAL = 13,

  FBMP_COL_NULL = 0,
  FBMP_COL_BLACK = 0xf,
  FBMP_COL_BLK_SDW = 2,

  PALNO_FONT = 1,//FBMP_COL_BLACK,
  PALNO_FONTSHADOW = FBMP_COL_BLK_SDW,
  PALNO_BACKGROUND = 15,//FBMP_COL_NULL,
};
//----------------------------------------------------------------------------------------------
//  BMPウィンドウ
//----------------------------------------------------------------------------------------------
enum{
  GAMECLEAR_BMPWIN_FRAME  = GFL_BG_FRAME0_M,
  GAMECLEAR_BMPWIN_PX1  = 2,
  GAMECLEAR_BMPWIN_PY1  = 18,
  GAMECLEAR_BMPWIN_SX   = 27,
  GAMECLEAR_BMPWIN_SY   = 4,
  GAMECLEAR_BMPWIN_PL   = FLD_SYSFONT_PAL,
};

// ウインドウ枠
enum{
  GAMECLEAR_WINFRAME_CHAR = 1,
  GAMECLEAR_WINFRAME_PAL  = 12,
};



//==============================================================================================
//==============================================================================================
//----------------------------------------------------------------------------------------------
//  プロトタイプ宣言
//----------------------------------------------------------------------------------------------

static void setup_bg_sys( HEAPID heapID );
static void release_bg_sys( void );
static void scr_msg_print( GAMECLEAR_MSG_WORK* wk, u16 msg_id, u8 x, u8 y );

static void UpdateFirstClearDendouData( GAMECLEAR_MSG_WORK* work ); // 初回クリア時の殿堂入りデータを登録する
static void UpdateFirstClearRecord( GAMECLEAR_MSG_WORK* work ); // 初回クリア時のレコードを登録する
static void UpdateDendouRecord( GAMECLEAR_MSG_WORK* work ); // 殿堂入り時のレコードを登録する
static void SaveFirstDendouiriTime( GAMECLEAR_MSG_WORK* work ); // 初「殿堂入り」の日時を記録する
static void DendouSave_init( GAMECLEAR_MSG_WORK* work ); // 殿堂入りデータの更新準備
static void DendouSave_main( GAMECLEAR_MSG_WORK* work ); // 殿堂入りデータの更新処理


static GFL_PROC_RESULT GameClearMsgProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameClearMsgProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT GameClearMsgProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static void _init_display( GAMECLEAR_MSG_WORK *wk, GAMECLEAR_MSG_PARAM *para );
static void _release_display( GAMECLEAR_MSG_WORK *wk);



//==============================================================================================
//==============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
const GFL_PROC_DATA GameClearMsgProcData = {
  GameClearMsgProc_Init,
  GameClearMsgProc_Main,
  GameClearMsgProc_End,
};

//----------------------------------------------------------------------------------
/**
 * @brief ゲームクリアセーブ画面初期化
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//----------------------------------------------------------------------------------
static GFL_PROC_RESULT GameClearMsgProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMECLEAR_MSG_WORK * gcmwk;
  GAMECLEAR_MSG_PARAM * para = pwk;
  GAMESYS_WORK * gsys = para->gsys;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMEOVER_MSG, 0x40000 );
  gcmwk = GFL_PROC_AllocWork(proc, sizeof(GAMECLEAR_MSG_WORK), HEAPID_GAMEOVER_MSG);
  GFL_STD_MemClear32( gcmwk, sizeof(GAMECLEAR_MSG_WORK) );
  gcmwk->heapID   = HEAPID_GAMEOVER_MSG;
  gcmwk->arc_id   = NARC_message_gameclear_dat;
  gcmwk->gamedata = GAMESYSTEM_GetGameData( gsys );
  gcmwk->mystatus = GAMEDATA_GetMyStatus( gcmwk->gamedata );

  // セーブ可能か？
  {
    SAVE_CONTROL_WORK* save;
    save = GAMEDATA_GetSaveControlWork( gcmwk->gamedata );

    if( SaveControl_IsOverwritingOtherData( save ) ) {
      // セーブ不可
      gcmwk->otherSaveExist = TRUE;
    }
  }
  
  return GFL_PROC_RES_FINISH;
}

enum{
  GAMECLEAR_SEQ_INIT=0,
  GAMECLEAR_SEQ_FADEIN,
  GAMECLEAR_SEQ_FADEIN_WAIT,
  GAMECLEAR_SEQ_KEY_WAIT,
  GAMECLEAR_SEQ_SAVETYPE_BRANCH,
  GAMECLEAR_SEQ_SAVE_DENDOU,
  GAMECLEAR_SEQ_SAVE_MAIN,
  GAMECLEAR_SEQ_MSG_SAVEEND,
  GAMECLEAR_SEQ_MSG_END,
  GAMECLEAR_SEQ_FADEOUT,
  GAMECLEAR_SEQ_FADEOUT_WAIT,
  GAMECLEAR_SEQ_EXIT,
  GAMECLEAR_SEQ_ALLEND,
  GAMECLEAR_SEQ_MSG_WAIT,
};


//----------------------------------------------------------------------------------
/**
 * @brief ゲームクリアセーブ画面メイン
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//----------------------------------------------------------------------------------
static GFL_PROC_RESULT GameClearMsgProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GAMECLEAR_MSG_WORK * wk = mywk;
  GAMECLEAR_MSG_PARAM * para = pwk;
  const u16 pal_white[]={0xffff};     //下画面0番パレット潰す用データ

  int trg = GFL_UI_KEY_GetTrg();

  switch( *seq ){
  case GAMECLEAR_SEQ_INIT:

    // 画面初期化
    _init_display(wk, para);

    (*seq)++;
    break;

  //メッセージ転送待ち
  case GAMECLEAR_SEQ_FADEIN:
    {
      G2_BlendNone();
      GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
      if(GetVersion()==VERSION_BLACK){
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
      }else{
        GXS_LoadBGPltt( pal_white, 0, 2);
        GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, 0 );
      }
    }
    (*seq)++;
    break;

  //メイン画面フェードイン待ち
  case GAMECLEAR_SEQ_FADEIN_WAIT:
    if( GFL_FADE_CheckFade() == FALSE ) {
      (*seq) = GAMECLEAR_SEQ_KEY_WAIT;
#if 0
      if ( para->clear_mode == GAMECLEAR_MODE_FIRST ) {
        //ゲームクリア　おめでとう！
        scr_msg_print( wk, msg_gameclear_first_01, 0, 0 );
        wk->next_seq = GAMECLEAR_SEQ_KEY_WAIT;
        (*seq)       = GAMECLEAR_SEQ_MSG_WAIT;
      } else {
        //でんどういり　おめでとう！
        scr_msg_print( wk, msg_gameclear_01, 0, 0 );
        wk->next_seq = GAMECLEAR_SEQ_KEY_WAIT;
        (*seq)       = GAMECLEAR_SEQ_MSG_WAIT;
    } 
    GFL_BMPWIN_MakeTransWindow( wk->bmpwin );
#endif
    }
    break;

  //キー待ち
  case GAMECLEAR_SEQ_KEY_WAIT:
    if( wk->otherSaveExist ) {
      (*seq) ++; 
    }
    else {
      //「レポートをかいています　でんげんをきらないでください」
      PMSND_PlaySE( SEQ_SE_DECIDE1 );
      scr_msg_print( wk, msg_gameclear_report_01, 0, 0 );
      GFL_BMPWIN_MakeTransWindow( wk->bmpwin );
      wk->next_seq = GAMECLEAR_SEQ_SAVETYPE_BRANCH;
      (*seq)       = GAMECLEAR_SEQ_MSG_WAIT;
      GAMEDATA_SaveAsyncStart( wk->gamedata );
    }
    break;

  case GAMECLEAR_SEQ_SAVETYPE_BRANCH: 
    if( wk->otherSaveExist == FALSE ) {

      //「初回クリア」をセーブ
      if( para->clear_mode == GAMECLEAR_MODE_FIRST ) {
        UpdateFirstClearDendouData( wk ); // 初回クリアデータ
        UpdateFirstClearRecord( wk ); // レコードデータ
      }
      //「殿堂入り」をセーブ
      else {
        SaveFirstDendouiriTime( wk ); // 初「殿堂入り」の日時
        UpdateDendouRecord( wk ); // 殿堂入り関連レコード
        DendouSave_init( wk );
      }
    }
    // 時間アイコン表示
    wk->TimeIconWork = TIMEICON_CreateTcbl( wk->pMsgTcbSys, wk->bmpwin, 15, 
                                            TIMEICON_DEFAULT_WAIT, wk->heapID );
    (*seq) ++;
    break;

  case GAMECLEAR_SEQ_SAVE_DENDOU:
    // 殿堂入りデータをセーブ
    if( wk->otherSaveExist==FALSE && wk->dendouSaveInitFlag ) {
      DendouSave_main( wk );

      // セーブ終了
      if( wk->dendouSaveFinishFlag ) { 
        (*seq) ++; 
      }
    }
    else {
      (*seq) ++;
    }
    break;

  case GAMECLEAR_SEQ_SAVE_MAIN:
    // 分割セーブ実行
    if( wk->otherSaveExist == FALSE ) {
      SAVE_RESULT result = GAMEDATA_SaveAsyncMain( wk->gamedata );

      // 結果を返す
      switch( result ) {
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
    else {
      (*seq) ++;
    }
    break;

  case GAMECLEAR_SEQ_MSG_SAVEEND:
    TIMEICON_Exit( wk->TimeIconWork );
    if (wk->otherSaveExist==FALSE && wk->saveSuccessFlag) {
      //「レポートをかきました」
      scr_msg_print( wk, msg_gameclear_report_02, 0, 0 );
      GFL_BMPWIN_MakeTransWindow( wk->bmpwin );
      wk->next_seq = GAMECLEAR_SEQ_MSG_END;
      (*seq)       = GAMECLEAR_SEQ_MSG_WAIT;
      PMSND_PlaySE( SEQ_SE_SAVE );
    } else {
      //「レポートがかけませんでした」
      scr_msg_print( wk, msg_gameclear_report_03, 0, 0 );
      GFL_BMPWIN_MakeTransWindow( wk->bmpwin );
      wk->next_seq = GAMECLEAR_SEQ_MSG_END;
      (*seq)       = GAMECLEAR_SEQ_MSG_WAIT;
    }
    break;

  case GAMECLEAR_SEQ_MSG_END:
    if( wk->wait>120 ) {
      (*seq) ++;
    }
    wk->wait++;
    break;

  case GAMECLEAR_SEQ_FADEOUT:
    if(GetVersion()==VERSION_BLACK){
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    }else{
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, 0 );
    }
    (*seq)++;
    break;

  //メイン画面フェードアウト待ち
  case GAMECLEAR_SEQ_FADEOUT_WAIT:
    if( GFL_FADE_CheckFade() == FALSE ) {

      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmpwin),  PALNO_BACKGROUND );    //塗りつぶし

      (*seq)++;
    }
    break;

  //終了開放
  case GAMECLEAR_SEQ_EXIT:
    _release_display(wk);

    (*seq) ++;
    break;

  case GAMECLEAR_SEQ_ALLEND:
    return GFL_PROC_RES_FINISH;

  // 文字表示待ち
  case GAMECLEAR_SEQ_MSG_WAIT:
    if(APP_PRINTSYS_COMMON_PrintStreamFunc( &wk->AppPrintWait, wk->printStream )){
      (*seq) = wk->next_seq;
    }
    break;
  }

  // メッセージタスクメイン
  if(wk->pMsgTcbSys){
    GFL_TCBL_Main( wk->pMsgTcbSys );
  }
  if(wk->TimeIconWork){
    TIMEICON_Main( wk->TimeIconWork );
  }

  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------------
/**
 * @brief ゲームクリアセーブ画面終了
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//----------------------------------------------------------------------------------
static GFL_PROC_RESULT GameClearMsgProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_GAMEOVER_MSG );
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// static 関数群
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
/**
 * @brief 画面初期化
 *
 * @param   wk    
 * @param   para    
 */
//----------------------------------------------------------------------------------
static void _init_display( GAMECLEAR_MSG_WORK *wk, GAMECLEAR_MSG_PARAM *para )
{
  GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
  GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
  MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
  (void)GX_DisableBankForLCDC();

  setup_bg_sys( wk->heapID );

  wk->fontHandle = GFL_FONT_Create(
    ARCID_FONT,
    NARC_font_large_gftr, //新フォントID
    GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  // 会話フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
                                 FLD_SYSFONT_PAL*0x20, 32, wk->heapID );

  //メッセージデータマネージャー作成
  wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
      ARCID_MESSAGE, wk->arc_id, wk->heapID);
  wk->wordset = WORDSET_Create( wk->heapID );

  // システムウインドウキャラクター転送
  BmpWinFrame_GraphicSet( GAMECLEAR_BMPWIN_FRAME, GAMECLEAR_WINFRAME_CHAR, 
                          GAMECLEAR_WINFRAME_PAL, MENU_TYPE_SYSTEM, wk->heapID );
  

  //ビットマップ追加
  wk->bmpwin = GFL_BMPWIN_Create(
    GAMECLEAR_BMPWIN_FRAME,                   //ウインドウ使用フレーム
    GAMECLEAR_BMPWIN_PX1,GAMECLEAR_BMPWIN_PY1,//ウインドウ領域の左上のX,Y座標（キャラ単位で指定）
    GAMECLEAR_BMPWIN_SX, GAMECLEAR_BMPWIN_SY, //ウインドウ領域のX,Yサイズ（キャラ単位で指定）
    GAMECLEAR_BMPWIN_PL,                      //ウインドウ領域のパレットナンバー  
    GFL_BMP_CHRAREA_GET_B                     //ウインドウキャラの取得方向
  );

  // メッセージ表示用タスクシステム確保
  wk->pMsgTcbSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 10, 32 );
  wk->expand_buf = GFL_STR_CreateBuffer( GAMECLEAR_MSG_BUF_SIZE, wk->heapID );
  
}


//----------------------------------------------------------------------------------
/**
 * @brief ワーク解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void _release_display( GAMECLEAR_MSG_WORK *wk )
{
    GFL_TCBL_DeleteAll( wk->pMsgTcbSys );
    GFL_TCBL_Exit( wk->pMsgTcbSys );
    wk->pMsgTcbSys = NULL;

    BmpWinFrame_Clear( wk->bmpwin, WINDOW_TRANS_ON );
    GFL_BMPWIN_Delete( wk->bmpwin );

    GFL_STR_DeleteBuffer( wk->expand_buf );
    WORDSET_Delete( wk->wordset );
    GFL_MSG_Delete( wk->msgman );
    GFL_BG_FreeBGControl( GAMECLEAR_BMPWIN_FRAME );

    GFL_FONT_Delete( wk->fontHandle );

    release_bg_sys();

}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setup_bg_sys( HEAPID heapID )
{
  static const GFL_DISP_VRAM SetBankData = {
    GX_VRAM_BG_128_B,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,       // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_0_A,        // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_01_FG,      // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_64K, // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K, // サブOBJマッピングモード
  };

  static const GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
  };

  static const GFL_BG_BGCNT_HEADER header = {
    0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
    GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
    GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
    GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
  };

  GFL_BG_Init( heapID );
  GFL_DISP_SetBank( &SetBankData );
  GFL_BG_SetBGMode( &BGsys_data );
  GFL_BG_SetBGControl( GAMECLEAR_BMPWIN_FRAME, &header, GFL_BG_MODE_TEXT );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_systemwin_nclr, PALTYPE_MAIN_BG, 
                                0x20*GAMECLEAR_BMPWIN_PL, 0x20, heapID );

  // ブラックバージョンでは背景を黒に・ホワイトでは白に
  if(GetVersion()== VERSION_WHITE){
    GFL_BG_SetBackGroundColor( GAMECLEAR_BMPWIN_FRAME, 0xffff );
  }else{
    GFL_BG_SetBackGroundColor( GAMECLEAR_BMPWIN_FRAME, 0 );
  }
  
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
 * @brief メッセージ表示
 *
 * @param wk      EV_WIN_WORK型のアドレス
 * @param msg_id    メッセージID
 * @param x     表示Ｘ座標
 * @param y     表示Ｙ座標
 *
 * @retval  none
 */
//--------------------------------------------------------------
static void scr_msg_print( GAMECLEAR_MSG_WORK* wk, u16 msg_id, u8 x, u8 y )
{
  PRINTSYS_LSB lsb = PRINTSYS_LSB_Make( PALNO_FONT, PALNO_FONTSHADOW, PALNO_BACKGROUND);
  STRBUF* tmp_buf = GFL_STR_CreateBuffer( GAMECLEAR_MSG_BUF_SIZE, wk->heapID );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmpwin),  0x0f0f );      //塗りつぶし

  GFL_MSG_GetString( wk->msgman, msg_id, tmp_buf );

  //主人公名セット
  WORDSET_RegisterPlayerName( wk->wordset, 0, wk->mystatus );

  WORDSET_ExpandStr( wk->wordset, wk->expand_buf, tmp_buf );
//  PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp( wk->bmpwin ), x, y, tmp_buf2, wk->fontHandle, PRINTSYS_LSB_Make( 1, 2, 15 ) );
  OS_Printf("msgspeed=%d\n",MSGSPEED_GetWait());

  wk->printStream = PRINTSYS_PrintStream(
    wk->bmpwin, x, y, wk->expand_buf, wk->fontHandle,
    MSGSPEED_GetWait(), wk->pMsgTcbSys, 0, wk->heapID, 0x0f0f );

  GFL_STR_DeleteBuffer( tmp_buf );

  BmpWinFrame_Write( wk->bmpwin, WINDOW_TRANS_OFF, GAMECLEAR_WINFRAME_CHAR, GAMECLEAR_WINFRAME_PAL );

  return;
}

//-----------------------------------------------------------------------------
/**
 * @brief 初回クリア時の殿堂入りデータを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateFirstClearDendouData( GAMECLEAR_MSG_WORK* work )
{
  RTCDate date;
  POKEPARTY* party; 
  DENDOU_RECORD* record; 

  RTC_GetDate( &date );
  party  = GAMEDATA_GetMyPokemon( work->gamedata ); 
  record = GAMEDATA_GetDendouRecord( work->gamedata ); 
  DendouRecord_Add( record, party, &date, HEAPID_PROC );
}

//-----------------------------------------------------------------------------
/**
 * @brief 初回クリア時のレコードを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateFirstClearRecord( GAMECLEAR_MSG_WORK* work )
{
  RECORD* record;
  PLAYTIME* ptime;
  u32 hour, minute, second, time;

  record = GAMEDATA_GetRecordPtr( work->gamedata );
  ptime  = GAMEDATA_GetPlayTimeWork( work->gamedata );

  // プレイ時間を取得
  hour   = PLAYTIME_GetHour( ptime );
  minute = PLAYTIME_GetMinute( ptime );
  second = PLAYTIME_GetSecond( ptime ); 
  GF_ASSERT( hour   <= PTIME_HOUR_MAX );
  GF_ASSERT( minute <= PTIME_MINUTE_MAX );
  GF_ASSERT( second <= PTIME_SECOND_MAX );

  // プレイ時間を登録
  time = hour * 10000 + minute * 100 + second;
  RECORD_Set( record, RECID_CLEAR_TIME, time );
}

//-----------------------------------------------------------------------------
/**
 * @brief 殿堂入り時のレコードを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateDendouRecord( GAMECLEAR_MSG_WORK* work )
{
  SAVE_CONTROL_WORK* save;
  RECORD* record;

  save = GAMEDATA_GetSaveControlWork( work->gamedata );
  record = GAMEDATA_GetRecordPtr( work->gamedata );

  // データ上書きチェック
  if( SaveControl_IsOverwritingOtherData( save ) ) { return; }

  RECORD_Add( record, RECID_DENDOU_CNT, 1 );
}

//-----------------------------------------------------------------------------
/**
 * @brief 初「殿堂入り」の日時を記録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void SaveFirstDendouiriTime( GAMECLEAR_MSG_WORK* work )
{
  RECORD* record;

  record = GAMEDATA_GetRecordPtr( work->gamedata );

  // 「殿堂入り」がはじめての場合
  if( RECORD_Get( record, RECID_DENDOU_CNT ) == 0 ) {
    // 現在日時を記録
    EVTIME_SetGameClearDateTime( work->gamedata ); 
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief 殿堂入りデータの更新準備
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DendouSave_init( GAMECLEAR_MSG_WORK* work )
{
  RTCDate date;
  POKEPARTY* party;
  SAVE_CONTROL_WORK* save;
  DENDOU_SAVEDATA* dendouData;
  LOAD_RESULT result; 

  RTC_GetDate( &date );
  party = GAMEDATA_GetMyPokemon( work->gamedata ); 
  save = GAMEDATA_GetSaveControlWork( work->gamedata );
  result = SaveControl_Extra_Load( save, SAVE_EXTRA_ID_DENDOU, HEAPID_PROC );
  dendouData = SaveControl_Extra_DataPtrGet( save, SAVE_EXTRA_ID_DENDOU, EXGMDATA_ID_DENDOU );

  // データ上書きチェック
  if( SaveControl_IsOverwritingOtherData( save ) ) { return; }

  DendouData_AddRecord( dendouData, party, &date, HEAPID_PROC ); 
  GAMEDATA_ExtraSaveAsyncStart( work->gamedata, SAVE_EXTRA_ID_DENDOU ); 

  work->dendouSaveInitFlag = TRUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 殿堂入りデータの更新処理
 *
 * @param work 
 */
//-----------------------------------------------------------------------------
static void DendouSave_main( GAMECLEAR_MSG_WORK* work )
{
  SAVE_CONTROL_WORK* save;
  SAVE_RESULT save_ret;

  GF_ASSERT( work->dendouSaveInitFlag );

  // データ上書きチェック
  save = GAMEDATA_GetSaveControlWork( work->gamedata );
  if( SaveControl_IsOverwritingOtherData( save ) ) { return; }

  // セーブ実行
  save_ret = GAMEDATA_ExtraSaveAsyncMain( work->gamedata, SAVE_EXTRA_ID_DENDOU ); 

  // セーブ終了
  if( save_ret == SAVE_RESULT_OK || save_ret == SAVE_RESULT_NG ) { 
    SaveControl_Extra_Unload( save, SAVE_EXTRA_ID_DENDOU ); // 外部データを解放
    work->dendouSaveFinishFlag = TRUE;
  }
} 
