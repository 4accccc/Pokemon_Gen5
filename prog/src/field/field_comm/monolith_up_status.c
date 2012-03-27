//==============================================================================
/**
 * @file    monolith_up_status.c
 * @brief   モノリス：上画面の情報(statusから移植)
 * @author  matsuda
 * @date    2009.12.04(金)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "monolith_common.h"
#include "arc_def.h"
#include "monolith_tool.h"
#include "msg/msg_monolith.h"
#include "monolith_snd_def.h"
#include "field/monolith_tool_notwifi.h"

#include "monolith.naix"
#include "../../../../resource/fldmapdata/flagwork/flag_define.h" //SYS_FLAG_


//==============================================================================
//  定数定義
//==============================================================================
///BMPWIN
enum{
  BMPWIN_TITLE,               ///<タイトル
  BMPWIN_LEVEL,               ///<モノリスレベル
  BMPWIN_WBLV,                ///<ホワイト＆ブラックのレベル
  BMPWIN_USE_GPOWER,          ///<使えるGパワー数
  
  BMPWIN_MAX,
};

///BMPWINのサイズ定義など
enum{
  BMPWIN_POS_TITLE_X = 2,
  BMPWIN_POS_TITLE_Y = 0,
  BMPWIN_POS_TITLE_SIZE_X = 32 - BMPWIN_POS_TITLE_X,
  BMPWIN_POS_TITLE_SIZE_Y = 3,
  
  //合計レベル
  BMPWIN_POS_LEVEL_X = 2,
  BMPWIN_POS_LEVEL_Y = 8,
  BMPWIN_POS_LEVEL_SIZE_X = 32 - BMPWIN_POS_LEVEL_X,
  BMPWIN_POS_LEVEL_SIZE_Y = 2,
  
  //ホワイト＆ブラックのレベル
  BMPWIN_POS_WBLV_X = 2,
  BMPWIN_POS_WBLV_Y = BMPWIN_POS_LEVEL_Y + 3,
  BMPWIN_POS_WBLV_SIZE_X = 32 - BMPWIN_POS_WBLV_X,
  BMPWIN_POS_WBLV_SIZE_Y = 2,
  
  //使えるGパワーの数
  BMPWIN_POS_USE_GPOWER_X = 2,
  BMPWIN_POS_USE_GPOWER_Y = BMPWIN_POS_WBLV_Y + 3,
  BMPWIN_POS_USE_GPOWER_SIZE_X = 32 - BMPWIN_POS_USE_GPOWER_X,
  BMPWIN_POS_USE_GPOWER_SIZE_Y = 2,
};

//==============================================================================
//  構造体定義
//==============================================================================
///ミッション説明画面制御ワーク
typedef struct{
  GFL_BMPWIN *bmpwin[BMPWIN_MAX];       ///< bmpwin
  PRINT_UTIL print_util[BMPWIN_MAX];    ///< printutil
  MONOLITH_CANCEL_ICON cancel_icon;     ///< キャンセルアイコン用
}MONOLITH_UPSTATUS_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT MonolithUpStatusProc_Init(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithUpStatusProc_Main(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT MonolithUpStatusProc_End(
  GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static void _UpSetup_BGFrameSetting( MONOLITH_UPSTATUS_WORK *msw );
static void _UpSetup_BGFrameExit( MONOLITH_UPSTATUS_WORK *msw );
static void _UpSetup_BGGraphicLoad( MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_UPSTATUS_WORK *msw);
static void _UpSetup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_UPSTATUS_WORK *msw);
static void _UpSetup_BmpWin_Exit(MONOLITH_UPSTATUS_WORK *msw);
static void _UpWrite_Status(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_UPSTATUS_WORK *msw);

//==============================================================================
//  データ
//==============================================================================
///モノリスパワーROCデータ
const GFL_PROC_DATA MonolithAppProc_Up_Status = {
  MonolithUpStatusProc_Init,
  MonolithUpStatusProc_Main,
  MonolithUpStatusProc_End,
};

///タッチ範囲テーブル
static const GFL_UI_TP_HITTBL StatusTouchRect[] = {
  {
    CANCEL_POS_Y - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_Y + CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X - CANCEL_TOUCH_RANGE_HALF,
    CANCEL_POS_X + CANCEL_TOUCH_RANGE_HALF,
  },
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};

// メッセージのID
static const s16 upstatus_mes_tbl[] = {
  msg_mono_title_000,	// ハイルツリー
  msg_mono_st_000,		// ハイルツリーのごうけいLV
  msg_mono_st_004,		// ブラック
  msg_mono_st_003,		// ホワイト
  msg_mono_st_005,		// 使えるGパワー
};

//==============================================================================
//
//  
//
//==============================================================================
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
static GFL_PROC_RESULT MonolithUpStatusProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_UPSTATUS_WORK *msw = mywk;
	ARCHANDLE *hdl;
  
  switch(*seq){
  case 0:
    msw = GFL_PROC_AllocWork(proc, sizeof(MONOLITH_UPSTATUS_WORK), HEAPID_MONOLITH);
    GFL_STD_MemClear(msw, sizeof(MONOLITH_UPSTATUS_WORK));
    
    //BG
    _UpSetup_BGFrameSetting( msw );
    _UpSetup_BGGraphicLoad(appwk, appwk->setup, msw);
    _UpSetup_BmpWin_Create(appwk->setup, msw);
    
    //メッセージ描画
    _UpWrite_Status(appwk, appwk->setup, msw);
    
    (*seq)++;
    break;
  case 1:
    {
      int i, no_trans = FALSE;
      for(i = 0; i < BMPWIN_MAX; i++){
        if(PRINT_UTIL_Trans(&msw->print_util[i], appwk->setup->printQue) == FALSE){
          no_trans = TRUE;
        }
      }
      if(no_trans == FALSE){
        GFL_BG_SetVisible(GFL_BG_FRAME0_M, VISIBLE_ON);
        GFL_BG_SetVisible(GFL_BG_FRAME2_M, VISIBLE_ON);
        GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
        return GFL_PROC_RES_FINISH;
      }
    }
    break;
  }
  
  return GFL_PROC_RES_CONTINUE;
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
static GFL_PROC_RESULT MonolithUpStatusProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_UPSTATUS_WORK *msw = mywk;
  int tp_ret;
  int i;

  if(appwk->force_finish == TRUE){
    return GFL_PROC_RES_FINISH;
  }
  
  for(i = 0; i < BMPWIN_MAX; i++){
    PRINT_UTIL_Trans(&msw->print_util[i], appwk->setup->printQue);
  }
  
  switch(*seq){
  case 0:
    {
      if(appwk->up_proc_finish == TRUE){
        return GFL_PROC_RES_FINISH;
      }
    }
    break;
  //
  default:
    break;
  }
  
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
static GFL_PROC_RESULT MonolithUpStatusProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
  MONOLITH_APP_PARENT *appwk = pwk;
	MONOLITH_UPSTATUS_WORK *msw = mywk;

  if(PRINTSYS_QUE_IsFinished(appwk->setup->printQue) == FALSE){
    return GFL_PROC_RES_CONTINUE;
  }
  
  GFL_DISP_GX_SetVisibleControlDirect(GX_PLANEMASK_BG3);
  
  //BG
  _UpSetup_BmpWin_Exit(msw);
  _UpSetup_BGFrameExit(msw);
  
  GFL_PROC_FreeWork(proc);
  return GFL_PROC_RES_FINISH;
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   BGフレーム設定
 *
 * @param	MONOLITH_UPSTATUS	*msw
 */
//--------------------------------------------------------------
static void _UpSetup_BGFrameSetting( MONOLITH_UPSTATUS_WORK *msw )
{
	static const GFL_BG_BGCNT_HEADER bgcnt_frame[] = {
		{//GFL_BG_FRAME0_S
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		},
		{//GFL_BG_FRAME2_S
			0, 0, MONO_BG_COMMON_SCR_SIZE, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x2000, MONO_BG_COMMON_CHARBASE, MONO_BG_COMMON_CHAR_SIZE,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		},
	};
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( GFL_BG_FRAME2_M,   &bgcnt_frame[1],   GFL_BG_MODE_TEXT );
    
    GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_FillScreen( GFL_BG_FRAME2_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
}

//--------------------------------------------------------------
/**
 * BGフレーム解放
 *
 * @param	MONOLITH_UPSTATUS	*msw
 */
//--------------------------------------------------------------
static void _UpSetup_BGFrameExit( MONOLITH_UPSTATUS_WORK *msw )
{
  GFL_BG_SetVisible(GFL_BG_FRAME0_M, VISIBLE_OFF);
  GFL_BG_SetVisible(GFL_BG_FRAME2_M, VISIBLE_OFF);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_M);
  GFL_BG_FreeBGControl(GFL_BG_FRAME2_M);
}

//--------------------------------------------------------------
/**
 * @brief   BGグラフィックをVRAMへ転送
 *
 * @param	MONOLITH_APP_PARENT	*appwk
 * @param	MONOLITH_SETUP		*setup
 * @param	MONOLITH_UPSTATUS	*msw
 */
//--------------------------------------------------------------
static void _UpSetup_BGGraphicLoad( MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_UPSTATUS_WORK *msw)
{
  GFL_ARCHDL_UTIL_TransVramScreen(setup->hdl, NARC_monolith_mono_bgu_map_lz_NSCR, 
    GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_MONOLITH);
  
  // レベルゲージ転送
  MonolithTool_BalanceGaugeRewrite( appwk, BALANCE_GAUGE_UP, GFL_BG_FRAME2_M);
  //
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
}

//--------------------------------------------------------------
/**
 * BMPWIN作成
 *
 * @param	MONOLITH_SETUP		*setup
 * @param	MONOLITH_UPSTATUS	*msw
 */
//--------------------------------------------------------------
static void _UpSetup_BmpWin_Create(MONOLITH_SETUP *setup, MONOLITH_UPSTATUS_WORK *msw)
{
  u8  frm;
  int i;
  
  frm = GFL_BG_FRAME0_M;
  //タイトル
  msw->bmpwin[BMPWIN_TITLE] = GFL_BMPWIN_Create( frm, 
    BMPWIN_POS_TITLE_X, BMPWIN_POS_TITLE_Y, 
    BMPWIN_POS_TITLE_SIZE_X, BMPWIN_POS_TITLE_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_TITLE] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_TITLE] );

  //レベル合計
  msw->bmpwin[BMPWIN_LEVEL] = GFL_BMPWIN_Create( frm, 
    BMPWIN_POS_LEVEL_X, BMPWIN_POS_LEVEL_Y, 
    BMPWIN_POS_LEVEL_SIZE_X, BMPWIN_POS_LEVEL_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_LEVEL] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_LEVEL] );

  //ブラックとホワイト
  msw->bmpwin[BMPWIN_WBLV] = GFL_BMPWIN_Create( frm, 
    BMPWIN_POS_WBLV_X, BMPWIN_POS_WBLV_Y, 
    BMPWIN_POS_WBLV_SIZE_X, BMPWIN_POS_WBLV_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_WBLV] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_WBLV] );

  //使えるデルパワー
  msw->bmpwin[BMPWIN_USE_GPOWER] = GFL_BMPWIN_Create( frm, 
    BMPWIN_POS_USE_GPOWER_X, BMPWIN_POS_USE_GPOWER_Y, 
    BMPWIN_POS_USE_GPOWER_SIZE_X, BMPWIN_POS_USE_GPOWER_SIZE_Y, 
    MONOLITH_BG_DOWN_FONT_PALNO, GFL_BMP_CHRAREA_GET_B);
  GFL_BMPWIN_TransVramCharacter( msw->bmpwin[BMPWIN_USE_GPOWER] );
  GFL_BMPWIN_MakeScreen( msw->bmpwin[BMPWIN_USE_GPOWER] );

  for(i = 0; i < BMPWIN_MAX; i++){
    PRINT_UTIL_Setup(&msw->print_util[i], msw->bmpwin[i]);
  }
  GFL_BG_LoadScreenReq( frm );
}

//--------------------------------------------------------------
/**
 * BMPWIN削除
 *
 * @param	MONOLITH_UPSTATUS	*msw
 */
//--------------------------------------------------------------
static void _UpSetup_BmpWin_Exit(MONOLITH_UPSTATUS_WORK *msw)
{
  int i;
  
  for(i = 0; i < BMPWIN_MAX; i++){
    GFL_BMPWIN_Delete(msw->bmpwin[i]);
  }
}

//--------------------------------------------------------------
/**
 * 各種情報表示
 *
 * @param	MONOLITH_APP_PARENT	*appwk
 * @param	MONOLITH_SETUP		*setup
 * @param	MONOLITH_UPSTATUS	*msw
 */
//--------------------------------------------------------------
static void _UpWrite_Status(MONOLITH_APP_PARENT *appwk, MONOLITH_SETUP *setup, MONOLITH_UPSTATUS_WORK *msw)
{
  STRBUF *strbuf, *expand_strbuf;
  const OCCUPY_INFO *occupy;
  MYSTATUS *myst = MonolithTool_GetMystatus(appwk);
  //
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( appwk->parent->gsys );
  EVENTWORK	*ev = GAMEDATA_GetEventWork( gamedata );
  BOOL	bflg;
  // チュートリアル中かどうか(BTSGF60対応)
  bflg = EVENTWORK_CheckEventFlag( ev, SYS_FLAG_FES_TUTORIAL_CLEAR );
  //
  occupy = MonolithTool_GetOccupyInfo(appwk);
  
  strbuf = GFL_STR_CreateBuffer(256, HEAPID_MONOLITH);
  expand_strbuf = GFL_STR_CreateBuffer( 256, HEAPID_MONOLITH );

  //タイトル
  GFL_MSG_GetString(setup->mm_monolith, upstatus_mes_tbl[0], strbuf);
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_TITLE], setup->printQue, 
    0, 4, strbuf, setup->font_handle);
  
  //ハイルツリーのレベル
  GFL_MSG_GetString(setup->mm_monolith, upstatus_mes_tbl[1], strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, occupy->black_level + occupy->white_level, 5, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT); // level + 1 = 表示を1origin化
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_LEVEL], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  
  //ブラックのレベル
  GFL_MSG_GetString(setup->mm_monolith, upstatus_mes_tbl[2], strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, occupy->black_level, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_WBLV], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  //ホワイトのレベル
  GFL_MSG_GetString(setup->mm_monolith, upstatus_mes_tbl[3], strbuf);
  WORDSET_RegisterNumber(setup->wordset, 0, occupy->white_level, 4, 
    STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_WBLV], setup->printQue, 
    BMPWIN_POS_WBLV_SIZE_X / 2 * 8, 0, expand_strbuf, setup->font_handle);
  
  //使えるGパワーの数
  GFL_MSG_GetString(setup->mm_monolith, upstatus_mes_tbl[4], strbuf);
  // チュートリアルクリアしているかどうか
  if(bflg == TRUE){
    WORDSET_RegisterNumber(setup->wordset, 0, 
      MonolithToolEx_CountUsePower(
      setup->powerdata, MonolithTool_GetOccupyInfo(appwk), 
      appwk->parent->monolith_status.gpower_distribution_bit), 
      4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  }
  // してない => 0個
  else{
    WORDSET_RegisterNumber(setup->wordset, 0, 0, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
  }
  WORDSET_ExpandStr( setup->wordset, expand_strbuf, strbuf );
  PRINT_UTIL_Print(&msw->print_util[BMPWIN_USE_GPOWER], setup->printQue, 
    0, 0, expand_strbuf, setup->font_handle);
  
  GFL_STR_DeleteBuffer(strbuf);
  GFL_STR_DeleteBuffer(expand_strbuf);
}

