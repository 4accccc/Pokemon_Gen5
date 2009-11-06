//============================================================================================
/**
 *  @file config_panel.c
 *  @brief  コンフィグ画面
 *  @date 06.01.30
 *  @author Miyuki Iwasawa
 *
 *  2008.12.11  tamada  DP環境より移植
 *  2009.9,11  nagihashi WB仕様で作り変え
 */
//============================================================================================
#include <gflib.h>


#include "system/gfl_use.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menu.h"
#include "font/font.naix"


#include "app/config_panel.h"
#include "savedata/config.h"


#include "arc_def.h"
#include "system/main.h"

#include "arc/config_gra.naix"

#include "message.naix"
#include "msg/msg_config.h"
#include "system/wipe.h"

#include "./config_dummy.h"

//============================================================================================
//============================================================================================
#define CONFIG_DECIDE_END (1)
#define CONFIG_CANCEL_END (2)

#define FONT_TALK_PAL (12)
#define FONT_SYS_PAL  (13)

#define WINCLR_COL(col) (((col)<<4)|(col))

#define CONFIG_BMPCHAR_BASE (10)
//20
#define BMPL_TITLE_PX (1)
#define BMPL_TITLE_PY (0)
#define BMPL_TITLE_SX (12)
#define BMPL_TITLE_SY (2)
#define BMPL_TITLE_PAL  (FONT_SYS_PAL)
#define BMPL_TITLE_CGX  (CONFIG_BMPCHAR_BASE)
#define BMPL_TITLE_FRM  (GFL_BG_FRAME1_M)

//600
#define BMPL_MENU_PX  (1)
#define BMPL_MENU_PY  (3)
#define BMPL_MENU_SX  (30)
#define BMPL_MENU_SY  (14)
#define BMPL_MENU_PAL (FONT_SYS_PAL)
#define BMPL_MENU_CGX (BMPL_TITLE_CGX+BMPL_TITLE_SX*BMPL_TITLE_SY)
#define BMPL_MENU_FRM (GFL_BG_FRAME1_M)

//112
#define BMPL_SUM_PX (2)
#define BMPL_SUM_PY (19)
#define BMPL_SUM_SX (27)
#define BMPL_SUM_SY (4)
#define BMPL_SUM_PAL  (FONT_TALK_PAL)
#define BMPL_SUM_CGX  (BMPL_MENU_CGX+BMPL_MENU_SX*BMPL_MENU_SY)
#define BMPL_SUM_FRM  (GFL_BG_FRAME1_M)

#define BMPL_MENU_WIN_CGX (BMPL_SUM_CGX+BMPL_SUM_SX*BMPL_SUM_SY)
#define BMPL_TALK_WIN_CGX (BMPL_MENU_WIN_CGX+MENU_WIN_CGX_SIZ)
#define BMPL_MENU_WIN_PAL (14)
#define BMPL_TALK_WIN_PAL (15)

// YesNoWin
#define BMPL_YESNO_CX (25)
#define BMPL_YESNO_CY (13)
#define BMPL_YESNO_CSX  (6)
#define BMPL_YESNO_CSY  (4)
#define BMPL_YESNO_PAL  (FONT_SYS_PAL)
#define BMPL_YESNO_CGX  (BMPL_TALK_WIN_CGX+TALK_WIN_CGX_SIZ)
#define BMPL_YESNO_FRM  (GFL_BG_FRAME3_M)

enum{
  WIN_TITLE,
  WIN_MENU,
  WIN_SUM,
  WIN_MAX
};

#define MENU_SEL_MAX  (20)
#define MENU_LINE_MAX (7)

#define MENU_STRSIZ (10*2)
#define SUM_STRSIZ  (256)

#define MENU_SEL_OFSX (12*8+4)
#define MENU_SEL_OFSY (0)
#define MENU_SEL_DSIZ (48)
#define MENU_SEL_ASIZX  (48*8)
#define MENU_SEL_ASIZY  (16)
#define MENU_WIN_OFSY (24)

//ウィンドウのところを漢字にするか？
#define USE_KANJI_MODE (1)

enum{
 MENU_MSG_SPD,
 MENU_SOUND,
 MENU_BTL_EFF,
 MENU_BTL_RULE,
 MENU_BTN_MODE,
#if USE_KANJI_MODE
 MENU_MOJIMODE,
#else
 MENU_WIN,
#endif
 MENU_MAX,
};

typedef enum{
  CONFIG_SEQ_INITFADEIN,      // 画面の初期化とフェードイン
  CONFIG_SEQ_INITFADEINWAIT,    // フェードインまち
  CONFIG_SEQ_MAIN,        // メイン
  CONFIG_SEQ_SETCHECKINIT,    // 変更を反映させるかチェック　メッセージ表示
  CONFIG_SEQ_SETCHECK,      // 変更を反映させるかチェック　YESNOウィンドウ表示
  CONFIG_SEQ_SETWAIT,       // 変更を反映させるかチェック　YESNOセレクト
  CONFIG_SEQ_FADEOUT,       // フェードアウト
  CONFIG_SEQ_FADEOUTWAIT,     // フェードアウトまち
  CONFIG_SEQ_RELEASE,       // 画面の開放
}CONFIG_SEQUENCE;

//-----------------------------------------------------------------------------
//外部公開ワーク
//-----------------------------------------------------------------------------
typedef struct _CONFIG_PARAM
{
  u16 msg_spd:4;
  u16 sound:2;
  u16 btl_eff:1;
  u16 btl_rule:1;
  u16 input_mode:2;
  u16 win_type:5;
  u16 moji_mode:1;
}CONFIG_PARAM;

typedef struct _CONFIG_MENU{
  u16 num;
  u16 sel;
  STRBUF  *dat[MENU_SEL_MAX];
}CONFIG_MENU;

typedef struct _CONFIG_MAIN_DAT
{
  int heapID;
  CONFIG_SEQUENCE seq;
  int sub_seq;
  u32 end_f;
  u32 line;
  u32 side;
  u32 wincgx_f;

  CONFIG_PARAM  param;
  CONFIG        *save;

  ///メッセージリソース
  GFL_MSGDATA*  pMsg;

  void* pScrBuf;
  NNSG2dScreenData* pScr01;

  GFL_BMPWIN *  win[WIN_MAX]; ///<BMPウィンドウデータ
  CONFIG_MENU   menu[MENU_LINE_MAX];

  BMPMENU_WORK* pYesNoWork; ///<BMPYESNOウィンドウデータ
  u32 msg_no;

  GFL_TCB * vintr_tcb;
  GFL_FONT * hSysFont;
  GFL_FONT * hMsgFont;
}CONFIG_MAIN_DAT;


//================================================================
///プロトタイプエリア
//================================================================
///VBlank関連
static void ConfigVBlank(GFL_TCB * tcb, void * work);
///VRAMバンク定義
static void ConfigVramBankSet(void);

static int ConfigInitCommon(CONFIG_MAIN_DAT* wk);
static int ConfigReleaseCommon(CONFIG_MAIN_DAT* wk);

static void ConfigBGLInit(CONFIG_MAIN_DAT* wk);
static void ConfigBGLRelease(CONFIG_MAIN_DAT* wk);
static void Config2DGraInit(CONFIG_MAIN_DAT* wk);
static void Config2DGraRelease(CONFIG_MAIN_DAT* wk);
static void ConfigBmpWinInit(CONFIG_MAIN_DAT* wk);
static void ConfigBmpWinRelease(CONFIG_MAIN_DAT* wk);
static void ConfigBmpWinWriteFirst(CONFIG_MAIN_DAT* wk);
static void ConfigBmpWinWriteMenu(CONFIG_MAIN_DAT* wk,u16 idx);
static void ConfigBmpWinWriteSum( CONFIG_MAIN_DAT* wk,u16 line, BOOL allput );
static void ConfigBmpWinWriteMessege( CONFIG_MAIN_DAT* wk, u16 msg_idx, BOOL allput );
static BOOL ConfigBmpWinWriteMessegeEndCheck( const CONFIG_MAIN_DAT* cp_wk );
static void ConfigMenuStrGet(CONFIG_MAIN_DAT* wk);
static void ConfigKeyIn(CONFIG_MAIN_DAT* wk);
static BOOL ConfigDiffParam( CONFIG_MAIN_DAT* p_wk );
static void ConfigYesNoWinInit( CONFIG_MAIN_DAT* p_wk );
static u32 ConfigYesNoWinMain( CONFIG_MAIN_DAT* p_wk );

///オーバーレイプロセス
static GFL_PROC_RESULT ConfigProc_Init( GFL_PROC *proc,int *seq, void *pwk, void *work);
static GFL_PROC_RESULT ConfigProc_Main( GFL_PROC *proc,int *seq, void *pwk, void *work);
static GFL_PROC_RESULT ConfigProc_End( GFL_PROC *proc,int *seq, void *pwk, void *work);

//================================================================
///データ定義エリア
//================================================================
const GFL_PROC_DATA ConfigPanelProcData =
{
  ConfigProc_Init,
  ConfigProc_Main,
  ConfigProc_End
};

//============================================================================================
//
//
//        プログラムエリア
//
//
//============================================================================================
//----------------------------------------------------------------
/**
 *  @brief  コンフィグ プロセス初期化
 *  @param  proc  プロセスデータ
 *  @param  seq   シーケンス
 *
 *  @return 処理状況
 */
//----------------------------------------------------------------
GFL_PROC_RESULT ConfigProc_Init( GFL_PROC *proc,int *seq, void *pwk, void *work)
{
  CONFIG_MAIN_DAT *wk = NULL;
  CONFIG * sp;

  sp = (CONFIG*)pwk;

  //ヒープ作成
  GFL_HEAP_CreateHeap(GFL_HEAPID_APP,HEAPID_CONFIG,0x10000 + 0x8000);

  wk = GFL_PROC_AllocWork( proc,sizeof(CONFIG_MAIN_DAT),HEAPID_CONFIG);
  GFL_STD_MemFill(wk,0,sizeof(CONFIG_MAIN_DAT));


  wk->heapID = HEAPID_CONFIG;
  if (sp == NULL) {
    //とりあえず
    sp = CONFIG_AllocWork(wk->heapID);
    CONFIG_Init(sp);
  }

  wk->save = sp;
  //パラメータ引継ぎ
  wk->param.msg_spd   = CONFIG_GetMsgSpeed(sp);
  wk->param.btl_eff   = CONFIG_GetWazaEffectMode(sp);
  wk->param.btl_rule    = CONFIG_GetBattleRule(sp);
  wk->param.sound     = CONFIG_GetSoundMode(sp);
  wk->param.input_mode  = CONFIG_GetInputMode(sp);
  wk->param.win_type    = CONFIG_GetWindowType(sp);
  wk->param.moji_mode    = CONFIG_GetMojiMode(sp);


  // メッセージ表示関係を設定
  MsgPrintSkipFlagSet(MSG_SKIP_OFF);  // スキップできない

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ プロセス終了
 *  @param  proc  プロセスデータ
 *  @param  seq   シーケンス
 *
 *  @return 処理状況
 */
//----------------------------------------------------------------
GFL_PROC_RESULT ConfigProc_End( GFL_PROC *proc,int *seq, void *pwk, void *work)
{
  CONFIG_MAIN_DAT* wk = work;

  //セーブパラメータ書き出し
  if(wk->end_f == CONFIG_DECIDE_END){
    wk->param.msg_spd = wk->menu[MENU_MSG_SPD].sel;
    wk->param.btl_eff = wk->menu[MENU_BTL_EFF].sel;
    wk->param.btl_rule = wk->menu[MENU_BTL_RULE].sel;
    wk->param.sound  = wk->menu[MENU_SOUND].sel;
    wk->param.input_mode   = wk->menu[MENU_BTN_MODE].sel;
#if USE_KANJI_MODE
    wk->param.moji_mode   = wk->menu[MENU_MOJIMODE].sel;
#else
    wk->param.win_type   = wk->menu[MENU_WIN].sel;
#endif
  }

  // 現在の設定を反映
  // コンフィグ内で設定された値も元に戻る
  CONFIG_SetMsgSpeed(wk->save,wk->param.msg_spd);
  CONFIG_SetWazaEffectMode(wk->save,wk->param.btl_eff);
  CONFIG_SetBattleRule(wk->save,wk->param.btl_rule);
  CONFIG_SetSoundMode(wk->save,wk->param.sound);
  CONFIG_SetInputMode(wk->save,wk->param.input_mode);
  CONFIG_SetWindowType(wk->save,wk->param.win_type);
  CONFIG_SetMojiMode(wk->save,wk->param.moji_mode);
  Snd_SetMonoFlag(wk->param.sound);
  CONFIG_SetKeyConfig(wk->param.input_mode);

  GFL_MSGSYS_SetLangID( wk->param.moji_mode );

  // メッセージ表示関係を設定
  MsgPrintSkipFlagSet(MSG_SKIP_ON); // スキップできる

  if (pwk == NULL) {
    GFL_HEAP_FreeMemory(wk->save);
  }
  //ワークエリア解放
  GFL_PROC_FreeWork(proc);


  GFL_HEAP_DeleteHeap(wk->heapID);

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ プロセスメイン
 *  @param  proc  プロセスデータ
 *  @param  seq   シーケンス
 *
 *  @return 処理状況
 */
//----------------------------------------------------------------
GFL_PROC_RESULT ConfigProc_Main( GFL_PROC *proc,int *seq, void *pwk, void *work)
{
  CONFIG_MAIN_DAT* wk = work;
  BOOL result;
  u32 yesno_res;

  switch((CONFIG_SEQUENCE)wk->seq){
  case CONFIG_SEQ_INITFADEIN:
    if(!ConfigInitCommon(wk)){
      return GFL_PROC_RES_CONTINUE;
    }
    WIPE_SYS_Start(WIPE_PATTERN_M,
        WIPE_TYPE_FADEIN,
        WIPE_TYPE_FADEIN,0x0000,COMM_BRIGHTNESS_SYNC,1,wk->heapID
        );
    break;
  case CONFIG_SEQ_INITFADEINWAIT:
    if(!WIPE_SYS_EndCheck()){
      return GFL_PROC_RES_CONTINUE;
    }
    break;
  case CONFIG_SEQ_MAIN:
#if 0
    //メイン
    if( ((GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE) && (wk->line == (MENU_LINE_MAX-1))) ||
      (GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL) ){

      result = ConfigDiffParam( wk );

      // 変更があったかチェック
      if( result == TRUE ){
        wk->seq = CONFIG_SEQ_SETCHECKINIT;
      }else{
        Snd_PlaySE(SEQ_SE_DP_SELECT);
        wk->end_f = CONFIG_CANCEL_END;
        wk->seq = CONFIG_SEQ_FADEOUT;
      }
      return GFL_PROC_RES_CONTINUE;
    }
    ConfigKeyIn(wk);
#endif
    //GSで決定後のはい・いいえが出なかったのでそっちにあわせる
    if((GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE) && (wk->line == (MENU_LINE_MAX-1)))
    {
      result = ConfigDiffParam( wk );

      // 変更があったかチェック
      if( result == TRUE )
      {
        Snd_PlaySE(SEQ_SE_DP_SAVE);
        wk->end_f = CONFIG_DECIDE_END;
      }
      else
      {
        Snd_PlaySE(SEQ_SE_DP_SELECT);
        wk->end_f = CONFIG_CANCEL_END;
      }
      wk->seq = CONFIG_SEQ_FADEOUT;
    }
    else
    if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL)
    {
      Snd_PlaySE(SEQ_SE_DP_SELECT);
      wk->end_f = CONFIG_CANCEL_END;
      wk->seq = CONFIG_SEQ_FADEOUT;
    }
    ConfigKeyIn(wk);

    return GFL_PROC_RES_CONTINUE;

  case CONFIG_SEQ_SETCHECKINIT:
    // 反映させるか聞く
    MsgPrintSkipFlagSet(MSG_SKIP_ON); // スキップできる
    ConfigBmpWinWriteMessege( wk, mes_config_comment07, FALSE );
    break;

  case CONFIG_SEQ_SETCHECK:
    // YESNOを出す
    if( ConfigBmpWinWriteMessegeEndCheck( wk ) ){
      MsgPrintSkipFlagSet(MSG_SKIP_OFF);  // スキップできる
      ConfigYesNoWinInit( wk );
      wk->seq = CONFIG_SEQ_SETWAIT;
    }
    return GFL_PROC_RES_CONTINUE;

  case CONFIG_SEQ_SETWAIT:
    // 選択してもらう
    yesno_res = ConfigYesNoWinMain( wk );
    if( yesno_res != BMPMENU_NULL )
    {
      if( yesno_res == 0 ) // セーブするなら
      {
        Snd_SeStopBySeqNo(SEQ_SE_DP_SELECT,0);    //同時に鳴るのを回避する
        Snd_PlaySE(SEQ_SE_DP_SAVE);
        wk->end_f = CONFIG_DECIDE_END;
      }
      else
      {
        wk->end_f = CONFIG_CANCEL_END;
      }
      wk->seq = CONFIG_SEQ_FADEOUT;
    }
    return GFL_PROC_RES_CONTINUE;

  case CONFIG_SEQ_FADEOUT:
#if 0
    //add pl MSGフリーズ対策 メッセージ表示タスク強制停止 080701 kaga
    if( GF_MSG_PrintEndCheck(wk->msg_no) ){
      GF_STR_PrintForceStop( wk->msg_no );
    }
#endif

    WIPE_SYS_Start(WIPE_PATTERN_M,
        WIPE_TYPE_FADEOUT,
        WIPE_TYPE_FADEOUT,0x0000,COMM_BRIGHTNESS_SYNC,1,wk->heapID
        );
    break;
  case CONFIG_SEQ_FADEOUTWAIT:
    if(!WIPE_SYS_EndCheck()){
      return GFL_PROC_RES_CONTINUE;
    }
    break;
  case CONFIG_SEQ_RELEASE:
    //終了処理
    if(!ConfigReleaseCommon(wk)){
      return GFL_PROC_RES_CONTINUE;
    }
    return GFL_PROC_RES_FINISH;
  }
  ++wk->seq;
  return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面VramBankセット
 */
//----------------------------------------------------------------
static void ConfigVramBankSet(void)
{
  GFL_DISP_VRAM vramSetTable = {
    GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_16_G,       // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,       // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_32K,   // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
  };

  GFL_DISP_SetBank( &vramSetTable );
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面VBlank
 */
//----------------------------------------------------------------
static void ConfigVBlank(GFL_TCB * tcb, void * work)
{
  CONFIG_MAIN_DAT* wk = work;

#if 0
  //会話ウィンドウキャラクタの再転送
  if(wk->wincgx_f){
    TalkWinGraphicSet(wk->bgl,BMPL_SUM_FRM,
      BMPL_TALK_WIN_CGX, BMPL_TALK_WIN_PAL, wk->menu[MENU_WIN].sel, wk->heapID);

    wk->wincgx_f = 0;
  }
  CATS_RenderOamTrans();
  NNS_GfdDoVramTransfer();  //VRam転送マネージャ実行
  GF_BGL_VBlankFunc( wk->bgl );
  OS_SetIrqCheckFlag( OS_IE_V_BLANK);
#endif
}

//----------------------------------------------------------------
/**
 *  @brief  各種初期化処理
 */
//----------------------------------------------------------------
static int ConfigInitCommon(CONFIG_MAIN_DAT *wk)
{
  switch(wk->sub_seq){
  case 0:

    GFL_DISP_GX_InitVisibleControl();
    GFL_DISP_GXS_InitVisibleControl();

    //Bankセット
    ConfigVramBankSet();

    GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

    WIPE_ResetWndMask(WIPE_DISP_MAIN);
    WIPE_ResetWndMask(WIPE_DISP_SUB);

    //BGLセット
    ConfigBGLInit(wk);
    break;
  case 1:
    //2Dリソース取得
    Config2DGraInit(wk);

    wk->hSysFont = GFL_FONT_Create(ARCID_FONT, NARC_font_small_gftr,
        GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID);
    wk->hMsgFont = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
        GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID);
    //メッセージリソース取得
    wk->pMsg = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL,ARCID_MESSAGE,
        NARC_message_config_dat,wk->heapID);
    ConfigMenuStrGet(wk);
    break;
  case 2:
    ConfigBmpWinInit(wk);
    ConfigBmpWinWriteFirst(wk);
    //通信アイコン用にOBJ面ON
    GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);

    GFL_BG_LoadScreenReq(GFL_BG_FRAME1_M);
    // 受信強度アイコンを通信接続中なら表示するし、
    // ユニオンのように通信回路は動作しているが接続はしていない状態なら出さない
    // Change by Mori 2006/07/19
    //WirelessIconEasyUnion();

    wk->vintr_tcb = GFUser_VIntr_CreateTCB(ConfigVBlank, wk, 0 );
    wk->sub_seq = 0;
    return 1;

  }
  ++wk->sub_seq;
  return 0;
}

//----------------------------------------------------------------
/**
 *  @brief  各種終了処理
 */
//----------------------------------------------------------------
static int ConfigReleaseCommon(CONFIG_MAIN_DAT *wk)
{
  int i = 0,j = 0;

  switch(wk->sub_seq){
  case 0:
    //Bmpウィンドウ破棄
    ConfigBmpWinRelease(wk);
    //メッセージリソース解放
    for(i = 0;i < MENU_LINE_MAX;i++){
      for(j = 0;j < wk->menu[i].num;j++){
        GFL_STR_DeleteBuffer(wk->menu[i].dat[j]);
      }
    }
    GFL_FONT_Delete(wk->hSysFont);
    GFL_FONT_Delete(wk->hMsgFont);
    GFL_MSG_Delete(wk->pMsg);
    //2Dリソース解放
    Config2DGraRelease(wk);
    ConfigBGLRelease(wk);
    break;
  case 1:
    GFL_TCB_DeleteTask(wk->vintr_tcb);

    GFL_DISP_GX_InitVisibleControl();
    GFL_DISP_GXS_InitVisibleControl();
    GX_SetVisiblePlane(0);
    GXS_SetVisiblePlane(0);
    wk->sub_seq = 0;
    return 1;
  }
  ++wk->sub_seq;
  return 0;
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面BGLシステム初期化
 */
//----------------------------------------------------------------
static void ConfigBGLInit(CONFIG_MAIN_DAT* wk)
{
  int i;

  GFL_BG_Init(wk->heapID);

  { //BG SYSTEM
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
    };
    GFL_BG_SetBGMode(&BGsys_data);
  }

  {
  GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
    { //MAIN BG0  選択枠線
      0,0,0x1000,0,GFL_BG_SCRSIZ_256x512,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800,GX_BG_CHARBASE_0x00000,0x4000,GX_BG_EXTPLTT_01,
      1,0,0,FALSE},
    { //MAIN BG1  テキスト表示プレーン
      0,0,0x1000,0,GFL_BG_SCRSIZ_256x512,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000,GX_BG_CHARBASE_0x04000,0x8000,GX_BG_EXTPLTT_01,
      2,0,0,FALSE},
    { //MAIN BG2
      0,0,0x1000,0,GFL_BG_SCRSIZ_256x512,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800,GX_BG_CHARBASE_0x00000,0x4000,GX_BG_EXTPLTT_01,
      3,0,0,FALSE},
    { //MAIN BG3  「はい・いいえ」表示用プレーン
      0,0,0x1000,0,GFL_BG_SCRSIZ_256x512,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000,GX_BG_CHARBASE_0x00000,0x4000,GX_BG_EXTPLTT_01,
      0,0,0,FALSE},
    { //SUB BG0
      0,0,0x1000,0,GFL_BG_SCRSIZ_256x512,GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800,GX_BG_CHARBASE_0x00000,0x4000,GX_BG_EXTPLTT_01,
      0,0,0,FALSE},
  };
  for(i = 0;i < 5;i++){
    static const int frame[5] = {
      GFL_BG_FRAME0_M,GFL_BG_FRAME1_M,GFL_BG_FRAME2_M,GFL_BG_FRAME3_M,GFL_BG_FRAME0_S
    };
    GFL_BG_SetBGControl(frame[i],&(TextBgCntDat[i]),GFL_BG_MODE_TEXT);
    GFL_BG_ClearScreen(frame[i]);
    GFL_BG_SetVisible(frame[i], VISIBLE_ON);
  }
  }
  GFL_BG_SetClearCharacter(GFL_BG_FRAME0_M,0x20,0x0000,wk->heapID);
  GFL_BG_SetClearCharacter(GFL_BG_FRAME0_S,0x20,0x0000,wk->heapID);
  GFL_BG_SetClearCharacter(GFL_BG_FRAME0_M,0x20,0x4000,wk->heapID);
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面BGLシステム破棄
 */
//----------------------------------------------------------------
static void ConfigBGLRelease(CONFIG_MAIN_DAT* wk)
{
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
  GFL_BG_FreeBGControl(GFL_BG_FRAME3_M);
  GFL_BG_FreeBGControl(GFL_BG_FRAME2_M);
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_M);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_M);

  GFL_BG_Exit();
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面 2Dリソース取得
 */
//----------------------------------------------------------------
static void Config2DGraInit(CONFIG_MAIN_DAT* wk)
{
  void* tmp;
  u32 size;
  ARCHANDLE* handle;
  void* pSrc;
  NNSG2dCharacterData* pChar;
  NNSG2dPaletteData*  pPal;

  handle = GFL_ARC_OpenDataHandle(ARCID_CONFIG_GRA,wk->heapID);

  //キャラクタ転送
  size = GFL_ARC_GetDataSizeByHandle(handle,NARC_config_gra_config01_ncgr);
  pSrc = GFL_HEAP_AllocMemoryLo(wk->heapID,size);
  GFL_ARC_LoadDataByHandle(handle,NARC_config_gra_config01_ncgr,(void*)pSrc);

  NNS_G2dGetUnpackedCharacterData(pSrc,&pChar);
  GFL_BG_LoadCharacter(GFL_BG_FRAME0_M,pChar->pRawData,pChar->szByte,0);
  GFL_BG_LoadCharacter(GFL_BG_FRAME0_S,pChar->pRawData,pChar->szByte,0);
  GFL_HEAP_FreeMemory(pSrc);

  //パレット転送
  size = GFL_ARC_GetDataSizeByHandle(handle,NARC_config_gra_config01_nclr);
  pSrc = GFL_HEAP_AllocMemoryLo(wk->heapID,size);
  GFL_ARC_LoadDataByHandle(handle,NARC_config_gra_config01_nclr,(void*)pSrc);

  NNS_G2dGetUnpackedPaletteData(pSrc,&pPal);
  GFL_BG_LoadPalette(GFL_BG_FRAME0_M,pPal->pRawData,0x20*1,0);
  GFL_BG_LoadPalette(GFL_BG_FRAME0_S,pPal->pRawData,0x20*1,0);
  GFL_HEAP_FreeMemory(pSrc);

  //スクリーン取得
  size = GFL_ARC_GetDataSizeByHandle(handle,NARC_config_gra_config01_nscr);
  wk->pScrBuf = GFL_HEAP_AllocMemory(wk->heapID,size);
  GFL_ARC_LoadDataByHandle(handle,NARC_config_gra_config01_nscr,(void*)wk->pScrBuf);
  NNS_G2dGetUnpackedScreenData(wk->pScrBuf,&(wk->pScr01));

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_FillScreen(GFL_BG_FRAME2_M,0x0001,0,0,32,32,GFL_BG_SCRWRT_PALIN );
  GFL_BG_FillScreen(GFL_BG_FRAME0_S,0x0001,0,0,32,32,GFL_BG_SCRWRT_PALIN );

  //BG描画
  GFL_BG_WriteScreenFree(GFL_BG_FRAME0_M,
      0,0,32,2,
      wk->pScr01->rawData,
      0,0,
      wk->pScr01->screenWidth/8,wk->pScr01->screenHeight/8);
  GFL_BG_SetScroll(GFL_BG_FRAME0_M,GFL_BG_SCROLL_Y_SET,-MENU_WIN_OFSY);

  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME2_M);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME0_M);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME0_S);
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面 2Dリソース解放
 */
//----------------------------------------------------------------
static void Config2DGraRelease(CONFIG_MAIN_DAT* wk)
{
  GFL_HEAP_FreeMemory(wk->pScrBuf);
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面 BMPウィンドウ初期化
 */
//----------------------------------------------------------------
static void ConfigBmpWinInit(CONFIG_MAIN_DAT* wk)
{
  GFL_BMPWIN_Init(wk->heapID);

  wk->win[WIN_TITLE] = GFL_BMPWIN_Create(BMPL_TITLE_FRM,
    BMPL_TITLE_PX, BMPL_TITLE_PY,
    BMPL_TITLE_SX, BMPL_TITLE_SY, BMPL_TITLE_PAL, GFL_BMP_CHRAREA_GET_F );

  wk->win[WIN_MENU] = GFL_BMPWIN_Create(BMPL_MENU_FRM,
    BMPL_MENU_PX, BMPL_MENU_PY,
    BMPL_MENU_SX, BMPL_MENU_SY, BMPL_MENU_PAL, GFL_BMP_CHRAREA_GET_F );

  wk->win[WIN_SUM] = GFL_BMPWIN_Create(BMPL_SUM_FRM,
    BMPL_SUM_PX, BMPL_SUM_PY,
    BMPL_SUM_SX, BMPL_SUM_SY, BMPL_SUM_PAL, GFL_BMP_CHRAREA_GET_F );

  //ウィンドウグラフィックセット
  BmpWinFrame_CgxSet(BMPL_MENU_FRM,
      BMPL_MENU_WIN_CGX, MENU_TYPE_SYSTEM, wk->heapID);
  BmpWinFrame_CgxSet(BMPL_MENU_FRM,
      BMPL_TALK_WIN_CGX, MENU_TYPE_SYSTEM, wk->heapID);

  //フォント用パレットセット
  //フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
    0x20*FONT_SYS_PAL, 0x20, HEAPID_CONFIG);
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
    0x20*FONT_SYS_PAL, 0x20, HEAPID_CONFIG);
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
    0x20*FONT_TALK_PAL, 0x20, HEAPID_CONFIG);
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
    0x20*FONT_TALK_PAL, 0x20, HEAPID_CONFIG);


  //BMPウィンドウクリア
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win[WIN_TITLE]), WINCLR_COL(FBMP_COL_NULL));
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win[WIN_MENU]), WINCLR_COL(FBMP_COL_WHITE));
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win[WIN_SUM]), WINCLR_COL(FBMP_COL_WHITE));
  GFL_BMPWIN_ClearScreen(wk->win[WIN_SUM]);
  GFL_BMPWIN_ClearScreen(wk->win[WIN_MENU]);
  GFL_BMPWIN_ClearScreen(wk->win[WIN_TITLE]);
  GFL_BMPWIN_MakeScreen(wk->win[WIN_SUM]);
  GFL_BMPWIN_MakeScreen(wk->win[WIN_MENU]);
  GFL_BMPWIN_MakeScreen(wk->win[WIN_TITLE]);

  //ウィンドウ描画
  BmpWinFrame_Write( wk->win[WIN_MENU], WINDOW_TRANS_ON, BMPL_MENU_WIN_CGX, BMPL_MENU_WIN_PAL );
  BmpWinFrame_Write( wk->win[WIN_SUM], WINDOW_TRANS_ON, BMPL_TALK_WIN_CGX, BMPL_TALK_WIN_PAL);
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面 BMPウィンドウ解放
 */
//----------------------------------------------------------------
static void ConfigBmpWinRelease(CONFIG_MAIN_DAT* wk)
{
  u16 i;

  //ウィンドウクリア
  BmpWinFrame_Clear(wk->win[WIN_MENU], WINDOW_TRANS_ON);
  BmpWinFrame_Clear(wk->win[WIN_SUM], WINDOW_TRANS_ON);

  for(i = 0;i < WIN_MAX;i++){
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win[i]), 0);
    GFL_BMPWIN_ClearScreen(wk->win[i]);
    GFL_BMPWIN_Delete(wk->win[i]);
  }
  GFL_BMPWIN_Exit();
}

/**
 *  @brief  コンフィグ画面 BMPウィンドウ初期描画
 */
static void ConfigBmpWinWriteFirst(CONFIG_MAIN_DAT* wk)
{
  u32 siz,ofs;
  u16 i;
  STRBUF  *buf;
  STRBUF  *sum;
  static const u8 menu_str[ MENU_LINE_MAX ] = {
    mes_config_menu01,
    mes_config_menu04,
    mes_config_menu02,
    mes_config_menu03,
    mes_config_menu05,
#if USE_KANJI_MODE
    mes_config_menu09,
#else
    mes_config_menu06,
#endif
    mes_config_menu08,
  };
  GFL_BMP_DATA * title_bmp;

  title_bmp = GFL_BMPWIN_GetBmp(wk->win[WIN_TITLE]);

  buf = GFL_STR_CreateBuffer(SUM_STRSIZ,wk->heapID);

  //せっていをかえる
  GFL_MSG_GetString(wk->pMsg,mes_config_title,buf);

  ofs = 2;
  GFL_FONTSYS_SetColor(FBMP_COL_BLACK,FBMP_COL_BLK_SDW,FBMP_COL_NULL);
  PRINTSYS_Print( title_bmp, ofs, 2, buf, wk->hSysFont);

  //メニュー
  ofs = 4;
  for(i = 0;i < MENU_LINE_MAX;i++){
    GFL_STR_ClearBuffer(buf);
    GFL_MSG_GetString(wk->pMsg,menu_str[i],buf);

    GFL_FONTSYS_SetDefaultColor();
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win[WIN_MENU]), ofs, 16*i, buf, wk->hSysFont);
  }
  for(i = 0;i < MENU_LINE_MAX;i++){
    ConfigBmpWinWriteMenu(wk,i);
  }

  //サムネイル
  ConfigBmpWinWriteSum( wk, 0, TRUE );

  GFL_BMPWIN_TransVramCharacter(wk->win[WIN_TITLE]);
  GFL_BMPWIN_TransVramCharacter(wk->win[WIN_MENU]);

  GFL_STR_DeleteBuffer(buf);
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面 セレクトメニュー文字列取得
 */
//----------------------------------------------------------------
static void ConfigMenuStrGet(CONFIG_MAIN_DAT* wk)
{
  u16 i,j;
  static const selSiz[MENU_LINE_MAX] = {
    MSGSPEED_MAX,
    SOUNDMODE_MAX,
    WAZAEFF_MODE_MAX,
    BATTLERULE_MAX,
    INPUTMODE_MAX,
#if USE_KANJI_MODE
    MOJIMODE_MAX,
#else
    WINTYPE_MAX,
#endif
    0
  };
  static const u8 sc_selMsg_start[MENU_LINE_MAX] = {
    mes_config_sm01_00,
    mes_config_sm04_00,
    mes_config_sm02_00,
    mes_config_sm03_00,
    mes_config_sm05_00,
#if USE_KANJI_MODE
    mes_config_sm07_01,
#else
    mes_config_sm06_00,
#endif
    0,
  };

  for(i = 0;i < MENU_LINE_MAX;i++){
    wk->menu[i].num = selSiz[i];
    for(j = 0;j < selSiz[i];j++){
      wk->menu[i].dat[j] = GFL_MSG_CreateString(wk->pMsg,sc_selMsg_start[i]+j);
    }
  }
  //現在の選択ナンバー取得
  wk->menu[MENU_MSG_SPD].sel = wk->param.msg_spd;
  wk->menu[MENU_BTL_EFF].sel = wk->param.btl_eff;
  wk->menu[MENU_BTL_RULE].sel = wk->param.btl_rule;
  wk->menu[MENU_SOUND].sel = wk->param.sound;
  wk->menu[MENU_BTN_MODE].sel = wk->param.input_mode;
#if USE_KANJI_MODE
  wk->menu[MENU_MOJIMODE].sel = wk->param.moji_mode;
#else
  wk->menu[MENU_WIN].sel = wk->param.win_type;
#endif
}

//----------------------------------------------------------------
/**
 *  @brief  コンフィグ画面 セレクトメニュー描画
 */
//----------------------------------------------------------------
static void ConfigBmpWinWriteMenu(CONFIG_MAIN_DAT* wk,u16 idx)
{
  u16 i;
  s8  x_ofs = 0;
  static const s8 ofs[] = { 0,0,0,0,0,0,0};

  //一旦クリア
  GFL_BMP_Fill(GFL_BMPWIN_GetBmp(wk->win[WIN_MENU]),
    MENU_SEL_OFSX+ofs[idx], MENU_SEL_OFSY+idx*MENU_SEL_ASIZY,MENU_SEL_ASIZX,MENU_SEL_ASIZY,
    WINCLR_COL(FBMP_COL_WHITE));

  //せっていをかえる
#if USE_KANJI_MODE
#else
  if(idx == MENU_WIN){
    GFL_FONTSYS_SetColor(FBMP_COL_RED,FBMP_COL_RED_SDW,FBMP_COL_WHITE);
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win[WIN_MENU]),
        1*MENU_SEL_DSIZ+MENU_SEL_OFSX,
        MENU_SEL_ASIZY*idx+MENU_SEL_OFSY,
        wk->menu[idx].dat[wk->menu[idx].sel], wk->hSysFont);
    GFL_BMPWIN_TransVramCharacter(wk->win[WIN_MENU]);

    //ウィンドウキャラクタ再送フラグOn
    wk->wincgx_f = 1;
    return;
  }
#endif

  //サウンドモードの場合、ステレオとモノラルを切り替える
  if(idx == MENU_SOUND){
    Snd_SetMonoFlag(wk->menu[idx].sel);
  }
  // ボタンモード反映
  else if(idx == MENU_BTN_MODE){
    CONFIG_SetKeyConfig(wk->menu[idx].sel);
  }
  // メッセージスピード反映
  else if(idx == MENU_MSG_SPD ){
    CONFIG_SetMsgSpeed( wk->save, wk->menu[idx].sel );

    //サムネイルを再描画
    ConfigBmpWinWriteSum( wk, idx, FALSE );
  }
  x_ofs = 0;
  for(i = 0;i < wk->menu[idx].num;i++){
    if(i == wk->menu[idx].sel){
      GFL_FONTSYS_SetColor(FBMP_COL_RED,FBMP_COL_RED_SDW,FBMP_COL_WHITE);
    }else{
      GFL_FONTSYS_SetColor(FBMP_COL_BLACK,FBMP_COL_BLK_SDW,FBMP_COL_WHITE);
    }

    if(idx == MENU_BTN_MODE){
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win[WIN_MENU]),
              MENU_SEL_OFSX-0+x_ofs,
              MENU_SEL_ASIZY*idx+MENU_SEL_OFSY,
              wk->menu[idx].dat[i],
              wk->hSysFont);

      x_ofs += PRINTSYS_GetStrWidth(wk->menu[idx].dat[i], wk->hSysFont, 0) + 12;
    }else{
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win[WIN_MENU]),
              i*MENU_SEL_DSIZ+MENU_SEL_OFSX+ofs[idx],
              MENU_SEL_ASIZY*idx+MENU_SEL_OFSY,
              wk->menu[idx].dat[i],
              wk->hSysFont);
    }
  }
  //GF_BGL_BmpWinOn(wk->win[WIN_MENU]);
  GFL_BG_LoadScreenReq(GFL_BG_FRAME1_M);
  GFL_BMPWIN_TransVramCharacter(wk->win[WIN_MENU]);
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージの表示
 *
 *  @param  wk    ワーク
 *  @param  msg_idx メッセージID
 *  @param  allput  一気に表示するか
 */
//-----------------------------------------------------------------------------
static void ConfigBmpWinWriteMessege( CONFIG_MAIN_DAT* wk, u16 msg_idx, BOOL allput )
{
  STRBUF  *buf;
  u8 msg_speed;

  // 話途中のときは削除する
  if( ConfigBmpWinWriteMessegeEndCheck( wk ) == FALSE ){
    // 強制終了
    //GF_STR_PrintForceStop( wk->msg_no );
  }

  // 現在のメッセージスピード設定
  msg_speed = MSGSPEED_GetWait();

  // BMPのクリーン
  //GF_BGL_BmpWinDataFill( &(wk->win[WIN_SUM]), FBMP_COL_WHITE);
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win[WIN_SUM]), FBMP_COL_WHITE);

  // カラー作成
  GFL_FONTSYS_SetColor(FBMP_COL_BLACK,FBMP_COL_BLK_SDW,FBMP_COL_WHITE);

  // STRBUF作成
  buf = GFL_STR_CreateBuffer(SUM_STRSIZ,wk->heapID);

  // 変更したメッセージスピードで書き込み
  GFL_MSG_GetString(wk->pMsg,msg_idx,buf);

  if( allput == FALSE ){
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win[WIN_SUM]),
        4,0, buf, wk->hMsgFont);
#if 0
    wk->msg_no = GF_STR_PrintColor( wk->win[WIN_SUM],FONT_TALK,buf,
              4,0,
              msg_speed,fcol,NULL );
#endif
  }else{
    PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->win[WIN_SUM]),
        4,0, buf, wk->hMsgFont);
#if 0
    GF_STR_PrintColor(  wk->win[WIN_SUM],FONT_TALK,buf,
              4,0,
              MSG_NO_PUT,fcol,NULL );
#endif
    //GF_BGL_BmpWinOnVReq( wk->win[WIN_SUM] );
    GFL_BMPWIN_TransVramCharacter( wk->win[WIN_SUM] );
  }

  // STRBUF破棄
  GFL_STR_DeleteBuffer( buf );
}

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージ終了待ち
 *
 *  @param  cp_wk ワーク
 *
 *  @retval TRUE  メッセージ終了
 *  @retval FALSE メッセージ途中
 */
//-----------------------------------------------------------------------------
static BOOL ConfigBmpWinWriteMessegeEndCheck( const CONFIG_MAIN_DAT* cp_wk )
{
  return TRUE;
#if 0
  if( GF_MSG_PrintEndCheck( cp_wk->msg_no ) == FALSE ){
    return TRUE;
  }
  return FALSE;
#endif
}

//-----------------------------------------------------------------------------
/**
 *  @brief  設定画面キー取得
 */
//-----------------------------------------------------------------------------
static void ConfigKeyIn(CONFIG_MAIN_DAT* wk)
{
  CONFIG_MENU *mp;

  mp = &(wk->menu[wk->line]);
  if(wk->line != MENU_MAX){ //決定ラインの時は左右キー入力無効
    if(GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT){
      mp->sel = (mp->sel+1)%mp->num;
      ConfigBmpWinWriteMenu(wk,wk->line);
      Snd_PlaySE(SEQ_SE_DP_SELECT);
    }else if(GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT){
      mp->sel = (mp->sel+mp->num-1)%mp->num;
      ConfigBmpWinWriteMenu(wk,wk->line);
      Snd_PlaySE(SEQ_SE_DP_SELECT);
    }
  }
  if(GFL_UI_KEY_GetTrg() & PAD_KEY_UP){
    wk->line = (wk->line+MENU_LINE_MAX-1)%MENU_LINE_MAX;
    GFL_BG_SetScrollReq(GFL_BG_FRAME0_M,GFL_BG_SCROLL_Y_SET,
        -(wk->line*MENU_SEL_ASIZY+MENU_WIN_OFSY));

    //サムネイル
    ConfigBmpWinWriteSum( wk, wk->line, TRUE );
    Snd_PlaySE(SEQ_SE_DP_SELECT);
  }else if(GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN){
    wk->line = (wk->line+1)%MENU_LINE_MAX;
    GFL_BG_SetScrollReq(GFL_BG_FRAME0_M,GFL_BG_SCROLL_Y_SET,
        -(wk->line*MENU_SEL_ASIZY+MENU_WIN_OFSY));

    //サムネイル
    ConfigBmpWinWriteSum( wk, wk->line, TRUE );
    Snd_PlaySE(SEQ_SE_DP_SELECT);
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  設定パラメータに変更があるかチェック
 *
 *  @param  p_wk  ワーク
 *
 *  @retval TRUE  設定パラメータに変更あり
 *  @retval FALSE 設定パラメータに変更なし
 */
//-----------------------------------------------------------------------------
static BOOL ConfigDiffParam( CONFIG_MAIN_DAT* p_wk )
{
  if( (p_wk->param.msg_spd  !=  p_wk->menu[MENU_MSG_SPD].sel) ||
    (p_wk->param.btl_eff  !=  p_wk->menu[MENU_BTL_EFF].sel) ||
    (p_wk->param.btl_rule !=  p_wk->menu[MENU_BTL_RULE].sel) ||
    (p_wk->param.sound    !=  p_wk->menu[MENU_SOUND].sel) ||
    (p_wk->param.input_mode !=  p_wk->menu[MENU_BTN_MODE].sel) ||
#if USE_KANJI_MODE
    (p_wk->param.moji_mode !=  p_wk->menu[MENU_MOJIMODE].sel) )
#else
    (p_wk->param.win_type !=  p_wk->menu[MENU_WIN].sel) )
#endif
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  YESNOウィンドウの初期化
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void ConfigYesNoWinInit( CONFIG_MAIN_DAT* p_wk )
{
#if 0
  static const BMPWIN_DAT c_data = {
    BMPL_YESNO_FRM,
    BMPL_YESNO_CX, BMPL_YESNO_CY,
    BMPL_YESNO_CSX, BMPL_YESNO_CSY,
    BMPL_YESNO_PAL, BMPL_YESNO_CGX
  };
  p_wk->pYesNoWork = BmpYesNoSelectInit( p_wk->bgl, &c_data,
      BMPL_MENU_WIN_CGX, BMPL_MENU_WIN_PAL, p_wk->heapID );
#endif
  static const BMPWIN_YESNO_DAT c_data =
  {
    BMPL_YESNO_FRM ,
    BMPL_YESNO_CX , BMPL_YESNO_CY ,
    BMPL_YESNO_PAL, BMPL_YESNO_CGX ,
  };
  p_wk->pYesNoWork = BmpMenu_YesNoSelectInit( &c_data,
       BMPL_MENU_WIN_CGX, BMPL_MENU_WIN_PAL,0, p_wk->heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  YESNOウィンドウメイン
 *
 *  @param  p_wk  ワーク
 *
 * @retval  "BMPMENU_NULL 選択されていない"
 * @retval  "0        はいを選択"
 * @retval  "BMPMENU_CANCEL いいえorキャンセル"
 */
//-----------------------------------------------------------------------------
static u32 ConfigYesNoWinMain( CONFIG_MAIN_DAT* p_wk )
{
  return BmpMenu_YesNoSelectMain( p_wk->pYesNoWork );
//  return BmpYesNoSelectMain( p_wk->pYesNoWork, p_wk->heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  サムネイル描画
 *
 *  @param  wk    ワーク
 *  @param  line  ライン
 *  @param  allput  一気にひょうじするか
 */
//-----------------------------------------------------------------------------
static void ConfigBmpWinWriteSum( CONFIG_MAIN_DAT* wk,u16 line, BOOL allput )
{
  static const u8 line_msg[ MENU_LINE_MAX ] = {
    mes_config_comment01,
    mes_config_comment04,
    mes_config_comment02,
    mes_config_comment03,
    mes_config_comment05,
#if USE_KANJI_MODE
    mes_config_comment11,
#else
    mes_config_comment06,
#endif
    mes_config_comment10,
  };

  //サムネイル
  ConfigBmpWinWriteMessege( wk, line_msg[line], allput );
}

